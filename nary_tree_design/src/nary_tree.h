#ifndef NARY_TREE_H
#define NARY_TREE_H

#include <stddef.h>
#include <stdio.h>

/* ---------------------------------------------------------------------------
 * Function-pointer typedefs for generic operations on void* data.
 * --------------------------------------------------------------------------- */
typedef void (*NaryFreeFunc)(void *data);
typedef int  (*NaryCmpFunc)(const void *a, const void *b);
typedef void (*NaryPrintFunc)(const void *data, FILE *out);

/* ---------------------------------------------------------------------------
 * NaryNode — LCRS (Left-Child, Right-Sibling) layout.
 *
 *   first_child  -> leftmost child (NULL if leaf)
 *   next_sibling -> next sibling to the right (NULL if last)
 *   parent       -> parent node (NULL if root or detached)
 * --------------------------------------------------------------------------- */
typedef struct NaryNode {
    void            *data;
    struct NaryNode *parent;
    struct NaryNode *first_child;
    struct NaryNode *next_sibling;
} NaryNode;

/* ---------------------------------------------------------------------------
 * NaryTree — Tree-level metadata wrapper around a root node.
 * --------------------------------------------------------------------------- */
typedef struct NaryTree {
    NaryNode       *root;
    NaryFreeFunc    free_func;
    NaryCmpFunc     cmp_func;
    NaryPrintFunc   print_func;
    size_t          node_count;
} NaryTree;

/* Callback for traversal: called once per node with optional user context. */
typedef void (*NaryVisitFunc)(NaryNode *node, void *user_data);

/* Callback for map/apply: transforms a node's data in-place. */
typedef void *(*NaryMapFunc)(void *data, void *user_data);

/* Callback for fold/reduce: accumulates a result across nodes. */
typedef void *(*NaryFoldFunc)(void *accumulator, void *data, void *user_data);

/* ===========================================================================
 * Category 1: Construction & Destruction
 *
 * Return codes:  0 = success, -1 = NULL input, -2 = precondition violation.
 * =========================================================================== */

/* Allocate a node with the given data. All pointers initialised to NULL.
   Returns NULL on allocation failure. */
NaryNode *nary_node_create(void *data);

/* Free a single node. Returns -2 if the node still has children.
   Calls free_func(node->data) if free_func is non-NULL.
   Does NOT unlink the node from its parent or siblings. */
int nary_node_destroy(NaryNode *node, NaryFreeFunc free_func);

/* Iteratively free a node and all its descendants (post-order, no recursion).
   Calls free_func on each node's data if free_func is non-NULL.
   Does NOT unlink the root of the subtree from its parent. */
int nary_subtree_destroy(NaryNode *node, NaryFreeFunc free_func);

/* Allocate and initialise a tree metadata struct.
   Any function pointer may be NULL. */
NaryTree *nary_tree_create(NaryFreeFunc free_func,
                           NaryCmpFunc  cmp_func,
                           NaryPrintFunc print_func);

/* Destroy every node in the tree, then free the NaryTree struct itself. */
int nary_tree_destroy(NaryTree *tree);

/* ===========================================================================
 * Category 2: Insertion
 *
 * Return codes:  0 = success, -1 = NULL input, -2 = precondition violation.
 * =========================================================================== */

/* Append a new child (created from `data`) as the last child of `parent`.
   Walks the sibling list to find the tail — O(k) where k = current child count.
   Returns 0 on success, -1 if parent is NULL, -2 on allocation failure. */
int nary_add_child_append(NaryTree *tree, NaryNode *parent, void *data);

/* Prepend a new child (created from `data`) as the first child of `parent`.
   O(1) — just rewires the first_child pointer.
   Returns 0 on success, -1 if parent is NULL, -2 on allocation failure. */
int nary_add_child_prepend(NaryTree *tree, NaryNode *parent, void *data);

/* Insert a new child (created from `data`) at position `index` among
   `parent`'s children. index == 0 means prepend, index >= child_count means
   append.
   Returns 0 on success, -1 if parent is NULL, -2 on allocation failure. */
int nary_insert_child_at(NaryTree *tree, NaryNode *parent,
                         size_t index, void *data);

/* Attach an existing detached subtree `subtree` as the last child of `parent`.
   The subtree must not already have a parent (i.e. subtree->parent == NULL).
   Returns 0 on success, -1 if parent or subtree is NULL,
   -2 if subtree already has a parent. */
int nary_attach_subtree(NaryTree *tree, NaryNode *parent, NaryNode *subtree);

/* ===========================================================================
 * Category 3: Deletion & Detachment
 *
 * Return codes:  0 = success, -1 = NULL input, -2 = precondition violation.
 * =========================================================================== */

/* Remove a node from the tree but re-parent all its children under the
   node's parent, preserving sibling order.  The removed node's data is
   freed via free_func (if non-NULL) and the node itself is freed.
   Returns -1 if node is NULL, -2 if the node is the tree root. */
int nary_remove_node_promote_children(NaryTree *tree, NaryNode *node);

