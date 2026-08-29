#include "Screen.h"

#include <iostream>

// Builds a blank canvas of the given size.
Screen::Screen(int width, int height)
    : m_width(width),
      m_height(height)
{
    clear();
}

// Wipes every cell back to a space.
void Screen::clear()
{
    m_rows.assign(m_height, std::string(m_width, ' '));
}

// Writes text at (x, y), clipping whatever falls outside the canvas.
void Screen::putText(int x, int y, const std::string& text)
{
    if (y < 0 || y >= m_height)
        return;

    for (int i = 0; i < static_cast<int>(text.size()); ++i)
    {
        const int column = x + i;
        if (column >= 0 && column < m_width)
            m_rows[y][column] = text[i];
    }
}

// Draws a one-character border with the title worked into the top edge.
// horizontalChar tells window frames ('=') and panel frames ('-') apart.
void Screen::drawFrame(int x, int y, int width, int height, const std::string& title,
                       char horizontalChar)
{
    if (width < 2 || height < 2)
        return;

    // the top and bottom edges
    const std::string edge = "+" + std::string(width - 2, horizontalChar) + "+";
    putText(x, y, edge);
    putText(x, y + height - 1, edge);

    // the side edges
    for (int row = y + 1; row < y + height - 1; ++row)
    {
        putText(x, row, "|");
        putText(x + width - 1, row, "|");
    }

    // the title, clipped so it never overwrites the corners
    std::string decorated = " " + title + " ";
    const int   room      = width - 4;
    if (static_cast<int>(decorated.size()) > room)
        decorated.resize(room);

    putText(x + 2, y, decorated);
}

// Prints the canvas with a column ruler on top and row numbers down the
// side (both modulo 10), so a click can be aimed straight off the printout.
void Screen::print() const
{
    // the column ruler
    std::string ruler = "  ";
    for (int column = 0; column < m_width; ++column)
    {
        ruler += static_cast<char>('0' + column % 10);
    }
    std::cout << ruler << "\n";

    // the rows, right-trimmed so transcripts carry no trailing spaces
    for (int row = 0; row < m_height; ++row)
    {
        std::string line = m_rows[row];
        line.erase(line.find_last_not_of(' ') + 1);
        std::cout << row % 10 << " " << line << "\n";
    }
}
