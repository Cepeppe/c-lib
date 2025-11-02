#include <stdio.h>
#include "linked_list/linked_list_tests.h"
#include "hashing/murmur3_tests.h"
#include "hashing/hashing_utils_tests.h"

int main(void) {
    run_all_linked_list_tests();
    test_murmur3();
    run_hashing_utils_tests();
    return 0;
}
