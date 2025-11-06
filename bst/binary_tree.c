#include <stdlib.h>
#include <string.h>
#include "binary_tree.h"

/*
 * BST implementation — ownership transfer + swap (no byte copying)
 *
 * - Insert takes ownership of the heap pointer passed by the caller.
 * - Internal operations move payloads by swapping node->data / node->data_size.
 * - The root pointer is never replaced; an empty tree is a sentinel root.
 * - deep_free is optional; if NULL, plain free() is used to release payloads.
 */

/* =============================== internal helpers =============================== */
static size_t bst_count_nodes(BinarySearchTreeNode* n);
static void   bst_fill_nodes_inorder(BinarySearchTreeNode* n, BinarySearchTreeNode** arr, size_t* idx);
static BinarySearchTreeNode* bst_link_balanced(BinarySearchTreeNode** arr, long lo, long hi);
static inline void bst_swap_payload(BinarySearchTreeNode* a, BinarySearchTreeNode* b);
static void   bst_default_preview(const void* data, size_t size);
static void   bst_print_node_line(BinarySearchTreeNode* n, void (*print_data)(const void*, size_t));
static void   bst_print_rec(BinarySearchTreeNode* n, const char* prefix, int is_right, void (*print_data)(const void*, size_t));

/* =============================== constructors/lookup =============================== */

BinarySearchTreeNode* bin_search_tree_build_empty(){
    BinarySearchTreeNode* tree = malloc(sizeof(BinarySearchTreeNode));
    if (tree == NULL){
        fprintf(stderr, "Failed bin_search_tree_build_empty: malloc failed\n");
        exit(BINARY_SEARCH_TREE_ALLOCATION_FAILED);
    }
    tree->data_size = 0;
    tree->data      = NULL;
    tree->left = tree->right = NULL;
    return tree;
}

BinarySearchTreeNode* bin_search_tree_alloc_node(){
    BinarySearchTreeNode* tree = malloc(sizeof(BinarySearchTreeNode));
    if (tree == NULL){
        fprintf(stderr, "Failed malloc at bin_search_tree_alloc_node\n");
        exit(BINARY_SEARCH_TREE_NODE_ALLOCATION_FAILED);
    }
    tree->data = NULL;
    tree->left = tree->right = NULL;
    tree->data_size = 0;
    return tree;
}

int is_bin_search_tree_null(BinarySearchTree tree){
    return tree == NULL;
}

/**
 * @brief Lookup in a BST; return the node holding data or NULL if absent.
 * - Requires: tree already allocated (non-NULL); data and compare non-NULL.
 * - Uses compare(data, node->data) to go left/right; duplicates compare == 0.
 * @complexity O(h) time; recursive where h = height.
 */
BinarySearchTreeNode* bin_search_tree_contains(
    BinarySearchTree tree,
    const void *data,
    bst_compare_fn compare
){
    if (is_bin_search_tree_null(tree)){
        fprintf(stderr, "Failed bin_search_tree_contains: invoked on a NULL binary search tree, you have to allocate it before using it\n");
        exit(BINARY_SEARCH_TREE_NOT_INITIALIZED);
    }
    if (data == NULL){
        fprintf(stderr, "Failed bin_search_tree_contains: invoked with data=NULL\n");
        exit(BINARY_SEARCH_TREE_CONTAINS_FAILED);
    }
    if (compare == NULL){
        fprintf(stderr, "Failed bin_search_tree_contains: compare function is NULL\n");
        exit(BINARY_SEARCH_TREE_CONTAINS_FAILED);
    }

    // handle sentinel root (empty tree)
    if (tree->data == NULL) {
        if (tree->left != NULL || tree->right != NULL){
            fprintf(stderr, "Failed bin_search_tree_contains: malformed tree (root->data == NULL with children)\n");
            exit(MALFORMED_BINARY_SEARCH_TREE);
        }
        return NULL; // empty tree: not found
    }

    int cmp = compare(data, tree->data);
    if (cmp == 0) return tree;
    if (cmp < 0) return (tree->left  ? bin_search_tree_contains(tree->left,  data, compare) : NULL);
    return (tree->right ? bin_search_tree_contains(tree->right, data, compare) : NULL);
}

