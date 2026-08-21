// id_api.c — CREXX plugin exposing UUIDv4 + UUIDv7 by including modules
// (-●-●)> dual-licensed WTFPL v2 / MIT

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

#include "crexxpa.h"   // CREXX / Plugin Architecture

#if defined(_WIN32)
#include <windows.h>
static SRWLOCK id_state_lock = SRWLOCK_INIT;
static void id_state_enter(void) { AcquireSRWLockExclusive(&id_state_lock); }
static void id_state_leave(void) { ReleaseSRWLockExclusive(&id_state_lock); }
#else
#include <pthread.h>
static pthread_mutex_t id_state_lock = PTHREAD_MUTEX_INITIALIZER;
static void id_state_enter(void) { (void)pthread_mutex_lock(&id_state_lock); }
static void id_state_leave(void) { (void)pthread_mutex_unlock(&id_state_lock); }
#endif

RXPA_PLUGIN_PROCESS_REENTRANT

#if defined(__linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

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
/* --------------------------------------------------------------------
   Bring in NANOID API + implementation
   Files expected next to this file (or in your include path):
     - nanoid.h
     - nanoid.c
-------------------------------------------------------------------- */
#ifndef NANOID_H
#include "nanoid.h"
#endif
#include "nanoid.c"

/* --------------------------------------------------------------------
   Bring in SNOWFLAKE API + implementation
   Files expected next to this file (or in your include path):
     - snowflake.h
     - snowflake.c
-------------------------------------------------------------------- */
#ifndef SNOWFLAKE_H
#include "snowflake.h"
#endif
#include "snowflake.c"

/* --------------------------------------------------------------------
   Bring in BASE58 API + implementation
   Files expected next to this file (or in your include path):
     - base58.h
     - base58.c
-------------------------------------------------------------------- */
#ifndef BASE58ID_H
#include "base58id.h"
#endif
#include "base58id.c"



/* ----------------------- CREXX Procedures ----------------------- */

/* UUIDv4 backed by the platform CSPRNG used by the UUIDv7 implementation. */
PROCEDURE(uuid4) {
    char out[37];
    uint8_t b[16];

    if (!uuidv7_csprng(b, sizeof(b)))
        RETURNSIGNAL(SIGNAL_FAILURE, "RXID.UUID4 random source failed")

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

    int generated;
    id_state_enter();
    generated = uuidv7_generate(u7);
    id_state_leave();
    if (generated) {
        uuidv_to_string(u7, s);
        RETURNSTR(s);
    } else {
        RETURNSIGNAL(SIGNAL_FAILURE, "RXID.UUID7 generation failed")
    }
    ENDPROC
}

PROCEDURE(ulid) {
    uint8_t u[16];
    char s[27];
    int generated;
    id_state_enter();
    generated = ulid_generate(u);
    id_state_leave();
    if (generated) {
        ulid_to_string(u, s);
        RETURNSTR(s);
    } else {
        RETURNSIGNAL(SIGNAL_FAILURE, "RXID.ULID generation failed")
    }
    ENDPROC
}

PROCEDURE(nanoid) {
    char s[NANOID_DEFAULT_SIZE + 1];
    if (nanoid_generate(s)) {
        RETURNSTR(s);
    } else {
        RETURNSIGNAL(SIGNAL_FAILURE, "RXID.NANOID generation failed")
    }
    ENDPROC
}
PROCEDURE(snowflake) {
    char s[21];
    if (snowflake_next_str(s)) {
        RETURNSTR(s);
    } else {
        RETURNSIGNAL(SIGNAL_FAILURE, "RXID.SNOWFLAKE generation failed")
    }
    ENDPROC
}

PROCEDURE(base58) {
    char s[64]; /* ample room for 16–32 raw bytes */
    if (base58id_generate(s, sizeof s)) {
        RETURNSTR(s);
    } else {
        RETURNSIGNAL(SIGNAL_FAILURE, "RXID.BASE58 generation failed")
    }
    ENDPROC
}

/* --------------------- Registration block --------------------- */
LOADFUNCS
    ADDPROC(uuid4, "rxid.uuid4", "b", ".string", "");
    ADDPROC(uuidv7, "rxid.uuid7", "b", ".string", "");
    ADDPROC(ulid, "rxid.ulid", "b", ".string", "");
    ADDPROC(nanoid, "rxid.nanoid", "b", ".string", "");
    ADDPROC(snowflake, "rxid.snowflake", "b", ".string", "");
    ADDPROC(base58, "rxid.base58", "b", ".string", "");
ENDLOADFUNCS
