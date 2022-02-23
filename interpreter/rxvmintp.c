//
// Created by adrian on 29/03/2021.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include "platform.h"
#include "rxas.h"
#include "rxvminst.h"
#include "rxastree.h"
#include "rxvmintp.h"
#include "rxvmvars.h"

/* This defines the expected max number of args - if a call has more args than
 * this then an oversized block will be malloced
 * In terms of memory usage / waste each one is only 2 x pointer size */
#define NOMINAL_NUM_ARGS 20

/* Stack Frame Factory */
RX_INLINE stack_frame *frame_f(
                    proc_constant *procedure,
                    int no_args,
                    stack_frame *parent,
                    bin_code *return_pc,
                    void *return_inst,
                    value *return_reg) {
    stack_frame *this;
    int num_locals;
    int nominal_num_locals;
    int i, j;
    size_t frame_size;
    value *value_buffer;

    num_locals = procedure->locals + procedure->binarySpace->globals + no_args + 1;
    nominal_num_locals = procedure->locals + procedure->binarySpace->globals + NOMINAL_NUM_ARGS + 1;

    /* Do we need an oversized block */
    if (num_locals > nominal_num_locals) nominal_num_locals = num_locals;

    if (*procedure->frame_free_list &&
        (*procedure->frame_free_list)->nominal_number_locals >= num_locals) {

        /* We can reuse this stack frame */
        this = *procedure->frame_free_list;
        *procedure->frame_free_list = this->prev_free;
        this->prev_free = 0;

        /* Reset Local Registers */
        for (i = 0; i < procedure->locals; i++) {
            this->locals[i] = this->baselocals[i];
            value_zero(this->locals[i]);
        }
        /* Make sure global registers are linked correctly */
        for (j = 0; j < procedure->binarySpace->globals; i++, j++) {
            this->locals[i] = this->baselocals[i];
        }
        /* Reset register a0 - number of arguments */
        this->locals[i] = this->baselocals[i];
        value_zero(this->locals[i]);
        this->locals[i]->int_value = no_args;
    }
    else {
        /* Need a new stack frame - allocate all the memory in one go */
        frame_size = sizeof(stack_frame) +
                     ( sizeof(value*) * nominal_num_locals * 2 ) +
                     ( sizeof(value) * (procedure->locals + 1)); /* +1 is for a0 */

        this = (stack_frame *) malloc( frame_size );
        this->prev_free = 0;

        this->baselocals = (value**)(this + 1);
        this->locals = this->baselocals + nominal_num_locals;
        value_buffer = (value*)(this->locals + nominal_num_locals);

        /* Link Locals */
        for (i = 0; i < procedure->locals; i++, value_buffer++) {
            value_init(value_buffer);
            this->locals[i] = value_buffer;
            this->baselocals[i] = value_buffer;
        }

        /* Link Globals */
        for (j = 0; j < procedure->binarySpace->globals; i++, j++) {
            this->baselocals[i] =  procedure->binarySpace->module->globals[j];
            this->locals[i] = procedure->binarySpace->module->globals[j];
        }

        /* Link a0 */
        value_init(value_buffer);
        this->locals[i] = value_buffer;
        this->baselocals[i] = value_buffer;
        this->locals[i]->int_value = no_args;

        this->nominal_number_locals = nominal_num_locals;
    }
    this->parent = parent;
    this->return_inst = return_inst;
    this->return_pc = return_pc;
    this->number_locals = num_locals;
    this->number_args = no_args;
    this->return_reg = return_reg;
    this->procedure = procedure;

    return this;
}

/* Free Stack Frame */
RX_INLINE void free_frame(stack_frame *frame) {
    /* Add to free list */
    frame->prev_free = *(frame->procedure->frame_free_list);
    *(frame->procedure->frame_free_list) = frame;
}

/* Clear Stack Frame - deallocating register contents */
RX_INLINE void clear_frame(stack_frame *frame) {
    int i;
    /* Reset Local Registers */
    for (i = 0; i < frame->procedure->locals; i++) {
        clear_value(frame->baselocals[i]);
    }
}

