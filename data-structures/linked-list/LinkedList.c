// LinkedList.c: the classic singly linked list operations, each one small enough to read on its
// own, plus the drawing that makes them visible. Every function that takes a struct Node* lives
// here. Reading what the user types is the one thing that never happens here: main.c owns the
// keyboard and the menu, and hands the list over to these functions.

#include "LinkedList.h"

#include <stdio.h>
#include <stdlib.h>

// the column the Josephus game's labels pad to, so the rings drawn after them line up
static const int kRingColumn = 19;

// room for one Josephus label, the longest of which is "person 41 is out:"
enum
{
    kLabelLength = 32
};

// Allocates one node, fully initialized; a machine out of memory ends the program politely.
static struct Node* NewNode(int value)
{
    struct Node* node = malloc(sizeof *node);

    if (node == NULL)
    {
        fprintf(stderr, "out of memory\n");
        exit(EXIT_FAILURE);
    }

    node->value = value;
    node->next  = NULL;
    return node;
}

// Inserts value as the new first node; returns the new head.
struct Node* ListInsertFront(struct Node* head, int value)
{
    struct Node* node = NewNode(value);

    node->next = head;
    return node;
}

// Inserts value after the last node; returns the (possibly new) head.
struct Node* ListInsertBack(struct Node* head, int value)
{
    struct Node* node = NewNode(value);

    if (head == NULL)
        return node;

    // walk to the tail, the one node whose next is NULL
    struct Node* tail = head;
    while (tail->next != NULL)
    {
        tail = tail->next;
    }

    tail->next = node;
    return head;
}

// Inserts value just before the first larger one, so a sorted list stays sorted.
struct Node* ListInsertSorted(struct Node* head, int value)
{
    // smaller than everything: the front is the sorted spot
    if (head == NULL || value <= head->value)
        return ListInsertFront(head, value);

    // walk to the last node whose value stays below the new one
    struct Node* before = head;
    while (before->next != NULL && before->next->value < value)
    {
        before = before->next;
    }

    // splice in behind it
    struct Node* node = NewNode(value);

    node->next   = before->next;
    before->next = node;
    return head;
}

// Removes the first node holding value, reporting through found; returns the (possibly new) head.
struct Node* ListDeleteValue(struct Node* head, int value, bool* found)
{
    *found = false;

    if (head == NULL)
        return NULL;

    // the head is its own case: no node points at it
    if (head->value == value)
    {
        struct Node* rest = head->next;

        free(head);
        *found = true;
        return rest;
    }

    // otherwise find the node standing just before the match
    struct Node* before = head;
    while (before->next != NULL && before->next->value != value)
    {
        before = before->next;
    }

    if (before->next != NULL)
    {
        struct Node* victim = before->next;

        before->next = victim->next;
        free(victim);
        *found = true;
    }

    return head;
}

// Frees every node; returns NULL so callers can write head = ListClear(head).
struct Node* ListClear(struct Node* head)
{
    while (head != NULL)
    {
        struct Node* next = head->next;

        free(head);
        head = next;
    }

    return NULL;
}

// Finds the first node holding value and reports its 1-based position; NULL when absent.
const struct Node* ListFind(const struct Node* head, int value, int* position)
{
    int at = 1;

    for (const struct Node* node = head; node != NULL; node = node->next)
    {
        if (node->value == value)
        {
            *position = at;
            return node;
        }

        at++;
    }

    *position = 0;
    return NULL;
}

// Finds the middle node with two walkers: slow takes one step for fast's two, so when fast runs
// off the end, slow stands in the middle (the later one of an even-length pair).
const struct Node* ListMiddle(const struct Node* head)
{
    const struct Node* slow = head;
    const struct Node* fast = head;

    while (fast != NULL && fast->next != NULL)
    {
        slow = slow->next;
        fast = fast->next->next;
    }

    return slow;
}

// Finds the n-th node from the end (n of 1 is the tail) in one pass: a leader walker is sent n
// steps ahead, then both walk together, and when the leader falls off the end the trailer stands
// on the answer. NULL when the list is shorter than n.
const struct Node* ListNthFromEnd(const struct Node* head, int n)
{
    // send the leader n steps ahead
    const struct Node* leader = head;
    for (int i = 0; i < n; i++)
    {
        if (leader == NULL)
            return NULL;

        leader = leader->next;
    }

    // walk both together, keeping the gap, until the leader falls off
    const struct Node* trailer = head;
    while (leader != NULL)
    {
        leader  = leader->next;
        trailer = trailer->next;
    }

    return trailer;
}

