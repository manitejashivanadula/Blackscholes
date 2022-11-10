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
// blpapi_messageformatter.h                                          -*-C++-*-
#ifndef INCLUDED_BLPAPI_MESSAGEFORMATTER
#define INCLUDED_BLPAPI_MESSAGEFORMATTER

//@PURPOSE: Format messages for publishing
//
//@CLASSES:
// blpapi::test::MessageFormatter: A Mechanism to format messages.
//
//@DESCRIPTION: This component formats messages.
//
/// Limitations
///-----
// Currently, the JSON and XML formatting methods have some known limitations.
//
// The parsers can not differentiate complex field types
// (sequences, choices, arrays) that are empty with complex field types that
// are missing / null. These fields are dropped and absent in the message
// contents.
//
// Enumerations of type "Datetime", or any "Datetime" element with timezone
// offset or sub-microsecond precision (e.g. nanoseconds) are not supported.

#ifndef INCLUDED_BLPAPI_CALL
#include <blpapi_call.h>
#endif

#ifndef INCLUDED_BLPAPI_DEFS
#include <blpapi_defs.h>
#endif

#ifndef INCLUDED_BLPAPI_EVENT
#include <blpapi_event.h>
#endif

#ifndef INCLUDED_BLPAPI_TOPIC
#include <blpapi_topic.h>
#endif

#ifndef INCLUDED_BLPAPI_TYPES
#include <blpapi_types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

