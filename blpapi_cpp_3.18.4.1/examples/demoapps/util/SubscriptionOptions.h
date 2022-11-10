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

#ifndef INCLUDED_SUBSCRIPTION_OPTIONS
#define INCLUDED_SUBSCRIPTION_OPTIONS

#include <blpapi_correlationid.h>
#include <blpapi_subscriptionlist.h>

#include <stdexcept>

#include <util/ArgParser.h>

// @PURPOSE:
//     Provide support for adding and parsing subscription options from the
//     command line.
//
// @CLASSES:
//     SubscriptionOptions: subscription command line options
//
// @DESCRIPTION:
//     Helper class that adds the options used to create a SubscriptionList to
//     ArgParser and creates a SubscriptionList using the command line
//     arguments.

namespace BloombergLP {

class SubscriptionOptions {
  public:
    SubscriptionOptions(ArgParser& argParser, bool isSnapshot = false);

    blpapi::SubscriptionList createSubscriptionList(
            const std::function<blpapi::CorrelationId(
                    size_t i, const std::string&)>& correlationIdGenerator);

    std::map<std::string, std::string> createSubscriptionStrings();
    // Creates a map from topics provided on the command line to their
    // corresponding subscription strings.

    void setUpSessionOptions(blpapi::SessionOptions& sessionOptions);
    // Adds default values to session option such as
    // default prefix and topic

    static constexpr const char *k_defaultService = "//blp/mktdata";
    static constexpr const char *k_defaultTopicPrefix = "/ticker/";

    std::string getService() const;
    const std::vector<std::string>& getTopics() const;
    const std::vector<std::string>& getFields() const;
    void addField(const std::string& field);

  private:
    static constexpr const char *k_defaultTopic = "IBM US Equity";

    std::string d_service;

    std::vector<std::string> d_topics;

    std::vector<std::string> d_fields;

    std::vector<std::string> d_options;

    std::string d_topicPrefix;

    void parseInterval(const char *value);
};

inline SubscriptionOptions::SubscriptionOptions(
        ArgParser& argParser, bool isSnapshot)
{

    ArgGroup& groupSubscription = argParser.addGroup("Subscriptions");

    groupSubscription.addArg("service", 's')
            .setMetaVar("service")
            .setDescription("service name")
            .setDefaultValue(k_defaultService)
            .setAction([this](const char *value) { d_service = value; });

    groupSubscription.addArg("topic", 't')
            .setMetaVar("topic")
            .setDescription(
                    "topic to subscribe. \nCan be one of the following: "
                    "\n - Instrument"
                    "\n - Instrument qualified with a prefix"
                    "\n - Instrument qualified with a service and a prefix")
            .setDefaultValue(k_defaultTopic)
            .setMode(ArgMode::MULTIPLE_VALUES) // Allow multiple
            .setAction([this](const char *value) {
                d_topics.emplace_back(value);
            });

    groupSubscription.addArg("field", 'f')
            .setMetaVar("field")
            .setDescription("field to subscribe")
            .setMode(ArgMode::MULTIPLE_VALUES) // Allow multiple
            .setAction([this](const char *value) {
                d_fields.emplace_back(value);
            });

    groupSubscription.addArg("option", 'o')
            .setMetaVar("option")
            .setDescription("subscription options")
            .setMode(ArgMode::MULTIPLE_VALUES) // Allow multiple
            .setAction([this](const char *value) {
                d_options.emplace_back(value);
            });

    groupSubscription.addArg("topic-prefix", 'x')
            .setMetaVar("prefix")
            .setDescription("the topic prefix to be used for subscriptions")
            .setDefaultValue(k_defaultTopicPrefix)
            .setAction([this](const char *value) { d_topicPrefix = value; });

    if (!isSnapshot) {
        groupSubscription.addArg("interval", 'i')
                .setMetaVar("interval")
                .setDescription("subscription option that specifies a time in "
                                "seconds to intervalize the subscriptions")
                .setAction(
                        [this](const char *value) { parseInterval(value); });
    }
}

inline void SubscriptionOptions::parseInterval(const char *value)
{
    // Validate by converting to a double
    std::stod(value);
    d_options.emplace_back(std::string("interval=") + value);
}

inline blpapi::SubscriptionList SubscriptionOptions::createSubscriptionList(
        const std::function<blpapi::CorrelationId(size_t, const std::string&)>&
                correlationIdGenerator)
{
    blpapi::SubscriptionList subscriptionList;
    for (size_t i = 0; i < d_topics.size(); ++i) {
        subscriptionList.add(d_topics[i].c_str(),
                d_fields,
                d_options,
                correlationIdGenerator(i, d_topics[i]));
    }
    return subscriptionList;
}

inline std::map<std::string, std::string>
SubscriptionOptions::createSubscriptionStrings()
{
    std::map<std::string, std::string> subStrings;
    blpapi::SubscriptionList subscriptionList;
    for (size_t i = 0; i < d_topics.size(); ++i) {

        // Use SubscriptionList to help construct subscription string
        subscriptionList.add(d_topics[i].c_str(),
                d_fields,
                d_options,
                blpapi::CorrelationId());
        subStrings.emplace(d_topics[i], subscriptionList.topicStringAt(i));
    }
    return subStrings;
}

inline void SubscriptionOptions::setUpSessionOptions(
        blpapi::SessionOptions& sessionOptions)
{
    sessionOptions.setDefaultTopicPrefix(d_topicPrefix.c_str());
    sessionOptions.setDefaultSubscriptionService(d_service.c_str());
}

inline std::string SubscriptionOptions::getService() const
{
    return d_service;
}

inline const std::vector<std::string>& SubscriptionOptions::getTopics() const
{
    return d_topics;
}

inline const std::vector<std::string>& SubscriptionOptions::getFields() const
{
    return d_fields;
}

inline void SubscriptionOptions::addField(const std::string& field)
{
    d_fields.emplace_back(field);
}
} // Close namespace BloombergLP

#endif // #ifndef INCLUDED_SUBSCRIPTION_OPTIONS
