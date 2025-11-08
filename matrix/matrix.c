#include "matrix.h"

/*
    Builds and allocs Matrix.
*/
Matrix* new_matrix(size_t m, size_t n, size_t size_of_element){

    if (m == 0 || n == 0 || size_of_element == 0) {
        fprintf(stderr, "new_matrix: one or more construction parameters are invalid: can not be 0\n");
        return NULL;
    }

    size_t bytes;
    if (!size_mul3_checked(m, n, size_of_element, &bytes)) {
        fprintf(stderr, "new_matrix: Unable to allocate requested matrix, requested sizes would result in an overflow for size_t datatype\n");
        return NULL;
    }

    Matrix* matrix;
    matrix=malloc(sizeof(Matrix));

    if(!matrix){
        fprintf(stderr, "new_matrix: Failed malloc while trying to allocate new Matrix structure\n");
        return NULL;
    }

    matrix->rows=m;
    matrix->cols=n;
    matrix->size_of_element = size_of_element;
    matrix->data=NULL;
    matrix->row=NULL;

    matrix->data = malloc(bytes);
    if(!matrix->data){
        fprintf(stderr, "new_matrix: Failed malloc while trying to allocate Matrix data memory space\n");
        free(matrix);
        return NULL;
    }

    if (mul_overflow_size_t(m, sizeof(void*), &bytes)) {
        fprintf(stderr, "new_matrix: overflow computing row table size\n");
        free(matrix->data);
        free(matrix);
        return NULL;
    }

    matrix->row = malloc(bytes);
    if(!matrix->row){
        fprintf(stderr, "new_matrix: Failed malloc while trying to allocate Matrix row pointers array\n");
        free(matrix->data);
        free(matrix);
        return NULL;
    }

    for(size_t i=0; i<m; i++){
        /* 
         cast to char* to work with byte arithmetic (couldn't add anything to a void*)
         we jump to the beginning ot row i: each time we calculate new total offset 
         multiplying the number of elements in the row for size of 
         each elements for the row of interest
        */
        matrix->row[i] = (char*) matrix->data + (size_t) i * n * size_of_element;
    }

    return matrix;
}


/*
    Destroys matrix
*/
void destroy_matrix(Matrix* matrix){
    if(!matrix) return;
    free(matrix->data);
    free(matrix->row);
    free(matrix);
}

/*
    Fill matrix with a scalar value pointed by k
    returns 1 if success 0 if failure
*/
int matrix_fill_scalar(Matrix *matrix, void *k) {
    if (!matrix || !k) return 0;

    // for 1-byte elements (e.g., uint8_t/char) can use memset
    if (matrix->size_of_element == 1) {
        memset(matrix->data, *(unsigned char*)k, matrix->rows * matrix->cols);
        return 1;
    }

    // Generic path: replicate per element
    char *p = (char*)matrix->data;
    const size_t step = matrix->size_of_element;
    const size_t total = (size_t) (matrix->rows * matrix->cols);

    //repeat total time, increase pointer of step each time
    for (size_t idx = 0; idx < total; ++idx, p += step) {
        memcpy(p, k, step);
    }

    return 1;
}

/*
    Builds matrix and sets all element values to k 
    k is a void* , it contains pointer to a primitive data type variable
    COMPLEX DATA TYPES ARE NOT SUPPORTED
    size_of_data is the size of data contained inside k (*k)
*/
Matrix* build_all_k_matrix(size_t m, size_t n, void* k, size_t size_of_data){

    Matrix* matrix = new_matrix(m, n, size_of_data);
    if(matrix == NULL) return NULL;

    if(!matrix_fill_scalar(matrix, k)){
        fprintf(stderr, "Failed matrix_fill_scalar in build_all_k_matrix\n");
        destroy_matrix(matrix);
        return NULL;
    }

    return matrix;
}

/*
    Multiply matrix for scalar
    we build a and return a new matrix
    User will handle memory by himself
*/
Matrix* matrix_multiply_for_scalar(size_t m, size_t n, void* k, size_t size_of_data){

    Matrix* matrix = new_matrix(m, n, size_of_data);
    if(matrix == NULL) return NULL;

    if(!matrix_fill_scalar(matrix, k)){
        fprintf(stderr, "Failed matrix_fill_scalar in build_all_k_matrix\n");
        destroy_matrix(matrix);
        return NULL;
    }

    return matrix;
}
 
