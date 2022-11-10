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
#ifndef INCLUDED_FIELDSEARCHREQUESTS
#define INCLUDED_FIELDSEARCHREQUESTS

#include <blpapi_element.h>
#include <blpapi_event.h>
#include <blpapi_message.h>
#include <blpapi_request.h>
#include <blpapi_service.h>

#include <snippets/apiflds/ApiFieldsRequestUtils.h>

namespace BloombergLP {

using blpapi::Element;
using blpapi::Event;
using blpapi::Message;
using blpapi::MessageIterator;
using blpapi::Request;
using blpapi::Service;

struct FieldSearchRequests {
    static Request createRequest(const Service& apifldsService);
    static void processResponse(const Event& event);
};

inline Request FieldSearchRequests::createRequest(
        const Service& apifldsService)
{
    Request request = apifldsService.createRequest("FieldSearchRequest");
    request.set("searchSpec", "last price");

    Element exclude = request.getElement("exclude");
    exclude.setElement("fieldType", "Static");

    request.set("returnFieldDocumentation", false);

    return request;
}

inline void FieldSearchRequests::processResponse(const Event& event)
{
    MessageIterator msgIter(event);
    while (msgIter.next()) {
        ApiFieldsRequestUtils::printHeader();

        Message msg = msgIter.message();
        Element fields = msg.getElement("fieldData");
        size_t numValues = fields.numValues();
        for (size_t i = 0; i < numValues; i++) {
            ApiFieldsRequestUtils::printField(fields.getValueAsElement(i));
        }

        std::cout << "\n";
    }
}
}

#endif // INCLUDED_FIELDSEARCHREQUESTS
