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
#ifndef INCLUDED_HISTORICALDATAREQUESTS
#define INCLUDED_HISTORICALDATAREQUESTS

#include <blpapi_element.h>
#include <blpapi_event.h>
#include <blpapi_message.h>
#include <blpapi_request.h>
#include <blpapi_service.h>

#include <string>
#include <vector>

#include <util/RequestOptions.h>

namespace BloombergLP {

using blpapi::Element;
using blpapi::Event;
using blpapi::Message;
using blpapi::MessageIterator;
using blpapi::Request;
using blpapi::Service;

struct HistoricalDataRequests {
    static Request createRequest(
            const Service& service, const RequestOptions& requestOptions);

    static void processResponseEvent(const Event& event);
};

inline Request HistoricalDataRequests::createRequest(
        const Service& service, const RequestOptions& requestOptions)
{
    Request request = service.createRequest("HistoricalDataRequest");

    Element securitiesElement = request.getElement("securities");
    for (const std::string& security : requestOptions.d_securities) {
        securitiesElement.appendValue(security.c_str());
    }

    Element fieldsElement = request.getElement("fields");
    for (const std::string& field : requestOptions.d_fields) {
        fieldsElement.appendValue(field.c_str());
    }

    request.set("periodicityAdjustment", "ACTUAL");
    request.set("periodicitySelection", "MONTHLY");
    request.set("startDate", "20200101");
    request.set("endDate", "20201231");
    request.set("maxDataPoints", 100);
    request.set("returnEids", true);

    return request;
}

inline void HistoricalDataRequests::processResponseEvent(const Event& event)
{
    MessageIterator msgIter(event);
    while (msgIter.next()) {
        Message msg = msgIter.message();
        std::cout << "Received response to request " << msg.getRequestId()
                  << "\n";
        msg.asElement().print(std::cout);
    }
}
}

#endif // INCLUDED_HISTORICALDATAREQUESTS