/*                            MATRIX MULTIPLY
 * Generic matrix multiplication using a small "operator table" (MatrixOps)
 * that defines how to handle a single element: write-zero and mul-add.
 * This allows the GEMM core to be type-agnostic while still supporting
 * multiple primitive types with correct semantics.
 *
 * Public API exposed in matrix.h:
 *  - Matrix* matrix_multiply(Matrix* A, Matrix* B)
 *      Auto-picks a built-in type by element size (see warnings below).
 *  - Typed wrappers to be explicit and unambiguous:
 *      * f64 (double), i64 (int64_t), u32 (uint32_t), size_t, long double
 *      Each has naive() and blocked(BS) variants.
 *
 * Safety & semantics:
 *  - Validates A && B && data buffers, and A->cols == B->rows.
 *  - Checks total bytes m*n*elem_size with overflow-protected math.
 *  - Initializes C to the additive identity using the type's set_zero().
 *  - Integer variants use the platform's native semantics:
 *      * unsigned wrap-around (well-defined)
 *      * signed overflow is UB in standard C (on most platforms it wraps);
 *        if you need saturation or wider accumulation, provide a custom ops.
 */


/* Built-in operator tables (zero + muladd) */

static void mm_zero_f64(void *dst) { *(double*)dst = 0.0; }
static void mm_muladd_f64(void *acc, const void *a, const void *b) {
    *(double*)acc += (*(const double*)a) * (*(const double*)b);
}
static const MatrixOps MM_OPS_F64 = { sizeof(double), mm_zero_f64, mm_muladd_f64 };

static void mm_zero_f32(void *dst) { *(float*)dst = 0.0f; }
static void mm_muladd_f32(void *acc, const void *a, const void *b) {
    *(float*)acc += (*(const float*)a) * (*(const float*)b);
}
static const MatrixOps MM_OPS_F32 = { sizeof(float), mm_zero_f32, mm_muladd_f32 };

static void mm_zero_i32(void *dst) { *(int32_t*)dst = 0; }
static void mm_muladd_i32(void *acc, const void *a, const void *b) {
    // WARNING: signed overflow is UB in C. Most mainstream targets wrap.
    *(int32_t*)acc += (*(const int32_t*)a) * (*(const int32_t*)b);
}
static const MatrixOps MM_OPS_I32 = { sizeof(int32_t), mm_zero_i32, mm_muladd_i32 };

static void mm_zero_i64(void *dst) { *(int64_t*)dst = 0; }
static void mm_muladd_i64(void *acc, const void *a, const void *b) {
    // WARNING: signed 64-bit overflow is UB in C. Most platforms wrap.
    *(int64_t*)acc += (*(const int64_t*)a) * (*(const int64_t*)b);
}
static const MatrixOps MM_OPS_I64 = { sizeof(int64_t), mm_zero_i64, mm_muladd_i64 };

static void mm_zero_u32(void *dst) { *(uint32_t*)dst = 0u; }
static void mm_muladd_u32(void *acc, const void *a, const void *b) {
    // Unsigned math is modulo 2^32 by C standard (well-defined wrap).
    *(uint32_t*)acc += (*(const uint32_t*)a) * (*(const uint32_t*)b);
}
static const MatrixOps MM_OPS_U32 = { sizeof(uint32_t), mm_zero_u32, mm_muladd_u32 };

static void mm_zero_st(void *dst) { *(size_t*)dst = (size_t)0; }
static void mm_muladd_st(void *acc, const void *a, const void *b) {
    // size_t is an unsigned integer; width depends on target (32/64-bit).
    *(size_t*)acc += (*(const size_t*)a) * (*(const size_t*)b);
}
static const MatrixOps MM_OPS_ST = { sizeof(size_t), mm_zero_st, mm_muladd_st };

static void mm_zero_ld(void *dst) { *(long double*)dst = 0.0L; }
static void mm_muladd_ld(void *acc, const void *a, const void *b) {
    *(long double*)acc += (*(const long double*)a) * (*(const long double*)b);
}
static const MatrixOps MM_OPS_LD = { sizeof(long double), mm_zero_ld, mm_muladd_ld };

/*  Internal helpers  */

/* Return minimum of two size_t values. */
static inline size_t mm_min_size(size_t a, size_t b) { return (a < b) ? a : b; }

