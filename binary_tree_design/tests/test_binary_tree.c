#define _POSIX_C_SOURCE 200809L
#include "binary_tree.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ---- Helper: counting free function ------------------------------------ */
static int g_free_count = 0;

static void counting_free(void *data)
{
    free(data);
    g_free_count++;
}

/* ---- Helper: string comparator ----------------------------------------- */
static int str_cmp(const void *a, const void *b)
{
    return strcmp((const char *)a, (const char *)b);
}

/* ---- Helper: string print function ------------------------------------- */
static void str_print(const void *data, FILE *out)
{
    fprintf(out, "%s", (const char *)data);
}

/* ---- Helper: clone a string -------------------------------------------- */
static void *str_clone(const void *data)
{
    return strdup((const char *)data);
}

/* ---- Helper: build a 7-node test tree ----------------------------------
 *
 *           A
 *          / \
 *         B   C
 *        / \   \
 *       D   E   F
 *      /
 *     G
 */
static BtNode *build_test_tree(void)
{
    BtNode *a = bt_node_create(strdup("A"));
    BtNode *b = bt_node_create(strdup("B"));
    BtNode *c = bt_node_create(strdup("C"));
    BtNode *d = bt_node_create(strdup("D"));
    BtNode *e = bt_node_create(strdup("E"));
    BtNode *f = bt_node_create(strdup("F"));
    BtNode *g = bt_node_create(strdup("G"));

    a->left  = b;  b->parent = a;
    a->right = c;  c->parent = a;
    b->left  = d;  d->parent = b;
    b->right = e;  e->parent = b;
    c->right = f;  f->parent = c;
    d->left  = g;  g->parent = d;

    return a;
}

/* ---- Helper: collect traversal results into a string ------------------- */
typedef struct {
    char buf[256];
    size_t pos;
} TraversalResult;

static void collect_visitor(BtNode *node, void *user_data)
{
    TraversalResult *r = user_data;
    const char *s = (const char *)node->data;
    size_t len = strlen(s);
    if (r->pos + len + 1 < sizeof(r->buf)) {
        if (r->pos > 0)
            r->buf[r->pos++] = ' ';
        memcpy(r->buf + r->pos, s, len);
        r->pos += len;
        r->buf[r->pos] = '\0';
    }
}

/* ========================================================================
 * Category 1 Tests: Construction & Destruction
 * ======================================================================== */

static void test_node_create_non_null_data(void)
{
    char *s   = strdup("hello");
    BtNode *n = bt_node_create(s);
    assert(n != NULL);
    assert(n->data == s);
    assert(n->parent == NULL);
    assert(n->left == NULL);
    assert(n->right == NULL);
    free(s);
    free(n);
    printf("  [PASS] node_create with non-NULL data\n");
}

static void test_node_create_null_data(void)
{
    BtNode *n = bt_node_create(NULL);
    assert(n != NULL);
    assert(n->data == NULL);
    free(n);
    printf("  [PASS] node_create with NULL data\n");
}

static void test_node_destroy_leaf(void)
{
    g_free_count = 0;
    BtNode *n = bt_node_create(strdup("leaf"));
    int rc = bt_node_destroy(n, counting_free);
    assert(rc == 0);
    assert(g_free_count == 1);
    printf("  [PASS] node_destroy on standalone leaf\n");
}

static void test_node_destroy_null(void)
{
    int rc = bt_node_destroy(NULL, counting_free);
    assert(rc == -1);
    printf("  [PASS] node_destroy with NULL node\n");
}

static void test_node_destroy_has_children(void)
{
    BtNode *parent = bt_node_create(strdup("parent"));
    BtNode *child  = bt_node_create(strdup("child"));
    parent->left = child;
    child->parent = parent;

    int rc = bt_node_destroy(parent, counting_free);
    assert(rc == -2);
    assert(parent->data != NULL);

    free(parent->data);
    free(parent);
    free(child->data);
    free(child);
    printf("  [PASS] node_destroy refuses when node has children\n");
}

static void test_node_destroy_null_free_func(void)
{
    BtNode *n = bt_node_create((void *)(long)42);
    int rc = bt_node_destroy(n, NULL);
    assert(rc == 0);
    printf("  [PASS] node_destroy with NULL free_func\n");
}

static void test_subtree_destroy_single(void)
{
    g_free_count = 0;
    BtNode *n = bt_node_create(strdup("solo"));
    int rc = bt_subtree_destroy(n, counting_free);
    assert(rc == 0);
    assert(g_free_count == 1);
    printf("  [PASS] subtree_destroy on single node\n");
}

static void test_subtree_destroy_tree(void)
{
    g_free_count = 0;
    BtNode *root = build_test_tree();
    int rc = bt_subtree_destroy(root, counting_free);
    assert(rc == 0);
    assert(g_free_count == 7);
    printf("  [PASS] subtree_destroy on 7-node tree\n");
}

static void test_subtree_destroy_null(void)
{
    int rc = bt_subtree_destroy(NULL, counting_free);
    assert(rc == -1);
    printf("  [PASS] subtree_destroy with NULL node\n");
}

