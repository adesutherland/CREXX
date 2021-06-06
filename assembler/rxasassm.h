/* REXX ASSEMBLER               */
/* The Assembler itself         */

#ifndef CREXX_RXASASSM_H
#define CREXX_RXASASSM_H

#define rxversion "cREXX-Phase-0 v0.1.6-i0052"

#include "rxas.h"
#include "rxasgrmr.h"

/** Generate code for an instructions */
void rxasgen0(Assembler_Context *context, Token *instrToken);
void rxasgen1(Assembler_Context *context, Token *instrToken, Token *operand1Token);
void rxasgen2(Assembler_Context *context, Token *instrToken, Token *operand1Token,
              Token *operand2Token);
void rxasgen3(Assembler_Context *context, Token *instrToken, Token *operand1Token,
              Token *operand2Token, Token *operand3Token);
void rxasproc(Assembler_Context *context, Token *funcToken, Token *localsToken);
void rxaslabl(Assembler_Context *context, Token *labelToken);
void rxassetg(Assembler_Context *context, Token *globalsToken);

/* Backpatch, check references and free backpatch information */
void backptch(Assembler_Context *context);

/* Free Resources */
void freeasbl(Assembler_Context *context);

#endif //CREXX_RXASASSM_H
