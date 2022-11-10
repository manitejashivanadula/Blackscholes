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

#ifndef INCLUDED_ARG_PARSER
#define INCLUDED_ARG_PARSER

//@PURPOSE:
//    Handles argument parsing for blpapi examples.
//
//@CLASSES:
//     ArgMode: Defines different modes of arguments.
//     Arg: Defines a command line argument.
//     ArgGroup: Groups arguments together.
//     ArgParser: Parses command line arguments.
//
//@DESCRIPTION:
//    This file defines classes and enum(s) used to represent,
//    group and parse command line arguments used by BLPAPI examples.

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <util/Utils.h>

namespace BloombergLP {

enum class ArgMode {
    // Defines how many values can be present in
    // a command line for an Arg.

    NO_VALUE,
    // Indicates this is a boolean type argument
    // that does not require a value, and can
    // only be specified once.

    SINGLE_VALUE,
    // Indicates this is an argument that requires a
    // value, and can only be specified once.

    MULTIPLE_NO_VALUE,
    // Indicates this is a boolean argument, can be
    // specified multiple times, e.g '-verbose'.

    MULTIPLE_VALUES
    // Indicates this is an argument that requires
    // a value and can be specified multiple times, e.g.
    // '-f BID -f ASK'.

};

class Arg {
    // An Arg is an optional argument that is associated with either a
    // shortForm, e.g -t, or a longForm, e.g --topic, in command line. An Arg
    // has the following attributes:
    //
    //   - Can have either a single value by default or multiple
    //    values, e.g. multiple topics.
    //   - Can have a default value, e.g. "IBM US Equity" for topic.
    //   - Can define an 'action' that is invoked when a value is
    //     read from the command line.
    //   - Optional by default and can be set to required.

    void addShortForm(char form);
    void addLongForm(const std::string& form);
    static const char PREFIX = '-';

  public:
    explicit Arg(const std::string& longForm, char shortForm = '\0');

    const std::string& getShortForm() const;

    const std::string& getLongForm() const;

    const std::string& getMetaVar() const;

    Arg& setMetaVar(std::string metaVar);

    const std::string& getDescription() const;

    Arg& setDescription(std::string description);

    ArgMode getMode() const;

    Arg& setMode(ArgMode mode);

    Arg& setChoices(std::vector<std::string> choices);

    bool isValidChoice(const std::string& choice) const;

    const std::vector<std::string>& getChoices() const;

    const std::string& getDefaultValue() const;

    Arg& setDefaultValue(std::string defaultValue);

    const std::function<void(const char *)>& getAction() const;

    Arg& setAction(std::function<void(const char *)> action);

    bool isRequired() const;

    Arg& setIsRequired(bool isRequired);

    std::string getFullName() const;

    std::string defaultValueToString() const;

  private:
    std::string d_shortForm;
    // Short form of the argument. Only one prefix char '-' is allowed,
    // followed by another char e.g '-t'. The length of the string must be
    // 2.

    std::string d_longForm;
    // Long form of the argument. Only two prefix chars '--' are allowed,
    // followed multiple chars e.g '--topic'.  The length of the string
    // must be at least 4.

    std::string d_metaVar;
    // A name for the argument in usage messages.  When the help message is
    // displayed, it is used to refer to the expected argument. e.g. -t
    // topic.  In this case, 'topic' is the metaVar

    std::vector<std::string> d_choices;
    // Allowed values for this argument.  If empty, any value is allowed.

    std::string d_description;

    std::string d_defaultValue;

    bool d_isRequired = false;
    // Optional by default

    std::function<void(const char *)> d_action;