static void test_tree_create(void)
{
    BtTree *t = bt_tree_create(counting_free, str_cmp, str_print);
    assert(t != NULL);
    assert(t->root == NULL);
    assert(t->node_count == 0);
    assert(t->free_func == counting_free);
    assert(t->cmp_func == str_cmp);
    free(t);
    printf("  [PASS] tree_create with function pointers\n");
}

static void test_tree_create_null_funcs(void)
{
    BtTree *t = bt_tree_create(NULL, NULL, NULL);
    assert(t != NULL);
    assert(t->free_func == NULL);
    free(t);
    printf("  [PASS] tree_create with NULL function pointers\n");
}

static void test_tree_destroy_empty(void)
{
    BtTree *t = bt_tree_create(counting_free, NULL, NULL);
    int rc = bt_tree_destroy(t);
    assert(rc == 0);
    printf("  [PASS] tree_destroy on empty tree\n");
}

static void test_tree_destroy_populated(void)
{
    g_free_count = 0;
    BtTree *t = bt_tree_create(counting_free, NULL, NULL);
    t->root       = build_test_tree();
    t->node_count = 7;

    int rc = bt_tree_destroy(t);
    assert(rc == 0);
    assert(g_free_count == 7);
    printf("  [PASS] tree_destroy on populated tree\n");
}

static void test_tree_destroy_null(void)
{
    int rc = bt_tree_destroy(NULL);
    assert(rc == -1);
    printf("  [PASS] tree_destroy with NULL tree\n");
}

/* ========================================================================
 * Category 2 Tests: Insertion
 * ======================================================================== */

static void test_set_left_child(void)
{
    BtTree *t = bt_tree_create(counting_free, NULL, NULL);
    BtNode *root = bt_node_create(strdup("root"));
    t->root = root;
    t->node_count = 1;

    int rc = bt_set_left_child(t, root, strdup("left"));
    assert(rc == 0);
    assert(root->left != NULL);
    assert(strcmp(root->left->data, "left") == 0);
    assert(root->left->parent == root);
    assert(t->node_count == 2);

    /* Fails if left already exists */
    char *dup_data = strdup("dup");
    rc = bt_set_left_child(t, root, dup_data);
    assert(rc == -2);
    free(dup_data);  /* Not consumed on failure — caller must free */
    bt_tree_destroy(t);
    printf("  [PASS] set_left_child\n");
}

static void test_set_right_child(void)
{
    BtTree *t = bt_tree_create(counting_free, NULL, NULL);
    BtNode *root = bt_node_create(strdup("root"));
    t->root = root;
    t->node_count = 1;

    int rc = bt_set_right_child(t, root, strdup("right"));
    assert(rc == 0);
    assert(root->right != NULL);
    assert(strcmp(root->right->data, "right") == 0);
    assert(root->right->parent == root);
    assert(t->node_count == 2);

    bt_tree_destroy(t);
    printf("  [PASS] set_right_child\n");
}

static void test_attach_left_subtree(void)
{
    BtTree *t = bt_tree_create(counting_free, NULL, NULL);
    BtNode *root = bt_node_create(strdup("root"));
    t->root = root;
    t->node_count = 1;

    BtNode *sub = bt_node_create(strdup("sub"));
    BtNode *sub_child = bt_node_create(strdup("sub_child"));
    sub->left = sub_child;
    sub_child->parent = sub;

    int rc = bt_attach_left_subtree(t, root, sub);
    assert(rc == 0);
    assert(root->left == sub);
    assert(sub->parent == root);
    assert(t->node_count == 3);

    /* Fails if left already occupied */
    BtNode *sub2 = bt_node_create(strdup("sub2"));
    rc = bt_attach_left_subtree(t, root, sub2);
    assert(rc == -2);
    bt_subtree_destroy(sub2, counting_free);

    bt_tree_destroy(t);
    printf("  [PASS] attach_left_subtree\n");
}

static void test_attach_right_subtree(void)
{
    BtTree *t = bt_tree_create(counting_free, NULL, NULL);
    BtNode *root = bt_node_create(strdup("root"));
    t->root = root;
    t->node_count = 1;

    BtNode *sub = bt_node_create(strdup("sub"));
    int rc = bt_attach_right_subtree(t, root, sub);
    assert(rc == 0);
    assert(root->right == sub);
    assert(sub->parent == root);
    assert(t->node_count == 2);

    bt_tree_destroy(t);
    printf("  [PASS] attach_right_subtree\n");
}

static void test_insertion_null_inputs(void)
{
    BtTree *t = bt_tree_create(counting_free, NULL, NULL);
    char *x = strdup("x");
    assert(bt_set_left_child(t, NULL, x) == -1);
    free(x);  /* Not consumed on failure */
    assert(bt_set_left_child(NULL, NULL, NULL) == -1);
    assert(bt_attach_left_subtree(t, NULL, NULL) == -1);
    bt_tree_destroy(t);
    printf("  [PASS] insertion NULL input handling\n");
}

