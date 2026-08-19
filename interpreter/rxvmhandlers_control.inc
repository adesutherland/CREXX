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

/* Control-flow RXAS handler definitions; body text is compiled by RXVM_HANDLER. */
RXVM_HANDLER(BPON, VM_ADVANCE(0);
            DEBUG("TRACE - BPON\n");
            rxvm_signal_pending_or(&pending_interrupts,
                                   rxsignal_mask(RXSIGNAL_BREAKPOINT));
            DISPATCH;

        /* Enable Breakpoints with op1 handler */

)

RXVM_HANDLER(BPON_FUNC, VM_ADVANCE(1);
            {
                proc_runtime *signal_function = PROC_OP(1);
                DEBUG("TRACE - BPON %s()\n", signal_function->name);
                if (!rxsignal_ensure_private_interrupt_table(current_frame))
                    RXVM_HANDLER_INTERRUPT_TABLE_OOM();
                current_frame->interrupt_table[RXSIGNAL_BREAKPOINT-1].response = RXSIGNAL_RESPONSE_CALL;
                current_frame->interrupt_table[RXSIGNAL_BREAKPOINT-1].function = signal_function;
                current_frame->interrupt_table[RXSIGNAL_BREAKPOINT-1].jump = 0;
                current_frame->interrupt_table[RXSIGNAL_BREAKPOINT-1].frame = 0;
                current_frame->interrupt_table[RXSIGNAL_BREAKPOINT-1].value_register = 0;
                current_frame->interrupt_table[RXSIGNAL_BREAKPOINT-1].stack_marker = 0;
                rxvm_signal_pending_or(&pending_interrupts,
                                       rxsignal_mask(RXSIGNAL_BREAKPOINT));
            }
            DISPATCH;

        /* Disable Breakpoints */

)

RXVM_HANDLER(BPOFF, VM_ADVANCE(0);
            DEBUG("TRACE - BPOFF\n");
            rxvm_signal_pending_and(&pending_interrupts,
                                    ~rxsignal_mask(RXSIGNAL_BREAKPOINT));
            DISPATCH;

        /* Set Signal op1 Handle to Ignore */

)

RXVM_HANDLER(SIGIGNORE_STRING, VM_ADVANCE(1);
            DEBUG("TRACE - SIGIGNORE \"%.*s\"\n", (int) op1S->string_len, op1S->string);
            {
                size_t sig = string_to_interrupt(op1S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    if (!rxsignal_ensure_private_interrupt_table(current_frame))
                        RXVM_HANDLER_INTERRUPT_TABLE_OOM();
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_IGNORE;
                    current_frame->interrupt_table[sig-1].function = 0;
                    current_frame->interrupt_table[sig-1].jump = 0;
                    current_frame->interrupt_table[sig-1].frame = 0;
                    current_frame->interrupt_table[sig-1].value_register = 0;
                    current_frame->interrupt_table[sig-1].stack_marker = 0;
                    ignore_interrupt((int)sig); // Set the corresponding native interrupt to ignore
                }
            }
            DISPATCH;

        /* Set Signal op1 Handle to Halt */

)

