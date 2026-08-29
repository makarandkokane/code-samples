#pragma once

#include <memory>
#include <string>
#include <vector>

// A classic binary search tree of unique integers. The tree owns its nodes
// through unique_ptr, so its shape is also the memory management; callers
// see values and visit orders, never a node.
class BinarySearchTree
{
public:
    // What one search did: the path it walked and whether it arrived.
    struct SearchResult
    {
        bool        found = false;
        int         steps = 0;
        std::string path;
    };

    // building and unbuilding
    bool insert(int value);
    bool remove(int value);
    void clear();

    // questions
    SearchResult search(int value) const;
    int          height() const;
    int          count() const;
    bool         empty() const;

    // the three walk orders, each in both styles; for any tree the recursive
    // and the iterative versions must produce the same sequence
    std::vector<int> preorderRecursive() const;
    std::vector<int> preorderIterative() const;
    std::vector<int> inorderRecursive() const;
    std::vector<int> inorderIterative() const;
    std::vector<int> postorderRecursive() const;
    std::vector<int> postorderIterative() const;

    // rebuilds the same values into the flattest possible shape
    void balance();

    // the sideways drawing: right subtree up, root at the left margin
    void print() const;

private:
    struct Node
    {
        // the payload and the two owned subtrees
        int                   value = 0;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
    };

    // the recursions, one small static function each
    static int                   subtreeHeight(const Node* node);
    static int                   subtreeCount(const Node* node);
    static void                  collectPreorder(const Node* node, std::vector<int>& visited);
    static void                  collectInorder(const Node* node, std::vector<int>& visited);
    static void                  collectPostorder(const Node* node, std::vector<int>& visited);
    static std::unique_ptr<Node> buildBalanced(const std::vector<int>& sorted, int first, int last);
    static void                  printSideways(const Node* node, int depth);

    // the root owns everything below it
    std::unique_ptr<Node> m_root;
};
