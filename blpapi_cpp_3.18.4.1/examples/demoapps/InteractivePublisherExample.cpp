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
#include <blpapi_message.h>
#include <blpapi_name.h>
#include <blpapi_names.h>
#include <blpapi_providersession.h>
#include <blpapi_topic.h>
#include <blpapi_topiclist.h>

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include <util/ArgParser.h>
#include <util/ConnectionAndAuthOptions.h>
#include <util/Utils.h>

using namespace BloombergLP;
using namespace blpapi;

namespace {

Name TOPIC("topic");
Name TOPICS("topics");
Name RESOLVED_TOPIC("resolvedTopic");

bool g_running = true;
std::map<std::string, Topic> g_activeTopics;
std::mutex g_mutex;
std::condition_variable g_activeTopicsCondition;

void formatMarketDataRecapEvent(EventFormatter *eventFormatter)
{
    eventFormatter->setElement("OPEN", 100.0);
}

void formatPageRecapEvent(EventFormatter *eventFormatter)
{
    const int rowCount = 5;
    eventFormatter->setElement("numRows", rowCount);
    eventFormatter->setElement("numCols", 80);
    eventFormatter->pushElement("rowUpdate");

    for (unsigned i = 1; i < rowCount; ++i) {
        eventFormatter->appendElement();
        eventFormatter->setElement("rowNum", static_cast<Int32>(i));
        eventFormatter->pushElement("spanUpdate");
        eventFormatter->appendElement();
        eventFormatter->setElement("startCol", 1);
        eventFormatter->setElement("length", 10);
        eventFormatter->setElement("text", "RECAP");
        eventFormatter->popElement();
        eventFormatter->popElement();
        eventFormatter->popElement();
    }

    eventFormatter->popElement();
}

void formatMarketDataEvent(
        EventFormatter *eventFormatter, const Topic& topic, bool publishNull)
{
    const static Name eventName("MarketDataEvents");
    const static Name high("HIGH");
    const static Name low("LOW");

    eventFormatter->appendMessage(eventName, topic);
    if (publishNull) {
        eventFormatter->setElementNull(high);
        eventFormatter->setElementNull(low);
    } else {
        const int seconds = 10;
        eventFormatter->setElement(high, seconds * 1.0);
        eventFormatter->setElement(low, seconds * 0.5);
    }
}

void formatPageDataEvent(
        EventFormatter *eventFormatter, const Topic& topic, bool publishNull)
{
    std::string text = "text";
    int numRows = 5;

    for (int i = 1; i <= numRows; ++i) {
        eventFormatter->appendMessage("RowUpdate", topic);
        eventFormatter->setElement("rowNum", i);

        if (publishNull) {
            eventFormatter->setElementNull("spanUpdate");
        } else {
            eventFormatter->pushElement("spanUpdate");
            eventFormatter->appendElement();
            eventFormatter->setElement("startCol", 1);
            eventFormatter->setElement(
                    "length", static_cast<Int32>(text.length()));

            eventFormatter->setElement("text", text.c_str());
            eventFormatter->popElement();
        }

        eventFormatter->popElement();
    }
}
} // namespace {

class MyEventHandler : public ProviderEventHandler {
    const std::string d_serviceName;
    const std::vector<int> d_eids;
    const int d_resolveSubServiceCode;
    const bool d_isPageData;

    void processTopicStatusEvent(
            const Event& event, ProviderSession *session) const;
    void processPermissionRequest(
            const Message& msg, ProviderSession *session) const;

  public:
    MyEventHandler(const std::string& serviceName,
            const std::vector<int>& eids,
            int resolveSubServiceCode,
            bool isPageData)
        : d_serviceName(serviceName)
        , d_eids(eids)
        , d_resolveSubServiceCode(resolveSubServiceCode)
        , d_isPageData(isPageData)
    {
    }

    bool processEvent(const Event& event, ProviderSession *session) override;
};