RXVM_HANDLER(SIGHALT_STRING, VM_ADVANCE(1);
            DEBUG("TRACE - SIGHALT \"%.*s\"\n", (int) op1S->string_len, op1S->string);
            {
                size_t sig = string_to_interrupt(op1S->string);
                if (sig == RXSIGNAL_MAX) { // Kill can be set to halt
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    if (!rxsignal_ensure_private_interrupt_table(current_frame))
                        RXVM_HANDLER_INTERRUPT_TABLE_OOM();
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_HALT;
                    current_frame->interrupt_table[sig-1].function = 0;
                    current_frame->interrupt_table[sig-1].jump = 0;
                    current_frame->interrupt_table[sig-1].frame = 0;
                    current_frame->interrupt_table[sig-1].value_register = 0;
                    current_frame->interrupt_table[sig-1].stack_marker = 0;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH;

        /* Set Signal op1 Handle to Silent Halt */

)

RXVM_HANDLER(SIGSHALT_STRING, VM_ADVANCE(1);
            DEBUG("TRACE - SIGSHALT \"%.*s\"\n", (int) op1S->string_len, op1S->string);
            {
                size_t sig = string_to_interrupt(op1S->string);
                if (sig == RXSIGNAL_MAX) { // KILL can be set to silent halt
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    if (!rxsignal_ensure_private_interrupt_table(current_frame))
                        RXVM_HANDLER_INTERRUPT_TABLE_OOM();
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_SILENT_HALT;
                    current_frame->interrupt_table[sig-1].function = 0;
                    current_frame->interrupt_table[sig-1].jump = 0;
                    current_frame->interrupt_table[sig-1].frame = 0;
                    current_frame->interrupt_table[sig-1].value_register = 0;
                    current_frame->interrupt_table[sig-1].stack_marker = 0;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH;

        /* Set Signal op2 Handle to Branch to op1 */

)

RXVM_HANDLER(SIGBR_ID_STRING, VM_ADVANCE(2);
            DEBUG("TRACE - SIGBR 0x%x,\"%.*s\"\n", (unsigned int)REG_IDX(1), (int)op2S->string_len, op2S->string);
            {
                size_t sig = string_to_interrupt(op2S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    if (!rxsignal_ensure_private_interrupt_table(current_frame))
                        RXVM_HANDLER_INTERRUPT_TABLE_OOM();
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_BRANCH;
                    current_frame->interrupt_table[sig-1].jump = REG_IDX(1);
                    current_frame->interrupt_table[sig-1].function = 0;
                    current_frame->interrupt_table[sig-1].frame = current_frame;
                    current_frame->interrupt_table[sig-1].value_register = 0;
                    current_frame->interrupt_table[sig-1].stack_marker = current_frame->interrupt_stack;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH;

        /* Set Signal op3 Handle to Branch to op1 and bind a .signal object to op2 */

)

RXVM_HANDLER(SIGBRV_ID_REG_STRING, VM_ADVANCE(3);
            DEBUG("TRACE - SIGBRV 0x%x,R%d,\"%.*s\"\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)op3S->string_len, op3S->string);
            {
                size_t sig = string_to_interrupt(op3S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    if (!rxsignal_ensure_private_interrupt_table(current_frame))
                        RXVM_HANDLER_INTERRUPT_TABLE_OOM();
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_BRANCH_VALUE;
                    current_frame->interrupt_table[sig-1].jump = REG_IDX(1);
                    current_frame->interrupt_table[sig-1].function = 0;
                    current_frame->interrupt_table[sig-1].frame = current_frame;
                    current_frame->interrupt_table[sig-1].value_register = REG_IDX(2);
                    current_frame->interrupt_table[sig-1].stack_marker = current_frame->interrupt_stack;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH;

        /* Set Signal op2 Handle to Call op1 */

)

RXVM_HANDLER(SIGCALL_FUNC_STRING, VM_ADVANCE(2);
            {
                proc_runtime *signal_function = PROC_OP(1);
                DEBUG("TRACE - SIGCALL %s(),\"%.*s\"\n", signal_function->name, (int)op2S->string_len, op2S->string);

                size_t sig = string_to_interrupt(op2S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    if (!rxsignal_ensure_private_interrupt_table(current_frame))
                        RXVM_HANDLER_INTERRUPT_TABLE_OOM();
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_CALL;
                    current_frame->interrupt_table[sig-1].function = signal_function;
                    current_frame->interrupt_table[sig-1].jump = 0;
                    current_frame->interrupt_table[sig-1].frame = 0;
                    current_frame->interrupt_table[sig-1].value_register = 0;
                    current_frame->interrupt_table[sig-1].stack_marker = 0;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH;

        /* Set Signal op2 Handle to action-aware Call op1 */

)

RXVM_HANDLER(SIGCALLA_FUNC_STRING, VM_ADVANCE(2);
            {
                proc_runtime *signal_function = PROC_OP(1);
                DEBUG("TRACE - SIGCALLA %s(),\"%.*s\"\n", signal_function->name, (int)op2S->string_len, op2S->string);

                size_t sig = string_to_interrupt(op2S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    if (!rxsignal_ensure_private_interrupt_table(current_frame))
                        RXVM_HANDLER_INTERRUPT_TABLE_OOM();
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_CALL_ACTION;
                    current_frame->interrupt_table[sig-1].function = signal_function;
                    current_frame->interrupt_table[sig-1].jump = 0;
                    current_frame->interrupt_table[sig-1].frame = 0;
                    current_frame->interrupt_table[sig-1].value_register = 0;
                    current_frame->interrupt_table[sig-1].stack_marker = 0;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH;

        /* Set Signal op3 Handle to Call op2 returning to op1 */

)

RXVM_HANDLER(SIGCALLBR_ID_FUNC_STRING, VM_ADVANCE(3);
            DEBUG("TRACE - SIGCALLBR 0x%x,%s(),\"%.*s\"\n", (unsigned int)REG_IDX(1), PROC_OP(2)->name, (int)op3S->string_len, op3S->string);
            {
                proc_runtime *signal_function = PROC_OP(2);
                size_t sig = string_to_interrupt(op3S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    if (!rxsignal_ensure_private_interrupt_table(current_frame))
                        RXVM_HANDLER_INTERRUPT_TABLE_OOM();
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_CALL_BRANCH;
                    current_frame->interrupt_table[sig-1].function = signal_function;
                    current_frame->interrupt_table[sig-1].jump = REG_IDX(1);
                    current_frame->interrupt_table[sig-1].frame = current_frame;
                    current_frame->interrupt_table[sig-1].value_register = 0;
                    current_frame->interrupt_table[sig-1].stack_marker = current_frame->interrupt_stack;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH;

        /* Set Signal op1 Handle to Return */

)

RXVM_HANDLER(SIGRET_STRING, VM_ADVANCE(1);
            DEBUG("TRACE - SIGRET \"%.*s\"\n", (int)op1S->string_len, op1S->string);
            {
                size_t sig = string_to_interrupt(op1S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    if (!rxsignal_ensure_private_interrupt_table(current_frame))
                        RXVM_HANDLER_INTERRUPT_TABLE_OOM();
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_RETURN;
                    current_frame->interrupt_table[sig-1].function = 0;
                    current_frame->interrupt_table[sig-1].jump = 0;
                    current_frame->interrupt_table[sig-1].frame = 0;
                    current_frame->interrupt_table[sig-1].value_register = 0;
                    current_frame->interrupt_table[sig-1].stack_marker = 0;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH;

        /* Push current Signal op1 handler on the current frame handler stack */

)

RXVM_HANDLER(SIGPUSH_STRING, VM_ADVANCE(1);
            DEBUG("TRACE - SIGPUSH \"%.*s\"\n", (int)op1S->string_len, op1S->string);
            {
                size_t sig = string_to_interrupt(op1S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) {
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                } else {
                    rxsignal_push_handler(current_frame, (unsigned char)sig);
                }
            }
            DISPATCH;

        /* Pop and restore current Signal op1 handler from the current frame handler stack */

)

RXVM_HANDLER(SIGPOP_STRING, VM_ADVANCE(1);
            DEBUG("TRACE - SIGPOP \"%.*s\"\n", (int)op1S->string_len, op1S->string);
            {
                size_t sig = string_to_interrupt(op1S->string);
                rxsignal_pop_result pop_result = RXSIGNAL_POP_NOT_FOUND;
                if (sig != RXSIGNAL_MAX && sig != RXSIGNAL_KILL)
                    pop_result = rxsignal_pop_handler(current_frame, (unsigned char)sig);
                if (pop_result == RXSIGNAL_POP_NO_MEMORY) RXVM_HANDLER_INTERRUPT_TABLE_OOM();
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL ||
                    pop_result == RXSIGNAL_POP_NOT_FOUND) {
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
            }
            DISPATCH;

        /* RXSIGNAL_STRING Signal type op1 */

)

RXVM_HANDLER(SIGNAL_REG, VM_ADVANCE(1);
            DEBUG("TRACE - SIGNAL R%d\n", (int)REG_IDX(1));
            null_terminate_string_buffer(op1R);
            SET_SIGNAL_FROM_NAME(op1R->string_value);
            DISPATCH;

        /* SIGNALT_STRING_REG Signal type op1 if op2 true */

)

RXVM_HANDLER(SIGNALT_STRING_REG, VM_ADVANCE(2);
            DEBUG("TRACE - SIGNALT \"%.*s\",R%d\n", (int)op1S->string_len, op1S->string, (int)REG_IDX(2));
            if (op2RI) {
                SET_SIGNAL_FROM_NAME(op1S->string);
            }
            DISPATCH;

        /* SIGNALF_STRING_REG Signal type op1 if op2 true */

)

RXVM_HANDLER(SIGNALF_STRING_REG, VM_ADVANCE(2);
            DEBUG("TRACE - SIGNALF \"%.*s\",R%d\n", (int)op1S->string_len, op1S->string, (int)REG_IDX(2));
            if (!op2RI) {
                SET_SIGNAL_FROM_NAME(op1S->string);
            }
            DISPATCH;

        /* SIGNAL_STRING_STRING Signal type op1 (message op2) */

)

RXVM_HANDLER(SIGNAL_STRING_STRING, VM_ADVANCE(2);
            DEBUG("TRACE - SIGNAL \"%.*s\",\"%.*s\"\n", (int)op1S->string_len, op1S->string, (int)op2S->string_len, op2S->string);
            SET_SIGNAL_MSG_FROM_NAME(op1S->string, op2S->string);
            DISPATCH;

        /* SIGNAL_STRING_REG Signal type op1 (payload op2) */

)

RXVM_HANDLER(SIGNAL_STRING_REG, VM_ADVANCE(2);
            DEBUG("TRACE - SIGNAL \"%.*s\",R%d\n", (int)op1S->string_len, op1S->string, (int)REG_IDX(2));
            SET_SIGNAL_PAYLOAD_FROM_NAME(op1S->string, op2R);
            DISPATCH;

        /* SIGNAL_REG_REG Signal type op1 (payload op2) */

)

RXVM_HANDLER(SIGNAL_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - SIGNAL R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            null_terminate_string_buffer(op1R);
            SET_SIGNAL_PAYLOAD_FROM_NAME(op1R->string_value, op2R);
            DISPATCH;

        /* SIGNALT_STRING_STRING_REG Signal type op1 (message op2) if op3 true */

)

RXVM_HANDLER(SIGNALT_STRING_STRING_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SIGNALT \"%.*s\",\"%.*s\",R%d\n", (int)op1S->string_len, op1S->string, (int)op2S->string_len, op2S->string, (int)REG_IDX(2));
            if (op3RI) {
                SET_SIGNAL_MSG_FROM_NAME(op1S->string, op2S->string);
            }
            DISPATCH;

        /* SIGNALF_STRING_STRING_REG Signal type op1 (message op2) if op3 true */

)

RXVM_HANDLER(SIGNALF_STRING_STRING_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SIGNALF \"%.*s\",\"%.*s\"\",R%d\n", (int)op1S->string_len, op1S->string, (int)op2S->string_len, op2S->string, (int)REG_IDX(2));
            if (!op3RI) {
                SET_SIGNAL_MSG_FROM_NAME(op1S->string, op2S->string);
            }
            DISPATCH;

        /* Meta Instructions */

        /* Load Module (op1 = module num of loaded op2) */

)

RXVM_HANDLER(METALOADMODULE_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - METALOADMODULE R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2));
            {
                null_terminate_string_buffer(op2R);
                /* Load the module */
                op1R->int_value = rxldmod(context, op2R->string_value);
                if (op1R->int_value > 0) {
                    /* Resolve canonical operands before copying the execution image. */
                    rxvm_link(context);
                    /* If successfully loaded, prepare execution state inside run(). */
                    int mod;
                    DEBUG("Preparing execution images\n");
                    for (mod = 0; mod < op1R->int_value; mod++) {
                        module *loaded_module = context->modules[mod];
                        VM_PREPARE_EXECUTION_IMAGE(loaded_module);
                    }
                    RXVM_INSTRUMENTATION_MODULES_CHANGED(context);
                }
            }
            DISPATCH;

            /* Load Instruction Code (op1 = (inst)op2[op3]) */

)

RXVM_HANDLER(METALOADINST_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - METALOADINST R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            {
                bin_code inst = context->modules[op2R->int_value - 1]->segment.binary[op3R->int_value];
                op1R->int_value = inst.instruction.opcode;
            }
            DISPATCH;

            /* Loaded Modules (op1=array loaded modules) */

)

RXVM_HANDLER(METALOADEDMODULES_REG, VM_ADVANCE(1);
            DEBUG("TRACE - METALOADEDMODULES R%d\n", (int) REG_IDX(1));
            /* op1R will become an array of module names */
            value_zero(op1R);
            set_num_attributes(op1R, context->num_modules);
            op1R->int_value = (rxinteger) context->num_modules; /* The cREXX convention for arrays */
            for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
                set_null_string(op1R->attributes[mod_index], context->modules[mod_index]->name);
            }
            DISPATCH;

        /* Loaded Exposed Procedures (op1 = array procedures in module op2) */

)

RXVM_HANDLER(METALOADEDEPROCS_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - METALOADEDEPROCS R%d\n", (int) REG_IDX(1));
            {
                chameleon_constant *c_entry;
                proc_constant *p_entry;
                expose_proc_constant *e_entry;
                int i;
                size_t entries;
                value *entry;

                /* Module to be listed */
                size_t mod = op2R->int_value - 1;

                /* op1R will become an array of procedure names and pointers */
                value_zero(op1R);

                /* How many entries are needed? */
                i = context->modules[mod]->expose_head;
                entries = 0;
                while (i != -1) {
                    c_entry = (chameleon_constant *) (context->modules[mod]->segment.const_pool + i);
                    if (c_entry->type == EXPOSE_PROC_CONST) {
                        if (!((expose_proc_constant *) c_entry)->imported) entries++;
                    }
                    i = ((expose_proc_constant *) c_entry)->next;
                }

                /* Set up array */
                set_num_attributes(op1R, entries);
                op1R->int_value = (rxinteger) entries; /* The cREXX convention for arrays */

                /* Populate array */
                i = context->modules[mod]->expose_head;
                entries = 0;
                while (i != -1) {
                    c_entry = (chameleon_constant *) (context->modules[mod]->segment.const_pool + i);
                    if (c_entry->type == EXPOSE_PROC_CONST) {
                        /* Exposed Procedure */
                        e_entry = (expose_proc_constant *) c_entry;
                        p_entry = (proc_constant *) (
                                context->modules[mod]->segment.const_pool
                                + e_entry->procedure);
                        if (!e_entry->imported) {
                            /* Exported - Procedure - populate object and add to array  */
                            entry = op1R->attributes[entries];
                            set_num_attributes(entry, 2);
                            set_null_string(entry->attributes[0], e_entry->index);
                            entry->attributes[1]->int_value =
                                    (rxinteger) rxvm_get_module_runtime_procedure(context->modules[mod],
                                                                                  e_entry->procedure);

                            entries++;
                        }
                    }
                    i = e_entry->next;
                }
            }
            DISPATCH;

        /* Loaded Procedures (op1 = array procedures in module op2) */

)

RXVM_HANDLER(METALOADEDPROCS_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - METALOADEDPROCS R%d\n", (int) REG_IDX(1));
            {
                chameleon_constant *c_entry;
                proc_constant *p_entry;
                int i;
                size_t entries;
                value *entry;

                /* Module to be listed */
                size_t mod = op2R->int_value - 1;

                /* op1R will become an array of procedure names and pointers */
                value_zero(op1R);

                /* How many entries are needed? */
                i = context->modules[mod]->proc_head;
                entries = 0;
                while (i != -1) {
                    c_entry = (chameleon_constant *) (context->modules[mod]->segment.const_pool + i);
                    if (c_entry->type == PROC_CONST) {
                        entries++;
                    }
                    i = ((proc_constant *) c_entry)->next;
                }

                /* Set up array */
                set_num_attributes(op1R, entries);
                op1R->int_value = (rxinteger) entries; /* The cREXX convention for arrays */

                /* Populate array */
                i = context->modules[mod]->proc_head;
                entries = 0;
                while (i != -1) {
                    c_entry = (chameleon_constant *) (context->modules[mod]->segment.const_pool + i);
                    if (c_entry->type == PROC_CONST) {
                        /* Exposed Procedure */
                        p_entry = (proc_constant *) c_entry;
                        entry = op1R->attributes[entries];
                        set_num_attributes(entry, 2);
                        set_null_string(entry->attributes[0], p_entry->name);
                        entry->attributes[1]->int_value =
                                (rxinteger) rxvm_get_module_runtime_procedure(context->modules[mod], (size_t)i);
                        entries++;
                    }
                    i = p_entry->next;
                }
            }
            DISPATCH;

            /* Decode opcode (op1 decoded op2) */

)

RXVM_HANDLER(METADECODEINST_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - METADECODEINST R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2));

            /*
             * Preserve the historical seven-attribute object for callers that
             * read all three legacy type slots, then extend it for wide forms.
             */
            value_zero(op1R);
            set_num_attributes(op1R, (size_t)4 +
                    (meta_map[op2R->int_value].operands < 3
                         ? 3
                         : meta_map[op2R->int_value].operands));

            /* Populate the object */
            op1R->attributes[0]->int_value = meta_map[op2R->int_value].opcode;
            set_null_string(op1R->attributes[1], meta_map[op2R->int_value].instruction);
            set_null_string(op1R->attributes[2], meta_map[op2R->int_value].desc);
            op1R->attributes[3]->int_value = (rxinteger)meta_map[op2R->int_value].operands;
            {
                size_t operand_index;
                for (operand_index = 0;
                     operand_index < meta_map[op2R->int_value].operands;
                     operand_index++) {
                    op1R->attributes[4 + operand_index]->int_value =
                            rxop_format_operand_type(meta_map[op2R->int_value].format,
                                                     operand_index);
                }
            }
            DISPATCH;

            /* Load Integer/Index Operand (op1 = (int)op2[op3]) */

)

RXVM_HANDLER(METALOADIOPERAND_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - METALOADIOPERAND R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            op1R->int_value = context->modules[op2R->int_value - 1]->segment.binary[op3R->int_value].iconst;
            DISPATCH;

            /* Load Float Operand (op1 = (float)op2[op3]) */

)

RXVM_HANDLER(METALOADFOPERAND_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - METALOADFOPERAND R%d,R%dR%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            op1R->float_value =
                    FLOAT_CONST_VALUE(context->modules[op2R->int_value - 1]->segment.const_pool,
                                      context->modules[op2R->int_value - 1]->segment.binary[op3R->int_value].index);
            DISPATCH;

            /* Load String Operand (op1 = (string)op2[op3]) */

)

RXVM_HANDLER(METALOADSOPERAND_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - METALOADSOPERAND R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            set_const_string(op1R,
                             (string_constant *) (
                                     context->modules[op2R->int_value - 1]->segment.const_pool +
                                     context->modules[op2R->int_value - 1]->segment.binary[op3R->int_value].index
                             ));

            DISPATCH;

            /* Load Procedure Operand (op1 = (proc)op2[op3]) */
            /* TODO needs to do more that get the function name - a function object is needed */

)

RXVM_HANDLER(METALOADPOPERAND_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - METALOADPOPERAND R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            {
                proc_constant
                        *proc =
                        (proc_constant *) (
                                context->modules[op2R->int_value - 1]->segment.const_pool +
                                context->modules[op2R->int_value - 1]->segment.binary[op3R->int_value].index
                        );
                set_null_string(op1R, proc->name);
            }
            DISPATCH;

            /* Load Metadata (op1 = (metadata)op2[op3]) */

)

RXVM_HANDLER(METALOADDATA_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - METALOADDATA R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            {
                module *metadata_module = context->modules[op2R->int_value - 1];
                unsigned char *pool = metadata_module->segment.const_pool;
                int i = metadata_module->meta_head;
                int j;
                size_t x;
                meta_entry *meta = 0;
                int size = 0;
                string_constant *string_entry;

                /* Clear return object */
                value_zero(op1R);

                /* Find the start of the metadata at this address. Generic
                 * introspection deliberately preserves the canonical chain. */
                while (i != -1) {
                    meta = (meta_entry *) (pool + i);
                    if (meta->address < op3R->int_value) i = meta->next;
                    else break;
                }
                if (i == -1 || meta->address > op3R->int_value) {
                    /* No metadata for the address */
                    DISPATCH;
                }
                int start = i;

                /* Count the canonical-order records at this address. */
                size = 0;
                while (i != -1) {
                    meta = (meta_entry *) (pool + i);
                    if (meta->address == op3R->int_value) {
                        i = meta->next;
                        size++;
                    } else break;
                }
                set_num_attributes(op1R, size);
                op1R->int_value = size; /* TODO Using int to store the size of an array ... approach tbc! */

                /* Populate output with the metadata */
                for (j = 0, i = start; j < size; j++, i = ((meta_entry *) (pool + i))->next) {
                    value_zero(op1R->attributes[j]);
                    switch (((meta_entry *) (pool + i))->base.type) {
                        case META_SOURCE_STEP:
                            set_null_string(op1R->attributes[j], ".meta_source_step");
                            set_num_attributes(op1R->attributes[j], 8);
                            op1R->attributes[j]->attributes[0]->int_value =
                                    (rxinteger) ((meta_source_step_constant *) (pool + i))->step_id;
                            op1R->attributes[j]->attributes[1]->int_value =
                                    (rxinteger) ((meta_source_step_constant *) (pool + i))->clause_id;
                            op1R->attributes[j]->attributes[2]->int_value =
                                    (rxinteger) ((meta_source_step_constant *) (pool + i))->flags;
                            x = (rxinteger) ((meta_source_step_constant *) (pool + i))->file;
                            set_const_string(op1R->attributes[j]->attributes[3], (string_constant *) (pool + x));
                            op1R->attributes[j]->attributes[4]->int_value =
                                    (rxinteger) ((meta_source_step_constant *) (pool + i))->line;
                            op1R->attributes[j]->attributes[5]->int_value =
                                    (rxinteger) ((meta_source_step_constant *) (pool + i))->active_start_column;
                            op1R->attributes[j]->attributes[6]->int_value =
                                    (rxinteger) ((meta_source_step_constant *) (pool + i))->active_end_column;
                            x = (rxinteger) ((meta_source_step_constant *) (pool + i))->source_line;
                            set_const_string(op1R->attributes[j]->attributes[7], (string_constant *) (pool + x));
                            break;
                        case META_TRACE_EVENT:
                            set_null_string(op1R->attributes[j], ".meta_trace_event");
                            set_num_attributes(op1R->attributes[j], 11);
                            op1R->attributes[j]->attributes[0]->int_value =
                                    (rxinteger) ((meta_trace_event_constant *) (pool + i))->kind;
                            op1R->attributes[j]->attributes[1]->int_value =
                                    (rxinteger) ((meta_trace_event_constant *) (pool + i))->mode_mask;
                            op1R->attributes[j]->attributes[2]->int_value =
                                    (rxinteger) ((meta_trace_event_constant *) (pool + i))->value_source;
                            op1R->attributes[j]->attributes[3]->int_value =
                                    (rxinteger) ((meta_trace_event_constant *) (pool + i))->value_type;
                            op1R->attributes[j]->attributes[4]->int_value =
                                    (rxinteger) ((meta_trace_event_constant *) (pool + i))->register_type;
                            op1R->attributes[j]->attributes[5]->int_value =
                                    ((meta_trace_event_constant *) (pool + i))->value_ref == RXBIN_TRACE_REF_NONE ?
                                    (rxinteger)-1 : (rxinteger) ((meta_trace_event_constant *) (pool + i))->value_ref;
                            op1R->attributes[j]->attributes[6]->int_value =
                                    (rxinteger) ((meta_trace_event_constant *) (pool + i))->source_step_id;
                            op1R->attributes[j]->attributes[7]->int_value =
                                    (rxinteger) ((meta_trace_event_constant *) (pool + i))->clause_id;
                            op1R->attributes[j]->attributes[8]->int_value =
                                    (rxinteger) ((meta_trace_event_constant *) (pool + i))->flags;
                            x = (rxinteger) ((meta_trace_event_constant *) (pool + i))->symbol;
                            if ((size_t)x == RXBIN_TRACE_REF_NONE) set_null_string(op1R->attributes[j]->attributes[9], "");
                            else {
                                string_entry = get_runtime_string_constant(metadata_module, x);
                                if (string_entry) set_const_string(op1R->attributes[j]->attributes[9], string_entry);
                                else set_null_string(op1R->attributes[j]->attributes[9], "");
                            }
                            x = (rxinteger) ((meta_trace_event_constant *) (pool + i))->resolved_name;
                            if ((size_t)x == RXBIN_TRACE_REF_NONE) set_null_string(op1R->attributes[j]->attributes[10], "");
                            else {
                                string_entry = get_runtime_string_constant(metadata_module, x);
                                if (string_entry) set_const_string(op1R->attributes[j]->attributes[10], string_entry);
                                else set_null_string(op1R->attributes[j]->attributes[10], "");
                            }
                            break;
                        case META_FUNC:
                            set_null_string(op1R->attributes[j], ".meta_func");
                            set_num_attributes(op1R->attributes[j], 5);
                            x = (rxinteger) ((meta_func_constant *) (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_func_constant *) (pool + i))->option;
                            set_const_string(op1R->attributes[j]->attributes[1], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_func_constant *) (pool + i))->type;
                            set_const_string(op1R->attributes[j]->attributes[2], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_func_constant *) (pool + i))->args;
                            set_const_string(op1R->attributes[j]->attributes[3], (string_constant *) (pool + x));
                            op1R->attributes[j]->attributes[4]->int_value = (rxinteger) ((meta_func_constant *) (pool +
                                                                                                                 i))->func;
                            break;
                        case META_REG:
                            set_null_string(op1R->attributes[j], ".meta_reg");
                            set_num_attributes(op1R->attributes[j], 4);
                            x = (rxinteger) ((meta_reg_constant *) (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_reg_constant *) (pool + i))->option;
                            set_const_string(op1R->attributes[j]->attributes[1], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_reg_constant *) (pool + i))->type;
                            set_const_string(op1R->attributes[j]->attributes[2], (string_constant *) (pool + x));
                            op1R->attributes[j]->attributes[3]->int_value = (rxinteger) ((meta_reg_constant *) (pool +
                                                                                                                i))->reg;
                            break;
                        case META_CONST:
                            set_null_string(op1R->attributes[j], ".meta_const");
                            set_num_attributes(op1R->attributes[j], 4);
                            x = (rxinteger) ((meta_const_constant *) (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_const_constant *) (pool + i))->option;
                            set_const_string(op1R->attributes[j]->attributes[1], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_const_constant *) (pool + i))->type;
                            set_const_string(op1R->attributes[j]->attributes[2], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_const_constant *) (pool + i))->constant;
                            set_const_string(op1R->attributes[j]->attributes[3], (string_constant *) (pool + x));
                            break;
                        case META_CLEAR:
                            set_null_string(op1R->attributes[j], ".meta_clear");
                            set_num_attributes(op1R->attributes[j], 1);
                            x = (rxinteger) ((meta_clear_constant *) (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            break;
                        case META_CLASS:
                            /* TODO: Implement META_CLASS */
                            break;
                        case META_ATTR:
                            /* TODO: Implement META_ATTR */
                            break;
                        case META_INTERFACE:
                            set_null_string(op1R->attributes[j], ".meta_interface");
                            set_num_attributes(op1R->attributes[j], 3);
                            x = (rxinteger) ((meta_interface_constant *) (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_interface_constant *) (pool + i))->option;
                            set_const_string(op1R->attributes[j]->attributes[1], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_interface_constant *) (pool + i))->type;
                            set_const_string(op1R->attributes[j]->attributes[2], (string_constant *) (pool + x));
                            break;
                        case META_IMPLEMENTS:
                            set_null_string(op1R->attributes[j], ".meta_implements");
                            set_num_attributes(op1R->attributes[j], 2);
                            x = (rxinteger) ((meta_implements_constant *) (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_implements_constant *) (pool + i))->interface_symbol;
                            set_const_string(op1R->attributes[j]->attributes[1], (string_constant *) (pool + x));
                            break;
                        case META_MEMBER:
                            set_null_string(op1R->attributes[j], ".meta_member");
                            set_num_attributes(op1R->attributes[j], 5);
                            x = (rxinteger) ((meta_member_constant *) (pool + i))->owner;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_member_constant *) (pool + i))->kind;
                            set_const_string(op1R->attributes[j]->attributes[1], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_member_constant *) (pool + i))->member;
                            set_const_string(op1R->attributes[j]->attributes[2], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_member_constant *) (pool + i))->type;
                            set_const_string(op1R->attributes[j]->attributes[3], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_member_constant *) (pool + i))->args;
                            set_const_string(op1R->attributes[j]->attributes[4], (string_constant *) (pool + x));
                            break;
                        case META_INLINE:
                            set_null_string(op1R->attributes[j], ".meta_inline");
                            set_num_attributes(op1R->attributes[j], 2);
                            x = (rxinteger) ((meta_inline_constant *) (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_inline_constant *) (pool + i))->payload;
                            set_const_string(op1R->attributes[j]->attributes[1], (string_constant *) (pool + x));
                            break;
                        case META_TASK_TARGET:
                            /* Compiler/linker relocation metadata is consumed
                             * before execution and has no source reflection. */
                            break;
                        case META_PROVIDER:
                            set_null_string(op1R->attributes[j], ".meta_provider");
                            set_num_attributes(op1R->attributes[j], 3);
                            x = (rxinteger) ((meta_provider_constant *)
                                    (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0],
                                             (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_provider_constant *)
                                    (pool + i))->provider;
                            set_const_string(op1R->attributes[j]->attributes[1],
                                             (string_constant *) (pool + x));
                            op1R->attributes[j]->attributes[2]->int_value =
                                    (rxinteger) ((meta_provider_constant *)
                                            (pool + i))->flags;
                            break;
                        case STRING_CONST:
                        case FLOAT_CONST:
                        case PROC_CONST:
                        case BINARY_CONST:
                        case DECIMAL_CONST:
                        case EXPOSE_REG_CONST:
                        case EXPOSE_PROC_CONST:
                            break;

                    }
                }
            }
            DISPATCH;

            /* METALOADCALLERADDR - Load caller address object to op1 */

)

RXVM_HANDLER(METALOADCALLERADDR_REG, VM_ADVANCE(1);
            DEBUG("TRACE - METALOADCALLERADDR R%d\n", (int) REG_IDX(1));
            {
                rxinteger mod_no = -1;
                rxinteger addr = -1;

                if (current_frame->parent != 0) {
                    mod_no = (rxinteger) current_frame->parent->procedure->binarySpace->module->module_number;
                    module *caller_module =
                            current_frame->parent->procedure->binarySpace->module;
                    addr = (rxinteger) (current_frame->return_pc -
                                        VM_MODULE_EXECUTION_BASE(caller_module));
                }

                /* Populate the result object */
                value_zero(op1R);
                set_num_attributes(op1R, 2);
                op1R->attributes[0]->int_value = mod_no;
                op1R->attributes[1]->int_value = addr;
            }
            DISPATCH;

        /* Regular Instructions */

        /* NULL (Clear the register) */

)

RXVM_HANDLER(NULL_REG, VM_ADVANCE(1);
            DEBUG("TRACE - NULL R%lu\n", REG_IDX(1));
            value_zero(op1R);
            DISPATCH;

        /* LOAD */

)

RXVM_HANDLER(LOAD_REG_STRING, VM_ADVANCE(2);
            DEBUG("TRACE - LOAD R%lu,\"%.*s\"\n",
                  REG_IDX(1), (int) (CONSTSTRING_OP(2))->string_len,
                  (CONSTSTRING_OP(2))->string);
            set_const_string(op1R, CONSTSTRING_OP(2));
            DISPATCH;


)

RXVM_HANDLER(LOAD_REG_BINARY, VM_ADVANCE(2);
            DEBUG("TRACE - LOAD R%lu,binary[%zu]\n",
                  REG_IDX(1), (CONSTSTRING_OP(2))->string_len);
            if (set_binary(op1R, (CONSTSTRING_OP(2))->string, (CONSTSTRING_OP(2))->string_len) != 0) {
                SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
            }
            DISPATCH;


)

RXVM_HANDLER(LOAD_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - LOAD R%lu,R%lu\n",
                  REG_IDX(1), REG_IDX(2));
            op1R=op2R;
            DISPATCH;


)

