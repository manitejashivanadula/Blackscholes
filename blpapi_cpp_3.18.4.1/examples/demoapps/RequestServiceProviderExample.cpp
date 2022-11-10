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
#include <blpapi_eventformatter.h>
#include <blpapi_message.h>
#include <blpapi_name.h>
#include <blpapi_providersession.h>

#include <iostream>

#include <util/ConnectionAndAuthOptions.h>
#include <util/Utils.h>

using namespace BloombergLP;
using namespace blpapi;

namespace {
const Name TIMESTAMP("timestamp");
const Name REFERENCE_DATA_REQUEST("ReferenceDataRequest");

const Name FIELDS("fields");
const Name FIELD_ID("fieldId");
const Name FIELD_DATA("fieldData");

const Name SECURITY("security");
const Name SECURITIES("securities");
const Name SECURITY_DATA("securityData");

const Name DATA("data");
const Name DOUBLE_VALUE("doubleValue");

const char *SERVICE = "//example/refdata";

Event createResponseEvent(const Service& service, const Message& msg)
{
    // A response event must contain only one response message and attach the
    // correlation ID of the request message.
    Event response = service.createResponseEvent(msg.correlationId());
    EventFormatter ef(response);

    // The parameter of EventFormatter.appendResponse(Name) is the name of the
    // operation instead of the response.
    ef.appendResponse(REFERENCE_DATA_REQUEST);
    Element securitiesElement = msg.getElement(SECURITIES);
    Element fieldsElement = msg.getElement(FIELDS);

    ef.setElement(TIMESTAMP, Utils::getCurrentTimeMilliSec());
    ef.pushElement(SECURITY_DATA);
    for (unsigned i = 0; i < securitiesElement.numValues(); ++i) {
        ef.appendElement();
        ef.setElement(SECURITY, securitiesElement.getValueAsString(i));
        ef.pushElement(FIELD_DATA);
        for (unsigned j = 0; j < fieldsElement.numValues(); ++j) {
            ef.appendElement();
            ef.setElement(FIELD_ID, fieldsElement.getValueAsString(j));
            ef.pushElement(DATA);
            ef.setElement(DOUBLE_VALUE, Utils::getCurrentTimeMilliSec());
            ef.popElement();
            ef.popElement();
        }

        ef.popElement();
        ef.popElement();
    }

    ef.popElement();

    return response;
}

class ServerEventHandler : public ProviderEventHandler {
  public:
    bool processEvent(
            const Event& event, ProviderSession *providerSession) override;
};

bool ServerEventHandler::processEvent(
        const Event& event, ProviderSession *providerSession)
{
    Event::EventType eventType = event.eventType();
    std::cout << "Received event\n";
    Utils::printEvent(event);

    if (Event::REQUEST != eventType) {
        return true;
    }

    MessageIterator it(event);
    while (it.next()) {
        Message msg = it.message();
        if (REFERENCE_DATA_REQUEST != msg.messageType()) {
            std::cout << "Waiting for requests..., Press ENTER to quit\n";
            continue;
        }

        if (msg.hasElement(TIMESTAMP)) {
            const auto requestTime = msg.getElementAsFloat64(TIMESTAMP);
            double latency = Utils::getCurrentTimeMilliSec() - requestTime;
            std::cout << "Request latency = " << latency << "\n";
        }

        Event response = createResponseEvent(
                providerSession->getService(SERVICE), msg);
        std::cout << "Publishing Response\n";
        Utils::printEvent(response);
        providerSession->sendResponse(response);
    }

    return true;
}
}

class RequestServiceProviderExample {
    ArgParser d_argParser;
    ConnectionAndAuthOptions d_connectionAndAuthOptions;

    bool parseCommandLine(int argc, const char **argv)
    {
        try {
            d_argParser.parse(argc, argv);
        } catch (const std::exception& ex) {
            std::cerr << "Failed to parse arguments: " << ex.what() << "\n";
            d_argParser.printHelp();
            return false;
        }

        return true;
    }

  public:
    RequestServiceProviderExample()
        : d_argParser("Request service provider example, to be used in "
                      "conjunction with RequestServiceConsumerExample.",
                "RequestServiceProviderExample")
        , d_connectionAndAuthOptions(d_argParser)
    {
    }

    void run(int argc, const char **argv)
    {
        if (!parseCommandLine(argc, argv)) {
            return;
        }

        ServerEventHandler handler;
        ProviderSession providerSession(
                d_connectionAndAuthOptions.createSessionOption(), &handler);

        if (!providerSession.start()) {
            std::cout << "Failed to start session\n";
            Utils::checkFailures(providerSession);
            return;
        }

        if (!providerSession.registerService(
                    SERVICE, providerSession.getAuthorizedIdentity())) {
            std::cout << ">>> Failed to register" << SERVICE << "\n";
            Utils::checkFailures(providerSession);
            providerSession.stop();
            return;
        }

        std::cout << "Service is registered successfully\n";
        std::cout << "Waiting for requests..., Press ENTER to quit\n";
        std::cin.get();

        providerSession.stop();
    }
};

int main(int argc, const char **argv)
{
    RequestServiceProviderExample example;
    try {
        example.run(argc, argv);
    } catch (const Exception& e) {
        std::cerr << ">>> Exception caught: " << e.description() << "\n";
    }
    return 0;
}
