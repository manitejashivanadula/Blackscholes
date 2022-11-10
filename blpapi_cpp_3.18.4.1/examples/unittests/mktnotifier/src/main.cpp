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

#include <blpapi_exception.h>
#include <blpapi_session.h>

#include "appconfig.h"
#include "application.h"
#include "authorizer.h"
#include "computeengine.h"
#include "eventprocessor.h"
#include "notifier.h"
#include "subscriber.h"
#include "tokengenerator.h"

#include <iostream>

namespace blp = BloombergLP::blpapi;

int main(int argc, char **argv)
{
    // Initialize config based on command line options
    AppConfig config;
    if (!config.parseCommandLine(argc, argv)) {
        std::cout << "Invalid command line parameters" << std::endl;
        return 1;
    }

    Notifier notifier;
    ComputeEngine computeEngine;
    EventProcessor eventProcessor(&notifier, &computeEngine);
    blp::SessionOptions sessionOptions;
    for (size_t i = 0; i < config.d_hosts.size(); ++i) {
        sessionOptions.setServerAddress(
                config.d_hosts[i].c_str(), config.d_port, i);
    }
    sessionOptions.setAuthenticationOptions(config.d_authOptions.c_str());

    blp::Session session(sessionOptions, &eventProcessor);
    TokenGenerator tokenGenerator(&session);

    Authorizer authorizer(&session, &tokenGenerator);
    Subscriber subscriber(&session);

    Application app(
            &session, &authorizer, &subscriber, &eventProcessor, &config);

    try {
        app.run();
    } catch (blp::Exception& e) {
        std::cerr << "Library Exception" << e.description() << std::endl;
    }

    // wait for enter key to exit application
    std::cout << "Press ENTER to quit" << std::endl;
    char dummy[2];
    std::cin.getline(dummy, 2);

    return 0;
}
