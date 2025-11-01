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
