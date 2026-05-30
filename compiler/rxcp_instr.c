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

static int get_operand_types(OpFormat format, OperandType *types) {
    switch (format) {
        case FMT_EMPTY: return 0;
        case FMT_C: types[0] = OP_CHAR; return 1;
        case FMT_F: types[0] = OP_FLOAT; return 1;
        case FMT_I: types[0] = OP_INT; return 1;
        case FMT_I_I: types[0] = OP_INT; types[1] = OP_INT; return 2;
        case FMT_I_I_I: types[0] = OP_INT; types[1] = OP_INT; types[2] = OP_INT; return 3;
        case FMT_I_I_R: types[0] = OP_INT; types[1] = OP_INT; types[2] = OP_REG; return 3;
        case FMT_I_R: types[0] = OP_INT; types[1] = OP_REG; return 2;
        case FMT_I_R_R: types[0] = OP_INT; types[1] = OP_REG; types[2] = OP_REG; return 3;
        case FMT_L: types[0] = OP_ID; return 1;
        case FMT_L_L_R: types[0] = OP_ID; types[1] = OP_ID; types[2] = OP_REG; return 3;
        case FMT_L_P_S: types[0] = OP_ID; types[1] = OP_FUNC; types[2] = OP_STRING; return 3;
        case FMT_L_R: types[0] = OP_ID; types[1] = OP_REG; return 2;
        case FMT_L_R_I: types[0] = OP_ID; types[1] = OP_REG; types[2] = OP_INT; return 3;
        case FMT_L_R_R: types[0] = OP_ID; types[1] = OP_REG; types[2] = OP_REG; return 3;
        case FMT_L_S: types[0] = OP_ID; types[1] = OP_STRING; return 2;
        case FMT_P: types[0] = OP_FUNC; return 1;
        case FMT_P_S: types[0] = OP_FUNC; types[1] = OP_STRING; return 2;
        case FMT_R: types[0] = OP_REG; return 1;
        case FMT_R_C: types[0] = OP_REG; types[1] = OP_CHAR; return 2;
        case FMT_R_D: types[0] = OP_REG; types[1] = OP_DECIMAL; return 2;
        case FMT_R_D_R: types[0] = OP_REG; types[1] = OP_DECIMAL; types[2] = OP_REG; return 3;
        case FMT_R_F: types[0] = OP_REG; types[1] = OP_FLOAT; return 2;
        case FMT_R_F_I: types[0] = OP_REG; types[1] = OP_FLOAT; types[2] = OP_INT; return 3;
        case FMT_R_F_R: types[0] = OP_REG; types[1] = OP_FLOAT; types[2] = OP_REG; return 3;
        case FMT_R_I: types[0] = OP_REG; types[1] = OP_INT; return 2;
        case FMT_R_I_I: types[0] = OP_REG; types[1] = OP_INT; types[2] = OP_INT; return 3;
        case FMT_R_I_R: types[0] = OP_REG; types[1] = OP_INT; types[2] = OP_REG; return 3;
        case FMT_R_P: types[0] = OP_REG; types[1] = OP_FUNC; return 2;
        case FMT_R_P_R: types[0] = OP_REG; types[1] = OP_FUNC; types[2] = OP_REG; return 3;
        case FMT_R_R: types[0] = OP_REG; types[1] = OP_REG; return 2;
        case FMT_R_R_D: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_DECIMAL; return 3;
        case FMT_R_R_F: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_FLOAT; return 3;
        case FMT_R_R_I: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_INT; return 3;
        case FMT_R_R_R: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_REG; return 3;
        case FMT_R_R_S: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_STRING; return 3;
        case FMT_R_S: types[0] = OP_REG; types[1] = OP_STRING; return 2;
        case FMT_R_S_I: types[0] = OP_REG; types[1] = OP_STRING; types[2] = OP_INT; return 3;
        case FMT_R_S_R: types[0] = OP_REG; types[1] = OP_STRING; types[2] = OP_REG; return 3;
        case FMT_R_S_S: types[0] = OP_REG; types[1] = OP_STRING; types[2] = OP_STRING; return 3;
        case FMT_S: types[0] = OP_STRING; return 1;
        case FMT_S_R: types[0] = OP_STRING; types[1] = OP_REG; return 2;
        case FMT_S_S: types[0] = OP_STRING; types[1] = OP_STRING; return 2;
        case FMT_S_S_R: types[0] = OP_STRING; types[1] = OP_STRING; types[2] = OP_REG; return 3;
        default: return 0;
    }
}

void *src_inst(const char* name, OperandType op1, OperandType op2, OperandType op3) {
    int i;
    for (i = 0; op_table[i].mnemonic != NULL; i++) {
        OperandType types[3] = {OP_NONE, OP_NONE, OP_NONE};
        if (!rxop_is_source_mnemonic(op_table[i].mnemonic)) continue;
        get_operand_types(op_table[i].format, types);
        if (mnemonic_matches(name, op_table[i].mnemonic) && 
            types[0] == op1 && types[1] == op2 && types[2] == op3) {
            return (void*)&op_table[i];
        }
    }
    return NULL;
}
