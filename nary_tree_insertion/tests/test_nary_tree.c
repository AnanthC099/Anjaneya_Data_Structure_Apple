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

/* ---- Helper: count children of a node ---------------------------------- */
static size_t count_children(NaryNode *node)
{
    size_t count = 0;
    for (NaryNode *c = node->first_child; c; c = c->next_sibling)
        count++;
    return count;
}

/* ---- Helper: get child at index ---------------------------------------- */
static NaryNode *child_at(NaryNode *node, size_t index)
{
    NaryNode *c = node->first_child;
    for (size_t i = 0; i < index && c; i++)
        c = c->next_sibling;
    return c;
}

/* ===========================================================================
 * Category 1: Construction & Destruction Tests
 * =========================================================================== */

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

/* ===========================================================================
 * Category 2: Insertion Tests
 * =========================================================================== */

/* --- nary_add_child_append ----------------------------------------------- */

static void test_append_to_empty_parent(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *root = nary_node_create(strdup("root"));
    tree->root = root;
    tree->node_count = 1;

    int rc = nary_add_child_append(tree, root, strdup("A"));
    assert(rc == 0);
    assert(tree->node_count == 2);
    assert(root->first_child != NULL);
    assert(strcmp((char *)root->first_child->data, "A") == 0);
    assert(root->first_child->parent == root);
    assert(root->first_child->next_sibling == NULL);

    nary_tree_destroy(tree);
    printf("  [PASS] append child to empty parent\n");
}

static void test_append_multiple_children(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *root = nary_node_create(strdup("root"));
    tree->root = root;
    tree->node_count = 1;

    nary_add_child_append(tree, root, strdup("A"));
    nary_add_child_append(tree, root, strdup("B"));
    nary_add_child_append(tree, root, strdup("C"));

    assert(tree->node_count == 4);
    assert(count_children(root) == 3);

    /* Verify order: A -> B -> C */
    NaryNode *c0 = child_at(root, 0);
    NaryNode *c1 = child_at(root, 1);
    NaryNode *c2 = child_at(root, 2);

    assert(strcmp((char *)c0->data, "A") == 0);
    assert(strcmp((char *)c1->data, "B") == 0);
    assert(strcmp((char *)c2->data, "C") == 0);

    /* All children have correct parent */
    assert(c0->parent == root);
    assert(c1->parent == root);
    assert(c2->parent == root);

    /* Sibling chain is correct */
    assert(c0->next_sibling == c1);
    assert(c1->next_sibling == c2);
    assert(c2->next_sibling == NULL);

    nary_tree_destroy(tree);
    printf("  [PASS] append multiple children preserves order\n");
}

static void test_append_null_parent(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    char *leaked = strdup("X");
    int rc = nary_add_child_append(tree, NULL, leaked);
    assert(rc == -1);
    free(leaked);
    nary_tree_destroy(tree);
    printf("  [PASS] append with NULL parent returns -1\n");
}

static void test_append_null_tree(void)
{
    /* tree pointer is NULL — should still work on the node, just skip count */
    NaryNode *root = nary_node_create(strdup("root"));
    int rc = nary_add_child_append(NULL, root, strdup("A"));
    assert(rc == 0);
    assert(root->first_child != NULL);
    assert(strcmp((char *)root->first_child->data, "A") == 0);

    /* Clean up manually */
    free(root->first_child->data);
    free(root->first_child);
    free(root->data);
    free(root);
    printf("  [PASS] append with NULL tree (standalone node operation)\n");
}

/* --- nary_add_child_prepend ---------------------------------------------- */

static void test_prepend_to_empty_parent(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *root = nary_node_create(strdup("root"));
    tree->root = root;
    tree->node_count = 1;

    int rc = nary_add_child_prepend(tree, root, strdup("A"));
    assert(rc == 0);
    assert(tree->node_count == 2);
    assert(root->first_child != NULL);
    assert(strcmp((char *)root->first_child->data, "A") == 0);
    assert(root->first_child->parent == root);

    nary_tree_destroy(tree);
    printf("  [PASS] prepend child to empty parent\n");
}

