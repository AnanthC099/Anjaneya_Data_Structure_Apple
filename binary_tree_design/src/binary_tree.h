#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#include <stddef.h>
#include <stdio.h>

/* ---------------------------------------------------------------------------
 * Function-pointer typedefs for generic operations on void* data.
 * --------------------------------------------------------------------------- */
typedef void (*BtFreeFunc)(void *data);
typedef int  (*BtCmpFunc)(const void *a, const void *b);
typedef void (*BtPrintFunc)(const void *data, FILE *out);

/* ---------------------------------------------------------------------------
 * BtNode — Binary tree node with explicit left/right children.
 *
 *   left   -> left child  (NULL if absent)
 *   right  -> right child (NULL if absent)
 *   parent -> parent node (NULL if root or detached)
 * --------------------------------------------------------------------------- */
typedef struct BtNode {
    void          *data;
    struct BtNode *parent;
    struct BtNode *left;
    struct BtNode *right;
} BtNode;

/* ---------------------------------------------------------------------------
 * BtTree — Tree-level metadata wrapper around a root node.
 * --------------------------------------------------------------------------- */
typedef struct BtTree {
    BtNode       *root;
    BtFreeFunc    free_func;
    BtCmpFunc     cmp_func;
    BtPrintFunc   print_func;
    size_t        node_count;
} BtTree;

/* Callback for traversal: called once per node with optional user context. */
typedef void (*BtVisitFunc)(BtNode *node, void *user_data);

/* Callback for map/apply: transforms a node's data in-place. */
typedef void *(*BtMapFunc)(void *data, void *user_data);

/* Callback for fold/reduce: accumulates a result across nodes. */
typedef void *(*BtFoldFunc)(void *accumulator, void *data, void *user_data);

/* ===========================================================================
 * Category 1: Construction & Destruction
 *
 * Return codes:  0 = success, -1 = NULL input, -2 = precondition violation.
 * =========================================================================== */

/* Allocate a node with the given data. All pointers initialised to NULL.
   Returns NULL on allocation failure. */
BtNode *bt_node_create(void *data);

/* Free a single node. Returns -2 if the node still has children.
   Calls free_func(node->data) if free_func is non-NULL.
   Does NOT unlink the node from its parent. */
int bt_node_destroy(BtNode *node, BtFreeFunc free_func);

/* Recursively free a node and all its descendants (post-order).
   Calls free_func on each node's data if free_func is non-NULL.
   Does NOT unlink the root of the subtree from its parent. */
int bt_subtree_destroy(BtNode *node, BtFreeFunc free_func);

/* Allocate and initialise a tree metadata struct.
   Any function pointer may be NULL. */
BtTree *bt_tree_create(BtFreeFunc free_func,
                       BtCmpFunc  cmp_func,
                       BtPrintFunc print_func);

/* Destroy every node in the tree, then free the BtTree struct itself. */
int bt_tree_destroy(BtTree *tree);

/* ===========================================================================
 * Category 2: Insertion
 *
 * Return codes:  0 = success, -1 = NULL input, -2 = precondition violation.
 * =========================================================================== */

/* Create a node from `data` and set it as the left child of `parent`.
   Returns -2 if parent already has a left child. */
int bt_set_left_child(BtTree *tree, BtNode *parent, void *data);

/* Create a node from `data` and set it as the right child of `parent`.
   Returns -2 if parent already has a right child. */
int bt_set_right_child(BtTree *tree, BtNode *parent, void *data);

/* Attach an existing detached subtree as the left child of `parent`.
   Returns -2 if parent already has a left child or subtree has a parent. */
int bt_attach_left_subtree(BtTree *tree, BtNode *parent, BtNode *subtree);

/* Attach an existing detached subtree as the right child of `parent`.
   Returns -2 if parent already has a right child or subtree has a parent. */
int bt_attach_right_subtree(BtTree *tree, BtNode *parent, BtNode *subtree);

/* ===========================================================================
 * Category 3: Deletion & Detachment
 *
 * Return codes:  0 = success, -1 = NULL input, -2 = precondition violation.
 * =========================================================================== */

/* Remove a node from the tree. If it has exactly one child, that child
   takes its place.  Returns -2 if the node has TWO children. */
int bt_remove_node_promote_child(BtTree *tree, BtNode *node);

/* Remove a node and its entire subtree.  Unlinks from parent, then
   destroys the subtree (post-order). */
