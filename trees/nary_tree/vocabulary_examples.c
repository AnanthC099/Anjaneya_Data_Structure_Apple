/*
 * ==============================================================================
 *   N-ARY TREE VOCABULARY — EXPLAINED WITH EXAMPLES
 * ==============================================================================
 *
 * This file explains every term from the "VOCABULARY (memorize these)" section
 * in 01_what_is_nary_tree.c, using the SAME example tree:
 *
 *               1            ← root
 *          /  |   \
 *         2    3    4        ← level 1
 *        /|\       / \
 *       5  6  7   8   9     ← level 2
 *          |
 *         10                 ← level 3
 *
 * Each term is explained, then demonstrated with C code you can run.
 *
 *
 * ==============================================================================
 *  1. ROOT — "the topmost node (no parent)"
 * ==============================================================================
 *
 *  The root is the starting point of the entire tree. It has NO parent.
 *  Every other node is a descendant of the root.
 *
 *  In our tree:  Node 1 is the root.
 *
 *  How to spot it: it's the node you pass to every recursive function.
 *  In code: build_example_tree() returns node1 — that's the root.
 *
 *  Real-world analogy:
 *      File system → "/" (root directory)
 *      Org chart   → CEO
 *      Family tree → oldest ancestor at the top
 *
 *
 * ==============================================================================
 *  2. PARENT — "a node that has children"
 * ==============================================================================
 *
 *  A parent is any node that has at least one child connected below it.
 *
 *  In our tree:
 *      Node 1 is the parent of nodes 2, 3, 4
 *      Node 2 is the parent of nodes 5, 6, 7
 *      Node 4 is the parent of nodes 8, 9
 *      Node 6 is the parent of node 10
 *
 *  NOT parents (they have 0 children):
 *      Nodes 3, 5, 7, 8, 9, 10
 *
 *  In code:
 *      if (node->child_count > 0)  → this node is a parent
 *
 *
 * ==============================================================================
 *  3. CHILD — "a node directly below another node"
 * ==============================================================================
 *
 *  A child is one level directly below its parent. The keyword is DIRECTLY —
 *  grandchildren don't count as children.
 *
 *  In our tree:
 *      Children of node 1: {2, 3, 4}
 *      Children of node 2: {5, 6, 7}
 *      Children of node 4: {8, 9}
 *      Children of node 6: {10}
 *      Children of node 3: {} (none — it's a leaf)
 *
 *  IMPORTANT: Node 5 is a child of node 2, but NOT a child of node 1.
 *  Node 5 is a GRANDCHILD (descendant) of node 1.
 *
 *  In code:
 *      parent->children[i] gives you the i-th child of parent.
 *      node1->children[0] == node2
 *      node1->children[1] == node3
 *      node1->children[2] == node4
 *
 *
 * ==============================================================================
 *  4. SIBLINGS — "nodes that share the same parent"
 * ==============================================================================
 *
 *  Siblings are like brothers and sisters — same parent, same level.
 *
 *  In our tree:
 *      {2, 3, 4} are siblings  (all children of node 1)
 *      {5, 6, 7} are siblings  (all children of node 2)
 *      {8, 9}    are siblings  (all children of node 4)
 *
 *  NOT siblings:
 *      {3, 5} are NOT siblings — even though they're on different levels,
 *      they have DIFFERENT parents (3's parent is 1, 5's parent is 2).
 *
 *      {5, 8} are NOT siblings — same level but different parents.
 *
 *  In code:
 *      All entries in parent->children[] are siblings of each other.
 *      node2->children[0], node2->children[1], node2->children[2]
 *      → nodes 5, 6, 7 are siblings.
 *
 *
 * ==============================================================================
 *  5. LEAF — "a node with NO children (also called external node)"
 * ==============================================================================
 *
 *  A leaf is a dead end — it has no children below it.
 *  Think of it like the leaves on an actual tree: they're at the tips.
 *
 *  In our tree:
 *      Leaves: {3, 5, 7, 8, 9, 10}
 *
 *  How to verify:
 *      Node 3  → child_count = 0 → LEAF ✓
 *      Node 5  → child_count = 0 → LEAF ✓
 *      Node 10 → child_count = 0 → LEAF ✓
 *      Node 2  → child_count = 3 → NOT a leaf (has children 5, 6, 7)
 *
 *  In code:
 *      if (node->child_count == 0) → this node is a leaf
 *
 *
 * ==============================================================================
 *  6. INTERNAL NODE — "a node that has at least one child"
 * ==============================================================================
 *
 *  Internal nodes are the OPPOSITE of leaves. They have children.
 *  "Internal" because they're in the interior of the tree, not at the edges.
 *
 *  In our tree:
 *      Internal nodes: {1, 2, 4, 6}
 *
 *      Node 1 → has children {2,3,4}  → INTERNAL
 *      Node 2 → has children {5,6,7}  → INTERNAL
 *      Node 4 → has children {8,9}    → INTERNAL
 *      Node 6 → has child {10}        → INTERNAL
 *
 *  Rule: Every node is EITHER a leaf OR an internal node. Never both.
 *        Internal nodes + Leaves = All nodes
 *        {1,2,4,6} + {3,5,7,8,9,10} = {1,2,3,4,5,6,7,8,9,10}  ✓
 *
 *
 * ==============================================================================
 *  7. EDGE — "the connection between parent and child"
 * ==============================================================================
 *
 *  An edge is the line/link connecting a parent to one of its children.
 *  Each parent-child relationship = 1 edge.
 *
 *  In our tree, the edges are:
 *      1→2,  1→3,  1→4       (3 edges from node 1)
 *      2→5,  2→6,  2→7       (3 edges from node 2)
 *      6→10                   (1 edge from node 6)
 *      4→8,  4→9             (2 edges from node 4)
 *                             ─────────────────────
 *                              Total: 9 edges
 *
 *  KEY FORMULA:
 *      Number of edges = Number of nodes - 1
 *      In our tree: 10 nodes - 1 = 9 edges  ✓
 *
 *  Why? Because every node (except root) has exactly ONE parent,
 *  so every node (except root) contributes exactly ONE edge.
 *
 *
 * ==============================================================================
 *  8. PATH — "sequence of nodes from one node to another"
 * ==============================================================================
 *
 *  A path is the trail you follow through edges to get from node A to node B.
 *  In a tree, there is EXACTLY ONE path between any two nodes.
 *
 *  In our tree:
 *      Path from 1 to 10:  1 → 2 → 6 → 10   (length 3 edges)
 *      Path from 1 to 9:   1 → 4 → 9         (length 2 edges)
 *      Path from 5 to 8:   5 → 2 → 1 → 4 → 8 (length 4 edges, goes UP then DOWN)
 *      Path from 1 to 1:   just 1             (length 0 edges)
 *
 *  IMPORTANT: In a tree, you NEVER have two different paths to the same node.
 *  That's what makes it a tree (vs. a graph which can have cycles/multiple paths).
 *
 *
 * ==============================================================================
 *  9. DEPTH — "how many edges from root to this node (root depth = 0)"
 * ==============================================================================
 *
 *  Depth measures how far DOWN a node is from the root.
 *  Think of it as "how many steps from the top?"
 *
 *  In our tree:
 *      Node 1  → depth 0  (it IS the root)
 *      Node 2  → depth 1  (1 edge: 1→2)
 *      Node 3  → depth 1  (1 edge: 1→3)
 *      Node 4  → depth 1  (1 edge: 1→4)
 *      Node 5  → depth 2  (2 edges: 1→2→5)
 *      Node 6  → depth 2  (2 edges: 1→2→6)
 *      Node 7  → depth 2  (2 edges: 1→2→7)
 *      Node 8  → depth 2  (2 edges: 1→4→8)
 *      Node 9  → depth 2  (2 edges: 1→4→9)
 *      Node 10 → depth 3  (3 edges: 1→2→6→10)
 *
 *  In code (from 01_what_is_nary_tree.c):
 *      find_depth(root, 10, 0) returns 3
 *      find_depth(root, 1, 0)  returns 0
 *
 *
 * ==============================================================================
 *  10. HEIGHT — "how many edges from this node to its deepest leaf"
 * ==============================================================================
 *
 *  Height measures how far DOWN you can go from a node. It looks DOWNWARD.
 *  (Depth looks UP to the root; Height looks DOWN to the deepest leaf.)
 *
 *  In our tree:
 *      Node 10 → height 0  (it's a leaf, no children below)
 *      Node 6  → height 1  (deepest leaf below is 10, 1 edge away)
 *      Node 5  → height 0  (leaf)
 *      Node 2  → height 2  (deepest leaf below is 10: 2→6→10 = 2 edges)
 *      Node 4  → height 1  (deepest leaves are 8,9, both 1 edge away)
 *      Node 3  → height 0  (leaf)
 *      Node 1  → height 3  (deepest leaf is 10: 1→2→6→10 = 3 edges)
 *
 *  "Height of the tree" = height of the root = 3
 *
 *  DEPTH vs HEIGHT (the classic confusion):
 *  ┌──────────┬────────────────────────────────────────┐
 *  │          │  Depth          │  Height               │
 *  ├──────────┼────────────────────────────────────────┤
 *  │ Measured │  from ROOT      │  from THIS NODE       │
 *  │ Goes     │  downward       │  downward to leaf     │
 *  │ Root     │  depth = 0      │  height = max depth   │
 *  │ Leaf     │  depth = varies │  height = 0           │
 *  └──────────┴────────────────────────────────────────┘
 *
 *
 * ==============================================================================
 *  11. LEVEL — "same as depth (Level 0 = root, Level 1 = root's children)"
 * ==============================================================================
 *
 *  Level and Depth are essentially the same thing.
 *  "Level" groups nodes by their depth.
 *
 *  In our tree:
 *      Level 0: {1}                (just the root)
 *      Level 1: {2, 3, 4}         (root's children)
 *      Level 2: {5, 6, 7, 8, 9}   (grandchildren of root)
 *      Level 3: {10}              (great-grandchild)
 *
 *  "Level-order traversal" means visiting all nodes level by level
 *  (also called Breadth-First Search / BFS):
 *      Visit order: 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
 *
 *
 * ==============================================================================
 *  12. SUBTREE — "a node and ALL its descendants"
 * ==============================================================================
 *
 *  Pick any node — that node plus everything below it forms a subtree.
 *  A subtree is itself a valid tree!
 *
 *  In our tree:
 *      Subtree rooted at node 2:
 *               2
 *              /|\
 *             5  6  7
 *                |
 *               10
 *      (contains nodes {2, 5, 6, 7, 10})
 *
 *      Subtree rooted at node 4:
 *               4
 *              / \
 *             8   9
 *      (contains nodes {4, 8, 9})
 *
 *      Subtree rooted at node 6:
 *               6
 *               |
 *              10
 *      (contains nodes {6, 10})
 *
 *      Subtree rooted at node 10:
 *              10
 *      (just one node — a single leaf IS a valid subtree)
 *
 *      Subtree rooted at node 1 = the ENTIRE tree.
 *
 *  This is why recursive functions work so beautifully on trees:
 *  "solve for this subtree" = "solve for this node + each child's subtree"
 *
 *
 * ==============================================================================
 *  13. DEGREE — "number of children a node has"
 * ==============================================================================
 *
 *  Degree = how many children. Simple.
 *
 *  In our tree:
 *      Node 1  → degree 3  (children: 2, 3, 4)
 *      Node 2  → degree 3  (children: 5, 6, 7)
 *      Node 3  → degree 0  (no children)
 *      Node 4  → degree 2  (children: 8, 9)
 *      Node 5  → degree 0
 *      Node 6  → degree 1  (child: 10)
 *      Node 7  → degree 0
 *      Node 8  → degree 0
 *      Node 9  → degree 0
 *      Node 10 → degree 0
 *
 *  "Degree of the tree" = MAXIMUM degree of any node = 3 (nodes 1 and 2)
 *
 *  In code:
 *      node->child_count IS the degree.
 *
 *  Connection to tree types:
 *      If max degree = 2 → it's a Binary tree
 *      If max degree = 3 → it's a Ternary tree
 *      If max degree = N → it's an N-ary tree
 *
 *
 * ==============================================================================
 *  14. FOREST — "a collection of trees (remove root → you get a forest)"
 * ==============================================================================
 *
 *  A forest is multiple separate trees grouped together.
 *
 *  If you DELETE node 1 (the root) from our tree, you get a FOREST of 3 trees:
 *
 *      Tree A:       Tree B:       Tree C:
 *        2              3             4
 *       /|\                          / \
 *      5  6  7                      8   9
 *         |
 *        10
 *
 *  These 3 trees together = a forest.
 *
 *  Real-world example:
 *      A file system with multiple drives:
 *          C:\    and    D:\    and    E:\
 *      Each drive is a separate tree. Together they form a forest.
 *
 *  In code: a forest is just an array of root pointers:
 *      NaryTreeNode *forest[] = {tree_a_root, tree_b_root, tree_c_root};
 *
 *
 * ==============================================================================
 *  QUICK REFERENCE CHEAT SHEET (for our example tree)
 * ==============================================================================
 *
 *               1
 *          /  |   \
 *         2    3    4
 *        /|\       / \
 *       5  6  7   8   9
 *          |
 *         10
 *
 *  ┌───────────────┬──────────────────────────────────┐
 *  │ Term          │ Example from our tree            │
 *  ├───────────────┼──────────────────────────────────┤
 *  │ Root          │ 1                                │
 *  │ Parents       │ 1, 2, 4, 6                       │
 *  │ Children of 1 │ {2, 3, 4}                        │
 *  │ Siblings      │ {2,3,4}, {5,6,7}, {8,9}          │
 *  │ Leaves        │ 3, 5, 7, 8, 9, 10                │
 *  │ Internal      │ 1, 2, 4, 6                       │
 *  │ Edges         │ 9 (= 10 nodes - 1)               │
 *  │ Path 1→10     │ 1 → 2 → 6 → 10                  │
 *  │ Depth of 10   │ 3                                │
 *  │ Height of 1   │ 3                                │
 *  │ Levels        │ 0:{1}, 1:{2,3,4}, 2:{5,6,7,8,9} │
 *  │               │ 3:{10}                           │
 *  │ Subtree at 2  │ {2,5,6,7,10}                     │
 *  │ Degree of 1   │ 3                                │
 *  │ Forest        │ Remove 1 → {tree@2, tree@3, tree@4} │
 *  └───────────────┴──────────────────────────────────┘
 */


