// Simple JSON Parser to parse the JSON body and populate the SHVBLOCK linked list
// The JSON format is defined in crexxjson.md and the c buffer is defined in crexxsaa.h */
// The library has been implemented in a single file for simplicity and to avoid external dependencies
//
// Created by and copyright (c) 2024 Adrian Sutherland

#include "httpcommon.h"
#include "crexxsaa.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

/* Block to hold the first SHVBLOCK and a pointer to the JSON buffer to facilitate freeing the buffers */
typedef struct SHVBUFFER {
    struct SHVBLOCK shvblock;     // The SHVBLOCK
    HTTPMessage *message;            // Pointer to the json buffer (the pointer is appended to the SHVBLOCK)
} SHVBUFFER;

/* Parse Function prototypes */
int parseShvblock(char **json, SHVBLOCK* current_shvblock); // Parse individual service block
int parseObject(char **json, OBJBLOCK* shvobject); // Parse value attribute
int parseValue(char **json, OBJBLOCK** shvobject_handle); // Parse object
int json_skip_whitespace(char **json);
int json_next_object(char **json);
int json_skip_block(char **json);
int json_skip_colon(char **json);
int json_skip_string(char **json, char **string);
char *json_peak_string(char *json, char *string);
int json_skip_null(char **json);
int json_skip_true(char **json);
int json_skip_false(char **json);
int json_skip_number(char **json, VALUETYPE *type_handle, long *integer_handle, double *real_handle); // Function to parse and skip a number
int json_peek_binary(char *json); // Function to peek if the next object might be a binary object
int json_skip_binary(char **json, char **binary_handle, size_t *length_handle); // Function to skip a binary object
long parseRequestCode(char *request_value); // Parse the request code (returns long so a negative value can be used for an error)
long parseResultCode(char *result_value); // Parse the result code (returns long so a negative value can be used for an error)

/* Other function prototypes */
int base64_decode(char *buffer); // Base64 decode function - decoding done in situ in the buffer, and returns the length of the decoded data
void FreeMemBlock(MEMBLOCK *memblock); // Free the MEMBLOCK, and linked list of members
void FreeObjBlock(OBJBLOCK *shvobject); // Free the MEMBLOCK
char* error_message(int error_code); // Return an error message for the error code
int json_escape_string(char *string); // Function to escape a string using JSON rules

/* Return an error message for the error code */
char* error_message(int error_code) {
    switch (error_code) {
        case PARSE_ERROR_OK:
            return "No error";
        case PARSE_ERROR_EXPECTING_OPEN_CURLY:
            return "Expecting {";
        case PARSE_ERROR_EXPECTING_SERVICE_BLOCKS:
            return "Expecting \"serviceBlocks\"";
        case PARSE_ERROR_EXPECTING_STRING:
            return "Expecting a string";
        case PARSE_ERROR_NO_CLOSING_QUOTE:
            return "No closing quote";
        case PARSE_ERROR_EXPECTING_COLON:
            return "Expecting :";
        case PARSE_ERROR_EXPECTING_ARRAY:
            return "Expecting [ for array";
        case PARSE_ERROR_EXPECTING_CLOSE_CURLY:
            return "Expecting }";
        case PARSE_ERROR_CREATE_SHVBLOCK_FAILED:
            return "Failed to create SHVBLOCK";
        case PARSE_ERROR_DUPLICATE_NAME:
            return "Duplicate name attribute";
        case PARSE_ERROR_DUPLICATE_REQUEST:
            return "Duplicate request attribute";
        case PARSE_ERROR_INVALID_REQUEST:
            return "Invalid request value";
        case PARSE_ERROR_DUPLICATE_RESULT:
            return "Duplicate result attribute";
        case PARSE_ERROR_INVALID_RESULT:
            return "Invalid result value";
        case PARSE_ERROR_DUPLICATE_VALUE:
            return "Duplicate value attribute";
        case PARSE_ERROR_INVALID_ATTRIBUTE:
            return "Invalid attribute";
        case PARSE_ERROR_MISSING_NAME:
            return "Missing name attribute";
        case PARSE_ERROR_DUPLICATE_CLASS:
            return "Duplicate class attribute";
        case PARSE_ERROR_DUPLICATE_MEMBERS:
            return "Duplicate members attribute";
        case PARSE_ERROR_MEMORY_ALLOCATION:
            return "Memory allocation error";
        case PARSE_ERROR_MISSING_CLASS:
            return "Missing class attribute";
        case PARSE_ERROR_MISSING_MEMBERS:
            return "Missing members attribute";
        case PARSE_ERROR_INVALID_TYPE:
            return "Invalid type";
        case PARSE_ERROR_UNEXPECTED_TOKEN:
            return "Unexpected token";
        case PARSE_ERROR_EXPECTING_BASE64:
            return "Expecting base64";
        case PARSE_ERROR_INVALID_BASE64:
            return "Invalid base64";
        case PARSE_ERROR_INVALID_STRING:
            return "Invalid string";
        case PARSE_ERROR_EXPECTED_COMMA_OR_CLOSE_CURLY:
            return "Expected comma or closing curly brace";
        case PARSE_ERROR_EXPECTED_COMMA_OR_CLOSE_ARRAY:
            return "Expected comma or closing array";
        default:
            return "Unknown error";
    }
}

