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
 * nary_subtree_destroy  (recursive post-order)
 * --------------------------------------------------------------------------- */
int nary_subtree_destroy(NaryNode *node, NaryFreeFunc free_func)
{
    if (!node)
        return -1;

    NaryNode *child = node->first_child;
    while (child) {
        NaryNode *next = child->next_sibling;
        nary_subtree_destroy(child, free_func);
        child = next;
    }

    if (free_func)
        free_func(node->data);
    free(node);
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
