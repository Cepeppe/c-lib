#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "hashmap_tests.h"
#include "../hashmap/hashmap.h"
#include "../linked_list/linked_list.h"

/* --------------- Minimal test harness --------------- */

static int g_passed = 0;
static int g_failed = 0;

#define TEST_OK()   do { g_passed++; } while(0)
#define TEST_FAIL(msg) do { g_failed++; fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, (msg)); } while(0)
#define EXPECT_TRUE(cond, msg) do { if ((cond)) TEST_OK(); else TEST_FAIL(msg); } while(0)
#define EXPECT_EQ_INT(a,b,msg) do { \
    int _aa=(int)(a), _bb=(int)(b); \
    if (_aa == _bb) TEST_OK(); \
    else { char _m[256]; snprintf(_m, sizeof(_m), "%s (got=%d, expected=%d)", (msg), _aa, _bb); TEST_FAIL(_m);} \
} while(0)

/* --------------- Helpers --------------- */

/* Find two short ASCII keys that hash into the same bucket. */
static int find_two_keys_same_bucket(char* out_k1, char* out_k2, size_t cap) {
    const size_t LIMIT = 20000;
    size_t first_seen_bucket[HASH_MAP_BUCKET_NUM];
    for (size_t i=0; i<HASH_MAP_BUCKET_NUM; ++i) first_seen_bucket[i] = (size_t)-1;

    char keybuf[64];
    for (size_t i = 0; i < LIMIT; ++i) {
        snprintf(keybuf, sizeof(keybuf), "k%zu", i);
        uint64_t h = generate_hash(keybuf, strlen(keybuf));
        size_t b = (size_t)(h % HASH_MAP_BUCKET_NUM);
        if (first_seen_bucket[b] == (size_t)-1) {
            first_seen_bucket[b] = i;
        } else {
            /* found collision w.r.t. bucket index */
            size_t j = first_seen_bucket[b];
            snprintf(out_k1, cap, "k%zu", j);
            snprintf(out_k2, cap, "k%zu", i);
            return 1;
        }
    }
    return 0;
}

/* --------------- Tests --------------- */

static void test_build_and_destroy(void) {
    HashMap* hm = build_hash_map();
    EXPECT_TRUE(hm != NULL, "build_hash_map returns non-NULL");
    hash_map_destroy(hm);
}

static void test_get_missing(void) {
    HashMap* hm = build_hash_map();
    const HashMapItem* it = hash_map_get(hm, "nope", 4);
    EXPECT_TRUE(it == NULL, "get on missing key returns NULL");
    hash_map_destroy(hm);
}

static void test_put_new_and_get(void) {
    HashMap* hm = build_hash_map();
    const char* k = "alpha";
    int v = 1234;

    int r = hash_map_put(hm, k, strlen(k), &v, sizeof(v));
    EXPECT_EQ_INT(r, 0, "put new returns 0 (new)");

    const HashMapItem* it = hash_map_get(hm, k, strlen(k));
    EXPECT_TRUE(it != NULL, "get after put returns item");
    EXPECT_TRUE(it->data_size == sizeof(int), "value size is sizeof(int)");
    EXPECT_TRUE(*(int*)it->data == 1234, "stored value matches");

    hash_map_destroy(hm);
}

static void test_put_update(void) {
    HashMap* hm = build_hash_map();
    const char* k = "beta";
    int v1 = 1, v2 = 2;

    int r = hash_map_put(hm, k, strlen(k), &v1, sizeof(v1));
    EXPECT_EQ_INT(r, 0, "first put is new");
    r = hash_map_put(hm, k, strlen(k), &v2, sizeof(v2));
    EXPECT_EQ_INT(r, 1, "second put updates existing");

    const HashMapItem* it = hash_map_get(hm, k, strlen(k));
    EXPECT_TRUE(it && *(int*)it->data == 2, "value updated to 2");

    hash_map_destroy(hm);
}