/* ========================================================================
 * Category 3 Tests: Deletion & Detachment
 * ======================================================================== */

static void test_remove_node_promote_child(void)
{
    g_free_count = 0;
    BtTree *t = bt_tree_create(counting_free, NULL, NULL);
    BtNode *root = bt_node_create(strdup("A"));
    t->root = root;
    t->node_count = 1;
    bt_set_left_child(t, root, strdup("B"));
    bt_set_right_child(t, root->left, strdup("C"));

    /* Remove B (has one child C) — C should take B's place */
    BtNode *b = root->left;
    int rc = bt_remove_node_promote_child(t, b);
    assert(rc == 0);
    assert(root->left != NULL);
    assert(strcmp(root->left->data, "C") == 0);
    assert(root->left->parent == root);
    assert(t->node_count == 2);

    bt_tree_destroy(t);
    printf("  [PASS] remove_node_promote_child (one child)\n");
}

static void test_remove_node_promote_child_two_children(void)
{
    BtTree *t = bt_tree_create(counting_free, NULL, NULL);
    t->root = build_test_tree();
    t->node_count = 7;

    /* B has two children (D, E) — should fail */
    int rc = bt_remove_node_promote_child(t, t->root->left);
    assert(rc == -2);

    bt_tree_destroy(t);
    printf("  [PASS] remove_node_promote_child refuses two children\n");
}

static void test_remove_subtree(void)
{
    g_free_count = 0;
    BtTree *t = bt_tree_create(counting_free, NULL, NULL);
    t->root = build_test_tree();
    t->node_count = 7;

    /* Remove left subtree (B and its descendants: B, D, E, G = 4 nodes) */
    int rc = bt_remove_subtree(t, t->root->left);
    assert(rc == 0);
    assert(t->root->left == NULL);
    assert(t->node_count == 3);  /* A, C, F */
    assert(g_free_count == 4);

    bt_tree_destroy(t);
    printf("  [PASS] remove_subtree\n");
}

static void test_detach_subtree(void)
{
    BtTree *t = bt_tree_create(counting_free, NULL, NULL);
    t->root = build_test_tree();
    t->node_count = 7;

    BtNode *b = t->root->left;
    int rc = bt_detach_subtree(t, b);
    assert(rc == 0);
    assert(t->root->left == NULL);
    assert(b->parent == NULL);
    assert(t->node_count == 3);  /* A, C, F */

    /* Detaching already-detached node fails */
    rc = bt_detach_subtree(t, b);
    assert(rc == -2);

    bt_subtree_destroy(b, counting_free);
    bt_tree_destroy(t);
    printf("  [PASS] detach_subtree\n");
}

/* ========================================================================
 * Category 4 Tests: Traversals
 * ======================================================================== */

static void test_pre_order(void)
{
    BtNode *root = build_test_tree();
    TraversalResult r = { .buf = "", .pos = 0 };
    bt_traverse_pre_order(root, collect_visitor, &r);
    assert(strcmp(r.buf, "A B D G E C F") == 0);
    bt_subtree_destroy(root, free);
    printf("  [PASS] pre-order traversal\n");
}

static void test_in_order(void)
{
    BtNode *root = build_test_tree();
    TraversalResult r = { .buf = "", .pos = 0 };
    bt_traverse_in_order(root, collect_visitor, &r);
    assert(strcmp(r.buf, "G D B E A C F") == 0);
    bt_subtree_destroy(root, free);
    printf("  [PASS] in-order traversal\n");
}

static void test_post_order(void)
{
    BtNode *root = build_test_tree();
    TraversalResult r = { .buf = "", .pos = 0 };
    bt_traverse_post_order(root, collect_visitor, &r);
    assert(strcmp(r.buf, "G D E B F C A") == 0);
    bt_subtree_destroy(root, free);
    printf("  [PASS] post-order traversal\n");
}

static void test_level_order(void)
{
    BtNode *root = build_test_tree();
    TraversalResult r = { .buf = "", .pos = 0 };
    bt_traverse_level_order(root, collect_visitor, &r);
    assert(strcmp(r.buf, "A B C D E F G") == 0);
    bt_subtree_destroy(root, free);
    printf("  [PASS] level-order traversal\n");
}

static void test_reverse_level_order(void)
{
    BtNode *root = build_test_tree();
    TraversalResult r = { .buf = "", .pos = 0 };
    bt_traverse_reverse_level_order(root, collect_visitor, &r);
    assert(strcmp(r.buf, "G D E F B C A") == 0);
    bt_subtree_destroy(root, free);
    printf("  [PASS] reverse level-order traversal\n");
}

static void test_traversal_null(void)
{
    TraversalResult r = { .buf = "", .pos = 0 };
    assert(bt_traverse_pre_order(NULL, collect_visitor, &r) == -1);
    assert(bt_traverse_in_order(NULL, collect_visitor, &r) == -1);
    assert(bt_traverse_post_order(NULL, collect_visitor, &r) == -1);
    assert(bt_traverse_level_order(NULL, collect_visitor, &r) == -1);
    printf("  [PASS] traversal NULL handling\n");
}

