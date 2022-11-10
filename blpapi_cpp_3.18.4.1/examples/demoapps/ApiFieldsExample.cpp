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
#include <blpapi_event.h>
#include <blpapi_exception.h>
#include <blpapi_names.h>
#include <blpapi_request.h>
#include <blpapi_session.h>

#include <exception>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <string>

#include <snippets/apiflds/CategorizedFieldSearchRequests.h>
#include <snippets/apiflds/FieldInfoRequests.h>
#include <snippets/apiflds/FieldListRequests.h>
#include <snippets/apiflds/FieldSearchRequests.h>
#include <util/ArgParser.h>
#include <util/ConnectionAndAuthOptions.h>

using namespace BloombergLP;
using namespace blpapi;

namespace {
const char *CATEGORIZED_FIELD_SEARCH_REQUEST("CategorizedFieldSearchRequest");
const char *FIELD_INFO_REQUEST("FieldInfoRequest");
const char *FIELD_LIST_REQUEST("FieldListRequest");
const char *FIELD_SEARCH_REQUEST("FieldSearchRequest");

const char *APIFLDS_SVC("//blp/apiflds");
}

class ApiFieldsExample {
    std::string d_requestType;

    Request createRequest(Session& session) const
    {
        Service apifldsService = session.getService(APIFLDS_SVC);
        const char *requestType = d_requestType.c_str();
        if (!std::strcmp(requestType, CATEGORIZED_FIELD_SEARCH_REQUEST)) {
            return CategorizedFieldSearchRequests::createRequest(
                    apifldsService);
        }

        if (!std::strcmp(requestType, FIELD_INFO_REQUEST)) {
            return FieldInfoRequests::createRequest(apifldsService);
        }

        if (!std::strcmp(requestType, FIELD_LIST_REQUEST)) {
            return FieldListRequests::createRequest(apifldsService);
        }

        if (!std::strcmp(requestType, FIELD_SEARCH_REQUEST)) {
            return FieldSearchRequests::createRequest(apifldsService);
        }

        throw std::invalid_argument("Unknown request type: " + d_requestType);
    }

    // This function processes the contents of a 'RESPONSE' or
    // 'PARTIAL_RESPONSE' to an apiflds request.
    void processResponse(const Event& event) const
    {
        const char *requestType = d_requestType.c_str();
        if (!std::strcmp(requestType, CATEGORIZED_FIELD_SEARCH_REQUEST)) {
            return CategorizedFieldSearchRequests::processResponse(event);
        }

        if (!std::strcmp(requestType, FIELD_INFO_REQUEST)) {
            return FieldInfoRequests::processResponse(event);
        }

        if (!std::strcmp(requestType, FIELD_LIST_REQUEST)) {
            return FieldListRequests::processResponse(event);
        }

        if (!std::strcmp(requestType, FIELD_SEARCH_REQUEST)) {
            return FieldSearchRequests::processResponse(event);
        }

        throw std::invalid_argument("Unknown request type: " + d_requestType);
    }

  public:
    int run(int argc, const char **argv)
    {
        ArgParser argParser("Find API Data Fields", "ApiFieldsExample");
        SessionOptions sessionOptions;

        try {
            ConnectionAndAuthOptions connectionAndAuthOptions(argParser);
            argParser.addArg("request", 'r')
                    .setMetaVar("requestType")
                    .setDescription("Specify API fields request type")
                    .setIsRequired(true)
                    .setChoices({ CATEGORIZED_FIELD_SEARCH_REQUEST,
                            FIELD_INFO_REQUEST,
                            FIELD_LIST_REQUEST,
                            FIELD_SEARCH_REQUEST })
                    .setAction([this](const char *value) {
                        d_requestType = value;
                    });
            argParser.parse(argc, argv);
            sessionOptions = connectionAndAuthOptions.createSessionOption();
        } catch (std::exception& ex) {
            std::cerr << ex.what() << "\n";
            argParser.printHelp();
            return 1;
        }

        Session session(sessionOptions);
        if (!session.start()) {
            std::cerr << "Failed to start session.\n";
            return 1;
        }

        if (!session.openService(APIFLDS_SVC)) {
            std::cerr << "Failed to open " << APIFLDS_SVC << "\n";
            session.stop();
            return 1;
        }

        Request request = createRequest(session);
        std::cout << "Sending Request: ";
        request.print(std::cout) << "\n";
        session.sendRequest(request);

        bool done = false;
        while (!done) {
            Event event = session.nextEvent();
            const Event::EventType eventType = event.eventType();

            if (eventType == Event::REQUEST_STATUS) {
                MessageIterator msgIter(event);
                while (msgIter.next()) {
                    Message msg = msgIter.message();
                    if (msg.messageType() == Names::requestFailure()) {
                        // Request has failed, exit
                        msg.print(std::cout);
                        done = true;
                        break;
                    }
                }
            } else if (eventType == Event::RESPONSE
                    || eventType == Event::PARTIAL_RESPONSE) {
                processResponse(event);

                // Received the final response, no further response events are
                // expected.
                if (eventType == Event::RESPONSE) {
                    done = true;
                }
            }
        }

        session.stop();
        return 0;
    }
};

int main(int argc, const char **argv)
{
    try {
        ApiFieldsExample example;
        return example.run(argc, argv);
    } catch (std::exception& e) {
        std::cerr << "!!!Exception: " << e.what() << "\n";
        return 1;
    }
}
