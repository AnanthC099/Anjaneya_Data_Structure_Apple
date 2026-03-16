/*
 * ==============================================================================
 *   N-ARY TREE — COMMON INTERVIEW PROBLEMS (C version, full explanations)
 * ==============================================================================
 *
 * Problems covered:
 *     1. Maximum Depth (LeetCode 559)
 *     2. N-ary Tree Level Order Traversal (LeetCode 429)
 *     3. Encode N-ary to Binary / Decode Binary to N-ary (LeetCode 431)
 *     4. Diameter of N-ary Tree (LeetCode 1522)
 *     5. Serialize and Deserialize N-ary Tree (LeetCode 428)
 *     6. Clone N-ary Tree
 *     7. Find all root-to-leaf paths
 *     8. Lowest Common Ancestor (LCA)
 *     9. Count nodes at each level
 *    10. Check if two N-ary trees are identical
 *
 * OUR EXAMPLE TREE:
 * =================
 *               1
 *          /  |   \
 *         2    3    4
 *        /|\       / \
 *       5  6  7   8   9
 *          |
 *         10
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 4
#define MAX_NODES 100


/* ============================================================================
 *  N-ARY TREE NODE
 * ============================================================================ */

typedef struct NaryTreeNode {
    int val;
    int child_count;
    int child_capacity;
    struct NaryTreeNode **children;
} NaryTreeNode;

NaryTreeNode *create_node(int val) {
    NaryTreeNode *node = (NaryTreeNode *)malloc(sizeof(NaryTreeNode));
    node->val = val;
    node->child_count = 0;
    node->child_capacity = INITIAL_CAPACITY;
    node->children = (NaryTreeNode **)calloc(INITIAL_CAPACITY, sizeof(NaryTreeNode *));
    return node;
}

void add_child(NaryTreeNode *parent, NaryTreeNode *child) {
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity *= 2;
        parent->children = (NaryTreeNode **)realloc(
            parent->children, parent->child_capacity * sizeof(NaryTreeNode *));
    }
    parent->children[parent->child_count++] = child;
}

void free_tree(NaryTreeNode *root) {
    if (!root) return;
    for (int i = 0; i < root->child_count; i++)
        free_tree(root->children[i]);
    free(root->children);
    free(root);
}

NaryTreeNode *build_example_tree(void) {
    NaryTreeNode *n1 = create_node(1),  *n2 = create_node(2);
    NaryTreeNode *n3 = create_node(3),  *n4 = create_node(4);
    NaryTreeNode *n5 = create_node(5),  *n6 = create_node(6);
    NaryTreeNode *n7 = create_node(7),  *n8 = create_node(8);
    NaryTreeNode *n9 = create_node(9),  *n10 = create_node(10);

    add_child(n1, n2); add_child(n1, n3); add_child(n1, n4);
    add_child(n2, n5); add_child(n2, n6); add_child(n2, n7);
    add_child(n6, n10);
    add_child(n4, n8); add_child(n4, n9);
    return n1;
}


/* ============================================================================
 *  BINARY TREE NODE (needed for Problem 3: Encode/Decode)
 * ============================================================================ */

typedef struct BinaryTreeNode {
    int val;
    struct BinaryTreeNode *left;
    struct BinaryTreeNode *right;
} BinaryTreeNode;

