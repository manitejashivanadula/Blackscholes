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

#ifndef INCLUDED_REQUEST_OPTIONS_H
#define INCLUDED_REQUEST_OPTIONS_H

// @PURPOSE:
//     Helper which provides support for argument parser options
//     used in constructing blpapi requests.
//
// @CLASSES:
//     RequestOptions:  A data structure that holds options
//     for creating a blpapi request.
//
//     Override: Defines a parameter override in a reference data request.
//
// @DESCRIPTION:
//     This component provides support for argument parser options
//     used in constructing blpapi requests.
//

#include <blpapi_datetime.h>

#include <string>
#include <time.h>
#include <vector>

#include <util/ArgParser.h>

namespace BloombergLP {

struct Override {
    std::string fieldId;
    std::string value;
};

class RequestOptions {
  public:
    inline RequestOptions();

    inline RequestOptions(ArgParser& argParser);

    inline void setDefaultValues();

    static constexpr const char *REFDATA_SERVICE = "//blp/refdata";
    static constexpr const char *INTRADAY_BAR_REQUEST = "IntradayBarRequest";
    static constexpr const char *INTRADAY_TICK_REQUEST = "IntradayTickRequest";
    static constexpr const char *REFERENCE_DATA_REQUEST
            = "ReferenceDataRequest";
    static constexpr const char *REFERENCE_DATA_REQUEST_OVERRIDE
            = "ReferenceDataRequestOverride";
    static constexpr const char *REFERENCE_DATA_REQUEST_TABLE_OVERRIDE
            = "ReferenceDataRequestTableOverride";
    static constexpr const char *HISTORICAL_DATA_REQUEST
            = "HistoricalDataRequest";

    std::string d_service;

    std::vector<std::string> d_securities;

    std::vector<std::string> d_fields;

    int d_barInterval;

    bool d_gapFillInitialBar;

    bool d_includeConditionCodes;

    std::string d_startDateTime;

    std::string d_endDateTime;

    std::string d_requestType;

    std::vector<std::string> d_eventTypes;

    std::vector<Override> d_overrides;

  private:
    inline void computeDefaultStartAndEndDateTime();

    inline void appendOverride(const std::string& overrideStr);

