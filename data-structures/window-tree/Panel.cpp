#include "Panel.h"

// A panel is a named container and nothing more.
Panel::Panel(std::string name)
    : Widget(std::move(name))
{
}

// Logs and the outline show this node as a Panel.
std::string Panel::typeName() const
{
    return "Panel";
}