static void test_prepend_multiple_children(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *root = nary_node_create(strdup("root"));
    tree->root = root;
    tree->node_count = 1;

    nary_add_child_prepend(tree, root, strdup("C"));
    nary_add_child_prepend(tree, root, strdup("B"));
    nary_add_child_prepend(tree, root, strdup("A"));

    assert(tree->node_count == 4);
    assert(count_children(root) == 3);

    /* Verify order: A -> B -> C (each prepend pushes to front) */
    NaryNode *c0 = child_at(root, 0);
    NaryNode *c1 = child_at(root, 1);
    NaryNode *c2 = child_at(root, 2);

    assert(strcmp((char *)c0->data, "A") == 0);
    assert(strcmp((char *)c1->data, "B") == 0);
    assert(strcmp((char *)c2->data, "C") == 0);

    /* Parent pointers correct */
    assert(c0->parent == root);
    assert(c1->parent == root);
    assert(c2->parent == root);

    nary_tree_destroy(tree);
    printf("  [PASS] prepend multiple children preserves reverse-insertion order\n");
}

static void test_prepend_null_parent(void)
{
    char *leaked = strdup("X");
    int rc = nary_add_child_prepend(NULL, NULL, leaked);
    assert(rc == -1);
    free(leaked);
    printf("  [PASS] prepend with NULL parent returns -1\n");
}

/* --- nary_insert_child_at ------------------------------------------------ */

static void test_insert_at_zero_empty_parent(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *root = nary_node_create(strdup("root"));
    tree->root = root;
    tree->node_count = 1;

    int rc = nary_insert_child_at(tree, root, 0, strdup("A"));
    assert(rc == 0);
    assert(tree->node_count == 2);
    assert(strcmp((char *)root->first_child->data, "A") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] insert_at index 0 on empty parent (prepend)\n");
}

static void test_insert_at_beginning(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *root = nary_node_create(strdup("root"));
    tree->root = root;
    tree->node_count = 1;

    nary_add_child_append(tree, root, strdup("B"));
    nary_add_child_append(tree, root, strdup("C"));

    /* Insert "A" at index 0 -> A, B, C */
    int rc = nary_insert_child_at(tree, root, 0, strdup("A"));
    assert(rc == 0);
    assert(tree->node_count == 4);
    assert(count_children(root) == 3);

    assert(strcmp((char *)child_at(root, 0)->data, "A") == 0);
    assert(strcmp((char *)child_at(root, 1)->data, "B") == 0);
    assert(strcmp((char *)child_at(root, 2)->data, "C") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] insert_at index 0 (prepend among existing children)\n");
}

static void test_insert_at_middle(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *root = nary_node_create(strdup("root"));
    tree->root = root;
    tree->node_count = 1;

    nary_add_child_append(tree, root, strdup("A"));
    nary_add_child_append(tree, root, strdup("C"));

    /* Insert "B" at index 1 -> A, B, C */
    int rc = nary_insert_child_at(tree, root, 1, strdup("B"));
    assert(rc == 0);
    assert(tree->node_count == 4);
    assert(count_children(root) == 3);

    assert(strcmp((char *)child_at(root, 0)->data, "A") == 0);
    assert(strcmp((char *)child_at(root, 1)->data, "B") == 0);
    assert(strcmp((char *)child_at(root, 2)->data, "C") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] insert_at middle index\n");
}

static void test_insert_at_end(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *root = nary_node_create(strdup("root"));
    tree->root = root;
    tree->node_count = 1;

    nary_add_child_append(tree, root, strdup("A"));
    nary_add_child_append(tree, root, strdup("B"));

    /* Insert "C" at index 2 (== child_count) -> append */
    int rc = nary_insert_child_at(tree, root, 2, strdup("C"));
    assert(rc == 0);
    assert(tree->node_count == 4);
    assert(count_children(root) == 3);

    assert(strcmp((char *)child_at(root, 0)->data, "A") == 0);
    assert(strcmp((char *)child_at(root, 1)->data, "B") == 0);
    assert(strcmp((char *)child_at(root, 2)->data, "C") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] insert_at end index (append)\n");
}

static void test_insert_at_beyond_end(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *root = nary_node_create(strdup("root"));
    tree->root = root;
    tree->node_count = 1;

    nary_add_child_append(tree, root, strdup("A"));

    /* Insert at index 100 (way beyond count) -> should append */
    int rc = nary_insert_child_at(tree, root, 100, strdup("B"));
    assert(rc == 0);
    assert(tree->node_count == 3);
    assert(count_children(root) == 2);

    assert(strcmp((char *)child_at(root, 0)->data, "A") == 0);
    assert(strcmp((char *)child_at(root, 1)->data, "B") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] insert_at beyond end clamps to append\n");
}

