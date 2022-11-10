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

#include <testSchemas.h>

const char *k_apiauthSchema("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\
<ServiceDefinition name=\"blp.apiauth\" version=\"1.0.7.2\">\
    <service name=\"//blp/apiauth\" version=\"1.0.7.2\">\
        <operation name=\"AuthorizationRequest\" serviceId=\"99\">\
            <request>Request</request>\
            <requestSelection>AuthorizationRequest</requestSelection>\
            <response>Response</response>\
            <responseSelection>AuthorizationSuccess</responseSelection>\
            <responseSelection>AuthorizationFailure</responseSelection>\
            <isAuthorizationRequest>true</isAuthorizationRequest>\
        </operation>\
        <operation name=\"TokenRequest\" serviceId=\"99\">\
            <request>Request</request>\
            <requestSelection>TokenRequest</requestSelection>\
            <response>Response</response>\
            <responseSelection>TokenResponse</responseSelection>\
        </operation>\
        <publisherSupportsRecap>false</publisherSupportsRecap>\
        <authoritativeSourceSupportsRecap>false</authoritativeSourceSupportsRecap>\
        <isInfrastructureService>false</isInfrastructureService>\
        <isMetered>false</isMetered>\
        <appendMtrId>false</appendMtrId>\
    </service>\
    <schema>\
        <sequenceType name=\"AuthorizationRequest\">\
            <description>seqAuthorizationRequest</description>\
            <element name=\"ipAddress\" type=\"String\" minOccurs=\"0\"\
                maxOccurs=\"1\">\
                <description></description>\
                <cacheable>true</cacheable>\
                <cachedOnlyOnInitialPaint>false</cachedOnlyOnInitialPaint>\
            </element>\
            <element name=\"token\" type=\"String\" minOccurs=\"0\" maxOccurs=\"1\">\
                <description></description>\
                <cacheable>true</cacheable>\
                <cachedOnlyOnInitialPaint>false</cachedOnlyOnInitialPaint>\
            </element>\
        </sequenceType>\
        <sequenceType name=\"TokenRequest\">\
            <description>seqTokenRequest</description>\
            <element name=\"uuid\" type=\"Int32\">\
                <description></description>\
                <cacheable>true</cacheable>\
                <cachedOnlyOnInitialPaint>false</cachedOnlyOnInitialPaint>\
            </element>\
            <element name=\"label\" type=\"String\">\
                <description></description>\
                <cacheable>true</cacheable>\
                <cachedOnlyOnInitialPaint>false</cachedOnlyOnInitialPaint>\
            </element>\
        </sequenceType>\
        <sequenceType name=\"ErrorInfo\">\
            <description>seqErrorInfo</description>\
            <element name=\"code\" type=\"Int32\">\
                <description></description>\
                <cacheable>true</cacheable>\
                <cachedOnlyOnInitialPaint>false</cachedOnlyOnInitialPaint>\
            </element>\
            <element name=\"message\" type=\"String\">\
                <description></description>\
                <cacheable>true</cacheable>\
                <cachedOnlyOnInitialPaint>false</cachedOnlyOnInitialPaint>\
            </element>\
            <element name=\"category\" type=\"String\">\
                <description></description>\
                <cacheable>true</cacheable>\
                <cachedOnlyOnInitialPaint>false</cachedOnlyOnInitialPaint>\
            </element>\
            <element name=\"subcategory\" type=\"String\">\
                <description></description>\
                <cacheable>true</cacheable>\
                <cachedOnlyOnInitialPaint>false</cachedOnlyOnInitialPaint>\
            </element>\
            <element name=\"source\" type=\"String\">\
                <description></description>\
                <cacheable>true</cacheable>\
                <cachedOnlyOnInitialPaint>false</cachedOnlyOnInitialPaint>\
            </element>\
        </sequenceType>\
        <sequenceType name=\"AuthorizationSuccess\">\
            <description>seqAuthorizationSuccess</description>\
        </sequenceType>\
        <sequenceType name=\"AuthorizationFailure\">\
            <description>seqAuthorizationFailure</description>\
            <element name=\"reason\" type=\"ErrorInfo\">\
                <description></description>\
                <cacheable>true</cacheable>\
                <cachedOnlyOnInitialPaint>false</cachedOnlyOnInitialPaint>\
            </element>\
        </sequenceType>\
        <sequenceType name=\"AuthorizationTokenResponse\">\
            <description>seqAuthorizationTokenResponse</description>\
        </sequenceType>\
        <sequenceType name=\"TokenResponse\">\
            <description>seqTokenResponse</description>\
            <element name=\"token\" type=\"String\">\
                <description></description>\
                <cacheable>true</cacheable>\
                <cachedOnlyOnInitialPaint>false</cachedOnlyOnInitialPaint>\
            </element>\
            <element name=\"key\" type=\"String\">\
                <description></description>\
                <cacheable>true</cacheable>\
                <cachedOnlyOnInitialPaint>false</cachedOnlyOnInitialPaint>\
            </element>\
        </sequenceType>\
        <choiceType name=\"Request\">\
            <description>choiceRequest</description>\
            <element name=\"AuthorizationRequest\" type=\"AuthorizationRequest\">\
                <description></description>\
                <cacheable>true</cacheable>\
                <cachedOnlyOnInitialPaint>false</cachedOnlyOnInitialPaint>\
            </element>\
            <element name=\"TokenRequest\" type=\"TokenRequest\">\
                <description></description>\
                <cacheable>true</cacheable>\
                <cachedOnlyOnInitialPaint>false</cachedOnlyOnInitialPaint>\
            </element>\
        </choiceType>\
        <choiceType name=\"Response\">\
            <description>choiceResponse</description>\
            <element name=\"AuthorizationSuccess\" type=\"AuthorizationSuccess\">\
                <description></description>\
                <cacheable>true</cacheable>\
                <cachedOnlyOnInitialPaint>false</cachedOnlyOnInitialPaint>\
            </element>\
            <element name=\"AuthorizationFailure\" type=\"AuthorizationFailure\">\
                <description></description>\
                <cacheable>true</cacheable>\
                <cachedOnlyOnInitialPaint>false</cachedOnlyOnInitialPaint>\
            </element>\
            <element name=\"TokenResponse\" type=\"TokenResponse\">\
                <description></description>\
                <cacheable>true</cacheable>\
                <cachedOnlyOnInitialPaint>false</cachedOnlyOnInitialPaint>\
            </element>\
        </choiceType>\
    </schema>\
</ServiceDefinition>");

