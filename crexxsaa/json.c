// Simple JSON Parser - implemented as a single function to parse the JSON body and populate the SHVBLOCK linked list
// The JSON format is defined in crexxjson.md and the c buffer is defined in crexxsaa.h */
// The library has been implemented in a single file for simplicity and to avoid external dependencies
//
// Created by and copyright (c) 2024 Adrian Sutherland

#include "httpclient.h"
#include "crexxsaa.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

/* Function prototypes */
int parseJSON(char* json, SHVBLOCK** shvblock_handle); // Parse the JSON body
char*  parseShvblock(char* json, SHVBLOCK* current_shvblock); // Parse individual service block
long parseRequestCode(char* request_value); // Parse the request code (returns long so a negative value can be used for an error)
long parseResultCode(char* request_value); // Parse the result code (returns long so a negative value can be used for an error)
char *parseObject(char* json, OBJBLOCK* shvobject); // Parse value attribute
char *parseValue(char* json, OBJBLOCK** shvobject_handle); // Parse object

char *json_skip_whitespace(char *json);
char *json_next_object(char *json);
char *json_skip_block(char *json);
char *json_skip_colon(char *json);
char *json_skip_string(char *json, char **string);
char *json_skip_null(char *json);
char *json_skip_true(char *json);
char *json_skip_false(char *json);
char *json_skip_number(char *json, VALUETYPE *type_handle, long *integer_handle, double *real_handle); // Function to parse and skip a number
int json_peek_binary(char *json); // Function to peek if the next object might be a binary object
char *json_skip_binary(char *json, char **binary_handle, size_t *length_handle); // Function to skip a binary object
int base64_decode(char *buffer); // Base64 decode function - decoding done in situ in the buffer, and returns the length of the decoded data
void FreeMemBlock(MEMBLOCK *memblock); // Free the MEMBLOCK, and linked list of members
void FreeObjBlock(OBJBLOCK *shvobject); // Free the MEMBLOCK

/* Free a MEMBLOCK linked list */
void FreeMemBlock(MEMBLOCK *memblock) {
    MEMBLOCK *memblock_next;

    while (memblock) {
        memblock_next = memblock->membernext;
        FreeObjBlock(memblock->memberobject);  // Recursively free the members
        free(memblock); // Free the object
        memblock = memblock_next;
    }
}

/* Free the OBJBLOCK */
void FreeObjBlock(OBJBLOCK *shvobject) {
    if (shvobject->type == VALUE_OBJECT || shvobject->type == VALUE_ARRAY) {
        FreeMemBlock(shvobject->value.members); // Free the members
    }
    free(shvobject); // Free the object
}

