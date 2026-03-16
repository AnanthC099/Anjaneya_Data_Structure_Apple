"""
==============================================================================
  N-ARY TREE — COMMON INTERVIEW PROBLEMS (with full explanations)
==============================================================================

These are the problems you WILL see at Apple/FAANG interviews.
Each one builds on the traversal patterns from file 02.

Problems covered:
    1. Maximum Depth (LeetCode 559)
    2. N-ary Tree Level Order Traversal (LeetCode 429)
    3. Encode N-ary to Binary / Decode Binary to N-ary (LeetCode 431)
    4. Diameter of N-ary Tree (LeetCode 1522)
    5. Serialize and Deserialize N-ary Tree (LeetCode 428)
    6. Clone N-ary Tree
    7. Find all root-to-leaf paths
    8. Lowest Common Ancestor (LCA)
    9. Count nodes at each level
   10. Check if two N-ary trees are identical
"""

from collections import deque


class NaryTreeNode:
    def __init__(self, val=0, children=None):
        self.val = val
        self.children = children if children is not None else []


def build_example_tree():
    node5 = NaryTreeNode(5)
    node7 = NaryTreeNode(7)
    node10 = NaryTreeNode(10)
    node8 = NaryTreeNode(8)
    node9 = NaryTreeNode(9)
    node3 = NaryTreeNode(3)
    node6 = NaryTreeNode(6, [node10])
    node2 = NaryTreeNode(2, [node5, node6, node7])
    node4 = NaryTreeNode(4, [node8, node9])
    root = NaryTreeNode(1, [node2, node3, node4])
    return root


# ============================================================================
#  PROBLEM 1: Maximum Depth of N-ary Tree (LeetCode 559)
# ============================================================================
#
#  "Given an n-ary tree, find its maximum depth."
#
#  Maximum depth = number of nodes along the longest path
#  from the root down to the farthest leaf.
#
#  Our tree:  1→2→6→10 is the longest path = depth 4 (or 3 edges)
#
#  Apple asks this because: it's the simplest tree recursion problem.
#  If you can't do this, the interview is over.

def max_depth_recursive(root):
    """
    Approach: The depth of a tree = 1 + max depth of any child subtree.

    Base case: None → 0, Leaf → 1
    Recursive: 1 + max(depth of each child)

    Time:  O(n) — visit every node
    Space: O(h) — recursion depth = height
    """
    if root is None:
        return 0

    if not root.children:
        return 1  # leaf node

    max_child_depth = 0
    for child in root.children:
        max_child_depth = max(max_child_depth, max_depth_recursive(child))

    return 1 + max_child_depth


def max_depth_bfs(root):
    """
    BFS approach: count the number of levels.

    Time:  O(n)
    Space: O(w) — max width
    """
    if root is None:
        return 0

    depth = 0
    queue = deque([root])

    while queue:
        depth += 1
        level_size = len(queue)
        for _ in range(level_size):
            node = queue.popleft()
            for child in node.children:
                queue.append(child)

    return depth


# One-liner for the flex (don't use in interviews, but understand it):
def max_depth_oneliner(root):
    return 1 + max((max_depth_oneliner(c) for c in root.children), default=0) if root else 0


# ============================================================================
#  PROBLEM 2: N-ary Tree Level Order Traversal (LeetCode 429)
# ============================================================================
#
#  "Given an n-ary tree, return the level order traversal of its nodes' values."
#
#  Already covered in 02_traversals.py — see level_order() there.
#  Including here for completeness.

def level_order_traversal(root):
    """Returns [[level0_vals], [level1_vals], ...]"""
    if not root:
        return []
    result = []
    queue = deque([root])
    while queue:
        level = []
        for _ in range(len(queue)):
            node = queue.popleft()
            level.append(node.val)
            queue.extend(node.children)
        result.append(level)
    return result