/* Free a MEMBLOCK linked list */
void FreeMemBlock(MEMBLOCK *memblock) { // NOLINT(misc-no-recursion) - suppress the clang-tidy warning about recursion
    MEMBLOCK *memblock_next;

    while (memblock) {
        memblock_next = memblock->membernext;
        FreeObjBlock(memblock->memberobject);  // Recursively free the members
        free(memblock); // Free the object
        memblock = memblock_next;
    }
}

/* Free the OBJBLOCK */
void FreeObjBlock(OBJBLOCK *shvobject) { // NOLINT(misc-no-recursion) - suppress the clang-tidy warning about recursion
    if (!shvobject) return;
    if (shvobject->type == VALUE_OBJECT || shvobject->type == VALUE_ARRAY) {
        FreeMemBlock(shvobject->value.members); // Free the members
    }
    free(shvobject); // Free the object
}

/* Create a SHVBLOCK */
SHVBLOCK* CreateShvBlock(SHVBLOCK* previous, HTTPMessage *messageBuffer) {
    SHVBLOCK* shvblock;
    if (previous) {
        /* Create the SHVBLOCK */
        shvblock = (SHVBLOCK*)malloc(sizeof(SHVBLOCK));
        if (!shvblock) {
            fprintf(stderr, "Memory allocation error\n");
            return 0;
        }
        previous->shvnext = shvblock; // Set the next SHVBLOCK in the linked list
    }
    else {
        /* Create the starting SHVBUFFER / SHVBLOCK */
        SHVBUFFER* shvbuffer = (SHVBUFFER*)malloc(sizeof(SHVBUFFER));
        if (!shvbuffer) {
            fprintf(stderr, "Memory allocation error\n");
            return NULL;
        }
        shvbuffer->message = messageBuffer;
        shvblock = (SHVBLOCK*)shvbuffer; // Set the SHVBLOCK pointer to the SHVBUFFER
    }

    // Clear the SHVBLOCK
    shvblock->shvname = NULL;
    shvblock->shvobject = NULL;
    shvblock->shvnext = NULL;
    shvblock->shvcode = 0;
    shvblock->shvret = 0;

    return shvblock;
}

/* Free the SHVBLOCK Variable Pool Linked List */
void FreeRexxVariablePoolResult(SHVBLOCK *shvblock) {
    SHVBLOCK *shvblock_next;
    SHVBUFFER *shvbuffer;

    /* Free the start SHVBLOCK which is a SHVBUFFER */
    shvbuffer = (SHVBUFFER*)shvblock;
    FreeObjBlock(shvbuffer->shvblock.shvobject); // Free the OBJBLOCK
    shvblock = shvbuffer->shvblock.shvnext;
    free_response(shvbuffer->message); // Free the http message contents
    free(shvbuffer->message); // Free the http message structure itself
    free(shvbuffer); // Free the SHVBUFFER

    /* Free the rest of the SHVBLOCK linked list */
    while (shvblock) {
        shvblock_next = shvblock->shvnext;
        FreeObjBlock(shvblock->shvobject); // Free the OBJBLOCK
        free(shvblock);
        shvblock = shvblock_next;
    }
}

/* Uncomment this if you don't have a strcasecmp function
static int strcasecmp(const char *s1, const char *s2) {
    while (*s1 && (tolower((unsigned char)*s1) == tolower((unsigned char)*s2))) {
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}
*/

/* Parses the JSON body */
int parseJSON(HTTPMessage *message, SHVBLOCK** shvblock_handle, PARSE_ERROR* error) {
    char *name;
    char* json = BODY(message); // Pointer to the current position in the JSON buffer
    int rc;
    SHVBLOCK* current_shvblock = 0; // This points to the current SHVBLOCK being populated
    *shvblock_handle = 0; // This points to the fist SHVBLOCK in the linked list (or NULL if there are no SHVBLOCKS yet)

    // Create the SHVBLOCK
    current_shvblock = CreateShvBlock(current_shvblock, message);
    if (!current_shvblock) {
        rc = PARSE_ERROR_CREATE_SHVBLOCK_FAILED;
        if (error) {
            error->error_code = rc;
            error->position = json - BODY(message);
            error->message = error_message(rc);
        }
        return rc;
    }
    *shvblock_handle = current_shvblock; // Update the head shvblock pointer

    // Skip to the start of the object
    rc = json_next_object(&json);
    if (rc != PARSE_ERROR_OK) {
        if (error) {
            error->error_code = rc;
            error->position = json - BODY(message);
            error->message = error_message(rc);
        }
        return rc;
    }

    // Skip to the serviceBlocks string
    rc = json_skip_string(&json, &name);
    if (rc != PARSE_ERROR_OK) {
        if (error) {
            error->error_code = rc;
            error->position = json - BODY(message);
            error->message = error_message(rc);
        }
        return rc;
    }
    if (strcmp(name, "serviceBlocks") != 0) {
        rc = PARSE_ERROR_EXPECTING_SERVICE_BLOCKS;
        if (error) {
            error->error_code = rc;
            error->position = json - BODY(message) - strlen(name) - 1;
            error->message = error_message(rc);
        }
        return rc;
    }

    // Skip to the array
    rc = json_skip_colon(&json);
    if (rc != PARSE_ERROR_OK) {
        if (error) {
            error->error_code = rc;
            error->position = json - BODY(message);
            error->message = error_message(rc);
        }
        return rc;
    }
    json_skip_whitespace(&json);
    if (*json != '[') {
        rc = PARSE_ERROR_EXPECTING_ARRAY;
        if (error) {
            error->error_code = rc;
            error->position = json - BODY(message);
            error->message = error_message(rc);
        }
        return rc;
    }
    json++;

    // Loop through the array
    while (*json != ']') {
        // Skip to the start of the object
        rc = json_next_object(&json);
        if (rc != PARSE_ERROR_OK) {
            if (error) {
                error->error_code = rc;
                error->position = json - BODY(message);
                error->message = error_message(rc);
            }
            return rc;
        }

        // Parse individual service block
        rc = parseShvblock(&json, current_shvblock);
        if (rc != PARSE_ERROR_OK) {
            if (error) {
                error->error_code = rc;
                error->position = json - BODY(message);
                error->message = error_message(rc);
            }
            return rc;
        }

        // Skip to the next object or the end of the array
        json_skip_whitespace(&json);
        if (*json != ']') { // If not the end of the array
            // If the next character is a comma, skip it
            if (*json == ',') json++;
            else {
                rc = PARSE_ERROR_EXPECTED_COMMA_OR_CLOSE_ARRAY;
                if (error) {
                    error->error_code = rc;
                    error->position = json - BODY(message);
                    error->message = error_message(rc);
                }
                return rc;
            }
        }
        // Skip any whitespace
        json_skip_whitespace(&json);

        // Create the next SHVBLOCK
        if (*json != ']') {
            current_shvblock = CreateShvBlock(current_shvblock, message);
            if (!current_shvblock) {
                rc = PARSE_ERROR_CREATE_SHVBLOCK_FAILED;
                if (error) {
                    error->error_code = rc;
                    error->position = json - BODY(message);
                    error->message = error_message(rc);
                }
                return rc;
            }
        }
    }
    json++; // Skip the end of the array

    json_skip_block(&json); // skip end of the blocks

    if (error) {
        error->error_code = PARSE_ERROR_OK;
        error->position = json - BODY(message);
        error->message = error_message(PARSE_ERROR_OK);
    }
    return PARSE_ERROR_OK;
}

