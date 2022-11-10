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
#ifndef INCLUDED_INTRADAYTICKREQUESTS
#define INCLUDED_INTRADAYTICKREQUESTS

#include <blpapi_element.h>
#include <blpapi_event.h>
#include <blpapi_message.h>
#include <blpapi_request.h>
#include <blpapi_service.h>

#include <iomanip>
#include <string>
#include <vector>

#include <util/RequestOptions.h>

namespace BloombergLP {

using blpapi::Element;
using blpapi::Event;
using blpapi::Message;
using blpapi::MessageIterator;
using blpapi::Name;
using blpapi::Request;
using blpapi::Service;

struct IntradayTickRequests {
    static Request createRequest(
            const Service& service, const RequestOptions& requestOptions);

    static void processResponseEvent(const Event& event);
};

inline Request IntradayTickRequests::createRequest(
        const Service& service, const RequestOptions& requestOptions)
{
    Request request = service.createRequest("IntradayTickRequest");

    request.set("security", requestOptions.d_securities[0].c_str());

    Element eventTypesElement = request.getElement("eventTypes");
    for (const std::string& eventType : requestOptions.d_eventTypes) {
        eventTypesElement.appendValue(eventType.c_str());
    }

    request.set("startDateTime", requestOptions.d_startDateTime.c_str());
    request.set("endDateTime", requestOptions.d_endDateTime.c_str());

    if (requestOptions.d_includeConditionCodes) {
        request.set("includeConditionCodes", true);
    }

    return request;
}

inline void IntradayTickRequests::processResponseEvent(const Event& event)
{
    static const Name TICK_DATA("tickData");
    static const Name CONDITION_CODES("conditionCodes");
    static const Name TICK_SIZE("size");
    static const Name TIME("time");
    static const Name TYPE("type");
    static const Name VALUE("value");
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

        Element data = msg.getElement(TICK_DATA).getElement(TICK_DATA);
        int numItems = static_cast<int>(data.numValues());
        std::cout << "TIME\t\t\t\tTYPE\tVALUE\t\tSIZE\tCC\n";
        std::cout << "----\t\t\t\t----\t-----\t\t----\t--\n";
        for (int i = 0; i < numItems; ++i) {
            Element item = data.getValueAsElement(i);
            std::string timeString = item.getElementAsString(TIME);
            std::string type = item.getElementAsString(TYPE);
            double value = item.getElementAsFloat64(VALUE);
            int size = item.getElementAsInt32(TICK_SIZE);
            std::string conditionCodes = "";
            if (item.hasElement(CONDITION_CODES)) {
                conditionCodes = item.getElementAsString(CONDITION_CODES);
            }

            std::cout.setf(std::ios::fixed, std::ios::floatfield);
            std::cout << timeString << "\t" << type << "\t"
                      << std::setprecision(3) << std::showpoint << value
                      << "\t\t" << size << "\t" << std::noshowpoint
                      << conditionCodes << "\n";
        }
    }
}
}

#endif // INCLUDED_INTRADAYTICKREQUESTS
