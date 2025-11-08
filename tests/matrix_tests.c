#include "matrix_tests.h"
#include <math.h>
#include <stdint.h>

/* ============================ counters & macro ============================ */

static int mat_passed = 0;
static int mat_failed = 0;
static const char* g_current_test = "(unset)";

/* Stampa un messaggio di failure con nome test, file/linea e descrizione */
#define MAT_EXPECT(cond, msg)                                                       \
    do {                                                                            \
        if ((cond)) {                                                               \
            mat_passed++;                                                           \
        } else {                                                                    \
            mat_failed++;                                                           \
            fprintf(stderr, "[MATRIX FAIL] test=\"%s\" @ %s:%d: %s\n",              \
                    g_current_test, __FILE__, __LINE__, (msg));                     \
        }                                                                           \
    } while (0)

/* Comodo per mostrare quale test sta partendo */
#define START_TEST(name_literal)                                                    \
    do {                                                                            \
        g_current_test = (name_literal);                                            \
        printf("[TEST] %s\n", g_current_test);                                      \
    } while (0)

/* ============================ (disattivato) stderr silencer ============================ */
/* Lasciamo le funzioni come stub, ma NON le chiamiamo nel runner */
static int mat_saved_stderr_fd = -1;
void mat_silence_stderr_begin(void) {
    /* disabilitato di proposito per far vedere gli errori */
    (void)mat_saved_stderr_fd;
}
void mat_silence_stderr_end(void) {
    /* disabilitato di proposito per far vedere gli errori */
}

/* ============================ tiny helpers ============================ */

static int nearly_equal(double a, double b, double tol) {
    double diff = fabs(a - b);
    double scale = fmax(1.0, fmax(fabs(a), fabs(b)));
    return diff <= tol * scale;
}

/* Check row pointers layout */
static int rows_are_strided_correctly(const Matrix* M) {
    if (!M || !M->row) return 0;
    for (size_t i = 0; i < M->rows; ++i) {
        const char* expect = (const char*)M->data + i * M->cols * M->size_of_element;
        if (M->row[i] != (void*)expect) return 0;
    }
    return 1;
}

/* Debug print for small matrices (double) */
static void print_matrix_f64(const Matrix* M) {
    if (!M) { printf("(null)\n"); return; }
    const double* d = (const double*)M->data;
    printf("Matrix %zux%zu (double):\n", M->rows, M->cols);
    for (size_t i=0;i<M->rows;++i) {
        printf("  [");
        for (size_t j=0;j<M->cols;++j) {
            printf("%s%.10g", (j? ", ":""), d[i*M->cols+j]);
        }
        printf("]\n");
    }
}
static void print_matrix_f64_ref(const double* ref, size_t r, size_t c) {
    printf("Ref %zux%zu (double):\n", r, c);
    for (size_t i=0;i<r;++i) {
        printf("  [");
        for (size_t j=0;j<c;++j) {
            printf("%s%.10g", (j? ", ":""), ref[i*c+j]);
        }
        printf("]\n");
    }
}

/* Debug print for small matrices (integers) */
static void print_matrix_i64(const Matrix* M) {
    if (!M) { printf("(null)\n"); return; }
    const int64_t* d = (const int64_t*)M->data;
    printf("Matrix %zux%zu (i64):\n", M->rows, M->cols);
    for (size_t i=0;i<M->rows;++i) {
        printf("  [");
        for (size_t j=0;j<M->cols;++j) {
            printf("%s%lld", (j? ", ":""), (long long)d[i*M->cols+j]);
        }
        printf("]\n");
    }
}
static void print_matrix_u32(const Matrix* M) {
    if (!M) { printf("(null)\n"); return; }
    const uint32_t* d = (const uint32_t*)M->data;
    printf("Matrix %zux%zu (u32):\n", M->rows, M->cols);
    for (size_t i=0;i<M->rows;++i) {
        printf("  [");
        for (size_t j=0;j<M->cols;++j) {
            printf("%s%u", (j? ", ":""), d[i*M->cols+j]);
        }
        printf("]\n");
    }
}
static void print_matrix_size(const Matrix* M) {
    if (!M) { printf("(null)\n"); return; }
    const size_t* d = (const size_t*)M->data;
    printf("Matrix %zux%zu (size_t):\n", M->rows, M->cols);
    for (size_t i=0;i<M->rows;++i) {
        printf("  [");
        for (size_t j=0;j<M->cols;++j) {
            printf("%s%zu", (j? ", ":""), d[i*M->cols+j]);
        }
        printf("]\n");
    }
}

