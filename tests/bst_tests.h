#ifndef BST_TESTS_H
#define BST_TESTS_H

#include "../bst/binary_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- stderr silencer platform shims (header scope) --- */
#include <stdio.h>

#if defined(_WIN32)
  #include <io.h>
  #include <fcntl.h>
  #define BST_DEV_NULL  "NUL"
  #define bst_dup       _dup
  #define bst_dup2      _dup2
  #define bst_open      _open
  #define bst_close     _close
  #define bst_fileno    _fileno
  #define BST_O_WRONLY  _O_WRONLY
#else
  #include <unistd.h>
  #include <fcntl.h>
  #define BST_DEV_NULL  "/dev/null"
  #define bst_dup       dup
  #define bst_dup2      dup2
  #define bst_open      open
  #define bst_close     close
  #define bst_fileno    fileno
  #define BST_O_WRONLY  O_WRONLY
#endif

/* Public helpers to silence/restore stderr during noisy tests */
void bst_silence_stderr_begin(void);
void bst_silence_stderr_end(void);

/* Entry point for BST tests. Prints a summary and does not exit. */
void run_all_bst_tests(void);

#endif /* BST_TESTS_H */
