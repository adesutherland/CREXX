/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * Shared Compiler Utilities
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "rxcp_util.h"
#include "rxcp_token.h"

void error_and_exit(int rc, char* message) {
    fprintf(stderr, "ERROR: %s - try \"rxc -h\"\n", message);
    exit(rc);
}

char* mprintf(const char* format, ...) {
    char *buffer;
    size_t buffer_len;
    size_t needed_len;
    va_list argptr;

    /* Guess a length which is likely to be big enough */
    buffer_len = 100; /* A stab in the dark! */
    buffer = malloc(buffer_len);

    va_start(argptr, format);
    needed_len = vsnprintf(buffer, buffer_len, format, argptr) + 1;
    va_end(argptr);
    if (needed_len > buffer_len) {
        /* Buffer not big enough - do it again */
        buffer_len = needed_len;
        free(buffer);
        buffer = malloc(buffer_len);
        va_start(argptr, format);
        vsnprintf(buffer, buffer_len, format, argptr);
        va_end(argptr);
    }
    return buffer;
}

#define ADD_CHAR_TO_BUFFER(ch) {out_len++; if (buffer_len) { *(buffer++) = (ch); buffer_len--; }}

static size_t encode_print(char* buffer, size_t buffer_len, const char* string, size_t length) {
    size_t out_len = 0;
    if (!length) {
        if (buffer_len) *buffer = 0;
        return 0;
    }
    while (length) {
        switch (*string) {
            case '\\':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('\\')
                break;
            case '\n':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('n')
                break;
            case '\t':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('t')
                break;
            case '\a':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('a')
                break;
            case '\b':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('b')
                break;
            case '\f':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('f')
                break;
            case '\r':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('r')
                break;
            case '\v':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('v')
                break;
            case '\'':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('\'')
                break;
            case '\"':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('\"')
                break;
            case 0:
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('0')
                break;
            case '\?':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('?')
                break;
            default:
                ADD_CHAR_TO_BUFFER(*string)
        }
        string++;
        length--;
    }
    if (buffer_len) *buffer = 0;
    else if (out_len > 0) *(buffer - 1) = 0;
    return out_len;
}

char* encdstrg(const char* string, size_t length) {
    char *buffer;
    size_t buffer_len;
    size_t needed_len;

    /* Guess a length which is likely to be big enough */
    buffer_len = (length * 2) + 1;
    buffer = malloc(buffer_len);

    needed_len = encode_print(buffer, buffer_len, string, length) + 1;
    if (needed_len > buffer_len) {
        /* Buffer not big enough - do it again */
        buffer_len = needed_len;
        free(buffer);
        buffer = malloc(buffer_len);
        encode_print(buffer, buffer_len, string, length);
    }
    return buffer;
}

static size_t encode_comment(char* buffer, size_t buffer_len, const char* string, size_t length) {
    size_t out_len = 0;
    while (length) {
        switch (*string) {
            case '\n':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('n')
                break;
            case '\t':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('t')
                break;
            case '\f':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('f')
                break;
            case '\r':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('r')
                break;
            case 0:
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('0')
                break;
            default:
                ADD_CHAR_TO_BUFFER(*string)
        }
        string++;
        length--;
    }
    if (buffer_len) *buffer = 0;
    else if (out_len > 0) *(buffer - 1) = 0;
    return out_len;
}

char* encode_comment_malloc(const char* string, size_t length) {
    char *buffer;
    size_t buffer_len;
    size_t needed_len;

    /* Guess a length which is likely to be big enough */
    buffer_len = (length * 2) + 1;
    buffer = malloc(buffer_len);

    needed_len = encode_comment(buffer, buffer_len, string, length) + 1;
    if (needed_len > buffer_len) {
        /* Buffer not big enough - do it again */
        buffer_len = needed_len;
        free(buffer);
        buffer = malloc(buffer_len);
        encode_comment(buffer, buffer_len, string, length);
    }
    return buffer;
}

static size_t encode_line_source(char* buffer, size_t buffer_len, const char* string, size_t length) {
    size_t out_len = 0;
    while (length) {
        switch (*string) {
            case '\n':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('n')
                break;
            case '\"':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('\"')
                break;
            case '\t':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('t')
                break;
            case '\f':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('f')
                break;
            case '\r':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('r')
                break;
            case 0:
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('0')
                break;
            default:
                ADD_CHAR_TO_BUFFER(*string)
        }
        string++;
        length--;
    }
    if (buffer_len) *buffer = 0;
    else if (out_len > 0) *(buffer - 1) = 0;
    return out_len;
}

