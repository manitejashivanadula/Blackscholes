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

#include <appconfig.h>
#include <application.h>
#include <authorizer.h>
#include <eventprocessor.h>
#include <mockSession.h>
#include <subscriber.h>

#include <blpapi_session.h>
#include <blpapi_sessionoptions.h>
#include <blpapi_testutil.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace testing;

namespace blp = BloombergLP::blpapi;
namespace blptst = blp::test;

class MockAuthorizer : public IAuthorizer {
  public:
    MOCK_METHOD3(authorize,
            bool(blp::Identity *identity,
                    const std::string& authOptions,
                    blp::EventQueue *queue));
};

class MockSubscriber : public ISubscriber {
  public:
    MOCK_METHOD5(subscribe,
            void(const std::string& service,
                    const std::vector<std::string>& topics,
                    const std::vector<std::string>& fields,
                    const std::vector<std::string>& options,
                    const blp::Identity& identity));
};

class MockEventProcessor : public blp::EventHandler {
  public:
    MOCK_METHOD2(processEvent,
            bool(const blp::Event& event, blp::Session *session));
};

class ApplicationTest : public testing::Test {
  protected:
    MockSession d_session;
    MockAuthorizer d_authorizer;
    MockSubscriber d_subscriber;
    MockEventProcessor d_eventProcessor;
    AppConfig d_config;
    Application d_application;

  public:
    ApplicationTest()
        : d_session()
        , d_authorizer()
        , d_subscriber()
        , d_eventProcessor()
        , d_config()
        , d_application(&d_session,
                  &d_authorizer,
                  &d_subscriber,
                  &d_eventProcessor,
                  &d_config)
    {
    }
};

//
// Concern:
// Verify that if Session fails to start, no authorization and
// subscriptions are made.
//
// Plan:
// Set up expectation to:
// a. start() to return false.
// b. authorize() is not called.
// c. subscribe() is not called.
//
TEST_F(ApplicationTest, SessionStartFail)
{
    EXPECT_CALL(d_session, start()).WillOnce(Return(false));
    EXPECT_CALL(d_authorizer, authorize(_, _, _)).Times(0);
    EXPECT_CALL(d_subscriber, subscribe(_, _, _, _, _)).Times(0);
    d_application.run();
}

//
// Concern:
// Verify that if authorization fails, no subscriptions are made.
//
// Plan:
// Set up expectaion to:
// a. start() to return true.
// b. authorize() fails and returns false.
// c. subscribe() is not called.
//
TEST_F(ApplicationTest, SessionAuthorizeFail)
{
    EXPECT_CALL(d_session, start()).WillOnce(Return(true));
    EXPECT_CALL(d_authorizer, authorize(_, _, _)).WillOnce(Return(false));
    EXPECT_CALL(d_subscriber, subscribe(_, _, _, _, _)).Times(0);
    d_application.run();
}

//
// Concern:
// Verify correct auth service and auth options are used for authorization.
//
TEST_F(ApplicationTest, AuthorizeWithConfig)
{
    d_config.d_authOptions = "app=teknavo:BBOX";
    EXPECT_CALL(d_session, start()).WillOnce(Return(true));

    std::string authOptions;

    EXPECT_CALL(d_authorizer, authorize(_, _, _))
            .WillOnce(DoAll(SaveArg<1>(&authOptions), Return(true)));

    EXPECT_CALL(d_subscriber, subscribe(_, _, _, _, _)).Times(1);
    d_application.run();
    ASSERT_EQ(authOptions, d_config.d_authOptions);
}

//
// Concern:
// Verify that correct service, topic and fields are used when
// subscribing session.
//
TEST_F(ApplicationTest, SubscribeWithConfig)
{
    std::vector<std::string> topics;
    topics.push_back("IBM US Equity");
    topics.push_back("MSFT US Equity");

    std::vector<std::string> fields;
    fields.push_back("LAST_PRICE");
    fields.push_back("BID");
    fields.push_back("ASK");

    d_config.d_service = "//blp/mktdata";
    d_config.d_topics = topics;
    d_config.d_fields = fields;

    EXPECT_CALL(d_session, start()).WillOnce(Return(true));
    EXPECT_CALL(d_authorizer, authorize(_, _, _)).WillOnce(Return(true));

    std::vector<std::string> topicsSaved;
    std::vector<std::string> fieldsSaved;

    EXPECT_CALL(d_subscriber, subscribe(_, _, _, _, _))
            .WillOnce(
                    DoAll(SaveArg<1>(&topicsSaved), SaveArg<2>(&fieldsSaved)));
    d_application.run();
    ASSERT_EQ(topicsSaved, d_config.d_topics);
    ASSERT_EQ(fieldsSaved, d_config.d_fields);
}
