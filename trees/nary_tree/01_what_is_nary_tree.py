"""
==============================================================================
  N-ARY TREE — COMPLETE GUIDE (Explained Like You're 5, Then Like You're 25)
==============================================================================

WHAT IS A TREE?
===============
Imagine your family tree:
    - Your grandparent is at the TOP (we call this the "root")
    - Your grandparent had children (parents, aunts, uncles)
    - Those children had their own children (you, cousins)
    - Someone with no children is called a "leaf"

That's literally a tree in computer science — flipped upside down.


WHAT IS AN N-ARY TREE?
======================
"N-ary" just means "each parent can have UP TO N children."

    - N = 2  → Binary tree (max 2 kids per parent) — you'll learn this separately
    - N = 3  → Ternary tree (max 3 kids)
    - N = anything → General tree (any number of kids)

When people say "N-ary tree" they usually mean the GENERAL case:
    each node can have 0, 1, 2, 3, ... ANY number of children.


REAL-WORLD EXAMPLES:
====================
1. FILE SYSTEM on your computer:
        /home
        ├── user
        │   ├── Documents
        │   │   ├── resume.pdf
        │   │   └── notes.txt
        │   ├── Pictures
        │   │   └── cat.jpg
        │   └── Music
        └── admin

   "home" has 2 children (user, admin).
   "user" has 3 children (Documents, Pictures, Music).
   "resume.pdf" has 0 children — it's a leaf.
   This is an N-ary tree!

2. COMPANY ORG CHART:
        CEO
        ├── VP Engineering
        │   ├── Team Lead 1
        │   │   ├── Dev A
        │   │   └── Dev B
        │   └── Team Lead 2
        │       └── Dev C
        ├── VP Marketing
        │   └── Designer
        └── VP Sales

3. HTML DOM (web pages):
        <html>
        ├── <head>
        │   └── <title>
        └── <body>
            ├── <div>
            │   ├── <p>
            │   └── <p>
            └── <footer>

4. JSON / nested dictionaries — any nested data is a tree!


VOCABULARY (memorize these):
=============================
    Root     = the topmost node (no parent)
    Parent   = a node that has children
    Child    = a node directly below another node
    Siblings = nodes that share the same parent
    Leaf     = a node with NO children (also called "external node")
    Internal = a node that has at least one child
    Edge     = the connection between parent and child
    Path     = sequence of nodes from one node to another
    Depth    = how many edges from root to this node (root depth = 0)
    Height   = how many edges from this node to its deepest leaf
    Level    = same as depth (Level 0 = root, Level 1 = root's children, ...)
    Subtree  = a node and ALL its descendants
    Degree   = number of children a node has
    Forest   = a collection of trees (remove root → you get a forest)


EXAMPLE WITH NUMBERS:
=====================

              1            ← root (depth=0, height=3)
         /  |   \
        2    3    4        ← depth=1
       /|\       / \
      5  6  7   8   9     ← depth=2
         |
        10                 ← depth=3 (leaf, also deepest)

    Node 1: degree=3 (children: 2,3,4), depth=0, height=3
    Node 2: degree=3 (children: 5,6,7), depth=1, height=2
    Node 3: degree=0 (leaf!),           depth=1, height=0
    Node 6: degree=1 (child: 10),       depth=2, height=1
    Node 10: degree=0 (leaf!),          depth=3, height=0

    Total nodes = 10
    Total edges = 9  (always nodes - 1 in a tree!)
    Total leaves = 4 (nodes 5, 7, 10, 8... wait, 9 too → 5, 7, 10, 8, 9 = 5 leaves)
    Let me recount: 5(leaf), 6(has child 10, not leaf), 7(leaf), 3(leaf),
                    8(leaf), 9(leaf), 10(leaf) → 6 leaves


WHY N-ARY TREES MATTER (especially at Apple):
===============================================
    - File systems (Finder, Spotlight indexing)
    - XML/JSON parsing
    - Trie (prefix tree) — autocomplete, spell check, Siri
    - Game trees (AI decision making)
    - Scene graphs (UIKit/SwiftUI view hierarchies!)
    - Abstract Syntax Trees (compiler, Swift compiler itself)
    - Database B-trees (Core Data, SQLite)
"""

# ============================================================================
#  STEP 1: HOW TO REPRESENT AN N-ARY TREE IN CODE
# ============================================================================

class NaryTreeNode:
    """
    Each node stores:
        1. val      → the data it holds (number, string, anything)
        2. children → a LIST of child nodes (could be empty for leaves)

    That's it. That's the whole data structure.

    Why a list? Because N-ary means "any number of children."
    We don't know in advance how many children a node will have,
    so we use a dynamic list.
    """

    def __init__(self, val=0, children=None):
        self.val = val
        self.children = children if children is not None else []

    def __repr__(self):
        return f"Node({self.val})"


