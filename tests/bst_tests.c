#include "bst_tests.h"

/* ============================ counters & macro ============================ */

static int bst_passed = 0;
static int bst_failed = 0;

#define BST_EXPECT(cond, msg)                                                      \
    do {                                                                           \
        if ((cond)) {                                                              \
            bst_passed++;                                                          \
        } else {                                                                   \
            bst_failed++;                                                          \
            fprintf(stderr, "[BST FAIL] %s:%d: %s\n", __FILE__, __LINE__, (msg));  \
        }                                                                          \
    } while (0)

/* --- stderr silencer implementation (test-only) --- */
static int bst_saved_stderr_fd = -1;

void bst_silence_stderr_begin(void) {
    // Flush current stderr buffer so nothing remains in-flight
    fflush(stderr);

    // Duplicate the current stderr FD so we can restore it later
    bst_saved_stderr_fd = bst_dup(bst_fileno(stderr));
    if (bst_saved_stderr_fd < 0) {
        // If we cannot dup, fail open (do nothing)
        return;
    }

    // Open the null device and redirect stderr there
    int devnull = bst_open(BST_DEV_NULL, BST_O_WRONLY);
    if (devnull >= 0) {
        bst_dup2(devnull, bst_fileno(stderr)); // stderr -> /dev/null or NUL
        bst_close(devnull);
    }
}

void bst_silence_stderr_end(void) {
    if (bst_saved_stderr_fd >= 0) {
        // Flush any buffered stderr pointing to /dev/null (harmless)
        fflush(stderr);

        // Restore the original stderr
        bst_dup2(bst_saved_stderr_fd, bst_fileno(stderr));
        bst_close(bst_saved_stderr_fd);
        bst_saved_stderr_fd = -1;
    }
}

/* ============================ helpers for data ============================ */

/* Free-counter: increments a global each time it frees a payload. */
static int g_bst_data_free_count = 0;
static void bst_data_free_counter(void* p) {
    if (p) {
        g_bst_data_free_count++;
        free(p);
    }
}

/* Allocates an int on heap and initializes it to v. */
static int* make_int(int v) {
    int* p = (int*)malloc(sizeof(int));
    if (!p) { fprintf(stderr, "make_int: malloc failed\n"); exit(99); }
    *p = v;
    return p;
}

/* Comparator for int payloads stored as int* (ownership by BST). */
static int int_ptr_compare(const void* a, const void* b) {
    const int A = *(const int*)a;
    const int B = *(const int*)b;
    if (A < B) return -1;
    if (A > B) return 1;
    return 0;
}

/* In-order traversal to collect values into array (for assertions). */
static void inorder_collect(BinarySearchTreeNode* n, int* out, size_t* idx) {
    if (!n || n->data == NULL) return; // skip sentinel/empty
    inorder_collect(n->left, out, idx);
    out[(*idx)++] = *(int*)n->data;
    inorder_collect(n->right, out, idx);
}

/* Computes height (edges count on longest path). Sentinel root (data==NULL) -> -1; single node -> 0 */
static int bst_height(BinarySearchTreeNode* n) {
    if (!n) return -1;
    if (n->data == NULL) return -1; // empty sentinel
    int lh = bst_height(n->left);
    int rh = bst_height(n->right);
    return (lh > rh ? lh : rh) + 1;
}

/* Finds node pointer for value v using contains. */
static BinarySearchTreeNode* find_node(BinarySearchTree t, int v) {
    int key = v; // stack int used by comparator (no ownership transfer)
    return bin_search_tree_contains(t, &key, int_ptr_compare);
}

/* =============================== test cases =============================== */

static void test_build_empty_sentinel(void) {
    BinarySearchTree t = bin_search_tree_build_empty();
    BST_EXPECT(t != NULL, "build_empty must return non-NULL");
    BST_EXPECT(t->data == NULL, "empty root must have data==NULL");
    BST_EXPECT(t->left == NULL && t->right == NULL, "empty root must have no children");
    binary_search_tree_destroy(t, NULL); // payloads none
}

