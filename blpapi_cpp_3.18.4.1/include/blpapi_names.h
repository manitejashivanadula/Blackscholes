/* Copyright 2021. Bloomberg Finance L.P.
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
// blpapi_names.h                                                     -*-C++-*-
#ifndef INCLUDED_BLPAPI_NAMES
#define INCLUDED_BLPAPI_NAMES

//@PURPOSE: Provide static 'blpapi::Name' instances for common message types.
//
//@CLASSES:
// blpapi::Names: A struct that defines static 'blpapi::Name' instances.
//
//@DESCRIPTION: This component provides a new utility class called
// 'blpapi::Names' that defines static 'blpapi::Name' instances for common
// message types.  C++17 or later users can use the static 'blpapi::Name'
// constants defined in 'blpapi::Names', e.g., 'blpapi::Names::SessionStarted',
// other users must use the static functions instead, e.g.,
// 'blpapi::Names::sessionStarted()'.

#ifdef __cplusplus

#include <blpapi_name.h>

namespace BloombergLP {
namespace blpapi {
// ============
// struct Names
// ============

struct Names {
    // This struct defines static functions that return static Name instances
    // for common message types. For C++17 or later, static Name constants are
    // defined as well.

    static const Name& slowConsumerWarning();
    static const Name& slowConsumerWarningCleared();
    static const Name& dataLoss();
    static const Name& requestTemplateAvailable();
    static const Name& requestTemplatePending();
    static const Name& requestTemplateTerminated();
    static const Name& subscriptionTerminated();
    static const Name& subscriptionStarted();
    static const Name& subscriptionFailure();
    static const Name& subscriptionStreamsActivated();
    static const Name& subscriptionStreamsDeactivated();
    static const Name& requestFailure();
    static const Name& tokenGenerationSuccess();
    static const Name& tokenGenerationFailure();
    static const Name& sessionStarted();
    static const Name& sessionTerminated();
    static const Name& sessionStartupFailure();
    static const Name& sessionConnectionUp();
    static const Name& sessionConnectionDown();
    static const Name& sessionClusterInfo();
    static const Name& sessionClusterUpdate();
    static const Name& serviceOpened();
    static const Name& serviceOpenFailure();
    static const Name& serviceRegistered();
    static const Name& serviceRegisterFailure();
    static const Name& serviceDeregistered();
    static const Name& serviceUp();
    static const Name& serviceDown();
    static const Name& serviceAvailabilityInfo();
    static const Name& resolutionSuccess();
    static const Name& resolutionFailure();
    static const Name& topicSubscribed();
    static const Name& topicUnsubscribed();
    static const Name& topicRecap();
    static const Name& topicActivated();
    static const Name& topicDeactivated();
    static const Name& topicCreated();
    static const Name& topicCreateFailure();
    static const Name& topicDeleted();
    static const Name& topicResubscribed();
    static const Name& permissionRequest();
    static const Name& permissionResponse();
    static const Name& authorizationSuccess();
    static const Name& authorizationFailure();
    static const Name& authorizationRevoked();

#if __cplusplus >= 201703L
    /** @name The following static constants are available for C++17 or later.
     */
    /**@{*/
    static const Name SlowConsumerWarning;
    static const Name SlowConsumerWarningCleared;
    static const Name DataLoss;
    static const Name RequestTemplateAvailable;
    static const Name RequestTemplatePending;
    static const Name RequestTemplateTerminated;
    static const Name SubscriptionTerminated;
    static const Name SubscriptionStarted;
    static const Name SubscriptionFailure;
    static const Name SubscriptionStreamsActivated;
    static const Name SubscriptionStreamsDeactivated;
    static const Name RequestFailure;
    static const Name TokenGenerationSuccess;
    static const Name TokenGenerationFailure;
    static const Name SessionStarted;
    static const Name SessionTerminated;
    static const Name SessionStartupFailure;
    static const Name SessionConnectionUp;
    static const Name SessionConnectionDown;
    static const Name SessionClusterInfo;
    static const Name SessionClusterUpdate;
    static const Name ServiceOpened;
    static const Name ServiceOpenFailure;
    static const Name ServiceRegistered;
    static const Name ServiceRegisterFailure;
    static const Name ServiceDeregistered;
    static const Name ServiceUp;
    static const Name ServiceDown;
    static const Name ServiceAvailabilityInfo;
    static const Name ResolutionSuccess;
    static const Name ResolutionFailure;
    static const Name TopicSubscribed;
    static const Name TopicUnsubscribed;
    static const Name TopicRecap;
    static const Name TopicActivated;
    static const Name TopicDeactivated;
    static const Name TopicCreated;
    static const Name TopicCreateFailure;
    static const Name TopicDeleted;
    static const Name TopicResubscribed;
    static const Name PermissionRequest;
    static const Name PermissionResponse;
    static const Name AuthorizationSuccess;
    static const Name AuthorizationFailure;
    static const Name AuthorizationRevoked;
    /**@}*/
