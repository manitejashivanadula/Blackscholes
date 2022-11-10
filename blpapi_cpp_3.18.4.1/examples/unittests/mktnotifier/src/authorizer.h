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

#ifndef _AUTHORIZER_H_
#define _AUTHORIZER_H_

#include <blpapi_correlationid.h>
#include <blpapi_identity.h>
#include <blpapi_service.h>
#include <blpapi_session.h>

#include "tokengenerator.h"

namespace blp = BloombergLP::blpapi;

class IAuthorizer {
  public:
    virtual bool authorize(blp::Identity *identity,
            const std::string& authOptions,
            blp::EventQueue *queue = 0)
            = 0;

    virtual ~IAuthorizer() { }
};

class Authorizer : public IAuthorizer {
  private:
    blp::Session *d_session;
    ITokenGenerator *d_tokenGenerator;

    bool authorize(blp::Identity *identity,
            const blp::Service& authService,
            const blp::CorrelationId& cid,
            blp::EventQueue *queue);

  public:
    Authorizer(blp::Session *session, ITokenGenerator *tokenGenerator)
        : d_session(session)
        , d_tokenGenerator(tokenGenerator)
    {
    }

    virtual bool authorize(blp::Identity *identity,
            const std::string& authOptions,
            blp::EventQueue *queue);
};

#endif
