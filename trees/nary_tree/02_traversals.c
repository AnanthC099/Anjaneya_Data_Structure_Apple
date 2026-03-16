/*
 * ==============================================================================
 *   N-ARY TREE TRAVERSALS — The 4 Ways to Visit Every Node (C version)
 * ==============================================================================
 *
 * "Traversal" = visiting every node in the tree exactly once, in some order.
 *
 * THE BIG PICTURE:
 * ================
 *
 *     DFS (Depth-First Search) — go DEEP before going WIDE
 *         1. Pre-order:   Visit node FIRST, then children (left to right)
 *         2. Post-order:  Visit children FIRST, then node
 *
 *     BFS (Breadth-First Search) — go WIDE before going DEEP
 *         3. Level-order: Visit ALL nodes at depth 0, then depth 1, then 2...
 *
 *     Bonus:
 *         4. Reverse Level-order: Level-order but bottom to top
 *
 *
 * WHEN TO USE WHICH (Apple interview cheat sheet):
 * ================================================
 *     Pre-order:   Copy a tree, serialize/deserialize, prefix expressions
 *     Post-order:  Delete a tree, calculate sizes, evaluate expressions
 *     Level-order: Find depth, shortest path, print level by level
 *
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
 *
 *
 * C-SPECIFIC NOTES:
 * =================
 * In Python, we just append to a list. In C, we need:
 *   - An array + index to collect results
 *   - A manual stack (array-based) for iterative DFS
 *   - A manual queue (array-based) for BFS
 *
 * This is MORE work but teaches you what's ACTUALLY happening under the hood.
 */

#include <stdio.h>
#include <stdlib.h>

#define INITIAL_CAPACITY 4
#define MAX_NODES 100   /* max nodes for result arrays and stack/queue */


/* ============================================================================
 *  NODE STRUCTURE (same as file 01)
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
 *  HELPER: Print an array
 * ============================================================================ */

void print_array(const char *label, int *arr, int size) {
    printf("  %s [", label);
    for (int i = 0; i < size; i++) {
        printf("%d%s", arr[i], i < size - 1 ? ", " : "");
    }
    printf("]\n");
}


/* ============================================================================
 *  TRAVERSAL 1: PRE-ORDER (Root → Children) — RECURSIVE
 * ============================================================================
 *
 *  "Pre" means "before" — visit the node BEFORE its children.
 *  Think of it as: "I introduce myself first, then introduce my kids."
 *
 *  For our tree:  1, 2, 5, 6, 10, 7, 3, 4, 8, 9
 *
 *  Time:  O(n) — visit every node once
 *  Space: O(h) — recursion stack = height of tree
 */

void preorder_recursive_helper(NaryTreeNode *node, int *result, int *idx) {
    if (node == NULL) return;

    result[(*idx)++] = node->val;            /* VISIT node FIRST (pre) */

    for (int i = 0; i < node->child_count; i++) {
        preorder_recursive_helper(node->children[i], result, idx);
    }
}

int preorder_recursive(NaryTreeNode *root, int *result) {
    int count = 0;
    preorder_recursive_helper(root, result, &count);
    return count;  /* return how many elements written */
}


/* ============================================================================
 *  TRAVERSAL 1b: PRE-ORDER — ITERATIVE (using a stack)
 * ============================================================================
 *
 *  Why learn this?
 *  1. Interviewers ask "can you do it without recursion?"
 *  2. Recursion can cause stack overflow on very deep trees
 *  3. It shows you understand what recursion is actually doing
 *
 *  Key trick: Push children in REVERSE order so leftmost is processed first.
 *  Stack = LIFO (Last In, First Out)
 *
 *  Time:  O(n)
 *  Space: O(n) — stack can hold up to n nodes in worst case
 */

int preorder_iterative(NaryTreeNode *root, int *result) {
    if (root == NULL) return 0;

    /*
     * We build a simple stack using an array.
     * stack[top] is the top element. top = -1 means empty.
     */
    NaryTreeNode *stack[MAX_NODES];
    int top = -1;
    int count = 0;

    /* Push root */
    stack[++top] = root;

    while (top >= 0) {
        /* Pop */
        NaryTreeNode *node = stack[top--];
        result[count++] = node->val;

        /* Push children in REVERSE order (so leftmost is on top) */
        for (int i = node->child_count - 1; i >= 0; i--) {
            stack[++top] = node->children[i];
        }
    }

    return count;
}


