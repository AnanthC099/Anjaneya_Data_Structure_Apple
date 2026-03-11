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

/* ---- Helper: build a 6-node tree via the insertion API ------------------
 *
 *         A
 *       / | \
 *      B  C  D
 *     / \
 *    E   F
 */
static NaryTree *build_test_tree(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *a = nary_node_create(strdup("A"));
    tree->root = a;
    tree->node_count = 1;

    nary_add_child_append(tree, a, strdup("B"));
    nary_add_child_append(tree, a, strdup("C"));
    nary_add_child_append(tree, a, strdup("D"));

    NaryNode *b = a->first_child;
    nary_add_child_append(tree, b, strdup("E"));
    nary_add_child_append(tree, b, strdup("F"));

    return tree;
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
    assert(parent->data != NULL);

    free(parent->data);
    free(parent);
    free(child->data);
    free(child);
    printf("  [PASS] node_destroy refuses when node has children\n");
}

static void test_node_destroy_null_free_func(void)
{
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
    NaryTree *tree = build_test_tree();
    NaryNode *root = tree->root;
    /* Detach root so we can destroy it manually */
    tree->root = NULL;
    int rc = nary_subtree_destroy(root, counting_free);
    assert(rc == 0);
    assert(g_free_count == 6);
    free(tree);
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
    NaryTree *t = build_test_tree();

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

    NaryNode *c0 = child_at(root, 0);
    NaryNode *c1 = child_at(root, 1);
    NaryNode *c2 = child_at(root, 2);

    assert(strcmp((char *)c0->data, "A") == 0);
    assert(strcmp((char *)c1->data, "B") == 0);
    assert(strcmp((char *)c2->data, "C") == 0);

    assert(c0->parent == root);
    assert(c1->parent == root);
    assert(c2->parent == root);

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
    NaryNode *root = nary_node_create(strdup("root"));
    int rc = nary_add_child_append(NULL, root, strdup("A"));
    assert(rc == 0);
    assert(root->first_child != NULL);
    assert(strcmp((char *)root->first_child->data, "A") == 0);

    free(root->first_child->data);
    free(root->first_child);
    free(root->data);
    free(root);
    printf("  [PASS] append with NULL tree (standalone node operation)\n");
}

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

    assert(strcmp((char *)child_at(root, 0)->data, "A") == 0);
    assert(strcmp((char *)child_at(root, 1)->data, "B") == 0);
    assert(strcmp((char *)child_at(root, 2)->data, "C") == 0);

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

    int rc = nary_insert_child_at(tree, root, 5, strdup("A"));
    assert(rc == 0);
    assert(tree->node_count == 2);
    assert(root->first_child != NULL);
    assert(strcmp((char *)root->first_child->data, "A") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] insert_at with index > 0 on childless parent\n");
}

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

    NaryNode *x = nary_node_create(strdup("X"));
    NaryNode *y = nary_node_create(strdup("Y"));
    NaryNode *z = nary_node_create(strdup("Z"));
    x->first_child  = y;
    y->parent        = x;
    y->next_sibling  = z;
    z->parent        = x;

    int rc = nary_attach_subtree(tree, root, x);
    assert(rc == 0);
    assert(tree->node_count == 4);
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

    NaryNode *other = nary_node_create(strdup("other"));
    int rc = nary_attach_subtree(tree, other, child_a);
    assert(rc == -2);

    free(other->data);
    free(other);
    nary_tree_destroy(tree);
    printf("  [PASS] attach_subtree refuses node that already has a parent\n");
}

static void test_mixed_operations_build_tree(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *root = nary_node_create(strdup("R"));
    tree->root = root;
    tree->node_count = 1;

    nary_add_child_append(tree, root, strdup("A"));
    nary_add_child_append(tree, root, strdup("C"));
    nary_insert_child_at(tree, root, 1, strdup("B"));

    assert(count_children(root) == 3);
    NaryNode *a = child_at(root, 0);
    NaryNode *b = child_at(root, 1);
    NaryNode *c = child_at(root, 2);
    assert(strcmp((char *)a->data, "A") == 0);
    assert(strcmp((char *)b->data, "B") == 0);
    assert(strcmp((char *)c->data, "C") == 0);

    nary_add_child_prepend(tree, a, strdup("E"));
    nary_add_child_prepend(tree, a, strdup("D"));

    assert(count_children(a) == 2);
    assert(strcmp((char *)child_at(a, 0)->data, "D") == 0);
    assert(strcmp((char *)child_at(a, 1)->data, "E") == 0);

    NaryNode *f = nary_node_create(strdup("F"));
    nary_attach_subtree(tree, c, f);

    assert(count_children(c) == 1);
    assert(strcmp((char *)child_at(c, 0)->data, "F") == 0);
    assert(tree->node_count == 7);

    assert(f->parent == c);
    assert(c->parent == root);
    assert(child_at(a, 0)->parent == a);
    assert(a->parent == root);

    nary_tree_destroy(tree);
    printf("  [PASS] mixed operations build correct tree structure\n");
}

