#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>   // fprintf
#include <stdlib.h>  // malloc, free
#include <stddef.h>  // size_t
#include <stdint.h>  // SIZE_MAX
#include <string.h>  // memset, memcpy

/*
    Matrix module does not own data. 
    Data destruction is responsability of the programmmer.
    The library offers method 
*/

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
 * operator table: (MatrixOps)
 * Defines how to handle a single element: write-zero and mul-add.
 * This allows the GEMM(GEneral Matrix–Matrix multiplication) 
 * core to be type-agnostic while still supporting multiple
 * primitive types with correct semantics.
 */
typedef struct MatrixOps {
    size_t elem_size;                       // sizeof(one element)
    void (*set_zero)(void *dst);            // write "zero" for the type into *dst
    void (*muladd)(void *acc, const void *a, const void *b); // *acc += (*a) * (*b)
} MatrixOps;

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

/*
    Multiply matrix for scalar
    we build a and return a new matrix
    User will handle memory by himself
*/
Matrix* matrix_multiply_for_scalar(size_t m, size_t n, void* k, size_t size_of_data);

/* =======================================================================
 *                           MATRIX MULTIPLY
 * =======================================================================
 *
 * Public facade (auto-dispatch by element size — WARNING: ambiguous for
 * some sizes), and explicit typed wrappers for clarity and safety.
 *
 * General contract (applies to all functions below):
 *  - A && B && A->data && B->data
 *  - A->cols == B->rows
 *  - A->size_of_element == B->size_of_element == sizeof(the declared type)
 *  - Data buffers hold row-major contiguous elements of that type.
 *  - Returns a newly-allocated Matrix C on success, or NULL on failure.
 *
 * Variants:
 *  - naive   : baseline triple loop; minimal overhead; good for small matrices
 *  - blocked : cache-friendly tiling; pass BS (block size, e.g., 32/64/128)
 */

 /* WARNING — SIZE-BASED DISPATCH IS AMBIGUOUS
 *
 * This facade picks the operator table by element SIZE only. That is convenient
 * for common floating types but inherently ambiguous for integers:
 *   - 4 bytes could be float, int32_t, or uint32_t
 *   - 8 bytes could be double, int64_t, or size_t (on LP64)
 *
 * Consequence:
 *   If you call matrix_multiply(A,B) with 4-byte NON-float data (int32_t/uint32_t)
 *   or 8-byte NON-double data (int64_t/size_t), the facade may select the WRONG
 *   operator table and produce numerically incorrect results.
 *
 * Recommendation:
 *   For integers and long double, use the explicit typed wrappers:
 *     - matrix_multiply_i64_*   for int64_t
 *     - matrix_multiply_u32_*   for uint32_t
 *     - matrix_multiply_size_*  for size_t
 *     - matrix_multiply_ld_*    for long double
 *   The typed wrappers are unambiguous and enforce the correct element size.
 */


/* matrix_multiply — Facade (size-based dispatch)
 * Computes C = A × B for row-major matrices and returns a newly allocated Matrix C.
 * Dispatch: chooses arithmetic by element SIZE only (float/double preferred).
 * WARNING: 4-byte and 8-byte sizes are ambiguous (float vs int32/uint32, double vs int64/size_t);
 * using this on non-floating data may pick the wrong ops and yield incorrect results.
 * Solution: for integers or long double use the typed wrappers
 * (matrix_multiply_i64_*, _u32_*, _size_*, _ld_*).
 * Errors: returns NULL on dim/type mismatch, overflow, or allocation failure.
 */
Matrix* matrix_multiply(Matrix *A, Matrix *B);


/* Typed wrappers — unambiguous
 * Purpose: bind arithmetic to the stated element type and validate sizeof(type),
 * avoiding size-based ambiguity of the generic facade.
 * Use for integers and long double (recommended), or for floats when you want clarity.
 * Variants:
 *   *_naive   — minimal overhead (good for small matrices)
 *   *_blocked — tiled kernel for cache reuse; pass BS (e.g., 32/64/128)
 */
