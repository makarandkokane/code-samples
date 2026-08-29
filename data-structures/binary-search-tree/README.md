# Binary search tree (C++17)

The classic ordered tree behind a menu: insert, delete, search with the steps counted, all three traversal orders in both their recursive and iterative forms, a balance operation for a tree that grew crooked, and a right-threaded variant that walks itself with no recursion and no stack. Plain C++17 and the standard library, no frameworks. The tree draws itself sideways after every change, so you always see the shape you are working on: tilt your head left and it is the usual picture.

```
Build
   1  insert a value
   2  insert several values
   3  load the sample tree
   4  load the worst case: 1 to 15 in sorted order
   5  delete a value
   6  clear the tree

Search
   7  search a value, counting the steps
   8  height and node count

Walk
   9  preorder, recursive then iterative
  10  inorder, recursive then iterative
  11  postorder, recursive then iterative

Balance
  12  balance the tree

The threaded tree
  13  build it from the current values
  14  walk it by threads alone
  15  show the threads

   0  quit
```

## The shape is the speed

A search walks one path from the root, so it can never take more steps than the tree is tall. That single sentence is the whole O(log N) argument, and the demo lets you feel it. Load the worst case (menu item 4): fifteen values inserted in sorted order, so every node chains to the right and the "tree" is a stick.

```
sorted input, so every node chains right; the tree is a stick:
                                                        15
                                                    14
                                                13
                                            12
                                        11
                                    10
                                9
                            8
                        7
                    6
                5
            4
        3
    2
1
15 values, height 15
```

Search for 15 and the path visits everything:

```
choice (m for the menu): 7
value: 15
1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7 -> 8 -> 9 -> 10 -> 11 -> 12 -> 13 -> 14 -> 15: found in 15 steps
the height is 15, and no search can take more steps than that
```

Now balance (menu item 12). The values come out of an inorder walk already sorted, and the rebuild puts the middle of every range at the root of every subtree, which is binary search frozen into a shape:

```
height 15 -> 4, same values, same inorder:
            15
        14
            13
    12
            11
        10
            9
8
            7
        6
            5
    4
            3
        2
            1
15 values, height 4
```

The same search now takes four steps: `8 -> 12 -> 14 -> 15: found in 4 steps`. Same values, same answers, a quarter of the work, and that gap grows with the data: a million values are twenty steps in a balanced tree.

## Three walks, two ways each

Menu items 9 to 11 run each traversal order twice, once by recursion and once with an explicit stack, and the program checks the two sequences against each other rather than asking you to squint:

```
choice (m for the menu): 9
preorder, recursive: 50 30 20 10 40 35 45 70 60 80 75
preorder, iterative: 50 30 20 10 40 35 45 70 60 80 75
the two agree, as they must

choice (m for the menu): 10
inorder, recursive: 10 20 30 35 40 45 50 60 70 75 80
inorder, iterative: 10 20 30 35 40 45 50 60 70 75 80
the two agree, as they must
```

The recursive versions are the three-line textbook functions. The iterative ones show what the call stack was silently doing for you: preorder keeps a stack of subtrees still owed a visit, inorder slides down left edges and remembers the way back, and postorder uses the trick version, a mirrored preorder reversed at the end. Inorder on a search tree always comes out sorted, which is exactly what the balance operation leans on.

Deleting is the classic three cases: a leaf is unlinked, a node with one child is replaced by that child, and a node with two children takes its inorder successor's value, after which the successor is removed instead.

## The threaded tree

Every leaf in a plain binary tree carries null pointers that do nothing. A right-threaded tree spends them: a node with no right child points its right pointer at its inorder successor instead, flagged as a thread. The payoff is menu item 14, a full inorder walk with no recursion and no stack, each step a single pointer hop.

```
choice (m for the menu): 14
inorder, by threads alone: 10 20 30 35 40 45 50 60 70 75 80
no recursion and no stack: every step was one pointer hop

choice (m for the menu): 15
  10 ~> 20
  20 ~> 30
  35 ~> 40
  45 ~> 50
  60 ~> 70
  75 ~> 80
  80 ~> (end of the walk)
```

The threaded tree lives in its own files, `ThreadedTree.h` and `ThreadedTree.cpp`, with its own node struct and no dependence on the main tree, because its invariants are different enough that mixing the two would blur both. It also owns its nodes by hand where the main tree uses `unique_ptr`: a thread is a non-owning link, and an owning pointer type cannot say so. Menu item 13 builds it as a snapshot of the current values, fed in preorder so it reproduces the search tree's exact shape.

## Code tour

| File | Role |
|------|------|
| `BinarySearchTree.h` / `.cpp` | The tree and everything it can do: insert, remove, search with the path recorded, height and count, the six traversals, the balance rebuild, and the sideways drawing. Nodes are owned through `unique_ptr`, so the tree's shape is also the memory management. |
| `ThreadedTree.h` / `.cpp` | The right-threaded variant, self-contained: insert with the thread bookkeeping, the thread-hopping inorder walk, and the thread report. |
| `main.cpp` | The menu, the typing, the guards, and one small handler per item. |

Every menu item maps to the functions it demonstrates:

| Menu item | Function |
|-----------|----------|
| 1, 2 insert | `BinarySearchTree::insert` |
| 3, 4 sample and worst case | `insert` in a fixed order, from `main.cpp` |
| 5 delete | `BinarySearchTree::remove` |
| 6 clear | `BinarySearchTree::clear` |
| 7 search | `BinarySearchTree::search` |
| 8 height and count | `BinarySearchTree::height`, `count` |
| 9, 10, 11 walks | `preorder/inorder/postorder`, `Recursive` and `Iterative` |
| 12 balance | `BinarySearchTree::balance` |
| 13 build threaded | `ThreadedTree::insert`, fed the main tree's preorder |
| 14 walk by threads | `ThreadedTree::inorder` |
| 15 show threads | `ThreadedTree::threadLines` |

## Build and run

```
cmake -S . -B build
cmake --build build --config Release
build\Release\binary-search-tree.exe      (Windows)
./build/binary-search-tree                (elsewhere)
```

## Where the mechanisms come from

All of it is textbook material on purpose: the insert and the successor-based delete, the stack traversals, the reversal trick for iterative postorder, the sorted-rebuild balance, and the right-threaded tree are classics every data structures course teaches. The code is written fresh for this repo; the algorithms are the field's. I wrote about the moment trees start paying rent in [Look for hierarchies in the data](https://makarandkokane.github.io/articles/software-development-tip-hierarchies.html), and about why this layer comes before design patterns in [Data structures before design patterns](https://makarandkokane.github.io/articles/software-development-tip-data-structures.html). This demo is where both articles go to run.
