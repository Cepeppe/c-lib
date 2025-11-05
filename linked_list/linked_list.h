#ifndef LINKED_LIST_H
#define LINKED_LIST_H
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST -97
#define FAILED_LINKED_LIST_ALLOCATION       -98

/*
    DESIGN CHOICEs: 
    
    Empty vs NULL
    -An empty linked list is a list having the first element initialized 
        but no data and no next (i.e. == NULL).
    -A LinkedList == NULL IS NOT AN EMPTY LINKED LIST
        Every node->data MUST point to heap memory obtained via malloc/calloc/realloc.
    It is the PROGRAMMER RESPONSABILITY TO INITIALIZE AND DESTROY THE LINKED LIST

    Linked list IS OWNER OF DATA
    -When inserting data into linked list, it becomes
        owner of the data and frees it when removed 
        (or linked list is destroyed). 
    
    DO NOT FREE DATA THAT WAS PUT INTO LINKED LIST
    Do NOT store addresses of stack variables or globals in node->data.
  
*/

/* linked list node */
typedef struct LinkedListNode{
    void* data;                    /* data pointer */
    struct LinkedListNode * next;  /* next node, or NULL */
} LinkedListNode;

/* A list is a pointer to the first node*/
typedef LinkedListNode* LinkedList;

/* Build an empty list (allocates head node, sets data/next to NULL) */
LinkedList build_empty_linked_list(void);

/* Return 1 if the list is logically empty (head allocated but no data/next), else 0 */
int is_linked_list_empty(LinkedList list);

/* Return 1 if the list pointer itself is NULL, else 0 */
int is_linked_list_null(LinkedList list);

/* Deep-free only node->data via callback, then null it out */
void free_linked_list_node_data(LinkedListNode* node, void (*deep_deallocate_node_data)(void* node_data));

/* Get pointer to head data; returns NULL if the list is empty */
void* get_linked_list_head_data(LinkedList list);

/* Get the tail pointer (head->next); returns NULL if empty */
LinkedList get_linked_list_tail(LinkedList list);

/* Compute number of nodes; returns 0 for an empty list */
size_t get_linked_list_size(LinkedList list);

/* Return pointer to last node; returns NULL if the list is empty */
LinkedListNode* get_linked_list_last_element(LinkedList list);

/* Append at end; fills head if list is empty (no new node allocated) */
void linked_list_push_back(LinkedList list, void* data);

/* Remove last node without freeing payload (data remains allocated) */
void linked_list_remove_last(LinkedList list);

/* Remove last node and deep-free its payload via callback */
void linked_list_remove_last_with(LinkedList list, void (*deep_deallocate_node_data)(void* node_data));

/* Push to front; allocates a new node and shifts previous head to second */
void linked_list_push_front(LinkedList list, void* data);

/* Remove first node and deep-free its payload via callback */
void linked_list_remove_first_with(LinkedList list, void (*deep_deallocate_node_data)(void* node_data));

/* Remove first node without freeing payload (data remains allocated) */
void linked_list_remove_first(LinkedList list);

/* Destroy the entire list without freeing payloads (only nodes) */
void linked_list_destroy(LinkedList list);

/* Destroy the entire list deep-freeing each payload via callback */
void linked_list_destroy_with(LinkedList list, void (*deep_deallocate_node_data)(void* node_data));

/* Remove the node immediately after the given node (no deep free) */
void linked_list_remove_next_node(LinkedListNode* node);

/* Remove the node after the given node and deep-free its data via callback */
void linked_list_remove_next_node_with(LinkedListNode* node, void (*deep_deallocate_node_data)(void* node_data));

/* 
    Free di un nodo *già scollegato* che contiene un HashMapItem:
    libera prima HashMapItem->data con deep_deallocate_hashmap_item_data,
    poi l’HashMapItem con deep_deallocate_hashmap_item, infine il nodo.
    PRE: 'node' non è NULL ed è già detach (nessun prev->next che lo punti).

    -deep_deallocate_hashmap_item_data:  Can be null if HashMapItem->data is a primitive type and was not allocated via malloc
*/

void linked_list_remove_hashmap_node_with(
    LinkedListNode* node,
    void (*deep_deallocate_hashmap_item)(void* item, void (*deep_deallocate_hashmap_item_data)(void*)),
    void (*deep_deallocate_hashmap_item_data)(void* data)
);

/* Debug print of the list using a user-provided payload printer */
void linked_list_debug_print(LinkedList list, void (*print_data)(void*));

#endif
