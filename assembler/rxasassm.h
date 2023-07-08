/* REXX ASSEMBLER               */
/* The Assembler itself         */

#ifndef CREXX_RXASASSM_H
#define CREXX_RXASASSM_H

#define rxversion "crexx-f0019"

#include "rxas.h"
#include "rxasgrmr.h"

/* Queue code for the keyhole optimiser */
void rxasque0(Assembler_Context *context, Assembler_Token *instrToken);
void rxasque1(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token);
void rxasque2(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token,
              Assembler_Token *operand2Token);
void rxasque3(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token,
              Assembler_Token *operand2Token, Assembler_Token *operand3Token);
void rxasqlbl(Assembler_Context *context, Assembler_Token *labelToken);
/* Source filename */
void rxasqmfl(Assembler_Context *context, Assembler_Token *file);
/* Source Line */
void rxasqmsr(Assembler_Context *context, Assembler_Token *line, Assembler_Token *column, Assembler_Token *source);
/* Function Metadata */
void rxasqmfu(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *func, Assembler_Token *args, Assembler_Token *inliner);
/* Register Metadata */
void rxasqmre(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *reg);
/* Constant Metadata */
void rxasqmct(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *constant);
/* Clear Metadata */
void rxasqmcl(Assembler_Context *context, Assembler_Token *symbol);

/* Flush the optimiser queue */
void flushopt(Assembler_Context *context);

/* Generate code for an instructions */
void rxasgen0(Assembler_Context *context, Assembler_Token *instrToken);
void rxasgen1(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token);
void rxasgen2(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token,
              Assembler_Token *operand2Token);
void rxasgen3(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token,
              Assembler_Token *operand2Token, Assembler_Token *operand3Token);
/** Generate code for an instruction with up to three operands
 *  NULLS in the operandToken's are used to detect the number of operands */
void rxasgen(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token,
             Assembler_Token *operand2Token, Assembler_Token *operand3Token);
void rxasproc(Assembler_Context *context, Assembler_Token *funcToken, Assembler_Token *localsToken);
void rxaslabl(Assembler_Context *context, Assembler_Token *labelToken);
void rxassetg(Assembler_Context *context, Assembler_Token *globalsToken);
void rxasexre(Assembler_Context *context, Assembler_Token *registerToken, Assembler_Token *exposeToken);
void rxasexpc(Assembler_Context *context, Assembler_Token *funcToken, Assembler_Token *localsToken, Assembler_Token *exposeToken);
void rxasdecl(Assembler_Context *context, Assembler_Token *funcToken, Assembler_Token *exposeToken);
/* Source filename */
void rxasmefl(Assembler_Context *context, Assembler_Token *file);
/* Source Line */
void rxasmesr(Assembler_Context *context, Assembler_Token *line, Assembler_Token *column, Assembler_Token *source);
/* Function Metadata */
void rxasmefu(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *func, Assembler_Token *args, Assembler_Token *inliner);
/* Register Metadata */
void rxasmere(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *reg);
/* Constant Metadata */
void rxasmect(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *constant);
/* Clear Metadata */
void rxasmecl(Assembler_Context *context, Assembler_Token *symbol);

/* Backpatch, check references and free backpatch information */
void backptch(Assembler_Context *context);

/* Free Resources */
void freeasbl(Assembler_Context *context);

#endif //CREXX_RXASASSM_H
