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
#include <blpapi_eventformatter.h>
#include <blpapi_exception.h>
#include <blpapi_message.h>
#include <blpapi_name.h>
#include <blpapi_names.h>
#include <blpapi_providersession.h>
#include <blpapi_topic.h>
#include <blpapi_topiclist.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <util/ArgParser.h>
#include <util/ConnectionAndAuthOptions.h>
#include <util/MaxEventsOption.h>

using namespace BloombergLP;
using namespace blpapi;

namespace {

const char *DEFAULT_MARKET_DATA_TOPIC = "IBM Equity";
const char *DEFAULT_PAGE_TOPIC = "1245/4/5";

bool g_running = true;
std::mutex g_lock;

void formatMarketData(EventFormatter *eventFormatter, const Topic& topic)
{
    const static Name eventName("MarketDataEvents");
    const static Name high("HIGH");
    const static Name low("LOW");

    eventFormatter->appendMessage(eventName, topic);

    const int seconds = 10;
    eventFormatter->setElement(high, seconds * 1.0);
    eventFormatter->setElement(low, seconds * 0.5);
}

void formatPageData(EventFormatter *eventFormatter, const Topic& topic)
{
    eventFormatter->appendMessage("RowUpdate", topic);
    eventFormatter->setElement("rowNum", 1);
    eventFormatter->pushElement("spanUpdate");
    eventFormatter->appendElement();
    eventFormatter->setElement("startCol", 1);

    std::string text = "May 21st 2021";
    eventFormatter->setElement("length", static_cast<Int32>(text.length()));
    eventFormatter->setElement("text", text.c_str());
    eventFormatter->popElement();
    eventFormatter->appendElement();
    eventFormatter->setElement(
            "startCol", static_cast<Int32>(text.length() + 10));

    text = "May 22st 2021";
    eventFormatter->setElement("length", static_cast<Int32>(text.length()));
    eventFormatter->setElement("text", text.c_str());
    eventFormatter->popElement();
    eventFormatter->popElement();
}
}

class MyStream {
    std::string d_id;
    Topic d_topic;

  public:
    MyStream(std::string const& id)
        : d_id(id)
    {
    }
    void setTopic(Topic const& topic) { d_topic = topic; }
    std::string const& getId() const { return d_id; }
    Topic const& getTopic() const { return d_topic; }
};

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
                    std::lock_guard<std::mutex> lg(g_lock);
                    std::cout
                            << "Session terminated. Stopping application..\n";
                    g_running = false;
                }
            }
        }
        return true;
    }
};

class BroadcastPublisherExample {
    ArgParser d_argParser;
    ConnectionAndAuthOptions d_connectionAndAuthOptions;
    std::string d_service;
    std::vector<std::string> d_topics;
    std::string d_groupId;
    bool d_isPageData;
    int d_priority;
    MaxEventsOption d_maxEventsOption;

    bool parseCommandLine(int argc, const char **argv)
    {
        try {
            ArgGroup& argGroupPublisher
                    = d_argParser.addGroup("Broadcast Publisher Options");
            argGroupPublisher.addArg("service", 's')
                    .setDescription("the service name")
                    .setMetaVar("service")
                    .setIsRequired(true)
                    .setAction(
                            [this](const char *value) { d_service = value; });

            argGroupPublisher.addArg("topic", 't')
                    .setDescription(
                            std::string(
                                    "topic to publish (default: mktdata \"")
                            + DEFAULT_MARKET_DATA_TOPIC + "\", page \""
                            + DEFAULT_PAGE_TOPIC + "\")")
                    .setMode(ArgMode::MULTIPLE_VALUES)
                    .setMetaVar("topic")
                    .setAction([this](const char *value) {
                        d_topics.emplace_back(value);
                    });

            argGroupPublisher.addArg("group-id", 'g')
                    .setDescription("publisher's group ID, default to an "
                                    "automatically generated unique value")
                    .setMetaVar("groupId")
                    .setAction(
                            [this](const char *value) { d_groupId = value; });

            argGroupPublisher.addArg("priority", 'p')
                    .setDescription("publisher's priority")
                    .setMetaVar("priority")
                    .setDefaultValue(std::to_string(
                            ServiceRegistrationOptions::PRIORITY_HIGH))
                    .setAction([this](const char *value) {
                        d_priority = std::atoi(value);
                    });

            argGroupPublisher.addArg("page", 'P')
                    .setDescription("enable publish as page")
                    .setMode(ArgMode::NO_VALUE)
                    .setAction([this](const char *value) {
                        d_isPageData = true;
                    });

            d_argParser.parse(argc, argv);
        } catch (std::exception& ex) {
            std::cerr << ex.what() << "\n";
            d_argParser.printHelp();
            return false;
        }

        if (d_topics.empty()) {
            d_topics.emplace_back(d_isPageData ? DEFAULT_PAGE_TOPIC
                                               : DEFAULT_MARKET_DATA_TOPIC);
        }

        return true;
    }

