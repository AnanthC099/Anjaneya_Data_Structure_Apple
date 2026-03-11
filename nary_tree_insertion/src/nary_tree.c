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

/* ===========================================================================
 * Category 2: Insertion
 * =========================================================================== */

/* ---------------------------------------------------------------------------
 * nary_add_child_append
 *
 * Append a new child as the last child of `parent`.
 * Walks the sibling chain to find the tail — O(k).
 * --------------------------------------------------------------------------- */
int nary_add_child_append(NaryTree *tree, NaryNode *parent, void *data)
{
    if (!parent)
        return -1;

    NaryNode *child = nary_node_create(data);
    if (!child)
        return -2;

    child->parent = parent;

    if (!parent->first_child) {
        parent->first_child = child;
    } else {
        NaryNode *cur = parent->first_child;
        while (cur->next_sibling)
            cur = cur->next_sibling;
        cur->next_sibling = child;
    }

    if (tree)
        tree->node_count++;

    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_add_child_prepend
 *
 * Prepend a new child as the first child of `parent`.
 * O(1) — the new node's next_sibling becomes the old first_child.
 * --------------------------------------------------------------------------- */
int nary_add_child_prepend(NaryTree *tree, NaryNode *parent, void *data)
{
    if (!parent)
        return -1;

    NaryNode *child = nary_node_create(data);
    if (!child)
        return -2;

    child->parent       = parent;
    child->next_sibling = parent->first_child;
    parent->first_child = child;

    if (tree)
        tree->node_count++;

    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_insert_child_at
 *
 * Insert a new child at position `index` among `parent`'s children.
 *   index == 0               → prepend (O(1))
 *   index >= child_count     → append  (O(k))
 *   otherwise                → splice into sibling list at position index
 * --------------------------------------------------------------------------- */
int nary_insert_child_at(NaryTree *tree, NaryNode *parent,
                         size_t index, void *data)
{
    if (!parent)
        return -1;

    /* Fast path: prepend */
    if (index == 0)
        return nary_add_child_prepend(tree, parent, data);

    NaryNode *child = nary_node_create(data);
    if (!child)
        return -2;

    child->parent = parent;

    /* Walk to the node just before the insertion point */
    NaryNode *prev = parent->first_child;

    /* If parent has no children yet, this is effectively an append */
    if (!prev) {
        parent->first_child = child;
        if (tree)
            tree->node_count++;
        return 0;
    }

    for (size_t i = 1; i < index && prev->next_sibling; i++)
        prev = prev->next_sibling;

    /* Splice the new child after `prev` */
    child->next_sibling = prev->next_sibling;
    prev->next_sibling  = child;

    if (tree)
        tree->node_count++;

    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_attach_subtree
 *
 * Graft an existing detached subtree as the last child of `parent`.
 * The subtree root must not already have a parent.
 * --------------------------------------------------------------------------- */
int nary_attach_subtree(NaryTree *tree, NaryNode *parent, NaryNode *subtree)
{
    if (!parent || !subtree)
        return -1;
    if (subtree->parent)
        return -2;

    subtree->parent = parent;

    if (!parent->first_child) {
        parent->first_child = subtree;
    } else {
        NaryNode *cur = parent->first_child;
        while (cur->next_sibling)
            cur = cur->next_sibling;
        cur->next_sibling = subtree;
    }

    /* Count nodes in the attached subtree to update tree->node_count */
    if (tree) {
        size_t count = 0;
        /* Iterative pre-order count using an explicit stack (array-based) */
        size_t cap   = 16;
        NaryNode **stack = malloc(cap * sizeof(NaryNode *));
        if (stack) {
            size_t top = 0;
            stack[top++] = subtree;
            while (top > 0) {
                NaryNode *n = stack[--top];
                count++;
                /* Push siblings right-to-left so leftmost is processed first */
                for (NaryNode *s = n->first_child; s; s = s->next_sibling) {
                    if (top == cap) {
                        cap *= 2;
                        NaryNode **tmp = realloc(stack, cap * sizeof(NaryNode *));
                        if (!tmp) {
                            free(stack);
                            stack = NULL;
                            break;
                        }
                        stack = tmp;
                    }
                    stack[top++] = s;
                }
                if (!stack)
                    break;
            }
            free(stack);
        }
        tree->node_count += count;
    }

    return 0;
}
