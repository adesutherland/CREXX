/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#ifndef CREXX_RXCC_H
#define CREXX_RXCC_H
#include <stddef.h>
#include "rxcp_types.h"
typedef struct RxccUnit RxccUnit;
typedef struct RxccRequest {
    const void *source;
    size_t length; /* Exact UTF-8 byte span; embedded NUL is source data. */
    const char *name;
    const char *location;
    const char *binary_imports; /* Existing semicolon-separated path contract. */
    const char *source_imports;
    int routine_mode;
    int noopt;
} RxccRequest;

/* Internal frontend service: parsed/mapped AST, not emitted RXAS/RXBIN.
 * The returned unit owns source, diagnostics, AST and all request state.
 * Invalid source returns a unit with nonzero status, never CLI exit(). */
RxccUnit *rxcc_parse(const RxccRequest *request);
int rxcc_status(const RxccUnit *unit);
const char *rxcc_failure(const RxccUnit *unit);
Context *rxcc_context(RxccUnit *unit);
void rxcc_destroy(RxccUnit *unit);
#endif