  public:
    BroadcastPublisherExample()
        : d_argParser(
                "Broadcast publisher example", "BroadcastPublisherExample")
        , d_connectionAndAuthOptions(d_argParser)
        , d_isPageData(false)
        , d_priority(ServiceRegistrationOptions::PRIORITY_HIGH)
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
            std::cerr << "Failed to start session."
                      << "\n";
            return;
        }

        if (!d_groupId.empty()) {
            // NOTE: perform explicit service registration here, instead of
            // letting createTopics() do it, as the latter approach doesn't
            // allow for custom ServiceRegistrationOptions
            ServiceRegistrationOptions serviceOptions;
            serviceOptions.setGroupId(d_groupId.c_str(),
                    static_cast<unsigned>(d_groupId.size()));
            serviceOptions.setServicePriority(d_priority);
            if (!session.registerService(d_service.c_str(),
                        session.getAuthorizedIdentity(),
                        serviceOptions)) {
                std::cerr << "Failed to register " << d_service << "\n";
                session.stop();
                return;
            }
        }

        TopicList topicList;
        for (const auto& topic : d_topics) {
            std::string userTopic(topic);
            if (!userTopic.empty() && userTopic[0] != '/') {
                userTopic = "/" + userTopic;
            }

            topicList.add((d_service + userTopic).c_str(),
                    CorrelationId(new MyStream(topic)));
        }

        // createTopics() is synchronous, topicList will be updated with the
        // results of topic creation (resolution will happen under the covers)
        session.createTopics(
                &topicList, ProviderSession::AUTO_REGISTER_SERVICES);

        Service service = session.getService(d_service.c_str());
        if (!service.isValid()) {
            std::cout << "Service registration failed: " << d_service << "\n";
            session.stop();
            return;
        }

        std::vector<MyStream *> myStreams;

        for (size_t i = 0; i < topicList.size(); ++i) {
            MyStream *stream = static_cast<MyStream *>(
                    topicList.correlationIdAt(i).asPointer());
            int status = topicList.statusAt(i);
            if (status == TopicList::CREATED) {
                std::cout << "Start publishing on topic: "
                          << topicList.topicStringAt(i) << "\n";
                Topic topic = session.getTopic(topicList.messageAt(i));
                stream->setTopic(topic);
                myStreams.push_back(stream);
            } else {
                std::cout << "Stream '" << stream->getId()
                          << "': topic not created, status = " << status
                          << "\n";
            }
        }

        if (myStreams.empty()) {
            std::cout << "No topics created for publishing\n";
            session.stop();
            return;
        }

        // Now we will start publishing
        for (int eventCount = 0; eventCount < d_maxEventsOption.getMaxEvents();
                ++eventCount) {
            {
                std::lock_guard<std::mutex> lg(g_lock);
                if (!g_running) {
                    break;
                }
            }

            Event event = service.createPublishEvent();
            EventFormatter eventFormatter(event);

            for (const auto *myStream : myStreams) {
                const Topic& topic = myStream->getTopic();
                if (d_isPageData) {
                    formatPageData(&eventFormatter, topic);
                } else {
                    formatMarketData(&eventFormatter, topic);
                }
            }

            std::cout << "Publishing event: ";
            Utils::printEvent(event);

            session.publish(event);
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }

        session.stop();
    }
};

int main(int argc, const char **argv)
{
    BroadcastPublisherExample example;
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