/* Builders from flat arrays (row-major) */
static Matrix* make_f64(size_t r, size_t c, const double* vals) {
    Matrix* M = new_matrix(r, c, sizeof(double));
    if (!M) return NULL;
    memcpy(M->data, vals, r*c*sizeof(double));
    return M;
}
static Matrix* make_ld(size_t r, size_t c, const long double* vals) {
    Matrix* M = new_matrix(r, c, sizeof(long double));
    if (!M) return NULL;
    memcpy(M->data, vals, r*c*sizeof(long double));
    return M;
}
static Matrix* make_i64(size_t r, size_t c, const int64_t* vals) {
    Matrix* M = new_matrix(r, c, sizeof(int64_t));
    if (!M) return NULL;
    memcpy(M->data, vals, r*c*sizeof(int64_t));
    return M;
}
static Matrix* make_u32(size_t r, size_t c, const uint32_t* vals) {
    Matrix* M = new_matrix(r, c, sizeof(uint32_t));
    if (!M) return NULL;
    memcpy(M->data, vals, r*c*sizeof(uint32_t));
    return M;
}
static Matrix* make_size(size_t r, size_t c, const size_t* vals) {
    Matrix* M = new_matrix(r, c, sizeof(size_t));
    if (!M) return NULL;
    memcpy(M->data, vals, r*c*sizeof(size_t));
    return M;
}
static Matrix* make_u8(size_t r, size_t c, const uint8_t* vals) {
    Matrix* M = new_matrix(r, c, sizeof(uint8_t));
    if (!M) return NULL;
    memcpy(M->data, vals, r*c*sizeof(uint8_t));
    return M;
}

