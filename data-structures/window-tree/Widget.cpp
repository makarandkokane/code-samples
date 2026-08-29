#include "Widget.h"

#include "Screen.h"

#include <algorithm>
#include <iostream>

namespace
{
// frame geometry: a border column plus a padding column left and right, and
// a border row above and below the stacked children
constexpr int kContentInsetX = 2;
constexpr int kContentInsetY = 1;

// the room "+- title -+" needs around the title text itself
constexpr int kTitleSlack = 6;

// Indentation for the pass logs: two spaces per tree level, starting indented
// so the unindented banner lines stand out.
std::string indent(int depth)
{
    return std::string(2 * depth + 2, ' ');
}
}

// A widget is born with a name; everything else is wired by addChild and
// filled in by the passes.
Widget::Widget(std::string name)
    : m_name(std::move(name))
{
}

// Hands the child to this widget, which owns it from here on. Returns the
// raw pointer so the caller can keep a handle for lookups.
Widget* Widget::addChild(std::unique_ptr<Widget> child)
{
    child->m_parent = this;
    m_children.push_back(std::move(child));
    return m_children.back().get();
}

// Measure, postorder: every visible child reports its footprint first, and
// only then can this widget compute its own (a container adds up children,
// a leaf reports its intrinsic size).
void Widget::measure(bool loud, int depth)
{
    if (!m_visible)
        return;

    for (const auto& child : m_children)
    {
        child->measure(loud, depth + 1);
    }

    m_preferred = preferredSize();
    if (loud)
    {
        std::cout << indent(depth) << label() << " -> " << m_preferred.width << "x"
                  << m_preferred.height << "\n";
    }
}

// Arrange, preorder: this widget fixes its own rectangle first, then seats
// each visible child inside it, top to bottom.
void Widget::arrange(int x, int y, bool loud, int depth)
{
    if (!m_visible)
        return;

    m_x      = x;
    m_y      = y;
    m_width  = m_preferred.width;
    m_height = m_preferred.height;
    if (loud)
        std::cout << indent(depth) << label() << " at (" << x << "," << y << ")\n";

    int childY = y + kContentInsetY;
    for (const auto& child : m_children)
    {
        if (!child->m_visible)
            continue;

        child->arrange(x + kContentInsetX, childY, loud, depth + 1);
        childY += child->m_preferred.height;
    }
}

// Paint, preorder: this widget draws itself first, then its children draw
// on top of it. A hidden widget removes its whole subtree from the screen,
// and a disabled one dims everything below it.
void Widget::paint(Screen& screen, bool loud, bool parentEnabled, int depth) const
{
    if (!m_visible)
    {
        if (loud)
            std::cout << indent(depth) << label() << ": hidden, its whole subtree is skipped\n";

        return;
    }

    const bool effectiveEnabled = parentEnabled && m_enabled;
    if (loud)
        std::cout << indent(depth) << label() << "\n";

    paintSelf(screen, effectiveEnabled);

    for (const auto& child : m_children)
    {
        child->paint(screen, loud, effectiveEnabled, depth + 1);
    }
}

// The veto pass, postorder: ask every child, then this widget itself. The
// walk deliberately visits everything instead of stopping at the first no,
// so the log shows the complete postorder order. Hidden widgets are asked
// too: invisible unsaved work is still unsaved work.
bool Widget::canClose(bool loud, int depth) const
{
    bool childrenAgree = true;
    for (const auto& child : m_children)
    {
        const bool childAgrees = child->canClose(loud, depth + 1);
        childrenAgree          = childrenAgree && childAgrees;
    }

    const bool selfAgrees = !hasUnsavedWork();
    if (loud)
    {
        // one verdict line per widget, printed in its postorder position
        std::string verdict = "nothing unsaved";
        if (!selfAgrees)
            verdict = "unsaved edits here, vetoes";
        else if (!childrenAgree)
            verdict = "a child said no";
        else if (!m_children.empty())
            verdict = "nothing unsaved down here";

        std::cout << indent(depth) << label() << ": " << verdict << "\n";
    }

    return childrenAgree && selfAgrees;
}

