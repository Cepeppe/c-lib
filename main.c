#include <stdio.h>
#include "tests/linked_list_tests.h"
#include "tests/murmur3_tests.h"
#include "tests/hashing_utils_tests.h"
#include "tests/linked_list_tests.h"
#include "tests/hashmap_tests.h"

#define SELF_EXE_REL "main.exe"                  /* works if CWD == exe folder */
/* Or use absolute path (escape backslashes): */
// #define SELF_EXE_ABS "C:\\Users\\giuse\\Desktop\\PROGETTI\\c-lib\\build\\main.exe"
int run_tests(){
    /* Hard-coded argv for LinkedList fatal subtests */
    char* argv_ll[] = {
        /* change to SELF_EXE_ABS if you prefer the absolute path */
        (char*)SELF_EXE_REL,
        NULL
    };
    int argc_ll = 1;

    test_murmur3();
    int rc1 = run_linked_list_tests(argc_ll, argv_ll);
    int rc2 = run_hashmap_tests();
    

    return (rc1 || rc2) ? 1 : 0;
}

int main(void) {

    
}