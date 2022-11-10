/* Copyright 2021, Bloomberg Finance L.P.
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
#include <blpapi_element.h>
#include <blpapi_event.h>
#include <blpapi_exception.h>
#include <blpapi_message.h>
#include <blpapi_names.h>
#include <blpapi_session.h>
#include <util/ArgParser.h>
#include <util/Utils.h>

#include <iostream>
#include <string>

using namespace BloombergLP;
using namespace blpapi;

namespace {
const char *AUTH_USER = "AuthenticationType=OS_LOGON";
const char *AUTH_APP_PREFIX
        = "AuthenticationMode=APPLICATION_ONLY;ApplicationAuthenticationType="
          "APPNAME_AND_KEY;ApplicationName=";
const char *AUTH_USER_APP_PREFIX
        = "AuthenticationMode=USER_AND_APPLICATION;AuthenticationType=OS_"
          "LOGON;ApplicationAuthenticationType=APPNAME_AND_KEY;"
          "ApplicationName=";
const char *AUTH_DIR_PREFIX
        = "AuthenticationType=DIRECTORY_SERVICE;DirSvcPropertyName=";

const char *AUTH_OPTION_USER = "user";
const char *AUTH_OPTION_APP = "app";
const char *AUTH_OPTION_USER_APP = "userapp";
const char *AUTH_OPTION_DIR = "dir";
}

class GenerateTokenExample {
    std::string d_host;
    int d_port;
    std::string d_authOptions;

  public:
    GenerateTokenExample()
        : d_host("localhost")
        , d_port(8194)
        , d_authOptions(AUTH_USER)
    {
    }

    bool parseAuthOptions(const std::string& value)
    {
        std::vector<std::string> tokens = Utils::split(value, '=');
        const std::string token0 = tokens[0];
        const std::string token1 = tokens.size() == 1u ? "" : tokens[1];

        if (token0 == AUTH_OPTION_USER) {
            d_authOptions = AUTH_USER;
        } else {
            if (token1.empty()) {
                std::cerr << "Invalid auth option " << value << "\n";
                return false;
            }

            if (token0 == AUTH_OPTION_APP) {
                d_authOptions = std::string(AUTH_APP_PREFIX) + token1;
            } else if (token0 == AUTH_OPTION_USER_APP) {
                d_authOptions = std::string(AUTH_USER_APP_PREFIX) + token1;
            } else if (token0 == AUTH_OPTION_DIR) {
                d_authOptions = std::string(AUTH_DIR_PREFIX) + token1;
            } else {
                std::cerr << "Invalid authentication option: " << value
                          << "\n";
                return false;
            }
        }

        return true;
    }

    void parseServerPort(const char *address)
    {
        std::vector<std::string> tokens = Utils::split(address, ':');
        if (tokens.size() != 2) {
            throw std::invalid_argument(
                    std::string("Invalid server option: ") + address);
        }

        d_host = tokens[0];
        d_port = std::stoi(tokens[1]);
    }

    void run(int argc, const char **argv)
    {
        ArgParser argParser(
                "Generate a token for a user to be used on the server side",
                "GenerateTokenExample");
        try {
            ArgGroup& groupServer = argParser.addGroup("Connections");
            Arg& arg = groupServer.addArg("host", 'H')
                               .setMetaVar("host:port")
                               .setDescription("Server name or IP and port "
                                               "separated by ':'")
                               .setIsRequired(true)
                               .setAction([this](const char *value) {
                                   parseServerPort(value);
                               });

            ArgGroup& groupAuth = argParser.addGroup("Authentication");
            Arg argAuth = groupAuth.addArg("auth", 'a')
                                  .setMetaVar("option")
                                  .setDescription(
                                          R"(Authentication option
      user              as a user using OS logon  information
      dir=<property>    as a user using directory services
      app=<app>         as the specified application
      userapp=<app>     as user and application using logon information for the user)")
                                  .setIsRequired(true)
                                  .setAction([this](const char *value) {
                                      parseAuthOptions(value);
                                  });
            argParser.parse(argc, argv);
        } catch (std::exception& ex) {
            std::cerr << ex.what() << "\n";
            argParser.printHelp();
            return;
        }

        SessionOptions sessionOptions;
        sessionOptions.setServerHost(d_host.c_str());
        sessionOptions.setServerPort(d_port);
        sessionOptions.setAuthenticationOptions(d_authOptions.c_str());

        std::cout << "Connecting to " << d_host << ":" << d_port << "\n";
        Session session(sessionOptions);
        if (!session.start()) {
            std::cerr << "Failed to start session.\n";
            return;
        }

        session.generateToken();
        while (true) {
            Event event = session.nextEvent();
            if (event.eventType() != Event::TOKEN_STATUS) {
                continue;
            }

            MessageIterator msgIter(event);
            while (msgIter.next()) {
                const Message msg = msgIter.message();

                const Name messageType = msg.messageType();
                if (messageType == Names::tokenGenerationSuccess()) {
                    const char *token = msg.getElementAsString("token");
                    std::cout << "Token is successfully generated: " << token
                              << "\n";
                } else if (messageType == Names::tokenGenerationFailure()) {
                    std::cout << "Failed to generate token:";
                    msg.print(std::cout) << "\n";
                }
            }

            break;
        }
    }
};

int main(int argc, const char **argv)
{
    GenerateTokenExample example;
    try {
        example.run(argc, argv);
    } catch (Exception& e) {
        std::cerr << "Library Exception!!! " << e.description() << "\n";
    }

    // wait for enter key to exit application
    std::cout << "Press ENTER to quit\n";
    char dummy[2];
    std::cin.getline(dummy, 2);

    return 0;
}
