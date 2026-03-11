#include "nary_tree.h"
#include <stdlib.h>

/* ===========================================================================
 * Internal helpers
 * =========================================================================== */

/* Count every node in the subtree rooted at `node` (iterative pre-order). */
static size_t count_subtree_nodes(NaryNode *node)
{
    if (!node)
        return 0;

    size_t count = 0;
    size_t cap   = 16;
    NaryNode **stack = malloc(cap * sizeof(NaryNode *));
    if (!stack)
        return 0;

    size_t top = 0;
    stack[top++] = node;

    while (top > 0) {
        NaryNode *n = stack[--top];
        count++;
        for (NaryNode *c = n->first_child; c; c = c->next_sibling) {
            if (top == cap) {
                cap *= 2;
                NaryNode **tmp = realloc(stack, cap * sizeof(NaryNode *));
                if (!tmp) {
                    free(stack);
                    return count;   /* best effort */
                }
                stack = tmp;
            }
            stack[top++] = c;
        }
    }

    free(stack);
    return count;
}

/* Unlink `node` from its parent's child list.  After this call the node's
   parent pointer is cleared and the sibling chain is patched.  Does NOT
   free anything. */
static void unlink_from_parent(NaryNode *node)
{
    NaryNode *par = node->parent;
    if (!par)
        return;

    if (par->first_child == node) {
        par->first_child = node->next_sibling;
    } else {
        NaryNode *prev = par->first_child;
        while (prev && prev->next_sibling != node)
            prev = prev->next_sibling;
        if (prev)
            prev->next_sibling = node->next_sibling;
    }
    node->parent       = NULL;
    node->next_sibling = NULL;
}

/* ===========================================================================
 * Category 1: Construction & Destruction
 * =========================================================================== */

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

    /* If parent has no children yet, this is effectively an append */
    NaryNode *prev = parent->first_child;
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

    if (tree)
        tree->node_count += count_subtree_nodes(subtree);

    return 0;
}

/* ===========================================================================
 * Category 3: Deletion & Detachment
 * =========================================================================== */

/* ---------------------------------------------------------------------------
 * nary_remove_node_promote_children
 *
 * Remove `node` from the tree and re-parent its children under the node's
 * parent, splicing them into the sibling list at the position where the
 * removed node used to be.
 *
 *   Before:  P -> ... -> N -> S -> ...       N has children C1 -> C2 -> C3
 *   After:   P -> ... -> C1 -> C2 -> C3 -> S -> ...
 *
 * The node's data is freed via tree->free_func and the node itself is freed.
 * --------------------------------------------------------------------------- */
int nary_remove_node_promote_children(NaryTree *tree, NaryNode *node)
{
    if (!node)
        return -1;

    /* Refuse to remove the root — use nary_tree_destroy or
       nary_remove_subtree for that. */
    if (!node->parent)
        return -2;

    NaryNode *par = node->parent;

    /* Gather the node's children chain head and tail */
    NaryNode *child_head = node->first_child;  /* may be NULL */
    NaryNode *child_tail = NULL;

    /* Re-parent every child and find the tail of the chain */
    for (NaryNode *c = child_head; c; c = c->next_sibling) {
        c->parent = par;
        child_tail = c;
    }

    /* Splice the children chain into the parent's child list at node's
       position.  We need to find the node's predecessor among its siblings. */
    if (par->first_child == node) {
        /* node is the first child */
        if (child_head) {
            /* Replace node with its children chain, append node's next_sibling
               after the last child */
            par->first_child = child_head;
            child_tail->next_sibling = node->next_sibling;
        } else {
            /* Node has no children — just remove it */
            par->first_child = node->next_sibling;
        }
    } else {
        /* Find the predecessor sibling */
        NaryNode *prev = par->first_child;
        while (prev->next_sibling != node)
            prev = prev->next_sibling;

        if (child_head) {
            prev->next_sibling       = child_head;
            child_tail->next_sibling = node->next_sibling;
        } else {
            prev->next_sibling = node->next_sibling;
        }
    }

    /* Free the removed node */
    node->first_child  = NULL;
    node->next_sibling = NULL;
    node->parent       = NULL;

    NaryFreeFunc ff = tree ? tree->free_func : NULL;
    if (ff)
        ff(node->data);
    free(node);

    /* Update node count: we removed exactly 1 node (the children remain) */
    if (tree)
        tree->node_count--;

    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_remove_subtree
 *
 * Unlink `node` from its parent, then destroy the entire subtree rooted
 * at `node` (post-order).  If `node` is the tree root, the tree becomes
 * empty.
 * --------------------------------------------------------------------------- */
int nary_remove_subtree(NaryTree *tree, NaryNode *node)
{
    if (!node)
        return -1;

    /* Count before destruction so we can update node_count */
    size_t removed = 0;
    if (tree)
        removed = count_subtree_nodes(node);

    /* Unlink from parent (or clear tree root) */
    if (node->parent) {
        unlink_from_parent(node);
    } else if (tree && tree->root == node) {
        tree->root = NULL;
    }

    /* Destroy the subtree */
    NaryFreeFunc ff = tree ? tree->free_func : NULL;
    nary_subtree_destroy(node, ff);

    if (tree)
        tree->node_count -= removed;

    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_detach_subtree
 *
 * Unlink `node` from its parent without freeing.  The node becomes the
 * root of an independent sub-tree.
 * --------------------------------------------------------------------------- */
int nary_detach_subtree(NaryTree *tree, NaryNode *node)
{
    if (!node)
        return -1;
    if (!node->parent)
        return -2;

    /* Count nodes being detached so we can update tree->node_count */
    size_t detached_count = 0;
    if (tree)
        detached_count = count_subtree_nodes(node);

    unlink_from_parent(node);

    if (tree)
        tree->node_count -= detached_count;

    return 0;
}
