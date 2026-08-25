#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdbool.h>

// One node of a singly linked list of ints; the whole demo hangs off this one struct.
struct Node
{
    int          value;
    struct Node* next;
};

// Building and dismantling. Every mutator returns the list's new head.
struct Node* ListInsertFront(struct Node* head, int value);
struct Node* ListInsertBack(struct Node* head, int value);
struct Node* ListInsertSorted(struct Node* head, int value);
struct Node* ListDeleteValue(struct Node* head, int value, bool* found);
struct Node* ListClear(struct Node* head);

// Questions a list can answer. Positions are 1-based, made for humans. The palindrome check
// borrows the list mutably but restores it exactly before it returns.
const struct Node* ListFind(const struct Node* head, int value, int* position);
const struct Node* ListMiddle(const struct Node* head);
const struct Node* ListNthFromEnd(const struct Node* head, int n);
bool               ListIsPalindrome(struct Node* head);

// Rearranging.
struct Node* ListReverse(struct Node* head);
struct Node* ListSort(struct Node* head);
struct Node* ListRotateLeft(struct Node* head, int k);
int          ListRemoveDuplicates(struct Node* head);

// The circle. Once a list is bent into one, ListPrint, ListCount, ListHasLoop, ListBreakCircle
// and ListJosephus are the only functions here that are still safe to call on it: every other one
// walks toward a NULL that is no longer there.
void ListMakeCircular(struct Node* head);
bool ListHasLoop(const struct Node* head);
void ListBreakCircle(struct Node* head);

// Showing a list: the boxes and arrows, then a node count, on one line. Both walks stop when a
// circle comes back around to the head, so both are safe on one; pass circular and the drawing
// ends with the way back to the head instead of with NULL.
void ListPrint(const struct Node* head, bool circular);
int  ListCount(const struct Node* head);

// The Josephus game, the circle's own party trick: people counted off around the ring, every
// step-th one out, the ring redrawn after each round. Takes a list already bent into a circle and
// hands back the survivor as a straight one-node list.
struct Node* ListJosephus(struct Node* head, int step);

#endif
