"""
==============================================================================
  N-ARY TREE TRAVERSALS — The 4 Ways to Visit Every Node
==============================================================================

"Traversal" = visiting every node in the tree exactly once, in some order.

There are 4 main traversals. Think of them as 4 different ways to
read every page of a book that has chapters, sections, and subsections.


THE BIG PICTURE:
================

    DFS (Depth-First Search) — go DEEP before going WIDE
        1. Pre-order:   Visit node FIRST, then children (left to right)
        2. Post-order:  Visit children FIRST, then node
        (In-order only makes sense for binary trees, skip for n-ary)

    BFS (Breadth-First Search) — go WIDE before going DEEP
        3. Level-order: Visit ALL nodes at depth 0, then depth 1, then depth 2...

    Bonus:
        4. Reverse Level-order: Level-order but bottom to top


WHEN TO USE WHICH (Apple interview cheat sheet):
================================================
    Pre-order:   Copy a tree, serialize/deserialize, prefix expressions
    Post-order:  Delete a tree, calculate sizes, evaluate expressions
    Level-order: Find depth, shortest path, connect nodes at same level
                 Print tree level by level (most common interview question!)


OUR EXAMPLE TREE:
=================
              1
         /  |   \
        2    3    4
       /|\       / \
      5  6  7   8   9
         |
        10
"""

from collections import deque


class NaryTreeNode:
    def __init__(self, val=0, children=None):
        self.val = val
        self.children = children if children is not None else []

    def __repr__(self):
        return f"Node({self.val})"


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
#  TRAVERSAL 1: PRE-ORDER (Root → Children)
# ============================================================================
#
#  "Pre" means "before" — visit the node BEFORE its children.
#
#  Think of it as: "I introduce myself first, then introduce my kids."
#
#  For our tree:  1, 2, 5, 6, 10, 7, 3, 4, 8, 9
#
#  How it works step by step:
#     Visit 1 → go to child 2
#       Visit 2 → go to child 5
#         Visit 5 → no children, backtrack
#       → go to child 6
#         Visit 6 → go to child 10
#           Visit 10 → no children, backtrack
#       → go to child 7
#         Visit 7 → no children, backtrack
#     → go to child 3
#       Visit 3 → no children, backtrack
#     → go to child 4
#       Visit 4 → go to child 8
#         Visit 8 → no children, backtrack
#       → go to child 9
#         Visit 9 → no children, backtrack

def preorder_recursive(root):
    """
    Pre-order traversal using recursion.
    Time:  O(n) — visit every node once
    Space: O(h) — recursion stack = height of tree
    """
    result = []

    def dfs(node):
        if node is None:
            return
        result.append(node.val)       # VISIT node FIRST (pre = before)
        for child in node.children:   # Then visit children left to right
            dfs(child)

    dfs(root)
    return result


def preorder_iterative(root):
    """
    Pre-order using a STACK (no recursion).

    Why learn this? Because:
    1. Interviewers ask "can you do it without recursion?"
    2. Recursion can cause stack overflow on very deep trees
    3. It shows you understand what recursion is actually doing

    Key trick: Push children in REVERSE order so leftmost is processed first.
    Stack = LIFO (Last In, First Out)
    If we push [child1, child2, child3], we pop child3 first.
    We want child1 first, so push [child3, child2, child1].

    Time:  O(n)
    Space: O(n) — stack can hold up to n nodes in worst case
    """
    if root is None:
        return []

    result = []
    stack = [root]

    while stack:
        node = stack.pop()
        result.append(node.val)

        # Push children in REVERSE order
        # so that leftmost child is on top of stack
        for child in reversed(node.children):
            stack.append(child)

    return result


# ============================================================================
#  TRAVERSAL 2: POST-ORDER (Children → Root)
# ============================================================================
#
#  "Post" means "after" — visit the node AFTER all its children.
#
#  Think of it as: "My kids introduce themselves first, then I go."
#
#  For our tree:  5, 10, 6, 7, 2, 3, 8, 9, 4, 1
#
#  Why useful? You need post-order when:
#     - Deleting a tree (delete children before parent)
#     - Calculating subtree sizes (need children's sizes first)
#     - Evaluating expression trees (operands before operator)

def postorder_recursive(root):
    """
    Post-order traversal using recursion.
    Time:  O(n)
    Space: O(h)
    """
    result = []

    def dfs(node):
        if node is None:
            return
        for child in node.children:   # Visit ALL children first
            dfs(child)
        result.append(node.val)       # THEN visit node (post = after)

    dfs(root)
    return result


