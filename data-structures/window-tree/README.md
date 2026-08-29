# Window tree (C++17)

A tiny windowing system in a console. One window, panels inside it, buttons and a text box inside those, drawn as ASCII on a 38x16 screen. The point is the shape: a GUI is a tree, and every job this demo does (laying widgets out, painting them, dispatching a click, closing safely) is one small recursive walk over that tree. Each pass logs the order it visits the widgets in, so you can watch preorder and postorder do real work instead of just traversing.

```
  01234567890123456789012345678901234567
0
1   += Settings ==================+
2   | +- Profile ---------------+ |
3   | | Name : [Makarand______] | |
4   | +-------------------------+ |
5   | +- Actions -+               |
6   | | [ Save ]  |               |
7   | | [ Close ] |               |
8   | | [ Help ]  |               |
9   | +-----------+               |
0   | +- Advanced -+              |
1   | | ( Reset )  |              |
2   | +------------+              |
3   +=============================+
4
5
```

The form is a settings window: a Profile panel with a text box, an Actions panel with Save, Close, and a Help button nobody handles (on purpose), and an Advanced panel that starts disabled, which is why its Reset renders in parentheses. Plain C++17 and the standard library, no frameworks. The tree's shape is also the ownership: every widget holds its children as `std::unique_ptr`, so the parent-child structure and the memory management are the same picture.

```
See the passes                       Change things
  1  paint the screen                  6  type into the name box
  2  print the widget tree             7  show / hide a widget
  3  layout: measure + arrange         8  enable / disable a widget
                                       9  ask the window to close
Click
  4  click a widget by name            0  quit
  5  click at coordinates x y
```

## Layout: measure up, arrange down

A container cannot know its size until every child has reported its own, so measuring is postorder: in the log, children print above the parent that adds them up. A child cannot know where it sits until its parent has fixed its own origin, so arranging is preorder: parents print first. Menu item 3 runs both, and the two logs are mirror images of each other:

```
choice (m for the menu): 3
-- measure, postorder: every child reports before its parent adds up
      TextBox Name -> 23x1
    Panel Profile -> 27x3
      Button Save -> 8x1
      Button Close -> 9x1
      Button Help -> 8x1
    Panel Actions -> 13x5
      Button Reset -> 9x1
    Panel Advanced -> 14x3
  Window Settings -> 31x13
-- arrange, preorder: a parent fixes its origin, then seats its children
  Window Settings at (2,1)
    Panel Profile at (4,2)
      TextBox Name at (6,3)
    Panel Actions at (4,5)
      Button Save at (6,6)
      Button Close at (6,7)
      Button Help at (6,8)
    Panel Advanced at (4,10)
      Button Reset at (6,11)
```

Painting (menu item 1) is preorder again, and for a reason you can see: the parent draws first, the children draw on top of it, which is what puts widgets inside their window instead of behind it. The layout is live, too. Hide the Advanced panel (menu item 7) and the window remeasures to 31x10: three rows shorter, because a hidden subtree stops taking part in the measure pass.

## The story of a click

Menu items 4 and 5 dispatch a click the way real toolkits do. First a hit test descends the tree to find the deepest widget under the point. Then the capture phase walks the path from the root down, and any disabled ancestor may swallow the event before the target ever sees it:

```
choice (m for the menu): 4
widget name: Reset
click at (10,11)
  hit test: the point lands on Button Reset
  capture (down): Settings -> Advanced: disabled, the click is swallowed
```

Enable Advanced (menu item 8) and click again, and the second half runs: the target raises an action, and the action bubbles up through its ancestors until one of them knows what it means. The button does not know what resetting is. Only the window does:

```
click at (10,11)
  hit test: the point lands on Button Reset
  capture (down): Settings -> Advanced -> Reset: clear, the target may react
  target: Reset raises "reset"
  bubble (up): "reset" passes Advanced
    Settings: resets the form to its defaults
    Name: text back to "Makarand"
  bubble (up): "reset" was handled by Settings
```

And when nobody along the way understands the action, it falls off the top, exactly like an unhandled event in a browser:

```
click at (10,8)
  hit test: the point lands on Button Help
  capture (down): Settings -> Actions -> Help: clear, the target may react
  target: Help raises "help"
  bubble (up): "help" passes Actions
  bubble (up): "help" passes Settings
  bubble (up): "help" reached the top unhandled; dropped
```

## The close veto

The reset above left the Name box with unsaved edits, and the screen marks it with a star: `Name*: [Makarand______]`. Ask the window to close now (menu item 9, or the Close button) and the last traversal runs: a postorder walk where every child answers before its parent decides, and one unsaved box anywhere below is enough to keep the window open.

```
-- can we close? postorder: every child answers before its parent
      TextBox Name: unsaved edits here, vetoes
    Panel Profile: a child said no
      Button Save: nothing unsaved
      Button Close: nothing unsaved
      Button Help: nothing unsaved
    Panel Actions: nothing unsaved down here
      Button Reset: nothing unsaved
    Panel Advanced: nothing unsaved down here
  Window Settings: a child said no
Settings stays open: save the edits, or reset them.
```

Click Save, watch "save" bubble up to the window, watch the star disappear, and the same walk then agrees:

```
  target: Close raises "close"
  bubble (up): "close" passes Actions
-- can we close? postorder: every child answers before its parent
      TextBox Name: nothing unsaved
    Panel Profile: nothing unsaved down here
    ...
  Window Settings: nothing unsaved down here
Settings closes.
```

The walk deliberately asks everything instead of stopping at the first no, so the log always shows the complete postorder pass. It asks hidden widgets too: invisible unsaved work is still unsaved work.

## Code tour

| File | Role |
|------|------|
| `Widget.h` / `Widget.cpp` | The tree and every walk over it: measure, arrange, paint, the close veto, the click dispatch, and the lookups. |
| `Window`, `Panel`, `Button`, `TextBox` | One node type each. A derived widget only decides how its one node measures, looks, and reacts; the traversals are inherited. |
| `Screen.h` / `Screen.cpp` | The ASCII canvas: a character grid with clipping, frames, and the coordinate rulers clicks are aimed with. |
| `main.cpp` | The menu, the fixed demo tree, and one small handler per item. |

Every pass is one function you can read on its own:

| What | Function | The traversal |
|------|----------|---------------|
| measure | `Widget::measure` | postorder: children report, parents combine |
| arrange | `Widget::arrange` | preorder: parents seat their children |
| paint | `Widget::paint` | preorder: parents first, children on top |
| close veto | `Widget::canClose` | postorder: children answer, parents decide |
| click dispatch | `Widget::click` | hit test down one path, capture down, bubble up |
| outline, search | `Widget::printOutline`, `Widget::find` | preorder |

There is no inorder pass here, and that is the honest part: inorder means left, node, right, so it needs a binary tree, and a window tree gives every container as many children as it has widgets.

## Build and run

```
cmake -S . -B build
cmake --build build --config Release
build\Release\window-tree.exe      (Windows)
./build/window-tree                (elsewhere)
```

## Where the mechanisms come from

Nothing here is invented and that is the point: capture and bubble are the DOM's own event model, measure-then-arrange is how every layout engine works from Qt to Android, and the close veto is the "unsaved changes" dialog every editor owes you. The code is written fresh for this repo; the mechanisms are the industry's. I wrote about the moment this way of seeing pays off in [Look for hierarchies in the data](https://makarandkokane.github.io/articles/software-development-tip-hierarchies.html), the story of an event that had to travel a window tree that nobody had noticed was a tree. This demo is that story, runnable.
