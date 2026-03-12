#include "binary_tree.h"
#include <stdlib.h>
#include <string.h>

/* ===========================================================================
 * Internal: simple dynamic queue for BFS traversals.
 * =========================================================================== */

typedef struct BtQueue {
    BtNode **buf;
    size_t   cap;
    size_t   head;
    size_t   tail;
    size_t   count;
} BtQueue;

static BtQueue *btq_create(size_t initial_cap)
{
    BtQueue *q = malloc(sizeof(BtQueue));
    if (!q) return NULL;
    if (initial_cap < 16) initial_cap = 16;
    q->buf = malloc(sizeof(BtNode *) * initial_cap);
    if (!q->buf) { free(q); return NULL; }
    q->cap   = initial_cap;
    q->head  = 0;
    q->tail  = 0;
    q->count = 0;
    return q;
}

static void btq_destroy(BtQueue *q)
{
    if (!q) return;
    free(q->buf);
    free(q);
}

static int btq_enqueue(BtQueue *q, BtNode *node)
{
    if (q->count == q->cap) {
        size_t new_cap = q->cap * 2;
        BtNode **new_buf = malloc(sizeof(BtNode *) * new_cap);
        if (!new_buf) return -1;
        for (size_t i = 0; i < q->count; i++)
            new_buf[i] = q->buf[(q->head + i) % q->cap];
        free(q->buf);
        q->buf  = new_buf;
        q->cap  = new_cap;
        q->head = 0;
        q->tail = q->count;
    }
    q->buf[q->tail] = node;
    q->tail = (q->tail + 1) % q->cap;
    q->count++;
    return 0;
}

static BtNode *btq_dequeue(BtQueue *q)
{
    if (q->count == 0) return NULL;
    BtNode *node = q->buf[q->head];
    q->head = (q->head + 1) % q->cap;
    q->count--;
    return node;
}

/* ===========================================================================
 * Internal: simple dynamic stack for iterative algorithms.
 * =========================================================================== */

typedef struct BtStack {
    BtNode **buf;
    size_t   cap;
    size_t   top;
} BtStack;

static BtStack *bts_create(size_t initial_cap)
{
    BtStack *s = malloc(sizeof(BtStack));
    if (!s) return NULL;
    if (initial_cap < 16) initial_cap = 16;
    s->buf = malloc(sizeof(BtNode *) * initial_cap);
    if (!s->buf) { free(s); return NULL; }
    s->cap = initial_cap;
    s->top = 0;
    return s;
}

static void bts_destroy(BtStack *s)
{
    if (!s) return;
    free(s->buf);
    free(s);
}

static int bts_push(BtStack *s, BtNode *node)
{
    if (s->top == s->cap) {
        size_t new_cap = s->cap * 2;
        BtNode **new_buf = realloc(s->buf, sizeof(BtNode *) * new_cap);
        if (!new_buf) return -1;
        s->buf = new_buf;
        s->cap = new_cap;
    }
    s->buf[s->top++] = node;
    return 0;
}

static BtNode *bts_pop(BtStack *s)
{
    if (s->top == 0) return NULL;
    return s->buf[--s->top];
}

/* ===========================================================================
 * Internal helper: count subtree size (used by deletion accounting).
 * =========================================================================== */
static size_t bt_size_internal(const BtNode *node)
{
    if (!node) return 0;
    return 1 + bt_size_internal(node->left) + bt_size_internal(node->right);
}

/* ===========================================================================
 * Category 1: Construction & Destruction
 * =========================================================================== */

BtNode *bt_node_create(void *data)
{
    BtNode *node = malloc(sizeof(BtNode));
    if (!node)
        return NULL;

    node->data   = data;
    node->parent = NULL;
    node->left   = NULL;
    node->right  = NULL;
    return node;
}

int bt_node_destroy(BtNode *node, BtFreeFunc free_func)
{
    if (!node)
        return -1;
    if (node->left || node->right)
        return -2;

    if (free_func)
        free_func(node->data);
    free(node);
    return 0;
}