/* Equality with debug on failure */
static int expect_equal_f64(const Matrix* M, const double* ref, size_t r, size_t c, double tol, const char* msg) {
    int ok = (M && M->rows==r && M->cols==c && M->size_of_element==sizeof(double));
    if (!ok) {
        MAT_EXPECT(0, msg);
        printf("  reason: shape/elem_size mismatch or NULL. got rows=%zu cols=%zu elem=%zu\n",
               M?M->rows:0, M?M->cols:0, M?M->size_of_element:0);
        return 0;
    }
    const double* d = (const double*)M->data;
    for (size_t i=0;i<r*c;++i) {
        if (!nearly_equal(d[i], ref[i], tol)) {
            MAT_EXPECT(0, msg);
            printf("  first mismatch at flat index %zu (row=%zu col=%zu): got=%.17g exp=%.17g tol=%g\n",
                   i, i/c, i%c, d[i], ref[i], tol);
            if (r<=4 && c<=4) { print_matrix_f64(M); print_matrix_f64_ref(ref,r,c); }
            return 0;
        }
    }
    MAT_EXPECT(1, msg);
    return 1;
}
static int expect_equal_i64(const Matrix* M, const int64_t* ref, size_t r, size_t c, const char* msg) {
    int ok = (M && M->rows==r && M->cols==c && M->size_of_element==sizeof(int64_t));
    if (!ok) {
        MAT_EXPECT(0, msg);
        printf("  reason: shape/elem_size mismatch or NULL. got rows=%zu cols=%zu elem=%zu\n",
               M?M->rows:0, M?M->cols:0, M?M->size_of_element:0);
        return 0;
    }
    const int64_t* d = (const int64_t*)M->data;
    for (size_t i=0;i<r*c;++i) {
        if (d[i] != ref[i]) {
            MAT_EXPECT(0, msg);
            printf("  first mismatch at flat index %zu (row=%zu col=%zu): got=%lld exp=%lld\n",
                   i, i/c, i%c, (long long)d[i], (long long)ref[i]);
            if (r<=4 && c<=4) print_matrix_i64(M);
            return 0;
        }
    }
    MAT_EXPECT(1, msg);
    return 1;
}
static int expect_equal_u32(const Matrix* M, const uint32_t* ref, size_t r, size_t c, const char* msg) {
    int ok = (M && M->rows==r && M->cols==c && M->size_of_element==sizeof(uint32_t));
    if (!ok) {
        MAT_EXPECT(0, msg);
        printf("  reason: shape/elem_size mismatch or NULL. got rows=%zu cols=%zu elem=%zu\n",
               M?M->rows:0, M?M->cols:0, M?M->size_of_element:0);
        return 0;
    }
    const uint32_t* d = (const uint32_t*)M->data;
    for (size_t i=0;i<r*c;++i) {
        if (d[i] != ref[i]) {
            MAT_EXPECT(0, msg);
            printf("  first mismatch at flat index %zu (row=%zu col=%zu): got=%u exp=%u\n",
                   i, i/c, i%c, d[i], ref[i]);
            if (r<=4 && c<=4) print_matrix_u32(M);
            return 0;
        }
    }
    MAT_EXPECT(1, msg);
    return 1;
}
static int expect_equal_size(const Matrix* M, const size_t* ref, size_t r, size_t c, const char* msg) {
    int ok = (M && M->rows==r && M->cols==c && M->size_of_element==sizeof(size_t));
    if (!ok) {
        MAT_EXPECT(0, msg);
        printf("  reason: shape/elem_size mismatch or NULL. got rows=%zu cols=%zu elem=%zu\n",
               M?M->rows:0, M?M->cols:0, M?M->size_of_element:0);
        return 0;
    }
    const size_t* d = (const size_t*)M->data;
    for (size_t i=0;i<r*c;++i) {
        if (d[i] != ref[i]) {
            MAT_EXPECT(0, msg);
            printf("  first mismatch at flat index %zu (row=%zu col=%zu): got=%zu exp=%zu\n",
                   i, i/c, i%c, d[i], ref[i]);
            if (r<=4 && c<=4) print_matrix_size(M);
            return 0;
        }
    }
    MAT_EXPECT(1, msg);
    return 1;
}

/* ============================ custom ops (file-scope) ============================ */
/* uint32_t arithmetic mod 100 */
static void u32_mod100_zero(void *dst) { *(uint32_t*)dst = 0u; }
static void u32_mod100_muladd(void *acc, const void *a, const void *b) {
    uint32_t t = (*(const uint32_t*)a) * (*(const uint32_t*)b);
    *(uint32_t*)acc = ( *(uint32_t*)acc + t ) % 100u;
}
static const MatrixOps U32_MOD100_OPS = {
    sizeof(uint32_t),
    u32_mod100_zero,
    u32_mod100_muladd
};

/* =============================== test cases =============================== */

static void test_new_matrix_and_rows(void) {
    START_TEST("new_matrix & row[] layout");
    Matrix* M = new_matrix(3, 4, sizeof(double));
    MAT_EXPECT(M != NULL, "new_matrix must succeed for 3x4 double");
    MAT_EXPECT(M && rows_are_strided_correctly(M), "row[] pointers must stride by cols*elem_size");
    destroy_matrix(M);
}

static void test_new_matrix_overflow_guard(void) {
    START_TEST("new_matrix overflow guard");
    Matrix* M = new_matrix(SIZE_MAX, SIZE_MAX, sizeof(double));
    MAT_EXPECT(M == NULL, "new_matrix must reject overflow sizes");
}

