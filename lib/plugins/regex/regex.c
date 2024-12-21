/**
 * @file regex.c
 * @brief Regular Expression Plugin for crexx/pa
 *
 * This module implements regular expression functionality with the following features:
 * - Pattern compilation and validation
 * - String matching
 * - Error reporting
 * - Memory management
 *
 * REXX Interface Functions:
 * - regex.compile(pattern) -> handle
 * - regex.match(handle, string) -> 1/0
 * - regex.free(handle) -> rc
 * - regex.error(handle) -> error message
 *
 * Error Codes:
 * - RX_SUCCESS (0): Operation successful
 * - RX_ERROR_PARAM (-1): Invalid parameters
 * - RX_ERROR_MEMORY (-2): Memory allocation failed
 * - RX_ERROR_COMPILE (-3): Pattern compilation failed
 * - RX_ERROR_EXEC (-4): Pattern matching failed
 *
 * Example Usage:
 * ```rexx
 * handle = regex.compile("^[A-Za-z0-9]+$")
 * if regex.match(handle, "Test123") then
 *    say "Pattern matches!"
 * call regex.free handle
 * ```
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>
#include "crexxpa.h"

// Error codes
#define RX_SUCCESS        0
#define RX_ERROR_PARAM   -1
#define RX_ERROR_MEMORY  -2
#define RX_ERROR_COMPILE -3
#define RX_ERROR_EXEC    -4

// Regex compilation flags
#define RX_BASIC        0      // Basic Regular Expressions (BRE)
#define RX_EXTENDED     1      // Extended Regular Expressions (ERE)
#define RX_ICASE       (1<<1)  // Case insensitive matching
#define RX_NEWLINE     (1<<2)  // Honor newline as special character
#define RX_NOSUB       (1<<3)  // Only report success/failure
#define RX_NOTBOL      (1<<4)  // ^ doesn't match beginning of string
#define RX_NOTEOL      (1<<5)  // $ doesn't match end of string
#define RX_MULTILINE   (1<<6)  // Multiline matching
#define RX_DOTALL      (1<<7)  // . matches newline
#define RX_VERBOSE     (1<<8)  // Allow pattern whitespace and comments

// System regex compilation flags from regex.h
#ifndef REG_EXTENDED
#define REG_EXTENDED 1
#endif

#ifndef REG_ICASE
#define REG_ICASE 2
#endif

#ifndef REG_NEWLINE
#define REG_NEWLINE 4
#endif

#ifndef REG_NOSUB
#define REG_NOSUB 8
#endif

#ifndef REG_NOTBOL
#define REG_NOTBOL 128
#endif

#ifndef REG_NOTEOL
#define REG_NOTEOL 256
#endif

// Structure to hold regex data
struct RegexHandle {
    regex_t regex;
    char* pattern;
    char error_msg[1024];
    int match_flags;    // Store match-time flags
};

void toUpperCase(char *str) {
    if (str == NULL) return; // Handle null pointer
    while (*str) {
        *str = toupper((unsigned char)*str); // Convert current character to uppercase
        str++;
    }
}

PROCEDURE(compile_pattern) {
    char* pattern = GETSTRING(ARG0);
    int flags = GETINT(ARG1);  // New parameter for flags
    struct RegexHandle* handle;
    int ret;
    int regflags = 0;

    if (!pattern) {
        RETURNINTX(RX_ERROR_PARAM);
    }

    // Map our flags to system regex flags
    if (flags & RX_EXTENDED) regflags |= REG_EXTENDED;
    if (flags & RX_ICASE)   regflags |= REG_ICASE;
    if (flags & RX_NEWLINE) regflags |= REG_NEWLINE;
    if (flags & RX_NOSUB)   regflags |= REG_NOSUB;

    handle = (struct RegexHandle*)malloc(sizeof(struct RegexHandle));
    if (!handle ) {
        printf("Allocation Error 1: %s \n",handle->pattern);
        RETURNINTX(RX_ERROR_MEMORY);
    }
    if ((intptr_t) handle<0) {
        printf("Allocation Error 2: %p %s\n",handle, handle->pattern);
        RETURNINTX(RX_ERROR_COMPILE);
    }

    // Initialize handle
    memset(handle, 0, sizeof(struct RegexHandle));
    handle->pattern = strdup(pattern);

    if (!handle->pattern) {
        printf("Allocation Error 3: %s\n",handle->pattern);
        free(handle);
        RETURNINTX(RX_ERROR_MEMORY);
    }

    // Compile the pattern
     ret = regcomp(&handle->regex, pattern, regflags);
    if (ret) {
        regerror(ret, &handle->regex, handle->error_msg, sizeof(handle->error_msg));
        printf("Compile error: %s\n", handle->error_msg);
        free(handle->pattern);
        free(handle);
        RETURNINTX(RX_ERROR_COMPILE);
    }
// Return handle as positive integer
    RETURNINTX((intptr_t)handle);
   ENDPROC
}

PROCEDURE(match_pattern) {
    struct RegexHandle* handle = (struct RegexHandle*)GETINT(ARG0);
    char* test_str = GETSTRING(ARG1);
    int flags = GETINT(ARG2);  // New parameter for match flags
    int ret;
    int regflags = 0;

    if (!handle || !test_str) {
        RETURNINTX(RX_ERROR_PARAM);
    }
    // Convert match flags
    if (flags & RX_NOTBOL) regflags |= REG_NOTBOL;
    if (flags & RX_NOTEOL) regflags |= REG_NOTEOL;

    ret = regexec(&handle->regex, test_str, 0, NULL, regflags);
    if (ret && ret != REG_NOMATCH) {
        regerror(ret, &handle->regex, handle->error_msg, sizeof(handle->error_msg));
        RETURNINTX(RX_ERROR_EXEC);
    }

    RETURNINTX(ret == 0 ? 1 : 0);
    ENDPROC
}

PROCEDURE(free_pattern) {
    struct RegexHandle* handle = (struct RegexHandle*)GETINT(ARG0);

    if (!handle) {
        RETURNINTX(RX_ERROR_PARAM);
    }

    regfree(&handle->regex);
    free(handle->pattern);
    free(handle);

    RETURNINTX(RX_SUCCESS);
    ENDPROC
}

PROCEDURE(get_error) {
    struct RegexHandle* handle = (struct RegexHandle*)GETINT(ARG0);
    if (!handle|| (int) handle<0) {
        RETURNSTRX("Invalid handle");
    }

   RETURNSTRX(handle->error_msg[0] ? handle->error_msg : "No error");
    ENDPROC
}

/**
 * @brief Calculates Levenshtein distance between two strings
 * @param s1 First string
 * @param s2 Second string
 * @return Distance or error code
 */
