#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "linked_list.h"
#include "linked_list_tests.h"

/* ============================================================
   Internal helpers (not part of the public API)
   ============================================================ */

/* Allocates an int on the heap and initializes it.
   IMPORTANT:
   Your linked list implementation assumes that node->data points
   to heap memory that can safely be freed() later.
   So we cannot just take the address of a stack int. */
static int* alloc_int(int value) {
    int* p = (int*) malloc(sizeof(int));
    assert(p != NULL && "malloc failed in alloc_int");
    *p = value;
    return p;
}

/* Function used as a callback for linked_list_debug_print().
   Prints integer data if present, or "NULL" if data == NULL. */
static void print_int(void* data) {
    if (data == NULL) {
        printf("NULL");
    } else {
        printf("%d", *(int*)data);
    }
}

/* Build a linked list from an array of ints.
   For each element in 'values', we allocate an int on the heap
   and push it to the back of the list.

   Returns the head of the list (which is NEVER NULL; at minimum it's
   an "empty logical list" created via build_empty_linked_list()).
*/
static LinkedList build_list_from_int_array(const int* values, size_t n) {
    LinkedList list = build_empty_linked_list();
    for (size_t i = 0; i < n; i++) {
        int* heap_num = alloc_int(values[i]);
        linked_list_push_back(list, heap_num);
    }
    return list;
}

/* Helper that retrieves the integer value at the given index using
   get_linked_list_at_index(). This function performs assertions about
   whether the index should exist or not.

   - If should_exist == 1, we assert that the node exists and has data.
   - If should_exist == 0, we assert that the node is NULL (index out of range).

   Returns the integer at that index if should_exist == 1.
   Returns 0 otherwise (unimportant).
*/
static int get_int_at_index_or_fail(LinkedList list, size_t index, int should_exist) {
    LinkedListNode* node = get_linked_list_at_index(list, index);
    if (!should_exist) {
        /* We expect the index NOT to exist. */
        assert(node == NULL);
        return 0;
    } else {
        assert(node != NULL);
        assert(node->data != NULL);
        return *(int*)(node->data);
    }
}

/* ============================================================
   TEST 1: creation and base invariants
   What we verify:
   - build_empty_linked_list() returns a non-NULL pointer
   - but that list is "logically empty" according to is_linked_list_empty()
   - is_linked_list_null() must distinguish NULL vs empty
   - head data and tail on empty list are consistent
   - size (iterative and recursive) on empty list is 0
   - get_linked_list_last_element() on an empty list returns the same node
   ============================================================ */
void test_build_and_empty_checks(void) {
    printf("[TEST] test_build_and_empty_checks...\n");

    LinkedList l = build_empty_linked_list();

    /* The list must be allocated => not NULL */
    assert(l != NULL);

    /* BUT by design it's considered "logically empty" */
    assert(is_linked_list_empty(l) == 1);
    assert(is_linked_list_null(l) == 0);

    /* get_linked_list_head_data() should return NULL for an empty logical list */
    assert(get_linked_list_head_data(l) == NULL);

    /* Tail of an empty logical list should be NULL */
    assert(get_linked_list_tail(l) == NULL);

    /* Size should be 0 in both iterative and recursive versions */
    assert(get_linked_list_size(l) == 0);
    assert(get_linked_list_size_recursive(l) == 0);

    /* get_linked_list_last_element() on an empty list
       should return the list itself (same pointer). */
    LinkedList last = get_linked_list_last_element(l);
    assert(last == l);
    assert(is_linked_list_empty(last) == 1);

    /* cleanup: destroy the list and free memory */
    linked_list_destroy(l);

    printf("[OK]  test_build_and_empty_checks\n");
}

/* ============================================================
   TEST 2: push_back and size
   What we verify:
   - pushing into an empty logical list should populate its head
   - size updates correctly
   - tail and last element logic
   - pushing a second element creates a new node at the end
   ============================================================ */
