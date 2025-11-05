#include "hashmap.h"

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
 * - Data ownership:
 *     * if deep_deallocate_hashmap_item_data != NULL → ownership of `data`
 *       is TRANSFERRED to the map, which will free it via that callback.
 *     * if deep_deallocate_hashmap_item_data == NULL → the map does NOT
 *       own `data` and will NOT free it.
 *
 * - Thread-safety: NOT thread-safe.
 * ============================================================================
 */

/*
 * 64-bit hash from MurmurHash3_x64_128 (lower 64 bits).
 * Note: MurmurHash3_x64_128 length parameter is `int`, hence the INT_MAX guard.
 */
uint64_t generate_hash(const void* key, size_t key_size) {

    if (key_size > INT_MAX) {
        fprintf(stderr, "You are trying to hash a hash map key that is too long\n");
        exit(TOO_LONG_HASHMAP_KEY);
    }

    /* MurmurHash3_x64_128 writes 128 bits (2x uint64_t). We use the lower 64 bits. */
    uint64_t hash_buffer[2];
    MurmurHash3_x64_128(key, (int)key_size, MUR_MUR_3_SEED, hash_buffer);
    uint64_t h64 = hash_buffer[0];

    return h64;
}

/*
 * Deep-free routine for a LinkedList payload that is a `HashMapItem*` (i.e., node->data).
 *
 * Compatible with linked_list_remove_hashmap_node_with(...), which passes:
 *   - `data` as the HashMapItem*,
 *   - `deep_deallocate_hashmap_item_data` as the optional deallocator for item->data.
 *
 * Frees, in order:
 *   1) item->key            (always heap-allocated by the map),
 *   2) item->data           (ONLY if a data-deallocator is provided),
 *   3) the `item` struct itself.
 */
static void hash_map_free_item_with(void* data,
                                    void (*deep_deallocate_hashmap_item_data)(void* node_data)) {
    if (data == NULL) return;

    HashMapItem* item = (HashMapItem*)data;
    free(item->key);

    if (deep_deallocate_hashmap_item_data != NULL) {
        deep_deallocate_hashmap_item_data(item->data);
    }

    free(item);
    return;
}

/*
 * Builds an empty HashMap with HASH_MAP_BUCKET_NUM buckets.
 * Each bucket is initialized as an empty LinkedList (sentinel head).
 */
HashMap* build_hash_map(void) {
    HashMap* hash_map = malloc(sizeof(HashMap));
    if (hash_map == NULL) {
        fprintf(stderr, "Failed malloc while trying to build a new hash map\n");
        exit(FAILED_HASH_MAP_ALLOCATION);
    }

    for (size_t i = 0; i < HASH_MAP_BUCKET_NUM; i++) {
        hash_map->buckets[i] = build_empty_linked_list();
    }

    return hash_map;
}

/*
 * Destroys the entire HashMap.
 * - `deep_deallocate_hashmap_item_data` deallocates HashMapItem->data (type-specific).
 *   It can be NULL when `data` is not dynamically allocated / not owned by the map.
 *
 * Implementation note:
 * We detach each node (set node->next = NULL) before handing it to
 * linked_list_remove_hashmap_node_with(...) to adhere to that function’s contract
 * (“standalone, already-detached node”).
 */
void hash_map_destroy(HashMap* hash_map,
                      void (*deep_deallocate_hashmap_item_data)(void* node_data)) {
    if (hash_map == NULL) {
        fprintf(stderr, "You tried to destroy a NULL hash map, this is a no-op\n");
        return;
    }

    for (size_t i = 0; i < HASH_MAP_BUCKET_NUM; i++) {
        LinkedList bucket_head = hash_map->buckets[i];

        while (bucket_head != NULL) {
            LinkedListNode* node = bucket_head;
            bucket_head = bucket_head->next;

            /* Detach to satisfy the “standalone node” precondition */
            node->next = NULL;

            linked_list_remove_hashmap_node_with(
                node,                              /* the node to free */
                hash_map_free_item_with,           /* frees HashMapItem + key + (optional) data */
                deep_deallocate_hashmap_item_data  /* (optional) data deallocator */
            );
        }
    }

    free(hash_map);
    return;
}