int bt_subtree_destroy(BtNode *node, BtFreeFunc free_func)
{
    if (!node)
        return -1;

    bt_subtree_destroy(node->left, free_func);
    bt_subtree_destroy(node->right, free_func);

    if (free_func)
        free_func(node->data);
    free(node);
    return 0;
}

BtTree *bt_tree_create(BtFreeFunc free_func,
                       BtCmpFunc  cmp_func,
                       BtPrintFunc print_func)
{
    BtTree *tree = malloc(sizeof(BtTree));
    if (!tree)
        return NULL;

    tree->root       = NULL;
    tree->free_func  = free_func;
    tree->cmp_func   = cmp_func;
    tree->print_func = print_func;
    tree->node_count = 0;
    return tree;
}

int bt_tree_destroy(BtTree *tree)
{
    if (!tree)
        return -1;

    if (tree->root)
        bt_subtree_destroy(tree->root, tree->free_func);

    free(tree);
    return 0;
}

/* ===========================================================================
 * Category 2: Insertion
 * =========================================================================== */

int bt_set_left_child(BtTree *tree, BtNode *parent, void *data)
{
    if (!tree || !parent)
        return -1;
    if (parent->left)
        return -2;

    BtNode *child = bt_node_create(data);
    if (!child)
        return -2;

    child->parent = parent;
    parent->left  = child;
    tree->node_count++;
    return 0;
}

int bt_set_right_child(BtTree *tree, BtNode *parent, void *data)
{
    if (!tree || !parent)
        return -1;
    if (parent->right)
        return -2;

    BtNode *child = bt_node_create(data);
    if (!child)
        return -2;

    child->parent  = parent;
    parent->right  = child;
    tree->node_count++;
    return 0;
}

int bt_attach_left_subtree(BtTree *tree, BtNode *parent, BtNode *subtree)
{
    if (!tree || !parent || !subtree)
        return -1;
    if (parent->left || subtree->parent)
        return -2;

    subtree->parent = parent;
    parent->left    = subtree;
    tree->node_count += bt_size_internal(subtree);
    return 0;
}

int bt_attach_right_subtree(BtTree *tree, BtNode *parent, BtNode *subtree)
{
    if (!tree || !parent || !subtree)
        return -1;
    if (parent->right || subtree->parent)
        return -2;

    subtree->parent = parent;
    parent->right   = subtree;
    tree->node_count += bt_size_internal(subtree);
    return 0;
}

/* ===========================================================================
 * Category 3: Deletion & Detachment
 * =========================================================================== */

int bt_remove_node_promote_child(BtTree *tree, BtNode *node)
{
    if (!tree || !node)
        return -1;
    if (node->left && node->right)
        return -2;

    BtNode *child = node->left ? node->left : node->right;

    if (node->parent) {
        if (node->parent->left == node)
            node->parent->left = child;
        else
            node->parent->right = child;
    } else {
        tree->root = child;
    }

    if (child)
        child->parent = node->parent;

    node->left  = NULL;
    node->right = NULL;
    if (tree->free_func)
        tree->free_func(node->data);
    free(node);
    tree->node_count--;
    return 0;
}

int bt_remove_subtree(BtTree *tree, BtNode *node)
{
    if (!tree || !node)
        return -1;

    size_t subtree_size = bt_size_internal(node);

    if (node->parent) {
        if (node->parent->left == node)
            node->parent->left = NULL;
        else
            node->parent->right = NULL;
    } else {
        tree->root = NULL;
    }

    node->parent = NULL;
    bt_subtree_destroy(node, tree->free_func);
    tree->node_count -= subtree_size;
    return 0;
}

int bt_detach_subtree(BtTree *tree, BtNode *node)
{
    if (!tree || !node)
        return -1;
    if (!node->parent)
        return -2;

    size_t subtree_size = bt_size_internal(node);

    if (node->parent->left == node)
        node->parent->left = NULL;
    else
        node->parent->right = NULL;

    node->parent = NULL;
    tree->node_count -= subtree_size;
    return 0;
}

/* ===========================================================================
 * Category 4: Traversals
 * =========================================================================== */

int bt_traverse_pre_order(BtNode *node, BtVisitFunc visit, void *user_data)
{
    if (!node || !visit)
        return -1;

    visit(node, user_data);
    if (node->left)
        bt_traverse_pre_order(node->left, visit, user_data);
    if (node->right)
        bt_traverse_pre_order(node->right, visit, user_data);
    return 0;
}

