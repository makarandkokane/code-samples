#pragma once

#include <string>
#include <vector>

// A right-threaded binary search tree, kept deliberately separate from
// BinarySearchTree: where a plain node wastes its null right pointer, a
// threaded node points it at the inorder successor instead and flags it as
// a thread. The payoff is an inorder walk with no recursion and no stack.
// The nodes are managed by hand, because a thread is a non-owning link and
// unique_ptr cannot say so.
class ThreadedTree
{
public:
    ThreadedTree() = default;
    ~ThreadedTree();

    // hand-managed nodes, so the tree must not be duplicated by accident
    ThreadedTree(const ThreadedTree&)            = delete;
    ThreadedTree& operator=(const ThreadedTree&) = delete;

    // building and unbuilding
    bool insert(int value);
    void clear();
    bool empty() const;

    // inorder by threads alone: every step is a single pointer hop
    std::vector<int> inorder() const;

    // one line per thread: which node points at which successor
    std::vector<std::string> threadLines() const;

private:
    struct Node
    {
        // the payload and the two links; right doubles as child or thread
        int   value = 0;
        Node* left  = nullptr;
        Node* right = nullptr;

        // true while right leads to the successor, not to a child
        bool rightIsThread = true;
    };

    // the two moves the threaded walk is made of
    static Node* leftmostFrom(Node* node);
    static Node* successorOf(Node* node);

    // the root of the hand-managed nodes
    Node* m_root = nullptr;
};
