/* cREXX License (MIT). Independent default-entry inventory oracle. */
#include "rxvmentry.h"
#include <stdio.h>
#include <string.h>

typedef struct { int code; const char *name; const char *response; } signal_row;
#define SIGNAL_ROW(name, response) { RXSIGNAL_##name, #name, #response },
static const signal_row signals[] = { RXVM_PROGRAM_DEFAULT_SIGNALS(SIGNAL_ROW) };
#undef SIGNAL_ROW

static int failures;
#define CHECK(condition, message) do { \
    if (!(condition)) { fprintf(stderr, "FAIL: %s\n", message); failures++; } \
} while (0)

static const char *expected_response(int code) {
    switch (code) {
        case RXSIGNAL_OVERFLOW_UNDERFLOW:
        case RXSIGNAL_INVALID_SIGNAL_CODE:
        case RXSIGNAL_POSIX_HUP:
        case RXSIGNAL_POSIX_USR1:
        case RXSIGNAL_POSIX_USR2:
        case RXSIGNAL_POSIX_CHLD:
        case RXSIGNAL_BREAKPOINT:
            return "IGNORE";
        case RXSIGNAL_CANCEL:
            return "SILENT_HALT";
        default:
            return "HALT";
    }
}

int main(void) {
    size_t i;
    unsigned int seen[RXSIGNAL_MAX] = { 0u };
    numeric_context numeric;
    CHECK(!strcmp(RXVM_PROGRAM_DEFAULT_CONTRACT, "program-default.v1"), "versioned root-default contract");
    CHECK(sizeof(signals) / sizeof(signals[0]) == 31u, "all 31 valid named signals are explicit");
    for (i = 0u; i < sizeof(signals) / sizeof(signals[0]); ++i) {
        const signal_row *row = &signals[i];
        CHECK(row->code > RXSIGNAL_NONE && row->code < RXSIGNAL_MAX, "sentinels cannot become known defaults");
        if (row->code <= RXSIGNAL_NONE || row->code >= RXSIGNAL_MAX) continue;
        CHECK(row->code == (int)i + 1, "inventory remains in canonical native code order");
        CHECK(seen[row->code]++ == 0u, "each signal has one response");
        CHECK(row->name[0] != '\0', "every valid signal has a canonical name");
        CHECK(!strcmp(row->response, expected_response(row->code)), "mixed root responses preserve their independent expected behavior");
    }
    for (i = 1u; i < RXSIGNAL_MAX; ++i) CHECK(seen[i] == 1u, "no valid signal is omitted");
#define NUMERIC_ROW(member, domain, default_value) numeric.member = default_value;
    RXVM_PROGRAM_DEFAULT_NUMERIC(NUMERIC_ROW)
#undef NUMERIC_ROW
    CHECK(numeric.digits == 18 && numeric.fuzz == 0, "DIGITS 18 and FUZZ 0");
    CHECK(numeric.form == NUMERIC_FORM_SCIENTIFIC && numeric.casetype == CASE_LOWER &&
          numeric.standard == NUMERIC_STANDARD_COMMON, "SCIENTIFIC LOWER COMMON numeric defaults");
    puts(failures ? "PROGRAM_DEFAULTS|failures=1" : "PROGRAM_DEFAULTS|failures=0");
    return failures ? 1 : 0;
}
