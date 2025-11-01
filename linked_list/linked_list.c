#include "linked_list.h"

LinkedList build_linked_list(void* data, LinkedList next){
    LinkedList list = (LinkedList) malloc(sizeof(LinkedListNode));
    if(list == NULL){
        fprintf(stderr, "Failed malloc while trying to build new linked list");
        exit(FAILED_LINKED_LIST_ALLOCATION);
    }

    list->data=data;
    list->next=next;
    return list;
}

LinkedList build_empty_linked_list(){
    LinkedList list = (LinkedList) malloc(sizeof(LinkedListNode));
    if(list == NULL){
        fprintf(stderr, "Failed malloc while trying to build new empty linked list");
        exit(FAILED_LINKED_LIST_ALLOCATION);
    }
    
    list->data=NULL;
    list->next=NULL;
    return list;
}

int is_linked_list_empty(LinkedList list){
    if (list == NULL) {
        fprintf(stderr, "You tried to check if a NULL linked list is empty");
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
        fprintf(stderr, "You tried to access head data in a NULL linked list");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }

    if(is_linked_list_empty(list)) return NULL;
    return list->data;
}

LinkedList get_linked_list_tail(LinkedList list){
    if (is_linked_list_null(list)){
        fprintf(stderr, "You tried to access a NULL linked list tail");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }
    if(is_linked_list_empty(list)) 
        return NULL;
    return list->next;
}

size_t get_linked_list_size(LinkedList list){
    size_t size = 0;

    if (is_linked_list_null(list)){
        fprintf(stderr, "You tried to calculate length on a NULL linked list");
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
                        fprintf(stderr, "You tried to calculate length (recursive) on a NULL linked list");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }

    if(is_linked_list_empty(list)) return 0;
    else if(list->data!=NULL && list->next==NULL) return 1;
    else return 1 + get_linked_list_size_recursive(list->next);
}

LinkedList get_linked_list_last_element(LinkedList list){
    if (is_linked_list_null(list)){
        fprintf(stderr, "You requested last element from a NULL linked list");
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

void linked_list_push_back(LinkedList list, void* new_data){
    if (is_linked_list_null(list)){
        fprintf(stderr, "You tried to push back an element on a NULL linked list");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    } else if (is_linked_list_empty(list)){
        list->data=new_data;
        return;
    }

    LinkedList last_element = get_linked_list_last_element(list);

    if(is_linked_list_empty(last_element)){
        last_element->data=new_data;
        return;
    } else {
        LinkedList new_element = build_empty_linked_list();
        new_element->data = new_data;
        last_element->next = new_element;
    }
    
    return;
}

void linked_list_remove_last(LinkedList list){
    if (is_linked_list_null(list)){
        fprintf(stderr, "You tried to remove last element from a NULL linked list");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }
    else if (is_linked_list_empty(list))
        return;
    else if (list->data != NULL && list->next==NULL){
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

void linked_list_push_front(LinkedList list, void* new_data) {
    if (is_linked_list_null(list)) {
        fprintf(stderr, "You tried to push front an element on a NULL linked list");
        exit(ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST);
    }

    if (is_linked_list_empty(list)) {
        list->data = new_data;
        return;
    }


    LinkedListNode* shifted_node = (LinkedListNode*) malloc(sizeof(LinkedListNode));
    if (shifted_node == NULL) {
        fprintf(stderr, "Failed malloc while trying to push_front on linked list");
        exit(FAILED_LINKED_LIST_ALLOCATION);
    }

    /* Copy current head content in the new node */
    shifted_node->data = list->data;
    shifted_node->next = list->next;

    /* Overwrite head new data and link with new head */
    list->data = new_data;
    list->next = shifted_node;

    return;
}

void linked_list_remove_first(LinkedList list) {
    if (is_linked_list_null(list)) {
        fprintf(stderr, "You tried to remove first element from a NULL linked list");
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

    /* Special case: logically empty list */
    if (is_linked_list_empty(list)) {
        printf("[HEAD|EMPTY] -> NULL\n");
        return;
    }

    /* Caso generale: almeno un elemento utile */
    printf("[HEAD] ");

    LinkedList curr = list;
    size_t index = 0;

    while (curr != NULL) {

        /* print node in a "box" ASCII */
        printf("-> [#%zu | data=", index);

        if (curr->data == NULL) {
            /* data == NULL -> print "NULL" */
            printf("NULL");
        } else {
            /* data type specific callback */
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

