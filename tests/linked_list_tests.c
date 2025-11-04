#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "linked_list_tests.h"
#include "../linked_list/linked_list.h"

/* ---------------- Minimal test harness ---------------- */

static int g_passed = 0;
static int g_failed = 0;

#define TEST_OK()   do { g_passed++; } while(0)
#define TEST_FAIL(msg) do { g_failed++; fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, (msg)); } while(0)
#define EXPECT_TRUE(cond, msg) do { if ((cond)) TEST_OK(); else TEST_FAIL(msg); } while(0)
#define EXPECT_EQ_UINT(a,b,msg) do { \
    unsigned long _aa=(unsigned long)(a), _bb=(unsigned long)(b); \
    if (_aa == _bb) TEST_OK(); \
    else { char _m[256]; snprintf(_m, sizeof(_m), "%s (got=%lu, expected=%lu)", (msg), _aa, _bb); TEST_FAIL(_m);} \
} while(0)

/* ---------------- Helpers for "fatal" tests ----------------
 * Some LinkedList APIs intentionally exit(...) on NULL input.
 * We invoke those in a subprocess so the suite keeps running.
 */

static int run_fatal_subprocess(const char* self_path, const char* flag, const char* case_name) {
    char cmd[1024];
#if defined(_WIN32)
    /* Simple command construction; adjust quoting if needed in your environment */
    snprintf(cmd, sizeof(cmd), "\"%s\" %s %s", self_path, flag, case_name);
#else
    snprintf(cmd, sizeof(cmd), "\"%s\" %s %s", self_path, flag, case_name);
#endif
    int rc = system(cmd);
    return rc; /* non-zero expected (child should exit abnormally) */
}

/* When launched with "--ll-fatal <case>", run exactly one fatal call. */
static int handle_ll_fatal_case(int argc, char** argv) {
    if (argc == 3 && strcmp(argv[1], "--ll-fatal") == 0) {
        const char* which = argv[2];

        if (strcmp(which, "is_empty_null") == 0) {
            (void)is_linked_list_empty(NULL);
        } else if (strcmp(which, "head_data_null") == 0) {
            (void)get_linked_list_head_data(NULL);
        } else if (strcmp(which, "tail_null") == 0) {
            (void)get_linked_list_tail(NULL);
        } else if (strcmp(which, "size_null") == 0) {
            (void)get_linked_list_size(NULL);
        } else if (strcmp(which, "size_rec_null") == 0) {
            (void)get_linked_list_size_recursive(NULL);
        } else if (strcmp(which, "last_elem_null") == 0) {
            (void)get_linked_list_last_element(NULL);
        } else if (strcmp(which, "push_back_null") == 0) {
            linked_list_push_back(NULL, (void*)1);
        } else if (strcmp(which, "push_front_null") == 0) {
            linked_list_push_front(NULL, (void*)1);
        } else if (strcmp(which, "remove_last_null") == 0) {
            linked_list_remove_last(NULL);
        } else if (strcmp(which, "remove_first_null") == 0) {
            linked_list_remove_first(NULL);
        } else if (strcmp(which, "get_at_index_null") == 0) {
            (void)get_linked_list_at_index(NULL, 0);
        } else if (strcmp(which, "remove_at_index_null") == 0) {
            (void)linked_list_remove_at_index(NULL, 0);
        } else if (strcmp(which, "reverse_null") == 0) {
            (void)linked_list_reverse(NULL);
        } else if (strcmp(which, "debug_print_null") == 0) {
            linked_list_debug_print(NULL, NULL);
        } else {
            fprintf(stderr, "Unknown fatal case: %s\n", which);
            exit(2); /* make the subprocess fail clearly */
        }

        /* If the function above did NOT exit, we consider it a failure for the child. */
        exit(0);
    }
    return 0;
}

/* ---------------- Payload helpers ---------------- */

typedef struct {
    int value;
} Box;

static Box* box_new(int v) {
    Box* b = (Box*)malloc(sizeof(Box));
    if (!b) { fprintf(stderr, "malloc failed in box_new\n"); exit(1); }
    b->value = v;
    return b;
}

static void print_box(void* p) {
    Box* b = (Box*)p;
    if (b) printf("%d", b->value);
    else   printf("NULL");
}

/* Counting-free callback to verify *_with(...) APIs call our destructor. */
static int g_free_calls = 0;
static void counting_free(void* p) {
    if (p) {
        g_free_calls++;
        free(p);
    }
}

/* ---------------- Non-fatal tests ---------------- */