PROCEDURE(levenshtein) {
    char* s1 = GETSTRING(ARG0);
    char* s2 = GETSTRING(ARG1);
    int len1, len2;
    int i, j;
    int *matrix;
    int result;

    // Check input parameters
    if (!s1 || !s2) {
        RETURNINTX(-1);  // Invalid parameters
    }

    // Get string lengths
    len1 = strlen(s1);
    len2 = strlen(s2);

    // Handle empty strings
    if (len1 == 0) {
        RETURNINTX(len2);
    }
    if (len2 == 0) {
        RETURNINTX(len1);
    }

    // Allocate matrix: (len1+1) x (len2+1)
    matrix = (int*)malloc((len1 + 1) * (len2 + 1) * sizeof(int));
    if (!matrix) {
        RETURNINTX(-2);  // Memory allocation error
    }

    // Initialize first row and column
    for (i = 0; i <= len1; i++) {
        matrix[i * (len2 + 1)] = i;  // First column
    }
    for (j = 0; j <= len2; j++) {
        matrix[j] = j;  // First row
    }

    // Fill the matrix using dynamic programming
    for (i = 1; i <= len1; i++) {
        for (j = 1; j <= len2; j++) {
            // Calculate cost of operations
            int deletion = matrix[(i-1) * (len2 + 1) + j] + 1;
            int insertion = matrix[i * (len2 + 1) + (j-1)] + 1;
            int substitution = matrix[(i-1) * (len2 + 1) + (j-1)] +
                               (s1[i-1] == s2[j-1] ? 0 : 1);

            // Choose minimum cost operation
            int min = deletion;
            if (insertion < min) min = insertion;
            if (substitution < min) min = substitution;

            // Store result in matrix
            matrix[i * (len2 + 1) + j] = min;
        }
    }

    // Get final distance from bottom-right cell
    result = matrix[len1 * (len2 + 1) + len2];

    // Free allocated memory
    free(matrix);

    // Return the Levenshtein distance
    RETURNINTX(result);
    ENDPROC
}

/**
 * @brief Calculates Hamming distance between two strings
 * @param s1 First string
 * @param s2 Second string
 * @param upper Convert to uppercase before comparison (1=yes, 0=no)
 * @return Distance or error code
 *
 * Error codes:
 * -1: Invalid parameters (NULL strings)
 * -2: Strings must be equal length
 */
PROCEDURE(hamming) {
    char* s1 = GETSTRING(ARG0);
    char* s2 = GETSTRING(ARG1);
    int upper = GETINT(ARG2);
    int len1=strlen(s1);
    int i;
    int distance = 0;

    // Check input parameters
    if (!s1 || !s2) {
        RETURNINTX(-1);  // Invalid parameters
    }
 // Strings must be equal length for Hamming distance
    if (len1 != strlen(s2)) {
        RETURNINTX(-2);  // Unequal lengths
    }
     if (upper) { // Convert to uppercase
       toUpperCase(s1);
       toUpperCase(s2);
     }
      // Calculate Hamming distance
    for (i = 0; i < len1; i++) {
        if (s1[i] != s2[i]) {
            distance++;
        }
    }
    RETURNINTX(distance);
    ENDPROC
}

/* -------------------------------------------------------------------------------------
 * Functions to be provided to rexx
 * -------------------------------------------------------------------------------------
 */
LOADFUNCS
    ADDPROC(compile_pattern, "regex.rxcompile",  "b", ".int", "pattern=.string,flags=.int");
    ADDPROC(match_pattern,   "regex.rxmatch",    "b", ".int", "handle=.int,string=.string,flags=.int");
    ADDPROC(free_pattern,    "regex.rxfree",     "b", ".int", "handle=.int");
    ADDPROC(get_error,       "regex.rxerror",    "b", ".string", "handle=.int");
    ADDPROC(levenshtein,     "regex.levenshtein","b", ".int", "string1=.string,string2=.string");
    ADDPROC(hamming,         "regex.hamming",    "b", ".int", "string1=.string,string2=.string,uppercase=.int");
ENDLOADFUNCS

