# General N-ary Tree — Design Plan (C)

## Core Concept

Each node holds a value and can have **zero or more** children — no fixed branching factor.
The two classic memory layouts to choose between (or support both):

### Layout Option A — Left-Child, Right-Sibling (LCRS)

Each node stores only two pointers: one to its **first child**, one to its **next sibling**.
Memory-lean, cache-friendlier for traversal across siblings, and every struct is a fixed size.

### Layout Option B — Dynamic Child Array

Each node stores a pointer to a **growable array** (or linked list) of children, plus a count/capacity.
More intuitive indexing (`children[i]`) but requires per-node allocation management.

> **Recommendation:** Design the API so the *user* doesn't care which layout backs it, but internally
> start with LCRS since it maps naturally to C without hidden `realloc` costs.

---

## Node-Level Data

Each node needs to carry:

- A **generic payload** (either `void*` or a typed value — decide up front whether this tree is
  type-generic via `void*` or macro-stamped).
- A **parent pointer** (enables upward traversal, ancestor queries, and O(1) detach).
- The **child/sibling linkage** (LCRS pair or dynamic array).
- Optionally a **cached subtree size or depth** if you want O(1) queries later.

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

| Operation              | Description                                                                              |
| ---------------------- | ---------------------------------------------------------------------------------------- |
| Add child (append)     | Given a parent, append a new child as the *last* child.                                  |
| Add child (prepend)    | Insert as the *first* child (O(1) in LCRS — just rewire first-child pointer).            |
| Insert child at index k| Insert among existing siblings at position k (walk k siblings in LCRS, or shift array).  |
| Attach subtree         | Take an existing detached subtree root and graft it as a child of a target node.         |

---

### 3 — Deletion & Detachment

| Operation                     | Description                                                                                          |
| ----------------------------- | ---------------------------------------------------------------------------------------------------- |
| Remove node (promote children)| Delete a node but re-parent all its children under the deleted node's parent, preserving their order. |
| Remove node (delete subtree)  | Delete the node and its entire subtree.                                                              |
| Detach subtree                | Unlink a node from its parent without freeing anything; returns an independent sub-tree.             |

---

### 4 — Traversals

| Traversal              | Description                                                        |
| ---------------------- | ------------------------------------------------------------------ |
| Pre-order              | Root → children left-to-right.                                     |
| Post-order             | Children left-to-right → root.                                     |
| Level-order (BFS)      | Using a queue.                                                     |
| Reverse level-order    | Deepest level first — BFS + stack.                                 |

**Notes:**

- All traversals should accept a **callback** (`void (*visit)(Node*, void* user_data)`) so the caller
  decides what to do at each node.
- Optionally provide an **iterative pre-order with explicit stack** for very deep trees where recursion
  would blow the call stack.

---

### 5 — Search & Query

| Operation          | Description                                                                                       |
| ------------------ | ------------------------------------------------------------------------------------------------- |
| Find by value      | DFS/BFS returning the first match (needs a comparator function pointer).                          |
| Find all by value  | Collect every match into a caller-supplied array or linked list.                                   |
| Find by path       | Given an array of child indices `[2, 0, 3]`, walk root → 2nd child → 0th child → 3rd child.      |
| Node exists        | Boolean check.                                                                                    |

---

### 6 — Structural Queries

| Query                | Description                                                       |
| -------------------- | ----------------------------------------------------------------- |
| Degree of a node     | Count of its immediate children.                                  |
| Degree of tree       | Max degree across all nodes.                                      |
| Depth of a node      | Distance from root (walk parent pointers).                        |
| Height of a node     | Longest path from that node down to a leaf (recursive).           |
| Height of tree       | Height of root.                                                   |
| Size                 | Total node count in the tree (or subtree).                        |
| Is leaf              | Node has zero children.                                           |
| Is root              | Node has no parent.                                               |
| Count leaves         | Traverse, count nodes with zero children.                         |
| Count internal nodes | Size minus leaf count.                                            |

---

### 7 — Relationship Queries

| Query                       | Description                                                                                                       |
| --------------------------- | ----------------------------------------------------------------------------------------------------------------- |
| Parent of                   | O(1) with parent pointer.                                                                                         |
| Children of                 | Return the first-child pointer (LCRS) or the array.                                                               |
| Siblings of                 | All children of the same parent, excluding self.                                                                  |
| Ancestors of                | Walk parent pointers to root, collect into a list.                                                                |
| Is ancestor / Is descendant | Walk upward from the candidate.                                                                                   |
| Lowest Common Ancestor (LCA)| Two nodes → find where ancestor paths merge. Naïve: collect both lists, compare from root. Efficient: depth + simultaneous upward walk. |
| Path between two nodes      | Path from A up to LCA, then down to B.                                                                           |

---

### 8 — Mutation / Restructuring

| Operation              | Description                                                                                              |
| ---------------------- | -------------------------------------------------------------------------------------------------------- |
| Move subtree           | Detach from current parent, attach under a new parent.                                                   |
| Swap two siblings      | Exchange positions among children of the same parent.                                                    |
| Swap two subtrees      | Exchange positions of two nodes (even with different parents).                                            |
| Sort children          | Given a comparator, reorder children of a specific node (or recursively for every node).                 |
| Flatten tree           | Restructure so all nodes become direct children of the root (depth becomes 1).                           |
| Merge trees            | Given two trees, make one the child of the other's root (or create a new root over both).                |

---

### 9 — Serialization & Deserialization

| Format                    | Description                                                                    |
| ------------------------- | ------------------------------------------------------------------------------ |
| Parenthesized string      | e.g. `A(B(E F) C D(G))` — useful for debugging. Read back to reconstruct.     |
| Bracket / indent text     | Pretty-print with indentation levels.                                          |
| Binary file               | Write nodes in pre-order with child-count markers; read back.                  |
| DOT format (Graphviz)     | For visualization — extremely helpful for debugging.                           |

---

### 10 — Copying & Comparison

| Operation              | Description                                                                                    |
| ---------------------- | ---------------------------------------------------------------------------------------------- |
| Deep clone             | Produce an independent copy of the entire tree.                                                |
| Structural equality    | Two trees have identical shape (ignore values).                                                |
| Full equality          | Identical shape *and* identical values at every corresponding position (needs comparator).     |
| Mirror / Reflect       | Reverse the order of children at every node.                                                   |
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

## Design Decisions to Lock Down Before Coding

### 1. Generic or Typed?

`void*` payload with function pointers for compare/print/free, **or** macro-generated
type-specific trees?

### 2. LCRS vs Dynamic Array?

LCRS is simpler in C; dynamic array is friendlier for random child access.
You could also do a hybrid: LCRS internally, but expose an API that hides it.

### 3. Thread Safety

Not needed initially, but if later: reader-writer lock at tree level, or per-node locks?

### 4. Memory Strategy

Individual `malloc` per node, **or** an arena/pool allocator for bulk allocation and fast teardown?

### 5. Error Handling Convention

Return codes, errno-style, or a result struct?

### 6. Iterator Pattern

C doesn't have native iterators, but you can design a `TreeIterator` struct that holds a
stack/queue internally and exposes `next()` for external iteration without callbacks.
