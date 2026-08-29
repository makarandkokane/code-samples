#pragma once

#include "Widget.h"

// The root of the tree. Bubbled actions land here, because only the window
// knows what "save", "reset" and "close" mean; closing is guarded by the
// postorder veto walk.
class Window : public Widget
{
public:
    explicit Window(std::string name);

    void requestClose();
    bool closed() const;

protected:
    std::string typeName() const override;
    void        paintSelf(Screen& screen, bool effectiveEnabled) const override;
    bool        onAction(const std::string& action) override;

private:
    // set once the veto walk agrees; the main loop exits on it
    bool m_closed = false;
};
