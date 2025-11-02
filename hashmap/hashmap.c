#include "hashmap.h"

HashMap* build_hash_map(){

    HashMap* hash_map = malloc(sizeof(HashMap));
    if(hash_map == NULL){
        fprintf(stderr, "Failed malloc while trying to build new hash map\n");
        exit(FAILED_HASH_MAP_ALLOCATION);
    }
    
    for(int i=0; i<HASH_MAP_BUCKET_NUM; i++){
        hash_map->hash_map_buckets[i] = build_empty_linked_list();
    }

    return hash_map;
}

void hash_map_destroy(HashMap* HashMap){
    //TODO
}