/* ========================================================================
 * Category 5 Tests: Search & Query
 * ======================================================================== */

static void test_find_by_value(void)
{
    BtNode *root = build_test_tree();
    BtNode *found = bt_find_by_value(root, "D", str_cmp);
    assert(found != NULL);
    assert(strcmp(found->data, "D") == 0);

    found = bt_find_by_value(root, "Z", str_cmp);
    assert(found == NULL);

    bt_subtree_destroy(root, free);
    printf("  [PASS] find_by_value\n");
}

static void test_find_all_by_value(void)
{
    /* Tree with duplicate values */
    BtNode *a = bt_node_create(strdup("X"));
    BtNode *b = bt_node_create(strdup("X"));
    BtNode *c = bt_node_create(strdup("Y"));
    a->left = b; b->parent = a;
    a->right = c; c->parent = a;

    BtNode *results[10];
    int count = bt_find_all_by_value(a, "X", str_cmp, results, 10);
    assert(count == 2);

    bt_subtree_destroy(a, free);
    printf("  [PASS] find_all_by_value\n");
}

static void test_node_exists(void)
{
    BtNode *root = build_test_tree();
    assert(bt_node_exists(root, "G", str_cmp) == 1);
    assert(bt_node_exists(root, "Z", str_cmp) == 0);
    assert(bt_node_exists(NULL, "A", str_cmp) == -1);
    bt_subtree_destroy(root, free);
    printf("  [PASS] node_exists\n");
}

/* ========================================================================
 * Category 6 Tests: Structural Queries
 * ======================================================================== */

static void test_depth(void)
{
    BtNode *root = build_test_tree();
    assert(bt_depth(root) == 0);
    assert(bt_depth(root->left) == 1);           /* B */
    assert(bt_depth(root->left->left) == 2);     /* D */
    assert(bt_depth(root->left->left->left) == 3); /* G */
    bt_subtree_destroy(root, free);
    printf("  [PASS] depth\n");
}

static void test_height(void)
{
    BtNode *root = build_test_tree();
    assert(bt_height(root) == 3);
    assert(bt_height(root->left) == 2);  /* B */
    assert(bt_height(root->right) == 1); /* C */
    assert(bt_height(root->left->left->left) == 0); /* G (leaf) */
    bt_subtree_destroy(root, free);
    printf("  [PASS] height\n");
}

static void test_size(void)
{
    BtNode *root = build_test_tree();
    assert(bt_size(root) == 7);
    assert(bt_size(root->left) == 4);  /* B subtree: B,D,E,G */
    assert(bt_size(root->right) == 2); /* C subtree: C,F */
    assert(bt_size(NULL) == 0);
    bt_subtree_destroy(root, free);
    printf("  [PASS] size\n");
}

static void test_is_leaf(void)
{
    BtNode *root = build_test_tree();
    assert(bt_is_leaf(root) == 0);
    assert(bt_is_leaf(root->left->right) == 1);     /* E */
    assert(bt_is_leaf(root->left->left->left) == 1); /* G */
    assert(bt_is_leaf(NULL) == 0);
    bt_subtree_destroy(root, free);
    printf("  [PASS] is_leaf\n");
}

static void test_is_root(void)
{
    BtNode *root = build_test_tree();
    assert(bt_is_root(root) == 1);
    assert(bt_is_root(root->left) == 0);
    assert(bt_is_root(NULL) == 0);
    bt_subtree_destroy(root, free);
    printf("  [PASS] is_root\n");
}

static void test_count_leaves(void)
{
    BtNode *root = build_test_tree();
    assert(bt_count_leaves(root) == 3);  /* G, E, F */
    assert(bt_count_leaves(root->left) == 2);  /* G, E */
    bt_subtree_destroy(root, free);
    printf("  [PASS] count_leaves\n");
}

static void test_count_internal(void)
{
    BtNode *root = build_test_tree();
    assert(bt_count_internal(root) == 4);  /* A, B, C, D */
    bt_subtree_destroy(root, free);
    printf("  [PASS] count_internal\n");
}

static void test_is_full(void)
{
    /* Full tree:    A
     *             / \
     *            B   C   */
    BtNode *a = bt_node_create(strdup("A"));
    BtNode *b = bt_node_create(strdup("B"));
    BtNode *c = bt_node_create(strdup("C"));
    a->left = b; b->parent = a;
    a->right = c; c->parent = a;
    assert(bt_is_full(a) == 1);
    bt_subtree_destroy(a, free);

    /* Not full: the test tree has C with only right child */
    BtNode *root = build_test_tree();
    assert(bt_is_full(root) == 0);
    bt_subtree_destroy(root, free);
    printf("  [PASS] is_full\n");
}

