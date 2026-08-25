// main.c: the menu. Reading what is typed, the guards on what may run, and one small handler per
// menu item, so a student can read just the one they came for. No function here takes a list to
// work on: everything that walks, changes or draws the list itself lives in LinkedList.c.

#include "LinkedList.h"

#include <stdio.h>

// an input line is read whole and then parsed; plenty of room for one typed number
enum
{
    kLineLength = 128
};

// Menu choices, exactly as PrintMenu shows them. Numbers are append-only, so a new item takes
// the next free number and is displayed inside its group, not in numeric order; that way old
// session transcripts stay valid. MenuAgain is the m key, not a number.
enum Menu
{
    MenuAgain        = -1,
    MenuQuit         = 0,
    MenuInsertFront  = 1,
    MenuInsertBack   = 2,
    MenuInsertSorted = 3,
    MenuDeleteValue  = 4,
    MenuLoadSample   = 5,
    MenuClear        = 6,
    MenuSearch       = 7,
    MenuFindMiddle   = 8,
    MenuReverse      = 9,
    MenuSort         = 10,
    MenuMakeCircular = 11,
    MenuDetectLoop   = 12,
    MenuBreakCircle  = 13,
    MenuJosephus     = 14,
    MenuNthFromEnd   = 15,
    MenuPalindrome   = 16,
    MenuRotateLeft   = 17,
    MenuDeduplicate  = 18
};

// the sample list: shuffled values with two repeats, so sorting, reversing and removing
// duplicates all have something to show
static const int kSampleValues[] = {7, 3, 12, 60, 3, 42, 7, 9};
static const int kSampleCount    = sizeof kSampleValues / sizeof kSampleValues[0];

// the Josephus game is capped at the legend's own head count, which also keeps rounds readable
static const int kMaxPeople = 41;

// the column where the menu's right half starts
static const int kMenuColumn = 31;

// The program's whole state.
struct App
{
    // the list, and whether its tail currently points back at its head
    struct Node* head;
    bool         circular;
};

// Reads one line and parses one integer; asks again on nonsense, false when input has ended.
static bool ReadInt(const char* prompt, int* out)
{
    char line[kLineLength];

    for (;;)
    {
        printf("%s", prompt);
        if (fgets(line, sizeof line, stdin) == NULL)
            return false;

        if (sscanf(line, "%d", out) == 1)
            return true;

        printf("that was not a number\n");
    }
}

// Reads a menu choice: a number, m for the menu again, end of input to quit.
static int ReadChoice(void)
{
    char line[kLineLength];

    for (;;)
    {
        printf("\nchoice (m for the menu): ");
        if (fgets(line, sizeof line, stdin) == NULL)
            return MenuQuit;

        if (line[0] == 'm' || line[0] == 'M')
            return MenuAgain;

        int choice = 0;
        if (sscanf(line, "%d", &choice) == 1)
            return choice;

        printf("a number please, or m\n");
    }
}

// Prints one menu row: the left cell padded to a fixed column, then the right cell.
static void MenuRow(const char* left, const char* right)
{
    printf("%-*s%s\n", kMenuColumn, left, right);
}

// Prints the menu, grouped so it reads as a story rather than a wall of numbers.
static void PrintMenu(void)
{
    printf("\n");
    MenuRow("Build", "Query");
    MenuRow("  1  insert at front", "  7  search a value");
    MenuRow("  2  insert at back", "  8  find the middle");
    MenuRow("  3  insert sorted", " 15  nth from the end");
    MenuRow("  4  delete a value", " 16  palindrome check");
    printf("  5  load the sample list\n");
    MenuRow("  6  clear the list", "Rearrange");
    MenuRow("", "  9  reverse");
    MenuRow("", " 10  sort (merge sort)");
    MenuRow("", " 18  remove duplicates (sorted)");
    printf("\n");
    printf("The Circle\n");
    MenuRow(" 11  make it circular", " 14  the Josephus game");
    MenuRow(" 12  detect a loop", " 17  rotate left by k");
    printf(" 13  break the circle\n");
    printf("\n");
    printf("  0  quit\n");
}

// Most operations only make sense on a straight list; this refuses once, uniformly.
static bool RefusedBecauseCircular(const struct App* app)
{
    if (app->circular)
    {
        printf("the list is a circle right now, break it first (menu 13)\n");
        return true;
    }

    return false;
}

// Frees the whole list, straightening it first if it is a circle.
static void FreeEverything(struct App* app)
{
    if (app->circular)
        ListBreakCircle(app->head);

    app->head     = ListClear(app->head);
    app->circular = false;
}