/*
 * Inserts or updates an entry (upsert).
 *
 * Returns:
 *   1 if the key already existed and its value was updated
 *   0 if this was a brand-new insertion
 *
 * Ownership rules recap:
 *   - The key is ALWAYS deep-copied here and thus owned/freed by the map.
 *   - `data` ownership is transferred to the map ONLY if
 *     `deep_deallocate_hashmap_item_data != NULL`; otherwise the map will not free it.
 */
int hash_map_put(HashMap* hash_map,
                 const void* key,
                 size_t key_size,
                 const void* data,
                 size_t data_size,
                 void (*deep_deallocate_hashmap_item_data)(void* node_data))
{
    if (hash_map == NULL) {
        fprintf(stderr, "You are trying to put data in a NULL hash map; did you call build_hash_map(void)?\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_HASHMAP);
    }

    /* Hash the key and pick the bucket */
    uint64_t h64 = generate_hash(key, key_size);
    size_t bucket_index = (size_t)(h64 % HASH_MAP_BUCKET_NUM);
    LinkedList bucket_list = hash_map->buckets[bucket_index];

    if (is_linked_list_null(bucket_list)) {
        fprintf(stderr, "Unexpected error: hash map buckets initialized, but a bucket list is NULL\n");
        exit(ATTEMPTED_ACCESS_TO_NULL_HASHMAP_BUCKET);
    }

    /* Empty logical list → insert as first item */
    if (is_linked_list_empty(bucket_list)) {

        void* key_copy = malloc(key_size);
        if (!key_copy) {
            fprintf(stderr, "Failed hash map put operation while copying key\n");
            exit(FAILED_HASH_MAP_ALLOCATION);
        }
        memcpy(key_copy, key, key_size);

        HashMapItem new_item_val;
        new_item_val.hash      = h64;
        new_item_val.key       = key_copy;     /* map owns and frees this */
        new_item_val.key_size  = key_size;
        new_item_val.data      = data;         /* ownership depends on callback presence */
        new_item_val.data_size = data_size;

        linked_list_push_back(bucket_list, ll_alloc_HashMapItem(new_item_val));
        return 0; /* new key */
    }

    /* Bucket has nodes → walk and search for key */
    LinkedList node = bucket_list;
    while (node != NULL) {
        HashMapItem* item = (HashMapItem*)node->data;

        if (item != NULL &&
            item->hash == h64 &&
            item->key_size == key_size &&
            memcmp(item->key, key, key_size) == 0)
        {
            /* Found existing key → update value.
             * If the map owns the old value (callback provided), free it first. */
            if (item->data != NULL && deep_deallocate_hashmap_item_data != NULL) {
                deep_deallocate_hashmap_item_data(item->data);
            }
            item->data      = data;       /* new value (ownership as per callback presence) */
            item->data_size = data_size;
            return 1; /* updated existing */
        }

        if (node->next == NULL) {
            /* Reached tail with no match → append */

            void* key_copy = malloc(key_size);
            if (!key_copy) {
                fprintf(stderr, "Failed hash map put operation while copying key\n");
                exit(FAILED_HASH_MAP_ALLOCATION);
            }
            memcpy(key_copy, key, key_size);

            HashMapItem new_item_val;
            new_item_val.hash      = h64;
            new_item_val.key       = key_copy;  /* map owns and frees this */
            new_item_val.key_size  = key_size;
            new_item_val.data      = data;      /* ownership depends on callback presence */
            new_item_val.data_size = data_size;

            linked_list_push_back(node, ll_alloc_HashMapItem(new_item_val));
            return 0; /* inserted new */
        }

        node = node->next;
    }

    /* Unreachable in normal flow */
    return 0;
}

/*
 * Removes an entry by key.
 *
 * Returns:
 *   1 if a matching entry was removed
 *   0 if the key was not found
 *
 * Implementation detail:
 * For head removal with multiple elements, we do not change the head pointer;
 * instead, we promote the second node's payload into the head and free the second node.
 */
int hash_map_remove(HashMap* hash_map,
                    const void* key,
                    size_t key_size,
                    void (*deep_deallocate_hashmap_item_data)(void* node_data))
{
    if (hash_map == NULL) {
        fprintf(stderr, "hash_map_remove called on a NULL hash map\n");
        return 0;
    }

    /* Locate bucket */
    uint64_t h64 = generate_hash(key, key_size);
    size_t bucket_index = (size_t)(h64 % HASH_MAP_BUCKET_NUM);
    LinkedList bucket_head = hash_map->buckets[bucket_index];

    /* Empty logical list → nothing to remove */
    if (is_linked_list_empty(bucket_head)) {
        return 0;
    }

    /* Case 1: check head */
    {
        HashMapItem* first_item = (HashMapItem*)bucket_head->data;

        if (first_item != NULL &&
            first_item->hash == h64 &&
            first_item->key_size == key_size &&
            memcmp(first_item->key, key, key_size) == 0)
        {
            if (bucket_head->next == NULL) {
                /* Single-node bucket: free payload and mark head empty */
                hash_map_free_item_with(first_item, deep_deallocate_hashmap_item_data);
                bucket_head->data = NULL;
                return 1;
            } else {
                /* Multi-node bucket: free head payload, promote second into head, free second node */
                hash_map_free_item_with(first_item, deep_deallocate_hashmap_item_data);

                LinkedListNode* second = bucket_head->next;

                bucket_head->data = second->data;  /* transfer payload */
                bucket_head->next = second->next;

                /* second is now a standalone node with NULLed fields */
                second->data = NULL;
                second->next = NULL;

                /* Free only the node structure (payload already transferred) */
                linked_list_destroy(second);
                return 1;
            }
        }
    }

    /* Case 2: walk with (prev, curr) and remove if found */
    LinkedList prev = bucket_head;
    LinkedList curr = bucket_head->next;

    while (curr != NULL) {
        HashMapItem* item = (HashMapItem*)curr->data;

        if (item != NULL &&
            item->hash == h64 &&
            item->key_size == key_size &&
            memcmp(item->key, key, key_size) == 0)
        {
            /* Stitch neighbors, then free the standalone `curr` node */
            prev->next = curr->next;
            linked_list_remove_hashmap_node_with(
                curr,
                hash_map_free_item_with,
                deep_deallocate_hashmap_item_data
            );
            return 1;
        }

        prev = curr;
        curr = curr->next;
    }

    /* Not found */
    return 0;
}

/*
 * Looks up an entry by key.
 *
 * Returns:
 *   const HashMapItem* (INTERNAL pointer; DO NOT MODIFY or FREE it)
 *   NULL if not found
 */
const HashMapItem* hash_map_get(HashMap* hash_map,
                                const void* key,
                                size_t key_size)
{
    if (hash_map == NULL) return NULL;

    /* Locate bucket */
    uint64_t h64 = generate_hash(key, key_size);
    size_t bucket_index = (size_t)(h64 % HASH_MAP_BUCKET_NUM);
    LinkedList bucket_list = hash_map->buckets[bucket_index];

    if (is_linked_list_empty(bucket_list)) {
        return NULL;
    }

    /* Walk bucket */
    LinkedList node = bucket_list;

    while (node != NULL) {
        HashMapItem* item = (HashMapItem*)node->data;

        if (item != NULL &&
            item->hash == h64 &&
            item->key_size == key_size &&
            memcmp(item->key, key, key_size) == 0)
        {
            /* INTERNAL pointer: read-only, lifetime managed by the map */
            return item;
        }

        if (node->next == NULL) {
            return NULL;
        }

        node = node->next;
    }

    return NULL; /* for completeness */
}
