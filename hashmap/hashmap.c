#include "hashmap.h"
#include <string.h>  /* memcpy, memcmp */

/*
 * Deep-free callback for LinkedList payloads.
 * Frees:
 *   - item->key (heap buffer)
 *   - item->data (heap buffer)
 *   - the HashMapItem struct itself
 */
static void hash_map_free_item(void* p) {
    HashMapItem* item = (HashMapItem*)p;
    if (!item) return;
    free(item->key);
    free(item->data);
    free(item);
}

/*
 * Builds an empty HashMap with HASH_MAP_BUCKET_NUM buckets.
 * Each bucket is an empty LinkedList (sentinel head).
 * The LinkedList is the OWNER of its node payloads.
 */
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
 * Destroys the entire HashMap.
 * Delegates deep payload destruction to the LinkedList by providing
 * the hash_map_free_item callback.
 */
void hash_map_destroy(HashMap* hash_map) {
    if (hash_map == NULL) {
        fprintf(stderr, "You tried to destroy a NULL hash map, this is a no-op\n");
        return;
    }

    for (size_t i = 0; i < HASH_MAP_BUCKET_NUM; i++) {
        LinkedList bucket = hash_map->buckets[i];
        /* LinkedList is the owner: it frees node payloads using the callback */
        linked_list_destroy_with(bucket, hash_map_free_item);
    }

    free(hash_map);
}

/*
 * 64-bit hash from MurmurHash3_x64_128 (lower 64 bits).
 */
uint64_t generate_hash(const void* key, size_t key_size) {
    uint32_t hash_words[4];
    MurmurHash3_x64_128(key, (int)key_size, MUR_MUR_3_SEED, hash_words);

    uint64_t h64 =
        ((uint64_t)hash_words[1] << 32) |
        (uint64_t)hash_words[0];

    return h64;
}

/*
 * Inserts or updates an entry.
 *
 * Returns:
 *   1 if the key already existed and its value was updated
 *   0 if this was a brand-new insertion
 *
 * Notes:
 * - Keys and values are deep-copied (clone_bytes).
 * - The created HashMapItem* is handed to the LinkedList, which becomes its owner.
 */
int hash_map_put(HashMap* hash_map,
                 const void* key,
                 size_t key_size,
                 const void* data,
                 size_t data_size)
{
    /* 1) hash key */
    uint64_t h64 = generate_hash(key, key_size);

    /* 2) pick bucket */
    size_t bucket_index = (size_t)(h64 % HASH_MAP_BUCKET_NUM);
    LinkedList bucket_list = hash_map->buckets[bucket_index];

    /* 3) empty logical list → insert as first item */
    if (is_linked_list_empty(bucket_list)) {
        HashMapItem new_item_val;
        new_item_val.hash      = h64;
        new_item_val.key       = clone_bytes(key,  key_size);
        new_item_val.key_size  = key_size;
        new_item_val.data      = clone_bytes(data, data_size);
        new_item_val.data_size = data_size;

        /* LinkedList takes ownership of the allocated HashMapItem* */
        linked_list_push_back(bucket_list, ll_alloc_HashMapItem(new_item_val));
        return 0; /* new key */
    }

    /* 4) bucket has nodes → walk and search for key */
    LinkedList node = bucket_list;
    while (node != NULL) {
        HashMapItem* item = (HashMapItem*) node->data;

        if (item != NULL &&
            item->hash == h64 &&
            item->key_size == key_size &&
            memcmp(item->key, key, key_size) == 0)
        {
            /* found existing key → update value (free old buffer, clone new) */
            if (item->data != NULL) free(item->data);
            item->data      = clone_bytes(data, data_size);
            item->data_size = data_size;
            return 1; /* updated existing */
        }

        if (node->next == NULL) {
            /* reached tail; no match → append new */
            HashMapItem new_item_val;
            new_item_val.hash      = h64;
            new_item_val.key       = clone_bytes(key, key_size);
            new_item_val.key_size  = key_size;
            new_item_val.data      = clone_bytes(data, data_size);
            new_item_val.data_size = data_size;

            /* LinkedList becomes owner of the new item */
            linked_list_push_back(bucket_list, ll_alloc_HashMapItem(new_item_val));
            return 0; /* inserted new */
        }

        node = node->next;
    }

    /* should not reach here */
    return 0;
}

/*
 * Removes an entry by key.
 *
 * Returns:
 *   1 if a matching entry was removed
 *   0 if the key was not found
 *
 * This function DELEGATES removal and deep-free to the LinkedList by
 * calling the *_with(...) variants and passing hash_map_free_item as
 * the payload destructor.
 */
int hash_map_remove(HashMap* hash_map,
                    const void* key,
                    size_t key_size)
{
    if (hash_map == NULL) {
        fprintf(stderr, "hash_map_remove called with NULL hash_map\n");
        return 0;
    }

    /* locate bucket */
    uint64_t h64 = generate_hash(key, key_size);
    size_t bucket_index = (size_t)(h64 % HASH_MAP_BUCKET_NUM);
    LinkedList bucket_head = hash_map->buckets[bucket_index];

    /* empty logical list → nothing to remove */
    if (is_linked_list_empty(bucket_head)) {
        return 0;
    }

    /* CASE 1: check head */
    {
        HashMapItem* first_item = (HashMapItem*) bucket_head->data;

        if (first_item != NULL &&
            first_item->hash == h64 &&
            first_item->key_size == key_size &&
            memcmp(first_item->key, key, key_size) == 0)
        {
            /* LinkedList is the owner: remove head and deep-free via callback */
            linked_list_remove_first_with(bucket_head, hash_map_free_item);
            return 1;
        }
    }

    /* CASE 2: not head → walk with (prev, curr) and remove-after-prev */
    LinkedList prev = bucket_head;
    LinkedList curr = bucket_head->next;

    while (curr != NULL) {
        HashMapItem* item = (HashMapItem*) curr->data;

        if (item != NULL &&
            item->hash == h64 &&
            item->key_size == key_size &&
            memcmp(item->key, key, key_size) == 0)
        {
            /* Delegate removal to LinkedList; it will call hash_map_free_item */
            linked_list_remove_after_with(prev, hash_map_free_item);
            return 1;
        }

        prev = curr;
        curr = curr->next;
    }

    /* not found */
    return 0;
}

/*
 * Looks up an entry by key.
 *
 * Returns:
 *   const HashMapItem* (internal pointer; DO NOT MODIFY the pointed data)
 *   NULL if not found
 */
const HashMapItem* hash_map_get(
                    HashMap* hash_map,
                    const void* key, 
                    size_t key_size)
{
    /* locate bucket */
    uint64_t h64 = generate_hash(key, key_size);
    size_t bucket_index = (size_t)(h64 % HASH_MAP_BUCKET_NUM);
    LinkedList bucket_list = hash_map->buckets[bucket_index];

    if (is_linked_list_empty(bucket_list)) {
        return NULL;
    }

    /* walk bucket */
    LinkedList node = bucket_list;

    while (node != NULL) {
        HashMapItem* item = (HashMapItem*) node->data;

        if (item != NULL &&
            item->hash == h64 &&
            item->key_size == key_size &&
            memcmp(item->key, key, key_size) == 0)
        {
            /* INTERNAL pointer: read-only! */
            return item;
        }

        if (node->next == NULL) {
            return NULL;
        }

        node = node->next;
    }

    return NULL; /* for completeness */
}
