/* REXX ASSEMBLER               */
/* The Assembler itself         */

#ifndef CREXX_RXASASSM_H
#define CREXX_RXASASSM_H

#define rxversion "cREXX F0036"

#include "rxas.h"
#include "rxasgrmr.h"

/* Queue code for the keyhole optimiser */
void rxasque0(Assembler_Context *context, Token *instrToken);
void rxasque1(Assembler_Context *context, Token *instrToken, Token *operand1Token);
void rxasque2(Assembler_Context *context, Token *instrToken, Token *operand1Token,
              Token *operand2Token);
void rxasque3(Assembler_Context *context, Token *instrToken, Token *operand1Token,
              Token *operand2Token, Token *operand3Token);
void rxasqlbl(Assembler_Context *context, Token *labelToken);

/* Flush the optimiser queue */
void flushopt(Assembler_Context *context);

/* Generate code for an instructions */
void rxasgen0(Assembler_Context *context, Token *instrToken);
void rxasgen1(Assembler_Context *context, Token *instrToken, Token *operand1Token);
void rxasgen2(Assembler_Context *context, Token *instrToken, Token *operand1Token,
              Token *operand2Token);
void rxasgen3(Assembler_Context *context, Token *instrToken, Token *operand1Token,
              Token *operand2Token, Token *operand3Token);
/** Generate code for an instruction with up to three operands
 *  NULLS in the operandToken's are used to detect the number of operands */
void rxasgen(Assembler_Context *context, Token *instrToken, Token *operand1Token,
             Token *operand2Token, Token *operand3Token);
void rxasproc(Assembler_Context *context, Token *funcToken, Token *localsToken);
void rxaslabl(Assembler_Context *context, Token *labelToken);
void rxassetg(Assembler_Context *context, Token *globalsToken);
void rxasexre(Assembler_Context *context, Token *registerToken, Token *exposeToken);
void rxasexpc(Assembler_Context *context, Token *funcToken, Token *localsToken, Token *exposeToken);
void rxasdecl(Assembler_Context *context, Token *funcToken, Token *exposeToken);

/* Backpatch, check references and free backpatch information */
void backptch(Assembler_Context *context);

/* Free Resources */
void freeasbl(Assembler_Context *context);

#endif //CREXX_RXASASSM_H
