/*
   Created by Adrian Sutherland on 03/03/2024.
   This is a public domain header file for the CREXX "SAA" API

   The header file is renamed crexxsaa.h (from rexxsaa.h) to avoid conflicts with the original REXX SAA API.

   The CREXX SAA API is a superset of the REXX SAA API and is designed to be used with the CREXX REXXSAA VM Server.

   The CREXX REXXSAA VM Server is a RESTful API server that allows REXX programs to interact with the server
   to set and retrieve variables, execute REXX commands, and perform other operations.

   The design of crexxsaa.h intended to be sufficiently backward compatible with the REXX SAA API to allow existing REXX programs
   to be ported with minimal changes.
*/

#ifndef REXXSAA_H
#define REXXSAA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <string.h> // For strlen()

/* Define SHVBLOCK and related structure for sharing variable data */
typedef struct SHVBLOCK SHVBLOCK;
typedef struct OBJBLOCK OBJBLOCK;
typedef struct MEMBLOCK MEMBLOCK;

/* VALUETYPE - Type of the value */
typedef enum VALUETYPE {
    VALUE_STRING = 0,
    VALUE_NULL,
    VALUE_BINARY,
    VALUE_INT,
    VALUE_FLOAT,
    VALUE_BOOL,
    VALUE_ARRAY, // For Arrays
    VALUE_OBJECT // For nested objects
} VALUETYPE;

/* SHVBLOCK - Structure for sharing variable data */
struct SHVBLOCK {
    SHVBLOCK *shvnext;            // Pointer to next SHVBLOCK
    char *shvname;                // Variable name (null-terminated)
    OBJBLOCK *shvobject;          // Object value
    unsigned long shvcode;        // Function code
    unsigned long shvret;         // Return code
};

/* OBJBLOCK - Structure for nested objects */
struct OBJBLOCK {
    VALUETYPE type;               // Type of the object
    char* typename;               // Name of the object type (used for type == VALUE_OBJECT only)
    union {
        char* string;             // String value
        struct {                  // Binary data
            char* data;           // For the avoidance of doubt, binary data is not null-terminated and not base64 encoded
            size_t length;
        } binary;
        long integer;             // Integer value
        double real;              // Float value
        char boolean;             // Boolean value (0 or 1)
        MEMBLOCK* members;        // Pointer to a linked list of members (type == VALUE_OBJECT) or array elements (type == VALUE_ARRAY)
    } value;
};

/* MEMBLOCK - Structure for linked list of members */
struct MEMBLOCK {
    MEMBLOCK *membernext; // Pointer to next SHVMEMBER member
    char* membername;             // Name of the member
    OBJBLOCK* memberobject;       // Pointer to an objects
};

/* Define function codes for SHVBLOCK */
#define RXSHV_SET      0x0001    /* Set variable value */
#define RXSHV_FETCH    0x0002    /* Fetch variable value */
#define RXSHV_DROP     0x0004    /* Drop variable */
#define RXSHV_NEXTV    0x0008    /* Fetch next variable */
#define RXSHV_PRIV     0x0010    /* Access private variable pool */
#define RXSHV_SYSET    0x0020    /* Set symbolic variable */
#define RXSHV_SYFET    0x0040    /* Fetch symbolic variable */
#define RXSHV_SYDRO    0x0080    /* Drop symbolic variable */
#define RXSHV_SYDEL    0x0100    /* Delayed fetch of symbolic variable */

/* Define return codes for SHVBLOCK */
#define RXSHV_OK       0         /* Successful completion */
#define RXSHV_NEWV     1         /* New variable created */
#define RXSHV_LVAR     2         /* Last variable retrieved */
#define RXSHV_TRUNC    3         /* Truncated value */
#define RXSHV_BADN     4         /* Invalid variable name */
#define RXSHV_MEMFL    5         /* Memory allocation failed */
#define RXSHV_BADF     6         /* Invalid function code */
#define RXSHV_NOAVL    7         /* No more variables available */
#define RXSHV_NOTEX    8         /* Variable does not exist */

/* RexxVariablePool - Interface to the REXX variable pool */
unsigned long RexxVariablePool(SHVBLOCK *request, SHVBLOCK **result);

/* FreeRexxVariablePoolResult - Free the memory allocated for the whole Variable Pool SHVBLOCK linked list returned by RexxVariablePool */
/* *** NOTE This is only valid for use with the shvblock returned by RexxVariablePool result *** */
void FreeRexxVariablePoolResult(SHVBLOCK *shvblock);

/* Migration helpers */

/* RXSTRING is a char* */
typedef char* RXSTRING;

/* Creates an RXSTRING; since RXSTRING is a char*, this macro isn't strictly necessary */
#define MAKERXSTRING(x,c)     ((x) = (c))

/* Checks if the RXSTRING is NULL */
#define RXNULLSTRING(x)       (!(x))

/* Gets the length of the RXSTRING */
#define RXSTRLEN(x)           ((x) ? strlen(x) : 0UL)

/*  Returns the RXSTRING itself; this macro isn't strictly necessary as RXSTRING is already a char* */
#define RXSTRPTR(x)           (x)

/* Checks if the RXSTRING is valid (not NULL and not empty) */
#define RXVALIDSTRING(x)      ((x) && strlen(x))

/* Checks if the RXSTRING is not NULL but is an empty string */
#define RXZEROLENSTRING(x)    ((x) && !strlen(x))

#ifdef __cplusplus
}
#endif

#endif /* REXXSAA_H */
