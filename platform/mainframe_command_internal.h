/* SPDX-License-Identifier: MIT */
#ifndef CREXX_MAINFRAME_COMMAND_INTERNAL_H
#define CREXX_MAINFRAME_COMMAND_INTERNAL_H
#include "mainframe_command.h"
#include <command.h>
struct crexx_command_handle { MfCommandSession discovery; unsigned capture_serial; };
int crexx_command_native_query(crexx_command_handle *, const char *, int *, MfCommandResult *);
int crexx_command_native_execute(crexx_command_handle *, const char *, const char *, unsigned, MfCommandResult *);
unsigned crexx_command_native_capabilities(crexx_command_handle *, const char *);
int crexx_command_native_capture(crexx_command_handle *, const char *, const char *, unsigned,
                                 const crexx_command_io *, MfCommandResult *);
#endif
