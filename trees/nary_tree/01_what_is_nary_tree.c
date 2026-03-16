/*
 * ==============================================================================
 *   N-ARY TREE — COMPLETE GUIDE (Explained Like You're 5, Then Like You're 25)
 * ==============================================================================
 *
 * WHAT IS A TREE?
 * ===============
 * Imagine your family tree:
 *     - Your grandparent is at the TOP (we call this the "root")
 *     - Your grandparent had children (parents, aunts, uncles)
 *     - Those children had their own children (you, cousins)
 *     - Someone with no children is called a "leaf"
 *
 * That's literally a tree in computer science — flipped upside down.
 *
 *
 * WHAT IS AN N-ARY TREE?
 * ======================
 * "N-ary" just means "each parent can have UP TO N children."
 *
 *     - N = 2  → Binary tree (max 2 kids per parent) — you'll learn this separately
 *     - N = 3  → Ternary tree (max 3 kids)
 *     - N = anything → General tree (any number of kids)
 *
 * When people say "N-ary tree" they usually mean the GENERAL case:
 *     each node can have 0, 1, 2, 3, ... ANY number of children.
 *
 *
 * REAL-WORLD EXAMPLES:
 * ====================
 * 1. FILE SYSTEM on your computer:
 *         /home
 *         ├── user
 *         │   ├── Documents
 *         │   │   ├── resume.pdf
 *         │   │   └── notes.txt
 *         │   ├── Pictures
 *         │   │   └── cat.jpg
 *         │   └── Music
 *         └── admin
 *
 *    "home" has 2 children (user, admin).
 *    "user" has 3 children (Documents, Pictures, Music).
 *    "resume.pdf" has 0 children — it's a leaf.
 *    This is an N-ary tree!
 *
 * 2. COMPANY ORG CHART:
 *         CEO
 *         ├── VP Engineering
 *         │   ├── Team Lead 1
 *         │   │   ├── Dev A
 *         │   │   └── Dev B
 *         │   └── Team Lead 2
 *         │       └── Dev C
 *         ├── VP Marketing
 *         │   └── Designer
 *         └── VP Sales
 *
 * 3. HTML DOM (web pages), JSON, UIKit view hierarchies, etc.
 *
 *
 * VOCABULARY (memorize these):
 * =============================
 *     Root     = the topmost node (no parent)
 *     Parent   = a node that has children
 *     Child    = a node directly below another node
 *     Siblings = nodes that share the same parent
 *     Leaf     = a node with NO children (also called "external node")
 *     Internal = a node that has at least one child
 *     Edge     = the connection between parent and child
 *     Path     = sequence of nodes from one node to another
 *     Depth    = how many edges from root to this node (root depth = 0)
 *     Height   = how many edges from this node to its deepest leaf
 *     Level    = same as depth (Level 0 = root, Level 1 = root's children)
 *     Subtree  = a node and ALL its descendants
 *     Degree   = number of children a node has
 *     Forest   = a collection of trees (remove root → you get a forest)
 *
 *
 * EXAMPLE TREE WE'LL BUILD:
 * =========================
 *
 *               1            ← root (depth=0, height=3)
 *          /  |   \
 *         2    3    4        ← depth=1
 *        /|\       / \
 *       5  6  7   8   9     ← depth=2
 *          |
 *         10                 ← depth=3 (leaf, deepest)
 *
 *
 * HOW C HANDLES THIS vs PYTHON:
 * ==============================
 * In Python:   children = []           (dynamic list, grows automatically)
 * In C:        children = malloc(...)  (YOU manage memory manually)
 *
 * We have TWO choices for storing children in C:
 *
 *   OPTION A: Dynamic array of child pointers
 *       struct Node {
 *           int val;
 *           int child_count;        // how many children
 *           int child_capacity;     // how much space allocated
 *           struct Node **children; // array of pointers to children
 *       };
 *
 *   OPTION B: Linked list of siblings (Left-Child Right-Sibling)
 *       struct Node {
 *           int val;
 *           struct Node *first_child;   // my first child
 *           struct Node *next_sibling;  // my next brother/sister
 *       };
 *
 *   We'll use OPTION A because it maps directly to the Python version
 *   and is more intuitive. Option B is covered in file 03 (encode/decode).
 */

#include <stdio.h>
#include <stdlib.h>

/* ============================================================================
 *  STEP 1: THE NODE STRUCTURE
 * ============================================================================
 *
 *  Each node stores:
 *      1. val            → the data (an integer for now)
 *      2. child_count    → how many children it currently has
 *      3. child_capacity → how much space is allocated for children
 *      4. children       → a dynamically allocated array of child POINTERS
 *
 *  Why pointers? Because children are separate nodes in memory.
 *  children[0] points to first child, children[1] to second, etc.
 *
 *  Memory layout:
 *
 *      Node (val=1)
 *      ┌──────────────────────────┐
 *      │ val = 1                  │
 *      │ child_count = 3          │
 *      │ child_capacity = 4       │
 *      │ children ───────────►[ptr][ptr][ptr][___]
 *      └──────────────────────────┘   │    │    │
 *                                     ▼    ▼    ▼
 *                                   Node  Node  Node
 *                                  (val=2)(val=3)(val=4)
 */

