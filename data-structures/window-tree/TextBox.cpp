#include "TextBox.h"

#include "Screen.h"

#include <iostream>

namespace
{
// how many characters of text the field shows
constexpr int kFieldWidth = 14;
}

// A text box starts clean, holding its default text.
TextBox::TextBox(std::string name, std::string text)
    : Widget(std::move(name)),
      m_text(text),
      m_defaultText(std::move(text))
{
}

// The menu types through this; any edit is unsaved until saved.
void TextBox::setText(std::string text)
{
    m_text  = std::move(text);
    m_dirty = true;
}

// Logs and the outline show this node as a TextBox.
std::string TextBox::typeName() const
{
    return "TextBox";
}

// A text box is one row: name, unsaved marker, and the fixed-width field.
Size TextBox::preferredSize() const
{
    Size size;
    size.width  = static_cast<int>(displayText(true).size());
    size.height = 1;
    return size;
}

// One line on the canvas.
void TextBox::paintSelf(Screen& screen, bool effectiveEnabled) const
{
    screen.putText(m_x, m_y, displayText(effectiveEnabled));
}

// A click puts the cursor here; there is no action to bubble.
std::optional<std::string> TextBox::onClick()
{
    std::cout << "    " << name() << ": the cursor lands in the box\n";
    return std::nullopt;
}

// The veto hook: unsaved edits anywhere keep the window open.
bool TextBox::hasUnsavedWork() const
{
    return m_dirty;
}

// The bubbled "save" reaches every box through the tree walk.
void TextBox::save()
{
    if (m_dirty)
        std::cout << "    " << name() << ": edits saved\n";

    m_dirty = false;
}

// The bubbled "reset" restores the default text, which itself counts as an
// edit until the next save.
void TextBox::resetContent()
{
    m_text  = m_defaultText;
    m_dirty = true;
    std::cout << "    " << name() << ": text back to \"" << m_defaultText << "\"\n";
}

// The outline marks a box that would veto a close.
std::string TextBox::extraTag() const
{
    if (m_dirty)
        return "  (unsaved edits)";

    return "";
}

// Composes the one-line rendering. The star marks unsaved edits, and
// parentheses replace the field's brackets while the box is disabled.
std::string TextBox::displayText(bool effectiveEnabled) const
{
    std::string field = m_text;
    field.resize(kFieldWidth, '_');

    const std::string marker = m_dirty ? "*" : " ";
    if (effectiveEnabled)
        return name() + marker + ": [" + field + "]";

    return name() + marker + ": (" + field + ")";
}
