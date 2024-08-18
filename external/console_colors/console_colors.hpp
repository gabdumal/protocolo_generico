#ifndef _CONSOLE_COLORS_HPP
#define _CONSOLE_COLORS_HPP

#include <iostream>

using namespace std;

enum Color
{
    BLACK = 30,
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    WHITE = 37
};

void setColor(ostream &outputStream, Color color = Color::BLACK, Color backgroundColor = Color::BLACK);

void resetColor(
    ostream &outputStream);

#endif // _CONSOLE_COLORS_HPP