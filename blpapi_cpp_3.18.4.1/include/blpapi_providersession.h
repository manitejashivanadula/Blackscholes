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
// blpapi_providersession.h                                           -*-C++-*-
#ifndef INCLUDED_BLPAPI_PROVIDERSESSION
#define INCLUDED_BLPAPI_PROVIDERSESSION

//@PURPOSE: Provide a session that can be used for providing services
//
//@CLASSES:
// blpapi::ProviderSession: Session with providing(publishing) services.
// blpapi::ProviderEventHandler: Event handler for ProviderSession
// blpapi::ServiceRegistrationOptions: Container holding registration options.
//
//@DESCRIPTION: ProviderSession inherits from AbstractSession. In addition to
// AbstractSession functionality, ProviderSession provides functions that are
// needed to support publishing like 'registerService', 'createTopics' and
// 'publish'.
//
/// Topic Life Cycle
///----------------
// A provider wishing to publish subscription data must explicitly open each
// topic on which they publish using 'ProviderSession::createTopics' (or
// 'ProviderSession::createTopicsAsync'). Creating a topic prepares the
// distribution and caching infrastructure for new data associated with the
// topic's resolved identifier. (Note that several different topics could
// resolve to the same ID.) Independent of a topic's creation status is its
// subscription status, i.e. whether there are subscribers ready to receive
// the data published. A topic that is both created and subscribed is
// *activated*.
//
// There are two models for managing topic creation: broadcast and
// interactive. Broadcast publishers proactively call
// 'ProviderSession::createTopic*' for each topic on which they intend to
// publish, while interactive publishers wait to receive a
// 'TopicSubscribed' message (within an 'Event' of type 'Event::TOPIC_STATUS')
// before calling 'ProviderSession::createTopic*' in response. Topics are
// resolved before they are created---it is possible that multiple different
// topic strings will map to the same underlying topic. See below for the
// behavior of the SDK when the same topic is created multiple times.
//
// For applications using multiple broadcast publishers with the same priority
// for the same service in the same group, note that successfully calling
// 'createTopic*' in a publisher does not mean that the topic will be
// subscribed or activated as the infrastructure can select a different
// publisher for the subscription.  That is, there's no guarantee of
// 'TopicSubscribed' and 'TopicActivated' events following 'TopicCreated'.
// All broadcast publisher instances in the same publisher group should
// call 'createTopic*' for the same topics and be able to handle
// 'TopicSubscribed' before 'TopicCreated' is received.
//
// After 'ProviderSession::createTopic*' is called, the publisher will receive
// a 'TopicCreated' message (within an 'Event::TOPIC_STATUS' event), and when
// there is at least one subscriber to the topic the publisher will then
// receive a 'TopicActivated' message (also within an 'Event::TOPIC_STATUS'
// event). As subscribers come and go, additional 'TopicSubscribed',
// 'TopicActivated', 'TopicUnsubscribed', and 'TopicDeactivated' messages may
// be received by the publisher. A 'Topic' object can be retrieved from each of
// these messages using the 'ProviderSession::getTopic' method, and this
// object can be used for subsequent calls to 'EventFormatter::appendMessage'
// and 'ProviderSession::deleteTopic'. In the case that the same resolved
// topic is created multiple times by a publisher using different names, it is
// unspecified which of those names will be returned by 'Message::topicName'
// for these (or other) messages.
//
// If a publisher no longer intends to publish data on a topic, it can call
// 'ProviderSession::deleteTopic*' to free the internal caching and
// distribution resources associated with the topic. When a resolved topic has
// been deleted the same number of times that it has been created,
// a 'TopicDeleted' message will be delivered, preceded by 'TopicDeactivated'
// message. No further messages can be published on a deleted topic.  Existing
// subscriptions are not affected and the publisher can call
// 'ProviderSession::createTopic*' to recreate the topic and proceed
// publishing.
//
// If a topic is no longer supported, the publisher can call
// 'ProviderSession::terminateSubscriptionsOnTopics' to terminate all existing
// subscriptions and delete the topics.  Subscribers will be delivered a
// 'SubscriptionTerminated' message indicating that the subscription has been
// terminated.
//
// Deregistering a service forces unsubscribes and deletes all topics
// associated with that service.
//
// Note that 'TopicActivated' and 'TopicDeactivated' messages are entirely
// redundant with 'TopicCreated', 'TopicSubscribed', 'TopicDeleted', and
// 'TopicUnsubscribed' messages, and are provided only for the convenience
// of publishers that do not maintain per-topic state.

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

#ifndef INCLUDED_BLPAPI_RESOLUTIONLIST
#include <blpapi_resolutionlist.h>
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

#ifndef INCLUDED_BLPAPI_TOPIC
#include <blpapi_topic.h>
#endif

#ifndef INCLUDED_BLPAPI_TOPICLIST
#include <blpapi_topiclist.h>
#endif

#ifndef INCLUDED_BLPAPI_TYPES
#include <blpapi_types.h>
#endif

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct blpapi_ServiceRegistrationOptions;
typedef struct blpapi_ServiceRegistrationOptions
        blpapi_ServiceRegistrationOptions_t;

typedef void (*blpapi_ProviderEventHandler_t)(blpapi_Event_t *event,
        blpapi_ProviderSession_t *session,
        void *userData);

BLPAPI_EXPORT
blpapi_ProviderSession_t *blpapi_ProviderSession_create(
        blpapi_SessionOptions_t *parameters,
        blpapi_ProviderEventHandler_t handler,
        blpapi_EventDispatcher_t *dispatcher,
        void *userData);

BLPAPI_EXPORT
void blpapi_ProviderSession_destroy(blpapi_ProviderSession_t *session);

BLPAPI_EXPORT
int blpapi_ProviderSession_start(blpapi_ProviderSession_t *session);

BLPAPI_EXPORT
int blpapi_ProviderSession_startAsync(blpapi_ProviderSession_t *session);

