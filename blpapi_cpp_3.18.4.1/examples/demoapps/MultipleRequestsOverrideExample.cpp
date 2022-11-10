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
#include <blpapi_event.h>
#include <blpapi_message.h>
#include <blpapi_name.h>
#include <blpapi_names.h>
#include <blpapi_request.h>
#include <blpapi_session.h>

#include <iostream>
#include <string>
#include <vector>

#include <snippets/requestresponse/ReferenceDataRequests.h>
#include <util/ConnectionAndAuthOptions.h>

using namespace BloombergLP;
using namespace blpapi;

class MultipleRequestsOverrideExample {
  public:
    void run(int argc, const char **argv)
    {
        ArgParser argParser("Multiple requests with override example",
                "MultipleRequestsOverrideExample");
        ConnectionAndAuthOptions connectionAndAuthOptions(argParser);
        try {
            argParser.parse(argc, argv);
        } catch (const std::exception& ex) {
            std::cerr << ex.what() << "\n";
            argParser.printHelp();
            return;
        }

        SessionOptions sessionOptions
                = connectionAndAuthOptions.createSessionOption();
        Session session(sessionOptions);

        if (!session.start()) {
            std::cerr << "Failed to connect!\n";
            return;
        }

        if (!session.openService(RequestOptions::REFDATA_SERVICE)) {
            std::cerr << "Failed to open " << RequestOptions::REFDATA_SERVICE
                      << "\n";
            session.stop();
            return;
        }

        Service refDataService
                = session.getService(RequestOptions::REFDATA_SERVICE);

        const std::string fieldIdVwapStartTime { "VWAP_START_TIME" };
        const std::string fieldIdVwapEndTime { "VWAP_END_TIME" };

        // Request 1
        const std::string startTime1 { "9:30" };
        const std::string endTime1 { "11:30" };

        RequestOptions requestOptions;
        requestOptions.d_securities = { "IBM US Equity", "MSFT US Equity" };
        requestOptions.d_fields = { "PX_LAST", "DS002" };
        requestOptions.d_overrides = { { fieldIdVwapStartTime, startTime1 },
            { fieldIdVwapEndTime, endTime1 } };

        Request request1 = ReferenceDataRequests::createRequest(
                refDataService, requestOptions);

        std::cout << "Sending request 1: ";
        request1.print(std::cout) << "\n";
        const CorrelationId correlationId1(1);
        session.sendRequest(request1, correlationId1);

        // Request 2
        const std::string startTime2 { "11:30" };
        const std::string endTime2 { "13:30" };
        requestOptions.d_overrides.clear();
        requestOptions.d_overrides = { { fieldIdVwapStartTime, startTime2 },
            { fieldIdVwapEndTime, endTime2 } };
        Request request2 = ReferenceDataRequests::createRequest(
                refDataService, requestOptions);

        std::cout << "Sending request 2: ";
        request2.print(std::cout) << "\n";
        const CorrelationId correlationId2(2);
        session.sendRequest(request2, correlationId2);

        // Wait for responses for both requests, expect 2 final responses
        // either failure or success.
        int finalResponseCount = 0;
        while (finalResponseCount < 2) {
            Event event = session.nextEvent();
            Event::EventType eventType = event.eventType();
            MessageIterator msgIter(event);
            while (msgIter.next()) {
                Message msg = msgIter.message();
                CorrelationId msgCorrelationId = msg.correlationId();
                if (eventType == Event::REQUEST_STATUS) {
                    if (msg.messageType() == Names::requestFailure()) {
                        if (correlationId1 == msgCorrelationId) {
                            std::cout << "Request 1 failed.\n";
                        } else if (correlationId2 == msgCorrelationId) {
                            std::cout << "Request 2 failed.\n";
                        }

                        ++finalResponseCount;
                    }
                } else if (eventType == Event::RESPONSE
                        || eventType == Event::PARTIAL_RESPONSE) {
                    if (correlationId1 == msgCorrelationId) {
                        std::cout << "Received response for request 1\n";
                    } else if (correlationId2 == msgCorrelationId) {
                        std::cout << "Received response for request 2\n";
                    }

                    if (eventType == Event::RESPONSE) {
                        ++finalResponseCount;
                    }
                }

                msg.print(std::cout) << "\n";
            }
        }

        session.stop();
    }
};

int main(int argc, const char **argv)
{
    MultipleRequestsOverrideExample example;
    try {
        example.run(argc, argv);
    } catch (Exception& e) {
        std::cerr << "Library Exception!!! " << e.description() << "\n";
    }

    // wait for enter key to exit application
    std::cout << "Press ENTER to quit\n";
    char dummy[2];
    std::cin.getline(dummy, 2);
    return 0;
}