/* ==================================== insert ==================================== */
/**
 * @brief Insert a value into the BST, transferring ownership of data to the tree.
 *        No bytes are copied. The caller must not free or use data after insertion.
 *        Requires a pre-allocated empty root (bin_search_tree_alloc_node()).
 *        Duplicates are not inserted (returns the existing node).
 * @param tree Non-NULL BST root (may be empty sentinel).
 * @return Pointer to node holding data (existing or newly created).
 * @complexity O(h) time; recursive where h = height.
 */
BinarySearchTreeNode* bin_search_tree_insert_node(
    BinarySearchTree tree,
    void *data,
    size_t data_size,
    bst_compare_fn compare
){
    if (is_bin_search_tree_null(tree)) {
        fprintf(stderr, "Failed bin_search_tree_insert_node: invoked on a NULL binary search tree\n");
        exit(BINARY_SEARCH_TREE_NOT_INITIALIZED);
    }
    if (compare == NULL){
        fprintf(stderr, "Failed bin_search_tree_insert_node: NULL compare function\n");
        exit(BINARY_SEARCH_TREE_INSERT_FAILED);
    }
    if (data == NULL){
        fprintf(stderr, "Failed bin_search_tree_insert_node: invoked with data=NULL\n");
        exit(BINARY_SEARCH_TREE_INSERT_FAILED);
    }
    if (data_size == 0){
        fprintf(stderr, "Failed bin_search_tree_insert_node: data_size == 0\n");
        exit(BINARY_SEARCH_TREE_INSERT_FAILED);
    }

    // empty sentinel root -> store payload directly (ownership transfer)
    if (tree->data == NULL){
        if (tree->left != NULL || tree->right != NULL){
            fprintf(stderr, "Failed bin_search_tree_insert_node: malformed tree (root->data==NULL with children)\n");
            exit(MALFORMED_BINARY_SEARCH_TREE);
        }
        tree->data = data;        // take ownership
        tree->data_size = data_size;
        return tree;
    }

    int cmp = compare(data, tree->data);
    if (cmp == 0) return tree; // already contained: caller must free his data to avoid leak

    if (cmp < 0){
        if (tree->left == NULL){ // insert here
            tree->left = bin_search_tree_alloc_node();
            tree->left->data      = data;       // take ownership
            tree->left->data_size = data_size;
            return tree->left;
        } else { // recurse left
            return bin_search_tree_insert_node(tree->left, data, data_size, compare);
        }
    } else { // cmp > 0
        if (tree->right == NULL){ // insert here
            tree->right = bin_search_tree_alloc_node();
            tree->right->data      = data;      // take ownership
            tree->right->data_size = data_size;
            return tree->right;
        } else { // recurse right
            return bin_search_tree_insert_node(tree->right, data, data_size, compare);
        }
    }
}

