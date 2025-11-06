#include "binary_tree.h"

/**
 *   @brief Builds new empty BST.
 *   @return pointer to new allocated BST
 */
BinarySearchTreeNode* bin_search_tree_build_empty(){
    BinarySearchTreeNode* tree = malloc(sizeof(BinarySearchTreeNode));
    if(tree==NULL){
        fprintf(stderr, "Failed bin_search_tree_build_empty: malloc failed\n");
        exit(BINARY_SEARCH_TREE_ALLOCATION_FAILED);
    }
    tree->data_size=0;
    tree->data=NULL;
    tree->left = tree->right = NULL;
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
    int (*compare)(const void* data_1, const void* data_2)
){  
    if (is_bin_search_tree_null(tree)){
        fprintf(stderr, "Failed bin_search_tree_contains: invoked on a NULL binary search tree, you have to allocate it before using it\n");
        exit(BINARY_SEARCH_TREE_NOT_INIZIALIZED);
    }
    if (data == NULL){
        fprintf(stderr, "Failed bin_search_tree_contains: invoked with data=NULL\n");
        exit(BINARY_SEARCH_TREE_CONTAINS_FAILED);
    }
    if (compare == NULL){
        fprintf(stderr, "Failed bin_search_tree_contains: compare function is NULL\n");
        exit(BINARY_SEARCH_TREE_CONTAINS_FAILED);
    }

    int compare_curr_node_res = compare(data, tree->data);
    if (compare_curr_node_res == 0) return tree;
    if (compare_curr_node_res < 0)
        return (tree->left  != NULL ? bin_search_tree_contains(tree->left,  data, compare) : NULL);
    return (tree->right != NULL ? bin_search_tree_contains(tree->right, data, compare) : NULL);
}

/**
 * @brief Allocate an empty BST node (data=NULL, size=0, no children).
 * - Exits on allocation failure.
 * @return Pointer to a zero-initialized node.
 * @complexity O(1).
 */
BinarySearchTreeNode* bin_search_tree_alloc_node(){
    BinarySearchTreeNode* tree;

    tree = malloc(sizeof(BinarySearchTreeNode));
    if (tree == NULL){
        fprintf(stderr, "Failed malloc at bin_search_tree_alloc_node\n");
        exit(BINARY_SEARCH_TREE_NODE_ALLOCATION_FAILED);
    }

    tree->data = NULL;
    tree->left = tree->right = NULL;
    tree->data_size = 0;

    return tree;
}

/**
 * @brief Insert a value into the BST, deep-copying the payload.
 * - Requires a pre-allocated empty root (bin_search_tree_alloc_node()).
 * - Descends with compare(data, node->data); duplicates are not inserted.
 * - Copies bytes via clone_bytes; tree owns the copy (free in destroy).
 * @param tree Non-NULL BST root (may be empty sentinel).
 * @return Pointer to node holding data (existing or newly created).
 * @complexity O(h) time; recursive where h = height.
 */
