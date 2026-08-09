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

/* Numeric RXAS handler definitions; body text is compiled by RXVM_HANDLER. */
RXVM_HANDLER(MINATTRS_REG_REG_INT, VM_ADVANCE(3);
        DEBUG("TRACE - MINATTRS R%lu,R%lu,%d\n", REG_IDX(1),REG_IDX(2),(int)op3I);
        if (op2RI + op3I > op1R->num_attributes) {
            /* Set the number of attributes to the requested number */
            set_num_attributes(op1R, op2RI + op3I);
        }
        DISPATCH;

        /* ------------------------------------------------------------------------------------
         *  MINATTRS_REG_REG_INT Ensure min number attributes op1.num_attributes >= op2 + op3
         * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(MINATTRS_REG_INT_INT, VM_ADVANCE(3);
            DEBUG("TRACE - MINATTRS R%lu,%d,%d\n", REG_IDX(1),(int)op2I,(int)op3I);
            if (op2I + op3I > op1R->num_attributes) {
                /* Set the number of attributes to the requested number */
                set_num_attributes(op1R, op2I + op3I);
            }
            DISPATCH;

        /* ------------------------------------------------------------------------------------
         *  GETABUFS_REG_REG Get attribute buffer size op1 = op2.max_attributes
         *  ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(GETABUFS_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - GETABUFS R%lu,R%lu\n", REG_IDX(1),REG_IDX(1));
            REG_RETURN_INT(rxvm_value_max_attributes(op2R))
            DISPATCH;


)

RXVM_HANDLER(INSATTRS_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - INSATTRS R%lu,R%lu,R%lu\n", REG_IDX(1),REG_IDX(2),REG_IDX(3));
            if (rxvm_insert_attributes_checked(op1R, op2RI, op3RI) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH;


)

RXVM_HANDLER(INSATTRS_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - INSATTRS R%lu,R%lu,%d\n", REG_IDX(1),REG_IDX(2),(int)op3I);
            if (rxvm_insert_attributes_checked(op1R, op2RI, op3I) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH;


)

RXVM_HANDLER(INSATTRS_REG_INT_REG, VM_ADVANCE(3);
            DEBUG("TRACE - INSATTRS R%lu,%d,R%lu\n", REG_IDX(1),(int)op2I,REG_IDX(3));
            if (rxvm_insert_attributes_checked(op1R, op2I, op3RI) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH;


)

RXVM_HANDLER(INSATTRS_REG_INT_INT, VM_ADVANCE(3);
            DEBUG("TRACE - INSATTRS R%lu,%d,%d\n", REG_IDX(1),(int)op2I,(int)op3I);
            if (rxvm_insert_attributes_checked(op1R, op2I, op3I) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH;


)

RXVM_HANDLER(INSATTRS1_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - INSATTRS1 R%lu,R%lu,R%lu\n", REG_IDX(1),REG_IDX(2),REG_IDX(3));
            if (rxvm_insert_attributes1_checked(op1R, op2RI, op3RI) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH;


)

RXVM_HANDLER(INSATTRS1_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - INSATTRS1 R%lu,R%lu,%d\n", REG_IDX(1),REG_IDX(2),(int)op3I);
            if (rxvm_insert_attributes1_checked(op1R, op2RI, op3I) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH;


)

RXVM_HANDLER(INSATTRS1_REG_INT_REG, VM_ADVANCE(3);
            DEBUG("TRACE - INSATTRS1 R%lu,%d,R%lu\n", REG_IDX(1),(int)op2I,REG_IDX(3));
            if (rxvm_insert_attributes1_checked(op1R, op2I, op3RI) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH;


)

RXVM_HANDLER(INSATTRS1_REG_INT_INT, VM_ADVANCE(3);
            DEBUG("TRACE - INSATTRS1 R%lu,%d,%d\n", REG_IDX(1),(int)op2I,(int)op3I);
            if (rxvm_insert_attributes1_checked(op1R, op2I, op3I) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH;


)

RXVM_HANDLER(DELATTRS_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - DELATTRS R%lu,R%lu,R%lu\n", REG_IDX(1),REG_IDX(2),REG_IDX(3));
            if (rxvm_delete_attributes_checked(op1R, op2RI, op3RI) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH;


)

RXVM_HANDLER(DELATTRS_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - DELATTRS R%lu,R%lu,%d\n", REG_IDX(1),REG_IDX(2),(int)op3I);
            if (rxvm_delete_attributes_checked(op1R, op2RI, op3I) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH;


)

RXVM_HANDLER(DELATTRS_REG_INT_REG, VM_ADVANCE(3);
            DEBUG("TRACE - DELATTRS R%lu,%d,R%lu\n", REG_IDX(1),(int)op2I,REG_IDX(3));
            if (rxvm_delete_attributes_checked(op1R, op2I, op3RI) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH;


)

RXVM_HANDLER(DELATTRS_REG_INT_INT, VM_ADVANCE(3);
            DEBUG("TRACE - DELATTRS R%lu,%d,%d\n", REG_IDX(1),(int)op2I,(int)op3I);
            if (rxvm_delete_attributes_checked(op1R, op2I, op3I) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH;


)

RXVM_HANDLER(DELATTRS1_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - DELATTRS1 R%lu,R%lu,R%lu\n", REG_IDX(1),REG_IDX(2),REG_IDX(3));
            if (rxvm_delete_attributes1_checked(op1R, op2RI, op3RI) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH;


)

RXVM_HANDLER(DELATTRS1_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - DELATTRS1 R%lu,R%lu,%d\n", REG_IDX(1),REG_IDX(2),(int)op3I);
            if (rxvm_delete_attributes1_checked(op1R, op2RI, op3I) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH;


)

RXVM_HANDLER(DELATTRS1_REG_INT_REG, VM_ADVANCE(3);
            DEBUG("TRACE - DELATTRS1 R%lu,%d,R%lu\n", REG_IDX(1),(int)op2I,REG_IDX(3));
            if (rxvm_delete_attributes1_checked(op1R, op2I, op3RI) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH;


)

RXVM_HANDLER(DELATTRS1_REG_INT_INT, VM_ADVANCE(3);
            DEBUG("TRACE - DELATTRS1 R%lu,%d,%d\n", REG_IDX(1),(int)op2I,(int)op3I);
            if (rxvm_delete_attributes1_checked(op1R, op2I, op3I) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH;


)

RXVM_HANDLER(DEC0, VM_ADVANCE(0);
            /* TODO This is really idec0 - i.e. it does not prime the int */
            DEBUG("TRACE - DEC0\n");
            {
                rxinteger result;
                if (!rxinteger_checked_sub(current_locals[0]->int_value, 1,
                                           &result)) {
                    SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
                }
                else current_locals[0]->int_value = result;
                DISPATCH;
            }

            /* ------------------------------------------------------------------------------------
         *  DEC1   R1--                                                       pej 7. April 2021
         *  -----------------------------------------------------------------------------------
         */

)

