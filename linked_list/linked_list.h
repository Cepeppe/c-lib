#ifndef LINKED_LIST_H
#define LINKED_LIST_H
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define ATTEMPTED_ACCESS_TO_NULL_LINKED_LIST -97
#define FAILED_LINKED_LIST_ALLOCATION -98

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


#endif