static void test_insert_and_contains_basic(void) {
    BinarySearchTree t = bin_search_tree_build_empty();

    /* Insert 5 unique ints (ownership transfer: do NOT free these here). */
    int* p10 = make_int(10);
    int* p5  = make_int(5);
    int* p15 = make_int(15);
    int* p2  = make_int(2);
    int* p7  = make_int(7);

    BST_EXPECT(bin_search_tree_insert_node(t, p10, sizeof(int), int_ptr_compare) != NULL, "insert 10");
    BST_EXPECT(bin_search_tree_insert_node(t, p5,  sizeof(int), int_ptr_compare) != NULL, "insert 5");
    BST_EXPECT(bin_search_tree_insert_node(t, p15, sizeof(int), int_ptr_compare) != NULL, "insert 15");
    BST_EXPECT(bin_search_tree_insert_node(t, p2,  sizeof(int), int_ptr_compare) != NULL, "insert 2");
    BST_EXPECT(bin_search_tree_insert_node(t, p7,  sizeof(int), int_ptr_compare) != NULL, "insert 7");

    /* contains must find them */
    BST_EXPECT(find_node(t, 10) != NULL, "contains(10)");
    BST_EXPECT(find_node(t, 5)  != NULL, "contains(5)");
    BST_EXPECT(find_node(t, 15) != NULL, "contains(15)");
    BST_EXPECT(find_node(t, 2)  != NULL, "contains(2)");
    BST_EXPECT(find_node(t, 7)  != NULL, "contains(7)");
    BST_EXPECT(find_node(t, 99) == NULL, "contains(99) must be NULL");

    /* duplicate insert: BST should NOT adopt the new pointer; caller must free it */
    int* dup5 = make_int(5);
    BinarySearchTreeNode* before = find_node(t, 5);
    BinarySearchTreeNode* ret    = bin_search_tree_insert_node(t, dup5, sizeof(int), int_ptr_compare);
    BST_EXPECT(ret == before, "duplicate insert returns existing node");
    free(dup5); /* caller frees duplicate attempt (not adopted) */

    /* destroy → frees exactly 5 owned payloads */
    g_bst_data_free_count = 0;
    binary_search_tree_destroy(t, bst_data_free_counter);
    BST_EXPECT(g_bst_data_free_count == 5, "destroy must free exactly 5 payloads");
}

static void test_find_min_max(void) {
    BinarySearchTree t = bin_search_tree_build_empty();
    (void)bin_search_tree_insert_node(t, make_int(50), sizeof(int), int_ptr_compare);
    (void)bin_search_tree_insert_node(t, make_int(20), sizeof(int), int_ptr_compare);
    (void)bin_search_tree_insert_node(t, make_int(70), sizeof(int), int_ptr_compare);
    (void)bin_search_tree_insert_node(t, make_int(10), sizeof(int), int_ptr_compare);
    (void)bin_search_tree_insert_node(t, make_int(30), sizeof(int), int_ptr_compare);

    BinarySearchTreeNode* mn = bin_search_tree_find_min(t);
    BinarySearchTreeNode* mx = bin_search_tree_find_max(t);
    BST_EXPECT(*(int*)mn->data == 10, "min must be 10");
    BST_EXPECT(*(int*)mx->data == 70, "max must be 70");

    g_bst_data_free_count = 0;
    binary_search_tree_destroy(t, bst_data_free_counter);
    BST_EXPECT(g_bst_data_free_count == 5, "destroy frees all 5 payloads");
}

static void test_delete_leaf(void) {
    BinarySearchTree t = bin_search_tree_build_empty();
    (void)bin_search_tree_insert_node(t, make_int(2), sizeof(int), int_ptr_compare);
    (void)bin_search_tree_insert_node(t, make_int(1), sizeof(int), int_ptr_compare); // leaf
    (void)bin_search_tree_insert_node(t, make_int(3), sizeof(int), int_ptr_compare);

    g_bst_data_free_count = 0;
    bin_search_tree_delete_node(t, &(int){1}, int_ptr_compare, bst_data_free_counter);
    BST_EXPECT(find_node(t, 1) == NULL, "leaf 1 must be deleted");
    BST_EXPECT(g_bst_data_free_count == 1, "exactly one payload free on leaf delete");

    /* clean up remaining (2,3) */
    binary_search_tree_destroy(t, bst_data_free_counter);
    BST_EXPECT(g_bst_data_free_count == 3, "destroy frees remaining 2 payloads (total 3)");
}