int bt_traverse_in_order(BtNode *node, BtVisitFunc visit, void *user_data)
{
    if (!node || !visit)
        return -1;

    if (node->left)
        bt_traverse_in_order(node->left, visit, user_data);
    visit(node, user_data);
    if (node->right)
        bt_traverse_in_order(node->right, visit, user_data);
    return 0;
}

int bt_traverse_post_order(BtNode *node, BtVisitFunc visit, void *user_data)
{
    if (!node || !visit)
        return -1;

    if (node->left)
        bt_traverse_post_order(node->left, visit, user_data);
    if (node->right)
        bt_traverse_post_order(node->right, visit, user_data);
    visit(node, user_data);
    return 0;
}

int bt_traverse_level_order(BtNode *node, BtVisitFunc visit, void *user_data)
{
    if (!node || !visit)
        return -1;

    BtQueue *q = btq_create(16);
    if (!q) return -1;

    btq_enqueue(q, node);
    while (q->count > 0) {
        BtNode *cur = btq_dequeue(q);
        visit(cur, user_data);
        if (cur->left)  btq_enqueue(q, cur->left);
        if (cur->right) btq_enqueue(q, cur->right);
    }

    btq_destroy(q);
    return 0;
}

int bt_traverse_reverse_level_order(BtNode *node, BtVisitFunc visit,
                                    void *user_data)
{
    if (!node || !visit)
        return -1;

    BtQueue *q = btq_create(16);
    BtStack *s = bts_create(16);
    if (!q || !s) { btq_destroy(q); bts_destroy(s); return -1; }

    btq_enqueue(q, node);
    while (q->count > 0) {
        BtNode *cur = btq_dequeue(q);
        bts_push(s, cur);
        /* Enqueue right first so left appears first when popping stack */
        if (cur->right) btq_enqueue(q, cur->right);
        if (cur->left)  btq_enqueue(q, cur->left);
    }

    while (s->top > 0) {
        BtNode *cur = bts_pop(s);
        visit(cur, user_data);
    }

    btq_destroy(q);
    bts_destroy(s);
    return 0;
}

/* ===========================================================================
 * Category 5: Search & Query
 * =========================================================================== */

BtNode *bt_find_by_value(BtNode *root, const void *target, BtCmpFunc cmp)
{
    if (!root || !cmp)
        return NULL;

    if (cmp(root->data, target) == 0)
        return root;

    BtNode *found = bt_find_by_value(root->left, target, cmp);
    if (found) return found;

    return bt_find_by_value(root->right, target, cmp);
}

static void bt_find_all_helper(BtNode *node, const void *target,
                                BtCmpFunc cmp, BtNode **results,
                                size_t max, size_t *count)
{
    if (!node || *count >= max)
        return;

    if (cmp(node->data, target) == 0)
        results[(*count)++] = node;

    bt_find_all_helper(node->left, target, cmp, results, max, count);
    bt_find_all_helper(node->right, target, cmp, results, max, count);
}

int bt_find_all_by_value(BtNode *root, const void *target, BtCmpFunc cmp,
                         BtNode **results, size_t max_results)
{
    if (!root || !cmp || !results)
        return -1;

    size_t count = 0;
    bt_find_all_helper(root, target, cmp, results, max_results, &count);
    return (int)count;
}

int bt_node_exists(BtNode *root, const void *target, BtCmpFunc cmp)
{
    if (!root || !cmp)
        return -1;

    return bt_find_by_value(root, target, cmp) ? 1 : 0;
}

/* ===========================================================================
 * Category 6: Structural Queries
 * =========================================================================== */

size_t bt_depth(const BtNode *node)
{
    if (!node) return 0;
    size_t d = 0;
    const BtNode *cur = node;
    while (cur->parent) {
        d++;
        cur = cur->parent;
    }
    return d;
}

size_t bt_height(const BtNode *node)
{
    if (!node) return 0;
    if (!node->left && !node->right) return 0;

    size_t lh = node->left  ? bt_height(node->left)  : 0;
    size_t rh = node->right ? bt_height(node->right) : 0;
    return 1 + (lh > rh ? lh : rh);
}

