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

#include <eventprocessor.h>
#include <eventqueue.h>
#include <mockSession.h>
#include <tokengenerator.h>

#include <blpapi_event.h>
#include <blpapi_session.h>
#include <blpapi_testutil.h>

#include <fstream>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <messageTypes.h>
#include <mockComputeEngine.h>
#include <mockNotifier.h>

using namespace testing;

namespace blp = BloombergLP::blpapi;
namespace blptst = blp::test;

class TokenGeneratorTest : public testing::Test {
  protected:
    MockSession *d_session;
    MockEventQueue *d_queue;
    TokenGenerator *d_tokenGenerator;
    blp::EventHandler *d_eventProcessor;
    MockNotifier *d_notifier;
    MockComputeEngine *d_computeEngine;

  public:
    virtual void SetUp()
    {
        d_session = new MockSession;
        d_queue = new MockEventQueue;
        d_tokenGenerator = new TokenGenerator(d_session);
    }

    virtual void TearDown()
    {
        delete d_tokenGenerator;
        delete d_queue;
        delete d_session;
    }
};

//
// Concern: Verify that in case of successful token generation, a valid token
// is received by the application.
//
// Plan:
// 1. Create a TokenStatus admin event using testutil's createEvent().
// 2. Obtain schema for TokenGenerationSuccess using testutil's
//    getAdminMessageDefinition().
// 3. Append a message of type TokenGenerationSuccess using testutil's
//    appendMessage().
// 4. Using the returned formatter, format the message. In this example
//    the message has body "{" "\"token\": \"dummyToken\"" "}".
//    `token` is the element name, and `dummyToken` is the token value
//    which will be delivered to client application.
// 5. Setup expectation and return the appropriate event.
// 6. Verify that the expected token value and received token values are same.
//

TEST_F(TokenGeneratorTest, TokenGenerationSuccess)
{
    blp::CorrelationId cid;

    const std::string k_expectedToken("dummyToken");

    const char *messageContent = "{"
                                 "\"token\": \"dummyToken\""
                                 "}";

    blp::Event event = blptst::TestUtil::createEvent(blp::Event::TOKEN_STATUS);
    const blp::SchemaElementDefinition schemaDef
            = blptst::TestUtil::getAdminMessageDefinition(TOKEN_SUCCESS);

    blptst::MessageFormatter formatter
            = blptst::TestUtil::appendMessage(event, schemaDef);

    formatter.formatMessageJson(messageContent);

    EXPECT_CALL(*d_session, generateToken(_, _)).WillOnce(Return(cid));
    EXPECT_CALL(*d_queue, nextEvent(_)).WillOnce(Return(event));

    std::string actualToken = d_tokenGenerator->generate(d_queue);
    ASSERT_EQ(k_expectedToken, actualToken);
}

//
// Concern: Verify that in case of failure in token generation, an empty token
// is received by the application.
//
// Plan:
// 1. Create a TokenStatus admin event using testutil's createEvent().
// 2. Obtain schema for TokenGenerationFailure using testutil's
//    getAdminMessageDefinition().
// 3. Append a message of type TokenGenerationFailure using testutil's
//    appendMessage().
// 4. Using the returned formatter, format the message. In this example
//    the message has body which contains the reason of failure.
//    The reason is delivered to the user application.
// 5. Setup expectation and return the appropriate event.
// 6. Verify that the actual token is empty.
//
TEST_F(TokenGeneratorTest, TokenGenerationFailure)
{
    blp::CorrelationId cid;

    const char *messageContent = "{"
                                 "\"reason\": "
                                 "{"
                                 "\"source\": \"apitkns (apiauth) on n795\","
                                 "\"category\": \"NO_AUTH\","
                                 "\"errorCode\": 3,"
                                 "\"description\": \"App not in emrs ...\","
                                 "\"subcategory\": \"INVALID_APP\""
                                 "}"
                                 "}";

    blp::Event event = blptst::TestUtil::createEvent(blp::Event::TOKEN_STATUS);
    const blp::SchemaElementDefinition schemaDef
            = blptst::TestUtil::getAdminMessageDefinition(TOKEN_FAILURE);

    blptst::MessageFormatter formatter
            = blptst::TestUtil::appendMessage(event, schemaDef);

    formatter.formatMessageJson(messageContent);

    EXPECT_CALL(*d_session, generateToken(_, _)).WillOnce(Return(cid));
    EXPECT_CALL(*d_queue, nextEvent(_)).WillOnce(Return(event));

    std::string actualToken = d_tokenGenerator->generate(d_queue);
    ASSERT_TRUE(actualToken.empty());
}