void test_push_back_and_size(void) {
    printf("[TEST] test_push_back_and_size...\n");

    LinkedList l = build_empty_linked_list();

    int* a = alloc_int(10);
    linked_list_push_back(l, a);

    /* After first push_back, list is no longer "logically empty" */
    assert(is_linked_list_empty(l) == 0);
    assert(get_linked_list_size(l) == 1);
    assert(get_linked_list_size_recursive(l) == 1);

    /* Head data must be 10 */
    assert(get_linked_list_head_data(l) != NULL);
    assert(*(int*)get_linked_list_head_data(l) == 10);

    /* With only one meaningful node, last element should be the head itself */
    LinkedList last = get_linked_list_last_element(l);
    assert(last == l);
    assert(*(int*)last->data == 10);

    /* Push a second value at the back */
    int* b = alloc_int(20);
    linked_list_push_back(l, b);

    assert(get_linked_list_size(l) == 2);
    assert(get_linked_list_size_recursive(l) == 2);

    /* Tail of the list should now be a valid node with value 20 */
    LinkedList tail = get_linked_list_tail(l);
    assert(tail != NULL);
    assert(*(int*)tail->data == 20);
    assert(tail->next == NULL);

    /* And the "last element" must also be that same node */
    LinkedList last2 = get_linked_list_last_element(l);
    assert(last2 != NULL);
    assert(*(int*)last2->data == 20);
    assert(last2->next == NULL);

    /* cleanup */
    linked_list_destroy(l);

    printf("[OK]  test_push_back_and_size\n");
}

/* ============================================================
   TEST 3: push_front and remove_first
   What we verify:
   - push_front() on an empty logical list just fills head->data
   - push_front() again on a non-empty list creates a new head
     by shifting the old head down
   - remove_first():
        * frees the old head's data
        * "promotes" the second node to become the new head
   - removing again on a single-node list resets it to a
     logically empty list (data=NULL, next=NULL)
   ============================================================ */
void test_push_front_and_remove_first(void) {
    printf("[TEST] test_push_front_and_remove_first...\n");

    LinkedList l = build_empty_linked_list();

    int* a = alloc_int(1);
    linked_list_push_front(l, a);
    /* list is now: [1] */

    assert(is_linked_list_empty(l) == 0);
    assert(get_linked_list_size(l) == 1);
    assert(*(int*)get_linked_list_head_data(l) == 1);

    int* b = alloc_int(2);
    linked_list_push_front(l, b);
    /* list is now: [2] -> [1] */

    assert(get_linked_list_size(l) == 2);
    assert(*(int*)get_linked_list_head_data(l) == 2);

    LinkedList second_node = get_linked_list_tail(l);
    assert(second_node != NULL);
    assert(*(int*)second_node->data == 1);

    /* Now remove_first():
       - frees old head data (2)
       - moves second node (1) into head */
    linked_list_remove_first(l);
    /* list should now be: [1] */

    assert(get_linked_list_size(l) == 1);
    assert(*(int*)get_linked_list_head_data(l) == 1);

    /* Remove again on single-node list:
       The list should become logically empty (data=NULL, next=NULL) */
    linked_list_remove_first(l);
    assert(is_linked_list_empty(l) == 1);
    assert(get_linked_list_size(l) == 0);

    /* cleanup */
    linked_list_destroy(l);

    printf("[OK]  test_push_front_and_remove_first\n");
}

/* ============================================================
   TEST 4: remove_last
   What we verify:
   - Start with 3 elements
   - remove_last() should:
        * free and detach the final node
        * update size and last element pointer
   - Eventually, when only one logical element remains,
     remove_last() should clear its data and leave a single
     "logically empty" node (data=NULL,next=NULL)
   - Calling remove_last() again on a logically empty list
     should do nothing and not crash
   ============================================================ */
void test_remove_last(void) {
    printf("[TEST] test_remove_last...\n");

    LinkedList l = build_empty_linked_list();

    linked_list_push_back(l, alloc_int(5));
    linked_list_push_back(l, alloc_int(6));
    linked_list_push_back(l, alloc_int(7));
    /* list: [5] -> [6] -> [7] */

    assert(get_linked_list_size(l) == 3);

    /* Remove last element (7) */
    linked_list_remove_last(l);
    assert(get_linked_list_size(l) == 2);

    LinkedList last_after_1 = get_linked_list_last_element(l);
    assert(last_after_1 != NULL);
    assert(*(int*)last_after_1->data == 6);
    assert(last_after_1->next == NULL);

    /* Remove last element again (6) */
    linked_list_remove_last(l);
    assert(get_linked_list_size(l) == 1);

    LinkedList last_after_2 = get_linked_list_last_element(l);
    assert(last_after_2 != NULL);
    assert(*(int*)last_after_2->data == 5);
    assert(last_after_2->next == NULL);

    /* Remove last element again (5).
       Your implementation does:
       - if there's only one node left, it sets node->data = NULL
         and leaves the node in place (making list logically empty).
       It does NOT free() that final node. */
    linked_list_remove_last(l);
    assert(is_linked_list_empty(l) == 1);
    assert(get_linked_list_size(l) == 0);

    /* Calling remove_last() again on an already empty logical list
       should be a no-op (and must not crash). */
    linked_list_remove_last(l);
    assert(is_linked_list_empty(l) == 1);
    assert(get_linked_list_size(l) == 0);

    /* cleanup */
    linked_list_destroy(l);

    printf("[OK]  test_remove_last\n");
}