def postorder_iterative(root):
    """
    Post-order iterative is TRICKY. There's a clever hack:

    Pre-order  = Root, Child1, Child2, Child3, ...
    Modified   = Root, Child3, Child2, Child1, ...  (children in reverse)
    Reversed   = ..., Child1, Child2, Child3, Root  ← that's post-order!

    So: do pre-order but push children left-to-right (not reversed),
    then reverse the final result.

    Time:  O(n)
    Space: O(n)
    """
    if root is None:
        return []

    result = []
    stack = [root]

    while stack:
        node = stack.pop()
        result.append(node.val)

        # Push children LEFT to RIGHT (opposite of pre-order!)
        for child in node.children:
            stack.append(child)

    return result[::-1]  # Reverse at the end!


# ============================================================================
#  TRAVERSAL 3: LEVEL-ORDER / BFS (Level by Level)
# ============================================================================
#
#  Visit all nodes at depth 0, then depth 1, then depth 2, ...
#
#  For our tree:
#     Level 0: [1]
#     Level 1: [2, 3, 4]
#     Level 2: [5, 6, 7, 8, 9]
#     Level 3: [10]
#
#  Uses a QUEUE (FIFO = First In, First Out) — NOT a stack!
#
#  THIS IS THE MOST ASKED TRAVERSAL IN INTERVIEWS.
#  Apple loves it because it naturally maps to UI hierarchies.

def level_order(root):
    """
    Level-order traversal using a queue.

    Returns a list of lists: [[level0], [level1], [level2], ...]

    How it works:
    1. Put root in queue
    2. While queue not empty:
       a. Count how many nodes are at current level (= queue size)
       b. Pop that many nodes, add their children to queue
       c. Collect their values as one level

    Time:  O(n)
    Space: O(w) where w = max width of tree (widest level)
    """
    if root is None:
        return []

    result = []
    queue = deque([root])  # deque is efficient for queue operations

    while queue:
        level_size = len(queue)   # How many nodes at THIS level?
        current_level = []

        for _ in range(level_size):
            node = queue.popleft()           # Remove from FRONT
            current_level.append(node.val)

            for child in node.children:      # Add children to BACK
                queue.append(child)

        result.append(current_level)

    return result


def level_order_flat(root):
    """
    Same as above but returns a flat list instead of grouped by level.
    Result: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    """
    if root is None:
        return []

    result = []
    queue = deque([root])

    while queue:
        node = queue.popleft()
        result.append(node.val)
        for child in node.children:
            queue.append(child)

    return result


# ============================================================================
#  TRAVERSAL 4: REVERSE LEVEL-ORDER (Bottom to Top)
# ============================================================================

def reverse_level_order(root):
    """
    Same as level-order, but reversed at the end.
    Result: [[10], [5, 6, 7, 8, 9], [2, 3, 4], [1]]

    When to use: "Print tree from bottom to top" type questions.
    """
    levels = level_order(root)
    return levels[::-1]


# ============================================================================
#  VISUAL COMPARISON
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
    print("  N-ARY TREE TRAVERSALS")
    print("=" * 60)

    print("\n--- Tree ---")
    print_tree(root)

    print("\n--- Pre-order (Root first, then children) ---")
    print(f"  Recursive:  {preorder_recursive(root)}")
    print(f"  Iterative:  {preorder_iterative(root)}")

    print("\n--- Post-order (Children first, then root) ---")
    print(f"  Recursive:  {postorder_recursive(root)}")
    print(f"  Iterative:  {postorder_iterative(root)}")

    print("\n--- Level-order / BFS (Level by level, top to bottom) ---")
    print(f"  Grouped:    {level_order(root)}")
    print(f"  Flat:       {level_order_flat(root)}")

    print("\n--- Reverse Level-order (Bottom to top) ---")
    print(f"  Grouped:    {reverse_level_order(root)}")

    print("\n--- Memory Aid ---")
    print("  PRE  = I go first, kids follow     → Root, C1, C2, C3")
    print("  POST = Kids go first, I go last    → C1, C2, C3, Root")
    print("  BFS  = Everyone at my depth first  → Level 0, 1, 2, 3")
    print()
    print("  DFS uses STACK  (recursion or explicit)")
    print("  BFS uses QUEUE  (always iterative)")