static void test_empty_list_basics(void) {
    LinkedList L = build_empty_linked_list();
    EXPECT_TRUE(is_linked_list_empty(L) == 1, "empty list should be empty");
    EXPECT_TRUE(get_linked_list_head_data(L) == NULL, "head data of empty should be NULL");
    EXPECT_TRUE(get_linked_list_tail(L) == NULL, "tail of empty should be NULL");
    EXPECT_EQ_UINT(get_linked_list_size(L), 0, "size(empty)=0");
    EXPECT_EQ_UINT(get_linked_list_size_recursive(L), 0, "size_recursive(empty)=0");
    EXPECT_TRUE(get_linked_list_last_element(L) == L, "last_element(empty) == head");
    linked_list_destroy(L);
}

static void test_push_back_and_last(void) {
    LinkedList L = build_empty_linked_list();
    linked_list_push_back(L, box_new(10));
    EXPECT_TRUE(!is_linked_list_empty(L), "after one push_back, list not empty");
    Box* h = (Box*)get_linked_list_head_data(L);
    EXPECT_TRUE(h && h->value == 10, "head value should be 10");
    LinkedList last = get_linked_list_last_element(L);
    EXPECT_TRUE(last == L, "with single node, last==head");
    linked_list_destroy(L);
}

static void test_push_front_and_promote(void) {
    LinkedList L = build_empty_linked_list();
    linked_list_push_front(L, box_new(1));
    linked_list_push_front(L, box_new(2));
    /* Now head=2, next=1 */
    Box* b = (Box*)get_linked_list_head_data(L);
    EXPECT_TRUE(b && b->value == 2, "head after two push_front should be 2");
    EXPECT_EQ_UINT(get_linked_list_size(L), 2, "size should be 2");
    linked_list_remove_first(L); /* remove 2, promote 1 into head */
    b = (Box*)get_linked_list_head_data(L);
    EXPECT_TRUE(b && b->value == 1, "head after remove_first should be 1");
    linked_list_destroy(L);
}

static void test_tail_behavior(void) {
    LinkedList L = build_empty_linked_list();
    linked_list_push_back(L, box_new(5));
    linked_list_push_back(L, box_new(6));
    LinkedList tail = get_linked_list_tail(L);
    EXPECT_TRUE(tail != NULL, "tail of 2-node list is not NULL");
    EXPECT_TRUE(((Box*)tail->data)->value == 6, "tail's head value should be 6");
    linked_list_destroy(L);
}

static void test_push_back_many_and_remove_last(void) {
    LinkedList L = build_empty_linked_list();
    linked_list_push_back(L, box_new(1));
    linked_list_push_back(L, box_new(2));
    linked_list_push_back(L, box_new(3));
    EXPECT_EQ_UINT(get_linked_list_size(L), 3, "size should be 3");
    linked_list_remove_last(L);
    EXPECT_EQ_UINT(get_linked_list_size(L), 2, "after remove_last, size=2");
    LinkedList last = get_linked_list_last_element(L);
    Box* lb = (Box*)last->data;
    EXPECT_TRUE(lb && lb->value == 2, "last element value should be 2");
    linked_list_destroy(L);
}

static void test_remove_at_index(void) {
    LinkedList L = build_empty_linked_list();
    linked_list_push_back(L, box_new(10)); /* idx 0 */
    linked_list_push_back(L, box_new(20)); /* idx 1 */
    linked_list_push_back(L, box_new(30)); /* idx 2 */

    EXPECT_EQ_UINT(get_linked_list_size(L), 3, "size before remove_at_index");

    /* remove middle */
    int r = linked_list_remove_at_index(L, 1);
    EXPECT_TRUE(r == 1, "remove_at_index(1) should succeed");
    EXPECT_EQ_UINT(get_linked_list_size(L), 2, "size after remove middle");
    Box* h = (Box*)get_linked_list_head_data(L);
    EXPECT_TRUE(h && h->value == 10, "head remains 10");
    LinkedList last = get_linked_list_last_element(L);
    Box* lb = (Box*)last->data;
    EXPECT_TRUE(lb && lb->value == 30, "tail remains 30");

    /* out-of-range */
    r = linked_list_remove_at_index(L, 99);
    EXPECT_TRUE(r == 0, "remove_at_index(out-of-range) should return 0");

    /* remove head (index 0) */
    r = linked_list_remove_at_index(L, 0);
    EXPECT_TRUE(r == 1, "remove_at_index(0) should succeed");
    EXPECT_EQ_UINT(get_linked_list_size(L), 1, "size after removing head");

    linked_list_destroy(L);
}

