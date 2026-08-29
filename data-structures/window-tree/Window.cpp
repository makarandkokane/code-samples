#include "Window.h"

#include "Screen.h"

#include <iostream>

// The root widget: a window frame with a name.
Window::Window(std::string name)
    : Widget(std::move(name))
{
}

// Logs and the outline show this node as a Window.
std::string Window::typeName() const
{
    return "Window";
}

// The window frame uses '=' so it reads differently from its panels.
void Window::paintSelf(Screen& screen, bool /*effectiveEnabled*/) const
{
    screen.drawFrame(m_x, m_y, m_width, m_height, name(), '=');
}

// Bubbled actions land here. The buttons only announce what happened; the
// window owns what "save", "reset" and "close" actually mean. Anything it
// does not know ("help") falls through and keeps bubbling.
bool Window::onAction(const std::string& action)
{
    if (action == "save")
    {
        int savedCount = 0;
        saveEverywhere(savedCount);
        if (savedCount == 0)
            std::cout << "    " << name() << ": nothing had unsaved edits\n";
        else
            std::cout << "    " << name() << ": saved; every box is clean again\n";

        return true;
    }

    if (action == "reset")
    {
        std::cout << "    " << name() << ": resets the form to its defaults\n";
        resetEverywhere();
        return true;
    }

    if (action == "close")
    {
        requestClose();
        return true;
    }

    return false;
}

// Close is a question, not a command: the postorder veto walk asks every
// widget below, and one unsaved box anywhere keeps the window open.
void Window::requestClose()
{
    std::cout << "-- can we close? postorder: every child answers before its parent\n";
    const bool agreed = canClose(true);

    if (agreed)
    {
        m_closed = true;
        std::cout << name() << " closes.\n";
    }
    else
    {
        std::cout << name() << " stays open: save the edits, or reset them.\n";
    }
}

// The main loop watches this to know when the demo is over.
bool Window::closed() const
{
    return m_closed;
}