static void test_insert_at_null_parent(void)
{
    char *leaked = strdup("X");
    int rc = nary_insert_child_at(NULL, NULL, 0, leaked);
    assert(rc == -1);
    free(leaked);
    printf("  [PASS] insert_at with NULL parent returns -1\n");
}

static void test_insert_at_index_on_no_children(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *root = nary_node_create(strdup("root"));
    tree->root = root;
    tree->node_count = 1;

    /* Insert at index 5 when parent has no children -> should still work */
    int rc = nary_insert_child_at(tree, root, 5, strdup("A"));
    assert(rc == 0);
    assert(tree->node_count == 2);
    assert(root->first_child != NULL);
    assert(strcmp((char *)root->first_child->data, "A") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] insert_at with index > 0 on childless parent\n");
}

/* --- nary_attach_subtree ------------------------------------------------- */

static void test_attach_subtree_single_node(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *root = nary_node_create(strdup("root"));
    tree->root = root;
    tree->node_count = 1;

    NaryNode *detached = nary_node_create(strdup("X"));
    int rc = nary_attach_subtree(tree, root, detached);
    assert(rc == 0);
    assert(tree->node_count == 2);
    assert(root->first_child == detached);
    assert(detached->parent == root);

    nary_tree_destroy(tree);
    printf("  [PASS] attach single detached node\n");
}

static void test_attach_subtree_multi_node(void)
{
    g_free_count = 0;
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *root = nary_node_create(strdup("root"));
    tree->root = root;
    tree->node_count = 1;

    /* Build a 3-node detached subtree:
     *    X
     *   / \
     *  Y   Z
     */
    NaryNode *x = nary_node_create(strdup("X"));
    NaryNode *y = nary_node_create(strdup("Y"));
    NaryNode *z = nary_node_create(strdup("Z"));
    x->first_child  = y;
    y->parent        = x;
    y->next_sibling  = z;
    z->parent        = x;

    int rc = nary_attach_subtree(tree, root, x);
    assert(rc == 0);
    assert(tree->node_count == 4);  /* 1 (root) + 3 (subtree) */
    assert(root->first_child == x);
    assert(x->parent == root);

    nary_tree_destroy(tree);
    printf("  [PASS] attach multi-node subtree updates count correctly\n");
}

static void test_attach_subtree_as_last_sibling(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *root = nary_node_create(strdup("root"));
    tree->root = root;
    tree->node_count = 1;

    nary_add_child_append(tree, root, strdup("A"));
    nary_add_child_append(tree, root, strdup("B"));

    NaryNode *detached = nary_node_create(strdup("C"));
    int rc = nary_attach_subtree(tree, root, detached);
    assert(rc == 0);
    assert(tree->node_count == 4);
    assert(count_children(root) == 3);

    /* C should be the last child */
    assert(strcmp((char *)child_at(root, 2)->data, "C") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] attach subtree appends as last child\n");
}

static void test_attach_subtree_null_parent(void)
{
    NaryNode *detached = nary_node_create(strdup("X"));
    int rc = nary_attach_subtree(NULL, NULL, detached);
    assert(rc == -1);
    free(detached->data);
    free(detached);
    printf("  [PASS] attach_subtree with NULL parent returns -1\n");
}

static void test_attach_subtree_null_subtree(void)
{
    NaryNode *root = nary_node_create(strdup("root"));
    int rc = nary_attach_subtree(NULL, root, NULL);
    assert(rc == -1);
    free(root->data);
    free(root);
    printf("  [PASS] attach_subtree with NULL subtree returns -1\n");
}

static void test_attach_subtree_already_parented(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *root = nary_node_create(strdup("root"));
    tree->root = root;
    tree->node_count = 1;

    nary_add_child_append(tree, root, strdup("A"));
    NaryNode *child_a = root->first_child;

    /* Try to attach child_a which already has a parent */
    NaryNode *other = nary_node_create(strdup("other"));
    int rc = nary_attach_subtree(tree, other, child_a);
    assert(rc == -2);

    free(other->data);
    free(other);
    nary_tree_destroy(tree);
    printf("  [PASS] attach_subtree refuses node that already has a parent\n");
}

/* --- Mixed insertion operations ------------------------------------------ */

