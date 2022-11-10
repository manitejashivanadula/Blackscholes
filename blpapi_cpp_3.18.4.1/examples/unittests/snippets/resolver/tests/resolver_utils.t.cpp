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

#include "resolver_utils.h"
#include "mockProviderSession.h"

#include <blpapi_testutil.h>

#include <blpapi_messageformatter.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <exception>

using namespace testing;

namespace {
// This namespace contain building blocks that are later used in the tests

blp::Name RESULT("result");
blp::Name REASON("reason");
blp::Name CATEGORY("category");
blp::Name NOT_AUTHORIZED("NOT_AUTHORIZED");
blp::Name TOPIC_PERMISSIONS("topicPermissions");
blp::Name PERMISSION_REQUEST("PermissionRequest");
blp::Name PERMISSION_RESPONSE("PermissionResponse");

const int ALLOWED_APP_ID(1234);
const int INVALID_APP_ID(4321);

blp::Event createPermissionEvent(
        const blp::CorrelationId cid, int applicationId);

blp::Message getFirstMessage(const blp::Event& event);

blp::Service getService();
}

// This test demonstrates how to mock interactions on objects of type
// `blp::ProviderSession`.
// In this test, we are setting the return value of the function, and
// verifying or the input parameters.
TEST(ResolverUtilTest, resolutionServiceRegistration)
{
    MockProviderSession mockSession;
    blp::Identity dummyIdentity;

    const char *serviceName = "//blp/mytestservice";
    blp::ServiceRegistrationOptions options;

    // Expected Actions
    EXPECT_CALL(mockSession, registerService(serviceName, _, _))
            .WillOnce(DoAll(SaveArg<2>(&options), Return(true)));

    resolutionServiceRegistration(mockSession, dummyIdentity, serviceName);

    // Test if the service is registered with the expected options.
    const int expectedPriority = 123;
    EXPECT_EQ(expectedPriority, options.getServicePriority());
    EXPECT_TRUE(options.getPartsToRegister()
            & blp::ServiceRegistrationOptions::PART_PUBLISHER_RESOLUTION);
}

// Successful permission response test
TEST(ResolverUtilTest, successfulResolution)
{
    MockProviderSession mockSession;
    blp::Service service = getService();

    blp::CorrelationId cid(1);
    blp::Event permissionEvent = createPermissionEvent(cid, ALLOWED_APP_ID);
    blp::Message permissionRequest = getFirstMessage(permissionEvent);

    blp::Event response;
    EXPECT_CALL(mockSession, sendResponse(_, false))
            .WillOnce(SaveArg<0>(&response));

    handlePermissionRequest(mockSession, service, permissionRequest);
    ASSERT_TRUE(Mock::VerifyAndClearExpectations(&mockSession));

    ASSERT_EQ(blp::Event::RESPONSE, response.eventType());
    blp::Message permissionResponse = getFirstMessage(response);
    EXPECT_EQ(cid, permissionResponse.correlationId());
    EXPECT_EQ(PERMISSION_RESPONSE, permissionResponse.messageType());
    ASSERT_TRUE(permissionResponse.hasElement(TOPIC_PERMISSIONS));

    blp::Element topicPermissions
            = permissionResponse.getElement(TOPIC_PERMISSIONS);

    const int topicCount = 2;
    ASSERT_EQ(2, topicPermissions.numValues());

    for (size_t i = 0; i < topicCount; ++i) {
        blp::Element topicPermission = topicPermissions.getValueAsElement(i);

        ASSERT_TRUE(topicPermission.hasElement(RESULT));
        EXPECT_EQ(0, topicPermission.getElementAsInt32(RESULT));
    }
}

