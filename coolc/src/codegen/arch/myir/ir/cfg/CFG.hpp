#pragma once

#include "codegen/arch/myir/ir/IR.hpp"
#include <functional>

namespace myir
{

class CFG : public IRObject
{
  public:
    enum DFSType
    {
        PREORDER,         // visit node before its successors
        INORDER,          // visit every pair (node, successor)
        POSTORDER,        // visit node after its successors
        REVERSE_POSTORDER // reversed POSTORDER
    };

  private:
    Block *_root;

    // traversals
    irvector<Block *> _postorder;
    irvector<Block *> _preorder;
    irvector<Block *> _reverse_postorder;

    // dominance info
    irunordered_map<Block *, Block *> _dominance;
    irunordered_map<Block *, irvector<Block *>> _dominator_tree;
    irunordered_map<Block *, irset<Block *>> _dominance_frontier;

    Block *dominance_intersect(Block *b1, Block *b2);

    // common DFS
    template <DFSType type> void dfs(Block *root, const std::function<void(Block *s, Block *d)> &visitor);

#ifdef DEBUG
    void dump_dominance();
    void dump_dominance_frontier();
#endif // DEBUG

  public:
    CFG()
        : _postorder(ALLOC), _preorder(ALLOC), _reverse_postorder(ALLOC), _dominance(ALLOC), _dominator_tree(ALLOC),
          _dominance_frontier(ALLOC), _root(nullptr)
    {
    }

    // graph traversals
    template <DFSType type> irvector<Block *> &traversal();

    // helpers
    void clear_visited();

    // setters
    inline void set_cfg(Block *b) { _root = b; }

    // getters
    inline Block *root() const { return _root; }
    inline bool empty() const { return !root(); }

    // for every Block* gather all blocks that dominate it
    // Implements approach was described in "A Simple, Fast Dominance Algorithm"
    // by Keith D. Cooper, Timothy J. Harvey, and Ken Kennedy
    irunordered_map<Block *, Block *> &dominance();
    irunordered_map<Block *, irvector<Block *>> &dominator_tree();
    bool dominate(Block *dominator, Block *dominatee);

    // for every Block* gather all blocks in its DF
    // Implements approach was described in "A Simple, Fast Dominance Algorithm"
    // by Keith D. Cooper, Timothy J. Harvey, and Ken Kennedy
    irunordered_map<Block *, irset<Block *>> &dominance_frontier();

    // debugging
    std::string dump() const;
};

// nice printing
extern void trim(std::string &where, const std::string &what);
}; // namespace myir