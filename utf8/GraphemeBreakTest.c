/* Test Suite - Auto generated file! */
#include <stdio.h>
#include <string.h>

void encodechar_utf32_8(unsigned int cp, char **buffer);
void append_to_buffer(char* to_append, char **buffer);
int lex_grapheme(char *str, char* out);

int tests_grapheme() {
    char in_buffer[250];
    char out_buffer[250];
    char expected_buffer[250];
    char* in;
    char* expected;
    int errors = 0;

    printf("0 tests run\n");
    if (errors) printf("*** %d ERRORS ***\n", errors);
    return errors;
}