int bt_remove_subtree(BtTree *tree, BtNode *node);

/* Detach a node from its parent without freeing anything.
   Returns -2 if node has no parent (already detached). */
int bt_detach_subtree(BtTree *tree, BtNode *node);

/* ===========================================================================
 * Category 4: Traversals
 *
 * All traversals accept a callback and optional user_data context pointer.
 * Return codes:  0 = success, -1 = NULL input.
 * =========================================================================== */

/* Pre-order: Root → left → right (recursive). */
int bt_traverse_pre_order(BtNode *node, BtVisitFunc visit, void *user_data);

/* In-order: Left → root → right (recursive). Unique to binary trees. */
int bt_traverse_in_order(BtNode *node, BtVisitFunc visit, void *user_data);

/* Post-order: Left → right → root (recursive). */
int bt_traverse_post_order(BtNode *node, BtVisitFunc visit, void *user_data);

/* Level-order (BFS): Uses a queue. */
int bt_traverse_level_order(BtNode *node, BtVisitFunc visit, void *user_data);

/* Reverse level-order: Deepest level first, using BFS + stack. */
int bt_traverse_reverse_level_order(BtNode *node, BtVisitFunc visit,
                                    void *user_data);

/* ===========================================================================
 * Category 5: Search & Query
 *
 * Return codes for int-returning functions:
 *   0 = success / found, -1 = NULL input, -2 = not found.
 * =========================================================================== */

/* DFS to find the first node whose data matches `target` via `cmp`.
   Returns the matching node, or NULL if not found. */
BtNode *bt_find_by_value(BtNode *root, const void *target, BtCmpFunc cmp);

/* Collect every node whose data matches `target` into `results`.
   `results` must be pre-allocated with at least `max_results` slots.
   Returns the number of matches found, or -1 on NULL input. */
int bt_find_all_by_value(BtNode *root, const void *target, BtCmpFunc cmp,
                         BtNode **results, size_t max_results);

/* Boolean check: does a node with data matching `target` exist?
   Returns 1 if found, 0 if not, -1 on NULL input. */
int bt_node_exists(BtNode *root, const void *target, BtCmpFunc cmp);

/* ===========================================================================
 * Category 6: Structural Queries
 * =========================================================================== */

/* Distance from `node` up to the root (walk parent pointers). Root = 0. */
size_t bt_depth(const BtNode *node);

/* Longest path from `node` down to a leaf. Leaf = 0. */
size_t bt_height(const BtNode *node);

/* Total node count in the subtree rooted at `node`. */
size_t bt_size(const BtNode *node);

/* 1 if node is a leaf (no children), 0 otherwise. */
int bt_is_leaf(const BtNode *node);

/* 1 if node has no parent, 0 otherwise. */
int bt_is_root(const BtNode *node);

/* Count of leaf nodes in the subtree rooted at `node`. */
size_t bt_count_leaves(const BtNode *node);

/* Count of internal (non-leaf) nodes in the subtree rooted at `node`. */
size_t bt_count_internal(const BtNode *node);

/* 1 if every node in the subtree has 0 or 2 children (full binary tree). */
int bt_is_full(const BtNode *node);

/* 1 if the tree is perfect (all internals have 2 children, leaves same depth). */
int bt_is_perfect(const BtNode *node);

/* 1 if the tree is height-balanced (|left_h - right_h| <= 1 at every node). */
int bt_is_balanced(const BtNode *node);

/* ===========================================================================
 * Category 7: Relationship Queries
 * =========================================================================== */

/* Return the parent of `node`, or NULL. */
BtNode *bt_parent_of(const BtNode *node);

/* Return the left child of `node`, or NULL. */
BtNode *bt_left_child_of(const BtNode *node);

/* Return the right child of `node`, or NULL. */
BtNode *bt_right_child_of(const BtNode *node);

/* Return the sibling of `node` (the other child of the same parent), or NULL. */
BtNode *bt_sibling_of(const BtNode *node);

/* Collect ancestors from `node` up to root into `out` (parent first, root last).
   `out` must be pre-allocated with at least `max` slots.
   Returns the number of ancestors collected, or -1 on NULL input. */
int bt_ancestors_of(const BtNode *node, BtNode **out, size_t max);

/* 1 if `ancestor` is an ancestor of `descendant`, 0 otherwise. */
int bt_is_ancestor(const BtNode *ancestor, const BtNode *descendant);

