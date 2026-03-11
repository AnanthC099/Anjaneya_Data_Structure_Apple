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
 *   first_child  → leftmost child (NULL if leaf)
 *   next_sibling → next sibling to the right (NULL if last)
 *   parent       → parent node (NULL if root or detached)
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

/* ---------------------------------------------------------------------------
 * Category 1: Construction & Destruction
 *
 * Return codes:  0 = success, -1 = NULL input, -2 = precondition violation.
 * --------------------------------------------------------------------------- */

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

/* ---------------------------------------------------------------------------
 * Category 2: Insertion
 *
 * Return codes:  0 = success, -1 = NULL input, -2 = precondition violation.
 * --------------------------------------------------------------------------- */

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

#endif /* NARY_TREE_H */