// Function to parse individual service block
int parseShvblock(char** json, SHVBLOCK* current_shvblock) {
    // Parse JSON Body using the json functions, example format:
    //    {
    //      "name": "<variable_name>",
    //      "request": "<request_code>",
    //      "result": "<result_code>",
    //      "value": "<value>"
    //    }
    // Note that value can be a string, number, boolean, object, array or null
    // Note that the first { has already been skipped

    // Loop round the object names (which can be in any order) and parse the values as they are found
    char *attribute;
    char *value;
    long result;
    char has_name = 0;
    char has_request = 0;
    char has_result = 0;
    char has_value = 0;
    int rc;

    while (*(*json) != '}') {
        // Skip to the attribute string
        rc = json_skip_string(json, &attribute);
        if (rc != PARSE_ERROR_OK) {
            return rc;
        }

        // Compare the attribute and set the corresponding SHVBLOCK value
        if (strcmp(attribute, "name") == 0) {
            // Check for duplicate
            if (has_name) {
                // Move the pointer back to the start of the attribute
                *json -= strlen(attribute) + 1;
                return PARSE_ERROR_DUPLICATE_NAME;
            }
            has_name = 1;
            // Skip to the value string
            rc = json_skip_colon(json);
            if (rc != PARSE_ERROR_OK) {
                return rc;
            }
            rc = json_skip_string(json, &value);
            if (rc != PARSE_ERROR_OK) {
                return rc;
            }
            // Set the name
            current_shvblock->shvname = value;

        } else if (strcmp(attribute, "request") == 0) {
            // Check for duplicate
            if (has_request) {
                // Move the pointer back to the start of the attribute
                *json -= strlen(attribute) + 1;
                return PARSE_ERROR_DUPLICATE_REQUEST;
            }
            has_request = 1;
            // Skip to the value string
            rc = json_skip_colon(json);
            if (rc != PARSE_ERROR_OK) {
                return rc;
            }
            rc = json_skip_string(json, &value);
            if (rc != PARSE_ERROR_OK) {
                return rc;
            }
            // Set the request code
            result = parseRequestCode(value);
            if (result < 0) {
                // If the request code is invalid, return an error
                // Move the pointer back to the start of the value
                *json -= strlen(value) + 1;
                return PARSE_ERROR_INVALID_REQUEST;
            }
            current_shvblock->shvcode = result;

        } else if (strcmp(attribute, "result") == 0) {
            // Check for duplicate
            if (has_result) {
                // Move the pointer back to the start of the attribute
                *json -= strlen(attribute) + 1;
                return PARSE_ERROR_DUPLICATE_RESULT;
            }
            has_result = 1;
            // Skip to the value string
            rc = json_skip_colon(json);
            if (rc != PARSE_ERROR_OK) {
                return rc;
            }
            rc = json_skip_string(json, &value);
            if (rc != PARSE_ERROR_OK) {
                return rc;
            }
            // Set the result code
            result = parseResultCode(value);
            if (result < 0) {
                // Move the pointer back to the start of the value
                *json -= strlen(value) + 1;
                return PARSE_ERROR_INVALID_RESULT;
            }
            current_shvblock->shvret = result;

        } else if (strcmp(attribute, "value") == 0) {
            // Check for duplicate
            if (has_value) {
                // Move the pointer back to the start of the attribute
                *json -= strlen(attribute) + 1;
                return PARSE_ERROR_DUPLICATE_VALUE;
            }
            has_value = 1;

            // Skip to the value
            rc = json_skip_colon(json);
            if (rc != PARSE_ERROR_OK) {
                return rc;
            }
            json_skip_whitespace(json);

            // Parse the object
            rc = parseValue(json, &(current_shvblock->shvobject));
            if (rc != PARSE_ERROR_OK) {
                return rc;
            }
        }

        else {
            // Move the pointer back to the start of the attribute
            *json -= strlen(attribute) + 1;
            return PARSE_ERROR_INVALID_ATTRIBUTE;
        }

        // Skip to the next attribute or the end of the block
        json_skip_whitespace(json);
        if (**json != '}') { // If not the end of the members
            // If the next character is a comma, skip it
            if (**json == ',') (*json)++;
            else return PARSE_ERROR_EXPECTED_COMMA_OR_CLOSE_CURLY;
        }
        // Skip any whitespace
        json_skip_whitespace(json);
    }

    if (has_name == 0) {
        return PARSE_ERROR_MISSING_NAME;
    }

    // Skip the end of the object
    (*json)++;

    return PARSE_ERROR_OK;
}

