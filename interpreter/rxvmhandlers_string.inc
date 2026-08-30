/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2026 the cREXX contributors
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

/* String RXAS handler definitions; body text is compiled by RXVM_HANDLER. */
RXVM_HANDLER(INOT_REG_INT, VM_ADVANCE(2);
            DEBUG("TRACE - INOT R%d,R%d\n", (int)REG_IDX(1), (int)op2I);
            REG_RETURN_INT(~op2I)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SAY_INT  Say op1                                                    pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SAY_INT, VM_ADVANCE(1);
            DEBUG("TRACE - SAY %d\n", (int)op1I);
            rxvm_mprintf("%" RXINTEGER_PRI "\n", op1I);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SAY_CHAR  Say op1                                                   pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SAY_CHAR, VM_ADVANCE(1);
            DEBUG("TRACE - SAY \'%c\'\n", (pc + (1))->cconst);
            rxvm_mprintf("%c\n", (pc + (1))->cconst);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SAY_FLOAT  Say op1                                                  pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SAY_FLOAT, VM_ADVANCE(1);
            DEBUG("TRACE - SAY %.15g\n", op1F);
            rxvm_mprintf("%.15g\n", op1F);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  LOAD_REG_FLOAT  Load op1 with op2                                   pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(LOAD_REG_FLOAT, VM_ADVANCE(2);
            DEBUG("TRACE - LOAD R%d,%.15g\n",(int)REG_IDX(1),op2F);
            REG_RETURN_FLOAT(op2F)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FADD_REG_REG_REG  Float Add (op1=op2+op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FADD_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - FADD R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2RF + op3RF)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FSUB_REG_REG_REG  Float Sub (op1=op2-op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */


)

RXVM_HANDLER(FSUB_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - FSUB R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2RF - op3RF)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FDIV_REG_REG_REG  Float Div (op1=op2/op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FDIV_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - FDIV R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2RF / op3RF)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FIDIV_REG_REG_REG  Integer Div for Float (op1=op2/op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FIDIV_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - FIDIV R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_FLOAT(trunc(op2RF / op3RF))
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FMULT_REG_REG_REG  Float Mult (op1=op2/op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FMULT_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - FMULT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2RF * op3RF)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FADD_REG_REG_FLOAT  Float Add (op1=op2+op3)                          pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FADD_REG_REG_FLOAT, VM_ADVANCE(3);
            DEBUG("TRACE - FADD R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_FLOAT(op2RF + op3F)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FSUB_REG_REG_FLOAT  Float Sub (op1=op2-op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FSUB_REG_REG_FLOAT, VM_ADVANCE(3);
            DEBUG("TRACE - FSUB R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_FLOAT(op2RF - op3F)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FDIV_REG_REG_FLOAT  Float Div (op1=op2/op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FDIV_REG_REG_FLOAT, VM_ADVANCE(3);
            DEBUG("TRACE - FDIV R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_FLOAT(op2RF / op3F)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FIDIV_REG_REG_FLOAT  Integer Div for Float(op1=op2/op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FIDIV_REG_REG_FLOAT, VM_ADVANCE(3);
            DEBUG("TRACE - FIDIV R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_FLOAT(trunc(op2RF / op3F))
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FMULT_REG_REG_FLOAT  Float Mult (op1=op2/op3)                       pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FMULT_REG_REG_FLOAT, VM_ADVANCE(3);
            DEBUG("TRACE - FMULT R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_FLOAT(op2RF * op3F)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FSUB_REG_FLOAT_REG  Float Sub (op1=op2-op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FSUB_REG_FLOAT_REG, VM_ADVANCE(3);
        DEBUG("TRACE - FSUB R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2F - op3RF)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FDIV_REG_FLOAT_REG  Float Div (op1=op2/op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FDIV_REG_FLOAT_REG, VM_ADVANCE(3);
        DEBUG("TRACE - FDIV R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2F / op3RF)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FIDIV_REG_FLOAT_REG  Integer Div for Float (op1=op2/op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FIDIV_REG_FLOAT_REG, VM_ADVANCE(3);
        DEBUG("TRACE - FIDIV R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
        REG_RETURN_FLOAT(trunc(op2F / op3RF))
        DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FPOW_REG_REG_FLOAT  op1=op2**op3 Float operationn                   pej 3 March 2022
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FPOW_REG_REG_FLOAT, VM_ADVANCE(3);
    DEBUG("TRACE - DIVF R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
    {
        REG_RETURN_FLOAT(pow(op2RF,op3F))
    }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FPOW_REG_FLOAT_REG  op1=op2**op3 Float operation
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FPOW_REG_FLOAT_REG, VM_ADVANCE(3);
    DEBUG("TRACE - DIVF R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
    {
        REG_RETURN_FLOAT(pow(op2F,op3RF))
    }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FPOW_REG_REG_REG  op1=op2**op3 Float operation                     pej 3 March 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FPOW_REG_REG_REG, VM_ADVANCE(3);
    {
        DEBUG("TRACE - FPOW R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2),
              (int) REG_IDX(3));
        REG_RETURN_FLOAT(pow(op2RF,op3RF))
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IPOW_REG_REG_REG  op1=op2**op2w Integer operation
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IPOW_REG_REG_REG, VM_ADVANCE(3);
        DEBUG("TRACE - IPOW R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));

        if (!rxinteger_checked_pow(op2R->int_value, op3R->int_value, &op1R->int_value)) SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
        DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IPOW_REG_REG_INT  op1=op2**op2w Integer operationn
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IPOW_REG_REG_INT, VM_ADVANCE(3);
        DEBUG("TRACE - IPOW R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);

        if (!rxinteger_checked_pow(op2R->int_value, op3I, &op1R->int_value)) SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
        DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IPOW_REG_INT_REG  op1=op2**op2w Integer operationn
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IPOW_REG_INT_REG, VM_ADVANCE(3);
        DEBUG("TRACE - IPOW R%d,%d,R%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));

        if (!rxinteger_checked_pow(op2I, op3R->int_value, &op1R->int_value)) SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
        DISPATCH;

/* ------------------------------------------------------------------------------------
 *  LINKARG_REG_REG_INT  Link args[op2+op3] to op1
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(LINKARG_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - LINKARG R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            op1R = current_locals[op2RI + op3I +
                                         current_frame->procedure->binarySpace->globals +
                                         current_frame->procedure->locals];
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  LINKARG_REG_INT  Link args[op2] to op1
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(LINKARG_REG_INT, VM_ADVANCE(2);
            DEBUG("TRACE - LINKARG R%d,%d\n", (int)REG_IDX(1), (int)op2I);
            op1R = current_locals[op2I +
                                         current_frame->procedure->binarySpace->globals +
                                         current_frame->procedure->locals];
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ITOS_REG  Set register string value from its int value
 *  -----------------------------------------------------------------------------------*/

)

RXVM_HANDLER(ITOS_REG, VM_ADVANCE(1);
            DEBUG("TRACE - ITOS R%lu\n", REG_IDX(1));
            int_to_string(&(current_frame->num_context), work1, op1R);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FTOS_REG  Set register string value from its float value
 *  -----------------------------------------------------------------------------------*/

)

RXVM_HANDLER(FTOS_REG, VM_ADVANCE(1);
            DEBUG("TRACE - FTOS R%lu\n", REG_IDX(1));
            float_to_string(&(current_frame->num_context), work1, op1R);
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ITOF_REG  Set register float value from its int value
 *  -----------------------------------------------------------------------------------*/

)

RXVM_HANDLER(ITOF_REG, VM_ADVANCE(1);
            DEBUG("TRACE - ITOF R%lu\n", REG_IDX(1));
            op1R->float_value = op1R->int_value;
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FTOI_REG  Set register int value from its float value
 *  -----------------------------------------------------------------------------------*/

)

RXVM_HANDLER(FTOI_REG, VM_ADVANCE(1);
            DEBUG("TRACE - FTOI R%lu\n", REG_IDX(1));
            int_from_float(op1R);
            if (op1R->float_value != (double)op1R->int_value) {
                SET_SIGNAL(RXSIGNAL_CONVERSION_ERROR);
            }
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FTOB_REG  Set register boolean (int set to 1 or 0) value from its float value
 *  -----------------------------------------------------------------------------------*/

)

RXVM_HANDLER(FTOB_REG, VM_ADVANCE(1);
            DEBUG("TRACE - FTOB R%lu\n", REG_IDX(1));
            int_from_float(op1R);

            if (op1R->float_value) op1R->int_value = 1;
            else op1R->int_value = 0;
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ITOB_REG  Set register boolean (int set to 1 or 0) value from its int value
 *  -----------------------------------------------------------------------------------*/

)

RXVM_HANDLER(ITOB_REG, VM_ADVANCE(1);
            DEBUG("TRACE - ITOB R%lu\n", REG_IDX(1));

            if (op1R->int_value) op1R->int_value = 1;
            else op1R->int_value = 0;
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ITOB_REG  Set register boolean (int set to 1 or 0) value from its string value
 *  -----------------------------------------------------------------------------------*/

)

RXVM_HANDLER(STOB_REG, VM_ADVANCE(1);
            DEBUG("TRACE - STOB R%lu\n", REG_IDX(1));

            if (op1R->string_length != 1) op1R->int_value = 0;
            else {
                if (op1R->string_value[0] == '1') {
                    op1R->int_value = 1;
                } else {
                    op1R->int_value = 0;
                }
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BTOI_REG  Set register integer value from its boolean value
 *  -----------------------------------------------------------------------------------*/

)

RXVM_HANDLER(BTOI_REG, VM_ADVANCE(1);
            DEBUG("TRACE - BTOI R%lu\n", REG_IDX(1));
            if (op1R->int_value) op1R->int_value = 1;
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BTOF_REG  Set register float value from its boolean value
 *  -----------------------------------------------------------------------------------*/

)

RXVM_HANDLER(BTOF_REG, VM_ADVANCE(1);
            DEBUG("TRACE - BTOF R%lu\n", REG_IDX(1));
            if (op1R->int_value) op1R->float_value = 1.0;
            else op1R->float_value = 0.0;
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BTOS_REG  Set register string value from its boolean value
 *  -----------------------------------------------------------------------------------*/

)

RXVM_HANDLER(BTOS_REG, VM_ADVANCE(1);
            DEBUG("TRACE - BTOS R%lu\n", REG_IDX(1));
            if (op1R->int_value) set_null_string(op1R,"1");
            else set_null_string(op1R,"0");
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  STOI_REG  Set register int value from its string value
 *  -----------------------------------------------------------------------------------*/

)

RXVM_HANDLER(STOI_REG, VM_ADVANCE(1);
            DEBUG("TRACE - STOI R%lu\n", REG_IDX(1));
            /* Convert a string to a integer - returns 1 on error */
            if (string2integer(&op1R->int_value, op1R->string_value, op1R->string_length)) {
                SET_SIGNAL(RXSIGNAL_CONVERSION_ERROR);
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  STOF_REG  Set register float value from its string value
 *  -----------------------------------------------------------------------------------*/

)

RXVM_HANDLER(STOF_REG, VM_ADVANCE(1);
            DEBUG("TRACE - STOF R%lu\n", REG_IDX(1));
            /* Convert a string to a float - returns 1 on error */
            if (string2float(&op1R->float_value, op1R->string_value, op1R->string_length)) {
                SET_SIGNAL(RXSIGNAL_CONVERSION_ERROR);
            }
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FFORMAT_REG_REG_REG  Set string from float use format string   pej 3. November 2021
 *  DEPRECATED - use FEXTR_REG_REG_REG
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FFORMAT_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - FFORMAT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            prep_string_buffer(op1R,SMALLEST_STRING_BUFFER_LENGTH); // Large enough for a float
            null_terminate_string_buffer(op3R);    // terminate format string explicitly, rexx vars aren't!
            finish_ascii_string_write(op1R,
                    rxvm_format_float_with_checked_format(op1R->string_value,SMALLEST_STRING_BUFFER_LENGTH,op3R->string_value,op2R->float_value));
            DISPATCH;
/* ------------------------------------------------------------------------------------
 * FEXTR_REG_REG_REG Extract float to string coefficient and decimal exponent integer
 * R3 contains the float value
 * R1 will store the coefficient as a string (or nan, inf, -inf)
 * R2 will store the decimal exponent as an integer
 * Output normalised, rounded to DBL_DIG (typically 15) decimal places (i.e. DBL_DIG-1 fractional digits), and
 * trimmed of trailing zeros
 * This instruction is designed to allow the user to format the float as they wish
 */

)

RXVM_HANDLER(FEXTR_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - FEXTR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            extract_double_decimal(&(current_frame->num_context), op1R, op2R, op3R->float_value);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  STRLOWER_REG_REG  translate string into lower case string              pej 23.10.21
 *  -----------------------------------------------------------------------------------
 */
// TODO: what to do if there is a length change of chars during translation

)

RXVM_HANDLER(STRLOWER_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - STRLOWER R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                set_value_string(op1R, op2R);
                clear_string_normalization_certificates(op1R);
                RXVM_BYTE_ONLY(
                size_t i;
                for (i = 0; i < op1R->string_length; ++i) {
                    op1R->string_value[i] =
                            (char)tolower((unsigned char)op1R->string_value[i]);
                }
                )
                RXVM_UTF8_ONLY(
                char *current;
                char *next;
                char *end;
                utf8_int32_t codepoint;
                utf8_int32_t mapped;
                REQUIRE_VALID_UTF8_REGISTER(op1R);
                current = op1R->string_value;
                end = current + op1R->string_length;
                while (current < end) {
                    next = utf8codepoint(current, &codepoint);
                    mapped = utf8lwrcodepoint(codepoint);
                    if (mapped != codepoint) {
                        /* The deliberately limited simple-case table preserves
                         * UTF-8 width, so the transform is allocation-free. */
                        utf8catcodepoint(current, mapped, (size_t)(next - current));
                    }
                    current = next;
                }
                if (op1R->string_chars == op1R->string_length)
                    mark_ascii_string_valid_count(op1R);
                )
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  STRUPPER_REG_REG  translate string into upper case string              pej 23.10.21
 *  -----------------------------------------------------------------------------------
 */
// TODO: what to do if there is a length change of chars during translation

)

RXVM_HANDLER(STRUPPER_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - STRUPPER R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                set_value_string(op1R, op2R);
                clear_string_normalization_certificates(op1R);
                RXVM_BYTE_ONLY(
                size_t i;
                for (i = 0; i < op1R->string_length; ++i) {
                    op1R->string_value[i] =
                            (char)toupper((unsigned char)op1R->string_value[i]);
                }
                )
                RXVM_UTF8_ONLY(
                char *current;
                char *next;
                char *end;
                utf8_int32_t codepoint;
                utf8_int32_t mapped;
                REQUIRE_VALID_UTF8_REGISTER(op1R);
                current = op1R->string_value;
                end = current + op1R->string_length;
                while (current < end) {
                    next = utf8codepoint(current, &codepoint);
                    mapped = utf8uprcodepoint(codepoint);
                    if (mapped != codepoint) {
                        /* See STRLOWER: simple mappings retain encoded width. */
                        utf8catcodepoint(current, mapped, (size_t)(next - current));
                    }
                    current = next;
                }
                if (op1R->string_chars == op1R->string_length)
                    mark_ascii_string_valid_count(op1R);
                )
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  STRCHAR_REG_REG_REG  String to Int op1 = op2[op3]                   pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(STRCHAR_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - STRCHAR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
                RXVM_UTF8_ONLY(
                int result;
                REQUIRE_VALID_UTF8_REGISTER(op2R);
                if (op3R->int_value < 0 ||
                    (size_t)op3R->int_value >= op2R->string_chars) {
                    SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                }
                else {
                    string_cache_seek_char(op2R, (size_t)op3R->int_value);
                    utf8codepoint(op2R->string_value + op2R->string_cache_byte_pos, &result);
                    REG_RETURN_INT(result)
                }
                )
                RXVM_BYTE_ONLY(
                if (op3R->int_value < 0 ||
                    (size_t)op3R->int_value >= op2R->string_length) {
                    SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                }
                else {
                    REG_RETURN_INT(op2R->string_value[op3R->int_value])
                }
                )
            }
            DISPATCH;
// Lazy Peter approach

)
#define setCodePointETC()       \
   {string_cache_seek_char(op2R, (size_t)op3R->int_value); \
    start = op2R->string_value + op2R->string_cache_byte_pos; \
    end = utf8codepoint(start, &ch); \
    bytelen = end - start;}
#define split2hex(chr,into)           \
    {op1R->string_value[into] = hexconst[(chr>> 4) & 0xF];   \
     op1R->string_value[into+1] = hexconst[chr & 0xF]  ; \
     op1R->string_value[into+2] = '\0';  }

/* ------------------------------------------------------------------------------------
 *  HEXCHAR_REG_REG_REG  op1 = hex(op2[op3])                       pej 04 November 2021
 *  -----------------------------------------------------------------------------------
 */

RXVM_HANDLER(HEXCHAR_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - HEXCHAR R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            {
                static const char hexconst[] = "0123456789ABCDEF";
                int ch, i, bytelen, mode;
                unsigned char bytebuf[4] = {0, 0, 0, 0};

                null_terminate_string_buffer(op1R);
                if (strstr(op1R->string_value, "UTFV1") > 0) goto hexm2;
                if (strstr(op1R->string_value, "UTFV2") > 0) goto hexm3;

                char *start, *end;
                RXVM_UTF8_ONLY(
                REQUIRE_VALID_UTF8_REGISTER(op2R);
                setCodePointETC()   //calculate Codepoint/length, etc. in a macro, character is in ch
                )
                RXVM_BYTE_ONLY(
                ch=op2R->string_value[op3R->int_value];
                )
                prep_string_buffer(op1R, 3);
                split2hex(ch, 0)                    // split the character into their 2 byte hex presentation
                finish_ascii_string_write(op1R, 2); // hex length is 2
                goto hexdispatch;
   /* ----- create full UTF8 code -------------- */
                hexm2:
                RXVM_UTF8_ONLY(
                REQUIRE_VALID_UTF8_REGISTER(op2R);
                setCodePointETC()   //calculate Codepoint/length, etc. in a macro, character is in ch
                for (i = 0; i < bytelen && i < 4; ++i) {
                    bytebuf[i] = (unsigned char) start[i];
                }
                )
                RXVM_BYTE_ONLY(
                ch=op2R->string_value[op3R->int_value];
                )
                prep_string_buffer(op1R, 9);
                for (i = 0; i < 4; ++i) {
                    unsigned char b = bytebuf[i];
                    split2hex(b, i*2)                     // split the character into their 2 byte hex presentation
                }
                finish_ascii_string_write(op1R, 8);
                goto hexdispatch;
    /* ----- add UTF8 code depending on length -------------- */
            hexm3:
                RXVM_UTF8_ONLY(
                REQUIRE_VALID_UTF8_REGISTER(op2R);
                setCodePointETC()   //calculate Codepoint/length, etc. in a macro
                )
                RXVM_BYTE_ONLY(
                ch=op2R->string_value[op3R->int_value];
                )
                prep_string_buffer(op1R, bytelen * 2 + 1);
                for (i = 0; i < bytelen; ++i) {
                    unsigned char b = start[i];
                    split2hex(b, i * 2)                     // split the character into their 2 byte hex presentation
                }
                finish_ascii_string_write(op1R, (size_t)bytelen * 2);
            }
        hexdispatch:
        DISPATCH;

/* ------------------------------------------------------------------------------------
 *  POSCHAR_REG_REG_REG  op1 position of op3 in op2                pej 05 November 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(POSCHAR_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - POSCHAR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
                rxinteger result = -1, i;
                int ch;

                REQUIRE_VALID_UTF8_REGISTER(op2R);
                RXVM_UTF8_ONLY(
                for (i = 0; i < op2R->string_chars; i++) {
                )
                RXVM_BYTE_ONLY(
                for (i = 0; i < op2R->string_length; i++) {
                )
                RXVM_UTF8_ONLY(
                    string_cache_seek_char(op2R, (size_t)i);
                    utf8codepoint(op2R->string_value + op2R->string_cache_byte_pos, &ch);
                )
                RXVM_BYTE_ONLY(
                    ch=op2R->string_value[i];
                )
                    if (ch == op3R->int_value) {
                        result = i;
                        break;
                    }
                }
                REG_RETURN_INT(result)
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BGT_ID_REG_REG  if op2>op3 goto op1                           pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BGT_ID_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - BGT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            if (current_locals[REG_IDX(2)]->int_value > current_locals[REG_IDX(3)]->int_value) {
                VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

            }
        DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BGT_ID_REG_INT  if op2>op3 goto op1                           pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BGT_ID_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - BGT 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            if (current_locals[REG_IDX(2)]->int_value > op3I) {
               VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

            }
        DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BGE_ID_REG_REG  if op2>=op3 goto op1                          pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BGE_ID_REG_REG, VM_ADVANCE(3);
       DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
       if (current_locals[REG_IDX(2)]->int_value >= current_locals[REG_IDX(3)]->int_value) {
          VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

       }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BGE_ID_REG_INT  if op2>=op3 goto op1                         pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BGE_ID_REG_INT, VM_ADVANCE(3);
        DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_locals[REG_IDX(2)]->int_value >= op3I) {
            VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

        }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BLT_ID_REG_REG  if op2<op3 goto op1                           pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BLT_ID_REG_REG, VM_ADVANCE(3);
        DEBUG("TRACE - BLT 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_locals[REG_IDX(2)]->int_value < current_locals[REG_IDX(3)]->int_value) {
            VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

        }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BLT_ID_REG_INT  if op2<op3 goto op1                           pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BLT_ID_REG_INT, VM_ADVANCE(3);
        DEBUG("TRACE - BGT 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_locals[REG_IDX(2)]->int_value < op3I) {
            VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

        }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BLE_ID_REG_REG  if op2<=op3 goto op1                          pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BLE_ID_REG_REG, VM_ADVANCE(3);
        DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_locals[REG_IDX(2)]->int_value <= current_locals[REG_IDX(3)]->int_value) {
            VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

        }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BLE_ID_REG_INT  if op2<=op3 goto op1                          pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BLE_ID_REG_INT, VM_ADVANCE(3);
        DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_locals[REG_IDX(2)]->int_value <= op3I) {
            VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

        }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BNE_ID_REG_INT  if op2!=op3 goto op1                          pej 14 September 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BNE_ID_REG_REG, VM_ADVANCE(3);
        DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_locals[REG_IDX(2)]->int_value != current_locals[REG_IDX(3)]->int_value) {
            VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

        }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BNE_ID_REG_INT  if op2!=op3 goto op1                          pej 14 September 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BNE_ID_REG_INT, VM_ADVANCE(3);
        DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_locals[REG_IDX(2)]->int_value != op3I) {
            VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

        }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BEQ_ID_REG_INT  if op2=op3 goto op1                           pej 14 September 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BEQ_ID_REG_REG, VM_ADVANCE(3);
        DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_locals[REG_IDX(2)]->int_value == current_locals[REG_IDX(3)]->int_value) {
            VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

        }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BEQ_ID_REG_INT  if op2=op3 goto op1                           pej 14 September 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BEQ_ID_REG_INT, VM_ADVANCE(3);
        DEBUG("TRACE - BGE 0x%x,R%d,%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
        if (current_locals[REG_IDX(2)]->int_value == op3I) {
            VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

        }
    DISPATCH;
 /* ------------------------------------------------------------------------------------
 *  BCT_REG_ID  dec op2; if op2>0 goto op1                           pej 26 August 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BCT_ID_REG, VM_ADVANCE(2);
            DEBUG("TRACE - BCT R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            if (!rxinteger_checked_sub(op2RI, 1, &op2R->int_value)) {
                SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
            }
            else if (op2R->int_value > 0) {
                VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

            }
        DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BCT_REG_REG_ID  dec op2, inc op3; if op2>0 goto op1              pej 26 August 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BCT_ID_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - BCT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
                rxinteger counter;
                rxinteger index;
                if (!rxinteger_checked_sub(op2RI, 1, &counter) ||
                    !rxinteger_checked_add(op3RI, 1, &index)) {
                    SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
                }
                else {
                    op2R->int_value = counter;
                    op3R->int_value = index;
                    if (counter > 0) {
                        VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

                    }
                }
            }
        DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BCF_ID_REG if op2=0 goto op1(if false) else dec op2
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BCF_ID_REG, VM_ADVANCE(2);
            DEBUG("TRACE - BCF R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            if (current_locals[REG_IDX(2)]->int_value == 0) {
                VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

            }
            else if (!rxinteger_checked_sub(op2RI, 1, &op2R->int_value)) {
                SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
            }
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BCF_ID_REG_REG  if op2=0 goto op1(if false) else dec op2 and inc op3
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BCF_ID_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - BCF R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            if (current_locals[REG_IDX(2)]->int_value == 0) {
                VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

            }
            else {
                rxinteger counter;
                rxinteger index;
                if (!rxinteger_checked_sub(op2RI, 1, &counter) ||
                    !rxinteger_checked_add(op3RI, 1, &index)) {
                    SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
                }
                else {
                    op2R->int_value = counter;
                    op3R->int_value = index;
                }
            }
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BCTNM_REG_ID  dec op2; if op2>=0 goto op1                           pej 26 August 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BCTNM_ID_REG, VM_ADVANCE(2);
            DEBUG("TRACE - BCTNM R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            if (!rxinteger_checked_sub(op2RI, 1, &op2R->int_value)) {
                SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
            }
            else if (op2R->int_value >= 0) {
                VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

            }
        DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BCTNM_REG_REG_ID  dec op2, inc op3; if op2>=0 goto op1           pej 26 August 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BCTNM_ID_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - BCTNM R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
                rxinteger counter;
                rxinteger index;
                if (!rxinteger_checked_sub(op2RI, 1, &counter) ||
                    !rxinteger_checked_add(op3RI, 1, &index)) {
                    SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
                }
                else {
                    op2R->int_value = counter;
                    op3R->int_value = index;
                    if (counter >= 0) {
                        VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

                    }
                }
            }
        DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BCTP_ID_REG  inc op2; goto op1                                     pej 11 June 2023
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BCTP_ID_REG, VM_ADVANCE(2);
            DEBUG("TRACE - BCTP R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            if (!rxinteger_checked_add(op2RI, 1, &op2R->int_value)) {
                SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
            }
            else {
                VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

            }
        DISPATCH;
 /* ------------------------------------------------------------------------------------
 *  FndBlnk REG_REG_REG  return first blank after op2[op3]          pej 27 August 2021
 *  Blanks are all unicode white spaces
 *  returned is the offset which is 0 based
 *  a negative return value means nothing found: to distinguish from offset 0
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FNDBLNK_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - FNDBLNK R%lu R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                rxinteger len;
                rxinteger result;
                int ch;
                REQUIRE_VALID_UTF8_REGISTER(op2R);
                RXVM_UTF8_ONLY(
                len = (rxinteger) op2R->string_chars;
                RXVM_ASCII_FAST_ONLY(
                if (len==op2R->string_length) {  /* it is plain ASCII */
                    if (op3R->int_value>=0) {
                        result = ascii_fwd_blank((const unsigned char *)op2R->string_value, op3R->int_value, len);
                        goto blankfound;
                    } else {
                        result = -op3R->int_value;   /* Convert to positive index */
                        if (result >= len) result = len - 1;  /* Clamp to valid range */
                        result = ascii_back_blank((unsigned char *)op2R->string_value, result, len);
                        goto blankfound;
                    }
                }
                )
                )
                RXVM_BYTE_ONLY(
                len = (rxinteger) op2R->string_length;
                )
                for (result = op3R->int_value; result < len; result++) {
                    RXVM_UTF8_ONLY(
                    string_cache_seek_char(op2R, (size_t)result);
                    utf8codepoint(op2R->string_value + op2R->string_cache_byte_pos, &ch);
                    )
                    RXVM_BYTE_ONLY(
                    ch = op2R->string_value[result];
                    )
                    if (IS_UNICODE_WHITESPACE(ch)) goto blankfound;
                }
                result = -len;
            blankfound:
             // printf("FBlank %d %d\n",fresult,result);
                REG_RETURN_INT(result)
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FndNBlnk REG_REG_REG  return first non blank after op2[op3]          pej 27 August 2021
 *  Blanks are all unicode white spaces
 *  returned is the offset which is 0 based
 *  a negative return value means nothing found: to distinguish from offset 0
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FNDNBLNK_REG_REG_REG,  VM_ADVANCE(3);
            DEBUG("TRACE - FNDNBLNK R%lu R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                rxinteger result;
                rxinteger len;
                int ch;
                REQUIRE_VALID_UTF8_REGISTER(op2R);

             RXVM_UTF8_ONLY(
             len = (rxinteger)op2R->string_chars;
             RXVM_ASCII_FAST_ONLY(
             if (len==op2R->string_length) {  /* it is plain ASCII */
                   if (op3R->int_value>=0) {
                        result = ascii_fwd_nonblank((const unsigned char *)op2R->string_value, op3R->int_value, len);
                        goto nonblankfound;
                    } else {
                        result = -op3R->int_value;   /* Convert to positive index */
                        if (result >= len) result = len - 1;  /* Clamp to valid range */
                        result = ascii_back_nonblank((unsigned char *)op2R->string_value, result, len);
                        goto nonblankfound;
                    }
             }
             )
             )
             RXVM_BYTE_ONLY(
                len = (rxinteger)op2R->string_length;
             )
                result = op3R->int_value;

                if (result >= 0) {  // FORWARD SEARCH
                    for (; result < len; result++) {
                        RXVM_UTF8_ONLY(
                        string_cache_seek_char(op2R, (size_t)result);
                        utf8codepoint(op2R->string_value + op2R->string_cache_byte_pos, &ch);
                        )
                        RXVM_BYTE_ONLY(
                        ch = op2R->string_value[result];
                        )
                        if (!IS_UNICODE_WHITESPACE(ch)) goto nonblankfound;
                    }
                    result = -len;  // Not found in forward scan
                } else { // REVERSE SEARCH
                    result = -result;  // Convert to positive index
                    if (result >= len) result = len - 1;  // Clamp to valid range
                    for (; result >= 0; result--) {
                        RXVM_UTF8_ONLY(
                        string_cache_seek_char(op2R, (size_t)result);
                        utf8codepoint(op2R->string_value + op2R->string_cache_byte_pos, &ch);
                        )
                        RXVM_BYTE_ONLY(
                        ch = op2R->string_value[result];
                        )
                        if (!IS_UNICODE_WHITESPACE(ch)) goto nonblankfound;
                    }
                    result = -1;  // Not found in reverse scan
                }
                nonblankfound:
         //       printf("FnBLANK %d %d\n",fresult,result);
                REG_RETURN_INT(result)
            }
            DISPATCH;

 /* ------------------------------------------------------------------------------------
  *  GETBYTE_REG_REG_REG  Int op1 = op2[op3]                             pej 19 Oct 2021
  *  -----------------------------------------------------------------------------------
  */

)

RXVM_HANDLER(GETBYTE_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - GETBYTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));

    if (op3R->int_value < 0 || (size_t)op3R->int_value >= op2R->binary_length) {
        REG_RETURN_INT(-1)
    }
    else {
        REG_RETURN_INT((unsigned char)op2R->binary_value[op3R->int_value])
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BLEN_REG_REG  Int op1 = byte length of op2
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BLEN_REG_REG, VM_ADVANCE(2);
    DEBUG("TRACE - BLEN R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
    REG_RETURN_INT((rxinteger)op2R->binary_length)
    DISPATCH;


)

RXVM_HANDLER(BLEN_REG_BINARY, VM_ADVANCE(2);
    DEBUG("TRACE - BLEN R%d,binary[%zu]\n", (int)REG_IDX(1), (size_t)(pc + 2)->index);
    REG_RETURN_INT((rxinteger)op2S->string_len)
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BRESIZE_REG_REG  resize op1 to op2 bytes, zero-filling growth
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BRESIZE_REG_REG, VM_ADVANCE(2);
    DEBUG("TRACE - BRESIZE R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
    if (op2R->int_value < 0 || (uintmax_t)op2R->int_value > (uintmax_t)SIZE_MAX) {
        SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
    }
    else {
        size_t old_length = rxvm_value_native_ops(op1R) ? 0 : op1R->binary_length;
        size_t new_length = (size_t)op2R->int_value;
        if (reserve_binary_buffer(op1R, new_length) != 0) {
            SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
        }
        else {
            if (new_length > old_length) {
                memset(op1R->binary_value + old_length, 0, new_length - old_length);
            }
            op1R->binary_length = new_length;
            clear_vm_private_flags(op1R);
        }
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BCLEAR_REG  clear binary payload length, preserving ordinary capacity
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BCLEAR_REG, VM_ADVANCE(1);
    DEBUG("TRACE - BCLEAR R%d\n", (int)REG_IDX(1));
    if (rxvm_value_native_ops(op1R)) {
        clear_binary_payload(op1R);
    }
    else {
        op1R->binary_length = 0;
        clear_vm_private_flags(op1R);
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BFILL_REG_REG  fill all current binary bytes in op1 with byte op2
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BFILL_REG_REG, VM_ADVANCE(2);
    DEBUG("TRACE - BFILL R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
    if (op2R->int_value < 0 || op2R->int_value > 255) {
        SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
    }
    else {
        if (op1R->binary_length) {
            memset(op1R->binary_value, (unsigned char)op2R->int_value, op1R->binary_length);
        }
        clear_vm_private_flags(op1R);
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BCOPY_REG_REG_REG / BCOPY_REG_BINARY_REG  copy op1 length bytes from source at op3
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BCOPY_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BCOPY R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t length = rxvm_value_native_ops(op1R) ? 0 : op1R->binary_length;
        size_t offset;

        if (!rxvm_binary_range(op2R, op3R->int_value, length, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else if (reserve_binary_buffer(op1R, length) != 0) {
            SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
        }
        else {
            op1R->binary_length = length;
            if (length) memmove(op1R->binary_value, op2R->binary_value + offset, length);
            clear_vm_private_flags(op1R);
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BCOPY_REG_BINARY_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BCOPY R%d,binary[%zu],R%d\n",
          (int)REG_IDX(1), (size_t)(pc + 2)->index, (int)REG_IDX(3));
    {
        string_constant *source = op2S;
        size_t length = rxvm_value_native_ops(op1R) ? 0 : op1R->binary_length;
        size_t offset;

        if (!rxvm_memory_range(source->string_len, op3R->int_value, length, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else if (reserve_binary_buffer(op1R, length) != 0) {
            SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
        }
        else {
            op1R->binary_length = length;
            if (length) memcpy(op1R->binary_value, source->string + offset, length);
            clear_vm_private_flags(op1R);
        }
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  Strict typed little-endian binary reads
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BGETU8_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETU8 R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        if (!rxvm_binary_range(op2R, op3R->int_value, 1, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            REG_RETURN_INT((rxinteger)rxvm_binary_read_le(op2R, offset, 1))
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BGETU8_REG_BINARY_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETU8 R%d,binary[%zu],R%d\n",
          (int)REG_IDX(1), (size_t)(pc + 2)->index, (int)REG_IDX(3));
    {
        string_constant *source = op2S;
        size_t offset;
        if (!rxvm_memory_range(source->string_len, op3R->int_value, 1, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            REG_RETURN_INT((rxinteger)rxvm_binary_read_le_bytes((const unsigned char *)source->string, offset, 1))
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BGETI8_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETI8 R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        if (!rxvm_binary_range(op2R, op3R->int_value, 1, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            REG_RETURN_INT(rxvm_sign_extend_le_value(rxvm_binary_read_le(op2R, offset, 1), 8))
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BGETI8_REG_BINARY_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETI8 R%d,binary[%zu],R%d\n",
          (int)REG_IDX(1), (size_t)(pc + 2)->index, (int)REG_IDX(3));
    {
        string_constant *source = op2S;
        size_t offset;
        if (!rxvm_memory_range(source->string_len, op3R->int_value, 1, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            REG_RETURN_INT(rxvm_sign_extend_le_value(rxvm_binary_read_le_bytes((const unsigned char *)source->string, offset, 1), 8))
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BGETU16_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETU16 R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        if (!rxvm_binary_range(op2R, op3R->int_value, 2, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            REG_RETURN_INT((rxinteger)rxvm_binary_read_le(op2R, offset, 2))
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BGETU16_REG_BINARY_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETU16 R%d,binary[%zu],R%d\n",
          (int)REG_IDX(1), (size_t)(pc + 2)->index, (int)REG_IDX(3));
    {
        string_constant *source = op2S;
        size_t offset;
        if (!rxvm_memory_range(source->string_len, op3R->int_value, 2, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            REG_RETURN_INT((rxinteger)rxvm_binary_read_le_bytes((const unsigned char *)source->string, offset, 2))
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BGETI16_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETI16 R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        if (!rxvm_binary_range(op2R, op3R->int_value, 2, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            REG_RETURN_INT(rxvm_sign_extend_le_value(rxvm_binary_read_le(op2R, offset, 2), 16))
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BGETI16_REG_BINARY_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETI16 R%d,binary[%zu],R%d\n",
          (int)REG_IDX(1), (size_t)(pc + 2)->index, (int)REG_IDX(3));
    {
        string_constant *source = op2S;
        size_t offset;
        if (!rxvm_memory_range(source->string_len, op3R->int_value, 2, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            REG_RETURN_INT(rxvm_sign_extend_le_value(rxvm_binary_read_le_bytes((const unsigned char *)source->string, offset, 2), 16))
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BGETU32_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETU32 R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        uint32_t result;
        if (!rxvm_binary_range(op2R, op3R->int_value, 4, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            result = (uint32_t)rxvm_binary_read_le(op2R, offset, 4);
            if (!rxvm_uint32_fits_rxinteger(result)) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            else {
                REG_RETURN_INT((rxinteger)result)
            }
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BGETU32_REG_BINARY_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETU32 R%d,binary[%zu],R%d\n",
          (int)REG_IDX(1), (size_t)(pc + 2)->index, (int)REG_IDX(3));
    {
        string_constant *source = op2S;
        size_t offset;
        uint32_t result;
        if (!rxvm_memory_range(source->string_len, op3R->int_value, 4, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            result = (uint32_t)rxvm_binary_read_le_bytes((const unsigned char *)source->string, offset, 4);
            if (!rxvm_uint32_fits_rxinteger(result)) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            else {
                REG_RETURN_INT((rxinteger)result)
            }
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BGETI32_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETI32 R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        if (!rxvm_binary_range(op2R, op3R->int_value, 4, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            REG_RETURN_INT(rxvm_sign_extend_le_value(rxvm_binary_read_le(op2R, offset, 4), 32))
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BGETI32_REG_BINARY_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETI32 R%d,binary[%zu],R%d\n",
          (int)REG_IDX(1), (size_t)(pc + 2)->index, (int)REG_IDX(3));
    {
        string_constant *source = op2S;
        size_t offset;
        if (!rxvm_memory_range(source->string_len, op3R->int_value, 4, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            REG_RETURN_INT(rxvm_sign_extend_le_value(rxvm_binary_read_le_bytes((const unsigned char *)source->string, offset, 4), 32))
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BGETF64_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETF64 R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        if (sizeof(double) != sizeof(uint64_t)) {
            SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "f64 requires 64-bit double");
        }
        else if (!rxvm_binary_range(op2R, op3R->int_value, 8, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            REG_RETURN_FLOAT(rxvm_binary_read_f64_le(op2R, offset))
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BGETF64_REG_BINARY_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETF64 R%d,binary[%zu],R%d\n",
          (int)REG_IDX(1), (size_t)(pc + 2)->index, (int)REG_IDX(3));
    {
        string_constant *source = op2S;
        size_t offset;
        if (sizeof(double) != sizeof(uint64_t)) {
            SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "f64 requires 64-bit double");
        }
        else if (!rxvm_memory_range(source->string_len, op3R->int_value, 8, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            REG_RETURN_FLOAT(rxvm_binary_read_f64_le_bytes((const unsigned char *)source->string, offset))
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BGETI64_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETI64 R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        rxinteger result;
        if (!rxvm_binary_range(op2R, op3R->int_value, 8, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else if (!rxvm_i64_raw_to_rxinteger(rxvm_binary_read_le(op2R, offset, 8), &result)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            REG_RETURN_INT(result)
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BGETI64_REG_BINARY_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETI64 R%d,binary[%zu],R%d\n",
          (int)REG_IDX(1), (size_t)(pc + 2)->index, (int)REG_IDX(3));
    {
        string_constant *source = op2S;
        size_t offset;
        rxinteger result;
        if (!rxvm_memory_range(source->string_len, op3R->int_value, 8, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else if (!rxvm_i64_raw_to_rxinteger(rxvm_binary_read_le_bytes((const unsigned char *)source->string, offset, 8), &result)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            REG_RETURN_INT(result)
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BGETF32_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETF32 R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        if (sizeof(float) != sizeof(uint32_t)) {
            SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "f32 requires 32-bit float");
        }
        else if (!rxvm_binary_range(op2R, op3R->int_value, 4, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            REG_RETURN_FLOAT(rxvm_binary_read_f32_le_bytes((const unsigned char *)op2R->binary_value, offset))
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BGETF32_REG_BINARY_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETF32 R%d,binary[%zu],R%d\n",
          (int)REG_IDX(1), (size_t)(pc + 2)->index, (int)REG_IDX(3));
    {
        string_constant *source = op2S;
        size_t offset;
        if (sizeof(float) != sizeof(uint32_t)) {
            SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "f32 requires 32-bit float");
        }
        else if (!rxvm_memory_range(source->string_len, op3R->int_value, 4, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            REG_RETURN_FLOAT(rxvm_binary_read_f32_le_bytes((const unsigned char *)source->string, offset))
        }
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  Strict typed little-endian binary writes
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BSETU8_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BSETU8 R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        if (!rxvm_binary_range(op1R, op2R->int_value, 1, &offset) ||
            op3R->int_value < 0 || op3R->int_value > UINT8_MAX) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            rxvm_binary_write_le(op1R, offset, 1, (uint64_t)op3R->int_value);
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BSETI8_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BSETI8 R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        if (!rxvm_binary_range(op1R, op2R->int_value, 1, &offset) ||
            op3R->int_value < INT8_MIN || op3R->int_value > INT8_MAX) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            rxvm_binary_write_le(op1R, offset, 1, (uint64_t)((uint8_t)op3R->int_value));
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BSETU16_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BSETU16 R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        if (!rxvm_binary_range(op1R, op2R->int_value, 2, &offset) ||
            op3R->int_value < 0 || op3R->int_value > UINT16_MAX) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            rxvm_binary_write_le(op1R, offset, 2, (uint64_t)op3R->int_value);
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BSETI16_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BSETI16 R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        if (!rxvm_binary_range(op1R, op2R->int_value, 2, &offset) ||
            op3R->int_value < INT16_MIN || op3R->int_value > INT16_MAX) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            rxvm_binary_write_le(op1R, offset, 2, (uint64_t)((uint16_t)op3R->int_value));
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BSETU32_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BSETU32 R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        if (!rxvm_binary_range(op1R, op2R->int_value, 4, &offset) ||
            op3R->int_value < 0 || (uintmax_t)op3R->int_value > UINT32_MAX) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            rxvm_binary_write_le(op1R, offset, 4, (uint64_t)op3R->int_value);
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BSETI32_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BSETI32 R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        if (!rxvm_binary_range(op1R, op2R->int_value, 4, &offset) ||
            op3R->int_value < INT32_MIN || op3R->int_value > INT32_MAX) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            rxvm_binary_write_le(op1R, offset, 4, (uint64_t)((uint32_t)op3R->int_value));
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BSETF64_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BSETF64 R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        if (sizeof(double) != sizeof(uint64_t)) {
            SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "f64 requires 64-bit double");
        }
        else if (!rxvm_binary_range(op1R, op2R->int_value, 8, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else if (rxvm_binary_write_f64_le(op1R, offset, op3R->float_value) != 0) {
            SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "f64 requires 64-bit double");
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BSETI64_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BSETI64 R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        if (!rxvm_binary_range(op1R, op2R->int_value, 8, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            rxvm_binary_write_le(op1R, offset, 8, (uint64_t)op3R->int_value);
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BSETF32_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BSETF32 R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        if (sizeof(float) != sizeof(uint32_t)) {
            SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "f32 requires 32-bit float");
        }
        else if (!rxvm_binary_range(op1R, op2R->int_value, 4, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else if (rxvm_binary_write_f32_le(op1R, offset, op3R->float_value) != 0) {
            SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "f32 requires 32-bit float");
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BCHECKRANGE_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BCHECKRANGE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    if (op3R->int_value < 0 ||
        (uintmax_t)op3R->int_value > (uintmax_t)SIZE_MAX ||
        !rxvm_binary_range(op1R, op2R->int_value, (size_t)op3R->int_value, 0)) {
        SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  Host-native packed numeric item access. Bounds are checked in items;
 *  memcpy keeps the scalar load/store strict-aliasing safe.
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(PGETI_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - PGETI R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        rxinteger result;
        if (!rxvm_packed_range(op2R, op3R->int_value, sizeof(result), &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            memcpy(&result, op2R->binary_value + offset, sizeof(result));
            REG_RETURN_INT(result)
        }
    }
    DISPATCH;


)

RXVM_HANDLER(PSETI_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - PSETI R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        rxinteger source = op3R->int_value;
        if (!rxvm_packed_range(op1R, op2R->int_value, sizeof(source), &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            memcpy(op1R->binary_value + offset, &source, sizeof(source));
        }
    }
    DISPATCH;


)

RXVM_HANDLER(PGETF_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - PGETF R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        double result;
        if (!rxvm_packed_range(op2R, op3R->int_value, sizeof(result), &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            memcpy(&result, op2R->binary_value + offset, sizeof(result));
            REG_RETURN_FLOAT(result)
        }
    }
    DISPATCH;


)

RXVM_HANDLER(PSETF_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - PSETF R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        double source = op3R->float_value;
        if (!rxvm_packed_range(op1R, op2R->int_value, sizeof(source), &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            memcpy(op1R->binary_value + offset, &source, sizeof(source));
        }
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  Binary memory strings, moves, and zero-copy compares
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BGETS_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETS R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        size_t length;
        if (!rxvm_find_nul_field((const unsigned char *)op2R->binary_value,
                                 op2R->binary_length, op3R->int_value, &offset, &length)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else if (set_string_validated(op1R, op2R->binary_value + offset, length) != 0) {
            SET_SIGNAL(RXSIGNAL_UNICODE_ERROR);
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BGETS_REG_BINARY_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BGETS R%d,binary[%zu],R%d\n",
          (int)REG_IDX(1), (size_t)(pc + 2)->index, (int)REG_IDX(3));
    {
        string_constant *source = op2S;
        size_t offset;
        size_t length;
        if (!rxvm_find_nul_field((const unsigned char *)source->string,
                                 source->string_len, op3R->int_value, &offset, &length)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else if (set_string_validated(op1R, source->string + offset, length) != 0) {
            SET_SIGNAL(RXSIGNAL_UNICODE_ERROR);
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BSETS_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BSETS R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        size_t width;
        if (
            !rxvm_checked_size_add(op3R->string_length, 1, &width) ||
            !rxvm_binary_range(op1R, op2R->int_value, width, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            if (op3R->string_length) memcpy(op1R->binary_value + offset, op3R->string_value, op3R->string_length);
            op1R->binary_value[offset + op3R->string_length] = 0;
            clear_vm_private_flags(op1R);
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BSETS_REG_REG_STRING, VM_ADVANCE(3);
    DEBUG("TRACE - BSETS R%d,R%d,string[%zu]\n",
          (int)REG_IDX(1), (int)REG_IDX(2), (size_t)(pc + 3)->index);
    {
        size_t offset;
        size_t width;
        if (op3S->string_len == SIZE_MAX || !rxvm_checked_size_add(op3S->string_len, 1, &width) ||
            !rxvm_binary_range(op1R, op2R->int_value, width, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            if (op3S->string_len) memcpy(op1R->binary_value + offset, op3S->string, op3S->string_len);
            op1R->binary_value[offset + op3S->string_len] = 0;
            clear_vm_private_flags(op1R);
        }
    }
    DISPATCH;


)

RXVM_HANDLER(SGET_REG_STRING_REG, VM_ADVANCE(3);
    DEBUG("TRACE - SGET R%d,string[%zu],R%d\n",
          (int)REG_IDX(1), (size_t)(pc + 2)->index, (int)REG_IDX(3));
    {
        size_t requested;
        size_t offset;
        size_t byte_length;
        int slice_rc;
        RXVM_UTF8_ONLY(
        if (!has_utf8_valid_count_or_empty(op1R)) refresh_utf8_flags(op1R);
        if (!has_utf8_valid_count_or_empty(op1R)) {
            SET_SIGNAL(RXSIGNAL_UNICODE_ERROR);
            DISPATCH;
        }
        requested = op1R->string_chars;
        )
        RXVM_BYTE_ONLY(
        requested = op1R->string_length;
        )
        slice_rc = rxvm_string_const_slice(op2S, op3R->int_value, requested, &offset, &byte_length);
        if (slice_rc == 0) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else if (slice_rc < 0) {
            SET_SIGNAL(RXSIGNAL_UNICODE_ERROR);
        }
        else if (set_string_validated(op1R, op2S->string + offset, byte_length) != 0) {
            SET_SIGNAL(RXSIGNAL_UNICODE_ERROR);
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BMOVE_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BMOVE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t source_offset;
        size_t target_offset;
        size_t length;
        if (op1R == op2R) {
            SET_SIGNAL(RXSIGNAL_INVALID_ARGUMENTS);
        }
        else if (!rxvm_rxinteger_to_size(op3R->int_value, &length) ||
                 !rxvm_binary_range(op2R, op2R->int_value, length, &source_offset) ||
                 !rxvm_binary_range(op1R, op1R->int_value, length, &target_offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            if (length) memmove(op1R->binary_value + target_offset, op2R->binary_value + source_offset, length);
            clear_vm_private_flags(op1R);
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BMEMMOVE_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BMEMMOVE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t source_offset;
        size_t target_offset;
        size_t length;
        if (!rxvm_rxinteger_to_size(op3R->int_value, &length) ||
            !rxvm_binary_range(op1R, op1R->int_value, length, &source_offset) ||
            !rxvm_binary_range(op1R, op2R->int_value, length, &target_offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            if (length) memmove(op1R->binary_value + target_offset, op1R->binary_value + source_offset, length);
            clear_vm_private_flags(op1R);
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BCMPB_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BCMPB R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        if (!rxvm_binary_range(op2R, op1R->int_value, op3R->binary_length, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            int result = rxvm_compare_bytes((const unsigned char *)op2R->binary_value + offset, op3R->binary_length,
                                            (const unsigned char *)op3R->binary_value, op3R->binary_length);
            REG_RETURN_INT(result)
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BCMPB_REG_BINARY_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BCMPB R%d,binary[%zu],R%d\n",
          (int)REG_IDX(1), (size_t)(pc + 2)->index, (int)REG_IDX(3));
    {
        string_constant *source = op2S;
        size_t offset;
        if (!rxvm_memory_range(source->string_len, op1R->int_value, op3R->binary_length, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            int result = rxvm_compare_bytes((const unsigned char *)source->string + offset, op3R->binary_length,
                                            (const unsigned char *)op3R->binary_value, op3R->binary_length);
            REG_RETURN_INT(result)
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BCMPB_REG_REG_BINARY, VM_ADVANCE(3);
    DEBUG("TRACE - BCMPB R%d,R%d,binary[%zu]\n",
          (int)REG_IDX(1), (int)REG_IDX(2), (size_t)(pc + 3)->index);
    {
        string_constant *needle = op3S;
        size_t offset;
        if (!rxvm_binary_range(op2R, op1R->int_value, needle->string_len, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            int result = rxvm_compare_bytes((const unsigned char *)op2R->binary_value + offset, needle->string_len,
                                            (const unsigned char *)needle->string, needle->string_len);
            REG_RETURN_INT(result)
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BCMPB_REG_BINARY_BINARY, VM_ADVANCE(3);
    DEBUG("TRACE - BCMPB R%d,binary[%zu],binary[%zu]\n",
          (int)REG_IDX(1), (size_t)(pc + 2)->index, (size_t)(pc + 3)->index);
    {
        string_constant *source = op2S;
        string_constant *needle = op3S;
        size_t offset;
        if (!rxvm_memory_range(source->string_len, op1R->int_value, needle->string_len, &offset)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            int result = rxvm_compare_bytes((const unsigned char *)source->string + offset, needle->string_len,
                                            (const unsigned char *)needle->string, needle->string_len);
            REG_RETURN_INT(result)
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BCMPS_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BCMPS R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        size_t offset;
        size_t length;
        if (!rxvm_find_nul_field((const unsigned char *)op2R->binary_value,
                                 op2R->binary_length, op1R->int_value, &offset, &length)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else if (!rxvm_binary_field_is_valid_utf8(op2R->binary_value + offset, length)) {
            SET_SIGNAL(RXSIGNAL_UNICODE_ERROR);
        }
        else {
            int result = rxvm_compare_bytes((const unsigned char *)op2R->binary_value + offset, length,
                                            (const unsigned char *)op3R->string_value, op3R->string_length);
            REG_RETURN_INT(result)
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BCMPS_REG_REG_STRING, VM_ADVANCE(3);
    DEBUG("TRACE - BCMPS R%d,R%d,string[%zu]\n",
          (int)REG_IDX(1), (int)REG_IDX(2), (size_t)(pc + 3)->index);
    {
        size_t offset;
        size_t length;
        if (!rxvm_find_nul_field((const unsigned char *)op2R->binary_value,
                                 op2R->binary_length, op1R->int_value, &offset, &length)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else if (!rxvm_binary_field_is_valid_utf8(op2R->binary_value + offset, length)) {
            SET_SIGNAL(RXSIGNAL_UNICODE_ERROR);
        }
        else {
            int result = rxvm_compare_bytes((const unsigned char *)op2R->binary_value + offset, length,
                                            (const unsigned char *)op3S->string, op3S->string_len);
            REG_RETURN_INT(result)
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BCMPS_REG_BINARY_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BCMPS R%d,binary[%zu],R%d\n",
          (int)REG_IDX(1), (size_t)(pc + 2)->index, (int)REG_IDX(3));
    {
        string_constant *source = op2S;
        size_t offset;
        size_t length;
        if (!rxvm_find_nul_field((const unsigned char *)source->string,
                                 source->string_len, op1R->int_value, &offset, &length)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else if (!rxvm_binary_field_is_valid_utf8(source->string + offset, length)) {
            SET_SIGNAL(RXSIGNAL_UNICODE_ERROR);
        }
        else {
            int result = rxvm_compare_bytes((const unsigned char *)source->string + offset, length,
                                            (const unsigned char *)op3R->string_value, op3R->string_length);
            REG_RETURN_INT(result)
        }
    }
    DISPATCH;


)

RXVM_HANDLER(BCMPS_REG_BINARY_STRING, VM_ADVANCE(3);
    DEBUG("TRACE - BCMPS R%d,binary[%zu],string[%zu]\n",
          (int)REG_IDX(1), (size_t)(pc + 2)->index, (size_t)(pc + 3)->index);
    {
        string_constant *source = op2S;
        size_t offset;
        size_t length;
        if (!rxvm_find_nul_field((const unsigned char *)source->string,
                                 source->string_len, op1R->int_value, &offset, &length)) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else if (!rxvm_binary_field_is_valid_utf8(source->string + offset, length)) {
            SET_SIGNAL(RXSIGNAL_UNICODE_ERROR);
        }
        else {
            int result = rxvm_compare_bytes((const unsigned char *)source->string + offset, length,
                                            (const unsigned char *)op3S->string, op3S->string_len);
            REG_RETURN_INT(result)
        }
    }
    DISPATCH;

    /* ------------------------------------------------------------------------------------
     *  SETBYTE_REG_REG_REG  op1[op2] = op3
     *  -----------------------------------------------------------------------------------
     */

)

RXVM_HANDLER(SETBYTE_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - SETBYTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    if (op2R->int_value < 0 || (size_t)op2R->int_value >= op1R->binary_length ||
        op3R->int_value < 0 || op3R->int_value > 255) {
        SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
    }
    else {
        op1R->binary_value[(size_t)op2R->int_value] = (char)(unsigned char)op3R->int_value;
        clear_vm_private_flags(op1R);
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BCONCAT_REG_REG_REG  op1 = op2 || op3
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BCONCAT_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BCONCAT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    if (concat_binary(op1R, op2R, op3R) != 0) {
        SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BAPPEND_REG_REG  op1 = op1 || op2
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BAPPEND_REG_REG, VM_ADVANCE(2);
    DEBUG("TRACE - BAPPEND R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
    if (append_binary_value(op1R, op2R) != 0) {
        SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BSLICE_REG_REG_REG_REG  op1 = op2[op3..op3 + op4)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BSLICE_REG_REG_REG_REG, VM_ADVANCE(4);
    DEBUG("TRACE - BSLICE R%d,R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2),
          (int)REG_IDX(3), (int)REG_IDX(4));
    if (op4R->int_value < 0) {
        SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
    }
    else if (slice_binary(op1R, op2R,
                          op3R->int_value < 0 ? 0u : (size_t)op3R->int_value,
                          (size_t)op4R->int_value) != 0) {
        SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BUPDATE_REG_REG_REG  overlay op3 into op1 at byte offset op2
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BUPDATE_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - BUPDATE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    if (op2R->int_value < 0 || (size_t)op2R->int_value > op1R->binary_length) {
        SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
    }
    else {
        size_t offset = (size_t)op2R->int_value;
        if (op3R->binary_length > op1R->binary_length - offset) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            if (op3R->binary_length) {
                memmove(op1R->binary_value + offset, op3R->binary_value, op3R->binary_length);
            }
            clear_vm_private_flags(op1R);
        }
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  STOBIN_REG  op1.binary = op1.string bytes
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(STOBIN_REG, VM_ADVANCE(1);
    DEBUG("TRACE - STOBIN R%d\n", (int)REG_IDX(1));
    if (set_binary(op1R, op1R->string_value, op1R->string_length) != 0) {
        SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BINTOS_REG  op1.string = op1.binary bytes, validated as UTF-8
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BINTOS_REG, VM_ADVANCE(1);
    DEBUG("TRACE - BINTOS R%d\n", (int)REG_IDX(1));
    if (!rxvm_value_string_metric_fits(op1R->binary_length)) {
        SET_SIGNAL_MSG(RXSIGNAL_OUT_OF_RANGE,
                       "Binary is too large for a string value");
    }
    else {
    RXVM_UTF8_ONLY(
    {
        size_t chars = 0;
        if (validate_utf8_bytes(op1R->binary_value, op1R->binary_length, &chars) != 0) {
            SET_SIGNAL_MSG(RXSIGNAL_UNICODE_ERROR, "Invalid UTF-8 in binary-to-string conversion");
        } else {
            set_string(op1R,
                       op1R->binary_value ? op1R->binary_value : "",
                       op1R->binary_length);
            rxvm_value_set_string_chars_known(op1R, chars);
            mark_utf8_valid_count(op1R);
        }
    }
    )
    RXVM_BYTE_ONLY(
    set_string(op1R,
               op1R->binary_value ? op1R->binary_value : "",
               op1R->binary_length);
    )
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  CONCCHAR_REG_REG_REG  op1=op2[op3]                                pej 27 August 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(CONCCHAR_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - CONCCHAR R%lu R%lu R%lu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                rxinteger temp = op3R->int_value;   // save offset, we misuse v3 later
                RXVM_UTF8_ONLY(
                int ch;
                REQUIRE_VALID_UTF8_REGISTER(op2R);
                string_cache_seek_char(op2R, (size_t)op3R->int_value);
                utf8codepoint(op2R->string_value + op2R->string_cache_byte_pos, &ch);
                op3R->int_value = ch;
                )
                RXVM_BYTE_ONLY(
                op3R->int_value=op2R->string_value[op3R->int_value - 1];
                )
                string_concat_char(op1R, op3R);
                op3R->int_value = temp;   // restore original v3
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  TRANSCHAR_REG_REG_REG  replace op1 if it is in op3-list by char in op2-list pej 7 November 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(TRANSCHAR_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - TRANSCHAR R%lu R%lu R%lu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                rxinteger val = op1R->int_value;
                rxinteger len, i;
                int ch;

                REQUIRE_VALID_UTF8_REGISTER(op2R);
                REQUIRE_VALID_UTF8_REGISTER(op3R);
                GETSTRLEN(len, op3R)

                for (i = 0; i < len; i++) {
                    GETSTRCHAR(ch, op3R, i)
                    if (val == ch) {
                        GETSTRCHAR(ch, op2R, i)
                        val = ch;
                        break;
                    }
                }
                REG_RETURN_INT(val)
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  DROPCHAR_REG_REG_REG  removes characters contained in op3-list pej 19 November 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DROPCHAR_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - DROPCHAR R%lu R%lu R%lu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                rxinteger i, len1, len2;
                int found;
                int ch;
                REQUIRE_VALID_UTF8_REGISTER(op2R);
                REQUIRE_VALID_UTF8_REGISTER(op3R);
                GETSTRLEN(len1, op2R)
                GETSTRLEN(len2, op3R)
                if (len2 == 0) len2 = (rxinteger) op3R->string_length;
                for (i = 0; i < len1; i++) {
                    rxinteger j;
                    GETSTRCHAR(ch, op2R, i)
                    op2R->int_value = ch;
                    found = 0;
                    for (j = 0; j < len2; j++) {
                        GETSTRCHAR(ch, op3R, j)
                        op3R->int_value = ch;
                        if (op2R->int_value == op3R->int_value) {
                            found = 1;  // found drop char
                            break;
                        }
                    }
                    if (found == 1) continue;
                    string_concat_char(op1R, op2R);
                }
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SUBSTRING_REG_REG_REG_REG op1 = op2[op3..op3 + op4) by character
 * ----------------------------------------------------------------------------------- */


)

RXVM_HANDLER(SUBSTRING_REG_REG_REG_REG, VM_ADVANCE(4);
            DEBUG("TRACE - SUBSTRING R%lu R%lu R%lu R%lu\n", REG_IDX(1), REG_IDX(2),
                  REG_IDX(3), REG_IDX(4));
            {
                rxinteger start = op3R->int_value;
                rxinteger length = op4R->int_value;
                REQUIRE_VALID_UTF8_REGISTER(op2R);
                if (length <= 0) {
                    PUTSTRLEN(op1R, 0);
                } else {
                    if (string_slice_at(op1R, op2R,
                                        start < 0 ? 0u : (size_t)start,
                                        (size_t)length) != 0) {
                        SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
                    }
                }
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
*  SUBSTCUT_REG_REG_REG op1=substr(op1,,op2) cuts off after op2   pej 13 November 2021
*  -----------------------------------------------------------------------------------
*/

)

RXVM_HANDLER(SUBSTCUT_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - SUBSTCUT R%lu R%lu\n", REG_IDX(1), REG_IDX(2));

        /* input parm op2 defines how many leading codepoints remain in the string */
           REQUIRE_VALID_UTF8_REGISTER(op1R);
           string_truncate_chars(op1R, (size_t) op2R->int_value);
           DISPATCH;

/* ------------------------------------------------------------------------------------
 *  PADSTR_REG_REG_REG op1=op2(repeated op3 times)                 pej 13 November 2021
 *  requires in op2R the codepoint of the character
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(PADSTR_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - PADSTR R%lu R%lu R%lu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                int i;
                for (i = 0; i < op3R->int_value; i++) {
                    string_concat_char(op1R, op2R);
                }
             }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  CNOP Dummy instruction for testing purposes                     pej 11 November 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(CNOP, VM_ADVANCE(0);
        DEBUG("TRACE - CNOP\n");
        DISPATCH;
/* ------------------------------------------------------------------------------------
 *  find substring in string                                           pej 27 June 2025
 *  the initial offset is in R1: it is 1-based
 *  returned is the offset 1-based both for better performance in calling REXX scripts
 *  In UTF builds, both the input and output positions are codepoint-based.
 *  0 means nothing found
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(STRPOS_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - STRPOS R%lu R%lu R%lu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                char *charpos;
                rxinteger start_pos = op1RI;
                size_t start_offset;
                size_t needle_length;

                REQUIRE_VALID_UTF8_REGISTER(op2R);
                REQUIRE_VALID_UTF8_REGISTER(op3R);
                if (start_pos <= 0) {
                    REG_RETURN_INT(0);
                } else {
                    RXVM_UTF8_ONLY(
                    start_offset = (size_t)(start_pos - 1);
                    if (start_offset >= op3R->string_chars) {
                        start_offset = op3R->string_length;
                    } else {
                        string_cache_seek_char(op3R, start_offset);
                        start_offset = op3R->string_cache_byte_pos;
                    }
                    )
                    RXVM_BYTE_ONLY(
                    start_offset = (size_t)(start_pos - 1);
                    )
                    if (start_offset >= op3R->string_length) {
                        REG_RETURN_INT(0);
                    } else {
                        needle_length = op2R->string_length;
                        charpos = NULL;
                        if (needle_length == 0) {
                            charpos = op3R->string_value + start_offset;
                        } else if (needle_length <= op3R->string_length - start_offset) {
                            char *candidate = op3R->string_value + start_offset;
                            char *last = op3R->string_value +
                                         op3R->string_length - needle_length;
                            while (candidate <= last) {
                                candidate = (char *)memchr(
                                        candidate,
                                        (unsigned char)op2R->string_value[0],
                                        (size_t)(last - candidate + 1));
                                if (!candidate) break;
                                if (memcmp(candidate, op2R->string_value,
                                           needle_length) == 0) {
                                    charpos = candidate;
                                    break;
                                }
                                ++candidate;
                            }
                        }
                        if (!charpos) {
                            REG_RETURN_INT(0);
                        } else {
                            RXVM_UTF8_ONLY(
                            size_t character_offset = 0;
                            (void)utf8nvalid_count(
                                    op3R->string_value,
                                    (size_t)(charpos - op3R->string_value),
                                    &character_offset);
                            REG_RETURN_INT((rxinteger)(character_offset + 1));
                            )
                            RXVM_BYTE_ONLY(
                            REG_RETURN_INT((rxinteger)(charpos - op3R->string_value) + 1);
                            )
                        }
                    }
                }
            }
            DISPATCH;

/*
 *   APPENDCHAR_REG_REG Append Concat Char op2 (as int) on op1
 */

)

RXVM_HANDLER(APPENDCHAR_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - APPENDCHAR R%lu R%lu\n", REG_IDX(1),
                  REG_IDX(2));
            string_concat_char(op1R, op2R);
            DISPATCH;

/*
 *   APPEND_REG_REG Append string op2 on op1
 */

)

RXVM_HANDLER(APPEND_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - APPEND R%lu R%lu\n", REG_IDX(1),
                  REG_IDX(2));
            string_append(op1R, op2R);
            DISPATCH;

/*
 *   SAPPEND_REG_REG Append with space string op2 on op1
 */

)

RXVM_HANDLER(SAPPEND_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - SAPPEND R%lu R%lu\n", REG_IDX(1),
                  REG_IDX(2));
            string_sappend(op1R, op2R);
            DISPATCH;

/*
 *   STRLEN_REG_REG String Length op1 = length(op2)
 */

)

RXVM_HANDLER(STRLEN_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - STRLEN R%lu R%lu\n", REG_IDX(1),
                  REG_IDX(2));
            REQUIRE_VALID_UTF8_REGISTER(op2R);
            RXVM_UTF8_ONLY(
            op1R->int_value = (rxinteger)op2R->string_chars;
            )
            RXVM_BYTE_ONLY(
            op1R->int_value = (rxinteger)op2R->string_length;
            )
            DISPATCH;

/*
 * SETTPMASK_REG_REG_INT masked replace of source-writable status flags
 */

)

RXVM_HANDLER(SETTPMASK_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - SETTPMASK R%d R%d %d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            op1R->status.all_type_flags =
                RXFLAGS_SOURCE_MASKED_REPLACE(op1R->status.all_type_flags,
                                              (uint32_t)op2R->int_value,
                                              (uint32_t)op3I);
            DISPATCH;

/*
 * GETTP_REG_REG gets readable register status flags (op1 = op2.flags)
 */

)

RXVM_HANDLER(GETTP_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - GETTP R%d R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            op1R->int_value = (rxinteger)(op2R->status.all_type_flags & RXFLAG_READABLE_MASK);
            DISPATCH;

/*
 * SETTP_REG_INT sets externally writable register status flags
 */

)

RXVM_HANDLER(SETTP_REG_INT, VM_ADVANCE(2);
            DEBUG("TRACE - SETTP R%d %d\n", (int)REG_IDX(1), (int)op2I);
            op1R->status.all_type_flags = RXFLAGS_PUBLIC_WRITE(op1R->status.all_type_flags, (uint32_t)op2I);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  LOADSETTP_REG_INT load register and set externally writable status flags
 *   op1=op2 and (op1.flags = op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(LOADSETTP_REG_INT_INT, VM_ADVANCE(3);
        DEBUG("TRACE - LOADSETTP R%d %d %d\n", (int)REG_IDX(1),(int)op2I,(int)op3I);

            op1R->int_value = op2I;
            op1R->status.all_type_flags =
                    RXFLAGS_PUBLIC_WRITE(op1R->status.all_type_flags,
                                         (uint32_t)op3I);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  LOADSETTP_REG_string load string to register and set externally writable status flags
 *   op1=op2 and (op1.flags = op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(LOADSETTP_REG_STRING_INT, VM_ADVANCE(3);
            DEBUG("TRACE - LOADSETTP R%d %s %d\n", (int)REG_IDX(1),op2S->string,(int) op3I);

            set_const_string(op1R, op2S);
            op1R->status.all_type_flags = RXFLAGS_PUBLIC_WRITE(op1R->status.all_type_flags, (uint32_t)op3I);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  LOADSETTP_REG_FLOAT float to load register and set externally writable status flags
 *   op1=op2 and (op1.flags = op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(LOADSETTP_REG_FLOAT_INT, VM_ADVANCE(3);
            DEBUG("TRACE - LOADSETTP R%d %.15g %d\n", (int)REG_IDX(1), op2F,(int) op3I);
            op1R->float_value = op2F;
            op1R->status.all_type_flags =
                    RXFLAGS_PUBLIC_WRITE(op1R->status.all_type_flags,
                                         (uint32_t)op3I);
            DISPATCH;

/*
 * SETORTP_REG_INT or externally writable register status flags
 */

)

RXVM_HANDLER(SETORTP_REG_INT, VM_ADVANCE(2);
            DEBUG("TRACE - SETORTP R%d %d\n", (int)REG_IDX(1), (int)op2I);
            op1R->status.all_type_flags = RXFLAGS_PUBLIC_OR(op1R->status.all_type_flags, (uint32_t)op2I);
            DISPATCH;

/*
 * GETANDTP_REG_REG_INT get readable register status flags with mask
 */

)

RXVM_HANDLER(GETANDTP_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - GETANDTP R%d R%d %d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            op1R->int_value = (rxinteger)((op2R->status.all_type_flags & RXFLAG_READABLE_MASK) & (uint32_t)op3I);
            DISPATCH;

/*
 * BRTPT_ID_REG if op2 has public status flags then goto op1
 */

)

RXVM_HANDLER(BRTPT_ID_REG, VM_ADVANCE(2);
            DEBUG("TRACE - BRTPT_ID_REG 0x%x R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2));
            if (op2R->status.all_type_flags & RXFLAG_PUBLIC_TEST_MASK) {
                VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

            }
            DISPATCH;

/*
 * BRTPANDT_ID_REG_INT if op2 readable status flags & op3 true then goto op1
*/

)

RXVM_HANDLER(BRTPANDT_ID_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - BRTPANDT_ID_REG_INT 0x%x R%d %d\n",
                  (unsigned int)REG_IDX(1),
                  (int)REG_IDX(2),(int)op3I);
            if ((op2R->status.all_type_flags & RXFLAG_READABLE_MASK) & (uint32_t)op3I) {
                VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

            }
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IRAND_REG_REG Random Number with seed register                 pej 27 February 2022
 *   op1=irand(op2)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IRAND_REG_REG, VM_ADVANCE(2);
             DEBUG("TRACE - IRAND R%d R%d \n", (int)REG_IDX(1), (int)REG_IDX(2));

            if (op2R->int_value<0) {                 // no seed set
                if (!hasSeed)  {       // seed still initial, set time based seed
                   initSeed = (time((time_t *) 0) % (3600 * 24)); // initial seed still active
                   srand((unsigned) initSeed);
                   hasSeed = 1;
                }
            } else {                                 // seed set re-init with new seed
                initSeed=op2R->int_value;
                srand((unsigned) initSeed);
                hasSeed = 1;
            }
            set_int(op1R, rand());   // receive new random value
        DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IRAND_REG_REG Random Number with seed register                 pej 27 February 2022
 *   op1=irand(op2)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IRAND_REG_INT, VM_ADVANCE(2);
             DEBUG("TRACE - IRAND R%d R%d \n", (int)REG_IDX(1), (int)op2I);
             if (op2I<0) {                                // no seed set
                if (!hasSeed)  {            // seed still initial, set time based seed
                    initSeed = (time((time_t *) 0) % (3600 * 24)); // initial seed still active
                    srand(initSeed);
                    hasSeed = 1;
                }
             } else {                                     // seed set and NE old seed, set it new
                initSeed=op2I;
                srand( initSeed);
                hasSeed = 1;
            }
            set_int(op1R, rand());   // receive new random value
        DISPATCH;

/* ------------------------------------------------------------------------------------
 *  rxvers  returns os information                                    pej 20. June 2023
 *  -----------------------------------------------------------------------------------
 */

// MACRO


)
#define FDATE (char const[]){ __DATE__[7], __DATE__[8], ..., ' ', ... , '\0' }


RXVM_HANDLER(RXVERS_REG, VM_ADVANCE(1);
    DEBUG("TRACE - RXVERS R%d\n", (int) REG_IDX(1));
    {
        char compile_date[9];
        char vers[256];
        const char *platform;
        const char *bits;

        platform = RXVM_PLATFORM_NAME;

        bits = sizeof(void *) == 4 ? "32" : "64";
        rxvm_format_compile_date(compile_date);

        snprintf(vers, sizeof(vers), "%s %s %s %s", platform, bits, rxversion, compile_date);

        set_null_string(op1R, vers);
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 * rxhash returns the conventional 32-bit FNV-1a hash of the first op3 bytes of op2.
 * The requested byte count is clamped to the available UTF-8 payload.
 * -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(RXHASH_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - RXHASH R%d R%d R%d \n", (int)REG_IDX(1),(int)REG_IDX(2),(int)REG_IDX(3));

    {
        uint32_t hash = UINT32_C(2166136261);
        size_t i;
        size_t len;
        rxinteger requested_len = op3R->int_value;

        REQUIRE_VALID_UTF8_REGISTER(op2R);

        if (requested_len <= 0) {
            len = 0;
        }
        else if ((uint64_t)requested_len > (uint64_t)op2R->string_length) {
            len = op2R->string_length;
        }
        else {
            len = (size_t)requested_len;
        }

        for (i = 0; i < len; i++) {
            hash ^= (uint8_t)op2R->string_value[i];
            hash *= UINT32_C(16777619);
        }

        op1R->int_value = (rxinteger)hash;
     }
     DISPATCH;
    /* File IO functions - mapped to C90 functions */

    /* fopen - op1 file*(int) = fopen filename op2(string) mode op3(string) */

)

RXVM_HANDLER(FOPEN_REG_REG_REG, VM_ADVANCE(3);
        DEBUG("TRACE - FOPEN R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
        {
            int fd;
            char* filename = reg2nullstring(op2R);
            char* mode = reg2nullstring(op3R);

            // If the filename is "stdout", "stderr", or "stdin" we use the C runtime's handle
            if (strcmp(filename, "stdout") == 0) {
                op1R->int_value = (rxinteger)stdout;
            } else if (strcmp(filename, "stderr") == 0) {
                op1R->int_value = (rxinteger)stderr;
            } else if (strcmp(filename, "stdin") == 0) {
                op1R->int_value = (rxinteger)stdin;
            } else {
                /* Otherwise we open the file normally */
                op1R->int_value = (rxinteger)fopen(filename, mode);

                /* If the open succeeds, add the FD_CLOEXEC so that the file is not available to an ADDRESSed command! */
                if (op1R->int_value) {
                    RXVM_WINDOWS_ONLY(
                    // On Windows, get the underlying OS HANDLE from the C-runtime file descriptor.
                    intptr_t handle = _get_osfhandle(_fileno((FILE*)op1R->int_value));
                    if (handle != -1) {
                        // Set the handle to be non-inheritable. This is the equivalent
                        // of POSIX's FD_CLOEXEC flag.
                        SetHandleInformation((HANDLE)handle, HANDLE_FLAG_INHERIT, 0);
                    }
                    )
                    RXVM_NONWINDOWS_ONLY(
                    fd = fileno((FILE*)op1R->int_value);
                    fcntl(fd, F_SETFD, FD_CLOEXEC);
                    )
                }
            }
            free(filename);
            free(mode);
        }
        DISPATCH;

    /* fclose - op1 rc(int) = fclose op2 file*(int) */

)

RXVM_HANDLER(FCLOSE_REG_REG, VM_ADVANCE(2);
        DEBUG("TRACE - FCLOSE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        op1R->int_value = fclose((FILE*)op2R->int_value);
        DISPATCH;

    /* fflush - op1 rc(int) = fflush op2 file*(int) */

)

RXVM_HANDLER(FFLUSH_REG_REG, VM_ADVANCE(2);
        DEBUG("TRACE - FFLUSH R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        op1R->int_value = fflush((FILE*)op2R->int_value);
        DISPATCH;

    /* freadb - op1(binary) = fread op2 file*(int) op3 bytes(int) */

)

RXVM_HANDLER(FREADB_REG_REG_REG, VM_ADVANCE(3);
        DEBUG("TRACE - FREADB R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
        {
            if (op3R->int_value < 0) {
                SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Invalid binary read size");
            }
            else if (op3R->int_value == 0) {
                if (prep_binary_buffer(op1R, 0) != 0) SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
            }
            else if (reserve_binary_buffer(op1R, (size_t)op3R->int_value) != 0) {
                SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
            }
            else {
                op1R->binary_length = fread(op1R->binary_value, 1, (size_t)op3R->int_value, (FILE *) op2R->int_value);
                clear_vm_private_flags(op1R);
            }
        }
        DISPATCH;

    /* freadline - op1 (string) = read until newline op2 file*(int) */

)

RXVM_HANDLER(FREADLINE_REG_REG, VM_ADVANCE(2);
        DEBUG("TRACE - FREADLINE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
    {
        int ch;
        FILE *stream = (FILE*)op2R->int_value;

        rxvm_value_set_string_length_known(op1R, 0u);
        string_cache_reset(op1R);

        if (op2R->int_value == 0) {
            // no file - raise error
            SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "File not open");
        }

        else {
            /* Read until EOF or newline */
            while ((ch = fgetc(stream)) != EOF) {

                /* End of line detection (LF) */
                if (ch == '\n') {
                    /*
                     * Do not probe beyond an interactive/still-open stdin
                     * line. The probe blocks until a second byte arrives,
                     * making linein(stdin) require an extra Return.
                     */
                    if (stream != stdin) {
                        ch = fgetc(stream);
                        if (ch != EOF) ungetc(ch, stream);
                        else if (op1R->string_length == 0) clearerr(stream);
                    }
                    break;
                }

                /* End of line detection (CR LF, or CR (old macs) */
                if (ch == '\r') {
                    if (stream != stdin) {
                        ch = fgetc(stream);
                        if (ch == '\n') {
                            ch = fgetc(stream);
                            if (ch != EOF) ungetc(ch, stream);
                            else if (op1R->string_length == 0) clearerr(stream);
                        }
                        else if (ch != EOF) ungetc(ch, stream);
                    }
                    break;
                }

                {
                    rxvm_string_metric next_length =
                            rxvm_value_string_size_add_or_fail(
                            op1R->string_length, 1u);
                    extend_string_buffer_metric(op1R, next_length);
                    op1R->string_value[next_length - 1u] = (char)ch;
                }
            }
        }
        }
        RXVM_UTF8_ONLY(
        {
            size_t chars = 0;
            if (validate_utf8_bytes(op1R->string_value, op1R->string_length, &chars) != 0) {
                SET_SIGNAL_MSG(RXSIGNAL_UNICODE_ERROR, "Invalid UTF-8 in text file read");
            } else {
                rxvm_value_set_string_chars_known(op1R, chars);
                string_cache_reset(op1R);
                if (chars == op1R->string_length)
                    mark_ascii_string_valid_count(op1R);
                else mark_utf8_valid_count(op1R);
            }
        }
        )
        RXVM_BYTE_ONLY(
        clear_vm_private_flags(op1R);
        )
        DISPATCH;

    /* freadbyte - op1 (int) = read byte op2 file*(int) */

)

RXVM_HANDLER(FREADBYTE_REG_REG, VM_ADVANCE(2);
        DEBUG("TRACE - FREADBYTE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        op1R->int_value = fgetc((FILE *)op2R->int_value);
        DISPATCH;

    /* freadcdpt - op1 (string and int) = read codepoint op2 file*(int) */

)