/* ===========================================================================
 *  RUNNABLE CODE: Demonstrates each vocabulary term programmatically
 * =========================================================================== */

#include <stdio.h>
#include <stdlib.h>

#define INITIAL_CAPACITY 4

typedef struct NaryTreeNode {
    int val;
    int child_count;
    int child_capacity;
    struct NaryTreeNode **children;
} NaryTreeNode;


NaryTreeNode *create_node(int val) {
    NaryTreeNode *node = (NaryTreeNode *)malloc(sizeof(NaryTreeNode));
    if (!node) { printf("ERROR: malloc failed!\n"); exit(1); }
    node->val = val;
    node->child_count = 0;
    node->child_capacity = INITIAL_CAPACITY;
    node->children = (NaryTreeNode **)calloc(INITIAL_CAPACITY, sizeof(NaryTreeNode *));
    if (!node->children) { free(node); printf("ERROR: calloc failed!\n"); exit(1); }
    return node;
}

void add_child(NaryTreeNode *parent, NaryTreeNode *child) {
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity *= 2;
        parent->children = (NaryTreeNode **)realloc(
            parent->children, parent->child_capacity * sizeof(NaryTreeNode *));
        if (!parent->children) { printf("ERROR: realloc failed!\n"); exit(1); }
    }
    parent->children[parent->child_count++] = child;
}

