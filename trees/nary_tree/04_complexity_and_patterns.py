"""
==============================================================================
  N-ARY TREE — TIME/SPACE COMPLEXITY & PATTERNS CHEAT SHEET
==============================================================================

This file is your QUICK REFERENCE before interviews.


=========================
  TIME COMPLEXITY TABLE
=========================

  Operation / Problem          | Time    | Space   | Technique
  -----------------------------|---------|---------|------------------
  Traversal (any)              | O(n)    | O(h)*   | DFS or BFS
  Max Depth                    | O(n)    | O(h)    | DFS
  Level Order                  | O(n)    | O(w)**  | BFS + Queue
  Diameter                     | O(n)    | O(h)    | DFS + 2 tallest
  Serialize / Deserialize      | O(n)    | O(n)    | Pre-order DFS
  Clone                        | O(n)    | O(n)    | DFS
  LCA                          | O(n)    | O(h)    | DFS
  Root-to-leaf paths           | O(n*h)  | O(n*h)  | DFS + Backtrack
  Encode/Decode (Binary↔Nary)  | O(n)    | O(n)    | Left-child Right-sibling
  Search for a value           | O(n)    | O(h)    | DFS or BFS

  * h = height of tree. Worst case h = n (skewed tree), best case h = log(n)
  ** w = maximum width of tree


=========================
  THE 3 PATTERNS
=========================

Every N-ary tree problem uses one (or a combo) of these:

PATTERN 1: Simple DFS (Top-Down)
---------------------------------
"Pass information FROM parent TO children"

    def solve(node, info_from_parent):
        if node is None:
            return
        # Use info_from_parent to compute something
        for child in node.children:
            solve(child, updated_info)

    Examples: find depth, root-to-leaf paths, check properties


PATTERN 2: Simple DFS (Bottom-Up)
----------------------------------
"Collect information FROM children, combine at parent"

    def solve(node):
        if node is None:
            return base_value
        child_results = [solve(child) for child in node.children]
        return combine(node.val, child_results)

    Examples: max depth, height, subtree size, diameter


PATTERN 3: BFS (Level-Order)
------------------------------
"Process nodes level by level"

    def solve(root):
        queue = deque([root])
        while queue:
            for _ in range(len(queue)):
                node = queue.popleft()
                # process node
                queue.extend(node.children)

    Examples: level order, min depth, rightmost nodes, widest level


=========================
  N-ARY vs BINARY TREE
=========================

  Feature              | Binary Tree              | N-ary Tree
  ---------------------|--------------------------|---------------------------
  Max children         | 2 (left, right)          | Unlimited
  Node structure       | val, left, right         | val, children[]
  In-order traversal   | Yes (left→root→right)    | Not defined (no left/right)
  Space per node       | O(1) - fixed pointers    | O(k) - k = num children
  BST property         | Yes (left < root < right)| No standard equivalent
  Height (balanced)    | O(log n)                 | O(log_k n) for k-ary
  Typical interview Q  | BST operations, LCA      | Traversals, serialize, LCA

  Converting between them:
    N-ary → Binary:  Left-Child Right-Sibling encoding
    Binary → N-ary:  Reverse of above


=========================
  APPLE-SPECIFIC TIPS
=========================

1. UIKit View Hierarchy is an N-ary tree!
   - UIView.subviews is the children array
   - hitTest() is DFS on the view tree
   - layoutSubviews() is bottom-up (post-order)

2. File System is an N-ary tree!
   - Spotlight indexing traverses the file tree
   - "Find all .swift files" = DFS with filtering

3. Swift Codable = Serialize/Deserialize pattern
   - JSONEncoder/Decoder works on tree-structured data

4. Core Data relationships can form trees
   - Parent-child entity relationships

5. Scene Kit / AR Kit scene graphs are N-ary trees


=========================
  INTERVIEW STRATEGIES
=========================

When you see a tree problem:

1. CLARIFY: Is it binary or N-ary? (They might not say)
2. ASK: Can the tree be empty (null root)? Can values repeat?
3. THINK: Which pattern? Top-down, bottom-up, or level-order?
4. START with the recursive solution (simpler to write)
5. MENTION the iterative alternative (shows depth of knowledge)
6. ANALYZE: State time O(n) and space O(h) or O(n)
7. TEST: Walk through your example tree step by step


=========================
  COMMON MISTAKES
=========================

1. Forgetting the None check at the start
2. Not handling the empty children list (leaf node)
3. Using a stack when you need a queue (DFS vs BFS mix-up)
4. Modifying the tree when you should be returning a new one
5. Not copying the path list in backtracking (paths share references!)
6. Off-by-one in depth/height (is root depth 0 or 1? CLARIFY!)
"""

# This file is documentation/reference only — no code to run.
# Review it before your interview!

if __name__ == "__main__":
    print("=" * 60)
    print("  N-ARY TREE — PATTERNS & COMPLEXITY CHEAT SHEET")
    print("=" * 60)
    print()
    print("  Read this file's docstring for the full reference.")
    print("  Key takeaway: There are only 3 patterns.")
    print()
    print("  1. DFS Top-Down  → pass info parent → child")
    print("  2. DFS Bottom-Up → collect info child → parent")
    print("  3. BFS Level     → process level by level")
    print()
    print("  Every problem is one of these three. Identify the")
    print("  pattern first, then the code writes itself.")