/* Interpreter */
RX_FLATTEN int run(int num_modules, module *program, int argc, char *argv[],
        int debug_mode) {
    proc_constant *procedure;
    int rc = 0;
    bin_code *pc, *next_pc;
#ifdef NTHREADED
    void *next_inst = 0;
#else
    void *next_inst = &&IUNKNOWN;
#endif
    stack_frame *current_frame = 0, *temp_frame;
    /* Linker Stuff */
    value *g_reg;
    int mod_index;
    chameleon_constant *c_entry;
    proc_constant *p_entry, *p_entry_linked;
    struct avl_tree_node *exposed_proc_tree = 0;
    struct avl_tree_node *exposed_reg_tree = 0;
    char check_breakpoint = 0;

    /*
     * Instruction database - loaded from a generated header file
     */
    Instruction *instruction;
    #include "instrset.h"  /* Set up void *address_map[] */

    /* Link Modules Together */
    DEBUG("Linking - Build Symbols\n");
    for (mod_index = 0; mod_index < num_modules; mod_index++) {
        size_t i = 0;
        while (i < program[mod_index].segment.const_size) {
            c_entry =
                    (chameleon_constant *) (
                            program[mod_index].segment.const_pool + i);
            switch (c_entry->type) {

                case PROC_CONST:
                    if (((proc_constant *) c_entry)->start != SIZE_MAX) {
                        /* Mark the owning module segment address */
                        ((proc_constant *) c_entry)->binarySpace =
                                &program[mod_index].segment;
                        /* Stack Frame Free List */
                        ((proc_constant *) c_entry)->frame_free_list = &(((proc_constant *) c_entry)->frame_free_list_head);
                        *(((proc_constant *) c_entry)->frame_free_list) = 0;
                    }
                    break;

                case EXPOSE_REG_CONST:
                    /* Exposed Register */
                    if (src_node(exposed_reg_tree,
                                 ((expose_reg_constant *) c_entry)->index,
                                 (size_t *) &g_reg)) {
                        /* Register already exposed / initialised */
                        program[mod_index]
                                .globals[((expose_reg_constant *) c_entry)
                                ->global_reg] =
                                g_reg;
                    } else {
                        /* Need to initialise a register and expose it in the search tree */
                        program[mod_index].globals[((expose_reg_constant *) c_entry)->global_reg] = value_f();
                        if (add_node(&exposed_reg_tree,
                                     ((expose_reg_constant *) c_entry)->index,
                                     (size_t) (program[mod_index]
                                             .globals[((expose_reg_constant *) c_entry)
                                             ->global_reg]))) {
                            fprintf(stderr,
                                    "Duplicate exposed register symbol: %s\n",
                                    ((expose_reg_constant *) c_entry)->index);
                            exit(-1); /* Duplicate */
                        }
                    }
                    break;

                case EXPOSE_PROC_CONST:
                    /* Exposed Procedure */
                    p_entry =
                            (proc_constant *) (
                                    program[mod_index].segment.const_pool
                                    + ((expose_proc_constant *) c_entry)
                                            ->procedure);

                    if (!((expose_proc_constant *) c_entry)->imported) {
                        if (add_node(&exposed_proc_tree,
                                     ((expose_proc_constant *) c_entry)->index,
                                     (size_t) p_entry)) {
                            fprintf(stderr,
                                    "Duplicate exposed symbol: %s\n",
                                    ((expose_proc_constant *) c_entry)->index);
                            exit(-1); /* Duplicate */
                        }
                    }
                    break;

                default:;
            }

            i += c_entry->size_in_pool;
        }
    }

    DEBUG("Linking - Resolve Symbols\n");
    for (mod_index = 0; mod_index < num_modules; mod_index++) {
        size_t i = 0;
        while (i < program[mod_index].segment.const_size) {
            c_entry =
                    (chameleon_constant *) (
                            program[mod_index].segment.const_pool + i);
            switch (c_entry->type) {
                case EXPOSE_PROC_CONST:
                    p_entry =
                            (proc_constant *) (
                                    program[mod_index].segment.const_pool
                                    + ((expose_proc_constant *) c_entry)
                                            ->procedure);
                    if (((expose_proc_constant *) c_entry)->imported) {
                        if (!src_node(exposed_proc_tree,
                                      ((expose_proc_constant *) c_entry)->index,
                                      (size_t *) &p_entry_linked)) {
                            fprintf(stderr, "Unimplemented symbol: %s\n",
                                    ((expose_proc_constant *) c_entry)->index);
                            exit(-1); /* Unimplemented */
                        }

                        /* Patch the procedure entry with the linked one */
                        p_entry->locals = p_entry_linked->locals;
                        p_entry->start = p_entry_linked->start;
                        p_entry->binarySpace = p_entry_linked->binarySpace;
                        p_entry->frame_free_list = p_entry_linked->frame_free_list;
                    }
                    break;

                default:;
            }

            i += c_entry->size_in_pool;
        }
    }

    /* Free Search Trees */
    DEBUG("Free linking trees\n");
    free_tree(&exposed_proc_tree);
    exposed_proc_tree = 0;
    free_tree(&exposed_reg_tree);
    exposed_reg_tree = 0;

    /* Allocate Module Globals */
    DEBUG("Allocate Module Globals\n");
    for (mod_index = 0; mod_index < num_modules; mod_index++) {
        int i;
        for (i = 0; i < program[mod_index].segment.globals; i++) {
            if (!program[mod_index].globals[i]) {
                program[mod_index].globals[i] = value_f();
            }
        }
    }

    /* Thread code */
#ifndef NTHREADED
    DEBUG("Threading\n");
    for (mod_index=0; mod_index<num_modules; mod_index++) {
        int i = 0, j;
        while (i < program[mod_index].segment.inst_size) {
            j = i;
            i += program[mod_index].segment.binary[i].instruction.no_ops + 1;
            program[mod_index].segment.binary[j].impl_address =
                    address_map[program[mod_index].segment.binary[j].instruction.opcode];
        }
    }
#endif

    /* Find the program's entry point
     * TODO The assembler should save this in the binary structure */
    DEBUG("Find program entry point\n");
    for (mod_index = 0; mod_index < num_modules; mod_index++) {
        size_t i = 0;
        while (i < program[mod_index].segment.const_size) {
            procedure =
                    (proc_constant *) (program[mod_index].segment.const_pool +
                                       i);
            if (procedure->base.type == PROC_CONST &&
                strcmp(procedure->name, "main") == 0)
                break;
            i += procedure->base.size_in_pool;
            procedure = 0;
        }
        if (procedure) break;
    }

    if (!procedure) {
        DEBUG("main() not found\n");
        goto interprt_finished;
    }

    DEBUG("Create first Stack Frame\n");
    current_frame = frame_f(procedure, argc, 0, 0, 0, 0);
    /* Arguments */
    /* a0 is already set by frame_f() */
    /* a1... */
    {
        int i, j;
        for (i = 0, j = procedure->binarySpace->globals + procedure->locals + 1; i < argc; i++, j++) {
            current_frame->baselocals[j] = value_f(); /* note that a1... needs mallocing */
            set_null_string(current_frame->baselocals[j], argv[i]);
            current_frame->locals[j] = current_frame->baselocals[j];
        }
    }


    /* Start */
    DEBUG("Starting inst# %s-0x%x\n",
          procedure->binarySpace->module->name, (int) procedure->start);
    next_pc = &(current_frame->procedure->binarySpace->binary[procedure->start]);
    CALC_DISPATCH_MANUAL;
    DISPATCH;

        /* Instruction implementations */
        /* ----------------------------------------------------------------------------
         * The following shortcut macros are used in the instruction implementation
         *      op1R   address the first register operand
         *      op2R   address the second register operand
         *      op3R   address the third  register operand
         *
         *      op1RI  integer of first register operand
         *      op2RI  integer of second register operand
         *      op3RI  integer of third register operand
         *
         *      op1RF  float of first register operand
         *      op2RF  float of second register operand
         *      op3RF  float of third register operand
         *
         *      op1I   integer value of first operand (non-register value)
         *      op2I   integer value of second operand (non-register value)
         *      op3I   integer value of third  operand (non-register value)
         *
         *      op1F   float value of first operand (non-register value)
         *      op2F   float value of second operand (non-register value)
         *      op3F   float value of third  operand (non-register value)
         *
         *      CONV2INT(integer-result-variable,value-to-be-converted)
         *      CONV2FLOAT(float-result-variable,value-to-be-converted)
         * ----------------------------------------------------------------------------
         */

        /* Breakpoint Support - this is only used/called when check_breakpoint is set */
    START_BREAKPOINT ;
            DEBUG("BREAKPOINT CHECK\n");
            END_BREAKPOINT ;

    START_OF_INSTRUCTIONS ;

        /* Enable Breakpoints */
        START_INSTRUCTION(BPON) CALC_DISPATCH(0);
            DEBUG("TRACE - BPON\n");
            check_breakpoint = 1;
            DISPATCH;

        /* Enable Breakpoints */
        START_INSTRUCTION(BPOFF) CALC_DISPATCH(0);
            DEBUG("TRACE - BPOFF\n");
            check_breakpoint = 0;
            DISPATCH;

        START_INSTRUCTION(LOAD_REG_INT) CALC_DISPATCH(2);
            DEBUG("TRACE - LOAD R%llu,%llu\n", REG_IDX(1), op2I);
            set_int(op1R, op2I);
            DISPATCH;

        START_INSTRUCTION(LOAD_REG_STRING) CALC_DISPATCH(2);
            DEBUG("TRACE - LOAD R%llu,\"%.*s\"\n",
                  REG_IDX(1), (int) (CONSTSTRING_OP(2))->string_len,
                  (CONSTSTRING_OP(2))->string);
            set_const_string(op1R, CONSTSTRING_OP(2));
            DISPATCH;

        /* Readline - Read a line from stdin to a register */
        START_INSTRUCTION(READLINE_REG) CALC_DISPATCH(1);
            DEBUG("TRACE - READLINE R%llu\n", REG_IDX(1));
            {
                size_t pos = 0;
                int ch;
                while ((ch = getchar()) != EOF) {
                    if (ch == '\n') break;
                    extend_string_buffer(op1R, pos+1);
                    op1R->string_value[pos] = (char)ch;
                    pos++;
                }
                op1R->string_pos = 0;
#ifndef NUTF8
                op1R->string_char_pos = 0;
                op1R->string_chars = utf8nlen(op1R->string_value, op1R->string_length);
#endif
            }
            DISPATCH;

        /* String Say - Deprecated */
        START_INSTRUCTION(SSAY_REG) CALC_DISPATCH(1);
            DEBUG("TRACE - SSAY (DEPRICATED) R%llu\n", REG_IDX(1));
            printf("%.*s", (int) op1R->string_length, op1R->string_value);
            DISPATCH;

        /* Say - Print string value of register as a line */
        START_INSTRUCTION(SAY_REG) CALC_DISPATCH(1);
            DEBUG("TRACE - SAY R%llu\n", REG_IDX(1));
            printf("%.*s\n", (int) op1R->string_length, op1R->string_value);
            DISPATCH;

        START_INSTRUCTION(SAY_STRING) CALC_DISPATCH(1);
            DEBUG("TRACE - SAY \"%.*s\"\n",
                  (int)op1S->string_len, op1S->string);
            printf("%.*s\n", (int) op1S->string_len, op1S->string);
            DISPATCH;

        START_INSTRUCTION(SCONCAT_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - SCONCAT R%llu,R%llu,R%llu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            string_sconcat(op1R, op2R, op3R);
            DISPATCH;

        START_INSTRUCTION(CONCAT_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - CONCAT R%llu,R%llu,R%llu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            string_concat(op1R, op2R, op3R);
            DISPATCH;

        START_INSTRUCTION(SCONCAT_REG_REG_STRING) CALC_DISPATCH(3);
            DEBUG("TRACE - SCONCAT R%llu,R%llu,\"%.*s\"\n", REG_IDX(1),
                  REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                  (CONSTSTRING_OP(3))->string);
            string_sconcat_var_const(op1R, op2R, op3S);
            DISPATCH;

        START_INSTRUCTION(CONCAT_REG_REG_STRING) CALC_DISPATCH(3);
            DEBUG("TRACE - CONCAT R%llu,R%llu,\"%.*s\"\n", REG_IDX(1),
                  REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                  (CONSTSTRING_OP(3))->string);
            string_concat_var_const(op1R, op2R, op3S);
            DISPATCH;

        START_INSTRUCTION(SCONCAT_REG_STRING_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - SCONCAT R%llu,\"%.*s\",R%llu\n", REG_IDX(1),
                  (int) (CONSTSTRING_OP(2))->string_len,
                  (CONSTSTRING_OP(2))->string, REG_IDX(3));
            string_sconcat_const_var(op1R, op2S, op3R);
            DISPATCH;

        START_INSTRUCTION(CONCAT_REG_STRING_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - CONCAT R%llu,\"%.*s\",R%llu\n", REG_IDX(1),
                  (int) (CONSTSTRING_OP(2))->string_len,
                  (CONSTSTRING_OP(2))->string, REG_IDX(3));
            string_concat_const_var(op1R, op2S, op3R);
            DISPATCH;

        START_INSTRUCTION(IMULT_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - IMULT R%llu,R%llu,R%llu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            REG_RETURN_INT(op2RI * op3RI);
            DISPATCH;

        START_INSTRUCTION(IMULT_REG_REG_INT) {
            CALC_DISPATCH(3);
            DEBUG("TRACE - IMULT R%llu,R%llu,%llu\n", REG_IDX(1),
                  REG_IDX(2), op3I);
            REG_RETURN_INT(op2RI * op3I);
            DISPATCH;
        }

        START_INSTRUCTION(IADD_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - IADD R%llu,R%llu,R%llu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            REG_RETURN_INT(op2RI + op3RI);
            DISPATCH;

        START_INSTRUCTION(ISUB_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - ISUB R%llu,R%llu,R%llu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            REG_RETURN_INT(op2RI - op3RI);
            DISPATCH;

        START_INSTRUCTION(IADD_REG_REG_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - IADD R%llu,R%llu,%llu\n", REG_IDX(1),
                  REG_IDX(2), op3I);
            REG_RETURN_INT(op2RI + op3I);
            DISPATCH;

        START_INSTRUCTION(CALL_FUNC) CALC_DISPATCH(1);
            /* New stackframe - grabbing procedure object from the caller frame */
            {
                proc_constant *called_function = PROC_OP(1);
                current_frame = frame_f(called_function, 0, current_frame, next_pc,
                                     next_inst, 0);
                DEBUG("TRACE - CALL %s()\n", called_function->name);

                /* Prepare dispatch to procedure as early as possible */
                next_pc = &(current_frame->procedure->binarySpace->binary[called_function->start]);
                CALC_DISPATCH_MANUAL;
                /* No Arguments - so nothing to do */
            }
            /* This gotos the start of the called procedure */
            DISPATCH;

        START_INSTRUCTION(CALL_REG_FUNC) CALC_DISPATCH(2);
            /* Clear target return value register */
            value_zero(op1R);
            /* New stackframe - grabbing procedure object from the caller frame */
            {
                proc_constant *called_function = PROC_OP(2);
                current_frame = frame_f(called_function, 0, current_frame, next_pc,
                                        next_inst, op1R);
                DEBUG("TRACE - CALL R%llu,%s()\n", REG_IDX(1), called_function->name);
                /* Prepare dispatch to procedure as early as possible */
                next_pc = &(current_frame->procedure->binarySpace->binary[called_function->start]);
                CALC_DISPATCH_MANUAL;
                /* No Arguments - so nothing to do */
            }
            /* This gotos the start of the called procedure */
            DISPATCH;

        START_INSTRUCTION(CALL_REG_FUNC_REG) CALC_DISPATCH(3);
            /* New stackframe - grabbing procedure object from the caller frame */
            {
                proc_constant *called_function = PROC_OP(2);
                current_frame =
                        frame_f(called_function, (int) op3R->int_value,
                                current_frame,
                                next_pc, next_inst, op1R);

                DEBUG("TRACE - CALL R%llu,%s,R%llu\n", REG_IDX(1), called_function->name,
                      REG_IDX(3));

                /* Prepare dispatch to procedure as early as possible */
                next_pc = &(current_frame->procedure->binarySpace->binary[called_function->start]);
                CALC_DISPATCH_MANUAL;

                /* Arguments - complex lets never have to change this code! */
                size_t j =
                        current_frame->procedure->binarySpace->globals +
                        current_frame->procedure->locals + 1; /* Callee register index */
                size_t k = (pc + 3)->index + 1; /* Caller register index */
                size_t i;
                for (   i = 0;
                        i < (current_frame->parent->locals[(pc + 3)->index])->int_value;
                        i++, j++, k++) {
                    current_frame->locals[j] = current_frame->parent->locals[k];
                }
            }
            /* This gotos the start of the called procedure */
            DISPATCH;

        START_INSTRUCTION(RET);
            DEBUG("TRACE - RET\n");
            /* Where we return to */
            next_pc = current_frame->return_pc;
            next_inst = current_frame->return_inst;
            /* back to the parents stack frame */
            temp_frame = current_frame;
            current_frame = current_frame->parent;
            free_frame(temp_frame);
            if (!current_frame) {
                DEBUG("TRACE - RET FROM MAIN()\n");
                /* Free Argument Values a1... */
                int i, j;
                for (i = 0, j = temp_frame->procedure->binarySpace->globals +
                                temp_frame->procedure->locals + 1;
                        i < argc;
                        i++, j++) {
                    clear_value(current_frame->baselocals[j]);
                    free(current_frame->baselocals[j]);
                }
                rc = 0;
                goto interprt_finished;
            }
            DISPATCH;

        START_INSTRUCTION(RET_REG);
            DEBUG("TRACE - RET R%llu\n", REG_IDX(1));
            /* Where we return to */
            next_pc = current_frame->return_pc;
            next_inst = current_frame->return_inst;
            /* Set the result register */
            if (current_frame->return_reg) move_value(current_frame->return_reg, op1R);
            /* back to the parents stack frame */
            temp_frame = current_frame;
            current_frame = current_frame->parent;
            if (!current_frame) rc = (int)(temp_frame->locals[(pc + 1)->index])->int_value; /* Exiting - grab the int rc */
            free_frame(temp_frame);
            if (!current_frame) {
                DEBUG("TRACE - RET FROM MAIN()\n");
                /* Free Argument Values a1... */
                int i, j;
                for (i = 0, j = temp_frame->procedure->binarySpace->globals +
                                temp_frame->procedure->locals + 1;
                        i < argc;
                        i++, j++) {
                    clear_value(current_frame->baselocals[j]);
                    free(current_frame->baselocals[j]);
                }
                goto interprt_finished;
            }
            DISPATCH;

        START_INSTRUCTION(RET_INT);
            DEBUG("TRACE - RET %d\n", (int)op1I);
            /* Where we return to */
            next_pc = current_frame->return_pc;
            next_inst = current_frame->return_inst;
            /* Set the result register */
            if (current_frame->return_reg)
                current_frame->return_reg->int_value = op1I;
            /* back to the parents stack frame */
            temp_frame = current_frame;
            current_frame = current_frame->parent;
            free_frame(temp_frame);
            if (!current_frame) {
                DEBUG("TRACE - RET FROM MAIN()\n");
                /* Free Argument Values a1... */
                int i, j;
                for (i = 0, j = temp_frame->procedure->binarySpace->globals +
                                temp_frame->procedure->locals + 1;
                        i < argc;
                        i++, j++) {
                    clear_value(current_frame->baselocals[j]);
                    free(current_frame->baselocals[j]);
                }
                rc = (int) op1I;
                goto interprt_finished;
            }
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  RET_FLOAT                                                        pej 12. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(RET_FLOAT);
            DEBUG("TRACE - RET %g\n", op1F);

            /* Where we return to */
            next_pc = current_frame->return_pc;
            next_inst = current_frame->return_inst;
            /* Set the result register */
            if (current_frame->return_reg)
                current_frame->return_reg->float_value = op1F;
            /* back to the parents stack frame */
            temp_frame = current_frame;
            current_frame = current_frame->parent;
            free_frame(temp_frame);
            if (!current_frame) {
                DEBUG("TRACE - RET FROM MAIN()\n");
                /* Free Argument Values a1... */
                int i, j;
                for (i = 0, j = temp_frame->procedure->binarySpace->globals +
                                temp_frame->procedure->locals + 1;
                        i < argc;
                        i++, j++) {
                    clear_value(current_frame->baselocals[j]);
                    free(current_frame->baselocals[j]);
                }
                rc = 0;
                goto interprt_finished;
            }
            DISPATCH;

            /* ------------------------------------------------------------------------------------
            *  RET_STRING                                                        pej 12. April 2021
            *  -----------------------------------------------------------------------------------
            */
        START_INSTRUCTION(RET_STRING);
            DEBUG("TRACE - RET \"%.*s\"\n", (int)op1S->string_len, op1S->string);

            /* Where we return to */
            next_pc = current_frame->return_pc;
            next_inst = current_frame->return_inst;
            /* Set the result register */
            if (current_frame->return_reg)
                set_const_string(current_frame->return_reg, CONSTSTRING_OP(1));
            /* back to the parents stack frame */
            temp_frame = current_frame;
            current_frame = current_frame->parent;
            free_frame(temp_frame);
            if (!current_frame) {
                DEBUG("TRACE - RET FROM MAIN()\n");
                /* Free Argument Values a1... */
                int i, j;
                for (i = 0, j = temp_frame->procedure->binarySpace->globals +
                                temp_frame->procedure->locals + 1;
                        i < argc;
                        i++, j++) {
                    clear_value(current_frame->baselocals[j]);
                    free(current_frame->baselocals[j]);
                }
                rc = 0;
                goto interprt_finished;
            }
            DISPATCH;

        START_INSTRUCTION(MOVE_REG_REG) CALC_DISPATCH(2); /* Deprecated */
            DEBUG("TRACE - MOVE R%llu,R%llu\n", REG_IDX(1), REG_IDX(2));
            move_value(op1R, op2R);
            DISPATCH;

        START_INSTRUCTION(SWAP_REG_REG) CALC_DISPATCH(2); /* Deprecated */
            DEBUG("TRACE - SWAP R%llu,R%llu\n", REG_IDX(1), REG_IDX(2));
            {
                value *v_temp;
                v_temp = op1R;
                op1R = op2R;
                op2R = v_temp;
            }
            DISPATCH;

        START_INSTRUCTION(DEC0) CALC_DISPATCH(0);
            /* TODO This is really idec0 - i.e. it does not prime the int */
            DEBUG("TRACE - DEC0\n");
            (current_frame->locals[0]->int_value)--;
            DISPATCH;

            /* ------------------------------------------------------------------------------------
         *  DEC1   R1--                                                       pej 7. April 2021
         *  -----------------------------------------------------------------------------------
         */
        START_INSTRUCTION(DEC1) CALC_DISPATCH(0);
            /* TODO This is really idec1 - i.e. it does not prime the int */
            DEBUG("TRACE - DEC1\n");
            (current_frame->locals[1]->int_value)--;
            DISPATCH;

            /* ------------------------------------------------------------------------------------
            *  DEC2   op2R--                                                       pej 7. April 2021
            *  -----------------------------------------------------------------------------------
            */
        START_INSTRUCTION(DEC2) CALC_DISPATCH(0);
            /* TODO This is really idec2 - i.e. it does not prime the int */
            DEBUG("TRACE - DEC2\n");
            (current_frame->locals[2]->int_value)--;
            DISPATCH;

        START_INSTRUCTION(DEC_REG) CALC_DISPATCH(1);
            /* TODO This is really idec reg - i.e. it does not prime the int */
            DEBUG("TRACE - DEC R%llu\n", REG_IDX(1));
            (current_frame->locals[REG_IDX(1)]->int_value)--;
            DISPATCH;

        START_INSTRUCTION(BR_ID);
            DEBUG("TRACE - BR 0x%x\n", (unsigned int)REG_IDX(1));
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL;
            DISPATCH;

            /* For these we optimise for condition to NOT be met because in a loop
             * these ae used to jump out of the loop when the end condition it met
             * (and every little bit helps to improve performance!)
             */

        START_INSTRUCTION(BRT_ID_REG) CALC_DISPATCH(2); /* i.e. if the condition is not met - this helps the
                                the real CPUs branch prediction (in theory) */
            DEBUG("TRACE - BRT 0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2));
            if (op2RI) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL;
            }
            DISPATCH;

        START_INSTRUCTION(BRF_ID_REG) CALC_DISPATCH(2); /* i.e. if the condition is not met - this helps the
                                  the real CPUs branch prediction (in theory) */
            DEBUG("TRACE - BRF 0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2));
            if (!(op2RI)) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL;
            }
            DISPATCH;

        START_INSTRUCTION(BRTF_ID_ID_REG)
            DEBUG("TRACE - BRTF 0x%x,0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            if (op3RI) next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            else next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(2);
            CALC_DISPATCH_MANUAL;
            DISPATCH;


        START_INSTRUCTION(TIME_REG) CALC_DISPATCH(1);
            DEBUG("TRACE - TIME R%d\n", (int)REG_IDX(1));
            {
                struct timeval tv;
                tzset();
                gettimeofday(&tv, NULL);
                REG_RETURN_INT(tv.tv_sec - timezone);
            }
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  XTIME return time properties                                  pej 02. December 2021
 * ------------------------------------------------------------------------------------
 */
        START_INSTRUCTION(XTIME_REG_STRING) CALC_DISPATCH(2);
        DEBUG("TRACE - XTIME R%d\n", (int)REG_IDX(1),(CONSTSTRING_OP(2))->string);

            tzset();
            switch ((CONSTSTRING_OP(2))->string[0]) {
                case 'Z':
                    tzset();
                    op1R->int_value  = timezone;
                    break;
                case 'T':  op1R->int_value  = clock(); break;
                case 'C':  op1R->int_value  = CLOCKS_PER_SEC; break;
                case 'N':  {
                     prep_string_buffer(op1R,2*SMALLEST_STRING_BUFFER_LENGTH); // Large enough for both time zone names
                     op1R->string_length = snprintf(op1R->string_value,2*SMALLEST_STRING_BUFFER_LENGTH,"%s;%s",tzname[0],tzname[1]);
                     op1R->string_pos = 0;
                     break;
                }
                case 'U':  {
                     time_t ctime;
                     rxinteger tm;
                     struct timeval tv;
                     struct tm *tmdata;
                     ctime = time(NULL);
                     tmdata = localtime(&ctime);
                     tzset();
                     tm=((tmdata->tm_hour * 3600) + (tmdata->tm_min  * 60) + (tmdata->tm_sec))+ timezone;
                     gettimeofday(&tv, NULL);
                     op1R->int_value = tm*1000000+tv.tv_usec;
                     break;
                }
            }
            DISPATCH;

/* ---------------------------------------------------------------------------------
 *  MTIME get time of the day in microseconds                      pej 31. October 2021
 * ------------------------------------------------------------------------------------
 */
        START_INSTRUCTION(MTIME_REG) CALC_DISPATCH(1);
            DEBUG("TRACE - MTIME R%d\n", (int)REG_IDX(1));
            {
                rxinteger tm;
                struct timeval tv;
                //struct timezone tz;
                time_t	ctime;
                struct tm *tmdata;

                ctime = time(NULL);
                tmdata = localtime(&ctime);
                tm =
                        ((tmdata->tm_hour * 3600) + (tmdata->tm_min * 60) +
                        (tmdata->tm_sec));
                gettimeofday(&tv, NULL);
                REG_RETURN_INT(tm * 1000000 + tv.tv_usec);
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  TRIMR  Trim right                                                 pej 7. April 2021
 * ------------------------------------------------------------------------------------
 */
        START_INSTRUCTION(TRIMR_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - TRIMR (DEPRECATED) R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            {
                int i = op1R->string_length - 1;
                while (i >= 0 && op1R->string_value[i] == ' ') {
                    op1R->string_value[i] = '\0';
                    i--;
                }
                op1R->string_length = i + 1;
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  TRIML  Trim left                                                  pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(TRIML_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - TRIML (DEPRECATED) R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            /* TODO - UTF etc */
            {
                int j = op1R->string_length - 1;
                int i = 0;
                while (i <= j && op1R->string_value[i] == ' ') i++;

                if (i >= j) {
                    op1R->string_length = 0;
                    op1R->string_value[0] = '\0';
                } else {
                    op1R->string_length = op1R->string_length - i;
                    memcpy(op1R->string_value, op1R->string_value + i,
                           op1R->string_length);
                    op1R->string_value[op1R->string_length] = '\0';
                }
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  INC0   R0++                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INC0) CALC_DISPATCH(0);
            DEBUG("TRACE - INC0\n");
            REG_VAL(0)->int_value++;
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  INC1   R1++                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INC1) CALC_DISPATCH(0);
            DEBUG("TRACE - INC1\n");
            REG_VAL(1)->int_value++;
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  INC2   op2R++                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INC2) CALC_DISPATCH(0);
            DEBUG("TRACE - INC2\n");
            REG_VAL(2)->int_value++;
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ISEX   op1 = -op1  decimal                                    pej 2. September 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISEX_REG) CALC_DISPATCH(1);
            DEBUG("TRACE - INC R%llu\n", REG_IDX(1));
            (current_frame->locals[REG_IDX(1)]->int_value)=0-(current_frame->locals[REG_IDX(1)]->int_value);
        DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FSEX   op1 = -op1  decimal                                    pej 2. September 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FSEX_REG) CALC_DISPATCH(1);
            DEBUG("TRACE - INC R%llu\n", REG_IDX(1));
            (current_frame->locals[REG_IDX(1)]->float_value)=0-(current_frame->locals[REG_IDX(1)]->float_value);
        DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ISUB_REG_REG_INT: Integer Subtract (op1=op2-op3)               pej 8. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISUB_REG_REG_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - ISUB R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI - op3I);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ISUB_REG_INT_REG: Integer Subtract (op1=op2-op3)               pej 8. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISUB_REG_INT_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - ISUB R%d,%d,R%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I - op3RI);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  AND_REG_REG_REG  Int Logical AND op1=(op2 && op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(AND_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - AND R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI && op3RI);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  OR_REG_REG_REG  Int Logical OR op1=(op2 || op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(OR_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - OR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI || op3RI);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  NOT_REG_REG  Int Logical NOT op1=!op2
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(NOT_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - NOT R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            if (op2RI) REG_RETURN_INT(0)
            else REG_RETURN_INT(1);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IEQ_REG_REG_REG  Int Equals op1=(op2==op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IEQ_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - IEQ R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI == op3RI);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IEQ_REG_REG_INT  Int Equals op1=(op2==op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IEQ_REG_REG_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - IEQ R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI == op3I);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  INE_REG_REG_REG  Int Equals op1=(op2!=op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INE_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - INE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI != op3RI);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  INE_REG_REG_INT  Int Equals op1=(op2!=op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INE_REG_REG_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - INE R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI != op3I);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IGT_REG_REG_REG  Int Greater than op1=(op2>op3)                      pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IGT_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - IGT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI > op3RI);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IGT_REG_REG_INT  Int Greater than op1=(op2>op3)                      pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IGT_REG_REG_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - IGT R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI > op3I);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IGT_REG_INT_REG  Int Greater than op1=(op2>op3)                      pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IGT_REG_INT_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - IGT R%d,%d,r%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I > op3RI);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ILT_REG_REG_REG  Int Less than op1=(op2<op3)                         pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ILT_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - ILT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI < op3RI);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ILT_REG_REG_INT  Int Less than op1=(op2<op3)                         pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ILT_REG_REG_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - ILT R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI < op3I);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ILT_REG_INT_REG  Int Less than op1=(op2<op3)                         pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ILT_REG_INT_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - ILT R%d,%d,r%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I < op3RI);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IGTE_REG_REG_REG  Int Greater Equal than op1=(op2>=op3)              pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IGTE_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - IGTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI >= op3RI);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IGTE_REG_REG_INT  Int Greater Equal than op1=(op2>=op3)              pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IGTE_REG_REG_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - IGTE R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI >= op3I);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IGTE_REG_INT_REG  Int Greater Equal than op1=(op2>=op3)              pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IGTE_REG_INT_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - IGTE R%d,%d,r%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I >= op3RI);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ILTE_REG_REG_REG  Int Less Equal than op1=(op2<=op3)                 pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ILTE_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - ILTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI <= op3RI);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ILTE_REG_REG_INT  Int Less Equal than op1=(op2<=op3)                 pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ILTE_REG_REG_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - ILTE R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI <= op3I);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ILTE_REG_INT_REG  Int Less Equal than op1=(op2<=op3)                 pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ILTE_REG_INT_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - ILTE R%d,%d,r%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I <= op3RI);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FEQ_REG_REG_REG  Float Equals op1=(op2==op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FEQ_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - FEQ R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF == op3RF);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FEQ_REG_REG_FLOAT  Float Equals op1=(op2==op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FEQ_REG_REG_FLOAT) CALC_DISPATCH(3);
            DEBUG("TRACE - FEQ R%d,R%d,%g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF == op3F);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FNE_REG_REG_REG  Float Not Equals op1=(op2!=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FNE_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - FNE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF != op3RF);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FNE_REG_REG_FLOAT  Float Not Equals op1=(op2!=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FNE_REG_REG_FLOAT) CALC_DISPATCH(3);
            DEBUG("TRACE - FNE R%d,R%d,%g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF != op3F);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FGT_REG_REG_REG  Float Greater than op1=(op2>op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FGT_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - FGT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF > op3RF);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FGT_REG_REG_FLOAT  Float Greater than op1=(op2>op3)
 *
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FGT_REG_REG_FLOAT) CALC_DISPATCH(3);
            DEBUG("TRACE - FGT R%d,R%d,%g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF > op3F);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FGT_REG_FLOAT_REG  Float Greater than op1=(op2>op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FGT_REG_FLOAT_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - FGT R%d,%g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_INT(op2F > op3RF);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FLT_REG_REG_REG  Float Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FLT_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - FLT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF < op3RF);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FLT_REG_REG_FLOAT  Float Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FLT_REG_REG_FLOAT) CALC_DISPATCH(3);
            DEBUG("TRACE - FLT R%d,R%d,%g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF < op3F);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FLT_REG_FLOAT_REG  Float Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FLT_REG_FLOAT_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - FLT R%d,%g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_INT(op2F < op3RF);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FGTE_REG_REG_REG  Float Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FGTE_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - FGTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF >= op3RF);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FGTE_REG_REG_FLOAT  Float Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FGTE_REG_REG_FLOAT) CALC_DISPATCH(3);
            DEBUG("TRACE - FGTE R%d,R%d,%g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF >= op3F);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FGTE_REG_FLOAT_REG  Float Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FGTE_REG_FLOAT_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - FGTE R%d,%g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_INT(op2F >= op3RF);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FLTE_REG_REG_REG  Float Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FLTE_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - FLTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF <= op3RF);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FLTE_REG_REG_FLOAT  Float Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FLTE_REG_REG_FLOAT) CALC_DISPATCH(3);
            DEBUG("TRACE - FLTE R%d,R%d,%g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF <= op3F);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FLTE_REG_FLOAT_REG  Float Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FLTE_REG_FLOAT_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - FLTE R%d,%g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_INT(op2F <= op3RF);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SEQ_REG_REG_REG  String Equals op1=(op2==op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SEQ_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - SEQ R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(!string_cmp_value(op2R, op3R));
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SEQ_REG_REG_STRING String Equals op1=(op2==op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SEQ_REG_REG_STRING) CALC_DISPATCH(3);
            DEBUG("TRACE - SEQ R%llu,R%llu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);
            REG_RETURN_INT(!string_cmp_const(op2R, op3S));
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  RSEQ_REG_REG_REG  String Equals op1=(op2=op3) non strict REXX comparison  pej 29. Nov 2021
 *  -----------------------------------------------------------------------------------
*/
        START_INSTRUCTION(RSEQ_REG_REG_REG) CALC_DISPATCH(3);
        {
            DEBUG("TRACE - RSEQ R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            int ch;
            int p1, p2;
            int len1, len2;

            GETSTRLEN(len1, op2R);
            GETSTRLEN(len2, op3R);

            // step 1 find last not blank character
            for (p1 = len1 - 1; p1 >= 0; p1--, len1--) {
                GETSTRCHAR(ch, op2R, p1);
                if (ch != ' ') break;
            }
            for (p2 = len2 - 1; p2 >= 0; p2--, len2--) {
                GETSTRCHAR(ch, op3R, p2);
                if (ch != ' ') break;
            }

            // step 2 find first non blank
            for (p1 = 0; p1 < len1; p1++) {
                GETSTRCHAR(ch, op2R, p1);
                if (ch != ' ') break;
            }
            for (p2 = 0; p2 < len2; p2++) {
                GETSTRCHAR(ch, op3R, p2);
                if (ch != ' ') break;
            }
            if (len1 - p1 != len2 - p2) REG_RETURN_INT(0)
            else {
                if (string_cmp(op2R->string_value + p1, len1 - p1,
                           op3R->string_value + p2, len2 - p2) == 0)
                    REG_RETURN_INT(1)
                else REG_RETURN_INT(0)
            }
        }
        DISPATCH;

/* ------------------------------------------------------------------------------------
 *  RSEQ_REG_REG_STRING String Equals op1=(op2=op3)  non strict REXX comparison
 *  TODO !!! not yet implemented !!!
 *  -----------------------------------------------------------------------------------
*/
        START_INSTRUCTION(RSEQ_REG_REG_STRING) CALC_DISPATCH(3);
            DEBUG("TRACE - RSEQ R%llu,R%llu,\"%.*s\"\n", REG_IDX(1),
                  REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                  (CONSTSTRING_OP(3))->string);
            REG_RETURN_INT(0);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SNE_REG_REG_REG  String Not Equals op1=(op2!=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SNE_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - SNE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) != 0);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SNE_REG_REG_STRING  String Not Equals op1=(op2!=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SNE_REG_REG_STRING) CALC_DISPATCH(3);
            DEBUG("TRACE - SNE R%llu,R%llu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);
            REG_RETURN_INT(string_cmp_const(op2R, op3S) != 0);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SGT_REG_REG_REG  String Greater than op1=(op2>op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SGT_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - SGT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) > 0);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SGT_REG_REG_STRING  String Greater than op1=(op2>op3)
 *
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SGT_REG_REG_STRING) CALC_DISPATCH(3);
            DEBUG("TRACE - SGT R%llu,R%llu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);

            REG_RETURN_INT(string_cmp_const(op2R, op3S) > 0);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SGT_REG_STRING_REG  String Greater than op1=(op2>op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SGT_REG_STRING_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - SGT R%llu,\"%.*s\",R%llu\n", REG_IDX(1),
                (int) (CONSTSTRING_OP(2))->string_len,
                (CONSTSTRING_OP(2))->string, REG_IDX(3));
            REG_RETURN_INT(string_cmp_const(op3R, op2S) < 0);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SLT_REG_REG_REG  String Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SLT_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - SLT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) < 0);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SLT_REG_REG_STRING  String Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SLT_REG_REG_STRING) CALC_DISPATCH(3);
            DEBUG("TRACE - SLT R%llu,R%llu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);

            REG_RETURN_INT(string_cmp_const(op2R, op3S) < 0);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SLT_REG_STRING_REG  String Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SLT_REG_STRING_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - SLT R%llu,\"%.*s\",R%llu\n", REG_IDX(1),
                (int) (CONSTSTRING_OP(2))->string_len,
                (CONSTSTRING_OP(2))->string, REG_IDX(3));
            REG_RETURN_INT(string_cmp_const(op3R, op2S) > 0);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SGTE_REG_REG_REG  String Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SGTE_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - SGTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) >= 0);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SGTE_REG_REG_STRING  String Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SGTE_REG_REG_STRING) CALC_DISPATCH(3);
            DEBUG("TRACE - SGTE R%llu,R%llu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);

            REG_RETURN_INT(string_cmp_const(op2R, op3S) >= 0);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SGTE_REG_STRING_REG  String Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SGTE_REG_STRING_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - SGTE R%llu,\"%.*s\",R%llu\n", REG_IDX(1),
                (int) (CONSTSTRING_OP(2))->string_len,
                (CONSTSTRING_OP(2))->string, REG_IDX(3));
            REG_RETURN_INT(string_cmp_const(op3R, op2S) <= 0);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SLTE_REG_REG_REG  String Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SLTE_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - SLTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) <= 0);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SLTE_REG_REG_STRING  String Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SLTE_REG_REG_STRING) CALC_DISPATCH(3);
            DEBUG("TRACE - SLTE R%llu,R%llu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);
            REG_RETURN_INT(string_cmp_const(op2R, op3S) <= 0);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SLTE_REG_STRING_REG  String Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SLTE_REG_STRING_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - SLTE R%llu,\"%.*s\",R%llu\n", REG_IDX(1),
                (int) (CONSTSTRING_OP(2))->string_len,
                (CONSTSTRING_OP(2))->string, REG_IDX(3));
            REG_RETURN_INT(string_cmp_const(op3R, op2S) >= 0);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  COPY_REG_REG  Copy op2 to op1
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(COPY_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - COPY R%llu,R%llu\n", REG_IDX(1), REG_IDX(2));
            copy_value(op1R, op2R);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SCOPY_REG_REG  Copy String op2 to op1
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SCOPY_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - SCOPY R%llu,R%llu\n", REG_IDX(1), REG_IDX(2));
            copy_string_value(op1R, op2R);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ICOPY_REG_REG  Copy Integer op2 to op1
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ICOPY_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - ICOPY R%llu,R%llu\n", REG_IDX(1), REG_IDX(2));
            op1R->int_value = op2R->int_value;
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FCOPY_REG_REG  Copy Float op2 to op1
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FCOPY_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - FCOPY R%llu,R%llu\n", REG_IDX(1), REG_IDX(2));
            op1R->float_value = op2R->float_value;
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  INC_REG  Increment Int (op1++)                                      pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INC_REG) CALC_DISPATCH(1);
            DEBUG("TRACE - INC R%llu\n", REG_IDX(1));
            (current_frame->locals[REG_IDX(1)]->int_value)++;
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IDIV_REG_REG_INT  Integer Divide (op1=op2/op3)                      pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IDIV_REG_REG_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - IDIV R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI / op3I);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IDIV_REG_INT_REG  Integer Divide (op1=op2/op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IDIV_REG_INT_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - IDIV R%d,%d,R%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I / op3RI);
            DISPATCH;