static void test_append_deep_chain(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *root = nary_node_create(strdup("R"));
    tree->root = root;
    tree->node_count = 1;

    NaryNode *current = root;
    const char *names[] = {"A", "B", "C", "D"};
    for (int i = 0; i < 4; i++) {
        nary_add_child_append(tree, current, strdup(names[i]));
        current = current->first_child;
        while (current->next_sibling)
            current = current->next_sibling;
    }

    assert(tree->node_count == 5);

    NaryNode *n = root;
    for (int i = 0; i < 4; i++) {
        assert(n->first_child != NULL);
        n = n->first_child;
        assert(strcmp((char *)n->data, names[i]) == 0);
    }
    assert(n->first_child == NULL);

    nary_tree_destroy(tree);
    printf("  [PASS] append builds deep chain correctly\n");
}

/* ===========================================================================
 * Category 3: Deletion & Detachment Tests
 * =========================================================================== */

/* --- nary_remove_node_promote_children ----------------------------------- */

static void test_promote_leaf_node(void)
{
    /*  A -> B(leaf)  =>  remove B  =>  A (no children)  */
    g_free_count = 0;
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *a = nary_node_create(strdup("A"));
    tree->root = a;
    tree->node_count = 1;
    nary_add_child_append(tree, a, strdup("B"));

    NaryNode *b = a->first_child;
    int rc = nary_remove_node_promote_children(tree, b);
    assert(rc == 0);
    assert(tree->node_count == 1);
    assert(a->first_child == NULL);
    assert(g_free_count == 1);  /* B's data freed */

    nary_tree_destroy(tree);
    printf("  [PASS] promote_children on leaf node removes it cleanly\n");
}

static void test_promote_first_child_with_children(void)
{
    /*
     *     A               A
     *   / | \    =>     / | | \
     *  B  C  D        E  F  C  D
     * / \
     * E  F
     *
     * Remove B, promote E and F under A, preserving position.
     */
    g_free_count = 0;
    NaryTree *tree = build_test_tree();   /* A(B(E,F), C, D) — 6 nodes */

    NaryNode *a = tree->root;
    NaryNode *b = a->first_child;
    NaryNode *e = b->first_child;
    NaryNode *f = e->next_sibling;
    NaryNode *c = b->next_sibling;

    int rc = nary_remove_node_promote_children(tree, b);
    assert(rc == 0);
    assert(tree->node_count == 5);
    assert(g_free_count == 1);  /* only B's data freed */

    /* A's children should now be: E -> F -> C -> D */
    assert(count_children(a) == 4);
    assert(child_at(a, 0) == e);
    assert(child_at(a, 1) == f);
    assert(child_at(a, 2) == c);
    assert(strcmp((char *)child_at(a, 3)->data, "D") == 0);

    /* Parent pointers updated */
    assert(e->parent == a);
    assert(f->parent == a);

    nary_tree_destroy(tree);
    printf("  [PASS] promote_children on first child with children\n");
}

static void test_promote_middle_child_with_children(void)
{
    /*
     *       A                  A
     *     / | \      =>     / | | \
     *    B  C  D           B  G  H  D
     *       |
     *      G-H
     *
     * Remove C (middle child of A) that has children G, H.
     */
    g_free_count = 0;
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *a = nary_node_create(strdup("A"));
    tree->root = a;
    tree->node_count = 1;

    nary_add_child_append(tree, a, strdup("B"));
    nary_add_child_append(tree, a, strdup("C"));
    nary_add_child_append(tree, a, strdup("D"));

    NaryNode *c_node = child_at(a, 1);
    nary_add_child_append(tree, c_node, strdup("G"));
    nary_add_child_append(tree, c_node, strdup("H"));
    /* tree now has 6 nodes: A, B, C, D, G, H */

    NaryNode *g = c_node->first_child;
    NaryNode *h = g->next_sibling;
    NaryNode *b = child_at(a, 0);
    NaryNode *d = child_at(a, 2);

    int rc = nary_remove_node_promote_children(tree, c_node);
    assert(rc == 0);
    assert(tree->node_count == 5);
    assert(g_free_count == 1);  /* C's data freed */

    /* A's children: B -> G -> H -> D */
    assert(count_children(a) == 4);
    assert(child_at(a, 0) == b);
    assert(child_at(a, 1) == g);
    assert(child_at(a, 2) == h);
    assert(child_at(a, 3) == d);

    assert(g->parent == a);
    assert(h->parent == a);

    nary_tree_destroy(tree);
    printf("  [PASS] promote_children on middle child with children\n");
}