size_t bt_size(const BtNode *node)
{
    return bt_size_internal(node);
}

int bt_is_leaf(const BtNode *node)
{
    if (!node) return 0;
    return (!node->left && !node->right) ? 1 : 0;
}

int bt_is_root(const BtNode *node)
{
    if (!node) return 0;
    return (node->parent == NULL) ? 1 : 0;
}

size_t bt_count_leaves(const BtNode *node)
{
    if (!node) return 0;
    if (!node->left && !node->right) return 1;
    return bt_count_leaves(node->left) + bt_count_leaves(node->right);
}

size_t bt_count_internal(const BtNode *node)
{
    if (!node) return 0;
    if (!node->left && !node->right) return 0;
    return 1 + bt_count_internal(node->left) + bt_count_internal(node->right);
}

int bt_is_full(const BtNode *node)
{
    if (!node) return 1;
    /* If exactly one child is NULL, not full */
    if ((!node->left) != (!node->right)) return 0;
    return bt_is_full(node->left) && bt_is_full(node->right);
}

int bt_is_perfect(const BtNode *node)
{
    if (!node) return 1;

    size_t h = bt_height(node);
    size_t expected_size = ((size_t)1 << (h + 1)) - 1;  /* 2^(h+1) - 1 */
    return bt_size(node) == expected_size ? 1 : 0;
}

static int bt_balanced_height(const BtNode *node)
{
    if (!node) return 0;

    int lh = bt_balanced_height(node->left);
    if (lh == -1) return -1;

    int rh = bt_balanced_height(node->right);
    if (rh == -1) return -1;

    int diff = lh - rh;
    if (diff < -1 || diff > 1) return -1;

    return 1 + (lh > rh ? lh : rh);
}

int bt_is_balanced(const BtNode *node)
{
    if (!node) return 1;
    return bt_balanced_height(node) != -1 ? 1 : 0;
}

/* ===========================================================================
 * Category 7: Relationship Queries
 * =========================================================================== */

BtNode *bt_parent_of(const BtNode *node)
{
    if (!node) return NULL;
    return node->parent;
}

BtNode *bt_left_child_of(const BtNode *node)
{
    if (!node) return NULL;
    return node->left;
}

BtNode *bt_right_child_of(const BtNode *node)
{
    if (!node) return NULL;
    return node->right;
}

BtNode *bt_sibling_of(const BtNode *node)
{
    if (!node || !node->parent) return NULL;
    if (node->parent->left == node)
        return node->parent->right;
    return node->parent->left;
}

int bt_ancestors_of(const BtNode *node, BtNode **out, size_t max)
{
    if (!node || !out)
        return -1;

    size_t count = 0;
    const BtNode *cur = node->parent;
    while (cur && count < max) {
        out[count++] = (BtNode *)cur;
        cur = cur->parent;
    }
    return (int)count;
}

int bt_is_ancestor(const BtNode *ancestor, const BtNode *descendant)
{
    if (!ancestor || !descendant) return 0;

    const BtNode *cur = descendant->parent;
    while (cur) {
        if (cur == ancestor) return 1;
        cur = cur->parent;
    }
    return 0;
}

int bt_is_descendant(const BtNode *descendant, const BtNode *ancestor)
{
    return bt_is_ancestor(ancestor, descendant);
}

BtNode *bt_lca(const BtNode *a, const BtNode *b)
{
    if (!a || !b) return NULL;

    /* Get depths */
    size_t da = bt_depth(a);
    size_t db = bt_depth(b);

    const BtNode *pa = a;
    const BtNode *pb = b;

    /* Level up the deeper node */
    while (da > db) { pa = pa->parent; da--; }
    while (db > da) { pb = pb->parent; db--; }

    /* Walk up together */
    while (pa != pb) {
        pa = pa->parent;
        pb = pb->parent;
    }

    return (BtNode *)pa;
}