#endif
};

inline const Name& Names::slowConsumerWarning()
{
    static const Name name { "SlowConsumerWarning" };
    return name;
}

inline const Name& Names::slowConsumerWarningCleared()
{
    static const Name name { "SlowConsumerWarningCleared" };
    return name;
}

inline const Name& Names::dataLoss()
{
    static const Name name { "DataLoss" };
    return name;
}

inline const Name& Names::requestTemplateAvailable()
{
    static const Name name { "RequestTemplateAvailable" };
    return name;
}

inline const Name& Names::requestTemplatePending()
{
    static const Name name { "RequestTemplatePending" };
    return name;
}

inline const Name& Names::requestTemplateTerminated()
{
    static const Name name { "RequestTemplateTerminated" };
    return name;
}

inline const Name& Names::subscriptionTerminated()
{
    static const Name name { "SubscriptionTerminated" };
    return name;
}

inline const Name& Names::subscriptionStarted()
{
    static const Name name { "SubscriptionStarted" };
    return name;
}

inline const Name& Names::subscriptionFailure()
{
    static const Name name { "SubscriptionFailure" };
    return name;
}

inline const Name& Names::subscriptionStreamsActivated()
{
    static const Name name { "SubscriptionStreamsActivated" };
    return name;
}

inline const Name& Names::subscriptionStreamsDeactivated()
{
    static const Name name { "SubscriptionStreamsDeactivated" };
    return name;
}

inline const Name& Names::requestFailure()
{
    static const Name name { "RequestFailure" };
    return name;
}

inline const Name& Names::tokenGenerationSuccess()
{
    static const Name name { "TokenGenerationSuccess" };
    return name;
}

inline const Name& Names::tokenGenerationFailure()
{
    static const Name name { "TokenGenerationFailure" };
    return name;
}

inline const Name& Names::sessionStarted()
{
    static const Name name { "SessionStarted" };
    return name;
}

inline const Name& Names::sessionTerminated()
{
    static const Name name { "SessionTerminated" };
    return name;
}

inline const Name& Names::sessionStartupFailure()
{
    static const Name name { "SessionStartupFailure" };
    return name;
}

inline const Name& Names::sessionConnectionUp()
{
    static const Name name { "SessionConnectionUp" };
    return name;
}

inline const Name& Names::sessionConnectionDown()
{
    static const Name name { "SessionConnectionDown" };
    return name;
}

inline const Name& Names::sessionClusterInfo()
{
    static const Name name { "SessionClusterInfo" };
    return name;
}

inline const Name& Names::sessionClusterUpdate()
{
    static const Name name { "SessionClusterUpdate" };
    return name;
}

inline const Name& Names::serviceOpened()
{
    static const Name name { "ServiceOpened" };
    return name;
}

inline const Name& Names::serviceOpenFailure()
{
    static const Name name { "ServiceOpenFailure" };
    return name;
}

inline const Name& Names::serviceRegistered()
{
    static const Name name { "ServiceRegistered" };
    return name;
}

inline const Name& Names::serviceRegisterFailure()
{
    static const Name name { "ServiceRegisterFailure" };
    return name;
}

inline const Name& Names::serviceDeregistered()
{
    static const Name name { "ServiceDeregistered" };
    return name;
}

inline const Name& Names::serviceUp()
{
    static const Name name { "ServiceUp" };
    return name;
}

inline const Name& Names::serviceDown()
{
    static const Name name { "ServiceDown" };
    return name;
}

inline const Name& Names::serviceAvailabilityInfo()
{
    static const Name name { "ServiceAvailabilityInfo" };
    return name;
}

inline const Name& Names::resolutionSuccess()
{
    static const Name name { "ResolutionSuccess" };
    return name;
}

inline const Name& Names::resolutionFailure()
{
    static const Name name { "ResolutionFailure" };
    return name;
}

inline const Name& Names::topicSubscribed()
{
    static const Name name { "TopicSubscribed" };
    return name;
}

inline const Name& Names::topicUnsubscribed()
{
    static const Name name { "TopicUnsubscribed" };
    return name;
}

inline const Name& Names::topicRecap()
{
    static const Name name { "TopicRecap" };
    return name;
}

inline const Name& Names::topicActivated()
{
    static const Name name { "TopicActivated" };
    return name;
}

