#ifndef LINKED_LIST_TEST_H
#define LINKED_LIST_TEST_H

#include <stddef.h>

/*
    Linked list test suite header.

    Each test_* function focuses on a specific behavior of the linked list:
    - construction and "empty vs NULL" semantics
    - push_back / push_front logic
    - remove_first / remove_last logic
    - size counting (iterative and recursive)
    - index access
    - in-place reverse

    run_all_linked_list_tests() executes all tests in sequence.

    The main() function (in linked_list_test.c) will call run_all_linked_list_tests().
*/

/* Run the full test suite.
   If any assert fails, the program aborts. */
void run_all_linked_list_tests(void);

/* Individual tests are also exposed so you can call them separately
   from a debugger, if you want to isolate failures. */
void test_build_and_empty_checks(void);
void test_push_back_and_size(void);
void test_push_front_and_remove_first(void);
void test_remove_last(void);
void test_get_at_index(void);
void test_reverse(void);

#endif /* LINKED_LIST_TEST_H */
