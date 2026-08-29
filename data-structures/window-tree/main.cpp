#include "Button.h"
#include "Panel.h"
#include "Screen.h"
#include "TextBox.h"
#include "Widget.h"
#include "Window.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

namespace
{
// the canvas and where the window sits on it
constexpr int kScreenWidth  = 38;
constexpr int kScreenHeight = 16;
constexpr int kWindowX      = 2;
constexpr int kWindowY      = 1;

// The demo tree, plus the typed handles the menu needs.
struct Ui
{
    std::unique_ptr<Window> window;
    TextBox*                nameBox = nullptr;
};

// Builds the fixed demo tree: a settings window holding a profile panel,
// an actions panel, and an advanced panel that starts disabled.
Ui buildUi()
{
    Ui ui;
    ui.window = std::make_unique<Window>("Settings");

    // the profile panel and its text box
    auto profile = std::make_unique<Panel>("Profile");
    auto nameBox = std::make_unique<TextBox>("Name", "Makarand");
    ui.nameBox   = nameBox.get();
    profile->addChild(std::move(nameBox));
    ui.window->addChild(std::move(profile));

    // the actions: Save and Close the window understands, Help nobody does
    auto actions = std::make_unique<Panel>("Actions");
    actions->addChild(std::make_unique<Button>("Save", "save"));
    actions->addChild(std::make_unique<Button>("Close", "close"));
    actions->addChild(std::make_unique<Button>("Help", "help"));
    ui.window->addChild(std::move(actions));

    // the advanced panel starts disabled, so clicks aimed inside it are
    // swallowed on the way down
    auto advanced = std::make_unique<Panel>("Advanced");
    advanced->addChild(std::make_unique<Button>("Reset", "reset"));
    advanced->setEnabled(false);
    ui.window->addChild(std::move(advanced));

    return ui;
}

// Quiet layout plus a repaint: the screen the user sees after every change.
void refresh(Widget& root, Screen& screen)
{
    root.measure(false);
    root.arrange(kWindowX, kWindowY, false);
    screen.clear();
    root.paint(screen, false);
    screen.print();
}

// The menu, grouped by what each item demonstrates.
void printMenu()
{
    std::cout << "\n"
                 "See the passes\n"
                 "  1  paint the screen, logging the order\n"
                 "  2  print the widget tree\n"
                 "  3  layout, loud: measure then arrange\n"
                 "\n"
                 "Click\n"
                 "  4  click a widget by name\n"
                 "  5  click at coordinates x y\n"
                 "\n"
                 "Change things\n"
                 "  6  type into the name box\n"
                 "  7  show / hide a widget\n"
                 "  8  enable / disable a widget\n"
                 "  9  ask the window to close\n"
                 "\n"
                 "  0  quit\n";
}

// One line of input; a false return means the input ended.
bool readLine(const std::string& prompt, std::string& line)
{
    std::cout << prompt;
    return static_cast<bool>(std::getline(std::cin, line));
}

// Menu 1: the paint pass with its preorder log, then the screen.
void showPaint(Ui& ui, Screen& screen)
{
    ui.window->measure(false);
    ui.window->arrange(kWindowX, kWindowY, false);
    screen.clear();
    std::cout << "-- paint, preorder: parents first, children draw on top\n";
    ui.window->paint(screen, true);
    screen.print();
}

// Menu 2: the model as an outline, state tags and all.
void showOutline(Ui& ui)
{
    std::cout << "-- the widget tree (a preorder outline)\n";
    ui.window->printOutline();
}

// Menu 3: both layout passes, loud. The measure log reads bottom-up per
// branch and the arrange log top-down: the two orders, side by side.
void showLayout(Ui& ui)
{
    std::cout << "-- measure, postorder: every child reports before its parent adds up\n";
    ui.window->measure(true);
    std::cout << "-- arrange, preorder: a parent fixes its origin, then seats its children\n";
    ui.window->arrange(kWindowX, kWindowY, true);
}

// Menu 4: aim a click at a widget's centre, then run the normal dispatch.
void handleClickByName(Ui& ui, Screen& screen)
{
    std::string widgetName;
    if (!readLine("widget name: ", widgetName))
        return;

    Widget* target = ui.window->find(widgetName);
    if (!target)
    {
        std::cout << "no widget is called \"" << widgetName << "\"\n";
        return;
    }
    if (!target->onScreen())
    {
        std::cout << widgetName << " is hidden; there is nothing to click\n";
        return;
    }

    ui.window->click(target->centerX(), target->centerY());
    if (!ui.window->closed())
        refresh(*ui.window, screen);
}

// Menu 5: a raw click, aimed with the printed rulers.
void handleClickAt(Ui& ui, Screen& screen)
{
    std::string line;
    if (!readLine("x y: ", line))
        return;

    std::istringstream in(line);
    int                x = 0;
    int                y = 0;
    if (!(in >> x >> y))
    {
        std::cout << "need two numbers, like: 12 6\n";
        return;
    }

    ui.window->click(x, y);
    if (!ui.window->closed())
        refresh(*ui.window, screen);
}

// Menu 6: edit the name box; the star appears until the next save.
void handleTyping(Ui& ui, Screen& screen)
{
    std::string text;
    if (!readLine("text: ", text))
        return;

    ui.nameBox->setText(text);
    std::cout << "typed; the box now has unsaved edits\n";
    refresh(*ui.window, screen);
}

// Menus 7 and 8 share this: pick a widget by name, then flip one flag.
Widget* pickWidget(Ui& ui)
{
    std::string widgetName;
    if (!readLine("widget name: ", widgetName))
        return nullptr;

    Widget* target = ui.window->find(widgetName);
    if (!target)
        std::cout << "no widget is called \"" << widgetName << "\"\n";

    return target;
}

// Menu 7: hide or show a widget; layout and screen follow along.
void handleShowHide(Ui& ui, Screen& screen)
{
    Widget* target = pickWidget(ui);
    if (!target)
        return;

    if (target == ui.window.get())
    {
        std::cout << "the window itself stays visible; hide a panel instead\n";
        return;
    }

    target->setVisible(!target->isVisible());
    if (target->isVisible())
        std::cout << target->name() << " is shown again\n";
    else
        std::cout << target->name() << " is now hidden\n";

    refresh(*ui.window, screen);
}

// Menu 8: enable or disable a widget; the screen dims or wakes it.
void handleEnableDisable(Ui& ui, Screen& screen)
{
    Widget* target = pickWidget(ui);
    if (!target)
        return;

    target->setEnabled(!target->isEnabled());
    if (target->isEnabled())
        std::cout << target->name() << " is enabled again\n";
    else
        std::cout << target->name() << " is now disabled\n";

    refresh(*ui.window, screen);
}

// One menu choice; returns false when the demo should stop reading.
bool handleChoice(const std::string& choice, Ui& ui, Screen& screen)
{
    if (choice == "0")
        return false;

    if (choice == "m")
        printMenu();
    else if (choice == "1")
        showPaint(ui, screen);
    else if (choice == "2")
        showOutline(ui);
    else if (choice == "3")
        showLayout(ui);
    else if (choice == "4")
        handleClickByName(ui, screen);
    else if (choice == "5")
        handleClickAt(ui, screen);
    else if (choice == "6")
        handleTyping(ui, screen);
    else if (choice == "7")
        handleShowHide(ui, screen);
    else if (choice == "8")
        handleEnableDisable(ui, screen);
    else if (choice == "9")
        ui.window->requestClose();
    else
        std::cout << "unknown choice; m shows the menu\n";

    return true;
}
}

// The demo: build the tree, show it, and hand control to the menu.
int main()
{
    Ui     ui = buildUi();
    Screen screen(kScreenWidth, kScreenHeight);

    std::cout << "Window tree: a settings window as a tree, and the traversals that run it.\n";
    printMenu();
    std::cout << "\n";
    refresh(*ui.window, screen);

    while (!ui.window->closed())
    {
        std::string choice;
        if (!readLine("\nchoice (m for the menu): ", choice))
            break;

        if (!handleChoice(choice, ui, screen))
            break;
    }

    if (ui.window->closed())
        std::cout << "\nthe Settings window closed; the demo is over\n";

    return 0;
}
