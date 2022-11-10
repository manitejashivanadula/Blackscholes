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

#include "resolver_utils.h"

#include <blpapi_eventformatter.h>

// 'blp::Name' objects are more expensive to construct than std::string, but
// are more efficient on use through the interface. By creating the
// 'blp::Name' objects in advance we can take advantage of the efficiency
// without paying the cost of constructing them when needed.
namespace {
blp::Name PERMISSION_REQUEST("PermissionRequest");
blp::Name PERMISSION_RESPONSE("PermissionResponse");
blp::Name TOPIC("topic");
blp::Name TOPICS("topics");
blp::Name TOPIC_PERMISSION("topicPermissions");
blp::Name RESULT("result");
blp::Name REASON("reason");
blp::Name SOURCE("source");
blp::Name CATEGORY("category");
blp::Name SUBCATEGORY("subcategory");
blp::Name DESCRIPTION("description");

const int ALLOWED_APP_ID(1234);
const char *RESOLVER_ID = "service:hostname";
// This can be any string, but it's helpful to provide information on the
// instance of the resolver that responded to debug failures in production.
}

// This helper demonstrates how to register a service.
//
// This helper assumes the following:
// * Specified `session` is already started
// * Specified `providerIdentity` is already authorized if auth is needed or
//   default-constructed if authorization is not required.
bool resolutionServiceRegistration(blp::ProviderSession& session,
        const blp::Identity& providerIdentity,
        const char *serviceName)
{
    // Prepare registration options
    blp::ServiceRegistrationOptions serviceOptions;

    const int dummyPriority = 123;
    serviceOptions.setServicePriority(dummyPriority);

    serviceOptions.setPartsToRegister(
            blp::ServiceRegistrationOptions::PART_PUBLISHER_RESOLUTION);

    // Call `registerService`
    if (!session.registerService(
                serviceName, providerIdentity, serviceOptions)) {
        std::cerr << "Failed to register " << serviceName << std::endl;
        return false;
    }
    return true;
}

bool handlePermissionRequest(blp::ProviderSession& session,
        const blp::Service& service,
        const blp::Message& request)
{
    assert(request.messageType() == PERMISSION_REQUEST);

    int disallowed = 1;
    if (request.hasElement("applicationId")
            && request.getElementAsInt32("applicationId") == ALLOWED_APP_ID) {
        disallowed = 0;
    }

    blp::Event response = service.createResponseEvent(request.correlationId());
    blp::EventFormatter formatter(response);
    formatter.appendResponse(PERMISSION_RESPONSE);

    blp::Element topics = request.getElement(TOPICS);
    formatter.pushElement(TOPIC_PERMISSION);
    for (unsigned int i = 0; i < topics.numValues(); ++i) {
        formatter.appendElement();
        formatter.setElement(TOPIC, topics.getValueAsString(i));
        formatter.setElement(RESULT, disallowed);
        if (disallowed) {
            formatter.pushElement(REASON);
            formatter.setElement(SOURCE, RESOLVER_ID);
            formatter.setElement(CATEGORY, "NOT_AUTHORIZED");
            formatter.setElement(SUBCATEGORY, "");
            formatter.setElement(DESCRIPTION, "Only app 1234 allowed");
            formatter.popElement();
        }
        formatter.popElement();
    }

    formatter.popElement();

    session.sendResponse(response);
    return true;
}
