#include "ThreadedTree.h"

// Frees the nodes on the way out.
ThreadedTree::~ThreadedTree()
{
    clear();
}

// The standard search tree descent, plus the bookkeeping that keeps the
// threads true: a new left child's successor is its parent, a new right
// child takes over the parent's old thread, and the parent's right pointer
// becomes a real child link. Returns false for a duplicate.
bool ThreadedTree::insert(int value)
{
    Node* fresh  = new Node;
    fresh->value = value;
    if (!m_root)
    {
        m_root = fresh;
        return true;
    }

    Node* node = m_root;
    while (true)
    {
        if (value == node->value)
        {
            delete fresh;
            return false;
        }

        if (value < node->value)
        {
            if (node->left)
            {
                node = node->left;
                continue;
            }

            // a left child's inorder successor is its own parent
            node->left   = fresh;
            fresh->right = node;
            return true;
        }

        if (!node->rightIsThread)
        {
            node = node->right;
            continue;
        }

        // a right child inherits the parent's old thread
        fresh->right        = node->right;
        node->right         = fresh;
        node->rightIsThread = false;
        return true;
    }
}

// Frees every node. The threaded walk collects them first, because deleting
// along the walk would read freed memory through the threads.
void ThreadedTree::clear()
{
    std::vector<Node*> nodes;
    Node*              node = leftmostFrom(m_root);
    while (node)
    {
        nodes.push_back(node);
        node = successorOf(node);
    }

    for (Node* doomed : nodes)
    {
        delete doomed;
    }

    m_root = nullptr;
}

// Whether anything has been built yet.
bool ThreadedTree::empty() const
{
    return !m_root;
}

// The threaded walk: start at the leftmost node, then from each node either
// hop the thread straight to the successor or slide to the leftmost node of
// the right subtree. No recursion, no stack, nothing remembered.
std::vector<int> ThreadedTree::inorder() const
{
    std::vector<int> visited;
    Node*            node = leftmostFrom(m_root);
    while (node)
    {
        visited.push_back(node->value);
        node = successorOf(node);
    }

    return visited;
}

// Reports each thread as "value ~> successor". The rightmost node has no
// successor; its thread marks the end of the walk.
std::vector<std::string> ThreadedTree::threadLines() const
{
    std::vector<std::string> lines;
    Node*                    node = leftmostFrom(m_root);
    while (node)
    {
        if (node->rightIsThread && node->right)
            lines.push_back(std::to_string(node->value) + " ~> " +
                            std::to_string(node->right->value));
        else if (node->rightIsThread)
            lines.push_back(std::to_string(node->value) + " ~> (end of the walk)");

        node = successorOf(node);
    }

    return lines;
}

// Slides down the left edge; where inorder walks begin.
ThreadedTree::Node* ThreadedTree::leftmostFrom(Node* node)
{
    while (node && node->left)
    {
        node = node->left;
    }

    return node;
}

// Where inorder goes next: across the thread, or down into the right subtree.
ThreadedTree::Node* ThreadedTree::successorOf(Node* node)
{
    if (node->rightIsThread)
        return node->right;

    return leftmostFrom(node->right);
}