// Function to parse request property
long parseRequestCode(char* request_value) {
    // Decode request_value string to numeric value
    // where request_value is one of "set", "fetch", "drop", "nextv", "priv", "syset", "syfet", "sydro", "sydel"

    /* Compare buffer with each request code and set the result according to the corresponding crexxsaa.h code */
    if (strcasecmp(request_value, "set") == 0) {
        return RXSHV_SET;
    }
    else if (strcasecmp(request_value, "fetch") == 0) {
        return RXSHV_FETCH;
    }
    else if (strcasecmp(request_value, "drop") == 0) {
        return RXSHV_DROP;
    }
    else if (strcasecmp(request_value, "nextv") == 0) {
        return RXSHV_NEXTV;
    }
    else if (strcasecmp(request_value, "priv") == 0) {
        return RXSHV_PRIV;
    }
    else if (strcasecmp(request_value, "syset") == 0) {
        return RXSHV_SYSET;
    }
    else if (strcasecmp(request_value, "syfet") == 0) {
        return RXSHV_SYFET;
    }
    else if (strcasecmp(request_value, "sydro") == 0) {
        return RXSHV_SYDRO;
    }
    else if (strcasecmp(request_value, "sydel") == 0) {
        return RXSHV_SYDEL;
    }

    // If the request value is not one of the above, return an error
    return -1;
}

// Function to parse result property
long parseResultCode(char* result_value) {
    // Decode result_value string to numeric value
    // where result_value is one of "ok", "newv", "lvar", "trunc", "badn", "memfl", "badf", "noavl", "notex"

    /* Compare buffer with each result code and set the result according to the corresponding crexxsaa.h code */
    if (strcasecmp(result_value, "ok") == 0) {
        return RXSHV_OK;
    }
    else if (strcasecmp(result_value, "newv") == 0) {
        return RXSHV_NEWV;
    }
    else if (strcasecmp(result_value, "lvar") == 0) {
        return RXSHV_LVAR;
    }
    else if (strcasecmp(result_value, "trunc") == 0) {
        return RXSHV_TRUNC;
    }
    else if (strcasecmp(result_value, "badn") == 0) {
        return RXSHV_BADN;
    }
    else if (strcasecmp(result_value, "memfl") == 0) {
        return RXSHV_MEMFL;
    }
    else if (strcasecmp(result_value, "badf") == 0) {
        return RXSHV_BADF;
    }
    else if (strcasecmp(result_value, "noavl") == 0) {
        return RXSHV_NOAVL;
    }
    else if (strcasecmp(result_value, "notex") == 0) {
        return RXSHV_NOTEX;
    }

    // If the result value is not one of the above, return an error
    return -1;
}

