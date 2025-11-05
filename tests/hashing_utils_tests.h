#ifndef HASHING_UTILS_TESTS_H
#define HASHING_UTILS_TESTS_H
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "../hashing/hashing_utils.h"

/*
 * hashing_utils_tests.h
 *
 * Declares a test runner for hashing_utils.
 *
 * also dumps some diagnostic info to stdout
 * (like hex bytes) so we can visually inspect correctness.
 */

/**
 * Runs all tests for hashing_utils (currently hu_raw_bytes_to_char_buffer()).
 * Returns 0 if all tests pass, non-zero if any test fails.
 */
int run_hashing_utils_tests(void);

#endif /* HASHING_UTILS_TESTS_H */
