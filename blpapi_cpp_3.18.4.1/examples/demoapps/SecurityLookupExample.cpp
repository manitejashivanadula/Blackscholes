/* Copyright 2021, Bloomberg Finance L.P.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:  The above
 * copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.  THE SOFTWARE IS PROVIDED "AS IS",
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <blpapi_element.h>
#include <blpapi_event.h>
#include <blpapi_exception.h>
#include <blpapi_message.h>
#include <blpapi_name.h>
#include <blpapi_request.h>
#include <blpapi_session.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <util/ArgParser.h>
#include <util/ConnectionAndAuthOptions.h>

#include <snippets/instruments/CurveListRequests.h>
#include <snippets/instruments/GovtListRequests.h>
#include <snippets/instruments/InstrumentListRequests.h>

using namespace BloombergLP;
using namespace blpapi;

namespace {
const Name REASON("reason");
const Name DESCRIPTION_ELEMENT("description");

const char *INSTRUMENTS_SERVICE("//blp/instruments");

const std::string INSTRUMENT_LIST_REQUEST("instrumentListRequest");
const std::string CURVE_LIST_REQUEST("curveListRequest");
const std::string GOVT_LIST_REQUEST("govtListRequest");

const Name ERROR_RESPONSE("ErrorResponse");
const Name INSTRUMENT_LIST_RESPONSE("InstrumentListResponse");
const Name CURVE_LIST_RESPONSE("CurveListResponse");
const Name GOVT_LIST_RESPONSE("GovtListResponse");

const char *DEFAULT_MAX_RESULTS("10");
const char *DEFAULT_QUERY_STRING("IBM");
}

class SecurityLookupExample {
    using Filters = std::vector<std::tuple<std::string, std::string>>;

    int d_maxResults;
    std::string d_requestType;
    Filters d_filters;
    std::string d_query;
    ArgParser d_argParser;
    ConnectionAndAuthOptions d_connectionAndAuthOptions;
    const std::vector<std::string> d_filtersInstruments;
    const std::vector<std::string> d_filtersGovt;
    const std::vector<std::string> d_filtersCurve;

    void processResponseEvent(const Event& event)
    {
        MessageIterator msgIter(event);
        while (msgIter.next()) {
            Message msg = msgIter.message();
            if (msg.messageType() == ERROR_RESPONSE) {
                std::string description
                        = msg.getElementAsString(DESCRIPTION_ELEMENT);
                std::cerr << "Received error: " << description << "\n";
            } else if (msg.messageType() == INSTRUMENT_LIST_RESPONSE) {
                InstrumentListRequests::processResponse(msg);
            } else if (msg.messageType() == CURVE_LIST_RESPONSE) {
                CurveListRequests::processResponse(msg);
            } else if (msg.messageType() == GOVT_LIST_RESPONSE) {
                GovtListRequests::processResponse(msg);
            } else {
                std::cerr << "Unexpected response: " << msg << "\n";
            }
        }
    }

    Request createRequest(Session& session)
    {
        const Service instrumentsService
                = session.getService(INSTRUMENTS_SERVICE);
        if (INSTRUMENT_LIST_REQUEST == d_requestType) {
            return InstrumentListRequests::createRequest(
                    instrumentsService, d_query, d_maxResults, d_filters);
        }

        if (CURVE_LIST_REQUEST == d_requestType) {
            return CurveListRequests::createRequest(
                    instrumentsService, d_query, d_maxResults, d_filters);
        }

        if (GOVT_LIST_REQUEST == d_requestType) {
            return GovtListRequests::createRequest(
                    instrumentsService, d_query, d_maxResults, d_filters);
        }

        throw std::invalid_argument("Unknown request " + d_requestType);
    }

  public:
    SecurityLookupExample()
        : d_maxResults()
        , d_requestType(INSTRUMENT_LIST_REQUEST)
        , d_query(DEFAULT_QUERY_STRING)
        , d_argParser("Security Lookup Example", "SecurityLookupExample")
        , d_connectionAndAuthOptions(d_argParser)
        , d_filtersInstruments({ "yellowKeyFilter", "languageOverride" })
        , d_filtersGovt({ "ticker", "partialMatch" })
        , d_filtersCurve({ "countryCode",
                  "currencyCode",
                  "type",
                  "subtype",
                  "curveid",
                  "bbgid" })
    {
    }

    bool parseCommandLine(int argc, const char **argv)
    {
        try {
            ArgGroup& argGroupLookup
                    = d_argParser.addGroup("Security Lookup Options");

            argGroupLookup.addArg("request", 'r')
                    .setMetaVar("requestType")
                    .setDescription("specify the request type")
                    .setDefaultValue(INSTRUMENT_LIST_REQUEST)
                    .setChoices({ INSTRUMENT_LIST_REQUEST,
                            CURVE_LIST_REQUEST,
                            GOVT_LIST_REQUEST })
                    .setAction([this](const char *value) {
                        d_requestType = value;
                    });

            argGroupLookup.addArg("security", 'S')
                    .setMetaVar("security")
                    .setDescription("security query string")
                    .setDefaultValue(DEFAULT_QUERY_STRING)
                    .setAction([this](const char *value) { d_query = value; });

            argGroupLookup.addArg("max-results")
                    .setMetaVar("maxResults")
                    .setDescription("max results returned in the response")
                    .setDefaultValue(DEFAULT_MAX_RESULTS)
                    .setAction([this](const char *value) {
                        d_maxResults = std::stoi(value);
                    });

            std::string description;
            description.append(
                    std::string("filter and value separated by '=', e.g., ")
                    + "countryCode=US\n"
                    + "The applicable filters for each request:\n"
                    + INSTRUMENT_LIST_REQUEST + ": "
                    + Utils::join(d_filtersInstruments, ", ") + "\n"
                    + CURVE_LIST_REQUEST + ": "
                    + Utils::join(d_filtersCurve, ", ") + "\n"
                    + GOVT_LIST_REQUEST + ": "
                    + Utils::join(d_filtersGovt, ", "));

            argGroupLookup.addArg("filter", 'F')
                    .setMetaVar("<filter>=<value>")
                    .setMode(ArgMode::MULTIPLE_VALUES)
                    .setDescription(description)
                    .setAction([this](const char *value) {
                        std::string s(value);
                        std::vector<std::string> tokens = Utils::split(s, '=');
                        if (tokens.size() != 2) {
                            throw std::invalid_argument(
                                    std::string("Invalid filter option ")
                                    + value);
                        }

                        d_filters.push_back(
                                std::make_tuple(tokens[0], tokens[1]));
                    });

            d_argParser.parse(argc, argv);
        } catch (const std::exception& ex) {
            std::cerr << "Failed to parse arguments: " << ex.what() << "\n";
            d_argParser.printHelp();
            return false;
        }

        return true;
    }

    // After we have sent the request we want to wait for the response.
    // Success response can come with a number of 'PARTIAL_RESPONSE' events
    // followed by a 'RESPONSE' event.  Failures will be delivered in a
    // 'REQUEST_STATUS' event holding a 'RequestFailure' message.
    void waitForResponse(Session& session)
    {
        bool done = false;
        while (!done) {
            Event event = session.nextEvent();
            if (event.eventType() == Event::PARTIAL_RESPONSE) {
                std::cout << "\nProcessing Partial Response\n";
                processResponseEvent(event);
            } else if (event.eventType() == Event::RESPONSE) {
                std::cout << "\nProcessing Response\n";
                processResponseEvent(event);
                done = true;
            } else if (event.eventType() == Event::REQUEST_STATUS) {
                MessageIterator msgIter(event);
                while (msgIter.next()) {
                    Message msg = msgIter.message();
                    if (msg.messageType() == Names::requestFailure()) {
                        std::cout
                                << "REQUEST FAILED: " << msg.getElement(REASON)
                                << "\n";
                        // Report the request id to contact support if needed.
                        Utils::printContactSupportMessage(msg);
                        done = true;
                    }
                }
            } else {
                // 'SESSION_STATUS' events can happen at any time and need to
                // be handled as the session can be terminated.
                done = !Utils::processGenericEvent(event);
            }
        }
    }

    void run(int argc, const char **argv)
    {
        if (!parseCommandLine(argc, argv)) {
            return;
        }

        Session session(d_connectionAndAuthOptions.createSessionOption());

        if (!session.start()) {
            std::cout << "Failed to start session\n";
            Utils::checkFailures(session);
            return;
        }

        if (!session.openService(INSTRUMENTS_SERVICE)) {
            std::cout << ">>> Failed to open " << INSTRUMENTS_SERVICE << "\n";
            session.stop();
            return;
        }

        const Request request = createRequest(session);
        std::cout << "Sending request: " << request << "\n";
        session.sendRequest(request, nullptr /* CorrelationID*/);
        waitForResponse(session);
        session.stop();
    }
};

int main(int argc, const char **argv)
{
    SecurityLookupExample example;
    try {
        example.run(argc, argv);
    } catch (Exception& e) {
        std::cerr << ">>> Exception caught: " << e.description() << "\n";
    }

    std::cout << "Press ENTER to quit\n";
    char dummy[2];
    std::cin.getline(dummy, 2);
    return 0;
}