// Following is a snippet of mktdata service schema. blpapi-mock example
// is using this schema to create mktdata events.
const char *k_mktdataSchema("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\
<ServiceDefinition name=\"blp.mktdata\" version=\"1.0.1.0\">\
   <service name=\"//blp/mktdata\" version=\"1.0.0.0\" authorizationService=\"//blp/apiauth\">\
      <event name=\"MarketDataEvents\" eventType=\"MarketDataUpdate\">\
         <eventId>0</eventId>\
         <eventId>1</eventId>\
         <eventId>2</eventId>\
         <eventId>3</eventId>\
         <eventId>4</eventId>\
         <eventId>9999</eventId>\
      </event>\
      <defaultServiceId>134217729</defaultServiceId> <!-- 0X8000001 -->\
      <resolutionService></resolutionService>\
      <recapEventId>9999</recapEventId>\
   </service>\
   <schema>\
      <sequenceType name=\"MarketDataUpdate\">\
         <description>fields in subscription</description>\
         <element name=\"LAST_PRICE\" type=\"Float64\" id=\"1\" minOccurs=\"0\" maxOccurs=\"1\">\
            <description>Last Trade/Last Price</description>\
            <alternateId>65536</alternateId>\
         </element>\
         <element name=\"BID\" type=\"Float64\" id=\"2\" minOccurs=\"0\" maxOccurs=\"1\">\
            <description>Bid Price</description>\
            <alternateId>131072</alternateId>\
         </element>\
         <element name=\"ASK\" type=\"Float64\" id=\"3\" minOccurs=\"0\" maxOccurs=\"1\">\
            <description>Ask Price</description>\
            <alternateId>196608</alternateId>\
         </element>\
         <element name=\"VOLUME\" type=\"Int64\" id=\"4\" minOccurs=\"0\" maxOccurs=\"1\">\
            <description>Volume</description>\
            <alternateId>458753</alternateId>\
         </element>\
      </sequenceType>\
   </schema>\
</ServiceDefinition>");

const char *getApiAuthSchemaString() { return k_apiauthSchema; }

const char *getMktDataSchemaString() { return k_mktdataSchema; }
