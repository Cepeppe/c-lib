#include "linked_list.h"


LinkedList build_linked_list_copy(const void* data, size_t size, LinkedList next) {
    LinkedList list = malloc(sizeof *list);
    if (list == NULL) {
        fprintf(stderr, "Failed malloc while trying to build new linked list\n");
        exit(FAILED_LINKED_LIST_ALLOCATION);
    }
    list->data = clone_bytes(data, size);
    list->next = next;
    return list;
}

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
void free_linked_list_node_data(LinkedListNode* node){
    if (node == NULL) return;
    if (node->data != NULL) {
        free(node->data);
        node->data = NULL;  /* avoid accidental double frees */
    }
}

/* Free LinkedListNode (calls free_linked_list_node_data)*/
void free_linked_list_node(LinkedListNode* node){
    if(node==NULL) return;
    free_linked_list_node_data(node);
    free(node);
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

size_t get_linked_list_size_recursive(LinkedList list){
    if (is_linked_list_null(list)){
                        fprintf(stderr, "You tried to calculate length (recursive) on a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }

    if(is_linked_list_empty(list)) return 0;
    else if(list->data!=NULL && list->next==NULL) return 1;
    else return 1 + get_linked_list_size_recursive(list->next);
}

LinkedList get_linked_list_last_element(LinkedList list){
    if (is_linked_list_null(list)){
        fprintf(stderr, "You requested last element from a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    } else if (is_linked_list_empty(list)){
        return list;
    }
    else if (list->data != NULL && list->next==NULL)
        return list;

    LinkedList temp = list;
    while(temp->next!=NULL && temp->next->next!=NULL){
        temp=temp->next;
    }
    return temp->next;
}

void linked_list_push_back(LinkedList list, void* data){
    if (is_linked_list_null(list)){
        fprintf(stderr, "You tried to push back an element on a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    } else if (is_linked_list_empty(list)){
        void* new_data = (void*) malloc(sizeof(data));
        memcpy(new_data, data, sizeof(data));
        list->data=new_data;
        return;
    }

    LinkedList last_element = get_linked_list_last_element(list);

    if(is_linked_list_empty(last_element)){
        void* new_data = (void*) malloc(sizeof(data));
        memcpy(new_data, data, sizeof(data));
        last_element->data=new_data;
        return;
    } else {
        LinkedList new_element = build_empty_linked_list();
        void* new_data = (void*) malloc(sizeof(data));
        memcpy(new_data, data, sizeof(data));
        new_element->data = new_data;
        last_element->next = new_element;
    }
    
    return;
}

void linked_list_remove_last(LinkedList list){
    if (is_linked_list_null(list)){
        fprintf(stderr, "You tried to remove last element from a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }
    else if (is_linked_list_empty(list))
        return;
    else if (list->data != NULL && list->next==NULL){
        free(list->data);
        list->data = NULL;
        return;
    }
    
    LinkedList temp = list;
    
    while(temp->next!=NULL && temp->next->next!=NULL){
        temp=temp->next;
    }

    free_linked_list_node(temp->next);
    temp->next = NULL;
}

void linked_list_push_front(LinkedList list, void* data) {
    if (is_linked_list_null(list)) {
        fprintf(stderr, "You tried to push front an element on a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }

    if (is_linked_list_empty(list)) {
        void* new_data = (void*) malloc(sizeof(data));
        memcpy(new_data, data, sizeof(data));
        list->data = new_data;
        return;
    }


    LinkedListNode* shifted_node = (LinkedListNode*) malloc(sizeof(LinkedListNode));
    if (shifted_node == NULL) {
        fprintf(stderr, "Failed malloc while trying to push_front on linked list\n");
        exit(FAILED_LINKED_LIST_ALLOCATION);
    }

    /* Copy current head content in the new node */
    shifted_node->data = list->data;
    shifted_node->next = list->next;

    /* Overwrite head new data and link with new head */
    void* new_data = (void*) malloc(sizeof(data));
    memcpy(new_data, data, sizeof(data));
    list->data = new_data;
    list->next = shifted_node;

    return;
}

void linked_list_remove_first(LinkedList list) {
    if (is_linked_list_null(list)) {
        fprintf(stderr, "You tried to remove first element from a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    } else if (is_linked_list_empty(list)) {
        return;
    }

    /* Single-element list: free data and turn into empty list */
    if (list->next == NULL) {
        if (list->data != NULL) {
            free(list->data);
        }
        list->data = NULL;
        /* list->next is already NULL */
        return;
    }

    /* General case: at least 2 nodes
       We "promote" the second node into the head,
       then free the old second node. */
    LinkedListNode* second = list->next;

    /* The current head's data is being removed, so free it */
    if (list->data != NULL) {
        free(list->data);
    }

    /* Move second's data/next into head */
    list->data = second->data;     /* take ownership of this pointer */
    list->next = second->next;

    /* Free the old second node (NOT its data) */
    free(second);
}

void linked_list_destroy(LinkedList list) {
    if (is_linked_list_null(list)) {
        return;
    }

    LinkedList current = list;
    while (current != NULL) {
        LinkedList next = current->next;
        free_linked_list_node(current);
        current = next;
    }
}

/* 
    Get node pointer at position 'index' (0-based).
    Returns NULL if index is out of bounds or list is logically empty
*/
LinkedListNode* get_linked_list_at_index(LinkedList list, size_t index) {
    if (is_linked_list_null(list)) {
        fprintf(stderr, "You tried to access index on a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }

    if (is_linked_list_empty(list)) {
        return NULL;
    } 
    
    if (index == 0) {
        return list;
    }

    size_t actual_index = 1;
    LinkedList current = list->next;

    while (current != NULL) {
        if (actual_index == index) {
            return current;
        }
        current = current->next;
        actual_index++;
    }

    /* index out of range */
    return NULL;
}

/*
 * Remove element at 0-based `index` from the linked list.
 *
 * Returns:
 *   1 -> element was removed
 *   0 -> list was logically empty OR index out of bounds
 *
 *  If list is NULL exit().
 *  If index == 0, we delegate to linked_list_remove_first(list), which:
 *      turns a 1-element list into an "empty head" (data=NULL,next=NULL), or
 *      copies the second node into the head and frees that second node.
 *   This avoids changing the caller's head pointer.
 *  For index > 0, unlink and free the target node:
 *      prev -> victim -> next    becomes    prev -> next
 *   We free both victim->data and victim itself via free_linked_list_node().
 */
int linked_list_remove_at_index(LinkedList list, size_t index) {
    if (is_linked_list_null(list)) {
        fprintf(stderr,
                "You tried to remove an element by index from a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }

    if (is_linked_list_empty(list)) {
        fprintf(stderr,
                "Tried to remove an element by index from an empty linked list\n");
        fprintf(stderr,
                "This is a no-op. No actions performed, list is still empty\n");
        return 0;
    }

    if (index == 0) {
        linked_list_remove_first(list);
        return 1;
    }

    /* Walk to node at (index-1). */
    LinkedList prev = list;
    size_t pos = 0;
    while (pos < index - 1 && prev != NULL) {
        prev = prev->next;
        pos++;
    }

    /* Out of bounds: we didn't reach (index-1), or there's no node at `index`. */
    if (prev == NULL || prev->next == NULL) {
        fprintf(stderr,
                "Tried to remove index %zu but it's out of bounds (stopped at %zu)\n",
                index, pos);
        return 0;
    }

    /* Unlink and free the victim node. */
    LinkedListNode* victim = prev->next;
    prev->next = victim->next;
    free_linked_list_node(victim);

    return 1;
}


/* Reverse the list in-place (iterative).
   Returns the new head (caller MUST assign the return value).
   Exits if list is NULL */
LinkedList linked_list_reverse(LinkedList list) {
    if (is_linked_list_null(list)) {
        fprintf(stderr, "You tried to reverse a NULL linked list\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }

    /* Trivial cases: empty logical list or single node */
    if (is_linked_list_empty(list) || list->next == NULL) {
        return list;
    }

    LinkedList prev = NULL;
    LinkedList curr = list;
    LinkedList next = NULL;

    while (curr != NULL) {
        next = curr->next;   /* save next */
        curr->next = prev;   /* reverse pointer */
        prev = curr;         /* advance prev */
        curr = next;         /* advance curr */
    }

    return prev;
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
