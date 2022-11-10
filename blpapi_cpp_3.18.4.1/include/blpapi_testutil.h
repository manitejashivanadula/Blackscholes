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
// blpapi_testutil.h                                                  -*-C++-*-
#ifndef INCLUDED_BLPAPI_TESTUTIL
#define INCLUDED_BLPAPI_TESTUTIL

//@PURPOSE: Provides static utility for unit testing BLPAPI applications.
//@CLASSES:
// blpapi::test::TestUtil: Static utility class for unit testing
//
//@SEE_ALSO: blpapi_messageformatter
//
//@DESCRIPTION: This component defines a class, TestUtil, that provides helpers
// for unit-testing BLPAPI. This includes serializing / deserializing services,
// creating test events, and appending / formatting messages in these events.
//
/// Usage
///-----
//
/// Example: Create subscription data with correlation id:
///  - - - - - - - - - - - - - - - - - - - - - - - - - - -
// For the purpose of unit testing the behavior of our tick processing, we
// want to create a subscription data event containing a single tick for a
// given subscription, identified with a correlation id 'correlationId' that
// is available in the test.
//
// We first create the service from a string:
//
//..
// using BloombergLP::blpapi::Event;
// using BloombergLP::blpapi::Name;
//..
// using BloombergLP::blpapi::test::MessageFormatter;
// using BloombergLP::blpapi::test::MessageProperties;
// using BloombergLP::blpapi::test::TestUtil;
//..
// const char *schema = "<ServiceDefinition name=...>...</ServiceDefinition>";
//
// std::istringstream stream(schema);
// Service service = TestUtil::deserializeService(stream);
//..
//
// We then create a subscription event
//
//..
// Event event = TestUtil::createSubscriptionEvent(service);
//..
//
// We prepare the message properties:
//
//..
// MessageProperties properties;
// properties.setCorrelationId(correlationId);
//..
//
// Then, we obtain the definition for the message from the schema:
//
//..
// const Name msgName("MarketDataEvents");
// const SchemaElementDefinition def = service.getEventDefinition(msgName);
//..
//
// Now we append a message and obtain a formatter:
//
//..
// MessageFormatter fmt = TestUtil::appendMessage(event, def, properties);
//..
//
// The formatter can now be used to fill in the data we want to be using in
// the test.
// Note that service creation and the definitions for the ticks can be shared
// by all of the unit tests that need to create the same type of events.

#include <blpapi_correlationid.h>
#include <blpapi_datetime.h>
#include <blpapi_defs.h>
#include <blpapi_event.h>
#include <blpapi_messageformatter.h>
#include <blpapi_service.h>
#include <blpapi_streamproxy.h>
#include <blpapi_topic.h>
#include <blpapi_types.h>

#ifdef __cplusplus
extern "C" {
#endif

BLPAPI_EXPORT
int blpapi_TestUtil_createEvent(blpapi_Event_t **event, int eventType);

BLPAPI_EXPORT
int blpapi_TestUtil_deserializeService(
        const char *schema, size_t schemaLength, blpapi_Service_t **service);

BLPAPI_EXPORT
int blpapi_TestUtil_serializeService(blpapi_StreamWriter_t streamWriter,
        void *stream,
        const blpapi_Service_t *service);

BLPAPI_EXPORT
int blpapi_TestUtil_appendMessage(blpapi_MessageFormatter_t **formatter,
        blpapi_Event_t *event,
        const blpapi_SchemaElementDefinition_t *messageType,
        const blpapi_MessageProperties_t *properties);

BLPAPI_EXPORT
int blpapi_TestUtil_createTopic(
        blpapi_Topic_t **topic, const blpapi_Service_t *service, int isActive);

BLPAPI_EXPORT
int blpapi_TestUtil_getAdminMessageDefinition(
        blpapi_SchemaElementDefinition_t **definition,
        blpapi_Name_t *messageName);

BLPAPI_EXPORT
int blpapi_MessageProperties_create(
        blpapi_MessageProperties_t **messageProperties);

BLPAPI_EXPORT
void blpapi_MessageProperties_destroy(
        blpapi_MessageProperties_t *messageProperties);

BLPAPI_EXPORT
int blpapi_MessageProperties_copy(blpapi_MessageProperties_t **dest,
        const blpapi_MessageProperties_t *src);

BLPAPI_EXPORT
int blpapi_MessageProperties_assign(blpapi_MessageProperties_t *lhs,
        const blpapi_MessageProperties_t *rhs);

BLPAPI_EXPORT
int blpapi_MessageProperties_setCorrelationIds(
        blpapi_MessageProperties_t *messageProperties,
        const blpapi_CorrelationId_t *correlationIds,
        size_t numCorrelationIds);

BLPAPI_EXPORT
int blpapi_MessageProperties_setRecapType(
        blpapi_MessageProperties_t *messageProperties,
        int recap,
        int fragment);

BLPAPI_EXPORT
int blpapi_MessageProperties_setTimeReceived(
        blpapi_MessageProperties_t *messageProperties,
        const blpapi_HighPrecisionDatetime_t *timestamp);

BLPAPI_EXPORT
int blpapi_MessageProperties_setService(
        blpapi_MessageProperties_t *messageProperties,
        const blpapi_Service_t *service);

BLPAPI_EXPORT
int blpapi_MessageProperties_setRequestId(
        blpapi_MessageProperties_t *messageProperties, const char *requestId);

#ifdef __cplusplus
} // extern "C"