// One click, end to end. Hit testing descends the tree (a preorder that
// follows just one path), the capture phase walks root to target with every
// ancestor free to swallow the event, the target reacts, and whatever action
// it raises bubbles back up until an ancestor knows what it means.
void Widget::click(int clickX, int clickY)
{
    std::cout << "click at (" << clickX << "," << clickY << ")\n";

    // hit test: find the deepest visible widget under the point
    Widget* target = hitTest(clickX, clickY);
    if (!target)
    {
        std::cout << "  hit test: nothing there\n";
        return;
    }

    std::cout << "  hit test: the point lands on " << target->label() << "\n";

    // the root-to-target path the capture phase walks
    std::vector<Widget*> path;
    for (Widget* widget = target; widget; widget = widget->m_parent)
    {
        path.push_back(widget);
    }
    std::reverse(path.begin(), path.end());

    // capture, downward: the first disabled widget on the path swallows the
    // click before it ever reaches the target
    std::string   route;
    const Widget* blocker = nullptr;
    for (const Widget* widget : path)
    {
        if (!route.empty())
            route += " -> ";

        route += widget->m_name;
        if (!widget->m_enabled)
        {
            blocker = widget;
            break;
        }
    }
    if (blocker)
    {
        std::cout << "  capture (down): " << route << ": disabled, the click is swallowed\n";
        return;
    }

    std::cout << "  capture (down): " << route << ": clear, the target may react\n";

    // the target's own reaction
    const std::optional<std::string> action = target->onClick();
    if (!action)
    {
        std::cout << "  target: " << target->m_name << " raised no action; nothing bubbles\n";
        return;
    }

    std::cout << "  target: " << target->m_name << " raises \"" << *action << "\"\n";

    // bubble, upward: climb toward the root until an ancestor handles it
    for (Widget* widget = target->m_parent; widget; widget = widget->m_parent)
    {
        if (widget->onAction(*action))
        {
            std::cout << "  bubble (up): \"" << *action << "\" was handled by " << widget->m_name
                      << "\n";
            return;
        }

        std::cout << "  bubble (up): \"" << *action << "\" passes " << widget->m_name << "\n";
    }

    std::cout << "  bubble (up): \"" << *action << "\" reached the top unhandled; dropped\n";
}

// The model itself, preorder: every widget, visible or not, with its
// rectangle and the state tags that matter.
void Widget::printOutline(bool parentVisible, bool parentEnabled, int depth) const
{
    const bool shown            = parentVisible && m_visible;
    const bool effectiveEnabled = parentEnabled && m_enabled;

    // the geometry, meaningful only while the widget is on screen
    std::string geometry;
    if (shown)
    {
        geometry = "  at (" + std::to_string(m_x) + "," + std::to_string(m_y) + ") " +
                   std::to_string(m_width) + "x" + std::to_string(m_height);
    }

    // the state tags
    std::string tags;
    if (!m_visible)
        tags += "  (hidden)";
    else if (!parentVisible)
        tags += "  (hidden with its parent)";

    if (!m_enabled)
        tags += "  (disabled)";
    else if (!effectiveEnabled)
        tags += "  (disabled through its parent)";

    tags += extraTag();
    std::cout << indent(depth) << label() << geometry << tags << "\n";

    for (const auto& child : m_children)
    {
        child->printOutline(shown, effectiveEnabled, depth + 1);
    }
}

// Preorder search by name; the first match wins (names are unique here).
Widget* Widget::find(const std::string& widgetName)
{
    if (m_name == widgetName)
        return this;

    for (const auto& child : m_children)
    {
        Widget* found = child->find(widgetName);
        if (found)
            return found;
    }

    return nullptr;
}

// Fans "save" out to the whole subtree; each widget with unsaved work
// counts itself before cleaning up.
void Widget::saveEverywhere(int& savedCount)
{
    if (hasUnsavedWork())
        ++savedCount;

    save();
    for (const auto& child : m_children)
    {
        child->saveEverywhere(savedCount);
    }
}

