/* Copyright 2019. Bloomberg Finance L.P.
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

#include <blpapi_testutil.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;

namespace blp = BloombergLP::blpapi;
namespace blptst = blp::test;

namespace {

const char *k_refdataschema("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\
<ServiceDefinition name=\"blp.refdata\" version=\"1.0.1.0\">\
   <service name=\"//blp/refdata\" version=\"1.0.0.0\">\
      <operation name=\"ReferenceDataRequest\" serviceId=\"84\">\
        <request>ReferenceDataRequest</request>\
        <response>Response</response>\
        <responseSelection>ReferenceDataResponse</responseSelection>\
      </operation>\
   </service>\
   <schema>\
    <sequenceType name=\"ReferenceDataRequest\">\
        <element name=\"securities\" type=\"String\" maxOccurs=\"unbounded\"/>\
        <element name=\"fields\" type=\"String\" maxOccurs=\"unbounded\"/>\
        <element name=\"overrides\" type=\"FieldOverride\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>\
    </sequenceType>\
    <sequenceType name=\"FieldOverride\">\
        <element name=\"fieldId\" type=\"String\"/>\
        <element name=\"value\" type=\"String\"/>\
    </sequenceType>\
    <choiceType name=\"Response\">\
        <element name=\"ReferenceDataResponse\" type=\"ReferenceDataResponseType\">\
            <cacheable>true</cacheable>\
            <cachedOnlyOnInitialPaint>false</cachedOnlyOnInitialPaint>\
        </element>\
    </choiceType>\
    <sequenceType name=\"ReferenceDataResponseType\">\
        <element name=\"responseError\" type=\"ErrorInfo\"       />\
        <element name=\"securityData\"  type=\"ReferenceSecurityData\"\
                                         minOccurs=\"1\" maxOccurs=\"unbounded\"/>\
    </sequenceType>\
    <sequenceType name=\"ReferenceSecurityData\">\
        <element name=\"security\"         type=\"String\"/>\
        <element name=\"eidData\"          type=\"Int64\"\
                                           minOccurs=\"0\" maxOccurs=\"unbounded\" />\
        <element name=\"securityError\"    type=\"ErrorInfo\"   \
                                           minOccurs=\"0\" maxOccurs=\"1\"/>\
        <element name=\"fieldExceptions\"  type=\"FieldException\"\
                                          minOccurs=\"0\" maxOccurs=\"unbounded\"/>\
        <element name=\"sequenceNumber\"  type=\"Int64\" \
                                          minOccurs=\"0\" maxOccurs=\"1\"/>\
        <element name=\"fieldData\" type=\"FieldData\"/>\
    </sequenceType>\
    <sequenceType name=\"FieldData\">\
      <description>The contents of this type depends on the response</description>\
        <element name=\"LAST_PRICE\" type=\"Float64\"  />\
        <element name=\"PX_LAST\"    type=\"Float64\"  />\
    </sequenceType>\
    <sequenceType name=\"FieldException\">\
      <element name=\"fieldId\"    type=\"String\"/> \
      <element name=\"errorInfo\"  type=\"ErrorInfo\"/>\
    </sequenceType>\
    <sequenceType name=\"ErrorInfo\">\
      <element name=\"source\"   type=\"String\" />\
      <element name=\"code\"     type=\"Int64\"   />\
      <element name=\"category\" type=\"String\"  />\
      <element name=\"message\"  type=\"String\"/>\
      <element name=\"subcategory\" type=\"String\"\
                                  minOccurs=\"0\" maxOccurs=\"1\"/>\
    </sequenceType>\
    </schema>\
</ServiceDefinition>");

blp::Name operationName("ReferenceDataRequest");

blp::Service getServiceFromString(const char *schema)
{
    std::istringstream schemaStream(schema);
    return blptst::TestUtil::deserializeService(schemaStream);
}

} // close anonymous namespace

// Concern:
// Verify that the provided sample refdata schema can be deserialized
TEST(RefdataTest, getService)
{
    blp::Service pingSvc = getServiceFromString(k_refdataschema);
}

// Concern:
// Verify that a successful ReferenceDataResponse can be created with the
// provided sample schema. Note that some of the fields are not filled in this
// example.
TEST(RefdataTest, RefdataResponseSuccess)
{
    blp::Service refdataSvc = getServiceFromString(k_refdataschema);

    blp::Event evt = blptst::TestUtil::createEvent(blp::Event::RESPONSE);

    const blp::SchemaElementDefinition schema
            = refdataSvc.getOperation(operationName).responseDefinition(0);

    blptst::MessageFormatter fmtter
            = blptst::TestUtil::appendMessage(evt, schema);

    const char *msgContent = "{                                        "
                             "  \"securityData\": [                    "
                             "    {                                    "
                             "      \"security\": \"IBM US Equity\",   "
                             "      \"fieldData\": {                   "
                             "        \"LAST_PRICE\": 138.533300       "
                             "      },                                 "
                             "      \"sequenceNumber\": 0              "
                             "    }                                    "
                             "  ]                                      "
                             "}                                        ";
    fmtter.formatMessageJson(msgContent);
}

// Concern:
// Verify that an unsuccessful ReferenceDataResponse can be created with the
// provided sample schema. Note that some of the fields are not filled in this
// example.
TEST(RefdataTest, RefdataResponseFailure)
{
    blp::Service refdataSvc = getServiceFromString(k_refdataschema);

    blp::Event evt = blptst::TestUtil::createEvent(blp::Event::RESPONSE);

    const blp::SchemaElementDefinition schema
            = refdataSvc.getOperation(operationName).responseDefinition(0);

    blptst::MessageFormatter fmtter
            = blptst::TestUtil::appendMessage(evt, schema);

    const char *msgContent = "{                                        "
                             "  \"securityData\": [                    "
                             "    {                                    "
                             "      \"security\": \"IBM US Equity\",   "
                             "      \"fieldData\": {                   "
                             "      },                                 "
                             "      \"fieldExceptions\": [ {           "
                             "         \"fieldId\": \"PX_LAST\",       "
                             "         \"errorInfo\": {                "
                             "           \"source\": \"src\",          "
                             "           \"code\": 5,                  "
                             "           \"category\": \"NO_AUTH\",    "
                             "           \"message\": \"Field ..\",    "
                             "           \"subcategory\": \"FIELD..\"  "
                             "         }                               "
                             "      }],                                "
                             "      \"sequenceNumber\": 0              "
                             "    }                                    "
                             "  ]                                      "
                             "}                                        ";
    fmtter.formatMessageJson(msgContent);
}

int main(int argc, char **argv)
{
    // The following line must be executed to initialize Google Mock (and
    // Google Test) before running the tests.
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
