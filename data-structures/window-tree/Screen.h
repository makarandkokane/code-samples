#pragma once

#include <string>
#include <vector>

// The ASCII canvas the widgets paint onto: a fixed grid of characters with
// clipping, printed with coordinate rulers so clicks can be aimed by eye.
class Screen
{
public:
    Screen(int width, int height);

    void clear();
    void putText(int x, int y, const std::string& text);
    void drawFrame(int x, int y, int width, int height, const std::string& title,
                   char horizontalChar);
    void print() const;

private:
    // the grid, one full-width string per row
    int                      m_width  = 0;
    int                      m_height = 0;
    std::vector<std::string> m_rows;
};