#define INITIAL_CAPACITY 4

typedef struct NaryTreeNode {
    int val;
    int child_count;
    int child_capacity;
    struct NaryTreeNode **children;  /* array of pointers to children */
} NaryTreeNode;


/* ============================================================================
 *  STEP 2: CREATE A NEW NODE
 * ============================================================================ */

NaryTreeNode *create_node(int val) {
    /*
     * malloc = "memory allocate" — asks the OS for a chunk of memory.
     *
     * sizeof(NaryTreeNode) = how many bytes one node needs.
     * We get back a pointer to that memory.
     *
     * ALWAYS check if malloc returned NULL (out of memory).
     */
    NaryTreeNode *node = (NaryTreeNode *)malloc(sizeof(NaryTreeNode));
    if (node == NULL) {
        printf("ERROR: malloc failed!\n");
        exit(1);
    }

    node->val = val;
    node->child_count = 0;
    node->child_capacity = INITIAL_CAPACITY;

    /*
     * Allocate space for INITIAL_CAPACITY child pointers.
     * Each pointer is sizeof(NaryTreeNode *) bytes.
     *
     * calloc zeros out the memory (all pointers start as NULL).
     */
    node->children = (NaryTreeNode **)calloc(INITIAL_CAPACITY,
                                              sizeof(NaryTreeNode *));
    if (node->children == NULL) {
        printf("ERROR: calloc failed!\n");
        free(node);
        exit(1);
    }

    return node;
}


/* ============================================================================
 *  STEP 3: ADD A CHILD TO A NODE
 * ============================================================================ */

void add_child(NaryTreeNode *parent, NaryTreeNode *child) {
    /*
     * If we've used all allocated space, DOUBLE the capacity.
     * This is the same strategy as Python's list or C++'s vector.
     *
     * realloc = "re-allocate" — gives us a bigger chunk of memory,
     * copies old data over, frees old chunk.
     */
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity *= 2;
        parent->children = (NaryTreeNode **)realloc(
            parent->children,
            parent->child_capacity * sizeof(NaryTreeNode *)
        );
        if (parent->children == NULL) {
            printf("ERROR: realloc failed!\n");
            exit(1);
        }
    }

    parent->children[parent->child_count] = child;
    parent->child_count++;
}


/* ============================================================================
 *  STEP 4: FREE THE ENTIRE TREE (prevent memory leaks!)
 * ============================================================================
 *
 *  In C, if you malloc something, you MUST free it.
 *  In Python, garbage collector does this for you.
 *
 *  We use POST-ORDER: free children first, then free the parent.
 *  Why? If you free the parent first, you lose the pointers to children
 *  and can never free them → MEMORY LEAK.
 */

void free_tree(NaryTreeNode *root) {
    if (root == NULL) return;

    /* Free all children first (post-order) */
    for (int i = 0; i < root->child_count; i++) {
        free_tree(root->children[i]);
    }

    /* Free the children array itself */
    free(root->children);

    /* Free this node */
    free(root);
}


/* ============================================================================
 *  STEP 5: BUILD THE EXAMPLE TREE
 * ============================================================================
 *
 *               1
 *          /  |   \
 *         2    3    4
 *        /|\       / \
 *       5  6  7   8   9
 *          |
 *         10
 */

NaryTreeNode *build_example_tree(void) {
    /* Create all nodes */
    NaryTreeNode *node1  = create_node(1);
    NaryTreeNode *node2  = create_node(2);
    NaryTreeNode *node3  = create_node(3);
    NaryTreeNode *node4  = create_node(4);
    NaryTreeNode *node5  = create_node(5);
    NaryTreeNode *node6  = create_node(6);
    NaryTreeNode *node7  = create_node(7);
    NaryTreeNode *node8  = create_node(8);
    NaryTreeNode *node9  = create_node(9);
    NaryTreeNode *node10 = create_node(10);

    /* Wire up the relationships */
    add_child(node1, node2);     /* 1 → [2, 3, 4] */
    add_child(node1, node3);
    add_child(node1, node4);

    add_child(node2, node5);     /* 2 → [5, 6, 7] */
    add_child(node2, node6);
    add_child(node2, node7);

    add_child(node6, node10);    /* 6 → [10] */

    add_child(node4, node8);     /* 4 → [8, 9] */
    add_child(node4, node9);

    return node1;
}


/* ============================================================================
 *  STEP 6: BASIC OPERATIONS
 * ============================================================================ */

