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

#ifndef _MOCKSESSION_H_
#define _MOCKSESSION_H_

#include <blpapi_correlationid.h>
#include <blpapi_event.h>
#include <blpapi_identity.h>
#include <blpapi_request.h>
#include <blpapi_requesttemplate.h>
#include <blpapi_service.h>
#include <blpapi_session.h>
#include <blpapi_subscriptionlist.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace blp = BloombergLP::blpapi;

class MockSession : public blp::Session {
  public:
    // It is important to pass a null handle to Session constructor. Without it
    // the Session will try to resolve/connect to Blooomberg endpoints which
    // may result in spurious warnings.
    MockSession()
        : blp::Session(0)
    {
    }

    MOCK_METHOD0(start, bool());

    MOCK_METHOD0(startAsync, bool());

    MOCK_METHOD0(stop, void());

    MOCK_METHOD0(stopAsync, void());

    MOCK_METHOD1(nextEvent, blp::Event(int));

    MOCK_METHOD1(tryNextEvent, int(blp::Event *));

    MOCK_METHOD1(openService, bool(const char *));

    MOCK_METHOD2(openServiceAsync,
            blp::CorrelationId(const char *, const blp::CorrelationId&));

    MOCK_METHOD4(sendAuthorizationRequest,
            blp::CorrelationId(const blp::Request&,
                    blp::Identity *,
                    const blp::CorrelationId&,
                    blp::EventQueue *));

    MOCK_METHOD1(cancel, void(const blp::CorrelationId&));

    MOCK_METHOD1(cancel, void(const std::vector<blp::CorrelationId>&));

    MOCK_METHOD2(cancel, void(const blp::CorrelationId *, size_t));

    MOCK_METHOD2(generateToken,
            blp::CorrelationId(const blp::CorrelationId&, blp::EventQueue *));

    MOCK_METHOD4(generateToken,
            blp::CorrelationId(const char *,
                    const char *,
                    const blp::CorrelationId&,
                    blp::EventQueue *));

    MOCK_CONST_METHOD1(
            getService, blp::Service(const char *serviceIdentifier));

    MOCK_METHOD0(createUserHandle, blp::UserHandle());

    MOCK_METHOD0(createIdentity, blp::Identity());

    MOCK_METHOD4(subscribe,
            void(const blp::SubscriptionList&,
                    const blp::Identity&,
                    const char *,
                    int));

    MOCK_METHOD3(
            subscribe, void(const blp::SubscriptionList&, const char *, int));

    MOCK_METHOD1(unsubscribe, void(const blp::SubscriptionList&));

    MOCK_METHOD1(resubscribe, void(const blp::SubscriptionList&));

    MOCK_METHOD3(resubscribe,
            void(const blp::SubscriptionList&, const char *, int));

    MOCK_METHOD2(resubscribe, void(const blp::SubscriptionList&, int));

    MOCK_METHOD4(resubscribe,
            void(const blp::SubscriptionList&, int, const char *, int));

    MOCK_METHOD2(setStatusCorrelationId,
            void(const blp::Service&, const blp::CorrelationId&));

    MOCK_METHOD3(setStatusCorrelationId,
            void(const blp::Service&,
                    const blp::Identity&,
                    const blp::CorrelationId&));

    MOCK_METHOD5(sendRequest,
            blp::CorrelationId(const blp::Request&,
                    const blp::CorrelationId&,
                    blp::EventQueue *,
                    const char *,
                    int));

    MOCK_METHOD6(sendRequest,
            blp::CorrelationId(const blp::Request&,
                    const blp::Identity&,
                    const blp::CorrelationId&,
                    blp::EventQueue *,
                    const char *,
                    int));

    MOCK_METHOD2(sendRequest,
            blp::CorrelationId(
                    const blp::RequestTemplate&, const blp::CorrelationId&));

    MOCK_METHOD3(createSnapshotRequestTemplate,
            blp::RequestTemplate(const char *,
                    const blp::CorrelationId&,
                    const blp::Identity&));
};

#endif