/* 1 if `descendant` is a descendant of `ancestor`, 0 otherwise. */
int bt_is_descendant(const BtNode *descendant, const BtNode *ancestor);

/* Lowest Common Ancestor of nodes `a` and `b`.
   Returns the LCA node, or NULL if they are not in the same tree. */
BtNode *bt_lca(const BtNode *a, const BtNode *b);

/* Path from node `a` to node `b` (through their LCA).
   `path` must be pre-allocated with at least `max` slots.
   Returns the path length, or -1 on NULL input, -2 if no common root. */
int bt_path_between(const BtNode *a, const BtNode *b,
                    BtNode **path, size_t max);

/* ===========================================================================
 * Category 8: Mutation & Restructuring
 *
 * Return codes:  0 = success, -1 = NULL input, -2 = precondition violation.
 * =========================================================================== */

/* Swap the left and right children of `node`. */
int bt_swap_children(BtNode *node);

/* Swap the positions of two nodes (may have different parents).
   Returns -2 if either node is an ancestor of the other. */
int bt_swap_subtrees(BtTree *tree, BtNode *a, BtNode *b);

/* Reverse left and right at every node in the subtree (in-place mirror). */
int bt_mirror(BtNode *node);

/* Flatten the tree into a right-skewed linked list following pre-order.
   After this, every node has NULL left and only right children. */
int bt_flatten(BtTree *tree);

/* Merge two trees: create a new root with `tree`'s root as left child
   and `other`'s root as right child.  `other` struct is freed.
   `root_data` is the data for the new root node. */
int bt_merge_trees(BtTree *tree, BtTree *other, void *root_data);

/* ===========================================================================
 * Category 9: Serialization & Deserialization
 * =========================================================================== */

/* Parenthesized string: e.g. "A(B(D E) C(- F))".
   Writes to `buf` of size `buf_size`. Returns chars written, or -1. */
int bt_serialize_parenthesized(const BtNode *node, BtPrintFunc print,
                               char *buf, size_t buf_size);

/* Pretty-print with indentation to `out` stream. */
int bt_print_indented(const BtNode *node, BtPrintFunc print, FILE *out);

/* DOT format (Graphviz) output to `out` stream. */
int bt_serialize_dot(const BtNode *node, BtPrintFunc print, FILE *out);

/* ===========================================================================
 * Category 10: Copying & Comparison
 * =========================================================================== */

/* Deep clone: produce an independent copy of the subtree rooted at `node`.
   `clone_data` duplicates each node's data. If NULL, shallow copy. */
BtNode *bt_deep_clone(const BtNode *node,
                      void *(*clone_data)(const void *));

/* 1 if the two subtrees have identical shape (ignoring values). */
int bt_structural_equal(const BtNode *a, const BtNode *b);

/* 1 if identical shape AND identical values at every position. */
int bt_full_equal(const BtNode *a, const BtNode *b, BtCmpFunc cmp);

/* 1 if `a` and `b` are mirrors of each other. */
int bt_is_mirror(const BtNode *a, const BtNode *b, BtCmpFunc cmp);

/* ===========================================================================
 * Category 11: Utility & Statistics
 * =========================================================================== */

/* Diameter: longest path (in edges) between any two nodes.
   Returns 0 for NULL or single node. */
size_t bt_diameter(const BtNode *node);

/* Count of nodes at depth `level` (root is level 0). */
size_t bt_width_at_level(const BtNode *node, size_t level);

/* Maximum width across all levels. Sets `*level_out` to the level
   that achieved it (if level_out is non-NULL). */
size_t bt_max_width(const BtNode *node, size_t *level_out);

/* Collect all root-to-leaf paths.
   `paths` is an array of `max_paths` BtNode** slots.
   `path_lens` receives the length of each path.
   Returns the number of paths collected, or -1 on NULL input. */
int bt_root_to_leaf_paths(const BtNode *node,
                          BtNode ***paths, size_t *path_lens,
                          size_t max_paths);

/* Apply a transformation function to every node's data in-place (pre-order). */
int bt_apply(BtNode *node, BtMapFunc func, void *user_data);

/* Fold / Reduce: aggregate all values using an accumulator (post-order).
   Returns the final accumulated result. */
void *bt_fold(const BtNode *node, BtFoldFunc func,
              void *initial, void *user_data);

#endif /* BINARY_TREE_H */
