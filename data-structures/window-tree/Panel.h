#pragma once

#include "Widget.h"

// A titled grouping frame: pure container, everything it does is inherited.
class Panel : public Widget
{
public:
    explicit Panel(std::string name);

protected:
    std::string typeName() const override;
};
