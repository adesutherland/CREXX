/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXCP_FLOW_H
#define CREXX_RXCP_FLOW_H

#include "rxcp_types.h"

typedef enum RxcpFlowValueKind {
    RXCP_FLOW_SOURCE_SYMBOL = 1,
    RXCP_FLOW_COMPILER_SYMBOL,
    RXCP_FLOW_AST_TEMPORARY
} RxcpFlowValueKind;

/* Build a fresh typed procedure-local flow overlay. When apply_transforms is
 * non-zero, apply only the bounded transformations whose proof predicates
 * succeed. The overlay never owns AST, Symbol or Scope objects. */
int rxcp_flow_analyze(Context *context, int apply_transforms);
void rxcp_flow_free(Context *context);

#endif /* CREXX_RXCP_FLOW_H */