/* ============================================================================
 *  TRAVERSAL 2: POST-ORDER (Children → Root) — RECURSIVE
 * ============================================================================
 *
 *  "Post" means "after" — visit the node AFTER all its children.
 *  Think of it as: "My kids introduce themselves first, then I go."
 *
 *  For our tree:  5, 10, 6, 7, 2, 3, 8, 9, 4, 1
 *
 *  Why useful?
 *     - Deleting a tree (free children before parent — we did this!)
 *     - Calculating subtree sizes (need children's sizes first)
 *     - Evaluating expression trees
 *
 *  Time:  O(n)
 *  Space: O(h)
 */

void postorder_recursive_helper(NaryTreeNode *node, int *result, int *idx) {
    if (node == NULL) return;

    for (int i = 0; i < node->child_count; i++) {   /* children FIRST */
        postorder_recursive_helper(node->children[i], result, idx);
    }

    result[(*idx)++] = node->val;                    /* THEN visit node */
}

int postorder_recursive(NaryTreeNode *root, int *result) {
    int count = 0;
    postorder_recursive_helper(root, result, &count);
    return count;
}


/* ============================================================================
 *  TRAVERSAL 2b: POST-ORDER — ITERATIVE
 * ============================================================================
 *
 *  Clever hack:
 *      Pre-order  = Root, Child1, Child2, Child3
 *      Modified   = Root, Child3, Child2, Child1  (children NOT reversed)
 *      Reversed   = Child1, Child2, Child3, Root  ← that's post-order!
 *
 *  So: do pre-order but push children left-to-right (not reversed),
 *  then reverse the final result.
 *
 *  Time:  O(n)
 *  Space: O(n)
 */

int postorder_iterative(NaryTreeNode *root, int *result) {
    if (root == NULL) return 0;

    NaryTreeNode *stack[MAX_NODES];
    int top = -1;
    int count = 0;

    stack[++top] = root;

    while (top >= 0) {
        NaryTreeNode *node = stack[top--];
        result[count++] = node->val;

        /* Push children LEFT to RIGHT (opposite of pre-order!) */
        for (int i = 0; i < node->child_count; i++) {
            stack[++top] = node->children[i];
        }
    }

    /* Reverse the result array */
    for (int i = 0; i < count / 2; i++) {
        int temp = result[i];
        result[i] = result[count - 1 - i];
        result[count - 1 - i] = temp;
    }

    return count;
}


/* ============================================================================
 *  TRAVERSAL 3: LEVEL-ORDER / BFS (Level by Level)
 * ============================================================================
 *
 *  Visit all nodes at depth 0, then depth 1, then depth 2, ...
 *
 *  For our tree:
 *      Level 0: [1]
 *      Level 1: [2, 3, 4]
 *      Level 2: [5, 6, 7, 8, 9]
 *      Level 3: [10]
 *
 *  Uses a QUEUE (FIFO = First In, First Out) — NOT a stack!
 *
 *  In C, we implement the queue with an array and front/rear indices.
 *
 *  THIS IS THE MOST ASKED TRAVERSAL IN INTERVIEWS.
 *
 *  Time:  O(n)
 *  Space: O(w) where w = max width of tree
 */

/* Level-order returning grouped levels */
typedef struct {
    int levels[MAX_NODES][MAX_NODES];   /* levels[i] = values at level i */
    int level_sizes[MAX_NODES];         /* how many nodes at each level */
    int num_levels;                     /* total number of levels */
} LevelOrderResult;

LevelOrderResult level_order(NaryTreeNode *root) {
    LevelOrderResult res;
    res.num_levels = 0;

    if (root == NULL) return res;

    /*
     * Queue: array of node pointers.
     * front = index of next element to dequeue.
     * rear  = index of next free slot to enqueue.
     */
    NaryTreeNode *queue[MAX_NODES];
    int front = 0, rear = 0;

    /* Enqueue root */
    queue[rear++] = root;

    while (front < rear) {
        int level_size = rear - front;  /* nodes at current level */
        int level_idx = res.num_levels;
        res.level_sizes[level_idx] = 0;

        for (int i = 0; i < level_size; i++) {
            /* Dequeue */
            NaryTreeNode *node = queue[front++];

            /* Record value */
            res.levels[level_idx][res.level_sizes[level_idx]++] = node->val;

            /* Enqueue all children */
            for (int j = 0; j < node->child_count; j++) {
                queue[rear++] = node->children[j];
            }
        }

        res.num_levels++;
    }

    return res;
}


