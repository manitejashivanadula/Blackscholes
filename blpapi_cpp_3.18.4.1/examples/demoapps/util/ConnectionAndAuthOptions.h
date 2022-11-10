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

#ifndef INCLUDED_CONNECTION_AND_AUTH_OPTIONS
#define INCLUDED_CONNECTION_AND_AUTH_OPTIONS

// @PURPOSE:
//     Helper class to add options for connection
//     and authorization.
//
// @CLASSES:
//    ConnectionAndAuthOptions: Connection and auth options definition.
//
// @DESCRIPTION:
//     Helper class that adds the options for connection and
//     authorization to ArgParser. It creates a SessionOptions
//     using the following command line arguments:
//
//     - A connections where servers, TLS and ZFP over
//       Leased lines are specified.
//     - Authorization options that is used as
//       session identity options.

#include <blpapi_sessionoptions.h>
#include <blpapi_zfputil.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include <util/ArgParser.h>
#include <util/Utils.h>

namespace BloombergLP {

struct ServerAddress {
    ServerAddress(const std::string& host, unsigned short port)
        : d_host(host)
        , d_port(port)
    {
    }

    std::string d_host;
    unsigned short d_port;
};

class ConnectionAndAuthOptions {
  public:
    ConnectionAndAuthOptions(
            ArgParser& argParser, bool isClientServerSetup = false);

    blpapi::SessionOptions createSessionOption() const;

    std::map<std::string, blpapi::AuthOptions>
    createClientServerSetupAuthOptions();

    int numClientServerSetupAuthOptions() const;

  private:
    std::vector<ServerAddress> d_servers;

    std::vector<std::tuple<std::string, std::string>> d_userIdAndIps;

    std::vector<std::string> d_tokens;

    blpapi::AuthOptions d_sessionIdentityAuthOptions;

    std::unique_ptr<blpapi::AuthApplication> d_authApplication;

    std::string d_clientCredentials;

    std::string d_clientCredentialsPassword;

    std::string d_trustMaterial;

    blpapi::ZfpUtil::Remote d_remote = blpapi::ZfpUtil::REMOTE_8194;

    bool d_readTlsData = false;

    bool d_zfpOverLeasedLine = false;

    bool d_clientServerSetup = false;

    void addArgGroupServer(ArgParser& argParser);

    void addArgGroupAuth(ArgParser& argParser);

    void addArgGroupTls(ArgParser& argParser);

    void addArgGroupZfpLeasedLine(ArgParser& argParser);

    void parseServerAddress(const char *address);

    void parseRemote(const char *port);

    void parseAuthOptions(const char *authOption);

    blpapi::TlsOptions createTlsOptions() const;

    blpapi::SessionOptions prepareStandardSessionOptions() const;
    // Prepares 'sessionOptions' for connections other than ZFP Leased
    // Line.

    blpapi::SessionOptions prepareZfpSessionOptions() const;
    // Prepares 'sessionOptions' for ZFP Leased Line connections.

    static int readFromFile(const std::string& filename, std::string& buffer);

    void addArgGroupAuthClientServerSetup(ArgParser&);

