/* Copyright 2021, Bloomberg Finance L.P.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:  The above
 * copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#include <blpapi_correlationid.h>
#include <blpapi_event.h>
#include <blpapi_exception.h>
#include <blpapi_message.h>
#include <blpapi_names.h>
#include <blpapi_session.h>

#include <util/ConnectionAndAuthOptions.h>
#include <util/SubscriptionOptions.h>
#include <util/Utils.h>

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <map>
#include <string>
#include <thread>

using namespace BloombergLP;
using namespace blpapi;

// Requests are throttled in the infrastructure. Snapshot RequestTemplate(s)
// sends one resolution request per topic (unlike normal subscriptions where
// multiple topics are resolved at once), which is likely to cause request
// throttling. Therefore, it is recommended to send RequestTemplate(s) in
// batches and limit the batch size to 50.
const int k_templateBatchSize = 50;

class SnapshotRequestTemplateExample : public EventHandler {
    ArgParser d_parser;
    ConnectionAndAuthOptions d_connectionAndAuthOptions;
    SubscriptionOptions d_subscriptionOptions;
    std::map<CorrelationId, std::string> d_topicMap;

    mutable std::mutex d_mutex;
    std::condition_variable d_templateBatchingCondition;
    size_t d_templateCount;

    std::map<CorrelationId, RequestTemplate> d_requestTemplates;

    std::condition_variable d_responseCondition;
    size_t d_responseCount;
    bool running;

    bool parseCommandLine(int argc, const char **argv)
    {
        try {
            d_parser.parse(argc, argv);
        } catch (const std::exception& ex) {
            std::cerr << "Failed to parse arguments: " << ex.what() << "\n";
            d_parser.printHelp();
            return false;
        }

        return true;
    }

  public:
    SnapshotRequestTemplateExample()
        : d_parser("Snapshot Request Example", "SnapshotRequestTemplate")
        , d_connectionAndAuthOptions(d_parser)
        , d_subscriptionOptions(d_parser, true)
        , d_templateCount(0)
        , d_responseCount(0)
        , running(true)
    {
    }

    void createTemplates(Session& session)
    {
        // NOTE: resources used by a snapshot request template are released
        // only when 'RequestTemplateTerminated' message is received or when
        // the session is destroyed. In order to release resources when request
        // template is not needed anymore, users should call 'Session::cancel'
        // and pass the 'CorrelationId' for the request template, or call
        // 'RequestTemplate::close'. If the 'Session::cancel' is used, all
        // outstanding requests are canceled and the underlying subscription is
        // closed immediately. If the template is closed with
        // 'RequestTemplate::close', the underlying subscription is closed only
        // when all outstanding requests are served.
        std::unique_lock<std::mutex> ul(d_mutex);
        d_responseCount = d_subscriptionOptions.getTopics().size();
        std::map<std::string, std::string> subscriptionStrings
                = d_subscriptionOptions.createSubscriptionStrings();
        for (const auto& entry : subscriptionStrings) {
            d_templateBatchingCondition.wait(ul, [this] {
                return !running || d_templateCount < k_templateBatchSize;
            });

            if (!running) {
                break;
            }

            const std::string& userTopic = entry.first;
            const std::string& subscriptionString = entry.second;
            std::cout << "Creating snapshot request template for " << userTopic
                      << "\n";

            CorrelationId cid(Utils::getNextIntegerCid());
            d_topicMap[cid] = userTopic;

            ++d_templateCount;
            d_requestTemplates[cid] = session.createSnapshotRequestTemplate(
                    subscriptionString.c_str(), cid);
        }

        // Wait until all request templates have finished, either success or
        // failure.
        d_responseCondition.wait(
                ul, [this] { return !running || d_responseCount == 0; });

        if (!running) {
            return;
        }

        std::cout << "All the request templates are created\n";
    }

    void sendRequests(Session& session)
    {
        while (true) {
            {
                std::unique_lock<std::mutex> ul(d_mutex);
                if (!running) {
                    break;
                }

                std::cout << "Sending snapshot request using the request "
                             "templates\n";

                d_responseCount = d_requestTemplates.size();
                for (const auto& keyVal : d_requestTemplates) {
                    CorrelationId cid(Utils::getNextIntegerCid());
                    const auto& userTopic = d_topicMap[keyVal.first];
                    d_topicMap[cid] = userTopic;

                    std::cout << "Sending request for " << userTopic << "\n";
                    session.sendRequest(keyVal.second, cid);
                }

                std::cout << "Waiting for all the responses..., Press "
                             "[Ctrl-C] to "
                             "exit\n";
                d_responseCondition.wait(ul,
                        [this] { return !running || d_responseCount == 0; });
            }

            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

    bool processEvent(const Event& event, Session *session) override
    {
        const auto eventType = event.eventType();
        MessageIterator it(event);
        while (it.next()) {
            auto msg = it.message();
            Name messageType = msg.messageType();
            auto cid = msg.correlationId();

            if (Names::requestTemplateAvailable() == messageType) {
                std::cout << "Request template is successfully created for "
                          << getTopic(cid) << "\n";

                std::lock_guard<std::mutex> lg(d_mutex);

                // Decrease template count
                --d_templateCount;
                d_templateBatchingCondition.notify_one();

                // Decrease response count
                --d_responseCount;
                d_responseCondition.notify_one();
            } else if (Names::requestTemplateTerminated() == messageType) {
                // Will also receive a 'RequestFailure' message preceding
                // 'RequestTemplateTerminated' for every pending request.
                std::cout << "Request template terminated for "
                          << getTopic(cid) << "\n";
                msg.print(std::cout);

                std::lock_guard<std::mutex> lg(d_mutex);

                // Remove the template
                d_requestTemplates.erase(cid);

                // Decrease template count
                --d_templateCount;
                d_templateBatchingCondition.notify_one();

                // Decrease response count
                --d_responseCount;
                d_responseCondition.notify_one();
            } else if (Event::PARTIAL_RESPONSE == eventType) {
                std::cout << "Received partial response for " << getTopic(cid)
                          << "\n";
                msg.print(std::cout);
            } else if (Event::RESPONSE == eventType) {
                std::cout << "Received response for " << getTopic(cid) << "\n";
                msg.print(std::cout);

                std::lock_guard<std::mutex> lg(d_mutex);

                // Decrease response count
                --d_responseCount;
                d_responseCondition.notify_one();
            } else if (messageType == Names::sessionTerminated()) {
                // Session could be terminated due to session identity
                // revocation.
                std::cout << "Session terminated, stopping...\n";
                std::lock_guard<std::mutex> lg(d_mutex);
                running = false;
                d_templateBatchingCondition.notify_one();
                d_responseCondition.notify_one();
            }
        }

        return true;
    }

    std::string getTopic(const CorrelationId& cid) const
    {
        const auto it = d_topicMap.find(cid);
        if (it != d_topicMap.end()) {
            return it->second;
        }

        return "topic-not-found";
    }

    void run(int argc, const char **argv)
    {
        if (!parseCommandLine(argc, argv)) {
            return;
        }

        auto sessionOptions = d_connectionAndAuthOptions.createSessionOption();
        d_subscriptionOptions.setUpSessionOptions(sessionOptions);
        Session session(sessionOptions, this);

        if (!session.start()) {
            std::cout << "Failed to start session\n";
            return;
        }

        if (!session.openService(d_subscriptionOptions.getService().c_str())) {
            std::cout << "Failed to open service '"
                      << d_subscriptionOptions.getService() << "'\n";
            session.stop();
            return;
        }

        createTemplates(session);

        if (d_requestTemplates.empty()) {
            std::cout << "Failed to create all the request templates, "
                         "stopping...\n";
        } else {
            sendRequests(session);
        }

        session.stop();
    }
};

int main(int argc, const char **argv)
{
    SnapshotRequestTemplateExample example;
    try {
        example.run(argc, argv);
    } catch (const Exception& e) {
        std::cerr << ">>> Exception caught: " << e.description() << "\n";
    }
    return 0;
}