# ============================================================================
#  PROBLEM 3: Encode N-ary Tree to Binary Tree (LeetCode 431) ★★★
# ============================================================================
#
#  This is a HARD and CLEVER problem. Apple loves it.
#
#  Problem: Convert an N-ary tree to a binary tree and back.
#  Why? Binary trees are simpler to store/process. Real systems do this.
#
#  THE TRICK — "Left-Child Right-Sibling" representation:
#
#  For each N-ary node:
#      - First child     → becomes LEFT child in binary tree
#      - Next sibling    → becomes RIGHT child in binary tree
#
#  N-ary tree:           Binary tree:
#       1                     1
#    / | \                   /
#   2   3  4                2
#  /|\                     / \
# 5  6  7                5    3
#                         \  / \
#                          6   4
#                           \
#                            7
#
#  See how:
#    Node 1's first child is 2 → 2 becomes 1's LEFT child
#    Node 2's sibling is 3     → 3 becomes 2's RIGHT child
#    Node 3's sibling is 4     → 4 becomes 3's RIGHT child
#    Node 2's first child is 5 → 5 becomes 2's LEFT child
#    Node 5's sibling is 6     → 6 becomes 5's RIGHT child
#    etc.

class BinaryTreeNode:
    def __init__(self, val=0, left=None, right=None):
        self.val = val
        self.left = left
        self.right = right


def encode_nary_to_binary(root):
    """
    Convert N-ary tree to Binary tree using left-child right-sibling.

    Time:  O(n)
    Space: O(n)
    """
    if root is None:
        return None

    binary_root = BinaryTreeNode(root.val)

    if root.children:
        # First child becomes left child
        binary_root.left = encode_nary_to_binary(root.children[0])

        # Remaining children become right chain of first child
        current = binary_root.left
        for i in range(1, len(root.children)):
            current.right = encode_nary_to_binary(root.children[i])
            current = current.right

    return binary_root


def decode_binary_to_nary(root):
    """
    Convert Binary tree back to N-ary tree.

    Left child  → first child of N-ary node
    Right chain → siblings (additional children of parent)

    Time:  O(n)
    Space: O(n)
    """
    if root is None:
        return None

    nary_root = NaryTreeNode(root.val)

    # Follow the LEFT child to get first child,
    # then follow RIGHT chain to get all siblings
    current = root.left
    while current:
        nary_root.children.append(decode_binary_to_nary(current))
        current = current.right

    return nary_root


# ============================================================================
#  PROBLEM 4: Diameter of N-ary Tree (LeetCode 1522) ★★★
# ============================================================================
#
#  "The diameter is the longest path between any two nodes."
#  The path does NOT need to pass through the root.
#
#  Key insight: The longest path through any node = sum of the TWO
#  longest paths going DOWN through its children.
#
#  Example:
#       1
#      /|\
#     2  3  4
#    /|     |
#   5  6    7
#
#  Longest path: 5→2→1→4→7 (length 4 edges)
#  Through node 1: longest child paths are 2(via node2) and 2(via node4)
#                   = 2 + 2 = 4

def diameter_of_nary_tree(root):
    """
    Find the diameter (longest path between any two nodes).

    For each node, compute heights of all children.
    The path through this node = sum of two largest heights + 2
    (or just the largest height + 1 if only one child).

    We track the global maximum.

    Time:  O(n)
    Space: O(h)
    """
    diameter = [0]  # Use list to allow mutation in nested function

    def height(node):
        """Returns the height of this subtree. Updates diameter as side effect."""
        if node is None:
            return -1

        if not node.children:
            return 0

        # Get heights of ALL children
        child_heights = []
        for child in node.children:
            child_heights.append(height(child))

        # Sort to get two largest
        child_heights.sort(reverse=True)

        # Path through this node using two tallest children
        if len(child_heights) >= 2:
            path_through_here = child_heights[0] + child_heights[1] + 2
        else:
            path_through_here = child_heights[0] + 1

        diameter[0] = max(diameter[0], path_through_here)

        return child_heights[0] + 1  # Height = tallest child + 1

    height(root)
    return diameter[0]


# ============================================================================
#  PROBLEM 5: Serialize / Deserialize N-ary Tree (LeetCode 428) ★★★
# ============================================================================
#
#  "Design an algorithm to serialize and deserialize an N-ary tree."
#
#  Serialize = convert tree to a string (to store in file/database/network)
#  Deserialize = convert string back to tree
#
#  Apple's Swift Codable protocol does exactly this for view hierarchies!
#
#  Strategy: Pre-order DFS. For each node, store:
#     value, number_of_children, then recurse on each child.
#
#  Example:  1[3] 2[3] 5[0] 6[1] 10[0] 7[0] 3[0] 4[2] 8[0] 9[0]
#            "1 3 2 3 5 0 6 1 10 0 7 0 3 0 4 2 8 0 9 0"

