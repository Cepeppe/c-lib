#ifndef STRING_H
#define STRING_H
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

typedef char* string;

size_t string_len(string str);
size_t string_len_including_terminator(string str);
string string_copy_new(const char* source);
string string_concat(const char* s1, const char* s2);
void string_trim(string s);
void string_split(const string s, const char* separators, size_t num_separators, string* splitted_tokens, size_t* num_tokens);

#endif