RXVM_HANDLER(DEC1, VM_ADVANCE(0);
            /* TODO This is really idec1 - i.e. it does not prime the int */
            DEBUG("TRACE - DEC1\n");
            {
                rxinteger result;
                if (!rxinteger_checked_sub(current_locals[1]->int_value, 1,
                                           &result)) {
                    SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
                }
                else current_locals[1]->int_value = result;
                DISPATCH;
            }

            /* ------------------------------------------------------------------------------------
            *  DEC2   op2R--                                                       pej 7. April 2021
            *  -----------------------------------------------------------------------------------
            */

)

RXVM_HANDLER(DEC2, VM_ADVANCE(0);
            /* TODO This is really idec2 - i.e. it does not prime the int */
            DEBUG("TRACE - DEC2\n");
            {
                rxinteger result;
                if (!rxinteger_checked_sub(current_locals[2]->int_value, 1,
                                           &result)) {
                    SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
                }
                else current_locals[2]->int_value = result;
                DISPATCH;
            }


)

RXVM_HANDLER(DEC_REG, VM_ADVANCE(1);
            /* TODO This is really idec reg - i.e. it does not prime the int */
            DEBUG("TRACE - DEC R%lu\n", REG_IDX(1));
            {
                rxinteger result;
                if (!rxinteger_checked_sub(
                        current_locals[REG_IDX(1)]->int_value, 1, &result)) {
                    SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
                }
                else current_locals[REG_IDX(1)]->int_value = result;
                DISPATCH;
            }


)

RXVM_HANDLER(BRT_ID_REG, VM_ADVANCE(2); /* i.e. if the condition is not met - this helps the
                                the real CPUs branch prediction (in theory) */
            DEBUG("TRACE - BRT 0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2));
            if (op2RI) {
                VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

            }
            DISPATCH;


)

RXVM_HANDLER(BRF_ID_REG, VM_ADVANCE(2); /* i.e. if the condition is not met - this helps the
                                  the real CPUs branch prediction (in theory) */
            DEBUG("TRACE - BRF 0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2));
            if (!(op2RI)) {
                VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

            }
            DISPATCH;


)

RXVM_HANDLER(BRTF_ID_ID_REG,
            DEBUG("TRACE - BRTF 0x%x,0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            if (op3RI) VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);
            else VM_SELECT_INDEX(REG_IDX(2), RXVM_TRANSITION_BRANCH);

            DISPATCH;


)

RXVM_HANDLER(JUMPS_REG_BINARY, VM_ADVANCE(2);
            DEBUG("TRACE - JUMPS R%d,binary[%zu]\n", (int)REG_IDX(1), (size_t)(pc + 2)->index);
            {
                size_t target;
                int found = rxvm_jtable_lookup(op2S, (const unsigned char *)op1R->string_value,
                                               op1R->string_length, &target);
                if (found < 0) {
                    SET_SIGNAL_MSG(RXSIGNAL_RXBIN_CORRUPTION, "Malformed jump table");
                }
                else if (found && target >= current_frame->procedure->binarySpace->inst_size) {
                    SET_SIGNAL_MSG(RXSIGNAL_RXBIN_CORRUPTION, "Jump table target out of range");
                }
                else if (found) {
                    VM_SELECT_INDEX(target, RXVM_TRANSITION_BRANCH);

                }
            }
            DISPATCH;


)

RXVM_HANDLER(JUMPB_REG_BINARY, VM_ADVANCE(2);
            DEBUG("TRACE - JUMPB R%d,binary[%zu]\n", (int)REG_IDX(1), (size_t)(pc + 2)->index);
            {
                size_t target;
                int found = rxvm_jtable_lookup(op2S, (const unsigned char *)op1R->binary_value,
                                               op1R->binary_length, &target);
                if (found < 0) {
                    SET_SIGNAL_MSG(RXSIGNAL_RXBIN_CORRUPTION, "Malformed jump table");
                }
                else if (found && target >= current_frame->procedure->binarySpace->inst_size) {
                    SET_SIGNAL_MSG(RXSIGNAL_RXBIN_CORRUPTION, "Jump table target out of range");
                }
                else if (found) {
                    VM_SELECT_INDEX(target, RXVM_TRANSITION_BRANCH);

                }
            }
            DISPATCH;


)

RXVM_HANDLER(JUMPBS_REG_REG_BINARY, VM_ADVANCE(3);
            DEBUG("TRACE - JUMPBS R%d,R%d,binary[%zu]\n", (int)REG_IDX(1), (int)REG_IDX(2), (size_t)(pc + 3)->index);
            {
                unsigned char algorithm;
                uint32_t fixed_key_length;
                uint32_t case_count;
                uint16_t header_size;
                size_t key_length;
                size_t offset;
                size_t target;
                if (!rxvm_jtable_read_header(op3S, &algorithm, &fixed_key_length,
                                             &case_count, &header_size)) {
                    SET_SIGNAL_MSG(RXSIGNAL_RXBIN_CORRUPTION, "Malformed jump table");
                }
                else if (fixed_key_length == 0 ||
                         !rxvm_binary_range(op1R, op2R->int_value,
                                            (size_t)fixed_key_length, &offset)) {
                    SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                }
                else {
                    int found;
                    key_length = (size_t)fixed_key_length;
                    found = rxvm_jtable_lookup_parsed(
                        op3S, (const unsigned char *)op1R->binary_value + offset,
                        key_length, &target, algorithm, fixed_key_length,
                        case_count, header_size);
                    if (found < 0) {
                        SET_SIGNAL_MSG(RXSIGNAL_RXBIN_CORRUPTION, "Malformed jump table");
                    }
                    else if (found && target >= current_frame->procedure->binarySpace->inst_size) {
                        SET_SIGNAL_MSG(RXSIGNAL_RXBIN_CORRUPTION, "Jump table target out of range");
                    }
                    else if (found) {
                        VM_SELECT_INDEX(target, RXVM_TRANSITION_BRANCH);

                    }
                }
            }
            DISPATCH;


)

RXVM_HANDLER(JUMPI_REG_BINARY, VM_ADVANCE(2);
            DEBUG("TRACE - JUMPI R%d,binary[%zu]\n", (int)REG_IDX(1), (size_t)(pc + 2)->index);
            {
                unsigned char key[8];
                uint64_t value = (uint64_t)(int64_t)op1R->int_value;
                size_t target;
                int i;
                int found;
                for (i = 0; i < 8; i++) {
                    key[i] = (unsigned char)((value >> (i * 8)) & 0xffu);
                }
                found = rxvm_jtable_lookup(op2S, key, sizeof(key), &target);
                if (found < 0) {
                    SET_SIGNAL_MSG(RXSIGNAL_RXBIN_CORRUPTION, "Malformed jump table");
                }
                else if (found && target >= current_frame->procedure->binarySpace->inst_size) {
                    SET_SIGNAL_MSG(RXSIGNAL_RXBIN_CORRUPTION, "Jump table target out of range");
                }
                else if (found) {
                    VM_SELECT_INDEX(target, RXVM_TRANSITION_BRANCH);

                }
            }
            DISPATCH;


)

RXVM_HANDLER(JUMPR_REG_BINARY, VM_ADVANCE(2);
            DEBUG("TRACE - JUMPR R%d,binary[%zu]\n", (int)REG_IDX(1), (size_t)(pc + 2)->index);
            {
                size_t key_length = op1R->string_length;
                size_t target;
                int found;

                while (key_length > 0 && op1R->string_value[key_length - 1] == ' ') key_length--;
                found = rxvm_jtable_lookup(op2S,
                                           (const unsigned char *)op1R->string_value,
                                           key_length,
                                           &target);
                if (found < 0) {
                    SET_SIGNAL_MSG(RXSIGNAL_RXBIN_CORRUPTION, "Malformed jump table");
                }
                else if (found && target >= current_frame->procedure->binarySpace->inst_size) {
                    SET_SIGNAL_MSG(RXSIGNAL_RXBIN_CORRUPTION, "Jump table target out of range");
                }
                else if (found) {
                    VM_SELECT_INDEX(target, RXVM_TRANSITION_BRANCH);

                }
            }
            DISPATCH;


)

RXVM_HANDLER(JUMPN_REG_BINARY, VM_ADVANCE(2);
            DEBUG("TRACE - JUMPN R%d,binary[%zu]\n", (int)REG_IDX(1), (size_t)(pc + 2)->index);
            {
                unsigned char key[RX_NUMERIC_KEY_SIZE];
                size_t target;
                int found = 0;

                if (rx_numeric_key_from_text(key,
                                             op1R->string_value,
                                             op1R->string_length,
                                             NULL)) {
                    found = rxvm_jtable_lookup(op2S, key, sizeof(key), &target);
                }
                if (found < 0) {
                    SET_SIGNAL_MSG(RXSIGNAL_RXBIN_CORRUPTION, "Malformed jump table");
                }
                else if (found && target >= current_frame->procedure->binarySpace->inst_size) {
                    SET_SIGNAL_MSG(RXSIGNAL_RXBIN_CORRUPTION, "Jump table target out of range");
                }
                else if (found) {
                    VM_SELECT_INDEX(target, RXVM_TRANSITION_BRANCH);

                }
            }
            DISPATCH;


)

RXVM_HANDLER(BINEQ_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - BINEQ R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2R->binary_length == op3R->binary_length &&
                           (!op2R->binary_length ||
                            memcmp(op2R->binary_value, op3R->binary_value,
                                   op2R->binary_length) == 0))
            DISPATCH;


)

RXVM_HANDLER(BINEQ_REG_REG_BINARY, VM_ADVANCE(3);
            DEBUG("TRACE - BINEQ R%d,R%d,binary[%zu]\n",
                  (int)REG_IDX(1), (int)REG_IDX(2), (size_t)(pc + 3)->index);
            REG_RETURN_INT(op2R->binary_length == op3S->string_len &&
                           (!op2R->binary_length ||
                            memcmp(op2R->binary_value, op3S->string,
                                   op2R->binary_length) == 0))
            DISPATCH;


)

RXVM_HANDLER(BINNE_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - BINNE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2R->binary_length != op3R->binary_length ||
                           (op2R->binary_length &&
                            memcmp(op2R->binary_value, op3R->binary_value,
                                   op2R->binary_length) != 0))
            DISPATCH;


)

RXVM_HANDLER(BINNE_REG_REG_BINARY, VM_ADVANCE(3);
            DEBUG("TRACE - BINNE R%d,R%d,binary[%zu]\n",
                  (int)REG_IDX(1), (int)REG_IDX(2), (size_t)(pc + 3)->index);
            REG_RETURN_INT(op2R->binary_length != op3S->string_len ||
                           (op2R->binary_length &&
                            memcmp(op2R->binary_value, op3S->string,
                                   op2R->binary_length) != 0))
            DISPATCH;


)
#define RXSTEM_REQUIRE_RESULT(call_, index_message_)                         \
        do {                                                                 \
            int rxstem_result__ = (call_);                                   \
            if (rxstem_result__ == RXSTEM_OUT_OF_MEMORY) {                   \
                SET_SIGNAL_MSG(RXSIGNAL_FAILURE,                             \
                               "Native stem allocation failed");            \
                DISPATCH;                                                    \
            }                                                                \
            if (rxstem_result__ == RXSTEM_OVERFLOW) {                        \
                SET_SIGNAL_MSG(RXSIGNAL_FAILURE,                             \
                               "Native stem capacity overflow");            \
                DISPATCH;                                                    \
            }                                                                \
            if (rxstem_result__ == RXSTEM_INVALID_INDEX) {                   \
                SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, index_message_);  \
                DISPATCH;                                                    \
            }                                                                \
            if (rxstem_result__ != RXSTEM_OK) {                              \
                SET_SIGNAL_MSG(RXSIGNAL_FAILURE,                             \
                               "Native stem representation is corrupt");    \
                DISPATCH;                                                    \
            }                                                                \
        } while (0)


