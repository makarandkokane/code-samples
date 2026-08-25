# Linked list demo (plain C)

A singly linked list of ints behind a menu, in plain C. Build the list, question it, rearrange it, bend it into a circle, and break back out. No C++, no frameworks, nothing beyond the standard library. I wrote it with students in mind: every menu item is answered by one small function you can read on its own, and the list draws itself after every operation, so you always see what the pointers just did.

```
Build                          Query
  1  insert at front             7  search a value
  2  insert at back              8  find the middle
  3  insert sorted              15  nth from the end
  4  delete a value             16  palindrome check
  5  load the sample list
  6  clear the list            Rearrange
                                 9  reverse
                                10  sort (merge sort)
                                18  remove duplicates (sorted)

The Circle
 11  make it circular           14  the Josephus game
 12  detect a loop              17  rotate left by k
 13  break the circle

  0  quit
```

A taste of a session:

```
choice (m for the menu): 5
sample list loaded
[7] -> [3] -> [12] -> [60] -> [3] -> [42] -> [7] -> [9] -> NULL   (8 nodes)

choice (m for the menu): 9
list reversed
[9] -> [7] -> [42] -> [3] -> [60] -> [12] -> [3] -> [7] -> NULL   (8 nodes)

choice (m for the menu): 10
sorted, ascending
[3] -> [3] -> [7] -> [7] -> [9] -> [12] -> [42] -> [60] -> NULL   (8 nodes)
```

## The famous ones

Five menu items are the questions every linked list interview reaches for, and they get the classic answers. Reverse is the three-pointer walk. Detect a loop is Floyd's tortoise and hare: a walker and a double-speed walker, and if the list is a circle the hare laps the tortoise. Nth from the end is the same two-walker idea in another costume: send the leader n steps ahead, then walk both together, and when the leader falls off the end the trailer stands on the answer. The palindrome check is this menu composing itself: find where the halves meet, reverse the back half, walk in from both ends comparing, then put everything back the way it was. And sort is merge sort, which is the point of picking it: merge sort is the linked list's own sorting algorithm, it never needs to jump to an index, and splitting and merging are both natural list moves, so it lands in the code as three small functions, `SplitInHalf`, `MergeSorted`, `ListSort`.

Continuing the session above, the sorted list loses its duplicates, takes a turn, and answers two questions:

```
choice (m for the menu): 18
2 duplicates removed
[3] -> [7] -> [9] -> [12] -> [42] -> [60] -> NULL   (6 nodes)

choice (m for the menu): 17
rotate left by: 2
rotated left by 2
[9] -> [12] -> [42] -> [60] -> [3] -> [7] -> NULL   (6 nodes)

choice (m for the menu): 15
n (1 is the tail): 2
counting 2 from the end: [3]
[9] -> [12] -> [42] -> [60] -> [3] -> [7] -> NULL   (6 nodes)

choice (m for the menu): 16
not a palindrome
[9] -> [12] -> [42] -> [60] -> [3] -> [7] -> NULL   (6 nodes)
```

Rotate left by k sits in The Circle group of the menu on purpose: the implementation bends the list into a circle, walks k steps, and cuts the circle open at the new head. A side effect of doing it that way is that a k larger than the length wraps around for free.

One guard worth noticing: while the list is a circle, the linear operations refuse to run, because every one of them walks toward a NULL that no longer exists. Break the circle first, says the program, and that refusal is itself the lesson.

I keep telling people that data structures come before design patterns, in those words, in [Data structures before design patterns](https://makarandkokane.github.io/articles/software-development-tip-data-structures.html). And I once wondered aloud whether most of our C++ should be plain C, in [Is C better than C++?](https://makarandkokane.github.io/articles/is-c-better-than-cpp.html). This folder is both of those posts put to work.

## The Josephus game

The circle needed a reason to exist, and there is no better one than the oldest: Flavius Josephus, 67 AD, the siege of Yodfat. The story goes that 41 trapped rebels stood in a circle and counted off, every third man falling, and Josephus worked out on the spot where to stand to be the last one standing. Menu item 14 plays it: n people in a circle, every k-th eliminated, the ring redrawn after every round, and the survivor breaks the circle, which turns the circular list back into a straight one.

```
choice (m for the menu): 14
how many people: 7
eliminate every k-th, k: 3
the circle:        [1] -> [2] -> [3] -> [4] -> [5] -> [6] -> [7] -> back to [1]
person  3 is out:  [1] -> [2] -> [4] -> [5] -> [6] -> [7] -> back to [1]
person  6 is out:  [1] -> [2] -> [4] -> [5] -> [7] -> back to [1]
person  2 is out:  [1] -> [4] -> [5] -> [7] -> back to [1]
person  7 is out:  [1] -> [4] -> [5] -> back to [1]
person  5 is out:  [1] -> [4] -> back to [1]
person  1 is out:  [4] -> back to [4]
person 4 survives and breaks the circle
[4] -> NULL   (1 node)
```

Run it with the legend's own numbers, 41 people and k of 3, and the survivor is person 31: the spot where the story says Josephus put himself.

## Code tour

| File | Role |
|------|------|
| `LinkedList.h` | The contract: one struct and nineteen small functions. |
| `LinkedList.c` | The list itself. Every function that takes a node pointer lives here: the splices, the walks, the game, and the drawing. |
| `main.c` | The menu: what gets typed, the guards on what may run, and one small handler per item. |

Every menu item lands in one function, so you can read exactly the one you care about:

| Menu item | Function | The idea |
|-----------|----------|----------|
| 1, 2 insert at front, at back | `ListInsertFront`, `ListInsertBack` | the two basic splices |
| 3 insert sorted | `ListInsertSorted` | stop before the first larger value |
| 4 delete a value | `ListDeleteValue` | the head is its own case |
| 7 search a value | `ListFind` | a plain walk with a counter |
| 8 find the middle | `ListMiddle` | slow and fast walkers |
| 9 reverse | `ListReverse` | the three-pointer walk |
| 10 sort | `ListSort` | merge sort: split, recurse, merge |
| 11 make it circular | `ListMakeCircular` | the tail's NULL aims back at the head |
| 12 detect a loop | `ListHasLoop` | Floyd's tortoise and hare |
| 13 break the circle | `ListBreakCircle` | find who points at the head, cut there |
| 14 the Josephus game | `ListJosephus` | deletion in a circle until one remains |
| 15 nth from the end | `ListNthFromEnd` | two walkers, a gap of n apart |
| 16 palindrome check | `ListIsPalindrome` | where the halves meet, reverse, compare, restore |
| 17 rotate left by k | `ListRotateLeft` | bend into a circle, walk k, cut |
| 18 remove duplicates | `ListRemoveDuplicates` | eat equal neighbors in a sorted list |

Items 5 and 6, the sample list and clear, are conveniences that live in `main.c`, and the line printed after every operation is `ListPrint`, which draws the boxes and arrows and counts the nodes. The menu numbers are append-only: a new item takes the next free number and is shown inside its group, so old transcripts stay valid.

## Build and run

Any C compiler is enough:

    cc main.c LinkedList.c -o linked-list

Or with CMake like the rest of this repo:

    cmake -S . -B build
    cmake --build build --config Release
    build\Release\linked-list.exe      (Windows)
    ./build/linked-list                (elsewhere)

Everything here is textbook material on purpose: reverse, merge sort, the tortoise and the hare, Josephus. The algorithms are everyone's. The code and the menu around them were written fresh for this repo.
