#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

class Screen;

// A widget's preferred footprint, decided in the measure pass.
struct Size
{
    int width  = 0;
    int height = 0;
};

// One node of the window tree. The base class owns the tree links and every
// recursive pass over them; a derived widget only decides how its one node
// measures, looks, and reacts. Containers stack their children vertically.
class Widget
{
public:
    explicit Widget(std::string name);
    virtual ~Widget() = default;

    Widget* addChild(std::unique_ptr<Widget> child);

    // the layout passes: measure reports upward (postorder), arrange seats
    // children downward (preorder)
    void measure(bool loud, int depth = 0);
    void arrange(int x, int y, bool loud, int depth = 0);

    // the paint pass: preorder, parents first so children draw on top
    void paint(Screen& screen, bool loud, bool parentEnabled = true, int depth = 0) const;

    // the veto pass: postorder, every child answers before its parent decides
    bool canClose(bool loud, int depth = 0) const;

    // one click end to end: hit test down, capture down, bubble the action up
    void click(int clickX, int clickY);

    // the smaller recursive walks
    void    printOutline(bool parentVisible = true, bool parentEnabled = true, int depth = 0) const;
    Widget* find(const std::string& widgetName);
    void    saveEverywhere(int& savedCount);
    void    resetEverywhere();

    // state the menu toggles, and the lookups the menu needs
    void               setVisible(bool visible);
    void               setEnabled(bool enabled);
    bool               isVisible() const;
    bool               isEnabled() const;
    bool               onScreen() const;
    int                centerX() const;
    int                centerY() const;
    const std::string& name() const;

protected:
    // what one node contributes to a pass; these defaults are the container
    // behaviors, leaf widgets override what differs
    virtual std::string                typeName() const;
    virtual Size                       preferredSize() const;
    virtual void                       paintSelf(Screen& screen, bool effectiveEnabled) const;
    virtual std::optional<std::string> onClick();
    virtual bool                       onAction(const std::string& action);
    virtual bool                       hasUnsavedWork() const;
    virtual void                       save();
    virtual void                       resetContent();
    virtual std::string                extraTag() const;

    std::string label() const;

    // geometry the passes fill in: the wish from measure, the rect from arrange
    Size m_preferred;
    int  m_x      = 0;
    int  m_y      = 0;
    int  m_width  = 0;
    int  m_height = 0;

private:
    Widget* hitTest(int pointX, int pointY);
    bool    contains(int pointX, int pointY) const;

    // tree links: a parent owns its children
    Widget*                              m_parent = nullptr;
    std::vector<std::unique_ptr<Widget>> m_children;

    // identity and the state the menu toggles
    std::string m_name;
    bool        m_visible = true;
    bool        m_enabled = true;
};
