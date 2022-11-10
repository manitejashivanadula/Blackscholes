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
// blpapi_authoptions.h                                            -*-C++-*-
#ifndef INCLUDED_BLPAPI_AUTHOPTIONS
#define INCLUDED_BLPAPI_AUTHOPTIONS

//@PURPOSE: Provide a configuration setting which uses a set of strongly-typed
// overloads to specify the settings used for authorization and authorization.
//
//@CLASSES:
// blpapi::AuthOptions: describe the authorization options to be used
// blpapi::AuthUser: user-specific authorization option
// blpapi::AuthApplication: application-specific authorization option
// blpapi::AuthToken: manual token authorization option
//
//@DESCRIPTION: This file defines a 'AuthOptions' class which is used to
// specify authorization options through strongly-typed overloads instead of
// using a bare const char * string.

#include <blpapi_call.h>
#include <blpapi_defs.h>
#include <blpapi_exception.h>
#include <blpapi_types.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

BLPAPI_EXPORT
int blpapi_AuthOptions_create_default(blpapi_AuthOptions_t **options);

BLPAPI_EXPORT
int blpapi_AuthOptions_create_forUserMode(
        blpapi_AuthOptions_t **options, const blpapi_AuthUser_t *user);

BLPAPI_EXPORT
int blpapi_AuthOptions_create_forAppMode(
        blpapi_AuthOptions_t **options, const blpapi_AuthApplication_t *app);

BLPAPI_EXPORT
int blpapi_AuthOptions_create_forUserAndAppMode(blpapi_AuthOptions_t **options,
        const blpapi_AuthUser_t *user,
        const blpapi_AuthApplication_t *app);

BLPAPI_EXPORT
int blpapi_AuthOptions_create_forToken(
        blpapi_AuthOptions_t **options, const blpapi_AuthToken_t *token);

BLPAPI_EXPORT
int blpapi_AuthOptions_duplicate(
        blpapi_AuthOptions_t **options, const blpapi_AuthOptions_t *dup);

BLPAPI_EXPORT
int blpapi_AuthOptions_copy(
        blpapi_AuthOptions_t *lhs, const blpapi_AuthOptions_t *rhs);

BLPAPI_EXPORT
void blpapi_AuthOptions_destroy(blpapi_AuthOptions_t *options);

BLPAPI_EXPORT
int blpapi_AuthUser_createWithLogonName(blpapi_AuthUser_t **user);

BLPAPI_EXPORT
int blpapi_AuthUser_createWithActiveDirectoryProperty(
        blpapi_AuthUser_t **user, const char *propertyName);

BLPAPI_EXPORT
int blpapi_AuthUser_createWithManualOptions(
        blpapi_AuthUser_t **user, const char *userId, const char *ipAddress);

BLPAPI_EXPORT
int blpapi_AuthUser_duplicate(
        blpapi_AuthUser_t **user, const blpapi_AuthUser_t *dup);

BLPAPI_EXPORT
int blpapi_AuthUser_copy(blpapi_AuthUser_t *lhs, const blpapi_AuthUser_t *rhs);

BLPAPI_EXPORT
void blpapi_AuthUser_destroy(blpapi_AuthUser_t *user);

BLPAPI_EXPORT
int blpapi_AuthApplication_create(
        blpapi_AuthApplication_t **app, const char *appName);

BLPAPI_EXPORT
int blpapi_AuthApplication_duplicate(
        blpapi_AuthApplication_t **app, const blpapi_AuthApplication_t *dup);

BLPAPI_EXPORT
int blpapi_AuthApplication_copy(
        blpapi_AuthApplication_t *lhs, const blpapi_AuthApplication_t *rhs);

BLPAPI_EXPORT
void blpapi_AuthApplication_destroy(blpapi_AuthApplication_t *app);

BLPAPI_EXPORT
int blpapi_AuthToken_create(blpapi_AuthToken_t **token, const char *tokenStr);

BLPAPI_EXPORT
int blpapi_AuthToken_duplicate(
        blpapi_AuthToken_t **token, const blpapi_AuthToken_t *dup);

BLPAPI_EXPORT
int blpapi_AuthToken_copy(
        blpapi_AuthToken_t *lhs, const blpapi_AuthToken_t *rhs);

BLPAPI_EXPORT
void blpapi_AuthToken_destroy(blpapi_AuthToken_t *token);

#ifdef __cplusplus
}

namespace BloombergLP {
namespace blpapi {

class AuthUser;
class AuthApplication;
class AuthToken;

// =================
// class AuthOptions
// =================

class AuthOptions {
    // Contains the authorization options which the user can set on
    // 'SessionOptions' as the authorization options for the session identity
    // or use to authorize other 'Identity's.