    void parseUserIdIp(const char *);
};

inline ConnectionAndAuthOptions::ConnectionAndAuthOptions(
        ArgParser& argParser, bool isClientServerSetup)
    : d_clientServerSetup(isClientServerSetup)
{
    addArgGroupServer(argParser);
    isClientServerSetup ? addArgGroupAuthClientServerSetup(argParser)
                        : addArgGroupAuth(argParser);
    addArgGroupTls(argParser);
    addArgGroupZfpLeasedLine(argParser);
}

inline std::map<std::string, blpapi::AuthOptions>
ConnectionAndAuthOptions::createClientServerSetupAuthOptions()
{
    std::map<std::string, blpapi::AuthOptions> authOptionsByIdentifier;
    for (const auto& userIdIp : d_userIdAndIps) {
        std::string userId = std::get<0>(userIdIp);
        std::string ip = std::get<1>(userIdIp);

        blpapi::AuthUser authUser = blpapi::AuthUser::createWithManualOptions(
                userId.c_str(), ip.c_str());

        blpapi::AuthOptions authOptions(authUser, *d_authApplication);
        authOptionsByIdentifier[userId + ":" + ip] = authOptions;
    }

    for (unsigned i = 0; i < d_tokens.size(); ++i) {
        blpapi::AuthOptions authOptions(
                blpapi::AuthToken(d_tokens[i].c_str()));

        authOptionsByIdentifier[std::string("token #") + std::to_string(i + 1)]
                = authOptions;
    }

    return authOptionsByIdentifier;
}

inline int ConnectionAndAuthOptions::numClientServerSetupAuthOptions() const
{
    return d_userIdAndIps.size() + d_tokens.size();
}

inline blpapi::SessionOptions
ConnectionAndAuthOptions::createSessionOption() const
{
    if (d_zfpOverLeasedLine) {
        return prepareZfpSessionOptions();
    }

    return prepareStandardSessionOptions();
}

inline blpapi::TlsOptions ConnectionAndAuthOptions::createTlsOptions() const
{
    blpapi::TlsOptions tlsOptions;

    if (d_readTlsData) {
        std::string clientCredentials;
        std::string trustMaterial;

        if (readFromFile(d_clientCredentials, clientCredentials)
                || readFromFile(d_trustMaterial, trustMaterial)) {
            throw std::invalid_argument(
                    "Failed to read TLS options from files");
        }

        tlsOptions
                = blpapi::TlsOptions::createFromBlobs(clientCredentials.data(),
                        static_cast<int>(clientCredentials.size()),
                        d_clientCredentialsPassword.c_str(),
                        trustMaterial.data(),
                        static_cast<int>(trustMaterial.size()));
    } else {
        tlsOptions = blpapi::TlsOptions::createFromFiles(
                d_clientCredentials.c_str(),
                d_clientCredentialsPassword.c_str(),
                d_trustMaterial.c_str());
    }

    return tlsOptions;
}

inline blpapi::SessionOptions
ConnectionAndAuthOptions::prepareStandardSessionOptions() const
{
    blpapi::SessionOptions sessionOptions;
    std::cout << "Connecting to: ";
    for (size_t i = 0; i < d_servers.size(); ++i) {
        std::cout << (i == 0 ? "" : ", ") << d_servers[i].d_host << ":"
                  << d_servers[i].d_port;

        sessionOptions.setServerAddress(
                d_servers[i].d_host.c_str(), d_servers[i].d_port, i);
    }
    std::cout << "\n";

    sessionOptions.setSessionIdentityOptions(d_sessionIdentityAuthOptions);

    if (!d_clientCredentials.empty() && !d_trustMaterial.empty()) {
        std::cout << "TlsOptions enabled\n";
        sessionOptions.setTlsOptions(createTlsOptions());
    }

    return sessionOptions;
}

inline blpapi::SessionOptions
ConnectionAndAuthOptions::prepareZfpSessionOptions() const
{
    if (d_clientCredentials.empty() || d_trustMaterial.empty()) {
        throw std::invalid_argument("ZFP connections require TLS parameters");
    }

    std::cout << "Creating a ZFP connection for leased lines\n";
    blpapi::SessionOptions sessionOptions
            = blpapi::ZfpUtil::getZfpOptionsForLeasedLines(
                    d_remote, createTlsOptions());
    sessionOptions.setSessionIdentityOptions(d_sessionIdentityAuthOptions);
    return sessionOptions;
}

inline void ConnectionAndAuthOptions::addArgGroupServer(ArgParser& argParser)
{
    ArgGroup& argGroupServer = argParser.addGroup("Connections");
    argGroupServer.addArg("host", 'H')
            .setMetaVar("host:port")
            .setDescription("Server name or IP and port separated by ':'")
            .setDefaultValue("localhost:8194")
            .setAction(
                    [this](const char *value) { parseServerAddress(value); })
            .setMode(ArgMode::MULTIPLE_VALUES); // allow multiple servers
}

inline void ConnectionAndAuthOptions::addArgGroupAuthClientServerSetup(
        ArgParser& argParser)
{
    ArgGroup& argGroupAuthorization = argParser.addGroup("Authorization");
    argGroupAuthorization.addArg("auth", 'a')
            .setMetaVar("app=<app>")
            .setDescription("authorize this application using the specified "
                            "application")
            .setIsRequired(true)
            .setAction([this](const char *value) { parseAuthOptions(value); });

    ArgGroup& argGroupEntitlements
            = argParser.addGroup("User Authorization/Entitlements");
    argGroupEntitlements.addArg("userid-ip", 'u')
            .setMetaVar("userId:IP")
            .setDescription(
                    "authorize a user using userId and IP separated by ':'")
            .setMode(ArgMode::MULTIPLE_VALUES)
            .setAction([this](const char *value) { parseUserIdIp(value); });

    argGroupEntitlements.addArg("token", 'T')
            .setMetaVar("token")
            .setDescription("authorize a user using the specified token")
            .setMode(ArgMode::MULTIPLE_VALUES)
            .setAction([this](const char *value) {
                d_tokens.emplace_back(value);
            });
}

inline void ConnectionAndAuthOptions::addArgGroupAuth(ArgParser& argParser)
{
    ArgGroup& argGroupAuth = argParser.addGroup("Authorization");

    static const char *AUTH_OPTION_NONE = "none";
    argGroupAuth.addArg("auth", 'a')
            .setMetaVar("option")
            .setDescription(R"(authorization option
none                  applicable to Desktop API product that requires
                          Bloomberg Professional service to be installed locally
user                  as a user using OS logon information
dir=<property>        as a user using directory services
app=<app>             as the specified application
userapp=<app>         as user and application using logon information for the user
manual=<app,ip,user>  as user and application, with manually provided
                          IP address and EMRS user)")
            .setDefaultValue(std::string(AUTH_OPTION_NONE))
            .setAction([this](const char *value) { parseAuthOptions(value); });
}

inline void ConnectionAndAuthOptions::addArgGroupTls(ArgParser& argParser)
{
    ArgGroup& argGroupTls = argParser.addGroup("TLS (specify all or none)");
    argGroupTls.addArg("tls-client-credentials")
            .setMetaVar("file")
            .setDescription("name a PKCS#12 file to use as a source of client "
                            "credentials")
            .setAction([this](const char *value) {
                d_clientCredentials = value;
            });

    argGroupTls.addArg("tls-client-credentials-password")
            .setMetaVar("password")
            .setDescription(
                    "specify password for accessing client credentials")
            .setAction([this](const char *value) {
                d_clientCredentialsPassword = value;
            });

    argGroupTls.addArg("tls-trust-material")
            .setMetaVar("file")
            .setDescription("name a PKCS#7 file to use as a source of trusted "
                            "certificates")
            .setAction([this](const char *value) { d_trustMaterial = value; });

    argGroupTls.addArg("read-certificate-files")
            .setDescription("read the TLS files and pass the blobs")
            .setMode(ArgMode::NO_VALUE)
            .setAction([this](const char *) { d_readTlsData = true; });
}

inline void ConnectionAndAuthOptions::addArgGroupZfpLeasedLine(
        ArgParser& argParser)
{
    ArgGroup& argGroupZfpLl = argParser.addGroup(
            "ZFP connections over leased lines (requires TLS)");
    argGroupZfpLl.addArg("zfp-over-leased-line")
            .setMetaVar("port")
            .setDescription(
                    R"(enable ZFP connections over leased lines on the specified port (8194 or 8196)
(When this option is enabled, option -H/--host is ignored.))")
            .setAction([this](const char *value) {
                parseRemote(value);
                d_zfpOverLeasedLine = true;
            });
}

inline void ConnectionAndAuthOptions::parseUserIdIp(const char *value)
{
    std::vector<std::string> tokens = Utils::split(value, ':');
    if (tokens.size() != 2) {
        throw std::invalid_argument(
                (std::string("Invalid userId:IP option: ") + value));
    }

    d_userIdAndIps.emplace_back(tokens[0], tokens[1]);
}

inline void ConnectionAndAuthOptions::parseServerAddress(const char *address)
{
    std::vector<std::string> tokens = Utils::split(address, ':');
    if (tokens.size() != 2) {
        throw std::invalid_argument(
                std::string("Invalid server option: ") + address);
    }

    int port = std::stoi(tokens[1]);
    d_servers.emplace_back(tokens[0], static_cast<unsigned short>(port));
}

inline void ConnectionAndAuthOptions::parseRemote(const char *port)
{
    if (!std::strcmp(port, "8194")) {
        d_remote = blpapi::ZfpUtil::Remote::REMOTE_8194;
    } else if (!std::strcmp(port, "8196")) {
        d_remote = blpapi::ZfpUtil::Remote::REMOTE_8196;
    } else {
        throw std::invalid_argument(std::string("Invalid ZFP port ") + port);
    }
}

inline void ConnectionAndAuthOptions::parseAuthOptions(const char *authOption)
{
    static const std::string AUTH_OPTION_NONE = "none";
    static const std::string AUTH_OPTION_USER = "user";
    static const std::string AUTH_OPTION_APP = "app";
    static const std::string AUTH_OPTION_USER_APP = "userapp";
    static const std::string AUTH_OPTION_DIR = "dir";
    static const std::string AUTH_OPTION_MANUAL = "manual";

    std::vector<std::string> tokens = Utils::split(authOption, '=');

    const std::string& authType = tokens[0];

    if (authType == AUTH_OPTION_NONE) {
        d_sessionIdentityAuthOptions = blpapi::AuthOptions();
    } else if (authType == AUTH_OPTION_USER) {
        d_sessionIdentityAuthOptions
                = blpapi::AuthOptions(blpapi::AuthUser::createWithLogonName());
    } else {
        if (tokens.size() == 1) {
            throw std::invalid_argument(
                    std::string("Invalid auth option ") + authOption);
        }

        if (authType == AUTH_OPTION_APP) {
            // Save AuthApplication for client server setup
            d_authApplication.reset(
                    new blpapi::AuthApplication(tokens[1].c_str()));

            d_sessionIdentityAuthOptions
                    = blpapi::AuthOptions(*d_authApplication);
        } else if (authType == AUTH_OPTION_USER_APP) {
            d_sessionIdentityAuthOptions = blpapi::AuthOptions(
                    blpapi::AuthUser::createWithLogonName(),
                    blpapi::AuthApplication(tokens[1].c_str()));
        } else if (authType == AUTH_OPTION_DIR) {
            d_sessionIdentityAuthOptions = blpapi::AuthOptions(
                    blpapi::AuthUser::createWithActiveDirectoryProperty(
                            tokens[1].c_str()));
        } else if (authType == AUTH_OPTION_MANUAL) {
            std::vector<std::string> params = Utils::split(tokens[1], ',');

            if (params.size() != 3) {
                throw std::invalid_argument(
                        "auth " + authType + " is missing values");
            }

            const std::string& appName = params[0];
            const std::string& ip = params[1];
            const std::string& userId = params[2];
            d_sessionIdentityAuthOptions = blpapi::AuthOptions(
                    blpapi::AuthUser::createWithManualOptions(
                            userId.c_str(), ip.c_str()),
                    blpapi::AuthApplication(appName.c_str()));
        } else {
            throw std::invalid_argument("Wrong auth option " + authType);
        }
    }

    if (d_clientServerSetup && !d_authApplication) {
        throw std::invalid_argument(
                (std::string("Invalid auth option ") + authOption));
    }
}

inline int ConnectionAndAuthOptions::readFromFile(
        const std::string& filename, std::string& buffer)
{
    std::ifstream in(
            filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
    if (in) {
        buffer.resize(static_cast<size_t>(in.tellg()));
        in.seekg(0, std::ios::beg);
        in.read(&buffer[0], buffer.size());
    }

    if (in.fail()) {
        std::cerr << "Failed to read file from " << filename << "\n";
        return 1;
    }

    std::cout << "Read " << buffer.size() << " bytes from " << filename
              << "\n";
    return 0;
}

} // Close namespace BloombergLP

#endif // #ifndef INCLUDED_CONNECTION_AND_AUTH_OPTIONS