    ArgMode d_mode = ArgMode::SINGLE_VALUE;
    // Single value by default
};

std::ostream& operator<<(std::ostream& stream, const Arg& arg);

inline Arg::Arg(const std::string& longForm, char shortForm)
{
    addShortForm(shortForm);
    addLongForm(longForm);
}

inline const std::string& Arg::getShortForm() const { return d_shortForm; }

inline void Arg::addShortForm(char shortForm)
{
    if (shortForm == '\0') {
        return;
    }

    if (!d_shortForm.empty()) {
        throw std::invalid_argument("Short form was specified more than once");
    }

    d_shortForm = std::string(1, PREFIX) + shortForm;
}

inline const std::string& Arg::getLongForm() const { return d_longForm; }

inline void Arg::addLongForm(const std::string& longForm)
{
    if (longForm.length() < 2) {
        throw std::invalid_argument(
                "Invalid long form. Should contain at least 2 characters.");
    }

    if (longForm[0] == PREFIX) {
        throw std::invalid_argument(
                std::string("Long form must not start with ") + PREFIX);
    }

    d_longForm = "--" + longForm;
}

inline const std::string& Arg::getMetaVar() const { return d_metaVar; }

inline Arg& Arg::setMetaVar(std::string metaVar)
{
    d_metaVar = std::move(metaVar);
    return *this;
}

inline const std::string& Arg::getDescription() const { return d_description; }

inline Arg& Arg::setDescription(std::string description)
{
    d_description = std::move(description);
    return *this;
}

inline ArgMode Arg::getMode() const { return d_mode; }

inline Arg& Arg::setMode(ArgMode mode)
{
    d_mode = mode;
    return *this;
}

inline Arg& Arg::setChoices(std::vector<std::string> choices)
{
    if (choices.empty()) {
        throw std::invalid_argument("Choices must not be empty");
    }

    d_choices = std::move(choices);

    return *this;
}

inline bool Arg::isValidChoice(const std::string& choice) const
{
    return d_choices.empty()
            || std::find(d_choices.begin(), d_choices.end(), choice)
            != d_choices.end();
}

inline const std::vector<std::string>& Arg::getChoices() const
{
    return d_choices;
}

inline const std::string& Arg::getDefaultValue() const
{
    return d_defaultValue;
}

inline Arg& Arg::setDefaultValue(std::string defaultValue)
{
    d_defaultValue = std::move(defaultValue);
    return *this;
}

inline const std::function<void(const char *)>& Arg::getAction() const
{
    return d_action;
}

inline Arg& Arg::setAction(std::function<void(const char *)> action)
{
    d_action = std::move(action);
    return *this;
}

inline bool Arg::isRequired() const { return d_isRequired; }

inline Arg& Arg::setIsRequired(bool isRequired)
{
    d_isRequired = isRequired;
    return *this;
}

inline std::string Arg::getFullName() const
{
    std::string forms;

    if (!d_shortForm.empty() && !d_longForm.empty()) {
        forms = d_shortForm + ", " + d_longForm;
    } else if (d_longForm.empty()) {
        forms = d_shortForm;
    } else if (d_shortForm.empty()) {
        forms = d_longForm;
    }

    if (d_mode != ArgMode::SINGLE_VALUE
            && d_mode != ArgMode::MULTIPLE_VALUES) {
        return forms;
    }

    if (!d_metaVar.empty()) {
        return forms + " " + d_metaVar;
    }

    // Use the capitalized word in longForm as metavar
    std::string metaVar = d_longForm.substr(2);

    std::transform(metaVar.begin(), metaVar.end(), metaVar.begin(), toupper);

    return forms + " " + metaVar;
}

inline std::string Arg::defaultValueToString() const
{
    return d_defaultValue.empty() ? "" : "(default: " + d_defaultValue + ")";
}

inline std::ostream& operator<<(std::ostream& stream, const Arg& arg)
{
    return stream << "[" << arg.getFullName() << "]";
}

class ArgRegistry {
    std::vector<Arg> d_args;
    std::map<std::string, std::vector<Arg>::size_type> d_argIndices;
    // A map of the short form and the long form of an argument to the
    // index of the argument in args.

  public:
    virtual ~ArgRegistry() { }
    virtual Arg& addArg(std::vector<Arg>::size_type& index,
            const std::string& longForm,
            char shortForm = '\0');
    // Adds an argument, return the index of the argument as well.

    const std::vector<Arg>& getArgs() const { return d_args; }
    const std::map<std::string, std::vector<Arg>::size_type>&
    getArgIndices() const
    {
        return d_argIndices;
    }
};

inline Arg& ArgRegistry::addArg(std::vector<Arg>::size_type& index,
        const std::string& longForm,
        char shortForm)
{
    index = d_args.size();
    d_args.emplace_back(longForm, shortForm);

    auto& arg = d_args.back();

    if (d_argIndices.find(arg.getShortForm()) != d_argIndices.end()
            || d_argIndices.find(arg.getLongForm()) != d_argIndices.end()) {
        throw std::invalid_argument(
                "Argument " + arg.getFullName() + " already exists.");
    }

    if (!arg.getShortForm().empty()) {
        d_argIndices[arg.getShortForm()] = index;
    }

    if (!arg.getLongForm().empty()) {
        d_argIndices[arg.getLongForm()] = index;
    }

    return arg;
}

class ArgGroup {
    // Defines a list of Args that conceptually belong to the same group. When
    // help is printed, these Args are grouped together.

    static const size_t DESCRIPTION_POSITION = 24u;
    // Align the description at this position, if the full name of the the
    // argument is longer than this position, wrap the description to next
    // line.

    std::string d_description;
    ArgRegistry *d_registry;
    std::vector<std::vector<Arg>::size_type> d_argIndices;
    // The indexes of the arguments in registry that belong to this group.

  public:
    ArgGroup(std::string description, ArgRegistry *registry);

    const std::string& getDescription() const;

