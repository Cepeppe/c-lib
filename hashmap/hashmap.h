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

/* deep free hash_map item */
static void hash_map_free_item(void* p);

/* clone arbitrary bytes into a new heap buffer that we own */
static void* clone_bytes(const void* src, size_t size);

/* builds empty hash map*/
HashMap* build_hash_map(void);

/*
 * Deep-free:
 * - for each node in each bucket calls linked_list_destroy(bucket) to free the nodes themselves
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
 * NOTE: CLONES both key and data into heap memory (using malloc+memcpy).
 *       The map becomes owner and will free them in hash_map_destroy().
 */
int hash_map_put(HashMap* hash_map,
                 const void* key,  size_t key_size,
                 const void* data, size_t data_size);

/*
 * Remove the entry with the given key (if present).
 *
 * Returns: 
 *  1 if key was found and removed
 *  0 if key not found
 *
 */
int hash_map_remove(HashMap* hash_map,
                    const void* key,
                    size_t key_size);

/*
 * Get by key
 *
 * Returns: 
 *  HashMapItem pointer associated to key (if exists)
 *  NULL otherwise
 * 
 * NOTE: it is returned a pointer to the object inside the hashmap,
 * return value is const so IT SHOULD NOT BE MODIFIED.
 * If you access in write mode on this object undefined behavious will arise.
 * 
 * IF YOU REALLY NEED TO ACCESS A HASHMAP STORED OBJECT IN WRITE MODE, 
 * CONSIDER USING APPROPRIATE EXPOSED APIs (e.g. PUT METHOD hash_map_put)
 * 
 * If you really want to use this method to modify in-place, make sure to deeply 
 * know the offered hash-map and linked list implementation in order to make sure
 * you are not breaking some invariants
 *
 */
const HashMapItem* hash_map_get(
                    HashMap* hash_map,
                    const void* key, 
                    size_t key_size);

#endif