BLPAPI_EXPORT
int blpapi_ProviderSession_stop(blpapi_ProviderSession_t *session);

BLPAPI_EXPORT
int blpapi_ProviderSession_stopAsync(blpapi_ProviderSession_t *session);

BLPAPI_EXPORT
int blpapi_ProviderSession_nextEvent(blpapi_ProviderSession_t *session,
        blpapi_Event_t **eventPointer,
        unsigned int timeoutInMilliseconds);

BLPAPI_EXPORT
int blpapi_ProviderSession_tryNextEvent(
        blpapi_ProviderSession_t *session, blpapi_Event_t **eventPointer);

BLPAPI_EXPORT
int blpapi_ProviderSession_registerService(blpapi_ProviderSession_t *session,
        const char *serviceName,
        const blpapi_Identity_t *identity,
        blpapi_ServiceRegistrationOptions_t *registrationOptions);

BLPAPI_EXPORT
int blpapi_ProviderSession_activateSubServiceCodeRange(
        blpapi_ProviderSession_t *session,
        const char *serviceName,
        int begin,
        int end,
        int priority);

BLPAPI_EXPORT
int blpapi_ProviderSession_deactivateSubServiceCodeRange(
        blpapi_ProviderSession_t *session,
        const char *serviceName,
        int begin,
        int end);

BLPAPI_EXPORT
int blpapi_ProviderSession_registerServiceAsync(
        blpapi_ProviderSession_t *session,
        const char *serviceName,
        const blpapi_Identity_t *identity,
        blpapi_CorrelationId_t *correlationId,
        blpapi_ServiceRegistrationOptions_t *registrationOptions);

BLPAPI_EXPORT
int blpapi_ProviderSession_deregisterService(
        blpapi_ProviderSession_t *session, const char *serviceName);

BLPAPI_EXPORT
int blpapi_ProviderSession_resolve(blpapi_ProviderSession_t *session,
        blpapi_ResolutionList_t *resolutionList,
        int resolveMode,
        const blpapi_Identity_t *identity);

BLPAPI_EXPORT
int blpapi_ProviderSession_resolveAsync(blpapi_ProviderSession_t *session,
        const blpapi_ResolutionList_t *resolutionList,
        int resolveMode,
        const blpapi_Identity_t *identity);

BLPAPI_EXPORT
int blpapi_ProviderSession_createTopics(blpapi_ProviderSession_t *session,
        blpapi_TopicList_t *topicList,
        int resolveMode,
        const blpapi_Identity_t *identity);

BLPAPI_EXPORT
int blpapi_ProviderSession_createTopicsAsync(blpapi_ProviderSession_t *session,
        const blpapi_TopicList_t *topicList,
        int resolveMode,
        const blpapi_Identity_t *identity);

BLPAPI_EXPORT
int blpapi_ProviderSession_getTopic(blpapi_ProviderSession_t *session,
        const blpapi_Message_t *message,
        blpapi_Topic_t **topic);

BLPAPI_EXPORT
int blpapi_ProviderSession_createTopic(blpapi_ProviderSession_t *session,
        const blpapi_Message_t *message,
        blpapi_Topic_t **topic);

BLPAPI_EXPORT
int blpapi_ProviderSession_createServiceStatusTopic(
        blpapi_ProviderSession_t *session,
        const blpapi_Service_t *service,
        blpapi_Topic_t **topic);

BLPAPI_EXPORT
int blpapi_ProviderSession_deleteTopics(blpapi_ProviderSession_t *session,
        const blpapi_Topic_t **topics,
        size_t numTopics);

BLPAPI_EXPORT
int blpapi_ProviderSession_terminateSubscriptionsOnTopics(
        blpapi_ProviderSession_t *session,
        const blpapi_Topic_t **topics,
        size_t numTopics,
        const char *message);

BLPAPI_EXPORT
int blpapi_ProviderSession_publish(
        blpapi_ProviderSession_t *session, blpapi_Event_t *event);

BLPAPI_EXPORT
int blpapi_ProviderSession_sendResponse(blpapi_ProviderSession_t *session,
        blpapi_Event_t *event,
        int isPartialResponse);

BLPAPI_EXPORT
blpapi_AbstractSession_t *blpapi_ProviderSession_getAbstractSession(
        blpapi_ProviderSession_t *session);

BLPAPI_EXPORT
int blpapi_ProviderSession_flushPublishedEvents(
        blpapi_ProviderSession_t *session, int *allFlushed, int timeoutMsecs);

// ServiceRegistrationOptions

BLPAPI_EXPORT
blpapi_ServiceRegistrationOptions_t *blpapi_ServiceRegistrationOptions_create(
        void);

BLPAPI_EXPORT
blpapi_ServiceRegistrationOptions_t *
blpapi_ServiceRegistrationOptions_duplicate(
        const blpapi_ServiceRegistrationOptions_t *parameters);

BLPAPI_EXPORT
void blpapi_ServiceRegistrationOptions_destroy(
        blpapi_ServiceRegistrationOptions_t *parameters);

BLPAPI_EXPORT
void blpapi_ServiceRegistrationOptions_copy(
        blpapi_ServiceRegistrationOptions_t *lhs,
        const blpapi_ServiceRegistrationOptions_t *rhs);

// SUBSERVICE CODES
BLPAPI_EXPORT
int blpapi_ServiceRegistrationOptions_addActiveSubServiceCodeRange(
        blpapi_ServiceRegistrationOptions_t *parameters,
        int start,
        int end,
        int priority);

BLPAPI_EXPORT
void blpapi_ServiceRegistrationOptions_removeAllActiveSubServiceCodeRanges(
        blpapi_ServiceRegistrationOptions_t *parameters);

BLPAPI_EXPORT
void blpapi_ServiceRegistrationOptions_setGroupId(
        blpapi_ServiceRegistrationOptions_t *parameters,
        const char *groupId,
        unsigned int groupIdLength);

