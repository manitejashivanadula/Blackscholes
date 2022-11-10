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

#include <blpapi_event.h>
#include <blpapi_messageformatter.h>
#include <blpapi_request.h>
#include <blpapi_session.h>
#include <blpapi_sessionoptions.h>
#include <blpapi_testutil.h>

#include <fstream>
#include <sstream>
#include <testSchemas.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <computeengine.h>
#include <eventprocessor.h>
#include <mockComputeEngine.h>
#include <mockNotifier.h>
#include <mockSession.h>
#include <notifier.h>

namespace blp = BloombergLP::blpapi;
namespace blptst = blp::test;

namespace {
const blp::Name SESSION_STARTED("SessionStarted");
const blp::Name SUBSCRIPTION_STARTED("SubscriptionStarted");
const blp::Name MKTDATA_EVENTS("MarketDataEvents");
}

class EventProcessorTest : public testing::Test {
  protected:
    MockSession *d_session;
    MockNotifier *d_notifier;
    MockComputeEngine *d_computeEngine;
    blp::EventHandler *d_eventProcessor;

  public:
    virtual void SetUp()
    {
        d_session = new MockSession;
        d_notifier = new MockNotifier;
        d_computeEngine = new MockComputeEngine;
        d_eventProcessor = new EventProcessor(d_notifier, d_computeEngine);
    }

    virtual void TearDown()
    {
        delete d_eventProcessor;
        delete d_computeEngine;
        delete d_notifier;
        delete d_session;
    }
};

//
// Concern: Verify that notifier receives `SessionStarted' message.
// Plan:
//
// 1. Create a SessionStatus admin event using testutil's createEvent().
// 2. Obtain the message schema using testUtil's getAdminMessageDefinition().
// 3. Append a message of type SessionStarted using testutil's
//    appendMessage().
// 4. Setup expectation and save the message in a local variable. This is
//    message that is passed to processEvent() of EventProcessor().
// 5. Verify that the expected and actual messages are same.
//
TEST_F(EventProcessorTest, NotifierReceivesSessionStarted)
{
    blp::Event event
            = blptst::TestUtil::createEvent(blp::Event::SESSION_STATUS);
    const blp::SchemaElementDefinition schemaDef
            = blptst::TestUtil::getAdminMessageDefinition(SESSION_STARTED);

    blptst::TestUtil::appendMessage(event, schemaDef);

    blp::Message actualMessage(0);
    EXPECT_CALL(*d_notifier, logSessionState(testing::_))
            .WillOnce(testing::SaveArg<0>(&actualMessage));

    d_eventProcessor->processEvent(event, d_session);

    ASSERT_EQ(SESSION_STARTED, actualMessage.messageType());
}

//
// Concern: Verify that notifier receives `SubscriptionStarted' message.
// Plan:
//
// 1. Create a SubscriptionStatus admin event using testutil's
//    createEvent().
// 2. Obtain the schema for SubscriptionStarted message by calling
//    getAdminMessageDefinition().
// 3. Append a message of type `SubscriptionStarted' using testutil's
//    appendMessage().
// 4. Setup expectation and save the message in a local variable. This is
//    message that is passed to `processEvent()` of `EventProcessor()`.
// 5. Verify that the expected and actual messages are same.
//
TEST_F(EventProcessorTest, NotifierReceivesSubscriptionStarted)
{
    blp::Event event
            = blptst::TestUtil::createEvent(blp::Event::SUBSCRIPTION_STATUS);
    const blp::SchemaElementDefinition schemaDef
            = blptst::TestUtil::getAdminMessageDefinition(
                    SUBSCRIPTION_STARTED);

    blptst::TestUtil::appendMessage(event, schemaDef);

    blp::Message actualMessage(0);
    EXPECT_CALL(*d_notifier, logSubscriptionState(testing::_))
            .WillOnce(testing::SaveArg<0>(&actualMessage));

    d_eventProcessor->processEvent(event, d_session);

    ASSERT_EQ(SUBSCRIPTION_STARTED, actualMessage.messageType());
}

//
// Concern
// Verify that:
// 1. Compute engine receives correct LAST_PRICE.
// 2. It performs correct computation and returns correct value.
// 3. Value is sent to terminal.
//
// Plan:
// 1. Obtain the service by deserializing its schema.
// 2. Create a SubscriptionEvent using testutil's createEvent().
// 3. Obtain the element schema definition from the service.
// 4. Append a message of type `MarketDataEvents' using testutil's
//    appendMessage().
// 5. Format the message using formatter returned by appendMessage().
//    In this example the message has body as represented by
//    "{" "\"LAST_PRICE\": 142.80" "}".
// 6. Setup expectation and save the expected values.
// 7. Verify that the expected and actual values are same.
//
TEST_F(EventProcessorTest, NotifierReceivesSubscriptionData)
{
    std::istringstream schemaStream(getMktDataSchemaString());
    blp::Service service = blptst::TestUtil::deserializeService(schemaStream);

    // MktData service only support flat schema.
    const char *messageContent = "{"
                                 "\"LAST_PRICE\": 142.80"
                                 "}";

    const blp::SchemaElementDefinition schemaDef
            = service.getEventDefinition(MKTDATA_EVENTS);

    blp::Event event
            = blptst::TestUtil::createEvent(blp::Event::SUBSCRIPTION_DATA);
    blptst::MessageFormatter formatter
            = blptst::TestUtil::appendMessage(event, schemaDef);
    formatter.formatMessageJson(messageContent);

    const double k_expectedLastPrice = 142.80;
    const double k_expectedComputeResult = 200.0;

    EXPECT_CALL(
            *d_computeEngine, someVeryComplexComputation(k_expectedLastPrice))
            .WillOnce(testing::Return(k_expectedComputeResult));

    EXPECT_CALL(*d_notifier, sendToTerminal(k_expectedComputeResult));

    d_eventProcessor->processEvent(event, d_session);
}