    blpapi_AuthOptions_t *d_handle_p;

  public:
    AuthOptions();
    // Create an 'AuthOptions' object to specify no authorization should
    // be used.

    AuthOptions(const AuthUser& user);
    // Create an 'AuthOptions' object for User Mode with Operating System
    // Login (Domain\User), Active Directory, or Email.
    //
    // The behavior is undefined when 'user' was created with
    // 'AuthUser::createWithManualOptions()'.

    AuthOptions(const AuthApplication& app);
    // Create an 'AuthOptions' object for Application Mode.

    AuthOptions(const AuthUser& user, const AuthApplication& app);
    // Create an 'AuthOptions' object for User and Application Mode.

    AuthOptions(const AuthToken& token);
    // Create an 'AuthOptions' object for Manual Token Mode.

    explicit AuthOptions(blpapi_AuthOptions_t *handle);
    // For internal use only.

    AuthOptions(const AuthOptions& options);
    // Copy constructor.

    ~AuthOptions();
    // Destructor.

    AuthOptions& operator=(const AuthOptions& rhs);
    // Assign to this object the value of the specified 'rhs' object.

    blpapi_AuthOptions_t *handle() const;
    // For internal use only.
};

// =======================
// class AuthUser
// =======================

class AuthUser {
    // Contains user-specific authorization options.

    blpapi_AuthUser_t *d_handle_p;

  public:
    explicit AuthUser(blpapi_AuthUser_t *handle);
    // For internal use only.

    AuthUser(const AuthUser& user);
    // Copy constructor.

    ~AuthUser();
    // Destructor.

    AuthUser& operator=(const AuthUser& rhs);
    // Assign to this object the value of the specified 'rhs' object.

    static AuthUser createWithLogonName();
    // Create an 'AuthUser' configured for Operating System Login
    // (Domain\User) authorization mode (OS_LOGON).

    static AuthUser createWithActiveDirectoryProperty(
            const char *propertyName);
    // Create an 'AuthUser' configured for Active Directory authorization
    // mode (DIRECTORY_SERVICE).
    //
    // The behavior is undefined when 'propertyName' is 'NULL' or empty.

    static AuthUser createWithManualOptions(
            const char *userId, const char *ipAddress);
    // Create an 'AuthUser' configured for manual authorization.
    //
    // The behavior is undefined when either 'userId' or 'ipAddress' is
    // 'NULL' or empty.

    const blpapi_AuthUser_t *handle() const;
    // For internal use only.
};

// ==============================
// class AuthApplication
// ==============================

class AuthApplication {
    // Contains application-specific authorization options.

    blpapi_AuthApplication_t *d_handle_p;

  public:
    explicit AuthApplication(const char *appName);
    // Create an 'AuthApplication' object for authorization.
    //
    // The behavior is undefined when 'appName' is 'NULL' or empty.

    explicit AuthApplication(blpapi_AuthApplication_t *handle);
    // For internal use only.

    AuthApplication(const AuthApplication& app);
    // Copy constructor.

    ~AuthApplication();
    // Destructor.

    AuthApplication& operator=(const AuthApplication& rhs);
    // Assign to this object the value of the specified 'rhs' object.

    const blpapi_AuthApplication_t *handle() const;
    // For internal use only.
};

// ========================
// class AuthToken
// ========================

class AuthToken {
    // Contains manual token authorization options.

    blpapi_AuthToken_t *d_handle_p;

  public:
    explicit AuthToken(const char *tokenStr);
    // Create an 'AuthToken' to use for authorization.
    //
    // The behavior is undefined when 'tokenStr' is 'NULL' or empty.

    explicit AuthToken(blpapi_AuthToken_t *handle);
    // For internal use only.

    AuthToken(const AuthToken& token);
    // Copy constructor.

    ~AuthToken();
    // Destructor.

    AuthToken& operator=(const AuthToken& rhs);
    // Assign to this object the value of the specified 'rhs' object.

    const blpapi_AuthToken_t *handle() const;
    // For internal use only.
};

// ============================================================================
//                      INLINE FUNCTION DEFINITIONS
// ============================================================================

// =================
// class AuthOptions
// =================

inline AuthOptions::AuthOptions()
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_AuthOptions_create_default)(&d_handle_p));
}

inline AuthOptions::AuthOptions(const AuthUser& user)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_AuthOptions_create_forUserMode)(
                    &d_handle_p, user.handle()));
}

inline AuthOptions::AuthOptions(const AuthApplication& app)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(
            blpapi_AuthOptions_create_forAppMode)(&d_handle_p, app.handle()));
}

