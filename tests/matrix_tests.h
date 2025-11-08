#ifndef MATRIX_TESTS_H
#define MATRIX_TESTS_H

#include "../matrix/matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- stderr silencer (same pattern used in BST tests) ---- */
#if defined(_WIN32)
  #ifndef NOMINMAX
  #define NOMINMAX
  #endif
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>   /* LARGE_INTEGER, QueryPerformance* */

  #include <io.h>
  #include <fcntl.h>
  #define MAT_DEV_NULL  "NUL"
  #define mat_dup       _dup
  #define mat_dup2      _dup2
  #define mat_open      _open
  #define mat_close     _close
  #define mat_fileno    _fileno
  #define MAT_O_WRONLY  _O_WRONLY
#else
  #include <unistd.h>
  #include <fcntl.h>
  #include <time.h>      /* clock_gettime, struct timespec */
  #define MAT_DEV_NULL  "/dev/null"
  #define mat_dup       dup
  #define mat_dup2      dup2
  #define mat_open      open
  #define mat_close     close
  #define mat_fileno    fileno
  #define MAT_O_WRONLY  O_WRONLY
#endif

/* Public helpers to silence/restore stderr during noisy tests */
void mat_silence_stderr_begin(void);
void mat_silence_stderr_end(void);

/* Entry point for Matrix tests. Prints a summary and does not exit. */
void run_all_matrix_tests(void);

#endif /* MATRIX_TESTS_H */
