#pragma once

#include "Widget.h"

// A leaf that turns a click into a named action for the tree above to
// interpret; the button itself has no idea what "save" means.
class Button : public Widget
{
public:
    Button(std::string name, std::string action);

protected:
    std::string                typeName() const override;
    Size                       preferredSize() const override;
    void                       paintSelf(Screen& screen, bool effectiveEnabled) const override;
    std::optional<std::string> onClick() override;

private:
    std::string displayText(bool effectiveEnabled) const;

    // the action this button raises when clicked
    std::string m_action;
};
