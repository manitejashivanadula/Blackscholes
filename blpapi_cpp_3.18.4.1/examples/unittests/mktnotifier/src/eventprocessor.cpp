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

#include "eventprocessor.h"

#include <blpapi_message.h>

namespace blp = BloombergLP::blpapi;

bool EventProcessor::processEvent(
        const blp::Event& event, blp::Session *session)
{
    blp::MessageIterator msgIter(event);
    while (msgIter.next()) {
        blp::Message msg = msgIter.message();
        switch (event.eventType()) {
        case blp::Event::SESSION_STATUS:
            d_notifier->logSessionState(msg);
            break;
        case blp::Event::SUBSCRIPTION_STATUS:
            d_notifier->logSubscriptionState(msg);
            break;
        case blp::Event::SUBSCRIPTION_DATA:
            if (msg.hasElement("LAST_PRICE")) {
                double lastPrice = msg.getElementAsFloat64("LAST_PRICE");
                double result = d_computeEngine->someVeryComplexComputation(
                        lastPrice);
                d_notifier->sendToTerminal(result);
            }
            break;
        default:
            return true;
        }
    }
    return true;
}