// Function to parse an object, e.g.
// {
//        "class": "complex_number",
//        "members": {
//            "<member_name>": <member value>,
//            ...
//        }
// }
// the object is parsed and the SHVBLOCK shvobject is populated
int parseObject(char** json, OBJBLOCK* shvobject) { // NOLINT(misc-no-recursion) - suppress the clang-tidy warning about recursion
    int rc;

    // Set initial values
    shvobject->type = VALUE_OBJECT;
    shvobject->typename = NULL;
    shvobject->value.members = NULL;

    // Loop around the object attributes
    char *attribute;
    char *value;
    char has_class = 0;
    char has_members = 0;
    while (**json != '}') {
        // Skip to the attribute string
        rc = json_skip_string(json, &attribute);
        if (rc != PARSE_ERROR_OK) {
            return rc;
        }

        // Compare the attribute and set the corresponding SHVBLOCK value
        if (strcmp(attribute, "class") == 0) {
            // Check for duplicate
            if (has_class) {
                // Move the pointer back to the start of the attribute
                *json -= strlen(attribute) + 1;
                return PARSE_ERROR_DUPLICATE_CLASS;
            }
            has_class = 1;
            // Skip to the value string
            rc = json_skip_colon(json);
            if (rc != PARSE_ERROR_OK) {
                return rc;
            }
            rc = json_skip_string(json, &value);
            if (rc != PARSE_ERROR_OK) {
                return rc;
            }
            // Set the class
            shvobject->typename = value;

        } else if (strcmp(attribute, "members") == 0) {
            // Check for duplicate
            if (has_members) {
                // Move the pointer back to the start of the attribute
                *json -= strlen(attribute) + 1;
                return PARSE_ERROR_DUPLICATE_MEMBERS;
            }
            has_members = 1;

            // Skip to the values
            rc = json_skip_colon(json);
            if (rc != PARSE_ERROR_OK) {
                return rc;
            }
            json_skip_whitespace(json);

            // Skip to the member start {
            rc = json_next_object(json);
            if (rc != PARSE_ERROR_OK) {
                return rc;
            }

            // Process the members
            MEMBLOCK *prev_object = NULL;
            MEMBLOCK *object;
            while (**json != '}') {

                // Create the MEMBLOCK
                object = (MEMBLOCK*)malloc(sizeof(MEMBLOCK));
                if (!object) {
                    return PARSE_ERROR_MEMORY_ALLOCATION;
                }

                // Set the head of the linked list
                if (!shvobject->value.members) shvobject->value.members = object;
                // Set the previous member's next pointer
                if (prev_object) {
                    prev_object->membernext = object;
                }
                prev_object = object;
                object->membernext = NULL;

                // Parse the member name
                rc = json_skip_string(json, &(object->membername));
                if (rc != PARSE_ERROR_OK) {
                    return rc;
                }

                // Skip to the :
                rc = json_skip_colon(json);
                if (rc != PARSE_ERROR_OK) {
                    return rc;
                }

                // Parse the member value
                rc = parseValue(json, &(object->memberobject));
                if (rc != PARSE_ERROR_OK) {
                    return rc;
                }

                // Skip to the next member or the end of the members
                json_skip_whitespace(json);
                if (**json != '}') { // If not the end of the members
                    // If the next character is a comma, skip it
                    if (**json == ',') (*json)++;
                    else return PARSE_ERROR_EXPECTED_COMMA_OR_CLOSE_CURLY;
                }
                // Skip any whitespace
                json_skip_whitespace(json);
            }
            // Skip the end of the members
            (*json)++;

        } else {
            // Move the pointer back to the start of the attribute
            *json -= strlen(attribute) + 1;
            return PARSE_ERROR_INVALID_ATTRIBUTE;
        }
        // Skip to the next attribute or the end of the block
        json_skip_whitespace(json);
        if (**json != '}') { // If not the end of the object
            // If the next character is a comma, skip it
            if (**json == ',') (*json)++;
            else return PARSE_ERROR_EXPECTED_COMMA_OR_CLOSE_CURLY;
        }
        // Skip any whitespace
        json_skip_whitespace(json);
    }

    // Check for required attributes
    if (!has_class) {
        return PARSE_ERROR_MISSING_CLASS;
    }
    if (!has_members) {
        return PARSE_ERROR_MISSING_MEMBERS;
    }

    // Skip the end of the object
    (*json)++;

    return PARSE_ERROR_OK;
}

// Function to parse a value
int parseValue(char** json, OBJBLOCK** shvobject_handle) { // NOLINT(misc-no-recursion) - suppress the clang-tidy warning about recursion
    // Parse JSON Body using the json functions, example format:
    //
    // Note The value element can be a
    // - `string` ("string value")
    // - `number` (1234.56 - int or float)
    // - `boolean` (true/false)
    // - `binary` (base64 encoded - e.g. { "base64": "c3RyaW5nIGJpbmFyeSBkYXRh" }
    // - `object` (I.e. an object member), e.g.
    //   {
    //        "class": "complex_number",
    //        "members": {
    //          "real": 3.2,
    //          "imaginary":  4.5
    //        }
    //    }
    // - `array` (an array of value types [value1, value2, ...])
    // - `null` (null for an empty value, although this use case is TBD)

    int rc;

    // Create a new OBJBLOCK
    *shvobject_handle = (OBJBLOCK*)malloc(sizeof(OBJBLOCK));
    if (!(*shvobject_handle)) {
        return PARSE_ERROR_MEMORY_ALLOCATION;
    }
    (*shvobject_handle)->type = VALUE_NULL;
    (*shvobject_handle)->typename = NULL;
    (*shvobject_handle)->value.string = NULL;

    // Work out what kind of object this is
    json_skip_whitespace(json);
    if (**json == '"') {
        // String
        rc = json_skip_string(json, &((*shvobject_handle)->value.string));
        if (rc != PARSE_ERROR_OK) {
            return rc;
        }
        (*shvobject_handle)->type = VALUE_STRING;
    }

    else if (**json == '-' || **json == '.' || isdigit(**json)) {
        // Number
        VALUETYPE type;
        long integer;
        double real;
        // Parse the number
        rc = json_skip_number(json, &type, &integer, &real);
        if (rc != PARSE_ERROR_OK) {
            return rc;
        }
        (*shvobject_handle)->type = type;
        if (type == VALUE_INT) {
            (*shvobject_handle)->value.integer = integer;
        }
        else {
            (*shvobject_handle)->value.real = real;
        }
    }

    else if (**json == '{') {
        // Object or Binary
        if (json_peek_binary(*json)) {
            // Binary
            rc = json_skip_binary(json, &((*shvobject_handle)->value.binary.data), &((*shvobject_handle)->value.binary.length));
            if (rc != PARSE_ERROR_OK) {
                return rc;
            }
            (*shvobject_handle)->type = VALUE_BINARY;
        }
        else {
            // Object
            (*shvobject_handle)->type = VALUE_OBJECT;
            (*shvobject_handle)->value.members = NULL;
            // Skip the {
            (*json)++;

            // Parse the object
            rc = parseObject(json, *shvobject_handle);
            if (rc != PARSE_ERROR_OK) {
                return rc;
            }
        }
    }

    else if (**json == '[') {
        // Array
        MEMBLOCK *prev_array_member = NULL;
        MEMBLOCK *array_member;
        (*shvobject_handle)->type = VALUE_ARRAY;
        (*shvobject_handle)->value.members = NULL;
        // Skip the [
        (*json)++;
        // Loop through the array
        while (**json != ']') {
            array_member = (MEMBLOCK*)malloc(sizeof(MEMBLOCK));
            if (!array_member) {
                return PARSE_ERROR_MEMORY_ALLOCATION;
            }

            // Set the head of the linked list
            if (!(*shvobject_handle)->value.members) (*shvobject_handle)->value.members = array_member;
            // Set the previous member's next pointer
            if (prev_array_member) {
                prev_array_member->membernext = array_member;
            }
            prev_array_member = array_member;
            array_member->membernext = NULL;

            // Skip to the member start {
            rc = json_next_object(json);
            if (rc != PARSE_ERROR_OK) {
                return rc;
            }

            // Parse the member name
            rc = json_skip_string(json, &(array_member->membername));
            if (rc != PARSE_ERROR_OK) {
                return rc;
            }

            // Skip to the :
            rc = json_skip_colon(json);
            if (rc != PARSE_ERROR_OK) {
                return rc;
            }

            // Parse the array member value
            rc = parseValue(json, &(array_member->memberobject));
            if (rc != PARSE_ERROR_OK) {
                return rc;
            }

            // Skip to the end of the object
            json_skip_whitespace(json);
            if (**json != '}') {
                return PARSE_ERROR_EXPECTING_CLOSE_CURLY;
            }
            (*json)++;

            // Skip to the next object or the end of the array
            json_skip_whitespace(json);
            if (**json != ']') { // If not the end of the array
                // If the next character is a comma, skip it
                if (**json == ',') (*json)++;
                else return PARSE_ERROR_EXPECTED_COMMA_OR_CLOSE_ARRAY;
            }
            // Skip any whitespace
            json_skip_whitespace(json);
        }
        // Skip the ]
        (*json)++;
    }

    else if (**json == 'n') {
        // Possibly Null - so check for null
        rc = json_skip_null(json);
        if (rc != PARSE_ERROR_OK) {
            return rc;
        }
        (*shvobject_handle)->type = VALUE_NULL;
    }

    else if (**json == 't') {
        // Possibly True - so check for true
        rc = json_skip_true(json);
        if (rc != PARSE_ERROR_OK) {
            return rc;
        }
        (*shvobject_handle)->type = VALUE_BOOL;
        (*shvobject_handle)->value.boolean = 1;
    }

    else if (**json == 'f') {
        // Possibly False - so check for false
        rc = json_skip_false(json);
        if (rc != PARSE_ERROR_OK) {
            return rc;
        }
        (*shvobject_handle)->type = VALUE_BOOL;
        (*shvobject_handle)->value.boolean = 0;
    }

    else {
        return PARSE_ERROR_INVALID_TYPE;
    }

    return PARSE_ERROR_OK;
}