void free_tree(NaryTreeNode *root) {
    if (!root) return;
    for (int i = 0; i < root->child_count; i++) free_tree(root->children[i]);
    free(root->children);
    free(root);
}

NaryTreeNode *build_example_tree(void) {
    NaryTreeNode *n1  = create_node(1);
    NaryTreeNode *n2  = create_node(2);
    NaryTreeNode *n3  = create_node(3);
    NaryTreeNode *n4  = create_node(4);
    NaryTreeNode *n5  = create_node(5);
    NaryTreeNode *n6  = create_node(6);
    NaryTreeNode *n7  = create_node(7);
    NaryTreeNode *n8  = create_node(8);
    NaryTreeNode *n9  = create_node(9);
    NaryTreeNode *n10 = create_node(10);

    add_child(n1, n2); add_child(n1, n3); add_child(n1, n4);
    add_child(n2, n5); add_child(n2, n6); add_child(n2, n7);
    add_child(n6, n10);
    add_child(n4, n8); add_child(n4, n9);
    return n1;
}


/* --- Demonstration functions --- */

/* 1. ROOT: check if a node is the root (no parent tracking, so root is what we pass in) */
void demo_root(NaryTreeNode *root) {
    printf("1. ROOT\n");
    printf("   The root node has value: %d\n", root->val);
    printf("   It has %d children and no parent.\n\n", root->child_count);
}

