#ifndef LINKED_LIST_H
#define LINKED_LIST_H
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST -97
#define FAILED_LINKED_LIST_ALLOCATION -98

/*
    DESIGN CHOICE: 
    
    An empty linked list IS a list having the first element initialized but no data and no next (i.e. == NULL).
    A LinkedList == NULL IS NOT AN EMPTY LINKED LIST
    It is the PROGRAMMER RESPONSABILITY TO INITIALIZE THE LINKED LIST

    IMPORTANT:
    Every node->data MUST point to heap memory obtained via malloc/calloc/realloc.
    The list will free node->data when elements are removed or destroyed.
    Do NOT store addresses of stack variables or globals in node->data.
  
*/

/* Singly linked list node */
typedef struct LinkedListNode{
    void* data;                    /* user data pointer */
    struct LinkedListNode * next;  /* next node, or NULL */
} LinkedListNode;

/* A list is a pointer to the first node*/
typedef LinkedListNode* LinkedList;

/* Create a new node with given data and next */
LinkedList build_linked_list(void* data, LinkedList next);

/* Return an empty list*/
LinkedList build_empty_linked_list();

/* Return 1 if list is empty, else 0 */
int is_linked_list_empty(LinkedList list);

/* Returns 1 if list is null, 0 otherwise */
int is_linked_list_null(LinkedList list);

/* 
    Free data field for a single LinkedListNode
    IMPORTANT: save and reconnect element before and after to avoid breaking the list 
*/
void free_linked_list_node_data(LinkedListNode* node);

/* Free LinkedListNode (calls free_linked_list_node_data)*/
void free_linked_list_node(LinkedListNode* node);

/* Get data from the first node, or NULL if list is empty */
void* get_linked_list_head_data(LinkedList list);

/* 
    Get the tail (list without the first node) 
    CAN RETURN NULL IF LIST HAS NO TAIL
*/
LinkedList get_linked_list_tail(LinkedList list);

/* Count how many nodes are in the list. Iterative */
size_t get_linked_list_size(LinkedList list);

/* Count how many nodes are in the list. Mind the possibility of stack explosion */
size_t get_linked_list_size_recursive(LinkedList list);

/* 
    Return pointer to last linked list element 
    If list is empty returns the list itself 
*/
LinkedList get_linked_list_last_element(LinkedList list);

/* Append new_data at the end of the list */
void linked_list_push_back(LinkedList list, void* new_data);

/* Remove last element*/
void linked_list_remove_last(LinkedList list);

/* Append new_data at the beginning of the list */
void linked_list_push_front(LinkedList list, void* new_data);

/* Remove first element*/
void linked_list_remove_first(LinkedList list);

/* Frees linked list memory */
void linked_list_destroy(LinkedList list);

/* Frees linked list memory */
LinkedListNode* get_linked_list_at_index(LinkedList list, size_t index);

/* Removes element at index (if present)*/
int linked_list_remove_at_index(LinkedList list, size_t index);

/* Returns a new linked list reversed in-place*/
LinkedList linked_list_reverse(LinkedList list);

/* Debug print*/
void linked_list_debug_print(LinkedList list, void (*print_data)(void*));

#endif
