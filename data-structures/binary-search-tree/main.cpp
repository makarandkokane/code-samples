#include "BinarySearchTree.h"
#include "ThreadedTree.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
// the sample tree: eleven values that build a reasonably bushy shape
constexpr int kSampleValues[] = {50, 30, 70, 20, 40, 60, 80, 35, 45, 75, 10};

// the worst case: this many values inserted in sorted order make a stick
constexpr int kWorstCaseTop = 15;

// The two trees the menu operates on. The threaded tree is its own tree,
// built on request as a snapshot of the search tree's values.
struct App
{
    BinarySearchTree tree;
    ThreadedTree     threaded;
    bool             threadedBuilt = false;
};

// The menu, grouped by what each item demonstrates.
void printMenu()
{
    std::cout << "\n"
                 "Build\n"
                 "   1  insert a value\n"
                 "   2  insert several values\n"
                 "   3  load the sample tree\n"
                 "   4  load the worst case: 1 to 15 in sorted order\n"
                 "   5  delete a value\n"
                 "   6  clear the tree\n"
                 "\n"
                 "Search\n"
                 "   7  search a value, counting the steps\n"
                 "   8  height and node count\n"
                 "\n"
                 "Walk\n"
                 "   9  preorder, recursive then iterative\n"
                 "  10  inorder, recursive then iterative\n"
                 "  11  postorder, recursive then iterative\n"
                 "\n"
                 "Balance\n"
                 "  12  balance the tree\n"
                 "\n"
                 "The threaded tree\n"
                 "  13  build it from the current values\n"
                 "  14  walk it by threads alone\n"
                 "  15  show the threads\n"
                 "\n"
                 "   0  quit\n";
}

// One line of input; a false return means the input ended.
bool readLine(const std::string& prompt, std::string& line)
{
    std::cout << prompt;
    return static_cast<bool>(std::getline(std::cin, line));
}

// One integer from one prompted line; complains and returns false on junk.
bool readValue(const std::string& prompt, int& value)
{
    std::string line;
    if (!readLine(prompt, line))
        return false;

    std::istringstream in(line);
    if (!(in >> value))
    {
        std::cout << "need a whole number\n";
        return false;
    }

    return true;
}

// A visit order as one line: "50 30 20 10".
std::string joinValues(const std::vector<int>& values)
{
    std::string joined;
    for (int value : values)
    {
        if (!joined.empty())
            joined += " ";

        joined += std::to_string(value);
    }

    return joined;
}

// "1 value" or "11 values": the demo talks in counts all the time.
std::string countOf(int count, const std::string& noun)
{
    std::string text = std::to_string(count) + " " + noun;
    if (count != 1)
        text += "s";

    return text;
}

// The screen the user sees after every change: the drawing, then the counts.
void report(const App& app)
{
    app.tree.print();
    if (!app.tree.empty())
        std::cout << countOf(app.tree.count(), "value") << ", height " << app.tree.height() << "\n";
}

// Menu 1: one value in, or the reason it stayed out.
void handleInsert(App& app)
{
    int value = 0;
    if (!readValue("value: ", value))
        return;

    if (app.tree.insert(value))
        std::cout << value << " goes in\n";
    else
        std::cout << value << " is already in the tree\n";

    report(app);
}

// Menu 2: a whole line of values, duplicates reported not inserted.
void handleInsertMany(App& app)
{
    std::string line;
    if (!readLine("values: ", line))
        return;

    // insert everything on the line, counting what happened
    std::istringstream in(line);
    int                value      = 0;
    int                inserted   = 0;
    int                duplicates = 0;
    while (in >> value)
    {
        if (app.tree.insert(value))
            ++inserted;
        else
            ++duplicates;
    }
    if (inserted == 0 && duplicates == 0)
    {
        std::cout << "need numbers, like: 8 3 10\n";
        return;
    }

    std::cout << inserted << " inserted, " << countOf(duplicates, "duplicate") << " ignored\n";
    report(app);
}

// Menu 3: a fresh start with a reasonably bushy tree.
void handleSample(App& app)
{
    app.tree.clear();
    for (int value : kSampleValues)
    {
        app.tree.insert(value);
    }

    std::cout << "the sample tree:\n";
    report(app);
}

// Menu 4: the shape that ruins the search, on purpose.
void handleWorstCase(App& app)
{
    app.tree.clear();
    for (int value = 1; value <= kWorstCaseTop; ++value)
    {
        app.tree.insert(value);
    }

    std::cout << "sorted input, so every node chains right; the tree is a stick:\n";
    report(app);
}

// Menu 5: one value out, or the reason nothing changed.
void handleRemove(App& app)
{
    int value = 0;
    if (!readValue("value: ", value))
        return;

    if (app.tree.remove(value))
        std::cout << value << " removed\n";
    else
        std::cout << value << " is not in the tree\n";

    report(app);
}

// Menu 6: back to nothing.
void handleClear(App& app)
{
    app.tree.clear();
    report(app);
}

