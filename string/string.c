#include "string.h"


size_t string_len(string str) {
    if (str == NULL) {
        fprintf(stderr, "Returning len 0 for null string");
        return 0;
    }

    size_t counter = 0;
    while (*str++) {
        counter++;
    }
    return counter;
}

size_t string_len_including_terminator(string str) {
    if (str == NULL) {
        fprintf(stderr, "Returning len (including terminators) 0 for null string");
        return 0;
    }
    return string_len(str) + 1;
}

/* Returns new string (allocated inside the function) which is a copy of s1 */
string string_copy_new(const char* source){

    if(source == NULL) {
        fprintf(stderr, "You are trying to copy a null string");
        return NULL;
    }

    size_t len = string_len_including_terminator(source);

    string new_string_result = (string) malloc(len * sizeof(char));
    if (new_string_result == NULL) {
        fprintf(stderr, "Failed malloc while trying to copy string");
        return NULL;
    }

    const char* temp_source = source;
    string buffer_string = new_string_result;

    while((*buffer_string++=*temp_source++) != '\0');
    return new_string_result;
}

/* Returns new string (allocated inside the function) which is a concatenation of s1 and s2 */
string string_concat(const char* s1, const char* s2){
        
    if(s1==NULL || s2 == NULL){
        fprintf(stderr, "You are trying concat one or more null strings");
        return NULL;
    }

    size_t len1=string_len(s1);
    size_t len2=string_len_including_terminator(s2);

    string new_string_result = (string) malloc((len1+len2)*sizeof(char));
    if (new_string_result == NULL) {
        fprintf(stderr, "Failed malloc while trying to concat strings");
        return NULL;
    }

    size_t index_s1=0, index_s2=0;
    size_t i=0;

    while(index_s1<len1){
        new_string_result[i]=s1[index_s1];
        i++; 
        index_s1++;
    }

    while(index_s2<len2){
        new_string_result[i]=s2[index_s2];
        i++; 
        index_s2++;
    }

    return new_string_result;
}

/* Trims a string (removes all initial/final trailing whitespaces and tabs) */
void string_trim(string s) {
    if (s == NULL) {
        fprintf(stderr, "You are trying to trim a null string\n");
        return;
    }

    // length without '\0'
    size_t len_no_term = string_len(s);

    if (len_no_term == 0) {
        return;
    }

    // find first char not ' ' && not '\t'
    size_t start = 0;
    while (start < len_no_term && (s[start] == ' ' || s[start] == '\t')) {
        start++;
    }

    // if string is all \t and spaces, turn it into empty string: ""
    if (start == len_no_term) {
        s[0] = '\0';
        return;
    }

    // find last char not ' ' && not '\t'
    size_t end = len_no_term - 1;
    while (end > start && (s[end] == ' ' || s[end] == '\t')) {
        end--;
    }

    // left shift: copy s[start..end] at the beginning and adds \0 after end-start+1 chars
    size_t write_index = 0;
    for (size_t read_index = start; read_index <= end; read_index++) {
        s[write_index++] = s[read_index];
    }
    s[write_index] = '\0';
}

void string_split(const string s, const char* separators, size_t num_separators, string* splitted_tokens, size_t* num_tokens){
   
}
