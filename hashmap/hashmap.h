#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include "../linked_list/linked_list.h"
#include "../hashing/murmur3.h"

#define HASH_MAP_BUCKET_NUM 500
#define FAILED_HASH_MAP_ALLOCATION -96
#define ATTEMPTED_ACCESS_TO_NULL_HASHMAP -95
#define ATTEMPTED_ACCESS_TO_NULL_HASHMAP_BUCKET -94
#define TOO_LONG_HASHMAP_KEY -93
#define MUR_MUR_3_SEED 32

/*
 * ============================================================================
 *  HashMap — Contracts & Ownership
 * ============================================================================
 * - Buckets: array of LinkedList with a sentinel head. A bucket is a logically
 *   empty list when head->data == NULL && head->next == NULL.
 *
 * - Key ownership: the key is ALWAYS deep-copied in hash_map_put() and will
 *   ALWAYS be freed by the map (see hash_map_free_item_with()).
 *
 * Ownership policy:
 * - Keys: the map ALWAYS owns keys. Keys are deep-copied on insertion and freed by the map.
 * - Data: ownership depends on whether a data-deallocator callback is provided to APIs:
 *     * if a non-NULL callback is passed -> ownership of 'data' is transferred to the map,
 *       and the map will free it using that callback;
 *     * if NULL is passed -> the map does NOT own 'data' and will NOT free it.
 *
 * - Thread-safety: NOT thread-safe.
 * ============================================================================
 */

/* Each entry stored in a bucket's linked list. */
typedef struct HashMapItem {
    uint64_t hash;      /* 64-bit hash of the key */
    void*    key;       /* heap copy of the key bytes (ALWAYS owned by the map) */
    size_t   key_size;  /* key length in bytes */
    void*    data;      /* value pointer (ownership depends on callback presence) */
    size_t   data_size; /* value length in bytes */
} HashMapItem;

/*
 * HashMap: fixed number of buckets.
 * Each bucket is a LinkedList with a sentinel head (never NULL after build).
 */
typedef struct HashMap {
    LinkedList buckets[HASH_MAP_BUCKET_NUM];
} HashMap;

/* ------------------------------------------------------------------------- */
/*                               API – Prototypes                            */
/* ------------------------------------------------------------------------- */

/* Compute a 64-bit hash for 'key' using MurmurHash3_x64_128 (lower 64 bits). */
uint64_t generate_hash(const void* key, size_t key_size);

/* Build a new hash map with HASH_MAP_BUCKET_NUM initialized (empty) buckets. */
HashMap* build_hash_map(void);

/* Destroy the entire map; optionally deep-free each item's data via callback. */
void hash_map_destroy(HashMap* hash_map,
                      void (*deep_deallocate_hashmap_item_data)(void* node_data));

/*
 * Insert or update (upsert) an entry.
 * Returns: 1 if updated an existing key; 0 if inserted a new key.
 * Ownership: key is always deep-copied; data ownership follows the callback rule above.
 */
int hash_map_put(HashMap* hash_map,
                 const void* key,
                 size_t key_size,
                 const void* data,
                 size_t data_size,
                 void (*deep_deallocate_hashmap_item_data)(void* node_data));

/*
 * Remove an entry by key.
 * Returns: 1 if a matching entry was removed; 0 if not found.
 * Will use the provided callback to free data if ownership was transferred.
 */
int hash_map_remove(HashMap* hash_map,
                    const void* key,
                    size_t key_size,
                    void (*deep_deallocate_hashmap_item_data)(void* node_data));

/*
 * Lookup an entry by key.
 * Returns: a const internal pointer to the stored HashMapItem, or NULL if not found.
 * IMPORTANT: do NOT modify or free the returned pointer; lifetime is managed by the map.
 */
const HashMapItem* hash_map_get(HashMap* hash_map,
                                const void* key,
                                size_t key_size);

/*
 * Allocate a HashMapItem on the heap and perform a SHALLOW struct copy of 'value'.
 * NOTE: This does NOT clone the key/data buffers; callers should ensure that 'value'
 *       already contains the desired (e.g., deep-copied) pointers.
 * Return type is void* for convenient use with generic linked list APIs.
 */
void* ll_alloc_HashMapItem(HashMapItem value);

#endif /* HASHMAP_H */
