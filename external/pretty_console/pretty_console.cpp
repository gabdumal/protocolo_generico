#include "pretty_console.hpp"

using namespace std;

void PrettyConsole::setForegroundColor(ostream &output_stream, Color foreground_color)
{
    output_stream << "\033[" << foreground_color << "m";
}

void PrettyConsole::resetForegroundColor(ostream &output_stream)
{
    setForegroundColor(output_stream);
}

void PrettyConsole::setBackgroundColor(ostream &output_stream, Color background_color)
{
    output_stream << "\033[" << background_color + 10 << "m";
}

void PrettyConsole::resetBackgroundColor(ostream &output_stream)
{
    setBackgroundColor(output_stream);
}

void PrettyConsole::setFormat(ostream &output_stream, Format format)
{
    output_stream << "\033[" << format << "m";
}

void PrettyConsole::resetFormat(ostream &output_stream)
{
    setFormat(output_stream);
}

void PrettyConsole::setDecoration(ostream &output_stream, Decoration decoration)
{
    setFormat(output_stream, decoration.format);
    setBackgroundColor(output_stream, decoration.background_color);
    setForegroundColor(output_stream, decoration.foreground_color);
}

void PrettyConsole::resetDecoration(ostream &output_stream)
{
    resetForegroundColor(output_stream);
    resetBackgroundColor(output_stream);
    resetFormat(output_stream);
}

void PrettyConsole::print(string message, Decoration decoration, ostream &output_stream)
{
    setFormat(output_stream, decoration.format);
    setBackgroundColor(output_stream, decoration.background_color);
    setForegroundColor(output_stream, decoration.foreground_color);
    output_stream << message;
    resetFormat(output_stream);
    resetBackgroundColor(output_stream);
    resetForegroundColor(output_stream);
}