/* Remove a node and its entire subtree from the tree.  Unlinks the node
   from its parent's child list, then destroys the subtree (post-order).
   Returns -1 if node is NULL.  If node is the root the tree becomes empty. */
int nary_remove_subtree(NaryTree *tree, NaryNode *node);

/* Detach a node from its parent without freeing anything.  The node
   becomes the root of an independent sub-tree (parent set to NULL,
   unlinked from sibling chain).
   Returns -1 if node is NULL, -2 if node has no parent (already detached). */
int nary_detach_subtree(NaryTree *tree, NaryNode *node);

/* ===========================================================================
 * Category 4: Traversals
 *
 * All traversals accept a callback and optional user_data context pointer.
 * Return codes:  0 = success, -1 = NULL input.
 * =========================================================================== */

/* Pre-order: Root, then children left-to-right (recursive). */
int nary_traverse_pre_order(NaryNode *node, NaryVisitFunc visit,
                            void *user_data);

/* Post-order: Children left-to-right, then root (recursive). */
int nary_traverse_post_order(NaryNode *node, NaryVisitFunc visit,
                             void *user_data);

/* Level-order (BFS): Uses a queue. */
int nary_traverse_level_order(NaryNode *node, NaryVisitFunc visit,
                              void *user_data);

/* Reverse level-order: Deepest level first, using BFS + stack. */
int nary_traverse_reverse_level_order(NaryNode *node, NaryVisitFunc visit,
                                      void *user_data);

/* ===========================================================================
 * Category 5: Search & Query
 *
 * Return codes for int-returning functions:
 *   0 = success / found, -1 = NULL input, -2 = not found.
 * =========================================================================== */

/* DFS to find the first node whose data matches `target` via `cmp`.
   Returns the matching node, or NULL if not found or inputs are NULL. */
NaryNode *nary_find_by_value(NaryNode *root, const void *target,
                             NaryCmpFunc cmp);

/* Collect every node whose data matches `target` into `results`.
   `results` must be pre-allocated with at least `max_results` slots.
   Returns the number of matches found, or -1 on NULL input. */
int nary_find_all_by_value(NaryNode *root, const void *target,
                           NaryCmpFunc cmp,
                           NaryNode **results, size_t max_results);

/* Walk from `root` following child indices in `path` array of length `len`.
   E.g. path = {2, 0, 3} walks root -> 2nd child -> 0th child -> 3rd child.
   Returns the target node, or NULL if path is invalid. */
NaryNode *nary_find_by_path(NaryNode *root, const size_t *path, size_t len);

/* Boolean check: does a node with data matching `target` exist? */
int nary_node_exists(NaryNode *root, const void *target, NaryCmpFunc cmp);

/* ===========================================================================
 * Category 6: Structural Queries
 * =========================================================================== */

/* Count of immediate children of `node`. Returns 0 if node is NULL. */
size_t nary_degree(const NaryNode *node);

/* Maximum degree across all nodes in the subtree rooted at `node`. */
size_t nary_tree_degree(const NaryNode *node);

/* Distance from `node` up to the root (walk parent pointers). Root = 0. */
size_t nary_depth(const NaryNode *node);

/* Longest path from `node` down to a leaf. Leaf = 0. */
size_t nary_height(const NaryNode *node);

/* Total node count in the subtree rooted at `node`. */
size_t nary_size(const NaryNode *node);

/* 1 if node is a leaf (no children), 0 otherwise. */
int nary_is_leaf(const NaryNode *node);

/* 1 if node has no parent, 0 otherwise. */
int nary_is_root(const NaryNode *node);

/* Count of leaf nodes in the subtree rooted at `node`. */
size_t nary_count_leaves(const NaryNode *node);

/* Count of internal (non-leaf) nodes in the subtree rooted at `node`. */
size_t nary_count_internal(const NaryNode *node);

/* ===========================================================================
 * Category 7: Relationship Queries
 * =========================================================================== */

/* Return the parent of `node`, or NULL. */
NaryNode *nary_parent_of(const NaryNode *node);

/* Return the first child of `node`, or NULL. */
NaryNode *nary_children_of(const NaryNode *node);

/* Collect all siblings of `node` (same parent, excluding self) into `out`.
   `out` must be pre-allocated with at least `max` slots.
   Returns number of siblings collected, or -1 on NULL input. */
int nary_siblings_of(const NaryNode *node, NaryNode **out, size_t max);

/* Collect ancestors from `node` up to root into `out` (parent first, root last).
   `out` must be pre-allocated with at least `max` slots.
   Returns the number of ancestors collected, or -1 on NULL input. */
int nary_ancestors_of(const NaryNode *node, NaryNode **out, size_t max);

/* 1 if `ancestor` is an ancestor of `descendant`, 0 otherwise. */
int nary_is_ancestor(const NaryNode *ancestor, const NaryNode *descendant);

/* 1 if `descendant` is a descendant of `ancestor`, 0 otherwise. */
int nary_is_descendant(const NaryNode *descendant, const NaryNode *ancestor);