// Menu 1: a typed value becomes the new first node.
static void DoInsertFront(struct App* app)
{
    if (RefusedBecauseCircular(app))
        return;

    int value = 0;
    if (!ReadInt("value: ", &value))
        return;

    app->head = ListInsertFront(app->head, value);
    printf("%d inserted at the front\n", value);
}

// Menu 2: a typed value becomes the new last node.
static void DoInsertBack(struct App* app)
{
    if (RefusedBecauseCircular(app))
        return;

    int value = 0;
    if (!ReadInt("value: ", &value))
        return;

    app->head = ListInsertBack(app->head, value);
    printf("%d inserted at the back\n", value);
}

// Menu 3: inserts before the first larger value, which keeps a sorted list sorted.
static void DoInsertSorted(struct App* app)
{
    if (RefusedBecauseCircular(app))
        return;

    int value = 0;
    if (!ReadInt("value: ", &value))
        return;

    app->head = ListInsertSorted(app->head, value);
    printf("%d inserted in sorted place\n", value);
}

// Menu 4: deletes the first node holding the typed value.
static void DoDeleteValue(struct App* app)
{
    if (RefusedBecauseCircular(app))
        return;

    int value = 0;
    if (!ReadInt("value to delete: ", &value))
        return;

    bool found = false;

    app->head = ListDeleteValue(app->head, value, &found);
    if (found)
        printf("first %d deleted\n", value);
    else
        printf("no %d in the list\n", value);
}

// Menu 5: replaces whatever is there with the built-in sample list.
static void DoLoadSample(struct App* app)
{
    FreeEverything(app);
    for (int i = 0; i < kSampleCount; i++)
    {
        app->head = ListInsertBack(app->head, kSampleValues[i]);
    }

    printf("sample list loaded\n");
}

// Menu 6: frees every node, circle or not.
static void DoClear(struct App* app)
{
    FreeEverything(app);
    printf("list cleared\n");
}

// Menu 7: linear search, reported with a 1-based position.
static void DoSearch(struct App* app)
{
    if (RefusedBecauseCircular(app))
        return;

    int value = 0;
    if (!ReadInt("value to find: ", &value))
        return;

    int position = 0;
    if (ListFind(app->head, value, &position) != NULL)
        printf("%d found at position %d\n", value, position);
    else
        printf("no %d in the list\n", value);
}

// Menu 8: the slow and fast walkers find the middle in one pass.
static void DoFindMiddle(struct App* app)
{
    if (RefusedBecauseCircular(app))
        return;

    const struct Node* middle = ListMiddle(app->head);

    if (middle == NULL)
        printf("an empty list has no middle\n");
    else
        printf("the middle node holds %d\n", middle->value);
}

// Menu 15: the n-th node from the end, found in one pass by two walkers a gap of n apart.
static void DoNthFromEnd(struct App* app)
{
    if (RefusedBecauseCircular(app))
        return;

    int n = 0;
    if (!ReadInt("n (1 is the tail): ", &n))
        return;

    if (n < 1)
    {
        printf("n must be at least 1\n");
        return;
    }

    const struct Node* node = ListNthFromEnd(app->head, n);

    if (node == NULL)
        printf("the list is shorter than %d\n", n);
    else
        printf("counting %d from the end: [%d]\n", n, node->value);
}

// Menu 16: the palindrome check, the two-walker walk and reverse put to work together.
static void DoPalindrome(struct App* app)
{
    if (RefusedBecauseCircular(app))
        return;

    if (ListIsPalindrome(app->head))
        printf("reads the same both ways: a palindrome\n");
    else
        printf("not a palindrome\n");
}

// Menu 9: the famous one.
static void DoReverse(struct App* app)
{
    if (RefusedBecauseCircular(app))
        return;

    app->head = ListReverse(app->head);
    printf("list reversed\n");
}

// Menu 10: merge sort, ascending.
static void DoSort(struct App* app)
{
    if (RefusedBecauseCircular(app))
        return;

    app->head = ListSort(app->head);
    printf("sorted, ascending\n");
}

// Menu 18: removes repeated values, meant for a sorted list where repeats stand together.
static void DoDeduplicate(struct App* app)
{
    if (RefusedBecauseCircular(app))
        return;

    int removed = ListRemoveDuplicates(app->head);

    if (removed == 0)
        printf("no neighboring duplicates\n");
    else if (removed == 1)
        printf("1 duplicate removed\n");
    else
        printf("%d duplicates removed\n", removed);
}

