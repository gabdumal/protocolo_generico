#include <console_colors.hpp>

using namespace std;

void setColor(ostream &outputStream = cout, Color color, Color backgroundColor)
{
    outputStream << "\033[" << color;
    if (backgroundColor != Color::BLACK)
    {
        outputStream << ";" << backgroundColor;
    }
    outputStream << "m";
}

void resetColor(ostream &outputStream = cout)
{
    outputStream << "\033[0m";
}