/* 2 & 3. PARENT and CHILD: show parent-child relationships */
void demo_parent_child(NaryTreeNode *node) {
    printf("2. PARENT & 3. CHILD\n");
    if (node->child_count > 0) {
        printf("   Node %d is a PARENT. Its children: {", node->val);
        for (int i = 0; i < node->child_count; i++) {
            printf("%d%s", node->children[i]->val,
                   i < node->child_count - 1 ? ", " : "");
        }
        printf("}\n");
    } else {
        printf("   Node %d has no children (it's a leaf, not a parent).\n", node->val);
    }
    printf("\n");
}

/* 4. SIBLINGS: print siblings (children of the same parent) */
void demo_siblings(NaryTreeNode *parent) {
    printf("4. SIBLINGS\n");
    if (parent->child_count < 2) {
        printf("   Node %d has fewer than 2 children — no sibling group.\n\n", parent->val);
        return;
    }
    printf("   Children of node %d are siblings of each other: {", parent->val);
    for (int i = 0; i < parent->child_count; i++) {
        printf("%d%s", parent->children[i]->val,
               i < parent->child_count - 1 ? ", " : "");
    }
    printf("}\n\n");
}

/* 5 & 6. LEAF and INTERNAL: classify every node */
void classify_nodes(NaryTreeNode *root, int *leaves, int *leaf_cnt,
                    int *internals, int *int_cnt) {
    if (!root) return;
    if (root->child_count == 0) {
        leaves[(*leaf_cnt)++] = root->val;
    } else {
        internals[(*int_cnt)++] = root->val;
    }
    for (int i = 0; i < root->child_count; i++)
        classify_nodes(root->children[i], leaves, leaf_cnt, internals, int_cnt);
}

