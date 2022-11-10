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

#include <authorizer.h>
#include <eventqueue.h>
#include <mockSession.h>
#include <tokengenerator.h>

#include <blpapi_event.h>
#include <blpapi_identity.h>
#include <blpapi_testutil.h>

#include <fstream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <messageTypes.h>
#include <testSchemas.h>

using namespace testing;

namespace blp = BloombergLP::blpapi;
namespace blptst = blp::test;

class MockTokenGenerator : public ITokenGenerator {
  public:
    MOCK_METHOD1(generate, std::string(blp::EventQueue *));
};

class AuthorizerTest : public testing::Test {
  protected:
    MockSession *d_session;
    MockTokenGenerator *d_tokenGenerator;
    IAuthorizer *d_authorizer;
    MockEventQueue *d_queue;

  public:
    virtual void SetUp()
    {
        d_session = new MockSession;
        d_tokenGenerator = new MockTokenGenerator;
        d_authorizer = new Authorizer(d_session, d_tokenGenerator);
        d_queue = new MockEventQueue;
    }

    virtual void TearDown()
    {
        delete d_queue;
        delete d_authorizer;
        delete d_tokenGenerator;
        delete d_session;
    }
};

// Concern: Verify that for a valid identity the authorization returns true.
//
// Plan:
// 1. Create blp::Service from apiAuth schema.
// 2. Set expectation to:
//    a. Obtain a valid identity from session.
//    b. Verify that service is opened and successfully retrieved.
//    c. Obtain a unique token.
//    d. Send an authorization request.
// 3. Create admin event to represent the authorization success.
// 4. Set up expectation to return the aforementioned event.
// 5. Verify the authorize returns true.
//
TEST_F(AuthorizerTest, AuthorizationSuccess)
{
    std::istringstream schemaStream(getApiAuthSchemaString());
    blp::Service service = blptst::TestUtil::deserializeService(schemaStream);

    blp::Identity identity;
    EXPECT_CALL(*d_session, createIdentity()).WillOnce(Return(identity));

    EXPECT_CALL(*d_session, openService(_)).WillOnce(Return(true));
    EXPECT_CALL(*d_session, getService(_)).WillOnce(Return(service));

    std::string token("abcdefg");
    EXPECT_CALL(*d_tokenGenerator, generate(_)).WillOnce(Return(token));

    blp::CorrelationId cid;
    EXPECT_CALL(*d_session, sendAuthorizationRequest(_, _, _, _))
            .WillOnce(DoAll(SaveArg<2>(&cid), Return(blp::CorrelationId())));

    blp::Event event = blptst::TestUtil::createEvent(blp::Event::RESPONSE);
    const blp::SchemaElementDefinition schemaDef
            = blptst::TestUtil::getAdminMessageDefinition(
                    AUTHORIZATION_SUCCESS);

    blptst::MessageFormatter formatter
            = blptst::TestUtil::appendMessage(event, schemaDef);

    // AuthorizationSuccess message does not have any fields, therefore
    // no formatting is required.

    EXPECT_CALL(*d_queue, nextEvent(_)).WillOnce(Return(event));

    bool res = d_authorizer->authorize(&identity, "auth_options", d_queue);

    ASSERT_TRUE(res);
}

// Concern: Verify that for a invalid identity the authorization returns false.
//
// Plan:
// 1. Create blp::Service from apiAuth schema.
// 2. Set expectation to:
//    a. Obtain a valid identity from session.
//    b. Verify that service is opened and successfully retrieved.
//    c. Obtain a unique token.
//    d. Send an authorization request.
// 3. Create and format an event to represent the authorization failure.
// 4. Set up expectation to return the aforementioned event.
// 5. Verify the authorize returns false.
//
TEST_F(AuthorizerTest, AuthorizationFailure)
{
    std::istringstream schemaStream(getApiAuthSchemaString());
    blp::Service service = blptst::TestUtil::deserializeService(schemaStream);

    blp::Identity identity;
    EXPECT_CALL(*d_session, createIdentity()).WillOnce(Return(identity));

    EXPECT_CALL(*d_session, openService(_)).WillOnce(Return(true));
    EXPECT_CALL(*d_session, getService(_)).WillOnce(Return(service));

    std::string token("abcdefg");
    EXPECT_CALL(*d_tokenGenerator, generate(_)).WillOnce(Return(token));

    blp::CorrelationId cid;
    EXPECT_CALL(*d_session, sendAuthorizationRequest(_, _, _, _))
            .WillOnce(DoAll(SaveArg<2>(&cid), Return(blp::CorrelationId())));

    const char *messageContent = "{                                        "
                                 "  \"reason\": {                          "
                                 "      \"code\": 101,                     "
                                 "      \"message\": \"Invalid user\",     "
                                 "      \"category\": \"BAD_ARGS\",        "
                                 "      \"subcategory\": \"INVALID_USER\", "
                                 "      \"source\": \"test-source\"        "
                                 "    }                                    "
                                 "}                                        ";

    blp::Name authRequest("AuthorizationRequest");
    blp::Event event = blptst::TestUtil::createEvent(blp::Event::RESPONSE);

    const blp::SchemaElementDefinition schemaDef
            = service.getOperation(authRequest)
                      .responseDefinition(AUTHORIZATION_FAILURE);

    blptst::MessageFormatter formatter
            = blptst::TestUtil::appendMessage(event, schemaDef);

    formatter.formatMessageJson(messageContent);

    EXPECT_CALL(*d_queue, nextEvent(_)).WillOnce(Return(event));

    bool res = d_authorizer->authorize(&identity, "auth_options", d_queue);

    ASSERT_FALSE(res);
}
