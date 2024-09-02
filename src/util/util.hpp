#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <iostream>
#include <pretty_console.hpp>

using namespace std;

namespace Util {
    void printInformation(string header, string information,
                          ostream &output_stream = cout,
                          PrettyConsole::Decoration header_decoration =
                              PrettyConsole::Decoration(),
                          PrettyConsole::Decoration information_decoration =
                              PrettyConsole::Decoration());

    string getFormattedBool(bool value);

    string getTextBetween(string text, optional<string> start,
                          optional<string> end);
    string getLineContent(int line, string content);
}  // namespace Util

#endif  // UTIL_HPP_
