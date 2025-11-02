#include "hashmap.h"
#include <string.h>  /* memcpy, memcmp */

/* clone arbitrary bytes into a new heap buffer that we own */
static void* clone_bytes(const void* src, size_t size) {
    if (size == 0) {
        return NULL;
    }
    void* dst = malloc(size);
    if (dst == NULL) {
        fprintf(stderr, "malloc failed in clone_bytes\n");
        exit(FAILED_HASH_MAP_ALLOCATION);
    }
    memcpy(dst, src, size);
    return dst;
}

HashMap* build_hash_map(void) {
    HashMap* hash_map = malloc(sizeof(HashMap));
    if (hash_map == NULL) {
        fprintf(stderr, "Failed malloc while trying to build new hash map\n");
        exit(FAILED_HASH_MAP_ALLOCATION);
    }
    
    for (size_t i = 0; i < HASH_MAP_BUCKET_NUM; i++) {
        hash_map->buckets[i] = build_empty_linked_list();
    }

    return hash_map;
}

/*
 * Go through every bucket:
 *   - for each node: free deep contents of HashMapItem, set node->data=NULL
 *   - destroy bucket list nodes
 * finally free the HashMap struct itself.
 */
void hash_map_destroy(HashMap* hash_map) {
    if (hash_map == NULL) {
        fprintf(stderr, "You tried to destroy a NULL hash map, this is a no-op\n");
        return;
    }

    for (size_t i = 0; i < HASH_MAP_BUCKET_NUM; i++) {
        LinkedList bucket = hash_map->buckets[i];

        LinkedList node = bucket;
        while (node != NULL) {
            if (node->data != NULL) {
                HashMapItem* item = (HashMapItem*) node->data;

                if (item->key  != NULL) free(item->key);
                if (item->data != NULL) free(item->data);

                free(item);

                /* avoid double free in linked_list_destroy() */
                node->data = NULL;
            }
            node = node->next;
        }

        linked_list_destroy(bucket);
    }

    free(hash_map);
}

uint64_t generate_hash(const void* key, size_t key_size) {
    uint32_t hash_words[4];
    MurmurHash3_x64_128(key, (int)key_size, MUR_MUR_3_SEED, hash_words);

    uint64_t h64 =
        ((uint64_t)hash_words[1] << 32) |
        (uint64_t)hash_words[0];

    return h64;
}

/*
 * Returns 1 if key existed and we updated its value,
 * Returns 0 if this was a brand new key.
 */
int hash_map_put(HashMap* hash_map,
                 const void* key,
                 size_t key_size,
                 const void* data,
                 size_t data_size)
{
    /* 1. hash key */
    uint64_t h64 = generate_hash(key, key_size);

    /* 2. pick bucket */
    size_t bucket_index = (size_t)(h64 % HASH_MAP_BUCKET_NUM);
    LinkedList bucket_list = hash_map->buckets[bucket_index];

    /* 3. empty logical list case: just insert as first item */
    if (is_linked_list_empty(bucket_list)) {
        HashMapItem new_item_val;
        new_item_val.hash      = h64;
        new_item_val.key       = clone_bytes(key,  key_size);
        new_item_val.key_size  = key_size;
        new_item_val.data      = clone_bytes(data, data_size);
        new_item_val.data_size = data_size;

        linked_list_push_back(bucket_list, ll_alloc_HashMapItem(new_item_val));
        return 0; /* new key */
    }

    /* 4. bucket already has nodes: walk the list */
    LinkedList node = bucket_list;
    while (node != NULL) {
        HashMapItem* item = (HashMapItem*) node->data;

        if (item != NULL &&
            item->hash == h64 &&
            item->key_size == key_size &&
            memcmp(item->key, key, key_size) == 0)
        {
            /* found existing key: update its value */

            if (item->data != NULL) {
                free(item->data);
            }

            item->data      = clone_bytes(data, data_size);
            item->data_size = data_size;
            return 1; /* updated existing */
        }

        if (node->next == NULL) {
            /* reached tail; no match so far => append new */
            HashMapItem new_item_val;
            new_item_val.hash      = h64;
            new_item_val.key       = clone_bytes(key,  key_size);
            new_item_val.key_size  = key_size;
            new_item_val.data      = clone_bytes(data, data_size);
            new_item_val.data_size = data_size;

            linked_list_push_back(bucket_list, ll_alloc_HashMapItem(new_item_val));
            return 0; /* inserted new */
        }

        node = node->next;
    }

    /* shouldn't really get here */
    return 0;
}
