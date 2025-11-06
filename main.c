#include <stdio.h>
#include "tests/linked_list_tests.h"
#include "tests/murmur3_tests.h"
#include "tests/hashing_utils_tests.h"
#include "tests/linked_list_tests.h"
#include "tests/hashmap_tests.h"
#include "tests/bst_tests.h"

int run_tests(){
    run_all_bst_tests();
    run_all_linked_list_tests();
    run_all_hashmap_tests();
    test_murmur3();
}

int main(void) {
    run_tests();
    return 0;
}