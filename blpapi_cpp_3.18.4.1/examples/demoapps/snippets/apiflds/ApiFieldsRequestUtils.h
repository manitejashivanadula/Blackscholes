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
#ifndef INCLUDED_APIFIELDSREQUESTUTILS
#define INCLUDED_APIFIELDSREQUESTUTILS

#include <blpapi_element.h>
#include <blpapi_name.h>

#include <iostream>
#include <string>

using namespace BloombergLP;
using blpapi::Element;
using blpapi::Name;

struct ApiFieldsRequestUtils {
    static void printHeader();
    static void printField(const Element& field);
    static std::string padString(const std::string& str, unsigned int width);

    static const int ID_LEN = 13;
    static const int MNEMONIC_LEN = 36;
    static const int DESC_LEN = 40;
};

inline void ApiFieldsRequestUtils::printHeader()
{
    std::cout << padString("FIELD ID", ID_LEN)
              << padString("MNEMONIC", MNEMONIC_LEN)
              << padString("DESCRIPTION", DESC_LEN) << "\n"
              << padString("-----------", ID_LEN)
              << padString("-----------", MNEMONIC_LEN)
              << padString("-----------", DESC_LEN) << "\n";
}

inline std::string ApiFieldsRequestUtils::padString(
        const std::string& str, unsigned int width)
{
    static const int PADDING_LENGTH = 45;

    if (str.length() >= width || str.length() >= PADDING_LENGTH) {
        return str;
    }

    return str + std::string(width - str.length(), ' ');
}

inline void ApiFieldsRequestUtils::printField(const Element& field)
{
    static const Name FIELD_ID("id");
    static const Name FIELD_INFO("fieldInfo");
    static const Name FIELD_MNEMONIC("mnemonic");
    static const Name FIELD_DESC("description");
    static const Name FIELD_ERROR("fieldError");
    static const Name FIELD_MSG("message");

    std::string fieldId = field.getElementAsString(FIELD_ID);
    if (field.hasElement(FIELD_INFO)) {
        Element fieldInfo = field.getElement(FIELD_INFO);
        std::string fieldMnemonic
                = fieldInfo.getElementAsString(FIELD_MNEMONIC);
        std::string fieldDesc = fieldInfo.getElementAsString(FIELD_DESC);

        std::cout << padString(fieldId, ID_LEN)
                  << padString(fieldMnemonic, MNEMONIC_LEN)
                  << padString(fieldDesc, DESC_LEN) << "\n";
    } else {
        Element fieldError = field.getElement(FIELD_ERROR);
        std::string errorMsg = fieldError.getElementAsString(FIELD_MSG);

        std::cout << "\nERROR: " << fieldId << " - " << errorMsg << "\n";
    }
}

#endif // INCLUDED_APIFIELDSREQUESTUTILS
