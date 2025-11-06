#ifndef BIN_TREE_H
#define BIN_TREE_H

#include <stddef.h>
#include <stdio.h>
#include "../bytes/bytes_utils.h"

#define BINARY_SEARCH_TREE_ALLOCATION_FAILED       -91
#define BINARY_SEARCH_TREE_NODE_ALLOCATION_FAILED  -90
#define BINARY_SEARCH_TREE_CONTAINS_FAILED         -89
#define BINARY_SEARCH_TREE_INSERT_FAILED           -88
#define MALFORMED_BINARY_SEARCH_TREE               -87
#define BINARY_SEARCH_TREE_NOT_INIZIALIZED         -86
#define BINARY_SEARCH_TREE_FIND_MIN_FAILED         -85
#define BINARY_SEARCH_TREE_DELETE_FAILED           -84


/** Opaque BST node storing an arbitrary byte payload. */
typedef struct BinarySearchTreeNode {
    void *data;                         /* deep-copied payload (owned by the tree) */
    size_t data_size;                   /* payload size in bytes */
    struct BinarySearchTreeNode *left;  /* left child */
    struct BinarySearchTreeNode *right; /* right child */
} BinarySearchTreeNode;

/** Root handle for the BST. */
typedef BinarySearchTreeNode* BinarySearchTree;

//////////////////////////////////////////////

/** Comparator: return <0, 0, >0 for (a < b), (a == b), (a > b). */
typedef int (*bst_compare_fn)(const void *a, const void *b);

/** Optional payload free-callback used by destroy (may be NULL). */
typedef void (*bst_free_fn)(void *payload);

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
 * @brief Insert a value into the BST, deep-copying the payload bytes.
 *        Requires a pre-allocated empty root (bin_search_tree_alloc_node()).
 *        Duplicates are not inserted (returns the existing node).
 * @param tree      Non-NULL BST root (may be empty sentinel: data==NULL, no children).
 * @param data      Bytes to insert (non-NULL), size > 0.
 * @param data_size Size of `data` in bytes.
 * @param compare   Comparator used for ordering.
 * @return Pointer to the node holding `data` (newly created or existing).
 * @complexity O(h) time (h = tree height); recursive.
 */
BinarySearchTreeNode* bin_search_tree_insert_node(
    BinarySearchTree tree,
    const void *data,
    size_t data_size,
    bst_compare_fn compare
);

/*
    Deletes node containing data (if present).
    Keeps the same root pointer (even when deleting the root).
    Notes:
    - deep_free_bin_search_tree_node_data is OPTIONAL: if NULL, plain free() is used on payload.
      This is safe for byte-buffers cloned via clone_bytes, but may leak for complex non-primitive payloads.
*/
void bin_search_tree_delete_node(
    BinarySearchTree tree, 
    const void* data, 
    int (*compare)(const void* data_1, const void* data_2),
    void (*deep_free_bin_search_tree_node_data)(void* data)
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
 *        If provided, calls `deep_free(node->data)` before freeing each node.
 * @param tree       Root handle (may be NULL; no-op with warning in impl).
 * @param deep_free  Optional payload free-callback (can be NULL).
 * @complexity O(n) time; O(h) stack.
 */
void binary_search_tree_destroy(
    BinarySearchTree tree,
    bst_free_fn deep_free
);

#endif /* BIN_TREE_H */
