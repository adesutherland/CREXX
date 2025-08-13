// idlib_api.c — CREXX plugin exposing UUIDv4 + UUIDv7 by including modules
// (-●-●)> dual-licensed WTFPL v2 / MIT

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

#include "crexxpa.h"   // CREXX / Plugin Architecture

#if defined(__linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

/* --------------------------------------------------------------------
   Bring in UUIDv4 API + implementation
   Files expected next to this file (or in your include path):
     - uuid4.h
     - uuid4.c
-------------------------------------------------------------------- */
#ifndef UUID4_H
#  include "uuid4.h"
#endif
/* Pull implementation directly into this TU */
#include "uuid4.c"

/* --------------------------------------------------------------------
   Bring in UUIDv7 API + implementation
   Files expected next to this file (or in your include path):
     - uuidv7.h
     - uuidv7.c
-------------------------------------------------------------------- */
#ifndef UUIDV7_H
#  include "uuidv7.h"
#endif
/* Pull implementation directly into this TU */
#include "uuidv7.c"

/* --------------------------------------------------------------------
   Bring in ULID API + implementation
   Files expected next to this file (or in your include path):
     - ulid.h
     - ulid.c
-------------------------------------------------------------------- */
#ifndef ULID_H
#include "ulid.h"
#endif
#include "ulid.c"


/* ----------------------- CREXX Procedures ----------------------- */

/* UUIDv4 via uuid4.c */
PROCEDURE(uuid) {
    UUID4_STATE_T state;
    UUID4_T u4;
    char buffer[UUID4_STR_BUFFER_SIZE];

    uuid4_seed(&state);
    uuid4_gen(&state, &u4);

    if (!uuid4_to_s(u4, buffer, sizeof(buffer)))
        RETURNSTR("-8");

    RETURNSTR(buffer);
    PROCRETURN
    ENDPROC
}

/* Simple rand()-based demo UUIDv4 (kept for compatibility) */
PROCEDURE(uuidt) {
    char out[37];
    uint8_t b[16];

    srand((unsigned int)time(NULL));
    for (int i = 0; i < 16; ++i) b[i] = rand() % 256;

    b[6] = (b[6] & 0x0F) | 0x40;  /* version 4 */
    b[8] = (b[8] & 0x3F) | 0x80;  /* variant 10 */

    snprintf(out, sizeof(out),
             "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             b[0], b[1], b[2], b[3],
             b[4], b[5],
             b[6], b[7],
             b[8], b[9],
             b[10], b[11], b[12],
             b[13], b[14], b[15]);

    RETURNSTR(out);
    PROCRETURN
    ENDPROC
}

/* UUIDv7 via uuidv7.c */
PROCEDURE(uuidv7) {
    uint8_t u7[16];
    char s[37];

    if (uuidv7_generate(u7)) {
        uuidv_to_string(u7, s);
        RETURNSTR(s);
    } else {
        RETURNSTR("ERROR: uuidv7_generate failed");
    }
    ENDPROC
}

PROCEDURE(ulid) {
    uint8_t u[16];
    char s[27];
    if (ulid_generate(u)) {
        ulid_to_string(u, s);
        RETURNSTR(s);
    } else {
        RETURNSTR("ERROR: ulid_generate failed");
    }
    ENDPROC
}



/* --------------------- Registration block --------------------- */
LOADFUNCS
    ADDPROC(uuid,   "idlib.uuid",   "b", ".string", "");
    ADDPROC(uuidt,  "idlib.uuidt",  "b", ".string", "");
    ADDPROC(uuidv7, "idlib.uuidv7",  "b", ".string", "");
    ADDPROC(ulid,   "idlib.ulid",    "b", ".string", "");
ENDLOADFUNCS
