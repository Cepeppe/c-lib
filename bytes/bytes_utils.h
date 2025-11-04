#ifndef BYTES_UTILS_H
#define BYTES_UTILS_H
#include <stdlib.h>
#include <stdio.h>

#define CLONE_BYTES_FAILURE_EXIT_CODE -1001

static void* clone_bytes(const void* src, size_t size);
#endif