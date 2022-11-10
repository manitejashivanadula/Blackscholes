/* Copyright 2012. Bloomberg Finance L.P.
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
// blpapi_session.h                                                   -*-C++-*-
#ifndef INCLUDED_BLPAPI_SESSION
#define INCLUDED_BLPAPI_SESSION

//@PURPOSE: Provide consumer session to get Bloomberg Service.
//
//@CLASSES:
// blpapi::Session: A consumer session for consuming service
//
//@DESCRIPTION: This component implements a consumer session for getting
// services.

#ifndef INCLUDED_BLPAPI_ABSTRACTSESSION
#include <blpapi_abstractsession.h>
#endif

#ifndef INCLUDED_BLPAPI_CALL
#include <blpapi_call.h>
#endif

#ifndef INCLUDED_BLPAPI_CORRELATIONID
#include <blpapi_correlationid.h>
#endif

#ifndef INCLUDED_BLPAPI_DEFS
#include <blpapi_defs.h>
#endif

#ifndef INCLUDED_BLPAPI_EVENT
#include <blpapi_event.h>
#endif

#ifndef INCLUDED_BLPAPI_EVENTDISPATCHER
#include <blpapi_eventdispatcher.h>
#endif

#ifndef INCLUDED_BLPAPI_IDENTITY
#include <blpapi_identity.h>
#endif

#ifndef INCLUDED_BLPAPI_REQUEST
#include <blpapi_request.h>
#endif

#ifndef INCLUDED_BLPAPI_REQUESTTEMPLATE
#include <blpapi_requesttemplate.h>
#endif

#ifndef INCLUDED_BLPAPI_SERVICE
#include <blpapi_service.h>
#endif

#ifndef INCLUDED_BLPAPI_SESSIONOPTIONS
#include <blpapi_sessionoptions.h>
#endif

#ifndef INCLUDED_BLPAPI_SUBSCRIPTIONLIST
#include <blpapi_subscriptionlist.h>
#endif

#ifndef INCLUDED_BLPAPI_TYPES
#include <blpapi_types.h>
#endif

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*blpapi_EventHandler_t)(
        blpapi_Event_t *event, blpapi_Session_t *session, void *userData);

typedef void (*blpapi_SubscriptionPreprocessErrorHandler_t)(
        const blpapi_CorrelationId_t *correlationId,
        const char *subscriptionString,
        int errorCode,
        const char *errorDescription,
        void *userData);

BLPAPI_EXPORT
blpapi_Session_t *blpapi_Session_create(blpapi_SessionOptions_t *parameters,
        blpapi_EventHandler_t handler,
        blpapi_EventDispatcher_t *dispatcher,
        void *userData);

BLPAPI_EXPORT
void blpapi_Session_destroy(blpapi_Session_t *session);

BLPAPI_EXPORT
int blpapi_Session_start(blpapi_Session_t *session);

BLPAPI_EXPORT
int blpapi_Session_startAsync(blpapi_Session_t *session);

BLPAPI_EXPORT
int blpapi_Session_stop(blpapi_Session_t *session);

BLPAPI_EXPORT
int blpapi_Session_stopAsync(blpapi_Session_t *session);

BLPAPI_EXPORT
int blpapi_Session_nextEvent(blpapi_Session_t *session,
        blpapi_Event_t **eventPointer,
        unsigned int timeoutInMilliseconds);

BLPAPI_EXPORT
int blpapi_Session_tryNextEvent(
        blpapi_Session_t *session, blpapi_Event_t **eventPointer);

BLPAPI_EXPORT
int blpapi_Session_subscribe(blpapi_Session_t *session,
        const blpapi_SubscriptionList_t *subscriptionList,
        const blpapi_Identity_t *handle,
        const char *requestLabel,
        int requestLabelLen);

BLPAPI_EXPORT
int blpapi_Session_subscribeEx(blpapi_Session_t *session,
        const blpapi_SubscriptionList_t *subscriptionList,
        const blpapi_Identity_t *handle,
        const char *requestLabel,
        int requestLabelLen,
        blpapi_SubscriptionPreprocessErrorHandler_t errorHandler,
        void *userData);

BLPAPI_EXPORT
int blpapi_Session_resubscribe(blpapi_Session_t *session,
        const blpapi_SubscriptionList_t *resubscriptionList,
        const char *requestLabel,
        int requestLabelLen);

BLPAPI_EXPORT
int blpapi_Session_resubscribeEx(blpapi_Session_t *session,
        const blpapi_SubscriptionList_t *resubscriptionList,
        const char *requestLabel,
        int requestLabelLen,
        blpapi_SubscriptionPreprocessErrorHandler_t errorHandler,
        void *userData);

BLPAPI_EXPORT
int blpapi_Session_resubscribeWithId(blpapi_Session_t *session,
        const blpapi_SubscriptionList_t *resubscriptionList,
        int resubscriptionId,
        const char *requestLabel,
        int requestLabelLen);

BLPAPI_EXPORT
int blpapi_Session_resubscribeWithIdEx(blpapi_Session_t *session,
        const blpapi_SubscriptionList_t *resubscriptionList,
        int resubscriptionId,
        const char *requestLabel,
        int requestLabelLen,
        blpapi_SubscriptionPreprocessErrorHandler_t errorHandler,
        void *userData);

BLPAPI_EXPORT
int blpapi_Session_unsubscribe(blpapi_Session_t *session,
        const blpapi_SubscriptionList_t *unsubscriptionList,
        const char *requestLabel,
        int requestLabelLen);

BLPAPI_EXPORT
int blpapi_Session_cancel(blpapi_Session_t *session,
        const blpapi_CorrelationId_t *correlationIds,
        size_t numCorrelationIds,
        const char *requestLabel,
        int requestLabelLen);

BLPAPI_EXPORT
int blpapi_Session_setStatusCorrelationId(blpapi_Session_t *session,
        const blpapi_Service_t *service,
        const blpapi_Identity_t *identity,
        const blpapi_CorrelationId_t *correlationId);

BLPAPI_EXPORT
int blpapi_Session_sendRequest(blpapi_Session_t *session,
        const blpapi_Request_t *request,
        blpapi_CorrelationId_t *correlationId,
        blpapi_Identity_t *identity,
        blpapi_EventQueue_t *eventQueue,
        const char *requestLabel,
        int requestLabelLen);

BLPAPI_EXPORT
int blpapi_Session_sendRequestTemplate(blpapi_Session_t *session,
        const blpapi_RequestTemplate_t *requestTemplate,
        blpapi_CorrelationId_t *correlationId);

BLPAPI_EXPORT
int blpapi_Session_createSnapshotRequestTemplate(
        blpapi_RequestTemplate_t **requestTemplate,
        blpapi_Session_t *session,
        const char *subscriptionString,
        const blpapi_Identity_t *identity,
        blpapi_CorrelationId_t *correlationId);

BLPAPI_EXPORT
int blpapi_Session_sendAuthorizationRequest(blpapi_Session_t *session,
        const blpapi_Request_t *request,
        blpapi_Identity_t *identity,
        blpapi_CorrelationId_t *correlationId,
        blpapi_EventQueue_t *eventQueue,
        const char *requestLabel,
        int requestLabelLen);

BLPAPI_EXPORT
int blpapi_Session_openService(
        blpapi_Session_t *session, const char *serviceName);

BLPAPI_EXPORT
int blpapi_Session_openServiceAsync(blpapi_Session_t *session,
        const char *serviceName,
        blpapi_CorrelationId_t *correlationId);

BLPAPI_EXPORT
int blpapi_Session_generateToken(blpapi_Session_t *session,
        blpapi_CorrelationId_t *correlationId,
        blpapi_EventQueue_t *eventQueue);

BLPAPI_EXPORT
int blpapi_Session_getService(blpapi_Session_t *session,
        blpapi_Service_t **service,
        const char *serviceName);

// Deprecated: Use blpapi_Session_createIdentity instead
BLPAPI_EXPORT
blpapi_UserHandle_t *blpapi_Session_createUserHandle(
        blpapi_Session_t *session);

BLPAPI_EXPORT
blpapi_Identity_t *blpapi_Session_createIdentity(blpapi_Session_t *session);

BLPAPI_EXPORT
blpapi_AbstractSession_t *blpapi_Session_getAbstractSession(
        blpapi_Session_t *session);

BLPAPI_EXPORT
blpapi_SubscriptionIterator_t *blpapi_SubscriptionItr_create(
        blpapi_Session_t *session);

BLPAPI_EXPORT
void blpapi_SubscriptionItr_destroy(blpapi_SubscriptionIterator_t *iterator);

BLPAPI_EXPORT
int blpapi_SubscriptionItr_next(blpapi_SubscriptionIterator_t *iterator,
        const char **subscriptionString,
        blpapi_CorrelationId_t *correlationId,
        int *status);

BLPAPI_EXPORT
int blpapi_SubscriptionItr_isValid(
        const blpapi_SubscriptionIterator_t *iterator);

#ifdef __cplusplus
}

#ifndef INCLUDED_CSTRING
#include <cstring> // for std::memset
#define INCLUDED_CSTRING
#endif

namespace BloombergLP {
namespace blpapi {

class Session;
// ==================
// class EventHandler
// ==================

class EventHandler {
    // Defines the callback supplied by the application to process
    // Events.
    //
    // Applications which use event driven programming supply
    // instances of their own subclass of this class to the Session to
    // receive callbacks when an Event is available.

  public:
    virtual ~EventHandler() = 0;

    virtual bool processEvent(const Event& event, Session *session) = 0;
    // An implementation of 'processEvent' should process the specified
    // 'event' which originates from the specified 'session'. The return
    // value is ignored.
    //
    // If the application wishes to further process the 'messages' in the
    // 'event' after returning from the 'processEvent()' call, the
    // application must make a copy of the 'event' to ensure the
    // underlying data is not freed.
};

extern "C" void blpapi_eventHandlerAdapter(
        blpapi_Event_t *event, blpapi_Session_t *, void *userData);
// Adapter 'blpapi_EventHandler_t' implementation that dispatches the
// specified 'event' to a 'blpapi::Session' pointed by 'userData'.

struct SubscriptionPreprocessError {
    enum Code {
        e_invalidSubscriptionString
        = BLPAPI_SUBSCRIPTIONPREPROCESS_INVALID_SUBSCRIPTION_STRING,
        e_correlationIdError
        = BLPAPI_SUBSCRIPTIONPREPROCESS_CORRELATIONID_ERROR
    };

    CorrelationId correlationId;
    std::string subscriptionString;
    int code;
    // The error code that is one of 'Code' enum values.
    std::string description;

    SubscriptionPreprocessError(const CorrelationId& correlationId,
            const std::string& subscriptionString,
            int code,
            const char *description)
        : correlationId(correlationId)
        , subscriptionString(subscriptionString)
        , code(code)
        , description(description)
    {
    }
};

inline std::ostream& operator<<(
        std::ostream& os, const SubscriptionPreprocessError& error)
{
    os << "{"
       << "correlationId: " << error.correlationId
       << ", subscriptionString: " << error.subscriptionString << ", code: ";
    if (error.code
            == SubscriptionPreprocessError::e_invalidSubscriptionString) {
        os << "InvalidSubscriptionString";
    } else if (error.code
            == SubscriptionPreprocessError::e_correlationIdError) {
        os << "CorrelationIdError";
    } else {
        os << "UNKNOWN";
    }

    os << "(" << error.code << ")";
    os << ", description: " << error.description << "}";

    return os;
}

extern "C" inline void blpapi_SubscriptionPreprocess_handleError(
        const blpapi_CorrelationId_t *correlationId,
        const char *subscriptionString,
        int errorCode,
        const char *description,
        void *userData)
{
    typedef std::vector<SubscriptionPreprocessError> Errors;
    Errors *errors = reinterpret_cast<Errors *>(userData);
    errors->emplace_back(
            *correlationId, subscriptionString, errorCode, description);
}

// =================================
// struct SubscriptionPreprocessMode
// =================================
struct SubscriptionPreprocessMode {
    enum Enum {
        // Enumeration used to defines how the invalid subscriptions are
        // managed in a 'SubscriptionList' provided to 'Session::subscribe()'
        // and 'Session::resubscribe()'.

        e_failOnFirstError,
        // In this mode, an exception is thrown on the first invalid
        // subscription in the 'SubscriptionList', and the entire batch
        // will be considered failed. Otherwise, all the subscriptions will
        // move forward and an empty list of 'SubscriptionPreprocessError'
        // is returned.

        e_returnIndividualErrors
        // In this mode, instead of throwing exceptions, a list of
        // 'SubscriptionPreprocessError' is returned, each representing
        // an error due to an invalid subscription in the
        // 'SubscriptionList'. The valid subscriptions will move forward.
    };
};

typedef std::vector<SubscriptionPreprocessError> SubscriptionPreprocessErrors;

// =============
// class Session
// =============

class Session : public AbstractSession {
    // This class provides a consumer session for making requests for Bloomberg
    // services.
    //
    // Sessions manage access to services either by requests and
    // responses or subscriptions. A Session can dispatch events and
    // replies in either a synchronous or asynchronous mode. The mode
    // of a Session is determined when it is constructed and cannot be
    // changed subsequently.
    //
    // A Session is asynchronous if an EventHandler object is
    // supplied when it is constructed. All incoming events are delivered to
    // the EventHandler supplied on construction.  Calls to 'nextEvent'
    // on an asynchronous session will fail.
    //
    // A Session is synchronous if an EventHandler object is not
    // supplied when it is constructed. The nextEvent() method must be
    // called to read incoming events.
    //
    // Several methods in Session take a CorrelationId parameter. The
    // application may choose to supply its own CorrelationId values
    // or allow the Session to create values. If the application
    // supplies its own CorrelationId values it must manage their
    // lifetime such that the same value is not reused for more than
    // one operation at a time. The lifetime of a CorrelationId begins
    // when it is supplied in a method invoked on a Session and ends
    // either when it is explicitly cancelled using cancel() or
    // unsubscribe(), when a RESPONSE Event (not a PARTIAL_RESPONSE)
    // containing it is received or when a SUBSCRIPTION_STATUS Event
    // which indicates that the subscription it refers to has been
    // terminated is received.
    //
    // When using an asynchronous Session the application must be
    // aware that because the callbacks are generated from another
    // thread they may be processed before the call which generates
    // them has returned. For example, the SESSION_STATUS Event
    // generated by a startAsync() may be processed before
    // startAsync() has returned (even though startAsync() itself will
    // not block).
    //
    // This becomes more significant when Session generated
    // CorrelationIds are in use. For example, if a call to
    // subscribe() which returns a Session generated CorrelationId has
    // not completed before the first Events which contain that
    // CorrelationId arrive the application may not be able to
    // interpret those events correctly. For this reason, it is
    // preferable to use user generated CorrelationIds when using
    // asynchronous Sessions. This issue does not arise when using a
    // synchronous Session as long as the calls to subscribe() etc. are
    // made on the same thread as the calls to nextEvent().

  public:
    typedef ::BloombergLP::blpapi::EventHandler EventHandler;

  private:
    blpapi_Session_t *d_handle_p;
    EventHandler *d_eventHandler_p;

    // NOT IMPLEMENTED
    Session(const Session&);
    Session& operator=(const Session&);

    friend void blpapi_eventHandlerAdapter(
            blpapi_Event_t *event, blpapi_Session_t *, void *userData);

  public:
    // The possible statuses a subscription may be in.
    enum SubscriptionStatus {
        UNSUBSCRIBED = BLPAPI_SUBSCRIPTIONSTATUS_UNSUBSCRIBED,
        // No longer active, terminated by API.
        SUBSCRIBING = BLPAPI_SUBSCRIPTIONSTATUS_SUBSCRIBING,
        // Initiated but no updates received.
        SUBSCRIBED = BLPAPI_SUBSCRIPTIONSTATUS_SUBSCRIBED,
        // Updates are flowing.
        CANCELLED = BLPAPI_SUBSCRIPTIONSTATUS_CANCELLED,
        // No longer active, terminated by Application.
        PENDING_CANCELLATION = BLPAPI_SUBSCRIPTIONSTATUS_PENDING_CANCELLATION
    };

    Session(const SessionOptions& options = SessionOptions(),
            EventHandler *eventHandler = 0,
            EventDispatcher *eventDispatcher = 0);
    // Construct a Session using the optionally specified
    // 'options', the optionally specified 'eventHandler' and the
    // optionally specified 'eventDispatcher'.
    //
    // See the SessionOptions documentation for details on what
    // can be specified in the 'options'.
    //
    // If 'eventHandler' is not null then this Session will operation
    // in asynchronous mode, otherwise the Session will operate in
    // synchronous mode.
    //
    // If 'eventDispatcher' is null then the Session will create a
    // default EventDispatcher for this Session which will use a
    // single thread for dispatching events. For more control over
    // event dispatching a specific instance of EventDispatcher
    // can be supplied. This can be used to share a single
    // EventDispatcher amongst multiple Session objects.
    //
    // If an 'eventDispatcher' is supplied which uses more than
    // one thread the Session will ensure that events which should
    // be ordered are passed to callbacks in a correct order. For
    // example, partial response to a request or updates to a
    // single subscription.
    //
    // The behavior is undefined if 'eventHandler' is null and the
    // 'eventDispatcher' is not null.
    //
    // Each EventDispatcher uses its own thread or pool of
    // threads so if you want to ensure that a session which
    // receives very large messages and takes a long time to
    // process them does not delay a session that receives small
    // messages and processes each one very quickly then give each
    // one a separate EventDispatcher.

    explicit Session(blpapi_Session_t *handle);

    virtual ~Session();
    // Destructor.

    // MANIPULATORS

    virtual bool start();
    // Attempt to start this Session and block until the Session
    // has started or failed to start. If the Session is started
    // successfully 'true' is returned, otherwise 'false' is
    // returned. Before start() returns a SESSION_STATUS Event is
    // generated. If this is an asynchronous Session then the
    // SESSION_STATUS may be processed by the registered
    // EventHandler before start() has returned. A Session may
    // only be started once.

    virtual bool startAsync();
    // Attempt to begin the process to start this Session and
    // return 'true' if successful, otherwise return 'false'. The
    // application must monitor events for a SESSION_STATUS Event
    // which will be generated once the Session has started or if
    // it fails to start. If this is an asynchronous Session then
    // the SESSION_STATUS Event may be processed by the registered
    // EventHandler before startAsync() has returned. A Session may
    // only be started once.

    virtual void stop();
    // Stop operation of this session and block until all callbacks to
    // EventHandler objects relating to this Session which are currently in
    // progress have completed (including the callback to handle the
    // SESSION_STATUS Event with SessionTerminated message this call
    // generates). Once this returns no further callbacks to EventHandlers
    // will occur. If stop() is called from within an EventHandler callback
    // the behavior is undefined and may result in a deadlock. Once a
    // Session has been stopped it can only be destroyed.

    virtual void stopAsync();
    // Begin the process to stop this Session and return immediately. The
    // application must monitor events for a SESSION_STATUS Event with
    // SessionTerminated message which will be generated once the
    // Session has been stopped. After this SESSION_STATUS Event no further
    // callbacks to EventHandlers will occur. This method can be called
    // from within an EventHandler callback to stop Sessions using
    // non-default (external) EventDispatcher. Once a Session has been
    // stopped it can only be destroyed.

    virtual Event nextEvent(int timeout = 0);
    // Returns the next available Event for this session. If there
    // is no event available this will block for up to the
    // specified 'timeoutMillis' milliseconds for an Event to
    // arrive. A value of 0 for 'timeoutMillis' (the default)
    // indicates nextEvent() should not timeout and will not
    // return until the next Event is available.
    //
    // If nextEvent() returns due to a timeout it will return an
    // event of type 'EventType::TIMEOUT'.
    //
    // If this is invoked on a Session which was created in
    // asynchronous mode an InvalidStateException is thrown.

    virtual int tryNextEvent(Event *event);
    // If there are Events available for the session, load the next Event
    // into event and return 0 indicating success. If there is no event
    // available for the session, return a non-zero value with no effect
    // on event. This method never blocks.

    virtual void subscribe(const SubscriptionList& subscriptionList,
            const Identity& identity,
            const char *requestLabel = 0,
            int requestLabelLen = 0);
    // Begin subscriptions for each entry in the specified
    // 'subscriptionList' using the specified 'identity' for
    // authorization. If the optional 'requestLabel' and
    // 'requestLabelLen' are provided they define a string which
    // will be recorded along with any diagnostics for this
    // operation. There must be at least 'requestLabelLen'
    // printable characters at the location 'requestLabel'.
    //
    // A SUBSCRIPTION_STATUS Event will be generated for each
    // entry in the 'subscriptionList'.

    virtual SubscriptionPreprocessErrors subscribe(
            const SubscriptionList& subscriptionList,
            const Identity& identity,
            SubscriptionPreprocessMode::Enum mode,
            const char *requestLabel = 0,
            int requestLabelLen = 0);
    // Begin subscriptions for each entry in the specified
    // 'subscriptionList' using the specified 'identity' for authorization.
    //
    // The specified 'mode' is used to determine how to preprocess
    // subscriptions. Refer to 'SubscriptionPreprocessMode' for an
    // explanation of the available modes.
    //
    // If the optional 'requestLabel' and 'requestLabelLen' are provided
    // they define a string which will be recorded along with any
    // diagnostics for this operation. There must be at least
    // 'requestLabelLen' printable characters at the location
    // 'requestLabel'.
    //
    // A SUBSCRIPTION_STATUS Event will be generated for each valid
    // subscriptions in the list.

    virtual void subscribe(const SubscriptionList& subscriptionList,
            const char *requestLabel = 0,
            int requestLabelLen = 0);
    // Begin subscriptions for each entry in the specified
    // 'subscriptionList' using the session identity
    // information. If the optional 'requestLabel' and
    // 'requestLabelLen' are provided they define a string which
    // will be recorded along with any diagnostics for this
    // operation. There must be at least 'requestLabelLen'
    // printable characters at the location 'requestLabel'.
    //
    // A SUBSCRIPTION_STATUS Event will be generated for each
    // entry in the 'subscriptionList'.
    //
    // The session identity will be used if it has been authorized.

    virtual SubscriptionPreprocessErrors subscribe(
            const SubscriptionList& subscriptionList,
            SubscriptionPreprocessMode::Enum mode,
            const char *requestLabel = 0,
            int requestLabelLen = 0);
    // Begin subscriptions for each entry in the specified
    // 'subscriptionList' using the session identity information.
    //
    // The specified 'mode' is used to determine how to preprocess
    // subscriptions. Refer to 'SubscriptionPreprocessMode' for an
    // explanation of the available modes.
    //
    // If the optional 'requestLabel' and 'requestLabelLen' are provided
    // they define a string which will be recorded along with any
    // diagnostics for this operation. There must be at least
    // 'requestLabelLen' printable characters at the location
    // 'requestLabel'.
    //
    // A SUBSCRIPTION_STATUS Event will be generated for each entry in the
    // 'subscriptionList'.
    //
    // The session identity will be used if it has been authorized.

    virtual void unsubscribe(const SubscriptionList& subscriptionList);
    // Cancel each of the current subscriptions identified by the
    // specified 'subscriptionList'. If the correlation ID of any
    // entry in the 'subscriptionList' does not identify a current
    // subscription then that entry is ignored. All entries which
    // have valid correlation IDs will be cancelled.
    //
    // Once this call returns the correlation ids in the
    // 'subscriptionList' will not be seen in any subsequent
    // Message obtained from a MessageIterator by calling
    // next(). However, any Message currently pointed to by a
    // MessageIterator when unsubscribe() is called is not
    // affected even if it has one of the correlation IDs in the
    // 'subscriptionList'. Also any Message where a reference has
    // been retained by the application may still contain a
    // correlation ID from the 'subscriptionList'. For these
    // reasons, although technically an application is free to
    // re-use the correlation IDs as soon as this method returns
    // it is preferable not to aggressively re-use correlation
    // IDs, particularly with an asynchronous Session.

    virtual void resubscribe(const SubscriptionList& subscriptions);
    // Modify each subscription in the specified
    // 'subscriptionList' to reflect the modified options
    // specified for it.
    //
    // For each entry in the 'subscriptionList' which has a
    // correlation ID which identifies a current subscription the
    // modified options replace the current options for the
    // subscription and a SUBSCRIPTION_STATUS event will be
    // generated in the event stream before the first update based
    // on the new options. If the correlation ID of an entry in
    // the 'subscriptionList' does not identify a current
    // subscription then an exception is thrown.

    virtual SubscriptionPreprocessErrors resubscribe(
            const SubscriptionList& subscriptions,
            SubscriptionPreprocessMode::Enum mode);
    // Modify each subscription in the specified 'subscriptionList' to
    // reflect the modified options specified for it.
    //
    // For each entry in the 'subscriptionList' which has a correlation ID
    // which identifies a current subscription the modified options replace
    // the current options for the subscription and a SUBSCRIPTION_STATUS
    // event will be generated in the event stream before the first update
    // based on the new options.
    //
    // The specified 'mode' is used to determine how to preprocess
    // subscriptions. Refer to 'SubscriptionPreprocessMode' for an
    // explanation of the available modes.

    virtual void resubscribe(const SubscriptionList& subscriptions,
            const char *requestLabel,
            int requestLabelLen);
    // Modify each subscription in the specified
    // 'subscriptionList' to reflect the modified options
    // specified for it. The specified 'requestLabel' and
    // 'requestLabelLen' define a string which
    // will be recorded along with any diagnostics for this
    // operation. There must be at least 'requestLabelLen'
    // printable characters at the location 'requestLabel'.
    //
    // For each entry in the 'subscriptionList' which has a
    // correlation ID which identifies a current subscription the
    // modified options replace the current options for the
    // subscription and a SUBSCRIPTION_STATUS event will be
    // generated in the event stream before the first update based
    // on the new options. If the correlation ID of an entry in
    // the 'subscriptionList' does not identify a current
    // subscription then an exception is thrown.

    virtual SubscriptionPreprocessErrors resubscribe(
            const SubscriptionList& subscriptions,
            const char *requestLabel,
            int requestLabelLen,
            SubscriptionPreprocessMode::Enum mode);
    // Modify each subscription in the specified
    // 'subscriptionList' to reflect the modified options
    // specified for it. The specified 'requestLabel' and
    // 'requestLabelLen' define a string which
    // will be recorded along with any diagnostics for this
    // operation. There must be at least 'requestLabelLen'
    // printable characters at the location 'requestLabel'.
    //
    // For each entry in the 'subscriptionList' which has a
    // correlation ID which identifies a current subscription the
    // modified options replace the current options for the
    // subscription and a SUBSCRIPTION_STATUS event will be
    // generated in the event stream before the first update based
    // on the new options.
    //
    // The specified 'mode' is used to determine how to preprocess
    // subscriptions. Refer to 'SubscriptionPreprocessMode' for an
    // explanation of the available modes.

    virtual void resubscribe(
            const SubscriptionList& subscriptions, int resubscriptionId);
    // Modify each subscription in the specified
    // 'subscriptionList' to reflect the modified options
    // specified for it.
    //
    // For each entry in the 'subscriptionList' which has a
    // correlation ID which identifies a current subscription the
    // modified options replace the current options for the
    // subscription and a SUBSCRIPTION_STATUS event containing the
    // specified 'resubscriptionId' will be generated in the event
    // stream before the first update based on the new options. If
    // the correlation ID of an entry in the 'subscriptionList'
    // does not identify a current subscription then an exception
    // is thrown.

    virtual SubscriptionPreprocessErrors resubscribe(
            const SubscriptionList& subscriptions,
            int resubscriptionId,
            SubscriptionPreprocessMode::Enum mode);
    // Modify each subscription in the specified 'subscriptionList' to
    // reflect the modified options specified for it.
    //
    // For each entry in the 'subscriptionList' which has a correlation ID
    // which identifies a current subscription the modified options replace
    // the current options for the subscription and a SUBSCRIPTION_STATUS
    // event containing the specified 'resubscriptionId' will be generated
    // in the event stream before the first update based on the new
    // options.
    //
    // The specified 'mode' is used to determine how to preprocess
    // subscriptions. Refer to 'SubscriptionPreprocessMode' for an
    // explanation of the available modes.

    virtual void resubscribe(const SubscriptionList& subscriptions,
            int resubscriptionId,
            const char *requestLabel,
            int requestLabelLen);
    // Modify each subscription in the specified
    // 'subscriptionList' to reflect the modified options
    // specified for it. The specified 'requestLabel' and
    // 'requestLabelLen' define a string which
    // will be recorded along with any diagnostics for this
    // operation. There must be at least 'requestLabelLen'
    // printable characters at the location 'requestLabel'.
    //
    // For each entry in the 'subscriptionList' which has a
    // correlation ID which identifies a current subscription the
    // modified options replace the current options for the
    // subscription and a SUBSCRIPTION_STATUS event containing the
    // specified 'resubscriptionId' will be generated in the event
    // stream before the first update based on the new options. If
    // the correlation ID of an entry in the 'subscriptionList'
    // does not identify a current subscription then an exception
    // is thrown.

    virtual SubscriptionPreprocessErrors resubscribe(
            const SubscriptionList& subscriptions,
            int resubscriptionId,
            const char *requestLabel,
            int requestLabelLen,
            SubscriptionPreprocessMode::Enum mode);
    // Modify each subscription in the specified
    // 'subscriptionList' to reflect the modified options
    // specified for it. The specified 'requestLabel' and
    // 'requestLabelLen' define a string which
    // will be recorded along with any diagnostics for this
    // operation. There must be at least 'requestLabelLen'
    // printable characters at the location 'requestLabel'.
    //
    // For each entry in the 'subscriptionList' which has a
    // correlation ID which identifies a current subscription the
    // modified options replace the current options for the
    // subscription and a SUBSCRIPTION_STATUS event containing the
    // specified 'resubscriptionId' will be generated in the event
    // stream before the first update based on the new options.
    //
    // The specified 'mode' is used to determine how to preprocess
    // subscriptions. Refer to 'SubscriptionPreprocessMode' for an
    // explanation of the available modes.

    virtual void setStatusCorrelationId(
            const Service& service, const CorrelationId& correlationID);

    virtual void setStatusCorrelationId(const Service& service,
            const Identity& identity,
            const CorrelationId& correlationID);
    // Set the CorrelationID on which service status messages will be
    // received.
    // Note: No service status messages are received prior to this call

    virtual CorrelationId sendRequest(const Request& request,
            const CorrelationId& correlationId = CorrelationId(),
            EventQueue *eventQueue = 0,
            const char *requestLabel = 0,
            int requestLabelLen = 0);
    // Send the specified 'request'. If the optionally specified
    // 'correlationId' is supplied use it otherwise create a CorrelationId.
    // The actual CorrelationId used is returned. If the optionally
    // specified 'eventQueue' is supplied all events relating to this
    // Request will arrive on that EventQueue. If the optional
    // 'requestLabel' and 'requestLabelLen' are provided they define a
    // string which will be recorded along with any diagnostics for this
    // operation. There must be at least 'requestLabelLen' printable
    // characters at the location 'requestLabel'.
    //
    // A successful request will generate zero or more PARTIAL_RESPONSE
    // Messages followed by exactly one RESPONSE Message. Once the final
    // RESPONSE Message has been received the correlation ID associated
    // with this request may be re-used. If the request fails at any stage
    // a REQUEST_STATUS will be generated after which the correlation ID
    // associated with the request may be re-used.
    //
    // The session identity will be used if it has been authorized.

    virtual CorrelationId sendRequest(const Request& request,
            const Identity& user,
            const CorrelationId& correlationId = CorrelationId(),
            EventQueue *eventQueue = 0,
            const char *requestLabel = 0,
            int requestLabelLen = 0);
    // Send the specified 'request' using the specified 'identity' for
    // authorization. If the optionally specified 'correlationId' is
    // supplied use it otherwise create a CorrelationId. The actual
    // CorrelationId used is returned. If the optionally specified
    // 'eventQueue' is supplied all events relating to this Request will
    // arrive on that EventQueue. If the optional 'requestLabel' and
    // 'requestLabelLen' are provided they define a string which will be
    // recorded along with any diagnostics for this operation. There must
    // be at least 'requestLabelLen' printable characters at the location
    // 'requestLabel'.
    //
    // A successful request will generate zero or more PARTIAL_RESPONSE
    // Messages followed by exactly one RESPONSE Message. Once the final
    // RESPONSE Message has been received the CorrelationId associated with
    // this request may be re-used. If the request fails at any stage a
    // REQUEST_STATUS will be generated after which the CorrelationId
    // associated with the request may be re-used.

    virtual CorrelationId sendRequest(const RequestTemplate& requestTemplate,
            const CorrelationId& correlationId = CorrelationId());
    // Send a request defined by the specified 'requestTemplate'. If the
    // optionally specified 'correlationId' is supplied, use it otherwise
    // create a new 'CorrelationId'. The actual 'CorrelationId' used is
    // returned.
    //
    // A successful request will generate zero or more 'PARTIAL_RESPONSE'
    // events followed by exactly one 'RESPONSE' event. Once the final
    // 'RESPONSE' event has been received the 'CorrelationId' associated
    // with  this request may be re-used. If the request fails at any stage
    // a 'REQUEST_STATUS' will be generated after which the 'CorrelationId'
    // associated with the request may be re-used.
    //
    // The session identity will be used if it has been authorized.

    virtual RequestTemplate createSnapshotRequestTemplate(
            const char *subscriptionString,
            const CorrelationId& statusCid,
            const Identity& identity = Identity());
    // Create a snapshot request template for getting subscription data
    // specified by the 'subscriptionString' using the optionally specified
    // 'identity' if all the following conditions are met: the session is
    // established, 'subscriptionString' is a valid subscription string and
    // 'statusCid' is not used in this session. If one or more conditions
    // are not met, an exception is thrown. The provided 'statusCid' will
    // be used for status updates about the created request template state
    // and an implied subscription associated with it delivered by
    // 'SUBSCRIPTION_STATUS' events.
    //
    // The benefit of the snapshot request templates is that these requests
    // may be serviced from a cache and the user may expect to see
    // significantly lower response time.
    //
    // There are 3 possible states for a created request template:
    // 'Pending', 'Available', and 'Terminated'. Right after creation a
    // request template is in the 'Pending' state.
    //
    // If a state is 'Pending', the user may send a request using this
    // template but there are no guarantees about response time since cache
    // is not available yet. Request template may transition into 'Pending'
    // state only from the 'Available' state. In this case the
    // 'RequestTemplatePending' message is generated.
    //
    // If state is 'Available', all requests will be serviced from a cache
    // and the user may expect to see significantly reduced latency. Note,
    // that a snapshot request template can transition out of the
    // 'Available' state concurrently with requests being sent, so no
    // guarantee of service from the cache can be provided. Request
    // template may transition into 'Available' state only from the
    // 'Pending' state. In this case the 'RequestTemplateAvailable' message
    // is generated. This message will also contain information about
    // currently used connection in the 'boundTo' field. Note that it is
    // possible to get the 'RequestTemplateAvailable' message with a new
    // connection information, even if a request template is already in the
    // 'Available' state.
    //
    // If state is 'Terminated', sending request will always result in a
    // failure response. Request template may transition into this state
    // from any other state. This is a final state and it is guaranteed
    // that the last message associated with the provided 'statusCid' will
    // be the 'RequestTemplateTerminated' message which is generated when a
    // request template transitions into this state. If a request template
    // transitions into this state, all outstanding requests will be failed
    // and appropriate messages will be generated for each request. After
    // receiving the 'RequestTemplateTerminated' message, 'statusCid' may
    // be reused.
    //
    // Note that resources used by a snapshot request template are released
    // only when request template transitions into the 'Terminated' state
    // or when session is destroyed. In order to release resources when
    // request template is not needed anymore, user should call the
    // 'Session::cancel(statusCid)' unless the 'RequestTemplateTerminated'
    // message was already received due to some problems. When the last
    // copy of a 'RequestTemplate' object goes out of scope and there are
    // no outstanding requests left, the snapshot request template will be
    // destroyed automatically. If the last copy of a 'RequestTemplate'
    // object goes out of scope while there are still some outstanding
    // requests left, snapshot service request template will be destroyed
    // automatically when the last request gets a final response.
    //
    // Note that it is possible to have a situation when response contains
    // data that were sent by a backend before user actually sends a
    // request template. For example it is possible if user requested data
    // were already in an incoming TCP queue.
    //
    // If 'identity' is not provided, the session identity will be used if
    // it has been authorized.

    // ACCESSORS

    blpapi_Session_t *handle() const;
};
// ==========================
// class SubscriptionIterator
// ==========================

class SubscriptionIterator {
    // An iterator which steps through all the subscriptions in a Session.
    //
    // The SubscriptionIterator can be used to iterate over all the
    // active subscriptions for a Session. However, with an
    // asynchronous Session it is possible for the set of active
    // subscriptions to change whilst the SubscriptionIterator is
    // being used. The SubscriptionIterator is guaranteed to never
    // return the same subscription twice. However, the subscription
    // the iterator points to may no longer be active. In this case
    // the result of subscriptionStatus() will be UNSUBSCRIBED or
    // CANCELLED.

    blpapi_SubscriptionIterator_t *d_handle_p;
    const char *d_subscriptionString;
    CorrelationId d_correlationId;
    int d_status;
    bool d_isValid;

  private:
    // NOT IMPLEMENTED
    SubscriptionIterator(const SubscriptionIterator&);
    SubscriptionIterator& operator=(const SubscriptionIterator&);

  public:
    SubscriptionIterator(Session *session);
    // Construct a forward iterator to iterate over the
    // subscriptions of the specified 'session'. The
    // SubscriptionIterator is created in a state where next()
    // must be called to advance it to the first item.

    ~SubscriptionIterator();
    // Destructor.

    // MANIPULATORS

    bool next();
    // Attempt to advance this iterator to the next subscription
    // record.  Returns 'true' on success and 'false' if there are
    // no more subscriptions. After next() returns true isValid()
    // is guaranteed to return true until the next call to
    // next(). After next() returns false isValid() will return
    // false.

    // ACCESSORS

    bool isValid() const;
    // Returns true if this iterator is currently positioned on a
    // valid subscription.  Returns false otherwise.

    const char *subscriptionString() const;
    // Returns a pointer to a null-terminated string which
    // contains the subscription string for this subscription. The
    // pointer returned remains valid until this
    // SubscriptionIterator is destroyed or the underlying
    // Session is destroyed or next() is called.

    const CorrelationId& correlationId() const;
    // Returns the CorrelationId for this subscription.

    Session::SubscriptionStatus subscriptionStatus() const;
    // Returns the status of this subscription.

    blpapi_SubscriptionIterator_t *handle() const;
};

// ============================================================================
//                      INLINE FUNCTION DEFINITIONS
// ============================================================================

// ------------------
// class EventHandler
// ------------------

inline EventHandler::~EventHandler() { }

// FREE FUNCTIONS
extern "C" inline void blpapi_eventHandlerAdapter(
        blpapi_Event_t *event, blpapi_Session_t *, void *userData)
{
    Session *session = reinterpret_cast<Session *>(userData);
    session->d_eventHandler_p->processEvent(Event(event), session);
}

// -------------
// class Session
// -------------

inline Session::Session(const SessionOptions& parameters,
        EventHandler *handler,
        EventDispatcher *dispatcher)
    : d_eventHandler_p(handler)
{
    if (dispatcher && !handler) {
        throw InvalidArgumentException(
                "Event dispatcher provided without handler.");
    }
    d_handle_p = blpapi_Session_create(parameters.handle(),
            handler ? &blpapi_eventHandlerAdapter : 0,
            dispatcher ? dispatcher->impl() : 0,
            this);
    initAbstractSessionHandle(blpapi_Session_getAbstractSession(d_handle_p));
}

inline Session::Session(blpapi_Session_t *newHandle)
    : d_handle_p(newHandle)
{
    initAbstractSessionHandle(blpapi_Session_getAbstractSession(d_handle_p));
}

inline Session::~Session() { blpapi_Session_destroy(d_handle_p); }

inline bool Session::start()
{
    return blpapi_Session_start(d_handle_p) ? false : true;
}

inline bool Session::startAsync()
{
    return blpapi_Session_startAsync(d_handle_p) ? false : true;
}

inline void Session::stop() { blpapi_Session_stop(d_handle_p); }

inline void Session::stopAsync() { blpapi_Session_stopAsync(d_handle_p); }

inline Event Session::nextEvent(int timeout)
{
    if (timeout < 0) {
        throw InvalidArgumentException("Timeout must be >= 0.");
    }

    blpapi_Event_t *event;
    ExceptionUtil::throwOnError(blpapi_Session_nextEvent(
            d_handle_p, &event, static_cast<unsigned>(timeout)));
    return Event(event);
}

inline int Session::tryNextEvent(Event *event)
{
    blpapi_Event_t *impl;
    int ret = blpapi_Session_tryNextEvent(d_handle_p, &impl);
    if (0 == ret) {
        *event = Event(impl);
    }
    return ret;
}

inline void Session::subscribe(const SubscriptionList& subscriptions,
        const char *requestLabel,
        int requestLabelLen)
{
    ExceptionUtil::throwOnError(blpapi_Session_subscribe(d_handle_p,
            subscriptions.impl(),
            0, // identityHandle
            requestLabel,
            requestLabelLen));
}

inline SubscriptionPreprocessErrors Session::subscribe(
        const SubscriptionList& subscriptions,
        SubscriptionPreprocessMode::Enum mode,
        const char *requestLabel,
        int requestLabelLen)
{
    SubscriptionPreprocessErrors errors;
    if (mode == SubscriptionPreprocessMode::e_failOnFirstError) {
        subscribe(subscriptions, requestLabel, requestLabelLen);
    } else {
        ExceptionUtil::throwOnError(
                BLPAPI_CALL(blpapi_Session_subscribeEx)(d_handle_p,
                        subscriptions.impl(),
                        0, // identityHandle
                        requestLabel,
                        requestLabelLen,
                        &blpapi_SubscriptionPreprocess_handleError,
                        &errors));
    }

    return errors;
}

inline void Session::subscribe(const SubscriptionList& subscriptions,
        const Identity& identity,
        const char *requestLabel,
        int requestLabelLen)
{
    ExceptionUtil::throwOnError(blpapi_Session_subscribe(d_handle_p,
            subscriptions.impl(),
            identity.handle(),
            requestLabel,
            requestLabelLen));
}

inline SubscriptionPreprocessErrors Session::subscribe(
        const SubscriptionList& subscriptions,
        const Identity& identity,
        SubscriptionPreprocessMode::Enum mode,
        const char *requestLabel,
        int requestLabelLen)
{
    SubscriptionPreprocessErrors errors;
    if (mode == SubscriptionPreprocessMode::e_failOnFirstError) {
        subscribe(subscriptions, identity, requestLabel, requestLabelLen);
    } else {
        ExceptionUtil::throwOnError(
                BLPAPI_CALL(blpapi_Session_subscribeEx)(d_handle_p,
                        subscriptions.impl(),
                        identity.handle(),
                        requestLabel,
                        requestLabelLen,
                        &blpapi_SubscriptionPreprocess_handleError,
                        &errors));
    }

    return errors;
}

inline void Session::resubscribe(const SubscriptionList& subscriptions)
{
    ExceptionUtil::throwOnError(blpapi_Session_resubscribe(d_handle_p,
            subscriptions.impl(),
            0, // requestLabel
            0)); // requestLabelLen
}

inline SubscriptionPreprocessErrors Session::resubscribe(
        const SubscriptionList& subscriptions,
        SubscriptionPreprocessMode::Enum mode)
{
    SubscriptionPreprocessErrors errors;
    if (mode == SubscriptionPreprocessMode::e_failOnFirstError) {
        resubscribe(subscriptions);
    } else {
        ExceptionUtil::throwOnError(
                BLPAPI_CALL(blpapi_Session_resubscribeEx)(d_handle_p,
                        subscriptions.impl(),
                        0, // requestLabel
                        0, // requestLabelLen
                        &blpapi_SubscriptionPreprocess_handleError,
                        &errors));
    }

    return errors;
}

inline void Session::resubscribe(const SubscriptionList& subscriptions,
        const char *requestLabel,
        int requestLabelLen)
{
    ExceptionUtil::throwOnError(blpapi_Session_resubscribe(
            d_handle_p, subscriptions.impl(), requestLabel, requestLabelLen));
}

inline SubscriptionPreprocessErrors Session::resubscribe(
        const SubscriptionList& subscriptions,
        const char *requestLabel,
        int requestLabelLen,
        SubscriptionPreprocessMode::Enum mode)
{
    SubscriptionPreprocessErrors errors;
    if (mode == SubscriptionPreprocessMode::e_failOnFirstError) {
        resubscribe(subscriptions, requestLabel, requestLabelLen);
    } else {
        ExceptionUtil::throwOnError(
                BLPAPI_CALL(blpapi_Session_resubscribeEx)(d_handle_p,
                        subscriptions.impl(),
                        requestLabel,
                        requestLabelLen,
                        &blpapi_SubscriptionPreprocess_handleError,
                        &errors));
    }

    return errors;
}

inline void Session::resubscribe(
        const SubscriptionList& subscriptions, int resubscriptionId)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL_SESSION_RESUBSCRIBEWITHID(d_handle_p,
                    subscriptions.impl(),
                    resubscriptionId,
                    0, // requestLabel
                    0)); // requestLabelLen
}

inline SubscriptionPreprocessErrors Session::resubscribe(
        const SubscriptionList& subscriptions,
        int resubscriptionId,
        SubscriptionPreprocessMode::Enum mode)
{
    SubscriptionPreprocessErrors errors;
    if (mode == SubscriptionPreprocessMode::e_failOnFirstError) {
        resubscribe(subscriptions, resubscriptionId);
    } else {
        ExceptionUtil::throwOnError(
                BLPAPI_CALL(blpapi_Session_resubscribeWithIdEx)(d_handle_p,
                        subscriptions.impl(),
                        resubscriptionId,
                        0, // requestLabel
                        0, // requestLabelLen
                        &blpapi_SubscriptionPreprocess_handleError,
                        &errors));
    }

    return errors;
}

inline void Session::resubscribe(const SubscriptionList& subscriptions,
        int resubscriptionId,
        const char *requestLabel,
        int requestLabelLen)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL_SESSION_RESUBSCRIBEWITHID(d_handle_p,
                    subscriptions.impl(),
                    resubscriptionId,
                    requestLabel,
                    requestLabelLen));
}

inline SubscriptionPreprocessErrors Session::resubscribe(
        const SubscriptionList& subscriptions,
        int resubscriptionId,
        const char *requestLabel,
        int requestLabelLen,
        SubscriptionPreprocessMode::Enum mode)
{
    SubscriptionPreprocessErrors errors;
    if (mode == SubscriptionPreprocessMode::e_failOnFirstError) {
        resubscribe(subscriptions,
                resubscriptionId,
                requestLabel,
                requestLabelLen);
    } else {
        ExceptionUtil::throwOnError(
                BLPAPI_CALL(blpapi_Session_resubscribeWithIdEx)(d_handle_p,
                        subscriptions.impl(),
                        resubscriptionId,
                        requestLabel,
                        requestLabelLen,
                        &blpapi_SubscriptionPreprocess_handleError,
                        &errors));
    }

    return errors;
}

inline void Session::unsubscribe(const SubscriptionList& list)
{
    ExceptionUtil::throwOnError(
            blpapi_Session_unsubscribe(d_handle_p, list.impl(), 0, 0));
}

inline void Session::setStatusCorrelationId(
        const Service& service, const CorrelationId& correlationID)
{
    ExceptionUtil::throwOnError(blpapi_Session_setStatusCorrelationId(
            d_handle_p, service.handle(), 0, &correlationID.impl()));
}

inline void Session::setStatusCorrelationId(const Service& service,
        const Identity& identity,
        const CorrelationId& correlationID)
{
    ExceptionUtil::throwOnError(
            blpapi_Session_setStatusCorrelationId(d_handle_p,
                    service.handle(),
                    identity.handle(),
                    &correlationID.impl()));
}

inline CorrelationId Session::sendRequest(const Request& request,
        const Identity& identity,
        const CorrelationId& correlationId,
        EventQueue *eventQueue,
        const char *requestLabel,
        int requestLabelLen)
{
    CorrelationId retCorrelationId(correlationId);

    ExceptionUtil::throwOnError(blpapi_Session_sendRequest(d_handle_p,
            request.handle(),
            &retCorrelationId.impl(),
            identity.handle(),
            eventQueue ? eventQueue->handle() : 0,
            requestLabel,
            requestLabelLen));

    return retCorrelationId;
}

inline CorrelationId Session::sendRequest(const Request& request,
        const CorrelationId& correlationId,
        EventQueue *eventQueue,
        const char *requestLabel,
        int requestLabelLen)
{
    CorrelationId retCorrelationId(correlationId);

    ExceptionUtil::throwOnError(blpapi_Session_sendRequest(d_handle_p,
            request.handle(),
            &retCorrelationId.impl(),
            0,
            eventQueue ? eventQueue->handle() : 0,
            requestLabel,
            requestLabelLen));

    return retCorrelationId;
}

inline CorrelationId Session::sendRequest(
        const RequestTemplate& requestTemplate,
        const CorrelationId& correlationId)
{
    CorrelationId retCorrelationId(correlationId);

    ExceptionUtil::throwOnError(BLPAPI_CALL(
            blpapi_Session_sendRequestTemplate)(
            d_handle_p, requestTemplate.impl(), &retCorrelationId.impl()));

    return retCorrelationId;
}

inline RequestTemplate Session::createSnapshotRequestTemplate(
        const char *subscriptionString,
        const CorrelationId& correlationID,
        const Identity& identity)
{
    blpapi_RequestTemplate_t *requestTemplate = 0;

    ExceptionUtil::throwOnError(BLPAPI_CALL(
            blpapi_Session_createSnapshotRequestTemplate)(&requestTemplate,
            d_handle_p,
            subscriptionString,
            identity.handle(),
            const_cast<blpapi_CorrelationId_t *>(&correlationID.impl())));

    return RequestTemplate(requestTemplate);
}

inline blpapi_Session_t *Session::handle() const { return d_handle_p; }

// --------------------------
// class SubscriptionIterator
// --------------------------

inline SubscriptionIterator::SubscriptionIterator(Session *session)
    : d_isValid(false)
{
    d_handle_p = blpapi_SubscriptionItr_create(session->handle());
}

inline SubscriptionIterator::~SubscriptionIterator()
{
    blpapi_SubscriptionItr_destroy(d_handle_p);
}

inline bool SubscriptionIterator::next()
{
    blpapi_CorrelationId_t nextCorrelationIdImpl;
    d_isValid = !blpapi_SubscriptionItr_next(d_handle_p,
            &d_subscriptionString,
            &nextCorrelationIdImpl,
            &d_status);

    d_correlationId = d_isValid ? CorrelationId(nextCorrelationIdImpl)
                                : CorrelationId();

    return d_isValid;
}

inline bool SubscriptionIterator::isValid() const { return d_isValid; }

inline blpapi_SubscriptionIterator_t *SubscriptionIterator::handle() const
{
    return d_handle_p;
}

inline const char *SubscriptionIterator::subscriptionString() const
{
    if (!isValid()) {
        ExceptionUtil::throwOnError(BLPAPI_ERROR_ILLEGAL_STATE);
    }

    return d_subscriptionString;
}

inline const CorrelationId& SubscriptionIterator::correlationId() const
{
    if (!isValid()) {
        ExceptionUtil::throwOnError(BLPAPI_ERROR_ILLEGAL_STATE);
    }

    return d_correlationId;
}

inline Session::SubscriptionStatus
SubscriptionIterator::subscriptionStatus() const
{
    if (!isValid()) {
        ExceptionUtil::throwOnError(BLPAPI_ERROR_ILLEGAL_STATE);
    }

    return static_cast<Session::SubscriptionStatus>(d_status);
}

} // close namespace blpapi
} // close namespace BloombergLP

#endif // ifdef __cplusplus

#endif // INCLUDED_BLPAPI_SESSION
