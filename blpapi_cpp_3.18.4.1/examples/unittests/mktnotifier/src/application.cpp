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

#include "application.h"

#include <blpapi_identity.h>

namespace blp = BloombergLP::blpapi;

void Application::run()
{
    bool rc;
    rc = d_session->start();

    if (!rc) {
        std::cerr << "Failed to start session." << std::endl;
        return;
    }

    blp::Identity identity;
    rc = d_authorizer->authorize(&identity, d_config->d_authOptions);

    if (!rc) {
        std::cerr << "No authorization" << std::endl;
        return;
    }

    d_subscriber->subscribe(d_config->d_service,
            d_config->d_topics,
            d_config->d_fields,
            d_config->d_options,
            identity);
}