// Function to skip whitespace
int json_skip_whitespace(char **json) {
    while (**json && (**json == ' ' || **json == '\t' || **json == '\n' || **json == '\r')) (*json)++;
    return PARSE_ERROR_OK;
}

// Function to move the char position to the character after the next "{"
int json_next_object(char **json) {
    while (**json) {
        if (**json == '{') {
            (*json)++;
            return PARSE_ERROR_OK;
        }
        if (**json == ' ' || **json == '\t' || **json == '\n' || **json == '\r') (*json)++;
        else return PARSE_ERROR_EXPECTING_OPEN_CURLY; // Invalid character
    }
    return PARSE_ERROR_EXPECTING_OPEN_CURLY;
}

// Function to skip whitespace and "}"
int json_skip_block(char **json) {
    json_skip_whitespace(json);
    if (**json == '}') {
        (*json)++;
        return PARSE_ERROR_OK;
    }
    return PARSE_ERROR_EXPECTING_CLOSE_CURLY;
}

// Function to skip whitespace and ":"
int json_skip_colon(char **json) {
    json_skip_whitespace(json);
    if (**json == ':') {
        (*json)++;
        return PARSE_ERROR_OK;
    }
    return PARSE_ERROR_EXPECTING_COLON;
}

// Function to skip a string (and update the string parameter to point to the
// start of the string which is null terminated)
int json_skip_string(char **json, char **string) {
    json_skip_whitespace(json);
    if (*(*json) != '"') return PARSE_ERROR_EXPECTING_STRING; // Not a string

    (*json)++;
    *string = (*json);
    while (**json){
        if (**json == '"') {
            // Need to check for escaped quotes
            if (*((*json) - 1) != '\\') break;
        }
        (*json)++;
    }
    if (**json != '"') return PARSE_ERROR_NO_CLOSING_QUOTE; // No closing quote
    **json = '\0'; // Null terminate the string
    (*json)++;

    // Process the JSON Escape characters in the string. One exception is \u0000 which is conversed to "\0"
    if (json_escape_string(*string) != 0) {
        // Move the pointer back to the start of the string
        *json = *string - 1;
        return PARSE_ERROR_INVALID_STRING;
    }

    return PARSE_ERROR_OK;
}

// Function to peak ahead for a specific string - returns null if not found or the start of the position after the string
// Note that this does not support escaped characters - particularly \"
char *json_peak_string(char *json, char *string) {
    json_skip_whitespace(&json);
    if (*json != '"') return NULL;
    json++;
    while (*json && *json != '"') {
        if (*json != *string) return NULL;
        json++;
        string++;
    }
    if (*json != '"') return NULL; // No closing quote
    return json + 1;
}