    Arg& addArg(const std::string& longForm, char shortForm = '\0');
    // Adds an argument to this group.

    const std::vector<std::vector<Arg>::size_type>& getArgs() const;

    void printHelp(std::ostream& os) const;

  private:
    static std::string getArgFullDescription(const Arg& arg);
    // The full description of the argument that includes the description,
    // the default value and whether the argument can be specified multiple
    // times.

    static std::string getIndentation(size_t length = DESCRIPTION_POSITION);
};

std::ostream& operator<<(std::ostream& stream, const ArgGroup& argGroup);

inline ArgGroup::ArgGroup(std::string description, ArgRegistry *registry)
    : d_description(std::move(description))
    , d_registry(registry)
{
    if (d_description.empty()) {
        throw std::invalid_argument("Description must not be empty");
    }
}

inline const std::string& ArgGroup::getDescription() const
{
    return d_description;
}

inline Arg& ArgGroup::addArg(const std::string& longForm, char shortForm)
{
    std::vector<Arg>::size_type index;
    auto& arg = d_registry->addArg(index, longForm, shortForm);
    d_argIndices.push_back(index);
    return arg;
}

inline const std::vector<std::vector<Arg>::size_type>&
ArgGroup::getArgs() const
{
    return d_argIndices;
}

inline void ArgGroup::printHelp(std::ostream& os) const
{
    os << d_description << ":\n";

    if (d_argIndices.empty()) {
        return;
    }

    const auto& args = d_registry->getArgs();
    for (auto index : d_argIndices) {
        const auto& arg = args[index];
        const auto& fullName = arg.getFullName();
        os << "  " << fullName; // add 2 space indentation
        const std::string fullDescription = getArgFullDescription(arg);
        if (!fullDescription.empty()) {
            auto length = 2u + fullName.length();

            // Must have one space in-between
            if (length + 2u > DESCRIPTION_POSITION) {
                // Wrap to next line
                os << "\n";

                // This is a new line, reset length
                length = 0;
            }

            // Add indentation up to description position
            auto remainingSpaces = DESCRIPTION_POSITION - length;
            os << getIndentation(remainingSpaces);
            os << fullDescription;
        }

        os << "\n";
    }
}

inline std::ostream& operator<<(std::ostream& stream, const ArgGroup& argGroup)
{
    return stream << argGroup.getDescription() << "\n";
}

inline std::string ArgGroup::getArgFullDescription(const Arg& arg)
{
    std::ostringstream stream;

    const std::string& description = arg.getDescription();

    std::vector<std::string> descriptionLines;

    std::regex pattern("[\r\n]+");
    std::copy(std::sregex_token_iterator(
                      description.begin(), description.end(), pattern, -1),
            std::sregex_token_iterator(),
            std::back_inserter(descriptionLines));

    std::string firstLineDescription = descriptionLines[0];
    const std::string& defaultValue = arg.defaultValueToString();
    auto helpAndDefault = firstLineDescription + " " + defaultValue;

    // Trim
    helpAndDefault.erase(0, helpAndDefault.find_first_not_of(' '));
    helpAndDefault.erase(helpAndDefault.find_last_not_of(' ') + 1);

    stream << helpAndDefault;

    const auto& mode = arg.getMode();
    if (mode == ArgMode::MULTIPLE_NO_VALUE
            || mode == ArgMode::MULTIPLE_VALUES) {
        if (!helpAndDefault.empty()) {
            stream << ". ";
        }

        stream << "Can be specified multiple times.";
    }

    // If the description contains new lines, each line is indented.
    for (int i = 1; i < descriptionLines.size(); ++i) {
        stream << "\n" << getIndentation() << descriptionLines[i];
    }

    if (!arg.getChoices().empty()) {
        if (!stream.str().empty()) {
            // Print choices in the next line and indent.
            stream << "\n" << getIndentation();
        }

        stream << "(Choices: " << Utils::join(arg.getChoices(), ", ") << ")";
    }

    return stream.str();
}

inline std::string ArgGroup::getIndentation(size_t length)
{
    return std::string(length, ' ');
}

class ArgParser {
    // To set up ArgParser, call ArgParser::addArg to add a single argument or
    // ArgParser::addGroup to add a group of arguments. When parsing command
    // line arguments, first collects the values of each argument and invokes
    // the argument action if defined, then applies the default value if an
    // argument has defined a default value but received no values from the
    // command line.

  public:
    ArgParser(std::string title, std::string exampleClassName);

    Arg& addArg(const std::string& longForm, char shortForm = '\0');
    // Adds an argument which has short and/or long form and will be
    // grouped as 'general'.

    ArgGroup& addGroup(std::string description);

    void parse(int argc, const char **argv);

    void printHelp(std::ostream& os);

    void printHelp();

