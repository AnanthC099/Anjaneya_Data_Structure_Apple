#include "nary_tree.h"
#include <stdlib.h>

/* ---------------------------------------------------------------------------
 * nary_node_create
 * --------------------------------------------------------------------------- */
NaryNode *nary_node_create(void *data)
{
    NaryNode *node = malloc(sizeof(NaryNode));
    if (!node)
        return NULL;

    node->data         = data;
    node->parent       = NULL;
    node->first_child  = NULL;
    node->next_sibling = NULL;
    return node;
}

/* ---------------------------------------------------------------------------
 * nary_node_destroy
 * --------------------------------------------------------------------------- */
int nary_node_destroy(NaryNode *node, NaryFreeFunc free_func)
{
    if (!node)
        return -1;
    if (node->first_child)
        return -2;

    if (free_func)
        free_func(node->data);
    free(node);
    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_subtree_destroy  (iterative post-order — no recursion)
 *
 * Uses parent pointers to walk the tree without an explicit stack.
 * Algorithm:
 *   1. Descend to the deepest-leftmost leaf.
 *   2. Free it, then move to its next sibling (or back up to parent).
 *   3. Repeat until the subtree root itself is freed.
 * --------------------------------------------------------------------------- */
int nary_subtree_destroy(NaryNode *node, NaryFreeFunc free_func)
{
    if (!node)
        return -1;

    NaryNode *cursor = node;

    while (cursor) {
        /* Descend to the deepest-leftmost leaf */
        while (cursor->first_child)
            cursor = cursor->first_child;

        /* cursor is now a leaf — save its context before freeing */
        NaryNode *sibling = cursor->next_sibling;
        NaryNode *par     = cursor->parent;

        /* Free the node's data and the node itself */
        if (free_func)
            free_func(cursor->data);
        free(cursor);

        /* If we just freed the subtree root, we're done */
        if (cursor == node)
            break;

        /* Unlink cursor from its parent's child list */
        if (par)
            par->first_child = sibling;

        /* Move to the next node to process */
        if (sibling)
            cursor = sibling;
        else
            cursor = par;
    }

    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_tree_create
 * --------------------------------------------------------------------------- */
NaryTree *nary_tree_create(NaryFreeFunc free_func,
                           NaryCmpFunc  cmp_func,
                           NaryPrintFunc print_func)
{
    NaryTree *tree = malloc(sizeof(NaryTree));
    if (!tree)
        return NULL;

    tree->root       = NULL;
    tree->free_func  = free_func;
    tree->cmp_func   = cmp_func;
    tree->print_func = print_func;
    tree->node_count = 0;
    return tree;
}

/* ---------------------------------------------------------------------------
 * nary_tree_destroy
 * --------------------------------------------------------------------------- */
int nary_tree_destroy(NaryTree *tree)
{
    if (!tree)
        return -1;

    if (tree->root)
        nary_subtree_destroy(tree->root, tree->free_func);

    free(tree);
    return 0;
}
