#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>        /* memcmp prototype; safe to have here */
#include "../linked_list/linked_list.h"
#include "../hashing/murmur3.h"

#define HASH_MAP_BUCKET_NUM 500
#define FAILED_HASH_MAP_ALLOCATION -96
#define MUR_MUR_3_SEED 32

/*
 * Each entry we store in a bucket's linked list.
 * key / data are HEAP COPIES that the map owns.
 */
typedef struct HashMapItem {
    uint64_t hash;      /* 64-bit hash of the key */
    void*    key;       /* heap copy of the key bytes */
    size_t   key_size;  /* key length in bytes */
    void*    data;      /* heap copy of the value bytes */
    size_t   data_size; /* value length in bytes */
} HashMapItem;

/*
 * HashMap: fixed number of buckets.
 * Each bucket is a LinkedList (your custom list),
 * never NULL (initialized with build_empty_linked_list()).
 */
typedef struct HashMap {
    LinkedList buckets[HASH_MAP_BUCKET_NUM];
} HashMap;

HashMap* build_hash_map(void);

/*
 * Deep-free:
 * - for each node in each bucket:
 *   - free(item->key), free(item->data), free(item)
 *   - set node->data = NULL
 * - then call linked_list_destroy(bucket) to free the nodes themselves
 * - finally free(hash_map)
 */
void hash_map_destroy(HashMap* hash_map);

/*
 * Helper to compute 64-bit hash from arbitrary key bytes.
 */
uint64_t generate_hash(const void* key, size_t key_size);

/*
 * Insert/update.
 * Returns 1 if key already existed (so we replaced its value),
 * 0 if this is a new key.
 *
 * NOTE: This function CLONES both key and data into heap memory
 *       (using malloc+memcpy).
 *       The map becomes owner and will free them in hash_map_destroy().
 */
int hash_map_put(HashMap* hash_map,
                 const void* key,  size_t key_size,
                 const void* data, size_t data_size);

#endif