BLPAPI_EXPORT
int blpapi_MessageFormatter_setValueBool(blpapi_MessageFormatter_t *formatter,
        const blpapi_Name_t *typeName,
        blpapi_Bool_t value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_setValueChar(blpapi_MessageFormatter_t *formatter,
        const blpapi_Name_t *typeName,
        char value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_setValueInt32(blpapi_MessageFormatter_t *formatter,
        const blpapi_Name_t *typeName,
        blpapi_Int32_t value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_setValueInt64(blpapi_MessageFormatter_t *formatter,
        const blpapi_Name_t *typeName,
        blpapi_Int64_t value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_setValueFloat32(
        blpapi_MessageFormatter_t *formatter,
        const blpapi_Name_t *typeName,
        blpapi_Float32_t value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_setValueFloat64(
        blpapi_MessageFormatter_t *formatter,
        const blpapi_Name_t *typeName,
        blpapi_Float64_t value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_setValueDatetime(
        blpapi_MessageFormatter_t *formatter,
        const blpapi_Name_t *typeName,
        const blpapi_Datetime_t *value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_setValueHighPrecisionDatetime(
        blpapi_MessageFormatter_t *formatter,
        const blpapi_Name_t *typeName,
        const blpapi_HighPrecisionDatetime_t *value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_setValueString(
        blpapi_MessageFormatter_t *formatter,
        const blpapi_Name_t *typeName,
        const char *value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_setValueFromName(
        blpapi_MessageFormatter_t *formatter,
        const blpapi_Name_t *typeName,
        const blpapi_Name_t *value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_setValueNull(
        blpapi_MessageFormatter_t *formatter, const blpapi_Name_t *typeName);

BLPAPI_EXPORT
int blpapi_MessageFormatter_pushElement(
        blpapi_MessageFormatter_t *formatter, const blpapi_Name_t *typeName);

BLPAPI_EXPORT
int blpapi_MessageFormatter_popElement(blpapi_MessageFormatter_t *formatter);

BLPAPI_EXPORT
int blpapi_MessageFormatter_appendValueBool(
        blpapi_MessageFormatter_t *formatter, blpapi_Bool_t value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_appendValueChar(
        blpapi_MessageFormatter_t *formatter, char value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_appendValueInt32(
        blpapi_MessageFormatter_t *formatter, blpapi_Int32_t value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_appendValueInt64(
        blpapi_MessageFormatter_t *formatter, blpapi_Int64_t value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_appendValueFloat32(
        blpapi_MessageFormatter_t *formatter, blpapi_Float32_t value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_appendValueFloat64(
        blpapi_MessageFormatter_t *formatter, blpapi_Float64_t value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_appendValueDatetime(
        blpapi_MessageFormatter_t *formatter, const blpapi_Datetime_t *value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_appendValueHighPrecisionDatetime(
        blpapi_MessageFormatter_t *formatter,
        const blpapi_HighPrecisionDatetime_t *value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_appendValueString(
        blpapi_MessageFormatter_t *formatter, const char *value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_appendValueFromName(
        blpapi_MessageFormatter_t *formatter, const blpapi_Name_t *value);

BLPAPI_EXPORT
int blpapi_MessageFormatter_appendElement(
        blpapi_MessageFormatter_t *formatter);

BLPAPI_EXPORT
int blpapi_MessageFormatter_FormatMessageJson(
        blpapi_MessageFormatter_t *formatter, const char *message);

BLPAPI_EXPORT
int blpapi_MessageFormatter_FormatMessageXml(
        blpapi_MessageFormatter_t *formatter, const char *message);

BLPAPI_EXPORT
int blpapi_MessageFormatter_copy(blpapi_MessageFormatter_t **formatter,
        const blpapi_MessageFormatter_t *original);

BLPAPI_EXPORT
int blpapi_MessageFormatter_assign(
        blpapi_MessageFormatter_t **lhs, const blpapi_MessageFormatter_t *rhs);

BLPAPI_EXPORT
int blpapi_MessageFormatter_destroy(blpapi_MessageFormatter_t *formatter);

#ifdef __cplusplus
} // extern "C"

namespace BloombergLP {
namespace blpapi {
namespace test {

// ======================
// class MessageFormatter
// ======================

class MessageFormatter {
    // 'MessageFormatter' is used to populate/format a message. It supports
    // writing once only to each field. Attempting to set an already set
    // element will fail.
    //
    // Note that the behavior is undefined if
    // - A message formatted with 'formatMessageJson()' or
    //  'formatMessageXml()' is further formatted.
    // - A message formatted with 'set...()' or 'append...()' is further
    //   formatted using 'formatMessageJson()/formatMessageXml()'.

  private:
    blpapi_MessageFormatter_t *d_handle;

  public:
    // CREATORS
    explicit MessageFormatter(blpapi_MessageFormatter_t *handle);
    // Creates 'MessageFormatter' from 'handle' and takes ownership of the
    // 'handle'.

    MessageFormatter(const MessageFormatter& original);
    // Creates 'MessageFormatter' from 'original'. Changes made by one
    // copy is visible to the other.

    ~MessageFormatter();
    // Destroy this 'MessageFormatter' and release the 'd_handle'.

    // MANIPULATORS
    MessageFormatter& operator=(const MessageFormatter& rhs);
    // Make this 'MessageFormatter' same as 'rhs'.

    void setElement(const Name& name, bool value);
    // Set the element with the specified 'name' to the specified 'value'
    // in the current message referenced by this 'MessageFormatter'. If the
    // 'name' is invalid for the current message or if the element
    // identified by 'name' has already been set an exception is thrown.

    void setElement(const Name& name, char value);
    // Set the element with the specified 'name' to the specified 'value'
    // in the current message referenced by this 'MessageFormatter'. If the
    // 'name' is invalid for the current message or if the element
    // identified by 'name' has already been set an exception is thrown.

    void setElement(const Name& name, Int32 value);
    // Set the element with the specified 'name' to the specified 'value'
    // in the current message referenced by this 'MessageFormatter'. If the
    // 'name' is invalid for the current message or if the element
    // identified by 'name' has already been set an exception is thrown.

    void setElement(const Name& name, Int64 value);
    // Set the element with the specified 'name' to the specified 'value'
    // in the current message referenced by this 'MessageFormatter'. If the
    // 'name' is invalid for the current message or if the element
    // identified by 'name' has already been set an exception is thrown.

    void setElement(const Name& name, Float32 value);
    // Set the element with the specified 'name' to the specified 'value'
    // in the current message referenced by this 'MessageFormatter'. If the
    // 'name' is invalid for the current message or if the element
    // identified by 'name' has already been set an exception is thrown.

    void setElement(const Name& name, Float64 value);
    // Set the element with the specified 'name' to the specified 'value'
    // in the current message referenced by this 'MessageFormatter'. If the
    // 'name' is invalid for the current message or if the element
    // identified by 'name' has already been set an exception is thrown.

    void setElement(const Name& name, const Datetime& value);
    // Set the element with the specified 'name' to the specified 'value'
    // in the current message referenced by this 'MessageFormatter'. If the
    // 'name' is invalid for the current message or if the element
    // identified by 'name' has already been set an exception is thrown.

    void setElement(const Name& name, const Datetime::HighPrecision& value);
    // Set the element with the specified 'name' to the specified 'value'
    // in the current message referenced by this 'MessageFormatter'. If the
    // 'name' is invalid for the current message or if the element
    // identified by 'name' has already been set an exception is thrown.

    void setElement(const Name& name, const char *value);
    // Set the element with the specified 'name' to the specified 'value'
    // in the current message referenced by this 'MessageFormatter'. If the
    // 'name' is invalid for the current message, or if the element
    // identified by 'name' has already been set an exception is thrown.
    // The behavior is undefined unless 'value' is not 'NULL'.
    // Clients wishing to format null values (e.g. for the purpose of cache
    // management) should *not* use this function; use 'setElementNull'
    // instead.

    void setElement(const Name& name, const Name& value);
    // Set the element with the specified 'name' to the specified 'value'
    // in the current message referenced by this 'MessageFormatter'. If the
    // 'name' is invalid for the current message or if the element
    // identified by 'name' has already been set an exception is thrown.

    void setElementNull(const Name& name);
    // Create a null element with the specified 'name'.  Note that whether
    // or not fields containing null values are published to subscribers is
    // dependent upon details of the service and schema configuration.  If
    // the 'name' is invalid for the current message or if the element
    // identified by 'name' has already been set an exception is thrown.

    void pushElement(const Name& name);
    // Change the level at which this 'MessageFormatter' is operating to
    // the specified element 'name'.  The element 'name' must identify
    // either a choice, a sequence or an array at the current level of the
    // schema or the behavior is undefined.  If the 'name' is invalid for
    // the current message or if the element identified by 'name' has
    // already been set an exception is thrown.
    //
    // After this returns, the context of the 'MessageFormatter' is set to
    // the element 'name' in the schema and any calls to 'setElement()' or
    // 'pushElement()' are applied at that level.
    //
    // If 'name' represents an array of scalars then 'appendValue()'
    // must be used to add values.
    //
    // If 'name' represents an array of complex types then 'appendElement()'
    // must be used.

    void popElement();
    // Undo the most recent call to 'pushElement()' or 'appendElement()' on
    // this 'MessageFormatter' and return the context of the
    // 'MessageFormatter' to where it was before the call to
    // 'pushElement()' or 'appendElement()'. Once 'popElement()' has been
    // called it is invalid to attempt to re-visit the same context.

    void appendValue(bool value);
    // Append the specified 'value' to element on which this
    // 'MessageFormatter' is operating.  Throw an exception if the schema
    // of the message is flat or the current element to which 'value' is
    // appended is not an array.

    void appendValue(char value);
    // Append the specified 'value' to element on which this
    // 'MessageFormatter' is operating. Throw an exception if the schema of
    // the message is flat or the current element to which 'value' is
    // appended is not an array.

    void appendValue(Int32 value);
    // Append the specified 'value' to element on which this
    // 'MessageFormatter' is operating. Throw an exception if the schema of
    // the message is flat or the current element to which 'value' is
    // appended is not an array.

    void appendValue(Int64 value);
    // Append the specified 'value' to element on which this
    // 'MessageFormatter' is operating. Throw an exception if the schema of
    // the message is flat or the current element to which 'value' is
    // appended is not an array.

    void appendValue(Float32 value);
    // Append the specified 'value' to element on which this
    // 'MessageFormatter' is operating. Throw an exception if the schema of
    // the message is flat or the current element to which 'value' is
    // appended is not an array.

    void appendValue(Float64 value);
    // Append the specified 'value' to element on which this
    // 'MessageFormatter' is operating. Throw an exception if the schema of
    // the message is flat or the current element to which 'value' is
    // appended is not an array.

    void appendValue(const Datetime& value);
    // Append the specified 'value' to element on which this
    // 'MessageFormatter' is operating. Throw an exception if the schema of
    // the message is flat or the current element to which 'value' is
    // appended is not an array.

    void appendValue(const Datetime::HighPrecision& value);
    // Append the specified 'value' to element on which this
    // 'MessageFormatter' is operating. Throw an exception if the schema of
    // the message is flat or the current element to which 'value' is
    // appended is not an array.

    void appendValue(const char *value);
    // Append the specified 'value' to element on which this
    // 'MessageFormatter' is operating. Throw an exception if the schema of
    // the message is flat or the current element to which 'value' is
    // appended is not an array.

    void appendValue(const Name& value);
    // Append the specified 'value' to element on which this
    // 'MessageFormatter' is operating. Throw an exception if the schema of
    // the message is flat or the current element to which 'value' is
    // appended is not an array.

    void appendElement();
    // Create an array element and append it to the element on which this
    // 'MessageFormatter' is operating on. Throw an exception if the schema
    // of the message is flat or the current element to which 'value' is
    // appended is not an array, a sequence or a choice.

    void formatMessageJson(const char *message);
    // Format the message from its 'JSON' representation.
    // An exception is thrown if the method fails to format the message.
    // The behavior is undefined if further formatting is done using
    // this 'MessageFormatter'.

    void formatMessageXml(const char *message);
    // Format the message from its 'XML' representation.
    // An exception is thrown if the method fails to format the message.
    // The behavior is undefined if further formatting is done using
    // this 'MessageFormatter'.

    // ACCESSORS
    blpapi_MessageFormatter_t *impl() const;
    // Returns underlying implementation of 'MessageFormatter'. For
    // *internal* use only.
};

// ============================================================================
//                      INLINE AND TEMPLATE FUNCTION IMPLEMENTATIONS
// ============================================================================

// ----------------------
// class MessageFormatter
// ----------------------

inline MessageFormatter::MessageFormatter(blpapi_MessageFormatter_t *handle)
    : d_handle(handle)
{
}

inline MessageFormatter::~MessageFormatter()
{
    if (d_handle) {
        BLPAPI_CALL_UNCHECKED(blpapi_MessageFormatter_destroy)(d_handle);
    }
}

inline MessageFormatter::MessageFormatter(const MessageFormatter& original)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(blpapi_MessageFormatter_copy)(
            &d_handle, original.impl()));
}

inline MessageFormatter& MessageFormatter::operator=(
        const MessageFormatter& rhs)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(blpapi_MessageFormatter_assign)(
            &d_handle, rhs.impl()));
    return *this;
}

inline void MessageFormatter::setElement(const Name& name, bool value)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageFormatter_setValueBool)(
                    d_handle, name.impl(), value));
}

inline void MessageFormatter::setElement(const Name& name, char value)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageFormatter_setValueChar)(
                    d_handle, name.impl(), value));
}

inline void MessageFormatter::setElement(const Name& name, Int32 value)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageFormatter_setValueInt32)(
                    d_handle, name.impl(), value));
}

inline void MessageFormatter::setElement(const Name& name, Int64 value)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageFormatter_setValueInt64)(
                    d_handle, name.impl(), value));
}

inline void MessageFormatter::setElement(const Name& name, Float32 value)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageFormatter_setValueFloat32)(
                    d_handle, name.impl(), value));
}
inline void MessageFormatter::setElement(const Name& name, Float64 value)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageFormatter_setValueFloat64)(
                    d_handle, name.impl(), value));
}
inline void MessageFormatter::setElement(
        const Name& name, const Datetime& value)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageFormatter_setValueDatetime)(
                    d_handle, name.impl(), &value.rawValue()));
}