static void test_destroy_null_is_noop(void) {
    START_TEST("destroy_matrix(NULL) is a no-op");
    destroy_matrix(NULL); /* should not crash */
    MAT_EXPECT(1, "destroy_matrix(NULL) did not crash");
}

static void test_fill_scalar_paths(void) {
    START_TEST("matrix_fill_scalar (u8 path & generic path)");

    /* 1-byte memset path */
    Matrix* A = new_matrix(2, 3, sizeof(uint8_t));
    uint8_t v8 = 0xAB;
    MAT_EXPECT(matrix_fill_scalar(A, &v8) == 1, "fill scalar u8");
    uint8_t ref8[6]; for (int i=0;i<6;++i) ref8[i]=0xAB;
    int ok8 = (A && A->rows==2 && A->cols==3 && A->size_of_element==1 &&
               memcmp(A->data, ref8, 6)==0);
    MAT_EXPECT(ok8, "fill u8 must set all bytes");
    destroy_matrix(A);

    /* generic memcpy-per-element path */
    Matrix* B = new_matrix(2, 2, sizeof(double));
    double v = 3.1415;
    MAT_EXPECT(matrix_fill_scalar(B, &v) == 1, "fill scalar double");
    double refB[4] = {v,v,v,v};
    expect_equal_f64(B, refB, 2, 2, 1e-12, "fill double must set all cells");
    destroy_matrix(B);
}

static void test_build_all_k_and_multiply_for_scalar(void) {
    START_TEST("build_all_k_matrix & matrix_multiply_for_scalar");
    double k = 7.0;
    Matrix* M1 = build_all_k_matrix(2, 3, &k, sizeof(double));
    MAT_EXPECT(M1 != NULL, "build_all_k_matrix returns matrix");

    Matrix* M2 = matrix_multiply_for_scalar(2, 3, &k, sizeof(double));
    MAT_EXPECT(M2 != NULL, "matrix_multiply_for_scalar returns matrix");

    double ref[6] = {7,7,7,7,7,7};
    expect_equal_f64(M1, ref, 2, 3, 1e-12, "build_all_k sets all to k");
    expect_equal_f64(M2, ref, 2, 3, 1e-12, "multiply_for_scalar (alias) sets all to k");

    destroy_matrix(M1);
    destroy_matrix(M2);
}