RXVM_HANDLER(LOAD_INT_INT, VM_ADVANCE(2); // TODO - review instruction
            DEBUG("TRACE - LOAD R%lu,R%lu\n",
                  (long) op1I, (long) op2I);
            REG_OP(op1I)=REG_OP(op2I);
        DISPATCH;


)

RXVM_HANDLER(LOAD_INT_REG, VM_ADVANCE(2); // TODO - review instruction
    DEBUG("TRACE - LOAD R%lu, R%d\n",
          REG_IDX(1), (int) op2I);
          REG_OP(op1I)=op2R;
    DISPATCH;

            /* Readline - Read a line from stdin to a register */

)

RXVM_HANDLER(READLINE_REG, VM_ADVANCE(1);
            DEBUG("TRACE - READLINE R%lu\n", REG_IDX(1));
            {
                size_t pos = 0;
                int ch;
                op1R->string_length = 0;
                while ((ch = getchar()) != EOF) {
                    if (ch == '\n') break;
                    extend_string_buffer_metric(
                            op1R,
                            rxvm_value_string_size_add_or_fail(pos, 1u));
                    op1R->string_value[pos] = (char) ch;
                    pos++;
                }
                string_cache_reset(op1R);
                RXVM_UTF8_ONLY(
                refresh_utf8_flags(op1R);
                )
                RXVM_BYTE_ONLY(
                clear_vm_private_flags(op1R);
                )
            }
            DISPATCH;


)

RXVM_HANDLER(SAY_REG, VM_ADVANCE(1);
            DEBUG("TRACE - SAY R%lu\n", REG_IDX(1));
            rxvm_mprintf("%.*s\n", (int) op1R->string_length, op1R->string_value);
            DISPATCH;


)

RXVM_HANDLER(SAY_STRING, VM_ADVANCE(1);
            DEBUG("TRACE - SAY \"%.*s\"\n",
                  (int) op1S->string_len, op1S->string);
            rxvm_mprintf("%.*s\n", (int) op1S->string_len, op1S->string);
            DISPATCH;


)

RXVM_HANDLER(SAYX_REG, VM_ADVANCE(1);
            DEBUG("TRACE - SAYX R%lu\n", REG_IDX(1));
            rxvm_mprintf("%.*s", (int) op1R->string_length, op1R->string_value);
            DISPATCH;


)

RXVM_HANDLER(SAYX_STRING, VM_ADVANCE(1);
            DEBUG("TRACE - SAYX \"%.*s\"\n",
                  (int) op1S->string_len, op1S->string);
            rxvm_mprintf("%.*s", (int) op1S->string_len, op1S->string);
            DISPATCH;


)

RXVM_HANDLER(SCONCAT_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SCONCAT R%lu,R%lu,R%lu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            string_sconcat(op1R, op2R, op3R);
            DISPATCH;


)

RXVM_HANDLER(CONCAT_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - CONCAT R%lu,R%lu,R%lu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            string_concat(op1R, op2R, op3R);
            DISPATCH;


)

RXVM_HANDLER(SCONCAT_REG_REG_STRING, VM_ADVANCE(3);
            DEBUG("TRACE - SCONCAT R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                  REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                  (CONSTSTRING_OP(3))->string);
            string_sconcat_var_const(op1R, op2R, op3S);
            DISPATCH;


)

RXVM_HANDLER(CONCAT_REG_REG_STRING, VM_ADVANCE(3);
            DEBUG("TRACE - CONCAT R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                  REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                  (CONSTSTRING_OP(3))->string);
            string_concat_var_const(op1R, op2R, op3S);
            DISPATCH;


)

RXVM_HANDLER(SCONCAT_REG_STRING_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SCONCAT R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                  (int) (CONSTSTRING_OP(2))->string_len,
                  (CONSTSTRING_OP(2))->string, REG_IDX(3));
            string_sconcat_const_var(op1R, op2S, op3R);
            DISPATCH;


)

RXVM_HANDLER(CONCAT_REG_STRING_REG, VM_ADVANCE(3);
            DEBUG("TRACE - CONCAT R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                  (int) (CONSTSTRING_OP(2))->string_len,
                  (CONSTSTRING_OP(2))->string, REG_IDX(3));
            string_concat_const_var(op1R, op2S, op3R);
            DISPATCH;


)

RXVM_HANDLER(IMULT_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - IMULT R%lu,R%lu,R%lu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            {
                rxinteger result;
                if (rxinteger_checked_mul(op2RI, op3RI, &result)) REG_RETURN_INT(result)
                else SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
            }
            DISPATCH;


)

RXVM_HANDLER(IMULT_REG_REG_INT, {
            VM_ADVANCE(3);
            DEBUG("TRACE - IMULT R%lu,R%lu,%lu\n", (long)REG_IDX(1),
                  (long)REG_IDX(2), (long)op3I);
            {
                rxinteger result;
                if (rxinteger_checked_mul(op2RI, op3I, &result)) REG_RETURN_INT(result)
                else SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
            }
            DISPATCH;
        }


)

RXVM_HANDLER(IADD_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - IADD R%lu,R%lu,R%lu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            {
                rxinteger result;
                if (rxinteger_checked_add(op2RI, op3RI, &result)) REG_RETURN_INT(result)
                else SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
            }
            DISPATCH;


)

RXVM_HANDLER(ISUB_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - ISUB R%lu,R%lu,R%lu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            {
                rxinteger result;
                if (rxinteger_checked_sub(op2RI, op3RI, &result)) REG_RETURN_INT(result)
                else SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
            }
            DISPATCH;


)

RXVM_HANDLER(IADD_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - IADD R%lu,R%lu,%d\n", REG_IDX(1),
                  REG_IDX(2), (int)op3I);
            {
                rxinteger result;
                if (rxinteger_checked_add(op2RI, op3I, &result)) REG_RETURN_INT(result)
                else SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
            }
            DISPATCH;


)

