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

#include "tokengenerator.h"

#include <blpapi_message.h>
#include <blpapi_name.h>

#include <iostream>

namespace blp = BloombergLP::blpapi;

namespace {
blp::Name TOKEN_SUCCESS("TokenGenerationSuccess");
blp::Name TOKEN_FAILURE("TokenGenerationFailure");
blp::Name TOKEN("token");
}

std::string TokenGenerator::generate(blp::EventQueue *queue)
{
    blp::EventQueue eventQueue;
    if (!queue) {
        queue = &eventQueue;
    }

    std::string token;
    d_session->generateToken(blp::CorrelationId(), queue);
    blp::Event event = queue->nextEvent();
    blp::MessageIterator iter(event);
    if (event.eventType() == blp::Event::TOKEN_STATUS
            || event.eventType() == blp::Event::REQUEST_STATUS) {
        blp::MessageIterator iter(event);
        while (iter.next()) {
            blp::Message msg = iter.message();
            if (msg.messageType() == TOKEN_SUCCESS) {
                token = msg.getElementAsString(TOKEN);
            } else if (msg.messageType() == TOKEN_FAILURE) {
                break;
            }
        }
    }

    return token;
}
