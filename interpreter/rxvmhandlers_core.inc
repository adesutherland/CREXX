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

/* Sequential core handler. */
RXVM_HANDLER(LOAD_REG_INT,
    VM_ADVANCE(2);
    DEBUG("TRACE - LOAD R%d,%d\n", (int) REG_IDX(1), (int) op2I);
    set_int(op1R, op2I);
    DISPATCH;
)

/* Branch core handler. */
RXVM_HANDLER(BR_ID,
    DEBUG("TRACE - BR 0x%x\n", (unsigned int) REG_IDX(1));
    VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);
    DISPATCH;
)

/* Signal core handler. */
RXVM_HANDLER(SIGNAL_STRING,
    VM_ADVANCE(1);
    DEBUG("TRACE - SIGNAL \"%.*s\"\n", (int) op1S->string_len, op1S->string);
    SET_SIGNAL_FROM_NAME(op1S->string);
    DISPATCH;
)

/* Call/frame core handler. */
RXVM_HANDLER(CALL_FUNC,
    VM_ADVANCE(1);
    {
        proc_runtime *called_function = PROC_OP(1);
        DEBUG("TRACE - CALL %s()\n", called_function->name);
        if (called_function->start == SIZE_MAX) {
            RXVM_INSTRUMENTATION_CALL(
                    called_function->binarySpace
                        ? RXVM_PROFILE_CALL_DIRECT_BYTECODE
                        : RXVM_PROFILE_CALL_DIRECT_NATIVE,
                    called_function, 0, RXVM_PROFILE_FRAME_NONE_FAILED,
                    RXVM_PROFILE_CALL_UNRESOLVED, current_frame,
                    current_module->module_number, VM_CANONICAL_INDEX(pc),
                    0, 0);
            SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, called_function->name)
            DISPATCH;
        }
        if (called_function->binarySpace == 0) {
            RXVM_INSTRUMENTATION_CALL(
                    RXVM_PROFILE_CALL_DIRECT_NATIVE, called_function, 0,
                    RXVM_PROFILE_FRAME_NO_CHILD_NATIVE,
                    RXVM_PROFILE_CALL_SUCCESS, current_frame,
                    current_module->module_number, VM_CANONICAL_INDEX(pc),
                    0, 0);
            RXVM_INSTRUMENTATION_NATIVE_BEGIN(called_function);
            rxvm_callfunc((void *) (called_function->start), 0, NULL, NULL,
                          signal_value);
            RXVM_INSTRUMENTATION_NATIVE_END();
            INTERRUPT_FROM_RXPA_SIGNAL(signal_value);
        } else {
            temp_frame = frame_f(called_function, 0, current_frame, next_pc, 0);
            if (!temp_frame) {
                RXVM_INSTRUMENTATION_CALL(
                        RXVM_PROFILE_CALL_DIRECT_BYTECODE,
                        called_function, 0,
                        RXVM_PROFILE_FRAME_NONE_FAILED,
                        RXVM_PROFILE_CALL_FRAME_FAILED, current_frame,
                        current_module->module_number,
                        VM_CANONICAL_INDEX(pc), 0, 0);
                SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Unable to allocate stack frame")
                DISPATCH;
            }
            RXVM_INSTRUMENTATION_CALL(
                    RXVM_PROFILE_CALL_DIRECT_BYTECODE, called_function, 0,
                    RXVM_PROFILE_FRAME_LAST_ACTIVATION,
                    RXVM_PROFILE_CALL_SUCCESS, current_frame,
                    current_module->module_number, VM_CANONICAL_INDEX(pc),
                    0, 0);
            VM_ACTIVATE_FRAME(temp_frame, RXVM_TRANSITION_CALL);
            VM_SELECT_INDEX(called_function->start, RXVM_TRANSITION_CALL);
        }
    }
    DISPATCH;
)

