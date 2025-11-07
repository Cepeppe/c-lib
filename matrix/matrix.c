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