static void test_delete_one_child_non_root(void) {
    /* Build: 4 -> left 2 -> left 1 (so node 2 has exactly one child). */
    BinarySearchTree t = bin_search_tree_build_empty();
    (void)bin_search_tree_insert_node(t, make_int(4), sizeof(int), int_ptr_compare);
    (void)bin_search_tree_insert_node(t, make_int(2), sizeof(int), int_ptr_compare);
    (void)bin_search_tree_insert_node(t, make_int(1), sizeof(int), int_ptr_compare);

    g_bst_data_free_count = 0;
    bin_search_tree_delete_node(t, &(int){2}, int_ptr_compare, bst_data_free_counter);
    BST_EXPECT(find_node(t, 2) == NULL, "node 2 deleted");
    BST_EXPECT(find_node(t, 1) != NULL && find_node(t, 4) != NULL, "nephew and root survive");
    BST_EXPECT(g_bst_data_free_count == 1, "payload of deleted node freed once");

    binary_search_tree_destroy(t, bst_data_free_counter);
    BST_EXPECT(g_bst_data_free_count == 3, "total frees: deleted(1) + destroy(2)");
}

static void test_delete_one_child_root(void) {
    /* Build: root=2, left=1 only; then delete 2 (root with one child). */
    BinarySearchTree t = bin_search_tree_build_empty();
    int* p2 = make_int(2);
    int* p1 = make_int(1);
    (void)bin_search_tree_insert_node(t, p2, sizeof(int), int_ptr_compare);
    (void)bin_search_tree_insert_node(t, p1, sizeof(int), int_ptr_compare);

    BinarySearchTreeNode* root_before = t;

    g_bst_data_free_count = 0;
    bin_search_tree_delete_node(t, &(int){2}, int_ptr_compare, bst_data_free_counter);

    /* Root pointer must be stable; root payload now must be 1 (adopted from child) */
    BST_EXPECT(t == root_before, "root pointer must remain the same");
    BST_EXPECT(*(int*)t->data == 1, "root payload must now be 1");
    BST_EXPECT(find_node(t, 2) == NULL && find_node(t, 1) != NULL, "2 gone, 1 remains");
    BST_EXPECT(g_bst_data_free_count == 1, "exactly one payload freed (the old root payload)");

    binary_search_tree_destroy(t, bst_data_free_counter);
    BST_EXPECT(g_bst_data_free_count == 2, "destroy frees remaining payload");
}

static void test_delete_two_children_root(void) {
    /* Build a classic BST: 5,3,7,2,4,6,8 then delete root(5). */
    BinarySearchTree t = bin_search_tree_build_empty();

    int* p5 = make_int(5);
    int* p3 = make_int(3);
    int* p7 = make_int(7);
    int* p2 = make_int(2);
    int* p4 = make_int(4);
    int* p6 = make_int(6);
    int* p8 = make_int(8);

    (void)bin_search_tree_insert_node(t, p5, sizeof(int), int_ptr_compare);
    (void)bin_search_tree_insert_node(t, p3, sizeof(int), int_ptr_compare);
    (void)bin_search_tree_insert_node(t, p7, sizeof(int), int_ptr_compare);
    (void)bin_search_tree_insert_node(t, p2, sizeof(int), int_ptr_compare);
    (void)bin_search_tree_insert_node(t, p4, sizeof(int), int_ptr_compare);
    (void)bin_search_tree_insert_node(t, p6, sizeof(int), int_ptr_compare);
    (void)bin_search_tree_insert_node(t, p8, sizeof(int), int_ptr_compare);

    BinarySearchTreeNode* root_before = t;

    /* Delete node with two children (the root). Under swap semantics:
       - successor is 6; one payload is freed (the old root payload after swap).
       - root pointer stays the same; root payload becomes 6. */
    g_bst_data_free_count = 0;
    bin_search_tree_delete_node(t, &(int){5}, int_ptr_compare, bst_data_free_counter);

    BST_EXPECT(t == root_before, "root pointer remains stable after deleting root with two children");
    BST_EXPECT(*(int*)t->data == 6, "root payload must now be 6 (successor)");
    BST_EXPECT(find_node(t, 5) == NULL, "5 must be gone");
    BST_EXPECT(find_node(t, 2) && find_node(t, 3) && find_node(t, 4), "left subtree intact");
    BST_EXPECT(find_node(t, 7) && find_node(t, 8), "right subtree intact (except successor relocated)");
    BST_EXPECT(g_bst_data_free_count == 1, "exactly one payload freed via swap strategy");

    binary_search_tree_destroy(t, bst_data_free_counter);
    BST_EXPECT(g_bst_data_free_count == 7, "destroy frees remaining 7 payloads");
}