static void test_remove_missing_and_present(void) {
    HashMap* hm = build_hash_map();
    const char* k1 = "gamma";
    int v = 7;

    int r = hash_map_remove(hm, k1, strlen(k1));
    EXPECT_EQ_INT(r, 0, "remove missing returns 0");

    hash_map_put(hm, k1, strlen(k1), &v, sizeof(v));
    EXPECT_TRUE(hash_map_get(hm, k1, strlen(k1)) != NULL, "get returns just-inserted key");

    r = hash_map_remove(hm, k1, strlen(k1));
    EXPECT_EQ_INT(r, 1, "remove existing returns 1");
    EXPECT_TRUE(hash_map_get(hm, k1, strlen(k1)) == NULL, "get after remove returns NULL");

    hash_map_destroy(hm);
}

/* Exercise both head and non-head removal in the same bucket. */
static void test_bucket_head_and_nonhead_removal(void) {
    char k1[64], k2[64];
    int have = find_two_keys_same_bucket(k1, k2, sizeof(k1));
    EXPECT_TRUE(have, "found two keys mapping to the same bucket (collision)");
    if (!have) return;

    int v1 = 111, v2 = 222;

    HashMap* hm = build_hash_map();

    /* Insert k1 then k2 so k1 is head, k2 follows in that bucket. */
    (void)hash_map_put(hm, k1, strlen(k1), &v1, sizeof(v1));
    (void)hash_map_put(hm, k2, strlen(k2), &v2, sizeof(v2));

    EXPECT_TRUE(hash_map_get(hm, k1, strlen(k1)) != NULL, "k1 present");
    EXPECT_TRUE(hash_map_get(hm, k2, strlen(k2)) != NULL, "k2 present");

    /* Remove head (k1) while bucket has >=2 nodes */
    int r = hash_map_remove(hm, k1, strlen(k1));
    EXPECT_EQ_INT(r, 1, "remove(k1) succeeds (head, >=2 nodes)");
    EXPECT_TRUE(hash_map_get(hm, k1, strlen(k1)) == NULL, "k1 gone");
    EXPECT_TRUE(hash_map_get(hm, k2, strlen(k2)) != NULL, "k2 still present");

    /* Reinsert k1, now remove non-head (k2) */
    (void)hash_map_put(hm, k1, strlen(k1), &v1, sizeof(v1));
    r = hash_map_remove(hm, k2, strlen(k2));
    EXPECT_EQ_INT(r, 1, "remove(k2) succeeds (non-head)");
    EXPECT_TRUE(hash_map_get(hm, k2, strlen(k2)) == NULL, "k2 gone");
    EXPECT_TRUE(hash_map_get(hm, k1, strlen(k1)) != NULL, "k1 still present");

    hash_map_destroy(hm);
}

/* Verify key/value deep cloning behavior. */
static void test_deep_clone_semantics(void) {
    HashMap* hm = build_hash_map();

    char keybuf[16];  strcpy(keybuf, "zzz");
    int  valbuf = 5;

    (void)hash_map_put(hm, keybuf, strlen(keybuf), &valbuf, sizeof(valbuf));

    /* mutate original buffers */
    strcpy(keybuf, "mutated");
    valbuf = 42;

    const HashMapItem* it = hash_map_get(hm, "zzz", 3);
    EXPECT_TRUE(it != NULL, "original key still retrievable");
    EXPECT_TRUE(*(int*)it->data == 5, "stored value unaffected by external mutation");

    hash_map_destroy(hm);
}

int run_hashmap_tests(void) {
    g_passed = g_failed = 0;

    test_build_and_destroy();
    test_get_missing();
    test_put_new_and_get();
    test_put_update();
    test_remove_missing_and_present();
    test_bucket_head_and_nonhead_removal();
    test_deep_clone_semantics();

    printf("[HashMap]    Passed: %d  Failed: %d\n", g_passed, g_failed);
    return (g_failed == 0) ? 0 : 1;
}