/* ----- Multiplication: floating examples from the provided images ----- */
static void test_mul_f64_examples_decimals(void) {
    START_TEST("matrix_multiply_* (double) - examples with decimals");

    /* Example 1 (2x2 * 2x2) */
    const double A1[4] = {1.2, -0.5, 3.1, 2.4}; /* row-major */
    const double B1[4] = {0.7, -1.3, 4.2, 0.6};
    const double C1[4] = {-1.26, -1.86, 12.25, -2.59};
    Matrix* A = make_f64(2,2,A1);
    Matrix* B = make_f64(2,2,B1);

    Matrix* Cn = matrix_multiply_f64_naive(A,B);
    Matrix* Cb = matrix_multiply_f64_blocked(A,B,64);
    Matrix* Cf = matrix_multiply(A,B); /* facade should be fine for doubles */

    expect_equal_f64(Cn, C1, 2,2, 1e-6,  "f64 naive matches example 1");
    expect_equal_f64(Cb, C1, 2,2, 1e-6,  "f64 blocked matches example 1");
    expect_equal_f64(Cf, C1, 2,2, 1e-6,  "facade matches example 1");

    destroy_matrix(Cn); destroy_matrix(Cb); destroy_matrix(Cf);
    destroy_matrix(A); destroy_matrix(B);

    /* Example 2 (2x3 * 3x2) */
    const double A2[6] = {0.5,1.2,-0.3,  2.0,-1.5,0.4};
    const double B2[6] = {1.1,-0.7, 0.8,2.5, -1.2,0.9};
    const double C2[4] = {1.87,2.38, 0.52,-4.79};
    A = make_f64(2,3,A2);
    B = make_f64(3,2,B2);
    Cn = matrix_multiply_f64_naive(A,B);
    Cb = matrix_multiply_f64_blocked(A,B,64);
    expect_equal_f64(Cn, C2, 2,2, 1e-6, "f64 naive matches example 2");
    expect_equal_f64(Cb, C2, 2,2, 1e-6, "f64 blocked matches example 2");
    destroy_matrix(Cn); destroy_matrix(Cb); destroy_matrix(A); destroy_matrix(B);

    /* Example 3 (3x3 * 3x3) */
    const double A3[9] = {0.2,-1.1,0.5,  1.3,0.4,-0.8,  2.1,-0.6,0.3};
    const double B3[9] = {1.0,0.4,0.9,   0.2,1.5,-1.1,  0.7,0.8,0.3};
    /* Correct expected result for the inputs above */
    const double C3[9] = {
        0.33, -1.17, 1.54,
        0.82,  0.48, 0.49,
        2.19,  0.18, 2.64
    };
    A = make_f64(3,3,A3);
    B = make_f64(3,3,B3);
    Cn = matrix_multiply_f64_naive(A,B);
    Cb = matrix_multiply_f64_blocked(A,B,64);
    expect_equal_f64(Cn, C3, 3,3, 1e-6, "f64 naive matches example 3");
    expect_equal_f64(Cb, C3, 3,3, 1e-6, "f64 blocked matches example 3");
    destroy_matrix(Cn); destroy_matrix(Cb); destroy_matrix(A); destroy_matrix(B);
}

/* ----- Multiplication: integer/simple cases for i64, u32, size_t ----- */
static void test_mul_integers_simple_exact(void) {
    START_TEST("matrix_multiply_* (integers) — simple exact");

    /* A(2x3) * B(3x2) with known result [[58,64],[139,154]] */
    const int64_t  Ai64[6] = {1,2,3, 4,5,6};
    const int64_t  Bi64[6] = {7,8, 9,10, 11,12};
    const int64_t  Ci64[4] = {58,64, 139,154};
    Matrix *A64 = make_i64(2,3,Ai64), *B64 = make_i64(3,2,Bi64);
    Matrix *C64n = matrix_multiply_i64_naive(A64,B64);
    Matrix *C64b = matrix_multiply_i64_blocked(A64,B64,64);
    expect_equal_i64(C64n, Ci64, 2,2, "i64 naive exact");
    expect_equal_i64(C64b, Ci64, 2,2, "i64 blocked exact");
    destroy_matrix(C64n); destroy_matrix(C64b); destroy_matrix(A64); destroy_matrix(B64);

    const uint32_t Au32[6] = {1,2,3, 4,5,6};
    const uint32_t Bu32[6] = {7,8, 9,10, 11,12};
    const uint32_t Cu32[4] = {58,64, 139,154};
    Matrix *A32 = make_u32(2,3,Au32), *B32 = make_u32(3,2,Bu32);
    Matrix *C32n = matrix_multiply_u32_naive(A32,B32);
    Matrix *C32b = matrix_multiply_u32_blocked(A32,B32,64);
    expect_equal_u32(C32n, Cu32, 2,2, "u32 naive exact");
    expect_equal_u32(C32b, Cu32, 2,2, "u32 blocked exact");
    destroy_matrix(C32n); destroy_matrix(C32b); destroy_matrix(A32); destroy_matrix(B32);

    const size_t Asz[6] = {1,2,3, 4,5,6};
    const size_t Bsz[6] = {7,8, 9,10, 11,12};
    const size_t Csz[4] = {58,64, 139,154};
    Matrix *AS = make_size(2,3,Asz), *BS = make_size(3,2,Bsz);
    Matrix *CSn = matrix_multiply_size_naive(AS,BS);
    Matrix *CSb = matrix_multiply_size_blocked(AS,BS,64);
    expect_equal_size(CSn, Csz, 2,2, "size_t naive exact");
    expect_equal_size(CSb, Csz, 2,2, "size_t blocked exact");
    destroy_matrix(CSn); destroy_matrix(CSb); destroy_matrix(AS); destroy_matrix(BS);
}

