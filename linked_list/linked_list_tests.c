#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "linked_list.h"
#include "linked_list_tests.h"
#include "../memory/ll_alloc_helpers.h"


/* ============================================================
   Internal helpers (test-only)
   ============================================================ */

/*
 * Callback for linked_list_debug_print().
 * Assumes node->data points to an int allocated on the heap.
 */
static void print_int(void* data) {
    if (data == NULL) {
        printf("NULL");
    } else {
        printf("%d", *(int*)data);
    }
}

/*
 * Build a linked list from an array of ints.
 *
 * For each value, we allocate heap memory using ll_alloc_int(value)
 * and push it to the back.
 *
 * Returns a non-NULL LinkedList. Even if n == 0, we return an
 * "empty logical list" from build_empty_linked_list().
 */
static LinkedList build_list_from_int_array(const int* values, size_t n) {
    LinkedList list = build_empty_linked_list();
    for (size_t i = 0; i < n; i++) {
        void* heap_num = ll_alloc_int(values[i]);
        linked_list_push_back(list, heap_num);
    }
    return list;
}

/*
 * Helper that retrieves the int at a given index using
 * get_linked_list_at_index(). This asserts expectations.
 *
 * If should_exist == 1:
 *   - Asserts that the node exists and data is not NULL
 *   - Returns the int value
 *
 * If should_exist == 0:
 *   - Asserts that the node is NULL (index is out of bounds)
 *   - Returns 0 (ignored by caller)
 */