/* ==================================== delete ==================================== */
/*
   Deletes node containing data (if present).
   Preserves the same root pointer (even when deleting the root).

   Ownership notes:
   - Each payload is owned by exactly one node.
   - On deletion, the node being physically removed frees its owned payload
     (deep_free if provided, else free()).
   - In two-children case, payloads are swapped before removing the successor,
     so the freed payload is the one currently stored in the node being removed.
*/
void bin_search_tree_delete_node(
    BinarySearchTree tree,
    const void* data,
    bst_compare_fn compare,
    bst_free_fn deep_free
){
    if (is_bin_search_tree_null(tree)){
        fprintf(stderr, "Failed bin_search_tree_delete_node: invoked on a NULL binary search tree, you have to allocate it before using it\n");
        exit(BINARY_SEARCH_TREE_NOT_INITIALIZED);
    }
    if (data == NULL){
        fprintf(stderr, "Failed bin_search_tree_delete_node: invoked with data=NULL\n");
        exit(BINARY_SEARCH_TREE_DELETE_FAILED);
    }
    if (compare == NULL){
        fprintf(stderr, "Failed bin_search_tree_delete_node: compare function is NULL\n");
        exit(BINARY_SEARCH_TREE_DELETE_FAILED);
    }

    // empty tree sentinel
    if (tree->data == NULL) {
        if (tree->left != NULL || tree->right != NULL){
            fprintf(stderr, "Failed bin_search_tree_delete_node: malformed tree (root->data == NULL with children)\n");
            exit(MALFORMED_BINARY_SEARCH_TREE);
        }
        fprintf(stderr, "You tried to remove data from an empty binary search tree. This is a no-op\n");
        return;
    }

    // 1) find the node (iterative)
    BinarySearchTreeNode* parent = NULL;
    BinarySearchTreeNode* curr   = tree;

    while (1) {
        if (curr->data == NULL) {
            fprintf(stderr, "Failed bin_search_tree_delete_node: malformed tree (encountered node->data == NULL)\n");
            exit(MALFORMED_BINARY_SEARCH_TREE);
        }
        int cmp = compare(data, curr->data);
        if (cmp == 0) break;

        if (cmp < 0) {
            if (curr->left == NULL) return; // not found
            parent = curr;
            curr   = curr->left;
        } else { // cmp > 0
            if (curr->right == NULL) return; // not found
            parent = curr;
            curr   = curr->right;
        }
    }

    /* Case A: leaf */
    if (curr->left == NULL && curr->right == NULL) {
        if (parent != NULL) {
            if (parent->left == curr) parent->left = NULL;
            else parent->right = NULL;

            if (deep_free) deep_free(curr->data);
            else free(curr->data);

            free(curr);
        } else {
            // leaf root -> sentinelize (keep root pointer stable)
            if (deep_free) deep_free(curr->data);
            else free(curr->data);

            curr->data = NULL;
            curr->data_size = 0;
            // left/right already NULL (leaf)
        }
        return;
    }

    /* Case B: exactly one child */
    if (curr->left == NULL || curr->right == NULL) {
        if (parent != NULL) {
            // reconnect parent to the unique child
            BinarySearchTreeNode* only_child = (curr->left ? curr->left : curr->right);

            if (parent->left == curr) {
                parent->left = only_child;
            } else { parent->right = only_child; }

            if (deep_free) {
                deep_free(curr->data);
            } else {
                free(curr->data);
            }

            free(curr);
            return;
        } else {
            // deleting root with one child:
            // move child's payload into root (ownership transfer) and free the child node
            BinarySearchTreeNode* victim = (curr->left ? curr->left : curr->right);

            // free old root payload first
            if (deep_free) {
                deep_free(curr->data);
            } else {
                free(curr->data);
            }

            // transfer ownership of victim payload into root
            curr->data      = victim->data;
            curr->data_size = victim->data_size;
            victim->data = NULL;        // avoid double-free
            victim->data_size = 0;

            // adopt victim's children
            curr->left  = victim->left;
            curr->right = victim->right;

            // free the victim node (no payload)
            free(victim);
            return;
        }
    }

    /* Case C: two children */
    {
        // find in-order successor (min in right subtree) and its parent
        BinarySearchTreeNode* succ = bin_search_tree_find_min(curr->right);

        BinarySearchTreeNode* succ_parent = curr;  // default if succ == curr->right
        if (succ != curr->right) {
            succ_parent = curr->right;
            while (succ_parent->left != succ) {
                succ_parent = succ_parent->left;
            }
        }

        // swap payloads (ownership moves with the pointer)
        bst_swap_payload(curr, succ);

        // now remove successor node (it has at most a right child)
        BinarySearchTreeNode* succ_repl = succ->right;  // replacement (could be NULL)
        if (succ_parent == curr) {
            curr->right = succ_repl;
        } else {
            succ_parent->left = succ_repl;
        }

        // free successor's payload (which is the original curr payload after swap) and the node
        if (deep_free) deep_free(succ->data);
        else           free(succ->data);
        free(succ);
        return;
    }
}

/* ==================================== min/max ==================================== */

BinarySearchTreeNode* bin_search_tree_find_min(BinarySearchTree tree) {
    if (is_bin_search_tree_null(tree)) {
        fprintf(stderr, "Failed bin_search_tree_find_min: invoked on a NULL binary search tree, you have to allocate it before using it\n");
        exit(BINARY_SEARCH_TREE_NOT_INITIALIZED);
    }
    if (tree->data == NULL) {
        fprintf(stderr, "Failed bin_search_tree_find_min: invoked on an empty/malformed tree (root->data == NULL)\n");
        exit(MALFORMED_BINARY_SEARCH_TREE);
    }
    BinarySearchTreeNode* cur = tree;
    while (cur->left) cur = cur->left;
    return cur;
}

