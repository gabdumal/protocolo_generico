#include <console_colors.hpp>

void setColor(TextColor textColor)
{
    cout << "\033[" << textColor << "m";
}

void resetColor() { cout << "\033[0m"; }