RXVM_HANDLER(STEMINIT_REG, VM_ADVANCE(1);
            DEBUG("TRACE - STEMINIT R%d\n", (int)REG_IDX(1));
            RXSTEM_REQUIRE_RESULT(rxstem_init(REG_OP(1)),
                                  "STEM index is outside the stored range");
            DISPATCH;


)

RXVM_HANDLER(STEMGET_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - STEMGET R%d,R%d,R%d\n",
                  (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REQUIRE_VALID_UTF8_REGISTER(REG_OP(3));
            {
                rxstem_key_parts parts = rxstem_one_part(REG_OP(3));
                RXSTEM_REQUIRE_RESULT(
                        rxstem_get_parts(REG_OP(1), REG_OP(2), &parts),
                        "STEM index is outside the stored range");
            }
            DISPATCH;


)

RXVM_HANDLER(STEMSET_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - STEMSET R%d,R%d,R%d\n",
                  (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REQUIRE_VALID_UTF8_REGISTER(REG_OP(2));
            REQUIRE_VALID_UTF8_REGISTER(REG_OP(3));
            {
                rxstem_key_parts parts = rxstem_one_part(REG_OP(2));
                RXSTEM_REQUIRE_RESULT(
                        rxstem_set_parts(REG_OP(1), &parts, REG_OP(3)),
                        "STEM index is outside the stored range");
            }
            DISPATCH;


)

RXVM_HANDLER(STEMRESET_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - STEMRESET R%d,R%d\n",
                  (int)REG_IDX(1), (int)REG_IDX(2));
            REQUIRE_VALID_UTF8_REGISTER(REG_OP(2));
            RXSTEM_REQUIRE_RESULT(rxstem_reset(REG_OP(1), REG_OP(2)),
                                  "STEM index is outside the stored range");
            DISPATCH;


)

RXVM_HANDLER(STEMGET2_REG_REG_REG_REG, VM_ADVANCE(4);
            DEBUG("TRACE - STEMGET2 R%d,R%d,R%d,R%d\n",
                  (int)REG_IDX(1), (int)REG_IDX(2),
                  (int)REG_IDX(3), (int)REG_IDX(4));
            REQUIRE_VALID_UTF8_REGISTER(REG_OP(3));
            REQUIRE_VALID_UTF8_REGISTER(REG_OP(4));
            {
                rxstem_key_parts parts =
                        rxstem_two_parts(REG_OP(3), REG_OP(4));
                RXSTEM_REQUIRE_RESULT(
                        rxstem_get_parts(REG_OP(1), REG_OP(2), &parts),
                        "STEM index is outside the stored range");
            }
            DISPATCH;


)

RXVM_HANDLER(STEMSET2_REG_REG_REG_REG, VM_ADVANCE(4);
            DEBUG("TRACE - STEMSET2 R%d,R%d,R%d,R%d\n",
                  (int)REG_IDX(1), (int)REG_IDX(2),
                  (int)REG_IDX(3), (int)REG_IDX(4));
            REQUIRE_VALID_UTF8_REGISTER(REG_OP(2));
            REQUIRE_VALID_UTF8_REGISTER(REG_OP(3));
            REQUIRE_VALID_UTF8_REGISTER(REG_OP(4));
            {
                rxstem_key_parts parts =
                        rxstem_two_parts(REG_OP(2), REG_OP(3));
                RXSTEM_REQUIRE_RESULT(
                        rxstem_set_parts(REG_OP(1), &parts, REG_OP(4)),
                        "STEM index is outside the stored range");
            }
            DISPATCH;


)

RXVM_HANDLER(STEMSIZE_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - STEMSIZE R%d,R%d\n",
                  (int)REG_IDX(1), (int)REG_IDX(2));
            {
                rxinteger size;
                RXSTEM_REQUIRE_RESULT(rxstem_size(&size, REG_OP(2)),
                                      "STEM index is outside the stored range");
                REG_RETURN_INT(size)
            }
            DISPATCH;


)

