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

#ifndef _NOTIFIER_H_
#define _NOTIFIER_H_

#include <blpapi_message.h>
#include <iostream>

namespace blp = BloombergLP::blpapi;

class INotifier {
  public:
    virtual void logSessionState(const blp::Message& msg) = 0;

    virtual void logSubscriptionState(const blp::Message& msg) = 0;

    virtual void sendToTerminal(double value) = 0;

    virtual ~INotifier() { }
};

// Notifier is an approximation of a class that attempts to:
// 1. Log all session events (for example SessionStartupFailure).
// 2. Log all subscription events (for example SubscriptionStarted).
// 3. Deliver subscription data to terminal ("Bloomberg terminal").
class Notifier : public INotifier {
  public:
    virtual void logSessionState(const blp::Message& msg)
    {
        std::cout << "Logging Session state with:\n";
        msg.print(std::cout) << std::endl;
    }

    virtual void logSubscriptionState(const blp::Message& msg)
    {
        std::cout << "Logging Subscription state with:\n";
        msg.print(std::cout) << std::endl;
    }

    virtual void sendToTerminal(double value)
    {
        std::cout << "VALUE = " << value << std::endl;
    }
};

#endif