BinarySearchTreeNode* bin_search_tree_insert_node(
    BinarySearchTree tree, 
    const void* data, 
    size_t data_size, 
    int (*compare)(const void* data_1, const void* data_2)
){
    if (is_bin_search_tree_null(tree)) {
        fprintf(stderr, "Failed bin_search_tree_insert_node: invoked on a NULL binary search tree\n");
        exit(BINARY_SEARCH_TREE_NOT_INIZIALIZED);
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

    if (tree->data == NULL){
        if (tree->left != NULL || tree->right != NULL){  
            fprintf(stderr, "Failed bin_search_tree_insert_node: invoked on a malformed tree having node->data NULL and left and/or right not NULL\n");
            exit(MALFORMED_BINARY_SEARCH_TREE);
        } else if (tree->left == NULL && tree->right == NULL){
            tree->data = clone_bytes(data, data_size);
            tree->data_size = data_size;
            return tree;
        }
    }

    int compare_curr_node_res = compare(data, tree->data);
    if (compare_curr_node_res == 0) return tree; // already contained

    if (compare_curr_node_res < 0){
        if (tree->left == NULL){ // insert here
            tree->left = bin_search_tree_alloc_node();
            tree->left->data = clone_bytes(data, data_size);
            tree->left->data_size = data_size;
            return tree->left;
        } else { // recurse left
            return bin_search_tree_insert_node(tree->left, data, data_size, compare);
        }
    } else { // compare_curr_node_res > 0
        if (tree->right == NULL){ // insert here
            tree->right = bin_search_tree_alloc_node();
            tree->right->data = clone_bytes(data, data_size);
            tree->right->data_size = data_size;
            return tree->right;
        } else { // recurse right
            return bin_search_tree_insert_node(tree->right, data, data_size, compare);
        }
    }

    // Unreachable
    return NULL;
}

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
){

    if (is_bin_search_tree_null(tree)){
        fprintf(stderr, "Failed bin_search_tree_delete_node: invoked on a NULL binary search tree, you have to allocate it before using it\n");
        exit(BINARY_SEARCH_TREE_NOT_INIZIALIZED);
    }
    if (data == NULL){
        fprintf(stderr, "Failed bin_search_tree_delete_node: invoked with data=NULL\n");
        exit(BINARY_SEARCH_TREE_DELETE_FAILED);
    }
    if (compare == NULL){
        fprintf(stderr, "Failed bin_search_tree_delete_node: compare function is NULL\n");
        exit(BINARY_SEARCH_TREE_DELETE_FAILED);
    }

    /* root sentinel (empty tree) handling */
    if (tree->data == NULL) {
        if (tree->left != NULL || tree->right != NULL){
            fprintf(stderr, "Failed bin_search_tree_delete_node: malformed tree (root->data == NULL with children)\n");
            exit(MALFORMED_BINARY_SEARCH_TREE);
        }
        fprintf(stderr, "You tried to remove data from an empty bin_search_tree_delete_node. This is a no-op\n");
        return;
    }

    // 1) find the node (iterative)
    BinarySearchTreeNode* parent = NULL;
    BinarySearchTreeNode* curr_node = tree;
    int compare_res;

    while (1) {
        if (curr_node->data == NULL) {
            fprintf(stderr, "Failed bin_search_tree_delete_node: malformed tree (encountered node->data == NULL)\n");
            exit(MALFORMED_BINARY_SEARCH_TREE);
        }

        compare_res = compare(data, curr_node->data);
        if (compare_res == 0) break;

        if (compare_res < 0) {
            if (curr_node->left == NULL) return; // not found
            parent = curr_node;
            curr_node = curr_node->left;
        } else { // compare_res > 0
            if (curr_node->right == NULL) return; // not found 
            parent = curr_node;
            curr_node = curr_node->right;
        }
    }

    /* 2) deletion cases */

    /* Case A: leaf */
    if (curr_node->left == NULL && curr_node->right == NULL) {

        if (parent != NULL) { /* disconnect from parent */
            if (parent->left == curr_node) {
                parent->left = NULL;
            } else { 
                parent->right = NULL;
            }

            // free data (optional callback)
            if (deep_free_bin_search_tree_node_data != NULL){
                deep_free_bin_search_tree_node_data(curr_node->data);
            } else { 
                free(curr_node->data); 
            }
            //free node after freeing data
            free(curr_node);
        } else {
            // leaf root -> turn root into sentinel (keep tree pointer stable)
            if (deep_free_bin_search_tree_node_data != NULL){ 
                deep_free_bin_search_tree_node_data(curr_node->data);
            } else { 
                free(curr_node->data);
            }

            curr_node->data = NULL;
            curr_node->data_size = 0;
        }
        return;
    }

    // Case B: exactly one child 
    if (curr_node->left == NULL || curr_node->right == NULL) {

        if (parent != NULL) { // reconnect parent with nephew ( parent->curr_node->nephew => parent->nephew )
            if (parent->left == curr_node)
                parent->left  = (curr_node->left != NULL) ? curr_node->left : curr_node->right;
            else
                parent->right = (curr_node->left != NULL) ? curr_node->left : curr_node->right;

            if (deep_free_bin_search_tree_node_data != NULL){
                deep_free_bin_search_tree_node_data(curr_node->data);
            } else { free(curr_node->data); }

            //free node
            free(curr_node);
            return;
        } else { // deleting root with one child: copy child's payload into root, adopt its children, free child
            BinarySearchTreeNode* victim = (curr_node->left != NULL) ? curr_node->left : curr_node->right;

            if (deep_free_bin_search_tree_node_data != NULL){
                deep_free_bin_search_tree_node_data(curr_node->data);
            } else { free(curr_node->data); }

            // copy payload
            curr_node->data = clone_bytes(victim->data, victim->data_size);
            curr_node->data_size = victim->data_size;

            // take over victim's children
            curr_node->left  = victim->left;
            curr_node->right = victim->right;

            // free victim payload via callback if provided, else free()
            if (deep_free_bin_search_tree_node_data != NULL){
                deep_free_bin_search_tree_node_data(victim->data);
            } else { free(victim->data); }

            //free victim node
            free(victim);
            return;
        }
    }

    //Case C: two children 
    {   
        // use in-order successor: minimum node in the right subtree of curr_node
        BinarySearchTreeNode* succ = bin_search_tree_find_min(curr_node->right);

        // find successor's parent by walking left from curr_node->right until we reach succ
        BinarySearchTreeNode* succ_parent = curr_node;  // default if succ == curr_node->right
        if (succ != curr_node->right) {
            succ_parent = curr_node->right;
            while (succ_parent->left != succ) {
                succ_parent = succ_parent->left;
            }
        }

        // replace current node payload with successor payload (keep node to preserve root pointer)
        if (deep_free_bin_search_tree_node_data != NULL) {
            deep_free_bin_search_tree_node_data(curr_node->data);
        } else {
            free(curr_node->data);
        }
        curr_node->data = clone_bytes(succ->data, succ->data_size);
        curr_node->data_size = succ->data_size;

        // unlink successor from its parent, reconnecting successor's right child (if any)
        // NB: successor can only have a right child (never a left one)
        BinarySearchTreeNode* succ_right = succ->right;
        if (succ_parent == curr_node) {
            // special case: successor is the immediate right child of curr_node
            curr_node->right = succ_right;
        } else {
            // general case: successor lies somewhere below curr_node->right
            succ_parent->left = succ_right;
        }

        // free the successor node (its payload has been cloned into curr_node)
        if (deep_free_bin_search_tree_node_data != NULL) {
            deep_free_bin_search_tree_node_data(succ->data);
        } else {
            free(succ->data);
        }
        free(succ);

        return;
    }
}

