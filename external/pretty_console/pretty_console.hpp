#ifndef PRETTY_CONSOLE_HPP_
#define PRETTY_CONSOLE_HPP_

#include <iostream>

using namespace std;

class PrettyConsole
{
public:
    static constexpr const char *tab = "    ";

    enum Color
    {
        BLACK = 30,
        RED = 31,
        GREEN = 32,
        YELLOW = 33,
        BLUE = 34,
        MAGENTA = 35,
        CYAN = 36,
        WHITE = 37,
        DEFAULT = 39,
        BRIGHT_BLACK = 90,
        BRIGHT_RED = 91,
        BRIGHT_GREEN = 92,
        BRIGHT_YELLOW = 93,
        BRIGHT_BLUE = 94,
        BRIGHT_MAGENTA = 95,
        BRIGHT_CYAN = 96,
        BRIGHT_WHITE = 97,
        BRIGHT_DEFAULT = 99
    };

    enum Format
    {
        NONE = 0,
        BOLD = 1,
        DIM = 2,
        UNDERLINED = 4,
        BLINK = 5,
        REVERSE = 7,
        HIDDEN = 8
    };

    struct Decoration
    {
        Color foreground_color;
        Color background_color;
        Format format;

        Decoration(Color foreground_color = Color::DEFAULT, Color background_color = Color::DEFAULT, Format format = Format::NONE) : foreground_color(foreground_color), background_color(background_color), format(format) {}
    };

    static void setForegroundColor(ostream &output_stream, Color foreground_color = Color::DEFAULT);
    static void resetForegroundColor(ostream &output_stream);

    static void setBackgroundColor(ostream &output_stream, Color background_color = Color::DEFAULT);
    static void resetBackgroundColor(ostream &output_stream);

    static void setFormat(ostream &output_stream, Format format = Format::NONE);
    static void resetFormat(ostream &output_stream);

    static void setDecoration(ostream &output_stream, Decoration decoration);
    static void resetDecoration(ostream &output_stream);

    static void print(string message, Decoration decoration, ostream &output_stream);
};

#endif // PRETTY_CONSOLE_HPP_
