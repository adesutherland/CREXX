/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXCP_PARTIAL_CALL_H
#define CREXX_RXCP_PARTIAL_CALL_H

#include <stddef.h>
#include "rxcp_types.h"

typedef struct RxcpPartialCallResult {
    ValueType type;
    rxinteger int_value;
    double float_value;
    char *decimal_value;
    char *string_value;
    size_t string_length;
} RxcpPartialCallResult;

/* Execute a complete callable body only when all actuals and every reached
 * language/RXAS operation have an exact bounded compile-time interpretation.
 * This is deliberately body-driven: callable names and BIF membership are not
 * part of foldability. */
int rxcp_partial_call_evaluate(Context *context,
                               ASTNode *call,
                               RxcpPartialCallResult *result);

void rxcp_partial_call_result_clear(RxcpPartialCallResult *result);

#endif /* CREXX_RXCP_PARTIAL_CALL_H */
