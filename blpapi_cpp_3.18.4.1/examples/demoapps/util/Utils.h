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

#ifndef INCLUDED_UTILS
#define INCLUDED_UTILS

//@PURPOSE: Contains all the utility functions used in the examples and
// argument processing.
//
//@CLASSES:
// Utils: provides utiltiy functions to examples and argument processing.
//
//@DESCRIPTION: See above.

#include <blpapi_element.h>
#include <blpapi_event.h>
#include <blpapi_exception.h>
#include <blpapi_message.h>
#include <blpapi_name.h>
#include <blpapi_names.h>
#include <blpapi_request.h>
#include <blpapi_session.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace BloombergLP {

class Utils {
    // Contains all the utility functions used in the examples and argument
    // processing.
  public:
    static std::vector<std::string> split(const std::string& str, char delim);
    // Splits the provided string on the passed delimiter and returns
    // splits in a vector.

    template <typename Container,
            typename Element = typename Container::value_type>
    static std::string join(
            const Container& container, const char *delimiter = nullptr);
    // Concatinates the elements inside the container separated by the
    // provided delimiter, and returns them as a string.
    //
    // NOTE:Container's interface must provide a `begin()` and an `end()`
    // functions.

    static std::string getFormattedCurrentTime();
    // Returns current time in 'YYYY/MM/DD HH:MM:SS.MILLISEC' format.

    static double getCurrentTimeMilliSec();
    // Provides current time in milliseconds.

    static bool processGenericMessage(
            blpapi::Event::EventType eventType, blpapi::Message message);
    // Prints the error if the 'message' is a failure message.
    //
    // When using a session identity, i.e.
    // 'SessionOptions::setSessionIdentityOptions(AuthOptions)', token
    // generation failure, authorization failure or revocation terminates
    // the session, in which case, applications only need to check session
    // status messages. Applications don't need to handle token or
    // authorization messages.
    //
    // Returns 'false' if session has failed to start or terminated, 'true'
    // otherwise.

    static bool processGenericEvent(const blpapi::Event& event);
    // Print the messages in the 'event'.  If the 'event' is a
    // 'SESSION_STATUS' event and it indicates the end of the session
    // return 'false'.

    static void checkFailures(blpapi::AbstractSession& session);
    // Checks failure events published by the session.
    //
    // Note that the loop uses 'blpapi::Session::tryNextEvent()' as all
    // events have been produced before calling this function, but there
    // could be no events at all in the queue if the OS fails to allocate
    // resources.

    static void printContactSupportMessage(const blpapi::Message& msg);
    // Prints contact support message.
    //
    // 'blpapi::Message' can have an associated 'RequestId' that is used to
    // identify the operation through the network. When contacting support
    // please provide the 'RequestId'.

    static void printEvent(const blpapi::Event& event);

    static int getNextIntegerCid();
};

inline std::string Utils::getFormattedCurrentTime()
{
    using sys_clock = std::chrono::system_clock;
    using time_point = std::chrono::time_point<sys_clock>;
    using milli_sec = std::chrono::milliseconds;

    time_point now = sys_clock::now();
    auto timer = std::chrono::system_clock::to_time_t(now);
    std::tm *bt = std::localtime(&timer);

    auto ms = std::chrono::duration_cast<milli_sec>(now.time_since_epoch())
            % 1000;

    std::ostringstream oss;

    oss << std::put_time(bt, "%Y/%m/%d %T");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return oss.str();
}

inline void Utils::printContactSupportMessage(const blpapi::Message& msg)
{
    const char *requestId = msg.getRequestId();
    if (!requestId) {
        return;
    }

    std::cout << "When contacting support please provide this request id: "
              << requestId << "'\n";
}

inline bool Utils::processGenericMessage(
        blpapi::Event::EventType eventType, blpapi::Message message)
{
    blpapi::Name messageType = message.messageType();
    if (eventType == blpapi::Event::SESSION_STATUS) {
        if (messageType == blpapi::Names::sessionTerminated()
                || messageType == blpapi::Names::sessionStartupFailure()) {
            std::cout << "Session failed to start or terminated\n";
            printContactSupportMessage(message);
            return false;
        }
    } else if (eventType == blpapi::Event::SERVICE_STATUS) {
        if (messageType == blpapi::Names::serviceOpenFailure()) {
            const char *serviceName
                    = message.getElementAsString("serviceName");
            std::cout << "Failed to open " << serviceName << "\n";
            printContactSupportMessage(message);
        } else if (messageType == blpapi::Names::serviceRegisterFailure()) {
            printContactSupportMessage(message);
        }
    }

    return true;
}