// Failed permission response tests
TEST(ResolverUtilTest, failedResolution)
{
    MockProviderSession mockSession;
    blp::Service service = getService();

    blp::CorrelationId cid(1);
    blp::Event permissionEvent = createPermissionEvent(cid, INVALID_APP_ID);
    blp::Message permissionRequest = getFirstMessage(permissionEvent);

    blp::Event response;
    EXPECT_CALL(mockSession, sendResponse(_, false))
            .WillOnce(SaveArg<0>(&response));

    handlePermissionRequest(mockSession, service, permissionRequest);
    ASSERT_TRUE(Mock::VerifyAndClearExpectations(&mockSession));

    ASSERT_EQ(blp::Event::RESPONSE, response.eventType());
    blp::Message permissionResponse = getFirstMessage(response);
    EXPECT_EQ(cid, permissionResponse.correlationId());
    EXPECT_EQ(PERMISSION_RESPONSE, permissionResponse.messageType());

    ASSERT_TRUE(permissionResponse.hasElement(TOPIC_PERMISSIONS));
    blp::Element topicPermissions
            = permissionResponse.getElement(TOPIC_PERMISSIONS);

    const int topicCount = 2;
    ASSERT_EQ(topicCount, topicPermissions.numValues());

    for (size_t i = 0; i < topicCount; ++i) {
        blp::Element topicPermission = topicPermissions.getValueAsElement(i);

        ASSERT_TRUE(topicPermission.hasElement(RESULT));
        EXPECT_EQ(1, topicPermission.getElementAsInt32(RESULT));

        ASSERT_TRUE(topicPermission.hasElement(REASON));
        blp::Element reason = topicPermission.getElement(REASON);

        ASSERT_TRUE(reason.hasElement(CATEGORY));
        EXPECT_EQ(NOT_AUTHORIZED, reason.getElementAsString(CATEGORY));
    }
}

int main(int argc, char **argv)
{
    // The following line must be executed to initialize Google Mock
    // (and Google Test) before running the tests.
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}

namespace {

blp::Event createPermissionEvent(
        const blp::CorrelationId cid, int applicationId)
{
    blp::test::MessageProperties props;
    props.setCorrelationId(cid);

    // Create sample blpapi event
    blp::Event request = blp::test::TestUtil::createEvent(blp::Event::REQUEST);

    const blp::SchemaElementDefinition schemaDef
            = blp::test::TestUtil::getAdminMessageDefinition(
                    PERMISSION_REQUEST);

    std::ostringstream content;
    content << "{                                                 "
               "    \"topics\":        [ \"topic1\", \"topic2\"], "
               "    \"serviceName\":   \"//blp/mytestservice\",   "
               "    \"applicationId\": "
            << applicationId << "}";

    blp::test::MessageFormatter formatter
            = blp::test::TestUtil::appendMessage(request, schemaDef, props);

    formatter.formatMessageJson(content.str().c_str());

    return request;
}

blp::Service getService()
{
    const char *schema
            = "<ServiceDefinition "
              "xsi:schemaLocation=\"http://bloomberg.com/schemas/apidd "
              "apidd.xsd\""
              "                   name=\"test-svc\""
              "                   version=\"1.0.0.0\""
              "                   "
              "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
              "  <service name=\"//blp-test/test-svc\" version=\"1.0.0.0\">"
              "    <event name=\"Events\" eventType=\"EventType\"/>"
              "    <defaultServiceId>12345</defaultServiceId>"
              "    <publisherSupportsRecap>false</publisherSupportsRecap>"
              "    "
              "<authoritativeSourceSupportsRecap>false</"
              "authoritativeSourceSupportsRecap>"
              "    "
              "<SubscriberResolutionServiceId>12346</"
              "SubscriberResolutionServiceId>"
              "  </service>"
              "  <schema>"
              "      <sequenceType name=\"EventType\">"
              "         <element name=\"price\" type=\"Float64\" "
              "minOccurs=\"0\" maxOccurs=\"1\"/>"
              "      </sequenceType>"
              "   </schema>"
              "</ServiceDefinition>";

    std::istringstream stream(schema);

    blp::Service service = blp::test::TestUtil::deserializeService(stream);

    return service;
}

blp::Message getFirstMessage(const blp::Event& event)
{
    blp::MessageIterator iter(event);
    if (!iter.next()) {
        throw std::runtime_error("No messages in event");
    }
    return iter.message(true);
}
}