// Menu 17: rotate left by k, using the circle: bend, walk k, cut.
static void DoRotateLeft(struct App* app)
{
    if (RefusedBecauseCircular(app))
        return;

    if (app->head == NULL)
    {
        printf("an empty list does not turn\n");
        return;
    }

    int k = 0;
    if (!ReadInt("rotate left by: ", &k))
        return;

    if (k < 1)
    {
        printf("k must be at least 1\n");
        return;
    }

    app->head = ListRotateLeft(app->head, k);
    printf("rotated left by %d\n", k);
}

// Menu 11: bends the tail's NULL back to the head.
static void DoMakeCircular(struct App* app)
{
    if (app->circular)
    {
        printf("it is already a circle\n");
        return;
    }

    if (app->head == NULL)
    {
        printf("an empty list has nothing to bend\n");
        return;
    }

    ListMakeCircular(app->head);
    app->circular = true;
    printf("the tail now points back at the head\n");
}

// Menu 12: Floyd's tortoise and hare, run for real, not read off the circular flag.
static void DoDetectLoop(const struct App* app)
{
    if (ListHasLoop(app->head))
        printf("the hare lapped the tortoise: there is a loop\n");
    else
        printf("the hare fell off the end: no loop\n");
}

// Menu 13: cuts the circle back into a straight list.
static void DoBreakCircle(struct App* app)
{
    if (!app->circular)
    {
        printf("the list is not circular\n");
        return;
    }

    ListBreakCircle(app->head);
    app->circular = false;
    printf("the circle is broken, the tail points at NULL again\n");
}

// Menu 14: the Josephus game. n people stand in a circle, every k-th is eliminated, and the
// last one standing breaks the circle. The game replaces whatever list was loaded.
static void DoJosephus(struct App* app)
{
    // the two numbers that define the game
    int people = 0;
    if (!ReadInt("how many people: ", &people))
        return;

    if (people < 1 || people > kMaxPeople)
    {
        printf("keep it between 1 and %d, the legend himself had 41\n", kMaxPeople);
        return;
    }

    int step = 0;
    if (!ReadInt("eliminate every k-th, k: ", &step))
        return;

    if (step < 1)
    {
        printf("k must be at least 1\n");
        return;
    }

    // a fresh circle of people numbered 1 to n replaces the list
    FreeEverything(app);
    for (int person = 1; person <= people; person++)
    {
        app->head = ListInsertBack(app->head, person);
    }

    ListMakeCircular(app->head);
    app->circular = true;

    // the game plays itself out and hands back the survivor as a straight one-node list
    app->head     = ListJosephus(app->head, step);
    app->circular = false;
}

// Routes one menu choice to its handler; unknown numbers get a gentle nudge.
static void RunChoice(struct App* app, int choice)
{
    switch (choice)
    {
    case MenuInsertFront:
        DoInsertFront(app);
        break;
    case MenuInsertBack:
        DoInsertBack(app);
        break;
    case MenuInsertSorted:
        DoInsertSorted(app);
        break;
    case MenuDeleteValue:
        DoDeleteValue(app);
        break;
    case MenuLoadSample:
        DoLoadSample(app);
        break;
    case MenuClear:
        DoClear(app);
        break;
    case MenuSearch:
        DoSearch(app);
        break;
    case MenuFindMiddle:
        DoFindMiddle(app);
        break;
    case MenuReverse:
        DoReverse(app);
        break;
    case MenuSort:
        DoSort(app);
        break;
    case MenuMakeCircular:
        DoMakeCircular(app);
        break;
    case MenuDetectLoop:
        DoDetectLoop(app);
        break;
    case MenuBreakCircle:
        DoBreakCircle(app);
        break;
    case MenuJosephus:
        DoJosephus(app);
        break;
    case MenuNthFromEnd:
        DoNthFromEnd(app);
        break;
    case MenuPalindrome:
        DoPalindrome(app);
        break;
    case MenuRotateLeft:
        DoRotateLeft(app);
        break;
    case MenuDeduplicate:
        DoDeduplicate(app);
        break;
    default:
        printf("no menu item %d\n", choice);
        break;
    }
}

// A menu program around one singly linked list: build it, question it, rearrange it, bend it
// into a circle and break out again. The list is drawn after every operation.
int main(void)
{
    struct App app = {.head = NULL, .circular = false};

    printf("a singly linked list, one menu at a time\n");
    PrintMenu();

    for (;;)
    {
        int choice = ReadChoice();

        if (choice == MenuQuit)
            break;

        if (choice == MenuAgain)
        {
            PrintMenu();
            continue;
        }

        RunChoice(&app, choice);
        ListPrint(app.head, app.circular);
    }

    FreeEverything(&app);
    printf("bye\n");
    return 0;
}
