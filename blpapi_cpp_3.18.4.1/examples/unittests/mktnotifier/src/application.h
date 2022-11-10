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

#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <blpapi_session.h>

#include "appconfig.h"
#include "authorizer.h"
#include "eventprocessor.h"
#include "subscriber.h"

namespace blp = BloombergLP::blpapi;

class Application {
  private:
    blp::Session *d_session;
    IAuthorizer *d_authorizer;
    ISubscriber *d_subscriber;
    blp::EventHandler *d_eventProcessor;
    AppConfig *d_config;

  public:
    Application(blp::Session *session,
            IAuthorizer *authorizer,
            ISubscriber *subscriber,
            blp::EventHandler *eventProcessor,
            AppConfig *config);

    void run();
};

inline Application::Application(blp::Session *session,
        IAuthorizer *authorizer,
        ISubscriber *subscriber,
        blp::EventHandler *eventProcessor,
        AppConfig *config)
    : d_session(session)
    , d_authorizer(authorizer)
    , d_subscriber(subscriber)
    , d_eventProcessor(eventProcessor)
    , d_config(config)
{
}

#endif
