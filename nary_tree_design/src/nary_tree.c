#define _POSIX_C_SOURCE 200809L
#include "nary_tree.h"
#include <stdlib.h>
#include <string.h>

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

/* Growable queue used by BFS-based algorithms. */
typedef struct {
    NaryNode **buf;
    size_t     head;
    size_t     tail;
    size_t     cap;
} NodeQueue;

static NodeQueue *queue_create(size_t initial_cap)
{
    NodeQueue *q = malloc(sizeof(NodeQueue));
    if (!q) return NULL;
    q->buf  = malloc(initial_cap * sizeof(NaryNode *));
    if (!q->buf) { free(q); return NULL; }
    q->head = 0;
    q->tail = 0;
    q->cap  = initial_cap;
    return q;
}

static void queue_destroy(NodeQueue *q)
{
    if (q) { free(q->buf); free(q); }
}

static int queue_empty(const NodeQueue *q)
{
    return q->head == q->tail;
}

static int queue_push(NodeQueue *q, NaryNode *node)
{
    if (q->tail == q->cap) {
        /* Compact or grow */
        if (q->head > 0) {
            size_t sz = q->tail - q->head;
            memmove(q->buf, q->buf + q->head, sz * sizeof(NaryNode *));
            q->head = 0;
            q->tail = sz;
        } else {
            size_t new_cap = q->cap * 2;
            NaryNode **tmp = realloc(q->buf, new_cap * sizeof(NaryNode *));
            if (!tmp) return -1;
            q->buf = tmp;
            q->cap = new_cap;
        }
    }
    q->buf[q->tail++] = node;
    return 0;
}