static void test_mixed_operations_build_tree(void)
{
    /*
     * Build this tree using various insertion operations:
     *
     *         R
     *       / | \
     *      A  B  C
     *     / \    |
     *    D   E   F
     */
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *root = nary_node_create(strdup("R"));
    tree->root = root;
    tree->node_count = 1;

    /* Append A, then C */
    nary_add_child_append(tree, root, strdup("A"));
    nary_add_child_append(tree, root, strdup("C"));

    /* Insert B at index 1 (between A and C) */
    nary_insert_child_at(tree, root, 1, strdup("B"));

    assert(count_children(root) == 3);
    NaryNode *a = child_at(root, 0);
    NaryNode *b = child_at(root, 1);
    NaryNode *c = child_at(root, 2);
    assert(strcmp((char *)a->data, "A") == 0);
    assert(strcmp((char *)b->data, "B") == 0);
    assert(strcmp((char *)c->data, "C") == 0);

    /* Add children to A: prepend E, then prepend D -> D, E */
    nary_add_child_prepend(tree, a, strdup("E"));
    nary_add_child_prepend(tree, a, strdup("D"));

    assert(count_children(a) == 2);
    assert(strcmp((char *)child_at(a, 0)->data, "D") == 0);
    assert(strcmp((char *)child_at(a, 1)->data, "E") == 0);

    /* Attach a detached subtree F under C */
    NaryNode *f = nary_node_create(strdup("F"));
    nary_attach_subtree(tree, c, f);

    assert(count_children(c) == 1);
    assert(strcmp((char *)child_at(c, 0)->data, "F") == 0);

    /* Total: R + A + B + C + D + E + F = 7 */
    assert(tree->node_count == 7);

    /* Verify parent pointers up the chain */
    assert(f->parent == c);
    assert(c->parent == root);
    assert(child_at(a, 0)->parent == a);
    assert(a->parent == root);

    nary_tree_destroy(tree);
    printf("  [PASS] mixed operations build correct tree structure\n");
}

static void test_append_deep_chain(void)
{
    /* Build a deep chain: R -> A -> B -> C -> D (each node has one child) */
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *root = nary_node_create(strdup("R"));
    tree->root = root;
    tree->node_count = 1;

    NaryNode *current = root;
    const char *names[] = {"A", "B", "C", "D"};
    for (int i = 0; i < 4; i++) {
        nary_add_child_append(tree, current, strdup(names[i]));
        current = current->first_child;
        /* Walk to the last child we just appended */
        while (current->next_sibling)
            current = current->next_sibling;
    }

    assert(tree->node_count == 5);

    /* Verify the chain */
    NaryNode *n = root;
    for (int i = 0; i < 4; i++) {
        assert(n->first_child != NULL);
        n = n->first_child;
        assert(strcmp((char *)n->data, names[i]) == 0);
    }
    assert(n->first_child == NULL);  /* D is a leaf */

    nary_tree_destroy(tree);
    printf("  [PASS] append builds deep chain correctly\n");
}

/* ---- Main --------------------------------------------------------------- */

int main(void)
{
    printf("N-ary Tree — Category 1: Construction & Destruction\n");
    printf("====================================================\n");

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

    printf("\nN-ary Tree — Category 2: Insertion\n");
    printf("====================================\n");

    /* nary_add_child_append */
    test_append_to_empty_parent();
    test_append_multiple_children();
    test_append_null_parent();
    test_append_null_tree();

    /* nary_add_child_prepend */
    test_prepend_to_empty_parent();
    test_prepend_multiple_children();
    test_prepend_null_parent();

    /* nary_insert_child_at */
    test_insert_at_zero_empty_parent();
    test_insert_at_beginning();
    test_insert_at_middle();
    test_insert_at_end();
    test_insert_at_beyond_end();
    test_insert_at_null_parent();
    test_insert_at_index_on_no_children();

    /* nary_attach_subtree */
    test_attach_subtree_single_node();
    test_attach_subtree_multi_node();
    test_attach_subtree_as_last_sibling();
    test_attach_subtree_null_parent();
    test_attach_subtree_null_subtree();
    test_attach_subtree_already_parented();

    /* Mixed operations */
    test_mixed_operations_build_tree();
    test_append_deep_chain();

    printf("\nAll 36 tests passed.\n");
    return 0;
}
