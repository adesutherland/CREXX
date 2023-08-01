/* Char Segmentation PoC */
#include <assert.h>
#include <stdio.h>
#include <string.h>

int lex_codepoint(char *str, char* out);
int lex_grapheme(char *str, char* out);
int tests();

/* Adds a codepoint to a utf-8 buffer */
void encodechar_utf32_8(unsigned int cp, char **buffer) {
    if (cp<0x80) *(*buffer)++=(char)cp;
    else if (cp<0x800) *(*buffer)++=(char)(192+cp/64), *(*buffer)++=(char)(128+cp%64);
    else if (cp<0x10000) *(*buffer)++=(char)(224+cp/4096), *(*buffer)++=(char)(128+cp/64%64), *(*buffer)++=(char)(128+cp%64);
    else if (cp<0x110000) *(*buffer)++=(char)(240+cp/262144), *(*buffer)++=(char)(128+cp/4096%64), *(*buffer)++=(char)(128+cp/64%64), *(*buffer)++=(char)(128+cp%64);
}

/* Adds string to utf-8 buffer */
void append_to_buffer(char* to_append, char **buffer) {
    int i;
    for (i=0; i<strlen(to_append); i++) {
        *(*buffer)++ = to_append[i];
    }
}

int lex(char *str, char* out) {
    int r;
    out[0] = 0;

//    lex_codepoint(str, out);
//    sprintf(out + strlen(out), " --- ");
    r = lex_grapheme(str, out);
    return r;
}

int main() {

    int errors;

    errors = tests();

    return errors?1:0;
}