/* ----- Multiplication: long double (sanity) ----- */
static void test_mul_long_double(void) {
    START_TEST("matrix_multiply_* (long double)");

    const long double A[4] = {1.0L, 2.0L, 3.0L, 4.0L};   /* 2x2 */
    const long double B[4] = {5.0L, 6.0L, 7.0L, 8.0L};
    const long double Cref[4] = {19.0L, 22.0L, 43.0L, 50.0L};
    Matrix* M1 = make_ld(2,2,A);
    Matrix* M2 = make_ld(2,2,B);
    Matrix* Cn = matrix_multiply_ld_naive(M1,M2);
    Matrix* Cb = matrix_multiply_ld_blocked(M1,M2,64);

    /* Usiamo confronto manuale con stampa in caso di differenze */
    int ok = (Cn && Cn->rows==2 && Cn->cols==2 && Cn->size_of_element==sizeof(long double));
    MAT_EXPECT(ok, "long double naive shape");
    if (ok) {
        const long double* d = (const long double*)Cn->data;
        for (int i=0;i<4;++i) {
            if (fabsl(d[i] - Cref[i]) > 1e-12L * fmaxl(1.0L, fmaxl(fabsl(d[i]), fabsl(Cref[i])))) {
                MAT_EXPECT(0, "long double naive values");
                printf("  mismatch idx=%d got=%.20Lg exp=%.20Lg\n", i, d[i], Cref[i]);
                break;
            }
        }
    }
    ok = (Cb && Cb->rows==2 && Cb->cols==2 && Cb->size_of_element==sizeof(long double));
    MAT_EXPECT(ok, "long double blocked shape");
    if (ok) {
        const long double* d = (const long double*)Cb->data;
        for (int i=0;i<4;++i) {
            if (fabsl(d[i] - Cref[i]) > 1e-12L * fmaxl(1.0L, fmaxl(fabsl(d[i]), fabsl(Cref[i])))) {
                MAT_EXPECT(0, "long double blocked values");
                printf("  mismatch idx=%d got=%.20Lg exp=%.20Lg\n", i, d[i], Cref[i]);
                break;
            }
        }
    }
    destroy_matrix(Cn); destroy_matrix(Cb); destroy_matrix(M1); destroy_matrix(M2);
}

/* ----- Facade: OK on doubles, mismatch rejection on typed API ----- */
static void test_facade_and_mismatch_checks(void) {
    START_TEST("facade (double OK) & typed mismatch rejection");

    /* Facade is fine for doubles */
    const double A[6] = {1,2,3, 4,5,6};            /* 2x3 */
    const double B[6] = {7,8, 9,10, 11,12};        /* 3x2 */
    const double Cref[4] = {58,64, 139,154};
    Matrix *MdA = make_f64(2,3,A), *MdB = make_f64(3,2,B);
    Matrix *Cf = matrix_multiply(MdA, MdB);
    expect_equal_f64(Cf, Cref, 2,2, 1e-12, "facade works for doubles");
    destroy_matrix(Cf); destroy_matrix(MdA); destroy_matrix(MdB);

    /* Typed API must reject size mismatch (A: double, B: float) */
    Matrix *A64 = new_matrix(2,2,sizeof(double));
    Matrix *B32 = new_matrix(2,2,sizeof(float));
    double one = 1.0; float fone = 1.0f;
    matrix_fill_scalar(A64, &one);
    matrix_fill_scalar(B32, &fone);

    Matrix *bad = matrix_multiply_f64_naive(A64, B32);   /* should fail preconditions and return NULL */
    MAT_EXPECT(bad == NULL, "typed f64 must return NULL on elem-size mismatch");

    destroy_matrix(A64); destroy_matrix(B32);
}