BLPAPI_EXPORT
int blpapi_ServiceRegistrationOptions_setServicePriority(
        blpapi_ServiceRegistrationOptions_t *parameters, int priority);

BLPAPI_EXPORT
void blpapi_ServiceRegistrationOptions_setPartsToRegister(
        blpapi_ServiceRegistrationOptions_t *parameters, int parts);

BLPAPI_EXPORT
int blpapi_ServiceRegistrationOptions_getGroupId(
        blpapi_ServiceRegistrationOptions_t *parameters,
        char *groupdIdBuffer,
        int *groupIdLength);

BLPAPI_EXPORT
int blpapi_ServiceRegistrationOptions_getServicePriority(
        blpapi_ServiceRegistrationOptions_t *parameters);

BLPAPI_EXPORT
int blpapi_ServiceRegistrationOptions_getPartsToRegister(
        blpapi_ServiceRegistrationOptions_t *parameters);

#ifdef __cplusplus
}

namespace BloombergLP {
namespace blpapi {

extern "C" inline void blpapi_providerEventHandlerAdapter(
        blpapi_Event_t *event,
        blpapi_ProviderSession_t *session,
        void *userData);

class ProviderSession;
// ==========================
// class ProviderEventHandler
// ==========================

class ProviderEventHandler {
  public:
    virtual ~ProviderEventHandler() { }

    virtual bool processEvent(const Event& event, ProviderSession *session)
            = 0;
    // An implementation of 'processEvent' should process the specified
    // 'event' which originates from the specified 'session'. The return
    // value is ignored.
    //
    // If the application wishes to further process the 'messages' in the
    // 'event' after returning from the 'processEvent()' call, the
    // application must make a copy of the 'event' to ensure the
    // underlying data is not freed.
};

// ================================
// class ServiceRegistrationOptions
// ================================

class ServiceRegistrationOptions {
    // Contains the options which the user can specify when registering
    // a service.
    //
    // To use non-default options to registerService, create a
    // ServiceRegistrationOptions instance and set the desired options and
    // then supply it when using the registerService interface.

    blpapi_ServiceRegistrationOptions_t *d_handle_p;

  public:
    enum ServiceRegistrationPriority {
        PRIORITY_LOW = BLPAPI_SERVICEREGISTRATIONOPTIONS_PRIORITY_LOW,
        PRIORITY_MEDIUM = BLPAPI_SERVICEREGISTRATIONOPTIONS_PRIORITY_MEDIUM,
        PRIORITY_HIGH = BLPAPI_SERVICEREGISTRATIONOPTIONS_PRIORITY_HIGH
    };

    enum RegistrationParts {
        // constants for specifying which part(s) of a service should be
        // registered

        PART_PUBLISHING = BLPAPI_REGISTRATIONPARTS_PUBLISHING,
        // register to receive subscribe and unsubscribe messages

        PART_OPERATIONS = BLPAPI_REGISTRATIONPARTS_OPERATIONS,
        // register to receive the request types corresponding to each
        // "operation" defined in the service metadata

        PART_SUBSCRIBER_RESOLUTION
        = BLPAPI_REGISTRATIONPARTS_SUBSCRIBER_RESOLUTION,
        // register to receive resolution requests (with message type
        // 'PermissionRequest') from subscribers

        PART_PUBLISHER_RESOLUTION
        = BLPAPI_REGISTRATIONPARTS_PUBLISHER_RESOLUTION,
        // register to receive resolution requests (with message type
        // 'PermissionRequest') from publishers (via
        // 'ProviderSession::createTopics')

        PART_DEFAULT = BLPAPI_REGISTRATIONPARTS_DEFAULT
        // register the parts of the service implied by options
        // specified in the service metadata
    };

    ServiceRegistrationOptions();
    // Create ServiceRegistrationOptions with default options.

    ServiceRegistrationOptions(const ServiceRegistrationOptions& original);
    // Copy Constructor

    ~ServiceRegistrationOptions();
    // Destroy this ServiceRegistrationOptions.

    // MANIPULATORS

    ServiceRegistrationOptions& operator=(
            const ServiceRegistrationOptions& rhs);
    // Assign to this object the value of the specified 'rhs' object.

    void setGroupId(const char *groupId, unsigned int groupIdLength);
    // Set the Group ID for the service to be registered to the specified
    // char array beginning at groupId with size groupIdLength.
    // If groupIdLength > MAX_GROUP_ID_SIZE (=64) only the first
    // MAX_GROUP_ID_SIZE chars are considered as Group Id.

    int setServicePriority(int priority);
    // Set the priority with which a service will be registered
    // to the specified 'priority', where numerically greater values of
    // 'priority' indicate higher priorities. The behavior is undefined
    // unless 'priority' is non-negative. Note that while the values of
    // 'enum ServiceRegistrationPriority' are suitable for use here, any
    // non-negative 'priority' is acceptable.
    //
    // By default, a service will be registered with priority
    // 'ServiceRegistrationPriority::PRIORITY_HIGH'.
    //
    // Note that this has no effect for resolution services.

    void setPartsToRegister(int parts);
    // Set the parts of the service to be registered to the specified
    // 'parts', which must be a bitwise-or of the options provided in
    // 'RegistrationParts', above.  This option defaults to
    // 'RegistrationParts::PARTS_DEFAULT'.

    void addActiveSubServiceCodeRange(int begin, int end, int priority);
    // Advertise the service to be registered to receive, with the
    // specified 'priority', subscriptions that the resolver has mapped to
    // a service code between the specified 'begin' and the specified 'end'
    // values, inclusive. Numerically greater values of 'priority'
    // indicate higher priorities. The behavior of this function is
    // undefined unless '0 <= begin <= end < (1 << 24)', and 'priority' is
    // non-negative.