/* Element-wise zeroing: safer than memset(0) for exotic types. */
static void mm_zero_buffer_elemwise(void *data, size_t elem_count, const MatrixOps *ops) {
    char *ptr = (char*)data;
    for (size_t i = 0; i < elem_count; ++i) {
        ops->set_zero(ptr + i * ops->elem_size);
    }
}

/* Validate pointers, dimensions and elem_size vs ops. */
static int mm_check_preconditions_generic(const Matrix *A, const Matrix *B,
                                          const MatrixOps *ops,
                                          size_t *m, size_t *p, size_t *n) {
    if (!A || !B) {
        fprintf(stderr, "matrix_multiply: NULL matrix pointer (A or B)\n");
        return 0;
    }
    if (!A->data || !B->data) {
        fprintf(stderr, "matrix_multiply: NULL data buffer (A->data or B->data)\n");
        return 0;
    }
    if (A->cols != B->rows) {
        fprintf(stderr, "matrix_multiply: dimension mismatch (A.cols != B.rows)\n");
        return 0;
    }
    if (!ops) {
        fprintf(stderr, "matrix_multiply: missing MatrixOps table\n");
        return 0;
    }
    if (A->size_of_element != B->size_of_element ||
        A->size_of_element != ops->elem_size) {
        fprintf(stderr,
            "matrix_multiply: element size/type mismatch (A=%zu, B=%zu, ops=%zu bytes)\n",
            A->size_of_element, B->size_of_element, ops->elem_size);
        return 0;
    }

    *m = A->rows;      // rows of A
    *p = A->cols;      // cols of A == rows of B
    *n = B->cols;      // cols of B
    return 1;
}

/*
 * mm_pick_builtin_ops_by_size — internal selector
 * Returns a MatrixOps* chosen only from element SIZE.
 * Handy for float/double, but ambiguous for integers (4 or 8 bytes collide).
 * If matrices hold non-floating types, prefer the typed wrappers to avoid wrong-ops dispatch.
 */
static const MatrixOps* mm_pick_builtin_ops_by_size(size_t elem_size) {
    if (elem_size == sizeof(long double) && sizeof(long double) != sizeof(double))
        return &MM_OPS_LD; // only if size is distinct from double
    if (elem_size == sizeof(double))  return &MM_OPS_F64;
    if (elem_size == sizeof(float))   return &MM_OPS_F32;
    if (elem_size == sizeof(int32_t)) return &MM_OPS_I32;
    if (elem_size == sizeof(int64_t)) return &MM_OPS_I64;
    if (elem_size == sizeof(uint32_t))return &MM_OPS_U32;
    if (elem_size == sizeof(size_t))  return &MM_OPS_ST;
    return NULL;
}

/*  Core GEMM implementations  */

/*
 * NAIVE GENERIC GEMM (row-major)
 * Computes C = A * B with triple-nested loops.
 * Order i-k-j improves reuse of B's rows in cache.
 * Complexity: O(m*p*n)
 */
static Matrix* matrix_multiply_ex_naive(const Matrix *A, const Matrix *B, const MatrixOps *ops) {
    size_t m, p, n;
    if (!mm_check_preconditions_generic(A, B, ops, &m, &p, &n)) return NULL;

    Matrix *C = new_matrix(m, n, ops->elem_size);
    if (!C) {
        fprintf(stderr, "matrix_multiply: failed to allocate result matrix\n");
        return NULL;
    }

    size_t total_bytes;
    if (!size_mul3_checked(m, n, ops->elem_size, &total_bytes)) {
        fprintf(stderr, "matrix_multiply: size overflow computing C bytes\n");
        destroy_matrix(C);
        return NULL;
    }
    mm_zero_buffer_elemwise(C->data, total_bytes / ops->elem_size, ops);

    const size_t as = A->cols;  // p
    const size_t bs = B->cols;  // n
    const size_t cs = C->cols;  // n

    const char *ad = (const char*)A->data;
    const char *bd = (const char*)B->data;
    char       *cd = (char*)C->data;

    for (size_t i = 0; i < m; ++i) {
        for (size_t k = 0; k < p; ++k) {
            const char *Aik = ad + ((i * as + k) * ops->elem_size);
            const char *Bk  = bd + ((k * bs) * ops->elem_size);   // row k of B
            char       *Ci  = cd + ((i * cs) * ops->elem_size);   // row i of C
            for (size_t j = 0; j < n; ++j) {
                char *Cij = Ci + (j * ops->elem_size);
                const char *Bkj = Bk + (j * ops->elem_size);
                ops->muladd(Cij, Aik, Bkj);
            }
        }
    }

    return C;
}

