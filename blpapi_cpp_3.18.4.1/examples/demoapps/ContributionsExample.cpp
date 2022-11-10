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
#include <blpapi_eventformatter.h>
#include <blpapi_exception.h>
#include <blpapi_message.h>
#include <blpapi_name.h>
#include <blpapi_names.h>
#include <blpapi_providersession.h>
#include <blpapi_topic.h>
#include <blpapi_topiclist.h>

#include <cassert>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#include <util/ArgParser.h>
#include <util/ConnectionAndAuthOptions.h>
#include <util/MaxEventsOption.h>

using namespace BloombergLP;
using namespace blpapi;

namespace {
const Name k_MarketData { "MarketData" };
const Name k_PageData { "PageData" };
const Name k_Bid { "BID" };
const Name k_Ask { "ASK" };
const Name k_RowUpdate { "rowUpdate" };
const Name k_RowNum { "rowNum" };
const Name k_SpanUpdate { "spanUpdate" };
const Name k_StartCol { "startCol" };
const Name k_Length { "length" };
const Name k_Text { "text" };
const Name k_Attr { "attr" };
const Name k_ContributorId { "contributorId" };
const Name k_ProductCode { "productCode" };
const Name k_PageNumber { "pageNumber" };

const char *k_DefaultMarketDataTopic { "/ticker/AUDEUR Curncy" };
const char *k_DefaultPageTopic { "/page/220/660/1" };

bool g_running = true;
std::mutex g_lock;
}

class MyEventHandler : public ProviderEventHandler {
  public:
    bool processEvent(const Event& event, ProviderSession *session) override
    {
        MessageIterator iter(event);
        while (iter.next()) {
            Message msg = iter.message();
            msg.print(std::cout) << "\n";
            if (event.eventType() == Event::SESSION_STATUS) {
                if (msg.messageType() == Names::sessionTerminated()) {
                    std::cout
                            << "Session terminated. Stopping application...\n";
                    std::lock_guard<std::mutex> lg(g_lock);
                    g_running = false;
                    break;
                }
            }
        }

        return true;
    }
};

class ContributionsExample {
    ArgParser d_argParser;
    ConnectionAndAuthOptions d_connectionAndAuthOptions;
    std::string d_service;
    std::string d_topic;
    bool d_pageEnabled;
    int d_contributorId;
    MaxEventsOption d_maxEventsOption;

    bool parseCommandLine(int argc, const char **argv)
    {
        try {
            ArgGroup& argGroupPublisher
                    = d_argParser.addGroup("Contribution Options");
            argGroupPublisher.addArg("service", 's')
                    .setDescription("service name")
                    .setMetaVar("service")
                    .setDefaultValue("//blp/mpfbapi")
                    .setAction(
                            [this](const char *value) { d_service = value; });

            argGroupPublisher.addArg("topic", 't')
                    .setDescription(std::string("topic to contribute "
                                                "(default: mktdata '")
                            + k_DefaultMarketDataTopic + "', page '"
                            + k_DefaultPageTopic + "'")
                    .setMetaVar("topic")
                    .setAction([this](const char *value) { d_topic = value; });

            argGroupPublisher.addArg("contributor-id", 'C')
                    .setDescription(
                            "contributor id, ignored unless page is enabled")
                    .setMetaVar("contributorId")
                    .setDefaultValue("8563")
                    .setAction([this](const char *value) {
                        d_contributorId = std::stoi(value);
                    });

            argGroupPublisher.addArg("page", 'P')
                    .setDescription("enable contribution as page")
                    .setMode(ArgMode::NO_VALUE)
                    .setAction([this](const char *value) {
                        d_pageEnabled = true;
                    });

            d_argParser.parse(argc, argv);
        } catch (const std::exception& ex) {
            std::cerr << ex.what() << "\n";
            d_argParser.printHelp();
            return false;
        }

        if (d_topic.empty()) {
            d_topic = d_pageEnabled ? k_DefaultPageTopic
                                    : k_DefaultMarketDataTopic;
        }

        return true;
    }

  public:
    ContributionsExample()
        : d_argParser("Contribution example", "ContributionExample")
        , d_connectionAndAuthOptions(d_argParser)
        , d_pageEnabled(false)
        , d_contributorId(8563)
        , d_maxEventsOption(d_argParser)
    {
    }