int bt_path_between(const BtNode *a, const BtNode *b,
                    BtNode **path, size_t max)
{
    if (!a || !b || !path)
        return -1;

    BtNode *lca = bt_lca(a, b);
    if (!lca)
        return -2;

    /* Collect path from a up to lca */
    size_t len = 0;
    const BtNode *cur = a;
    while (cur != lca && len < max) {
        path[len++] = (BtNode *)cur;
        cur = cur->parent;
    }
    if (len < max)
        path[len++] = lca;

    /* Collect path from lca down to b (reverse of b-to-lca) */
    size_t b_len = 0;
    BtNode *b_path[256];
    cur = b;
    while (cur != lca && b_len < 256) {
        b_path[b_len++] = (BtNode *)cur;
        cur = cur->parent;
    }

    /* Append b_path in reverse (skip lca, it's already in) */
    for (size_t i = b_len; i > 0 && len < max; i--)
        path[len++] = b_path[i - 1];

    return (int)len;
}

/* ===========================================================================
 * Category 8: Mutation & Restructuring
 * =========================================================================== */

int bt_swap_children(BtNode *node)
{
    if (!node)
        return -1;

    BtNode *tmp = node->left;
    node->left  = node->right;
    node->right = tmp;
    return 0;
}

int bt_swap_subtrees(BtTree *tree, BtNode *a, BtNode *b)
{
    if (!tree || !a || !b)
        return -1;
    if (a == b)
        return 0;
    if (bt_is_ancestor(a, b) || bt_is_ancestor(b, a))
        return -2;

    BtNode *pa = a->parent;
    BtNode *pb = b->parent;

    /* Swap parent linkages for a */
    if (pa) {
        if (pa->left == a) pa->left = b;
        else               pa->right = b;
    } else {
        tree->root = b;
    }

    /* Swap parent linkages for b */
    if (pb) {
        if (pb->left == b) pb->left = a;
        else               pb->right = a;
    } else {
        tree->root = a;
    }

    a->parent = pb;
    b->parent = pa;
    return 0;
}

int bt_mirror(BtNode *node)
{
    if (!node)
        return -1;

    BtNode *tmp = node->left;
    node->left  = node->right;
    node->right = tmp;

    if (node->left)  bt_mirror(node->left);
    if (node->right) bt_mirror(node->right);
    return 0;
}

int bt_flatten(BtTree *tree)
{
    if (!tree)
        return -1;
    if (!tree->root)
        return 0;

    BtNode *cur = tree->root;
    while (cur) {
        if (cur->left) {
            /* Find the rightmost node of the left subtree */
            BtNode *pred = cur->left;
            while (pred->right)
                pred = pred->right;

            /* Move right subtree under predecessor */
            pred->right = cur->right;
            if (cur->right)
                cur->right->parent = pred;

            /* Move left subtree to right */
            cur->right = cur->left;
            cur->left  = NULL;
        }
        if (cur->right)
            cur->right->parent = cur;
        cur = cur->right;
    }

    return 0;
}

int bt_merge_trees(BtTree *tree, BtTree *other, void *root_data)
{
    if (!tree || !other)
        return -1;

    BtNode *new_root = bt_node_create(root_data);
    if (!new_root)
        return -2;

    new_root->left = tree->root;
    if (tree->root)
        tree->root->parent = new_root;

    new_root->right = other->root;
    if (other->root)
        other->root->parent = new_root;

    tree->root = new_root;
    tree->node_count += other->node_count + 1;

    /* Free the other tree struct (not its nodes, they're now ours) */
    other->root = NULL;
    other->node_count = 0;
    free(other);
    return 0;
}

/* ===========================================================================
 * Category 9: Serialization & Deserialization
 * =========================================================================== */