void demo_leaf_internal(NaryTreeNode *root) {
    int leaves[20], internals[20], lc = 0, ic = 0;
    classify_nodes(root, leaves, &lc, internals, &ic);

    printf("5. LEAVES (no children)\n   {");
    for (int i = 0; i < lc; i++) printf("%d%s", leaves[i], i < lc-1 ? ", " : "");
    printf("}\n\n");

    printf("6. INTERNAL NODES (have children)\n   {");
    for (int i = 0; i < ic; i++) printf("%d%s", internals[i], i < ic-1 ? ", " : "");
    printf("}\n\n");
}

/* 7. EDGE: count edges */
int count_edges(NaryTreeNode *root) {
    if (!root) return 0;
    int edges = root->child_count;  /* one edge per child */
    for (int i = 0; i < root->child_count; i++)
        edges += count_edges(root->children[i]);
    return edges;
}

void demo_edges(NaryTreeNode *root) {
    int total_edges = count_edges(root);
    printf("7. EDGES\n");
    printf("   Total edges: %d  (= total nodes - 1)\n\n", total_edges);
}

/* 8. PATH: find and print path from root to a target */
int find_path(NaryTreeNode *root, int target, int *path, int *path_len) {
    if (!root) return 0;
    path[*path_len] = root->val;
    (*path_len)++;
    if (root->val == target) return 1;
    for (int i = 0; i < root->child_count; i++) {
        if (find_path(root->children[i], target, path, path_len)) return 1;
    }
    (*path_len)--;  /* backtrack */
    return 0;
}

void demo_path(NaryTreeNode *root, int target) {
    int path[20], path_len = 0;
    printf("8. PATH\n");
    if (find_path(root, target, path, &path_len)) {
        printf("   Path from root to %d: ", target);
        for (int i = 0; i < path_len; i++)
            printf("%d%s", path[i], i < path_len-1 ? " -> " : "");
        printf("  (length: %d edges)\n\n", path_len - 1);
    }
}

/* 9. DEPTH: edges from root to a node */
int find_depth(NaryTreeNode *root, int target, int depth) {
    if (!root) return -1;
    if (root->val == target) return depth;
    for (int i = 0; i < root->child_count; i++) {
        int r = find_depth(root->children[i], target, depth + 1);
        if (r != -1) return r;
    }
    return -1;
}

void demo_depth(NaryTreeNode *root) {
    printf("9. DEPTH (edges from root)\n");
    int nodes[] = {1, 2, 5, 10, 4, 8};
    for (int i = 0; i < 6; i++)
        printf("   Depth of node %2d = %d\n", nodes[i],
               find_depth(root, nodes[i], 0));
    printf("\n");
}

/* 10. HEIGHT: edges from a node to its deepest leaf */
int find_height(NaryTreeNode *root) {
    if (!root) return -1;
    if (root->child_count == 0) return 0;
    int max_h = 0;
    for (int i = 0; i < root->child_count; i++) {
        int h = find_height(root->children[i]);
        if (h > max_h) max_h = h;
    }
    return 1 + max_h;
}

