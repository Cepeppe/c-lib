#include "bytes_utils.h"

/* clone arbitrary bytes into a new heap buffer that we own */
static void* clone_bytes(const void* src, size_t size) {
    if (size == 0) {
        return NULL;
    }
    void* dst = malloc(size);
    if (dst == NULL) {
        fprintf(stderr, "malloc failed in clone_bytes\n");
        exit(CLONE_BYTES_FAILURE_EXIT_CODE);
    }
    memcpy(dst, src, size);
    return dst;
}