/* -----------------------------------------------------------------------------------
 *  IDIV_REG_REG_REG  Integer Divide (op1=op2/op3)                      pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IDIV_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - IDIV R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI / op3RI);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IMOD_REG_REG_INT  Integer Modulo (op1=op2 & op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IMOD_REG_REG_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - IMOD R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI % op3I);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IMOD_REG_INT_REG  Integer Modulo (op1=op2 % op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IMOD_REG_INT_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - IMOD R%d,%d,R%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I % op3RI);
            DISPATCH;

/* -----------------------------------------------------------------------------------
 *  IMOD_REG_REG_REG  Integer Modulo (op1=op2 % op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IMOD_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - IMOD R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI % op3RI);
            DISPATCH;
 /* ------------------------------------------------------------------------------------
  *  IOR_REG_REG_REG bitwise OR (op1=op2|op3)                           pej 17 Oct 2021
  *  -----------------------------------------------------------------------------------
  */
        START_INSTRUCTION(IOR_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - IOR R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI | op3RI);
            DISPATCH;
 /* -----------------------------------------------------------------------------------
  *  IOR_REG_REG_INT  bitwise OR (op1=op2|op3)                          pej 17 Oct 2021
  *  ----------------------------------------------------------------------------------
  */
        START_INSTRUCTION(IOR_REG_REG_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - IOR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI | op3I);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IAND_REG_REG_INT  bitwise AND (op1=op2&op3)                         pej 17 Oct 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IAND_REG_REG_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - IAND R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI & op3I);
            DISPATCH;
