#define _POSIX_C_SOURCE 200809L
#include "nary_tree.h"
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

/* ---- Helper: build a 6-node tree manually (LCRS wiring) ----------------
 *
 *         A
 *       / | \
 *      B  C  D
 *     / \
 *    E   F
 */
static NaryNode *build_test_tree(void)
{
    NaryNode *a = nary_node_create(strdup("A"));
    NaryNode *b = nary_node_create(strdup("B"));
    NaryNode *c = nary_node_create(strdup("C"));
    NaryNode *d = nary_node_create(strdup("D"));
    NaryNode *e = nary_node_create(strdup("E"));
    NaryNode *f = nary_node_create(strdup("F"));

    /* A's children: B -> C -> D */
    a->first_child  = b;
    b->parent       = a;
    b->next_sibling = c;
    c->parent       = a;
    c->next_sibling = d;
    d->parent       = a;

    /* B's children: E -> F */
    b->first_child  = e;
    e->parent       = b;
    e->next_sibling = f;
    f->parent       = b;

    return a;
}

/* ---- Tests -------------------------------------------------------------- */

static void test_node_create_non_null_data(void)
{
    char *s   = strdup("hello");
    NaryNode *n = nary_node_create(s);
    assert(n != NULL);
    assert(n->data == s);
    assert(n->parent == NULL);
    assert(n->first_child == NULL);
    assert(n->next_sibling == NULL);
    free(s);
    free(n);
    printf("  [PASS] node_create with non-NULL data\n");
}

static void test_node_create_null_data(void)
{
    NaryNode *n = nary_node_create(NULL);
    assert(n != NULL);
    assert(n->data == NULL);
    free(n);
    printf("  [PASS] node_create with NULL data\n");
}

static void test_node_destroy_leaf(void)
{
    g_free_count = 0;
    NaryNode *n = nary_node_create(strdup("leaf"));
    int rc = nary_node_destroy(n, counting_free);
    assert(rc == 0);
    assert(g_free_count == 1);
    printf("  [PASS] node_destroy on standalone leaf\n");
}

static void test_node_destroy_null(void)
{
    int rc = nary_node_destroy(NULL, counting_free);
    assert(rc == -1);
    printf("  [PASS] node_destroy with NULL node\n");
}

static void test_node_destroy_has_children(void)
{
    NaryNode *parent = nary_node_create(strdup("parent"));
    NaryNode *child  = nary_node_create(strdup("child"));
    parent->first_child = child;
    child->parent       = parent;

    int rc = nary_node_destroy(parent, counting_free);
    assert(rc == -2);
    /* parent must still be accessible */
    assert(parent->data != NULL);

    /* Clean up manually */
    free(parent->data);
    free(parent);
    free(child->data);
    free(child);
    printf("  [PASS] node_destroy refuses when node has children\n");
}

static void test_node_destroy_null_free_func(void)
{
    /* Data is a plain integer cast — no free needed */
    NaryNode *n = nary_node_create((void *)(long)42);
    int rc = nary_node_destroy(n, NULL);
    assert(rc == 0);
    printf("  [PASS] node_destroy with NULL free_func\n");
}

static void test_subtree_destroy_single(void)
{
    g_free_count = 0;
    NaryNode *n = nary_node_create(strdup("solo"));
    int rc = nary_subtree_destroy(n, counting_free);
    assert(rc == 0);
    assert(g_free_count == 1);
    printf("  [PASS] subtree_destroy on single node\n");
}

static void test_subtree_destroy_tree(void)
{
    g_free_count = 0;
    NaryNode *root = build_test_tree();
    int rc = nary_subtree_destroy(root, counting_free);
    assert(rc == 0);
    assert(g_free_count == 6);
    printf("  [PASS] subtree_destroy on 6-node tree\n");
}

static void test_subtree_destroy_null(void)
{
    int rc = nary_subtree_destroy(NULL, counting_free);
    assert(rc == -1);
    printf("  [PASS] subtree_destroy with NULL node\n");
}

static void test_tree_create_all_funcs(void)
{
    NaryTree *t = nary_tree_create(counting_free, NULL, NULL);
    assert(t != NULL);
    assert(t->root == NULL);
    assert(t->node_count == 0);
    assert(t->free_func == counting_free);
    free(t);
    printf("  [PASS] tree_create with function pointers\n");
}

static void test_tree_create_null_free(void)
{
    NaryTree *t = nary_tree_create(NULL, NULL, NULL);
    assert(t != NULL);
    assert(t->free_func == NULL);
    free(t);
    printf("  [PASS] tree_create with NULL free_func\n");
}

static void test_tree_destroy_empty(void)
{
    NaryTree *t = nary_tree_create(counting_free, NULL, NULL);
    int rc = nary_tree_destroy(t);
    assert(rc == 0);
    printf("  [PASS] tree_destroy on empty tree\n");
}

static void test_tree_destroy_populated(void)
{
    g_free_count = 0;
    NaryTree *t = nary_tree_create(counting_free, NULL, NULL);
    t->root       = build_test_tree();
    t->node_count = 6;

    int rc = nary_tree_destroy(t);
    assert(rc == 0);
    assert(g_free_count == 6);
    printf("  [PASS] tree_destroy on populated tree\n");
}

static void test_tree_destroy_null(void)
{
    int rc = nary_tree_destroy(NULL);
    assert(rc == -1);
    printf("  [PASS] tree_destroy with NULL tree\n");
}

/* ---- Main --------------------------------------------------------------- */

int main(void)
{
    printf("N-ary Tree — Category 1: Construction & Destruction\n");
    printf("===================================================\n");

    test_node_create_non_null_data();
    test_node_create_null_data();
    test_node_destroy_leaf();
    test_node_destroy_null();
    test_node_destroy_has_children();
    test_node_destroy_null_free_func();
    test_subtree_destroy_single();
    test_subtree_destroy_tree();
    test_subtree_destroy_null();
    test_tree_create_all_funcs();
    test_tree_create_null_free();
    test_tree_destroy_empty();
    test_tree_destroy_populated();
    test_tree_destroy_null();

    printf("\nAll 14 tests passed.\n");
    return 0;
}