# ============================================================================
#  STEP 2: BUILD A TREE BY HAND
# ============================================================================

def build_example_tree():
    """
    Builds this tree:

              1
         /  |   \
        2    3    4
       /|\       / \
      5  6  7   8   9
         |
        10

    We build BOTTOM-UP (leaves first, then parents).
    """

    # Create leaf nodes (no children)
    node5 = NaryTreeNode(5)
    node7 = NaryTreeNode(7)
    node10 = NaryTreeNode(10)
    node8 = NaryTreeNode(8)
    node9 = NaryTreeNode(9)
    node3 = NaryTreeNode(3)

    # Create internal nodes (they have children)
    node6 = NaryTreeNode(6, [node10])          # 6 → [10]
    node2 = NaryTreeNode(2, [node5, node6, node7])  # 2 → [5, 6, 7]
    node4 = NaryTreeNode(4, [node8, node9])    # 4 → [8, 9]

    # Create root
    root = NaryTreeNode(1, [node2, node3, node4])  # 1 → [2, 3, 4]

    return root


# ============================================================================
#  STEP 3: BASIC OPERATIONS — Let's verify our tree works
# ============================================================================

def count_nodes(root):
    """
    Count total nodes in the tree.

    Logic: 1 (for myself) + count of all nodes in each child's subtree

    This is RECURSION:
        - Base case: if root is None, return 0
        - Recursive case: 1 + sum of count_nodes(child) for each child
    """
    if root is None:
        return 0

    total = 1  # count myself
    for child in root.children:
        total += count_nodes(child)
    return total


def find_height(root):
    """
    Height = longest path from this node to any leaf below it.

    A leaf has height 0.
    An internal node's height = 1 + max(height of any child).

    Example:
        Node 1's height = 1 + max(height(2), height(3), height(4))
                        = 1 + max(2, 0, 1) = 3
    """
    if root is None:
        return -1  # convention: empty tree has height -1

    if not root.children:
        return 0   # leaf node

    max_child_height = 0
    for child in root.children:
        child_height = find_height(child)
        max_child_height = max(max_child_height, child_height)

    return 1 + max_child_height


def find_leaves(root):
    """Collect all leaf nodes (nodes with no children)."""
    if root is None:
        return []

    if not root.children:
        return [root.val]  # I'm a leaf!

    leaves = []
    for child in root.children:
        leaves.extend(find_leaves(child))
    return leaves


def find_depth(root, target, current_depth=0):
    """
    Find the depth of a specific node value.
    Depth = number of edges from root to this node.
    """
    if root is None:
        return -1

    if root.val == target:
        return current_depth

    for child in root.children:
        result = find_depth(child, target, current_depth + 1)
        if result != -1:
            return result

    return -1  # not found in this subtree


# ============================================================================
#  STEP 4: PRINT THE TREE (so we can actually see it!)
# ============================================================================

def print_tree(node, prefix="", is_last=True):
    """
    Pretty-prints the tree like the Linux `tree` command.

    Output:
    1
    ├── 2
    │   ├── 5
    │   ├── 6
    │   │   └── 10
    │   └── 7
    ├── 3
    └── 4
        ├── 8
        └── 9
    """
    if node is None:
        return

    connector = "└── " if is_last else "├── "

    if prefix == "":
        # Root node — no connector
        print(f"{node.val}")
    else:
        print(f"{prefix}{connector}{node.val}")

    # Update prefix for children
    if prefix == "":
        child_prefix = ""
    else:
        child_prefix = prefix + ("    " if is_last else "│   ")

    for i, child in enumerate(node.children):
        is_last_child = (i == len(node.children) - 1)
        print_tree(child, child_prefix, is_last_child)


# ============================================================================
#  RUN EVERYTHING
# ============================================================================

if __name__ == "__main__":
    print("=" * 60)
    print("  N-ARY TREE — Building & Basic Operations")
    print("=" * 60)

    root = build_example_tree()

    print("\n--- Tree Structure ---")
    print_tree(root)

    print(f"\n--- Basic Stats ---")
    print(f"  Total nodes:  {count_nodes(root)}")
    print(f"  Tree height:  {find_height(root)}")
    print(f"  Leaf nodes:   {find_leaves(root)}")
    print(f"  Depth of 10:  {find_depth(root, 10)}")
    print(f"  Depth of 1:   {find_depth(root, 1)}")
    print(f"  Depth of 4:   {find_depth(root, 4)}")

    print("\n--- Key Insight ---")
    print("  Every recursive function on an N-ary tree follows the SAME pattern:")
    print("    1. Handle base case (None or leaf)")
    print("    2. Process current node")
    print("    3. Loop through children and recurse")
    print("  That's it. Master this pattern and you can solve ANY tree problem.")