/* -----------------------------------------------------------------------------------
 *  IAND_REG_REG_REG  bitwise AND (op1=op2&op3)                        pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IAND_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - IAND R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI & op3RI);
            DISPATCH;

/* -----------------------------------------------------------------------------------
 *  IXOR_REG_REG_REG  bitwise XOR (op1=op2^op3)                        pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IXOR_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - IXOR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI ^ op3RI);
            DISPATCH;

/* -----------------------------------------------------------------------------------
 *  IXOR_REG_REG_INT  bitwise XOR (op1=op2^op3)                        pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IXOR_REG_REG_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - IXOR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI ^ op3I);
            DISPATCH;

/* -----------------------------------------------------------------------------------
 *  ISHL_REG_REG_REG  bitwise shift logical left (op1=op2<<op3)         pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISHL_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - ISHL R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI << op3RI);
            DISPATCH;

/* -----------------------------------------------------------------------------------
 *  ISHL_REG_REG_INT  bitwise shift logical left (op1=op2<<op3)         pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISHL_REG_REG_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - ISHL R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI << op3I);
            DISPATCH;
/* -----------------------------------------------------------------------------------
 *  ISHR_REG_REG_REG  bitwise shift logical right (op1=op2>>op3)       pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISHR_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - ISHR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI >> op3RI);
            DISPATCH;

/* -----------------------------------------------------------------------------------
 *  ISHR_REG_REG_INT  bitwise shift logical right (op1=op2>>op3)       pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISHR_REG_REG_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - IXSHL R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI >> op3I);
            DISPATCH;
/* -----------------------------------------------------------------------------------
 *  INOT_REG_REG  inverts all bits of an integer (op1=~op2)            pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INOT_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - INOT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            REG_RETURN_INT(~op2RI);
            DISPATCH;

/* -----------------------------------------------------------------------------------
 *  INOT_REG_INT  inverts all bits of an integer (op1=~op2)            pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INOT_REG_INT) CALC_DISPATCH(2);
            DEBUG("TRACE - INOT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)op2I);
            REG_RETURN_INT(~op2I);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SAY_INT  Say op1                                                    pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SAY_INT) CALC_DISPATCH(1);
            DEBUG("TRACE - SAY %d\n", (int)op1I);
#ifdef __32BIT__
            printf("%ld\n", op1I);
#else
            printf("%lld\n", op1I);
#endif
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SAY_CHAR  Say op1                                                   pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SAY_CHAR) CALC_DISPATCH(1);
            DEBUG("TRACE - SAY \'%c\'\n", (pc + (1))->cconst);
            printf("%c\n", (pc + (1))->cconst);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  SAY_FLOAT  Say op1                                                  pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SAY_FLOAT) CALC_DISPATCH(1);
            DEBUG("TRACE - SAY %g\n", op1F);
            printf("%g\n", op1F);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  LOAD_REG_FLOAT  Load op1 with op2                                   pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(LOAD_REG_FLOAT) CALC_DISPATCH(2);
            DEBUG("TRACE - LOAD R%d,%g\n",(int)REG_IDX(1),op2F);
            REG_RETURN_FLOAT(op2F);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FADD_REG_REG_REG  Float Add (op1=op2+op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FADD_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - FADD R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2RF + op3RF);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FSUB_REG_REG_REG  Float Sub (op1=op2-op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

        START_INSTRUCTION(FSUB_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - FSUB R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2RF - op3RF);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FDIV_REG_REG_REG  Float Div (op1=op2/op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FDIV_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - FDIV R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2RF / op3RF);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FMULT_REG_REG_REG  Float Mult (op1=op2/op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FMULT_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - FMULT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2RF * op3RF);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FADD_REG_REG_FLOAT  Float Add (op1=op2+op3)                          pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FADD_REG_REG_FLOAT) CALC_DISPATCH(3);
            DEBUG("TRACE - FADD R%d,R%d,%g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_FLOAT(op2RF + op3F);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FSUB_REG_REG_FLOAT  Float Sub (op1=op2-op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FSUB_REG_REG_FLOAT) CALC_DISPATCH(3);
            DEBUG("TRACE - FSUB R%d,R%d,%g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_FLOAT(op2RF - op3F);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FDIV_REG_REG_FLOAT  Float Div (op1=op2/op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FDIV_REG_REG_FLOAT) CALC_DISPATCH(3);
            DEBUG("TRACE - FDIV R%d,R%d,%g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_FLOAT(op2RF / op3F);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FMULT_REG_REG_FLOAT  Float Mult (op1=op2/op3)                       pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FMULT_REG_REG_FLOAT) CALC_DISPATCH(3);
            DEBUG("TRACE - FMULT R%d,R%d,%g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_FLOAT(op2RF * op3F);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FSUB_REG_FLOAT_REG  Float Sub (op1=op2-op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FSUB_REG_FLOAT_REG) CALC_DISPATCH(3);
        DEBUG("TRACE - FSUB R%d,%g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2F - op3RF);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FDIV_REG_FLOAT_REG  Float Div (op1=op2/op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FDIV_REG_FLOAT_REG) CALC_DISPATCH(3);
        DEBUG("TRACE - FDIV R%d,%g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2F / op3RF);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IPOW_REG_REG_REG  op1=op2**op2w Integer operation                pej 22 August 2021
 *  -----------------------------------------------------------------------------------
 */
            START_INSTRUCTION(IPOW_REG_REG_REG) CALC_DISPATCH(3);
            {
                DEBUG("TRACE - DIVF R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2),
                    (int) REG_IDX(3));
                rxinteger  i1 = 1;
                rxinteger  i2 = op2RI;
                rxinteger  i3 = op3RI;
                while (i3 != 0) {
                    if ((i3 & 1) == 1) i1 *= i2;
                    i3 >>= 1;
                    i2 *= i2;
                }
                REG_RETURN_INT(i1);
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IPOW_REG_REG_INT  op1=op2**op2w Integer operationn               pej 22 August 2021
 *  -----------------------------------------------------------------------------------
 */
            START_INSTRUCTION(IPOW_REG_REG_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - DIVF R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
                rxinteger i1 = 1;
                rxinteger i2 = op2RI;
                rxinteger i3 = op3I;
                while (i3 != 0) {
                    if ((i3 & 1) == 1) i1 *= i2;
                    i3 >>= 1;
                    i2 *= i2;
                }
                REG_RETURN_INT(i1);
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  AMAP_REG_REG  Link r1 to Arg[r2]              TODO Rename to ALINK
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(AMAP_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - AMAP R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            op1R = current_frame->locals[op2RI +
                                         current_frame->procedure->binarySpace->globals +
                                         current_frame->procedure->locals];
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  AMAP_REG_INT  Link r1 to Arg[i2]              TODO Rename to ALINK
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(AMAP_REG_INT) CALC_DISPATCH(2);
            DEBUG("TRACE - AMAP R%d,%d\n", (int)REG_IDX(1), (int)op2I);
            op1R = current_frame->locals[op2I +
                                         current_frame->procedure->binarySpace->globals +
                                         current_frame->procedure->locals];

            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  ITOS_REG  Set register string value from its int value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(ITOS_REG) CALC_DISPATCH(1);
            DEBUG("TRACE - ITOS R%llu\n", REG_IDX(1));
            string_from_int(op1R);
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FTOS_REG  Set register string value from its float value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(FTOS_REG) CALC_DISPATCH(1);
            DEBUG("TRACE - FTOS R%llu\n", REG_IDX(1));
            string_from_float(op1R);
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ITOF_REG  Set register float value from its int value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(ITOF_REG) CALC_DISPATCH(1);
            DEBUG("TRACE - ITOF R%llu\n", REG_IDX(1));
            op1R->float_value = op1R->int_value;
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FTOI_REG  Set register int value from its float value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(FTOI_REG) CALC_DISPATCH(1);
            DEBUG("TRACE - FTOI R%llu\n", REG_IDX(1));
            int_from_float(op1R);
            if (op1R->float_value != (double)op1R->int_value) {
                goto converror;
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FTOB_REG  Set register boolean (int set to 1 or 0) value from its float value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(FTOB_REG) CALC_DISPATCH(1);
            DEBUG("TRACE - FTOB R%llu\n", REG_IDX(1));
            int_from_float(op1R);

            if (op1R->float_value) op1R->int_value = 1;
            else op1R->int_value = 0;
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  STOI_REG  Set register int value from its string value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(STOI_REG) CALC_DISPATCH(1);
            DEBUG("TRACE - STOI R%llu\n", REG_IDX(1));
            /* Convert a string to a integer - returns 1 on error */
            if (string2integer(&op1R->int_value, op1R->string_value, op1R->string_length)) {
                goto converror;
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  STOF_REG  Set register float value from its string value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(STOF_REG) CALC_DISPATCH(1);
            DEBUG("TRACE - STOF R%llu\n", REG_IDX(1));
            /* Convert a string to a float - returns 1 on error */
            if (string2float(&op1R->float_value, op1R->string_value, op1R->string_length)) {
                goto converror;
            }
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FFORMAT_REG_REG_REG  Set string from float use format string   pej 3. November 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FFORMAT_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - FFORMAT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            prep_string_buffer(op1R,SMALLEST_STRING_BUFFER_LENGTH); // Large enough for a float
            op3R->string_value[op3R->string_length]='\0';    // terminate format string explicitly, rexx vars aren't!
            op1R->string_length = snprintf(op1R->string_value,SMALLEST_STRING_BUFFER_LENGTH,op3R->string_value,op2R->float_value);
            op1R->string_pos = 0;
  #ifndef NUTF8
            op1R->string_char_pos = 0;
            op1R->string_chars = op1R->string_length;
  #endif
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  STRLOWER_REG_REG  translate string into lower case string              pej 23.10.21
 *  -----------------------------------------------------------------------------------
 */
// TODO: what to do if there is a length change of chars during translation
            START_INSTRUCTION(STRLOWER_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - STRLOWER R%llu\n", (int)REG_IDX(1), (int)REG_IDX(2));
            {
                set_value_string(op1R, op2R);
#ifdef NUTF8
                char c;
                for (c = op1R->string_value; *c; ++c) *c = (char)tolower(*c);
#else
                utf8lwr(op1R->string_value);
#endif
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  STRUPPER_REG_REG  translate string into upper case string              pej 23.10.21
 *  -----------------------------------------------------------------------------------
 */
// TODO: what to do if there is a length change of chars during translation
            START_INSTRUCTION(STRUPPER_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - STRUPPER R%llu\n", (int)REG_IDX(1), (int)REG_IDX(2));
            {
                set_value_string(op1R, op2R);
#ifdef NUTF8
                char c;
                for (c = op1R->string_value ; *c; ++c) *c = (char)toupper(*c);
#else
                utf8upr(op1R->string_value);
#endif
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  STRCHAR_REG_REG_REG  String to Int op1 = op2[op3]                   pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(STRCHAR_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - STRCHAR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
#ifndef NUTF8
                int result;
                string_set_byte_pos(op2R, op3R->int_value);
                utf8codepoint(op2R->string_value + op2R->string_pos, &result);
                REG_RETURN_INT(result);
#else
                REG_RETURN_INT(v2->string_value[op3R->int_value]);
#endif
            }
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  HEXCHAR_REG_REG_REG  op1 = hex(op2[op3])                       pej 04 November 2021
 *  -----------------------------------------------------------------------------------
 */
            START_INSTRUCTION(HEXCHAR_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - HEXCHAR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
                static const char hexconst[] = {'0','1','2','3','4','5','6','7','8','9','a', 'b', 'c', 'd', 'e', 'f','A', 'B', 'C', 'D', 'E', 'f'};
                int ch;
#ifndef NUTF8
                string_set_byte_pos(op2R, op3R->int_value);
                utf8codepoint(op2R->string_value + op2R->string_pos, &ch);
#else
                ch=v2->string_value[v3->int_value];
#endif
                rxinteger lhs = (ch >> 4) & 15;   // extract left hand side of value
                rxinteger rhs = (ch) & 15; // extract right hand side of value
                op1R->string_value[0] = hexconst[lhs];    // set first character of hex value
                op1R->string_value[1] = hexconst[rhs];    // set first character of hex value
                op1R->string_value[2] = '\0';            // set end of string, just to be safe
                PUTSTRLEN(op1R, 2);                  // hex length is 2
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  POSCHAR_REG_REG_REG  op1 position of op3 in op2                pej 05 November 2021
 *  -----------------------------------------------------------------------------------
 */
            START_INSTRUCTION(POSCHAR_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - POSCHAR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
                rxinteger result = -1, i;
                int ch;

                for (i = 0; i < op2R->string_length; i++) {
#ifndef NUTF8
                    string_set_byte_pos(op2R, i);
                    utf8codepoint(op2R->string_value + op2R->string_pos, &ch);
#else
                    ch=op2R->string_value[i];
#endif
                    if (ch == op3R->int_value) {
                        result = i;
                        break;
                    }
                }
                REG_RETURN_INT(result);
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  BGT_ID_REG_REG  if op2>op3 goto op1                           pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BGT_ID_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - BGT R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            if (current_frame->locals[REG_IDX(2)]->int_value > current_frame->locals[REG_IDX(3)]->int_value) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL;
            }
        DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BGT_ID_REG_INT  if op2>op3 goto op1                           pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BGT_ID_REG_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - BGT R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            if (current_frame->locals[REG_IDX(2)]->int_value > op3I) {
               next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
               CALC_DISPATCH_MANUAL;
            }
        DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BGE_ID_REG_REG  if op2>=op3 goto op1                          pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BGE_ID_REG_REG) CALC_DISPATCH(3);
       DEBUG("TRACE - BGE R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
       if (current_frame->locals[REG_IDX(2)]->int_value >= current_frame->locals[REG_IDX(3)]->int_value) {
          next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
          CALC_DISPATCH_MANUAL;
       }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BGE_ID_REG_INT  if op2>=op3 goto op1                         pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BGE_ID_REG_INT) CALC_DISPATCH(3);
        DEBUG("TRACE - BGE R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value >= op3I) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL;
        }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BLT_ID_REG_REG  if op2<op3 goto op1                           pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BLT_ID_REG_REG) CALC_DISPATCH(3);
        DEBUG("TRACE - BLT R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value < current_frame->locals[REG_IDX(3)]->int_value) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL;
        }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BLT_ID_REG_INT  if op2<op3 goto op1                           pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BLT_ID_REG_INT) CALC_DISPATCH(3);
        DEBUG("TRACE - BGT R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value < op3I) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL;
        }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BLE_ID_REG_REG  if op2<=op3 goto op1                          pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BLE_ID_REG_REG) CALC_DISPATCH(3);
        DEBUG("TRACE - BGE R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value <= current_frame->locals[REG_IDX(3)]->int_value) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL;
        }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BLE_ID_REG_INT  if op2<=op3 goto op1                          pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BLE_ID_REG_INT) CALC_DISPATCH(3);
        DEBUG("TRACE - BGE R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value <= op3I) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL;
        }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BNE_ID_REG_INT  if op2!=op3 goto op1                          pej 14 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BNE_ID_REG_REG) CALC_DISPATCH(3);
        DEBUG("TRACE - BGE R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value != current_frame->locals[REG_IDX(3)]->int_value) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL;
        }
    DISPATCH;

/* ------------------------------------------------------------------------------------
 *  BNE_ID_REG_INT  if op2!=op3 goto op1                          pej 14 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BNE_ID_REG_INT) CALC_DISPATCH(3);
        DEBUG("TRACE - BGE R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value != op3I) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL;
        }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BEQ_ID_REG_INT  if op2=op3 goto op1                           pej 14 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BEQ_ID_REG_REG) CALC_DISPATCH(3);
        DEBUG("TRACE - BGE R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value == current_frame->locals[REG_IDX(3)]->int_value) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL;
        }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BEQ_ID_REG_INT  if op2=op3 goto op1                           pej 14 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BEQ_ID_REG_INT) CALC_DISPATCH(3);
        DEBUG("TRACE - BGE 0x%x,R%d,%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
        if (current_frame->locals[REG_IDX(2)]->int_value == op3I) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL;
        }
    DISPATCH;
 /* ------------------------------------------------------------------------------------
 *  BCT_REG_ID  dec op2; if op2>0 goto op1                           pej 26 August 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCT_ID_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - BCT R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            (current_frame->locals[REG_IDX(2)]->int_value)--;
            if (current_frame->locals[REG_IDX(2)]->int_value > 0) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL;
            }
        DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BCT_REG_REG_ID  dec op2, inc op3; if op2>0 goto op1              pej 26 August 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCT_ID_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - BCT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            (current_frame->locals[REG_IDX(2)]->int_value)--;
            (current_frame->locals[REG_IDX(3)]->int_value)++;
            if (current_frame->locals[REG_IDX(2)]->int_value>0) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL;
            }
        DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BCF_ID_REG if op2=0 goto op1(if false) else dec op2
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCF_ID_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - BCF R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            if (current_frame->locals[REG_IDX(2)]->int_value == 0) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL;
            }
            else (current_frame->locals[REG_IDX(2)]->int_value)--;
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BCF_ID_REG_REG  if op2=0 goto op1(if false) else dec op2 and inc op3
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCF_ID_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - BCF R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            if (current_frame->locals[REG_IDX(2)]->int_value == 0) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL;
            }
            else {
                (current_frame->locals[REG_IDX(2)]->int_value)--;
                (current_frame->locals[REG_IDX(3)]->int_value)++;
            }
            DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BCTNM_REG_ID  dec op2; if op2>=0 goto op1                           pej 26 August 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCTNM_ID_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - BCTNM R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            (current_frame->locals[REG_IDX(2)]->int_value)--;
            if (current_frame->locals[REG_IDX(2)]->int_value>=0) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL;
            }
        DISPATCH;
/* ------------------------------------------------------------------------------------
 *  BCTNM_REG_REG_ID  dec op2, inc op3; if op2>=0 goto op1              pej 26 August 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCTNM_ID_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - BCTNM R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            (current_frame->locals[REG_IDX(2)]->int_value)--;
            (current_frame->locals[REG_IDX(3)]->int_value)++;
            if (current_frame->locals[REG_IDX(2)]->int_value>=0) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL;
            }
        DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FndBlnk REG_REG_REG  return first blank after op2[op3]          pej 27 August 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FNDBLNK_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - FNDBLNK R%llu R%llu\n", REG_IDX(1), REG_IDX(2));
            {
                rxinteger len;
                rxinteger result;
                int ch;
#ifndef NUTF8
                len = (rxinteger) op2R->string_chars;
#else
                len = (rxinteger) op2R->string_length;
#endif
                for (result = op3R->int_value; result < len; result++) {
#ifndef NUTF8
                    string_set_byte_pos(op2R, result);
                    utf8codepoint(op2R->string_value + op2R->string_pos, &ch);
#else
                    ch = op2R->string_value[result];
#endif
                    if (ch == ' ') goto blankfound;
                }
                result = -len;
            blankfound:
                REG_RETURN_INT(result);
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  FndNBlnk REG_REG_REG  return first blank after op2[op3]          pej 27 August 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FNDNBLNK_REG_REG_REG) CALC_DISPATCH(3);
           DEBUG("TRACE - FNDNBLNK R%llu R%llu\n", REG_IDX(1),
              REG_IDX(2));
    {
        rxinteger result;
        rxinteger len;
        int ch;
#ifndef NUTF8
        len = (rxinteger)op2R->string_chars;
#else
        len = (rxinteger)op2R->string_length;
#endif
        for (result = op3R->int_value; result < len; result++) {
#ifndef NUTF8
            string_set_byte_pos(op2R, result);
            utf8codepoint(op2R->string_value + op2R->string_pos, &ch);
#else
            ch = op2R->string_value[result];
#endif
            if (ch != ' ') goto nonblankfound;
        }
        result = -len;

    nonblankfound:
        REG_RETURN_INT(result);
    }
    DISPATCH;
 /* ------------------------------------------------------------------------------------
  *  GETBYTE_REG_REG_REG  Int op1 = op2[op3]                             pej 19 Oct 2021
  *  -----------------------------------------------------------------------------------
  */
    START_INSTRUCTION(GETBYTE_REG_REG_REG) CALC_DISPATCH(3);
    DEBUG("TRACE - GETBYTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));

    /* TODO */
    REG_RETURN_INT(0);
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  CONCCHAR_REG_REG_REG  op1=op2[op3]                                pej 27 August 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(CONCCHAR_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - CONCCHAR R%llu R%llu R%llu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                rxinteger temp = op3R->int_value;   // save offset, we misuse v3 later
#ifndef NUTF8
                int ch;
                string_set_byte_pos(op2R, op3R->int_value);
                utf8codepoint(op2R->string_value + op2R->string_pos, &ch);
                op3R->int_value = ch;
#else
                op3R->int_value=op2R->string_value[v3->int_value - 1];
#endif
                string_concat_char(op1R, op3R);
                op3R->int_value = temp;   // restore original v3
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  TRANSCHAR_REG_REG_REG  replace op1 if it is in op3-list by char in op2-list pej 7 November 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(TRANSCHAR_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - TRANSCHAR R%llu R%llu R%llu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                rxinteger val = op1R->int_value;
                rxinteger len, i;
                int ch;

                GETSTRLEN(len, op3R);

                for (i = 0; i < len; i++) {
                    GETSTRCHAR(ch, op3R, i);
                    if (val == ch) {
                        GETSTRCHAR(ch, op2R, i);
                        val = ch;
                        break;
                    }
                }
                REG_RETURN_INT(val);
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  DROPCHAR_REG_REG_REG  removes characters contained in op3-list pej 19 November 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(DROPCHAR_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - DROPCHAR R%llu R%llu R%llu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                rxinteger i, len1, len2;
                int found;
                int ch;
                GETSTRLEN(len1, op2R);
                GETSTRLEN(len2, op3R);
                if (len2 == 0) len2 = (rxinteger) op3R->string_length;
                for (i = 0; i < len1; i++) {
                    rxinteger j;
                    GETSTRCHAR(ch, op2R, i);
                    op2R->int_value = ch;
                    found = 0;
                    for (j = 0; j < len2; j++) {
                        GETSTRCHAR(ch, op3R, j);
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
 *  SUBSTRING_REG_REG_REG op1=substr(op2,op3) substring from  offset op3  pej 12 November 2021
 *
 *  !!! the position parameter is offset +1, this is an exception from normally     !!!
 *  !!! using the offset. Reason: this instruction will be used directly from rexx  !!!
 *  !!  so we save one instruction                                                  !!
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SUBSTRING_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - SUBSTRING R%llu R%llu R%llu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                rxinteger offset = op3R->int_value - 1;   /* make position to offset  */
                rxinteger len, i;
                int ch;
                PUTSTRLEN(op1R, 0);      /* reset length of target  */
                GETSTRLEN(len, op3R);
                for (i = offset; i < len; i++) {
                    GETSTRCHAR(ch, op2R, i);
                    op2R->int_value = ch;
                    string_concat_char(op1R, op2R);
                }
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
*  SUBSTCUT_REG_REG_REG op1=substr(op1,,op2) cuts off after op2   pej 13 November 2021
*  -----------------------------------------------------------------------------------
*/
        START_INSTRUCTION(SUBSTCUT_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - SUBSTCUT R%llu R%llu\n", REG_IDX(1), REG_IDX(2));

            PUTSTRLEN(op1R,op2R->int_value) ;

        DISPATCH;

/* ------------------------------------------------------------------------------------
 *  PADSTR_REG_REG_REG op1=op2(repeated op3 times)                 pej 13 November 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(PADSTR_REG_REG_REG) CALC_DISPATCH(3);
            DEBUG("TRACE - PADSTR R%llu R%llu R%llu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                int pad;
                int i;
                PUTSTRLEN(op1R, 0);        /* reset length of target  */
                GETSTRCHAR(pad, op2R, 0);       /* fetch pad character   */
                op2R->int_value = pad;
                for (i = 0; i < op3R->int_value; i++) {
                    string_concat_char(op1R, op2R);
                }
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  CNOP Dummy instruction for testing purposes                     pej 11 November 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(CNOP) CALC_DISPATCH(0);
        DEBUG("TRACE - CNOP\n");
        DISPATCH;

/*
 *   APPENDCHAR_REG_REG Append Concat Char op2 (as int) on op1
 */
        START_INSTRUCTION(APPENDCHAR_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - APPENDCHAR R%llu R%llu\n", REG_IDX(1),
                  REG_IDX(2));
            string_concat_char(op1R, op2R);
            DISPATCH;

/*
 *   APPEND_REG_REG Append string op2 on op1
 */
        START_INSTRUCTION(APPEND_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - APPEND R%llu R%llu\n", REG_IDX(1),
                  REG_IDX(2));
            string_append(op1R, op2R);
            DISPATCH;

/*
 *   SAPPEND_REG_REG Append with space string op2 on op1
 */
        START_INSTRUCTION(SAPPEND_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - SAPPEND R%llu R%llu\n", REG_IDX(1),
                  REG_IDX(2));
            string_sappend(op1R, op2R);
            DISPATCH;

/*
 *   STRLEN_REG_REG String Length op1 = length(op2)
 */
        START_INSTRUCTION(STRLEN_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - STRLEN R%llu R%llu\n", REG_IDX(1),
                  REG_IDX(2));
#ifndef NUTF8
            op1R->int_value = (rxinteger)op2R->string_chars;
#else
            op1R->int_value = (rxinteger)op2R->string_length;
#endif
            DISPATCH;

/*
 * SETSTRPOS_REG_REG - Set String (op1) charpos set to op2
 */
        START_INSTRUCTION(SETSTRPOS_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - SETSTRPOS R%llu R%llu\n", REG_IDX(1),
                  REG_IDX(2));
#ifndef NUTF8
            string_set_byte_pos(op1R, op2R->int_value);
#else
            op1R->string_pos = op2R->int_value;
#endif
            DISPATCH;

/*
 * GETSTRPOS_REG_REG - Get String (op2) charpos into op1
 */
        START_INSTRUCTION(GETSTRPOS_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - GETSTRPOS R%llu R%llu\n", REG_IDX(1),
                  REG_IDX(2));
#ifndef NUTF8
            op1R->int_value = (int) op2R->string_char_pos;
#else
            op1R->int_value = op2R->string_pos;
#endif
            DISPATCH;

/*
 * STRCHAR_REG_REG - op1 (as int) = op2[charpos]
 */
        START_INSTRUCTION(STRCHAR_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - STRCHAR R%llu R%llu\n", REG_IDX(1),
                  REG_IDX(2));
            {
                int ch;
#ifndef NUTF8
                utf8codepoint(op2R->string_value + op2R->string_pos, &ch);
                op1R->int_value = ch;
#else
                op1R->int_value = op2R->string_value[op2R->string_pos];
#endif
            }
            DISPATCH;

/*
 * GETTP_REG_REG gets the register type flag (op1 = op2.typeflag)
 */
        START_INSTRUCTION(GETTP_REG_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - GETTP R%d R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            op1R->int_value = op2R->status.all_type_flags;
            DISPATCH;

/*
 * SETTP_REG_INT sets the register type flag (op1.typeflag = op2)
 */
        START_INSTRUCTION(SETTP_REG_INT) CALC_DISPATCH(2);
            DEBUG("TRACE - SETTP R%d %d\n", (int)REG_IDX(1), (int)op2I);
            op1R->status.all_type_flags = op2I;
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  LOADSETTP_REG_INT load register & set the register type flag pej 11 November 2021
 *   op1=op2 and (op1.typeflag = op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(LOADSETTP_REG_INT_INT) CALC_DISPATCH(3);
        DEBUG("TRACE - LOADSETTP R%d %d %d\n", (int)REG_IDX(1),(int)op2I,(int)op3I);

            op1R->int_value = op2I;
            op1R->status.all_type_flags = op3I;
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  LOADSETTP_REG_string load string to register & set the register type flag pej 11 November 2021
 *   op1=op2 and (op1.typeflag = op3)
 *  -----------------------------------------------------------------------------------
 */
            START_INSTRUCTION(LOADSETTP_REG_STRING_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - LOADSETTP R%d %s %d\n", (int)REG_IDX(1),op2S->string,(int) op3I);

            set_const_string(op1R, op2S);
            op1R->status.all_type_flags = op3I;
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  LOADSETTP_REG_FLOAT float to load register & set the register type flag pej 11 November 2021
 *   op1=op2 and (op1.typeflag = op3)
 *  -----------------------------------------------------------------------------------
 */
            START_INSTRUCTION(LOADSETTP_REG_FLOAT_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - LOADSETTP R%d %g %d\n", (int)REG_IDX(1), op2F,(int) op3I);
            op1R->float_value = op2F;
            op1R->status.all_type_flags = op3I;
            DISPATCH;

/*
 * SETORTP_REG_INT or the register type flag (op1.typeflag = op1.typeflag || op2)
 */
        START_INSTRUCTION(SETORTP_REG_INT) CALC_DISPATCH(2);
            DEBUG("TRACE - SETORTP R%d %d\n", (int)REG_IDX(1), (int)op2I);
            op1R->status.all_type_flags = op1R->status.all_type_flags | op2I;
            DISPATCH;

/*
 * BRTPT_ID_REG if op2.typeflag true then goto op1
 */
        START_INSTRUCTION(BRTPT_ID_REG) CALC_DISPATCH(2);
            DEBUG("TRACE - BRTPT_ID_REG 0x%x R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2));
            if (op2R->status.all_type_flags) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL;
            }
            DISPATCH;

/*
 * BRTPANDT_ID_REG_INT if op2.typeflag && op3 true then goto op1
*/
        START_INSTRUCTION(BRTPANDT_ID_REG_INT) CALC_DISPATCH(3);
            DEBUG("TRACE - BRTPANDT_ID_REG_INT 0x%x R%d %d\n",
                  (unsigned int)REG_IDX(1),
                  (int)REG_IDX(2),(int)op3I);
            if (op2R->status.all_type_flags & op3I) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL;
            }
            DISPATCH;

/* ---------------------------------------------------------------------------
 * load instructions not yet implemented generated from the instruction table
 *      and scan of this module                              pej 8. April 2021
 * ---------------------------------------------------------------------------
 */
#include "instrmiss.h"

        START_INSTRUCTION(IUNKNOWN)
        START_INSTRUCTION(INULL)
            printf("ERROR - Unknown instruction - aborting\n");
            goto interprt_finished;
        convlength:
            DEBUG("maximum string length exceeded\n");
            goto SIGNAL;
        converror:
            DEBUG("Conversion error occurred\n");
            goto SIGNAL;
        notreg:
            DEBUG("Register not initialised\n");
            goto SIGNAL;

        START_INSTRUCTION(EXIT)
#ifndef NDEBUG
            if (debug_mode) printf("TRACE - EXIT\n");
#endif
            rc = 0;
            goto interprt_finished;

    END_OF_INSTRUCTIONS;
    SIGNAL:
    printf("\n\nTRACE - Signal Received - aborting\n");
    rc = -255;
    goto interprt_finished;

    interprt_finished:

    /* Unwind any stack frames */
    while (current_frame) {
        temp_frame = current_frame->parent;
        free_frame(current_frame);
        current_frame = temp_frame;
    }

    /* Deallocate Frames */
    /* We need to loop through each procedure in each module */
    DEBUG("Deallocating Frames and Registers\n");
    for (mod_index = 0; mod_index < num_modules; mod_index++) {
        size_t i = 0;
        while (i < program[mod_index].segment.const_size) {
            c_entry =
                    (chameleon_constant *) (
                            program[mod_index].segment.const_pool + i);
            switch (c_entry->type) {

                case PROC_CONST:
                    if (((proc_constant *) c_entry)->start != SIZE_MAX) {
                        /* Free frames in the procedures free list */
                        while (*(((proc_constant *) c_entry)->frame_free_list)) {
                            temp_frame = *(((proc_constant *)c_entry)->frame_free_list);
                            *(((proc_constant *) c_entry)->frame_free_list) = temp_frame->prev_free;
                            clear_frame(temp_frame);
                            free(temp_frame);
                        }
                    }
                    break;

                default:;
            }
            i += c_entry->size_in_pool;
        }
    }

    /* Deallocate Globals */
    for (mod_index = 0; mod_index < num_modules; mod_index++) {
        int i;
        for (i = 0; i < program[mod_index].segment.globals; i++) {
            clear_value(program[mod_index].globals[i]);
            free(program[mod_index].globals[i]);
        }
    }

#ifndef NDEBUG
    if (debug_mode) printf("Interpreter Finished\n");
#endif

    return rc;
}
