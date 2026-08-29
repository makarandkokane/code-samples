#include "Button.h"

#include "Screen.h"

// A button knows its label (the name) and the action it raises.
Button::Button(std::string name, std::string action)
    : Widget(std::move(name)),
      m_action(std::move(action))
{
}

// Logs and the outline show this node as a Button.
std::string Button::typeName() const
{
    return "Button";
}

// A button is exactly as big as its rendering.
Size Button::preferredSize() const
{
    Size size;
    size.width  = static_cast<int>(displayText(true).size());
    size.height = 1;
    return size;
}

// One line on the canvas.
void Button::paintSelf(Screen& screen, bool effectiveEnabled) const
{
    screen.putText(m_x, m_y, displayText(effectiveEnabled));
}

// The click reaction: raise the action and let it bubble.
std::optional<std::string> Button::onClick()
{
    return m_action;
}

// Brackets while clickable, parentheses while disabled.
std::string Button::displayText(bool effectiveEnabled) const
{
    if (effectiveEnabled)
        return "[ " + name() + " ]";

    return "( " + name() + " )";
}