#include <cstdio>
#include <fstream>
#include <vector>

namespace BloombergLP {
namespace blpapi {
namespace test {

// =======================
// class MessageProperties
// =======================

class MessageProperties {
    // This class represents properties of a message that are not part of
    // the message contents, for example the correlation ids, or timestamp.

  private:
    blpapi_MessageProperties_t *d_handle_p;

  public:
    // CREATORS
    MessageProperties();
    // Creates a 'MessageProperties' with default values. Default
    // value for 'CorrelationId' property is an empty vector. Default
    // value for 'RecapType' property is 'Message::RecapType::e_none'.
    // Default value for 'FragmentType' property is
    // 'Message::FRAGMENT_NONE'. Value for 'Service' and timestamp is
    // "unset".

    MessageProperties(const MessageProperties& original);
    // Creates a 'MessageProperties' from 'original'.

    ~MessageProperties();
    // Destroys this 'MessageProperties'.

    // MANIPULATORS
    MessageProperties& operator=(const MessageProperties& rhs);
    // Make this 'MessageProperties' same as 'rhs'.

    MessageProperties& setCorrelationIds(
            const blpapi_CorrelationId_t *correlationIds,
            size_t correlationIdCount);
    // Sets the 'CorrelationId' property. 'correlationIdCount' provides
    // the number of 'correlationIds' to be added. The behavior is
    // undefined unless 'correlationIds' points to an array of at least
    // 'correlationIdCount' elements. The default value of
    // 'CorrelationId' property is an empty vector.

    MessageProperties& setCorrelationIds(
            const std::vector<CorrelationId>& correlationIds);
    // Sets the 'CorrelationId' property. 'correlationIds' provides
    // the 'CorrelationId' to be added.

    MessageProperties& setCorrelationId(const CorrelationId& correlationId);
    // Equivalent to 'setCorrelationIds(&correlationId, 1)'.

    MessageProperties& setRecapType(Message::RecapType::Type recapType,
            Message::Fragment fragmentType = Message::FRAGMENT_NONE);
    // Sets the 'RecapType' and 'FragmentType' properties.

    MessageProperties& setTimeReceived(const Datetime& timeReceived);
    // Sets the time received property. The default timestamp is
    // "unset", so attempting to retrieve the timestamp of a message
    // with default properties will throw an exception.

    MessageProperties& setService(const Service& service);
    // Sets the service property.

    MessageProperties& setRequestId(const char *requestId);
    // Sets the request id property. A copy of this string
    // is expected to be returned by 'Message::getRequestId()'.
    // If 'requestId' is empty or null, the method throws.

    // ACCESSORS
    blpapi_MessageProperties_t *handle() const;
    // Return the handle of the current 'MessageProperties'. For
    // *internal* use only.
};

// ==============
// class TestUtil
// ==============

class TestUtil {
    // This class provides a set of utility functions to allow SDK clients
    // to create events/messages for unit-testing their applications.

  public:
    // TYPES

    // *Deprecated*
    // Following typedef is provided for backwards compatibility. It will be
    // removed in a future release.
    typedef test::MessageProperties MessageProperties;

    // CLASS METHODS
    static Event createEvent(Event::EventType eventType);
    // Creates and returns an 'Event' to be used for testing with the
    // specified 'eventType'. The returned 'Event' cannot be used for
    // publishing. The behavior is undefined if 'EventFormatter' is
    // used with the returned 'Event'.

    static MessageFormatter appendMessage(Event& event,
            const SchemaElementDefinition& elementDef,
            const MessageProperties& properties = MessageProperties());
    // Creates a new message and appends it to the specified 'event'.
    // Returns a 'MessageFormatter' to format the last appended message.
    // The specified 'event' must be a test 'Event' created from
    // 'TestUtil::createEvent()'. 'elementDef' is used to verify and encode
    // the contents of the message and the specified 'properties' are used
    // to set the metadata properties for the message. An exception is
    // thrown if the method fails to append the message.

    static Service deserializeService(std::istream& stream);
    // Creates a 'Service' object from the specified 'stream'. The
    // format of the stream must be 'XML'. The stream should only contain
    // ASCII characters without any embedded 'null' characters. Returns
    // the 'Service' object on success or throws 'blpapi::Exception' on
    // failure.

    static void serializeService(std::ostream& stream, const Service& service);
    // Serialize the specific 'service' into the specified 'stream' in
    // 'XML' format. An exception is thrown if the service can't be
    // serialized successfully.

