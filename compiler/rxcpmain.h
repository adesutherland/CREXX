/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */

#ifndef CREXX_RXCPMAIN_H
#define CREXX_RXCPMAIN_H

#include "rxcp_types.h"
#include "rxcp_util.h"
#include "rxcp_token.h"
#include "rxcp_sym.h"
#include "rxcp_ast.h"
#include "rxcp_ctx.h"

/* Validation API */
void validate_ast(Context *context);
void rxcp_val(Context *context);
void rxcp_bvl(Context *context);

#endif //CREXX_RXCPMAIN_H