    void removeAllActiveSubServiceCodeRanges();
    // Remove all previously added sub-service code ranges.

    // ACCESSORS
    blpapi_ServiceRegistrationOptions_t *handle() const;

    int getGroupId(char *groupId, int *groupIdLength) const;
    // Copy the previously specified groupId at the memory location
    // specified by groupId and the set the size of the groupId returned at
    // groupIdLength. A Non-zero value indicates an error.
    // The behavior is undefined if there is not enough space to copy the
    // group Id to the specified buffer.
    // Note that the groupId is not null terminated and buffer for groupId
    // needs to be at most MAX_GROUP_ID_SIZE.

    int getServicePriority() const;
    // Return the priority with which a service will be registered.
    //
    // Note that the priority is used for both the publishing part and the
    // operations part. it is not supported for resolution part.

    int getPartsToRegister() const;
    // Return the parts of the service to be registered.  See
    // 'RegistrationParts', above for additional details.
};
// =====================
// class ProviderSession
// =====================

class ProviderSession : public AbstractSession {
    // This class provides a session that can be used for providing services.
    //
    // It inherits from AbstractSession. In addition to the AbstractSession
    // functionality a ProviderSession provides the following
    // functions to applications.
    //
    // A provider can register to provide services using
    // 'ProviderSession::registerService*'. Before registering to provide a
    // service the provider must have established its identity. Then
    // the provider can create topics and publish events on topics. It can also
    // get requests from the event queue and send responses back.
    //
    // After users have registered a service they will start receiving
    // subscription requests ('TopicSubscribed' message in 'TOPIC_STATUS') for
    // topics which belong to the service. If the resolver has specified
    // 'subServiceCode' for topics in 'PermissionResponse', then only providers
    // who have activated the 'subServiceCode' will get the subscription
    // request. Where multiple providers have registered the same service and
    // sub-service code (if any), the provider that registered the highest
    // priority for the sub-service code will receive subscription requests; if
    // multiple providers have registered the same sub-service code with the
    // same priority (or the resolver did not set a sub-service code for the
    // subscription), the subscription request will be routed to one of the
    // providers with the highest service priority.

  public:
    typedef ::BloombergLP::blpapi::ProviderEventHandler EventHandler;

  private:
    blpapi_ProviderSession_t *d_handle_p;
    EventHandler *d_eventHandler_p;

  private:
    friend void blpapi_providerEventHandlerAdapter(blpapi_Event_t *event,
            blpapi_ProviderSession_t *session,
            void *userData);

    void dispatchEvent(const Event& event);

  private:
    // NOT IMPLEMENTED
    ProviderSession(const ProviderSession&);
    ProviderSession& operator=(const ProviderSession&);

  public:
    enum ResolveMode {
        AUTO_REGISTER_SERVICES = BLPAPI_RESOLVEMODE_AUTO_REGISTER_SERVICES,
        // Try to register services found in ResolutionList if necessary.
        DONT_REGISTER_SERVICES = BLPAPI_RESOLVEMODE_DONT_REGISTER_SERVICES
        // Fail to resolve a topic if the service has not already been
        // registered.
    };

    ProviderSession(const SessionOptions& options = SessionOptions(),
            EventHandler *eventHandler = 0,
            EventDispatcher *eventDispatcher = 0);
    // Construct a Session using the optionally specified
    // 'options', the optionally specified 'eventHandler' and the
    // optionally specified 'eventDispatcher'.
    //
    // See the SessionOptions documentation for details on what
    // can be specified in the 'options'.
    //
    // If 'eventHandler' is not null then this Session will operate
    // in asynchronous mode; otherwise, the Session will operate in
    // synchronous mode.
    //
    // If 'eventDispatcher' is null and a not null 'eventHandler' is
    // provided, then the Session will create an EventDispatcher for this
    // Session which will use a single thread for dispatching events. For
    // more control over event dispatching a specific instance of
    // EventDispatcher can be supplied. This can be used to share a single
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

    explicit ProviderSession(blpapi_ProviderSession_t *handle);

    virtual ~ProviderSession();
    // Destructor.

    // MANIPULATORS
    virtual bool start();
    // See AbstractSession::start().

    virtual bool startAsync();
    // See AbstractSession::startAsync().

    virtual void stop();
    // See AbstractSession::stop().

    virtual void stopAsync();
    // See AbstractSession::stopAsync().

    virtual Event nextEvent(int timeout = 0);
    // See AbstractSession::nextEvent().

    virtual int tryNextEvent(Event *event);
    // See AbstractSession::tryNextEvent()

    virtual bool registerService(const char *serviceName,
            const Identity& providerIdentity = Identity(),
            const ServiceRegistrationOptions& registrationOptions
            = ServiceRegistrationOptions());
    // Attempt to register the service identified by the specified
    // 'serviceName' and block until the service is either registered
    // successfully or has failed to be registered. The optionally
    // specified 'providerIdentity' is used to verify permissions
    // to provide the service being registered. The optionally
    // specified 'registrationOptions' is used to specify the
    // group ID and service priority of the service being registered.
    // Returns 'true' if the service is registered successfully and
    // 'false' if the service cannot be registered successfully.
    //
    // The 'serviceName' must be a full qualified service name. That is it
    // must be of the form '//<namespace>/<local-name>'.
    //
    // This function does not return until a SERVICE_STATUS event has been
    // generated. Note that if the session was created in asynchronous
    // mode, the event may be processed before the function returns.
    //
    // When 'providerIdentity' is not provided, the session identity will
    // be used if it has been authorized.

    virtual void activateSubServiceCodeRange(
            const char *serviceName, int begin, int end, int priority);
    // Register to receive, with the specified 'priority', subscriptions
    // for the specified 'service' that the resolver has mapped to a
    // service code between the specified 'begin' and the specified 'end'
    // values, inclusive. Numerically greater values of 'priority' indicate
    // higher priorities. The behavior of this function is undefined
    // unless 'service' has already been successfully registered,
    // '0 <= begin <= end < (1 << 24)', and 'priority' is non-negative.