static int get_int_at_index_or_fail(LinkedList list, size_t index, int should_exist) {
    LinkedListNode* node = get_linked_list_at_index(list, index);
    if (!should_exist) {
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
   - that list is logically empty (is_linked_list_empty == 1)
   - is_linked_list_null distinguishes NULL vs empty
   - head data is NULL, tail on empty is NULL
   - size is 0 (iterative and recursive)
   - get_linked_list_last_element() on empty returns same pointer
   ============================================================ */
void test_build_and_empty_checks(void) {
    printf("[TEST] test_build_and_empty_checks...\n");

    LinkedList l = build_empty_linked_list();

    /* Must be allocated */
    assert(l != NULL);

    /* Logically empty by design */
    assert(is_linked_list_empty(l) == 1);
    assert(is_linked_list_null(l) == 0);

    /* Head data of empty list is NULL */
    assert(get_linked_list_head_data(l) == NULL);

    /* Tail of an empty list is NULL */
    assert(get_linked_list_tail(l) == NULL);

    /* Size must be 0 */
    assert(get_linked_list_size(l) == 0);
    assert(get_linked_list_size_recursive(l) == 0);

    /* last element of empty list should be itself */
    LinkedList last = get_linked_list_last_element(l);
    assert(last == l);
    assert(is_linked_list_empty(last) == 1);

    linked_list_destroy(l);

    printf("[OK]  test_build_and_empty_checks\n");
}


/* ============================================================
   TEST 2: push_back and size
   We verify:
   - pushing into an empty logical list should populate its head
   - size updates correctly
   - last element logic
   - pushing a second element appends a new node
   ============================================================ */
void test_push_back_and_size(void) {
    printf("[TEST] test_push_back_and_size...\n");

    LinkedList l = build_empty_linked_list();

    /* Push first value */
    linked_list_push_back(l, ll_alloc_int(10));

    assert(is_linked_list_empty(l) == 0);
    assert(get_linked_list_size(l) == 1);
    assert(get_linked_list_size_recursive(l) == 1);

    /* Head data must be 10 */
    assert(get_linked_list_head_data(l) != NULL);
    assert(*(int*)get_linked_list_head_data(l) == 10);

    /* With only one node, last element is the head */
    LinkedList last = get_linked_list_last_element(l);
    assert(last == l);
    assert(*(int*)last->data == 10);

    /* Push second value */
    linked_list_push_back(l, ll_alloc_int(20));

    assert(get_linked_list_size(l) == 2);
    assert(get_linked_list_size_recursive(l) == 2);

    /* Tail (head->next) should be 20 */
    LinkedList tail = get_linked_list_tail(l);
    assert(tail != NULL);
    assert(*(int*)tail->data == 20);
    assert(tail->next == NULL);

    /* "last element" should be same tail */
    LinkedList last2 = get_linked_list_last_element(l);
    assert(last2 != NULL);
    assert(*(int*)last2->data == 20);
    assert(last2->next == NULL);

    linked_list_destroy(l);

    printf("[OK]  test_push_back_and_size\n");
}


/* ============================================================
   TEST 3: push_front and remove_first
   We verify:
   - push_front() on empty fills head->data
   - push_front() again shifts old head down (creates a new node)
   - remove_first() frees old head data and promotes the second node
   - removing again on a single-node list leaves a logically empty list
   ============================================================ */
void test_push_front_and_remove_first(void) {
    printf("[TEST] test_push_front_and_remove_first...\n");

    LinkedList l = build_empty_linked_list();

    linked_list_push_front(l, ll_alloc_int(1));
    /* list: [1] */

    assert(is_linked_list_empty(l) == 0);
    assert(get_linked_list_size(l) == 1);
    assert(*(int*)get_linked_list_head_data(l) == 1);

    linked_list_push_front(l, ll_alloc_int(2));
    /* list: [2] -> [1] */

    assert(get_linked_list_size(l) == 2);
    assert(*(int*)get_linked_list_head_data(l) == 2);

    LinkedList second_node = get_linked_list_tail(l);
    assert(second_node != NULL);
    assert(*(int*)second_node->data == 1);

    /* remove_first():
       - frees old head data (2)
       - promotes second node (1) into head */
    linked_list_remove_first(l);
    /* list now: [1] */

    assert(get_linked_list_size(l) == 1);
    assert(*(int*)get_linked_list_head_data(l) == 1);

    /* remove_first() again on single-node:
       list becomes logically empty (data=NULL,next=NULL) */
    linked_list_remove_first(l);
    assert(is_linked_list_empty(l) == 1);
    assert(get_linked_list_size(l) == 0);

    linked_list_destroy(l);

    printf("[OK]  test_push_front_and_remove_first\n");
}


/* ============================================================
   TEST 4: remove_last
   We verify:
   - Start with [5]->[6]->[7]
   - remove_last() drops the tail
   - Eventually removing last element leaves a logically empty list
   - Calling remove_last() again on empty is a no-op
   ============================================================ */
void test_remove_last(void) {
    printf("[TEST] test_remove_last...\n");

    LinkedList l = build_empty_linked_list();

    linked_list_push_back(l, ll_alloc_int(5));
    linked_list_push_back(l, ll_alloc_int(6));
    linked_list_push_back(l, ll_alloc_int(7));
    /* list: [5] -> [6] -> [7] */

    assert(get_linked_list_size(l) == 3);

    /* Remove last (7) */
    linked_list_remove_last(l);
    assert(get_linked_list_size(l) == 2);

    LinkedList last_after_1 = get_linked_list_last_element(l);
    assert(last_after_1 != NULL);
    assert(*(int*)last_after_1->data == 6);
    assert(last_after_1->next == NULL);

    /* Remove last again (6) */
    linked_list_remove_last(l);
    assert(get_linked_list_size(l) == 1);

    LinkedList last_after_2 = get_linked_list_last_element(l);
    assert(last_after_2 != NULL);
    assert(*(int*)last_after_2->data == 5);
    assert(last_after_2->next == NULL);

    /* Remove last again (5).
       Implementation: turns list into logically empty
       (data=NULL, next=NULL) instead of freeing the head. */
    linked_list_remove_last(l);
    assert(is_linked_list_empty(l) == 1);
    assert(get_linked_list_size(l) == 0);

    /* Calling remove_last() again should do nothing, not crash */
    linked_list_remove_last(l);
    assert(is_linked_list_empty(l) == 1);
    assert(get_linked_list_size(l) == 0);

    linked_list_destroy(l);

    printf("[OK]  test_remove_last\n");
}


/* ============================================================
   TEST 5: get_linked_list_at_index
   We verify:
   - On [100]->[200]->[300], indices 0..2 exist
   - Index 3 is out of range and returns NULL
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

    /* 3 is out of bounds */
    (void)get_int_at_index_or_fail(l, 3, 0);

    linked_list_destroy(l);

    printf("[OK]  test_get_at_index\n");
}


/* ============================================================
   TEST 6: reverse
   We verify:
   - Reverse [10]->[20]->[30] => [30]->[20]->[10]
   - Reverse single-element list returns same head
   - Reverse logically empty list returns same head
   - linked_list_debug_print doesn't crash
   ============================================================ */
void test_reverse(void) {
    printf("[TEST] test_reverse...\n");

    /* Case A: multi-node reverse */
    {
        const int vals[3] = {10, 20, 30};
        LinkedList l = build_list_from_int_array(vals, 3);

        LinkedList reversed = linked_list_reverse(l);
        /* After reverse: expected order 30 -> 20 -> 10 */

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

        /* Just sanity: shouldn't crash */
        linked_list_debug_print(reversed, print_int);

        linked_list_destroy(reversed);
    }

    /* Case B: single-element reverse */
    {
        LinkedList single = build_empty_linked_list();
        linked_list_push_back(single, ll_alloc_int(42));
        assert(get_linked_list_size(single) == 1);

        LinkedList reversed_single = linked_list_reverse(single);
        /* Should be same pointer */
        assert(reversed_single == single);
        assert(get_linked_list_size(reversed_single) == 1);
        assert(*(int*)get_linked_list_head_data(reversed_single) == 42);

        linked_list_destroy(reversed_single);
    }

    /* Case C: reverse on logically empty list */
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
   TEST 7: remove_at_index (NEW)
   We verify:
   - Removing index 0 updates head value in-place via linked_list_remove_first
     and does NOT change the head pointer.
   - Removing a middle index unlinks that node.
   - Removing the last valid index works.
   - Removing an out-of-bounds index returns 0 and does nothing.
   - Eventually removing until empty turns the list into a logically
     empty head node (data=NULL, next=NULL).
   - Calling again on an empty list returns 0 and does not crash.
   ============================================================ */
void test_remove_at_index_scenarios(void) {
    printf("[TEST] test_remove_at_index_scenarios...\n");

    /* Start with [10] -> [20] -> [30] -> [40] */
    const int vals[4] = {10, 20, 30, 40};
    LinkedList l = build_list_from_int_array(vals, 4);

    assert(get_linked_list_size(l) == 4);
    assert(get_linked_list_size_recursive(l) == 4);

    /* Keep original head pointer to ensure it does not change
       after removing index 0. */
    LinkedList original_head = l;

    /* --- Remove index 0 (head) --- */
    {
        int rc = linked_list_remove_at_index(l, 0);
        assert(rc == 1);

        /* Size should now be 3 */
        assert(get_linked_list_size(l) == 3);
        assert(get_linked_list_size_recursive(l) == 3);

        /* Head pointer should be unchanged (in-place removal semantics) */
        assert(l == original_head);

        /* New head data should be 20 (the old second element) */
        assert(get_linked_list_head_data(l) != NULL);
        assert(*(int*)get_linked_list_head_data(l) == 20);

        /* Check order now: [20] -> [30] -> [40] */
        assert(get_int_at_index_or_fail(l, 0, 1) == 20);
        assert(get_int_at_index_or_fail(l, 1, 1) == 30);
        assert(get_int_at_index_or_fail(l, 2, 1) == 40);
        (void)get_int_at_index_or_fail(l, 3, 0); /* out of range */
    }

    /* --- Remove middle index (1) ---
       Current list is [20] -> [30] -> [40].
       Removing index 1 should remove "30" -> left [20] -> [40].
    */
    {
        int rc = linked_list_remove_at_index(l, 1);
        assert(rc == 1);

        assert(get_linked_list_size(l) == 2);
        assert(get_linked_list_size_recursive(l) == 2);

        /* Now expect: [20] -> [40] */
        assert(get_int_at_index_or_fail(l, 0, 1) == 20);
        assert(get_int_at_index_or_fail(l, 1, 1) == 40);
        (void)get_int_at_index_or_fail(l, 2, 0);
    }

    /* --- Remove out of bounds ---
       Try to remove index 5 (doesn't exist).
       Should return 0 and not modify.
    */
    {
        int rc = linked_list_remove_at_index(l, 5);
        assert(rc == 0);

        /* Still [20] -> [40] */
        assert(get_linked_list_size(l) == 2);
        assert(get_linked_list_size_recursive(l) == 2);
        assert(get_int_at_index_or_fail(l, 0, 1) == 20);
        assert(get_int_at_index_or_fail(l, 1, 1) == 40);
    }

    /* --- Remove last valid index ---
       Current list is [20] -> [40].
       Removing index 1 should remove "40", leaving [20].
    */
    {
        int rc = linked_list_remove_at_index(l, 1);
        assert(rc == 1);

        assert(get_linked_list_size(l) == 1);
        assert(get_linked_list_size_recursive(l) == 1);

        assert(get_int_at_index_or_fail(l, 0, 1) == 20);
        (void)get_int_at_index_or_fail(l, 1, 0);

        /* Tail should be NULL after head->next */
        LinkedList tail_now = get_linked_list_tail(l);
        assert(tail_now == NULL);
    }

    /* --- Remove index 0 again (single element) ---
       Now list is [20] with no next.
       Removing index 0 should make the list logically empty
       (data=NULL, next=NULL), not free the head pointer.
    */
    {
        int rc = linked_list_remove_at_index(l, 0);
        assert(rc == 1);

        assert(is_linked_list_empty(l) == 1);
        assert(get_linked_list_size(l) == 0);
        assert(get_linked_list_size_recursive(l) == 0);

        /* If we ask for index 0 now, it's out of range */
        (void)get_int_at_index_or_fail(l, 0, 0);

        /* Tail of a logically empty list should be NULL */
        assert(get_linked_list_tail(l) == NULL);
    }

    /* --- Remove from an already empty logical list ---
       Should return 0 and not crash.
    */
    {
        int rc = linked_list_remove_at_index(l, 0);
        assert(rc == 0);

        assert(is_linked_list_empty(l) == 1);
        assert(get_linked_list_size(l) == 0);
    }

    linked_list_destroy(l);

    printf("[OK]  test_remove_at_index_scenarios\n");
}


/* ============================================================
   TEST 8: general tail/last consistency on small lists
   (extra sanity test for edge cases)
   We verify:
   - On a single-element list, tail() == NULL but last_element == head
   - On empty logical list, tail() == NULL and last_element == head
   ============================================================ */
void test_tail_and_last_consistency(void) {
    printf("[TEST] test_tail_and_last_consistency...\n");

    /* Case 1: logically empty */
    {
        LinkedList l = build_empty_linked_list();
        assert(is_linked_list_empty(l) == 1);

        assert(get_linked_list_tail(l) == NULL);

        LinkedList last = get_linked_list_last_element(l);
        assert(last == l);                 /* same pointer */
        assert(is_linked_list_empty(last));

        linked_list_destroy(l);
    }

    /* Case 2: single element [99] */
    {
        LinkedList l = build_empty_linked_list();
        linked_list_push_back(l, ll_alloc_int(99));

        assert(is_linked_list_empty(l) == 0);
        assert(get_linked_list_size(l) == 1);

        /* tail of single-node list is NULL (head->next == NULL) */
        assert(get_linked_list_tail(l) == NULL);

        /* last element should be the head itself */
        LinkedList last = get_linked_list_last_element(l);
        assert(last == l);
        assert(last->next == NULL);
        assert(*(int*)last->data == 99);

        linked_list_destroy(l);
    }

    printf("[OK]  test_tail_and_last_consistency\n");
}


/* ============================================================
   Global runner
   ============================================================ */
void run_all_linked_list_tests(void) {
    test_build_and_empty_checks();
    test_push_back_and_size();
    test_push_front_and_remove_first();
    test_remove_last();
    test_get_at_index();
    test_reverse();
    test_remove_at_index_scenarios();
    test_tail_and_last_consistency();
}