/* ----- Explicit typed wrappers (unambiguous and recommended) ----- */

/* double (f64) */
Matrix* matrix_multiply_f64_naive   (const Matrix *A, const Matrix *B);
Matrix* matrix_multiply_f64_blocked (const Matrix *A, const Matrix *B, size_t BS);

/* int64_t (i64) */
Matrix* matrix_multiply_i64_naive   (const Matrix *A, const Matrix *B);
Matrix* matrix_multiply_i64_blocked (const Matrix *A, const Matrix *B, size_t BS);

/* uint32_t (u32) */
Matrix* matrix_multiply_u32_naive   (const Matrix *A, const Matrix *B);
Matrix* matrix_multiply_u32_blocked (const Matrix *A, const Matrix *B, size_t BS);

/* size_t (st) */
Matrix* matrix_multiply_size_naive  (const Matrix *A, const Matrix *B);
Matrix* matrix_multiply_size_blocked(const Matrix *A, const Matrix *B, size_t BS);

/* long double (ld) */
Matrix* matrix_multiply_ld_naive    (const Matrix *A, const Matrix *B);
Matrix* matrix_multiply_ld_blocked  (const Matrix *A, const Matrix *B, size_t BS);

/* -----------------------------------------------------------------------
 * Optional (advanced): generic entry points that accept a custom operator
 * table (opaque pointer). This allows you to plug in your own arithmetic
 * (e.g., saturating integers, fixed-point, wider accumulators).
 * To use them, pass the address of an internal MatrixOps instance or your
 * own compatible struct. See matrix.c for details.
 * ----------------------------------------------------------------------- */
Matrix* matrix_multiply_naive_ops   (Matrix *A, Matrix *B, const void *ops_opaque);
Matrix* matrix_multiply_blocked_ops (Matrix *A, Matrix *B, const void *ops_opaque, size_t BS);

/* -----------------------------------------------------------------------
 * ADVANCED: Generic matrix multiply with a custom operator table (opaque).
 *
 * Purpose:
 *   Allow callers to plug in custom arithmetic semantics without changing
 *   the core GEMM loops: e.g., saturating integers, wider accumulators,
 *   fixed-point, modular math, or user-defined numeric types.
 *
 * Operator table (opaque handle):
 *   'ops_opaque' must point to a struct that is layout-compatible with the
 *   library’s internal "MatrixOps":
 *       size_t elem_size;
 *       void (*set_zero)(void *dst);                        // write additive identity
 *       void (*muladd)(void *acc, const void *a, const void *b); // *acc += (*a) * (*b)
 *
 * Contract:
 *   - A && B && A->data && B->data.
 *   - A->cols == B->rows.
 *   - A->size_of_element == B->size_of_element == ops->elem_size.
 *   - Row-major contiguous storage for A, B; result C is newly allocated.
 *
 * Returns:
 *   A newly allocated Matrix C on success, or NULL on error (dimension/type
 *   mismatch, allocation failure, overflow in size computations, etc.).
 *
 * Variants:
 *   - matrix_multiply_naive_ops   : baseline triple loop; minimal overhead.
 *   - matrix_multiply_blocked_ops : tiled kernel; better cache reuse; BS is the
 *                                   block size in elements (typical: 32/64/128).
 *
 * Note:
 *   Advanced API. For standard primitive types prefer the typed
 *   wrappers (matrix_multiply_f64_*, _i64_*, _u32_*, _size_*, _ld_*) or the
 *   default matrix_multiply(A,B) when appropriate.
 * ----------------------------------------------------------------------- */
Matrix* matrix_multiply_naive_ops   (Matrix *A, Matrix *B, const void *ops_opaque);
Matrix* matrix_multiply_blocked_ops (Matrix *A, Matrix *B, const void *ops_opaque, size_t BS);

#endif /* MATRIX_H */
