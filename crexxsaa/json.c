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

/* Function prototypes */
int parseJSON(char* json, SHVBLOCK** shvblock_handle);

char *json_skip_whitespace(char *json);
char *json_skip_object(char *json);
char *json_next_object(char *json);
char *json_skip_block(char *json);
char *json_skip_colon(char *json);
char *json_skip_comma(char *json);
char *json_skip_string(char *json, char **string);

/* Free the SHVOBJECT */
void FreeRexxObject(Object *shvobject) {
    Object *shvobject_next;

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

/* Parses the JSON body */
int parseJSON(char* json, SHVBLOCK** shvblock_handle) {
    char *name, *value;
    int name_length, value_length;




    // Skip to the start of the object
    json = json_next_object(json);
    if (!json) {
        fprintf(stderr, "Invalid JSON format - no start of object\n");
        return 1;
    }

    // Skip to the name string
    json = json_skip_string(json, &name);
    if (!json) {
        fprintf(stderr, "Invalid JSON format - no \"serviceBlocks\"\n");
        return 1;
    }
    if (strcmp(name, "serviceBlocks") != 0) {
        fprintf(stderr, "Invalid JSON format - no \"serviceBlocks\", found \"%s\"\n", name);
        return 1;
    }

    // Skip to the value string
    json = json_skip_colon(json);
    json = json_skip_string(json, &value, &value_length);
    if (!json) {
        fprintf(stderr, "Invalid JSON format - no value string\n");
        return 1;
    }

    // Null-terminate the strings
    name[name_length] = '\0';
    value[value_length] = '\0';

    printf("name: %s\n", name);
    printf("value: %s\n", value);
}

// Function to parse request property
int parseRequest(char* json) {
    // Parse JSON Body using the json functions, example format:
    //    "<request_code>" [,]
    //  where <request_code> is one of "set", "fetch", "drop", "nextv", "priv", "syset", "syfet", "sydro", "sydel"
    char *request;
    char *buffer;
    size_t buffer_size;
    int result;

    // Skip to the name string
    json = json_skip_string(json, &buffer, &buffer_size);
    if (!json) {
        fprintf(stderr, "Invalid JSON format - no request_code\n");
        return 1;
    }
    /* Compare buffer with each request code and set the result acordimng to the corresponding crexxsaa.h code */
    if (strcmp(buffer, "set") == 0) {
        result = 0;
    } else if (strcmp(buffer, "fetch") == 0) {
        result = 1;
    } else if (strcmp(buffer, "drop") == 0) {
        result = 2;
    } else if (strcmp(buffer, "nextv") == 0) {
        result = 3;
    } else if (strcmp(buffer, "priv") == 0) {
        result = 4;
    } else if (strcmp(buffer, "syset") == 0) {
        result = 5;
    } else if (strcmp(buffer, "syfet") == 0) {
        result = 6;
    } else if (strcmp(buffer, "sydro") == 0) {
        result = 7;
    } else if (strcmp(buffer, "sydel") == 0) {
        result = 8;
    } else {
        fprintf(stderr, "Invalid request_code\n");
        return 1;
    }

    // Skip to the value string
    json = json_skip_colon(json);
    json = json_skip_string(json, &value, &value_length);
    if (!json) {
        fprintf(stderr, "Invalid JSON format - no value string\n");
        return 1;
    }

    // Null-terminate the strings
    name[name_length] = '\0';
    value[value_length] = '\0';

    printf("name: %s\n", name);
    printf("value: %s\n", value);
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