def serialize(root):
    """
    Serialize tree to string.
    Format: "val num_children val num_children ..."

    Time:  O(n)
    Space: O(n)
    """
    if root is None:
        return ""

    result = []

    def dfs(node):
        result.append(str(node.val))
        result.append(str(len(node.children)))
        for child in node.children:
            dfs(child)

    dfs(root)
    return " ".join(result)


def deserialize(data):
    """
    Deserialize string back to tree.

    Time:  O(n)
    Space: O(n)
    """
    if not data:
        return None

    tokens = iter(data.split())  # Use iterator for easy sequential access

    def dfs():
        val = int(next(tokens))
        num_children = int(next(tokens))

        node = NaryTreeNode(val)
        for _ in range(num_children):
            node.children.append(dfs())

        return node

    return dfs()


# ============================================================================
#  PROBLEM 6: Clone an N-ary Tree
# ============================================================================
#
#  Create a deep copy — every node is a NEW object.
#  This tests understanding of recursion and object references.

def clone_tree(root):
    """
    Deep clone of an N-ary tree.

    Time:  O(n)
    Space: O(n)
    """
    if root is None:
        return None

    new_node = NaryTreeNode(root.val)
    for child in root.children:
        new_node.children.append(clone_tree(child))

    return new_node


# ============================================================================
#  PROBLEM 7: All Root-to-Leaf Paths ★★
# ============================================================================
#
#  "Print every path from root to every leaf."
#
#  Our tree paths:
#     1 → 2 → 5
#     1 → 2 → 6 → 10
#     1 → 2 → 7
#     1 → 3
#     1 → 4 → 8
#     1 → 4 → 9
#
#  This is a BACKTRACKING problem — a key pattern!

def all_root_to_leaf_paths(root):
    """
    Find all paths from root to every leaf.

    Uses backtracking: add node to path, recurse, remove node.

    Time:  O(n * h) — n nodes, each path up to h long
    Space: O(h) for recursion + O(n*h) for storing all paths
    """
    if root is None:
        return []

    all_paths = []

    def dfs(node, current_path):
        current_path.append(node.val)

        if not node.children:
            # Leaf! Save a copy of the current path
            all_paths.append(list(current_path))
        else:
            for child in node.children:
                dfs(child, current_path)

        # BACKTRACK — remove this node before returning to parent
        current_path.pop()

    dfs(root, [])
    return all_paths


# ============================================================================
#  PROBLEM 8: Lowest Common Ancestor (LCA) ★★★
# ============================================================================
#
#  "Find the lowest (deepest) node that is an ancestor of both p and q."
#
#  Example: LCA(5, 7) = 2  (both are children of 2)
#           LCA(5, 8) = 1  (5 is under 2, 8 is under 4, both under 1)
#           LCA(10, 7) = 2 (10 is under 6→2, 7 is under 2)
#
#  This is asked in EVERY Apple interview loop. Know it cold.

def lowest_common_ancestor(root, p, q):
    """
    Find LCA of nodes with values p and q.

    Approach: DFS. For each node:
    - If this node IS p or q, it could be the LCA.
    - Count how many of its subtrees contain p or q.
    - If 2+ subtrees contain targets (or this node is a target + 1 subtree),
      this node is the LCA.

    Time:  O(n)
    Space: O(h)
    """
    lca_result = [None]

    def dfs(node):
        """Returns True if this subtree contains p or q."""
        if node is None:
            return False

        # Check if current node is p or q
        is_target = (node.val == p or node.val == q)

        # Count children subtrees that contain p or q
        count = 0
        for child in node.children:
            if dfs(child):
                count += 1

        # If this node is the LCA:
        #   Case 1: two different children subtrees contain p and q
        #   Case 2: this node is p (or q), and one child subtree has q (or p)
        if (count >= 2) or (is_target and count >= 1):
            lca_result[0] = node

        return is_target or count > 0

    dfs(root)
    return lca_result[0]


