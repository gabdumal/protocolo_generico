#include "util.hpp"

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

void Util::printInformation(string header, string information,
                            ostream &output_stream,
                            PrettyConsole::Decoration header_decoration,
                            PrettyConsole::Decoration information_decoration) {
    ostringstream intermediate_stream;
    PrettyConsole::print(header, header_decoration, intermediate_stream);
    intermediate_stream << endl;

    PrettyConsole::setDecoration(intermediate_stream, information_decoration);
    std::istringstream information_stream(information);
    std::string line;
    while (std::getline(information_stream, line))
        intermediate_stream << PrettyConsole::tab << line << endl;
    PrettyConsole::resetDecoration(intermediate_stream);
    intermediate_stream << endl;

    output_stream << intermediate_stream.str();
}

string Util::getTextBetween(string text, optional<string> start,
                            optional<string> end) {
    size_t start_position = 0;
    if (start.has_value()) {
        start_position = text.find(start.value());
        if (start_position == string::npos) return "";
        start_position += start.value().length();
    }

    size_t end_position = text.length();
    if (end.has_value()) {
        end_position = text.find(end.value(), start_position);
        if (end_position == string::npos) return "";
    }

    return text.substr(start_position, end_position - start_position);
}

string Util::getLineContent(int line, string content) {
    istringstream content_stream(content);
    string line_content;
    for (int i = 0; i < line; i++) {
        getline(content_stream, line_content);
    }
    return line_content;
}
