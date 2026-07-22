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

#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include "../binutils/include/rxdefs.h"

extern const OpInfo op_table[];

static int mnemonic_matches(const char *search, const char *full_name) {
    int i = 0;
    while (search[i] && full_name[i] && full_name[i] != '_' && 
           tolower((unsigned char)search[i]) == tolower((unsigned char)full_name[i])) {
        i++;
    }
    return (search[i] == 0 && (full_name[i] == 0 || full_name[i] == '_'));
}

void *src_instv(const char *name, const OperandType *operands, size_t operand_count) {
    int i;
    for (i = 0; op_table[i].mnemonic != NULL; i++) {
        if (!rxop_is_source_mnemonic(op_table[i].mnemonic)) continue;
        if (mnemonic_matches(name, op_table[i].mnemonic) &&
            rxop_format_matches(op_table[i].format, operands, operand_count)) {
            return (void*)&op_table[i];
        }
    }
    return NULL;
}

void *src_inst(const char* name, OperandType op1, OperandType op2, OperandType op3) {
    OperandType operands[3];
    size_t operand_count = 0;

    if (op1 != OP_NONE) operands[operand_count++] = op1;
    if (op2 != OP_NONE) operands[operand_count++] = op2;
    if (op3 != OP_NONE) operands[operand_count++] = op3;
    return src_instv(name, operands, operand_count);
}