    virtual void deactivateSubServiceCodeRange(
            const char *serviceName, int begin, int end);
    // De-register to receive subscriptions for the specified 'service'
    // that the resolver has mapped to a service code between the specified
    // 'begin' and the specified 'end' values, inclusive. The behavior of
    // this function is undefined unless 'service' has already been
    // successfully registered and '0 <= begin <= end < (1 << 24)'.

    virtual CorrelationId registerServiceAsync(const char *serviceName,
            const Identity& providerIdentity = Identity(),
            const CorrelationId& correlationId = CorrelationId(),
            const ServiceRegistrationOptions& registrationOptions
            = ServiceRegistrationOptions());
    // Begin the process of registering the service identified by
    // the specified 'serviceName' and return immediately. The optionally
    // specified 'providerIdentity' is used to verify permissions
    // to provide the service being registered. The optionally
    // specified 'correlationId' is used to track 'Event' objects generated
    // as a result of this call. Return the actual 'CorrelationId' object
    // that will identify 'Event' objects. The optionally specified
    // 'registrationOptions' is used to specify the group ID and service
    // priority of the service being registered.
    //
    // The 'serviceName' must be a full qualified service name. That is it
    // must be of the form '//<namespace>/<local-name>'.
    //
    // The application must monitor events for a SERVICE_STATUS
    // Event which will be generated once the service has been
    // successfully registered or registration has failed.
    //
    // When 'providerIdentity' is not provided, the session identity will
    // be used if it has been authorized.

    virtual bool deregisterService(const char *serviceName);
    // Deregister the service, including all registered parts, identified
    // by the specified 'serviceName'. The identity in the service
    // registration is reused to verify permissions for deregistration. If
    // the service is not registered nor in pending registration, return
    // false; return true otherwise. If the service is in pending
    // registration, cancel the pending registration. If the service is
    // registered, send a deregistration request; generate TOPIC_STATUS
    // events containing a TopicUnsubscribed message for each subscribed
    // topic, a TopicDeactivated message for each active topic and a
    // TopicDeleted for each created topic; generate REQUEST_STATUS events
    // containing a RequestFailure message for each pending incoming
    // request; and generate a SERVICE_STATUS Event containing a
    // ServiceDeregistered message. All published events on topics created
    // on this service will be ignored after this method returns.

    virtual void resolve(ResolutionList *resolutionList,
            ResolveMode resolveMode = DONT_REGISTER_SERVICES,
            const Identity& providerIdentity = Identity());
    // Resolves the topics in the specified 'resolutionList' and
    // updates the 'resolutionList' with the results of the
    // resolution process. If the specified 'resolveMode' is
    // DONT_REGISTER_SERVICES (the default) then all the services
    // referenced in the topics in the 'resolutionList' must
    // already have been registered using registerService(). If
    // 'resolveMode' is AUTO_REGISTER_SERVICES then the specified
    // 'providerIdentity' should be supplied and ProviderSession
    // will automatically attempt to register any services
    // reference in the topics in the 'resolutionList' that have
    // not already been registered. Once resolveSync() returns
    // each entry in the 'resolutionList' will have been updated
    // with a new status.
    //
    // Before resolveSync() returns one or more RESOLUTION_STATUS
    // events and, if 'resolveMode' is AUTO_REGISTER_SERVICES,
    // zero or more SERVICE_STATUS events are generated. If this
    // is an asynchronous ProviderSession then these Events may be
    // processed by the registered EventHandler before resolve()
    // has returned.
    //
    // If 'providerIdentity' is not provided, the session identity will be
    // used if it has been authorized.

    virtual void resolveAsync(const ResolutionList& resolutionList,
            ResolveMode resolveMode = DONT_REGISTER_SERVICES,
            const Identity& providerIdentity = Identity());
    // Begin the resolution of the topics in the specified
    // 'resolutionList'. If the specified 'resolveMode' is
    // DONT_REGISTER_SERVICES (the default) then all the services
    // referenced in the topics in the 'resolutionList' must
    // already have been registered using registerService(). If
    // 'resolveMode' is AUTO_REGISTER_SERVICES then the specified
    // 'providerIdentity' should be supplied and ProviderSession
    // will automatically attempt to register any services
    // reference in the topics in the 'resolutionList' that have
    // not already been registered.
    //
    // One or more RESOLUTION_STATUS events will be delivered with
    // the results of the resolution. These events may be
    // generated before or after resolve() returns. If
    // AUTO_REGISTER_SERVICES is specified SERVICE_STATUS events
    // may also be generated before or after resolve() returns.
    //
    // If 'providerIdentity' is not provided, the session identity will be
    // used if it has been authorized.

    virtual Topic createTopic(const Message& message);
    // DEPRECATED
    // Create a topic from the specified 'message', which must be of type
    // 'ResolutionSuccess'. This method is deprecated; use 'createTopics'
    // or 'createTopicsAsync', which handle resolution automatically.

    virtual Topic getTopic(const Message& message);
    // Finds a previously created Topic object based on the specified
    // 'message'. The 'message' must be one of the following
    // types: TopicCreated, TopicActivated, TopicDeactivated,
    // TopicSubscribed, TopicUnsubscribed, TopicRecap.
    // If the 'message' is not valid then invoking isValid() on the
    // returned Topic will return false.

    virtual Topic createServiceStatusTopic(const Service& service);
    // Creates a Service Status Topic which is to be used to provide
    // service status. On success invoking isValid() on the returned Topic
    // will return false.

    virtual void publish(const Event& event);
    // Publish messages contained in the specified 'event'.

    virtual void sendResponse(
            const Event& event, bool isPartialResponse = false);
    // Send the response event for previously received request

