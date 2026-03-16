/*
 * ==============================================================================
 *   N-ARY TREE — TIME/SPACE COMPLEXITY & PATTERNS CHEAT SHEET (C version)
 * ==============================================================================
 *
 * This file is your QUICK REFERENCE before interviews.
 *
 *
 * =========================
 *   TIME COMPLEXITY TABLE
 * =========================
 *
 *   Operation / Problem          | Time    | Space   | Technique
 *   -----------------------------|---------|---------|------------------
 *   Traversal (any)              | O(n)    | O(h)*   | DFS or BFS
 *   Max Depth                    | O(n)    | O(h)    | DFS
 *   Level Order                  | O(n)    | O(w)**  | BFS + Queue
 *   Diameter                     | O(n)    | O(h)    | DFS + 2 tallest
 *   Serialize / Deserialize      | O(n)    | O(n)    | Pre-order DFS
 *   Clone                        | O(n)    | O(n)    | DFS
 *   LCA                          | O(n)    | O(h)    | DFS
 *   Root-to-leaf paths           | O(n*h)  | O(n*h)  | DFS + Backtrack
 *   Encode/Decode (Binary<->Nary)| O(n)    | O(n)    | Left-child Right-sibling
 *   Search for a value           | O(n)    | O(h)    | DFS or BFS
 *
 *   * h = height of tree. Worst case h = n (skewed), best case h = log(n)
 *   ** w = maximum width of tree
 *
 *
 * =========================
 *   THE 3 PATTERNS (in C)
 * =========================
 *
 * Every N-ary tree problem uses one (or a combo) of these:
 *
 *
 * PATTERN 1: Simple DFS (Top-Down)
 * ---------------------------------
 * "Pass information FROM parent TO children"
 *
 *     void solve(NaryTreeNode *node, int info_from_parent) {
 *         if (node == NULL) return;
 *         // Use info_from_parent to compute something
 *         for (int i = 0; i < node->child_count; i++) {
 *             solve(node->children[i], updated_info);
 *         }
 *     }
 *
 *     Examples: find depth, root-to-leaf paths, check properties
 *
 *
 * PATTERN 2: Simple DFS (Bottom-Up)
 * ----------------------------------
 * "Collect information FROM children, combine at parent"
 *
 *     int solve(NaryTreeNode *node) {
 *         if (node == NULL) return BASE_VALUE;
 *         int best = INITIAL;
 *         for (int i = 0; i < node->child_count; i++) {
 *             int child_result = solve(node->children[i]);
 *             best = combine(best, child_result);
 *         }
 *         return final_answer(node->val, best);
 *     }
 *
 *     Examples: max depth, height, subtree size, diameter
 *
 *
 * PATTERN 3: BFS (Level-Order)
 * ------------------------------
 * "Process nodes level by level"
 *
 *     void solve(NaryTreeNode *root) {
 *         NaryTreeNode *queue[MAX];
 *         int front = 0, rear = 0;
 *         queue[rear++] = root;
 *
 *         while (front < rear) {
 *             int level_size = rear - front;
 *             for (int i = 0; i < level_size; i++) {
 *                 NaryTreeNode *node = queue[front++];
 *                 // process node
 *                 for (int j = 0; j < node->child_count; j++)
 *                     queue[rear++] = node->children[j];
 *             }
 *         }
 *     }
 *
 *     Examples: level order, min depth, rightmost nodes, widest level
 *
 *
 * =========================
 *   N-ARY vs BINARY TREE
 * =========================
 *
 *   Feature              | Binary Tree              | N-ary Tree
 *   ---------------------|--------------------------|---------------------------
 *   Max children         | 2 (left, right)          | Unlimited
 *   Node structure       | val, left, right         | val, children[], count
 *   In-order traversal   | Yes (left→root→right)    | Not defined (no left/right)
 *   Memory per node      | Fixed (2 pointers)       | Variable (dynamic array)
 *   BST property         | Yes (left < root < right)| No standard equivalent
 *   Height (balanced)    | O(log n)                 | O(log_k n) for k-ary
 *
 *   Converting between them:
 *     N-ary → Binary:  Left-Child Right-Sibling encoding
 *     Binary → N-ary:  Reverse of above
 *
 *
 * =========================
 *   C-SPECIFIC GOTCHAS
 * =========================
 *
 * 1. MEMORY MANAGEMENT:
 *    - Every malloc() MUST have a matching free()
 *    - Free in POST-ORDER (children before parent)
 *    - Use valgrind to detect leaks: valgrind ./a.out
 *
 * 2. DYNAMIC ARRAYS:
 *    - Track both count AND capacity
 *    - Double capacity on overflow (amortized O(1) insertion)
 *    - realloc() can return a DIFFERENT pointer — always reassign!
 *
 * 3. RECURSION DEPTH:
 *    - Default stack size ~1-8MB depending on OS
 *    - Very deep trees (100k+ levels) will SEGFAULT
 *    - Solution: iterative with explicit stack (array-based)
 *
 * 4. NULL CHECKS:
 *    - Always check malloc return for NULL
 *    - Always check node != NULL before accessing ->val
 *
 * 5. PASSING RESULTS:
 *    - C can't return arrays. Options:
 *      a) Pass array + index pointer as parameters
 *      b) Use a global variable (not ideal, but simple)
 *      c) Return a struct containing array + size
 *      d) malloc a result array and return pointer + size via out-param
 *
 *
 * =========================
 *   TWO WAYS TO STORE N-ARY TREE IN C
 * =========================
 *
 *   METHOD A: Dynamic Array of Children (what we used)
 *   ────────────────────────────────────────────────────
 *   typedef struct Node {
 *       int val;
 *       int child_count;
 *       int child_capacity;
 *       struct Node **children;    // malloc'd array of pointers
 *   } Node;
 *
 *   Pros: Direct child access by index O(1), familiar API
 *   Cons: Extra memory for capacity tracking, realloc overhead
 *
 *
 *   METHOD B: Left-Child Right-Sibling (linked list)
 *   ────────────────────────────────────────────────────
 *   typedef struct Node {
 *       int val;
 *       struct Node *first_child;   // my leftmost child
 *       struct Node *next_sibling;  // my next brother/sister
 *   } Node;
 *
 *   Pros: Fixed 2 pointers per node (like binary tree!), no realloc
 *   Cons: No random access to children, must walk sibling chain
 *
 *   This IS the binary encoding from Problem 3!
 *   first_child = left, next_sibling = right.
 *
 *
 * =========================
 *   APPLE-SPECIFIC TIPS
 * =========================
 *
 * 1. UIKit View Hierarchy is an N-ary tree!
 *    - UIView.subviews is the children array
 *    - hitTest() is DFS on the view tree
 *    - layoutSubviews() is bottom-up (post-order)
 *
 * 2. File System is an N-ary tree!
 *    - In C: opendir/readdir/closedir traverses the tree
 *    - "Find all .c files" = DFS with filtering
 *
 * 3. Core Foundation trees (CFTree) use left-child right-sibling!
 *    - CFTreeGetFirstChild, CFTreeGetNextSibling
 *    - This is METHOD B above
 *
 * 4. XPC (inter-process communication) serializes tree structures
 *
 * 5. IOKit device tree is a real N-ary tree in the kernel
 *
 *
 * =========================
 *   INTERVIEW STRATEGIES
 * =========================
 *
 * When you see a tree problem:
 *
 * 1. CLARIFY: Is it binary or N-ary? (They might not say)
 * 2. ASK: Can the tree be empty (NULL root)? Can values repeat?
 * 3. THINK: Which pattern? Top-down, bottom-up, or level-order?
 * 4. START with the recursive solution (simpler to write)
 * 5. MENTION the iterative alternative (shows depth of knowledge)
 * 6. ANALYZE: State time O(n) and space O(h) or O(n)
 * 7. TEST: Walk through your example tree step by step
 * 8. In C: MENTION memory management (when do you free?)
 *
 *
 * =========================
 *   COMMON MISTAKES (C-specific)
 * =========================
 *
 * 1. Forgetting to free the tree (memory leak)
 * 2. Freeing parent before children (dangling pointers, crash)
 * 3. Not checking malloc return value
 * 4. Using array index out of bounds (no bounds checking in C!)
 * 5. Stack overflow on deep recursion
 * 6. Forgetting to update child_count after adding a child
 * 7. realloc returning different pointer — old pointer now invalid
 * 8. Confusing child_count (used slots) with child_capacity (allocated slots)
 */

#include <stdio.h>

int main(void) {
    printf("============================================================\n");
    printf("  N-ARY TREE — PATTERNS & COMPLEXITY CHEAT SHEET (C)\n");
    printf("============================================================\n");
    printf("\n");
    printf("  Read this file's comments for the full reference.\n");
    printf("  Key takeaway: There are only 3 patterns.\n");
    printf("\n");
    printf("  1. DFS Top-Down  -> pass info parent -> child\n");
    printf("  2. DFS Bottom-Up -> collect info child -> parent\n");
    printf("  3. BFS Level     -> process level by level\n");
    printf("\n");
    printf("  Every problem is one of these three. Identify the\n");
    printf("  pattern first, then the code writes itself.\n");
    printf("\n");
    printf("  C bonus patterns to remember:\n");
    printf("  - Always free() in post-order\n");
    printf("  - Track count + capacity for dynamic arrays\n");
    printf("  - Use array + front/rear for queue, array + top for stack\n");
    printf("  - Pass result arrays as parameters (C can't return arrays)\n");

    return 0;
}