inline bool Utils::processGenericEvent(const blpapi::Event& event)
{
    blpapi::MessageIterator msgIter(event);
    while (msgIter.next()) {
        blpapi::Message msg = msgIter.message();
        msg.print(std::cout) << "\n";
        if (!processGenericMessage(event.eventType(), msg)) {
            return false;
        }
    }
    return true;
}

inline void Utils::checkFailures(blpapi::AbstractSession& session)
{
    blpapi::Event event;
    while (!session.tryNextEvent(&event)) {
        if (!processGenericEvent(event)) {
            break;
        }
    }
}

inline std::vector<std::string> Utils::split(
        const std::string& str, char delim)
{
    std::string::size_type start = 0u, pos = 0u;
    std::vector<std::string> tokens;

    while ((pos = str.find(delim, start)) != std::string::npos) {
        tokens.push_back(str.substr(start, pos - start));
        start = pos + 1;
    }

    if (start != str.size()) {
        tokens.push_back(str.substr(start));
    }

    return tokens;
}

template <typename Container, typename Element>
inline std::string Utils::join(
        const Container& container, const char *delimiter)
{
    std::ostringstream os;
    auto start = std::begin(container);
    auto end = std::end(container);

    if (start != end) {
        auto last = std::prev(end);
        std::copy(start, last, std::ostream_iterator<Element>(os, delimiter));

        // Copy last element.
        os << *last;
    }

    return os.str();
}

inline double Utils::getCurrentTimeMilliSec()
{
    auto timeMilliSec
            = std::chrono::time_point_cast<std::chrono::milliseconds>(
                    std::chrono::high_resolution_clock::now());
    return timeMilliSec.time_since_epoch().count() / 1000.0;
}

inline void Utils::printEvent(const blpapi::Event& event)
{
    blpapi::MessageIterator msgIter(event);
    while (msgIter.next()) {
        blpapi::Message msg = msgIter.message();
        msg.print(std::cout) << "\n";
    }
}

inline int Utils::getNextIntegerCid()
{
    static std::atomic<int> cid(10000);
    return ++cid;
}

#define BLPAPI_EVENT_TYPE_CASE(X)                                             \
    case blpapi::Event::X:                                                    \
        return os << #X;
inline std::ostream& operator<<(
        std::ostream& os, const blpapi::Event::EventType eventType)
{
    switch (eventType) {
        BLPAPI_EVENT_TYPE_CASE(ADMIN)
        BLPAPI_EVENT_TYPE_CASE(SESSION_STATUS)
        BLPAPI_EVENT_TYPE_CASE(SUBSCRIPTION_STATUS)
        BLPAPI_EVENT_TYPE_CASE(REQUEST_STATUS)
        BLPAPI_EVENT_TYPE_CASE(RESPONSE)
        BLPAPI_EVENT_TYPE_CASE(PARTIAL_RESPONSE)
        BLPAPI_EVENT_TYPE_CASE(SUBSCRIPTION_DATA)
        BLPAPI_EVENT_TYPE_CASE(SERVICE_STATUS)
        BLPAPI_EVENT_TYPE_CASE(TIMEOUT)
        BLPAPI_EVENT_TYPE_CASE(AUTHORIZATION_STATUS)
        BLPAPI_EVENT_TYPE_CASE(RESOLUTION_STATUS)
        BLPAPI_EVENT_TYPE_CASE(TOPIC_STATUS)
        BLPAPI_EVENT_TYPE_CASE(TOKEN_STATUS)
        BLPAPI_EVENT_TYPE_CASE(REQUEST)
    default:
        BLPAPI_EVENT_TYPE_CASE(UNKNOWN)
    }
}

}
#undef BLPAPI_EVENT_TYPE_CASE

#endif
