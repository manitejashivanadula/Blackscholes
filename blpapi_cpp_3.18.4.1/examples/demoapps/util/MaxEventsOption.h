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

#ifndef INCLUDED_MAX_EVENTS_OPTION
#define INCLUDED_MAX_EVENTS_OPTION

// @PURPOSE:
//     Helper to add max number of events to arg parser.
//
// @CLASSES:
//     MaxEventsOption:  Max events option definition.
//
// @DESCRIPTION:
//     Helper class that adds the option for the maximum
//     number of events  before stopping to ArgParser and
//     parse the command line argument to an
//     integer as the maximum number of events.

#include <climits>

#include <util/ArgParser.h>

namespace BloombergLP {

class MaxEventsOption {
  public:
    MaxEventsOption(ArgParser& argParser);

    int getMaxEvents() const;

  private:
    int d_maxEvents = INT_MAX;
};

inline MaxEventsOption::MaxEventsOption(ArgParser& argParser)
{
    std::function<void(const char *)> action
            = [this](const char *value) { d_maxEvents = std::stoi(value); };

    argParser.addArg("max-events")
            .setMetaVar("maxEvents")
            .setDescription("number of events to process before stopping")
            .setDefaultValue(std::to_string(d_maxEvents))
            .setAction(action);
}

inline int MaxEventsOption::getMaxEvents() const { return d_maxEvents; }

} // Close namespace BloombergLP

#endif // #ifndef INCLUDED_MAX_EVENTS_OPTION