/* Create a SHVBLOCK */
SHVBLOCK* CreateShvBlock(SHVBLOCK* previous, char* JsonBuffer) {
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
            return 0;
        }
        shvbuffer->json = JsonBuffer;
        shvblock = &(shvbuffer->shvblock); // Set the SHVBLOCK pointer to the SHVBLOCK in the SHVBUFFER (SHVBLOCK is
                                           // the first element in the SHVBUFFER and has the same address)
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
void FreeRexxVariablePool(SHVBLOCK *shvblock) {
    SHVBLOCK *shvblock_next;
    SHVBUFFER *shvbuffer;

    /* Free the start SHVBLOCK which is a SHVBUFFER */
    shvbuffer = (SHVBUFFER*)shvblock;
    FreeObjBlock(shvbuffer->shvblock.shvobject); // Free the OBJBLOCK
    shvblock = shvbuffer->shvblock.shvnext;
    free(shvbuffer->json); // Free the JSON buffer
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
int parseJSON(char* start_json, SHVBLOCK** shvblock_handle) {
    char *name;
    char* json = start_json; // Pointer to the current position in the JSON buffer
    SHVBLOCK* current_shvblock = 0; // This points to the current SHVBLOCK being populated
    *shvblock_handle = 0; // This points to the fist SHVBLOCK in the linked list (or NULL if there are no SHVBLOCKS yet)

    // Skip to the start of the object
    json = json_next_object(json);
    if (!json) {
        fprintf(stderr, "Invalid JSON format - no start of object\n");
        return 1;
    }

    // Skip to the serviceBlocks string
    json = json_skip_string(json, &name);
    if (!json) {
        fprintf(stderr, "Invalid JSON format - no \"serviceBlocks\"\n");
        return 1;
    }
    if (strcmp(name, "serviceBlocks") != 0) {
        fprintf(stderr, "Invalid JSON format - no \"serviceBlocks\", found \"%s\"\n", name);
        return 1;
    }

    // Skip to the array
    json = json_skip_colon(json);
    json = json_skip_whitespace(json);
    if (*json != '[') {
        fprintf(stderr, "Invalid JSON format - no start of array\n");
        return 1;
    }
    json++;

    // Loop through the array
    while (*json != ']') {
        // Skip to the start of the object
        json = json_next_object(json);
        if (!json) {
            fprintf(stderr, "Invalid JSON format - no start of object\n");
            return 1;
        }

        // Create the SHVBLOCK
        current_shvblock = CreateShvBlock(current_shvblock, start_json);
        if (!current_shvblock) {
            fprintf(stderr, "Failed to create SHVBLOCK\n");
            return 1;
        }
        if (*shvblock_handle == NULL) *shvblock_handle = current_shvblock; // Update the head shvblock pointer if this is the first SHVBLOCK

        // Parse individual service block
        json = parseShvblock(json, current_shvblock);
        if (!json) {
            fprintf(stderr, "Failed to parse service block\n");
            return 1;
        }

        // Skip to the next object or the end of the array
        json = json_skip_whitespace(json);
        // If the next character is a comma, skip it
        if (*json == ',') json++;
        // Skip any whitespace
        json = json_skip_whitespace(json);
    }
    return 0;
}

// Function to parse individual service block
char* parseShvblock(char* json, SHVBLOCK* current_shvblock) {
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

    while (*json != '}') {
        // Skip to the attribute string
        json = json_skip_string(json, &attribute);
        if (!json) {
            fprintf(stderr, "Invalid JSON format - no attribute\n");
            return NULL;
        }

        // Compare the attribute and set the corresponding SHVBLOCK value
        if (strcmp(attribute, "name") == 0) {
            // Skip to the value string
            json = json_skip_colon(json);
            json = json_skip_string(json, &value);
            if (!json) {
                fprintf(stderr, "Invalid JSON format - no name value\n");
                return NULL;
            }
            // Check for duplicate
            if (has_name) {
                fprintf(stderr, "Duplicate name attribute\n");
                return NULL;
            }
            has_name = 1;
            // Set the name
            current_shvblock->shvname = value;

        } else if (strcmp(attribute, "request") == 0) {
            // Skip to the value string
            json = json_skip_colon(json);
            json = json_skip_string(json, &value);
            if (!json) {
                fprintf(stderr, "Invalid JSON format - no request value\n");
                return NULL;
            }
            // Check for duplicate
            if (has_request) {
                fprintf(stderr, "Duplicate request attribute\n");
                return NULL;
            }
            has_request = 1;

            // Set the request code
            result = parseRequestCode(value);
            if (result < 0) {
                fprintf(stderr, "Invalid request value\n");
                return NULL;
            }
            current_shvblock->shvcode = result;

        } else if (strcmp(attribute, "result") == 0) {
            // Skip to the value string
            json = json_skip_colon(json);
            json = json_skip_string(json, &value);
            if (!json) {
                fprintf(stderr, "Invalid JSON format - no result value\n");
                return NULL;
            }
            // Check for duplicate
            if (has_result) {
                fprintf(stderr, "Duplicate result attribute\n");
                return NULL;
            }
            has_result = 1;

            // Set the result code
            result = parseResultCode(value);
            if (result < 0) {
                fprintf(stderr, "Invalid result value\n");
                return NULL;
            }
            current_shvblock->shvret = result;

        } else if (strcmp(attribute, "value") == 0) {

            // Check for duplicate
            if (has_value) {
                fprintf(stderr, "Duplicate value attribute\n");
                return NULL;
            }
            has_value = 1;

            // Skip to the value
            json = json_skip_colon(json);
            json = json_skip_whitespace(json);

            // Parse the object
            json = parseValue(json, &(current_shvblock->shvobject));
            if (!json) {
                fprintf(stderr, "Failed to parse object\n");
                return NULL;
            }
        }

        else {
            fprintf(stderr, "Invalid attribute\n");
            return NULL;
        }

        // Skip to the next attribute or the end of the block
        json = json_skip_whitespace(json);
        // If the next character is a comma, skip it
        if (*json == ',') json++;
        // Skip any whitespace
        json = json_skip_whitespace(json);
    }

    if (has_name == 0) {
        fprintf(stderr, "Missing required attribute (name)\n");
        return NULL;
    }

    // Skip the end of the object
    json++;

    return json;
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
char *parseObject(char* json, OBJBLOCK* shvobject) {

    // Set initial values
    shvobject->type = VALUE_OBJECT;
    shvobject->typename = NULL;
    shvobject->value.members = NULL;

    // Loop around the object attributes
    char *attribute;
    char *value;
    char has_class = 0;
    char has_members = 0;
    while (*json != '}') {
        // Skip to the attribute string
        json = json_skip_string(json, &attribute);
        if (!json) {
            fprintf(stderr, "Invalid JSON format - no attribute\n");
            return NULL;
        }

        // Compare the attribute and set the corresponding SHVBLOCK value
        if (strcmp(attribute, "class") == 0) {
            // Skip to the value string
            json = json_skip_colon(json);
            json = json_skip_string(json, &value);
            if (!json) {
                fprintf(stderr, "Invalid JSON format - no class value\n");
                return NULL;
            }
            // Check for duplicate
            if (has_class) {
                fprintf(stderr, "Duplicate class attribute\n");
                return NULL;
            }
            has_class = 1;
            // Set the class
            shvobject->typename = value;
        } else if (strcmp(attribute, "members") == 0) {
            // Check for duplicate
            if (has_members) {
                fprintf(stderr, "Duplicate members attribute\n");
                return NULL;
            }
            has_members = 1;

            // Skip to the values
            json = json_skip_colon(json);
            if (!json) {
                fprintf(stderr, "Invalid JSON format - no colon after members\n");
                return NULL;
            }
            json = json_skip_whitespace(json);

            // Skip to the member start {
            json = json_next_object(json);
            if (!json) {
                fprintf(stderr, "Invalid JSON format - no start of object in members\n");
                return NULL;
            }

            // Process the members
            MEMBLOCK *prev_object = NULL;
            MEMBLOCK *object;
            while (*json != '}') {
                // Skip to the start of the object
                json = json_next_object(json);
                if (!json) {
                    fprintf(stderr, "Invalid JSON format - no start of object in members\n");
                    return NULL;
                }

                // Create the MEMBLOCK
                object = (MEMBLOCK*)malloc(sizeof(MEMBLOCK));
                if (!object) {
                    fprintf(stderr, "Memory allocation error\n");
                    return NULL;
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
                json = json_skip_string(json, &(object->membername));
                if (!json) {
                    fprintf(stderr, "Invalid JSON format - no member name in members\n");
                    return NULL;
                }

                // Skip to the :
                json = json_skip_colon(json);
                if (!json) {
                    fprintf(stderr, "Invalid JSON format - no colon after member name in members\n");
                    return NULL;
                }

                // Parse the member value
                json = parseValue(json, &(object->memberobject));
                if (!json) {
                    fprintf(stderr, "Failed to parse object in members\n");
                    return NULL;
                }


                // Skip to the next object or the end of the object
                json = json_skip_whitespace(json);
                // If the next character is a comma, skip it
                if (*json == ',') json++;
                // Skip any whitespace
                json = json_skip_whitespace(json);
            }

        } else {
            fprintf(stderr, "Invalid attribute\n");
            return NULL;
        }

    }

    // Check for required attributes
    if (!has_class) {
        fprintf(stderr, "Missing required attribute (class)\n");
        return NULL;
    }
    if (!has_members) {
        fprintf(stderr, "Missing required attribute (members)\n");
        return NULL;
    }

    // Skip the end of the object
    json++;

    return json;
}

// Function to parse a value
char *parseValue(char* json, OBJBLOCK** shvobject_handle) {
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

    // Create a new OBJBLOCK
    *shvobject_handle = (OBJBLOCK*)malloc(sizeof(OBJBLOCK));
    if (!(*shvobject_handle)) {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }
    (*shvobject_handle)->type = VALUE_NULL;
    (*shvobject_handle)->typename = NULL;
    (*shvobject_handle)->value.string = NULL;

    // Work out what kind of object this is
    char *string_value;
    json = json_skip_whitespace(json);
    if (*json == '"') {
        // String
        json = json_skip_string(json, &((*shvobject_handle)->value.string));
        (*shvobject_handle)->type = VALUE_STRING;
    }

    else if (*json == '-' || *json == '.' || isdigit(*json)) {
        // Number
        VALUETYPE type;
        long integer;
        double real;
        // Parse the number
        json = json_skip_number(json, &type, &integer, &real);
        if (!json) {
            fprintf(stderr, "Invalid value type\n");
            return NULL;
        }
        (*shvobject_handle)->type = type;
        if (type == VALUE_INT) {
            (*shvobject_handle)->value.integer = integer;
        }
        else {
            (*shvobject_handle)->value.real = real;
        }
    }

    else if (*json == '{') {
        // Object or Binary
        if (json_peek_binary(json)) {
            // Binary
            json = json_skip_binary(json, &((*shvobject_handle)->value.binary.data), &((*shvobject_handle)->value.binary.length));
            if (!json) {
                fprintf(stderr, "Invalid value type (binary)\n");
                return NULL;
            }
            (*shvobject_handle)->type = VALUE_BINARY;
        }
        else {
            // Object
            MEMBLOCK *prev_array_member = NULL;
            MEMBLOCK *array_member;
            (*shvobject_handle)->type = VALUE_OBJECT;
            (*shvobject_handle)->value.members = NULL;
            // Skip the {
            json++;

            // Parse the object
            json = parseObject(json, *shvobject_handle);
            if (!json) {
                fprintf(stderr, "Failed to parse object\n");
                return NULL;
            }
        }
    }

    else if (*json == '[') {
        // Array
        MEMBLOCK *prev_array_member = NULL;
        MEMBLOCK *array_member;
        (*shvobject_handle)->type = VALUE_ARRAY;
        (*shvobject_handle)->value.members = NULL;
        // Skip the [
        json++;
        // Loop through the array
        while (*json != ']') {
            array_member = (MEMBLOCK*)malloc(sizeof(MEMBLOCK));
            if (!array_member) {
                fprintf(stderr, "Memory allocation error\n");
                return NULL;
            }

            // Set the head of the linked list
            if (!(*shvobject_handle)->value.members) (*shvobject_handle)->value.members = array_member;
            // Set the previous member's next pointer
            if (prev_array_member) {
                prev_array_member->membernext = array_member;
                prev_array_member = array_member;
            }
            array_member->membernext = NULL;

            // Skip to the member start {
            json = json_next_object(json);
            if (!json) {
                fprintf(stderr, "Invalid JSON format - no start of object in array\n");
                return NULL;
            }

            // Parse the member name
            json = json_skip_string(json, &(array_member->membername));
            if (!json) {
                fprintf(stderr, "Invalid JSON format - no member name in array\n");
                return NULL;
            }

            // Skip to the :
            json = json_skip_colon(json);
            if (!json) {
                fprintf(stderr, "Invalid JSON format - no colon after member name in array\n");
                return NULL;
            }

            // Parse the array member value
            json = parseValue(json, &(array_member->memberobject));
            if (!json) {
                fprintf(stderr, "Failed to parse object in array\n");
                return NULL;
            }

            // Skip to the end of the object
            json = json_skip_whitespace(json);
            if (*json != '}') {
                fprintf(stderr, "Invalid JSON format - no end of object in array\n");
                return NULL;
            }
            json++;

            // Skip to the next object or the end of the array
            json = json_skip_whitespace(json);
            // If the next character is a comma, skip it
            if (*json == ',') json++;
            // Skip any whitespace
            json = json_skip_whitespace(json);
        }
        // Skip the ]
        json++;
    }

    else if (*json == 'n') {
        // Possibly Null - so check for null
        json = json_skip_null(json);
        if (!json) {
            fprintf(stderr, "Invalid value type\n");
            return NULL;
        }
        (*shvobject_handle)->type = VALUE_NULL;
    }

    else if (*json == 't') {
        // Possibly True - so check for true
        json = json_skip_true(json);
        if (!json) {
            fprintf(stderr, "Invalid value type\n");
            return NULL;
        }
        (*shvobject_handle)->type = VALUE_BOOL;
        (*shvobject_handle)->value.boolean = 1;
    }

    else if (*json == 'f') {
        // Possibly False - so check for false
        json = json_skip_false(json);
        if (!json) {
            fprintf(stderr, "Invalid value type\n");
            return NULL;
        }
        (*shvobject_handle)->type = VALUE_BOOL;
        (*shvobject_handle)->value.boolean = 0;
    }

    else {
        fprintf(stderr, "Invalid value type\n");
        return NULL;
    }

    return json;
}

// Function to skip whitespace
char *json_skip_whitespace(char *json) {
    while (*json && (*json == ' ' || *json == '\t' || *json == '\n' || *json == '\r')) json++;
    return json;
}

// Function to move the char position to the character after the next "{"
char *json_next_object(char *json) {
    while (*json) {
        if (*json == '{') return json + 1;
        if (*json == ' ' || *json == '\t' || *json == '\n' || *json == '\r') json++;
        else return NULL; // Invalid character
    }
    return NULL;
}

// Function to move the char position to the character after the current block
char *json_skip_block(char *json) {
    int depth = 1;
    while (*json) {
        if (*json == '{') depth++;
        if (*json == '}') depth--;
        if (depth == 0) return json + 1;
        json++;
    }
    return NULL;
}

// Function to skip ":" and whitespace
char *json_skip_colon(char *json) {
    json = json_skip_whitespace(json);
    if (*json == ':') return json + 1;
    return NULL;
}

// Function to skip a string (and update the string parameter to point to the
// start of the string which is null terminated)
char *json_skip_string(char *json, char **string) {
    json = json_skip_whitespace(json);
    if (*json != '"') return NULL;
    json++;
    *string = json;
    while (*json && *json != '"') json++;
    if (*json != '"') return NULL; // No closing quote
    *json = '\0'; // Null terminate the string
    return json + 1;
}

// Function to skip a null token
char *json_skip_null(char *json) {
    // Skip the null token
    if (strncmp(json, "null", 4) != 0) {
        return NULL; // Not a null token
    }
    json += 4;

    // Check the next character
    if (*json == ' ' || *json == '\t' || *json == '\n' || *json == '\r' ||
        *json == ']' || *json == '}' || *json == ',' || *json == '\0') {
        return json;
    }

    return NULL; // Invalid character after null token
}

// Function to skip a true token
char *json_skip_true(char *json) {
    // Skip the true token
    if (strncmp(json, "true", 4) != 0) {
        return NULL; // Not a true token
    }
    json += 4;

    // Check the next character
    if (*json == ' ' || *json == '\t' || *json == '\n' || *json == '\r' ||
        *json == ']' || *json == '}' || *json == ',' || *json == '\0') {
        return json;
    }

    return NULL; // Invalid character after true token
}

// Function to skip a false token
char *json_skip_false(char *json) {
    // Skip the false token
    if (strncmp(json, "false", 5) != 0) {
        return NULL; // Not a false token
    }
    json += 5;

    // Check the next character
    if (*json == ' ' || *json == '\t' || *json == '\n' || *json == '\r' ||
        *json == ']' || *json == '}' || *json == ',' || *json == '\0') {
        return json;
    }

    return NULL; // Invalid character after false token
}

// Function to parse and skip a number
char *json_skip_number(char *json, VALUETYPE *type_handle, long *integer_handle, double *real_handle) {
    // Parse and skip a number (integer or float) setting the type_handle to the type
    // and the integer_handle or real_handle to the value
    // Return the position after the number (or NULL if the number is invalid)

    char is_valid = 0;
    char is_negative = 0;
    *integer_handle = 0;
    *real_handle = 0.0;
    *type_handle = VALUE_INT;

    // Parse the number
    // Leading - sign
    if (*json == '-') {
        json++;
        is_negative = 1;
        is_valid = 1;
    }
    // Leading 0s
    while (*json == '0') {
        json++;
        is_valid = 1;
    }
    // Digits
    while (*json >= '0' && *json <= '9') {
        *integer_handle = *integer_handle * 10 + (*json - '0');
        *real_handle = *real_handle * 10.0 + (*json - '0');
        json++;
        is_valid = 1;
    }
    // Decimal point
    if (*json == '.') {
        json++;
        *type_handle = VALUE_FLOAT;
        double decimal = 0.1;
        while (*json >= '0' && *json <= '9') {
            *real_handle += (*json - '0') * decimal;
            decimal *= 0.1;
            json++;
            is_valid = 1;
        }
    }
    // Exponent
    if (*json == 'e' || *json == 'E') {
        json++;
        *type_handle = VALUE_FLOAT;
        int exponent = 0;
        char is_negative_exponent = 0;
        if (*json == '-') {
            json++;
            is_negative_exponent = 1;
        }
        else if (*json == '+') {
            json++;
        }
        while (*json >= '0' && *json <= '9') {
            exponent = exponent * 10 + (*json - '0');
            json++;
            is_valid = 1;
        }
        if (is_negative_exponent) {
            *real_handle /= pow(10, exponent);
        }
        else {
            *real_handle *= pow(10, exponent);
        }
    }

    if (!is_valid) {
        return NULL; // Invalid number
    }

    if (*type_handle == VALUE_INT) {
        *real_handle = 0;
        if (is_negative) *integer_handle = -*integer_handle;
    }
    else {
        *integer_handle = 0;
        if (is_negative) *real_handle = -*real_handle;
    }

    return json;
}

// Function to peek if the next object might be a binary object
int json_peek_binary(char *json) {
    // We check for this format - only checking that the "base64" is present
    // {
    //  "base64": "c3RyaW5nIGJpbmFyeSBkYXRh"
    // }
    // Return 1 if it is a binary object, 0 if it is not (any errors will be picked up by the main parser)

    char* attribute;

    // Skip to the start of the object
    json = json_next_object(json);
    if (!json) {
        return 0;
    }

    // Skip to the base64 attribute
    // todo - we need to peek for a string here (not mess with the json buffer)
    json = json_skip_string(json, &attribute);
    if (!json) {
        return 0;
    }
    if (strcmp(attribute, "base64") != 0) {
        return 0;
    }
    // We need to fix the attribute sting (json buffer) as it has been null terminated by putting the " back
    *(json-1) = '"';

    return 1; // It might be a binary object
}

// Function to skip/parse a binary object
char *json_skip_binary(char *json, char **binary_handle, size_t *length_handle) {
    // Parse the binary object and return the base64 string and length
    // Return the position after the object (or NULL if the object is invalid)

    char *attribute;
    char *value;
    *binary_handle = NULL;
    *length_handle = 0;

    // Skip to the start of the object
    json = json_next_object(json);
    if (!json) {
        return NULL;
    }

    // Skip to the base64 attribute
    json = json_skip_string(json, &attribute);
    if (!json) {
        return NULL;
    }
    if (strcmp(attribute, "base64") != 0) {
        return NULL;
    }

    // Skip to the value string
    json = json_skip_colon(json);
    json = json_skip_string(json, &value);
    if (!json) {
        return NULL;
    }

    // Skip to the end of the object
    json = json_skip_block(json);
    if (!json) {
        return NULL;
    }

    // Decode the base64 string into binary, in situ in the base64 string as it will be shorter, setting the *length_handle
    // to the length of the binary data
    *binary_handle = value;
    *length_handle = base64_decode(value);
    if (*length_handle < 0) { // base64_decode returns -1 if there is an error
        return NULL;
    }

    return json;
}

// Base64 decode function - decoding done in situ in the buffer, and returns the length of the decoded data
// Return -1 on decoding error
int base64_decode(char *buffer) {

    static const char base64_table[65] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    // This initializes the base64 decoding table - this could be moved outside the function for efficiency but
    // it is only done once per call to the function so it is not a big overhead
    int i;
    int dtable[256] = {-1};
    for (i = 0; i < 64; i++)
        dtable[(uint8_t) base64_table[i]] = (uint8_t) i;
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