    void run(int argc, const char **argv)
    {
        if (!parseCommandLine(argc, argv)) {
            return;
        }

        SessionOptions sessionOptions
                = d_connectionAndAuthOptions.createSessionOption();

        MyEventHandler myEventHandler;
        ProviderSession session(sessionOptions, &myEventHandler);

        if (!session.start()) {
            std::cerr << "Failed to start session.\n";
            return;
        }

        TopicList topicList;
        topicList.add((d_service + d_topic).c_str(),
                CorrelationId(const_cast<char *>(d_topic.c_str())));

        // createTopics() is synchronous, topicList will be updated with the
        // results of topic creation (resolution will happen under the cover.)
        session.createTopics(
                &topicList, ProviderSession::AUTO_REGISTER_SERVICES);

        Service service = session.getService(d_service.c_str());
        if (!service.isValid()) {
            std::cout << "Service registration failed: " << d_service << "\n";
            session.stop();
            return;
        }

        const int resolutionStatus = topicList.statusAt(0);
        if (resolutionStatus != TopicList::CREATED) {
            std::cout << d_topic
                      << ": topic not resolved, status = " << resolutionStatus
                      << "\n";
            session.stop();
            return;
        }

        // Now we will start publishing
        const Topic topic = session.getTopic(topicList.messageAt(0));
        int value = 1;
        for (int i = 0; i < d_maxEventsOption.getMaxEvents(); ++i) {
            {
                std::lock_guard<std::mutex> lg(g_lock);
                if (!g_running) {
                    break;
                }
            }

            Event event = service.createPublishEvent();
            EventFormatter eventFormatter(event);

            if (d_pageEnabled) {
                formatPageDataEvent(&eventFormatter, topic);
            } else {
                formatMktDataEvent(&eventFormatter, topic, ++value);
            }

            std::cout << "Publishing event: ";
            Utils::printEvent(event);

            session.publish(event);
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }

        session.stop();
    }

    static void formatMktDataEvent(
            EventFormatter *eventFormatter, const Topic& topic, int value)
    {
        assert(eventFormatter);

        eventFormatter->appendMessage(k_MarketData, topic);
        eventFormatter->setElement(k_Bid, 0.5 * value);
        eventFormatter->setElement(k_Ask, value);
    }

    void formatPageDataEvent(
            EventFormatter *eventFormatter, const Topic& topic) const
    {
        assert(eventFormatter);

        eventFormatter->appendMessage(k_PageData, topic);
        eventFormatter->pushElement(k_RowUpdate);

        eventFormatter->appendElement();
        eventFormatter->setElement(k_RowNum, 1);
        eventFormatter->pushElement(k_SpanUpdate);

        eventFormatter->appendElement();
        eventFormatter->setElement(k_StartCol, 20);
        eventFormatter->setElement(k_Length, 4);
        eventFormatter->setElement(k_Text, "TEST");
        eventFormatter->popElement();

        eventFormatter->appendElement();
        eventFormatter->setElement(k_StartCol, 25);
        eventFormatter->setElement(k_Length, 4);
        eventFormatter->setElement(k_Text, "PAGE");
        eventFormatter->popElement();

        char buffer[10];
        time_t rawtime;
        std::time(&rawtime);
        int length = static_cast<int>(
                std::strftime(buffer, 10, "%X", std::localtime(&rawtime)));
        eventFormatter->appendElement();
        eventFormatter->setElement(k_StartCol, 30);
        eventFormatter->setElement(k_Length, length);
        eventFormatter->setElement(k_Text, buffer);
        eventFormatter->setElement(k_Attr, "BLINK");
        eventFormatter->popElement();

        eventFormatter->popElement();
        eventFormatter->popElement();

        eventFormatter->appendElement();
        eventFormatter->setElement(k_RowNum, 2);
        eventFormatter->pushElement(k_SpanUpdate);
        eventFormatter->appendElement();
        eventFormatter->setElement(k_StartCol, 20);
        eventFormatter->setElement(k_Length, 9);
        eventFormatter->setElement(k_Text, "---------");
        eventFormatter->setElement(k_Attr, "UNDERLINE");
        eventFormatter->popElement();
        eventFormatter->popElement();
        eventFormatter->popElement();

        eventFormatter->appendElement();
        eventFormatter->setElement(k_RowNum, 3);
        eventFormatter->pushElement(k_SpanUpdate);
        eventFormatter->appendElement();
        eventFormatter->setElement(k_StartCol, 10);
        eventFormatter->setElement(k_Length, 9);
        eventFormatter->setElement(k_Text, "TEST LINE");
        eventFormatter->popElement();
        eventFormatter->appendElement();
        eventFormatter->setElement(k_StartCol, 23);
        eventFormatter->setElement(k_Length, 5);
        eventFormatter->setElement(k_Text, "THREE");
        eventFormatter->popElement();
        eventFormatter->popElement();
        eventFormatter->popElement();
        eventFormatter->popElement();

        eventFormatter->setElement(k_ContributorId, d_contributorId);
        eventFormatter->setElement(k_ProductCode, 1);
        eventFormatter->setElement(k_PageNumber, 1);
    }
};

int main(int argc, const char **argv)
{
    ContributionsExample example;
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
