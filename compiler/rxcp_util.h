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
 * Shared Compiler Utilities Header
 */

#ifndef CREXX_RXCP_UTIL_H
#define CREXX_RXCP_UTIL_H

#include <stdio.h>
#include <stddef.h>
#include "rxcp_types.h"

/* Generic error printing and exit */
void error_and_exit(int rc, char* message);

/* printf - but returns a malloced buffer with the result */
char* mprintf(const char* format, ...);

/* Encodes a string into a malloced buffer for output/assembly */
char* encdstrg(const char* string, size_t length);

/* Encodes a string into a malloced buffer for comments */
char* encode_comment_malloc(const char* string, size_t length);

/* Encodes a string into a malloced buffer for line source metadata */
char* encode_line_source_malloc(const char* string, size_t length);

/* Utility to check if a token (typically an IDENTIFIER) is a certain value */
int tokenis(Token *token, const char* value);

/* Hex and Binary character conversion utilities */
int hexchar2int(char hexbyte);
int binchar2int(const char* bin);

/* Escape a character for Rexx string literals or error messages */
char* escape_character(unsigned char c);

/* Print characters with escaping for unexpected content */
void prt_unex(FILE *output, const char *ptr, int len);

/* Convert Token ID to human-readable string */
const char* token_to_string(int token_id);

/* Convert NodeType to human-readable string */
const char* node_type_to_string(NodeType type);

#endif //CREXX_RXCP_UTIL_H