/*
 * BLOCKED (tiled) GENERIC GEMM
 * Processes submatrices (tiles) of size BS to improve cache locality.
 * BS is expressed in elements (e.g., 32, 64, 128). Tune to your CPU.
 */
static Matrix* matrix_multiply_ex_blocked(const Matrix *A, const Matrix *B,
                                          const MatrixOps *ops, size_t BS) {
    size_t m, p, n;
    if (!mm_check_preconditions_generic(A, B, ops, &m, &p, &n)) return NULL;
    if (BS == 0) BS = 64;

    Matrix *C = new_matrix(m, n, ops->elem_size);
    if (!C) {
        fprintf(stderr, "matrix_multiply: failed to allocate result matrix\n");
        return NULL;
    }

    size_t total_bytes;
    if (!size_mul3_checked(m, n, ops->elem_size, &total_bytes)) {
        fprintf(stderr, "matrix_multiply: size overflow computing C bytes\n");
        destroy_matrix(C);
        return NULL;
    }
    mm_zero_buffer_elemwise(C->data, total_bytes / ops->elem_size, ops);

    const size_t as = A->cols;  // p
    const size_t bs = B->cols;  // n
    const size_t cs = C->cols;  // n

    const char *ad = (const char*)A->data;
    const char *bd = (const char*)B->data;
    char       *cd = (char*)C->data;

    for (size_t ii = 0; ii < m; ii += BS) {
        const size_t i_max = mm_min_size(ii + BS, m);
        for (size_t kk = 0; kk < p; kk += BS) {
            const size_t k_max = mm_min_size(kk + BS, p);
            for (size_t jj = 0; jj < n; jj += BS) {
                const size_t j_max = mm_min_size(jj + BS, n);

                for (size_t i = ii; i < i_max; ++i) {
                    char *Ci = cd + ((i * cs) * ops->elem_size);
                    for (size_t k = kk; k < k_max; ++k) {
                        const char *Aik = ad + ((i * as + k) * ops->elem_size);
                        const char *Bk  = bd + ((k * bs) * ops->elem_size);
                        for (size_t j = jj; j < j_max; ++j) {
                            char *Cij = Ci + (j * ops->elem_size);
                            const char *Bkj = Bk + (j * ops->elem_size);
                            ops->muladd(Cij, Aik, Bkj);
                        }
                    }
                }
            }
        }
    }

    return C;
}

/* Public facade + typed wrappers */

/* 
 * matrix_multiply — facade with heuristic kernel choice
 * Purpose: C = A × B (row-major). Allocates C and runs the naive or blocked kernel.
 * Dispatch: picks a built-in MatrixOps by element SIZE (float/double preferred).
 * WARNING: size-based dispatch is ambiguous (4B: float/int32/uint32, 8B: double/int64/size_t).
 * If A/B store integers or long double, call the typed wrappers instead.
 * Errors: returns NULL on NULL inputs, dim/type mismatch, overflow, or allocation failure.
 */

Matrix* matrix_multiply(Matrix *A, Matrix *B) {
    if (!A || !B) {
        fprintf(stderr, "matrix_multiply: NULL matrix pointer\n");
        return NULL;
    }
    if (A->cols != B->rows) {
        fprintf(stderr, "matrix_multiply: dimension mismatch (A.cols != B.rows)\n");
        return NULL;
    }
    if (B->size_of_element != A->size_of_element) {
        fprintf(stderr, "matrix_multiply: element size mismatch between A and B\n");
        return NULL;
    }

    const MatrixOps *ops = mm_pick_builtin_ops_by_size(A->size_of_element);
    if (!ops) {
        fprintf(stderr,
            "matrix_multiply: unsupported or ambiguous element size %zu. "
            "Use typed wrappers (e.g., matrix_multiply_i64_blocked) or the _ops APIs.\n",
            A->size_of_element);
        return NULL;
    }

    const size_t m = A->rows, p = A->cols, n = B->cols;
    const int use_blocked =
        (m >= 64 && p >= 64 && n >= 64) || (m * n >= 4096) || (p >= 64);

    if (use_blocked) {
        return matrix_multiply_ex_blocked(A, B, ops, /*BS=*/64);
    } else {
        return matrix_multiply_ex_naive(A, B, ops);
    }
}

