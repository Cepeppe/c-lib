#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "hashing_utils_tests.h"

static void debug_print_hex(const char* buf, size_t len) {
    printf("  hex bytes: ");
    for (size_t i = 0; i < len; i++) {
        printf("%02x ", (unsigned char)buf[i]);
    }
    printf("\n");
}

static bool test_case_string(void) {
    const char *key = "ciao";
    size_t key_len = strlen(key);

    char *dumped = raw_bytes_to_char_buffer(key, key_len);
    if (dumped == NULL) {
        return false;
    }

    bool ok = (memcmp(dumped, key, key_len) == 0);

    printf("  original (as %%s): \"%s\"\n", key);
    printf("  dumped   (as %%s): \"%s\"\n", dumped);
    debug_print_hex(dumped, key_len);

    free(dumped);
    return ok;
}

static bool test_case_int(void) {
    int value = 12345;
    size_t len = sizeof(value);

    char *dumped = raw_bytes_to_char_buffer(&value, len);
    if (dumped == NULL) {
        return false;
    }

    bool ok = (memcmp(dumped, &value, len) == 0);

    printf("  original int: %d\n", value);
    printf("  dumped as %%s (will likely cut at first \\0): \"%s\"\n", dumped);
    debug_print_hex(dumped, len);

    free(dumped);
    return ok;
}

static bool test_case_float(void) {
    float value = 3.14f;
    size_t len = sizeof(value);

    char *dumped = raw_bytes_to_char_buffer(&value, len);
    if (dumped == NULL) {
        return false;
    }

    bool ok = (memcmp(dumped, &value, len) == 0);

    printf("  original float: %f\n", value);
    printf("  dumped as %%s (may show weird chars / stop early): \"%s\"\n", dumped);
    debug_print_hex(dumped, len);

    free(dumped);
    return ok;
}

typedef struct Small {
    uint32_t id;
    uint8_t  flag;
} Small;

static bool test_case_struct(void) {
    Small s;
    s.id   = 42;
    s.flag = 1;

    size_t len = sizeof(s);

    char *dumped = raw_bytes_to_char_buffer(&s, len);
    if (dumped == NULL) {
        return false;
    }

    bool ok = (memcmp(dumped, &s, len) == 0);

    printf("  original struct Small { id=%u, flag=%u }\n", s.id, s.flag);
    printf("  dumped as %%s (likely stops at first \\0): \"%s\"\n", dumped);
    debug_print_hex(dumped, len);

    free(dumped);
    return ok;
}

int run_hashing_utils_tests(void) {
    int failed = 0;

    printf("[TEST] test_raw_bytes_to_char_buffer_string...\n");
    if (test_case_string()) {
        printf("[OK]   test_raw_bytes_to_char_buffer_string\n");
    } else {
        printf("[FAIL] test_raw_bytes_to_char_buffer_string\n");
        failed++;
    }
    printf("\n");

    printf("[TEST] test_raw_bytes_to_char_buffer_int...\n");
    if (test_case_int()) {
        printf("[OK]   test_raw_bytes_to_char_buffer_int\n");
    } else {
        printf("[FAIL] test_raw_bytes_to_char_buffer_int\n");
        failed++;
    }
    printf("\n");

    printf("[TEST] test_raw_bytes_to_char_buffer_float...\n");
    if (test_case_float()) {
        printf("[OK]   test_raw_bytes_to_char_buffer_float\n");
    } else {
        printf("[FAIL] test_raw_bytes_to_char_buffer_float\n");
        failed++;
    }
    printf("\n");

    printf("[TEST] test_raw_bytes_to_char_buffer_struct...\n");
    if (test_case_struct()) {
        printf("[OK]   test_raw_bytes_to_char_buffer_struct\n");
    } else {
        printf("[FAIL] test_raw_bytes_to_char_buffer_struct\n");
        failed++;
    }
    printf("\n");

    if (failed == 0) {
        printf("    hashing_utils) All hashing_utils tests PASSED.\n");
    } else {
        printf("    hashing_utils) %d hashing_utils test(s) FAILED.\n", failed);
    }

    return (failed == 0) ? 0 : 1;
}