    virtual void createTopics(TopicList *topicList,
            ResolveMode resolveMode = DONT_REGISTER_SERVICES,
            const Identity& providerIdentity = Identity());
    // Creates the topics in the specified 'topicList' and
    // updates the 'topicList' with the results of the
    // creation process. If service needs to be registered,
    // 'providerIdentity' should be supplied.
    // Once a call to this function returns,
    // each entry in the 'topicList' will have been updated
    // with a new topic creation status.
    //
    // Before createTopics() returns one or more RESOLUTION_STATUS
    // events, zero or more SERVICE_STATUS events and one or more
    // TOPIC_STATUS events are generated.
    // If this is an asynchronous ProviderSession then these
    // Events may be processed by the registered EventHandler
    // before createTopics() has returned.
    //
    // If 'providerIdentity' is not provided, the session identity will be
    // used if it has been authorized.

    virtual void createTopicsAsync(const TopicList& topicList,
            ResolveMode resolveMode = DONT_REGISTER_SERVICES,
            const Identity& providerIdentity = Identity());
    // Creates the topics in the specified 'topicList' and
    // updates the 'topicList' with the results of the
    // creation process. If service needs to be registered,
    // 'providerIdentity' should be supplied.
    //
    // One or more RESOLUTION_STATUS events, zero or more
    // SERVICE_STATUS events and one or more TOPIC_STATUS
    // events are generated.
    // If this is an asynchronous ProviderSession then these
    // Events may be processed by the registered EventHandler
    // before createTopics() has returned.
    //
    // If 'providerIdentity' is not provided, the session identity will be
    // used if it has been authorized.

    virtual void deleteTopic(const Topic& topic);
    // Remove one reference from the specified 'topic'.  If the number of
    // times this function, 'terminateSubscriptionsOnTopic',
    // 'updateResolvedTopics' or 'requireNewPermissionCheck' are called
    // is the same as the number of times 'topic' was created by
    // 'createTopics', then 'topic' is deleted: a 'TopicDeleted' message is
    // delivered, preceded by 'TopicDeactivated' message (See "Topic Life
    // Cycle", above, for additional details).  The behavior of this
    // function is undefined if 'topic' has already been deleted.  Further,
    // the behavior is undefined if a provider attempts to publish a
    // message on a deleted topic.

    virtual void deleteTopics(const std::vector<Topic>& topics);
    // Delete each topic in the specified 'topics'. See
    // 'deleteTopic(const Topic&)' for additional details.

    virtual void deleteTopics(const Topic *topics, size_t numTopics);
    // Delete the first 'numTopics' elements of the specified 'topics'
    // array. See 'deleteTopic(const Topic&)' for additional details.

    virtual void terminateSubscriptionsOnTopic(
            const Topic& topic, const char *message = 0);
    // Delete the specified 'topic' (See 'deleteTopic(const Topic&)' for
    // additional details).  Furthermore, proactively terminate all current
    // subscriptions on 'topic'.  The optionally specified 'message' can be
    // used to convey additional information to subscribers regarding the
    // termination.  This message is contained in the 'description' of
    // 'reason' in a 'SubscriptionTerminated' message.

    virtual void terminateSubscriptionsOnTopics(
            const std::vector<Topic>& topics, const char *message = 0);
    // Terminate subscriptions on each topic in the specified 'topics'.
    // See
    // 'terminateSubscriptionsOnTopic(const Topic&, const char* message)'
    // for additional details.

    virtual void terminateSubscriptionsOnTopics(
            const Topic *topics, size_t numTopics, const char *message = 0);
    // Terminate subscriptions on the first 'numTopics' topics in the
    // specified 'topics'.  See
    // 'terminateSubscriptionsOnTopic(const Topic&, const char* message)'
    // for additional details.

    blpapi_ProviderSession_t *handle() const;
    // Return the handle to this provider session

    virtual bool flushPublishedEvents(int timeoutMsecs);
    // Wait at most 'timeoutMsecs' milliseconds for all the published
    // events to be sent through the underlying channel. The method returns
    // either as soon as all the published events have been sent out or
    // when it has waited 'timeoutMs' milliseconds. Return true if
    // all the published events have been sent; false otherwise.
    // The behavior is undefined unless the specified 'timeoutMsecs' is
    // a non-negative value. When 'timeoutMsecs' is 0, the method checks
    // if all the published events have been sent and returns without
    // waiting.
};

//=============================================================================
//                           INLINE FUNCTION DEFINITIONS
//=============================================================================

// ---------------------
// class ProviderSession
// ---------------------

inline ProviderSession::ProviderSession(const SessionOptions& parameters,
        EventHandler *handler,
        EventDispatcher *dispatcher)
    : d_eventHandler_p(handler)
{
    if (dispatcher && !handler) {
        throw InvalidArgumentException(
                "Event dispatcher provided without handler.");
    }
    d_handle_p = blpapi_ProviderSession_create(parameters.handle(),
            handler ? &blpapi_providerEventHandlerAdapter : 0,
            dispatcher ? dispatcher->impl() : 0,
            this);
    initAbstractSessionHandle(
            blpapi_ProviderSession_getAbstractSession(d_handle_p));
}

inline ProviderSession::ProviderSession(blpapi_ProviderSession_t *newHandle)
    : d_handle_p(newHandle)
{
    initAbstractSessionHandle(
            blpapi_ProviderSession_getAbstractSession(d_handle_p));
}

inline ProviderSession::~ProviderSession()
{
    blpapi_ProviderSession_destroy(d_handle_p);
}

inline bool ProviderSession::start()
{
    return blpapi_ProviderSession_start(d_handle_p) ? false : true;
}

inline bool ProviderSession::startAsync()
{
    return blpapi_ProviderSession_startAsync(d_handle_p) ? false : true;
}

inline void ProviderSession::stop()
{
    blpapi_ProviderSession_stop(d_handle_p);
}

inline void ProviderSession::stopAsync()
{
    blpapi_ProviderSession_stopAsync(d_handle_p);
}

inline Event ProviderSession::nextEvent(int timeout)
{
    if (timeout < 0) {
        throw InvalidArgumentException("Timeout must be >= 0.");
    }

    blpapi_Event_t *event;
    ExceptionUtil::throwOnError(blpapi_ProviderSession_nextEvent(
            d_handle_p, &event, static_cast<unsigned>(timeout)));
    return Event(event);
}

inline int ProviderSession::tryNextEvent(Event *event)
{
    blpapi_Event_t *impl;
    int ret = blpapi_ProviderSession_tryNextEvent(d_handle_p, &impl);
    if (0 == ret) {
        *event = Event(impl);
    }
    return ret;
}

inline bool ProviderSession::registerService(const char *serviceName,
        const Identity& identity,
        const ServiceRegistrationOptions& registrationOptions)
{
    return blpapi_ProviderSession_registerService(d_handle_p,
                   serviceName,
                   identity.handle(),
                   registrationOptions.handle())
            ? false
            : true;
}
inline void ProviderSession::activateSubServiceCodeRange(
        const char *serviceName, int begin, int end, int priority)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL_PROVIDERSESSION_ACTIVATESUBSERVICECODERANGE(
                    d_handle_p, serviceName, begin, end, priority));
}