char* encode_line_source_malloc(const char* string, size_t length) {
    char *buffer;
    size_t buffer_len;
    size_t needed_len;

    /* Guess a length which is likely to be big enough */
    buffer_len = (length * 2) + 1;
    buffer = malloc(buffer_len);

    needed_len = encode_line_source(buffer, buffer_len, string, length) + 1;
    if (needed_len > buffer_len) {
        /* Buffer not big enough - do it again */
        buffer_len = needed_len;
        free(buffer);
        buffer = malloc(buffer_len);
        encode_line_source(buffer, buffer_len, string, length);
    }
    return buffer;
}

#undef ADD_CHAR_TO_BUFFER

int tokenis(Token *token, const char* value) {
    char text_buffer[15];
    int val;
    int i;
    if (!token || !token->token_string || !value) return 0;

    val = (int)strlen(value);
    if (val > 14) val = 14;
    strncpy(text_buffer, token->token_string, val);
    // lowercase the buffer
    for (i = 0; i < val; i++) {
        text_buffer[i] = (char)tolower(text_buffer[i]);
    }
    text_buffer[val] = 0;

    if (strcmp(text_buffer, value) == 0) return 1;
    return 0;
}

int hexchar2int(char hexbyte) {
    int val = -1;

    // transform hex character to the 4bit equivalent number
    if (hexbyte >= '0' && hexbyte <= '9') val = hexbyte - '0';
    else if (hexbyte >= 'a' && hexbyte <='f') val = hexbyte - 'a' + 10;
    else if (hexbyte >= 'A' && hexbyte <='F') val = hexbyte - 'A' + 10;

    return val;
}

int binchar2int(const char* bin) {
    int result = 0;

    if (bin[0] == '1') result += 8;
    if (bin[1] == '1') result += 4;
    if (bin[2] == '1') result += 2;
    if (bin[3] == '1') result++;

    return result;
}

char* escape_character(unsigned char c) {
    static char buffer[5];
    buffer[0] = '\\';
    buffer[2] = 0;
    /* Encode C style */
    switch (c) {
        case '\\':
            buffer[1] = '\\';
            break;
        case '\n':
            buffer[1] = 'n';
            break;
        case '\t':
            buffer[1] = 't';
            break;
        case '\a':
            buffer[1] = 'a';
            break;
        case '\b':
            buffer[1] = 'b';
            break;
        case '\f':
            buffer[1] = 'f';
            break;
        case '\r':
            buffer[1] = 'r';
            break;
        case '\v':
            buffer[1] = 'v';
            break;
        case '\'':
            buffer[1] = '\'';
            break;
        case '\"':
            buffer[1] = '\"';
            break;
        case 0:
            buffer[1] = '0';
            break;
        case '\?':
            buffer[1] = '\?';
            break;
        default:
            /* Should we escape this character? */
            if (isprint(c)) {
                buffer[0] = (char)c;
                buffer[1] = 0;
            } else {
                /* Escape as a hex character */
                buffer[1] = 'x';
                snprintf(buffer + 2, 3, "%02x", c);
            }
            break;
    }
    return buffer;
}

void prt_unex(FILE* output, const char *ptr, int len) {
    int i;
    if (!ptr) return;
    for (i = 0; i < len; i++, ptr++) {
        switch (*ptr) {
            case '\0':
                fprintf(output, "\\0");
                break;
            case '\a':
                fprintf(output, "\\a");
                break;
            case '\b':
                fprintf(output, "\\b");
                break;
            case '\f':
                fprintf(output, "\\f");
                break;
            case '\n':
                fprintf(output, "\\n");
                break;
            case '\r':
                fprintf(output, "\\r");
                break;
            case '\t':
                fprintf(output, "\\t");
                break;
            case '\v':
                fprintf(output, "\\v");
                break;
            case '\\':
                fprintf(output, "\\\\");
                break;
            case '\?':
                fprintf(output, "\\?");
                break;
            case '\'':
                fprintf(output, "\\'");
                break;
            case '\"':
                fprintf(output, "\\\"");
                break;
            default:
                /* Should we escape this character? */
                if (isprint((unsigned char)*ptr)) {
                    fprintf(output, "%c", *ptr);
                }
                else {
                    /* Escape as a hex character */
                    fprintf(output, "\\x%02x", (unsigned char)*ptr);
                }
        }
    }
}