static void test_get_at_index(void) {
    LinkedList L = build_empty_linked_list();
    linked_list_push_back(L, box_new(7));
    linked_list_push_back(L, box_new(8));
    linked_list_push_back(L, box_new(9));
    LinkedListNode* n0 = get_linked_list_at_index(L, 0);
    LinkedListNode* n1 = get_linked_list_at_index(L, 1);
    LinkedListNode* n2 = get_linked_list_at_index(L, 2);
    LinkedListNode* n3 = get_linked_list_at_index(L, 3);
    EXPECT_TRUE(n0 && ((Box*)n0->data)->value == 7, "index 0 == 7");
    EXPECT_TRUE(n1 && ((Box*)n1->data)->value == 8, "index 1 == 8");
    EXPECT_TRUE(n2 && ((Box*)n2->data)->value == 9, "index 2 == 9");
    EXPECT_TRUE(n3 == NULL, "index 3 out-of-range should be NULL");
    linked_list_destroy(L);
}

static void test_reverse(void) {
    LinkedList L = build_empty_linked_list();
    linked_list_push_back(L, box_new(1));
    linked_list_push_back(L, box_new(2));
    linked_list_push_back(L, box_new(3));
    LinkedList R = linked_list_reverse(L); /* must assign returned head! */
    EXPECT_TRUE(((Box*)R->data)->value == 3, "reversed head == 3");
    EXPECT_TRUE(((Box*)R->next->data)->value == 2, "reversed second == 2");
    EXPECT_TRUE(((Box*)R->next->next->data)->value == 1, "reversed third == 1");
    linked_list_destroy(R);
}

static void test_debug_print_smoke(void) {
    LinkedList L = build_empty_linked_list();
    linked_list_push_back(L, box_new(42));
    linked_list_debug_print(L, print_box); /* smoke test: just ensure it doesn't crash */
    linked_list_destroy(L);
}

static void test_owner_with_variants(void) {
    g_free_calls = 0;

    /* remove_first_with on 2-node list */
    {
        LinkedList L = build_empty_linked_list();
        linked_list_push_back(L, box_new(10));
        linked_list_push_back(L, box_new(20));
        linked_list_remove_first_with(L, counting_free);
        EXPECT_EQ_UINT(g_free_calls, 1, "remove_first_with should call free_fn once");
        /* remaining one node */
        linked_list_destroy_with(L, counting_free);
        EXPECT_EQ_UINT(g_free_calls, 2, "destroy_with should free remaining node");
    }

    /* remove_after_with on 3-node list (remove middle) */
    {
        LinkedList L = build_empty_linked_list();
        linked_list_push_back(L, box_new(1));  /* head */
        linked_list_push_back(L, box_new(2));  /* middle (victim) */
        linked_list_push_back(L, box_new(3));  /* tail  */

        LinkedList head = L;
        linked_list_remove_after_with(head, counting_free); /* remove middle */
        EXPECT_EQ_UINT(g_free_calls, 3, "cumulative frees after previous test + this remove_after_with == 3");

        /* destroy_with for the remaining 2 nodes */
        linked_list_destroy_with(L, counting_free);
        EXPECT_TRUE(g_free_calls >= 4, "destroy_with should increase free count");
    }
}

/* ---------------- Fatal tests (via subprocess) ---------------- */

static void test_fatal_apis(const char* self_path) {
    const char* cases[] = {
        "is_empty_null", "head_data_null", "tail_null", "size_null",
        "size_rec_null", "last_elem_null", "push_back_null", "push_front_null",
        "remove_last_null", "remove_first_null",
        "get_at_index_null", "remove_at_index_null", "reverse_null", "debug_print_null"
    };
    size_t n = sizeof(cases)/sizeof(cases[0]);
    for (size_t i = 0; i < n; ++i) {
        int rc = run_fatal_subprocess(self_path, "--ll-fatal", cases[i]);
        /* We expect the child to exit non-zero (because the function calls exit()). */
        if (rc != 0) TEST_OK(); else TEST_FAIL("fatal case did not exit as expected");
    }
}

/* ---------------- Suite runner ---------------- */

int run_linked_list_tests(int argc, char** argv) {
    /* If invoked as a fatal child process, handle and return immediately. */
    if (handle_ll_fatal_case(argc, argv)) {
        return 0;
    }

    g_passed = g_failed = 0;

    /* Non-fatal tests */
    test_empty_list_basics();
    test_push_back_and_last();
    test_push_front_and_promote();
    test_tail_behavior();
    test_push_back_many_and_remove_last();
    test_remove_at_index();
    test_get_at_index();
    test_reverse();
    test_debug_print_smoke();
    test_owner_with_variants();

    /* Fatal tests (run in subprocesses using argv[0]) */
    test_fatal_apis(argv[0]);

    printf("[LinkedList] Passed: %d  Failed: %d\n", g_passed, g_failed);
    return (g_failed == 0) ? 0 : 1;
}
