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
#include <blpapi_element.h>
#include <blpapi_event.h>
#include <blpapi_exception.h>
#include <blpapi_message.h>
#include <blpapi_name.h>
#include <blpapi_names.h>
#include <blpapi_request.h>
#include <blpapi_session.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
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
const static char *k_refDataSvc = "//blp/refdata";
} // anonymous namespace

class UserModeExample {
  private:
    std::atomic<ExampleState> d_exampleState;
    ArgParser d_argParser;
    ConnectionAndAuthOptions d_connectionAndAuthOptions;
    SessionRouter<Session> d_sessionRouter;
    Service d_blpRefDataSvc;
    std::vector<std::string> d_securities;
    std::map<CorrelationId, Identity> d_cidToIdentity;
    std::map<CorrelationId, std::string> d_cidToStr;
    // Map contains 'CorrelationId' to 'UserIdentifier' mapping

    bool parseCommandLine(int argc, const char **argv)
    {
        try {
            d_argParser.addArg("security", 'S')
                    .setMetaVar("security")
                    .setDescription("security used in ReferenceDataRequest")
                    .setDefaultValue("IBM US Equity")
                    .setMode(ArgMode::MULTIPLE_VALUES)
                    .setAction([this](const char *value) {
                        d_securities.emplace_back(value);
                    });

            d_argParser.parse(argc, argv);

            if (d_connectionAndAuthOptions.numClientServerSetupAuthOptions()
                    == 0) {
                std::cerr << "No userId:IP or token specified.\n";
                d_argParser.printHelp();
                return false;
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse arguments: " << e.what() << "\n";
            d_argParser.printHelp();
            return false;
        }

        return true;
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
        Identity identity = session->getAuthorizedIdentity(cid);
        d_cidToIdentity[cid] = identity;

        if (d_blpRefDataSvc.isValid()) {
            sendRefDataRequest(session, identity, userIdentifier);
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
        d_cidToStr.erase(it);
    }

    void authorizeUsers(Session *session)
    {
        std::map<std::string, AuthOptions> authOptionList
                = d_connectionAndAuthOptions
                          .createClientServerSetupAuthOptions();

        // Authorize each of the users
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
        session->openServiceAsync(k_refDataSvc);
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

        openServices(session);
        authorizeUsers(session);
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
        const char *serviceName = message.getElementAsString("serviceName");
        Service service = session->getService(serviceName);

        if (std::strcmp(serviceName, k_refDataSvc) != 0) {
            std::cout << "A service was opened: " << serviceName << "\n";
            return;
        }

        d_blpRefDataSvc = service;
        for (const auto& cidAndIdentity : d_cidToIdentity) {
            const CorrelationId& corId = cidAndIdentity.first;
            const Identity& identity = cidAndIdentity.second;

            const auto iterator = d_cidToStr.find(corId);
            if (iterator == d_cidToStr.end()) {
                std::cout << "User identifier not found for correlationId: "
                          << corId << "\n";
            }

            sendRefDataRequest(session, identity, iterator->second);
        }
    }

    void serviceOpenFailureHandler(
            Session *session, const Event&, const Message& message)
    {
        const char *serviceName = message.getElementAsString("serviceName");
        if (std::strcmp(serviceName, k_refDataSvc) == 0) {
            stop(session);
            return;
        }

        std::cout << "A service which is unknown failed to open: "
                  << serviceName << "\n";
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
    }

    void sendRefDataRequest(Session *session,
            const Identity& identity,
            const std::string& userIdentifier)
    {
        Request request
                = d_blpRefDataSvc.createRequest("ReferenceDataRequest");

        // Add securities.
        Element securitiesElement = request.getElement("securities");
        for (const auto& security : d_securities) {
            securitiesElement.appendValue(security.c_str());
        }

        // Add fields
        Element fields = request.getElement("fields");
        fields.appendValue("PX_LAST");
        fields.appendValue("DS002");

        request.set("returnEids", true);

        std::cout << "Sending RefDataRequest on behalf of " << userIdentifier
                  << " ...\n";

        CorrelationId correlationId(Utils::getNextIntegerCid());

        d_sessionRouter.registerMessageHandler(correlationId,
                [this, userIdentifier](Session *,
                        const Event& event,
                        const Message& message) {
                    if (message.messageType() == Names::requestFailure()) {
                        std::cout << "Request for " << userIdentifier
                                  << " failed.\n";
                        return;
                    }

                    std::cout << "Received response for " << userIdentifier
                              << ".\n";
                });

        session->sendRequest(request, identity, correlationId);
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

        session->cancel(&cids[0], cids.size());

        try {
            session->stopAsync();
        } catch (const Exception& e) {
            std::cerr << e.description() << "\n";
        }
    }

  public:
    UserModeExample()
        : d_exampleState(ExampleState::STARTING)
        , d_argParser("User mode Example", "UserModeExample")
        , d_connectionAndAuthOptions(d_argParser, true)
    {
    }

    void run(int argc, const char **argv)
    {
        if (!parseCommandLine(argc, argv)) {
            return;
        }

        registerCallbacks();

        // Use the specified application as the session identity to authorize.
        // This may cause the session to stop and the example to terminate if
        // the identity is revoked.
        SessionOptions sessionOptions
                = d_connectionAndAuthOptions.createSessionOption();
        Session session(sessionOptions, &d_sessionRouter);
        session.startAsync();

        // The main thread is not blocked and the example is running
        // asynchronously.
        while (d_exampleState != ExampleState::TERMINATED) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
};

int main(int argc, const char **argv)
{
    UserModeExample example;
    try {
        example.run(argc, argv);
    } catch (Exception& e) {
        std::cerr << "Library Exception! " << e.description() << "\n";
    }

    // Wait for enter key to exit application
    std::cout << "Press ENTER to quit\n";
    char dummy[2];
    std::cin.getline(dummy, 2);
    return 0;
}
