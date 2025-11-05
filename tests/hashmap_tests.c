#include "hashmap_tests.h"

static int hm_passed = 0;
static int hm_failed = 0;

#define HM_EXPECT(cond, msg)                                                    \
    do {                                                                        \
        if ((cond)) {                                                           \
            hm_passed++;                                                        \
        } else {                                                                \
            hm_failed++;                                                        \
            fprintf(stderr, "[HM FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg);  \
        }                                                                       \
    } while (0)

/* -------- Helpers for data ownership testing -------- */

static int g_data_free_count = 0;
static void data_free_counter(void* p) {
    if (p) {
        g_data_free_count++;
        free(p);
    }
}

/* Compute bucket index (utility only for tests) */
static size_t bucket_of(const void* key, size_t key_size) {
    uint64_t h = generate_hash(key, key_size);
    return (size_t)(h % HASH_MAP_BUCKET_NUM);
}

/* Find a different key that collides into the same bucket as (k,ks).
 * Very simple search by appending a suffix; deterministic enough for tests. */
static void make_colliding_key(char* out, size_t out_cap,
                               const void* base_key, size_t base_len,
                               size_t target_bucket)
{
    /* Start from base + "#i" */
    for (unsigned i = 0; i < 200000; ++i) {
        int n = snprintf(out, (int)out_cap, "%.*s#%u", (int)base_len, (const char*)base_key, i);
        if (n <= 0 || (size_t)n >= out_cap) continue;
        if (bucket_of(out, (size_t)n) == target_bucket) return;
    }
    /* Fallback: keep the base (worst case will not collide) */
    snprintf(out, (int)out_cap, "%.*s#X", (int)base_len, (const char*)base_key);
}

/* -------- Individual tests -------- */

static void test_build_map_buckets_initialized(void) {
    HashMap* m = build_hash_map();
    /* sanity check some buckets (not NULL, empty) */
    for (size_t i = 0; i < HASH_MAP_BUCKET_NUM; ++i) {
        HM_EXPECT(m->buckets[i] != NULL, "Bucket head must be allocated");
        HM_EXPECT(is_linked_list_empty(m->buckets[i]) == 1, "New bucket must be logically empty");
    }
    hash_map_destroy(m, NULL);
}

static void test_put_get_without_data_ownership(void) {
    HashMap* m = build_hash_map();

    const char* key = "alpha";
    const char* value = "VALUE_NOT_OWNED";
    size_t ks = strlen(key);
    size_t vs = strlen(value);

    int r = hash_map_put(m, key, ks, (const void*)value, vs, NULL); /* data not owned */
    HM_EXPECT(r == 0, "First put must return 0 (new insertion)");

    const HashMapItem* it = hash_map_get(m, key, ks);
    HM_EXPECT(it != NULL, "Get must find the inserted key");
    HM_EXPECT(it->data == value, "Data pointer must match when not owned");
    HM_EXPECT(it->data_size == vs, "Data size must match");

    hash_map_destroy(m, NULL); /* no data free expected */
}

static void test_put_updates_and_frees_old_value_when_owned(void) {
    HashMap* m = build_hash_map();

    const char* key = "k1";
    size_t ks = strlen(key);

    /* First value (owned) */
    char* v1 = (char*)malloc(6);
    memcpy(v1, "hello", 6); /* include terminator for demonstration */
    g_data_free_count = 0;
    int r1 = hash_map_put(m, key, ks, v1, 6, data_free_counter);
    HM_EXPECT(r1 == 0, "First put should insert new key");

    /* Update with a new value (owned) → old must be freed exactly once */
    char* v2 = (char*)malloc(4);
    memcpy(v2, "bye", 4);
    int r2 = hash_map_put(m, key, ks, v2, 4, data_free_counter);
    HM_EXPECT(r2 == 1, "Second put with same key should update");

    HM_EXPECT(g_data_free_count == 1, "Old owned value must be freed once on update");

    const HashMapItem* it = hash_map_get(m, key, ks);
    HM_EXPECT(it != NULL, "Get must still find key after update");
    HM_EXPECT(it->data == v2, "Data pointer must be updated to the new value");
    HM_EXPECT(it->data_size == 4, "Data size must reflect the updated value");

    hash_map_destroy(m, data_free_counter); /* frees v2 */
    HM_EXPECT(g_data_free_count == 2, "Destroy should free the last owned value");
}

static void test_key_is_deep_copied(void) {
    HashMap* m = build_hash_map();

    char key_buf[4] = {'a','b','c','\0'};
    const char* expected = "abc";
    const char* value = "X";
    size_t ks = 3;

    (void)hash_map_put(m, key_buf, ks, value, 1, NULL);
    /* mutate the original key buffer */
    key_buf[0] = 'Z';

    const HashMapItem* it = hash_map_get(m, expected, ks);
    HM_EXPECT(it != NULL, "Get must still find the entry by the original content");
    HM_EXPECT(memcmp(it->key, expected, ks) == 0, "Stored key must be an independent deep copy");

    hash_map_destroy(m, NULL);
}

static void test_remove_head_singleton_and_multinode(void) {
    HashMap* m = build_hash_map();

    const char* k1 = "HEAD-ONLY";
    size_t ks1 = strlen(k1);
    char* v1 = (char*)malloc(2); memcpy(v1, "A", 2);
    g_data_free_count = 0;

    /* single-node bucket removal */
    (void)hash_map_put(m, k1, ks1, v1, 2, data_free_counter);
    HM_EXPECT(hash_map_remove(m, k1, ks1, data_free_counter) == 1, "Remove must succeed for single-node bucket");
    HM_EXPECT(hash_map_get(m, k1, ks1) == NULL, "Get must return NULL after removal");
    HM_EXPECT(g_data_free_count == 1, "Owned value must be freed on removal");

    /* multi-node head removal (promote second) */
    const char* base = "COLLIDE";
    size_t base_len = strlen(base);
    size_t target_bucket = bucket_of(base, base_len);

    /* insert head */
    char* vH = (char*)malloc(2); memcpy(vH, "H", 2);
    (void)hash_map_put(m, base, base_len, vH, 2, data_free_counter);

    /* find another key colliding into same bucket to become second */
    char k2buf[64];
    make_colliding_key(k2buf, sizeof k2buf, base, base_len, target_bucket);

    char* vS = (char*)malloc(2); memcpy(vS, "S", 2);
    (void)hash_map_put(m, k2buf, strlen(k2buf), vS, 2, data_free_counter);

    /* remove head; second should be promoted, and still retrievable */
    size_t frees_before = g_data_free_count;
    HM_EXPECT(hash_map_remove(m, base, base_len, data_free_counter) == 1, "Remove head from multi-node bucket must succeed");
    HM_EXPECT(hash_map_get(m, base, base_len) == NULL, "Head key must no longer be present after removal");
    HM_EXPECT(hash_map_get(m, k2buf, strlen(k2buf)) != NULL, "Second key must survive head removal");
    HM_EXPECT(g_data_free_count == frees_before + 1, "Owned head value must be freed exactly once");

    hash_map_destroy(m, data_free_counter); /* frees vS if still present */
}

static void test_remove_non_head(void) {
    HashMap* m = build_hash_map();

    const char* base = "COLLIDE-NH";
    size_t base_len = strlen(base);
    size_t target_bucket = bucket_of(base, base_len);

    /* Try to build two extra keys that collide in the same target bucket */
    char k2buf[64], k3buf[64];
    make_colliding_key(k2buf, sizeof k2buf, base, base_len, target_bucket);
    make_colliding_key(k3buf, sizeof k3buf, base, base_len, target_bucket);

    /* Fallback: if they don't collide, deterministically craft colliding keys */
    if (bucket_of(k2buf, strlen(k2buf)) != target_bucket || strcmp(k2buf, base) == 0) {
        unsigned long long s = 1;
        for (;; ++s) {
            int n = snprintf(k2buf, sizeof k2buf, "%s#%llu", base, s);
            if (n > 0 && (size_t)n < sizeof k2buf &&
                bucket_of(k2buf, (size_t)n) == target_bucket &&
                strcmp(k2buf, base) != 0) {
                break;
            }
        }
    }
    if (bucket_of(k3buf, strlen(k3buf)) != target_bucket || strcmp(k3buf, base) == 0 || strcmp(k3buf, k2buf) == 0) {
        unsigned long long s = 1000000ULL; /* start far to avoid k2 suffixes */
        for (;; ++s) {
            int n = snprintf(k3buf, sizeof k3buf, "%s#%llu", base, s);
            if (n > 0 && (size_t)n < sizeof k3buf &&
                bucket_of(k3buf, (size_t)n) == target_bucket &&
                strcmp(k3buf, base) != 0 &&
                strcmp(k3buf, k2buf) != 0) {
                break;
            }
        }
    }

    /* Sanity: now they MUST collide */
    HM_EXPECT(bucket_of(k2buf, strlen(k2buf)) == target_bucket, "k2 must collide in target bucket");
    HM_EXPECT(bucket_of(k3buf, strlen(k3buf)) == target_bucket, "k3 must collide in target bucket");

    /* Allocate tiny values (owned by the map via data_free_counter) */
    char* v1 = (char*)malloc(2); memcpy(v1, "1", 2);
    char* v2 = (char*)malloc(2); memcpy(v2, "2", 2);
    char* v3 = (char*)malloc(2); memcpy(v3, "3", 2);

    /* Insert in order: head (base), middle (k2), tail (k3) */
    (void)hash_map_put(m, base,  base_len,      v1, 2, data_free_counter); /* head */
    (void)hash_map_put(m, k2buf, strlen(k2buf), v2, 2, data_free_counter); /* middle */
    (void)hash_map_put(m, k3buf, strlen(k3buf), v3, 2, data_free_counter); /* tail */

    /* Bucket must now contain exactly 3 nodes */
    LinkedList bucket = m->buckets[target_bucket];
    HM_EXPECT(!is_linked_list_empty(bucket), "Bucket must be non-empty before removal");
    size_t size_before = get_linked_list_size(bucket);
    HM_EXPECT(size_before == 3, "Bucket size must be 3 before removal");

    /* Tail before should be k3 */
    LinkedListNode* tail_before = get_linked_list_last_element(bucket);
    HM_EXPECT(tail_before != NULL, "Tail must exist before removal");
    HashMapItem* tail_item_before = (HashMapItem*)tail_before->data;
    HM_EXPECT(tail_item_before != NULL, "Tail item must be non-null before removal");
    HM_EXPECT(tail_item_before->key_size == strlen(k3buf), "Tail key size must match before removal");
    HM_EXPECT(memcmp(tail_item_before->key, k3buf, strlen(k3buf)) == 0, "Tail before must be k3");

    /* Remove the non-head (middle) key */
    size_t frees_before = (size_t)g_data_free_count;
    HM_EXPECT(hash_map_remove(m, k2buf, strlen(k2buf), data_free_counter) == 1,
              "Remove non-head must succeed");

    /* Middle must be gone; head and tail must still be findable */
    HM_EXPECT(hash_map_get(m, k2buf, strlen(k2buf)) == NULL, "Removed middle key must be gone");
    HM_EXPECT(hash_map_get(m, base,  base_len)      != NULL, "Head must still exist");
    HM_EXPECT(hash_map_get(m, k3buf, strlen(k3buf)) != NULL, "Tail must still exist");

    /* Bucket size decreased by 1 and the (logical) tail persists as k3 */
    bucket = m->buckets[target_bucket];
    HM_EXPECT(!is_linked_list_empty(bucket), "Bucket must remain non-empty after removal");
    size_t size_after = get_linked_list_size(bucket);
    HM_EXPECT(size_after == size_before - 1, "Bucket size must decrease by 1");

    LinkedListNode* tail_after = get_linked_list_last_element(bucket);
    HM_EXPECT(tail_after != NULL, "Tail must still exist");
    HashMapItem* tail_item_after = (HashMapItem*)tail_after->data;
    HM_EXPECT(tail_item_after != NULL, "Tail item must be non-null after removal");
    HM_EXPECT(tail_item_after->key_size == strlen(k3buf), "Tail key size must match after removal");
    HM_EXPECT(memcmp(tail_item_after->key, k3buf, strlen(k3buf)) == 0, "Tail after must still be k3");

    /* Exactly one owned value (middle) must have been freed */
    HM_EXPECT((size_t)g_data_free_count == frees_before + 1,
              "Owned middle value must be freed once");

    hash_map_destroy(m, data_free_counter);
}



static void test_get_missing_returns_null(void) {
    HashMap* m = build_hash_map();
    HM_EXPECT(hash_map_get(m, "nope", 4) == NULL, "Get on missing key must return NULL");
    hash_map_destroy(m, NULL);
}

/* -------- Entry point -------- */

void run_all_hashmap_tests(void) {
    hm_passed = hm_failed = 0;
    printf("[TEST] testing hashmap...\n");

    test_build_map_buckets_initialized();
    test_put_get_without_data_ownership();
    test_put_updates_and_frees_old_value_when_owned();
    test_key_is_deep_copied();
    test_remove_head_singleton_and_multinode();
    test_remove_non_head();
    test_get_missing_returns_null();

    if (hm_failed == 0) {
        printf("[TEST OK]  hashmap: passed=%d failed=%d\n", hm_passed, hm_failed);
    } else {
        printf("[TEST FAIL] hashmap: passed=%d failed=%d\n", hm_passed, hm_failed);
    }
}