# ============================================================================
#  PROBLEM 9: Count Nodes at Each Level
# ============================================================================

def nodes_per_level(root):
    """
    Returns a dict: {level: count}
    Simple BFS application.
    """
    if root is None:
        return {}

    counts = {}
    queue = deque([(root, 0)])

    while queue:
        node, level = queue.popleft()
        counts[level] = counts.get(level, 0) + 1
        for child in node.children:
            queue.append((child, level + 1))

    return counts


# ============================================================================
#  PROBLEM 10: Check if Two N-ary Trees are Identical
# ============================================================================

def are_identical(root1, root2):
    """
    Two trees are identical if they have the same structure AND values.

    Time:  O(n)
    Space: O(h)
    """
    if root1 is None and root2 is None:
        return True
    if root1 is None or root2 is None:
        return False
    if root1.val != root2.val:
        return False
    if len(root1.children) != len(root2.children):
        return False

    for c1, c2 in zip(root1.children, root2.children):
        if not are_identical(c1, c2):
            return False

    return True


# ============================================================================
#  RUN ALL PROBLEMS
# ============================================================================

def print_tree(node, prefix="", is_last=True):
    if node is None:
        return
    connector = "└── " if is_last else "├── "
    if prefix == "":
        print(f"    {node.val}")
    else:
        print(f"    {prefix}{connector}{node.val}")
    child_prefix = prefix + ("    " if is_last else "│   ") if prefix else ""
    for i, child in enumerate(node.children):
        print_tree(child, child_prefix, i == len(node.children) - 1)


if __name__ == "__main__":
    root = build_example_tree()

    print("=" * 60)
    print("  N-ARY TREE — COMMON INTERVIEW PROBLEMS")
    print("=" * 60)

    print("\n--- Tree ---")
    print_tree(root)

    # Problem 1
    print(f"\n[1] Max Depth (recursive): {max_depth_recursive(root)}")
    print(f"    Max Depth (BFS):       {max_depth_bfs(root)}")

    # Problem 2
    print(f"\n[2] Level Order: {level_order_traversal(root)}")

    # Problem 3
    print(f"\n[3] Encode N-ary → Binary → Decode back to N-ary")
    binary = encode_nary_to_binary(root)
    decoded = decode_binary_to_nary(binary)
    print(f"    Original level order:  {level_order_traversal(root)}")
    print(f"    Decoded level order:   {level_order_traversal(decoded)}")
    print(f"    Match: {level_order_traversal(root) == level_order_traversal(decoded)}")

    # Problem 4
    print(f"\n[4] Diameter: {diameter_of_nary_tree(root)}")

    # Problem 5
    serialized = serialize(root)
    print(f"\n[5] Serialized:   '{serialized}'")
    deserialized = deserialize(serialized)
    print(f"    Deserialized: {level_order_traversal(deserialized)}")
    print(f"    Match: {level_order_traversal(root) == level_order_traversal(deserialized)}")

    # Problem 6
    cloned = clone_tree(root)
    print(f"\n[6] Clone identical: {are_identical(root, cloned)}")
    print(f"    Clone is new obj: {root is not cloned}")

    # Problem 7
    print(f"\n[7] All root-to-leaf paths:")
    for path in all_root_to_leaf_paths(root):
        print(f"      {' → '.join(map(str, path))}")

    # Problem 8
    lca1 = lowest_common_ancestor(root, 5, 7)
    lca2 = lowest_common_ancestor(root, 5, 8)
    lca3 = lowest_common_ancestor(root, 10, 7)
    lca4 = lowest_common_ancestor(root, 8, 9)
    print(f"\n[8] LCA(5, 7)  = Node({lca1.val})")
    print(f"    LCA(5, 8)  = Node({lca2.val})")
    print(f"    LCA(10, 7) = Node({lca3.val})")
    print(f"    LCA(8, 9)  = Node({lca4.val})")

    # Problem 9
    print(f"\n[9] Nodes per level: {nodes_per_level(root)}")

    # Problem 10
    cloned = clone_tree(root)
    other = NaryTreeNode(99)
    print(f"\n[10] Identical to clone: {are_identical(root, cloned)}")
    print(f"     Identical to other: {are_identical(root, other)}")
