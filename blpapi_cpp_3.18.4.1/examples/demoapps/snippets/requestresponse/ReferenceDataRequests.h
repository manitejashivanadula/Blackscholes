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
#ifndef INCLUDED_REFERENCEDATAREQUESTS
#define INCLUDED_REFERENCEDATAREQUESTS

#include <blpapi_element.h>
#include <blpapi_event.h>
#include <blpapi_message.h>
#include <blpapi_request.h>
#include <blpapi_service.h>

#include <sstream>
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

struct ReferenceDataRequests {
    static Request createRequest(
            const Service& service, const RequestOptions& requestOptions);
    static Request createTableOverrideRequest(
            const Service& service, const RequestOptions& requestOptions);

    static void processResponseEvent(const Event& event);
};

inline Request ReferenceDataRequests::createRequest(
        const Service& service, const RequestOptions& requestOptions)
{
    Request request = service.createRequest("ReferenceDataRequest");

    Element securitiesElement = request.getElement("securities");
    for (const std::string& security : requestOptions.d_securities) {
        securitiesElement.appendValue(security.c_str());
    }

    Element fieldsElement = request.getElement("fields");
    for (const std::string& field : requestOptions.d_fields) {
        fieldsElement.appendValue(field.c_str());
    }

    if (!requestOptions.d_overrides.empty()) {
        Element overridesElement = request.getElement("overrides");
        for (const auto& entry : requestOptions.d_overrides) {
            Element overrideElement = overridesElement.appendElement();
            overrideElement.setElement("fieldId", entry.fieldId.c_str());
            overrideElement.setElement("value", entry.value.c_str());
        }
    }

    return request;
}

inline Request ReferenceDataRequests::createTableOverrideRequest(
        const Service& service, const RequestOptions& requestOptions)
{
    Request request = service.createRequest("ReferenceDataRequest");

    Element securitiesElement = request.getElement("securities");
    for (const std::string& security : requestOptions.d_securities) {
        securitiesElement.appendValue(security.c_str());
    }

    Element fieldsElement = request.getElement("fields");
    for (const std::string& field : requestOptions.d_fields) {
        fieldsElement.appendValue(field.c_str());
    }

    // Add scalar overrides to request.
    Element overrides = request.getElement("overrides");
    Element override1 = overrides.appendElement();
    override1.setElement("fieldId", "ALLOW_DYNAMIC_CASHFLOW_CALCS");
    override1.setElement("value", "Y");
    Element override2 = overrides.appendElement();
    override2.setElement("fieldId", "LOSS_SEVERITY");
    override2.setElement("value", 31);

    // Add table overrides to request.
    Element tableOverrides = request.getElement("tableOverrides");
    Element tableOverride = tableOverrides.appendElement();
    tableOverride.setElement("fieldId", "DEFAULT_VECTOR");
    Element rows = tableOverride.getElement("row");

    // Layout of input table is specified by the definition of
    // 'DEFAULT_VECTOR'. Attributes are specified in the first rows.
    // Subsequent rows include rate, duration, and transition.
    Element row = rows.appendElement();
    Element cols = row.getElement("value");
    cols.appendValue("Anchor"); // Anchor type
    cols.appendValue("PROJ"); // PROJ = Projected
    row = rows.appendElement();
    cols = row.getElement("value");
    cols.appendValue("Type"); // Type of default
    cols.appendValue("CDR"); // CDR = Conditional Default Rate

    struct RateVector {
        float rate;
        int duration;
        const char *transition;
    } rateVectors[] = {
        { 1.0, 12, "S" }, // S = Step
        { 2.0, 12, "R" } // R = Ramp
    };

    for (int i = 0; i < sizeof(rateVectors) / sizeof(rateVectors[0]); ++i) {
        const RateVector& rateVector = rateVectors[i];

        row = rows.appendElement();
        cols = row.getElement("value");
        cols.appendValue(rateVector.rate);
        cols.appendValue(rateVector.duration);
        cols.appendValue(rateVector.transition);
    }

    return request;
}

inline void ReferenceDataRequests::processResponseEvent(const Event& event)
{
    static const Name SECURITY_DATA("securityData");
    static const Name SECURITY("security");
    static const Name FIELD_DATA("fieldData");
    static const Name SECURITY_ERROR("securityError");
    static const Name FIELD_EXCEPTIONS("fieldExceptions");
    static const Name FIELD_ID("fieldId");
    static const Name ERROR_INFO("errorInfo");
    static const Name REASON("reason");
    static const Name RESPONSE_ERROR("responseError");

    MessageIterator msgIter(event);
    while (msgIter.next()) {
        Message msg = msgIter.message();
        std::cout << "Received response to request " << msg.getRequestId()
                  << "\n";
        if (msg.hasElement(RESPONSE_ERROR)) {
            std::cout << "\tSECURITY FAILED: "
                      << msg.getElement(RESPONSE_ERROR);
            continue;
        }

        Element securities = msg.getElement(SECURITY_DATA);
        size_t numSecurities = securities.numValues();
        std::cout << "Processing " << numSecurities << " securities:" << '\n';
        for (size_t i = 0; i < numSecurities; ++i) {
            Element security = securities.getValueAsElement(i);
            std::string ticker = security.getElementAsString(SECURITY);
            std::cout << "\nTicker: " + ticker << '\n';
            if (security.hasElement(SECURITY_ERROR)) {
                std::cout << "\tSECURITY FAILED: "
                          << security.getElement(SECURITY_ERROR);
                continue;
            }

            if (security.hasElement(FIELD_DATA)) {
                const Element fields = security.getElement(FIELD_DATA);
                if (fields.numElements() > 0) {
                    std::cout << "FIELD\t\tVALUE\n";
                    std::cout << "-----\t\t-----\n";
                    size_t numElements = fields.numElements();
                    for (size_t j = 0; j < numElements; ++j) {
                        Element field = fields.getElement(j);
                        std::cout << field.name() << "\t\t";
                        field.print(std::cout) << '\n';
                    }
                }
            }
            std::cout << '\n';
            Element fieldExceptions = security.getElement(FIELD_EXCEPTIONS);
            if (fieldExceptions.numValues() > 0) {
                std::cout << "FIELD\t\tEXCEPTION\n";
                std::cout << "-----\t\t---------\n";
                for (size_t k = 0; k < fieldExceptions.numValues(); ++k) {
                    Element fieldException
                            = fieldExceptions.getValueAsElement(k);
                    Element errInfo = fieldException.getElement(ERROR_INFO);
                    std::cout << fieldException.getElementAsString(FIELD_ID)
                              << "\t\t";
                    errInfo.print(std::cout) << "\n";
                }
            }
        }
    }
}
}

#endif // INCLUDED_REFERENCEDATAREQUESTS