// Menu 7: the search, told as the path it walked.
void handleSearch(App& app)
{
    if (app.tree.empty())
    {
        std::cout << "the tree is empty; nothing to search\n";
        return;
    }

    int value = 0;
    if (!readValue("value: ", value))
        return;

    const BinarySearchTree::SearchResult result = app.tree.search(value);
    if (result.found)
        std::cout << result.path << ": found in " << countOf(result.steps, "step") << "\n";
    else
        std::cout << result.path << ": not found, gave up after " << countOf(result.steps, "step")
                  << "\n";

    std::cout << "the height is " << app.tree.height()
              << ", and no search can take more steps than that\n";
}

// Menu 8: the two numbers every other item keeps talking about.
void handleStats(App& app)
{
    if (app.tree.empty())
    {
        std::cout << "the tree is empty\n";
        return;
    }

    // the flattest height these values could have: floor(log2(count)) + 1
    int flattest = 0;
    for (int remaining = app.tree.count(); remaining > 0; remaining /= 2)
    {
        ++flattest;
    }

    std::cout << countOf(app.tree.count(), "value") << ", height " << app.tree.height() << "\n";
    std::cout << "the flattest possible height for that many values is " << flattest << "\n";
}

// Menus 9 to 11 share this: both versions of one order, checked against
// each other, because the iterative rewrite must not change the order.
void printWalk(const std::string& order, const std::vector<int>& recursive,
               const std::vector<int>& iterative)
{
    std::cout << order << ", recursive: " << joinValues(recursive) << "\n";
    std::cout << order << ", iterative: " << joinValues(iterative) << "\n";
    if (recursive == iterative)
        std::cout << "the two agree, as they must\n";
    else
        std::cout << "THE TWO DISAGREE: the iterative rewrite is wrong\n";
}

// Menus 9 to 11: one traversal order, both ways.
void handleWalk(App& app, const std::string& choice)
{
    if (app.tree.empty())
    {
        std::cout << "the tree is empty; nothing to walk\n";
        return;
    }

    if (choice == "9")
        printWalk("preorder", app.tree.preorderRecursive(), app.tree.preorderIterative());
    else if (choice == "10")
        printWalk("inorder", app.tree.inorderRecursive(), app.tree.inorderIterative());
    else
        printWalk("postorder", app.tree.postorderRecursive(), app.tree.postorderIterative());
}

// Menu 12: same values, flattest shape.
void handleBalance(App& app)
{
    if (app.tree.empty())
    {
        std::cout << "the tree is empty; nothing to balance\n";
        return;
    }

    const int before = app.tree.height();
    app.tree.balance();
    std::cout << "height " << before << " -> " << app.tree.height()
              << ", same values, same inorder:\n";
    report(app);
}

// Menu 13: snapshot the current values into the threaded tree. Inserting
// them in preorder reproduces the search tree's exact shape.
void handleBuildThreaded(App& app)
{
    if (app.tree.empty())
    {
        std::cout << "the tree is empty; nothing to thread\n";
        return;
    }

    app.threaded.clear();
    for (int value : app.tree.preorderRecursive())
    {
        app.threaded.insert(value);
    }

    app.threadedBuilt = true;
    std::cout << "threaded tree built: the same " << app.tree.count()
              << " values in the same shape, every null right now a thread\n";
}

// Menu 14: the walk the threads were built for.
void handleWalkThreaded(App& app)
{
    if (!app.threadedBuilt)
    {
        std::cout << "no threaded tree yet; build it first (menu 13)\n";
        return;
    }

    std::cout << "inorder, by threads alone: " << joinValues(app.threaded.inorder()) << "\n";
    std::cout << "no recursion and no stack: every step was one pointer hop\n";
}

// Menu 15: where each thread points.
void handleShowThreads(App& app)
{
    if (!app.threadedBuilt)
    {
        std::cout << "no threaded tree yet; build it first (menu 13)\n";
        return;
    }

    for (const std::string& line : app.threaded.threadLines())
    {
        std::cout << "  " << line << "\n";
    }
}

// One menu choice; returns false when the demo should stop reading.
bool handleChoice(const std::string& choice, App& app)
{
    if (choice == "0")
        return false;

    if (choice == "m")
        printMenu();
    else if (choice == "1")
        handleInsert(app);
    else if (choice == "2")
        handleInsertMany(app);
    else if (choice == "3")
        handleSample(app);
    else if (choice == "4")
        handleWorstCase(app);
    else if (choice == "5")
        handleRemove(app);
    else if (choice == "6")
        handleClear(app);
    else if (choice == "7")
        handleSearch(app);
    else if (choice == "8")
        handleStats(app);
    else if (choice == "9" || choice == "10" || choice == "11")
        handleWalk(app, choice);
    else if (choice == "12")
        handleBalance(app);
    else if (choice == "13")
        handleBuildThreaded(app);
    else if (choice == "14")
        handleWalkThreaded(app);
    else if (choice == "15")
        handleShowThreads(app);
    else
        std::cout << "unknown choice; m shows the menu\n";

    return true;
}
}

// The demo: an empty tree, the menu, and the loop that reads choices.
int main()
{
    App app;

    std::cout << "Binary search tree: search by halving, walk in every order, "
                 "balance what grew crooked.\n";
    printMenu();

    while (true)
    {
        std::string choice;
        if (!readLine("\nchoice (m for the menu): ", choice))
            break;

        if (!handleChoice(choice, app))
            break;
    }

    return 0;
}