inline AuthOptions::AuthOptions(
        const AuthUser& user, const AuthApplication& app)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_AuthOptions_create_forUserAndAppMode)(
                    &d_handle_p, user.handle(), app.handle()));
}

inline AuthOptions::AuthOptions(const AuthToken& token)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(
            blpapi_AuthOptions_create_forToken)(&d_handle_p, token.handle()));
}

inline AuthOptions::AuthOptions(blpapi_AuthOptions_t *handle)
    : d_handle_p(handle)
{
}

inline AuthOptions::AuthOptions(const AuthOptions& options)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(blpapi_AuthOptions_duplicate)(
            &d_handle_p, options.handle()));
}

inline AuthOptions::~AuthOptions()
{
    BLPAPI_CALL_UNCHECKED(blpapi_AuthOptions_destroy)(d_handle_p);
}

inline AuthOptions& AuthOptions::operator=(const AuthOptions& rhs)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_AuthOptions_copy)(d_handle_p, rhs.handle()));

    return *this;
}

inline blpapi_AuthOptions_t *AuthOptions::handle() const { return d_handle_p; }

// =======================
// class AuthUser
// =======================

inline AuthUser::AuthUser(blpapi_AuthUser_t *handle)
    : d_handle_p(handle)
{
}

inline AuthUser::AuthUser(const AuthUser& user)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(blpapi_AuthUser_duplicate)(
            &d_handle_p, user.handle()));
}

inline AuthUser::~AuthUser()
{
    BLPAPI_CALL_UNCHECKED(blpapi_AuthUser_destroy)(d_handle_p);
}

inline AuthUser& AuthUser::operator=(const AuthUser& rhs)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_AuthUser_copy)(d_handle_p, rhs.handle()));

    return *this;
}

inline AuthUser AuthUser::createWithLogonName()
{
    blpapi_AuthUser_t *handle = NULL;

    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_AuthUser_createWithLogonName)(&handle));

    AuthUser user(handle);

    return user;
}

inline AuthUser AuthUser::createWithActiveDirectoryProperty(
        const char *propertyName)
{
    blpapi_AuthUser_t *handle = NULL;

    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_AuthUser_createWithActiveDirectoryProperty)(
                    &handle, propertyName));

    AuthUser user(handle);

    return user;
}

inline AuthUser AuthUser::createWithManualOptions(
        const char *userId, const char *ipAddress)
{
    blpapi_AuthUser_t *handle = NULL;

    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_AuthUser_createWithManualOptions)(
                    &handle, userId, ipAddress));

    AuthUser user(handle);

    return user;
}

inline const blpapi_AuthUser_t *AuthUser::handle() const { return d_handle_p; }

// ==============================
// class AuthApplication
// ==============================

inline AuthApplication::AuthApplication(const char *name)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_AuthApplication_create)(&d_handle_p, name));
}

inline AuthApplication::AuthApplication(blpapi_AuthApplication_t *handle)
    : d_handle_p(handle)
{
}

inline AuthApplication::AuthApplication(const AuthApplication& app)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(blpapi_AuthApplication_duplicate)(
            &d_handle_p, app.handle()));
}

inline AuthApplication::~AuthApplication()
{
    BLPAPI_CALL_UNCHECKED(blpapi_AuthApplication_destroy)(d_handle_p);
}

inline AuthApplication& AuthApplication::operator=(const AuthApplication& rhs)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(blpapi_AuthApplication_copy)(
            d_handle_p, rhs.handle()));

    return *this;
}

inline const blpapi_AuthApplication_t *AuthApplication::handle() const
{
    return d_handle_p;
}

// ========================
// class AuthToken
// ========================

inline AuthToken::AuthToken(const char *tokenStr)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_AuthToken_create)(&d_handle_p, tokenStr));
}

inline AuthToken::AuthToken(blpapi_AuthToken_t *handle)
    : d_handle_p(handle)
{
}

inline AuthToken::AuthToken(const AuthToken& token)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL(blpapi_AuthToken_duplicate)(
            &d_handle_p, token.handle()));
}

inline AuthToken::~AuthToken()
{
    BLPAPI_CALL_UNCHECKED(blpapi_AuthToken_destroy)(d_handle_p);
}

inline AuthToken& AuthToken::operator=(const AuthToken& rhs)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_AuthToken_copy)(d_handle_p, rhs.handle()));

    return *this;
}

inline const blpapi_AuthToken_t *AuthToken::handle() const
{
    return d_handle_p;
}

} // close namespace blpapi
} // close namespace BloombergLP

#endif // #ifdef __cplusplus
#endif // #ifndef INCLUDED_BLPAPI_AUTHOPTIONS
