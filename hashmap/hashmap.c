#include "hashmap.h"
#include <string.h>  /* memcpy, memcmp */



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
            new_item_val.key       = clone_bytes(key, key_size);
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


/*
 * Remove the entry with the given key (if present).
 *
 * Returns:
 *   1 -> key was found and removed
 *   0 -> key not found
 *
 * This function:
 *  - frees the key bytes
 *  - frees the value bytes
 *  - frees the HashMapItem struct
 *  - fixes the linked list links
 *  - keeps bucket head stable (we never lose the pointer to the first node)
 *
 * IMPORTANT:
 * no call to linked_list_remove_at_index() 
 * hash map owns its data and performs his own deep cleanup:
 *   free(item->key);
 *   free(item->data);
 *   free(item);
 */
int hash_map_remove(HashMap* hash_map,
                    const void* key,
                    size_t key_size)
{
    if (hash_map == NULL) {
        fprintf(stderr, "hash_map_remove called with NULL hash_map\n");
        return 0;
    }

    /* hash the key and locate the correct bucket */
    uint64_t h64 = generate_hash(key, key_size);
    size_t bucket_index = (size_t)(h64 % HASH_MAP_BUCKET_NUM);
    LinkedList bucket_head = hash_map->buckets[bucket_index];

    /* empty logical list -> nothing to remove */
    if (is_linked_list_empty(bucket_head)) {
        return 0;
    }

    /*
     * CASE 1: the element to remove might be in the head node (index 0).
     * We handle index 0 specially because linked list implementation
     * keeps the same head node pointer instead of freeing/replacing it.
     */
    {
        HashMapItem* first_item = (HashMapItem*) bucket_head->data;

        if (first_item != NULL &&
            first_item->hash == h64 &&
            first_item->key_size == key_size &&
            memcmp(first_item->key, key, key_size) == 0)
        {
            /* We found the key in the first node. */

            if (bucket_head->next == NULL) {
                /*
                 * The bucket has only one node.
                 * We free the item's deep data (key/data + item),
                 * then turn this node into a logical empty list
                 */

                if (first_item->key  != NULL) free(first_item->key);
                if (first_item->data != NULL) free(first_item->data);

                linked_list_remove_first(bucket_head);

                bucket_head->data = NULL;
                /* bucket_head->next is already NULL */

                return 1;
            } else {
                /*
                 * The bucket has at least two nodes.
                 * We want to remove the first logical element,
                 * but we can't just free the head node because code
                 * outside still holds bucket_head.
                 *  - Free the current head's item (deep free).
                 *  - Promote the second node into the head:
                 *      head->data = second->data;
                 *      head->next = second->next;
                 *    Then free ONLY the second node struct (not its data,
                 *    because we just moved that pointer to head->data).
                 */

                LinkedListNode* second = bucket_head->next;

                /* deep free the old head's item */
                if (first_item->key  != NULL) free(first_item->key);
                if (first_item->data != NULL) free(first_item->data);
                linked_list_remove_first(bucket_head);

                return 1;
            }
        }
    }

    /*
     * CASE 2: the element to remove is not the head.
     * We walk the list with (prev, curr) and remove curr when we find a match.
     */
    LinkedList prev = bucket_head;
    LinkedList curr = bucket_head->next;

    while (curr != NULL) {
        HashMapItem* item = (HashMapItem*) curr->data;

        if (item != NULL &&
            item->hash == h64 &&
            item->key_size == key_size &&
            memcmp(item->key, key, key_size) == 0)
        {
            /*
             * Unlink `curr` from the chain
             *   prev -> curr -> next
             */
            prev->next = curr->next;

            /*
             * Deep free the HashMapItem:
             * - free key bytes
             * - free value bytes
             * - free the item struct itself
             */
            if (item->key  != NULL) free(item->key);
            if (item->data != NULL) free(item->data);
            
            linked_list_remove_first(bucket_head);

            return 1;
        }

        prev = curr;
        curr = curr->next;
    }

    /* key not found */
    return 0;
}

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
                    size_t key_size){
                        
    /* hash key */
    uint64_t h64 = generate_hash(key, key_size);

    /* pick bucket */
    size_t bucket_index = (size_t)(h64 % HASH_MAP_BUCKET_NUM);
    LinkedList bucket_list = hash_map->buckets[bucket_index];

    /* empty logical list case */
    if (is_linked_list_empty(bucket_list)) {
        return NULL;
    }

    /* bucket list has nodes */
    LinkedList node = bucket_list;

    HashMapItem* item; //inside here the result is placed

    while (node != NULL) {
        item = (HashMapItem*) node->data;

        if (item != NULL &&
            item->hash == h64 &&
            item->key_size == key_size &&
            memcmp(item->key, key, key_size) == 0)
        {
            return item;
        }

        if (node->next == NULL) {
            
            return NULL;
        }

        node = node->next;
    }

    /* shouldn't get here */
    return 0;

}
