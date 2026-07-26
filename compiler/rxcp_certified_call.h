/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXCP_CERTIFIED_CALL_H
#define CREXX_RXCP_CERTIFIED_CALL_H

#include <stddef.h>
#include "rxcp_types.h"

typedef struct RxcpCertifiedCallResult {
    ValueType type;
    rxinteger int_value;
    double float_value;
    char *decimal_value;
    char *string_value;
    size_t string_length;
} RxcpCertifiedCallResult;

/* Return non-zero when the resolved call names an enabled certificate.  This
 * is only a cheap registry lookup; provider/body proof is performed by the
 * evaluator before any AST replacement. */
int rxcp_certified_call_candidate(ASTNode *call);

/* Evaluate one resolved call whose actuals have already been reduced to typed
 * constants.  Return non-zero only for a proved, non-signalling semantic cell.
 * Failure is the normal fail-closed result and leaves the Level B call intact. */
int rxcp_certified_call_evaluate(Context *context,
                                 ASTNode *call,
                                 RxcpCertifiedCallResult *result);

void rxcp_certified_call_result_clear(RxcpCertifiedCallResult *result);

#endif /* CREXX_RXCP_CERTIFIED_CALL_H */
