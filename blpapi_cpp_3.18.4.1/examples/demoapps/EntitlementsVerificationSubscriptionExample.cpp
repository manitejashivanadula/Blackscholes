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
#include <blpapi_exception.h>
#include <blpapi_message.h>
#include <blpapi_name.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <util/ArgParser.h>
#include <util/ConnectionAndAuthOptions.h>
#include <util/SubscriptionOptions.h>
#include <util/Utils.h>
#include <util/events/SessionRouter.h>

using namespace BloombergLP;
using namespace blpapi;

class EntitlementsVerificationSubscriptionExample {
    std::atomic<ExampleState> d_exampleState;
    ArgParser d_parser;
    ConnectionAndAuthOptions d_connectionAndAuthOptions;
    SubscriptionOptions d_subOptions;
    SessionRouter<Session> d_sessionRouter;
    std::map<CorrelationId, Identity> d_cidToIdentity;
    const Name d_eid;

    std::map<CorrelationId, std::string> d_cidToStr;
    // Map contains 'CorrelationId' to 'UserIdentifier' mapping and
    // 'CorrelationId' to 'Topic' mapping.

    bool parseCommandLine(int argc, const char **argv)
    {
        try {
            d_parser.parse(argc, argv);

            if (d_connectionAndAuthOptions.numClientServerSetupAuthOptions()
                    == 0) {
                std::cerr << "No userId:IP or token specified.\n";
                d_parser.printHelp();
                return false;
            }

            bool hasEid = false;
            for (auto const& field : d_subOptions.getFields()) {
                if (field == d_eid.string()) {
                    hasEid = true;
                    break;
                }
            }

            if (!hasEid) {
                d_subOptions.addField(d_eid.string());
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse arguments: " << e.what() << "\n";
            d_parser.printHelp();
            return false;
        }

        return true;
    }

    void stop(Session *session)
    {
        // Cancel all the authorized identities.
        std::vector<CorrelationId> cids;
        std::for_each(d_cidToIdentity.begin(),
                d_cidToIdentity.end(),
                [&cids](const std::pair<CorrelationId, Identity>& keyVal) {
                    cids.emplace_back(keyVal.first);
                });

        if (!cids.empty()) {
            session->cancel(&cids[0], cids.size());
        }

        try {
            session->stopAsync();
        } catch (const blpapi::Exception& e) {
            std::cerr << e.description() << "\n";
        }
    }

  public:
    EntitlementsVerificationSubscriptionExample()
        : d_exampleState(ExampleState::STARTING)
        , d_parser("Entitlements Verification Subscription Example",
                  "EntitlementsVerificationSubscriptionExample")
        , d_connectionAndAuthOptions(d_parser, true)
        , d_subOptions(d_parser)
        , d_eid("EID")
    {
    }

    void exceptionHandler(
            Session *session, const Event&, const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        stop(session);
    }

    void authorizationSuccessHandler(
            Session *session, const Event&, const Message& message)
    {
        CorrelationId cid = message.correlationId();
        const auto it = d_cidToStr.find(cid);

        if (it == d_cidToStr.end()) {
            return;
        }

        const std::string& userIdentifier = it->second;

        std::cout << "Successfully authorized " << userIdentifier << "\n";
        Identity id = session->getAuthorizedIdentity(cid);
        d_cidToIdentity[cid] = id;

        // Deliver init paint to the user. For the purpose of simplicity, this
        // example doesn't maintain an init paint cache.
    }

    void authorizationFailureHandler(
            Session *, const Event&, const Message& message)
    {
        CorrelationId cid = message.correlationId();
        d_sessionRouter.deregisterMessageHandler(cid);

        const auto it = d_cidToStr.find(cid);

        if (it == d_cidToStr.end()) {
            return;
        }

        std::cerr << "Failed to authorize " << it->second << "\n";
    }

    void authorizationRevokedHandler(
            Session *, const Event&, const Message& message)
    {
        CorrelationId cid = message.correlationId();
        d_sessionRouter.deregisterMessageHandler(cid);
        const auto it = d_cidToStr.find(cid);

        if (it == d_cidToStr.end()) {
            return;
        }

        std::cerr << "Authorization revoked for " << it->second << "\n";

        d_cidToIdentity.erase(cid);
    }

    void authorizeUsers(Session *session)
    {
        std::map<std::string, AuthOptions> authOptionList
                = d_connectionAndAuthOptions
                          .createClientServerSetupAuthOptions();

        for (const auto& idAndAuthOptions : authOptionList) {
            const std::string& userIdentifier = idAndAuthOptions.first;
            const AuthOptions& authOptions = idAndAuthOptions.second;
            CorrelationId cid(Utils::getNextIntegerCid());
            d_cidToStr[cid] = userIdentifier;
            session->generateAuthorizedIdentity(authOptions, cid);
        }
    }

    void openServices(Session *session)
    {
        session->openServiceAsync(d_subOptions.getService().c_str());
    }

    void sessionStartedHandler(Session *session, const Event&, const Message&)
    {
        // Add the authorization message handlers after the session started to
        // only react to authorization messages of users, i.e., avoid those of
        // session identity.

        d_exampleState = ExampleState::STARTED;
        d_sessionRouter.registerMessageHandler(Names::authorizationSuccess(),
                [this](Session *s, const Event& e, const Message& m) {
                    authorizationSuccessHandler(s, e, m);
                });

        d_sessionRouter.registerMessageHandler(Names::authorizationFailure(),
                [this](Session *s, const Event& e, const Message& m) {
                    authorizationFailureHandler(s, e, m);
                });

        d_sessionRouter.registerMessageHandler(Names::authorizationRevoked(),
                [this](Session *s, const Event& e, const Message& m) {
                    authorizationRevokedHandler(s, e, m);
                });

        authorizeUsers(session);
        openServices(session);
    }

    void sessionStartupFailureHandler(Session *, const Event&, const Message&)
    {
        std::cerr << "Failed to start the session. Exiting...\n";
        d_exampleState = ExampleState::TERMINATED;
    }

    void sessionTerminatedHandler(Session *, const Event&, const Message&)
    {
        d_exampleState = ExampleState::TERMINATED;
    }

    void serviceOpenedHandler(
            Session *session, const Event&, const Message& message)
    {
        const std::string serviceName(
                message.getElementAsString("serviceName"));
        Service service = session->getService(serviceName.c_str());

        if (serviceName != this->d_subOptions.getService()) {
            std::cout << "A service was opened: " << serviceName << "\n";
            return;
        }

        subscribe(session);
    }

    void serviceOpenFailureHandler(
            Session *session, const Event&, const Message& message)
    {
        const std::string serviceName(
                message.getElementAsString("serviceName"));
        if (serviceName == this->d_subOptions.getService()) {
            stop(session);
            return;
        }

        const std::string errorMsg
                = "A service which is unknown failed to open: " + serviceName;
        throw std::invalid_argument(errorMsg.c_str());
    }

    void entitlementChangedHandler(
            Session *, const Event&, const Message& message)
    {
        const auto it = d_cidToStr.find(message.correlationId());
        if (it == d_cidToStr.end()) {
            return;
        }

        // This is just informational. Continue to use existing identity.
        std::cout << "Entitlements updated for " << it->second << "\n";
    }

    void subscriptionDataHandler(Session *, const Event& event)
    {
        blpapi::MessageIterator msgIter(event);
        while (msgIter.next()) {
            blpapi::Message msg = msgIter.message();
            const auto idIt = d_cidToStr.find(msg.correlationId());
            if (idIt == d_cidToStr.end()) {
                return;
            }

            const std::string& topic = idIt->second;

            Service service = msg.service();
            if (!msg.hasElement(d_eid, true /* excludeNullElements */)) {
                // No Entitlements are required to access this data.
                std::cout << "No entitlements are required for: " << topic
                          << "\n";

                // Now distribute message to the user.
                return;
            }

            Element entitlements = msg.getElement(d_eid);
            std::vector<int> failedEntitlements;

            for (auto const& cidToIdentity : d_cidToIdentity) {
                failedEntitlements.clear();
                failedEntitlements.resize(entitlements.numValues());

                const auto itr = d_cidToStr.find(cidToIdentity.first);
                if (itr == d_cidToStr.end()) {
                    continue;
                }

                const std::string& userIdentifier = itr->second;
                Identity id = cidToIdentity.second;
                int failureCount = failedEntitlements.size();

                if (id.hasEntitlements(service,
                            entitlements,
                            &failedEntitlements[0],
                            &failureCount)) {
                    std::cout << userIdentifier
                              << " is entitled to get data for: " << topic
                              << "\n";

                    // Now distribute message to the user.
                } else {
                    std::cout << userIdentifier
                              << " is NOT entitled to get data"
                              << "for: " << topic << " - Failed eids: ";
                    for (int i = 0; i < failureCount; ++i) {
                        std::cout << (i == 0 ? "" : ", ")
                                  << failedEntitlements[i];
                    }
                    std::cout << "\n";
                }
            }
        }
    }

    void subscriptionFailureHandler(const char *prefix, const Message& m)
    {
        const auto it = d_cidToStr.find(m.correlationId());

        std::cout << prefix << (it != d_cidToStr.end() ? it->second : "")
                  << "\n";
    }

    void subscribe(Session *session)
    {
        d_sessionRouter.registerMessageHandler(Names::subscriptionFailure(),
                [this](Session *, const Event&, const Message& message) {
                    subscriptionFailureHandler(
                            "Subscription failed: ", message);
                });

        d_sessionRouter.registerMessageHandler(Names::subscriptionTerminated(),
                [this](Session *, const Event&, const Message& message) {
                    subscriptionFailureHandler(
                            "Subscription terminated: ", message);
                });

        d_sessionRouter.registerEventHandler(Event::SUBSCRIPTION_DATA,
                [this](Session *s, const Event& event) {
                    subscriptionDataHandler(s, event);
                });

        std::cout << "Subscribing...\n";
        auto const subscriptions = d_subOptions.createSubscriptionList(

                [this](size_t, const std::string& topic) {
                    CorrelationId cid(Utils::getNextIntegerCid());
                    d_cidToStr[cid] = topic;
                    return cid;
                });
        session->subscribe(subscriptions);
    }

    void registerCallbacks()
    {
        d_sessionRouter.registerExceptionHandler(
                [this](Session *session,
                        const Event& event,
                        const std::exception& exception) {
                    exceptionHandler(session, event, exception);
                });

        d_sessionRouter.registerMessageHandler(Names::sessionStarted(),
                [this](Session *session,
                        const Event& event,
                        const Message& message) {
                    sessionStartedHandler(session, event, message);
                });

        d_sessionRouter.registerMessageHandler(Names::sessionStartupFailure(),
                [this](Session *session,
                        const Event& event,
                        const Message& message) {
                    sessionStartupFailureHandler(session, event, message);
                });

        d_sessionRouter.registerMessageHandler(Names::sessionTerminated(),
                [this](Session *session,
                        const Event& event,
                        const Message& message) {
                    sessionTerminatedHandler(session, event, message);
                });

        d_sessionRouter.registerMessageHandler(Names::serviceOpened(),
                [this](Session *session,
                        const Event& event,
                        const Message& message) {
                    serviceOpenedHandler(session, event, message);
                });

        d_sessionRouter.registerMessageHandler(Names::serviceOpenFailure(),
                [this](Session *session,
                        const Event& event,
                        const Message& message) {
                    serviceOpenFailureHandler(session, event, message);
                });

        d_sessionRouter.registerMessageHandler(Name("EntitlementChanged"),
                [this](Session *session,
                        const Event& event,
                        const Message& message) {
                    entitlementChangedHandler(session, event, message);
                });
    }

    void run(int argc, const char **argv)
    {
        if (!parseCommandLine(argc, argv)) {
            return;
        }

        registerCallbacks();
        SessionOptions sessionOptions
                = d_connectionAndAuthOptions.createSessionOption();
        Session session(sessionOptions, &d_sessionRouter);
        session.startAsync();

        while (d_exampleState != ExampleState::TERMINATED) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
};

int main(int argc, const char **argv)
{
    EntitlementsVerificationSubscriptionExample example;
    try {
        example.run(argc, argv);
    } catch (Exception& e) {
        std::cerr << "Library Exception! " << e.description() << "\n";
    }

    // wait for enter key to exit application
    std::cout << "Press ENTER to quit\n";
    char dummy[2];
    std::cin.getline(dummy, 2);
    return 0;
}
