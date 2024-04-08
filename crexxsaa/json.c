//
// Created by Adrian Sutherland on 26/03/2024.
//

// Simple JSON Parser

/* The JSON Schema is as follows:
 * {
 *   "$schema": "http://json-schema.org/draft-07/schema#",
 *   "type": "object",
 *   "properties": {
 *     "serviceBlocks": {
 *       "type": "array",
 *       "items": {
 *         "type": "object",
 *         "properties": {
 *           "name": {
 *             "type": "string"
 *           },
 *           "request": {
 *             "type": "string",
 *             "enum": ["set", "fetch", "drop", "nextv", "priv", "syset", "syfet", "sydro", "sydel"]
 *           },
 *           "result": {
 *             "type": "string",
 *             "enum": ["ok", "newv", "lvar", "trunc", "badn", "memfl", "badf", "noavl", "notex"]
 *           },
 *           "value": {
 *             "oneOf": [
 *               {
 *                 "type": "string"
 *              },
 *              {
 *                 "type": "number"
 *              },
 *               {
 *                 "type": "boolean"
 *               },
 *               {
 *                 "type": "object",
 *                 "properties": {
 *                  "base64": {
 *                     "type": "string"
 *                   }
 *                 },
 *                 "required": ["base64"]
 *               },
 *               {
 *                 "type": "object",
 *                 "properties": {
 *                   "class": {
 *                     "type": "string"
 *                   },
 *                   "members": {
 *                     "type": "array",
 *                     "items": {
 *                       "type": "object",
 *                       "properties": {
 *                         "name": {
 *                           "type": "string"
 *                         },
 *                         "value": {
 *                           "oneOf": [
 *                             {
 *                               "type": "string"
 *                             },
 *                             {
 *                               "type": "number"
 *                             },
 *                             {
 *                               "type": "boolean"
 *                             },
 *                             {
 *                               "type": "object",
 *                               "properties": {
 *                                 "base64": {
 *                                   "type": "string"
 *                                 }
 *                               },
 *                               "required": ["base64"]
 *                             },
 *                             {
 *                               "$ref": "#"
 *                             }
 *                           ]
 *                         }
 *                       },
 *                       "required": ["name", "value"]
 *                     }
 *                   }
 *                 },
 *                 "required": ["class", "members"]
 *               },
 *               {
 *                 "type": "array",
 *                 "items": {
 *                   "$ref": "#/properties/serviceBlocks/items/properties/value/oneOf/3"
 *                }
 *               },
 *               {
 *                 "type": "null"
 *               }
 *             ]
 *           }
 *         },
 *         "required": ["name", "request"]
 *       }
 *     }
 *   },
 *   "required": ["serviceBlocks"]
 * }
 *
*/

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
char* parseShvblock(char* json, SHVBLOCK* current_shvblock); // Parse individual service block
long parseRequestCode(char* request_value); // Parse the request code (returns long so a negative value can be used for an error)
long parseResultCode(char* request_value); // Parse the result code (returns long so a negative value can be used for an error)
char * parseObject(char* json, SHVOBJECT** shvobject_handle, SHVOBJECT* prev_shvobject); // Parse value attribute

char *json_skip_whitespace(char *json);
char *json_skip_object(char *json);
char *json_next_object(char *json);
char *json_skip_block(char *json);
char *json_skip_colon(char *json);
char *json_skip_comma(char *json);
char *json_skip_string(char *json, char **string);
char *json_skip_null(char *json);
char *json_skip_true(char *json);
char *json_skip_false(char *json);

/* Free the SHVOBJECT */
void FreeRexxObject(SHVOBJECT *shvobject) {
    SHVOBJECT *shvobject_next;

    while (shvobject) {
        shvobject_next = shvobject->objnext;
        FreeRexxObject(shvobject->value.member);  // Recursively free the members
        free(shvobject); // Free the object
        shvobject = shvobject_next;
    }
}

