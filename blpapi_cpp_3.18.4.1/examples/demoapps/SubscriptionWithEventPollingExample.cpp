/* Copyright 2021, Bloomberg Finance L.P.
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
#include <blpapi_correlationid.h>
#include <blpapi_event.h>
#include <blpapi_exception.h>
#include <blpapi_message.h>
#include <blpapi_name.h>
#include <blpapi_names.h>
#include <blpapi_session.h>
#include <blpapi_subscriptionlist.h>

#include <fstream>
#include <iostream>
#include <string>

#include <util/ConnectionAndAuthOptions.h>
#include <util/MaxEventsOption.h>
#include <util/SubscriptionOptions.h>
#include <util/Utils.h>

using namespace BloombergLP;
using namespace blpapi;

class SubscriptionWithEventPollingExample {
  public:
    void run(int argc, const char **argv)
    {
        ArgParser argParser("Subscription example with event polling",
                "SubscriptionWithEventPollingExample");
        ConnectionAndAuthOptions connectionAndAuthOptions(argParser);
        SubscriptionOptions subscriptionOptions(argParser);
        MaxEventsOption maxEventsOption(argParser);
        try {
            argParser.parse(argc, argv);
        } catch (std::exception& ex) {
            std::cerr << ex.what() << "\n";
            argParser.printHelp();
            return;
        }

        SessionOptions sessionOptions
                = connectionAndAuthOptions.createSessionOption();
        subscriptionOptions.setUpSessionOptions(sessionOptions);
        Session session(sessionOptions);
        if (!session.start()) {
            Utils::checkFailures(session);
            std::cerr << "Failed to start session.\n";
            return;
        }

        if (!session.openService(subscriptionOptions.getService().c_str())) {
            Utils::checkFailures(session);
            session.stop();
            return;
        }

        SubscriptionList subscriptions
                = subscriptionOptions.createSubscriptionList(
                        [](size_t, const std::string& topic) {
                            return CorrelationId(
                                    const_cast<char *>(topic.c_str()));
                        });
        session.subscribe(subscriptions);

        int eventCount = 0;
        bool done = false;
        while (!done) {
            Event event = session.nextEvent();
            Event::EventType eventType = event.eventType();

            MessageIterator msgIter(event);
            while (msgIter.next()) {
                Message msg = msgIter.message();
                msg.print(std::cout) << "\n";

                Name messageType = msg.messageType();
                CorrelationId msgCorrelationId = msg.correlationId();
                if (eventType == Event::SUBSCRIPTION_STATUS) {
                    if (messageType == Names::subscriptionFailure()
                            || messageType
                                    == Names::subscriptionTerminated()) {
                        const char *topic = static_cast<char *>(
                                msgCorrelationId.asPointer());
                        std::cout << "Subscription failed for topic " << topic
                                  << "\n";
                        Utils::printContactSupportMessage(msg);
                    }
                } else if (eventType == Event::SUBSCRIPTION_DATA) {
                    const char *topic = static_cast<char *>(
                            msgCorrelationId.asPointer());
                    std::cout << "Received subscription data for topic "
                              << topic << "\n";
                    if (msg.recapType() == Message::RecapType::e_solicited) {
                        if (msg.getRequestId() != nullptr) {
                            // An init paint tick can have an associated
                            // RequestId that is used to identify the source of
                            // the data and can be used when contacting
                            // support.
                            std::cout << "Received init paint with RequestId "
                                      << msg.getRequestId() << "\n";
                        }
                    }
                } else {
                    done = !Utils::processGenericMessage(eventType, msg);
                }
            }

            if (eventType == Event::SUBSCRIPTION_DATA) {
                if (++eventCount >= maxEventsOption.getMaxEvents()) {
                    break;
                }
            }
        }
    }
};

int main(int argc, const char **argv)
{
    SubscriptionWithEventPollingExample example;
    try {
        example.run(argc, argv);
    } catch (Exception& e) {
        std::cerr << "Library Exception!!! " << e.description() << "\n";
    }

    // wait for enter key to exit application
    std::cout << "Press ENTER to quit\n";
    char dummy[2];
    std::cin.getline(dummy, 2);
    return 0;
}