static void test_is_perfect(void)
{
    /* Perfect tree:  A
     *              / \
     *             B   C  */
    BtNode *a = bt_node_create(strdup("A"));
    BtNode *b = bt_node_create(strdup("B"));
    BtNode *c = bt_node_create(strdup("C"));
    a->left = b; b->parent = a;
    a->right = c; c->parent = a;
    assert(bt_is_perfect(a) == 1);
    bt_subtree_destroy(a, free);

    /* Not perfect */
    BtNode *root = build_test_tree();
    assert(bt_is_perfect(root) == 0);
    bt_subtree_destroy(root, free);
    printf("  [PASS] is_perfect\n");
}

static void test_is_balanced(void)
{
    /* Balanced tree: A(B, C) */
    BtNode *a = bt_node_create(strdup("A"));
    BtNode *b = bt_node_create(strdup("B"));
    BtNode *c = bt_node_create(strdup("C"));
    a->left = b; b->parent = a;
    a->right = c; c->parent = a;
    assert(bt_is_balanced(a) == 1);
    bt_subtree_destroy(a, free);

    /* Unbalanced: a chain A -> B -> C -> D (left-skewed) */
    BtNode *u1 = bt_node_create(strdup("A"));
    BtNode *u2 = bt_node_create(strdup("B"));
    BtNode *u3 = bt_node_create(strdup("C"));
    BtNode *u4 = bt_node_create(strdup("D"));
    u1->left = u2; u2->parent = u1;
    u2->left = u3; u3->parent = u2;
    u3->left = u4; u4->parent = u3;
    assert(bt_is_balanced(u1) == 0);
    bt_subtree_destroy(u1, free);
    printf("  [PASS] is_balanced\n");
}

/* ========================================================================
 * Category 7 Tests: Relationship Queries
 * ======================================================================== */

static void test_parent_of(void)
{
    BtNode *root = build_test_tree();
    assert(bt_parent_of(root) == NULL);
    assert(bt_parent_of(root->left) == root);
    assert(bt_parent_of(NULL) == NULL);
    bt_subtree_destroy(root, free);
    printf("  [PASS] parent_of\n");
}

static void test_child_accessors(void)
{
    BtNode *root = build_test_tree();
    assert(bt_left_child_of(root) == root->left);
    assert(bt_right_child_of(root) == root->right);
    assert(bt_left_child_of(NULL) == NULL);
    assert(bt_right_child_of(NULL) == NULL);
    bt_subtree_destroy(root, free);
    printf("  [PASS] left_child_of / right_child_of\n");
}

static void test_sibling_of(void)
{
    BtNode *root = build_test_tree();
    assert(bt_sibling_of(root->left) == root->right);   /* B's sibling = C */
    assert(bt_sibling_of(root->right) == root->left);   /* C's sibling = B */
    assert(bt_sibling_of(root) == NULL);                 /* root has no sibling */
    assert(bt_sibling_of(root->right->right) == NULL);   /* F: C has no left child */
    bt_subtree_destroy(root, free);
    printf("  [PASS] sibling_of\n");
}

static void test_ancestors_of(void)
{
    BtNode *root = build_test_tree();
    BtNode *g = root->left->left->left;  /* G */

    BtNode *anc[10];
    int count = bt_ancestors_of(g, anc, 10);
    assert(count == 3);  /* D, B, A */
    assert(anc[0] == root->left->left);  /* D */
    assert(anc[1] == root->left);        /* B */
    assert(anc[2] == root);              /* A */

    bt_subtree_destroy(root, free);
    printf("  [PASS] ancestors_of\n");
}

static void test_is_ancestor_descendant(void)
{
    BtNode *root = build_test_tree();
    assert(bt_is_ancestor(root, root->left->left->left) == 1);  /* A ancestor of G */
    assert(bt_is_ancestor(root->left, root->right) == 0);        /* B not ancestor of C */
    assert(bt_is_descendant(root->left->left->left, root) == 1); /* G descendant of A */
    bt_subtree_destroy(root, free);
    printf("  [PASS] is_ancestor / is_descendant\n");
}

static void test_lca(void)
{
    BtNode *root = build_test_tree();
    BtNode *g = root->left->left->left;  /* G */
    BtNode *e = root->left->right;       /* E */
    BtNode *f = root->right->right;      /* F */

    assert(bt_lca(g, e) == root->left);  /* LCA(G,E) = B */
    assert(bt_lca(g, f) == root);        /* LCA(G,F) = A */
    assert(bt_lca(g, g) == g);           /* LCA(G,G) = G */

    bt_subtree_destroy(root, free);
    printf("  [PASS] lca\n");
}

static void test_path_between(void)
{
    BtNode *root = build_test_tree();
    BtNode *g = root->left->left->left;  /* G */
    BtNode *e = root->left->right;       /* E */

    BtNode *path[10];
    int len = bt_path_between(g, e, path, 10);
    /* G -> D -> B -> E = length 4 */
    assert(len == 4);
    assert(path[0] == g);
    assert(strcmp(path[1]->data, "D") == 0);
    assert(strcmp(path[2]->data, "B") == 0);
    assert(path[3] == e);

    bt_subtree_destroy(root, free);
    printf("  [PASS] path_between\n");
}

