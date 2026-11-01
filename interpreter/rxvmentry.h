/* cREXX License (MIT). Canonical VM root-entry defaults and catalog input. */
#ifndef CREXX_RXVMENTRY_H
#define CREXX_RXVMENTRY_H

#include "rxsignal.h"
#include "rxvalue.h"

#define RXVM_PROGRAM_DEFAULT_CONTRACT "program-default.v1"

/* All valid signal codes, in native code order. Response tokens expand to
 * the existing RXSIGNAL_RESPONSE_* enum at runtime and exact names in the
 * generated catalog. The invalid RXSIGNAL_MAX sentinel is not a row. */
#define RXVM_PROGRAM_DEFAULT_SIGNALS(X) \
    X(KILL, HALT) \
    X(FAILURE, HALT) \
    X(ERROR, HALT) \
    X(OVERFLOW_UNDERFLOW, IGNORE) \
    X(DIVISION_BY_ZERO, HALT) \
    X(CONVERSION_ERROR, HALT) \
    X(INVALID_ARGUMENTS, HALT) \
    X(OUT_OF_RANGE, HALT) \
    X(UNICODE_ERROR, HALT) \
    X(UNKNOWN_INSTRUCTION, HALT) \
    X(FUNCTION_NOT_FOUND, HALT) \
    X(NOT_IMPLEMENTED, HALT) \
    X(INVALID_SIGNAL_CODE, IGNORE) \
    X(REFERENCE_INVALID, HALT) \
    X(NOTREADY, HALT) \
    X(OBJECT_NOT_INITIALIZED, HALT) \
    X(RXBIN_CORRUPTION, HALT) \
    X(CANCEL, SILENT_HALT) \
    X(QUIT, HALT) \
    X(TERM, HALT) \
    X(POSIX_INT, HALT) \
    X(POSIX_HUP, IGNORE) \
    X(POSIX_USR1, IGNORE) \
    X(POSIX_USR2, IGNORE) \
    X(POSIX_CHLD, IGNORE) \
    X(CHANNEL_ERROR, HALT) \
    X(TASK_FAILURE, HALT) \
    X(CLASSIC_SYNTAX, HALT) \
    X(CLASSIC_CONDITION, HALT) \
    X(OTHER, HALT) \
    X(BREAKPOINT, IGNORE)

/* Native numeric member, canonical RXOP_CONTEXT_* domain, value owner. */
#define RXVM_PROGRAM_DEFAULT_NUMERIC(X) \
    X(digits, DIGITS, DEFAULT_NUMERIC_DIGITS) \
    X(fuzz, FUZZ, DEFAULT_NUMERIC_FUZZ) \
    X(form, FORM, DEFAULT_NUMERIC_FORM) \
    X(casetype, CASE, DEFAULT_NUMERIC_CASE) \
    X(standard, STANDARD, DEFAULT_NUMERIC_STANDARD)

#define RXVM_ENTRY_COUNT_SIGNAL(name, response) + 1
#define RXVM_ENTRY_COUNT_NUMERIC(member, domain, value) + 1
enum {
    RXVM_PROGRAM_DEFAULT_SIGNAL_COUNT = 0 RXVM_PROGRAM_DEFAULT_SIGNALS(RXVM_ENTRY_COUNT_SIGNAL),
    RXVM_PROGRAM_DEFAULT_NUMERIC_COUNT = 0 RXVM_PROGRAM_DEFAULT_NUMERIC(RXVM_ENTRY_COUNT_NUMERIC)
};
#undef RXVM_ENTRY_COUNT_NUMERIC
#undef RXVM_ENTRY_COUNT_SIGNAL

typedef char rxvm_program_defaults_cover_valid_signals[
        RXVM_PROGRAM_DEFAULT_SIGNAL_COUNT == RXSIGNAL_MAX - 1 ? 1 : -1];
typedef char rxvm_program_defaults_cover_numeric_fields[
        RXVM_PROGRAM_DEFAULT_NUMERIC_COUNT == 5 ? 1 : -1];

#endif
