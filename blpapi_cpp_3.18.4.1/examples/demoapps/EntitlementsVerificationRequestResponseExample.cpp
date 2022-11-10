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
#include <iterator>
#include <string>
#include <thread>
#include <vector>

#include <util/ArgParser.h>
#include <util/ConnectionAndAuthOptions.h>
#include <util/Utils.h>
#include <util/events/SessionRouter.h>

using namespace BloombergLP;
using namespace blpapi;

namespace {
const char *const REF_DATA_SERVICE_NAME = "//blp/refdata";
} // anonymous namespace

class EntitlementsVerificationRequestResponseExample {
    std::atomic<ExampleState> d_exampleState;
    ArgParser d_parser;
    ConnectionAndAuthOptions d_connectionAndAuthOptions;
    SessionRouter<Session> d_sessionRouter;
    std::map<CorrelationId, Identity> d_cidToIdentity;
    std::vector<std::string> d_securities;
    std::map<CorrelationId, std::string> d_cidToStr;
    // Map contains 'CorrelationId' to 'UserIdentifier' mapping and
    // 'CorrelationId' to 'Topic' mapping.

    bool d_finalResponseReceived;
    std::vector<Event> d_responses;

    bool parseCommandLine(int argc, const char **argv)
    {
        try {
            d_parser.addArg("security", 'S')
                    .setMetaVar("security")
                    .setDefaultValue("IBM US Equity")
                    .setDescription("security used in ReferenceDataRequest")
                    .setMode(ArgMode::MULTIPLE_VALUES)
                    .setAction([this](const char *value) {
                        this->d_securities.emplace_back(value);
                    });

            d_parser.parse(argc, argv);

            if (d_connectionAndAuthOptions.numClientServerSetupAuthOptions()
                    == 0) {
                std::cerr << "No userId:IP or token specified\n";
                d_parser.printHelp();
                return false;
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
        cids.reserve(d_cidToIdentity.size());

        std::transform(d_cidToIdentity.begin(),
                d_cidToIdentity.end(),
                std::back_inserter(cids),
                [](const std::pair<CorrelationId, Identity>& keyVal) {
                    return keyVal.first;
                });

        if (!cids.empty()) {
            session->cancel(&cids[0], cids.size());
        }

        try {
            session->stopAsync();
        } catch (const Exception& e) {
            std::cerr << e.description() << "\n";
        }
    }

  public:
    EntitlementsVerificationRequestResponseExample()
        : d_exampleState(ExampleState::STARTING)
        , d_parser("Entitlements Verification Request/Response Example",
                  "EntitlementsVerificationRequestResponseExample")
        , d_connectionAndAuthOptions(d_parser, true /* isClientServerSetup */)
        , d_finalResponseReceived(false)
    {
    }

    void exceptionHandler(
            Session *session, const Event&, const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        stop(session);
    }

    void distributeResponse(const Event& event,
            const std::string& userIdentifier,
            const Identity& id)
    {
        static const Name eidData("eidData");
        static const Name security("security");
        static const Name securityData("securityData");
        static const Name responseError("responseError");

        std::vector<int> failedEntitlements;

        MessageIterator it(event);
        while (it.next()) {
            Message msg = it.message();

            if (msg.hasElement(responseError)) {
                continue;
            }

            const Service service = msg.service();
            const Element securityElement = msg.getElement(securityData);
            auto securitiesCount = securityElement.numValues();

            std::cout << "Processing " << securitiesCount << " securities:\n";
            for (unsigned i = 0; i < securitiesCount; ++i) {
                const Element sec = securityElement.getValueAsElement(i);
                const std::string ticker(sec.getElementAsString(security));
                if (!sec.hasElement(eidData, true /* excludeNullElements */)) {
                    std::cout << "No entitlements are required for: " << ticker
                              << "\n";
                    // Now distribute message to the user.
                    continue;
                }

                Element entitlements = sec.getElement(eidData);

                failedEntitlements.clear();
                failedEntitlements.resize(entitlements.numValues());
                int failureCount = failedEntitlements.size();

                if (id.hasEntitlements(service,
                            entitlements,
                            &failedEntitlements[0],
                            &failureCount)) {
                    std::cout << userIdentifier
                              << " is entitled to get data for: " << ticker
                              << "\n";
                } else {
                    std::cout << userIdentifier
                              << " is NOT entitled to get data for: " << ticker
                              << "- Failed EIDs: ";

                    for (auto j = 0; j < failureCount; ++j) {
                        std::cout << (j == 0 ? "" : ", ")
                                  << failedEntitlements[j];
                    }

                    std::cout << "\n";
                }
            }
        }
    }

    void distributeResponses(
            const std::string& userIdentifier, const Identity& id)
    {
        for (const auto& response : d_responses) {
            distributeResponse(response, userIdentifier, id);
        }
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

        if (d_finalResponseReceived) {
            distributeResponses(userIdentifier, id);
        }
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
        session->openServiceAsync(REF_DATA_SERVICE_NAME);
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

        if (serviceName != REF_DATA_SERVICE_NAME) {
            std::cout << "A service was opened: " << serviceName << "\n";
            return;
        }

        Service service = session->getService(REF_DATA_SERVICE_NAME);
        sendRefDataRequest(session, service);
    }

    void serviceOpenFailureHandler(
            Session *session, const Event&, const Message& message)
    {
        const std::string serviceName(
                message.getElementAsString("serviceName"));
        if (serviceName == REF_DATA_SERVICE_NAME) {
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

    void processRequestStatus(Session *session, const Event& event)
    {
        MessageIterator msgIter(event);
        while (msgIter.next()) {
            blpapi::Message msg = msgIter.message();
            if (Names::requestFailure() == msg.messageType()) {
                std::cerr << "Request, failed, stopping application ...\n";
                stop(session);
                return;
            }
        }
    }

    void processPartialResponse(const Event& event)
    {
        std::cout << "Received partial response\n";
        d_responses.emplace_back(event);
    }

    void processResponseMessage(const Event& event)
    {
        std::cout << "Received final response\n";
        d_finalResponseReceived = true;
        d_responses.emplace_back(event);

        // Distribute all the cached responses to the identities that have
        // been authorized so far.

        for (const auto& entry : d_cidToIdentity) {
            const auto it = d_cidToStr.find(entry.first);
            if (it == d_cidToStr.end()) {
                continue;
            }

            const std::string& userIdentifier = it->second;
            distributeResponses(userIdentifier, entry.second);
        }
    }

    void sendRefDataRequest(Session *session, const Service& service)
    {
        Request request = service.createRequest("ReferenceDataRequest");
        Element securitiesElement = request.getElement("securities");

        for (auto const& security : d_securities) {
            securitiesElement.appendValue(security.c_str());
        }

        // Add fields
        Element fields = request.getElement("fields");
        fields.appendValue("PX_LAST");
        fields.appendValue("DS002");

        request.set("returnEids", true);
        std::cout << "Sending RefDataRequest ...\n";

        CorrelationId cid(Utils::getNextIntegerCid());
        d_sessionRouter.registerEventHandler(Event::REQUEST_STATUS,
                [this](Session *session, const Event& event) {
                    processRequestStatus(session, event);
                });
        d_sessionRouter.registerEventHandler(Event::PARTIAL_RESPONSE,
                [this](Session *, const Event& event) {
                    processPartialResponse(event);
                });
        d_sessionRouter.registerEventHandler(
                Event::RESPONSE, [this](Session *, const Event& event) {
                    processResponseMessage(event);
                });

        session->sendRequest(request, cid);
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
        Session session(d_connectionAndAuthOptions.createSessionOption(),
                &d_sessionRouter);
        session.startAsync();

        while (d_exampleState != ExampleState::TERMINATED) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
};

int main(int argc, const char **argv)
{
    EntitlementsVerificationRequestResponseExample example;
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
