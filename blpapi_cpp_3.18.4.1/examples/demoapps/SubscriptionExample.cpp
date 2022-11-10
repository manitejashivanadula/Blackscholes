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
#include <blpapi_message.h>
#include <blpapi_name.h>
#include <blpapi_names.h>
#include <blpapi_session.h>
#include <blpapi_subscriptionlist.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>

#include <util/ConnectionAndAuthOptions.h>
#include <util/SubscriptionOptions.h>
#include <util/Utils.h>

using namespace BloombergLP;
using namespace blpapi;

namespace {

class SessionEventHandler : public EventHandler {
    const std::unordered_map<int, std::string>& d_topicMap;
    void processSubscriptionStatus(const Event& event);
    void processSubscriptionDataEvent(const Event& event);
    void processGenericMessage(const Event& event);

  public:
    SessionEventHandler(const std::unordered_map<int, std::string>& topicMap)
        : d_topicMap(topicMap)
    {
    }

    bool processEvent(const Event& event, Session *providerSession) override;
};

void SessionEventHandler::processSubscriptionStatus(const Event& event)
{
    MessageIterator msgIter(event);
    while (msgIter.next()) {
        Message msg = msgIter.message();
        auto topicId = msg.correlationId().asInteger();
        auto topic = d_topicMap.at(topicId);
        std::cout << Utils::getFormattedCurrentTime() << ": " << topic << "\n";
        msg.print(std::cout) << "\n";

        const Name& messageType = msg.messageType();
        if (Names::subscriptionFailure() == messageType) {
            std::cout << "Subscription for " << topic << " failed";
        } else if (Names::subscriptionTerminated() == messageType) {
            // Subscription can be terminated if the session identity is
            // revoked.
            std::cout << "Subscription for " << topic << " terminated";
        }
    }
}

void SessionEventHandler::processSubscriptionDataEvent(const Event& event)
{
    MessageIterator msgIter(event);
    while (msgIter.next()) {
        Message msg = msgIter.message();
        auto topicId = msg.correlationId().asInteger();
        auto topic = d_topicMap.at(topicId);
        std::cout << Utils::getFormattedCurrentTime() << ": " << topic << "\n";
        msg.print(std::cout) << "\n";
    }
}

void SessionEventHandler::processGenericMessage(const Event& event)
{
    MessageIterator msgIter(event);
    while (msgIter.next()) {
        Message msg = msgIter.message();

        const Name& messageType = msg.messageType();
        if (Names::slowConsumerWarning() == messageType) {
            std::cout << Names::slowConsumerWarning()
                      << " - The event queue is "
                      << "beginning to approach its maximum capacity and "
                      << "the application is not processing the data fast "
                      << "enough. This could lead to ticks being dropped"
                      << " (DataLoss).\n";
        } else if (Names::slowConsumerWarningCleared() == messageType) {
            std::cout << Names::slowConsumerWarningCleared() << " - the event "
                      << "queue has shrunk enough that there is no "
                      << "longer any immediate danger of overflowing the "
                      << "queue. If any precautionary actions were taken "
                      << "when SlowConsumerWarning message was delivered, "
                      << "it is now safe to continue as normal.\n";
        } else if (Names::dataLoss() == messageType) {
            msg.print(std::cout) << "\n";

            auto topicId = msg.correlationId().asInteger();
            auto topic = d_topicMap.at(topicId);
            std::cout << Names::dataLoss()
                      << " - The application is too slow to "
                      << "process events and the event queue is overflowing. "
                      << "Data is lost for topic " << topic << ".\n";
        } else if (event.eventType() == Event::SESSION_STATUS) {
            if (msg.messageType() == Names::sessionTerminated()) {
                std::cout << "Session terminated\n";
                return;
            } else {
                msg.print(std::cout) << "\n";
            }
        } else {
            msg.print(std::cout) << "\n";
        }
    }
}

bool SessionEventHandler::processEvent(
        const Event& event, Session *providerSession)
{
    const Event::EventType eventType = event.eventType();

    std::cout << "Processing " << eventType << "\n";

    try {
        switch (eventType) {
        case Event::SUBSCRIPTION_DATA:
            processSubscriptionDataEvent(event);
            break;
        case Event::SUBSCRIPTION_STATUS:
            processSubscriptionStatus(event);
            break;
        default:
            processGenericMessage(event);
            break;
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to process event " << eventType << ":"
                  << e.what();
    }

    return true;
}
}

class SubscriptionExample {
    static const int DEFAULT_EVENT_QUEUE_SIZE = 10000;
    ArgParser d_argParser;
    ConnectionAndAuthOptions d_connectionAndAuthOptions;
    SubscriptionOptions d_subscriptionOptions;
    int d_eventQueueSize;
    std::unordered_map<int, std::string> d_topicMap;

    bool parseCommandLine(int argc, const char **argv)
    {
        try {
            d_argParser.addArg("event-queue-size", 'q')
                    .setMetaVar("eventQueueSize")
                    .setDescription("The maximum number of events that is "
                                    "buffered by the session")
                    .setDefaultValue(std::to_string(DEFAULT_EVENT_QUEUE_SIZE))
                    .setAction([this](const char *value) {
                        d_eventQueueSize = std::stol(value);
                    });

            d_argParser.parse(argc, argv);
        } catch (const std::exception& ex) {
            std::cerr << "Failed to parse arguments: " << ex.what() << "\n";
            d_argParser.printHelp();
            return false;
        }

        return true;
    }

  public:
    SubscriptionExample()
        : d_argParser("Asynchronous subscription with event handler",
                "SubscriptionExample")
        , d_connectionAndAuthOptions(d_argParser)
        , d_subscriptionOptions(d_argParser)
        , d_eventQueueSize()
    {
    }

    void run(int argc, const char **argv)
    {
        if (!parseCommandLine(argc, argv)) {
            return;
        }

        SessionOptions sessionOptions
                = d_connectionAndAuthOptions.createSessionOption();
        d_subscriptionOptions.setUpSessionOptions(sessionOptions);
        sessionOptions.setMaxEventQueueSize(d_eventQueueSize);

        SessionEventHandler handler(d_topicMap);
        Session session(sessionOptions, &handler);

        if (!session.start()) {
            std::cout << "Failed to start session\n";
            Utils::checkFailures(session);
            return;
        }

        const std::string& serviceName = d_subscriptionOptions.getService();
        if (!session.openService(serviceName.c_str())) {
            std::cout << ">>> Failed to open " << serviceName << "\n";
            Utils::checkFailures(session);
            session.stop();
            return;
        }

        std::cout << "Subscribing...\n";
        const SubscriptionList subscriptions
                = d_subscriptionOptions.createSubscriptionList(
                        [this](size_t topicId, const std::string& topic) {
                            d_topicMap[topicId] = topic;
                            return CorrelationId(topicId);
                        });

        session.subscribe(subscriptions);

        std::cout << "Press ENTER to quit\n";
        std::cin.get();
        session.stop();
    }
};

int main(int argc, const char **argv)
{
    SubscriptionExample example;
    try {
        example.run(argc, argv);
    } catch (const Exception& e) {
        std::cerr << ">>> Exception caught: " << e.description() << "\n";
    }
    return 0;
}
