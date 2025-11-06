#include "hashing_utils.h"

/**
 * raw_bytes_to_char_buffer
 *
 * Takes an arbitrary memory region [data, data+len)
 * and returns a newly allocated char* containing exactly those bytes,
 * followed by a '\0' terminator for convenience 
 *      Note: alredy terminated strings (e.g. "abc\0") are obviously allowed but 
 *          be careful to uniform, in your code behaviour might 
 *          change if new string becomes "abc\0\0" and you tought 
 *          operations are performed on just "abc\0"
 *       
 * (ALWAYS MIND IF YOUR USAGE INCLUDES \0 IN INPUT STRINGS OR NOT)
 *
 * NOTES:
 * - The returned buffer is length (len + 1).
 * - We copy all bytes 1:1 into the first len positions.
 * - We then force buffer[len] = '\0'.
 *
 * IMPORTANT:
 * - The content may include '\0' in the middle. So treating this as a
 *   "regular C string" (printf("%s"), strlen(), etc.) will likely NOT
 *   reflect the full length.
 * - You MUST remember the original length len externally if you want
 *   to use the entire buffer.
 *
 * Caller must free() the returned pointer.
 */

char* raw_bytes_to_char_buffer(const void* data, size_t len) {
    if (data == NULL || len == 0) {
        char* empty = (char*)malloc(1);
        if (empty != NULL) {
            empty[0] = '\0';
        }
        return empty;
    }

    char* out = (char*)malloc(len + 1);
    if (out == NULL) {
        return NULL;
    }

    memcpy(out, data, len);
    out[len] = '\0'; /* safety terminator */

    return out;
}