/* Craete a SHVBLOCK */
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
    shvblock->shvvalue = NULL;
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
    FreeRexxObject(shvbuffer->shvblock.shvobject); // Free the SHVOBJECT
    shvblock_next = shvbuffer->shvblock.shvnext;
    free(shvbuffer->json); // Free the JSON buffer
    free(shvbuffer); // Free the SHVBUFFER

    /* Free the rest of the SHVBLOCK linked list */
    while (shvblock) {
        shvblock_next = shvblock->shvnext;
        FreeRexxObject(shvblock->shvobject); // Free the SHVOBJECT
        free(shvblock);
        shvblock = shvblock_next;
    }
}

/* Uncomment this if you don;t have a strcasecmp function
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

            // If the value is a string, parse it - it is the simple case where we just store the string
            if (*json == '"') {
                json = json_skip_string(json, &value);
                if (!json) {
                    fprintf(stderr, "Invalid JSON format - no value string value\n");
                    return NULL;
                }

                // Set the value
                current_shvblock->shvvalue = value;
                current_shvblock->shvobject = NULL;
            }

            else {
                // If the value is not a string, it must be an object
                // Parse the object
                json = parseObject(json, &(current_shvblock->shvobject), NULL);
                if (!json) {
                    fprintf(stderr, "Failed to parse object\n");
                    return NULL;
                }
                current_shvblock->shvvalue = NULL;
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

// Function to parse a value attribute
char *parseObject(char* json, SHVOBJECT** shvobject_handle, SHVOBJECT* prev_shvobject) {
    // Parse JSON Body using the json functions, example format:
    //
    // Note The value element can be a
    // - `string` ("string value")
    // - `number` (1234.56 - int or float)
    // - `boolean` (true/false)
    // - `binary` (base64 encoded - e.g. { "base64": "c3RyaW5nIGJpbmFyeSBkYXRh" }
    // - `object` (I.e. an object member), e.g.
    //   {
    //     "class": "complex_number",
    //              "members": [
    //      {
    //          "name": "real",
    //                  "value": 3.2
    //      },
    //      {
    //          "name": "imaginary",
    //                  "value": 4.5
    //      }
    //    }
    // - `array` (an array of value types [value1, value2, ...])
    // - `null` (null for an empty value, although this use case is TBD)

    // Create a new SHVOBJECT
    *shvobject_handle = (SHVOBJECT*)malloc(sizeof(SHVOBJECT));
    if (!(*shvobject_handle)) {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }
    (*shvobject_handle)->objnext = NULL;
    (*shvobject_handle)->type = VALUE_NULL;
    (*shvobject_handle)->typeName = NULL;
    (*shvobject_handle)->value.string = NULL;
    if (prev_shvobject) prev_shvobject->objnext = *shvobject_handle; // Link the new object to the previous object

    // Work out what kind of object this is
    char *string_value;
    json = json_skip_whitespace(json);
    if (*json == '"') {
        // String
        json_skip_string(json, &((*shvobject_handle)->value.string));
        (*shvobject_handle)->type = VALUE_STRING;
    }
    else if (*json == '-' || *json == '.' || isdigit(*json)) {
        // Number
    }
    else if (*json == '{') {
        // Object or Binary
    }
    else if (*json == '[') {
        // Array
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

}

// Function to skip whitespace
char *json_skip_whitespace(char *json) {
    while (*json && (*json == ' ' || *json == '\t' || *json == '\n' || *json == '\r')) json++;
    return json;
}

// Function to move the char position past the next "{"
char *json_skip_object(char *json) {
    int depth = 0;
    while (*json) {
        if (*json == '{') depth++;
        if (*json == '}') depth--;
        if (depth == 0) return json;
        json++;
    }
    return NULL;
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

// Function to skip "," and whitespace
char *json_skip_comma(char *json) {
    json = json_skip_whitespace(json);
    if (*json == ',') return json + 1;
    return NULL;
}

// Function to skip a string (and update the string parameter to point to the
// start of the string which is null terminated)
char *json_skip_string(char *json, char **string) {
    json = json_skip_whitespace(json);
    if (*json != '"') return NULL;
    json++;
    *string = json + 1;
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
char *json_skip_number(char *json, SHVVALUETYPE *type_handle, long *integer_handle, double *real_handle) {
    // Parse and skip a nuumber (integer or float) setting the type_handle to the type
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