RXVM_HANDLER(ERASE_REG, VM_ADVANCE(1);
            DEBUG("TRACE - ERASE R%lu\n", REG_IDX(1));
            value_zero(op1R);
            DISPATCH;

/* ====================================================================================
 * Numeric Mode instructions
 * ==================================================================================== */

/* ------------------------------------------------------------------------------------
 *  SETNUMDGTS_REG Set Numeric Digits digits=op1 (>4)
 *  ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(SETNUMDGTS_REG, VM_ADVANCE(1);
    DEBUG("TRACE - SETNUMDGTS R%lu\n", REG_IDX(1));
    if (op1R->int_value < 1) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Digits must be greater than 0");
        DISPATCH;
    }
    current_frame->num_context.digits = op1R->int_value;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SETNUMDGTS_INT Set Numeric Digits digits=op1 (>4)
 * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(SETNUMDGTS_INT, VM_ADVANCE(1);
    DEBUG("TRACE - SETNUMDGTS %d\n", (int)op1I);
    if (op1I < 1) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Digits must be greater than 0");
        DISPATCH;
    }
    current_frame->num_context.digits = op1I;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  GETNUMDGTS_REG Get Numeric Digits op1=digits
 * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(GETNUMDGTS_REG, VM_ADVANCE(1);
    DEBUG("TRACE - GETNUMDGTS R%lu\n", REG_IDX(1));
    op1R->int_value = (rxinteger)current_frame->num_context.digits;
    DISPATCH;

/* ------------------------------------------------------------------------------------
 * SETNUMFUZ_REG Set Numeric Fuzz digits=op1 (>=0)
 * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(SETNUMFUZ_REG, VM_ADVANCE(1);
    DEBUG("TRACE - SETNUMFUZ R%lu\n", REG_IDX(1));
    if (op1R->int_value < 0) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Fuzz must be zero or greater");
        DISPATCH;
    }
    current_frame->num_context.fuzz = op1R->int_value;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 * SETNUMFUZ_INT Set Numeric Fuzz digits=op1 (>=0)
 * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(SETNUMFUZ_INT, VM_ADVANCE(1);
    DEBUG("TRACE - SETNUMFUZ %d\n", (int)op1I);
    if (op1I < 0) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Fuzz must be zero or greater");
        DISPATCH;
    }
    current_frame->num_context.fuzz = op1I;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 * GETNUMFUZ_REG Get Numeric Fuzz op1=digits
 * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(GETNUMFUZ_REG, VM_ADVANCE(1);
    DEBUG("TRACE - GETNUMFUZ R%lu\n", REG_IDX(1));
    op1R->int_value = (rxinteger)current_frame->num_context.fuzz;
    DISPATCH;

/* ------------------------------------------------------------------------------------
 * SETNUMFRM_REG Set Numeric Form=op1 (1=sci,2=eng)
 * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(SETNUMFRM_REG, VM_ADVANCE(1);
    DEBUG("TRACE - SETNUMFRM R%lu\n", REG_IDX(1));
    if (op1R->int_value < 1 || op1R->int_value > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Form must be 1 (scientific) or 2 (engineering)");
        DISPATCH;
    }
    current_frame->num_context.form = (int)op1R->int_value;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 * SETNUMFRM_INT Set Numeric Form=op1 (1=sci,2=eng)
 * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(SETNUMFRM_INT, VM_ADVANCE(1);
    DEBUG("TRACE - SETNUMFRM %d\n", (int)op1I);
    if (op1I < 1 || op1I > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Form must be 1 (scientific) or 2 (engineering)");
        DISPATCH;
    }
    current_frame->num_context.form = (int)op1I;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 * GETNUMFRM_REG Get Numeric Form=op1 (1=sci,2=eng)
 * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(GETNUMFRM_REG, VM_ADVANCE(1);
    DEBUG("TRACE - GETNUMFRM R%lu\n", REG_IDX(1));
    op1R->int_value = (rxinteger)current_frame->num_context.form;
    DISPATCH;

/* ------------------------------------------------------------------------------------
 * SETNUMCAS_REG Set Numeric Case=op1 (1=lower,2=upper)
 * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(SETNUMCAS_REG, VM_ADVANCE(1);
    DEBUG("TRACE - SETNUMCAS R%lu\n", REG_IDX(1));
    if (op1R->int_value < 1 || op1R->int_value > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Case must be 1 (lower) or 2 (upper)");
        DISPATCH;
    }
    current_frame->num_context.casetype = (int)op1R->int_value;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 * SETNUMCAS_INT Set Numeric Case=op1 (1=lower,2=upper)
 * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(SETNUMCAS_INT, VM_ADVANCE(1);
    DEBUG("TRACE - SETNUMCAS %d\n", (int)op1I);
    if (op1I < 1 || op1I > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Case must be 1 (lower) or 2 (upper)");
        DISPATCH;
    }
    current_frame->num_context.casetype = (int)op1I;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 * GETNUMCAS_REG Get Numeric Case=op1 (1=lower,2=upper)
 * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(GETNUMCAS_REG, VM_ADVANCE(1);
    DEBUG("TRACE - GETNUMCAS R%lu\n", REG_IDX(1));
    op1R->int_value = (rxinteger)current_frame->num_context.casetype;
    DISPATCH;

/* ------------------------------------------------------------------------------------
 * SETNUMSTD_REG Set Numeric Standard=op1 (1=common,2=classic)
 * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(SETNUMSTD_REG, VM_ADVANCE(1);
    DEBUG("TRACE - SETNUMSTD R%lu\n", REG_IDX(1));
    if (op1R->int_value < 1 || op1R->int_value > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Standard must be 1 (common) or 2 (classic)");
        DISPATCH;
    }
    current_frame->num_context.standard = (int)op1R->int_value;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 * SETNUMSTD_INT Set Numeric Standard=op1 (1=common,2=classic)
 * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(SETNUMSTD_INT, VM_ADVANCE(1);
    DEBUG("TRACE - SETNUMSTD %d\n", (int)op1I);
    if (op1I < 1 || op1I > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Standard must be 1 (common) or 2 (classic)");
        DISPATCH;
    }
    current_frame->num_context.standard = (int)op1I;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 * GETNUMSTD_REG Get Numeric Standard=op1 (1=common,2=classic)
 * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(GETNUMSTD_REG, VM_ADVANCE(1);
    DEBUG("TRACE - GETNUMSTD R%lu\n", REG_IDX(1));
    op1R->int_value = (rxinteger)current_frame->num_context.standard;
    DISPATCH;

/* ------------------------------------------------------------------------------------
 * NUMSCI_INT_INT_INT Setup Scientific Numeric digits=op1, case=op2, std=op3, fuzz=0, form=sci
 * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(NUMSCI_INT_INT_INT, VM_ADVANCE(3);
    DEBUG("TRACE - NUMSCI %d,%d,%d\n", (int)op1I, (int)op2I, (int)op3I);
    if (op1I < 5) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Digits must be greater than 4");
        DISPATCH;
    }
    if (op2I < 1 || op2I > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Case must be 1 (lower) or 2 (upper)");
        DISPATCH;
    }
    if (op3I < 1 || op3I > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Standard must be 1 (common) or 2 (classic)");
        DISPATCH;
    }
    current_frame->num_context.digits = op1I;
    current_frame->num_context.fuzz = 0;
    current_frame->num_context.form = 1; // scientific
    current_frame->num_context.casetype = (int)op2I;
    current_frame->num_context.standard = (int)op3I;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 * NUMENG_INT_INT_INT Setup Engineering Numeric digits=op1, case=op2, std=op3, fuzz=0, form=eng
 * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(NUMENG_INT_INT_INT, VM_ADVANCE(3);
    DEBUG("TRACE - NUMENG %d,%d,%d\n", (int)op1I, (int)op2I, (int)op3I);
    if (op1I < 5) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Digits must be greater than 4");
        DISPATCH;
    }
    if (op2I < 1 || op2I > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Case must be 1 (lower) or 2 (upper)");
        DISPATCH;
    }
    if (op3I < 1 || op3I > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Standard must be 1 (common) or 2 (classic)");
        DISPATCH;
    }
    current_frame->num_context.digits = op1I;
    current_frame->num_context.fuzz = 0;
    current_frame->num_context.form = 2; // engineering
    current_frame->num_context.casetype = (int)op2I;
    current_frame->num_context.standard = (int)op3I;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH;

/* ====================================================================================
 * Decimal Plugin instructions
 * ==================================================================================== */

/* ------------------------------------------------------------------------------------
 * DECPLNM_REG_REG_REG Get Decimal Plugin Name op1=name op2=description op3=version
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DECPLNM_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - DECPLNM R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_null_string(op1R, current_frame->decimal->base.name);
    set_null_string(op2R, current_frame->decimal->base.description);
    set_null_string(op3R, current_frame->decimal->base.version);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  LOAD_REG_DECIMAL Load op1 with op2
 *  -----------------------------------------------------------------------------------
*/

)

