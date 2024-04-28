//
// Created by Adrian Sutherland on 26/03/2024.
//

#ifndef REXXSAAPOC_HTTPCLIENT_H
#define REXXSAAPOC_HTTPCLIENT_H

#include "crexxsaa.h"

// Parse Error Structure
typedef struct {
    int error_code;
    size_t position; // Note the position is approximate and may not be accurate
    char* message;
} PARSE_ERROR;

// parseJSON error codes
#define PARSE_ERROR_OK 0
#define PARSE_ERROR_EXPECTING_OPEN_CURLY 1
#define PARSE_ERROR_EXPECTING_SERVICE_BLOCKS 2
#define PARSE_ERROR_EXPECTING_STRING 3
#define PARSE_ERROR_NO_CLOSING_QUOTE 4
#define PARSE_ERROR_EXPECTING_COLON 5
#define PARSE_ERROR_EXPECTING_ARRAY 6
#define PARSE_ERROR_EXPECTING_CLOSE_CURLY 7
#define PARSE_ERROR_CREATE_SHVBLOCK_FAILED 8
#define PARSE_ERROR_DUPLICATE_NAME 9
#define PARSE_ERROR_DUPLICATE_REQUEST 10
#define PARSE_ERROR_INVALID_REQUEST 11
#define PARSE_ERROR_DUPLICATE_RESULT 12
#define PARSE_ERROR_INVALID_RESULT 13
#define PARSE_ERROR_DUPLICATE_VALUE 14
#define PARSE_ERROR_INVALID_ATTRIBUTE 15
#define PARSE_ERROR_MISSING_NAME 16
#define PARSE_ERROR_DUPLICATE_CLASS 17
#define PARSE_ERROR_DUPLICATE_MEMBERS 18
#define PARSE_ERROR_MEMORY_ALLOCATION 19
#define PARSE_ERROR_MISSING_CLASS 20
#define PARSE_ERROR_MISSING_MEMBERS 21
#define PARSE_ERROR_INVALID_TYPE 22
#define PARSE_ERROR_UNEXPECTED_TOKEN 23
#define PARSE_ERROR_INVALID_NUMBER 24
#define PARSE_ERROR_EXPECTING_BASE64 25
#define PARSE_ERROR_INVALID_BASE64 26
#define PARSE_ERROR_INVALID_STRING 27

// parseJSON - Parse JSON string and return SHVBLOCK structure
// json - JSON string to parse
// shvblock_handle - Pointer to SHVBLOCK handle
// error - Pointer to PARSE_ERROR structure (if this is NULL, the function will not return error details)
// Returns 0 if successful, otherwise returns an error code (see PARSE_ERROR structure for details
int parseJSON(char* json, SHVBLOCK** shvblock_handle, PARSE_ERROR* error);

#endif //REXXSAAPOC_HTTPCLIENT_H