static int bt_ser_paren_helper(const BtNode *node, BtPrintFunc print,
                                char *buf, size_t buf_size, size_t *pos)
{
    if (!node) {
        if (*pos + 1 < buf_size)
            buf[(*pos)++] = '-';
        return 0;
    }

    /* Print data to a temp buffer via print func */
    FILE *tmp = tmpfile();
    if (!tmp) return -1;
    print(node->data, tmp);
    fflush(tmp);
    long len = ftell(tmp);
    rewind(tmp);

    for (long i = 0; i < len && *pos + 1 < buf_size; i++) {
        int ch = fgetc(tmp);
        if (ch == EOF) break;
        buf[(*pos)++] = (char)ch;
    }
    fclose(tmp);

    if (node->left || node->right) {
        if (*pos + 1 < buf_size) buf[(*pos)++] = '(';
        bt_ser_paren_helper(node->left, print, buf, buf_size, pos);
        if (*pos + 1 < buf_size) buf[(*pos)++] = ' ';
        bt_ser_paren_helper(node->right, print, buf, buf_size, pos);
        if (*pos + 1 < buf_size) buf[(*pos)++] = ')';
    }

    return 0;
}

int bt_serialize_parenthesized(const BtNode *node, BtPrintFunc print,
                               char *buf, size_t buf_size)
{
    if (!node || !print || !buf || buf_size == 0)
        return -1;

    size_t pos = 0;
    bt_ser_paren_helper(node, print, buf, buf_size, &pos);
    if (pos < buf_size)
        buf[pos] = '\0';
    else
        buf[buf_size - 1] = '\0';
    return (int)pos;
}

static void bt_print_indent_helper(const BtNode *node, BtPrintFunc print,
                                    FILE *out, int depth)
{
    if (!node) return;

    for (int i = 0; i < depth; i++)
        fprintf(out, "  ");
    print(node->data, out);
    fprintf(out, "\n");

    bt_print_indent_helper(node->left, print, out, depth + 1);
    bt_print_indent_helper(node->right, print, out, depth + 1);
}

int bt_print_indented(const BtNode *node, BtPrintFunc print, FILE *out)
{
    if (!node || !print || !out)
        return -1;

    bt_print_indent_helper(node, print, out, 0);
    return 0;
}

static void bt_dot_helper(const BtNode *node, BtPrintFunc print,
                           FILE *out, size_t *id_counter)
{
    if (!node) return;

    size_t my_id = (*id_counter)++;

    fprintf(out, "  n%zu [label=\"", my_id);
    print(node->data, out);
    fprintf(out, "\"];\n");

    if (node->left) {
        size_t child_id = *id_counter;
        bt_dot_helper(node->left, print, out, id_counter);
        fprintf(out, "  n%zu -> n%zu [label=\"L\"];\n", my_id, child_id);
    }
    if (node->right) {
        size_t child_id = *id_counter;
        bt_dot_helper(node->right, print, out, id_counter);
        fprintf(out, "  n%zu -> n%zu [label=\"R\"];\n", my_id, child_id);
    }
}

int bt_serialize_dot(const BtNode *node, BtPrintFunc print, FILE *out)
{
    if (!node || !print || !out)
        return -1;

    fprintf(out, "digraph BinaryTree {\n");
    fprintf(out, "  node [shape=circle];\n");
    size_t id_counter = 0;
    bt_dot_helper(node, print, out, &id_counter);
    fprintf(out, "}\n");
    return 0;
}

/* ===========================================================================
 * Category 10: Copying & Comparison
 * =========================================================================== */

BtNode *bt_deep_clone(const BtNode *node,
                      void *(*clone_data)(const void *))
{
    if (!node) return NULL;

    void *data_copy = clone_data ? clone_data(node->data) : node->data;
    BtNode *clone = bt_node_create(data_copy);
    if (!clone) return NULL;

    clone->left = bt_deep_clone(node->left, clone_data);
    if (clone->left)
        clone->left->parent = clone;

    clone->right = bt_deep_clone(node->right, clone_data);
    if (clone->right)
        clone->right->parent = clone;

    return clone;
}

int bt_structural_equal(const BtNode *a, const BtNode *b)
{
    if (!a && !b) return 1;
    if (!a || !b) return 0;
    return bt_structural_equal(a->left, b->left) &&
           bt_structural_equal(a->right, b->right);
}

int bt_full_equal(const BtNode *a, const BtNode *b, BtCmpFunc cmp)
{
    if (!a && !b) return 1;
    if (!a || !b) return 0;
    if (!cmp) return 0;
    if (cmp(a->data, b->data) != 0) return 0;
    return bt_full_equal(a->left, b->left, cmp) &&
           bt_full_equal(a->right, b->right, cmp);
}

