/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* REXX ASSEMBLER               */
/* The Assembler itself         */

#ifndef CREXX_RXASASSM_H
#define CREXX_RXASASSM_H

#include "crexx_version.h"
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
/* Source Step */
void rxasqmstp(Assembler_Context *context, Assembler_Token *step, Assembler_Token *clause, Assembler_Token *flags,
               Assembler_Token *file, Assembler_Token *line, Assembler_Token *start, Assembler_Token *end,
               Assembler_Token *source);
void rxasqmte(Assembler_Context *context, Assembler_Token *kind, Assembler_Token *mode_mask,
              Assembler_Token *value_source, Assembler_Token *value_type, Assembler_Token *register_type,
              Assembler_Token *value_ref, Assembler_Token *source_step, Assembler_Token *clause,
              Assembler_Token *flags, Assembler_Token *symbol, Assembler_Token *resolved_name);
/* Function Metadata */
void rxasqmfu(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *func, Assembler_Token *args);
/* Register Metadata */
void rxasqmre(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *reg);
/* Constant Metadata */
void rxasqmct(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *constant);
/* Class Metadata */
void rxasqmclss(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type);
/* Attribute Metadata */
void rxasqmattr(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *reg);
/* Interface Metadata */
void rxasqmintf(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type);
/* Implements Metadata */
void rxasqmimpl(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *interface_symbol);
/* Interface Member Metadata */
void rxasqmmemb(Assembler_Context *context, Assembler_Token *owner, Assembler_Token *kind, Assembler_Token *member, Assembler_Token *type, Assembler_Token *args);
/* Inline Metadata */
void rxasqmil(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *payload);
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
/* Source Step */
void rxasmestp(Assembler_Context *context, Assembler_Token *step, Assembler_Token *clause, Assembler_Token *flags,
               Assembler_Token *file, Assembler_Token *line, Assembler_Token *start, Assembler_Token *end,
               Assembler_Token *source);
void rxasmete(Assembler_Context *context, Assembler_Token *kind, Assembler_Token *mode_mask,
              Assembler_Token *value_source, Assembler_Token *value_type, Assembler_Token *register_type,
              Assembler_Token *value_ref, Assembler_Token *source_step, Assembler_Token *clause,
              Assembler_Token *flags, Assembler_Token *symbol, Assembler_Token *resolved_name);
/* Function Metadata */
void rxasmefu(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *func, Assembler_Token *args);
/* Register Metadata */
void rxasmere(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *reg);
/* Constant Metadata */
void rxasmect(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *constant);
/* Class Metadata */
void rxasmeclss(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type);
/* Attribute Metadata */
void rxasmeattr(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *reg);
/* Interface Metadata */
void rxasmeintf(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type);
/* Implements Metadata */
void rxasmeimpl(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *interface_symbol);
/* Interface Member Metadata */
void rxasmememb(Assembler_Context *context, Assembler_Token *owner, Assembler_Token *kind, Assembler_Token *member, Assembler_Token *type, Assembler_Token *args);
/* Inline Metadata */
void rxasmeil(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *payload);
/* Clear Metadata */
void rxasmecl(Assembler_Context *context, Assembler_Token *symbol);

/* Backpatch, check references and free backpatch information */
void backptch(Assembler_Context *context);

/* Free Resources */
void freeasbl(Assembler_Context *context);

#endif //CREXX_RXASASSM_H