// Function to Escape characters in the string using JSON standards.
// One exception is \u0000 which is conversed to "\0"
// The string is updated in place
// Returns 0 if the string is valid, otherwise -1 if the string is invalid
int json_escape_string(char *string) {
    char *json_read = string;
    char *json_write = string;
    while (json_read[0]) {
        if (json_read[0] == '\\') {
            if (json_read[1] == '\\') {
                json_write[0] = '\\';
                json_write++;
                json_read += 2;
            }
            else if (json_read[1] == '"') {
                json_write[0] = '\"';
                json_write++;
                json_read += 2;
            }
            else if (json_read[1] == 'n') {
                json_write[0] = '\n';
                json_write++;
                json_read += 2;
            }
            else if (json_read[1] == 'r') {
                json_write[0] = '\r';
                json_write++;
                json_read += 2;
            }
            else if (json_read[1] == 't') {
                json_write[0] = '\t';
                json_write++;
                json_read += 2;
            }
            else if (json_read[1] == 'b') {
                json_write[0] = '\b';
                json_write++;
                json_read += 2;
            }
            else if (json_read[1] == 'f') {
                json_write[0] = '\f';
                json_write++;
                json_read += 2;
            }
            else if (json_read[1] == '/') {
                json_write[0] = '/';
                json_write++;
                json_read += 2;
            }
            else if (json_read[1] == 'u') {
                // Calculate the unicode value
                json_read++; // Skip the '\'
                int unicode = 0;
                int i;
                for (i = 0; i < 4; i++) {
                    json_read++;
                    if (*json_read >= '0' && *json_read <= '9') {
                        unicode = unicode * 16 + (*json_read - '0');
                    } else if (*json_read >= 'a' && *json_read <= 'f') {
                        unicode = unicode * 16 + (*json_read - 'a' + 10);
                    } else if (*json_read >= 'A' && *json_read <= 'F') {
                        unicode = unicode * 16 + (*json_read - 'A' + 10);
                    } else {
                        return -1;
                    }
                }
                // If unicode is 0, then it is a null character and should be converted to "\0"
                // Not ideal however it is the only way to represent a null character in a null terminated string
                if (unicode == 0) {
                    json_write[0] = '\\';
                    json_write[1] = '0';
                    json_write += 2;
                }
                else {
                    // Convert the unicode to utf-8
                    if (unicode < 0x80) {
                        json_write[0] = (char)unicode;
                        json_write++;
                    } else if (unicode < 0x800) {
                        json_write[0] = (char)(0xC0 | (unicode >> 6));
                        json_write[1] = (char)(0x80 | (unicode & 0x3F));
                        json_write += 2;
                    } else {
                        json_write[0] = (char)(0xE0 | (unicode >> 12));
                        json_write[1] = (char)(0x80 | ((unicode >> 6) & 0x3F));
                        json_write[2] = (char)(0x80 | (unicode & 0x3F));
                        json_write += 3;
                    }
                }
                json_read++; // Skip the last digit
            }
            else {
                return -1;
            }
        }
        else {
            json_write[0] = json_read[0];
            json_read++;
            json_write++;
        }
    }
    *json_write = '\0';
    return 0;
}

// Function to skip a null token
int json_skip_null(char **json) {
    // Skip the null token
    if (strncmp(*json, "null", 4) != 0) {
        return PARSE_ERROR_UNEXPECTED_TOKEN; // Not a null token
    }
    (*json) += 4;

    // Check the next character
    if (**json == ' ' || **json == '\t' || **json == '\n' || **json == '\r' ||
        **json == ']' || **json == '}' || **json == ',' || **json == '\0') {
        return PARSE_ERROR_OK;
    }
    // Move the pointer back to the start of the null token
    (*json) -= 4;

    return PARSE_ERROR_UNEXPECTED_TOKEN; // Invalid character after null token
}

// Function to skip a true token
int json_skip_true(char **json) {
    // Skip the true token
    if (strncmp(*json, "true", 4) != 0) {
        return PARSE_ERROR_UNEXPECTED_TOKEN; // Not a true token
    }
    (*json) += 4;

    // Check the next character
    if (**json == ' ' || **json == '\t' || **json == '\n' || **json == '\r' ||
        **json == ']' || **json == '}' || **json == ',' || **json == '\0') {
        return PARSE_ERROR_OK;
    }

    return PARSE_ERROR_UNEXPECTED_TOKEN; // Invalid character after true token
}

// Function to skip a false token
int json_skip_false(char **json) {
    // Skip the false token
    if (strncmp(*json, "false", 5) != 0) {
        return PARSE_ERROR_UNEXPECTED_TOKEN; // Not a false token
    }
    (*json) += 5;

    // Check the next character
    if (**json == ' ' || **json == '\t' || **json == '\n' || **json == '\r' ||
        **json == ']' || **json == '}' || **json == ',' || **json == '\0') {
        return PARSE_ERROR_OK;
    }

    return PARSE_ERROR_UNEXPECTED_TOKEN; // Invalid character after false token
}

