/* Copyright 2018. Bloomberg Finance L.P.
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
// blpapi_zfputil.h                                                   -*-C++-*-
#ifndef INCLUDED_BLPAPI_ZFPUTIL
#define INCLUDED_BLPAPI_ZFPUTIL

//@PURPOSE: Automatic creation of SessionOptions for Zero Footprint solution
// clients that leverage private leased lines to the Bloomberg network.
//
//@CLASSES:
//  blpapi::ZfpUtil: Wrapper for Zero Footprint utilities
//
//@SEE_ALSO: blpapi_sessionoptions
//           blpapi_tlsoptions
//
//@DESCRIPTION: The utilities provided in this component are designed for
// the Zero Footprint solution for BLPAPI.
//
/// Usage
///-----
// The following snippet shows how to use a ZfpUtil method for starting a
// 'Session' when using leased lines.
//..
// blpapi::TlsOptions tlsOptions
//                          = blpapi::TlsOptions::createFromFiles( /* ... */ );
//
// blpapi::SessionOptions opts = blpapi::ZfpUtil::getZfpOptionsForLeasedLines(
//                                                blpapi::ZfpUtil::REMOTE_8194,
//                                                tlsOptions);
//
// opts.setAuthorizationOptions(/* ... */);
//
// blpapi::Session session = blpapi::Session(opts);
// session.start();
//..
// Note that the resulting 'SessionOptions' can be used in multiple sessions,
// so this operation only needs to be called once per application.

#ifndef INCLUDED_BLPAPI_CALL
#include <blpapi_call.h>
#endif

#ifndef INCLUDED_BLPAPI_EXCEPTION
#include <blpapi_exception.h>
#endif

#ifndef INCLUDED_BLPAPI_DEFS
#include <blpapi_defs.h>
#endif

#ifndef INCLUDED_BLPAPI_SESSIONOPTIONS
#include <blpapi_sessionoptions.h>
#endif

#ifndef INCLUDED_BLPAPI_TLSOPTIONS
#include <blpapi_tlsoptions.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

BLPAPI_EXPORT
int blpapi_ZfpUtil_getOptionsForLeasedLines(
        blpapi_SessionOptions_t *sessionOptions,
        const blpapi_TlsOptions_t *tlsOptions,
        int remote);

#ifdef __cplusplus
}

namespace BloombergLP {
namespace blpapi {

// =============
// class ZfpUtil
// =============

class ZfpUtil {
    // A wrapper for Zero Footprint solution utilities

  public:
    enum Remote {
        REMOTE_8194 = BLPAPI_ZFPUTIL_REMOTE_8194,
        REMOTE_8196 = BLPAPI_ZFPUTIL_REMOTE_8196
    };

    static SessionOptions getZfpOptionsForLeasedLines(
            Remote remote, const TlsOptions& tlsOptions);
    // Creates a 'SessionOptions' object for applications that leverages
    // private leased lines to Bloomberg network. The 'SessionOptions'
    // object returned is only valid for private leased line connectivity.
    // This is a costly operation that is preferably called once per
    // application during start. The returned 'SessionOptions' can be
    // copied and reused on different sessions. On failure
    // (e.g. connectivity issues), 'blpapi::Exception' is thrown.
};

// ============================================================================
//                      INLINE FUNCTION DEFINITIONS
// ============================================================================

// -------------
// class ZfpUtil
// -------------
inline SessionOptions ZfpUtil::getZfpOptionsForLeasedLines(
        Remote remote, const TlsOptions& tlsOptions)
{
    SessionOptions so;
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_ZfpUtil_getOptionsForLeasedLines)(
                    so.handle(), tlsOptions.handle(), remote));
    return so;
}

} // close namespace blpapi
} // close namespace BloombergLP

#endif // #ifdef __cplusplus
#endif // #ifndef INCLUDED_BLPAPI_ZFPUTIL