static void test_promote_last_child_with_children(void)
{
    /*
     *     A            A
     *    / \    =>    / | \
     *   B   C       B   E  F
     *      / \
     *     E   F
     *
     * Remove C (last child) which has children E, F.
     */
    g_free_count = 0;
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *a = nary_node_create(strdup("A"));
    tree->root = a;
    tree->node_count = 1;

    nary_add_child_append(tree, a, strdup("B"));
    nary_add_child_append(tree, a, strdup("C"));

    NaryNode *c_node = child_at(a, 1);
    nary_add_child_append(tree, c_node, strdup("E"));
    nary_add_child_append(tree, c_node, strdup("F"));

    NaryNode *b = child_at(a, 0);
    NaryNode *e = c_node->first_child;
    NaryNode *f = e->next_sibling;

    int rc = nary_remove_node_promote_children(tree, c_node);
    assert(rc == 0);
    assert(tree->node_count == 4);

    /* A's children: B -> E -> F */
    assert(count_children(a) == 3);
    assert(child_at(a, 0) == b);
    assert(child_at(a, 1) == e);
    assert(child_at(a, 2) == f);
    assert(f->next_sibling == NULL);

    assert(e->parent == a);
    assert(f->parent == a);

    nary_tree_destroy(tree);
    printf("  [PASS] promote_children on last child with children\n");
}

static void test_promote_null_node(void)
{
    int rc = nary_remove_node_promote_children(NULL, NULL);
    assert(rc == -1);
    printf("  [PASS] promote_children with NULL node returns -1\n");
}

static void test_promote_root_node(void)
{
    NaryTree *tree = build_test_tree();
    int rc = nary_remove_node_promote_children(tree, tree->root);
    assert(rc == -2);
    nary_tree_destroy(tree);
    printf("  [PASS] promote_children on root returns -2\n");
}

/* --- nary_remove_subtree ------------------------------------------------- */

static void test_remove_subtree_leaf(void)
{
    /*  A(B, C, D(E,F))  =>  remove D  =>  A(B, C)  */
    g_free_count = 0;
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *a = nary_node_create(strdup("A"));
    tree->root = a;
    tree->node_count = 1;

    nary_add_child_append(tree, a, strdup("B"));
    nary_add_child_append(tree, a, strdup("C"));
    nary_add_child_append(tree, a, strdup("D"));

    NaryNode *d = child_at(a, 2);
    nary_add_child_append(tree, d, strdup("E"));
    nary_add_child_append(tree, d, strdup("F"));
    /* 6 nodes total */

    int rc = nary_remove_subtree(tree, d);
    assert(rc == 0);
    assert(tree->node_count == 3);  /* A, B, C remain */
    assert(g_free_count == 3);      /* D, E, F freed */
    assert(count_children(a) == 2);
    assert(strcmp((char *)child_at(a, 0)->data, "B") == 0);
    assert(strcmp((char *)child_at(a, 1)->data, "C") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] remove_subtree on interior node with children\n");
}

static void test_remove_subtree_single_leaf(void)
{
    g_free_count = 0;
    NaryTree *tree = build_test_tree();  /* A(B(E,F), C, D) — 6 nodes */

    NaryNode *b = tree->root->first_child;
    NaryNode *e = b->first_child;  /* leaf */

    int rc = nary_remove_subtree(tree, e);
    assert(rc == 0);
    assert(tree->node_count == 5);
    assert(g_free_count == 1);
    /* B now only has F */
    assert(count_children(b) == 1);
    assert(strcmp((char *)b->first_child->data, "F") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] remove_subtree on single leaf\n");
}

static void test_remove_subtree_root(void)
{
    g_free_count = 0;
    NaryTree *tree = build_test_tree();  /* 6 nodes */

    int rc = nary_remove_subtree(tree, tree->root);
    assert(rc == 0);
    assert(tree->root == NULL);
    assert(tree->node_count == 0);
    assert(g_free_count == 6);

    free(tree);  /* tree struct itself */
    printf("  [PASS] remove_subtree on root empties tree\n");
}

