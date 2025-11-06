#ifndef BIN_TREE_H
#define BIN_TREE_H

#include <stddef.h>
#include <stdio.h>

/*
 * Binary Search Tree (BST) — ownership & no-copy design
 *
 * - Ownership: the tree takes ownership of payload pointers passed to insert.
 *   Payloads MUST be heap-allocated and MUST NOT be used/freed by the caller
 *   after insertion. The tree frees them on node deletion or destroy.
 *
 * - No copying: the tree never deep-copies bytes. Internal operations move
 *   payloads by swapping `data`/`data_size` between nodes when needed
 *   (e.g., delete with two children, delete root with one child).
 *
 * - Root sentinel: an empty BST is a single root node with data==NULL and no children.
 *   The root pointer is stable and never replaced (root-preserving API).
 *
 * - Comparator: a total order over payloads is required. The comparator is
 *   used as compare(a,b): <0 => a<b, 0 => a==b, >0 => a>b.
 */

#define BINARY_SEARCH_TREE_ALLOCATION_FAILED        -91
#define BINARY_SEARCH_TREE_NODE_ALLOCATION_FAILED   -90
#define BINARY_SEARCH_TREE_CONTAINS_FAILED          -89
#define BINARY_SEARCH_TREE_INSERT_FAILED            -88
#define MALFORMED_BINARY_SEARCH_TREE                -87
#define BINARY_SEARCH_TREE_NOT_INITIALIZED          -86
#define BINARY_SEARCH_TREE_FIND_MIN_FAILED          -85
#define BINARY_SEARCH_TREE_DELETE_FAILED            -84

/** Opaque BST node storing an owned payload pointer. */
typedef struct BinarySearchTreeNode {
    void *data;                         // owned payload pointer (heap-allocated)
    size_t data_size;                   // payload size in bytes
    struct BinarySearchTreeNode *left;  // left child
    struct BinarySearchTreeNode *right; // right child
} BinarySearchTreeNode;

/** Root handle for the BST. */
typedef BinarySearchTreeNode* BinarySearchTree;

/** Comparator: return <0, 0, >0 for (a < b), (a == b), (a > b). */
typedef int (*bst_compare_fn)(const void *a, const void *b);

/** Optional payload free-callback (may be NULL; fallback is plain free()). */
typedef void (*bst_free_fn)(void *payload);

/**
 * @brief Builds a new empty BST node (root sentinel): data=NULL, size=0, no children.
 * @return Newly allocated root node; exits on failure.
 */
BinarySearchTreeNode* bin_search_tree_build_empty(void);

/**
 * @brief Allocate an empty BST node (data=NULL, size=0, no children).
 * @return Newly allocated node (never NULL; exits on failure).
 * @complexity O(1).
 */
BinarySearchTreeNode* bin_search_tree_alloc_node(void);

/**
 * @brief Return 1 if the tree handle is NULL, 0 otherwise.
 */
int is_bin_search_tree_null(BinarySearchTree tree);

/**
 * @brief Lookup in the BST; find the node holding `data` or return NULL if absent.
 * @param tree     Non-NULL BST root (must be allocated beforehand).
 * @param data     Key/payload to search (non-NULL, read-only).
 * @param compare  Comparator used for ordering (<0 left, 0 equal, >0 right).
 * @return Pointer to the matching node, or NULL if not found.
 * @complexity O(h) time (h = tree height); recursive.
 */
BinarySearchTreeNode* bin_search_tree_contains(
    BinarySearchTree tree,
    const void *data,
    bst_compare_fn compare
);

/**
 * @brief Insert a value into the BST, transferring ownership of `data` to the tree.
 *        No bytes are copied. The caller must not free or use `data` after insertion.
 *        Requires a pre-allocated empty root (bin_search_tree_alloc_node()).
 *        Duplicates are not inserted (returns the existing node).
 * @param tree      Non-NULL BST root (may be empty sentinel: data==NULL, no children).
 * @param data      Heap-allocated payload pointer to take ownership of (non-NULL).
 * @param data_size Size of `data` in bytes (>0).
 * @param compare   Comparator used for ordering.
 * @return Pointer to the node holding `data` (newly created or existing).
 * @complexity O(h) time (h = tree height); recursive.
 */
BinarySearchTreeNode* bin_search_tree_insert_node(
    BinarySearchTree tree,
    void *data,
    size_t data_size,
    bst_compare_fn compare
);

/*
    Deletes node containing data (if present). Root pointer is preserved.
    Ownership model: each payload is owned by exactly one node at a time.
    On deletion, the owned payload is freed (deep_free if provided, else free()).
*/
void bin_search_tree_delete_node(
    BinarySearchTree tree, 
    const void* data, 
    bst_compare_fn compare,
    bst_free_fn deep_free /* optional, can be NULL */
);

/**
 * Rebalance the BST in-place while preserving the *same* root node object.
 * Only pointers between nodes are relinked to produce a balanced shape.
 * Payloads are never copied; ownership does not change.
 */
void bin_search_tree_rebalance(
    BinarySearchTree tree
);

/**
 * Pretty-print the BST (sideways) with node address, data size, children links, and data preview.
 * If print_data is NULL, a hex preview (up to 8 bytes) is shown.
 */
void bin_search_tree_pretty_print(
    BinarySearchTree tree,
    void (*print_data)(const void* data, size_t size) /* optional, can be NULL */
);

/**
 * Find the minimum node in a BST (leftmost node).
 * Requires a non-NULL, non-empty tree (tree->data != NULL).
 * Exits on uninitialized/empty tree per project policy.
 * @return Pointer to the minimum node.
 * 
 * complexity O(h) time, iterative.
 */
BinarySearchTreeNode* bin_search_tree_find_min(BinarySearchTree tree);

/**
 * Find the maximum node in a BST (rightmost node).
 * Requires non-NULL, non-empty tree (tree->data != NULL). Exits on violation.
 * @return Pointer to the maximum node. 
 * 
 * complexity O(h) time, iterative.
 */
BinarySearchTreeNode* bin_search_tree_find_max(BinarySearchTree tree);

/**
 * @brief Recursively destroy the entire BST (post-order).
 *        If provided, calls `deep_free(node->data)` before freeing each node;
 *        otherwise payload is freed with plain free().
 * @param tree       Root handle (may be NULL; no-op with warning in impl).
 * @param deep_free  Optional payload free-callback (can be NULL).
 * @complexity O(n) time; O(h) stack.
 */
void binary_search_tree_destroy(
    BinarySearchTree tree,
    bst_free_fn deep_free
);

#endif /* BIN_TREE_H */