    static Topic createTopic(const Service& service, bool isActive = true);
    // Create a valid 'Topic' with the specified 'service' to support
    // testing publishers. The expected use case is to support returning a
    // custom 'Topic' while mocking `session.getTopic()` methods.

    static SchemaElementDefinition getAdminMessageDefinition(
            const Name& messageName);
    // Return the definition for an admin message of the specified
    // 'messageName'.
};

// ============================================================================
//                      INLINE FUNCTION DEFINITIONS
// ============================================================================

// --------------
// class TestUtil
// --------------

inline Event TestUtil::createEvent(Event::EventType eventType)
{
    blpapi_Event_t *event = 0;
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_TestUtil_createEvent)(&event, eventType));
    return event;
}

inline Service TestUtil::deserializeService(std::istream& stream)
{
    blpapi_Service_t *buffer = 0;

    // This is a workaround for SunOS compiler not supporting construction from
    // 'std::istreambuf_iterator'.
    std::string schema;
    std::getline(stream, schema, '\0');

    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_TestUtil_deserializeService)(
                    schema.c_str(), schema.length(), &buffer));
    return Service(buffer, Service::TestService());
}

inline void TestUtil::serializeService(
        std::ostream& stream, const Service& service)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(blpapi_TestUtil_serializeService)(
            StreamProxyOstream::writeToStream, &stream, service.handle()));
}

inline MessageFormatter TestUtil::appendMessage(Event& event,
        const SchemaElementDefinition& elementDef,
        const MessageProperties& properties)
{
    blpapi_MessageFormatter_t *formatter = 0;
    ExceptionUtil::throwOnError(BLPAPI_CALL(blpapi_TestUtil_appendMessage)(
            &formatter, event.impl(), elementDef.impl(), properties.handle()));

    return MessageFormatter(formatter);
}

inline Topic TestUtil::createTopic(const Service& service, bool isActive)
{
    blpapi_Topic_t *topic = 0;
    ExceptionUtil::throwOnError(BLPAPI_CALL(blpapi_TestUtil_createTopic)(
            &topic, service.handle(), isActive));
    return Topic(topic);
}

inline SchemaElementDefinition TestUtil::getAdminMessageDefinition(
        const Name& messageName)
{
    blpapi_SchemaElementDefinition_t *definition = 0;
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_TestUtil_getAdminMessageDefinition)(
                    &definition, messageName.impl()));
    return SchemaElementDefinition(definition);
}

inline MessageProperties::MessageProperties()
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageProperties_create)(&d_handle_p));
}

inline MessageProperties::MessageProperties(const MessageProperties& original)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(blpapi_MessageProperties_copy)(
            &d_handle_p, original.handle()));
}

inline MessageProperties& MessageProperties::operator=(
        const MessageProperties& rhs)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(blpapi_MessageProperties_assign)(
            d_handle_p, rhs.handle()));
    return *this;
}

inline MessageProperties::~MessageProperties()
{
    BLPAPI_CALL_UNCHECKED(blpapi_MessageProperties_destroy)(d_handle_p);
}

inline blpapi_MessageProperties_t *MessageProperties::handle() const
{
    return d_handle_p;
}

inline MessageProperties& MessageProperties::setCorrelationId(
        const CorrelationId& cid)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageProperties_setCorrelationIds)(
                    d_handle_p, &cid.impl(), 1));
    return *this;
}

inline MessageProperties& MessageProperties::setCorrelationIds(
        const std::vector<CorrelationId>& cids)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageProperties_setCorrelationIds)(
                    d_handle_p, &cids[0].impl(), cids.size()));
    return *this;
}

inline MessageProperties& MessageProperties::setCorrelationIds(
        const blpapi_CorrelationId_t *correlationIds,
        size_t correlationIdCount)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageProperties_setCorrelationIds)(
                    d_handle_p, correlationIds, correlationIdCount));
    return *this;
}

inline MessageProperties& MessageProperties::setRecapType(
        Message::RecapType::Type recapType, Message::Fragment fragmentType)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageProperties_setRecapType)(d_handle_p,
                    static_cast<int>(recapType),
                    static_cast<int>(fragmentType)));
    return *this;
}

inline MessageProperties& MessageProperties::setTimeReceived(
        const Datetime& timeReceived)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageProperties_setTimeReceived)(
                    d_handle_p, &timeReceived.rawHighPrecisionValue()));
    return *this;
}

inline MessageProperties& MessageProperties::setService(const Service& service)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageProperties_setService)(
                    d_handle_p, service.handle()));
    return *this;
}

inline MessageProperties& MessageProperties::setRequestId(
        const char *requestId)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(
            blpapi_MessageProperties_setRequestId)(d_handle_p, requestId));
    return *this;
}

} // close namespace test

// *Deprecated*
// Following typedef is provided for backwards compatibility.  It will be
// removed in a future release.
typedef test::TestUtil TestUtil;

} // close namespace blpapi
} // close namespace BloombergLP

#endif // #ifdef __cplusplus
#endif // #ifndef INCLUDED_BLPAPI_TESTUTIL