inline void ProviderSession::deactivateSubServiceCodeRange(
        const char *serviceName, int begin, int end)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL_PROVIDERSESSION_DEACTIVATESUBSERVICECODERANGE(
                    d_handle_p, serviceName, begin, end));
}

inline CorrelationId ProviderSession::registerServiceAsync(
        const char *serviceName,
        const Identity& identity,
        const CorrelationId& correlationId,
        const ServiceRegistrationOptions& registrationOptions)
{
    blpapi_CorrelationId_t retv = correlationId.impl();
    ExceptionUtil::throwOnError(
            blpapi_ProviderSession_registerServiceAsync(d_handle_p,
                    serviceName,
                    identity.handle(),
                    &retv,
                    registrationOptions.handle()));

    return retv;
}

inline bool ProviderSession::deregisterService(const char *serviceName)
{
    return BLPAPI_CALL_PROVIDERSESSION_DEREGISTERSERVICE(
                   d_handle_p, serviceName)
                    == 0
            ? true
            : false;
}

inline void ProviderSession::resolve(ResolutionList *resolutionList,
        ResolveMode resolveMode,
        const Identity& identity)
{
    ExceptionUtil::throwOnError(blpapi_ProviderSession_resolve(d_handle_p,
            resolutionList->impl(),
            resolveMode,
            identity.handle()));
    return;
}

inline void ProviderSession::resolveAsync(const ResolutionList& resolutionList,
        ResolveMode resolveMode,
        const Identity& identity)
{
    ExceptionUtil::throwOnError(blpapi_ProviderSession_resolveAsync(d_handle_p,
            resolutionList.impl(),
            resolveMode,
            identity.handle()));
    return;
}

inline Topic ProviderSession::createTopic(const Message& message)
{
    blpapi_Topic_t *topic;
    ExceptionUtil::throwOnError(blpapi_ProviderSession_createTopic(
            d_handle_p, message.impl(), &topic));

    return Topic(topic);
}

inline Topic ProviderSession::getTopic(const Message& message)
{
    blpapi_Topic_t *topic;
    ExceptionUtil::throwOnError(blpapi_ProviderSession_getTopic(
            d_handle_p, message.impl(), &topic));

    return Topic(topic);
}

inline Topic ProviderSession::createServiceStatusTopic(const Service& service)
{
    blpapi_Topic_t *topic;
    ExceptionUtil::throwOnError(
            blpapi_ProviderSession_createServiceStatusTopic(
                    d_handle_p, service.handle(), &topic));

    return Topic(topic);
}

inline void ProviderSession::createTopics(TopicList *topicList,
        ResolveMode resolveMode,
        const Identity& identity)
{
    ExceptionUtil::throwOnError(blpapi_ProviderSession_createTopics(
            d_handle_p, topicList->impl(), resolveMode, identity.handle()));
    return;
}

inline void ProviderSession::createTopicsAsync(const TopicList& topicList,
        ResolveMode resolveMode,
        const Identity& identity)
{
    ExceptionUtil::throwOnError(blpapi_ProviderSession_createTopicsAsync(
            d_handle_p, topicList.impl(), resolveMode, identity.handle()));
    return;
}

inline void ProviderSession::deleteTopic(const Topic& topic)
{
    const blpapi_Topic_t *topicImpl = topic.impl();
    ExceptionUtil::throwOnError(BLPAPI_CALL_PROVIDERSESSION_DELETETOPICS(
            d_handle_p, &topicImpl, 1));
}

inline void ProviderSession::deleteTopics(const std::vector<Topic>& topics)
{
    if (topics.size() == 0) {
        return;
    }
    std::vector<const blpapi_Topic_t *> topicImplList;
    topicImplList.reserve(topics.size());
    for (std::vector<Topic>::const_iterator it = topics.begin();
            it != topics.end();
            ++it) {
        topicImplList.push_back(it->impl());
    }
    ExceptionUtil::throwOnError(BLPAPI_CALL_PROVIDERSESSION_DELETETOPICS(
            d_handle_p, &topicImplList[0], topicImplList.size()));
}

inline void ProviderSession::deleteTopics(
        const Topic *topics, size_t numTopics)
{
    if (numTopics == 0) {
        return;
    }
    std::vector<const blpapi_Topic_t *> topicImplList;
    topicImplList.reserve(numTopics);
    for (size_t i = 0; i < numTopics; ++i) {
        topicImplList.push_back(topics[i].impl());
    }
    ExceptionUtil::throwOnError(BLPAPI_CALL_PROVIDERSESSION_DELETETOPICS(
            d_handle_p, &topicImplList[0], topicImplList.size()));
}

