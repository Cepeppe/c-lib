#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "../string/string.h"
#include "../linked_list/linked_list.h"

#define HASH_MAP_BUCKET_NUM 500 // 500 * 4BYTE = 2 kb

#define FAILED_HASH_MAP_ALLOCATION -96

typedef struct HashMapItem{
    string* hash;
    void* data;
    size_t data_size;
} HashMapItem;

typedef struct HashMap{
    LinkedList hash_map_buckets[HASH_MAP_BUCKET_NUM];
} HashMap;

HashMap* build_hash_map();
void hash_map_destroy(HashMap* HashMap);

#endif