    std::string d_defaultIntradayBarEndDateTime;
    std::string d_defaultIntradayTickEndDateTime;
};

RequestOptions::RequestOptions()
    : d_includeConditionCodes(false)
    , d_barInterval(60)
    , d_gapFillInitialBar(false)
    , d_requestType("ReferenceDataRequest")
{
    computeDefaultStartAndEndDateTime();
}

RequestOptions::RequestOptions(ArgParser& argParser)
    : d_includeConditionCodes(false)
    , d_barInterval(60)
    , d_gapFillInitialBar(false)
    , d_requestType("ReferenceDataRequest")
{
    computeDefaultStartAndEndDateTime();

    ArgGroup& argGroupRequest = argParser.addGroup("Request Options");

    std::string defaultBarInterval = "5"; // 5 minutes
    argGroupRequest.addArg("service", 's')
            .setDescription("The service name")
            .setMetaVar("service")
            .setDefaultValue(REFDATA_SERVICE)
            .setAction([this](const char *value) { d_service = value; });

    argGroupRequest.addArg("security", 'S')
            .setDescription("Security to request")
            .setMetaVar("security")
            .setMode(ArgMode::MULTIPLE_VALUES)
            .setAction([this](const char *value) {
                d_securities.push_back(value);
            });

    argGroupRequest.addArg("field", 'f')
            .setDescription("Field to request")
            .setMetaVar("field")
            .setMode(ArgMode::MULTIPLE_VALUES)
            .setAction(
                    [this](const char *value) { d_fields.push_back(value); });

    argGroupRequest.addArg("event", 'e')
            .setDescription("Event type")
            .setMetaVar("eventType")
            .setMode(ArgMode::MULTIPLE_VALUES)
            .setDefaultValue("TRADE")
            .setAction([this](const char *value) {
                d_eventTypes.push_back(value);
            });

    argGroupRequest.addArg("interval", 'i')
            .setDescription("Bar interval in minutes")
            .setMetaVar("barInterval")
            .setDefaultValue(defaultBarInterval)
            .setAction([this](const char *value) {
                d_barInterval = std::stoi(value);
            });

    argGroupRequest.addArg("include-condition-codes", 'I')
            .setDescription("Include condition codes")
            .setMode(ArgMode::NO_VALUE)
            .setAction([this](const char *value) {
                d_includeConditionCodes = true;
            });

    argGroupRequest.addArg("gap-fill-initial-bar", 'G')
            .setDescription("Gap fill initial bar")
            .setMode(ArgMode::NO_VALUE)
            .setAction(
                    [this](const char *value) { d_gapFillInitialBar = true; });

    argGroupRequest.addArg("start-date")
            .setDescription(
                    "Start datetime in the format of YYYY-MM-DD'T'HH:mm:ss")
            .setMetaVar("startDateTime")
            .setAction([this](const char *value) { d_startDateTime = value; });

    argGroupRequest.addArg("end-date")
            .setDescription(
                    "End datetime in the format of YYYY-MM-DD'T'HH:mm:ss")
            .setMetaVar("endDateTime")
            .setAction([this](const char *value) { d_endDateTime = value; });

    argGroupRequest.addArg("override", 'O')
            .setDescription("Field to override")
            .setMetaVar("<fieldId>=<value>")
            .setMode(ArgMode::MULTIPLE_VALUES)
            .setAction([this](const char *value) { appendOverride(value); });

    argGroupRequest.addArg("request", 'r')
            .setDescription("Request type.\n"
                            "To retrieve reference data: \n"
                            "\t-r, --request "
                    + std::string(REFERENCE_DATA_REQUEST)
                    + "\n"
                      "\t[-S, --security <security = {IBM US Equity, MSFT US "
                      "Equity}>]\n"
                      "\t[-f, --field <field = PX_LAST>]\n"
                      "To retrieve reference data with overrides: \n"
                      "\t-r, --request "
                    + std::string(REFERENCE_DATA_REQUEST_OVERRIDE)
                    + "\n"
                      "\t[-S, --security <security = {IBM US Equity, MSFT US "
                      "Equity}>]\n"
                      "\t[-f, --field <field = PX_LAST>]\n"
                      "\t[-O, --override <<fieldId>=<value> = "
                      "{VWAP_START_TIME=9:30, "
                      "VWAP_END_TIME=11:30}]\n"
                      "To retrieve reference data with table overrides: \n"
                      "\t-r, --request "
                    + std::string(REFERENCE_DATA_REQUEST_TABLE_OVERRIDE)
                    + "\n"
                      "\t[-S, --security <security = FHR 3709 FA Mtge>]\n"
                      "\t[-f, --field <field = {MTG_CASH_FLOW, SETTLE_DT}>]\n"
                      "To retrieve intraday bars: \n"
                      "\t-r, --request "
                    + std::string(INTRADAY_BAR_REQUEST)
                    + "\n"
                      "\t[-S, --security <security = IBM US Equity>]\n"
                      "\t[-e, --event <event = TRADE>]\n"
                      "\t[-i, --interval <barInterval = "
                    + defaultBarInterval
                    + ">]\n"
                      "\t[--start-date <startDateTime = "
                    + d_startDateTime
                    + ">]\n"
                      "\t[--end-date <endDateTime = "
                    + d_defaultIntradayBarEndDateTime
                    + ">]\n"
                      "\t[-G, --gap-fill-initial-bar]\n"
                      "\t\t1) All times are in GMT.\n"
                      "\t\t2) Only one security can be specified.\n"
                      "\t\t3) Only one event can be specified.\n"
                      "To retrieve intraday raw ticks: \n"
                      "\t-r "
                    + std::string(INTRADAY_TICK_REQUEST)
                    + "\n"
                      "\t[-e, --event <event = TRADE>]\n"
                      "\t[--start-date <startDateTime = "
                    + d_startDateTime
                    + ">]\n"
                      "\t[--end-date <endDateTime = "
                    + d_defaultIntradayTickEndDateTime
                    + ">]\n"
                      "\t[-I, --include-condition-codes]\n"
                      "\t\t1) All times are in GMT.\n"
                      "\t\t2) Only one security can be specified.\n"
                      "To retrieve historical data: \n"
                      "\t-r "
                    + std::string(HISTORICAL_DATA_REQUEST)
                    + "\n"
                      "\t[-S, --security <security = {IBM US Equity, MSFT US "
                      "Equity}>]\n"
                      "\t[-f <field = PX_LAST>]\n")
            .setMetaVar("requestType")
            .setDefaultValue(REFERENCE_DATA_REQUEST)
            .setChoices({ REFERENCE_DATA_REQUEST,
                    REFERENCE_DATA_REQUEST_OVERRIDE,
                    REFERENCE_DATA_REQUEST_TABLE_OVERRIDE,
                    INTRADAY_BAR_REQUEST,
                    INTRADAY_TICK_REQUEST,
                    HISTORICAL_DATA_REQUEST })
            .setAction([this](const char *value) { d_requestType = value; });
}

void RequestOptions::setDefaultValues()
{
    const char *requestType = d_requestType.c_str();

    if (d_securities.empty()) {
        if (!std::strcmp(requestType, REFERENCE_DATA_REQUEST_TABLE_OVERRIDE)) {
            d_securities.emplace_back("FHR 3709 FA Mtge");
        } else {
            d_securities.emplace_back("IBM US Equity");
            d_securities.emplace_back("MSFT US Equity");
        }
    }

    if (d_fields.empty()) {
        if (!std::strcmp(requestType, REFERENCE_DATA_REQUEST_TABLE_OVERRIDE)) {
            d_fields.emplace_back("MTG_CASH_FLOW");
            d_fields.emplace_back("SETTLE_DT");
        } else {
            d_fields.emplace_back("PX_LAST");
            if (!std::strcmp(requestType, REFERENCE_DATA_REQUEST_OVERRIDE)) {
                d_fields.emplace_back("DS002");
                d_fields.emplace_back("EQY_WEIGHTED_AVG_PX");
            }
        }
    }

    if (d_overrides.empty()
            && !std::strcmp(requestType, REFERENCE_DATA_REQUEST_OVERRIDE)) {
        d_overrides.push_back({ "VWAP_START_TIME", "9:30" });
        d_overrides.push_back({ "VWAP_END_TIME", "11:30" });
    }

    if (d_eventTypes.empty()) {
        d_eventTypes.emplace_back("TRADE");
    }

    if (d_endDateTime.empty()) {
        d_endDateTime = d_requestType == INTRADAY_BAR_REQUEST
                ? d_defaultIntradayBarEndDateTime
                : d_defaultIntradayTickEndDateTime;
    }
}

void RequestOptions::computeDefaultStartAndEndDateTime()
{
    struct tm *tm_p;
    time_t currTime = time(0);

    const int DAY_IN_SECONDS = 86400;
    while (currTime > 0) {
        currTime -= DAY_IN_SECONDS; // Go back one day
        tm_p = localtime(&currTime);
        if (tm_p == NULL) {
            throw std::runtime_error(
                    "Unable to get default start and end date time");
        }

        // if not sunday / saturday, assign values & return
        if (tm_p->tm_wday != 0 && tm_p->tm_wday != 6) { // Sun/Sat
            break;
        }
    }

    int year = tm_p->tm_year + 1900;
    int month = tm_p->tm_mon + 1;
    int day = tm_p->tm_mday;

    // The start date is the market open time (GMT) on previous trading day.
    blpapi::Datetime startDate(year, month, day, 14, 30, 0);
    std::ostringstream oss;
    oss << startDate;
    d_startDateTime = oss.str();

    // the next 5 minutes is the default IntradayTick end date
    blpapi::Datetime intradayTickEndDate(year, month, day, 14, 35, 0);
    oss.str("");
    oss << intradayTickEndDate;
    d_defaultIntradayTickEndDateTime = oss.str();

    // the next 1 hr is the default IntradayBar end date
    blpapi::Datetime intradayBarEndDate(year, month, day, 15, 30, 0);
    oss.str("");
    oss << intradayBarEndDate;
    d_defaultIntradayBarEndDateTime = oss.str();
}

void RequestOptions::appendOverride(const std::string& overrideStr)
{
    std::vector<std::string> fieldIdAndValue = Utils::split(overrideStr, '=');

    if (fieldIdAndValue.size() != 2) {
        throw std::invalid_argument("Invalid override " + overrideStr);
    }

    Override override({ fieldIdAndValue[0], fieldIdAndValue[1] });
    d_overrides.push_back(override);
}
}
#endif // INCLUDED_REQUEST_OPTIONS_H
