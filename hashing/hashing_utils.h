#ifndef HASHING_UTILS_H
#define HASHING_UTILS_H

/*
 * Helpers for hashing/generic keys.
 *
 * raw_bytes_to_char_buffer(data, len):
 *   - Copies `len` raw bytes from `data` into a newly malloc'ed buffer.
 *   - Appends '\0' at the end.
 *
 * Notes:
 *   - Buffer may contain '\0' in the middle, so don't rely on strlen/printf("%s")
 *     for full length. Track `len` separately.
 *   - Caller must free() the returned buffer.
 */

#include <stddef.h>  /* size_t */
#include <stdlib.h>  /* malloc */
#include <string.h>  /* memcpy */

char* raw_bytes_to_char_buffer(const void* data, size_t len);

#endif /* HASHING_UTILS_H */