inline const Name& Names::topicDeactivated()
{
    static const Name name { "TopicDeactivated" };
    return name;
}

inline const Name& Names::topicCreated()
{
    static const Name name { "TopicCreated" };
    return name;
}

inline const Name& Names::topicCreateFailure()
{
    static const Name name { "TopicCreateFailure" };
    return name;
}

inline const Name& Names::topicDeleted()
{
    static const Name name { "TopicDeleted" };
    return name;
}

inline const Name& Names::topicResubscribed()
{
    static const Name name { "TopicResubscribed" };
    return name;
}

inline const Name& Names::permissionRequest()
{
    static const Name name { "PermissionRequest" };
    return name;
}

inline const Name& Names::permissionResponse()
{
    static const Name name { "PermissionResponse" };
    return name;
}

inline const Name& Names::authorizationSuccess()
{
    static const Name name { "AuthorizationSuccess" };
    return name;
}

inline const Name& Names::authorizationFailure()
{
    static const Name name { "AuthorizationFailure" };
    return name;
}

inline const Name& Names::authorizationRevoked()
{
    static const Name name { "AuthorizationRevoked" };
    return name;
}

#if __cplusplus >= 201703L
inline const Name Names::SlowConsumerWarning { slowConsumerWarning() };

inline const Name Names::SlowConsumerWarningCleared {
    slowConsumerWarningCleared()
};

inline const Name Names::DataLoss { dataLoss() };

inline const Name Names::RequestTemplateAvailable {
    requestTemplateAvailable()
};

inline const Name Names::RequestTemplatePending { requestTemplatePending() };

inline const Name Names::RequestTemplateTerminated {
    requestTemplateTerminated()
};

inline const Name Names::SubscriptionTerminated { subscriptionTerminated() };

inline const Name Names::SubscriptionStarted { subscriptionStarted() };

inline const Name Names::SubscriptionFailure { subscriptionFailure() };

inline const Name Names::SubscriptionStreamsActivated {
    subscriptionStreamsActivated()
};

inline const Name Names::SubscriptionStreamsDeactivated {
    subscriptionStreamsDeactivated()
};

inline const Name Names::RequestFailure { requestFailure() };

inline const Name Names::TokenGenerationSuccess { tokenGenerationSuccess() };

inline const Name Names::TokenGenerationFailure { tokenGenerationFailure() };

inline const Name Names::SessionStarted { sessionStarted() };

inline const Name Names::SessionTerminated { sessionTerminated() };

inline const Name Names::SessionStartupFailure { sessionStartupFailure() };

inline const Name Names::SessionConnectionUp { sessionConnectionUp() };

inline const Name Names::SessionConnectionDown { sessionConnectionDown() };

inline const Name Names::SessionClusterInfo { sessionClusterInfo() };

inline const Name Names::SessionClusterUpdate { sessionClusterUpdate() };

inline const Name Names::ServiceOpened { serviceOpened() };

inline const Name Names::ServiceOpenFailure { serviceOpenFailure() };

inline const Name Names::ServiceRegistered { serviceRegistered() };

inline const Name Names::ServiceRegisterFailure { serviceRegisterFailure() };

inline const Name Names::ServiceDeregistered { serviceDeregistered() };

inline const Name Names::ServiceUp { serviceUp() };

inline const Name Names::ServiceDown { serviceDown() };

inline const Name Names::ServiceAvailabilityInfo { serviceAvailabilityInfo() };

inline const Name Names::ResolutionSuccess { resolutionSuccess() };

inline const Name Names::ResolutionFailure { resolutionFailure() };

inline const Name Names::TopicSubscribed { topicSubscribed() };

inline const Name Names::TopicUnsubscribed { topicUnsubscribed() };

inline const Name Names::TopicRecap { topicRecap() };

inline const Name Names::TopicActivated { topicActivated() };

inline const Name Names::TopicDeactivated { topicDeactivated() };

inline const Name Names::TopicCreated { topicCreated() };

inline const Name Names::TopicCreateFailure { topicCreateFailure() };

inline const Name Names::TopicDeleted { topicDeleted() };

inline const Name Names::TopicResubscribed { topicResubscribed() };

inline const Name Names::PermissionRequest { permissionRequest() };

inline const Name Names::PermissionResponse { permissionResponse() };

inline const Name Names::AuthorizationSuccess { authorizationSuccess() };

inline const Name Names::AuthorizationFailure { authorizationFailure() };

inline const Name Names::AuthorizationRevoked { authorizationRevoked() };

#endif

} // close namespace blpapi
} // close namespace BloombergLP

#endif // ifdef __cplusplus

#endif // #ifndef INCLUDED_BLPAPI_NAMES
