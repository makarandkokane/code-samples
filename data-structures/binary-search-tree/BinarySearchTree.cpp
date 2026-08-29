#include "BinarySearchTree.h"

#include <algorithm>
#include <iostream>
#include <stack>

namespace
{
// indentation per tree level in the sideways drawing
constexpr int kIndentPerLevel = 4;
}

// Descends to the value's one possible slot and fills it. Returns false,
// changing nothing, when the value is already in the tree.
bool BinarySearchTree::insert(int value)
{
    std::unique_ptr<Node>* slot = &m_root;
    while (*slot)
    {
        if (value == (*slot)->value)
            return false;

        slot = value < (*slot)->value ? &(*slot)->left : &(*slot)->right;
    }

    *slot          = std::make_unique<Node>();
    (*slot)->value = value;
    return true;
}

// Removes the value's node, the classic three cases: a leaf is unlinked, a
// node with one child is replaced by that child, and a node with two
// children takes its inorder successor's value and the successor, which can
// have no left child, is removed instead. Returns false if the value is absent.
bool BinarySearchTree::remove(int value)
{
    // find the owning slot of the node that holds the value
    std::unique_ptr<Node>* slot = &m_root;
    while (*slot && (*slot)->value != value)
    {
        slot = value < (*slot)->value ? &(*slot)->left : &(*slot)->right;
    }
    if (!*slot)
        return false;

    // two children: steal the successor's value, then remove the successor
    if ((*slot)->left && (*slot)->right)
    {
        std::unique_ptr<Node>* successor = &(*slot)->right;
        while ((*successor)->left)
        {
            successor = &(*successor)->left;
        }

        (*slot)->value = (*successor)->value;
        slot           = successor;
    }

    // at most one child now, and it takes the removed node's place
    std::unique_ptr<Node> orphan = std::move((*slot)->left ? (*slot)->left : (*slot)->right);
    *slot                        = std::move(orphan);
    return true;
}

// Empties the tree; the unique_ptr chain frees every node.
void BinarySearchTree::clear()
{
    m_root.reset();
}

// Walks from the root toward the value, recording the path. Each step is one
// node visited, so no search can ever take more steps than the tree's
// height: that is the whole O(log N) argument, and the whole case for balance.
BinarySearchTree::SearchResult BinarySearchTree::search(int value) const
{
    SearchResult result;
    const Node*  node = m_root.get();
    while (node)
    {
        ++result.steps;
        if (!result.path.empty())
            result.path += " -> ";

        result.path += std::to_string(node->value);
        if (value == node->value)
        {
            result.found = true;
            return result;
        }

        node = value < node->value ? node->left.get() : node->right.get();
    }

    return result;
}

// The longest root-to-leaf path, counted in nodes; an empty tree is 0.
int BinarySearchTree::height() const
{
    return subtreeHeight(m_root.get());
}

// How many values the tree holds.
int BinarySearchTree::count() const
{
    return subtreeCount(m_root.get());
}

// Whether there is anything to search, walk, or balance.
bool BinarySearchTree::empty() const
{
    return !m_root;
}

// Preorder by recursion: node, left, right.
std::vector<int> BinarySearchTree::preorderRecursive() const
{
    std::vector<int> visited;
    collectPreorder(m_root.get(), visited);
    return visited;
}

// The same order without the call stack: an explicit stack of nodes still to
// visit, the right child pushed before the left so the left is visited first.
std::vector<int> BinarySearchTree::preorderIterative() const
{
    std::vector<int>        visited;
    std::stack<const Node*> pending;
    if (m_root)
        pending.push(m_root.get());

    while (!pending.empty())
    {
        const Node* node = pending.top();
        pending.pop();
        visited.push_back(node->value);
        if (node->right)
            pending.push(node->right.get());

        if (node->left)
            pending.push(node->left.get());
    }

    return visited;
}

// Inorder by recursion: left, node, right. On a search tree this comes out
// sorted, which is the property the balance rebuild leans on.
std::vector<int> BinarySearchTree::inorderRecursive() const
{
    std::vector<int> visited;
    collectInorder(m_root.get(), visited);
    return visited;
}