BinaryTreeNode *create_binary_node(int val) {
    BinaryTreeNode *node = (BinaryTreeNode *)malloc(sizeof(BinaryTreeNode));
    node->val = val;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void free_binary_tree(BinaryTreeNode *root) {
    if (!root) return;
    free_binary_tree(root->left);
    free_binary_tree(root->right);
    free(root);
}


/* ============================================================================
 *  PROBLEM 1: Maximum Depth of N-ary Tree (LeetCode 559)
 * ============================================================================
 *
 *  "Given an n-ary tree, find its maximum depth."
 *
 *  Maximum depth = number of nodes along the longest path
 *  from root to farthest leaf.
 *
 *  Our tree:  1→2→6→10 = depth 4
 *
 *  Apple asks this because: simplest tree recursion problem.
 *  If you can't do this, the interview is over.
 */

/*
 * Recursive approach:
 * depth(node) = 1 + max(depth(child) for each child)
 *
 * Time:  O(n)
 * Space: O(h) — recursion depth
 */
int max_depth_recursive(NaryTreeNode *root) {
    if (root == NULL) return 0;
    if (root->child_count == 0) return 1;  /* leaf */

    int max_child = 0;
    for (int i = 0; i < root->child_count; i++) {
        int d = max_depth_recursive(root->children[i]);
        if (d > max_child) max_child = d;
    }
    return 1 + max_child;
}

/*
 * BFS approach: count levels.
 * Time:  O(n)
 * Space: O(w) — max width
 */
int max_depth_bfs(NaryTreeNode *root) {
    if (root == NULL) return 0;

    NaryTreeNode *queue[MAX_NODES];
    int front = 0, rear = 0;
    int depth = 0;

    queue[rear++] = root;

    while (front < rear) {
        int level_size = rear - front;
        depth++;
        for (int i = 0; i < level_size; i++) {
            NaryTreeNode *node = queue[front++];
            for (int j = 0; j < node->child_count; j++) {
                queue[rear++] = node->children[j];
            }
        }
    }
    return depth;
}


/* ============================================================================
 *  PROBLEM 2: Level Order Traversal (LeetCode 429)
 * ============================================================================
 *  Already covered in 02_traversals.c. Included here for completeness.
 */

void level_order_print(NaryTreeNode *root) {
    if (root == NULL) return;

    NaryTreeNode *queue[MAX_NODES];
    int front = 0, rear = 0;
    int level = 0;

    queue[rear++] = root;

    printf("    [");
    while (front < rear) {
        int level_size = rear - front;
        printf("[");
        for (int i = 0; i < level_size; i++) {
            NaryTreeNode *node = queue[front++];
            printf("%d%s", node->val, i < level_size - 1 ? ", " : "");
            for (int j = 0; j < node->child_count; j++)
                queue[rear++] = node->children[j];
        }
        printf("]%s", front < rear ? ", " : "");
        level++;
    }
    printf("]\n");
}


/* ============================================================================
 *  PROBLEM 3: Encode N-ary to Binary Tree (LeetCode 431) ★★★
 * ============================================================================
 *
 *  THE TRICK — "Left-Child Right-Sibling" representation:
 *
 *  For each N-ary node:
 *      - First child     → becomes LEFT child in binary tree
 *      - Next sibling    → becomes RIGHT child in binary tree
 *
 *  N-ary tree:           Binary tree:
 *       1                     1
 *    / | \                   /
 *   2   3  4                2
 *  /|\                     / \
 * 5  6  7                5    3
 *                         \  / \
 *                          6   4
 *                           \
 *                            7
 *
 *  This is actually how many C data structures store trees internally!
 *  It's memory-efficient: exactly 2 pointers per node instead of variable.
 *
 *  Time:  O(n)
 *  Space: O(n)
 */

BinaryTreeNode *encode_nary_to_binary(NaryTreeNode *root) {
    if (root == NULL) return NULL;

    BinaryTreeNode *bnode = create_binary_node(root->val);

    if (root->child_count > 0) {
        /* First child → left */
        bnode->left = encode_nary_to_binary(root->children[0]);

        /* Remaining children → right chain of first child */
        BinaryTreeNode *current = bnode->left;
        for (int i = 1; i < root->child_count; i++) {
            current->right = encode_nary_to_binary(root->children[i]);
            current = current->right;
        }
    }

    return bnode;
}

NaryTreeNode *decode_binary_to_nary(BinaryTreeNode *root) {
    if (root == NULL) return NULL;

    NaryTreeNode *nnode = create_node(root->val);

    /* Left child → first child, follow right chain for siblings */
    BinaryTreeNode *current = root->left;
    while (current) {
        add_child(nnode, decode_binary_to_nary(current));
        current = current->right;
    }

    return nnode;
}


/* ============================================================================
 *  PROBLEM 4: Diameter of N-ary Tree (LeetCode 1522) ★★★
 * ============================================================================
 *
 *  "The diameter is the longest path between any two nodes."
 *  Path does NOT need to pass through root.
 *
 *  Key insight: Longest path through any node = sum of the TWO
 *  tallest child subtree heights + 2.
 *
 *  Time:  O(n)
 *  Space: O(h)
 */

int diameter_result;  /* global to track max diameter */

int diameter_height(NaryTreeNode *node) {
    /* Returns height of subtree. Updates diameter_result as side effect. */
    if (node == NULL) return -1;
    if (node->child_count == 0) return 0;

    /* Find two tallest children */
    int tallest1 = -1, tallest2 = -1;

    for (int i = 0; i < node->child_count; i++) {
        int h = diameter_height(node->children[i]);
        if (h >= tallest1) {
            tallest2 = tallest1;
            tallest1 = h;
        } else if (h > tallest2) {
            tallest2 = h;
        }
    }

    /* Path through this node using two tallest children */
    int path;
    if (tallest2 >= 0) {
        path = tallest1 + tallest2 + 2;
    } else {
        path = tallest1 + 1;
    }

    if (path > diameter_result) {
        diameter_result = path;
    }

    return tallest1 + 1;
}

int diameter_of_nary_tree(NaryTreeNode *root) {
    diameter_result = 0;
    diameter_height(root);
    return diameter_result;
}


/* ============================================================================
 *  PROBLEM 5: Serialize / Deserialize N-ary Tree (LeetCode 428) ★★★
 * ============================================================================
 *
 *  Serialize = convert tree to a string/array
 *  Deserialize = convert back to tree
 *
 *  Strategy: Pre-order DFS. For each node, store:
 *     value, number_of_children, then recurse on each child.
 *
 *  Example: [1,3, 2,3, 5,0, 6,1, 10,0, 7,0, 3,0, 4,2, 8,0, 9,0]
 *            val,#kids, val,#kids, ...
 *
 *  Time:  O(n)
 *  Space: O(n)
 */

void serialize_helper(NaryTreeNode *node, int *buffer, int *idx) {
    if (node == NULL) return;
    buffer[(*idx)++] = node->val;
    buffer[(*idx)++] = node->child_count;
    for (int i = 0; i < node->child_count; i++) {
        serialize_helper(node->children[i], buffer, idx);
    }
}

int serialize(NaryTreeNode *root, int *buffer) {
    int count = 0;
    serialize_helper(root, buffer, &count);
    return count;  /* total ints written */
}

NaryTreeNode *deserialize_helper(int *buffer, int *idx) {
    int val = buffer[(*idx)++];
    int num_children = buffer[(*idx)++];

    NaryTreeNode *node = create_node(val);
    for (int i = 0; i < num_children; i++) {
        add_child(node, deserialize_helper(buffer, idx));
    }
    return node;
}

NaryTreeNode *deserialize(int *buffer, int size) {
    if (size == 0) return NULL;
    int idx = 0;
    return deserialize_helper(buffer, &idx);
}


/* ============================================================================
 *  PROBLEM 6: Clone an N-ary Tree
 * ============================================================================
 *
 *  Deep copy — every node is a NEW malloc'd object.
 *  Tests understanding of recursion and memory allocation.
 *
 *  Time:  O(n)
 *  Space: O(n)
 */

NaryTreeNode *clone_tree(NaryTreeNode *root) {
    if (root == NULL) return NULL;

    NaryTreeNode *new_node = create_node(root->val);
    for (int i = 0; i < root->child_count; i++) {
        add_child(new_node, clone_tree(root->children[i]));
    }
    return new_node;
}


/* ============================================================================
 *  PROBLEM 7: All Root-to-Leaf Paths ★★
 * ============================================================================
 *
 *  "Print every path from root to every leaf."
 *
 *  Our tree paths:
 *      1 → 2 → 5
 *      1 → 2 → 6 → 10
 *      1 → 2 → 7
 *      1 → 3
 *      1 → 4 → 8
 *      1 → 4 → 9
 *
 *  This is a BACKTRACKING problem — a key pattern!
 *
 *  In C, we use a fixed-size path array and track the current depth.
 *
 *  Time:  O(n * h)
 *  Space: O(h) for the path array
 */

void all_paths_helper(NaryTreeNode *node, int *path, int depth) {
    if (node == NULL) return;

    path[depth] = node->val;

    if (node->child_count == 0) {
        /* Leaf! Print the path */
        printf("      ");
        for (int i = 0; i <= depth; i++) {
            printf("%d%s", path[i], i < depth ? " -> " : "");
        }
        printf("\n");
        return;
    }

    for (int i = 0; i < node->child_count; i++) {
        all_paths_helper(node->children[i], path, depth + 1);
        /* BACKTRACK happens naturally — depth stays the same for next sibling */
    }
}

void all_root_to_leaf_paths(NaryTreeNode *root) {
    int path[MAX_NODES];
    all_paths_helper(root, path, 0);
}


/* ============================================================================
 *  PROBLEM 8: Lowest Common Ancestor (LCA) ★★★
 * ============================================================================
 *
 *  "Find the lowest (deepest) node that is an ancestor of both p and q."
 *
 *  LCA(5, 7) = 2   (both are children of 2)
 *  LCA(5, 8) = 1   (5 under 2, 8 under 4, both under 1)
 *  LCA(10,7) = 2   (10 under 6→2, 7 under 2)
 *
 *  Asked in EVERY Apple interview loop. Know it cold.
 *
 *  Time:  O(n)
 *  Space: O(h)
 */

NaryTreeNode *lca_result;  /* global to store result */

/*
 * Returns 1 if the subtree contains p or q, 0 otherwise.
 * Sets lca_result when the LCA is found.
 */
int lca_helper(NaryTreeNode *node, int p, int q) {
    if (node == NULL) return 0;

    int is_target = (node->val == p || node->val == q);

    int found_count = 0;
    for (int i = 0; i < node->child_count; i++) {
        if (lca_helper(node->children[i], p, q)) {
            found_count++;
        }
    }

    /*
     * This node is LCA if:
     *   Case 1: two children subtrees contain p and q respectively
     *   Case 2: this IS p or q, and one child subtree has the other
     */
    if (found_count >= 2 || (is_target && found_count >= 1)) {
        lca_result = node;
    }

    return is_target || found_count > 0;
}

NaryTreeNode *lowest_common_ancestor(NaryTreeNode *root, int p, int q) {
    lca_result = NULL;
    lca_helper(root, p, q);
    return lca_result;
}


/* ============================================================================
 *  PROBLEM 9: Count Nodes at Each Level
 * ============================================================================ */

void nodes_per_level(NaryTreeNode *root) {
    if (root == NULL) return;

    NaryTreeNode *queue[MAX_NODES];
    int front = 0, rear = 0;
    int level = 0;

    queue[rear++] = root;

    while (front < rear) {
        int level_size = rear - front;
        printf("    Level %d: %d node(s)\n", level, level_size);

        for (int i = 0; i < level_size; i++) {
            NaryTreeNode *node = queue[front++];
            for (int j = 0; j < node->child_count; j++) {
                queue[rear++] = node->children[j];
            }
        }
        level++;
    }
}


/* ============================================================================
 *  PROBLEM 10: Check if Two N-ary Trees are Identical
 * ============================================================================ */

int are_identical(NaryTreeNode *r1, NaryTreeNode *r2) {
    if (r1 == NULL && r2 == NULL) return 1;
    if (r1 == NULL || r2 == NULL) return 0;
    if (r1->val != r2->val) return 0;
    if (r1->child_count != r2->child_count) return 0;

    for (int i = 0; i < r1->child_count; i++) {
        if (!are_identical(r1->children[i], r2->children[i])) {
            return 0;
        }
    }
    return 1;
}


/* ============================================================================
 *  PRETTY PRINT
 * ============================================================================ */

void print_tree_helper(NaryTreeNode *node, char *prefix, int is_last, int is_root) {
    if (!node) return;
    if (is_root)
        printf("    %d\n", node->val);
    else
        printf("    %s%s%d\n", prefix, is_last ? "└── " : "├── ", node->val);

    char new_prefix[256];
    if (is_root) new_prefix[0] = '\0';
    else snprintf(new_prefix, sizeof(new_prefix), "%s%s",
                  prefix, is_last ? "    " : "│   ");

    for (int i = 0; i < node->child_count; i++)
        print_tree_helper(node->children[i], new_prefix,
                         i == node->child_count - 1, 0);
}

void print_tree(NaryTreeNode *root) {
    print_tree_helper(root, "", 1, 1);
}


/* ============================================================================
 *  MAIN — Run all problems
 * ============================================================================ */

int main(void) {
    NaryTreeNode *root = build_example_tree();

    printf("============================================================\n");
    printf("  N-ARY TREE — COMMON INTERVIEW PROBLEMS (C version)\n");
    printf("============================================================\n");

    printf("\n--- Tree ---\n");
    print_tree(root);

    /* Problem 1: Max Depth */
    printf("\n[1] Max Depth (recursive): %d\n", max_depth_recursive(root));
    printf("    Max Depth (BFS):       %d\n", max_depth_bfs(root));

    /* Problem 2: Level Order */
    printf("\n[2] Level Order:\n");
    level_order_print(root);

    /* Problem 3: Encode / Decode */
    printf("\n[3] Encode N-ary -> Binary -> Decode back to N-ary\n");
    BinaryTreeNode *binary = encode_nary_to_binary(root);
    NaryTreeNode *decoded = decode_binary_to_nary(binary);
    printf("    Original: "); level_order_print(root);
    printf("    Decoded:  "); level_order_print(decoded);
    printf("    Match: %s\n", are_identical(root, decoded) ? "Yes" : "No");
    free_binary_tree(binary);
    free_tree(decoded);

    /* Problem 4: Diameter */
    printf("\n[4] Diameter: %d\n", diameter_of_nary_tree(root));

    /* Problem 5: Serialize / Deserialize */
    int buffer[MAX_NODES * 2];
    int buf_size = serialize(root, buffer);
    printf("\n[5] Serialized:   [");
    for (int i = 0; i < buf_size; i++)
        printf("%d%s", buffer[i], i < buf_size - 1 ? ", " : "");
    printf("]\n");
    NaryTreeNode *deserialized = deserialize(buffer, buf_size);
    printf("    Deserialized: "); level_order_print(deserialized);
    printf("    Match: %s\n", are_identical(root, deserialized) ? "Yes" : "No");
    free_tree(deserialized);

    /* Problem 6: Clone */
    NaryTreeNode *cloned = clone_tree(root);
    printf("\n[6] Clone identical: %s\n", are_identical(root, cloned) ? "Yes" : "No");
    printf("    Clone is new obj: %s\n", root != cloned ? "Yes" : "No");
    free_tree(cloned);

    /* Problem 7: All paths */
    printf("\n[7] All root-to-leaf paths:\n");
    all_root_to_leaf_paths(root);

    /* Problem 8: LCA */
    NaryTreeNode *lca;
    lca = lowest_common_ancestor(root, 5, 7);
    printf("\n[8] LCA(5, 7)  = Node(%d)\n", lca ? lca->val : -1);
    lca = lowest_common_ancestor(root, 5, 8);
    printf("    LCA(5, 8)  = Node(%d)\n", lca ? lca->val : -1);
    lca = lowest_common_ancestor(root, 10, 7);
    printf("    LCA(10, 7) = Node(%d)\n", lca ? lca->val : -1);
    lca = lowest_common_ancestor(root, 8, 9);
    printf("    LCA(8, 9)  = Node(%d)\n", lca ? lca->val : -1);

    /* Problem 9: Nodes per level */
    printf("\n[9] Nodes per level:\n");
    nodes_per_level(root);

    /* Problem 10: Identical */
    cloned = clone_tree(root);
    NaryTreeNode *other = create_node(99);
    printf("\n[10] Identical to clone: %s\n", are_identical(root, cloned) ? "Yes" : "No");
    printf("     Identical to other: %s\n", are_identical(root, other) ? "Yes" : "No");
    free_tree(cloned);
    free_tree(other);

    /* Clean up */
    free_tree(root);

    return 0;
}