// Reverses the list in place with the classic three-pointer walk; returns the new head.
struct Node* ListReverse(struct Node* head)
{
    struct Node* previous = NULL;
    struct Node* current  = head;

    while (current != NULL)
    {
        struct Node* next = current->next;

        current->next = previous;
        previous      = current;
        current       = next;
    }

    return previous;
}

// Answers whether the values read the same both ways, by composing two earlier moves: find the
// last node of the front half with the slow and fast walkers, reverse the back half, and walk in
// from both ends comparing. The back half is reversed again before returning, so the list leaves
// this function exactly as it came in.
bool ListIsPalindrome(struct Node* head)
{
    if (head == NULL || head->next == NULL)
        return true;

    // find the last node of the front half
    struct Node* front = head;
    struct Node* fast  = head->next;

    while (fast != NULL && fast->next != NULL)
    {
        front = front->next;
        fast  = fast->next->next;
    }

    // reverse the back half and compare the halves, walking in from both ends
    struct Node* back  = ListReverse(front->next);
    struct Node* left  = head;
    struct Node* right = back;
    bool         same  = true;

    while (right != NULL)
    {
        if (left->value != right->value)
        {
            same = false;
            break;
        }

        left  = left->next;
        right = right->next;
    }

    // put the back half back the way it was
    front->next = ListReverse(back);
    return same;
}

// Cuts the list after its lower middle and returns the second half. The same slow and fast
// walkers as ListMiddle, with fast given a head start so the cut lands before the middle.
static struct Node* SplitInHalf(struct Node* head)
{
    struct Node* slow = head;
    struct Node* fast = head->next;

    while (fast != NULL && fast->next != NULL)
    {
        slow = slow->next;
        fast = fast->next->next;
    }

    struct Node* second = slow->next;

    slow->next = NULL;
    return second;
}

// Weaves two sorted lists into one sorted list. The stack-local dummy head is the classic trick
// that spares every "is this the first node" check.
static struct Node* MergeSorted(struct Node* first, struct Node* second)
{
    struct Node  dummy = {.value = 0, .next = NULL};
    struct Node* tail  = &dummy;

    while (first != NULL && second != NULL)
    {
        if (first->value <= second->value)
        {
            tail->next = first;
            first      = first->next;
        }
        else
        {
            tail->next = second;
            second     = second->next;
        }

        tail = tail->next;
    }

    // one side ran out; the other is already sorted, so it chains on whole
    if (first != NULL)
        tail->next = first;
    else
        tail->next = second;

    return dummy.next;
}

// Sorts ascending with merge sort, the linked list's own sorting algorithm: it never needs to
// jump to an index, and splitting and merging are both natural list moves. Returns the new head.
struct Node* ListSort(struct Node* head)
{
    // zero or one node is already sorted, and is the recursion's floor
    if (head == NULL || head->next == NULL)
        return head;

    struct Node* second = SplitInHalf(head);
    struct Node* left   = ListSort(head);
    struct Node* right  = ListSort(second);

    return MergeSorted(left, right);
}

// Rotates the list left by k places with this demo's own circle trick: bend the list into a
// circle, walk k steps to the node that becomes the new tail, and cut there. Walking a circle
// makes k larger than the length wrap around for free. Returns the new head.
struct Node* ListRotateLeft(struct Node* head, int k)
{
    if (head == NULL || k <= 0)
        return head;

    ListMakeCircular(head);

    // the k-th node becomes the new tail
    struct Node* tail = head;
    for (int i = 1; i < k; i++)
    {
        tail = tail->next;
    }

    // cut the circle just after it
    struct Node* newHead = tail->next;

    tail->next = NULL;
    return newHead;
}

// Removes repeated values from a sorted list, keeping the first of each run, and returns how
// many nodes were freed. On an unsorted list it only collapses neighboring repeats.
int ListRemoveDuplicates(struct Node* head)
{
    int removed = 0;

    for (struct Node* node = head; node != NULL; node = node->next)
    {
        // eat every immediate neighbor holding the same value
        while (node->next != NULL && node->next->value == node->value)
        {
            struct Node* victim = node->next;

            node->next = victim->next;
            free(victim);
            removed++;
        }
    }

    return removed;
}

