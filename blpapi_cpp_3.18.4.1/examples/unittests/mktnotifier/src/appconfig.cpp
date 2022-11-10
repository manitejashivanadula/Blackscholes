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

#include "appconfig.h"
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace {
const std::string AUTH_USER = "AuthenticationType=OS_LOGON";
const std::string AUTH_APP_PREFIX
        = "AuthenticationMode=APPLICATION_ONLY;"
          "ApplicationAuthenticationType=APPNAME_AND_KEY;"
          "ApplicationName=";
const std::string AUTH_USER_APP_PREFIX
        = "AuthenticationMode=USER_AND_APPLICATION;"
          "AuthenticationType=OS_LOGON;"
          "ApplicationAuthenticationType=APPNAME_AND_KEY;"
          "ApplicationName=";
const std::string AUTH_USER_APP_MANUAL_PREFIX
        = "AuthenticationMode=USER_AND_APPLICATION;"
          "AuthenticationType=MANUAL;"
          "ApplicationAuthenticationType=APPNAME_AND_KEY;"
          "ApplicationName=";
const std::string AUTH_DIR_PREFIX = "AuthenticationType=DIRECTORY_SERVICE;"
                                    "DirSvcPropertyName=";

const char AUTH_OPTION_NONE[] = "none";
const char AUTH_OPTION_USER[] = "user";
const char AUTH_OPTION_APP[] = "app=";
const char AUTH_OPTION_USER_APP[] = "userapp=";
const char AUTH_OPTION_DIR[] = "dir=";

const char USAGE[]
        = "Retrieve realtime data.\n\n"
          "Usage:\n"
          "\t[-ip   <ipAddress>]    server name or IP (default: localhost)\n"
          "\t[-p    <tcpPort>]      server port (default: 8194)\n"
          "\t[-s    <service>]      service name (default: //blp/mktdata))\n"
          "\t[-t    <topic>]        topic name (default: /ticker/IBM US "
          "Equity)\n"
          "\t[-f    <field>]        field to subscribe to (default: empty)\n"
          "\t[-o    <option>]       subscription options (default: empty)\n"
          "\t[-auth <option>]       authentication option (default: user):\n"
          "\t\tnone\n"
          "\t\tuser                    as a user using OS logon information\n"
          "\t\tdir=<property>          as a user using directory services\n"
          "\t\tapp=<app>               as the specified application\n"
          "\t\tuserapp=<app>           as user and application using logon "
          "information\n"
          "\t\t                        for the user\n"
          "\t\tmanual=<app>,<ip>,<usr> as user and application, with manually "
          "provided\n"
          "\t\t                        IP address and EMRS user\n"
          "\n";
}

AppConfig::AppConfig()
    : d_port(8194)
{
}

void AppConfig::printUsage() { std::cout << USAGE << std::flush; }

bool AppConfig::parseCommandLine(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "-ip") && i + 1 < argc) {
            d_hosts.push_back(argv[++i]);
        } else if (!std::strcmp(argv[i], "-p") && i + 1 < argc) {
            d_port = std::atoi(argv[++i]);
        } else if (!std::strcmp(argv[i], "-s") && i + 1 < argc) {
            d_service = argv[++i];
        } else if (!std::strcmp(argv[i], "-t") && i + 1 < argc) {
            d_topics.push_back(argv[++i]);
        } else if (!std::strcmp(argv[i], "-f") && i + 1 < argc) {
            d_fields.push_back(argv[++i]);
        } else if (!std::strcmp(argv[i], "-o") && i + 1 < argc) {
            d_options.push_back(argv[++i]);
        } else if (!std::strcmp(argv[i], "-auth") && i + 1 < argc) {
            ++i;
            if (!std::strcmp(argv[i], AUTH_OPTION_NONE)) {
                d_authOptions.clear();
            } else if (!strcmp(argv[i], AUTH_OPTION_APP)) {
                d_authOptions.clear();
                d_authOptions.append(AUTH_APP_PREFIX);
                d_authOptions.append(argv[i] + strlen(AUTH_OPTION_APP));
            } else if (!strcmp(argv[i], AUTH_OPTION_USER_APP)) {
                d_authOptions.clear();
                d_authOptions.append(AUTH_USER_APP_PREFIX);
                d_authOptions.append(argv[i] + strlen(AUTH_OPTION_USER_APP));
            } else if (!strcmp(argv[i], AUTH_OPTION_DIR)) {
                d_authOptions.clear();
                d_authOptions.append(AUTH_DIR_PREFIX);
                d_authOptions.append(argv[i] + strlen(AUTH_OPTION_DIR));
            } else if (!std::strcmp(argv[i], AUTH_OPTION_USER)) {
                d_authOptions.assign(AUTH_USER);
            } else {
                printUsage();
                return false;
            }
        } else {
            printUsage();
            std::cerr << "\nUnexpected option: '" << argv[i] << "'\n\n";
            return false;
        }
    }

    if (d_hosts.empty()) {
        d_hosts.push_back("localhost");
    }

    if (d_topics.empty()) {
        d_topics.push_back("/ticker/IBM US Equity");
    }

    if (d_fields.empty()) {
        d_fields.emplace_back("LAST_PRICE");
    }

    return true;
}
