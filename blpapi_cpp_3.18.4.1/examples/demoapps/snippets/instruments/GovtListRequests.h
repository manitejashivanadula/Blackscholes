/* Copyright 2021, Bloomberg Finance L.P.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions: The above copyright
 * notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef INCLUDED_GOVTLISTREQUESTS
#define INCLUDED_GOVTLISTREQUESTS

using namespace BloombergLP;
using namespace blpapi;

class GovtListRequests {
  public:
    static Request createRequest(const Service& instrumentsService,
            const std::string& query,
            int maxResults,
            const std::vector<std::tuple<std::string, std::string>>& filters)
    {
        static const Name NAME_QUERY("query");
        static const Name NAME_MAX_RESULTS("maxResults");

        Request request = instrumentsService.createRequest("govtListRequest");
        request.set(NAME_QUERY, query.c_str());
        request.set(NAME_MAX_RESULTS, maxResults);

        for (const auto& filter : filters) {
            const std::string& name = std::get<0>(filter);
            const std::string& value = std::get<1>(filter);

            try {
                request.set(name.c_str(), value.c_str());
            } catch (const NotFoundException& e) {
                throw std::invalid_argument(
                        "Filter not found: " + name + e.description());
            } catch (const InvalidConversionException& e) {
                throw std::invalid_argument("Invalid value: " + value
                        + " for filter: " + name + e.description());
            }
        }

        return request;
    }

    static void processResponse(const Message& msg)
    {
        static const Name NAME_NAME("name");
        static const Name NAME_RESULTS("results");
        static const Name NAME_TICKER("ticker");
        static const Name NAME_PARSEKY("parseky");

        Element results = msg.getElement(NAME_RESULTS);
        int numResults = results.numValues();
        std::cout << "Processing " << numResults << " results:\n";
        for (int i = 0; i < numResults; ++i) {
            Element result = results.getValueAsElement(i);
            std::string parsekey = result.getElementAsString(NAME_PARSEKY);
            std::string name = result.getElementAsString(NAME_NAME);
            std::string ticker = result.getElementAsString(NAME_TICKER);
            std::cout << "    " << (i + 1) << " " << parsekey << ", " << name
                      << " - " << ticker << "\n";
        }
    }
};
#endif