inline void MessageFormatter::setElement(
        const Name& name, const Datetime::HighPrecision& value)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageFormatter_setValueHighPrecisionDatetime)(
                    d_handle, name.impl(), &value));
}

inline void MessageFormatter::setElement(const Name& name, const char *value)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageFormatter_setValueString)(
                    d_handle, name.impl(), value));
}
inline void MessageFormatter::setElement(const Name& name, const Name& value)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageFormatter_setValueFromName)(
                    d_handle, name.impl(), value.impl()));
}

inline void MessageFormatter::setElementNull(const Name& name)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(
            blpapi_MessageFormatter_setValueNull)(d_handle, name.impl()));
}

inline void MessageFormatter::pushElement(const Name& name)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(
            blpapi_MessageFormatter_pushElement)(d_handle, name.impl()));
}

inline void MessageFormatter::popElement()
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageFormatter_popElement)(d_handle));
}

inline void MessageFormatter::appendValue(bool value)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(
            blpapi_MessageFormatter_appendValueBool)(d_handle, value));
}

inline void MessageFormatter::appendValue(char value)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(
            blpapi_MessageFormatter_appendValueChar)(d_handle, value));
}

inline void MessageFormatter::appendValue(Int32 value)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(
            blpapi_MessageFormatter_appendValueInt32)(d_handle, value));
}

