/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */

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

#endif //CREXX_RXCP_UTIL_H