void demo_height(NaryTreeNode *root) {
    printf("10. HEIGHT (edges to deepest leaf below)\n");
    printf("    Height of tree (root node 1) = %d\n", find_height(root));
    /* Show height of node 2's subtree */
    printf("    Height of node 2's subtree   = %d\n", find_height(root->children[0]));
    /* Height of a leaf */
    printf("    Height of node 3 (leaf)      = %d\n", find_height(root->children[1]));
    printf("\n");
}

/* 11. LEVEL: group nodes by depth */
void collect_level(NaryTreeNode *root, int target_level, int current_level,
                   int *result, int *count) {
    if (!root) return;
    if (current_level == target_level) {
        result[(*count)++] = root->val;
        return;
    }
    for (int i = 0; i < root->child_count; i++)
        collect_level(root->children[i], target_level, current_level + 1, result, count);
}

void demo_levels(NaryTreeNode *root) {
    printf("11. LEVEL (= depth grouping)\n");
    for (int level = 0; level <= 3; level++) {
        int nodes[20], cnt = 0;
        collect_level(root, level, 0, nodes, &cnt);
        printf("    Level %d: {", level);
        for (int i = 0; i < cnt; i++)
            printf("%d%s", nodes[i], i < cnt-1 ? ", " : "");
        printf("}\n");
    }
    printf("\n");
}

/* 12. SUBTREE: count nodes in a subtree */
int count_nodes(NaryTreeNode *root) {
    if (!root) return 0;
    int c = 1;
    for (int i = 0; i < root->child_count; i++) c += count_nodes(root->children[i]);
    return c;
}

void demo_subtree(NaryTreeNode *root) {
    printf("12. SUBTREE\n");
    printf("    Subtree rooted at node 1 contains %d nodes (entire tree)\n", count_nodes(root));
    printf("    Subtree rooted at node 2 contains %d nodes\n", count_nodes(root->children[0]));
    printf("    Subtree rooted at node 4 contains %d nodes\n", count_nodes(root->children[2]));
    printf("    Subtree rooted at node 6 contains %d nodes\n",
           count_nodes(root->children[0]->children[1]));
    printf("\n");
}

/* 13. DEGREE: number of children */
void demo_degree(NaryTreeNode *root) {
    printf("13. DEGREE (number of children)\n");
    printf("    Node 1: degree %d\n", root->child_count);
    printf("    Node 2: degree %d\n", root->children[0]->child_count);
    printf("    Node 3: degree %d\n", root->children[1]->child_count);
    printf("    Node 4: degree %d\n", root->children[2]->child_count);
    printf("    Node 6: degree %d\n", root->children[0]->children[1]->child_count);
    printf("    Degree of the tree (max degree) = 3\n\n");
}

/* 14. FOREST: demonstrate removing root to create a forest */
void demo_forest(NaryTreeNode *root) {
    printf("14. FOREST (remove root -> multiple trees)\n");
    printf("    If we remove node 1, we get %d separate trees:\n", root->child_count);
    for (int i = 0; i < root->child_count; i++) {
        printf("    Tree %d: rooted at node %d (%d nodes)\n",
               i + 1, root->children[i]->val, count_nodes(root->children[i]));
    }
    printf("\n");
}


int main(void) {
    printf("================================================================\n");
    printf("  N-ARY TREE VOCABULARY — DEMONSTRATED WITH EXAMPLES\n");
    printf("================================================================\n\n");
    printf("  Example tree:\n");
    printf("               1\n");
    printf("          /  |   \\\n");
    printf("         2    3    4\n");
    printf("        /|\\       / \\\n");
    printf("       5  6  7   8   9\n");
    printf("          |\n");
    printf("         10\n\n");
    printf("================================================================\n\n");

    NaryTreeNode *root = build_example_tree();

    demo_root(root);
    demo_parent_child(root);                   /* show for root (node 1) */
    demo_parent_child(root->children[1]);      /* show for leaf (node 3) */
    demo_siblings(root);                       /* siblings under node 1 */
    demo_leaf_internal(root);
    demo_edges(root);
    demo_path(root, 10);
    demo_depth(root);
    demo_height(root);
    demo_levels(root);
    demo_subtree(root);
    demo_degree(root);
    demo_forest(root);

    printf("================================================================\n");
    printf("  TIP: Run this code, then re-read the vocabulary list.\n");
    printf("  After seeing the numbers, the definitions will click!\n");
    printf("================================================================\n");

    free_tree(root);
    return 0;
}
