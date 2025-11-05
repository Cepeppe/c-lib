#include "linked_list.h"

// builds linked list
// It is the PROGRAMMER RESPONSABILITY TO CALL 
// THIS METHOD BEFORE USING LINKED LIST
LinkedList build_empty_linked_list(){
    LinkedList list = (LinkedList) malloc(sizeof(LinkedListNode));
    if(list == NULL){
        fprintf(stderr, "Failed malloc while trying to build new empty linked list\n");
        exit(FAILED_LINKED_LIST_ALLOCATION);
    }
    
    list->data=NULL;
    list->next=NULL;
    return list;
}

//An empty linked list is a list having the first element initialized 
//but no data and no next (i.e. == NULL).
int is_linked_list_empty(LinkedList list){
    if (list == NULL) {
        fprintf(stderr, "You tried to check if a NULL linked list is empty\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }

    return (list->data==NULL && list->next==NULL ? 1 : 0);
}

/* Returns 1 if list is null, 0 otherwise */
int is_linked_list_null(LinkedList list){
    return list == NULL ? 1 : 0;
}

/* Free data field for a single LinkedListNode */
void free_linked_list_node_data(LinkedListNode* node, void (*deep_deallocate_node_data)(void* data)){
    if (node == NULL) return;
    if (node->data != NULL) {
        deep_deallocate_node_data(node->data);
        node->data = NULL;  /* avoid accidental double frees */
    }
}

void* get_linked_list_head_data(LinkedList list){
    if (is_linked_list_null(list)){
        fprintf(stderr, "You tried to access head data in a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }

    if(is_linked_list_empty(list)) return NULL;
    return list->data;
}

LinkedList get_linked_list_tail(LinkedList list){
    if (is_linked_list_null(list)){
        fprintf(stderr, "You tried to access a NULL linked list tail\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }
    if(is_linked_list_empty(list)) 
        return NULL;
    return list->next;
}

size_t get_linked_list_size(LinkedList list){
    size_t size = 0;

    if (is_linked_list_null(list)){
        fprintf(stderr, "You tried to calculate length on a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    } else if (is_linked_list_empty(list)){
        return size;
    } 

    size++;
    LinkedList temp_list = list;

    while(temp_list->next!=NULL){
        temp_list = temp_list->next;
        size++;
    }

    return size;
}

// Returns pointer to last element, if list is empty returns null
LinkedListNode* get_linked_list_last_element(LinkedList list){
    if (is_linked_list_null(list)){
        fprintf(stderr, "You requested last element from a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }
    if (is_linked_list_empty(list)){
        return NULL;
    }
    LinkedListNode* curr = list;
    while (curr->next != NULL) curr = curr->next;
    return curr;
}


// Appends data in place, do not allocate the pointer referencing data until that data it's removed from list
void linked_list_push_back(LinkedList list, void* data){
    if (is_linked_list_null(list)){
        fprintf(stderr, "You tried to push back an element on a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }
    if (is_linked_list_empty(list)){
        list->data = data;
        return;
    }
    LinkedListNode* last = get_linked_list_last_element(list);
    LinkedListNode* n = (LinkedListNode*) malloc(sizeof(LinkedListNode));
    if (!n){
        fprintf(stderr, "Failed malloc while trying to push_back on linked list\n");
        exit(FAILED_LINKED_LIST_ALLOCATION);
    }
    n->data = data;
    n->next = NULL;
    last->next = n;
}


// Removes last from linked list without deep free of data
void linked_list_remove_last(LinkedList list){
    if (is_linked_list_null(list)){
        fprintf(stderr, "You tried to remove last element from a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    } else if (is_linked_list_empty(list)){
        return;
    } else if (list->next==NULL){
        list->data=NULL;
        return;
    }

    // Can't simply call get_linked_list_last_element becasue
    // we need also the element just before it to set its next to NULL    
    while(list->next!=NULL && list->next->next!=NULL){ //for ex.   [c, k, s, t] at index 2 iterations stop and l[2]->next is returned
        list=list->next;
    }

    LinkedListNode* before_last = list;
    LinkedListNode* last = list->next;
    free(last);
    before_last->next=NULL;
}

// Linked list has data ownership, so it will deep free last element memory space using this function
void linked_list_remove_last_with(LinkedList list, void (*deep_deallocate_node_data)(void* data)){
    if (is_linked_list_null(list)){
        fprintf(stderr, "You tried to remove last element from a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    } else if (is_linked_list_empty(list)){
        return;
    } else if (list->next==NULL){
        deep_deallocate_node_data(list->data);
        list->data=NULL;
        return;
    }

    // Can't simply call get_linked_list_last_element becasue
    // we need also the element just before it to set its next to NULL    
    while(list->next!=NULL && list->next->next!=NULL){ //for ex.   [c, k, s, t] at index 2 iterations stop and l[2]->next is returned
        list=list->next;
    }

    LinkedListNode* before_last = list;
    LinkedListNode* last = list->next;
    deep_deallocate_node_data(last->data);
    free(last);
    before_last->next=NULL;
}

void linked_list_push_front(LinkedList list, void* data) {
    if (is_linked_list_null(list)) {
        fprintf(stderr, "You tried to push front an element on a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }

    if (is_linked_list_empty(list)) {
        list->data = data;
        return;
    }
    
    LinkedListNode* shifted_node = (LinkedListNode*) malloc(sizeof(LinkedListNode));

    if (shifted_node == NULL) {
        fprintf(stderr, "Failed malloc while trying to push_front on linked list\n");
        exit(FAILED_LINKED_LIST_ALLOCATION);
    }

    // Copy current head content in the new node (which will be put 2nd)
    shifted_node->data = list->data;
    shifted_node->next = list->next;

    //Overwrite head data and link head node again 
    list->data = data;
    list->next = shifted_node;

    return;
}

// Removes first node and performs deep free of data
void linked_list_remove_first_with(LinkedList list, void (*deep_deallocate_node_data)(void* data)) {
    if (is_linked_list_null(list)) {
        fprintf(stderr, "You tried to remove first element from a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    } else if (is_linked_list_empty(list)) {
        return;
    }

    // Single-element list: just deallocate data and turn into empty list
    if (list->next == NULL) {
        if (list->data != NULL && deep_deallocate_node_data!=NULL) {
            deep_deallocate_node_data(list->data);
        }
        list->data = NULL;
        return;
    }

    // General case: at least 2 nodes, 
    //  1.  promote the second node into the head
    //  2.  free the old second node
    LinkedListNode* second = list->next;

    // The current head's data is being removed, so free it
    if (list->data != NULL) {
        deep_deallocate_node_data(list->data);
    }

    // Move second's data/next into head
    list->data = second->data; 
    list->next = second->next;

    //Free old second node
    free(second);
    
    return;
}

// Removes first node and performs deep free of data
void linked_list_remove_first(LinkedList list) {
    if (is_linked_list_null(list)) {
        fprintf(stderr, "You tried to remove first element from a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    } else if (is_linked_list_empty(list)) {
        return;
    }

    // Single-element list
    if (list->next == NULL) {
        list->data=NULL;
        return;
    }

    // General case: at least 2 nodes, 
    //  1.  promote the second node into the head
    //  2.  free the old second node
    LinkedListNode* second = list->next;

    // Move second's data/next into head
    list->data = second->data; 
    list->next = second->next;

    //Free old second node
    free(second);
    
    return;
}

void linked_list_destroy(LinkedList list) {
    if (is_linked_list_null(list)) {
        fprintf(stderr, "You are trying to destroy a NULL linked list, this is a no-op\n");
        return;
    }

    LinkedList current = list;
    while (current != NULL) {
        LinkedList next = current->next;
        free(current);
        current = next;
    }

    return;
}

void linked_list_destroy_with(LinkedList list, void (*deep_deallocate_node_data)(void* data)) {
    if (is_linked_list_null(list)) {
        fprintf(stderr, "You are trying to destroy a NULL linked list, this is a no-op\n");
        return;
    }

    LinkedList current = list;
    while (current != NULL) {
        LinkedList next = current->next;
        deep_deallocate_node_data(current->data);
        free(current);
        current = next;
    }

    return;
}

//Removes node after the chosen one
void linked_list_remove_next_node(LinkedListNode* node){
    if (is_linked_list_null(node)) {
        fprintf(stderr, "You are trying to remove next node for a NULL node\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }

    if(node->next == NULL) return;

    LinkedListNode* following_one = node->next;
    node->next=following_one->next;

    free(following_one);
        
    return;
}

//Removes node after the chosen one and performs deep free of memory
void linked_list_remove_next_node_with(LinkedListNode* node, void (*deep_deallocate_node_data)(void* data)){
    if (is_linked_list_null(node)) {
        fprintf(stderr, "You are trying to remove next node for a NULL node\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }

    if(node->next == NULL) return;

    LinkedListNode* following_one = node->next;
    node->next=following_one->next;

    if(following_one->data!=NULL){
        deep_deallocate_node_data(following_one->data);
    }
    free(following_one);
        
    return;
}

/*
 * Free a standalone LinkedListNode containing a HashMapItem.
 * PRE: the node is already detached from any list (no prev->next pointing to it).
 * Effect: frees HashMapItem->data via deep_deallocate_hashmap_item_data,
 *         then frees the HashMapItem itself via deep_deallocate_hashmap_item,
 *         then frees the node.
 * 
 * NB: DOES NOT RECONNECT PREVIOUS AND FOLLOWING NODES
 */
void linked_list_remove_hashmap_node_with(
    //Pointer to linked list node
    LinkedListNode* node,
    //Function pointer to hash map item deallocator function (frees list->data, which is a HashMapItem)
    void (*deep_deallocate_hashmap_item)(void* data, void (*deep_deallocate_hashmap_item_data)(void*)),
    //Function pointer to hashmap item data deallocator function (frees list->data->data or, in other terms, HashMapItem->data)
    // Can be null if HashMapItem->data is a primitive type and was not allocated via malloc
    void (*deep_deallocate_hashmap_item_data)(void* data)
){
    if (is_linked_list_null(node)) {
        fprintf(stderr, "You are trying to remove a NULL node\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    } else if (is_linked_list_empty(node)){
        fprintf(stderr, "You are trying to remove an empty linked list node, this is a no-op\n");
        return;
    }

    node->next = NULL;
    if (node->data!=NULL) {
        deep_deallocate_hashmap_item(node->data, deep_deallocate_hashmap_item_data);
    }

    free(node);

    return;
}

void linked_list_debug_print(LinkedList list, void (*print_data)(void*)) {
    if (is_linked_list_null(list)) {
        fprintf(stderr, "You tried to debug-print a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }

    /* empty logical list */
    if (is_linked_list_empty(list)) {
        printf("[HEAD|EMPTY @%p] -> NULL\n", (void*)list);
        return;
    }

    printf("[HEAD] ");

    LinkedList curr = list;
    size_t index = 0;

    while (curr != NULL) {

        /* example: -> [#0 @0x12345678 | data=...] */
        printf("[#%zu | 0x%p | data=", index, (void*)curr);

        if (curr->data == NULL) {
            printf("NULL");
        } else {
            /* user callback prints the payload */
            print_data(curr->data);
        }

        printf("] ");

        if (curr->next != NULL) {
            printf("==> ");
        }

        curr = curr->next;
        index++;
    }

    printf("NULL\n");
}