BinarySearchTreeNode* bin_search_tree_find_max(BinarySearchTree tree) {
    if (is_bin_search_tree_null(tree)) {
        fprintf(stderr, "Failed bin_search_tree_find_max: tree is NULL\n");
        exit(BINARY_SEARCH_TREE_NOT_INITIALIZED);
    }
    if (tree->data == NULL) {
        fprintf(stderr, "Failed bin_search_tree_find_max: empty/malformed tree (root->data == NULL)\n");
        exit(MALFORMED_BINARY_SEARCH_TREE);
    }
    BinarySearchTreeNode* cur = tree;
    while (cur->right) cur = cur->right;
    return cur;
}

/* =================================== destroy =================================== */
/**
 * @brief Recursively destroy the BST (post-order).
 * - If provided, calls deep_free(node->data) before freeing the node.
 * - If deep_free is NULL, payload is freed with plain free().
 * - No-op (with warning) if tree is NULL.
 * @complexity O(n) time; O(h) stack.
 */
void binary_search_tree_destroy(BinarySearchTree tree, bst_free_fn deep_free){
    if (tree == NULL) {
        fprintf(stderr, "You are trying to destroy a non-initialized binary search tree, this is a no-op\n");
        return;
    }

    binary_search_tree_destroy(tree->left,  deep_free);
    binary_search_tree_destroy(tree->right, deep_free);

    if (tree->data != NULL){
        if (deep_free) deep_free(tree->data);
        else           free(tree->data);
    }
    free(tree);
}

/* =================================== rebalance =================================== */
/*
 * Rebalance in place: build a balanced shape by relinking existing node pointers.
 * Payloads are never copied; ownership remains with the nodes that hold them.
 * The root node object is preserved; its left/right get reassigned to balanced subtrees.
 */
void bin_search_tree_rebalance(BinarySearchTree tree){
    if (is_bin_search_tree_null(tree)){
        fprintf(stderr, "Failed bin_search_tree_rebalance: invoked on a NULL binary search tree, you have to allocate it before using it\n");
        exit(BINARY_SEARCH_TREE_NOT_INITIALIZED);
    }
    if (tree->data == NULL){
        if (tree->left != NULL || tree->right != NULL){
            fprintf(stderr, "Failed bin_search_tree_rebalance: malformed tree (root->data == NULL with children)\n");
            exit(MALFORMED_BINARY_SEARCH_TREE);
        }
        return; // empty sentinel: nothing to do
    }

    size_t n = bst_count_nodes(tree);
    if (n <= 1) return;

    // collect nodes in-order (sorted by key)
    BinarySearchTreeNode** nodes = (BinarySearchTreeNode**) malloc(n * sizeof(BinarySearchTreeNode*));
    if (!nodes){
        fprintf(stderr, "Failed bin_search_tree_rebalance: malloc nodes failed\n");
        exit(BINARY_SEARCH_TREE_NODE_ALLOCATION_FAILED);
    }
    size_t idx = 0;
    bst_fill_nodes_inorder(tree, nodes, &idx);

    // find root index in in-order list
    size_t r = 0;
    while (r < n && nodes[r] != tree) ++r;
    if (r == n){
        fprintf(stderr, "Failed bin_search_tree_rebalance: root not found in traversal\n");
        exit(MALFORMED_BINARY_SEARCH_TREE);
    }

    // link balanced left/right subtrees from slices excluding the root
    BinarySearchTreeNode* new_left  = (r > 0) ? bst_link_balanced(nodes, 0, (long)r - 1) : NULL;
    BinarySearchTreeNode* new_right = (r + 1 < n) ? bst_link_balanced(nodes, (long)r + 1, (long)n - 1) : NULL;

    // attach to the original root object
    tree->left  = new_left;
    tree->right = new_right;

    free(nodes);
}

/* ================================= pretty print ================================= */

