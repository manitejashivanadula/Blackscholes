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

#include "subscriber.h"

namespace blp = BloombergLP::blpapi;

void Subscriber::subscribe(const std::string& service,
        const std::vector<std::string>& topics,
        const std::vector<std::string>& fields,
        const std::vector<std::string>& options,
        const blp::Identity& identity)
{
    blp::SubscriptionList subscriptions;
    for (size_t i = 0; i < topics.size(); ++i) {
        std::string topic(service + topics[i]);
        subscriptions.add(topic.c_str(),
                fields,
                options,
                blp::CorrelationId((char *)topics[i].c_str()));
    }

    d_session->subscribe(subscriptions, identity);
}
