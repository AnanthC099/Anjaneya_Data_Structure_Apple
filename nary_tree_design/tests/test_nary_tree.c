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

/* ---- Helper: string comparator ----------------------------------------- */
static int str_cmp(const void *a, const void *b)
{
    return strcmp((const char *)a, (const char *)b);
}

/* ---- Helper: string printer -------------------------------------------- */
static void str_print(const void *data, FILE *out)
{
    fprintf(out, "%s", (const char *)data);
}

/* ---- Helper: string cloner --------------------------------------------- */
static void *str_clone(const void *data)
{
    return strdup((const char *)data);
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
    NaryTree *tree = nary_tree_create(counting_free, str_cmp, str_print);
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
    assert(strcmp((char *)child_at(root, 0)->data, "A") == 0);
    assert(strcmp((char *)child_at(root, 1)->data, "B") == 0);
    assert(strcmp((char *)child_at(root, 2)->data, "C") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] append multiple children preserves order\n");
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
    assert(strcmp((char *)child_at(root, 0)->data, "A") == 0);
    assert(strcmp((char *)child_at(root, 1)->data, "B") == 0);
    assert(strcmp((char *)child_at(root, 2)->data, "C") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] prepend multiple children preserves reverse-insertion order\n");
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
    assert(strcmp((char *)child_at(root, 0)->data, "A") == 0);
    assert(strcmp((char *)child_at(root, 1)->data, "B") == 0);
    assert(strcmp((char *)child_at(root, 2)->data, "C") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] insert_at middle index\n");
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

/* ===========================================================================
 * Category 3: Deletion & Detachment Tests
 * =========================================================================== */

static void test_promote_first_child_with_children(void)
{
    g_free_count = 0;
    NaryTree *tree = build_test_tree();

    NaryNode *a = tree->root;
    NaryNode *b = a->first_child;
    NaryNode *e = b->first_child;
    NaryNode *f = e->next_sibling;
    NaryNode *c = b->next_sibling;

    int rc = nary_remove_node_promote_children(tree, b);
    assert(rc == 0);
    assert(tree->node_count == 5);
    assert(g_free_count == 1);

    assert(count_children(a) == 4);
    assert(child_at(a, 0) == e);
    assert(child_at(a, 1) == f);
    assert(child_at(a, 2) == c);
    assert(e->parent == a);
    assert(f->parent == a);

    nary_tree_destroy(tree);
    printf("  [PASS] promote_children on first child with children\n");
}

static void test_remove_subtree_leaf(void)
{
    g_free_count = 0;
    NaryTree *tree = build_test_tree();

    NaryNode *b = tree->root->first_child;
    NaryNode *e = b->first_child;

    int rc = nary_remove_subtree(tree, e);
    assert(rc == 0);
    assert(tree->node_count == 5);
    assert(g_free_count == 1);
    assert(count_children(b) == 1);
    assert(strcmp((char *)b->first_child->data, "F") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] remove_subtree on single leaf\n");
}

static void test_detach_then_reattach(void)
{
    g_free_count = 0;
    NaryTree *tree = build_test_tree();

    NaryNode *a = tree->root;
    NaryNode *b = a->first_child;
    NaryNode *d = child_at(a, 2);

    int rc = nary_detach_subtree(tree, b);
    assert(rc == 0);
    assert(tree->node_count == 3);

    rc = nary_attach_subtree(tree, d, b);
    assert(rc == 0);
    assert(tree->node_count == 6);

    assert(count_children(a) == 2);
    assert(d->first_child == b);
    assert(b->parent == d);

    nary_tree_destroy(tree);
    printf("  [PASS] detach then reattach preserves subtree\n");
}

/* ===========================================================================
 * Category 4: Traversal Tests
 * =========================================================================== */

typedef struct {
    char labels[64][16];
    size_t count;
} TraversalLog;

static void log_visitor(NaryNode *node, void *user_data)
{
    TraversalLog *log = user_data;
    strncpy(log->labels[log->count], (char *)node->data, 15);
    log->labels[log->count][15] = '\0';
    log->count++;
}

static void test_pre_order_traversal(void)
{
    NaryTree *tree = build_test_tree();
    TraversalLog log = { .count = 0 };

    int rc = nary_traverse_pre_order(tree->root, log_visitor, &log);
    assert(rc == 0);
    assert(log.count == 6);
    /* Pre-order: A, B, E, F, C, D */
    assert(strcmp(log.labels[0], "A") == 0);
    assert(strcmp(log.labels[1], "B") == 0);
    assert(strcmp(log.labels[2], "E") == 0);
    assert(strcmp(log.labels[3], "F") == 0);
    assert(strcmp(log.labels[4], "C") == 0);
    assert(strcmp(log.labels[5], "D") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] pre-order traversal\n");
}

static void test_post_order_traversal(void)
{
    NaryTree *tree = build_test_tree();
    TraversalLog log = { .count = 0 };

    int rc = nary_traverse_post_order(tree->root, log_visitor, &log);
    assert(rc == 0);
    assert(log.count == 6);
    /* Post-order: E, F, B, C, D, A */
    assert(strcmp(log.labels[0], "E") == 0);
    assert(strcmp(log.labels[1], "F") == 0);
    assert(strcmp(log.labels[2], "B") == 0);
    assert(strcmp(log.labels[3], "C") == 0);
    assert(strcmp(log.labels[4], "D") == 0);
    assert(strcmp(log.labels[5], "A") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] post-order traversal\n");
}

static void test_level_order_traversal(void)
{
    NaryTree *tree = build_test_tree();
    TraversalLog log = { .count = 0 };

    int rc = nary_traverse_level_order(tree->root, log_visitor, &log);
    assert(rc == 0);
    assert(log.count == 6);
    /* Level-order: A, B, C, D, E, F */
    assert(strcmp(log.labels[0], "A") == 0);
    assert(strcmp(log.labels[1], "B") == 0);
    assert(strcmp(log.labels[2], "C") == 0);
    assert(strcmp(log.labels[3], "D") == 0);
    assert(strcmp(log.labels[4], "E") == 0);
    assert(strcmp(log.labels[5], "F") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] level-order traversal\n");
}

static void test_reverse_level_order_traversal(void)
{
    NaryTree *tree = build_test_tree();
    TraversalLog log = { .count = 0 };

    int rc = nary_traverse_reverse_level_order(tree->root, log_visitor, &log);
    assert(rc == 0);
    assert(log.count == 6);
    /* Reverse level-order: F, E, D, C, B, A */
    assert(strcmp(log.labels[0], "F") == 0);
    assert(strcmp(log.labels[1], "E") == 0);
    assert(strcmp(log.labels[2], "D") == 0);
    assert(strcmp(log.labels[3], "C") == 0);
    assert(strcmp(log.labels[4], "B") == 0);
    assert(strcmp(log.labels[5], "A") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] reverse level-order traversal\n");
}

static void test_traversal_null_inputs(void)
{
    assert(nary_traverse_pre_order(NULL, log_visitor, NULL) == -1);
    assert(nary_traverse_post_order(NULL, log_visitor, NULL) == -1);
    assert(nary_traverse_level_order(NULL, log_visitor, NULL) == -1);
    assert(nary_traverse_reverse_level_order(NULL, log_visitor, NULL) == -1);
    printf("  [PASS] traversal NULL inputs return -1\n");
}

static void test_traversal_single_node(void)
{
    NaryNode *n = nary_node_create(strdup("X"));
    TraversalLog log = { .count = 0 };

    nary_traverse_pre_order(n, log_visitor, &log);
    assert(log.count == 1);
    assert(strcmp(log.labels[0], "X") == 0);

    log.count = 0;
    nary_traverse_level_order(n, log_visitor, &log);
    assert(log.count == 1);

    free(n->data);
    free(n);
    printf("  [PASS] traversal on single node\n");
}

/* ===========================================================================
 * Category 5: Search & Query Tests
 * =========================================================================== */

static void test_find_by_value_exists(void)
{
    NaryTree *tree = build_test_tree();

    NaryNode *found = nary_find_by_value(tree->root, "E", str_cmp);
    assert(found != NULL);
    assert(strcmp((char *)found->data, "E") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] find_by_value returns matching node\n");
}

static void test_find_by_value_not_found(void)
{
    NaryTree *tree = build_test_tree();

    NaryNode *found = nary_find_by_value(tree->root, "Z", str_cmp);
    assert(found == NULL);

    nary_tree_destroy(tree);
    printf("  [PASS] find_by_value returns NULL for missing value\n");
}

static void test_find_by_value_root(void)
{
    NaryTree *tree = build_test_tree();

    NaryNode *found = nary_find_by_value(tree->root, "A", str_cmp);
    assert(found == tree->root);

    nary_tree_destroy(tree);
    printf("  [PASS] find_by_value finds root\n");
}

static void test_find_all_by_value(void)
{
    /* Build tree with duplicate values */
    NaryTree *tree = nary_tree_create(counting_free, str_cmp, str_print);
    NaryNode *root = nary_node_create(strdup("A"));
    tree->root = root;
    tree->node_count = 1;

    nary_add_child_append(tree, root, strdup("B"));
    nary_add_child_append(tree, root, strdup("A"));  /* duplicate */
    nary_add_child_append(tree, root, strdup("C"));

    NaryNode *results[4];
    int count = nary_find_all_by_value(root, "A", str_cmp, results, 4);
    assert(count == 2);
    assert(strcmp((char *)results[0]->data, "A") == 0);
    assert(strcmp((char *)results[1]->data, "A") == 0);
    assert(results[0] == root);
    assert(results[1] != root);

    nary_tree_destroy(tree);
    printf("  [PASS] find_all_by_value finds duplicates\n");
}

static void test_find_by_path(void)
{
    NaryTree *tree = build_test_tree();
    /*         A
     *       / | \
     *      B  C  D
     *     / \
     *    E   F
     */

    /* Path to F: root -> child 0 (B) -> child 1 (F) */
    size_t path[] = {0, 1};
    NaryNode *found = nary_find_by_path(tree->root, path, 2);
    assert(found != NULL);
    assert(strcmp((char *)found->data, "F") == 0);

    /* Path to D: root -> child 2 (D) */
    size_t path2[] = {2};
    found = nary_find_by_path(tree->root, path2, 1);
    assert(found != NULL);
    assert(strcmp((char *)found->data, "D") == 0);

    /* Invalid path */
    size_t path3[] = {5};
    found = nary_find_by_path(tree->root, path3, 1);
    assert(found == NULL);

    /* Empty path returns root */
    found = nary_find_by_path(tree->root, NULL, 0);
    assert(found == tree->root);

    nary_tree_destroy(tree);
    printf("  [PASS] find_by_path navigates correctly\n");
}

static void test_node_exists(void)
{
    NaryTree *tree = build_test_tree();

    assert(nary_node_exists(tree->root, "E", str_cmp) == 1);
    assert(nary_node_exists(tree->root, "Z", str_cmp) == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] node_exists returns correct boolean\n");
}

/* ===========================================================================
 * Category 6: Structural Query Tests
 * =========================================================================== */

static void test_degree(void)
{
    NaryTree *tree = build_test_tree();

    assert(nary_degree(tree->root) == 3);       /* A has B,C,D */
    assert(nary_degree(tree->root->first_child) == 2); /* B has E,F */
    assert(nary_degree(child_at(tree->root, 1)) == 0); /* C is leaf */
    assert(nary_degree(NULL) == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] degree of nodes\n");
}

static void test_tree_degree(void)
{
    NaryTree *tree = build_test_tree();
    assert(nary_tree_degree(tree->root) == 3);
    nary_tree_destroy(tree);
    printf("  [PASS] tree degree\n");
}

static void test_depth(void)
{
    NaryTree *tree = build_test_tree();

    assert(nary_depth(tree->root) == 0);
    assert(nary_depth(tree->root->first_child) == 1);  /* B */
    NaryNode *e = tree->root->first_child->first_child;
    assert(nary_depth(e) == 2);  /* E */

    nary_tree_destroy(tree);
    printf("  [PASS] depth of nodes\n");
}

static void test_height(void)
{
    NaryTree *tree = build_test_tree();

    assert(nary_height(tree->root) == 2);       /* A -> B -> E */
    assert(nary_height(tree->root->first_child) == 1); /* B -> E */
    NaryNode *e = tree->root->first_child->first_child;
    assert(nary_height(e) == 0);                /* E is leaf */

    nary_tree_destroy(tree);
    printf("  [PASS] height of nodes\n");
}

static void test_size(void)
{
    NaryTree *tree = build_test_tree();

    assert(nary_size(tree->root) == 6);
    assert(nary_size(tree->root->first_child) == 3);  /* B, E, F */
    assert(nary_size(child_at(tree->root, 1)) == 1);  /* C alone */

    nary_tree_destroy(tree);
    printf("  [PASS] size of subtrees\n");
}

static void test_is_leaf_is_root(void)
{
    NaryTree *tree = build_test_tree();

    assert(nary_is_root(tree->root) == 1);
    assert(nary_is_root(tree->root->first_child) == 0);
    assert(nary_is_leaf(tree->root) == 0);
    NaryNode *e = tree->root->first_child->first_child;
    assert(nary_is_leaf(e) == 1);
    assert(nary_is_leaf(NULL) == 0);
    assert(nary_is_root(NULL) == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] is_leaf and is_root\n");
}

static void test_count_leaves(void)
{
    NaryTree *tree = build_test_tree();
    /* Leaves: E, F, C, D = 4 */
    assert(nary_count_leaves(tree->root) == 4);
    assert(nary_count_leaves(tree->root->first_child) == 2); /* E, F */

    nary_tree_destroy(tree);
    printf("  [PASS] count_leaves\n");
}

static void test_count_internal(void)
{
    NaryTree *tree = build_test_tree();
    /* Internal: A, B = 2 */
    assert(nary_count_internal(tree->root) == 2);

    nary_tree_destroy(tree);
    printf("  [PASS] count_internal\n");
}

/* ===========================================================================
 * Category 7: Relationship Query Tests
 * =========================================================================== */

static void test_parent_and_children(void)
{
    NaryTree *tree = build_test_tree();

    assert(nary_parent_of(tree->root) == NULL);
    assert(nary_parent_of(tree->root->first_child) == tree->root);
    assert(nary_children_of(tree->root) == tree->root->first_child);
    assert(nary_children_of(child_at(tree->root, 1)) == NULL);

    nary_tree_destroy(tree);
    printf("  [PASS] parent_of and children_of\n");
}

static void test_siblings_of(void)
{
    NaryTree *tree = build_test_tree();

    NaryNode *sibs[8];
    NaryNode *b = tree->root->first_child;
    int count = nary_siblings_of(b, sibs, 8);
    assert(count == 2);  /* C and D */
    assert(strcmp((char *)sibs[0]->data, "C") == 0);
    assert(strcmp((char *)sibs[1]->data, "D") == 0);

    /* Root has no siblings */
    count = nary_siblings_of(tree->root, sibs, 8);
    assert(count == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] siblings_of\n");
}

static void test_ancestors_of(void)
{
    NaryTree *tree = build_test_tree();

    NaryNode *ancestors[8];
    NaryNode *e = tree->root->first_child->first_child;  /* E */
    int count = nary_ancestors_of(e, ancestors, 8);
    assert(count == 2);  /* B, A */
    assert(strcmp((char *)ancestors[0]->data, "B") == 0);
    assert(strcmp((char *)ancestors[1]->data, "A") == 0);

    /* Root has no ancestors */
    count = nary_ancestors_of(tree->root, ancestors, 8);
    assert(count == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] ancestors_of\n");
}

static void test_is_ancestor_descendant(void)
{
    NaryTree *tree = build_test_tree();

    NaryNode *a = tree->root;
    NaryNode *b = a->first_child;
    NaryNode *e = b->first_child;
    NaryNode *c = b->next_sibling;

    assert(nary_is_ancestor(a, e) == 1);
    assert(nary_is_ancestor(b, e) == 1);
    assert(nary_is_ancestor(c, e) == 0);
    assert(nary_is_ancestor(e, a) == 0);

    assert(nary_is_descendant(e, a) == 1);
    assert(nary_is_descendant(a, e) == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] is_ancestor and is_descendant\n");
}

static void test_lca(void)
{
    NaryTree *tree = build_test_tree();

    NaryNode *a = tree->root;
    NaryNode *b = a->first_child;
    NaryNode *e = b->first_child;
    NaryNode *f = e->next_sibling;
    NaryNode *c = b->next_sibling;
    NaryNode *d = c->next_sibling;

    /* LCA(E, F) = B */
    assert(nary_lca(e, f) == b);

    /* LCA(E, C) = A */
    assert(nary_lca(e, c) == a);

    /* LCA(E, D) = A */
    assert(nary_lca(e, d) == a);

    /* LCA(B, D) = A */
    assert(nary_lca(b, d) == a);

    /* LCA(E, E) = E */
    assert(nary_lca(e, e) == e);

    /* LCA(A, E) = A */
    assert(nary_lca(a, e) == a);

    nary_tree_destroy(tree);
    printf("  [PASS] lowest common ancestor\n");
}

static void test_path_between(void)
{
    NaryTree *tree = build_test_tree();

    NaryNode *e = tree->root->first_child->first_child;
    NaryNode *d = child_at(tree->root, 2);

    NaryNode *path[16];
    int len = nary_path_between(e, d, path, 16);
    assert(len == 4);  /* E -> B -> A -> D */
    assert(strcmp((char *)path[0]->data, "E") == 0);
    assert(strcmp((char *)path[1]->data, "B") == 0);
    assert(strcmp((char *)path[2]->data, "A") == 0);
    assert(strcmp((char *)path[3]->data, "D") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] path_between\n");
}

/* ===========================================================================
 * Category 8: Mutation & Restructuring Tests
 * =========================================================================== */

static void test_move_subtree(void)
{
    NaryTree *tree = build_test_tree();

    NaryNode *a = tree->root;
    NaryNode *b = a->first_child;
    NaryNode *d = child_at(a, 2);

    /* Move B (with E, F) under D */
    int rc = nary_move_subtree(tree, b, d);
    assert(rc == 0);
    assert(b->parent == d);
    assert(d->first_child == b);

    /* A should now have C, D only */
    assert(count_children(a) == 2);
    assert(strcmp((char *)child_at(a, 0)->data, "C") == 0);
    assert(strcmp((char *)child_at(a, 1)->data, "D") == 0);

    /* B still has E, F */
    assert(count_children(b) == 2);

    nary_tree_destroy(tree);
    printf("  [PASS] move_subtree\n");
}

static void test_move_subtree_ancestor_check(void)
{
    NaryTree *tree = build_test_tree();

    NaryNode *a = tree->root;
    NaryNode *e = a->first_child->first_child;

    /* Cannot move A under E (A is ancestor of E) */
    int rc = nary_move_subtree(tree, a, e);
    assert(rc == -2);

    nary_tree_destroy(tree);
    printf("  [PASS] move_subtree rejects ancestor-to-descendant\n");
}

static void test_swap_siblings(void)
{
    NaryTree *tree = build_test_tree();

    NaryNode *a = tree->root;
    NaryNode *b = child_at(a, 0);
    NaryNode *c = child_at(a, 1);
    NaryNode *d = child_at(a, 2);

    /* Swap B and D (non-adjacent) */
    int rc = nary_swap_siblings(b, d);
    assert(rc == 0);
    assert(child_at(a, 0) == d);
    assert(child_at(a, 1) == c);
    assert(child_at(a, 2) == b);

    nary_tree_destroy(tree);
    printf("  [PASS] swap_siblings non-adjacent\n");
}

static void test_swap_siblings_adjacent(void)
{
    NaryTree *tree = build_test_tree();

    NaryNode *a = tree->root;
    NaryNode *b = child_at(a, 0);
    NaryNode *c = child_at(a, 1);

    /* Swap B and C (adjacent) */
    int rc = nary_swap_siblings(b, c);
    assert(rc == 0);
    assert(child_at(a, 0) == c);
    assert(child_at(a, 1) == b);
    assert(strcmp((char *)child_at(a, 2)->data, "D") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] swap_siblings adjacent\n");
}

static void test_swap_siblings_different_parents(void)
{
    NaryTree *tree = build_test_tree();

    NaryNode *b = tree->root->first_child;
    NaryNode *e = b->first_child;

    /* C and E have different parents */
    NaryNode *c = b->next_sibling;
    int rc = nary_swap_siblings(c, e);
    assert(rc == -2);

    nary_tree_destroy(tree);
    printf("  [PASS] swap_siblings rejects different parents\n");
}

static void test_swap_subtrees(void)
{
    /*
     *    A             A
     *   / \    =>     / \
     *  B   C         C   B
     *  |   |         |   |
     *  D   E         E   D
     */
    NaryTree *tree = nary_tree_create(counting_free, str_cmp, str_print);
    NaryNode *a = nary_node_create(strdup("A"));
    tree->root = a;
    tree->node_count = 1;

    nary_add_child_append(tree, a, strdup("B"));
    nary_add_child_append(tree, a, strdup("C"));

    NaryNode *b = child_at(a, 0);
    NaryNode *c = child_at(a, 1);
    nary_add_child_append(tree, b, strdup("D"));
    nary_add_child_append(tree, c, strdup("E"));

    int rc = nary_swap_subtrees(tree, b, c);
    assert(rc == 0);
    assert(child_at(a, 0) == c);
    assert(child_at(a, 1) == b);
    assert(c->parent == a);
    assert(b->parent == a);

    nary_tree_destroy(tree);
    printf("  [PASS] swap_subtrees with different parents\n");
}

static void test_sort_children(void)
{
    NaryTree *tree = nary_tree_create(counting_free, str_cmp, str_print);
    NaryNode *root = nary_node_create(strdup("root"));
    tree->root = root;
    tree->node_count = 1;

    nary_add_child_append(tree, root, strdup("C"));
    nary_add_child_append(tree, root, strdup("A"));
    nary_add_child_append(tree, root, strdup("B"));

    int rc = nary_sort_children(root, str_cmp);
    assert(rc == 0);
    assert(strcmp((char *)child_at(root, 0)->data, "A") == 0);
    assert(strcmp((char *)child_at(root, 1)->data, "B") == 0);
    assert(strcmp((char *)child_at(root, 2)->data, "C") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] sort_children\n");
}

static void test_flatten(void)
{
    NaryTree *tree = build_test_tree();

    int rc = nary_flatten(tree);
    assert(rc == 0);
    /* All 5 non-root nodes become direct children of A */
    assert(count_children(tree->root) == 5);
    /* Pre-order: B, E, F, C, D */
    assert(strcmp((char *)child_at(tree->root, 0)->data, "B") == 0);
    assert(strcmp((char *)child_at(tree->root, 1)->data, "E") == 0);
    assert(strcmp((char *)child_at(tree->root, 2)->data, "F") == 0);
    assert(strcmp((char *)child_at(tree->root, 3)->data, "C") == 0);
    assert(strcmp((char *)child_at(tree->root, 4)->data, "D") == 0);

    /* All children are leaves now */
    for (NaryNode *c = tree->root->first_child; c; c = c->next_sibling) {
        assert(c->first_child == NULL);
        assert(c->parent == tree->root);
    }

    nary_tree_destroy(tree);
    printf("  [PASS] flatten tree\n");
}

static void test_merge_trees(void)
{
    g_free_count = 0;
    NaryTree *tree1 = nary_tree_create(counting_free, str_cmp, str_print);
    NaryNode *a = nary_node_create(strdup("A"));
    tree1->root = a;
    tree1->node_count = 1;
    nary_add_child_append(tree1, a, strdup("B"));

    NaryTree *tree2 = nary_tree_create(counting_free, str_cmp, str_print);
    NaryNode *x = nary_node_create(strdup("X"));
    tree2->root = x;
    tree2->node_count = 1;
    nary_add_child_append(tree2, x, strdup("Y"));

    int rc = nary_merge_trees(tree1, tree2);
    assert(rc == 0);
    assert(tree1->node_count == 4);  /* A, B, X, Y */

    /* X should be a child of A now */
    assert(count_children(a) == 2);
    assert(strcmp((char *)child_at(a, 0)->data, "B") == 0);
    assert(strcmp((char *)child_at(a, 1)->data, "X") == 0);
    assert(child_at(a, 1)->parent == a);

    /* tree2 was freed by merge, only destroy tree1 */
    nary_tree_destroy(tree1);
    printf("  [PASS] merge_trees\n");
}

/* ===========================================================================
 * Category 9: Serialization Tests
 * =========================================================================== */

static void test_serialize_parenthesized(void)
{
    NaryTree *tree = build_test_tree();

    char buf[256];
    int len = nary_serialize_parenthesized(tree->root, str_print, buf, 256);
    assert(len > 0);
    /* Expected: "A(B(E F) C D)" */
    assert(strcmp(buf, "A(B(E F) C D)") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] serialize_parenthesized\n");
}

static void test_serialize_parenthesized_single(void)
{
    NaryNode *n = nary_node_create(strdup("X"));
    char buf[64];
    int len = nary_serialize_parenthesized(n, str_print, buf, 64);
    assert(len > 0);
    assert(strcmp(buf, "X") == 0);

    free(n->data);
    free(n);
    printf("  [PASS] serialize_parenthesized single node\n");
}

static void test_print_indented(void)
{
    NaryTree *tree = build_test_tree();

    char buf[1024];
    FILE *f = fmemopen(buf, sizeof(buf), "w");
    assert(f != NULL);
    int rc = nary_print_indented(tree->root, str_print, f);
    fclose(f);
    assert(rc == 0);

    /* Verify output contains expected structure */
    assert(strstr(buf, "A\n") != NULL);
    assert(strstr(buf, "  B\n") != NULL);
    assert(strstr(buf, "    E\n") != NULL);
    assert(strstr(buf, "    F\n") != NULL);
    assert(strstr(buf, "  C\n") != NULL);
    assert(strstr(buf, "  D\n") != NULL);

    nary_tree_destroy(tree);
    printf("  [PASS] print_indented\n");
}

static void test_serialize_dot(void)
{
    NaryTree *tree = build_test_tree();

    char buf[2048];
    FILE *f = fmemopen(buf, sizeof(buf), "w");
    assert(f != NULL);
    int rc = nary_serialize_dot(tree->root, str_print, f);
    fclose(f);
    assert(rc == 0);

    assert(strstr(buf, "digraph NaryTree") != NULL);
    assert(strstr(buf, "->") != NULL);

    nary_tree_destroy(tree);
    printf("  [PASS] serialize_dot\n");
}

static void test_serialization_null(void)
{
    char buf[64];
    assert(nary_serialize_parenthesized(NULL, str_print, buf, 64) == -1);
    assert(nary_print_indented(NULL, str_print, stdout) == -1);
    assert(nary_serialize_dot(NULL, str_print, stdout) == -1);
    printf("  [PASS] serialization NULL inputs\n");
}

/* ===========================================================================
 * Category 10: Copying & Comparison Tests
 * =========================================================================== */

static void test_deep_clone(void)
{
    NaryTree *tree = build_test_tree();

    NaryNode *clone = nary_deep_clone(tree->root, str_clone);
    assert(clone != NULL);

    /* Clone has same structure */
    assert(strcmp((char *)clone->data, "A") == 0);
    assert(clone->parent == NULL);
    assert(count_children(clone) == 3);
    assert(strcmp((char *)clone->first_child->data, "B") == 0);
    assert(count_children(clone->first_child) == 2);

    /* Clone is independent — different pointers */
    assert(clone != tree->root);
    assert(clone->first_child != tree->root->first_child);

    /* Full equality */
    assert(nary_full_equal(tree->root, clone, str_cmp) == 1);

    nary_subtree_destroy(clone, free);
    nary_tree_destroy(tree);
    printf("  [PASS] deep_clone\n");
}

static void test_structural_equal(void)
{
    NaryTree *tree1 = build_test_tree();
    NaryTree *tree2 = build_test_tree();

    assert(nary_structural_equal(tree1->root, tree2->root) == 1);

    /* Add a node to tree2 to make them unequal */
    nary_add_child_append(tree2, child_at(tree2->root, 1), strdup("G"));
    assert(nary_structural_equal(tree1->root, tree2->root) == 0);

    nary_tree_destroy(tree1);
    nary_tree_destroy(tree2);
    printf("  [PASS] structural_equal\n");
}

static void test_full_equal(void)
{
    NaryTree *tree1 = build_test_tree();
    NaryNode *clone = nary_deep_clone(tree1->root, str_clone);

    assert(nary_full_equal(tree1->root, clone, str_cmp) == 1);

    /* Change a value to make them unequal */
    free(clone->first_child->data);
    clone->first_child->data = strdup("Z");
    assert(nary_full_equal(tree1->root, clone, str_cmp) == 0);

    nary_subtree_destroy(clone, free);
    nary_tree_destroy(tree1);
    printf("  [PASS] full_equal\n");
}

static void test_mirror(void)
{
    NaryTree *tree = build_test_tree();

    int rc = nary_mirror(tree->root);
    assert(rc == 0);

    /* After mirror: A's children are D, C, B (reversed) */
    assert(strcmp((char *)child_at(tree->root, 0)->data, "D") == 0);
    assert(strcmp((char *)child_at(tree->root, 1)->data, "C") == 0);
    assert(strcmp((char *)child_at(tree->root, 2)->data, "B") == 0);

    /* B's children are also reversed: F, E */
    NaryNode *b = child_at(tree->root, 2);
    assert(strcmp((char *)child_at(b, 0)->data, "F") == 0);
    assert(strcmp((char *)child_at(b, 1)->data, "E") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] mirror\n");
}

static void test_is_mirror(void)
{
    NaryTree *tree1 = build_test_tree();
    NaryNode *clone = nary_deep_clone(tree1->root, str_clone);

    /* A tree is not its own mirror (unless symmetric) */
    nary_mirror(clone);
    assert(nary_is_mirror(tree1->root, clone, str_cmp) == 1);

    nary_subtree_destroy(clone, free);
    nary_tree_destroy(tree1);
    printf("  [PASS] is_mirror\n");
}

static void test_equality_null(void)
{
    NaryNode *n = nary_node_create(strdup("A"));
    assert(nary_structural_equal(NULL, NULL) == 1);
    assert(nary_structural_equal(n, NULL) == 0);
    assert(nary_full_equal(NULL, NULL, str_cmp) == 1);
    free(n->data);
    free(n);
    printf("  [PASS] equality NULL cases\n");
}

/* ===========================================================================
 * Category 11: Utility & Statistics Tests
 * =========================================================================== */

static void test_diameter(void)
{
    NaryTree *tree = build_test_tree();
    /*         A
     *       / | \
     *      B  C  D
     *     / \
     *    E   F
     *
     * Longest path: E -> B -> A -> C (or D) = 3 edges
     */
    assert(nary_diameter(tree->root) == 3);

    /* Single node diameter = 0 */
    NaryNode *n = nary_node_create(strdup("X"));
    assert(nary_diameter(n) == 0);
    free(n->data);
    free(n);

    nary_tree_destroy(tree);
    printf("  [PASS] diameter\n");
}

static void test_width_at_level(void)
{
    NaryTree *tree = build_test_tree();

    assert(nary_width_at_level(tree->root, 0) == 1);  /* A */
    assert(nary_width_at_level(tree->root, 1) == 3);  /* B, C, D */
    assert(nary_width_at_level(tree->root, 2) == 2);  /* E, F */
    assert(nary_width_at_level(tree->root, 3) == 0);  /* nothing */

    nary_tree_destroy(tree);
    printf("  [PASS] width_at_level\n");
}

static void test_max_width(void)
{
    NaryTree *tree = build_test_tree();

    size_t level;
    size_t max_w = nary_max_width(tree->root, &level);
    assert(max_w == 3);   /* level 1 has B, C, D */
    assert(level == 1);

    nary_tree_destroy(tree);
    printf("  [PASS] max_width\n");
}

static void test_root_to_leaf_paths(void)
{
    NaryTree *tree = build_test_tree();

    NaryNode **paths[8];
    size_t lens[8];
    int count = nary_root_to_leaf_paths(tree->root, paths, lens, 8);
    assert(count == 4);  /* E, F, C, D are leaves */

    /* Path to E: A -> B -> E (len 3) */
    assert(lens[0] == 3);
    assert(strcmp((char *)paths[0][0]->data, "A") == 0);
    assert(strcmp((char *)paths[0][1]->data, "B") == 0);
    assert(strcmp((char *)paths[0][2]->data, "E") == 0);

    /* Path to F: A -> B -> F (len 3) */
    assert(lens[1] == 3);
    assert(strcmp((char *)paths[1][2]->data, "F") == 0);

    /* Path to C: A -> C (len 2) */
    assert(lens[2] == 2);
    assert(strcmp((char *)paths[2][1]->data, "C") == 0);

    /* Path to D: A -> D (len 2) */
    assert(lens[3] == 2);
    assert(strcmp((char *)paths[3][1]->data, "D") == 0);

    for (int i = 0; i < count; i++)
        free(paths[i]);

    nary_tree_destroy(tree);
    printf("  [PASS] root_to_leaf_paths\n");
}

static void *int_accumulate(void *acc, void *data, void *user_data)
{
    (void)user_data;
    size_t *sum = acc;
    *sum += strlen((const char *)data);
    return sum;
}

static void test_fold(void)
{
    NaryTree *tree = build_test_tree();

    /* Sum string lengths (each node has 1-char string) */
    size_t sum = 0;
    void *result = nary_fold(tree->root, int_accumulate, &sum, NULL);
    assert(result == &sum);
    assert(sum == 6);  /* A + B + C + D + E + F = 6 chars */

    nary_tree_destroy(tree);
    printf("  [PASS] fold\n");
}

static void *uppercase_mapper(void *data, void *user_data)
{
    (void)user_data;
    char *s = data;
    for (size_t i = 0; s[i]; i++) {
        if (s[i] >= 'a' && s[i] <= 'z')
            s[i] -= 32;
    }
    return s;
}

static void test_apply(void)
{
    NaryTree *tree = nary_tree_create(counting_free, str_cmp, str_print);
    NaryNode *root = nary_node_create(strdup("abc"));
    tree->root = root;
    tree->node_count = 1;
    nary_add_child_append(tree, root, strdup("def"));

    int rc = nary_apply(root, uppercase_mapper, NULL);
    assert(rc == 0);
    assert(strcmp((char *)root->data, "ABC") == 0);
    assert(strcmp((char *)root->first_child->data, "DEF") == 0);

    nary_tree_destroy(tree);
    printf("  [PASS] apply\n");
}

static void test_utility_null(void)
{
    assert(nary_diameter(NULL) == 0);
    assert(nary_width_at_level(NULL, 0) == 0);
    assert(nary_max_width(NULL, NULL) == 0);
    assert(nary_root_to_leaf_paths(NULL, NULL, NULL, 0) == -1);
    assert(nary_apply(NULL, uppercase_mapper, NULL) == -1);
    printf("  [PASS] utility NULL inputs\n");
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
    test_prepend_multiple_children();
    test_insert_at_middle();
    test_attach_subtree_multi_node();

    printf("\nN-ary Tree — Category 3: Deletion & Detachment\n");
    printf("=================================================\n");

    test_promote_first_child_with_children();
    test_remove_subtree_leaf();
    test_detach_then_reattach();

    printf("\nN-ary Tree — Category 4: Traversals\n");
    printf("======================================\n");

    test_pre_order_traversal();
    test_post_order_traversal();
    test_level_order_traversal();
    test_reverse_level_order_traversal();
    test_traversal_null_inputs();
    test_traversal_single_node();

    printf("\nN-ary Tree — Category 5: Search & Query\n");
    printf("==========================================\n");

    test_find_by_value_exists();
    test_find_by_value_not_found();
    test_find_by_value_root();
    test_find_all_by_value();
    test_find_by_path();
    test_node_exists();

    printf("\nN-ary Tree — Category 6: Structural Queries\n");
    printf("==============================================\n");

    test_degree();
    test_tree_degree();
    test_depth();
    test_height();
    test_size();
    test_is_leaf_is_root();
    test_count_leaves();
    test_count_internal();

    printf("\nN-ary Tree — Category 7: Relationship Queries\n");
    printf("================================================\n");

    test_parent_and_children();
    test_siblings_of();
    test_ancestors_of();
    test_is_ancestor_descendant();
    test_lca();
    test_path_between();

    printf("\nN-ary Tree — Category 8: Mutation & Restructuring\n");
    printf("====================================================\n");

    test_move_subtree();
    test_move_subtree_ancestor_check();
    test_swap_siblings();
    test_swap_siblings_adjacent();
    test_swap_siblings_different_parents();
    test_swap_subtrees();
    test_sort_children();
    test_flatten();
    test_merge_trees();

    printf("\nN-ary Tree — Category 9: Serialization\n");
    printf("=========================================\n");

    test_serialize_parenthesized();
    test_serialize_parenthesized_single();
    test_print_indented();
    test_serialize_dot();
    test_serialization_null();

    printf("\nN-ary Tree — Category 10: Copying & Comparison\n");
    printf("=================================================\n");

    test_deep_clone();
    test_structural_equal();
    test_full_equal();
    test_mirror();
    test_is_mirror();
    test_equality_null();

    printf("\nN-ary Tree — Category 11: Utility & Statistics\n");
    printf("=================================================\n");

    test_diameter();
    test_width_at_level();
    test_max_width();
    test_root_to_leaf_paths();
    test_fold();
    test_apply();
    test_utility_null();

    printf("\nAll tests passed.\n");
    return 0;
}
