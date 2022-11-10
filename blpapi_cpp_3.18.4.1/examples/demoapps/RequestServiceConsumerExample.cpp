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
#include <blpapi_message.h>
#include <blpapi_name.h>
#include <blpapi_names.h>
#include <blpapi_providersession.h>
#include <blpapi_session.h>

#include <chrono>
#include <iostream>

#include <util/ConnectionAndAuthOptions.h>
#include <util/Utils.h>

using namespace BloombergLP;
using namespace blpapi;

namespace {
const Name FIELDS("fields");
const Name SECURITIES("securities");
const Name TIMESTAMP("timestamp");

const char *SERVICE = "//example/refdata";

const char *REQUEST = "ReferenceDataRequest";
}

class RequestServiceConsumerExample {
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

    void sendRequest(Session& session)
    {
        const Service service = session.getService(SERVICE);
        Request request = service.createRequest(REQUEST);

        static const char *securities[2]
                = { "IBM US Equity", "MSFT US Equity" };
        Element securityElement = request.getElement(SECURITIES);
        for (const auto& security : securities) {
            securityElement.appendValue(security);
        }

        static const char *fields[2] = { "PX_LAST", "DS002" };
        Element fieldElement = request.getElement(FIELDS);
        for (const auto& field : fields) {
            securityElement.appendValue(field);
        }

        const auto timeStamp
                = static_cast<Float64>(Utils::getCurrentTimeMilliSec());
        request.set(TIMESTAMP, timeStamp);

        std::cout << "Sending Request:\n";
        request.print(std::cout);

        session.sendRequest(request);
    }

    void waitForResponse(Session& session)
    {
        bool done = false;
        while (!done) {
            Event event = session.nextEvent();
            std::cout << "Received Event:\n";

            const Event::EventType eventType = event.eventType();

            MessageIterator it(event);
            if (Event::REQUEST_STATUS == eventType) {
                while (it.next()) {
                    Message msg = it.message();
                    if (Names::requestFailure() == msg.messageType()) {
                        std::cerr << "Request Failed!\n";
                        msg.print(std::cerr) << "\n";
                        done = true;
                    }
                }
            } else if (Event::RESPONSE == eventType) {
                while (it.next()) {
                    Message msg = it.message();
                    if (msg.hasElement(TIMESTAMP)) {
                        const auto responseTime
                                = msg.getElementAsFloat64(TIMESTAMP);
                        const double latency = Utils::getCurrentTimeMilliSec()
                                - responseTime;
                        std::cout << "Response latency = " << latency << "\n";
                    }
                }

                done = true;
            } else {
                done = !Utils::processGenericEvent(event);
            }
        }
    }

  public:
    RequestServiceConsumerExample()
        : d_argParser("Request service consumer example, to be used in "
                      "conjunction with RequestServiceProviderExample.",
                "RequestServiceConsumerExample")
        , d_connectionAndAuthOptions(d_argParser)
    {
    }

    void run(int argc, const char **argv)
    {
        if (!parseCommandLine(argc, argv)) {
            return;
        }

        Session session(d_connectionAndAuthOptions.createSessionOption());

        if (!session.start()) {
            std::cout << "Failed to start session\n";
            Utils::checkFailures(session);
            return;
        }

        if (!session.openService(SERVICE)) {
            std::cout << ">>> Failed to open " << SERVICE << "\n";
            Utils::checkFailures(session);
            session.stop();
            return;
        }

        sendRequest(session);
        waitForResponse(session);
        session.stop();
    }
};

int main(int argc, const char **argv)
{
    RequestServiceConsumerExample example;
    try {
        example.run(argc, argv);
    } catch (const Exception& e) {
        std::cerr << ">>> Exception caught: " << e.description() << "\n";
    }
    return 0;
}