RXVM_HANDLER(STEMKEYAT_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - STEMKEYAT R%d,R%d,R%d\n",
                  (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            RXSTEM_REQUIRE_RESULT(
                    rxstem_key_at(REG_OP(1), REG_OP(2), REG_OP(3)->int_value),
                    "STEM key index is outside the stored range");
            DISPATCH;


)

RXVM_HANDLER(STEMVALUEAT_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - STEMVALUEAT R%d,R%d,R%d\n",
                  (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            RXSTEM_REQUIRE_RESULT(
                    rxstem_value_at(REG_OP(1), REG_OP(2), REG_OP(3)->int_value),
                    "STEM value index is outside the stored range");
            DISPATCH;


)
#undef RXSTEM_REQUIRE_RESULT


RXVM_HANDLER(TIME_REG, VM_ADVANCE(1);
            DEBUG("TRACE - TIME R%d\n", (int)REG_IDX(1));
            {
                rxvm_timeval tv;
                rxvm_get_timeval(&tv);
                REG_RETURN_INT(tv.tv_sec - rxvm_timezone_seconds())
            }
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  XTIME return time properties                                  pej 02. December 2021
 * ------------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(XTIME_REG_STRING, VM_ADVANCE(2);
        DEBUG("TRACE - XTIME R%d,\"%s\"\n", (int)REG_IDX(1),(CONSTSTRING_OP(2))->string);

            switch ((CONSTSTRING_OP(2))->string[0]) {
                case 'Z':
                    op1R->int_value = rxvm_timezone_seconds();
                    break;
                case 'T':  op1R->int_value  = (rxinteger)clock(); break;
                case 'C':  op1R->int_value  = CLOCKS_PER_SEC; break;
                case 'N':  {
                     (void) rxvm_timezone_seconds();
                     prep_string_buffer(op1R,2*SMALLEST_STRING_BUFFER_LENGTH); // Large enough for both time zone names
                     snprintf(op1R->string_value,2*SMALLEST_STRING_BUFFER_LENGTH,
                              "%s;%s", RXVM_TZNAME[0], RXVM_TZNAME[1]);
                     finish_string_write(op1R, strlen(op1R->string_value));
                     break;  // time zone names
                }
                case 'U':  {
                     time_t ctime;
                     rxinteger tm;
                     rxvm_timeval tv;
                     struct tm *tmdata;
                     rxvm_get_timeval(&tv);
                     ctime = tv.tv_sec;
                     tmdata = localtime(&ctime);
                     tm = ((tmdata->tm_hour * 3600) + (tmdata->tm_min * 60) +
                           tmdata->tm_sec) + rxvm_timezone_seconds();
                     op1R->int_value = tm*1000000+tv.tv_usec;
                     break;  // UTC Time
                }
            }
            DISPATCH;

/* ---------------------------------------------------------------------------------
 *  MTIME get time of the day in microseconds                      pej 31. October 2021
 * ------------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(MTIME_REG, VM_ADVANCE(1);
            DEBUG("TRACE - MTIME R%d\n", (int)REG_IDX(1));
            {
                rxinteger tm;
                rxvm_timeval tv;
                time_t	ctime;
                struct tm *tmdata;

                rxvm_get_timeval(&tv);
                ctime = tv.tv_sec;
                tmdata = localtime(&ctime);
                tm =
                        ((tmdata->tm_hour * 3600) + (tmdata->tm_min * 60) +
                        (tmdata->tm_sec));
                REG_RETURN_INT(tm * 1000000 + tv.tv_usec)
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  TRIMR  Trim right                                                 pej 7. April 2021
 * ------------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(TRIMR_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - TRIMR (DEPRECATED) R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            {
                size_t i = op1R->string_length;
                while (i > 0 && op1R->string_value[i - 1] == ' ') {
                    i--;
                }
                rxvm_value_set_string_length_known(op1R, i);
                null_terminate_string_buffer(op1R);
                finish_string_write(op1R, op1R->string_length);
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  TRIML  Trim left                                                  pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(TRIML_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - TRIML (DEPRECATED) R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            /* TODO - UTF etc */
            {
                size_t i = 0;
                while (i < op1R->string_length && op1R->string_value[i] == ' ') i++;

                if (i >= op1R->string_length) {
                    op1R->string_length = 0;
                    null_terminate_string_buffer(op1R);
                } else {
                    rxvm_value_set_string_length_known(
                            op1R, op1R->string_length - i);
                    memcpy(op1R->string_value, op1R->string_value + i,
                           op1R->string_length);
                    null_terminate_string_buffer(op1R);
                }
                finish_string_write(op1R, op1R->string_length);
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  INC0   R0++                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(INC0, VM_ADVANCE(0);
            DEBUG("TRACE - INC0\n");
            {
                rxinteger result;
                if (!rxinteger_checked_add(REG_VAL(0)->int_value, 1, &result)) {
                    SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
                }
                else REG_VAL(0)->int_value = result;
                DISPATCH;
            }

/* ------------------------------------------------------------------------------------
 *  INC1   R1++                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(INC1, VM_ADVANCE(0);
            DEBUG("TRACE - INC1\n");
            {
                rxinteger result;
                if (!rxinteger_checked_add(REG_VAL(1)->int_value, 1, &result)) {
                    SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
                }
                else REG_VAL(1)->int_value = result;
                DISPATCH;
            }

/* ------------------------------------------------------------------------------------
 *  INC2   op2R++                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(INC2, VM_ADVANCE(0);
            DEBUG("TRACE - INC2\n");
            {
                rxinteger result;
                if (!rxinteger_checked_add(REG_VAL(2)->int_value, 1, &result)) {
                    SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
                }
                else REG_VAL(2)->int_value = result;
                DISPATCH;
            }
/* ------------------------------------------------------------------------------------
 *  ISEX   op1 = -op1  decimal                                    pej 2. September 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(ISEX_REG, VM_ADVANCE(1);
            DEBUG("TRACE - INC R%lu\n", REG_IDX(1));
            if (!rxinteger_checked_neg(current_locals[REG_IDX(1)]->int_value,
                                       &current_locals[REG_IDX(1)]->int_value)) {
                SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
            }
        DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FSEX   op1 = -op1  decimal                                    pej 2. September 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FSEX_REG, VM_ADVANCE(1);
            DEBUG("TRACE - INC R%lu\n", REG_IDX(1));
            (current_locals[REG_IDX(1)]->float_value)=0-(current_locals[REG_IDX(1)]->float_value);
        DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ISUB_REG_REG_INT: Integer Subtract (op1=op2-op3)               pej 8. April 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(ISUB_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - ISUB R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            {
                rxinteger result;
                if (rxinteger_checked_sub(op2RI, op3I, &result)) REG_RETURN_INT(result)
                else SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ISUB_REG_INT_REG: Integer Subtract (op1=op2-op3)               pej 8. April 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(ISUB_REG_INT_REG, VM_ADVANCE(3);
            DEBUG("TRACE - ISUB R%d,%d,R%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            {
                rxinteger result;
                if (rxinteger_checked_sub(op2I, op3RI, &result)) REG_RETURN_INT(result)
                else SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  AND_REG_REG_REG  Int Logical AND op1=(op2 && op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(AND_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - AND R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI && op3RI)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  OR_REG_REG_REG  Int Logical OR op1=(op2 || op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(OR_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - OR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI || op3RI)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  NOT_REG_REG  Int Logical NOT op1=!op2
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(NOT_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - NOT R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            if (op2RI) REG_RETURN_INT(0)
            else REG_RETURN_INT(1)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IEQ_REG_REG_REG  Int Equals op1=(op2==op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IEQ_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - IEQ R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI == op3RI)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IEQ_REG_REG_INT  Int Equals op1=(op2==op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IEQ_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - IEQ R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI == op3I)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  INE_REG_REG_REG  Int Equals op1=(op2!=op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(INE_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - INE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI != op3RI)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  INE_REG_REG_INT  Int Equals op1=(op2!=op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(INE_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - INE R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI != op3I)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IGT_REG_REG_REG  Int Greater than op1=(op2>op3)                      pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IGT_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - IGT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI > op3RI)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IGT_REG_REG_INT  Int Greater than op1=(op2>op3)                      pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IGT_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - IGT R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI > op3I)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IGT_REG_INT_REG  Int Greater than op1=(op2>op3)                      pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IGT_REG_INT_REG, VM_ADVANCE(3);
            DEBUG("TRACE - IGT R%d,%d,r%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I > op3RI)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ILT_REG_REG_REG  Int Less than op1=(op2<op3)                         pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(ILT_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - ILT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI < op3RI)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ILT_REG_REG_INT  Int Less than op1=(op2<op3)                         pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(ILT_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - ILT R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI < op3I)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ILT_REG_INT_REG  Int Less than op1=(op2<op3)                         pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(ILT_REG_INT_REG, VM_ADVANCE(3);
            DEBUG("TRACE - ILT R%d,%d,r%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I < op3RI)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IGTE_REG_REG_REG  Int Greater Equal than op1=(op2>=op3)              pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IGTE_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - IGTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI >= op3RI)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IGTE_REG_REG_INT  Int Greater Equal than op1=(op2>=op3)              pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IGTE_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - IGTE R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI >= op3I)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IGTE_REG_INT_REG  Int Greater Equal than op1=(op2>=op3)              pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IGTE_REG_INT_REG, VM_ADVANCE(3);
            DEBUG("TRACE - IGTE R%d,%d,r%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I >= op3RI)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ILTE_REG_REG_REG  Int Less Equal than op1=(op2<=op3)                 pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(ILTE_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - ILTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI <= op3RI)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ILTE_REG_REG_INT  Int Less Equal than op1=(op2<=op3)                 pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(ILTE_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - ILTE R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI <= op3I)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ILTE_REG_INT_REG  Int Less Equal than op1=(op2<=op3)                 pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(ILTE_REG_INT_REG, VM_ADVANCE(3);
            DEBUG("TRACE - ILTE R%d,%d,r%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I <= op3RI)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IGTBR_ID_REG_REG  if op2>op3 ; goto op1                             pej 12 June 2023
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IGTBR_ID_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - IGTBR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2),(int)REG_IDX(3));
    if (op2RI > op3RI) {
        VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

    }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ILTBR_ID_REG_REG  if op2<op3 ; goto op1                             pej 14 June 2023
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(ILTBR_ID_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - ILTBR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2),(int)REG_IDX(3));
    if (op2RI < op3RI) {
        VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FEQ_REG_REG_REG  Float Equals op1=(op2==op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FEQ_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - FEQ R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF == op3RF)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FEQ_REG_REG_FLOAT  Float Equals op1=(op2==op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FEQ_REG_REG_FLOAT, VM_ADVANCE(3);
            DEBUG("TRACE - FEQ R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF == op3F)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FNE_REG_REG_REG  Float Not Equals op1=(op2!=op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FNE_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - FNE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF != op3RF)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FNE_REG_REG_FLOAT  Float Not Equals op1=(op2!=op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FNE_REG_REG_FLOAT, VM_ADVANCE(3);
            DEBUG("TRACE - FNE R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF != op3F)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FGT_REG_REG_REG  Float Greater than op1=(op2>op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FGT_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - FGT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF > op3RF)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FGT_REG_REG_FLOAT  Float Greater than op1=(op2>op3)
 *
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FGT_REG_REG_FLOAT, VM_ADVANCE(3);
            DEBUG("TRACE - FGT R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF > op3F)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FGT_REG_FLOAT_REG  Float Greater than op1=(op2>op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FGT_REG_FLOAT_REG, VM_ADVANCE(3);
            DEBUG("TRACE - FGT R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_INT(op2F > op3RF)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FLT_REG_REG_REG  Float Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FLT_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - FLT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF < op3RF)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FLT_REG_REG_FLOAT  Float Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FLT_REG_REG_FLOAT, VM_ADVANCE(3);
            DEBUG("TRACE - FLT R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF < op3F)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FLT_REG_FLOAT_REG  Float Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FLT_REG_FLOAT_REG, VM_ADVANCE(3);
            DEBUG("TRACE - FLT R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_INT(op2F < op3RF)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FGTE_REG_REG_REG  Float Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FGTE_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - FGTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF >= op3RF)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FGTE_REG_REG_FLOAT  Float Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FGTE_REG_REG_FLOAT, VM_ADVANCE(3);
            DEBUG("TRACE - FGTE R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF >= op3F)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FGTE_REG_FLOAT_REG  Float Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FGTE_REG_FLOAT_REG, VM_ADVANCE(3);
            DEBUG("TRACE - FGTE R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_INT(op2F >= op3RF)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FLTE_REG_REG_REG  Float Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FLTE_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - FLTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF <= op3RF)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FLTE_REG_REG_FLOAT  Float Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FLTE_REG_REG_FLOAT, VM_ADVANCE(3);
            DEBUG("TRACE - FLTE R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF <= op3F)
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FLTE_REG_FLOAT_REG  Float Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FLTE_REG_FLOAT_REG, VM_ADVANCE(3);
            DEBUG("TRACE - FLTE R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_INT(op2F <= op3RF)
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FGTBR_ID_REG_REG  if op2>op3 ; goto op1                            pej 14 June 2023
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FGTBR_ID_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - FGTBR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2),(int)REG_IDX(3));
    if (op2RF > op3RF) {
        VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FLTBR_ID_REG_REG  if op2>op3 ; goto op1                            pej 14 June 2023
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FLTBR_ID_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - FLTBR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2),(int)REG_IDX(3));
    if (op2RF < op3RF) {
        VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SEQ_REG_REG_REG  String Equals op1=(op2==op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SEQ_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SEQ R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(!string_cmp_value(op2R, op3R))
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SEQ_REG_REG_STRING String Equals op1=(op2==op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SEQ_REG_REG_STRING, VM_ADVANCE(3);
            DEBUG("TRACE - SEQ R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);
            REG_RETURN_INT(!string_cmp_const(op2R, op3S))
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  RSEQ_REG_REG_REG  String Equals op1=(op2=op3) non strict REXX comparison  pej 29. Nov 2021
 *  -----------------------------------------------------------------------------------
*/

)

RXVM_HANDLER(RSEQ_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - RSEQ R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(rxvm_trimmed_space_equal(op2R->string_value, op2R->string_length,
                                                    op3R->string_value, op3R->string_length))
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  RSEQ_REG_REG_STRING String Equals op1=(op2=op3)  non strict REXX comparison
 *  -----------------------------------------------------------------------------------
*/

)

RXVM_HANDLER(RSEQ_REG_REG_STRING, VM_ADVANCE(3);
            DEBUG("TRACE - RSEQ R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                  REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                  (CONSTSTRING_OP(3))->string);
            REG_RETURN_INT(rxvm_trimmed_space_equal(op2R->string_value, op2R->string_length,
                                                    op3S->string, op3S->string_len))
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SNE_REG_REG_REG  String Not Equals op1=(op2!=op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SNE_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SNE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) != 0)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SNE_REG_REG_STRING  String Not Equals op1=(op2!=op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SNE_REG_REG_STRING, VM_ADVANCE(3);
            DEBUG("TRACE - SNE R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);
            REG_RETURN_INT(string_cmp_const(op2R, op3S) != 0)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SGT_REG_REG_REG  String Greater than op1=(op2>op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SGT_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SGT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) > 0)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SGT_REG_REG_STRING  String Greater than op1=(op2>op3)
 *
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SGT_REG_REG_STRING, VM_ADVANCE(3);
            DEBUG("TRACE - SGT R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);

            REG_RETURN_INT(string_cmp_const(op2R, op3S) > 0)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SGT_REG_STRING_REG  String Greater than op1=(op2>op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SGT_REG_STRING_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SGT R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                (int) (CONSTSTRING_OP(2))->string_len,
                (CONSTSTRING_OP(2))->string, REG_IDX(3));
            REG_RETURN_INT(string_cmp_const(op3R, op2S) < 0)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SLT_REG_REG_REG  String Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SLT_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SLT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) < 0)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SLT_REG_REG_STRING  String Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SLT_REG_REG_STRING, VM_ADVANCE(3);
            DEBUG("TRACE - SLT R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);

            REG_RETURN_INT(string_cmp_const(op2R, op3S) < 0)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SLT_REG_STRING_REG  String Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SLT_REG_STRING_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SLT R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                (int) (CONSTSTRING_OP(2))->string_len,
                (CONSTSTRING_OP(2))->string, REG_IDX(3));
            REG_RETURN_INT(string_cmp_const(op3R, op2S) > 0)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SGTE_REG_REG_REG  String Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SGTE_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SGTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) >= 0)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SGTE_REG_REG_STRING  String Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SGTE_REG_REG_STRING, VM_ADVANCE(3);
            DEBUG("TRACE - SGTE R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);

            REG_RETURN_INT(string_cmp_const(op2R, op3S) >= 0)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SGTE_REG_STRING_REG  String Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SGTE_REG_STRING_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SGTE R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                (int) (CONSTSTRING_OP(2))->string_len,
                (CONSTSTRING_OP(2))->string, REG_IDX(3));
            REG_RETURN_INT(string_cmp_const(op3R, op2S) <= 0)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SLTE_REG_REG_REG  String Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SLTE_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SLTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) <= 0)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SLTE_REG_REG_STRING  String Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SLTE_REG_REG_STRING, VM_ADVANCE(3);
            DEBUG("TRACE - SLTE R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);
            REG_RETURN_INT(string_cmp_const(op2R, op3S) <= 0)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SLTE_REG_STRING_REG  String Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SLTE_REG_STRING_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SLTE R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                (int) (CONSTSTRING_OP(2))->string_len,
                (CONSTSTRING_OP(2))->string, REG_IDX(3));
            REG_RETURN_INT(string_cmp_const(op3R, op2S) >= 0)
            DISPATCH;


)

RXVM_HANDLER(REQ_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - REQ R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    REG_RETURN_INT(rxvm_loose_compare_values(op2R, op3R) == 0)
    DISPATCH;

)

RXVM_HANDLER(REQ_REG_REG_STRING, VM_ADVANCE(3);
    DEBUG("TRACE - REQ R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
        REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
        (CONSTSTRING_OP(3))->string);
    REG_RETURN_INT(rxvm_loose_compare_value_const(op2R, op3S) == 0)
    DISPATCH;

)

RXVM_HANDLER(REQ_REG_STRING_REG, VM_ADVANCE(3);
    DEBUG("TRACE - REQ R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
        (int) (CONSTSTRING_OP(2))->string_len,
        (CONSTSTRING_OP(2))->string, REG_IDX(3));
    REG_RETURN_INT(rxvm_loose_compare_const_value(op2S, op3R) == 0)
    DISPATCH;

)

RXVM_HANDLER(RNE_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - RNE R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    REG_RETURN_INT(rxvm_loose_compare_values(op2R, op3R) != 0)
    DISPATCH;

)

RXVM_HANDLER(RNE_REG_REG_STRING, VM_ADVANCE(3);
    DEBUG("TRACE - RNE R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
        REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
        (CONSTSTRING_OP(3))->string);
    REG_RETURN_INT(rxvm_loose_compare_value_const(op2R, op3S) != 0)
    DISPATCH;

)

RXVM_HANDLER(RNE_REG_STRING_REG, VM_ADVANCE(3);
    DEBUG("TRACE - RNE R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
        (int) (CONSTSTRING_OP(2))->string_len,
        (CONSTSTRING_OP(2))->string, REG_IDX(3));
    REG_RETURN_INT(rxvm_loose_compare_const_value(op2S, op3R) != 0)
    DISPATCH;

)

RXVM_HANDLER(RGT_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - RGT R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    REG_RETURN_INT(rxvm_loose_compare_values(op2R, op3R) > 0)
    DISPATCH;

)

RXVM_HANDLER(RGT_REG_REG_STRING, VM_ADVANCE(3);
    DEBUG("TRACE - RGT R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
        REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
        (CONSTSTRING_OP(3))->string);
    REG_RETURN_INT(rxvm_loose_compare_value_const(op2R, op3S) > 0)
    DISPATCH;

)

RXVM_HANDLER(RGT_REG_STRING_REG, VM_ADVANCE(3);
    DEBUG("TRACE - RGT R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
        (int) (CONSTSTRING_OP(2))->string_len,
        (CONSTSTRING_OP(2))->string, REG_IDX(3));
    REG_RETURN_INT(rxvm_loose_compare_const_value(op2S, op3R) > 0)
    DISPATCH;

)

RXVM_HANDLER(RGTE_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - RGTE R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    REG_RETURN_INT(rxvm_loose_compare_values(op2R, op3R) >= 0)
    DISPATCH;

)

RXVM_HANDLER(RGTE_REG_REG_STRING, VM_ADVANCE(3);
    DEBUG("TRACE - RGTE R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
        REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
        (CONSTSTRING_OP(3))->string);
    REG_RETURN_INT(rxvm_loose_compare_value_const(op2R, op3S) >= 0)
    DISPATCH;

)

RXVM_HANDLER(RGTE_REG_STRING_REG, VM_ADVANCE(3);
    DEBUG("TRACE - RGTE R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
        (int) (CONSTSTRING_OP(2))->string_len,
        (CONSTSTRING_OP(2))->string, REG_IDX(3));
    REG_RETURN_INT(rxvm_loose_compare_const_value(op2S, op3R) >= 0)
    DISPATCH;

)

RXVM_HANDLER(RLT_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - RLT R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    REG_RETURN_INT(rxvm_loose_compare_values(op2R, op3R) < 0)
    DISPATCH;

)

RXVM_HANDLER(RLT_REG_REG_STRING, VM_ADVANCE(3);
    DEBUG("TRACE - RLT R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
        REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
        (CONSTSTRING_OP(3))->string);
    REG_RETURN_INT(rxvm_loose_compare_value_const(op2R, op3S) < 0)
    DISPATCH;

)

RXVM_HANDLER(RLT_REG_STRING_REG, VM_ADVANCE(3);
    DEBUG("TRACE - RLT R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
        (int) (CONSTSTRING_OP(2))->string_len,
        (CONSTSTRING_OP(2))->string, REG_IDX(3));
    REG_RETURN_INT(rxvm_loose_compare_const_value(op2S, op3R) < 0)
    DISPATCH;

)

RXVM_HANDLER(RLTE_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - RLTE R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    REG_RETURN_INT(rxvm_loose_compare_values(op2R, op3R) <= 0)
    DISPATCH;

)

RXVM_HANDLER(RLTE_REG_REG_STRING, VM_ADVANCE(3);
    DEBUG("TRACE - RLTE R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
        REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
        (CONSTSTRING_OP(3))->string);
    REG_RETURN_INT(rxvm_loose_compare_value_const(op2R, op3S) <= 0)
    DISPATCH;

)

RXVM_HANDLER(RLTE_REG_STRING_REG, VM_ADVANCE(3);
    DEBUG("TRACE - RLTE R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
        (int) (CONSTSTRING_OP(2))->string_len,
        (CONSTSTRING_OP(2))->string, REG_IDX(3));
    REG_RETURN_INT(rxvm_loose_compare_const_value(op2S, op3R) <= 0)
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  COPY_REG_REG  Copy op2 to op1
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(COPY_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - COPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            copy_value(op1R, op2R);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SCOPY_REG_REG  Copy String op2 to op1
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(SCOPY_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - SCOPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            copy_string_value(op1R, op2R);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BCOPY_REG_REG  Copy Binary op2 to op1
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BCOPY_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - BCOPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            copy_binary_value(op1R, op2R);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ICOPY_REG_REG  Copy Integer op2 to op1
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(ICOPY_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - ICOPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            RXVM_INSTRUMENTATION_VALUE_TYPED(
                    RXVM_PROFILE_VALUE_INTEGER_COPY,
                    RXVM_PROFILE_VALUE_SCALAR, sizeof(op2R->int_value));
            op1R->int_value = op2R->int_value;
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FCOPY_REG_REG  Copy Float op2 to op1
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FCOPY_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - FCOPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            RXVM_INSTRUMENTATION_VALUE_TYPED(
                    RXVM_PROFILE_VALUE_FLOAT_COPY,
                    RXVM_PROFILE_VALUE_SCALAR, sizeof(op2R->float_value));
            op1R->float_value = op2R->float_value;
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ACOPY_REG_REG Copy status Attributes op2 to op1
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(ACOPY_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - ACOPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            RXVM_INSTRUMENTATION_VALUE_TYPED(
                    RXVM_PROFILE_VALUE_STATUS_COPY,
                    RXVM_PROFILE_VALUE_SCALAR,
                    sizeof(op2R->status.all_type_flags));
            op1R->status.all_type_flags = op2R->status.all_type_flags;
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  INC_REG  Increment Int (op1++)                                      pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(INC_REG, VM_ADVANCE(1);
            DEBUG("TRACE - INC R%lu\n", REG_IDX(1));
            {
                rxinteger result;
                if (!rxinteger_checked_add(
                        current_locals[REG_IDX(1)]->int_value, 1, &result)) {
                    SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
                }
                else current_locals[REG_IDX(1)]->int_value = result;
                DISPATCH;
            }

/* ------------------------------------------------------------------------------------
 *  IDIV_REG_REG_INT  Integer Divide (op1=op2/op3)                      pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IDIV_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - IDIV R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            if (op3I == 0) { SET_SIGNAL(RXSIGNAL_DIVISION_BY_ZERO) }
            else if (op2RI == RXINTEGER_MIN && op3I == -1) { SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW) }
            else REG_RETURN_INT(op2RI / op3I)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IDIV_REG_INT_REG  Integer Divide (op1=op2/op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IDIV_REG_INT_REG, VM_ADVANCE(3);
            DEBUG("TRACE - IDIV R%d,%d,R%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            if (op3RI == 0) { SET_SIGNAL(RXSIGNAL_DIVISION_BY_ZERO) }
            else if (op2I == RXINTEGER_MIN && op3RI == -1) { SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW) }
            else REG_RETURN_INT(op2I / op3RI)
            DISPATCH;

/* -----------------------------------------------------------------------------------
 *  IDIV_REG_REG_REG  Integer Divide (op1=op2/op3)                      pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IDIV_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - IDIV R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            if (op3RI == 0) { SET_SIGNAL(RXSIGNAL_DIVISION_BY_ZERO) }
            else if (op2RI == RXINTEGER_MIN && op3RI == -1) { SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW) }
            else REG_RETURN_INT(op2RI / op3RI)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IMOD_REG_REG_INT  Integer Modulo (op1=op2 & op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IMOD_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - IMOD R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            if (op3I == 0) { SET_SIGNAL(RXSIGNAL_DIVISION_BY_ZERO) }
            else if (op2RI == RXINTEGER_MIN && op3I == -1) { SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW) }
            else REG_RETURN_INT(op2RI % op3I)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IMOD_REG_INT_REG  Integer Modulo (op1=op2 % op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IMOD_REG_INT_REG, VM_ADVANCE(3);
            DEBUG("TRACE - IMOD R%d,%d,R%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            if (op3RI == 0) { SET_SIGNAL(RXSIGNAL_DIVISION_BY_ZERO) }
            else if (op2I == RXINTEGER_MIN && op3RI == -1) { SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW) }
            else REG_RETURN_INT(op2I % op3RI)
            DISPATCH;

/* -----------------------------------------------------------------------------------
 *  IMOD_REG_REG_REG  Integer Modulo (op1=op2 % op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IMOD_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - IMOD R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            if (op3RI == 0) { SET_SIGNAL(RXSIGNAL_DIVISION_BY_ZERO) }
            else if (op2RI == RXINTEGER_MIN && op3RI == -1) { SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW) }
            else REG_RETURN_INT(op2RI % op3RI)
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FMOD_REG_REG_FLOAT Float Modulo (op1=op2 & op3)
 *  -----------------------------------------------------------------------------------
*/

)

RXVM_HANDLER(FMOD_REG_REG_FLOAT, VM_ADVANCE(3);
    DEBUG("TRACE - FMOD R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
    REG_RETURN_FLOAT(fmod(op2RF, op3F))
    DISPATCH;
/* ------------------------------------------------------------------------------------
*  FMOD_REG_FLOAT_REG  Float Modulo (op1=op2 % op3)
*  -----------------------------------------------------------------------------------
*/

)

RXVM_HANDLER(FMOD_REG_FLOAT_REG, VM_ADVANCE(3);
    DEBUG("TRACE - FMOD R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
    REG_RETURN_FLOAT(fmod(op2F, op3RF))
    DISPATCH;
/* -----------------------------------------------------------------------------------
*  FMOD_REG_REG_REG  Float Modulo (op1=op2 % op3)
*  -----------------------------------------------------------------------------------
*/

)

RXVM_HANDLER(FMOD_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - FMOD R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    REG_RETURN_FLOAT(fmod(op2RF, op3RF))
    DISPATCH;



/* ------------------------------------------------------------------------------------
 *  DMOD_REG_REG_DECIMAL Decimal Modulo (op1=op2 & op3)
 *  -----------------------------------------------------------------------------------
*/

)

RXVM_HANDLER(DMOD_REG_REG_DECIMAL, VM_ADVANCE(3);
    DEBUG("TRACE - DMOD R%d,R%d,%s\n", (int)REG_IDX(1), (int)REG_IDX(2), op3S->string);
    {
        value *op = decimal_literal_value(current_frame->decimal, op3S->string);
        value *work = value_f();
        // First, Calculate the decimal division (truncated)
        current_frame->decimal->decimalDiv(current_frame->decimal, work, op2R, op);
        current_frame->decimal->decimalTruncate(current_frame->decimal, work, work);
        // Now calculate the remainder
        current_frame->decimal->decimalMul(current_frame->decimal, work, work, op);
        current_frame->decimal->decimalSub(current_frame->decimal, op1R, op2R, work);
        // Clear the temporary value
        value_free(work);
        free_decimal_literal_value(op);
    }
    // Check for errors
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
*  FMOD_REG_DECIMAL_REG  Decimal Modulo (op1=op2 % op3)
*  -----------------------------------------------------------------------------------
*/

)

RXVM_HANDLER(DMOD_REG_DECIMAL_REG, VM_ADVANCE(3);
    DEBUG("TRACE - FMOD R%d,%s,R%d\n", (int)REG_IDX(1), op2S->string, (int)REG_IDX(3));
    {
        value *op = decimal_literal_value(current_frame->decimal, op2S->string);
        value *work = value_f();
        // First, Calculate the decimal division (truncated)
        current_frame->decimal->decimalDiv(current_frame->decimal, work, op, op3R);
        current_frame->decimal->decimalTruncate(current_frame->decimal, work, work);
        // Now calculate the remainder
        current_frame->decimal->decimalMul(current_frame->decimal, work, work, op3R);
        current_frame->decimal->decimalSub(current_frame->decimal, op1R, op, work);
        // Clear the temporary value
        value_free(work);
        free_decimal_literal_value(op);
    }
    // Check for errors
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* -----------------------------------------------------------------------------------
*  FMOD_REG_REG_REG Decimal Modulo (op1=op2 % op3)
*  -----------------------------------------------------------------------------------
*/

)

RXVM_HANDLER(DMOD_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - DMOD R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        value *work = value_f();
        // First, Calculate the decimal division (truncated)
        current_frame->decimal->decimalDiv(current_frame->decimal, work, op2R, op3R);
        current_frame->decimal->decimalTruncate(current_frame->decimal, work, work);
        // Now calculate the remainder
        current_frame->decimal->decimalMul(current_frame->decimal, work, work, op3R);
        current_frame->decimal->decimalSub(current_frame->decimal, op1R, op2R, work);
        // Clear the temporary value
        value_free(work);
    }
    // Check for errors
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
  *  IOR_REG_REG_REG bitwise OR (op1=op2|op3)                           pej 17 Oct 2021
  *  -----------------------------------------------------------------------------------
  */

)

RXVM_HANDLER(IOR_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - IOR R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI | op3RI)
            DISPATCH;
 /* -----------------------------------------------------------------------------------
  *  IOR_REG_REG_INT  bitwise OR (op1=op2|op3)                          pej 17 Oct 2021
  *  ----------------------------------------------------------------------------------
  */

)

RXVM_HANDLER(IOR_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - IOR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI | op3I)
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IAND_REG_REG_INT  bitwise AND (op1=op2&op3)                         pej 17 Oct 2021
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IAND_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - IAND R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI & op3I)
            DISPATCH;
/* -----------------------------------------------------------------------------------
 *  IAND_REG_REG_REG  bitwise AND (op1=op2&op3)                        pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IAND_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - IAND R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI & op3RI)
            DISPATCH;

/* -----------------------------------------------------------------------------------
 *  IXOR_REG_REG_REG  bitwise XOR (op1=op2^op3)                        pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IXOR_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - IXOR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI ^ op3RI)
            DISPATCH;

/* -----------------------------------------------------------------------------------
 *  IXOR_REG_REG_INT  bitwise XOR (op1=op2^op3)                        pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(IXOR_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - IXOR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI ^ op3I)
            DISPATCH;

/* -----------------------------------------------------------------------------------
 *  ISHL_REG_REG_REG  bitwise shift logical left (op1=op2<<op3)         pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(ISHL_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - ISHL R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI << op3RI)
            DISPATCH;

/* -----------------------------------------------------------------------------------
 *  ISHL_REG_REG_INT  bitwise shift logical left (op1=op2<<op3)         pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(ISHL_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - ISHL R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI << op3I)
            DISPATCH;
/* -----------------------------------------------------------------------------------
 *  ISHR_REG_REG_REG  bitwise shift logical right (op1=op2>>op3)       pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(ISHR_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - ISHR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI >> op3RI)
            DISPATCH;

/* -----------------------------------------------------------------------------------
 *  ISHR_REG_REG_INT  bitwise shift logical right (op1=op2>>op3)       pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(ISHR_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - IXSHL R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI >> op3I)
            DISPATCH;
/* -----------------------------------------------------------------------------------
 *  INOT_REG_REG  inverts all bits of an integer (op1=~op2)            pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(INOT_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - INOT R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            REG_RETURN_INT(~op2RI)
            DISPATCH;

/* -----------------------------------------------------------------------------------
 *  INOT_REG_INT  inverts all bits of an integer (op1=~op2)            pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */

)
