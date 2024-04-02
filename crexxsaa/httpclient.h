//
// Created by Adrian Sutherland on 26/03/2024.
//

#ifndef REXXSAAPOC_HTTPCLIENT_H
#define REXXSAAPOC_HTTPCLIENT_H

#include "crexxsaa.h"

/* Block to hold the first SHVBLOCK and a pointer to the JSON buffer to facilitate freeing the buffers */
typedef struct SHVBUFFER {
    struct SHVBLOCK shvblock;     // The SHVBLOCK
    char *json;            // Pointer to the json buffer (the pointer is appended to the SHVBLOCK)
} SHVBUFFER;

int parseJSON(char* json, SHVBLOCK** shvblock_handle);


#endif //REXXSAAPOC_HTTPCLIENT_H