  private:
    std::string d_title;

    std::string d_exampleClassName;

    ArgRegistry d_registry;

    std::vector<ArgGroup> d_argGroups;

    std::map<std::vector<Arg>::size_type, std::vector<std::string>> d_values;
    // A map of the index of an argument to the values from the command
    // line.
};

inline ArgParser::ArgParser(std::string title, std::string exampleClassName)
    : d_title(std::move(title))
    , d_exampleClassName(std::move(exampleClassName))
{
    ArgGroup& argGroupHelp = addGroup("General");

    // Add general group with help argument by default.
    Arg& argHelp = argGroupHelp.addArg("help", 'h');

    const std::function<void(const char *)> actionFunction
            = [this](const char *) {
                  // Print help message and exit
                  printHelp();
                  std::exit(EXIT_SUCCESS);
              };

    argHelp.setDescription("Show this help message and exit")
            .setMode(ArgMode::NO_VALUE)
            .setAction(actionFunction);
}

inline Arg& ArgParser::addArg(const std::string& longForm, char shortForm)
{
    // Add the arg to the first group which is the general group.
    return d_argGroups[0].addArg(longForm, shortForm);
}

inline ArgGroup& ArgParser::addGroup(std::string description)
{
    d_argGroups.emplace_back(std::move(description), &d_registry);
    return d_argGroups.back();
}

inline void ArgParser::parse(int argc, const char **argv)
{
    const auto& args = d_registry.getArgs();
    const auto& argIndices = d_registry.getArgIndices();
    for (int i = 1; i < argc; ++i) {
        const char *option = argv[i];

        auto argIndicesIterator = argIndices.find(option);
        std::vector<Arg>::size_type index;
        if (argIndicesIterator != argIndices.end()) {
            index = argIndicesIterator->second;
        } else {
            throw std::invalid_argument(
                    "Unknown argument " + std::string(option));
        }

        if (index > (args.size() - 1)) {
            throw std::invalid_argument(
                    "Unknown argument " + std::string(option));
        }

        const auto& arg = args[index];
        std::vector<std::string>& argValues = d_values[index];

        const ArgMode argMode = arg.getMode();
        if ((argMode != ArgMode::MULTIPLE_NO_VALUE
                    && argMode != ArgMode::MULTIPLE_VALUES)
                && !argValues.empty()) {
            throw std::invalid_argument("Option [" + arg.getFullName()
                    + "] must be specified only once.");
        }

        const char *value = "";
        if (argMode == ArgMode::SINGLE_VALUE
                || argMode == ArgMode::MULTIPLE_VALUES) {
            if (i + 1 == argc) {
                throw std::invalid_argument("Option [" + arg.getFullName()
                        + "] is missing a value");
            }

            value = argv[++i];

            // Verify if the value is one of the choices
            if (!arg.isValidChoice(value)) {
                std::ostringstream out;
                std::vector<std::string> possibleChoices = arg.getChoices();
                std::copy(possibleChoices.begin(),
                        possibleChoices.end() - 1,
                        std::ostream_iterator<std::string>(out, ", "));
                out << arg.getChoices().back();

                throw std::invalid_argument("Option [" + arg.getFullName()
                        + "]: invalid choice '" + value + "' (choose from "
                        + out.str() + ")");
            }
        }

        // If an argument requires no value, add empty string to values to know
        // how many times it is specified, useful for -verbose
        argValues.emplace_back(value);

        // Invoke the action if it exists
        if (arg.getAction()) {
            arg.getAction()(value);
        }
    }

    // Validate required arguments and apply the default value if the argument
    // is not present but has a default value.
    for (std::vector<Arg>::size_type index = 0; index < args.size(); ++index) {
        auto valueIterator = d_values.find(index);
        if (valueIterator != d_values.end()) {
            continue;
        }

        // Throw if no values from command line or no default value if the
        // argument is required.
        const auto& arg = args[index];
        const std::string& defaultValue = arg.getDefaultValue();
        if (defaultValue.empty()) {
            if (arg.isRequired()) {
                throw std::invalid_argument(
                        "Missing option [" + arg.getFullName() + "]");
            }
        } else {
            // Apply the default value
            d_values[index].push_back(defaultValue);

            // Invoke the action if it exists
            if (arg.getAction()) {
                arg.getAction()(defaultValue.c_str());
            }
        }
    }
}

inline void ArgParser::printHelp(std::ostream& os)
{
    os << d_title << "\nUsage: " << d_exampleClassName
       << " [-h|--help] [options]\n";

    for (const auto& group : d_argGroups) {
        os << "\n";
        group.printHelp(os);
    }
}

inline void ArgParser::printHelp() { printHelp(std::cout); }
} // Close namespace BloombergLP

#endif // #ifndef INCLUDED_ARG_PARSER