/* ========================================================================
 * Category 8 Tests: Mutation & Restructuring
 * ======================================================================== */

static void test_swap_children(void)
{
    BtNode *root = build_test_tree();
    BtNode *orig_left  = root->left;
    BtNode *orig_right = root->right;

    int rc = bt_swap_children(root);
    assert(rc == 0);
    assert(root->left == orig_right);
    assert(root->right == orig_left);

    bt_subtree_destroy(root, free);
    printf("  [PASS] swap_children\n");
}

static void test_swap_subtrees(void)
{
    BtTree *t = bt_tree_create(counting_free, NULL, NULL);
    t->root = build_test_tree();
    t->node_count = 7;

    BtNode *d = t->root->left->left;    /* D */
    BtNode *c = t->root->right;         /* C */

    int rc = bt_swap_subtrees(t, d, c);
    assert(rc == 0);
    /* Now B's left should be C, A's right should be D */
    assert(t->root->left->left == c);
    assert(t->root->right == d);

    bt_tree_destroy(t);
    printf("  [PASS] swap_subtrees\n");
}

static void test_swap_subtrees_ancestor(void)
{
    BtTree *t = bt_tree_create(counting_free, NULL, NULL);
    t->root = build_test_tree();
    t->node_count = 7;

    /* A is ancestor of D — should fail */
    int rc = bt_swap_subtrees(t, t->root, t->root->left->left);
    assert(rc == -2);

    bt_tree_destroy(t);
    printf("  [PASS] swap_subtrees refuses ancestor swap\n");
}

static void test_mirror(void)
{
    BtNode *root = build_test_tree();
    bt_mirror(root);

    /* After mirror: A's left=C, right=B; B's left=E, right=D; etc. */
    TraversalResult r = { .buf = "", .pos = 0 };
    bt_traverse_pre_order(root, collect_visitor, &r);
    assert(strcmp(r.buf, "A C F B E D G") == 0);

    bt_subtree_destroy(root, free);
    printf("  [PASS] mirror\n");
}

static void test_flatten(void)
{
    BtTree *t = bt_tree_create(free, NULL, NULL);
    /* Build a small tree: A(B(D, E), C) */
    BtNode *a = bt_node_create(strdup("A"));
    BtNode *b = bt_node_create(strdup("B"));
    BtNode *c = bt_node_create(strdup("C"));
    BtNode *d = bt_node_create(strdup("D"));
    BtNode *e = bt_node_create(strdup("E"));
    a->left = b; b->parent = a;
    a->right = c; c->parent = a;
    b->left = d; d->parent = b;
    b->right = e; e->parent = b;
    t->root = a;
    t->node_count = 5;

    bt_flatten(t);

    /* Pre-order linked list: A -> B -> D -> E -> C */
    BtNode *cur = t->root;
    assert(strcmp(cur->data, "A") == 0); assert(cur->left == NULL);
    cur = cur->right;
    assert(strcmp(cur->data, "B") == 0); assert(cur->left == NULL);
    cur = cur->right;
    assert(strcmp(cur->data, "D") == 0); assert(cur->left == NULL);
    cur = cur->right;
    assert(strcmp(cur->data, "E") == 0); assert(cur->left == NULL);
    cur = cur->right;
    assert(strcmp(cur->data, "C") == 0); assert(cur->left == NULL);
    assert(cur->right == NULL);

    bt_tree_destroy(t);
    printf("  [PASS] flatten\n");
}

static void test_merge_trees(void)
{
    BtTree *t1 = bt_tree_create(free, NULL, NULL);
    t1->root = bt_node_create(strdup("L"));
    t1->node_count = 1;

    BtTree *t2 = bt_tree_create(free, NULL, NULL);
    t2->root = bt_node_create(strdup("R"));
    t2->node_count = 1;

    int rc = bt_merge_trees(t1, t2, strdup("ROOT"));
    assert(rc == 0);
    assert(strcmp(t1->root->data, "ROOT") == 0);
    assert(strcmp(t1->root->left->data, "L") == 0);
    assert(strcmp(t1->root->right->data, "R") == 0);
    assert(t1->node_count == 3);

    bt_tree_destroy(t1);
    /* t2 was freed by merge */
    printf("  [PASS] merge_trees\n");
}

/* ========================================================================
 * Category 9 Tests: Serialization
 * ======================================================================== */

static void test_serialize_parenthesized(void)
{
    /* Simple tree: A(B, C) */
    BtNode *a = bt_node_create(strdup("A"));
    BtNode *b = bt_node_create(strdup("B"));
    BtNode *c = bt_node_create(strdup("C"));
    a->left = b; b->parent = a;
    a->right = c; c->parent = a;

    char buf[256];
    int len = bt_serialize_parenthesized(a, str_print, buf, sizeof(buf));
    assert(len > 0);
    assert(strcmp(buf, "A(B C)") == 0);

    bt_subtree_destroy(a, free);
    printf("  [PASS] serialize_parenthesized\n");
}

