# Binary Tree — Design Plan (C)

## Core Concept

Each node holds a value and has at most **two** children — a **left** child and a **right** child.
This is the classical binary tree structure, not a binary *search* tree (no ordering invariant
is enforced at the tree level, though BST-style insertion is provided as a convenience).

### Memory Layout

Each node stores three pointers: `left`, `right`, and `parent`.  Fixed-size struct, no
dynamic child arrays needed.

> **Key difference from N-ary tree:** Instead of LCRS (first_child / next_sibling), each
> node explicitly stores its two children.  This makes left/right navigation O(1) and
> simplifies many algorithms (especially in-order traversal, which has no N-ary analogue).

---

## Node-Level Data

Each node carries:

- A **generic payload** (`void*` with function pointers for compare/print/free).
- A **parent pointer** (enables upward traversal, ancestor queries, and O(1) detach).
- **Left and right child pointers** (`left`, `right`).

---

## Operations

### 1 — Construction & Destruction

| Operation          | Description                                                                                  |
| ------------------ | -------------------------------------------------------------------------------------------- |
| Create node        | Allocate a node with a given value, no parent, no children.                                  |
| Destroy node       | Free a single node (only valid if it's a leaf or has already been detached).                 |
| Destroy subtree    | Recursively free a node and every descendant (post-order walk).                              |
| Destroy entire tree| Destroy subtree starting at root, then free any tree-level metadata struct.                  |

---

### 2 — Insertion

| Operation                | Description                                                                            |
| ------------------------ | -------------------------------------------------------------------------------------- |
| Set left child           | Create a node from data and set it as the left child of parent. Fails if left exists.  |
| Set right child          | Create a node from data and set it as the right child of parent. Fails if right exists.|
| Attach left subtree      | Attach an existing detached subtree as the left child. Fails if left exists.           |
| Attach right subtree     | Attach an existing detached subtree as the right child. Fails if right exists.         |

---

### 3 — Deletion & Detachment

| Operation                     | Description                                                                                          |
| ----------------------------- | ---------------------------------------------------------------------------------------------------- |
| Remove node (promote children)| Delete a node; if it has one child, that child takes its place. Fails if node has two children.      |
| Remove subtree                | Remove a node and its entire subtree from the tree.                                                  |
| Detach subtree                | Unlink a node from its parent without freeing anything; returns an independent sub-tree.             |

---

### 4 — Traversals

| Traversal              | Description                                                        |
| ---------------------- | ------------------------------------------------------------------ |
| Pre-order              | Root → left → right.                                               |
| In-order               | Left → root → right (unique to binary trees).                      |
| Post-order             | Left → right → root.                                               |
| Level-order (BFS)      | Using a queue.                                                     |
| Reverse level-order    | Deepest level first — BFS + stack.                                 |

**Notes:**

- All traversals accept a **callback** (`void (*visit)(BtNode*, void* user_data)`).
- In-order traversal is the key traversal unique to binary trees; it produces sorted
  output when used on a BST.

---

### 5 — Search & Query

| Operation          | Description                                                                                       |
| ------------------ | ------------------------------------------------------------------------------------------------- |
| Find by value      | DFS returning the first match (needs a comparator function pointer).                              |
| Find all by value  | Collect every match into a caller-supplied array.                                                 |
| Node exists        | Boolean check.                                                                                    |

---

### 6 — Structural Queries

| Query                | Description                                                       |
| -------------------- | ----------------------------------------------------------------- |
| Depth of a node      | Distance from root (walk parent pointers).                        |
| Height of a node     | Longest path from that node down to a leaf.                       |
| Height of tree       | Height of root.                                                   |
| Size                 | Total node count in the tree (or subtree).                        |
| Is leaf              | Node has zero children.                                           |
| Is root              | Node has no parent.                                               |
| Count leaves         | Traverse, count nodes with zero children.                         |
| Count internal nodes | Size minus leaf count.                                            |
| Is full              | Every node has 0 or 2 children (binary-tree-specific).            |
| Is perfect           | All internal nodes have 2 children and all leaves at same depth.  |
| Is balanced          | Height difference between left and right subtrees <= 1 at every node. |

---

### 7 — Relationship Queries

| Query                       | Description                                                                                                       |
| --------------------------- | ----------------------------------------------------------------------------------------------------------------- |
| Parent of                   | O(1) with parent pointer.                                                                                         |
| Left child of               | O(1).                                                                                                             |
| Right child of              | O(1).                                                                                                             |
| Sibling of                  | The other child of the same parent.                                                                               |
| Ancestors of                | Walk parent pointers to root, collect into a list.                                                                |
| Is ancestor / Is descendant | Walk upward from the candidate.                                                                                   |
| Lowest Common Ancestor (LCA)| Two nodes → find where ancestor paths merge.                                                                      |
| Path between two nodes      | Path from A up to LCA, then down to B.                                                                           |

---

### 8 — Mutation / Restructuring

| Operation              | Description                                                                                              |
| ---------------------- | -------------------------------------------------------------------------------------------------------- |
| Move subtree           | Detach from current parent, attach under a new parent (left or right).                                   |
| Swap children          | Swap the left and right children of a node.                                                              |
| Swap two subtrees      | Exchange positions of two nodes (even with different parents).                                            |
| Mirror / Reflect       | Reverse left and right at every node (in-place).                                                         |
| Flatten to linked list | Restructure into a right-skewed tree following pre-order (like the classic interview problem).            |
| Merge trees            | Given two trees, create a new root over both.                                                            |

---

### 9 — Serialization & Deserialization

| Format                    | Description                                                                    |
| ------------------------- | ------------------------------------------------------------------------------ |
| Parenthesized string      | e.g. `A(B(D E) C(- F))` — `-` marks NULL children. Read back to reconstruct. |
| Bracket / indent text     | Pretty-print with indentation levels.                                          |
| DOT format (Graphviz)     | For visualization.                                                             |

---

### 10 — Copying & Comparison

| Operation              | Description                                                                                    |
| ---------------------- | ---------------------------------------------------------------------------------------------- |
| Deep clone             | Produce an independent copy of the entire tree.                                                |
| Structural equality    | Two trees have identical shape (ignore values).                                                |
| Full equality          | Identical shape AND identical values at every position (needs comparator).                     |
| Is mirror              | Check if two trees are mirrors of each other.                                                  |

---

### 11 — Utility / Statistics

| Operation                      | Description                                                                    |
| ------------------------------ | ------------------------------------------------------------------------------ |
| Diameter                       | Longest path (in edges) between any two nodes in the tree.                     |
| Width at level k               | Count of nodes at a given depth.                                               |
| Max width                      | The level with the most nodes.                                                 |
| Collect all root-to-leaf paths | Useful for debugging and certain algorithms.                                   |
| Apply / Map                    | Apply a transformation function to every node's value in-place.                |
| Fold / Reduce                  | Aggregate all values using an accumulator (post-order natural).                |

---

## Design Decisions

### 1. Generic via `void*`

Same approach as the N-ary tree: `void*` payload with function pointers for compare/print/free.

### 2. Explicit Left/Right Pointers

Each node stores `left` and `right` directly — the natural layout for a binary tree.

### 3. Parent Pointer

Included for O(1) upward traversal, ancestor queries, and detach operations.

### 4. Memory Strategy

Individual `malloc` per node.

### 5. Error Handling Convention

Return codes: `0` = success, `-1` = NULL input, `-2` = precondition violation.

### 6. Thread Safety

Not needed initially.