static void test_rebalance_reduces_height_and_preserves_order(void) {
    /* Insert ascending to create a skewed tree, then rebalance. */
    BinarySearchTree t = bin_search_tree_build_empty();

    const int N = 15; // odd count gives a nice center
    for (int i = 1; i <= N; ++i) {
        (void)bin_search_tree_insert_node(t, make_int(i), sizeof(int), int_ptr_compare);
    }

    int h_before = bst_height(t);

    // collect inorder before (should be 1..N)
    int* arr_before = (int*)malloc(sizeof(int) * (size_t)N);
    size_t idx = 0;
    inorder_collect(t, arr_before, &idx);
    BST_EXPECT((int)idx == N, "inorder count before rebalance must be N");
    for (int i = 0; i < N; ++i) {
        BST_EXPECT(arr_before[i] == (i + 1), "inorder sequence before rebalance must be sorted 1..N");
    }

    // rebalance
    bin_search_tree_rebalance(t); // shape-only; payloads preserved

    int h_after = bst_height(t);
    BST_EXPECT(h_after <= h_before, "height must not increase after rebalance");
    BST_EXPECT(h_after <= 4, "height should be close to log2(15) ≈ 3.9 (<=4)"); // sanity bound

    // collect inorder after
    int* arr_after = (int*)malloc(sizeof(int) * (size_t)N);
    idx = 0;
    inorder_collect(t, arr_after, &idx);
    BST_EXPECT((int)idx == N, "inorder count after rebalance must be N");
    for (int i = 0; i < N; ++i) {
        BST_EXPECT(arr_after[i] == (i + 1), "inorder sequence after rebalance must remain 1..N");
    }

    // pretty print as smoke test (does not assert)
    bin_search_tree_pretty_print(t, NULL);

    // cleanup
    free(arr_before);
    free(arr_after);
    g_bst_data_free_count = 0;
    binary_search_tree_destroy(t, bst_data_free_counter);
    BST_EXPECT(g_bst_data_free_count == N, "destroy frees all N payloads after rebalance");
}

/* ================================ entry point ================================ */

void run_all_bst_tests(void) {
    bst_passed = 0;
    bst_failed = 0;
    printf("[TEST] testing binary search tree...\n");

    // bst_silence_stderr_begin();
    // test_destroy_null_is_noop();
    // test_delete_on_empty_tree_is_noop();
    // test_pretty_print_on_empty_tree();
    // bst_silence_stderr_end();
    
    bst_silence_stderr_begin();
    test_build_empty_sentinel();
    test_insert_and_contains_basic();
    test_delete_one_child_non_root();
    test_delete_one_child_root();
    test_delete_two_children_root();
    test_rebalance_reduces_height_and_preserves_order();
    bst_silence_stderr_end();

    if (bst_failed == 0) {
        printf("[TEST OK]  bst: passed=%d failed=%d\n", bst_passed, bst_failed);
    } else {
        printf("[TEST FAIL] bst: passed=%d failed=%d\n", bst_passed, bst_failed);
    }
}
