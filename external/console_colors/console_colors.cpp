#include <console_colors.hpp>
#include <sstream>

using namespace std;

void ConsoleColors::setColor(ostream &outputStream = cout, Color color, Color backgroundColor)
{
    outputStream << "\033[" << color << ";" << backgroundColor + 10 << "m";
}

void ConsoleColors::resetColor(ostream &outputStream = cout)
{
    setColor(outputStream, Color::DEFAULT, Color::DEFAULT);
}

void ConsoleColors::printInformation(string header, string information, ostream &outputStream, Color headerColor, Color headerBackgroundColor, Color informationColor, Color informationBackgroundColor)
{
    ostringstream intermediateStream;
    setColor(intermediateStream, headerColor, headerBackgroundColor);
    intermediateStream << header;
    resetColor(intermediateStream);
    intermediateStream << endl;

    setColor(intermediateStream, informationColor, informationBackgroundColor);
    std::istringstream informationStream(information);
    std::string line;
    while (std::getline(informationStream, line))
        intermediateStream << tab << line << endl;
    resetColor(intermediateStream);
    intermediateStream << endl;

    outputStream << intermediateStream.str();
}
