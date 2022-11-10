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

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include <snippets/requestresponse/HistoricalDataRequests.h>
#include <snippets/requestresponse/IntradayBarRequests.h>
#include <snippets/requestresponse/IntradayTickRequests.h>
#include <snippets/requestresponse/ReferenceDataRequests.h>

#include <util/ConnectionAndAuthOptions.h>
#include <util/RequestOptions.h>
#include <util/Utils.h>

#include <string.h>

using namespace BloombergLP;
using namespace blpapi;
using blpapi::Name;

namespace {
const Name REASON("reason");
}

class RequestResponseExample {
  private:
    ArgParser d_argParser;
    ConnectionAndAuthOptions d_connectionAndAuthOptions;
    RequestOptions d_requestOptions;

    bool parseCommandLine(int argc, const char **argv)
    {
        try {
            d_argParser.parse(argc, argv);
        } catch (const std::exception& ex) {
            std::cerr << ex.what() << "\n";
            d_argParser.printHelp();
            return false;
        }

        d_requestOptions.setDefaultValues();

        return true;
    }

    Request createRequest(Session& session)
    {
        Service service
                = session.getService(d_requestOptions.d_service.c_str());
        const char *requestType = d_requestOptions.d_requestType.c_str();
        if (!std::strcmp(requestType, RequestOptions::INTRADAY_BAR_REQUEST)) {
            return IntradayBarRequests::createRequest(
                    service, d_requestOptions);
        }

        if (!std::strcmp(requestType, RequestOptions::INTRADAY_TICK_REQUEST)) {
            return IntradayTickRequests::createRequest(
                    service, d_requestOptions);
        }

        if (!std::strcmp(requestType, RequestOptions::REFERENCE_DATA_REQUEST)
                || !std::strcmp(requestType,
                        RequestOptions::REFERENCE_DATA_REQUEST_OVERRIDE)) {
            return ReferenceDataRequests::createRequest(
                    service, d_requestOptions);
        }

        if (!std::strcmp(requestType,
                    RequestOptions::REFERENCE_DATA_REQUEST_TABLE_OVERRIDE)) {
            return ReferenceDataRequests::createTableOverrideRequest(
                    service, d_requestOptions);
        }

        if (!std::strcmp(
                    requestType, RequestOptions::HISTORICAL_DATA_REQUEST)) {
            return HistoricalDataRequests::createRequest(
                    service, d_requestOptions);
        }

        throw InvalidArgumentException(
                "Unknown request type: " + d_requestOptions.d_requestType);
    }

    // This function processes the contents of a 'RESPONSE' or
    // 'PARTIAL_RESPONSE' to a request.
    void processResponseEvent(const Event& event)
    {
        const char *requestType = d_requestOptions.d_requestType.c_str();
        if (!std::strcmp(requestType, RequestOptions::INTRADAY_BAR_REQUEST)) {
            return IntradayBarRequests::processResponseEvent(event);
        }

        if (!std::strcmp(requestType, RequestOptions::INTRADAY_TICK_REQUEST)) {
            return IntradayTickRequests::processResponseEvent(event);
        }

        if (!std::strcmp(
                    requestType, RequestOptions::REFERENCE_DATA_REQUEST)) {
            return ReferenceDataRequests::processResponseEvent(event);
        }

        if (!std::strcmp(requestType,
                    RequestOptions::REFERENCE_DATA_REQUEST_OVERRIDE)) {
            return ReferenceDataRequests::processResponseEvent(event);
        }

        if (!std::strcmp(requestType,
                    RequestOptions::REFERENCE_DATA_REQUEST_TABLE_OVERRIDE)) {
            return ReferenceDataRequests::processResponseEvent(event);
        }

        if (!std::strcmp(
                    requestType, RequestOptions::HISTORICAL_DATA_REQUEST)) {
            return HistoricalDataRequests::processResponseEvent(event);
        }

        throw InvalidArgumentException(
                "Unknown request type: " + d_requestOptions.d_requestType);
    }

    // After we have sent the request we want to wait for the response.
    // Success response can come with a number of 'PARTIAL_RESPONSE' events
    // followed by a 'RESPONSE' event.  Failures will be delivered in a
    // 'REQUEST_STATUS' event holding a 'RequestFailure' message.
    void waitForResponse(Session& session)
    {
        bool done = false;
        while (!done) {
            Event event = session.nextEvent();
            if (event.eventType() == Event::PARTIAL_RESPONSE) {
                std::cout << "Processing Partial Response\n";
                processResponseEvent(event);
            } else if (event.eventType() == Event::RESPONSE) {
                std::cout << "Processing Response\n";
                processResponseEvent(event);
                done = true;
            } else if (event.eventType() == Event::REQUEST_STATUS) {
                MessageIterator msgIter(event);
                while (msgIter.next()) {
                    Message msg = msgIter.message();
                    if (msg.messageType() == Names::requestFailure()) {
                        std::cout
                                << "REQUEST FAILED: " << msg.getElement(REASON)
                                << "\n";
                        // Report the request id to contact support if needed.
                        Utils::printContactSupportMessage(msg);
                        done = true;
                    }
                }
            } else {
                // 'SESSION_STATUS' events can happen at any time and need to
                // be handled as the session can be terminated.
                done = !Utils::processGenericEvent(event);
            }
        }
    }

  public:
    RequestResponseExample()
        : d_argParser("Request/Response Example", "RequestResponseExample")
        , d_connectionAndAuthOptions(d_argParser)
        , d_requestOptions(d_argParser)
    {
    }

    int run(int argc, const char **argv)
    {
        if (!parseCommandLine(argc, argv)) {
            return 1;
        }

        SessionOptions sessionOptions
                = d_connectionAndAuthOptions.createSessionOption();
        Session session(sessionOptions);
        if (!session.start()) {
            std::cout << "Failed to start session.\n";
            // Print the events in the queue to know what failed
            Utils::checkFailures(session);
            return 1;
        }

        if (!session.openService(d_requestOptions.d_service.c_str())) {
            std::cout << "Failed to open " << d_requestOptions.d_service
                      << "\n";
            Utils::checkFailures(session);
            session.stop();
            return 1;
        }

        // wait for events from session.

        // The request id is auto generated by the library and should be used
        // when contacting support.  Note that the same request id will also be
        // present in the response messages.  Requests will always have a
        // request id.
        Request request = createRequest(session);
        std::cout << "Sending Request: " << request << " with request id '"
                  << request.getRequestId() << "\n";
        session.sendRequest(request);

        waitForResponse(session);
        session.stop();
        return 0;
    }
};

int main(int argc, const char **argv)
{
    RequestResponseExample example;
    try {
        return example.run(argc, argv);
    } catch (Exception& e) {
        std::cerr << "Library Exception!!! " << e.description() << "\n";
        return 1;
    }
}