/* Return/terminal core handlers. */
RXVM_HANDLER(RET_INT,
    DEBUG("TRACE - RET %d\n", (int) op1I);
    {
        RXVM_INSTRUMENTATION_RETURN(
                !current_frame->parent
                    ? RXVM_PROFILE_RETURN_TERMINAL_EXTERNAL
                    : (current_frame->return_reg
                        ? RXVM_PROFILE_RETURN_IMMEDIATE
                        : RXVM_PROFILE_RETURN_VALUE_IGNORED));
        next_pc = current_frame->return_pc;
        {
            unsigned char is_interrupt = current_frame->is_interrupt;
            if (current_frame->return_reg)
                current_frame->return_reg->int_value = op1I;
            temp_frame = current_frame;
            VM_ACTIVATE_FRAME_OR_NULL(current_frame->parent,
                                      RXVM_TRANSITION_RETURN);
            if (!current_frame) {
                int i;
                int j;
                int num_args;
                DEBUG("TRACE - RETURNING FROM MAIN()\n");
                if (context->ext_proc && context->ext_args) {
                    int k;
                    int m;
                    int nargs = (int) temp_frame->baselocals[
                            temp_frame->procedure->binarySpace->globals +
                            temp_frame->procedure->locals]->int_value;
                    for (k = 0,
                         m = temp_frame->procedure->binarySpace->globals +
                             temp_frame->procedure->locals + 1;
                         k < nargs && k < context->ext_argc;
                         k++, m++) {
                        copy_value(context->ext_args[k], temp_frame->locals[m]);
                    }
                }
                num_args = (int) temp_frame->baselocals[
                        temp_frame->procedure->binarySpace->globals +
                        temp_frame->procedure->locals]->int_value;
                for (i = 0,
                     j = temp_frame->procedure->binarySpace->globals +
                         temp_frame->procedure->locals + 1;
                     i < num_args;
                     i++, j++) {
                    value_free(temp_frame->baselocals[j]);
                }
                rc = (int) op1I;
                free_frame(temp_frame);
                arguments_array = 0;
                RXVM_HANDLER_FINISH();
            }
            if (current_frame->decimal) {
                current_frame->decimal->num_context = &current_frame->num_context;
                current_frame->decimal->syncNumericContext(current_frame->decimal);
            }
            HANDLE_INTERRUPT_ACTION_RETURN()
            free_frame(temp_frame);
            VM_SELECT_POINTER(next_pc, RXVM_TRANSITION_RETURN);
            if (is_interrupt == RXSIGNAL_BREAKPOINT)
                VM_RESUME_INTERRUPTED(is_interrupt);
            DISPATCH;
        }
    }
)

/* Process-private fused handlers. */
RXVM_PRIVATE_HANDLER(PRIVATE_R1_RELINK,
        RXVM_PRIVATE_R1_RELINK_REG_REG, OP_UNLINK_REG,
    DEBUG("TRACE - UNLINK R%lu\n", REG_IDX(1));
    {
        size_t site_index = VM_CANONICAL_INDEX(pc);
        size_t destination = REG_IDX(1);
        size_t source_register = (pc + 4)->index;
        value *source = current_locals[source_register];
        rxvm_reference_cell *cell;
        value *target;

        current_locals[destination] = current_frame->baselocals[destination];
        if (context->debug_mode ||
                (pending_interrupts &
                 rxsignal_mask(RXSIGNAL_BREAKPOINT)) != 0) {
            VM_SELECT_INDEX(site_index + 2u, RXVM_TRANSITION_SEQUENTIAL);
            DISPATCH;
        }
        cell = rxvm_reference_payload_cell(source);
        target = rxvm_reference_cell_target(cell);
        if (!target) {
            VM_SELECT_INDEX(site_index + 2u, RXVM_TRANSITION_SEQUENTIAL);
            DISPATCH;
        }
        current_locals[destination] = target;
        VM_SELECT_INDEX(site_index + 5u, RXVM_TRANSITION_SEQUENTIAL);
        DISPATCH;
    }
)