// Fans "reset" out to the whole subtree.
void Widget::resetEverywhere()
{
    resetContent();
    for (const auto& child : m_children)
    {
        child->resetEverywhere();
    }
}

// The menu's visibility toggle.
void Widget::setVisible(bool visible)
{
    m_visible = visible;
}

// The menu's enabled toggle.
void Widget::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

// Whether this widget itself is visible (ancestors not considered).
bool Widget::isVisible() const
{
    return m_visible;
}

// Whether this widget itself is enabled (ancestors not considered).
bool Widget::isEnabled() const
{
    return m_enabled;
}

// True while this widget and every ancestor above it are visible.
bool Widget::onScreen() const
{
    for (const Widget* widget = this; widget; widget = widget->m_parent)
    {
        if (!widget->m_visible)
            return false;
    }

    return true;
}

// Where a by-name click aims: the middle of the arranged rectangle.
int Widget::centerX() const
{
    return m_x + m_width / 2;
}

// The vertical middle of the arranged rectangle.
int Widget::centerY() const
{
    return m_y + m_height / 2;
}

// The widget's name, as the menu and the logs use it.
const std::string& Widget::name() const
{
    return m_name;
}

// The type shown before the name in logs; derived widgets override.
std::string Widget::typeName() const
{
    return "Widget";
}

// A container's footprint: the widest child plus the frame, the children
// stacked, and never narrower than its own title.
Size Widget::preferredSize() const
{
    // stack the visible children
    int contentWidth  = 0;
    int contentHeight = 0;
    for (const auto& child : m_children)
    {
        if (!child->m_visible)
            continue;

        contentWidth = std::max(contentWidth, child->m_preferred.width);
        contentHeight += child->m_preferred.height;
    }

    // the frame must also fit its own title
    const int titleWidth = static_cast<int>(m_name.size()) + kTitleSlack;

    Size size;
    size.width  = std::max(contentWidth + 2 * kContentInsetX, titleWidth);
    size.height = contentHeight + 2 * kContentInsetY;
    return size;
}

// A container shows itself as a titled panel frame.
void Widget::paintSelf(Screen& screen, bool /*effectiveEnabled*/) const
{
    screen.drawFrame(m_x, m_y, m_width, m_height, m_name, '-');
}

// A plain widget has no click reaction of its own.
std::optional<std::string> Widget::onClick()
{
    return std::nullopt;
}

// A plain widget understands no bubbled actions.
bool Widget::onAction(const std::string& /*action*/)
{
    return false;
}

// Nothing to lose by default; TextBox overrides.
bool Widget::hasUnsavedWork() const
{
    return false;
}

// Nothing to save by default; TextBox overrides.
void Widget::save()
{
}

// Nothing to reset by default; TextBox overrides.
void Widget::resetContent()
{
}

// No extra outline tag by default; TextBox marks its unsaved edits.
std::string Widget::extraTag() const
{
    return "";
}

// "Panel Actions", "Button Save": how every log names a widget.
std::string Widget::label() const
{
    return typeName() + " " + m_name;
}

// The descent behind hit testing: does the point land on this widget, and
// if so, on which descendant. Hidden widgets are not on screen and cannot
// be hit; disabled ones can, and the capture phase decides their fate.
Widget* Widget::hitTest(int pointX, int pointY)
{
    if (!m_visible || !contains(pointX, pointY))
        return nullptr;

    for (const auto& child : m_children)
    {
        Widget* hit = child->hitTest(pointX, pointY);
        if (hit)
            return hit;
    }

    return this;
}

// Point-in-rectangle, on the arranged screen coordinates.
bool Widget::contains(int pointX, int pointY) const
{
    const bool insideX = pointX >= m_x && pointX < m_x + m_width;
    const bool insideY = pointY >= m_y && pointY < m_y + m_height;
    return insideX && insideY;
}