/* ----- Naive vs Blocked parity on small case (double) ----- */
static void test_naive_blocked_parity_small(void) {
    START_TEST("naive vs blocked parity (double)");

    const double A[9] = {1,2,3, 4,5,6, 7,8,9};
    const double B[9] = {9,8,7, 6,5,4, 3,2,1};
    Matrix *M1 = make_f64(3,3,A), *M2 = make_f64(3,3,B);
    Matrix *Cn = matrix_multiply_f64_naive(M1,M2);
    Matrix *Cb = matrix_multiply_f64_blocked(M1,M2,32);

    /* Confronto con tolleranza stretta ma non zero (loop order diverso) */
    int ok = (Cn && Cb && Cn->rows==Cb->rows && Cn->cols==Cb->cols);
    MAT_EXPECT(ok, "parity shapes");
    if (ok) {
        const double *dn = (const double*)Cn->data;
        const double *db = (const double*)Cb->data;
        for (size_t i=0;i<9;++i) {
            if (!nearly_equal(dn[i], db[i], 1e-12)) {
                MAT_EXPECT(0, "naive vs blocked: values differ");
                printf("  mismatch idx=%zu (r=%zu,c=%zu): naive=%.17g blocked=%.17g\n",
                       i, i/3, i%3, dn[i], db[i]);
                if (M1 && M2) { /* opzionale stampa input */ }
                break;
            }
        }
    }
    destroy_matrix(Cn); destroy_matrix(Cb); destroy_matrix(M1); destroy_matrix(M2);
}

/* ----- Advanced _ops: uint32_t modulo-100 arithmetic demo ----- */
static void test_ops_mod100_u32(void) {
    START_TEST("_ops: uint32 mod 100 arithmetic");

    const uint32_t A[6] = {15,22,37, 41,5,9};     /* 2x3 */
    const uint32_t B[6] = {3,7, 11,13, 17,19};    /* 3x2 */
    /* Plain product:
         Row0: [15*3+22*11+37*17, 15*7+22*13+37*19] = [  45+242+629, 105+286+703] = [916,1094]
         Row1: [41*3+5*11+9*17,    41*7+5*13+9*19]  = [ 123+55+153,  287+65+171 ] = [331,523]
       Mod 100 → [[16, 94],[31, 23]]
    */
    const uint32_t Cref[4] = {16,94, 31,23};

    Matrix *M1 = make_u32(2,3,A), *M2 = make_u32(3,2,B);
    Matrix *C  = matrix_multiply_blocked_ops(M1, M2, &U32_MOD100_OPS, 32);

    expect_equal_u32(C, Cref, 2,2, "_ops modulo-100 arithmetic works");

    destroy_matrix(C); destroy_matrix(M1); destroy_matrix(M2);
}

/* ================================ entry point ================================ */

void run_all_matrix_tests(void) {
    mat_passed = 0;
    mat_failed = 0;
    printf("[TEST] testing matrix module...\n");

    /* NOT silencing stderr anymore to show detailed failures */
    /* mat_silence_stderr_begin(); */

    test_new_matrix_and_rows();
    test_new_matrix_overflow_guard();
    test_destroy_null_is_noop();
    test_fill_scalar_paths();
    test_build_all_k_and_multiply_for_scalar();
    test_mul_f64_examples_decimals();
    test_mul_integers_simple_exact();
    test_mul_long_double();
    test_facade_and_mismatch_checks();
    test_naive_blocked_parity_small();
    test_ops_mod100_u32();

    /* mat_silence_stderr_end(); */

    if (mat_failed == 0) {
        printf("[TEST OK]  matrix: passed=%d failed=%d\n", mat_passed, mat_failed);
    } else {
        printf("[TEST FAIL] matrix: passed=%d failed=%d\n", mat_passed, mat_failed);
    }
}