/*
 * Count total nodes in the tree.
 *
 * Logic: 1 (for myself) + count of all nodes in each child's subtree
 * This is RECURSION:
 *     Base case: if root is NULL, return 0
 *     Recursive case: 1 + sum of count_nodes(child) for each child
 */
int count_nodes(NaryTreeNode *root) {
    if (root == NULL) return 0;

    int total = 1;  /* count myself */
    for (int i = 0; i < root->child_count; i++) {
        total += count_nodes(root->children[i]);
    }
    return total;
}


/*
 * Height = longest path from this node to any leaf below it.
 * A leaf has height 0. An empty tree has height -1.
 */
int find_height(NaryTreeNode *root) {
    if (root == NULL) return -1;
    if (root->child_count == 0) return 0;  /* leaf */

    int max_child_height = 0;
    for (int i = 0; i < root->child_count; i++) {
        int h = find_height(root->children[i]);
        if (h > max_child_height) {
            max_child_height = h;
        }
    }
    return 1 + max_child_height;
}


/*
 * Collect all leaf node values into an array.
 * Returns the count of leaves found.
 *
 * In C, we can't return a dynamic list easily, so we pass an array
 * and an index pointer to fill in.
 */
void find_leaves(NaryTreeNode *root, int *leaves, int *count) {
    if (root == NULL) return;

    if (root->child_count == 0) {
        leaves[*count] = root->val;
        (*count)++;
        return;
    }

    for (int i = 0; i < root->child_count; i++) {
        find_leaves(root->children[i], leaves, count);
    }
}


/*
 * Find the depth of a node with a specific value.
 * Depth = number of edges from root to this node.
 * Returns -1 if not found.
 */
int find_depth(NaryTreeNode *root, int target, int current_depth) {
    if (root == NULL) return -1;
    if (root->val == target) return current_depth;

    for (int i = 0; i < root->child_count; i++) {
        int result = find_depth(root->children[i], target, current_depth + 1);
        if (result != -1) return result;
    }
    return -1;
}


/* ============================================================================
 *  STEP 7: PRINT THE TREE (like Linux `tree` command)
 * ============================================================================ */

void print_tree_helper(NaryTreeNode *node, char *prefix, int is_last, int is_root) {
    if (node == NULL) return;

    if (is_root) {
        printf("%d\n", node->val);
    } else {
        printf("%s%s%d\n", prefix, is_last ? "└── " : "├── ", node->val);
    }

    /* Build new prefix for children */
    char new_prefix[256];
    if (is_root) {
        new_prefix[0] = '\0';  /* root has no prefix */
    } else {
        snprintf(new_prefix, sizeof(new_prefix), "%s%s",
                 prefix, is_last ? "    " : "│   ");
    }

    for (int i = 0; i < node->child_count; i++) {
        int child_is_last = (i == node->child_count - 1);
        print_tree_helper(node->children[i], new_prefix, child_is_last, 0);
    }
}

void print_tree(NaryTreeNode *root) {
    print_tree_helper(root, "", 1, 1);
}


/* ============================================================================
 *  MAIN — Run everything
 * ============================================================================ */

int main(void) {
    printf("============================================================\n");
    printf("  N-ARY TREE — Building & Basic Operations (C version)\n");
    printf("============================================================\n");

    NaryTreeNode *root = build_example_tree();

    printf("\n--- Tree Structure ---\n");
    print_tree(root);

    printf("\n--- Basic Stats ---\n");
    printf("  Total nodes:  %d\n", count_nodes(root));
    printf("  Tree height:  %d\n", find_height(root));

    /* Find leaves */
    int leaves[20];
    int leaf_count = 0;
    find_leaves(root, leaves, &leaf_count);
    printf("  Leaf nodes:   [");
    for (int i = 0; i < leaf_count; i++) {
        printf("%d%s", leaves[i], i < leaf_count - 1 ? ", " : "");
    }
    printf("]\n");

    printf("  Depth of 10:  %d\n", find_depth(root, 10, 0));
    printf("  Depth of 1:   %d\n", find_depth(root, 1, 0));
    printf("  Depth of 4:   %d\n", find_depth(root, 4, 0));

    printf("\n--- Key Insight ---\n");
    printf("  Every recursive function on an N-ary tree follows the SAME pattern:\n");
    printf("    1. Handle base case (NULL or leaf)\n");
    printf("    2. Process current node\n");
    printf("    3. Loop through children[0..child_count-1] and recurse\n");
    printf("  That's it. Master this pattern and you can solve ANY tree problem.\n");

    printf("\n--- C-Specific Insight ---\n");
    printf("  In C, you MUST also:\n");
    printf("    4. malloc() when creating nodes\n");
    printf("    5. free() when destroying the tree (post-order!)\n");
    printf("    6. Handle realloc() when children array grows\n");
    printf("  Forgetting free → memory leak. Freeing too early → crash.\n");

    /* Clean up! */
    free_tree(root);

    return 0;
}
