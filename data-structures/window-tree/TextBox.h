#pragma once

#include "Widget.h"

// A leaf holding editable text. It remembers whether it has unsaved edits,
// shows a star while it does, and vetoes the window's close until saved.
class TextBox : public Widget
{
public:
    TextBox(std::string name, std::string text);

    void setText(std::string text);

protected:
    std::string                typeName() const override;
    Size                       preferredSize() const override;
    void                       paintSelf(Screen& screen, bool effectiveEnabled) const override;
    std::optional<std::string> onClick() override;
    bool                       hasUnsavedWork() const override;
    void                       save() override;
    void                       resetContent() override;
    std::string                extraTag() const override;

private:
    std::string displayText(bool effectiveEnabled) const;

    // the edited state: the text, the default reset returns to, and whether
    // there are edits newer than the last save
    std::string m_text;
    std::string m_defaultText;
    bool        m_dirty = false;
};
