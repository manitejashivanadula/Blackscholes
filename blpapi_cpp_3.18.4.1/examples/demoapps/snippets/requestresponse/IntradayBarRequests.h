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
#ifndef INCLUDED_INTRADAYBARREQUESTS
#define INCLUDED_INTRADAYBARREQUESTS

#include <blpapi_datetime.h>
#include <blpapi_element.h>
#include <blpapi_event.h>
#include <blpapi_message.h>
#include <blpapi_name.h>
#include <blpapi_request.h>
#include <blpapi_service.h>

#include <cassert>
#include <iomanip>
#include <string>
#include <vector>

#include <util/RequestOptions.h>

namespace BloombergLP {

using blpapi::Datetime;
using blpapi::DatetimeParts;
using blpapi::Element;
using blpapi::Event;
using blpapi::Message;
using blpapi::MessageIterator;
using blpapi::Name;
using blpapi::Request;
using blpapi::Service;

struct IntradayBarRequests {
    static Request createRequest(
            const Service& service, const RequestOptions& requestOptions);

    static void processResponseEvent(const Event& event);
};

inline Request IntradayBarRequests::createRequest(
        const Service& service, const RequestOptions& requestOptions)
{
    Request request = service.createRequest("IntradayBarRequest");

    // only one security/eventType per request
    request.set("security", requestOptions.d_securities[0].c_str());
    request.set("eventType", requestOptions.d_eventTypes[0].c_str());
    request.set("interval", requestOptions.d_barInterval);

    request.set("startDateTime", requestOptions.d_startDateTime.c_str());
    request.set("endDateTime", requestOptions.d_endDateTime.c_str());

    if (requestOptions.d_gapFillInitialBar) {
        request.set("gapFillInitialBar", true);
    }

    return request;
}

inline void IntradayBarRequests::processResponseEvent(const Event& event)
{
    static const Name BAR_DATA("barData");
    static const Name BAR_TICK_DATA("barTickData");
    static const Name OPEN("open");
    static const Name HIGH("high");
    static const Name LOW("low");
    static const Name CLOSE("close");
    static const Name VOLUME("volume");
    static const Name NUM_EVENTS("numEvents");
    static const Name TIME("time");
    static const Name RESPONSE_ERROR("responseError");
    static const Name CATEGORY("category");
    static const Name MESSAGE("message");

    MessageIterator msgIter(event);
    while (msgIter.next()) {
        Message msg = msgIter.message();
        std::cout << "Received response to request " << msg.getRequestId()
                  << "\n";
        if (msg.hasElement(RESPONSE_ERROR)) {
            Element responseError = msg.getElement(RESPONSE_ERROR);
            std::cout << "REQUEST FAILED: "
                      << responseError.getElementAsString(CATEGORY) << " ("
                      << responseError.getElementAsString(MESSAGE) << ")\n";

            continue;
        }

        Element data = msg.getElement(BAR_DATA).getElement(BAR_TICK_DATA);
        int numBars = static_cast<int>(data.numValues());
        std::cout << "Response contains " << numBars << " bars\n";
        std::cout << "Datetime\t\tOpen\t\tHigh\t\tLow\t\tClose"
                  << "\t\tNumEvents\tVolume\n";
        for (int i = 0; i < numBars; ++i) {
            Element bar = data.getValueAsElement(i);
            Datetime time = bar.getElementAsDatetime(TIME);
            assert(time.hasParts(DatetimeParts::DATE | DatetimeParts::HOURS
                    | DatetimeParts::MINUTES));
            double open = bar.getElementAsFloat64(OPEN);
            double high = bar.getElementAsFloat64(HIGH);
            double low = bar.getElementAsFloat64(LOW);
            double close = bar.getElementAsFloat64(CLOSE);
            int numEvents = bar.getElementAsInt32(NUM_EVENTS);
            long long volume = bar.getElementAsInt64(VOLUME);

            std::cout.setf(std::ios::fixed, std::ios::floatfield);
            std::cout << time.month() << '/' << time.day() << '/'
                      << time.year() << " " << time.hours() << ":"
                      << time.minutes() << "\t\t" << std::showpoint
                      << std::setprecision(3) << open << "\t\t" << high
                      << "\t\t" << low << "\t\t" << close << "\t\t"
                      << numEvents << "\t\t" << std::noshowpoint << volume
                      << "\n";
        }
    }
}
}

#endif // INCLUDED_INTRADAYBARREQUESTS