inline void ProviderSession::terminateSubscriptionsOnTopic(
        const Topic& topic, const char *message)
{
    const blpapi_Topic_t *topicImpl = topic.impl();
    ExceptionUtil::throwOnError(
            BLPAPI_CALL_PROVIDERSESSION_TERMINATESUBSCRIPTIONSONTOPICS(
                    d_handle_p, &topicImpl, 1, message));
}

inline void ProviderSession::terminateSubscriptionsOnTopics(
        const std::vector<Topic>& topics, const char *message)
{
    if (topics.empty()) {
        return;
    }
    terminateSubscriptionsOnTopics(&topics.front(), topics.size(), message);
}

inline void ProviderSession::terminateSubscriptionsOnTopics(
        const Topic *topics, size_t numTopics, const char *message)
{
    if (numTopics == 0) {
        return;
    }
    std::vector<const blpapi_Topic_t *> topicImplList;
    topicImplList.reserve(numTopics);
    for (size_t i = 0; i < numTopics; ++i) {
        topicImplList.push_back(topics[i].impl());
    }
    ExceptionUtil::throwOnError(
            BLPAPI_CALL_PROVIDERSESSION_TERMINATESUBSCRIPTIONSONTOPICS(
                    d_handle_p,
                    &topicImplList[0],
                    topicImplList.size(),
                    message));
}

inline void ProviderSession::publish(const Event& event)
{
    ExceptionUtil::throwOnError(
            blpapi_ProviderSession_publish(d_handle_p, event.impl()));
    return;
}

inline void ProviderSession::sendResponse(
        const Event& event, bool isPartialResponse)
{
    ExceptionUtil::throwOnError(blpapi_ProviderSession_sendResponse(
            d_handle_p, event.impl(), isPartialResponse));
    return;
}

inline blpapi_ProviderSession_t *ProviderSession::handle() const
{
    return d_handle_p;
}

inline void ProviderSession::dispatchEvent(const Event& event)
{
    d_eventHandler_p->processEvent(event, this);
}

inline bool ProviderSession::flushPublishedEvents(int timeoutMsecs)
{
    int allFlushed = 0;
    int rc = BLPAPI_CALL(blpapi_ProviderSession_flushPublishedEvents)(
            d_handle_p, &allFlushed, timeoutMsecs);
    if (rc != 0) {
        ExceptionUtil::throwOnError(rc);
    }
    return allFlushed ? true : false;
}

// --------------------------------
// class ServiceRegistrationOptions
// --------------------------------

inline ServiceRegistrationOptions::ServiceRegistrationOptions()
{
    d_handle_p = blpapi_ServiceRegistrationOptions_create();
}

inline ServiceRegistrationOptions::ServiceRegistrationOptions(
        const ServiceRegistrationOptions& original)
{
    d_handle_p
            = blpapi_ServiceRegistrationOptions_duplicate(original.handle());
}

inline ServiceRegistrationOptions::~ServiceRegistrationOptions()
{
    blpapi_ServiceRegistrationOptions_destroy(d_handle_p);
}

inline ServiceRegistrationOptions& ServiceRegistrationOptions::operator=(
        const ServiceRegistrationOptions& rhs)
{
    blpapi_ServiceRegistrationOptions_copy(this->handle(), rhs.handle());
    return *this;
}

// SUBSERVICE CODES
inline void ServiceRegistrationOptions::addActiveSubServiceCodeRange(
        int begin, int end, int priority)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL_SERVICEREGISTRATIONOPTIONS_ADDACTIVATESUBSERVICECODERANGE(
                    d_handle_p, begin, end, priority));
}

inline void ServiceRegistrationOptions::removeAllActiveSubServiceCodeRanges()
{
    BLPAPI_CALL_SERVICEREGISTRATIONOPTIONS_REMOVEALLACTIVESUBSERVICECODERANGES(
            d_handle_p);
}

inline void ServiceRegistrationOptions::setGroupId(
        const char *groupId, unsigned int groupIdLength)
{
    blpapi_ServiceRegistrationOptions_setGroupId(
            d_handle_p, groupId, groupIdLength);
}

inline int ServiceRegistrationOptions::setServicePriority(int priority)
{
    return blpapi_ServiceRegistrationOptions_setServicePriority(
            d_handle_p, priority);
}

inline void ServiceRegistrationOptions::setPartsToRegister(int parts)
{
    BLPAPI_CALL_SERVICEREGISTRATIONOPTIONS_SETPARTSTOREGISTER(
            d_handle_p, parts);
}

inline int ServiceRegistrationOptions::getGroupId(
        char *groupIdBuffer, int *groupIdLength) const
{
    return blpapi_ServiceRegistrationOptions_getGroupId(
            d_handle_p, groupIdBuffer, groupIdLength);
}

inline int ServiceRegistrationOptions::getServicePriority() const
{
    return blpapi_ServiceRegistrationOptions_getServicePriority(d_handle_p);
}

inline int ServiceRegistrationOptions::getPartsToRegister() const
{
    return BLPAPI_CALL_SERVICEREGISTRATIONOPTIONS_GETPARTSTOREGISTER(
            d_handle_p);
}

inline blpapi_ServiceRegistrationOptions_t *
ServiceRegistrationOptions::handle() const
{
    return d_handle_p;
}

// --------------------------
// class ProviderEventHandler
// --------------------------

extern "C" inline void blpapi_providerEventHandlerAdapter(
        blpapi_Event_t *event, blpapi_ProviderSession_t *, void *userData)
{
    reinterpret_cast<ProviderSession *>(userData)->dispatchEvent(Event(event));
}

} // close namespace blpapi
} // close namespace BloombergLP

#endif // #ifdef __cplusplus
#endif // #ifndef INCLUDED_BLPAPI_PROVIDERSESSION