static NaryNode *queue_pop(NodeQueue *q)
{
    if (queue_empty(q)) return NULL;
    return q->buf[q->head++];
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
 * --------------------------------------------------------------------------- */
int nary_remove_node_promote_children(NaryTree *tree, NaryNode *node)
{
    if (!node)
        return -1;

    if (!node->parent)
        return -2;

    NaryNode *par = node->parent;

    /* Gather the node's children chain head and tail */
    NaryNode *child_head = node->first_child;
    NaryNode *child_tail = NULL;

    /* Re-parent every child and find the tail of the chain */
    for (NaryNode *c = child_head; c; c = c->next_sibling) {
        c->parent = par;
        child_tail = c;
    }

    /* Splice the children chain into the parent's child list */
    if (par->first_child == node) {
        if (child_head) {
            par->first_child = child_head;
            child_tail->next_sibling = node->next_sibling;
        } else {
            par->first_child = node->next_sibling;
        }
    } else {
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

    if (tree)
        tree->node_count--;

    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_remove_subtree
 * --------------------------------------------------------------------------- */
int nary_remove_subtree(NaryTree *tree, NaryNode *node)
{
    if (!node)
        return -1;

    size_t removed = 0;
    if (tree)
        removed = count_subtree_nodes(node);

    if (node->parent) {
        unlink_from_parent(node);
    } else if (tree && tree->root == node) {
        tree->root = NULL;
    }

    NaryFreeFunc ff = tree ? tree->free_func : NULL;
    nary_subtree_destroy(node, ff);

    if (tree)
        tree->node_count -= removed;

    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_detach_subtree
 * --------------------------------------------------------------------------- */
int nary_detach_subtree(NaryTree *tree, NaryNode *node)
{
    if (!node)
        return -1;
    if (!node->parent)
        return -2;

    size_t detached_count = 0;
    if (tree)
        detached_count = count_subtree_nodes(node);

    unlink_from_parent(node);

    if (tree)
        tree->node_count -= detached_count;

    return 0;
}

/* ===========================================================================
 * Category 4: Traversals
 * =========================================================================== */

/* ---------------------------------------------------------------------------
 * nary_traverse_pre_order  (recursive: root, then children left-to-right)
 * --------------------------------------------------------------------------- */
int nary_traverse_pre_order(NaryNode *node, NaryVisitFunc visit,
                            void *user_data)
{
    if (!node || !visit)
        return -1;

    visit(node, user_data);

    for (NaryNode *c = node->first_child; c; c = c->next_sibling)
        nary_traverse_pre_order(c, visit, user_data);

    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_traverse_post_order  (recursive: children left-to-right, then root)
 * --------------------------------------------------------------------------- */
int nary_traverse_post_order(NaryNode *node, NaryVisitFunc visit,
                             void *user_data)
{
    if (!node || !visit)
        return -1;

    for (NaryNode *c = node->first_child; c; c = c->next_sibling)
        nary_traverse_post_order(c, visit, user_data);

    visit(node, user_data);

    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_traverse_level_order  (BFS using a queue)
 * --------------------------------------------------------------------------- */
int nary_traverse_level_order(NaryNode *node, NaryVisitFunc visit,
                              void *user_data)
{
    if (!node || !visit)
        return -1;

    NodeQueue *q = queue_create(16);
    if (!q)
        return -1;

    queue_push(q, node);

    while (!queue_empty(q)) {
        NaryNode *cur = queue_pop(q);
        visit(cur, user_data);

        for (NaryNode *c = cur->first_child; c; c = c->next_sibling)
            queue_push(q, c);
    }

    queue_destroy(q);
    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_traverse_reverse_level_order  (BFS + stack reversal)
 *
 * BFS collects nodes into a buffer, then visits them in reverse order.
 * --------------------------------------------------------------------------- */
int nary_traverse_reverse_level_order(NaryNode *node, NaryVisitFunc visit,
                                      void *user_data)
{
    if (!node || !visit)
        return -1;

    /* BFS into a collector array */
    size_t cap = 16;
    size_t len = 0;
    NaryNode **collector = malloc(cap * sizeof(NaryNode *));
    if (!collector)
        return -1;

    NodeQueue *q = queue_create(16);
    if (!q) { free(collector); return -1; }

    queue_push(q, node);

    while (!queue_empty(q)) {
        NaryNode *cur = queue_pop(q);
        if (len == cap) {
            cap *= 2;
            NaryNode **tmp = realloc(collector, cap * sizeof(NaryNode *));
            if (!tmp) { free(collector); queue_destroy(q); return -1; }
            collector = tmp;
        }
        collector[len++] = cur;

        for (NaryNode *c = cur->first_child; c; c = c->next_sibling)
            queue_push(q, c);
    }

    queue_destroy(q);

    /* Visit in reverse order */
    for (size_t i = len; i > 0; i--)
        visit(collector[i - 1], user_data);

    free(collector);
    return 0;
}

/* ===========================================================================
 * Category 5: Search & Query
 * =========================================================================== */

/* ---------------------------------------------------------------------------
 * nary_find_by_value  (DFS pre-order)
 * --------------------------------------------------------------------------- */
NaryNode *nary_find_by_value(NaryNode *root, const void *target,
                             NaryCmpFunc cmp)
{
    if (!root || !cmp)
        return NULL;

    if (cmp(root->data, target) == 0)
        return root;

    for (NaryNode *c = root->first_child; c; c = c->next_sibling) {
        NaryNode *found = nary_find_by_value(c, target, cmp);
        if (found)
            return found;
    }

    return NULL;
}

/* ---------------------------------------------------------------------------
 * nary_find_all_by_value
 * --------------------------------------------------------------------------- */
typedef struct {
    NaryNode  **results;
    size_t      max;
    size_t      count;
    const void *target;
    NaryCmpFunc cmp;
} FindAllCtx;

static void find_all_visitor(NaryNode *node, void *user_data)
{
    FindAllCtx *ctx = user_data;
    if (ctx->count >= ctx->max)
        return;
    if (ctx->cmp(node->data, ctx->target) == 0)
        ctx->results[ctx->count++] = node;
}

int nary_find_all_by_value(NaryNode *root, const void *target,
                           NaryCmpFunc cmp,
                           NaryNode **results, size_t max_results)
{
    if (!root || !cmp || !results)
        return -1;

    FindAllCtx ctx = {
        .results = results,
        .max     = max_results,
        .count   = 0,
        .target  = target,
        .cmp     = cmp
    };

    nary_traverse_pre_order(root, find_all_visitor, &ctx);
    return (int)ctx.count;
}

/* ---------------------------------------------------------------------------
 * nary_find_by_path
 * --------------------------------------------------------------------------- */
NaryNode *nary_find_by_path(NaryNode *root, const size_t *path, size_t len)
{
    if (!root || (!path && len > 0))
        return NULL;

    NaryNode *cur = root;
    for (size_t i = 0; i < len; i++) {
        NaryNode *child = cur->first_child;
        for (size_t j = 0; j < path[i] && child; j++)
            child = child->next_sibling;
        if (!child)
            return NULL;
        cur = child;
    }
    return cur;
}

/* ---------------------------------------------------------------------------
 * nary_node_exists
 * --------------------------------------------------------------------------- */
int nary_node_exists(NaryNode *root, const void *target, NaryCmpFunc cmp)
{
    return nary_find_by_value(root, target, cmp) != NULL ? 1 : 0;
}

/* ===========================================================================
 * Category 6: Structural Queries
 * =========================================================================== */

/* ---------------------------------------------------------------------------
 * nary_degree — count of immediate children
 * --------------------------------------------------------------------------- */
size_t nary_degree(const NaryNode *node)
{
    if (!node)
        return 0;

    size_t count = 0;
    for (NaryNode *c = node->first_child; c; c = c->next_sibling)
        count++;
    return count;
}

/* ---------------------------------------------------------------------------
 * nary_tree_degree — max degree across all nodes
 * --------------------------------------------------------------------------- */
static void tree_degree_visitor(NaryNode *node, void *user_data)
{
    size_t *max_deg = user_data;
    size_t deg = nary_degree(node);
    if (deg > *max_deg)
        *max_deg = deg;
}

size_t nary_tree_degree(const NaryNode *node)
{
    if (!node)
        return 0;

    size_t max_deg = 0;
    nary_traverse_pre_order((NaryNode *)node, tree_degree_visitor, &max_deg);
    return max_deg;
}

/* ---------------------------------------------------------------------------
 * nary_depth — distance from node up to root
 * --------------------------------------------------------------------------- */
size_t nary_depth(const NaryNode *node)
{
    if (!node)
        return 0;

    size_t depth = 0;
    const NaryNode *cur = node;
    while (cur->parent) {
        depth++;
        cur = cur->parent;
    }
    return depth;
}

/* ---------------------------------------------------------------------------
 * nary_height — longest path from node down to a leaf
 * --------------------------------------------------------------------------- */
size_t nary_height(const NaryNode *node)
{
    if (!node)
        return 0;
    if (!node->first_child)
        return 0;

    size_t max_h = 0;
    for (NaryNode *c = node->first_child; c; c = c->next_sibling) {
        size_t h = nary_height(c);
        if (h > max_h)
            max_h = h;
    }
    return max_h + 1;
}

/* ---------------------------------------------------------------------------
 * nary_size — total node count in subtree
 * --------------------------------------------------------------------------- */
size_t nary_size(const NaryNode *node)
{
    return count_subtree_nodes((NaryNode *)node);
}

/* ---------------------------------------------------------------------------
 * nary_is_leaf
 * --------------------------------------------------------------------------- */
int nary_is_leaf(const NaryNode *node)
{
    if (!node) return 0;
    return node->first_child == NULL ? 1 : 0;
}

/* ---------------------------------------------------------------------------
 * nary_is_root
 * --------------------------------------------------------------------------- */
int nary_is_root(const NaryNode *node)
{
    if (!node) return 0;
    return node->parent == NULL ? 1 : 0;
}

/* ---------------------------------------------------------------------------
 * nary_count_leaves
 * --------------------------------------------------------------------------- */
static void count_leaves_visitor(NaryNode *node, void *user_data)
{
    size_t *count = user_data;
    if (!node->first_child)
        (*count)++;
}

size_t nary_count_leaves(const NaryNode *node)
{
    if (!node)
        return 0;

    size_t count = 0;
    nary_traverse_pre_order((NaryNode *)node, count_leaves_visitor, &count);
    return count;
}

/* ---------------------------------------------------------------------------
 * nary_count_internal
 * --------------------------------------------------------------------------- */
size_t nary_count_internal(const NaryNode *node)
{
    if (!node)
        return 0;

    return nary_size(node) - nary_count_leaves(node);
}

/* ===========================================================================
 * Category 7: Relationship Queries
 * =========================================================================== */

/* ---------------------------------------------------------------------------
 * nary_parent_of
 * --------------------------------------------------------------------------- */
NaryNode *nary_parent_of(const NaryNode *node)
{
    if (!node) return NULL;
    return node->parent;
}

/* ---------------------------------------------------------------------------
 * nary_children_of
 * --------------------------------------------------------------------------- */
NaryNode *nary_children_of(const NaryNode *node)
{
    if (!node) return NULL;
    return node->first_child;
}

/* ---------------------------------------------------------------------------
 * nary_siblings_of
 * --------------------------------------------------------------------------- */
int nary_siblings_of(const NaryNode *node, NaryNode **out, size_t max)
{
    if (!node || !out)
        return -1;
    if (!node->parent)
        return 0;

    size_t count = 0;
    for (NaryNode *c = node->parent->first_child; c; c = c->next_sibling) {
        if (c == node)
            continue;
        if (count < max)
            out[count] = c;
        count++;
    }
    return (int)count;
}

/* ---------------------------------------------------------------------------
 * nary_ancestors_of
 * --------------------------------------------------------------------------- */
int nary_ancestors_of(const NaryNode *node, NaryNode **out, size_t max)
{
    if (!node || !out)
        return -1;

    size_t count = 0;
    const NaryNode *cur = node->parent;
    while (cur) {
        if (count < max)
            out[count] = (NaryNode *)cur;
        count++;
        cur = cur->parent;
    }
    return (int)count;
}

/* ---------------------------------------------------------------------------
 * nary_is_ancestor
 * --------------------------------------------------------------------------- */
int nary_is_ancestor(const NaryNode *ancestor, const NaryNode *descendant)
{
    if (!ancestor || !descendant)
        return 0;

    const NaryNode *cur = descendant->parent;
    while (cur) {
        if (cur == ancestor)
            return 1;
        cur = cur->parent;
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_is_descendant
 * --------------------------------------------------------------------------- */
int nary_is_descendant(const NaryNode *descendant, const NaryNode *ancestor)
{
    return nary_is_ancestor(ancestor, descendant);
}

/* ---------------------------------------------------------------------------
 * nary_lca  (Lowest Common Ancestor)
 *
 * Depth-based approach: bring both nodes to the same depth, then walk up
 * together until they meet.
 * --------------------------------------------------------------------------- */
NaryNode *nary_lca(const NaryNode *a, const NaryNode *b)
{
    if (!a || !b)
        return NULL;

    size_t da = nary_depth(a);
    size_t db = nary_depth(b);

    const NaryNode *pa = a;
    const NaryNode *pb = b;

    /* Bring the deeper node up to the same depth */
    while (da > db) { pa = pa->parent; da--; }
    while (db > da) { pb = pb->parent; db--; }

    /* Walk up together until they meet */
    while (pa != pb) {
        pa = pa->parent;
        pb = pb->parent;
    }

    return (NaryNode *)pa;
}

/* ---------------------------------------------------------------------------
 * nary_path_between
 *
 * Path from `a` up to LCA, then down to `b`.
 * --------------------------------------------------------------------------- */
int nary_path_between(const NaryNode *a, const NaryNode *b,
                      NaryNode **path, size_t max)
{
    if (!a || !b || !path)
        return -1;

    NaryNode *lca = nary_lca(a, b);
    if (!lca)
        return -2;

    /* Collect path from a up to LCA */
    size_t a_len = 0;
    const NaryNode *cur = a;
    while (cur != lca) {
        a_len++;
        cur = cur->parent;
    }
    a_len++; /* include LCA */

    /* Collect path from b up to LCA */
    size_t b_len = 0;
    cur = b;
    while (cur != lca) {
        b_len++;
        cur = cur->parent;
    }
    /* b_len does NOT include LCA (already counted in a_len) */

    size_t total = a_len + b_len;

    /* Fill path: a -> ... -> LCA -> ... -> b */
    size_t idx = 0;
    cur = a;
    for (size_t i = 0; i < a_len && idx < max; i++) {
        path[idx++] = (NaryNode *)cur;
        cur = cur->parent;
    }

    /* Path from LCA to b needs to be reversed.
       Collect b's ancestors up to (not including) LCA in a temp buffer,
       then reverse them into path. */
    if (b_len > 0) {
        NaryNode **b_path = malloc(b_len * sizeof(NaryNode *));
        if (!b_path)
            return (int)idx;

        cur = b;
        for (size_t i = 0; i < b_len; i++) {
            b_path[i] = (NaryNode *)cur;
            cur = cur->parent;
        }

        /* Add in reverse order */
        for (size_t i = b_len; i > 0 && idx < max; i--)
            path[idx++] = b_path[i - 1];

        free(b_path);
    }

    return (int)total;
}

/* ===========================================================================
 * Category 8: Mutation & Restructuring
 * =========================================================================== */

/* ---------------------------------------------------------------------------
 * nary_move_subtree
 * --------------------------------------------------------------------------- */
int nary_move_subtree(NaryTree *tree, NaryNode *node, NaryNode *new_parent)
{
    if (!node || !new_parent)
        return -1;

    /* Check that new_parent is not a descendant of node */
    if (nary_is_ancestor(node, new_parent))
        return -2;

    /* Detach from current parent */
    if (node->parent) {
        size_t saved_count = tree ? tree->node_count : 0;
        int rc = nary_detach_subtree(tree, node);
        if (rc != 0)
            return rc;
        /* Restore count since we'll re-add via attach */
        if (tree)
            tree->node_count = saved_count;
    }

    /* Reattach — but node now has no parent, so attach_subtree works
       but would re-count. We handle it manually to avoid double-counting. */
    node->parent = new_parent;

    if (!new_parent->first_child) {
        new_parent->first_child = node;
    } else {
        NaryNode *cur = new_parent->first_child;
        while (cur->next_sibling)
            cur = cur->next_sibling;
        cur->next_sibling = node;
    }

    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_swap_siblings
 * --------------------------------------------------------------------------- */
int nary_swap_siblings(NaryNode *a, NaryNode *b)
{
    if (!a || !b)
        return -1;
    if (a == b)
        return 0;
    if (!a->parent || !b->parent || a->parent != b->parent)
        return -2;

    NaryNode *par = a->parent;

    /* Find predecessors of a and b in sibling chain */
    NaryNode *prev_a = NULL;
    NaryNode *prev_b = NULL;

    for (NaryNode *c = par->first_child; c; c = c->next_sibling) {
        if (c->next_sibling == a) prev_a = c;
        if (c->next_sibling == b) prev_b = c;
    }

    /* Handle adjacent nodes: ensure a comes before b */
    if (a->next_sibling == b) {
        /* a -> b -> X  =>  b -> a -> X */
        a->next_sibling = b->next_sibling;
        b->next_sibling = a;
        if (prev_a)
            prev_a->next_sibling = b;
        else
            par->first_child = b;
    } else if (b->next_sibling == a) {
        /* b -> a -> X  =>  a -> b -> X */
        b->next_sibling = a->next_sibling;
        a->next_sibling = b;
        if (prev_b)
            prev_b->next_sibling = a;
        else
            par->first_child = a;
    } else {
        /* Non-adjacent: swap next_sibling pointers and predecessor links */
        NaryNode *tmp = a->next_sibling;
        a->next_sibling = b->next_sibling;
        b->next_sibling = tmp;

        if (prev_a)
            prev_a->next_sibling = b;
        else
            par->first_child = b;

        if (prev_b)
            prev_b->next_sibling = a;
        else
            par->first_child = a;
    }

    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_swap_subtrees
 *
 * Swap positions of two nodes that may have different parents.
 * --------------------------------------------------------------------------- */
int nary_swap_subtrees(NaryTree *tree, NaryNode *a, NaryNode *b)
{
    (void)tree;

    if (!a || !b)
        return -1;
    if (a == b)
        return 0;
    /* Refuse if one is an ancestor of the other */
    if (nary_is_ancestor(a, b) || nary_is_ancestor(b, a))
        return -2;

    /* If same parent, delegate to swap_siblings */
    if (a->parent && a->parent == b->parent)
        return nary_swap_siblings(a, b);

    NaryNode *pa = a->parent;
    NaryNode *pb = b->parent;
    NaryNode *a_next = a->next_sibling;
    NaryNode *b_next = b->next_sibling;

    /* Fix a's position: replace a with b in pa's child list */
    if (pa) {
        if (pa->first_child == a) {
            pa->first_child = b;
        } else {
            NaryNode *prev = pa->first_child;
            while (prev->next_sibling != a)
                prev = prev->next_sibling;
            prev->next_sibling = b;
        }
    }

    /* Fix b's position: replace b with a in pb's child list */
    if (pb) {
        if (pb->first_child == b) {
            pb->first_child = a;
        } else {
            NaryNode *prev = pb->first_child;
            while (prev->next_sibling != b)
                prev = prev->next_sibling;
            prev->next_sibling = a;
        }
    }

    /* Swap parent and sibling pointers */
    a->parent       = pb;
    b->parent       = pa;
    a->next_sibling = b_next;
    b->next_sibling = a_next;

    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_sort_children  (insertion sort on the sibling linked list)
 * --------------------------------------------------------------------------- */
int nary_sort_children(NaryNode *node, NaryCmpFunc cmp)
{
    if (!node || !cmp)
        return -1;
    if (!node->first_child || !node->first_child->next_sibling)
        return 0; /* 0 or 1 children — already sorted */

    /* Convert to array, sort, rebuild list */
    size_t n = nary_degree(node);
    NaryNode **arr = malloc(n * sizeof(NaryNode *));
    if (!arr)
        return -1;

    NaryNode *c = node->first_child;
    for (size_t i = 0; i < n; i++) {
        arr[i] = c;
        c = c->next_sibling;
    }

    /* Insertion sort */
    for (size_t i = 1; i < n; i++) {
        NaryNode *key = arr[i];
        size_t j = i;
        while (j > 0 && cmp(arr[j - 1]->data, key->data) > 0) {
            arr[j] = arr[j - 1];
            j--;
        }
        arr[j] = key;
    }

    /* Rebuild sibling chain */
    node->first_child = arr[0];
    for (size_t i = 0; i < n - 1; i++)
        arr[i]->next_sibling = arr[i + 1];
    arr[n - 1]->next_sibling = NULL;

    free(arr);
    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_flatten — all nodes become direct children of root (pre-order)
 * --------------------------------------------------------------------------- */
int nary_flatten(NaryTree *tree)
{
    if (!tree || !tree->root)
        return -1;

    /* Collect all non-root nodes in pre-order */
    size_t total = tree->node_count;
    if (total <= 1)
        return 0;

    NaryNode **nodes = malloc(total * sizeof(NaryNode *));
    if (!nodes)
        return -1;

    size_t idx = 0;
    /* Pre-order iterative collection */
    size_t stack_cap = 16;
    size_t stack_top = 0;
    NaryNode **stack = malloc(stack_cap * sizeof(NaryNode *));
    if (!stack) { free(nodes); return -1; }

    stack[stack_top++] = tree->root;

    while (stack_top > 0) {
        NaryNode *n = stack[--stack_top];
        nodes[idx++] = n;

        /* Push children in reverse order so leftmost is processed first */
        size_t child_count = 0;
        for (NaryNode *ch = n->first_child; ch; ch = ch->next_sibling)
            child_count++;

        if (child_count > 0) {
            NaryNode **children = malloc(child_count * sizeof(NaryNode *));
            if (children) {
                NaryNode *ch = n->first_child;
                for (size_t i = 0; i < child_count; i++) {
                    children[i] = ch;
                    ch = ch->next_sibling;
                }
                for (size_t i = child_count; i > 0; i--) {
                    if (stack_top == stack_cap) {
                        stack_cap *= 2;
                        NaryNode **tmp = realloc(stack,
                                                 stack_cap * sizeof(NaryNode *));
                        if (tmp) stack = tmp;
                    }
                    if (stack_top < stack_cap)
                        stack[stack_top++] = children[i - 1];
                }
                free(children);
            }
        }
    }
    free(stack);

    /* Rebuild: root has all other nodes as direct children */
    NaryNode *root = nodes[0];
    root->first_child = NULL;
    root->parent      = NULL;

    if (idx > 1) {
        root->first_child = nodes[1];
        for (size_t i = 1; i < idx; i++) {
            nodes[i]->parent       = root;
            nodes[i]->first_child  = NULL;
            nodes[i]->next_sibling = (i + 1 < idx) ? nodes[i + 1] : NULL;
        }
    }

    free(nodes);
    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_merge_trees
 * --------------------------------------------------------------------------- */
int nary_merge_trees(NaryTree *tree, NaryTree *other)
{
    if (!tree || !other)
        return -1;
    if (!tree->root || !other->root)
        return -2;

    NaryNode *other_root = other->root;
    size_t other_count   = other->node_count;

    /* Attach other's root as last child of tree's root */
    other_root->parent = tree->root;

    if (!tree->root->first_child) {
        tree->root->first_child = other_root;
    } else {
        NaryNode *cur = tree->root->first_child;
        while (cur->next_sibling)
            cur = cur->next_sibling;
        cur->next_sibling = other_root;
    }

    tree->node_count += other_count;

    /* Free the other tree struct (not the nodes) */
    other->root       = NULL;
    other->node_count = 0;
    free(other);

    return 0;
}

/* ===========================================================================
 * Category 9: Serialization & Deserialization
 * =========================================================================== */

/* ---------------------------------------------------------------------------
 * nary_serialize_parenthesized
 *
 * Produces a string like "A(B(E F) C D(G))" by writing each node's label
 * via the print function and recursing into children.
 * --------------------------------------------------------------------------- */
static int parenthesized_helper(const NaryNode *node, NaryPrintFunc print,
                                char *buf, size_t buf_size, size_t *pos)
{
    /* Print node label into a temp buffer, then copy to output */
    char tmp[256];
    memset(tmp, 0, sizeof(tmp));
    FILE *tmpf = fmemopen(tmp, sizeof(tmp) - 1, "w");
    if (!tmpf)
        return -1;
    print(node->data, tmpf);
    fflush(tmpf);
    fclose(tmpf);

    size_t label_len = strlen(tmp);
    for (size_t i = 0; i < label_len && *pos < buf_size - 1; i++)
        buf[(*pos)++] = tmp[i];

    if (node->first_child) {
        if (*pos < buf_size - 1) buf[(*pos)++] = '(';

        for (NaryNode *c = node->first_child; c; c = c->next_sibling) {
            if (c != node->first_child && *pos < buf_size - 1)
                buf[(*pos)++] = ' ';
            parenthesized_helper(c, print, buf, buf_size, pos);
        }

        if (*pos < buf_size - 1) buf[(*pos)++] = ')';
    }

    return 0;
}

int nary_serialize_parenthesized(const NaryNode *node, NaryPrintFunc print,
                                 char *buf, size_t buf_size)
{
    if (!node || !print || !buf || buf_size == 0)
        return -1;

    size_t pos = 0;
    parenthesized_helper(node, print, buf, buf_size, &pos);
    buf[pos] = '\0';
    return (int)pos;
}

/* ---------------------------------------------------------------------------
 * nary_print_indented
 * --------------------------------------------------------------------------- */
static void print_indented_helper(const NaryNode *node, NaryPrintFunc print,
                                  FILE *out, int depth)
{
    for (int i = 0; i < depth; i++)
        fprintf(out, "  ");
    print(node->data, out);
    fprintf(out, "\n");

    for (NaryNode *c = node->first_child; c; c = c->next_sibling)
        print_indented_helper(c, print, out, depth + 1);
}

int nary_print_indented(const NaryNode *node, NaryPrintFunc print, FILE *out)
{
    if (!node || !print || !out)
        return -1;

    print_indented_helper(node, print, out, 0);
    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_serialize_dot  (Graphviz DOT format)
 * --------------------------------------------------------------------------- */
static void dot_helper(const NaryNode *node, NaryPrintFunc print, FILE *out)
{
    /* Print node definition */
    fprintf(out, "  \"%p\" [label=\"", (const void *)node);
    print(node->data, out);
    fprintf(out, "\"];\n");

    for (NaryNode *c = node->first_child; c; c = c->next_sibling) {
        fprintf(out, "  \"%p\" -> \"%p\";\n",
                (const void *)node, (const void *)c);
        dot_helper(c, print, out);
    }
}

int nary_serialize_dot(const NaryNode *node, NaryPrintFunc print, FILE *out)
{
    if (!node || !print || !out)
        return -1;

    fprintf(out, "digraph NaryTree {\n");
    fprintf(out, "  node [shape=circle];\n");
    dot_helper(node, print, out);
    fprintf(out, "}\n");
    return 0;
}

/* ===========================================================================
 * Category 10: Copying & Comparison
 * =========================================================================== */

/* ---------------------------------------------------------------------------
 * nary_deep_clone
 * --------------------------------------------------------------------------- */
NaryNode *nary_deep_clone(const NaryNode *node,
                          void *(*clone_data)(const void *))
{
    if (!node)
        return NULL;

    void *new_data = clone_data ? clone_data(node->data) : node->data;
    NaryNode *copy = nary_node_create(new_data);
    if (!copy)
        return NULL;

    /* Clone children */
    NaryNode *prev_clone = NULL;
    for (NaryNode *c = node->first_child; c; c = c->next_sibling) {
        NaryNode *child_clone = nary_deep_clone(c, clone_data);
        if (!child_clone)
            return copy;  /* partial clone — best effort */

        child_clone->parent = copy;

        if (!prev_clone)
            copy->first_child = child_clone;
        else
            prev_clone->next_sibling = child_clone;

        prev_clone = child_clone;
    }

    return copy;
}

/* ---------------------------------------------------------------------------
 * nary_structural_equal — same shape, ignoring values
 * --------------------------------------------------------------------------- */
int nary_structural_equal(const NaryNode *a, const NaryNode *b)
{
    if (!a && !b) return 1;
    if (!a || !b) return 0;

    const NaryNode *ca = a->first_child;
    const NaryNode *cb = b->first_child;

    while (ca && cb) {
        if (!nary_structural_equal(ca, cb))
            return 0;
        ca = ca->next_sibling;
        cb = cb->next_sibling;
    }

    /* Both must have exhausted children at the same time */
    return (ca == NULL && cb == NULL) ? 1 : 0;
}

/* ---------------------------------------------------------------------------
 * nary_full_equal — same shape + same values
 * --------------------------------------------------------------------------- */
int nary_full_equal(const NaryNode *a, const NaryNode *b, NaryCmpFunc cmp)
{
    if (!a && !b) return 1;
    if (!a || !b) return 0;
    if (!cmp)     return 0;

    if (cmp(a->data, b->data) != 0)
        return 0;

    const NaryNode *ca = a->first_child;
    const NaryNode *cb = b->first_child;

    while (ca && cb) {
        if (!nary_full_equal(ca, cb, cmp))
            return 0;
        ca = ca->next_sibling;
        cb = cb->next_sibling;
    }

    return (ca == NULL && cb == NULL) ? 1 : 0;
}

/* ---------------------------------------------------------------------------
 * nary_mirror — reverse children order at every node
 * --------------------------------------------------------------------------- */
int nary_mirror(NaryNode *node)
{
    if (!node)
        return -1;

    /* Reverse the sibling linked list of children */
    NaryNode *prev = NULL;
    NaryNode *cur  = node->first_child;
    while (cur) {
        NaryNode *next = cur->next_sibling;
        cur->next_sibling = prev;
        prev = cur;
        cur = next;
    }
    node->first_child = prev;

    /* Recurse into each child */
    for (NaryNode *c = node->first_child; c; c = c->next_sibling)
        nary_mirror(c);

    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_is_mirror — check if two subtrees are mirrors
 * --------------------------------------------------------------------------- */
int nary_is_mirror(const NaryNode *a, const NaryNode *b, NaryCmpFunc cmp)
{
    if (!a && !b) return 1;
    if (!a || !b) return 0;
    if (!cmp)     return 0;

    if (cmp(a->data, b->data) != 0)
        return 0;

    /* Collect children of a and b */
    size_t na = nary_degree(a);
    size_t nb = nary_degree(b);
    if (na != nb) return 0;
    if (na == 0)  return 1;

    /* Compare a's children forward against b's children backward */
    NaryNode **b_children = malloc(nb * sizeof(NaryNode *));
    if (!b_children) return 0;

    NaryNode *c = b->first_child;
    for (size_t i = 0; i < nb; i++) {
        b_children[i] = c;
        c = c->next_sibling;
    }

    c = a->first_child;
    for (size_t i = 0; i < na; i++) {
        if (!nary_is_mirror(c, b_children[nb - 1 - i], cmp)) {
            free(b_children);
            return 0;
        }
        c = c->next_sibling;
    }

    free(b_children);
    return 1;
}

/* ===========================================================================
 * Category 11: Utility & Statistics
 * =========================================================================== */

/* ---------------------------------------------------------------------------
 * nary_diameter — longest path (in edges) between any two nodes
 *
 * For each node, the diameter is the maximum of:
 *   1. The diameter of any subtree.
 *   2. The sum of the two tallest subtree heights (path goes through node).
 * --------------------------------------------------------------------------- */
static size_t diameter_helper(const NaryNode *node, size_t *height_out)
{
    if (!node) {
        *height_out = 0;
        return 0;
    }

    if (!node->first_child) {
        *height_out = 0;
        return 0;
    }

    size_t max_diam   = 0;
    size_t max_h1     = 0;  /* tallest child height */
    size_t max_h2     = 0;  /* second tallest */

    for (NaryNode *c = node->first_child; c; c = c->next_sibling) {
        size_t ch;
        size_t cd = diameter_helper(c, &ch);
        if (cd > max_diam)
            max_diam = cd;

        size_t child_h = ch + 1;
        if (child_h >= max_h1) {
            max_h2 = max_h1;
            max_h1 = child_h;
        } else if (child_h > max_h2) {
            max_h2 = child_h;
        }
    }

    /* Path through this node uses top-2 heights */
    size_t through = max_h1 + max_h2;
    if (through > max_diam)
        max_diam = through;

    *height_out = max_h1;
    return max_diam;
}

size_t nary_diameter(const NaryNode *node)
{
    if (!node)
        return 0;

    size_t h;
    return diameter_helper(node, &h);
}

/* ---------------------------------------------------------------------------
 * nary_width_at_level
 * --------------------------------------------------------------------------- */
size_t nary_width_at_level(const NaryNode *node, size_t level)
{
    if (!node)
        return 0;
    if (level == 0)
        return 1;

    size_t count = 0;
    for (NaryNode *c = node->first_child; c; c = c->next_sibling)
        count += nary_width_at_level(c, level - 1);
    return count;
}

/* ---------------------------------------------------------------------------
 * nary_max_width
 * --------------------------------------------------------------------------- */
size_t nary_max_width(const NaryNode *node, size_t *level_out)
{
    if (!node) {
        if (level_out) *level_out = 0;
        return 0;
    }

    size_t h = nary_height(node);
    size_t max_w = 0;
    size_t max_level = 0;

    for (size_t lv = 0; lv <= h; lv++) {
        size_t w = nary_width_at_level(node, lv);
        if (w > max_w) {
            max_w = w;
            max_level = lv;
        }
    }

    if (level_out)
        *level_out = max_level;
    return max_w;
}

/* ---------------------------------------------------------------------------
 * nary_root_to_leaf_paths
 * --------------------------------------------------------------------------- */
static void collect_paths(const NaryNode *node,
                          NaryNode **current_path, size_t path_len,
                          NaryNode ***paths, size_t *path_lens,
                          size_t max_paths, size_t *count)
{
    if (!node || *count >= max_paths)
        return;

    current_path[path_len] = (NaryNode *)node;
    path_len++;

    if (!node->first_child) {
        /* Leaf: copy current path into output */
        NaryNode **path_copy = malloc(path_len * sizeof(NaryNode *));
        if (path_copy) {
            memcpy(path_copy, current_path, path_len * sizeof(NaryNode *));
            paths[*count]     = path_copy;
            path_lens[*count] = path_len;
            (*count)++;
        }
    } else {
        for (NaryNode *c = node->first_child; c; c = c->next_sibling)
            collect_paths(c, current_path, path_len,
                          paths, path_lens, max_paths, count);
    }
}

int nary_root_to_leaf_paths(const NaryNode *node,
                            NaryNode ***paths, size_t *path_lens,
                            size_t max_paths)
{
    if (!node || !paths || !path_lens)
        return -1;

    /* Temp buffer for current path (max depth = tree height + 1) */
    size_t max_depth = nary_height(node) + 1;
    NaryNode **current = malloc(max_depth * sizeof(NaryNode *));
    if (!current)
        return -1;

    size_t count = 0;
    collect_paths(node, current, 0, paths, path_lens, max_paths, &count);

    free(current);
    return (int)count;
}

/* ---------------------------------------------------------------------------
 * nary_apply — transform every node's data in-place (pre-order)
 * --------------------------------------------------------------------------- */
int nary_apply(NaryNode *node, NaryMapFunc func, void *user_data)
{
    if (!node || !func)
        return -1;

    node->data = func(node->data, user_data);

    for (NaryNode *c = node->first_child; c; c = c->next_sibling)
        nary_apply(c, func, user_data);

    return 0;
}

/* ---------------------------------------------------------------------------
 * nary_fold — aggregate values post-order
 * --------------------------------------------------------------------------- */
void *nary_fold(const NaryNode *node, NaryFoldFunc func,
                void *initial, void *user_data)
{
    if (!node || !func)
        return initial;

    void *acc = initial;

    /* Post-order: children first */
    for (NaryNode *c = node->first_child; c; c = c->next_sibling)
        acc = nary_fold(c, func, acc, user_data);

    /* Then this node */
    acc = func(acc, node->data, user_data);

    return acc;
}
