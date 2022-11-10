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

#ifndef SESSION_ROUTER_H
#define SESSION_ROUTER_H

#include <blpapi_event.h>
#include <blpapi_message.h>

#include <exception>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <set>

#include <util/Utils.h>

namespace BloombergLP {

enum class ExampleState { STARTING, STARTED, TERMINATED };

template <typename SessionType>
class SessionRouter : public SessionType::EventHandler
// 'SessionRouter' provides user means to register for events/messages they
// are interested in. It promotes async processing of event and messages.
{
  private:
    using Cid = blpapi::CorrelationId;
    using Event = blpapi::Event;
    using Message = blpapi::Message;
    using Name = blpapi::Name;

  public:
    using MessageHandler
            = std::function<void(SessionType *, const Event&, const Message&)>;
    // Represents 'MessageHandler' that a user can register for a particular
    // 'event'. The registered callback will be called as per following:
    // - if registered for an 'event', it will be called once per 'message'
    // - if registered for a 'MessageType', it will be called for the
    //   'MessageType' in 'event'
    // - if registered for a 'CorrelationId', it will be called
    //   for each message associated with registered 'CorrelationId'

    using EventHandler = std::function<void(SessionType *, const Event&)>;
    // Represents 'EventHandler' that a user can register for a particular
    // 'event'. The registered callback will be called once for the registered
    // 'EventType'.

    using ExceptionHandler = std::function<void(SessionType *session,
            const Event& event,
            const std::exception& exception)>;
    // Represents 'ExceptionHandler' that a user can register for handling
    // exceptions that may occur during processing events/messages.
    //
    // If no exception handler is registered, the exception is caught and
    // printed to 'std:cerr'.

  private:
    using MessageHandlersByCid = std::map<Cid, MessageHandler>;
    using MessageHandlersByEvent = std::map<Event::EventType, MessageHandler>;
    using MessageHandlersByName = std::map<Name, MessageHandler>;

    using EventHandlersByEvent = std::map<Event::EventType, EventHandler>;

    mutable std::mutex d_mutex;

  private:
    MessageHandlersByCid d_messageHandlersByCid;
    MessageHandlersByEvent d_messageHandlersByEventType;
    MessageHandlersByName d_messageHandlersByName;

    EventHandlersByEvent d_eventHandlersByEventType;

    ExceptionHandler d_exceptionHandler;

  public:
    SessionRouter();
    ~SessionRouter();

    void registerMessageHandler(
            Event::EventType eventType, const MessageHandler& messageHandler);
    // Registers 'messageHandler' for the given 'eventType'. The
    // 'messageHandler' is called once for each message in the registered
    // event.

    void registerMessageHandler(
            const Name& messageType, const MessageHandler& messageHandler);
    // Registers 'messageHandler' for the given 'messageType'. The
    // 'messageHandler' is called once for the registered 'messageType'.

    void registerMessageHandler(
            const Cid& cid, const MessageHandler& messageHandler);
    // Registers 'messageHandler' for the given 'cid'.

    void registerExceptionHandler(const ExceptionHandler& exceptionHandler);

    void registerEventHandler(
            Event::EventType eventType, const EventHandler& eventHandler);

    void deregisterMessageHandler(Event::EventType eventType);

    void deregisterMessageHandler(const Name& messageType);

    void deregisterMessageHandler(const Cid& cid);

    void deregisterExceptionHandler();

    void deregisterEventHandler(Event::EventType eventType);

    bool processEvent(const Event& event, SessionType *session) override;
};

template <typename SessionType>
inline SessionRouter<SessionType>::SessionRouter()
    : d_exceptionHandler(nullptr)
{
}

template <typename SessionType>
inline SessionRouter<SessionType>::~SessionRouter()
{
}

template <typename SessionType>
inline void SessionRouter<SessionType>::registerEventHandler(
        Event::EventType eventType, const EventHandler& eventHandler)
{
    std::lock_guard<std::mutex> lg(d_mutex);
    d_eventHandlersByEventType[eventType] = eventHandler;
}

template <typename SessionType>
inline void SessionRouter<SessionType>::registerMessageHandler(
        Event::EventType eventType, const MessageHandler& messageHandler)
{
    std::lock_guard<std::mutex> lg(d_mutex);
    d_messageHandlersByEventType[eventType] = messageHandler;
}

template <typename SessionType>
inline void SessionRouter<SessionType>::registerMessageHandler(
        const Name& messageType, const MessageHandler& messageHandler)
{
    std::lock_guard<std::mutex> lg(d_mutex);
    d_messageHandlersByName[messageType] = messageHandler;
}

template <typename SessionType>
inline void SessionRouter<SessionType>::registerMessageHandler(
        const Cid& cid, const MessageHandler& messageHandler)
{
    std::lock_guard<std::mutex> lg(d_mutex);
    d_messageHandlersByCid[cid] = messageHandler;
}

template <typename SessionType>
inline void SessionRouter<SessionType>::registerExceptionHandler(
        const ExceptionHandler& exceptionHandler)
{
    std::lock_guard<std::mutex> lg(d_mutex);
    d_exceptionHandler = exceptionHandler;
}

template <typename SessionType>
inline void SessionRouter<SessionType>::deregisterEventHandler(
        Event::EventType eventType)
{
    std::lock_guard<std::mutex> lg(d_mutex);
    d_eventHandlersByEventType.erase(eventType);
}

template <typename SessionType>
inline void SessionRouter<SessionType>::deregisterMessageHandler(
        Event::EventType eventType)
{
    std::lock_guard<std::mutex> lg(d_mutex);
    d_messageHandlersByEventType.erase(eventType);
}

template <typename SessionType>
inline void SessionRouter<SessionType>::deregisterMessageHandler(
        const Name& messageType)
{
    std::lock_guard<std::mutex> lg(d_mutex);
    d_messageHandlersByName.erase(messageType);
}

template <typename SessionType>
inline void SessionRouter<SessionType>::deregisterMessageHandler(
        const Cid& cid)
{
    std::lock_guard<std::mutex> lg(d_mutex);
    d_messageHandlersByCid.erase(cid);
}

template <typename SessionType>
inline void SessionRouter<SessionType>::deregisterExceptionHandler()
{
    std::lock_guard<std::mutex> lg(d_mutex);
    d_exceptionHandler = nullptr;
}

template <typename SessionType>
inline bool SessionRouter<SessionType>::processEvent(
        const Event& event, SessionType *session)
{
    try {
        Utils::printEvent(event);

        EventHandler eh;
        {
            std::lock_guard<std::mutex> lg(d_mutex);
            auto const it = d_eventHandlersByEventType.find(event.eventType());
            if (it != d_eventHandlersByEventType.end()) {
                eh = it->second;
            }
        }

        if (eh) {
            eh(session, event);
        }

        // Invoke registered messagehandlers.
        blpapi::MessageIterator it(event);
        while (it.next()) {
            Message msg = it.message();
            const int cidCount = msg.numCorrelationIds();

            for (unsigned i = 0; i < cidCount; ++i) {
                auto const& cid = msg.correlationId(i);
                MessageHandler mh;
                {
                    std::lock_guard<std::mutex> lg(d_mutex);
                    auto const it = d_messageHandlersByCid.find(cid);
                    if (it != d_messageHandlersByCid.end()) {
                        mh = it->second;
                    }
                }

                if (mh) {
                    mh(session, event, msg);
                }
            }

            {
                MessageHandler mh;
                {
                    std::lock_guard<std::mutex> lg(d_mutex);
                    auto const it = d_messageHandlersByEventType.find(
                            event.eventType());
                    if (it != d_messageHandlersByEventType.end()) {
                        mh = it->second;
                    }
                }

                if (mh) {
                    mh(session, event, msg);
                }
            }

            {
                MessageHandler mh;
                {
                    std::lock_guard<std::mutex> lg(d_mutex);
                    auto const it
                            = d_messageHandlersByName.find(msg.messageType());
                    if (it != d_messageHandlersByName.end()) {
                        mh = it->second;
                    }
                }

                if (mh) {
                    mh(session, event, msg);
                }
            }
        }
    } catch (const std::exception& exception) {
        ExceptionHandler eh;
        {
            std::lock_guard<std::mutex> lg(d_mutex);
            eh = d_exceptionHandler;
        }

        if (eh) {
            eh(session, event, exception);
        } else {
            std::cerr << "Exception in processing events\n"
                      << exception.what() << "\n";
        }

        return false;
    }

    return true;
}
} // end namespace BloombergLP
#endif
