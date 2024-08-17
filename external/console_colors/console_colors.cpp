#include <console_colors.hpp>

using namespace std;

void setColor(ostream &outputStream = cout, TextColor textColor, TextColor backgroundColor)
{
    outputStream << "\033[" << textColor;
    if (backgroundColor != TextColor::BLACK)
    {
        outputStream << ";" << backgroundColor;
    }
    outputStream << "m";
}

void resetColor(ostream &outputStream = cout)
{
    outputStream << "\033[0m";
}
