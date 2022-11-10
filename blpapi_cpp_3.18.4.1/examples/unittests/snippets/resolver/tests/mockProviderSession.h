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

#ifndef MOCKPROVIDERSESSION_H_
#define MOCKPROVIDERSESSION_H_

#include <blpapi_providersession.h>

#include <gmock/gmock.h>

namespace blp = BloombergLP::blpapi;

class MockProviderSession : public blp::ProviderSession {
  public:
    // It is important to pass a null handle to ProviderSession constructor.
    // Without it the ProviderSession will try to resolve/connect to Blooomberg
    // endpoints which may result in spurious warnings.
    MockProviderSession()
        : ProviderSession(0)
    {
    }

    MOCK_METHOD0(start, bool());

    MOCK_METHOD0(startAsync, bool());

    MOCK_METHOD0(stop, void());

    MOCK_METHOD0(stopAsync, void());

    MOCK_METHOD1(nextEvent, blp::Event(int));

    MOCK_METHOD1(tryNextEvent, int(blp::Event *));

    MOCK_METHOD3(registerService,
            bool(const char *,
                    const blp::Identity&,
                    const blp::ServiceRegistrationOptions&));

    MOCK_METHOD4(
            activateSubServiceCodeRange, void(const char *, int, int, int));

    MOCK_METHOD3(deactivateSubServiceCodeRange, void(const char *, int, int));

    MOCK_METHOD4(registerServiceAsync,
            blp::CorrelationId(const char *,
                    const blp::Identity&,
                    const blp::CorrelationId&,
                    const blp::ServiceRegistrationOptions&));

    MOCK_METHOD1(deregisterService, bool(const char *));

    MOCK_METHOD3(resolve,
            void(blp::ResolutionList *, ResolveMode, const blp::Identity&));

    MOCK_METHOD3(resolveAsync,
            void(blp::ResolutionList *, ResolveMode, const blp::Identity&));

    MOCK_METHOD1(createTopic, blp::Topic(const blp::Message&));

    MOCK_METHOD1(getTopic, blp::Topic(const blp::Message&));

    MOCK_METHOD1(createServiceStatusTopic, blp::Topic(const blp::Service&));

    MOCK_METHOD1(publish, void(const blp::Event&));

    MOCK_METHOD2(sendResponse, void(const blp::Event&, bool));

    MOCK_METHOD3(createTopics,
            void(blp::TopicList *, ResolveMode, const blp::Identity&));

    MOCK_METHOD3(createTopicsAsync,
            void(blp::TopicList *, ResolveMode, const blp::Identity&));

    MOCK_METHOD1(deleteTopic, void(const blp::Topic&));

    MOCK_METHOD1(deleteTopics, void(const std::vector<blp::Topic>&));

    MOCK_METHOD2(deleteTopics, void(const blp::Topic *, size_t));

    MOCK_METHOD2(terminateSubscriptionsOnTopic,
            void(const blp::Topic&, const char *));

    MOCK_METHOD2(terminateSubscriptionsOnTopics,
            void(const std::vector<blp::Topic>&, const char *));

    MOCK_METHOD3(terminateSubscriptionsOnTopics,
            void(const blp::Topic *, size_t, const char *));

    MOCK_METHOD1(flushPublishedEvents, bool(int));

    MOCK_METHOD1(openService, bool(const char *));

    MOCK_METHOD2(openServiceAsync,
            blp::CorrelationId(const char *, const blp::CorrelationId&));

    MOCK_METHOD4(sendAuthorizationRequest,
            blp::CorrelationId(const blp::Request&,
                    blp::Identity *,
                    const blp::CorrelationId&,
                    blp::EventQueue *));

    MOCK_METHOD2(generateToken,
            blp::CorrelationId(const blp::CorrelationId&, blp::EventQueue *));

    MOCK_METHOD4(generateToken,
            blp::CorrelationId(const char *,
                    const char *,
                    const blp::CorrelationId&,
                    blp::EventQueue *));

    MOCK_CONST_METHOD1(
            getService, blp::Service(const char *serviceIdentifier));

    // MOCK_METHOD0(createUserHandle, blp::UserHandle()); This function is
    // deprecated and should not be used, consider using 'createIdentity'
    // instead of testing any existing use.

    MOCK_METHOD0(createIdentity, blp::Identity());
};

#endif // #ifndef _MOCKPROVIDERSESSION_H_