bool MyEventHandler::processEvent(const Event& event, ProviderSession *session)
{
    const Event::EventType eventType = event.eventType();
    if (eventType == Event::SESSION_STATUS) {
        Utils::printEvent(event);
        MessageIterator iter(event);
        while (iter.next()) {
            Message msg = iter.message();
            if (msg.messageType() == Names::sessionTerminated()) {
                std::lock_guard<std::mutex> guard(g_mutex);
                g_running = false;
                break;
            }
        }
    } else if (eventType == Event::TOPIC_STATUS) {
        processTopicStatusEvent(event, session);
    } else if (eventType == Event::RESOLUTION_STATUS) {
        MessageIterator iter(event);
        while (iter.next()) {
            Message msg = iter.message();
            if (msg.messageType() == Names::resolutionSuccess()) {
                const char *resolvedTopic
                        = msg.getElementAsString(RESOLVED_TOPIC);
                std::cout << "ResolvedTopic: " << resolvedTopic << "\n";
            } else if (msg.messageType() == Names::resolutionFailure()) {
                std::cout << "Topic resolution failed (cid = "
                          << msg.correlationId() << ")\n";
            }
        }
    } else if (eventType == Event::REQUEST) {
        MessageIterator iter(event);
        while (iter.next()) {
            Message msg = iter.message();
            msg.print(std::cout);
            if (msg.messageType() == Names::permissionRequest()) {
                processPermissionRequest(msg, session);
            }
        }
    } else {
        Utils::printEvent(event);
    }

    return true;
}

void MyEventHandler::processTopicStatusEvent(
        const Event& event, ProviderSession *session) const
{
    TopicList topicsToCreate;
    std::vector<Topic> unsubscribedTopics;
    MessageIterator iter(event);
    while (iter.next()) {
        Message msg = iter.message();
        msg.print(std::cout) << "\n";

        Name messageType = msg.messageType();
        std::string topicStr = msg.getElementAsString(TOPIC);
        Topic topic = session->getTopic(msg);
        if (messageType == Names::topicSubscribed()) {
            if (!topic.isValid()) {
                // Add the topic contained in the message to TopicList
                topicsToCreate.add(msg);
            }
        } else if (messageType == Names::topicUnsubscribed()) {
            unsubscribedTopics.push_back(topic);

            std::lock_guard<std::mutex> guard(g_mutex);
            g_activeTopics.erase(topicStr);
            g_activeTopicsCondition.notify_all();
        } else if (messageType == Names::topicActivated()) {
            std::lock_guard<std::mutex> guard(g_mutex);
            g_activeTopics[topicStr] = topic;
            g_activeTopicsCondition.notify_all();
        } else if (messageType == Names::topicRecap()) {
            // Here we send a recap in response to a Recap request.
            Service service = topic.service();
            CorrelationId recapCid = msg.correlationId();

            Event recapEvent = service.createPublishEvent();
            EventFormatter eventFormatter(recapEvent);
            eventFormatter.appendRecapMessage(topic, &recapCid);

            if (d_isPageData) {
                formatPageRecapEvent(&eventFormatter);
            } else {
                formatMarketDataRecapEvent(&eventFormatter);
            }

            session->publish(recapEvent);
        }
    }

    // createTopicsAsync will result in RESOLUTION_STATUS, TOPIC_CREATED
    // events.
    if (topicsToCreate.size()) {
        session->createTopicsAsync(topicsToCreate);
    }

    // Delete all the unsubscribed topics.
    if (!unsubscribedTopics.empty()) {
        session->deleteTopics(unsubscribedTopics);
    }
}

void MyEventHandler::processPermissionRequest(
        const Message& msg, ProviderSession *session) const
{
    Service service = session->getService(d_serviceName.c_str());

    // A response event must contain only one response message and attach the
    // correlation ID of the request message.
    Event response = service.createResponseEvent(msg.correlationId());
    int permission = 1; // ALLOWED: 0, DENIED: 1
    EventFormatter ef(response);
    if (msg.hasElement("uuid")) {
        const int uuid = msg.getElementAsInt32("uuid");
        std::cout << "uuid = " << uuid << "\n";
        permission = 0;
    }

    if (msg.hasElement("applicationId")) {
        const int applicationId = msg.getElementAsInt32("applicationId");
        std::cout << "applicationId = " << applicationId << "\n";
        permission = 0;
    }

    ef.appendResponse(Names::permissionResponse());
    ef.pushElement("topicPermissions");

    // For each of the topics in the request, add an entry to the response
    Element topicsElement = msg.getElement(TOPICS);
    for (size_t i = 0; i < topicsElement.numValues(); ++i) {
        ef.appendElement();
        ef.setElement("topic", topicsElement.getValueAsString(i));
        if (d_resolveSubServiceCode != INT_MIN) {
            try {
                ef.setElement("subServiceCode", d_resolveSubServiceCode);
                std::cout << "Mapping topic "
                          << topicsElement.getValueAsString(i)
                          << " to subServiceCode " << d_resolveSubServiceCode
                          << "\n";
            } catch (Exception& e) {
                std::cerr << "subServiceCode could not be set."
                             " Resolving without subServiceCode"
                          << e.description() << "\n";
            }
        }

        ef.setElement("result", permission); // ALLOWED: 0, DENIED: 1

        if (permission == 1) {
            // DENIED

            ef.pushElement("reason");
            ef.setElement("source", "My Publisher Name");
            ef.setElement("category", "NOT_AUTHORIZED");
            // or BAD_TOPIC, or custom

            ef.setElement("subcategory", "Publisher Controlled");
            ef.setElement(
                    "description", "Permission denied by My Publisher Name");
            ef.popElement();
        } else {
            if (!d_eids.empty()) {
                ef.pushElement("permissions");
                ef.appendElement();
                ef.setElement("permissionService", "//blp/blpperm");

                ef.pushElement("eids");
                for (auto eid : d_eids) {
                    ef.appendValue(eid);
                }

                ef.popElement();
                ef.popElement();
                ef.popElement();
            }
        }

        ef.popElement();
    }

    ef.popElement();

    // Service is implicit in the Event. sendResponse has a second
    // parameter - partialResponse - that defaults to false.
    session->sendResponse(response);
}

