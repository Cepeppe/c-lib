#include "ll_alloc_helpers.h"
#include "../hashmap/hashmap.h"

void* ll_alloc_int(int value) {
    int* p = (int*) malloc(sizeof(int));
    if (p == NULL) {
        fprintf(stderr, "malloc failed in ll_alloc_int\n");
        exit(MALLOC_FAILURE_EXIT_CODE);
    }
    *p = value;
    return (void*) p;
}

void* ll_alloc_float(float value) {
    float* p = (float*) malloc(sizeof(float));
    if (p == NULL) {
        fprintf(stderr, "malloc failed in ll_alloc_float\n");
        exit(MALLOC_FAILURE_EXIT_CODE);
    }
    *p = value;
    return (void*) p;
}

void* ll_alloc_double(double value) {
    double* p = (double*) malloc(sizeof(double));
    if (p == NULL) {
        fprintf(stderr, "malloc failed in ll_alloc_double\n");
        exit(MALLOC_FAILURE_EXIT_CODE);
    }
    *p = value;
    return (void*) p;
}

void* ll_alloc_char(char value){
    char* p = (char*) malloc(sizeof(char));
    if (p == NULL) {
        fprintf(stderr, "malloc failed in ll_alloc_char\n");
        exit(MALLOC_FAILURE_EXIT_CODE);
    }
    *p = value;
    return (void*) p;
}

//ll_alloc_HashMapItem: allocates struct on heap and peforms shallow copy of fields
//DOES NOT PERFORM KEY DEEP COPY
void* ll_alloc_HashMapItem(HashMapItem value) {
    HashMapItem* p = (HashMapItem*) malloc(sizeof(HashMapItem));
    if (p == NULL) {
        fprintf(stderr, "malloc failed in ll_alloc_HashMapItem\n");
        exit(MALLOC_FAILURE_EXIT_CODE);
    }
    *p = value;
    return (void*) p;
}