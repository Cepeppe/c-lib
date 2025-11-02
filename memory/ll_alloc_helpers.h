#ifndef LL_ALLOC_HELPERS_H
#define LL_ALLOC_HELPERS_H
#include<stdlib.h>
#include <stdio.h>
#include <stdlib.h>

#define MALLOC_FAILURE_EXIT_CODE -99
/* allocators for common types */
void* ll_alloc_int(int value);
void* ll_alloc_float(float value);
void* ll_alloc_double(double value);
void* ll_alloc_char(char value);


#endif