static void test_print_indented(void)
{
    BtNode *root = build_test_tree();
    /* Just verify it doesn't crash */
    FILE *devnull = fopen("/dev/null", "w");
    int rc = bt_print_indented(root, str_print, devnull);
    assert(rc == 0);
    fclose(devnull);
    bt_subtree_destroy(root, free);
    printf("  [PASS] print_indented\n");
}

static void test_serialize_dot(void)
{
    BtNode *root = build_test_tree();
    FILE *devnull = fopen("/dev/null", "w");
    int rc = bt_serialize_dot(root, str_print, devnull);
    assert(rc == 0);
    fclose(devnull);
    bt_subtree_destroy(root, free);
    printf("  [PASS] serialize_dot\n");
}

/* ========================================================================
 * Category 10 Tests: Copying & Comparison
 * ======================================================================== */

static void test_deep_clone(void)
{
    BtNode *root = build_test_tree();
    BtNode *clone = bt_deep_clone(root, str_clone);
    assert(clone != NULL);

    /* Same structure and values but different pointers */
    assert(clone != root);
    assert(strcmp(clone->data, root->data) == 0);
    assert(clone->data != root->data);

    /* Verify structure */
    assert(bt_size(clone) == 7);
    assert(clone->left->parent == clone);
    assert(strcmp(clone->left->left->left->data, "G") == 0);

    bt_subtree_destroy(root, free);
    bt_subtree_destroy(clone, free);
    printf("  [PASS] deep_clone\n");
}

static void test_structural_equal(void)
{
    BtNode *root1 = build_test_tree();
    BtNode *root2 = build_test_tree();
    assert(bt_structural_equal(root1, root2) == 1);

    /* Make them different */
    BtNode *extra = bt_node_create(strdup("X"));
    root2->right->left = extra;
    extra->parent = root2->right;
    assert(bt_structural_equal(root1, root2) == 0);

    bt_subtree_destroy(root1, free);
    bt_subtree_destroy(root2, free);
    printf("  [PASS] structural_equal\n");
}

static void test_full_equal(void)
{
    BtNode *root1 = build_test_tree();
    BtNode *root2 = build_test_tree();
    assert(bt_full_equal(root1, root2, str_cmp) == 1);

    /* Change a value */
    free(root2->left->data);
    root2->left->data = strdup("Z");
    assert(bt_full_equal(root1, root2, str_cmp) == 0);

    bt_subtree_destroy(root1, free);
    bt_subtree_destroy(root2, free);
    printf("  [PASS] full_equal\n");
}

static void test_is_mirror_check(void)
{
    /* Build mirror pair */
    BtNode *a1 = bt_node_create(strdup("A"));
    BtNode *b1 = bt_node_create(strdup("B"));
    BtNode *c1 = bt_node_create(strdup("C"));
    a1->left = b1; b1->parent = a1;
    a1->right = c1; c1->parent = a1;

    BtNode *a2 = bt_node_create(strdup("A"));
    BtNode *b2 = bt_node_create(strdup("B"));
    BtNode *c2 = bt_node_create(strdup("C"));
    a2->left = c2; c2->parent = a2;
    a2->right = b2; b2->parent = a2;

    assert(bt_is_mirror(a1, a2, str_cmp) == 1);
    assert(bt_is_mirror(a1, a1, str_cmp) == 0);  /* Not a mirror of itself (B!=C) */

    bt_subtree_destroy(a1, free);
    bt_subtree_destroy(a2, free);
    printf("  [PASS] is_mirror\n");
}

/* ========================================================================
 * Category 11 Tests: Utility & Statistics
 * ======================================================================== */

static void test_diameter(void)
{
    BtNode *root = build_test_tree();
    /* Longest path: G-D-B-A-C-F = 5 edges */
    assert(bt_diameter(root) == 5);
    assert(bt_diameter(NULL) == 0);
    bt_subtree_destroy(root, free);
    printf("  [PASS] diameter\n");
}

static void test_width_at_level(void)
{
    BtNode *root = build_test_tree();
    assert(bt_width_at_level(root, 0) == 1);  /* A */
    assert(bt_width_at_level(root, 1) == 2);  /* B, C */
    assert(bt_width_at_level(root, 2) == 3);  /* D, E, F */
    assert(bt_width_at_level(root, 3) == 1);  /* G */
    bt_subtree_destroy(root, free);
    printf("  [PASS] width_at_level\n");
}

static void test_max_width(void)
{
    BtNode *root = build_test_tree();
    size_t level;
    size_t w = bt_max_width(root, &level);
    assert(w == 3);
    assert(level == 2);
    bt_subtree_destroy(root, free);
    printf("  [PASS] max_width\n");
}