static void test_remove_subtree_middle_child(void)
{
    /*  A(B, C, D)  =>  remove C  =>  A(B, D) */
    g_free_count = 0;
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *a = nary_node_create(strdup("A"));
    tree->root = a;
    tree->node_count = 1;

    nary_add_child_append(tree, a, strdup("B"));
    nary_add_child_append(tree, a, strdup("C"));
    nary_add_child_append(tree, a, strdup("D"));

    NaryNode *c_node = child_at(a, 1);
    int rc = nary_remove_subtree(tree, c_node);
    assert(rc == 0);
    assert(tree->node_count == 3);
    assert(count_children(a) == 2);
    assert(strcmp((char *)child_at(a, 0)->data, "B") == 0);
    assert(strcmp((char *)child_at(a, 1)->data, "D") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] remove_subtree on middle child\n");
}

static void test_remove_subtree_null(void)
{
    int rc = nary_remove_subtree(NULL, NULL);
    assert(rc == -1);
    printf("  [PASS] remove_subtree with NULL node returns -1\n");
}

/* --- nary_detach_subtree ------------------------------------------------- */

static void test_detach_leaf(void)
{
    g_free_count = 0;
    NaryTree *tree = build_test_tree();  /* A(B(E,F), C, D) — 6 nodes */

    NaryNode *b = tree->root->first_child;
    NaryNode *e = b->first_child;

    int rc = nary_detach_subtree(tree, e);
    assert(rc == 0);
    assert(tree->node_count == 5);

    /* E is now detached — independent root */
    assert(e->parent == NULL);
    assert(e->next_sibling == NULL);

    /* B now has only F */
    assert(count_children(b) == 1);
    assert(strcmp((char *)b->first_child->data, "F") == 0);

    /* Clean up detached node */
    counting_free(e->data);
    free(e);

    nary_tree_destroy(tree);
    printf("  [PASS] detach leaf node\n");
}

static void test_detach_subtree_with_children(void)
{
    g_free_count = 0;
    NaryTree *tree = build_test_tree();  /* A(B(E,F), C, D) — 6 nodes */

    NaryNode *a = tree->root;
    NaryNode *b = a->first_child;

    int rc = nary_detach_subtree(tree, b);
    assert(rc == 0);
    assert(tree->node_count == 3);  /* A, C, D remain */

    /* B is detached with its subtree intact */
    assert(b->parent == NULL);
    assert(b->next_sibling == NULL);
    assert(count_children(b) == 2);  /* E, F still there */
    assert(strcmp((char *)b->first_child->data, "E") == 0);

    /* A now has C, D */
    assert(count_children(a) == 2);
    assert(strcmp((char *)child_at(a, 0)->data, "C") == 0);
    assert(strcmp((char *)child_at(a, 1)->data, "D") == 0);

    /* Clean up detached subtree */
    nary_subtree_destroy(b, counting_free);

    nary_tree_destroy(tree);
    printf("  [PASS] detach subtree with children\n");
}

static void test_detach_middle_child(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *a = nary_node_create(strdup("A"));
    tree->root = a;
    tree->node_count = 1;

    nary_add_child_append(tree, a, strdup("B"));
    nary_add_child_append(tree, a, strdup("C"));
    nary_add_child_append(tree, a, strdup("D"));

    NaryNode *c_node = child_at(a, 1);
    int rc = nary_detach_subtree(tree, c_node);
    assert(rc == 0);
    assert(tree->node_count == 3);

    assert(c_node->parent == NULL);
    assert(c_node->next_sibling == NULL);

    /* A has B, D */
    assert(count_children(a) == 2);
    assert(strcmp((char *)child_at(a, 0)->data, "B") == 0);
    assert(strcmp((char *)child_at(a, 1)->data, "D") == 0);

    /* Clean up */
    counting_free(c_node->data);
    free(c_node);

    nary_tree_destroy(tree);
    printf("  [PASS] detach middle child\n");
}

static void test_detach_last_child(void)
{
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *a = nary_node_create(strdup("A"));
    tree->root = a;
    tree->node_count = 1;

    nary_add_child_append(tree, a, strdup("B"));
    nary_add_child_append(tree, a, strdup("C"));

    NaryNode *c_node = child_at(a, 1);
    int rc = nary_detach_subtree(tree, c_node);
    assert(rc == 0);
    assert(tree->node_count == 2);

    assert(count_children(a) == 1);
    assert(strcmp((char *)child_at(a, 0)->data, "B") == 0);

    counting_free(c_node->data);
    free(c_node);

    nary_tree_destroy(tree);
    printf("  [PASS] detach last child\n");
}

