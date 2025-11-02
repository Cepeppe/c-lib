#ifndef LINKED_LIST_TEST_H
#define LINKED_LIST_TEST_H

#include <stddef.h>

/*
 * Linked list test suite header.
 *
 * Model recap:
 * - A LinkedList is always a valid pointer (never expected to be NULL in normal usage).
 * - An "empty list" is a single allocated node with data == NULL and next == NULL.
 * - Removing the only element does NOT free the head node, it just clears it.
 * - Removing elements in the middle/tail frees those nodes completely.
 *
 * The test suite checks construction, push/pop at both ends, size helpers,
 * index access, reverse(), and removal by index.
 *
 * run_all_linked_list_tests() runs all tests in order.
 * Each test_* can also be called individually for debugging.
 */

/* Runs all tests. Aborts on first assert failure. */
void run_all_linked_list_tests(void);

/* ------------------------
 * Individual tests
 * ------------------------ */

/*
 * test_build_and_empty_checks
 * - empty list allocation is non-NULL
 * - "empty vs NULL" semantics behave as expected
 * - size == 0, tail == NULL, last == head on empty list
 */
void test_build_and_empty_checks(void);

/*
 * test_push_back_and_size
 * - push_back on empty fills head->data
 * - appending more elements creates new nodes
 * - size (iter + recursive) updates correctly
 * - tail/last element tracking is correct
 */
void test_push_back_and_size(void);

/*
 * test_push_front_and_remove_first
 * - push_front on empty sets head
 * - push_front on non-empty shifts old head down
 * - remove_first frees head data and promotes next node
 * - removing again on single-node leaves an "empty" head node
 */
void test_push_front_and_remove_first(void);

/*
 * test_remove_last
 * - removing last element from multi-node list frees the tail node
 * - repeated remove_last shrinks list down to one node
 * - removing last element from a single-node list frees its data
 *   and turns the head into an empty node instead of freeing it
 * - calling remove_last on an already-empty list is a no-op
 */
void test_remove_last(void);

/*
 * test_get_at_index
 * - get_linked_list_at_index returns correct nodes for valid indices
 * - returns NULL for out-of-range
 */
void test_get_at_index(void);

/*
 * test_reverse
 * - reverse() inverts multi-node lists
 * - reverse() on single-node or empty returns the same pointer
 * - debug print doesn't crash after reverse
 */
void test_reverse(void);

/*
 * test_remove_at_index_scenarios
 * - linked_list_remove_at_index() on:
 *   * index 0 (removes head in-place, like remove_first)
 *   * middle index (unlink/free that node)
 *   * last index (unlink/free tail)
 *   * out-of-bounds (returns 0, no change)
 *   * single-element list (leaves an empty head node)
 *   * already-empty list (returns 0, no crash)
 *
 * Contract: returns 1 if something was removed, 0 otherwise.
 */
void test_remove_at_index_scenarios(void);

/*
 * test_tail_and_last_consistency
 * - On empty list:
 *   tail() == NULL, last_element() == head pointer
 * - On single-element list:
 *   same invariants (tail NULL, last=head)
 * - After appends:
 *   last_element() tracks the true last node
 */
void test_tail_and_last_consistency(void);

#endif /* LINKED_LIST_TEST_H */