int bt_is_mirror(const BtNode *a, const BtNode *b, BtCmpFunc cmp)
{
    if (!a && !b) return 1;
    if (!a || !b) return 0;
    if (!cmp) return 0;
    if (cmp(a->data, b->data) != 0) return 0;
    return bt_is_mirror(a->left, b->right, cmp) &&
           bt_is_mirror(a->right, b->left, cmp);
}

/* ===========================================================================
 * Category 11: Utility & Statistics
 * =========================================================================== */

static size_t bt_diameter_helper(const BtNode *node, size_t *height_out)
{
    if (!node) {
        *height_out = 0;
        return 0;
    }

    size_t lh, rh;
    size_t ld = bt_diameter_helper(node->left, &lh);
    size_t rd = bt_diameter_helper(node->right, &rh);

    *height_out = 1 + (lh > rh ? lh : rh);

    size_t through_root = lh + rh;
    size_t max_child = ld > rd ? ld : rd;
    return through_root > max_child ? through_root : max_child;
}

size_t bt_diameter(const BtNode *node)
{
    if (!node) return 0;
    size_t h;
    return bt_diameter_helper(node, &h);
}

size_t bt_width_at_level(const BtNode *node, size_t level)
{
    if (!node) return 0;
    if (level == 0) return 1;
    return bt_width_at_level(node->left, level - 1) +
           bt_width_at_level(node->right, level - 1);
}

size_t bt_max_width(const BtNode *node, size_t *level_out)
{
    if (!node) {
        if (level_out) *level_out = 0;
        return 0;
    }

    size_t h = bt_height(node);
    size_t max_w = 0;
    size_t max_l = 0;

    for (size_t l = 0; l <= h; l++) {
        size_t w = bt_width_at_level(node, l);
        if (w > max_w) {
            max_w = w;
            max_l = l;
        }
    }

    if (level_out) *level_out = max_l;
    return max_w;
}

static void bt_paths_helper(const BtNode *node,
                             BtNode **current_path, size_t path_len,
                             BtNode ***paths, size_t *path_lens,
                             size_t max_paths, size_t *count)
{
    if (!node || *count >= max_paths)
        return;

    current_path[path_len] = (BtNode *)node;
    path_len++;

    if (!node->left && !node->right) {
        /* Leaf — store a copy of the current path */
        paths[*count] = malloc(sizeof(BtNode *) * path_len);
        if (paths[*count]) {
            memcpy(paths[*count], current_path, sizeof(BtNode *) * path_len);
            path_lens[*count] = path_len;
            (*count)++;
        }
        return;
    }

    bt_paths_helper(node->left, current_path, path_len,
                    paths, path_lens, max_paths, count);
    bt_paths_helper(node->right, current_path, path_len,
                    paths, path_lens, max_paths, count);
}

int bt_root_to_leaf_paths(const BtNode *node,
                          BtNode ***paths, size_t *path_lens,
                          size_t max_paths)
{
    if (!node || !paths || !path_lens)
        return -1;

    size_t h = bt_height(node);
    BtNode **current_path = malloc(sizeof(BtNode *) * (h + 1));
    if (!current_path) return -1;

    size_t count = 0;
    bt_paths_helper(node, current_path, 0,
                    paths, path_lens, max_paths, &count);

    free(current_path);
    return (int)count;
}

int bt_apply(BtNode *node, BtMapFunc func, void *user_data)
{
    if (!node || !func)
        return -1;

    node->data = func(node->data, user_data);
    if (node->left)  bt_apply(node->left, func, user_data);
    if (node->right) bt_apply(node->right, func, user_data);
    return 0;
}

static void *bt_fold_helper(const BtNode *node, BtFoldFunc func,
                             void *acc, void *user_data)
{
    if (!node) return acc;
    acc = bt_fold_helper(node->left, func, acc, user_data);
    acc = bt_fold_helper(node->right, func, acc, user_data);
    return func(acc, node->data, user_data);
}

void *bt_fold(const BtNode *node, BtFoldFunc func,
              void *initial, void *user_data)
{
    if (!node || !func)
        return initial;
    return bt_fold_helper(node, func, initial, user_data);
}
