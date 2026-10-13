/* SPDX-License-Identifier: MIT
 * Synchronous native command policy. No interpreter values or native ABI
 * parameter layouts cross this boundary. One handle belongs to one VM. */
#ifndef CREXX_MAINFRAME_COMMAND_H
#define CREXX_MAINFRAME_COMMAND_H
#include <stddef.h>
typedef struct crexx_command_handle crexx_command_handle;
enum { CREXX_COMMAND_NORMAL, CREXX_COMMAND_ERROR, CREXX_COMMAND_FAILURE };
enum { CREXX_COMMAND_CAPTURE_OUTPUT = 1, CREXX_COMMAND_CAPTURE_ERROR = 2,
       CREXX_COMMAND_INPUT = 4 };
typedef int (*crexx_command_sink)(void *, const char *, size_t);
typedef struct crexx_command_io {
    unsigned requested;
    crexx_command_sink output, error;
    void *owner;
    const unsigned char *input;
    size_t input_length;
} crexx_command_io;
typedef struct crexx_command_result {
    int rc, condition, status, service_rc, reason_rc, module_rc;
    unsigned command_rc_valid, abend_code;
    char diagnostic[192];
} crexx_command_result;
crexx_command_handle *crexx_command_open(void);
void crexx_command_close(crexx_command_handle *);
/* Names only are normalized. Match caches a routing object even when a native
 * subcommand is absent; query/execute recheck presence, without timers. */
int crexx_command_name(const char *, size_t, char [9]);
int crexx_command_match(const char *, size_t);
const char *crexx_command_profile(void);
int crexx_command_query(crexx_command_handle *, const char *, int *, crexx_command_result *);
unsigned crexx_command_capabilities(crexx_command_handle *, const char *);
int crexx_command_execute(crexx_command_handle *, const char *, const char *, size_t,
                          const crexx_command_io *, crexx_command_result *);
#endif
