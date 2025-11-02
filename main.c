#include <stdio.h>
#include "linked_list/linked_list_tests.h"
#include "hashing/murmur3_tests.h"

int main(void) {
    run_all_linked_list_tests();
    test_murmur3();
    return 0;
}