/* Level-order flat (simple BFS) */
int level_order_flat(NaryTreeNode *root, int *result) {
    if (root == NULL) return 0;

    NaryTreeNode *queue[MAX_NODES];
    int front = 0, rear = 0;
    int count = 0;

    queue[rear++] = root;

    while (front < rear) {
        NaryTreeNode *node = queue[front++];
        result[count++] = node->val;

        for (int i = 0; i < node->child_count; i++) {
            queue[rear++] = node->children[i];
        }
    }

    return count;
}


/* ============================================================================
 *  TRAVERSAL 4: REVERSE LEVEL-ORDER (Bottom to Top)
 * ============================================================================ */

void print_level_order(LevelOrderResult *res, int reverse) {
    if (reverse) {
        for (int l = res->num_levels - 1; l >= 0; l--) {
            printf("    Level %d: [", l);
            for (int i = 0; i < res->level_sizes[l]; i++) {
                printf("%d%s", res->levels[l][i],
                       i < res->level_sizes[l] - 1 ? ", " : "");
            }
            printf("]\n");
        }
    } else {
        for (int l = 0; l < res->num_levels; l++) {
            printf("    Level %d: [", l);
            for (int i = 0; i < res->level_sizes[l]; i++) {
                printf("%d%s", res->levels[l][i],
                       i < res->level_sizes[l] - 1 ? ", " : "");
            }
            printf("]\n");
        }
    }
}


/* ============================================================================
 *  PRETTY PRINT TREE
 * ============================================================================ */

void print_tree_helper(NaryTreeNode *node, char *prefix, int is_last, int is_root) {
    if (!node) return;
    if (is_root)
        printf("%d\n", node->val);
    else
        printf("%s%s%d\n", prefix, is_last ? "└── " : "├── ", node->val);

    char new_prefix[256];
    if (is_root)
        new_prefix[0] = '\0';
    else
        snprintf(new_prefix, sizeof(new_prefix), "%s%s",
                 prefix, is_last ? "    " : "│   ");

    for (int i = 0; i < node->child_count; i++)
        print_tree_helper(node->children[i], new_prefix,
                         i == node->child_count - 1, 0);
}

void print_tree(NaryTreeNode *root) {
    print_tree_helper(root, "", 1, 1);
}


/* ============================================================================
 *  MAIN — Run all traversals
 * ============================================================================ */

int main(void) {
    NaryTreeNode *root = build_example_tree();
    int result[MAX_NODES];
    int count;

    printf("============================================================\n");
    printf("  N-ARY TREE TRAVERSALS (C version)\n");
    printf("============================================================\n");

    printf("\n--- Tree ---\n");
    print_tree(root);

    /* Pre-order */
    printf("\n--- Pre-order (Root first, then children) ---\n");
    count = preorder_recursive(root, result);
    print_array("Recursive:", result, count);
    count = preorder_iterative(root, result);
    print_array("Iterative:", result, count);

    /* Post-order */
    printf("\n--- Post-order (Children first, then root) ---\n");
    count = postorder_recursive(root, result);
    print_array("Recursive:", result, count);
    count = postorder_iterative(root, result);
    print_array("Iterative:", result, count);

    /* Level-order */
    printf("\n--- Level-order / BFS (Level by level, top to bottom) ---\n");
    LevelOrderResult lor = level_order(root);
    print_level_order(&lor, 0);

    count = level_order_flat(root, result);
    print_array("Flat:     ", result, count);

    /* Reverse level-order */
    printf("\n--- Reverse Level-order (Bottom to top) ---\n");
    print_level_order(&lor, 1);

    /* Memory aids */
    printf("\n--- Memory Aid ---\n");
    printf("  PRE  = I go first, kids follow     -> Root, C1, C2, C3\n");
    printf("  POST = Kids go first, I go last    -> C1, C2, C3, Root\n");
    printf("  BFS  = Everyone at my depth first  -> Level 0, 1, 2, 3\n\n");
    printf("  DFS uses STACK  (recursion or explicit array)\n");
    printf("  BFS uses QUEUE  (array with front/rear pointers)\n");

    printf("\n--- C vs Python ---\n");
    printf("  Python: result.append(val)        → dynamic list\n");
    printf("  C:      result[count++] = val     → manual index tracking\n");
    printf("  Python: deque([root])             → built-in queue\n");
    printf("  C:      array + front/rear index  → manual queue\n");
    printf("  Python: [root]                    → built-in stack\n");
    printf("  C:      array + top index         → manual stack\n");

    free_tree(root);
    return 0;
}