// Inorder without recursion: slide down the left edge remembering the way
// back on a stack, visit the deepest remembered node, then turn right once.
std::vector<int> BinarySearchTree::inorderIterative() const
{
    std::vector<int>        visited;
    std::stack<const Node*> pending;
    const Node*             node = m_root.get();
    while (node || !pending.empty())
    {
        // the left edge, remembered
        while (node)
        {
            pending.push(node);
            node = node->left.get();
        }

        // visit, then move into the right subtree
        node = pending.top();
        pending.pop();
        visited.push_back(node->value);
        node = node->right.get();
    }

    return visited;
}

// Postorder by recursion: left, right, node.
std::vector<int> BinarySearchTree::postorderRecursive() const
{
    std::vector<int> visited;
    collectPostorder(m_root.get(), visited);
    return visited;
}

// Postorder without recursion, the trick version: run a mirrored preorder
// (node, right, left) and reverse it, which is exactly left, right, node.
std::vector<int> BinarySearchTree::postorderIterative() const
{
    std::vector<int>        reversed;
    std::stack<const Node*> pending;
    if (m_root)
        pending.push(m_root.get());

    while (!pending.empty())
    {
        const Node* node = pending.top();
        pending.pop();
        reversed.push_back(node->value);
        if (node->left)
            pending.push(node->left.get());

        if (node->right)
            pending.push(node->right.get());
    }

    std::reverse(reversed.begin(), reversed.end());
    return reversed;
}

// Rebuilds the same values into the flattest shape: the sorted values come
// out of an inorder walk, and the middle-first rebuild puts the median at
// every root, halving the range at each level like binary search itself.
void BinarySearchTree::balance()
{
    const std::vector<int> sorted = inorderRecursive();
    m_root                        = buildBalanced(sorted, 0, static_cast<int>(sorted.size()) - 1);
}

// Draws the tree sideways: right subtree above, root at the left margin,
// left subtree below. Tilt your head left and it is the usual picture.
void BinarySearchTree::print() const
{
    if (!m_root)
    {
        std::cout << "(empty tree)\n";
        return;
    }

    printSideways(m_root.get(), 0);
}

// Height of one subtree: the deeper child plus this node.
int BinarySearchTree::subtreeHeight(const Node* node)
{
    if (!node)
        return 0;

    return 1 + std::max(subtreeHeight(node->left.get()), subtreeHeight(node->right.get()));
}

// Node count of one subtree: both children plus this node.
int BinarySearchTree::subtreeCount(const Node* node)
{
    if (!node)
        return 0;

    return 1 + subtreeCount(node->left.get()) + subtreeCount(node->right.get());
}

// The preorder recursion behind preorderRecursive.
void BinarySearchTree::collectPreorder(const Node* node, std::vector<int>& visited)
{
    if (!node)
        return;

    visited.push_back(node->value);
    collectPreorder(node->left.get(), visited);
    collectPreorder(node->right.get(), visited);
}

// The inorder recursion behind inorderRecursive.
void BinarySearchTree::collectInorder(const Node* node, std::vector<int>& visited)
{
    if (!node)
        return;

    collectInorder(node->left.get(), visited);
    visited.push_back(node->value);
    collectInorder(node->right.get(), visited);
}

// The postorder recursion behind postorderRecursive.
void BinarySearchTree::collectPostorder(const Node* node, std::vector<int>& visited)
{
    if (!node)
        return;

    collectPostorder(node->left.get(), visited);
    collectPostorder(node->right.get(), visited);
    visited.push_back(node->value);
}

// The balance rebuild: the middle of the range becomes this subtree's root,
// the two halves become its subtrees.
std::unique_ptr<BinarySearchTree::Node>
BinarySearchTree::buildBalanced(const std::vector<int>& sorted, int first, int last)
{
    if (first > last)
        return nullptr;

    const int middle = first + (last - first) / 2;
    auto      node   = std::make_unique<Node>();
    node->value      = sorted[static_cast<size_t>(middle)];
    node->left       = buildBalanced(sorted, first, middle - 1);
    node->right      = buildBalanced(sorted, middle + 1, last);
    return node;
}

// The drawing recursion: right first, so the greater values print on top.
void BinarySearchTree::printSideways(const Node* node, int depth)
{
    if (!node)
        return;

    printSideways(node->right.get(), depth + 1);
    std::cout << std::string(static_cast<size_t>(depth) * kIndentPerLevel, ' ') << node->value
              << "\n";
    printSideways(node->left.get(), depth + 1);
}