RXVM_HANDLER(LOAD_REG_DECIMAL, VM_ADVANCE(2);
    DEBUG("TRACE - LOAD R%d,%s\n",(int)REG_IDX(1),op2S->string);
    current_frame->decimal->decimalFromString(current_frame->decimal, op1R, op2S->string);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 * Convert decimal string to Decimal                              added August 2024 pej
 * ------------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(STOD_REG,
    VM_ADVANCE(1);
    DEBUG("TRACE - STOD R%lu\n", REG_IDX(1));
    // Ensure the string is null terminated
    null_terminate_string_buffer(op1R);
    // Convert
    current_frame->decimal->decimalFromString(current_frame->decimal, op1R, op1R->string_value);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 * Convert Integer to Decimal                                     added August 2024 pej
 * ------------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(ITOD_REG,
    VM_ADVANCE(1);
    DEBUG("TRACE - ITOD R%lu\n", REG_IDX(1));
    /* Integer-to-decimal is total for every rxinteger. The plugin contract
     * clears stale diagnostics; allocation failure follows the VM OOM panic
     * convention rather than becoming a language signal. */
    current_frame->decimal->decimalFromInt(current_frame->decimal, op1R, op1R->int_value);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 * Convert Boolean to Decimal                                     added August 2024 pej
 * ------------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(BTOD_REG,
    VM_ADVANCE(1);
    DEBUG("TRACE - BTOD R%lu\n", REG_IDX(1));
    /* Uses the same total integer-to-decimal plugin contract as ITOD. */
    current_frame->decimal->decimalFromInt(current_frame->decimal, op1R, op1R->int_value ? 1 : 0);
    DISPATCH;
    /* ------------------------------------------------------------------------------------
 * Convert Float to Decimal                                       added August 2024 pej
 * ------------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(FTOD_REG,
    VM_ADVANCE(1);
    DEBUG("TRACE - FTOD R%lu\n",REG_IDX(1));
    current_frame->decimal->decimalFromDouble(current_frame->decimal, op1R, op1R->float_value);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 * Convert Decimal to string                                        17. August 2024 pej
 * ------------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DTOS_REG,
    VM_ADVANCE(1);
    DEBUG("TRACE - DTOS R%lu\n", REG_IDX(1));
    /* Determine how long the string needs to be */
    size_t string_size = current_frame->decimal->getRequiredStringSize(current_frame->decimal);
    /* Ensure the string buffer is big enough */
    extend_string_buffer(op1R, string_size);
    /* Convert */
    current_frame->decimal->decimalToString(current_frame->decimal, op1R, op1R->string_value);
    finish_ascii_string_write(op1R, strlen(op1R->string_value));
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 * Convert Decimal to integer                                       17. August 2024 pej
 * ------------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DTOI_REG,
    VM_ADVANCE(1);
    DEBUG("TRACE - DTOI R%lu\n", REG_IDX(1));
    current_frame->decimal->decimalToInt(current_frame->decimal, op1R, &op1R->int_value);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 * Convert Decimal to Boolean
 * ------------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DTOB_REG,
    VM_ADVANCE(1);
    DEBUG("TRACE - DTOB R%lu\n", REG_IDX(1));
    op1R->int_value = current_frame->decimal->decimalIsZero(current_frame->decimal, op1R) ? 0 : 1; // Convert to boolean
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DTOF_REG  Convert Decimal Number to Float op1=f2dec(op2)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
*/

)

RXVM_HANDLER(DTOF_REG, // label not yet defined
    VM_ADVANCE(1);
    DEBUG("TRACE - DTOF_REG\n");
    current_frame->decimal->decimalToDouble(current_frame->decimal, op1R, &op1R->float_value);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 * Decimal addition                                               added August 2024 pej
 * ------------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DADD_REG_REG_REG,
    VM_ADVANCE(3);
    DEBUG("TRACE - DADD R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    current_frame->decimal->decimalAdd(current_frame->decimal, op1R, op2R, op3R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DADD_REG_REG_DECIMAL  Decimal Add (op1=op2+op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DADD_REG_REG_DECIMAL,
    VM_ADVANCE(3);
    DEBUG("TRACE - DADD R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    {
        value *op = decimal_literal_value(current_frame->decimal, op3S->string);
        current_frame->decimal->decimalAdd(current_frame->decimal, op1R, op2R, op);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 * Decimal subtraction                                            added August 2024 pej
 * ------------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DSUB_REG_REG_REG,
    VM_ADVANCE(3);
    DEBUG("TRACE - DSUB R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    current_frame->decimal->decimalSub(current_frame->decimal, op1R, op2R, op3R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DSUB_REG_REG_DECIMAL  Decimal Subtract (op1=op2-op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DSUB_REG_REG_DECIMAL,
    VM_ADVANCE(3);
    DEBUG("TRACE - DSUB R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    {
        value *op = decimal_literal_value(current_frame->decimal, op3S->string);
        current_frame->decimal->decimalSub(current_frame->decimal, op1R, op2R, op);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DSUB_REG_REG_DECIMAL  Decimal Subtract (op1=op2-op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DSUB_REG_DECIMAL_REG,
    VM_ADVANCE(3);
    DEBUG("TRACE - DSUB R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    {
        value *op = decimal_literal_value(current_frame->decimal, op2S->string);
        current_frame->decimal->decimalSub(current_frame->decimal, op1R, op, op3R);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;

/* ------------------------------------------------------------------------------------
 * Decimal Multiply                                               added August 2024 pej
 * ------------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DMULT_REG_REG_REG,
    VM_ADVANCE(3);
    DEBUG("TRACE - DMULT R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    current_frame->decimal->decimalMul(current_frame->decimal, op1R, op2R, op3R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DMULT_REG_REG_DECIMAL Decimal Multiply (op1=op2*op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DMULT_REG_REG_DECIMAL,
    VM_ADVANCE(3);
    DEBUG("TRACE - DMULT R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    {
        value *op = decimal_literal_value(current_frame->decimal, op3S->string);
        current_frame->decimal->decimalMul(current_frame->decimal, op1R, op2R, op);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 * Decimal division                                               added August 2024 pej
 * ------------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DDIV_REG_REG_REG,
    VM_ADVANCE(3);
    DEBUG("TRACE - DDIV R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    current_frame->decimal->decimalDiv(current_frame->decimal, op1R, op2R, op3R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DDIV_REG_DECIMAL_REG  Decimal Divide  (op1=op2/op3)                   pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DDIV_REG_DECIMAL_REG,
    VM_ADVANCE(3);
    DEBUG("TRACE - DDIV R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    {
        value *op = decimal_literal_value(current_frame->decimal, op2S->string);
        current_frame->decimal->decimalDiv(current_frame->decimal, op1R, op, op3R);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DDIV_REG_REG_DECIMAL  Decimal Divide (op1=op2/op3)                   pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DDIV_REG_REG_DECIMAL,
    VM_ADVANCE(3);
    DEBUG("TRACE - DDIV R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    {
        value *op = decimal_literal_value(current_frame->decimal, op3S->string);
        current_frame->decimal->decimalDiv(current_frame->decimal, op1R, op2R, op);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 * DIDIV_REG_REG_REG Decimal integer division
 * ------------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DIDIV_REG_REG_REG,
    VM_ADVANCE(3);
    DEBUG("TRACE - DIDIV R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    current_frame->decimal->decimalDiv(current_frame->decimal, op1R, op2R, op3R);
    // Note: This is integer division, so the result is truncated
    current_frame->decimal->decimalTruncate(current_frame->decimal, op1R, op1R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DIDIV_REG_DECIMAL_REG  Decimal Integer Divide  (op1=op2/op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DIDIV_REG_DECIMAL_REG,
    VM_ADVANCE(3);
    DEBUG("TRACE - DIDIV R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    {
        value *op = decimal_literal_value(current_frame->decimal, op2S->string);
        current_frame->decimal->decimalDiv(current_frame->decimal, op1R, op, op3R);
        // Note: This is integer division, so the result is truncated
        current_frame->decimal->decimalTruncate(current_frame->decimal, op1R, op1R);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DIDIV_REG_REG_DECIMAL  Decimal Integer Divide (op1=op2/op3)
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DIDIV_REG_REG_DECIMAL,
    VM_ADVANCE(3);
    DEBUG("TRACE - DIDIV R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    {
        value *op = decimal_literal_value(current_frame->decimal, op3S->string);
        current_frame->decimal->decimalDiv(current_frame->decimal, op1R, op2R, op);
        // Note: This is integer division, so the result is truncated
        current_frame->decimal->decimalTruncate(current_frame->decimal, op1R, op1R);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DEQ_REG_REG_REG  Decimal Equals op1=(op2==op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DEQ_REG_REG_REG,
    VM_ADVANCE(3);
    DEBUG("TRACE - DEQ R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) == 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DNE_REG_REG_REG  Decimal Not equals op1=(op2!=op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DNE_REG_REG_REG,
    VM_ADVANCE(3);
    DEBUG("TRACE - DNE R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) != 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DGT_REG_REG_REG  Decimal Greater than op1=(op2>op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DGT_REG_REG_REG, // label not yet defined
    VM_ADVANCE(3);
    DEBUG("TRACE - DGT R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) > 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DGTE_REG_REG_REG  Decimal Greater than equals op1=(op2>=op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DGTE_REG_REG_REG, // label not yet defined
    VM_ADVANCE(3);
    DEBUG("TRACE - DGTE R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) >= 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DLT_REG_REG_REG  Decimal Less than op1=(op2<op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DLT_REG_REG_REG, // label not yet defined
    VM_ADVANCE(3);
    DEBUG("TRACE - DLT R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) < 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DLTE_REG_REG_REG  Decimal Less than equals op1=(op2<=op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DLTE_REG_REG_REG, // label not yet defined
    VM_ADVANCE(3);
    DEBUG("TRACE - DLTE R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) <= 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DEQ_REG_REG_DECIMAL  Decimal Equals op1=(op2==op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DEQ_REG_REG_DECIMAL, // label not yet defined
    VM_ADVANCE(3);
    DEBUG("TRACE - DEQ R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op2R, op3S->string) == 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DLTBR_ID_REG_REG  Decimal Less than if (op2<op3) goto op1              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DLTBR_ID_REG_REG,
    VM_ADVANCE(3); // This branch prediction for the condition not being met
    DEBUG("TRACE - DLTBR 0x%x,0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    if (current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) < 0) {
        VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DGTBR_ID_REG_REG  Decimal Greater than if (op2>op3) goto op1              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DGTBR_ID_REG_REG,
    VM_ADVANCE(3);
    DEBUG("TRACE - DLTBR 0x%x,0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    if (current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) > 0) {
        VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DEQBR_ID_REG_REG  Decimal Equal if (op2=op3) goto op1              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DEQBR_ID_REG_REG,
    VM_ADVANCE(3);
    DEBUG("TRACE - DEQBR 0x%x,0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    if (current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) == 0) {
        VM_SELECT_INDEX(REG_IDX(1), RXVM_TRANSITION_BRANCH);

    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DNE_REG_REG_DECIMAL  Decimal Not equals op1=(op2!=op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DNE_REG_REG_DECIMAL,
    VM_ADVANCE(3);
    DEBUG("TRACE - DNE R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op2R, op3S->string) != 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DGT_REG_REG_DECIMAL  Decimal Greater than op1=(op2>op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DGT_REG_REG_DECIMAL,
    VM_ADVANCE(3);
    DEBUG("TRACE - DGT R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op2R, op3S->string) > 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DGT_REG_DECIMAL_REG  Decimal Greater than op1=(op2>op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DGT_REG_DECIMAL_REG,
    VM_ADVANCE(3);
    DEBUG("TRACE - DGT R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op3R, op2S->string) < 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DGTE_REG_REG_DECIMAL  Decimal Greater than equals op1=(op2>=op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DGTE_REG_REG_DECIMAL,
    VM_ADVANCE(3);
    DEBUG("TRACE - DGTE R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op2R, op3S->string) >= 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DGTE_REG_DECIMAL_REG  Decimal Greater than equals op1=(op2>=op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DGTE_REG_DECIMAL_REG, // label not yet defined
    VM_ADVANCE(3);
    DEBUG("TRACE - DGTE R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op3R, op2S->string) <= 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DLT_REG_REG_DECIMAL  Decimal Less than op1=(op2<op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DLT_REG_REG_DECIMAL,
    VM_ADVANCE(3);
    DEBUG("TRACE - DLT R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op2R, op3S->string) < 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DLT_REG_DECIMAL_REG  Decimal Less than op1=(op2<op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DLT_REG_DECIMAL_REG,
    VM_ADVANCE(3);
    DEBUG("TRACE - DLT R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op3R, op2S->string) > 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DLTE_REG_REG_DECIMAL  Decimal Less than equals op1=(op2<=op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DLTE_REG_REG_DECIMAL,
    VM_ADVANCE(3);
    DEBUG("TRACE - DLTE R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op2R, op3S->string) <= 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DLTE_REG_DECIMAL_REG  Decimal Less than equals op1=(op2<=op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DLTE_REG_DECIMAL_REG,
    VM_ADVANCE(3);
    DEBUG("TRACE - DLTE R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op3R, op2S->string) >= 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DCOPY_REG_REG  Copy Decimal op2 to op1              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DCOPY_REG_REG, // label not yet defined
    VM_ADVANCE(2);
    DEBUG("TRACE - DCOPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
    if (op1R == op2R) {
        // NOP
        DISPATCH;
    }
    RXVM_INSTRUMENTATION_VALUE_TYPED(
            RXVM_PROFILE_VALUE_DECIMAL_COPY,
            RXVM_PROFILE_VALUE_DECIMAL,
            rxvm_value_decimal_length(op2R));
    /* A zero logical length is decimal absence even when lazy backing storage
     * remains allocated.  Typed copy propagates that absence without changing
     * any unrelated component of the destination. */
    if (rxvm_value_decimal_data(op2R) == NULL ||
        rxvm_value_decimal_length(op2R) == 0) {
        rxvm_value_set_decimal_length(op1R, 0u);
        DISPATCH;
    }
    {
        size_t decimal_length = rxvm_value_decimal_length(op2R);
        void *decimal = rxvm_value_reserve_decimal(op1R, decimal_length);
        if (!decimal) RX_PANIC_OOM(
                "copy decimal sidecar payload", decimal_length,
                "DCOPY destination");
        memcpy(decimal, rxvm_value_decimal_data(op2R), decimal_length);
    }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DSEX_REG  Decimal op1 = -op1 (sign change)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DSEX_REG,
    VM_ADVANCE(1);
    DEBUG("TRACE - DSEX R%lu\n", REG_IDX(1));
    if (rxvm_value_decimal_data(op1R) == NULL) {
        // Signal error
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "No Source Decimal Value")
        DISPATCH;
    }
    current_frame->decimal->decimalNeg(current_frame->decimal, op1R, op1R);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DPOW_REG_REG_REG  op1=op2**op3              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DPOW_REG_REG_REG,
    VM_ADVANCE(3);
    DEBUG("TRACE - DPOW R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    current_frame->decimal->decimalPow(current_frame->decimal, op1R, op2R, op3R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DPOW_REG_REG_DECIMAL  op1=op2**op3              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DPOW_REG_REG_DECIMAL,
    VM_ADVANCE(3);
    DEBUG("TRACE - DPOW R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    {
        value *op = decimal_literal_value(current_frame->decimal, op3S->string);
        current_frame->decimal->decimalPow(current_frame->decimal, op1R, op2R, op);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DPOW_REG_DECIMAL_REG  op1=op2**op3              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(DPOW_REG_DECIMAL_REG,
    VM_ADVANCE(3);
    DEBUG("TRACE - DPOW R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    {
        value *op = decimal_literal_value(current_frame->decimal, op2S->string);
        current_frame->decimal->decimalPow(current_frame->decimal, op1R, op, op3R);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH;
/* ------------------------------------------------------------------------------------
 * DEXTR_REG_REG_REG Extract decimal to string coefficient and decimal exponent integer
 * R3 contains the decimal value
 * R1 will store the coefficient as a string (or nan, inf, -inf)
 * R2 will store the decimal exponent as an integer
 * Output normalised, rounded to the digits setting decimal places, and
 * trimmed of trailing zeros
 * This instruction is designed to allow the user to format the float as they wish
 */

)

RXVM_HANDLER(DEXTR_REG_REG_REG,
    VM_ADVANCE(3);
    DEBUG("TRACE - DEXTR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    prep_string_buffer(op1R, current_frame->decimal->getRequiredStringSize(current_frame->decimal));
    current_frame->decimal->decimalExtract(current_frame->decimal, op1R->string_value, &(op2R->int_value), op3R);
    finish_ascii_string_write(op1R, strlen(op1R->string_value));
    DISPATCH;
/* ====================================================================================
 * End of Decimal Plugin instructions
 * ====================================================================================
 */

)

RXVM_HANDLER(CALL_REG_FUNC, VM_ADVANCE(2);
            {
                /* Clear target return value register */
                value_zero(op1R);

                proc_runtime *called_function = PROC_OP(2);
                DEBUG("TRACE - CALL R%lu,%s()\n", REG_IDX(1), called_function->name);

                if (called_function->start == SIZE_MAX) {
                    RXVM_INSTRUMENTATION_CALL(
                            called_function->binarySpace
                                ? RXVM_PROFILE_CALL_DIRECT_BYTECODE
                                : RXVM_PROFILE_CALL_DIRECT_NATIVE,
                            called_function, 0, RXVM_PROFILE_FRAME_NONE_FAILED,
                            RXVM_PROFILE_CALL_UNRESOLVED, current_frame,
                            current_module->module_number, VM_CANONICAL_INDEX(pc),
                            0, 0);
                    SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, called_function->name);
                    DISPATCH;
                }

                if (called_function->binarySpace == 0) {
                    /* This is a native plugin function */
                    RXVM_INSTRUMENTATION_CALL(
                            RXVM_PROFILE_CALL_DIRECT_NATIVE, called_function, 0,
                            RXVM_PROFILE_FRAME_NO_CHILD_NATIVE,
                            RXVM_PROFILE_CALL_SUCCESS, current_frame,
                            current_module->module_number, VM_CANONICAL_INDEX(pc),
                            0, 0);
                    RXVM_INSTRUMENTATION_NATIVE_BEGIN(called_function);
                    rxvm_call_native_procedure(called_function, 0, NULL,
                                               op1R, signal_value);
                    RXVM_INSTRUMENTATION_NATIVE_END();
                    INTERRUPT_FROM_RXPA_SIGNAL(signal_value);
                    RXVM_EXTERNAL_SAFEPOINT();
                } else {
                    /* This is a CREXX Procedure */
                    /* New stackframe - grabbing a procedure object from the caller frame */
                    temp_frame = frame_f(called_function, 0, current_frame, next_pc, op1R);
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
                    /* Prepare dispatch to procedure as early as possible */
                    VM_ACTIVATE_FRAME(temp_frame, RXVM_TRANSITION_CALL);
                    VM_SELECT_INDEX(called_function->start, RXVM_TRANSITION_CALL);

                    /* No Arguments - so nothing to do */
                }
            }
            DISPATCH;


)
#define RXVM_MAPPED_CALL_BODY(RESULT_, FUNCTION_, ARG_COUNT_, ARG_BASE_)         \
        do {                                                                    \
            proc_runtime *mapped_called__ = (FUNCTION_);                        \
            value *mapped_result__ = (RESULT_);                                 \
            int mapped_arg_count__ = (int)(ARG_COUNT_);                         \
            size_t mapped_arg_base__ = (size_t)(ARG_BASE_);                     \
            value **mapped_args__ = current_locals + mapped_arg_base__;         \
            if (mapped_called__->start == SIZE_MAX) {                           \
                RXVM_INSTRUMENTATION_CALL(                                      \
                        mapped_called__->binarySpace                            \
                            ? RXVM_PROFILE_CALL_DIRECT_BYTECODE                 \
                            : RXVM_PROFILE_CALL_DIRECT_NATIVE,                  \
                        mapped_called__, mapped_arg_count__,                    \
                        RXVM_PROFILE_FRAME_NONE_FAILED,                         \
                        RXVM_PROFILE_CALL_UNRESOLVED, current_frame,            \
                        current_module->module_number, VM_CANONICAL_INDEX(pc),  \
                        mapped_arg_base__, 1);                                  \
                SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, mapped_called__->name); \
                DISPATCH;                                                       \
            }                                                                   \
            if (mapped_called__->binarySpace == 0) {                            \
                RXVM_INSTRUMENTATION_CALL(                                      \
                        RXVM_PROFILE_CALL_DIRECT_NATIVE, mapped_called__,       \
                        mapped_arg_count__, RXVM_PROFILE_FRAME_NO_CHILD_NATIVE, \
                        RXVM_PROFILE_CALL_SUCCESS, current_frame,               \
                        current_module->module_number, VM_CANONICAL_INDEX(pc),  \
                        mapped_arg_base__, 1);                                  \
                RXVM_INSTRUMENTATION_NATIVE_BEGIN(mapped_called__);             \
                rxvm_call_native_procedure(mapped_called__,                    \
                              mapped_arg_count__, mapped_args__,                \
                              mapped_result__, signal_value);                   \
                RXVM_INSTRUMENTATION_NATIVE_END();                              \
                INTERRUPT_FROM_RXPA_SIGNAL(signal_value);                       \
                RXVM_EXTERNAL_SAFEPOINT();                                      \
            } else {                                                            \
                size_t mapped_j__;                                              \
                size_t mapped_i__;                                              \
                temp_frame = frame_f(mapped_called__, mapped_arg_count__,       \
                                     current_frame, next_pc, mapped_result__);   \
                if (!temp_frame) {                                              \
                    RXVM_INSTRUMENTATION_CALL(                                  \
                            RXVM_PROFILE_CALL_DIRECT_BYTECODE, mapped_called__, \
                            mapped_arg_count__, RXVM_PROFILE_FRAME_NONE_FAILED, \
                            RXVM_PROFILE_CALL_FRAME_FAILED, current_frame,      \
                            current_module->module_number,                      \
                            VM_CANONICAL_INDEX(pc), mapped_arg_base__, 1);       \
                    SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Unable to allocate stack frame"); \
                    DISPATCH;                                                   \
                }                                                               \
                assert(mapped_arg_base__ <= UINT32_MAX);                        \
                temp_frame->caller_arg_base = (uint32_t)mapped_arg_base__;      \
                RXVM_INSTRUMENTATION_CALL(                                      \
                        RXVM_PROFILE_CALL_DIRECT_BYTECODE, mapped_called__,     \
                        temp_frame->number_args,                                \
                        RXVM_PROFILE_FRAME_LAST_ACTIVATION,                     \
                        RXVM_PROFILE_CALL_SUCCESS, current_frame,               \
                        current_module->module_number, VM_CANONICAL_INDEX(pc),  \
                        mapped_arg_base__, 1);                                  \
                VM_ACTIVATE_FRAME(temp_frame, RXVM_TRANSITION_CALL);            \
                VM_SELECT_INDEX(mapped_called__->start, RXVM_TRANSITION_CALL);  \
                mapped_j__ = current_frame->procedure->binarySpace->globals +   \
                             current_frame->procedure->locals + 1;              \
                for (mapped_i__ = 0; mapped_i__ < (size_t)mapped_arg_count__;   \
                     mapped_i__++, mapped_j__++) {                              \
                    current_frame->baselocals[mapped_j__] =                     \
                            current_frame->parent->locals[mapped_arg_base__ + mapped_i__]; \
                    current_locals[mapped_j__] = current_frame->baselocals[mapped_j__]; \
                }                                                               \
            }                                                                   \
        } while (0)


RXVM_HANDLER(CALL_REG_FUNC_REG, VM_ADVANCE(3);
            {
                proc_runtime *called_function = PROC_OP(2);
                DEBUG("TRACE - CALL R%lu,%s,R%lu\n", REG_IDX(1), called_function->name, REG_IDX(3));
                if (called_function->start == SIZE_MAX) {
                    RXVM_INSTRUMENTATION_CALL(
                            called_function->binarySpace
                                ? RXVM_PROFILE_CALL_DIRECT_BYTECODE
                                : RXVM_PROFILE_CALL_DIRECT_NATIVE,
                            called_function, op3R->int_value,
                            RXVM_PROFILE_FRAME_NONE_FAILED,
                            RXVM_PROFILE_CALL_UNRESOLVED, current_frame,
                            current_module->module_number, VM_CANONICAL_INDEX(pc),
                            REG_IDX(3) + 1, 1);
                    SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, called_function->name);
                    DISPATCH;
                }

                if (called_function->binarySpace == 0) {
                    /* This is a native plugin function */
                    RXVM_INSTRUMENTATION_CALL(
                            RXVM_PROFILE_CALL_DIRECT_NATIVE, called_function,
                            op3R->int_value,
                            RXVM_PROFILE_FRAME_NO_CHILD_NATIVE,
                            RXVM_PROFILE_CALL_SUCCESS, current_frame,
                            current_module->module_number, VM_CANONICAL_INDEX(pc),
                            REG_IDX(3) + 1, 1);
                    RXVM_INSTRUMENTATION_NATIVE_BEGIN(called_function);
                    rxvm_call_native_procedure(called_function,
                                               op3R->int_value, (&(op3R)) + 1,
                                               op1R, signal_value);
                    RXVM_INSTRUMENTATION_NATIVE_END();
                    INTERRUPT_FROM_RXPA_SIGNAL(signal_value);
                    RXVM_EXTERNAL_SAFEPOINT();
                } else {
                    /* This is a CREXX Procedure */
                    /* New stackframe - grabbing a procedure object from the caller frame */
                    size_t caller_arg_base = REG_IDX(3) + 1;
                    temp_frame = frame_f(called_function, (int) op3R->int_value, current_frame, next_pc, op1R);
                    if (!temp_frame) {
                        RXVM_INSTRUMENTATION_CALL(
                                RXVM_PROFILE_CALL_DIRECT_BYTECODE,
                                called_function, op3R->int_value,
                                RXVM_PROFILE_FRAME_NONE_FAILED,
                                RXVM_PROFILE_CALL_FRAME_FAILED, current_frame,
                                current_module->module_number,
                                VM_CANONICAL_INDEX(pc), caller_arg_base, 1);
                        SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Unable to allocate stack frame")
                        DISPATCH;
                    }
                    assert(caller_arg_base <= UINT32_MAX);
                    temp_frame->caller_arg_base = (uint32_t)caller_arg_base;
                    RXVM_INSTRUMENTATION_CALL(
                            RXVM_PROFILE_CALL_DIRECT_BYTECODE, called_function,
                            temp_frame->number_args,
                            RXVM_PROFILE_FRAME_LAST_ACTIVATION,
                            RXVM_PROFILE_CALL_SUCCESS, current_frame,
                            current_module->module_number, VM_CANONICAL_INDEX(pc),
                            caller_arg_base, 1);
                    /* Prepare dispatch to procedure as early as possible */
                    VM_ACTIVATE_FRAME(temp_frame, RXVM_TRANSITION_CALL);
                    VM_SELECT_INDEX(called_function->start, RXVM_TRANSITION_CALL);


                    /* Arguments - complex lets never have to change this code! */
                    size_t j =
                            current_frame->procedure->binarySpace->globals +
                            current_frame->procedure->locals + 1; /* Callee register index */
                    size_t k = (pc + 3)->index + 1; /* Caller register index */
                    size_t i;
                    for (i = 0;
                         i < (current_frame->parent->locals[(pc + 3)->index])->int_value;
                         i++, j++, k++) {
                        /* Arguments have no frame-owned value storage.  Keep
                         * their entry mapping in baselocals so UNLINK and a
                         * cold signal unwind can restore an argument-slot call
                         * window without copying or losing exposed links. */
                        current_frame->baselocals[j] = current_frame->parent->locals[k];
                        current_locals[j] = current_frame->baselocals[j];
                    }
                }
            }
            DISPATCH;


)

RXVM_HANDLER(DCALL_REG_REG_REG, VM_ADVANCE(3);
            {
                /* Function pointer is in register 2 */
                proc_runtime *called_function = (proc_runtime *) op2R->int_value;
                DEBUG("TRACE - DCALL R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
                if (called_function->start == SIZE_MAX) {
                    RXVM_INSTRUMENTATION_CALL(
                            called_function->binarySpace
                                ? RXVM_PROFILE_CALL_DYNAMIC_BYTECODE
                                : RXVM_PROFILE_CALL_DYNAMIC_NATIVE,
                            called_function, op3R->int_value,
                            RXVM_PROFILE_FRAME_NONE_FAILED,
                            RXVM_PROFILE_CALL_UNRESOLVED, current_frame,
                            current_module->module_number, VM_CANONICAL_INDEX(pc),
                            REG_IDX(3) + 1, 1);
                    SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, called_function->name);
                    DISPATCH;
                }

                if (called_function->binarySpace == 0) {
                    /* This is a native plugin function */
                    RXVM_INSTRUMENTATION_CALL(
                            RXVM_PROFILE_CALL_DYNAMIC_NATIVE, called_function,
                            op3R->int_value,
                            RXVM_PROFILE_FRAME_NO_CHILD_NATIVE,
                            RXVM_PROFILE_CALL_SUCCESS, current_frame,
                            current_module->module_number, VM_CANONICAL_INDEX(pc),
                            REG_IDX(3) + 1, 1);
                    RXVM_INSTRUMENTATION_NATIVE_BEGIN(called_function);
                    rxvm_call_native_procedure(called_function,
                                               op3R->int_value, (&(op3R)) + 1,
                                               op1R, signal_value);
                    RXVM_INSTRUMENTATION_NATIVE_END();
                    INTERRUPT_FROM_RXPA_SIGNAL(signal_value);
                    RXVM_EXTERNAL_SAFEPOINT();
                } else {
                    /* This is a CREXX Procedure */
                    size_t caller_arg_base = REG_IDX(3) + 1;
                    temp_frame = frame_f(called_function, (int) op3R->int_value, current_frame, next_pc, op1R);
                    if (!temp_frame) {
                        RXVM_INSTRUMENTATION_CALL(
                                RXVM_PROFILE_CALL_DYNAMIC_BYTECODE,
                                called_function, op3R->int_value,
                                RXVM_PROFILE_FRAME_NONE_FAILED,
                                RXVM_PROFILE_CALL_FRAME_FAILED, current_frame,
                                current_module->module_number,
                                VM_CANONICAL_INDEX(pc), caller_arg_base, 1);
                        SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Unable to allocate stack frame")
                        DISPATCH;
                    }
                    assert(caller_arg_base <= UINT32_MAX);
                    temp_frame->caller_arg_base = (uint32_t)caller_arg_base;
                    RXVM_INSTRUMENTATION_CALL(
                            RXVM_PROFILE_CALL_DYNAMIC_BYTECODE, called_function,
                            temp_frame->number_args,
                            RXVM_PROFILE_FRAME_LAST_ACTIVATION,
                            RXVM_PROFILE_CALL_SUCCESS, current_frame,
                            current_module->module_number, VM_CANONICAL_INDEX(pc),
                            caller_arg_base, 1);
                    /* Prepare dispatch to procedure as early as possible */
                    VM_ACTIVATE_FRAME(temp_frame, RXVM_TRANSITION_CALL);
                    VM_SELECT_INDEX(called_function->start, RXVM_TRANSITION_CALL);


                    /* Arguments - complex lets never have to change this code! */
                    size_t j =
                            current_frame->procedure->binarySpace->globals +
                            current_frame->procedure->locals + 1; /* Callee register index */
                    size_t k = (pc + 3)->index + 1; /* Caller register index */
                    size_t i;
                    for (i = 0;
                         i < (current_frame->parent->locals[(pc + 3)->index])->int_value;
                         i++, j++, k++) {
                        current_frame->baselocals[j] = current_frame->parent->locals[k];
                        current_locals[j] = current_frame->baselocals[j];
                    }
                }
            }
            DISPATCH;


)

RXVM_HANDLER(RET,
            DEBUG("TRACE - RET\n");
            {
                RXVM_INSTRUMENTATION_RETURN(
                        current_frame->parent
                            ? RXVM_PROFILE_RETURN_VOID
                            : RXVM_PROFILE_RETURN_TERMINAL_EXTERNAL);
                /* Where we return to */
                next_pc = current_frame->return_pc;
                unsigned char is_interrupt = current_frame->is_interrupt;
                /* back to the parent's stack frame */
                temp_frame = current_frame;
                VM_ACTIVATE_FRAME_OR_NULL(current_frame->parent, RXVM_TRANSITION_RETURN);
                if (!current_frame) {
                    DEBUG("TRACE - RETURNING FROM MAIN()\n");
                    /* Copy back arguments for external calls */
                    if (context->ext_proc && context->ext_args) {
                        int k, m;
                        int nargs = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                                temp_frame->procedure->locals]->int_value;
                        for (k = 0, m = temp_frame->procedure->binarySpace->globals + temp_frame->procedure->locals + 1;
                             k < nargs && k < context->ext_argc;
                             k++, m++) {
                            copy_value(context->ext_args[k], temp_frame->locals[m]);
                        }
                    }
                    /* Free Argument Values a1... */
                    int i, j;
                    /* a0 is the number of args */
                    int num_args = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                               temp_frame->procedure->locals]->int_value;
                    for (i = 0, j = temp_frame->procedure->binarySpace->globals + temp_frame->procedure->locals + 1;
                         i < num_args;
                         i++, j++) {
                        value_free(temp_frame->baselocals[j]);
                    }
                    rc = 0;
                    free_frame(temp_frame);
                    arguments_array = 0; /* We have freed it in the loop above */
                    RXVM_HANDLER_FINISH();
                }
                HANDLE_INTERRUPT_ACTION_RETURN()
                free_frame(temp_frame);
                VM_SELECT_POINTER(next_pc, RXVM_TRANSITION_RETURN);
                if (is_interrupt == RXSIGNAL_BREAKPOINT) {
                    VM_RESUME_INTERRUPTED(is_interrupt); /* Bypass the still-pending breakpoint check. */
                }
                DISPATCH;
            }


)

RXVM_HANDLER(RET_REG,
            DEBUG("TRACE - RET R%lu\n", REG_IDX(1));
            {
                if (!current_frame->parent) {
                    RXVM_INSTRUMENTATION_RETURN(
                            RXVM_PROFILE_RETURN_TERMINAL_EXTERNAL);
                } else if (!current_frame->return_reg) {
                    RXVM_INSTRUMENTATION_RETURN(
                            RXVM_PROFILE_RETURN_VALUE_IGNORED);
                } else if (REG_IDX(1) >= current_frame->procedure->locals ||
                           current_locals[REG_IDX(1)] !=
                               current_frame->baselocals[REG_IDX(1)]) {
                    RXVM_INSTRUMENTATION_RETURN(
                            RXVM_PROFILE_RETURN_COPY_NONLOCAL);
                } else {
                    RXVM_INSTRUMENTATION_RETURN(
                            RXVM_PROFILE_RETURN_MOVE_LOCAL);
                }
                /* Where we return to */
                next_pc = current_frame->return_pc;
                unsigned char is_interrupt = current_frame->is_interrupt;
                int return_rc = (int)op1R->int_value;
                /* Set the result register */
                if (current_frame->return_reg) {
                    if (REG_IDX(1) >= current_frame->procedure->locals || /* Not a local */
                        current_locals[REG_IDX(1)] != current_frame->baselocals[REG_IDX(1)]) /* swapped/linked so might not be a local really */
                        copy_value(current_frame->return_reg,
                                   op1R); /* Must do a copy if it could be an argument, object attribute or global because ... */
                    else
                        move_value(current_frame->return_reg,
                                   op1R); /* ... the faster move deletes the source which is ok for locals going out of scope */
                }
                /* back to the parents stack frame */
                temp_frame = current_frame;
                VM_ACTIVATE_FRAME_OR_NULL(current_frame->parent, RXVM_TRANSITION_RETURN);
                if (!current_frame) {
                    DEBUG("TRACE - RETURNING FROM MAIN()\n");
                    /* Exiting - grab the int rc */
                    if (temp_frame->return_reg) rc = (int) (temp_frame->return_reg->int_value);
                    else rc = return_rc;

                    /* Copy back arguments for external calls */
                    if (context->ext_proc && context->ext_args) {
                        int k, m;
                        int nargs = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                                temp_frame->procedure->locals]->int_value;
                        for (k = 0, m = temp_frame->procedure->binarySpace->globals + temp_frame->procedure->locals + 1;
                             k < nargs && k < context->ext_argc;
                             k++, m++) {
                            copy_value(context->ext_args[k], temp_frame->locals[m]);
                        }
                    }
                    /* Free Argument Values a1... */
                    int i, j;
                    /* a0 is the number of args */
                    int num_args = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                               temp_frame->procedure->locals]->int_value;
                    for (i = 0, j =
                                        temp_frame->procedure->binarySpace
                                                ->globals +
                                        temp_frame->procedure->locals + 1;
                            i < num_args;
                            i++, j++) {
                        value_free(temp_frame->baselocals[j]);
                    }
                    free_frame(temp_frame);
                    arguments_array = 0; /* We have freed it in the loop above */
                    RXVM_HANDLER_FINISH();
                }
                // Set the numeric context of the decimal plugin
                if (current_frame->decimal) {
                    current_frame->decimal->num_context = &current_frame->num_context;
                    current_frame->decimal->syncNumericContext(current_frame->decimal);
                }
                HANDLE_INTERRUPT_ACTION_RETURN()
                free_frame(temp_frame);
                VM_SELECT_POINTER(next_pc, RXVM_TRANSITION_RETURN);
                if (is_interrupt == RXSIGNAL_BREAKPOINT) {
                    VM_RESUME_INTERRUPTED(is_interrupt); /* Bypass the still-pending breakpoint check. */
                }
                DISPATCH;
            }


)

RXVM_HANDLER(RET_FLOAT,
            DEBUG("TRACE - RET %.15g\n", op1F);
            {
                RXVM_INSTRUMENTATION_RETURN(
                        !current_frame->parent
                            ? RXVM_PROFILE_RETURN_TERMINAL_EXTERNAL
                            : (current_frame->return_reg
                                ? RXVM_PROFILE_RETURN_IMMEDIATE
                                : RXVM_PROFILE_RETURN_VALUE_IGNORED));
                /* Where we return to */
                next_pc = current_frame->return_pc;
                unsigned char is_interrupt = current_frame->is_interrupt;
                /* Set the result register */
                if (current_frame->return_reg)
                    current_frame->return_reg->float_value = op1F;
                /* back to the parents stack frame */
                temp_frame = current_frame;
                VM_ACTIVATE_FRAME_OR_NULL(current_frame->parent, RXVM_TRANSITION_RETURN);
                if (!current_frame) {
                    DEBUG("TRACE - RETURNING FROM MAIN()\n");
                    /* Copy back arguments for external calls */
                    if (context->ext_proc && context->ext_args) {
                        int k, m;
                        int nargs = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                                temp_frame->procedure->locals]->int_value;
                        for (k = 0, m = temp_frame->procedure->binarySpace->globals + temp_frame->procedure->locals + 1;
                             k < nargs && k < context->ext_argc;
                             k++, m++) {
                            copy_value(context->ext_args[k], temp_frame->locals[m]);
                        }
                    }
                    /* Free Argument Values a1... */
                    int i, j;
                    /* a0 is the number of args */
                    int num_args = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                               temp_frame->procedure->locals]->int_value;
                    for (i = 0, j = temp_frame->procedure->binarySpace->globals +
                                    temp_frame->procedure->locals + 1;
                            i < num_args;
                            i++, j++) {
                        value_free(temp_frame->baselocals[j]);
                    }
                    rc = 0;
                    free_frame(temp_frame);
                    arguments_array = 0; /* We have freed it in the loop above */
                    RXVM_HANDLER_FINISH();
                }
                // Set the numeric context of the decimal plugin
                if (current_frame->decimal) {
                    current_frame->decimal->num_context = &current_frame->num_context;
                    current_frame->decimal->syncNumericContext(current_frame->decimal);
                }
                HANDLE_INTERRUPT_ACTION_RETURN()
                free_frame(temp_frame);
                VM_SELECT_POINTER(next_pc, RXVM_TRANSITION_RETURN);
                if (is_interrupt == RXSIGNAL_BREAKPOINT) {
                    VM_RESUME_INTERRUPTED(is_interrupt); /* Bypass the still-pending breakpoint check. */
                }
                DISPATCH;
            }

            /* ------------------------------------------------------------------------------------
            *  RET_STRING                                                        pej 12. April 2021
            *  -----------------------------------------------------------------------------------
            */

)

RXVM_HANDLER(RET_STRING,
            DEBUG("TRACE - RET \"%.*s\"\n", (int)op1S->string_len, op1S->string);
            {
                RXVM_INSTRUMENTATION_RETURN(
                        !current_frame->parent
                            ? RXVM_PROFILE_RETURN_TERMINAL_EXTERNAL
                            : (current_frame->return_reg
                                ? RXVM_PROFILE_RETURN_IMMEDIATE
                                : RXVM_PROFILE_RETURN_VALUE_IGNORED));
                /* Where we return to */
                next_pc = current_frame->return_pc;
                unsigned char is_interrupt = current_frame->is_interrupt;
                /* Set the result register */
                if (current_frame->return_reg)
                    set_const_string(current_frame->return_reg, CONSTSTRING_OP(1));
                /* back to the parents stack frame */
                temp_frame = current_frame;
                VM_ACTIVATE_FRAME_OR_NULL(current_frame->parent, RXVM_TRANSITION_RETURN);
                if (!current_frame) {
                    DEBUG("TRACE - RETURNING FROM MAIN()\n");
                    /* Copy back arguments for external calls */
                    if (context->ext_proc && context->ext_args) {
                        int k, m;
                        int nargs = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                                temp_frame->procedure->locals]->int_value;
                        for (k = 0, m = temp_frame->procedure->binarySpace->globals + temp_frame->procedure->locals + 1;
                             k < nargs && k < context->ext_argc;
                             k++, m++) {
                            copy_value(context->ext_args[k], temp_frame->locals[m]);
                        }
                    }
                    /* Free Argument Values a1... */
                    int i, j;
                    /* a0 is the number of args */
                    int num_args = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                               temp_frame->procedure->locals]->int_value;
                    for (i = 0, j = temp_frame->procedure->binarySpace->globals +
                            temp_frame->procedure->locals + 1;
                            i < num_args;
                            i++, j++) {
                        value_free(temp_frame->baselocals[j]);
                    }
                    rc = 0;
                    free_frame(temp_frame);
                    RXVM_HANDLER_FINISH();
                }
                // Set the numeric context of the decimal plugin
                if (current_frame->decimal) {
                    current_frame->decimal->num_context = &current_frame->num_context;
                    current_frame->decimal->syncNumericContext(current_frame->decimal);
                }
                HANDLE_INTERRUPT_ACTION_RETURN()
                free_frame(temp_frame);
                VM_SELECT_POINTER(next_pc, RXVM_TRANSITION_RETURN);
                if (is_interrupt == RXSIGNAL_BREAKPOINT) {
                    VM_RESUME_INTERRUPTED(is_interrupt); /* Bypass the still-pending breakpoint check. */
                }
                DISPATCH;
            }


)

RXVM_HANDLER(MOVE_REG_REG, VM_ADVANCE(2); /* Deprecated */
            DEBUG("TRACE - MOVE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            move_value(op1R, op2R);
            DISPATCH;


)

RXVM_HANDLER(SWAP_REG_REG, VM_ADVANCE(2); /* Deprecated */
            DEBUG("TRACE - SWAP R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                value *v_temp;
                v_temp = op1R;
                op1R = op2R;
                op2R = v_temp;
                RXVM_INSTRUMENTATION_SWAP(current_frame, REG_IDX(1), REG_IDX(2));
            }
            DISPATCH;


)

RXVM_HANDLER(MKREF_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - MKREF R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                size_t site_index = VM_CANONICAL_INDEX(pc);
                const bin_code *pair = 0;
                int minlinkattr1_pair = 0;
                void *owner = 0;
                rxvm_ref_owner_kind owner_kind;
                rxvm_reference_cell *cell;
                int current_frame_owner = 0;

                if (site_index >= 5u) {
                    pair = current_canonical_base + site_index - 5u;
                    minlinkattr1_pair =
                            pair[0].instruction.opcode ==
                                    OP_MINLINKATTR1_REG_REG_REG_INT &&
                            pair[0].instruction.no_ops == 4 &&
                            pair[1].index == REG_IDX(2);
                }

                if (minlinkattr1_pair) {
                    size_t root_register = pair[2].index;
                    size_t index_register = pair[3].index;
                    rxinteger index;
                    value *root = 0;

                    if (root_register < (size_t)current_frame->procedure->locals &&
                        root_register < current_frame->number_locals &&
                        index_register < current_frame->number_locals &&
                        rxinteger_checked_add(current_locals[index_register]->int_value,
                                              pair[4].iconst,
                                              &index)) {
                        root = current_locals[root_register];
                        if (root == current_frame->baselocals[root_register] &&
                            index >= 1 && (size_t)index <= root->num_attributes &&
                            root->attributes && root->unlinked_attributes &&
                            root->attributes[(size_t)index - 1u] == op2R &&
                            root->unlinked_attributes[(size_t)index - 1u] == op2R) {
                            owner_kind = RXVM_REF_ATTRIBUTE;
                            current_frame_owner = 1;
                        }
                    }
                } else if (REG_IDX(2) < (size_t)current_frame->procedure->locals &&
                           REG_IDX(2) < current_frame->number_locals &&
                           current_locals[REG_IDX(2)] ==
                                   current_frame->baselocals[REG_IDX(2)] &&
                           op2R == current_frame->baselocals[REG_IDX(2)]) {
                    owner_kind = RXVM_REF_LOCAL;
                    owner = current_frame;
                    current_frame_owner = 1;
                }

                if (!current_frame_owner) {
                    owner_kind = rxvm_reference_owner_kind_for_storage(current_frame,
                                                                       REG_IDX(2),
                                                                       op2R,
                                                                       &owner);
                }
                cell = rxvm_reference_identity_for_context(&context->references,
                                                           op2R,
                                                           owner_kind,
                                                           owner,
                                                           REG_IDX(2),
                                                           0);
                if (!cell) {
                    SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
                    DISPATCH;
                }

                if (current_frame_owner) current_frame->has_reference_lifetimes = 1;
                else rxvm_mark_reference_lifetime_owner(current_frame, op2R);
                clear_value_contents(op1R);
                rxvm_reference_value_set_payload(op1R, cell);
            }
            DISPATCH;


)

RXVM_HANDLER(DEREF_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - DEREF R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                rxvm_reference_cell *cell = rxvm_reference_payload_cell(op2R);
                value *target = rxvm_reference_cell_target(cell);
                if (!target) {
                    SET_SIGNAL_MSG(RXSIGNAL_REFERENCE_INVALID, "Reference is invalid");
                    DISPATCH;
                }
                copy_value(op1R, target);
            }
            DISPATCH;


)

RXVM_HANDLER(LINKREF_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - LINKREF R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                rxvm_reference_cell *cell = rxvm_reference_payload_cell(op2R);
                value *target = rxvm_reference_cell_target(cell);
                if (!target) {
                    SET_SIGNAL_MSG(RXSIGNAL_REFERENCE_INVALID, "Reference is invalid");
                    DISPATCH;
                }
                op1R = target;
            }
            DISPATCH;


)

RXVM_HANDLER(SETREF_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - SETREF R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                rxvm_reference_cell *cell = rxvm_reference_payload_cell(op1R);
                value *target = rxvm_reference_cell_target(cell);
                if (!target) {
                    SET_SIGNAL_MSG(RXSIGNAL_REFERENCE_INVALID, "Reference is invalid");
                    DISPATCH;
                }
                copy_value(target, op2R);
            }
            DISPATCH;


)

RXVM_HANDLER(REFVALID_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - REFVALID R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                int valid = rxvm_reference_cell_is_valid(rxvm_reference_payload_cell(op2R));
                value_zero(op1R);
                op1R->int_value = valid ? 1 : 0;
            }
            DISPATCH;


)

RXVM_HANDLER(UNREF_REG, VM_ADVANCE(1);
            DEBUG("TRACE - UNREF R%lu\n", REG_IDX(1));
            clear_value_contents(op1R);
            DISPATCH;


)

RXVM_HANDLER(ENDLIFE_REG, VM_ADVANCE(1);
            DEBUG("TRACE - ENDLIFE R%lu\n", REG_IDX(1));
            release_value_reference_lifetime(op1R);
            DISPATCH;

        /* Link attribute op3 of op2 to op1 */

)

RXVM_HANDLER(LINKATTR_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - LINKATTR R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            if (op3R->int_value < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            if (op3R->int_value >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            op1R = op2R->attributes[op3R->int_value];
            DISPATCH;

        /* Link attribute op3 of op2 to op1 */

)

RXVM_HANDLER(LINKATTR_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - LINKATTR R%lu,R%lu,%d\n", REG_IDX(1), REG_IDX(2), (int)op3I);
            if ((int)op3I < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            if ((int)op3I >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            op1R = op2R->attributes[(int)op3I];
            DISPATCH;

        /* Link attribute op3 (1 base) of op2 to op1 */

)

RXVM_HANDLER(LINKATTR1_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - LINKATTR1 R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            if (op3R->int_value - 1 < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            if (op3R->int_value - 1 >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            op1R = op2R->attributes[op3R->int_value - 1];
            DISPATCH;

        /* Link attribute op3 (1 base) of op2 to op1 */

)

RXVM_HANDLER(LINKATTR1_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - LINKATTR1 R%lu,R%lu,%d\n", REG_IDX(1), REG_IDX(2), (int)op3I);
            if ((int)op3I - 1 < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            if ((int)op3I - 1 >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            op1R = op2R->attributes[(int)op3I - 1];
            DISPATCH;


)

RXVM_PRIVATE_HANDLER(PRIVATE_R2_COPYATTR1, RXVM_PRIVATE_R2_COPYATTR1_REG_REG_INT, OP_LINKATTR1_REG_REG_INT,
            DEBUG("TRACE - LINKATTR1 R%lu,R%lu,%d\n",
                  REG_IDX(1), REG_IDX(2), (int)op3I);
            {
                size_t site_index = VM_CANONICAL_INDEX(pc);
                size_t temporary = REG_IDX(1);
                size_t destination = (pc + 5)->index;
                value *object = op2R;
                value *source;

                if ((int)op3I - 1 < 0 ||
                        (int)op3I - 1 >= object->num_attributes) {
                    /* Canonical LINKATTR1 signals before changing the temporary
                     * and resumes at COPY if the signal is ignored. */
                    VM_SELECT_INDEX(site_index + 4u,
                                    RXVM_TRANSITION_SEQUENTIAL);
                    SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                    DISPATCH;
                }

                source = object->attributes[(int)op3I - 1];
                if (context->debug_mode ||
                        (pending_interrupts & rxsignal_mask(RXSIGNAL_BREAKPOINT)) != 0 ||
                        !rxvm_reference_payload_cell(source)) {
                    /* Preserve instruction-by-instruction debug/breakpoint
                     * observation and the complete generic value fallback. */
                    current_locals[temporary] = source;
                    VM_SELECT_INDEX(site_index + 4u,
                                    RXVM_TRANSITION_SEQUENTIAL);
                    DISPATCH;
                }

                copy_value(current_locals[destination], source);
                current_locals[temporary] = current_frame->baselocals[temporary];
                VM_SELECT_INDEX(site_index + 9u, RXVM_TRANSITION_SEQUENTIAL);
                DISPATCH;
            }

        /* Link op3 to attribute op1 of op2 */

)

RXVM_HANDLER(LINKTOATTR_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - LINKTOATTR R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            if (op1R->int_value < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            if (op1R->int_value >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            op2R->attributes[op1R->int_value] = op3R;
            DISPATCH;

        /* Link op3 to attribute op1 of op2 */

)

RXVM_HANDLER(LINKTOATTR_INT_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - LINKTOATTR %d,R%lu,R%lu\n", (int)op1I, REG_IDX(2), REG_IDX(3));
            if ((int)op1I < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            if ((int)op1I >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            op2R->attributes[(int)op1I] = op3R;
            DISPATCH;

        /* Link op3 to attribute op1 (1 base) of op2 */

)

RXVM_HANDLER(LINKTOATTR1_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - LINKTOATTR1 R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            if (op1R->int_value - 1 < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            if (op1R->int_value - 1 >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            op2R->attributes[op1R->int_value - 1] = op3R;
            DISPATCH;

        /* Link op3 to attribute op1 (1 base) of op2 */

)

RXVM_HANDLER(LINKTOATTR1_INT_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - LINKTOATTR1 %d,R%lu,R%lu\n", (int)op1I, REG_IDX(2), REG_IDX(3));
            if ((int)op1I - 1 < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            if ((int)op1I - 1 >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            op2R->attributes[(int)op1I -1] = op3R;
            DISPATCH;

        /* Unlink attribute op1 of op2 */

)

RXVM_HANDLER(UNLINKATTR_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - UNLINKATTR R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            if (op1R->int_value < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            if (op1R->int_value >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            op2R->attributes[op1R->int_value] = op2R->unlinked_attributes[op1R->int_value];
            DISPATCH;

        /* Unlink attribute op1 of op2 */

)

RXVM_HANDLER(UNLINKATTR_INT_REG, VM_ADVANCE(2);
            DEBUG("TRACE - UNLINKATTR %d,R%lu\n", (int)op1I, REG_IDX(2));
            if ((int)op1I < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            if ((int)op1I >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            op2R->attributes[(int)op1I] = op2R->unlinked_attributes[(int)op1I];
            DISPATCH;

        /* Unlink attribute op1 (1 base) of op2 */

)

RXVM_HANDLER(UNLINKATTR1_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - UNLINKATTR1 R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            if (op1R->int_value - 1 < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            if (op1R->int_value - 1 >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            op2R->attributes[op1R->int_value - 1] = op2R->unlinked_attributes[op1R->int_value - 1];
            DISPATCH;

        /* Unlink attribute op1 (1 base) of op2 */

)

RXVM_HANDLER(UNLINKATTR1_INT_REG, VM_ADVANCE(2);
            DEBUG("TRACE - UNLINKATTR1 %d,R%lu\n", (int)op1I, REG_IDX(2));
            if ((int)op1I - 1 < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            if ((int)op1I - 1 >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            op2R->attributes[(int)op1I - 1] = op2R->unlinked_attributes[(int)op1I - 1];
            DISPATCH;

            /* Link op2 to op1 */

)

RXVM_HANDLER(LINK_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - LINK R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            op1R = op2R;
            DISPATCH;

        /* Link parent-frame-register[op2] to op1 */

)

RXVM_HANDLER(METALINKPREG_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - METALINKPREG R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            op1R = current_frame->parent->locals[op2R->int_value];
            DISPATCH;

        /* Unlink op1 */

)

RXVM_HANDLER(UNLINK_REG, VM_ADVANCE(1);
            DEBUG("TRACE - UNLINK R%lu\n", REG_IDX(1));
            op1R = (current_frame->baselocals[(pc + 1)->index]);
            DISPATCH;

        /* ------------------------------------------------------------------------------------
         *  GETATTRS_REG_REG Get Number Attributes op1 = op2.num_attributes
         *  ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(GETATTRS_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - GETATTRS R%lu,R%lu\n", REG_IDX(1),REG_IDX(2));
            REG_RETURN_INT(op2R->num_attributes)
            DISPATCH;

        /* ------------------------------------------------------------------------------------
         *  GETATTRS_REG_REG_INT Get Number Attributes op1 = op2.num_attributes + op3
         *  ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(GETATTRS_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - GETATTRS R%lu,R%lu,%d\n", REG_IDX(1),REG_IDX(2),(int)op3I);
            REG_RETURN_INT(op2R->num_attributes + op3I)
            DISPATCH;

        /* ------------------------------------------------------------------------------------
         *  SETATTRS_REG_REG Set Number Attributes op1.num_attributes = op2
         * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(SETATTRS_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - SETATTRS R%lu,R%lu\n", REG_IDX(1),REG_IDX(2));
            set_num_attributes(op1R, op2RI);
            DISPATCH;

        /* ------------------------------------------------------------------------------------
         *  SETATTRS_REG_REG Set Number Attributes op1.num_attributes = op2
         * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(SETATTRS_REG_INT, VM_ADVANCE(2);
            DEBUG("TRACE - SETATTRS R%lu,%d\n", REG_IDX(1),(int)op2I);
            set_num_attributes(op1R, op2I);
            DISPATCH;

        /* ------------------------------------------------------------------------------------
         *  SETATTRS_REG_REG_INT Set Number Attributes op1.num_attributes = op2 + op3
         * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(SETATTRS_REG_REG_INT, VM_ADVANCE(3);
            DEBUG("TRACE - SETATTRS R%lu,R%lu,%d\n", REG_IDX(1),REG_IDX(2), (int)op3I);
            set_num_attributes(op1R, op2RI + op3I);
            DISPATCH;

        /* ------------------------------------------------------------------------------------
         *  SETATTRS_REG_INT_INT Set Number Attributes op1.num_attributes = op2 + op3
         * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(SETATTRS_REG_INT_INT, VM_ADVANCE(3);
            DEBUG("TRACE - SETATTRS R%lu,%d,%d\n", REG_IDX(1),(int)op2I, (int)op3I);
            set_num_attributes(op1R, op2I + op3I);
            DISPATCH;

        /* ------------------------------------------------------------------------------------
         *  MINATTRS_REG_REG Ensure min number attributes op1.num_attributes >= op2
         * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(MINATTRS_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - MINATTRS R%lu,R%lu\n", REG_IDX(1),REG_IDX(2));
            if (op2RI > op1R->num_attributes) {
                set_num_attributes(op1R, op2RI);
            }
            DISPATCH;

        /* ------------------------------------------------------------------------------------
         *  MINATTRS_REG_REG Ensure min number attributes op1.num_attributes >= op2
         * ----------------------------------------------------------------------------------- */

)

RXVM_HANDLER(MINATTRS_REG_INT, VM_ADVANCE(2);
            DEBUG("TRACE - MINATTRS R%lu,%d\n", REG_IDX(1),(int)op2I);
            if (op2I > op1R->num_attributes) {
                set_num_attributes(op1R, op2I);
            }
            DISPATCH;

        /* ------------------------------------------------------------------------------------
         *  MINATTRS_REG_REG_INT Ensure min number attributes op1.num_attributes >= op2 + op3
         * ----------------------------------------------------------------------------------- */

)