class InteractivePublisherExample {
    ArgParser d_argParser;
    ConnectionAndAuthOptions d_connectionAndAuthOptions;

    int d_priority;
    std::string d_service;
    std::vector<int> d_eids;
    std::string d_groupId;
    int d_clearInterval;

    bool d_useSsc;
    int d_sscBegin;
    int d_sscEnd;
    int d_sscPriority;

    int d_resolveSubServiceCode;
    bool d_isPageData;

    void parseSubServiceCodeRangeAndPriority(const char *value)
    {
        d_useSsc = true;

        std::vector<std::string> tokens = Utils::split(value, ',');
        if (tokens.size() != 3u) {
            throw std::invalid_argument(
                    std::string("Invalid sub-service code range: ") + value);
        }

        d_sscBegin = std::stol(tokens[0]);
        d_sscEnd = std::stol(tokens[1]);
        d_sscPriority = std::stol(tokens[2]);
    }

    bool parseCommandLine(int argc, const char **argv)
    {
        try {
            ArgGroup& argGroupPublisher
                    = d_argParser.addGroup("Publisher Options");
            argGroupPublisher.addArg("service", 's')
                    .setDescription("the service name")
                    .setMetaVar("service")
                    .setIsRequired(true)
                    .setAction(
                            [this](const char *value) { d_service = value; });
            argGroupPublisher.addArg("group-id", 'g')
                    .setDescription(
                            "the group ID of the service, default to an "
                            "automatically generated unique value")
                    .setMetaVar("groupId")
                    .setAction(
                            [this](const char *value) { d_groupId = value; });
            argGroupPublisher.addArg("priority", 'p')
                    .setDescription("the service priority")
                    .setMetaVar("priority")
                    .setDefaultValue(std::to_string(d_priority))
                    .setAction([this](const char *value) {
                        d_priority = std::stol(value);
                    });
            argGroupPublisher.addArg("register-ssc")
                    .setDescription("specify active sub-service code range "
                                    "and priority separated by ','")
                    .setMetaVar("begin,end,priority")
                    .setAction([this](const char *value) {
                        parseSubServiceCodeRangeAndPriority(value);
                    });
            argGroupPublisher.addArg("clear-cache")
                    .setDescription("number of events after which cache will "
                                    "be cleared "
                                    "(default: 0, i.e cache never cleared)")
                    .setMetaVar("eventCount")
                    .setAction([this](const char *value) {
                        d_clearInterval = std::stol(value);
                    });
            argGroupPublisher.addArg("resolve-ssc")
                    .setDescription("sub-service code to be used in "
                                    "permission response")
                    .setMetaVar("subServiceCode")
                    .setAction([this](const char *value) {
                        d_resolveSubServiceCode = std::stol(value);
                    });
            argGroupPublisher.addArg("eid", 'E')
                    .setDescription(
                            "EIDs that are used in permission response")
                    .setMetaVar("eid")
                    .setAction([this](const char *value) {
                        d_eids.push_back(std::stol(value));
                    });
            argGroupPublisher.addArg("page", 'P')
                    .setDescription("enable publish as page")
                    .setMode(ArgMode::NO_VALUE)
                    .setAction([this](const char *) { d_isPageData = true; });

            d_argParser.parse(argc, argv);
        } catch (const std::exception& ex) {
            std::cerr << "Failed to parse arguments: " << ex.what() << "\n";
            d_argParser.printHelp();
            return false;
        }

        return true;
    }

