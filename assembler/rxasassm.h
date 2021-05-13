/* REXX ASSEMBLER               */
/* The Assembler itself         */

#ifndef CREXX_RXASASSM_H
#define CREXX_RXASASSM_H

#define rxversion "cREXX-Phase-0 v0.1.0"

#include "rxas.h"
#include "rxasgrmr.h"

/** Generate code for an instructions */
void rxas_gen0(Assembler_Context *context, Token *instrToken);
void rxas_gen1(Assembler_Context *context, Token *instrToken, Token *operand1Token);
void rxas_gen2(Assembler_Context *context, Token *instrToken, Token *operand1Token,
               Token *operand2Token);
void rxas_gen3(Assembler_Context *context, Token *instrToken, Token *operand1Token,
               Token *operand2Token, Token *operand3Token);
void rxas_proc(Assembler_Context *context, Token *funcToken, Token *localsToken);
void rxas_label(Assembler_Context *context, Token *labelToken);
void rxas_setglobals(Assembler_Context *context, Token *globalsToken);

/* Backpatch, check references and free backpatch information */
void backpatch(Assembler_Context *context);

/* Free Resources */
void free_assembler(Assembler_Context *context);

#endif //CREXX_RXASASSM_H