inline void MessageFormatter::appendValue(Int64 value)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(
            blpapi_MessageFormatter_appendValueInt64)(d_handle, value));
}

inline void MessageFormatter::appendValue(Float32 value)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(
            blpapi_MessageFormatter_appendValueFloat32)(d_handle, value));
}

inline void MessageFormatter::appendValue(Float64 value)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(
            blpapi_MessageFormatter_appendValueFloat64)(d_handle, value));
}

inline void MessageFormatter::appendValue(const Datetime& value)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageFormatter_appendValueDatetime)(
                    d_handle, &value.rawValue()));
}

inline void MessageFormatter::appendValue(const Datetime::HighPrecision& value)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(
            blpapi_MessageFormatter_appendValueHighPrecisionDatetime)(
            d_handle, &value));
}

inline void MessageFormatter::appendValue(const char *value)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(
            blpapi_MessageFormatter_appendValueString)(d_handle, value));
}

inline void MessageFormatter::appendValue(const Name& value)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageFormatter_appendValueFromName)(
                    d_handle, value.impl()));
}

inline void MessageFormatter::appendElement()
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_MessageFormatter_appendElement)(d_handle));
}

inline blpapi_MessageFormatter_t *MessageFormatter::impl() const
{
    return d_handle;
}

inline void MessageFormatter::formatMessageJson(const char *message)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(
            blpapi_MessageFormatter_FormatMessageJson)(d_handle, message));
}

inline void MessageFormatter::formatMessageXml(const char *message)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(
            blpapi_MessageFormatter_FormatMessageXml)(d_handle, message));
}

} // close namespace test

// *Deprecated*
// Following typedef is provided for backwards compatibility. It will be
// removed in a future release.
typedef test::MessageFormatter MessageFormatter;

} // close namespace blpapi
} // close namespace BloombergLP

#endif // #ifdef __cplusplus
#endif // #ifndef INCLUDED_BLPAPI_MESSAGEFORMATTER
