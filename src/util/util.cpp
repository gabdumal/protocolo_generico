#include "util.hpp"

#include <iostream>
#include <sstream>

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

string Util::getLineContent(int line, string content) {
    istringstream content_stream(content);
    string line_content;
    for (int i = 0; i < line; i++) {
        getline(content_stream, line_content);
    }
    return line_content;
}
