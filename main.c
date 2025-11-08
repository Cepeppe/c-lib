#include <stdio.h>
#include "tests/linked_list_tests.h"
#include "tests/murmur3_tests.h"
#include "tests/hashing_utils_tests.h"
#include "tests/linked_list_tests.h"
#include "tests/hashmap_tests.h"
#include "tests/bst_tests.h"
#include "tests/matrix_tests.h"

int run_tests(){
    run_all_matrix_tests();
    run_all_bst_tests();
    run_all_linked_list_tests();
    run_all_hashmap_tests();
    test_murmur3();
    return 0;
}

int main(void) {
    return run_tests();
}