/* ----- Typed wrappers: explicit, unambiguous API for the user ----- */
/* Each function validates that A/B actually carry the declared element size. */

Matrix* matrix_multiply_f64_naive   (const Matrix *A, const Matrix *B) { return matrix_multiply_ex_naive  (A, B, &MM_OPS_F64); }
Matrix* matrix_multiply_f64_blocked (const Matrix *A, const Matrix *B, size_t BS) { return matrix_multiply_ex_blocked(A, B, &MM_OPS_F64, BS); }

Matrix* matrix_multiply_i64_naive   (const Matrix *A, const Matrix *B) { return matrix_multiply_ex_naive  (A, B, &MM_OPS_I64); }
Matrix* matrix_multiply_i64_blocked (const Matrix *A, const Matrix *B, size_t BS) { return matrix_multiply_ex_blocked(A, B, &MM_OPS_I64, BS); }

Matrix* matrix_multiply_u32_naive   (const Matrix *A, const Matrix *B) { return matrix_multiply_ex_naive  (A, B, &MM_OPS_U32); }
Matrix* matrix_multiply_u32_blocked (const Matrix *A, const Matrix *B, size_t BS) { return matrix_multiply_ex_blocked(A, B, &MM_OPS_U32, BS); }

Matrix* matrix_multiply_size_naive  (const Matrix *A, const Matrix *B) { return matrix_multiply_ex_naive  (A, B, &MM_OPS_ST); }
Matrix* matrix_multiply_size_blocked(const Matrix *A, const Matrix *B, size_t BS) { return matrix_multiply_ex_blocked(A, B, &MM_OPS_ST, BS); }

Matrix* matrix_multiply_ld_naive    (const Matrix *A, const Matrix *B) { return matrix_multiply_ex_naive  (A, B, &MM_OPS_LD); }
Matrix* matrix_multiply_ld_blocked  (const Matrix *A, const Matrix *B, size_t BS) { return matrix_multiply_ex_blocked(A, B, &MM_OPS_LD, BS); }

/* =========================================================================
 * ADVANCED API: *_ops variants (custom arithmetic via an opaque ops table)
 *
 * What these do:
 *   These entry points let the caller supply a custom operator table that
 *   defines how to:
 *     - write the additive identity for one element (set_zero), and
 *     - perform a fused multiply-add on one element (muladd):
 *         *acc += (*a) * (*b)
 *   This enables non-standard arithmetic (e.g., saturating integers, wider
 *   accumulators, fixed-point, modular math, user-defined numeric types)
 *   without modifying the core GEMM loops.
 *
 * Expectations / contract:
 *   - 'ops_opaque' must point to a struct layout-compatible with the internal
 *     MatrixOps:
 *       { size_t elem_size; set_zero; muladd; }.
 *   - A and B must use row-major contiguous storage.
 *   - Dimensions must conform: A->cols == B->rows.
 *   - Element sizes must match the ops table:
 *       A->size_of_element == B->size_of_element == ops->elem_size.
 *
 * Which one to call:
 *   - matrix_multiply_naive_ops   : simple triple-loop kernel (good for small sizes).
 *   - matrix_multiply_blocked_ops : tiled kernel with block size BS (elements),
 *                                   improving cache reuse on medium/large sizes.
 *
 * Note:
 *   Advanced feature. For common primitive types prefer the explicit
 *   typed wrappers (matrix_multiply_f64_*, _i64_*, _u32_*, _size_*, _ld_*) or
 *   the size-based facade matrix_multiply(A,B) when ambiguity is not a concern.
 * ========================================================================= */
Matrix* matrix_multiply_naive_ops   (Matrix *A, Matrix *B, const void *ops_opaque) {
    const MatrixOps *ops = (const MatrixOps*)ops_opaque;
    return matrix_multiply_ex_naive(A, B, ops);
}
Matrix* matrix_multiply_blocked_ops (Matrix *A, Matrix *B, const void *ops_opaque, size_t BS) {
    const MatrixOps *ops = (const MatrixOps*)ops_opaque;
    return matrix_multiply_ex_blocked(A, B, ops, BS);
}
