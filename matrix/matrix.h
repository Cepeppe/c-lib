#ifndef MATRIX_H
#define MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>   // fprintf
#include <stdlib.h>  // malloc, free
#include <stddef.h>  // size_t
#include <stdint.h>  // SIZE_MAX
#include <string.h>  // memset, memcpy

// Optional error codes (present even if not used in .c)
#define INVALID_MATRIX_CREATION_PARAMETER -80
#define FAILED_MATRIX_ALLOCATION         -79

typedef struct Matrix {
    size_t rows;             // number of rows (M)
    size_t cols;             // number of columns (N)
    size_t size_of_element;  // sizeof(element), e.g., sizeof(double)
    void  *data;             // contiguous block: rows * cols * size_of_element
    void **row;              // row[i] points to the start of row i within data
} Matrix;

/*
 * Internal helper: multiply a*b with overflow check.
 * Returns 1 if OVERFLOW occurred, 0 if OK. On OK, *out = a*b.
 */
static inline int mul_overflow_size_t(size_t a, size_t b, size_t *out) {
    if (a == 0 || b == 0) { *out = 0; return 0; }
    if (a > SIZE_MAX / b)  { return 1; }
    *out = a * b;
    return 0;
}

/*
 * Internal helper: compute a*b*c into *out with overflow check.
 * Returns 1 if OK (no overflow) and sets *out; returns 0 if OVERFLOW.
 * (Matches usage in new_matrix: if (!size_mul3_checked(...)) => error)
 */
static inline int size_mul3_checked(size_t a, size_t b, size_t c, size_t *out) {
    size_t ab;
    if (mul_overflow_size_t(a, b, &ab)) return 0;        // overflow -> fail
    if (mul_overflow_size_t(ab, c, out)) return 0;       // overflow -> fail
    return 1;                                            // OK
}

/*
 * Builds and allocs Matrix.
 * m: rows, n: cols, size_of_element: sizeof(element type)
 * Returns NULL on failure.
 */
Matrix* new_matrix(size_t m, size_t n, size_t size_of_element);

/*
 * Destroys matrix (no-op on NULL).
 */
void destroy_matrix(Matrix* matrix);

/*
 * Fill matrix with a scalar value pointed by k.
 * Returns 1 on success, 0 on failure.
 * NOTE: k must point to a single element of the matrix's element type.
 */
int matrix_fill_scalar(Matrix *matrix, void *k);

/*
 * Build matrix m x n and set all elements to *k.
 * k points to a primitive value; complex types are NOT supported.
 * size_of_data must be sizeof(*k).
 * Returns NULL on failure.
 */
Matrix* build_all_k_matrix(size_t m, size_t n, void* k, size_t size_of_data);

#ifdef __cplusplus
}
#endif

#endif /* MATRIX_H */
