#include "linked_list_tests.h"

static int ll_passed = 0;
static int ll_failed = 0;

#define LL_EXPECT(cond, msg)                                                    \
    do {                                                                        \
        if ((cond)) {                                                           \
            ll_passed++;                                                        \
        } else {                                                                \
            ll_failed++;                                                        \
            fprintf(stderr, "[LL FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg);  \
        }                                                                       \
    } while (0)

/* -------- Helpers for deep-free counters -------- */

static int g_free_count_payload = 0;

static void free_int_payload(void* p) {
    if (p) {
        g_free_count_payload++;
        free(p);
    }
}

/* A minimal HashMapItem deallocator compatible with
 * linked_list_remove_hashmap_node_with(...) signature.
 * It frees key, then (optionally) data via callback, then frees the item.
 */
static int g_free_count_hm_item = 0;
static int g_free_count_hm_data = 0;

static void hm_item_free_for_ll(void* item_void,
                                void (*deep_deallocate_hashmap_item_data)(void*)) {
    if (!item_void) return;
    g_free_count_hm_item++;
    HashMapItem* item = (HashMapItem*)item_void;

    /* Simulate the map’s ownership guarantees for the test */
    if (item->key) free(item->key);
    if (deep_deallocate_hashmap_item_data && item->data) {
        g_free_count_hm_data++;
        deep_deallocate_hashmap_item_data(item->data);
    }
    free(item);
}

/* -------- Individual tests -------- */

static void test_build_and_empty(void) {
    LinkedList l = build_empty_linked_list();
    LL_EXPECT(is_linked_list_empty(l) == 1, "Newly built list should be empty");
    LL_EXPECT(get_linked_list_size(l) == 0, "Empty list size must be 0");
    LL_EXPECT(get_linked_list_head_data(l) == NULL, "Head data of empty list must be NULL");
    LL_EXPECT(get_linked_list_tail(l) == NULL, "Tail of empty list must be NULL");
    linked_list_destroy(l);
}

static void test_push_back_basic(void) {
    LinkedList l = build_empty_linked_list();

    int* a = malloc(sizeof *a); *a = 11;
    linked_list_push_back(l, a);
    LL_EXPECT(is_linked_list_empty(l) == 0, "List should not be empty after first push_back");
    LL_EXPECT(get_linked_list_size(l) == 1, "Size should be 1 after first push_back");
    LL_EXPECT(get_linked_list_last_element(l) == l, "With one element, last should be head");
    LL_EXPECT(get_linked_list_head_data(l) == a, "Head data must be the pushed pointer");

    int* b = malloc(sizeof *b); *b = 22;
    linked_list_push_back(l, b);
    LL_EXPECT(get_linked_list_size(l) == 2, "Size should be 2 after second push_back");
    LL_EXPECT(get_linked_list_last_element(l)->data == b, "Last node data must be second pointer");

    /* cleanup without deep free: then free manually */
    linked_list_remove_last(l);  /* removes node with b but does not free payload */
    LL_EXPECT(get_linked_list_size(l) == 1, "Size should be 1 after remove_last");
    free(b);

    linked_list_remove_last_with(l, free_int_payload); /* remove last payload (a) */
    LL_EXPECT(is_linked_list_empty(l) == 1, "List should be empty after removing the only element");
    linked_list_destroy(l);
}

static void test_push_front_and_remove_first(void) {
    LinkedList l = build_empty_linked_list();

    int* a = malloc(sizeof *a); *a = 33;
    linked_list_push_front(l, a);
    LL_EXPECT(get_linked_list_size(l) == 1, "push_front on empty should set size=1");
    LL_EXPECT(get_linked_list_head_data(l) == a, "Head must contain first element");

    int* b = malloc(sizeof *b); *b = 44;
    linked_list_push_front(l, b);
    LL_EXPECT(get_linked_list_size(l) == 2, "Size should be 2 after second push_front");
    LL_EXPECT(get_linked_list_head_data(l) == b, "Head should be the most recent push_front");

    /* remove first with deep free */
    g_free_count_payload = 0;
    linked_list_remove_first_with(l, free_int_payload);
    LL_EXPECT(get_linked_list_size(l) == 1, "After remove_first_with, size should be 1");
    LL_EXPECT(g_free_count_payload == 1, "Deep free on head payload must be called once");

    /* remove remaining first without deep free, then free manually */
    void* remaining = get_linked_list_head_data(l);
    linked_list_remove_first(l);
    LL_EXPECT(is_linked_list_empty(l) == 1, "List should be empty after removing last head");
    free(remaining);

    linked_list_destroy(l);
}

static void test_remove_next_variants(void) {
    LinkedList l = build_empty_linked_list();

    int* a = malloc(sizeof *a); *a = 1;
    int* b = malloc(sizeof *b); *b = 2;
    int* c = malloc(sizeof *c); *c = 3;

    linked_list_push_back(l, a);
    linked_list_push_back(l, b);
    linked_list_push_back(l, c);
    LL_EXPECT(get_linked_list_size(l) == 3, "Expect 3 nodes in the list");

    /* remove-next (no deep free): remove 'b' */
    linked_list_remove_next_node(l);
    LL_EXPECT(get_linked_list_size(l) == 2, "After remove_next_node, size should be 2");
    LL_EXPECT(l->next->data == c, "After removing middle, head->next should be the last (c)");

    /* Now list is [a, c]; remove-next-with to remove 'c' */
    g_free_count_payload = 0;
    linked_list_remove_next_node_with(l, free_int_payload);
    LL_EXPECT(get_linked_list_size(l) == 1, "After remove_next_node_with, size should be 1");
    LL_EXPECT(g_free_count_payload == 1, "Deep free should be called once for removed next node");

    /* cleanup: remove 'a' */
    linked_list_remove_first_with(l, free_int_payload);
    linked_list_destroy(l);

    /* Manually free the middle that was removed without deep free */
    free(b);
}

static void test_destroy_with_deallocator(void) {
    LinkedList l = build_empty_linked_list();

    int* a = malloc(sizeof *a);
    int* b = malloc(sizeof *b);
    int* c = malloc(sizeof *c);
    linked_list_push_back(l, a);
    linked_list_push_back(l, b);
    linked_list_push_back(l, c);

    g_free_count_payload = 0;
    linked_list_destroy_with(l, free_int_payload);
    LL_EXPECT(g_free_count_payload == 3, "destroy_with must deep-free all payloads");
}

/* Test the specialized helper: linked_list_remove_hashmap_node_with(...) */
static void test_ll_remove_hashmap_node_with(void) {
    /* Build a detached node that holds a HashMapItem */
    LinkedListNode* node = (LinkedListNode*)malloc(sizeof *node);
    node->next = NULL;

    HashMapItem* item = (HashMapItem*)malloc(sizeof *item);
    const char* k = "KEY";
    size_t ks = 3;
    item->key = malloc(ks);
    memcpy(item->key, k, ks);
    item->key_size = ks;

    item->data_size = sizeof(int);
    item->data = malloc(item->data_size);
    item->hash = 0;

    node->data = item;

    /* ---- baseline counters ---- */
    size_t before_payload = g_free_count_payload;
    size_t before_hm_item = g_free_count_hm_item;
    size_t before_hm_data = g_free_count_hm_data;

    linked_list_remove_hashmap_node_with(
        node,
        hm_item_free_for_ll,
        free_int_payload
    );

    LL_EXPECT(g_free_count_hm_item == before_hm_item + 1, "HashMapItem deallocator must be called once");
    LL_EXPECT(g_free_count_hm_data == before_hm_data + 1, "HashMapItem data deallocator must be invoked once");
    LL_EXPECT(g_free_count_payload == before_payload + 1, "Underlying payload free must be called once");
}


/* -------- Entry point -------- */

void run_all_linked_list_tests(void) {
    ll_passed = ll_failed = 0;
    printf("[TEST] testing linked_list...\n");

    test_build_and_empty();
    test_push_back_basic();
    test_push_front_and_remove_first();
    test_remove_next_variants();
    test_destroy_with_deallocator();
    test_ll_remove_hashmap_node_with();

    if (ll_failed == 0) {
        printf("[TEST OK]  linked_list: passed=%d failed=%d\n", ll_passed, ll_failed);
    } else {
        printf("[TEST FAIL] linked_list: passed=%d failed=%d\n", ll_passed, ll_failed);
    }
}