  public:
    InteractivePublisherExample()
        : d_argParser(
                "Interactive data publisher", "InteractivePublisherExample")
        , d_connectionAndAuthOptions(d_argParser)
        , d_priority(ServiceRegistrationOptions::PRIORITY_HIGH)
        , d_clearInterval(0)
        , d_useSsc(false)
        , d_resolveSubServiceCode(INT_MIN)
        , d_isPageData(false)
    {
    }

    void activate(ProviderSession *session) const
    {
        if (d_useSsc) {
            std::cout << "Activating sub service code range "
                      << "[" << d_sscBegin << ", " << d_sscEnd
                      << "] @ priority " << d_sscPriority << "\n";
            session->activateSubServiceCodeRange(
                    d_service.c_str(), d_sscBegin, d_sscEnd, d_sscPriority);
        }
    }

    void deactivate(ProviderSession *session) const
    {
        if (d_useSsc) {
            std::cout << "Deactivating sub service code range [" << d_sscBegin
                      << ", " << d_sscEnd << "] @ priority " << d_sscPriority
                      << "\n";

            session->deactivateSubServiceCodeRange(
                    d_service.c_str(), d_sscBegin, d_sscEnd);
        }
    }

    void run(int argc, const char **argv)
    {
        if (!parseCommandLine(argc, argv)) {
            return;
        }

        MyEventHandler myEventHandler(
                d_service, d_eids, d_resolveSubServiceCode, d_isPageData);
        ProviderSession session(
                d_connectionAndAuthOptions.createSessionOption(),
                &myEventHandler);

        if (!session.start()) {
            std::cerr << "Failed to start session.\n";
            return;
        }

        ServiceRegistrationOptions serviceOptions;
        serviceOptions.setGroupId(
                d_groupId.c_str(), static_cast<int>(d_groupId.size()));
        serviceOptions.setServicePriority(d_priority);
        if (d_useSsc) {
            std::cout << "Adding active sub service code range "
                      << "[" << d_sscBegin << ", " << d_sscEnd
                      << "] @ priority " << d_sscPriority << "\n";
            try {
                serviceOptions.addActiveSubServiceCodeRange(
                        d_sscBegin, d_sscEnd, d_sscPriority);
            } catch (Exception& e) {
                std::cerr << "FAILED to add active sub service codes."
                             " Exception "
                          << e.description() << "\n";
            }
        }

        if (!session.registerService(d_service.c_str(),
                    session.getAuthorizedIdentity(),
                    serviceOptions)) {
            std::cerr << "Failed to register " << d_service << "\n";
            session.stop();
            return;
        }

        std::cout << "Service registered: " << d_service << "\n";
        Service service = session.getService(d_service.c_str());

        // Now we will start publishing
        int eventCount = 0;
        while (true) {
            Event event;
            {
                std::unique_lock<std::mutex> lg(g_mutex);
                if (!g_running) {
                    break;
                }

                if (!g_activeTopicsCondition.wait_for(lg,
                            std::chrono::seconds(1),
                            [] { return !g_activeTopics.empty(); })) {
                    continue;
                }

                bool publishNull = false;
                if (d_clearInterval > 0 && eventCount == d_clearInterval) {
                    eventCount = 0;
                    publishNull = true;
                }

                event = service.createPublishEvent();
                EventFormatter eventFormatter(event);
                for (const auto& kvPair : g_activeTopics) {
                    const Topic& topic = kvPair.second;
                    if (d_isPageData) {
                        formatPageDataEvent(
                                &eventFormatter, topic, publishNull);
                    } else {
                        formatMarketDataEvent(
                                &eventFormatter, topic, publishNull);
                    }
                }
            }

            std::cout << "Publishing event:\n";
            Utils::printEvent(event);
            session.publish(event);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (++eventCount % 10 == 0) {
                deactivate(&session);
                std::this_thread::sleep_for(std::chrono::seconds(30));
                activate(&session);
            }
        }

        session.stop();
    }
};

int main(int argc, const char **argv)
{
    InteractivePublisherExample example;
    try {
        example.run(argc, argv);
    } catch (Exception& e) {
        std::cerr << "Library Exception!!! " << e.description() << "\n";
    }

    // wait for enter key to exit application
    std::cout << "Press ENTER to quit"
              << "\n";
    char dummy[2];
    std::cin.getline(dummy, 2);
    return 0;
}