// Bends the list into a circle: the tail's next, the one NULL every list has, is aimed back at
// the head. An empty list has nothing to bend and is left alone.
void ListMakeCircular(struct Node* head)
{
    if (head == NULL)
        return;

    struct Node* tail = head;
    while (tail->next != NULL)
    {
        tail = tail->next;
    }

    tail->next = head;
}

// Floyd's tortoise and hare: a slow walker and a double-speed one. On a straight list the hare
// falls off the end; on a circle it laps the tortoise and they meet.
bool ListHasLoop(const struct Node* head)
{
    const struct Node* slow = head;
    const struct Node* fast = head;

    while (fast != NULL && fast->next != NULL)
    {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast)
            return true;
    }

    return false;
}

// Straightens a circle: finds the node pointing back at the head and cuts there. Safe on a
// straight list too, where the walk simply ends at the tail.
void ListBreakCircle(struct Node* head)
{
    if (head == NULL)
        return;

    struct Node* node = head;
    while (node->next != NULL && node->next != head)
    {
        node = node->next;
    }

    node->next = NULL;
}

// Draws the chain from head as boxes and arrows, without a newline; the caller ends the line. The
// head must not be NULL. A circle is walked once round and then says so, rather than forever.
static void DrawChain(const struct Node* head, bool circular)
{
    const struct Node* node = head;

    do
    {
        printf("[%d]", node->value);
        node = node->next;
        if (node != NULL && node != head)
            printf(" -> ");
    } while (node != NULL && node != head);

    if (circular)
        printf(" -> back to [%d]", head->value);
    else
        printf(" -> NULL");
}

// Counts the nodes with a walk that stops at NULL or on coming back around to the head.
int ListCount(const struct Node* head)
{
    if (head == NULL)
        return 0;

    int                count = 0;
    const struct Node* node  = head;

    do
    {
        count++;
        node = node->next;
    } while (node != NULL && node != head);

    return count;
}

// The status line the demo shows after every operation: the whole list, drawn, plus a node count.
void ListPrint(const struct Node* head, bool circular)
{
    if (head == NULL)
    {
        printf("(empty list)\n");
        return;
    }

    DrawChain(head, circular);

    // the count, with its grammar and its circular tag sorted out
    int count = ListCount(head);

    if (count == 1)
        printf("   (1 node");
    else
        printf("   (%d nodes", count);

    if (circular)
        printf(", circular");

    printf(")\n");
}

// One line of the Josephus game: the label padded to a fixed column, then the ring as it stands,
// so the rings line up under each other however wide the labels are.
static void AnnounceRing(const char* label, const struct Node* head)
{
    printf("%-*s", kRingColumn, label);
    DrawChain(head, true);
    printf("\n");
}

// One elimination round, announced: who is out, and the ring that remains.
static void AnnounceRound(const struct Node* head, int victim)
{
    char label[kLabelLength];

    snprintf(label, sizeof label, "person %2d is out:", victim);
    AnnounceRing(label, head);
}

// Plays the Josephus game: counting from the first person, every step-th one is unlinked and
// freed, and the ring is redrawn after every round, until one person is left. head must be a list
// already bent into a circle. The survivor comes back as a straight one-node list, so the caller
// is holding an ordinary list again.
struct Node* ListJosephus(struct Node* head, int step)
{
    AnnounceRing("the circle:", head);

    // the count starts at person 1, so the walker starts one node before it, on the tail
    struct Node* before = head;
    while (before->next != head)
    {
        before = before->next;
    }

    // every round: walk step-1 places, unlink the step-th person, count on from the next one
    while (before->next != before)
    {
        for (int i = 1; i < step; i++)
        {
            before = before->next;
        }

        struct Node* victim = before->next;
        int          out    = victim->value;

        before->next = victim->next;
        if (victim == head)
            head = victim->next;

        free(victim);
        AnnounceRound(head, out);
    }

    // last one standing: the self-loop straightens into a one-node list
    before->next = NULL;
    printf("person %d survives and breaks the circle\n", before->value);
    return before;
}