static void test_detach_null_node(void)
{
    int rc = nary_detach_subtree(NULL, NULL);
    assert(rc == -1);
    printf("  [PASS] detach with NULL node returns -1\n");
}

static void test_detach_already_detached(void)
{
    NaryNode *n = nary_node_create(strdup("X"));
    int rc = nary_detach_subtree(NULL, n);
    assert(rc == -2);  /* no parent */
    free(n->data);
    free(n);
    printf("  [PASS] detach already-detached node returns -2\n");
}

static void test_detach_then_reattach(void)
{
    /*
     * Detach B from A(B(E,F), C, D), then attach B under D.
     *
     *   Before: A(B(E,F), C, D)
     *   After:  A(C, D(B(E,F)))
     */
    g_free_count = 0;
    NaryTree *tree = build_test_tree();  /* A(B(E,F), C, D) — 6 nodes */

    NaryNode *a = tree->root;
    NaryNode *b = a->first_child;
    NaryNode *d = child_at(a, 2);

    /* Detach B */
    int rc = nary_detach_subtree(tree, b);
    assert(rc == 0);
    assert(tree->node_count == 3);

    /* Reattach B under D */
    rc = nary_attach_subtree(tree, d, b);
    assert(rc == 0);
    assert(tree->node_count == 6);

    /* Verify structure: A(C, D(B(E,F))) */
    assert(count_children(a) == 2);
    assert(strcmp((char *)child_at(a, 0)->data, "C") == 0);
    assert(strcmp((char *)child_at(a, 1)->data, "D") == 0);
    assert(d->first_child == b);
    assert(b->parent == d);
    assert(count_children(b) == 2);

    nary_tree_destroy(tree);
    printf("  [PASS] detach then reattach preserves subtree\n");
}

static void test_detach_only_child(void)
{
    /* A has only child B. Detach B => A has no children. */
    NaryTree *tree = nary_tree_create(counting_free, NULL, NULL);
    NaryNode *a = nary_node_create(strdup("A"));
    tree->root = a;
    tree->node_count = 1;
    nary_add_child_append(tree, a, strdup("B"));

    NaryNode *b = a->first_child;
    int rc = nary_detach_subtree(tree, b);
    assert(rc == 0);
    assert(tree->node_count == 1);
    assert(a->first_child == NULL);
    assert(b->parent == NULL);

    counting_free(b->data);
    free(b);
    nary_tree_destroy(tree);
    printf("  [PASS] detach only child leaves parent childless\n");
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

    test_append_to_empty_parent();
    test_append_multiple_children();
    test_append_null_parent();
    test_append_null_tree();

    test_prepend_to_empty_parent();
    test_prepend_multiple_children();
    test_prepend_null_parent();

    test_insert_at_zero_empty_parent();
    test_insert_at_beginning();
    test_insert_at_middle();
    test_insert_at_end();
    test_insert_at_beyond_end();
    test_insert_at_null_parent();
    test_insert_at_index_on_no_children();

    test_attach_subtree_single_node();
    test_attach_subtree_multi_node();
    test_attach_subtree_as_last_sibling();
    test_attach_subtree_null_parent();
    test_attach_subtree_null_subtree();
    test_attach_subtree_already_parented();

    test_mixed_operations_build_tree();
    test_append_deep_chain();

    printf("\nN-ary Tree — Category 3: Deletion & Detachment\n");
    printf("=================================================\n");

    /* nary_remove_node_promote_children */
    test_promote_leaf_node();
    test_promote_first_child_with_children();
    test_promote_middle_child_with_children();
    test_promote_last_child_with_children();
    test_promote_null_node();
    test_promote_root_node();

    /* nary_remove_subtree */
    test_remove_subtree_leaf();
    test_remove_subtree_single_leaf();
    test_remove_subtree_root();
    test_remove_subtree_middle_child();
    test_remove_subtree_null();

    /* nary_detach_subtree */
    test_detach_leaf();
    test_detach_subtree_with_children();
    test_detach_middle_child();
    test_detach_last_child();
    test_detach_null_node();
    test_detach_already_detached();
    test_detach_then_reattach();
    test_detach_only_child();

    printf("\nAll 55 tests passed.\n");
    return 0;
}
