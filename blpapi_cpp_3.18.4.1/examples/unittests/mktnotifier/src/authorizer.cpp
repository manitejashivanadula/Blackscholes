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

#include "authorizer.h"

#include <blpapi_message.h>
#include <blpapi_name.h>
#include <time.h>

namespace blp = BloombergLP::blpapi;

namespace {
blp::Name AUTHORIZATION_SUCCESS("AuthorizationSuccess");
blp::Name TOKEN("token");
const char AUTH_SERVICE[] = "//blp/apiauth";
}

bool Authorizer::authorize(blp::Identity *identity,
        const blp::Service& authService,
        const blp::CorrelationId& cid,
        blp::EventQueue *queue)
{
    std::string token = d_tokenGenerator->generate();

    if (token.length() == 0) {
        std::cout << "Failed to get token" << std::endl;
        return false;
    }

    blp::Request authRequest = authService.createAuthorizationRequest();
    authRequest.set(TOKEN, token.c_str());

    blp::EventQueue eventQueue;
    if (!queue) {
        queue = &eventQueue;
    }
    d_session->sendAuthorizationRequest(authRequest, identity, cid, queue);

    time_t startTime = time(0);
    const int WAIT_TIME_SECONDS = 10;
    while (true) {
        blp::Event event = queue->nextEvent(WAIT_TIME_SECONDS * 1000);
        if (event.eventType() == blp::Event::RESPONSE
                || event.eventType() == blp::Event::REQUEST_STATUS
                || event.eventType() == blp::Event::PARTIAL_RESPONSE
                || event.eventType() == blp::Event::AUTHORIZATION_STATUS) {
            blp::MessageIterator msgIter(event);
            while (msgIter.next()) {
                blp::Message msg = msgIter.message();
                if (msg.messageType() == AUTHORIZATION_SUCCESS) {
                    return true;
                } else {
                    return false;
                }
            }
        }

        time_t endTime = time(0);
        if (endTime - startTime > WAIT_TIME_SECONDS) {
            return false;
        }
    }
}

bool Authorizer::authorize(blp::Identity *identity,
        const std::string& authOptions,
        blp::EventQueue *queue)
{
    if (!authOptions.empty()) {
        *identity = d_session->createIdentity();
        if (d_session->openService(AUTH_SERVICE)) {
            blp::Service service = d_session->getService(AUTH_SERVICE);
            blp::CorrelationId cid((void *)"auth");
            return authorize(identity, service, cid, queue);
        }
    }

    return true;
}