/* Lowest Common Ancestor of nodes `a` and `b`.
   Returns the LCA node, or NULL if they are not in the same tree. */
NaryNode *nary_lca(const NaryNode *a, const NaryNode *b);

/* Path from node `a` to node `b` (through their LCA).
   `path` must be pre-allocated with at least `max` slots.
   Returns the path length, or -1 on NULL input, -2 if no common root. */
int nary_path_between(const NaryNode *a, const NaryNode *b,
                      NaryNode **path, size_t max);

/* ===========================================================================
 * Category 8: Mutation & Restructuring
 *
 * Return codes:  0 = success, -1 = NULL input, -2 = precondition violation.
 * =========================================================================== */

/* Detach subtree rooted at `node` from its current parent and attach it
   as the last child of `new_parent`. */
int nary_move_subtree(NaryTree *tree, NaryNode *node, NaryNode *new_parent);

/* Swap the positions of two siblings (children of the same parent).
   Returns -2 if they do not share a parent. */
int nary_swap_siblings(NaryNode *a, NaryNode *b);

/* Swap the positions of two nodes (may have different parents).
   Returns -2 if either node is an ancestor of the other. */
int nary_swap_subtrees(NaryTree *tree, NaryNode *a, NaryNode *b);

/* Sort the children of `node` using `cmp`. */
int nary_sort_children(NaryNode *node, NaryCmpFunc cmp);

/* Flatten the tree: all nodes become direct children of the root (depth 1).
   Uses pre-order to determine child ordering. */
int nary_flatten(NaryTree *tree);

/* Merge two trees: `other`'s root becomes the last child of `tree`'s root.
   After merge, `other` is destroyed (the struct, not the nodes). */
int nary_merge_trees(NaryTree *tree, NaryTree *other);

/* ===========================================================================
 * Category 9: Serialization & Deserialization
 * =========================================================================== */

/* Parenthesized string: e.g. "A(B(E F) C D(G))".
   Writes to `buf` of size `buf_size`. Returns number of chars written,
   or -1 on NULL input. */
int nary_serialize_parenthesized(const NaryNode *node, NaryPrintFunc print,
                                 char *buf, size_t buf_size);

/* Pretty-print with indentation to `out` stream.
   Returns 0 on success, -1 on NULL input. */
int nary_print_indented(const NaryNode *node, NaryPrintFunc print,
                        FILE *out);

/* DOT format (Graphviz) output to `out` stream.
   Returns 0 on success, -1 on NULL input. */
int nary_serialize_dot(const NaryNode *node, NaryPrintFunc print,
                       FILE *out);

/* ===========================================================================
 * Category 10: Copying & Comparison
 * =========================================================================== */

/* Deep clone: produce an independent copy of the subtree rooted at `node`.
   `clone_data` is called to duplicate each node's data. If NULL, data
   pointers are copied shallowly.
   Returns the root of the cloned tree, or NULL on failure. */
NaryNode *nary_deep_clone(const NaryNode *node,
                          void *(*clone_data)(const void *));

/* 1 if the two subtrees have identical shape (ignoring values), 0 otherwise. */
int nary_structural_equal(const NaryNode *a, const NaryNode *b);

/* 1 if the two subtrees have identical shape AND identical values at every
   corresponding position (using `cmp`), 0 otherwise. */
int nary_full_equal(const NaryNode *a, const NaryNode *b, NaryCmpFunc cmp);

/* Reverse the order of children at every node in the subtree (in-place).
   Returns 0 on success, -1 on NULL input. */
int nary_mirror(NaryNode *node);

/* 1 if `a` and `b` are mirrors of each other, 0 otherwise. */
int nary_is_mirror(const NaryNode *a, const NaryNode *b, NaryCmpFunc cmp);

/* ===========================================================================
 * Category 11: Utility & Statistics
 * =========================================================================== */

/* Diameter: longest path (in edges) between any two nodes.
   Returns 0 for NULL or single node. */
size_t nary_diameter(const NaryNode *node);

/* Count of nodes at depth `level` (root is level 0). */
size_t nary_width_at_level(const NaryNode *node, size_t level);

/* The maximum width across all levels.  Sets `*level_out` to the level
   that achieved it (if level_out is non-NULL). */
size_t nary_max_width(const NaryNode *node, size_t *level_out);

/* Collect all root-to-leaf paths.  Each path is an array of NaryNode pointers.
   `paths` is an array of `max_paths` NaryNode** slots.
   `path_lens` receives the length of each path.
   Returns the number of paths collected, or -1 on NULL input. */
int nary_root_to_leaf_paths(const NaryNode *node,
                            NaryNode ***paths, size_t *path_lens,
                            size_t max_paths);

/* Apply a transformation function to every node's data in-place (pre-order). */
int nary_apply(NaryNode *node, NaryMapFunc func, void *user_data);

/* Fold / Reduce: aggregate all values using an accumulator (post-order).
   Returns the final accumulated result. */
void *nary_fold(const NaryNode *node, NaryFoldFunc func,
                void *initial, void *user_data);

#endif /* NARY_TREE_H */