static void test_root_to_leaf_paths(void)
{
    BtNode *root = build_test_tree();
    BtNode **paths[10];
    size_t path_lens[10];

    int count = bt_root_to_leaf_paths(root, paths, path_lens, 10);
    assert(count == 3);  /* G, E, F paths */

    /* Path to G: A-B-D-G (len 4) */
    assert(path_lens[0] == 4);
    assert(strcmp(((char *)paths[0][0]->data), "A") == 0);
    assert(strcmp(((char *)paths[0][3]->data), "G") == 0);

    for (int i = 0; i < count; i++)
        free(paths[i]);

    bt_subtree_destroy(root, free);
    printf("  [PASS] root_to_leaf_paths\n");
}

/* --- Map/Fold helpers (must be at file scope for ISO C) --- */

static void *int_doubler(void *data, void *ud)
{
    (void)ud;
    return (void *)((long)data * 2);
}

static void *int_summer(void *acc, void *data, void *ud)
{
    (void)ud;
    return (void *)((long)acc + (long)data);
}

static void test_apply(void)
{
    BtNode *a = bt_node_create((void *)(long)1);
    BtNode *b = bt_node_create((void *)(long)2);
    BtNode *c = bt_node_create((void *)(long)3);
    a->left = b; b->parent = a;
    a->right = c; c->parent = a;

    bt_apply(a, int_doubler, NULL);
    assert((long)a->data == 2);
    assert((long)b->data == 4);
    assert((long)c->data == 6);

    free(a); free(b); free(c);
    printf("  [PASS] apply\n");
}

static void test_fold(void)
{
    BtNode *a = bt_node_create((void *)(long)1);
    BtNode *b = bt_node_create((void *)(long)2);
    BtNode *c = bt_node_create((void *)(long)3);
    a->left = b; b->parent = a;
    a->right = c; c->parent = a;

    void *result = bt_fold(a, int_summer, (void *)(long)0, NULL);
    assert((long)result == 6);

    free(a); free(b); free(c);
    printf("  [PASS] fold\n");
}

/* ---- Main --------------------------------------------------------------- */

int main(void)
{
    printf("Binary Tree — Full Test Suite (All 11 Categories)\n");
    printf("==================================================\n\n");

    printf("Category 1: Construction & Destruction\n");
    printf("---------------------------------------\n");
    test_node_create_non_null_data();
    test_node_create_null_data();
    test_node_destroy_leaf();
    test_node_destroy_null();
    test_node_destroy_has_children();
    test_node_destroy_null_free_func();
    test_subtree_destroy_single();
    test_subtree_destroy_tree();
    test_subtree_destroy_null();
    test_tree_create();
    test_tree_create_null_funcs();
    test_tree_destroy_empty();
    test_tree_destroy_populated();
    test_tree_destroy_null();

    printf("\nCategory 2: Insertion\n");
    printf("---------------------\n");
    test_set_left_child();
    test_set_right_child();
    test_attach_left_subtree();
    test_attach_right_subtree();
    test_insertion_null_inputs();

    printf("\nCategory 3: Deletion & Detachment\n");
    printf("----------------------------------\n");
    test_remove_node_promote_child();
    test_remove_node_promote_child_two_children();
    test_remove_subtree();
    test_detach_subtree();

    printf("\nCategory 4: Traversals\n");
    printf("----------------------\n");
    test_pre_order();
    test_in_order();
    test_post_order();
    test_level_order();
    test_reverse_level_order();
    test_traversal_null();

    printf("\nCategory 5: Search & Query\n");
    printf("--------------------------\n");
    test_find_by_value();
    test_find_all_by_value();
    test_node_exists();

    printf("\nCategory 6: Structural Queries\n");
    printf("-------------------------------\n");
    test_depth();
    test_height();
    test_size();
    test_is_leaf();
    test_is_root();
    test_count_leaves();
    test_count_internal();
    test_is_full();
    test_is_perfect();
    test_is_balanced();

    printf("\nCategory 7: Relationship Queries\n");
    printf("---------------------------------\n");
    test_parent_of();
    test_child_accessors();
    test_sibling_of();
    test_ancestors_of();
    test_is_ancestor_descendant();
    test_lca();
    test_path_between();

    printf("\nCategory 8: Mutation & Restructuring\n");
    printf("-------------------------------------\n");
    test_swap_children();
    test_swap_subtrees();
    test_swap_subtrees_ancestor();
    test_mirror();
    test_flatten();
    test_merge_trees();

    printf("\nCategory 9: Serialization\n");
    printf("-------------------------\n");
    test_serialize_parenthesized();
    test_print_indented();
    test_serialize_dot();

    printf("\nCategory 10: Copying & Comparison\n");
    printf("----------------------------------\n");
    test_deep_clone();
    test_structural_equal();
    test_full_equal();
    test_is_mirror_check();

    printf("\nCategory 11: Utility & Statistics\n");
    printf("----------------------------------\n");
    test_diameter();
    test_width_at_level();
    test_max_width();
    test_root_to_leaf_paths();
    test_apply();
    test_fold();

    printf("\n==================================================\n");
    printf("All tests passed.\n");
    return 0;
}