void bin_search_tree_pretty_print(
    BinarySearchTree tree,
    void (*print_data)(const void* data, size_t size) // optional, can be NULL
){
    if (is_bin_search_tree_null(tree)){
        fprintf(stderr, "Failed bin_search_tree_pretty_print: invoked on a NULL binary search tree, you have to allocate it before using it\n");
        exit(BINARY_SEARCH_TREE_NOT_INITIALIZED);
    }
    if (tree->data == NULL){
        if (tree->left != NULL || tree->right != NULL){
            fprintf(stderr, "Failed bin_search_tree_pretty_print: malformed tree (root->data == NULL with children)\n");
            exit(MALFORMED_BINARY_SEARCH_TREE);
        }
        printf("(empty BST)\n");
        return;
    }

    // root line
    bst_print_node_line(tree, print_data);
    // print right subtree above, left below
    if (tree->right) bst_print_rec(tree->right, "", 1, print_data);
    if (tree->left)  bst_print_rec(tree->left,  "", 0, print_data);
}

/* =========================== utility helper functions =========================== */

// in-order count (only proper nodes: data != NULL)
static size_t bst_count_nodes(BinarySearchTreeNode* n){
    if (!n) return 0;
    if (n->data == NULL){
        if (n->left != NULL || n->right != NULL){
            fprintf(stderr, "Failed bin_search_tree_rebalance: encountered node with data==NULL and children\n");
            exit(MALFORMED_BINARY_SEARCH_TREE);
        }
        return 0; // sentinel-like node (should happen only at root if empty)
    }
    return 1 + bst_count_nodes(n->left) + bst_count_nodes(n->right);
}

// in-order fill: node pointers
static void bst_fill_nodes_inorder(BinarySearchTreeNode* n, BinarySearchTreeNode** arr, size_t* idx){
    if (!n) return;
    bst_fill_nodes_inorder(n->left, arr, idx);
    if (n->data != NULL) arr[(*idx)++] = n;
    bst_fill_nodes_inorder(n->right, arr, idx);
}

// build balanced shape from [lo..hi] array of nodes
static BinarySearchTreeNode* bst_link_balanced(BinarySearchTreeNode** arr, long lo, long hi){
    if (hi < lo) return NULL;
    long mid = lo + (hi - lo) / 2;
    BinarySearchTreeNode* root = arr[mid];
    root->left  = bst_link_balanced(arr, lo,     mid - 1);
    root->right = bst_link_balanced(arr, mid + 1, hi);
    return root;
}

// swap payload pointers and sizes between two nodes
static inline void bst_swap_payload(BinarySearchTreeNode* a, BinarySearchTreeNode* b){
    void*  pd = a->data;      a->data = b->data;      b->data = pd;
    size_t ps = a->data_size; a->data_size = b->data_size; b->data_size = ps;
}

// default hex preview for up to 8 bytes
static void bst_default_preview(const void* data, size_t size){
    const unsigned char* b = (const unsigned char*) data;
    size_t lim = (size < 8 ? size : 8);
    printf("0x");
    for (size_t i = 0; i < lim; ++i) printf("%02X", b[i]);
    if (size > lim) printf("…");
}

// one-line node info
static void bst_print_node_line(BinarySearchTreeNode* n, void (*print_data)(const void*, size_t)){
    printf("[node=%p size=%zu L=%p R=%p data=", (void*)n, n->data_size, (void*)n->left, (void*)n->right);
    if (print_data) print_data(n->data, n->data_size);
    else            bst_default_preview(n->data, n->data_size);
    printf("]\n");
}

// recursive sideways printer (right on top, left below) with ASCII branches
static void bst_print_rec(BinarySearchTreeNode* n, const char* prefix, int is_right,
                          void (*print_data)(const void*, size_t)) {
    if (!n) return;

    char next_prefix_right[512];
    char next_prefix_left[512];

    snprintf(next_prefix_right, sizeof(next_prefix_right), "%s%s", prefix, (is_right ? "    " : BST_VBAR));
    snprintf(next_prefix_left,  sizeof(next_prefix_left),  "%s%s", prefix, (is_right ? "    " : BST_VBAR));

    if (n->right) bst_print_rec(n->right, next_prefix_right, 1, print_data);

    printf("%s%s", prefix, (is_right ? BST_JR : BST_JL));
    bst_print_node_line(n, print_data);

    if (n->left)  bst_print_rec(n->left,  next_prefix_left,  0, print_data);
}