/**
 * Find the minimum node in a BST (leftmost node).
 * Requires a non-NULL, non-empty tree (tree->data != NULL).
 * Exits on uninitialized/empty tree per project policy.
 * @return Pointer to the minimum node.
 * 
 * complexity O(h) time, iterative.
 */
BinarySearchTreeNode* bin_search_tree_find_min(BinarySearchTree tree) {
    if (is_bin_search_tree_null(tree)) {
        fprintf(stderr, "Failed bin_search_tree_find_min: invoked on a NULL binary search tree, you have to allocate it before using it\n");
        exit(BINARY_SEARCH_TREE_NOT_INIZIALIZED);
    }
    if (tree->data == NULL) {
        fprintf(stderr, "Failed bin_search_tree_find_min: invoked on an empty/malformed tree (root->data == NULL)\n");
        exit(MALFORMED_BINARY_SEARCH_TREE);
    }

    BinarySearchTreeNode* cur = tree;
    while (cur->left != NULL) cur = cur->left;
    return cur;
}


/**
 * Find the maximum node in a BST (rightmost node).
 * Requires non-NULL, non-empty tree (tree->data != NULL). Exits on violation.
 * @return Pointer to the maximum node. 
 * 
 * complexity O(h) time, iterative.
 */
BinarySearchTreeNode* bin_search_tree_find_max(BinarySearchTree tree) {
    if (is_bin_search_tree_null(tree)) {
        fprintf(stderr, "Failed bin_search_tree_find_max: tree is NULL\n");
        exit(BINARY_SEARCH_TREE_NOT_INIZIALIZED);
    }
    if (tree->data == NULL) {
        fprintf(stderr, "Failed bin_search_tree_find_max: empty/malformed tree (root->data == NULL)\n");
        exit(MALFORMED_BINARY_SEARCH_TREE);
    }

    BinarySearchTreeNode* cur = tree;
    while (cur->right != NULL) cur = cur->right;
    return cur;
}


/**
 * @brief Recursively destroy the BST (post-order).
 * - If provided, calls deep_free_bin_search_tree_node_data(node->data) before freeing the node.
 * - No-op (with warning) if tree is NULL.
 * @complexity O(n) time; O(h) stack.
 */
void binary_search_tree_destroy(BinarySearchTree tree, void (*deep_free_bin_search_tree_node_data)(void* data)){
    if(tree == NULL) {
        fprintf(stderr, "You are trying to destroy a non-initialized binary search tree, this is a no-op\n");
        return;
    }

    binary_search_tree_destroy(tree->left,  deep_free_bin_search_tree_node_data);
    binary_search_tree_destroy(tree->right, deep_free_bin_search_tree_node_data);

    if (tree->data != NULL)
        deep_free_bin_search_tree_node_data(tree->data);

    free(tree);
    return;
}

/**
 * @brief Utility: return 1 if tree is NULL, 0 otherwise.
 */
int is_bin_search_tree_null(BinarySearchTree tree){
    return tree==NULL;
}