/* ============================================================
   TEST 5: get_linked_list_at_index
   What we verify:
   - For a list with 3 elements, indices 0,1,2 must exist
     and return correct values.
   - An out-of-range index returns NULL.
   ============================================================ */
void test_get_at_index(void) {
    printf("[TEST] test_get_at_index...\n");

    const int vals[3] = {100, 200, 300};
    LinkedList l = build_list_from_int_array(vals, 3);
    /* list: [100] -> [200] -> [300] */

    int v0 = get_int_at_index_or_fail(l, 0, 1);
    int v1 = get_int_at_index_or_fail(l, 1, 1);
    int v2 = get_int_at_index_or_fail(l, 2, 1);

    assert(v0 == 100);
    assert(v1 == 200);
    assert(v2 == 300);

    /* Index 3 is out of range -> should return NULL */
    (void)get_int_at_index_or_fail(l, 3, 0);

    /* cleanup */
    linked_list_destroy(l);

    printf("[OK]  test_get_at_index\n");
}

/* ============================================================
   TEST 6: reverse
   What we verify:
   - Reversing a multi-node list should flip the order
   - Reversing a single-node list should return the same head
   - Reversing a logically empty list should return the same head
   - After reversing, we can still destroy the list safely
   - We call linked_list_debug_print() once to sanity check
     it doesn't crash on a normal list
   ============================================================ */
void test_reverse(void) {
    printf("[TEST] test_reverse...\n");

    /* Case 1: reverse [10] -> [20] -> [30] => [30] -> [20] -> [10] */
    {
        const int vals[3] = {10, 20, 30};
        LinkedList l = build_list_from_int_array(vals, 3);

        /* reverse() returns the NEW head.
           IMPORTANT: you MUST reassign the variable. */
        LinkedList reversed = linked_list_reverse(l);

        /* Check order: 30, 20, 10 */
        assert(get_linked_list_size(reversed) == 3);

        LinkedList cur = reversed;

        assert(cur != NULL);
        assert(*(int*)cur->data == 30);
        cur = cur->next;

        assert(cur != NULL);
        assert(*(int*)cur->data == 20);
        cur = cur->next;

        assert(cur != NULL);
        assert(*(int*)cur->data == 10);
        cur = cur->next;

        assert(cur == NULL);

        /* Debug print should not crash */
        linked_list_debug_print(reversed, print_int);

        /* Cleanup:
           After reversing, 'reversed' IS the correct head.
           'l' is now the former tail, so do NOT destroy 'l' separately.
           Just destroy 'reversed'. */
        linked_list_destroy(reversed);
    }

    /* Case 2: reverse a single-element list */
    {
        LinkedList single = build_empty_linked_list();
        linked_list_push_back(single, alloc_int(42));
        assert(get_linked_list_size(single) == 1);

        LinkedList reversed_single = linked_list_reverse(single);
        /* With only one node, reverse() must return the same pointer */
        assert(reversed_single == single);
        assert(get_linked_list_size(reversed_single) == 1);
        assert(*(int*)get_linked_list_head_data(reversed_single) == 42);

        linked_list_destroy(reversed_single);
    }

    /* Case 3: reverse a logically empty list */
    {
        LinkedList empty = build_empty_linked_list();
        assert(is_linked_list_empty(empty) == 1);

        LinkedList reversed_empty = linked_list_reverse(empty);
        assert(reversed_empty == empty);
        assert(is_linked_list_empty(reversed_empty) == 1);

        linked_list_destroy(reversed_empty);
    }

    printf("[OK]  test_reverse\n");
}

/* ============================================================
   Global runner and main()
   ============================================================ */

/* Runs all tests in a "reasonable" order.
   If any assert fails, the process will abort.
   If we get to the end, all tests passed. */
void run_all_linked_list_tests(void) {
    test_build_and_empty_checks();
    test_push_back_and_size();
    test_push_front_and_remove_first();
    test_remove_last();
    test_get_at_index();
    test_reverse();
}