// Function to parse and skip a number
int json_skip_number(char **json, VALUETYPE *type_handle, long *integer_handle, double *real_handle) {
    // Parse and skip a number (integer or float) setting the type_handle to the type
    // and the integer_handle or real_handle to the value
    // Return the position after the number (or NULL if the number is invalid)

    char is_negative = 0;
    *integer_handle = 0;
    *type_handle = VALUE_INT;
    char *value = *json;  // Remember the start of the value

    // Parse the number
    // Leading - sign
    if (**json == '-') {
        (*json)++;
        is_negative = 1;
    }
    // Leading 0s
    while (**json == '0') {
        (*json)++;
    }
    // Digits
    while (**json >= '0' && **json <= '9') {
        *integer_handle = *integer_handle * 10 + (**json - '0');
        (*json)++;
    }
    // Decimal point
    if (**json == '.') {
        (*json)++;
        *type_handle = VALUE_FLOAT;
        while (**json >= '0' && **json <= '9') {
            (*json)++;
        }
    }
    // Exponent
    if (**json == 'e' || **json == 'E') {
        (*json)++;
        *type_handle = VALUE_FLOAT;
        if (**json == '-' || **json == '+') {
            (*json)++;
        }
        while (**json >= '0' && **json <= '9') {
            (*json)++;
        }
    }

    if (value == *json) {
        return PARSE_ERROR_EXPECTED_NUMBER; // No number found
    }

    if (*type_handle == VALUE_INT) {
        *real_handle = 0;
        if (is_negative) *integer_handle = -*integer_handle;
    }
    else {
        *integer_handle = 0;
        // Use strtod() for conversion as it handles edge cases & rounding better than the above code would
        // The above code has checked that the format is valid, so this should not fail
        *real_handle = strtod(value, NULL);
    }

    return PARSE_ERROR_OK;
}

// Function to peek if the next object might be a binary object
int json_peek_binary(char *json) {
    // We check for this format - only checking that the "base64" is present
    // {
    //  "base64": "c3RyaW5nIGJpbmFyeSBkYXRh"
    // }
    // Return 1 if it is a binary object, 0 if it is not (any errors will be picked up by the main parser)

    int rc;

    // Skip to the start of the object
    rc = json_next_object(&json);
    if (rc != PARSE_ERROR_OK) {
        return 0;
    }

    // Skip to the base64 attribute
    json = json_peak_string(json, "base64");
    if (!json) {
        return 0;
    }

    return 1; // It "might" be a binary object
}

// Function to skip/parse a binary object
int json_skip_binary(char **json, char **binary_handle, size_t *length_handle) {
    // Parse the binary object and return the base64 string and length
    // Return the position after the object (or NULL if the object is invalid)

    char *attribute;
    char *value;
    *binary_handle = NULL;
    *length_handle = 0;
    int rc;
    int len;

    // Skip to the start of the object
    rc = json_next_object(json);
    if (rc != PARSE_ERROR_OK) {
        return rc;
    }

    // Skip to the base64 attribute
    rc = json_skip_string(json, &attribute);
    if (rc != PARSE_ERROR_OK) {
        return rc;
    }

    // Note this error is never returned as it should have been is checked in the peek function
    if (strcmp(attribute, "base64") != 0) {
        return PARSE_ERROR_EXPECTING_BASE64;
    }

    // Skip to the value string
    rc = json_skip_colon(json);
    if (rc != PARSE_ERROR_OK) {
        return rc;
    }
    rc = json_skip_string(json, &value);
    if (rc != PARSE_ERROR_OK) {
        return rc;
    }

    // Decode the base64 string into binary, in situ in the base64 string as it will be shorter, setting the *length_handle
    // to the length of the binary data
    *binary_handle = value;
    len = base64_decode(value);
    if (len < 0) { // base64_decode returns -1 if there is an error
        // Move the pointer back to the start of the value
        *json -= strlen(value) + 1;
        return PARSE_ERROR_INVALID_BASE64;
    }
    *length_handle = (size_t)len;

    // Skip to the end of the object
    rc = json_skip_block(json);
    if (rc != PARSE_ERROR_OK) {
        return rc;
    }

    return PARSE_ERROR_OK;
}

// Base64 decode function - decoding done in situ in the buffer, and returns the length of the decoded data
// Return -1 on decoding error
int base64_decode(char *buffer) {

    static const char base64_table[65] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    // This initializes the base64 decoding table - this could be moved outside the function for efficiency but
    // it is only done once per call to the function so it is not a big overhead
    int i;
    int dtable[256];
    // Initialize the table
    // Make each entry -1 to indicate that it is not a valid base64 character
    for (i = 0; i < 256; i++)
        dtable[i] = -1;
    // Set the valid base64 characters
    for (i = 0; i < 64; i++)
        dtable[(uint8_t) base64_table[i]] = (uint8_t) i;
    // Set the padding character
    dtable[(uint8_t) '='] = 0;

    char *src = buffer;
    char *dst = buffer;

    int output_length = 0;

    while (*src && src[1]) {
        if (dtable[(uint8_t) src[0]] == -1 || dtable[(uint8_t) src[1]] == -1) {
            return -1; // Invalid base64 character
        }
        uint8_t a = dtable[(uint8_t) *src++];
        uint8_t b = dtable[(uint8_t) *src++];
        *dst++ = (char)((a << 2) | (b >> 4));
        output_length++;

        if (*src && *src != '=') {
            if (dtable[(uint8_t) *src] == -1) {
                return -1; // Invalid base64 character
            }
            uint8_t c = dtable[(uint8_t) *src++];
            *dst++ = (char)(((b & 0xf) << 4) | (c >> 2));
            output_length++;

            if (*src && *src != '=') {
                if (dtable[(uint8_t) *src] == -1) {
                    return -1; // Invalid base64 character
                }
                uint8_t d = dtable[(uint8_t) *src++];
                *dst++ = (char)(((c & 0x3) << 6) | d);
                output_length++;
            }
        }
    }

    return output_length;
}
