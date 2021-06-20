//
// Created by adrian on 29/03/2021.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include "platform.h"
#include "rxvmintp.h"
#include "rxas.h"
#include "rxvminst.h"
//#include "rxvmvars.h"
#include "rxastree.h"

/* Signals an error - this function does not return */
void dosignal(int code) {
    printf("signal %d\n", code);
    // TODO longjmp()
    exit(-1);
}

typedef struct stack_frame stack_frame;

struct stack_frame {
    stack_frame *parent;
    bin_space *module;
    void *return_inst;
    bin_code *return_pc;
    value **return_reg;
    size_t number_locals;
//    var_pool pool;
    value *locals[1]; /* Must be last member */
};
/* Macros */

/* Stack Frame Factory */
stack_frame *frame_f(module *program, proc_constant *procedure, int no_args,
                     stack_frame *parent, bin_code *return_pc, void* return_inst,
                     value **return_reg) {
    stack_frame *this;
    int num_locals;
    int i,j;

    num_locals = procedure->locals + procedure->module->globals + no_args + 1;
    this = (stack_frame*)calloc(1,sizeof(stack_frame)
                                  + (sizeof(value *) * (num_locals)));
    this->parent = parent;
    this->return_inst = return_inst;
    this->return_pc = return_pc;
    this->number_locals = num_locals;
    this->return_reg = return_reg;
    this->module = procedure->module;

    /* Locals */
    for (i = 0; i < procedure->locals; i++) {
        this->locals[i] = value_int_f(this, 0);
    }

    /* Globals */
    for (j = 0; j < this->module->globals; i++, j++) {
        this->locals[i] = program[this->module->module_index].globals[j];
    }

    return this;
}

/* Free Stack Frame */
void free_frame(stack_frame *frame) {
    /* TODO Free Variable Pool */
    int l;
    for (l=0; l<frame->number_locals; l++)
        free_value(frame, frame->locals[l]);
    free(frame);
}

/* Interpreter */
int run(int num_modules, module *program, int argc, char *argv[],
        int debug_mode) {
    proc_constant *procedure;
    size_t i, j;
    int mod_index;
    bin_code *pc, *next_pc;
    void *next_inst;
    stack_frame *current_frame = 0, *temp_frame;
    value *v1, *v2, *v3;
    rxinteger i1, i2, i3;
    double f1, f2, f3;
    char *converr;
    string_constant *s1, *s2, *s3;
    proc_constant *p1, *p2, *p3;
    /* Linker Stuff */
    chameleon_constant *c_entry;
    proc_constant *p_entry, *p_entry_linked;
    struct avl_tree_node *exposed_proc_tree = 0;
    struct avl_tree_node *exposed_reg_tree = 0;
    value *g_reg;

    /*
     * Instruction database - loaded from a generated header file
     */
    Instruction *instruction;
    #include "instrset.h"  /* Set up void *address_map[] */

    /* Link Modules Together */
    DEBUG("Linking - Build Symbols\n");
    for (mod_index=0; mod_index<num_modules; mod_index++) {
        i = 0;
        while (i < program[mod_index].segment.const_size) {
            c_entry = (chameleon_constant *)(program[mod_index].segment.const_pool + i);
            switch(c_entry->type) {

                case PROC_CONST:
                    if ( ((proc_constant*)c_entry)->start != SIZE_MAX ) {
                        /* Mark the owning module segment address */
                        ((proc_constant*)c_entry)->module = &program[mod_index].segment;
                    }
                    break;

                case EXPOSE_REG_CONST:
                    /* Exposed Register */
                    if (src_node(exposed_reg_tree, ((expose_reg_constant *)c_entry)->index,
                                      (size_t*)&g_reg)) {
                        /* Register already exposed / initialised */
                        program[mod_index].globals[((expose_reg_constant *)c_entry)->global_reg] =
                                    g_reg;
                    }
                    else {
                        /* Need to initialise a register and expose it in the search tree */
                        program[mod_index].globals[((expose_reg_constant *)c_entry)->global_reg] =
                                    value_int_f(program[mod_index].globals, 0);
                        if (add_node(&exposed_reg_tree,
                                     ((expose_reg_constant *) c_entry)->index,
                                     (size_t)(program[mod_index].globals[((expose_reg_constant *)c_entry)->global_reg]))) {
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

                default: ;
            }

            i += c_entry->size_in_pool;
        }
    }

    DEBUG("Linking - Resolve Symbols\n");
    for (mod_index=0; mod_index<num_modules; mod_index++) {
        i = 0;
        while (i < program[mod_index].segment.const_size) {
            c_entry = (chameleon_constant *)(program[mod_index].segment.const_pool + i);
            switch(c_entry->type) {
                case EXPOSE_PROC_CONST:
                    p_entry = (proc_constant *)(program[mod_index].segment.const_pool
                                                + ((expose_proc_constant*)c_entry)->procedure);
                    if ( ((expose_proc_constant *)c_entry)->imported ) {
                        if (!src_node(exposed_proc_tree, ((expose_proc_constant *)c_entry)->index,
                                      (size_t*)&p_entry_linked)) {
                            fprintf(stderr, "Unimplemented symbol: %s\n",
                                    ((expose_proc_constant *) c_entry)->index);
                            exit(-1); /* Unimplemented */
                        }

                        /* Patch the procedure entry with the linked one */
                        p_entry->locals = p_entry_linked->locals;
                        p_entry->start = p_entry_linked->start;
                        p_entry->module = p_entry_linked->module;
                    }
                    break;

                default: ;
            }

            i += c_entry->size_in_pool;
        }
    }

    /* Allocate Module Globals */
    DEBUG("Allocate Module Globals\n");
    for (mod_index=0; mod_index<num_modules; mod_index++) {
        for (i = 0; i < program[mod_index].segment.globals; i++ ) {
            if (!program[mod_index].globals[i])
                program[mod_index].globals[i] =
                        value_int_f(program[mod_index].globals, 0);
        }
    }

        /* Thread code - simples! */
#ifndef NTHREADED
    DEBUG("Threading\n");
    for (mod_index=0; mod_index<num_modules; mod_index++) {
        i = 0;
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
    for (mod_index=0; mod_index<num_modules; mod_index++) {
        i = 0;
        while (i < program[mod_index].segment.const_size) {
            procedure = (proc_constant *) (program[mod_index].segment.const_pool + i);
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
    current_frame = frame_f(program, procedure, argc, 0, 0, 0, 0);
    /* Arguments */
    current_frame->locals[current_frame->module->globals + procedure->locals] =
            value_int_f(current_frame, argc);
    for (i = 0, j = current_frame->module->globals + procedure->locals + 1; i < argc; i++, j++) {
        current_frame->locals[j] = value_nullstring_f(current_frame, argv[i]);
    }
    /* Start */
    DEBUG("Starting inst# %s-0x%x\n", program[current_frame->module->module_index].name,
          (int)procedure->start);
    next_pc = &(current_frame->module->binary[procedure->start]);
    CALC_DISPATCH_MANUAL;
    DISPATCH;

    START_OF_INSTRUCTIONS;

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
START_INSTRUCTION(LOAD_REG_INT)
    CALC_DISPATCH(2)
    DEBUG("TRACE - LOAD_REG_INT R%llu %llu\n", REG_IDX(1), op2I);

    set_int(op1R, op2I);

    DISPATCH;

START_INSTRUCTION(LOAD_REG_STRING)
    CALC_DISPATCH(2)
    DEBUG("TRACE - LOAD_REG_STRING R%llu \"%.*s\"\n",
          REG_IDX(1), (int) (CONSTSTRING_OP(2))->string_len, (CONSTSTRING_OP(2))->string);
    set_conststring(op1R, CONSTSTRING_OP(2));

    DISPATCH;

    /* TODO Depreciated */
START_INSTRUCTION(SAY_REG)
    CALC_DISPATCH(1)
    DEBUG("TRACE - SAY_REG R%llu\n", REG_IDX(1));

    prime_string(op1R);
    printf("%.*s", (int) op1R->string_length, op1R->string_value);
    DISPATCH;

/* String Say */
START_INSTRUCTION(SSAY_REG)
    CALC_DISPATCH(1)
    DEBUG("TRACE - SSAY_REG R%llu\n", REG_IDX(1));
    printf("%.*s", (int) op1R->string_length, op1R->string_value);
    DISPATCH;

START_INSTRUCTION(SAY_STRING)
    CALC_DISPATCH(1);
    DEBUG("TRACE - SAY_STRING constant index 0x%x\n", (unsigned int) (pc+1)->index);
    s1 = CONSTSTRING_OP(1);
    printf("%.*s", (int) s1->string_len, s1->string);
    DISPATCH;

START_INSTRUCTION(SCONCAT_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - SCONCAT_REG_REG_REG R%llu R%llu R%llu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));

    v1 = op1R;
    v2 = op2R;
    v3 = op3R;
    string_sconcat(v1,v2,v3);
    DISPATCH;

START_INSTRUCTION(CONCAT_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - CONCAT_REG_REG_REG R%llu R%llu R%llu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));

    v1 = op1R;
    v2 = op2R;
    v3 = op3R;
    string_concat(v1,v2,v3);
    DISPATCH;

START_INSTRUCTION(IMULT_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - IMULT_REG_REG_REG R%llu R%llu R%llu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));

    REG_RETURN_INT(op2RI * op3RI);

    DISPATCH;

START_INSTRUCTION(IMULT_REG_REG_INT)
    {
        CALC_DISPATCH(3);
        DEBUG("TRACE - IMULT_REG_REG_INT R%llu R%llu %llu\n", REG_IDX(1), REG_IDX(2), op3I);

        REG_RETURN_INT(op2RI * op3I);

        DISPATCH;
    }

    START_INSTRUCTION(IADD_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - IADD_REG_REG_REG R%llu R%llu R%llu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));

    REG_RETURN_INT(op2RI + op3RI);

    DISPATCH;

    START_INSTRUCTION(ISUB_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - ISUB_REG_REG_REG R%llu R%llu R%llu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));

    REG_RETURN_INT(op2RI - op3RI);

    DISPATCH;

    START_INSTRUCTION(IADD_REG_REG_INT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - IADD_REG_REG_INT R%llu R%llu %llu\n", REG_IDX(1), REG_IDX(2), op3I);

    REG_RETURN_INT(op2RI + op3I);

    DISPATCH;

    START_INSTRUCTION(CALL_FUNC)
    CALC_DISPATCH(1);
    p1 = PROC_OP(1); /* This is the target */
    /* New stackframe */
    current_frame = frame_f(program, p1, 0, current_frame, next_pc,
                            next_inst, 0);
    DEBUG("TRACE - CALL_FUNC %s()\n",p1->name);
    /* Prepare dispatch to procedure as early as possible */
    next_pc = &(current_frame->module->binary[p1->start]);
    CALC_DISPATCH_MANUAL;
    /* Arguments - none */
    current_frame->locals[current_frame->module->globals + p1->locals] = value_int_f(current_frame, 0);
    /* This gotos the start of the called procedure */
    DISPATCH;

    START_INSTRUCTION(CALL_REG_FUNC)
    CALC_DISPATCH(2);
    v1 = op1R;
    p2 = PROC_OP(2); /* This is the target */
    /* Clear target return value register */
    free_value(current_frame, v1);
    op1R = 0;
    /* New stackframe */
    current_frame = frame_f(program, p2, 0, current_frame, next_pc,
                            next_inst, &(op1R));
    DEBUG("TRACE - CALL_REG_FUNC R%llu=%s()\n", REG_IDX(1), p2->name);
    /* Prepare dispatch to procedure as early as possible */
    next_pc = &(current_frame->module->binary[p2->start]);
    CALC_DISPATCH_MANUAL;
    /* Arguments - none */
    current_frame->locals[current_frame->module->globals + p2->locals] = value_int_f(current_frame, 0);
    /* This gotos the start of the called procedure */
    DISPATCH;

    START_INSTRUCTION(CALL_REG_FUNC_REG)
    CALC_DISPATCH(3);
    v1 = op1R;
    p2 = PROC_OP(2); /* This is the target */
    v3 = op3R;
    if (!v3 || !v3->status.primed_int) ERROR("ERROR: CALL_REG_FUNC_REG Arg Reg not an integer");
    /* Clear target return value register */
    free_value(current_frame, v1);
    op1R = 0;
    /* New stackframe */
    current_frame = frame_f(program, p2, (int)v3->int_value, current_frame, next_pc,
                            next_inst, &(op1R));

    DEBUG("TRACE - CALL_REG_FUNC_REG R%llu=%s(R%llu...)\n", REG_IDX(1),
          p2->name, REG_IDX(3));
    /* Prepare dispatch to procedure as early as possible */
    next_pc = &(current_frame->module->binary[p2->start]);
    CALC_DISPATCH_MANUAL;
    /* Arguments - complex lets never have to change this code! */
    current_frame->locals[current_frame->module->globals + p2->locals] =
            current_frame->parent->locals[(pc + (3))->index];
    for (i=0; i<v3->int_value; i++) {
        current_frame->locals[current_frame->module->globals + p2->locals + i + 1] =
                current_frame->parent->locals[(pc + (3))->index + i + 1];
    }
    /* This gotos the start of the called procedure */
    DISPATCH;

    START_INSTRUCTION(RET)
    CALC_DISPATCH(0);
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
        goto interprt_finished;
    }
    DISPATCH;

    START_INSTRUCTION(RET_REG)
    CALC_DISPATCH(1);
    DEBUG("TRACE - RET_REG\n");
    v1 = op1R;
    /* Where we return to */
    next_pc = current_frame->return_pc;
    next_inst = current_frame->return_inst;
    /* Set the result register */
    if (current_frame->return_reg) {
        *(current_frame->return_reg) = v1;
        if (v1) v1->owner = current_frame->parent;
    }
    /* back to the parents stack frame */
    temp_frame = current_frame;
    current_frame = current_frame->parent;
    if (!current_frame) ERROR("ERROR - Past top of procedure stack - aborting\n");

    free_frame(temp_frame);
    DISPATCH;

    START_INSTRUCTION(RET_INT)
    CALC_DISPATCH(1);
    DEBUG("TRACE - RET_INT\n");
    i1 = op1I;
    /* Where we return to */
    next_pc = current_frame->return_pc;
    next_inst = current_frame->return_inst;
    /* Set the result register */
    if (current_frame->return_reg)
        *(current_frame->return_reg) = value_int_f(current_frame->parent,
                                                   i1);
    /* back to the parents stack frame */
    temp_frame = current_frame;
    current_frame = current_frame->parent;
    if (!current_frame) ERROR("ERROR - Past top of procedure stack - aborting\n");

    free_frame(temp_frame);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  RET_FLOAT                                                        pej 12. April 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(RET_FLOAT)
    CALC_DISPATCH(1);
    DEBUG("TRACE - RET_FLOAT\n");

    f1 = op1F;
    /* Where we return to */
    next_pc = current_frame->return_pc;
    next_inst = current_frame->return_inst;
    /* Set the result register */
    if (current_frame->return_reg)
        *(current_frame->return_reg) = value_float_f(current_frame->parent,f1);
    /* back to the parents stack frame */
    temp_frame = current_frame;
    current_frame = current_frame->parent;
    if (!current_frame) ERROR("ERROR - Past top of procedure stack - aborting\n");

    free_frame(temp_frame);
    DISPATCH;
    /* ------------------------------------------------------------------------------------
    *  RET_FLOAT                                                        pej 12. April 2021
    *  -----------------------------------------------------------------------------------
    */
    START_INSTRUCTION(RET_STRING)
    CALC_DISPATCH(1);
    DEBUG("TRACE - RET_FLOAT\n");

    s1 = CONSTSTRING_OP(1);

    /* Where we return to */
    next_pc = current_frame->return_pc;
    next_inst = current_frame->return_inst;
    /* Set the result register */
    if (current_frame->return_reg)
        *(current_frame->return_reg) = value_conststring_f(current_frame->parent,s1);
    /* back to the parents stack frame */
    temp_frame = current_frame;
    current_frame = current_frame->parent;
    if (!current_frame) ERROR("ERROR - Past top of procedure stack - aborting\n");

    free_frame(temp_frame);
    DISPATCH;

    START_INSTRUCTION(INIT_REG)
    CALC_DISPATCH(1);
    DEBUG("TRACE - INIT_REG R%llu \n", REG_IDX(1));

    op1R = value_f(current_frame);

    DISPATCH;

    START_INSTRUCTION(FREE_REG) // label not yet defined
    CALC_DISPATCH(1);
    DEBUG("TRACE - FREE_REG R%llu \n", REG_IDX(1));

    /* v1 needs to be deallocated */
    free_value(current_frame, op1R);
    op1R = NULL;

    DISPATCH;

    START_INSTRUCTION(MOVE_REG_REG)
    CALC_DISPATCH(2);
    DEBUG("TRACE - MOVE_REG_REG R%llu R%llu\n", REG_IDX(1), REG_IDX(2));

    /* v1 needs to be deallocated */
    free_value(current_frame, op1R);

    /* Now move the register; if op2 is null, well so be it, no harm done */
    op1R = op2R;
    op2R = NULL;

    DISPATCH;

    START_INSTRUCTION(DEC0)
    /* TODO This is really idec0 - i.e. it does not prime the int */
    CALC_DISPATCH(0);
    DEBUG("TRACE - DEC0\n");
    (current_frame->locals[0]->int_value)--;
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DEC1   R1--                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DEC1)
    /* TODO This is really idec1 - i.e. it does not prime the int */
    CALC_DISPATCH(0);
    DEBUG("TRACE - DEC1\n");
    (current_frame->locals[1]->int_value)--;
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DEC2   op2R--                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DEC2)
    /* TODO This is really idec2 - i.e. it does not prime the int */
    CALC_DISPATCH(0);
    DEBUG("TRACE - DEC2\n");
    (current_frame->locals[2]->int_value)--;
    DISPATCH;

    START_INSTRUCTION(DEC_REG)
    /* TODO This is really idec reg - i.e. it does not prime the int */
    CALC_DISPATCH(1);
    DEBUG("TRACE - DEC_REG R%llu\n", REG_IDX(1));
    (current_frame->locals[REG_IDX(1)]->int_value)--;
    DISPATCH;

    START_INSTRUCTION(BR_ID)
    next_pc = current_frame->module->binary + REG_IDX(1);
    CALC_DISPATCH_MANUAL;
    DISPATCH;

    /* For these we optimise for condition to NOT be met because in a loop
     * these ae used to jump out of the loop when the end condition it met
     * (and every little bit helps to improve performance!)
     */
    START_INSTRUCTION(BRT_ID_REG)
    CALC_DISPATCH(2); /* i.e. if the condition is not met - this helps the
                                the real CPUs branch prediction (in theory) */
    if (op2RI) {
        next_pc = current_frame->module->binary + REG_IDX(1);
        CALC_DISPATCH_MANUAL;
    }
    DISPATCH;

    START_INSTRUCTION(BRF_ID_REG)
    CALC_DISPATCH(2); /* i.e. if the condition is not met - this helps the
                                  the real CPUs branch prediction (in theory) */
    DEBUG("TRACE - BRF_ID_REG R%llu\n", REG_IDX(1));
    if (!(op2RI)) {
        next_pc = current_frame->module->binary + REG_IDX(1);
        CALC_DISPATCH_MANUAL;
    }
    DISPATCH;

    START_INSTRUCTION(IMASTER_REG)
    CALC_DISPATCH(1);
    DEBUG("TRACE - IMASTER_REG R%llu\n", REG_IDX(1));

    master_int(op1R);
    DISPATCH;

    START_INSTRUCTION(FMASTER_REG)
    CALC_DISPATCH(1);
    DEBUG("TRACE - FMASTER_REG R%llu\n", REG_IDX(1));

    master_float(op1R);
    DISPATCH;

    START_INSTRUCTION(SMASTER_REG)
    CALC_DISPATCH(1);
    DEBUG("TRACE - SMASTER_REG R%llu\n", REG_IDX(1));

    master_string(op1R);
    DISPATCH;

    START_INSTRUCTION(TIME_REG)
    CALC_DISPATCH(1);
    DEBUG("TRACE - TIME R%llu\n", REG_IDX(1));

    set_int(op1R, time(NULL));

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  TRIMR  Trim right                                                 pej 7. April 2021
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(TRIMR_REG_REG)
    CALC_DISPATCH(2);
    DEBUG("TRACE - TRIMR_REG_REG\n") ;
    v1 = op1R;
    prime_string(v1);

    i = v1->string_length - 1;
    while (i >= 0 && v1->string_value[i] == ' ') {
        v1->string_value[i] = '\0';
        i--;
    }
    v1->string_length=i+1;
    op1R=v1;
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  TRIML  Trim left                                                  pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(TRIML_REG_REG)
    CALC_DISPATCH(2);
    DEBUG("TRACE - TRIML_REG_REG\n") ;
    v1 = op1R;
    prime_string(v1);

    j = v1->string_length - 1;
    i=0;
    while (i <= j && v1->string_value[i] == ' ') i++;

    if (i>=j) {
        v1->string_length=0;
        v1->string_value[0] = '\0';
    } else {
        v1->string_length= v1->string_length-i;
        memcpy(v1->string_value,v1->string_value+i,v1->string_length);
        v1->string_value[v1->string_length] = '\0';
    }
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  INC0   R0++                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(INC0)
    CALC_DISPATCH(0);
    DEBUG("TRACE - INC0\n");
    REG_VAL(0)->int_value++;

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  INC1   R1++                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(INC1)
    CALC_DISPATCH(0);
    DEBUG("TRACE - INC1\n");
    REG_VAL(1)->int_value++;

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  INC2   op2R++                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(INC2)
    CALC_DISPATCH(0);
    DEBUG("TRACE - INC2\n");
    REG_VAL(2)->int_value++;

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ISUB_REG_REG_INT: Integer Subtract (op1=op2-op3)               pej 8. April 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(ISUB_REG_REG_INT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - ISUB_REG_REG_INT\n") ;

    REG_RETURN_INT(op2RI - op3I);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IEQ_REG_REG_REG  Int Equals op1=(op2==op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(IEQ_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - ISUB_REG_REG_REG\n") ;

    if (op2RI == op3RI) REG_RETURN_INT(1)
    else REG_RETURN_INT(0);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IEQ_REG_REG_INT  Int Equals op1=(op2==op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(IEQ_REG_REG_INT) // label not yet defined
    CALC_DISPATCH(3);
    DEBUG("TRACE - IEQ_REG_REG_INT\n") ;

    if (op2RI == op3I) REG_RETURN_INT(1)
    else REG_RETURN_INT(0);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  INE_REG_REG_REG  Int Equals op1=(op2!=op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(INE_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - INE_REG_REG_REG\n") ;

    if (op2RI != op3RI) REG_RETURN_INT(1)
    else REG_RETURN_INT(0)

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  INE_REG_REG_INT  Int Equals op1=(op2!=op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(INE_REG_REG_INT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - INE_REG_REG_INT\n") ;

    if (op2RI != op3I) REG_RETURN_INT(1)
    else REG_RETURN_INT(0);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IGT_REG_REG_REG  Int Greater than op1=(op2>op3)                      pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(IGT_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - IGT_REG_REG_REG\n") ;

    if (op2RI > op3RI) REG_RETURN_INT(1)
    else REG_RETURN_INT(0)

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IGT_REG_REG_INT  Int Greater than op1=(op2>op3)                      pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(IGT_REG_REG_INT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - IGT_REG_REG_INT\n") ;

    if (op2RI > op3I) REG_RETURN_INT(1)
    else REG_RETURN_INT(0)

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IGT_REG_INT_REG  Int Greater than op1=(op2>op3)                      pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(IGT_REG_INT_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - IGT_REG_INT_REG\n") ;

    if (op2I>op3RI) REG_RETURN_INT(1)
    else REG_RETURN_INT(0)

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ILT_REG_REG_REG  Int Less than op1=(op2<op3)                         pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(ILT_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - ILT_REG_REG_REG\n") ;

    if (op2RI < op3RI) REG_RETURN_INT(1)
    else REG_RETURN_INT(0)

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ILT_REG_REG_INT  Int Less than op1=(op2<op3)                         pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(ILT_REG_REG_INT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - ILT_REG_REG_INT\n") ;

    if (op2RI < op3I) REG_RETURN_INT(1)
    else REG_RETURN_INT(0)

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ILT_REG_INT_REG  Int Less than op1=(op2<op3)                         pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(ILT_REG_INT_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - ILT_REG_INT_REG\n") ;

    if (op2I<op3RI) REG_RETURN_INT(1)
    else REG_RETURN_INT(0)

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IGTE_REG_REG_REG  Int Greater Equal than op1=(op2>=op3)              pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(IGTE_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - IGTE_REG_REG_REG\n") ;

    if (op2RI >= op3RI) REG_RETURN_INT(1)
    else REG_RETURN_INT(0)

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IGTE_REG_REG_INT  Int Greater Equal than op1=(op2>=op3)              pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(IGTE_REG_REG_INT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - IGTE_REG_REG_INT\n") ;

    if (op2RI >= op3I) REG_RETURN_INT(1)
    else REG_RETURN_INT(0)

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IGTE_REG_INT_REG  Int Greater Equal than op1=(op2>=op3)              pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(IGTE_REG_INT_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - IGTE_REG_INT_REG\n") ;

    if (op2I>=op3RI) REG_RETURN_INT(1)
    else REG_RETURN_INT(0)

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ILTE_REG_REG_REG  Int Less Equal than op1=(op2<=op3)                 pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(ILTE_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - ILTE_REG_REG_REG\n") ;

    if (op2RI <= op3RI) REG_RETURN_INT(1)
    else REG_RETURN_INT(0);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ILTE_REG_REG_INT  Int Less Equal than op1=(op2<=op3)                 pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(ILTE_REG_REG_INT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - ILTE_REG_REG_INT\n") ;

    if (op2RI <= op3I) REG_RETURN_INT(1)
    else REG_RETURN_INT(0);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ILTE_REG_INT_REG  Int Less Equal than op1=(op2<=op3)                 pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(ILTE_REG_INT_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - ILTE_REG_INT_REG\n") ;

    if (op2I<=op3RI) REG_RETURN_INT(1)
    else REG_RETURN_INT(0);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  COPY_REG_REG  Copy op2 to op1                                       pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(COPY_REG_REG) // label not yet defined
    CALC_DISPATCH(2);
    op1R = value_f(current_frame);

    memcpy(op1R , op2R, sizeof(value));
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  INC_REG  Increment Int (op1++)                                      pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(INC_REG) // label not yet defined
    CALC_DISPATCH(1);
    DEBUG("TRACE - INC_REG\n") ;
    (current_frame->locals[REG_IDX(1)]->int_value)++;
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IDIV_REG_REG_INT  Integer Divide (op1=op2/op3)                      pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(IDIV_REG_REG_INT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - IDIV_REG_REG_INT\n") ;

    REG_RETURN_INT(op2RI / op3I);
    DISPATCH;
    /* -----------------------------------------------------------------------------------
    *  IDIV_REG_REG_REG  Integer Divide (op1=op2/op3)                      pej 10 Apr 2021
    *  -----------------------------------------------------------------------------------
    */
    START_INSTRUCTION(IDIV_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - IDIV_REG_REG_REG\n") ;

    REG_RETURN_INT(op2RI / op3RI);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  SAY_INT  Say op1                                                    pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(SAY_INT)
    CALC_DISPATCH(1);
    DEBUG("TRACE - SAY_INT\n") ;
#ifdef __32BIT__
    printf("%ld", op1I);
#else
    printf("%lld", op1I);
#endif
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  SAY_CHAR  Say op1                                                   pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(SAY_CHAR)
    CALC_DISPATCH(1);
    DEBUG("TRACE - SAY_CHAR\n") ;

    printf("%c", (pc+(1))->cconst);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  SAY_FLOAT  Say op1                                                  pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(SAY_FLOAT)
    CALC_DISPATCH(1);
    DEBUG("TRACE - SAY_FLOAT\n") ;

    printf("%g", op1F);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  LOAD_REG_FLOAT  Load op1 with op2                                   pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(LOAD_REG_FLOAT)
    CALC_DISPATCH(2);
    DEBUG("TRACE - LOAD_REG_FLOAT\n") ;

    REG_RETURN_FLOAT(op2F);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FADD_REG_REG_REG  Float Add (op1=op2+op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FADD_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - FADD_REG_REG_REG\n") ;

    REG_RETURN_FLOAT(op2RF + op3RF);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FSUB_REG_REG_REG  Float Sub (op1=op2-op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FSUB_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - FSUB_REG_REG_REG\n") ;

    REG_RETURN_FLOAT(op2RF - op3RF);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FDIV_REG_REG_REG  Float Div (op1=op2/op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FDIV_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - FDIV_REG_REG_REG\n") ;

    REG_RETURN_FLOAT(op2RF / op3RF);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FMULT_REG_REG_REG  Float Mult (op1=op2/op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FMULT_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - FMULT_REG_REG_REG\n") ;

    REG_RETURN_FLOAT(op2RF * op3RF);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FADD_REG_REG_FLOAT  Float Add (op1=op2+op3)                          pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FADD_REG_REG_FLOAT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - FADD_REG_REG_FLOAT\n") ;

    REG_RETURN_FLOAT(op2RF + op3F);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FSUB_REG_REG_FLOAT  Float Sub (op1=op2-op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FSUB_REG_REG_FLOAT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - FSUB_REG_REG_FLOAT\n") ;

    REG_RETURN_FLOAT(op2RF - op3F);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FDIV_REG_REG_FLOAT  Float Div (op1=op2/op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FDIV_REG_REG_FLOAT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - FDIV_REG_REG_FLOAT\n") ;

    REG_RETURN_FLOAT(op2RF / op3F);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FMULT_REG_REG_FLOAT  Float Mult (op1=op2/op3)                       pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FMULT_REG_REG_FLOAT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - FMULT_REG_REG_FLOAT\n") ;

    REG_RETURN_FLOAT(op2RF * op3F);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FSUB_REG_FLOAT_REG  Float Sub (op1=op2-op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FSUB_REG_FLOAT_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - FSUB_REG_FLOAT_REG\n") ;

    REG_RETURN_FLOAT(op2F - op3RF);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FDIV_REG_FLOAT_REG  Float Div (op1=op2/op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FDIV_REG_FLOAT_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - FDIV_REG_FLOAT_REG\n") ;

    REG_RETURN_FLOAT(op2F / op3RF);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  STR2INT_REG_REG_REG  String to Int op1 = op2[op3]                   pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(STR2INT_REG_REG_REG) // label not yet defined
    CALC_DISPATCH(3);
    DEBUG("TRACE - STRINT_REG_REG_REG\n") ;

    v2 = op2R;
    v3=op3R;
    i1=v2->string_value[v3->int_value-1]-'0';

    REG_RETURN_INT(i1);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ADDI_REG_REG_INT  Convert and Add to Integer (op1=op2+op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(ADDI_REG_REG_INT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - ADDI_REG_REG_INT\n");

    CONV2INT(i2, op2R);

    REG_RETURN_INT(i2+op3I);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ADDI_REG_REG_REG  Convert and Add to Integer (op1=op2+op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(ADDI_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - ADDI_REG_REG_REG\n");

    CONV2INT(i2, op2R);
    CONV2INT(i3, op3R);

    REG_RETURN_INT(i2+i3);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  SUBI_REG_REG_REG  Convert and Subtract to Integer (op1=op2-op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(SUBI_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - SUBI_REG_REG_REG\n");

    CONV2INT(i2, op2R);
    CONV2INT(i3, op3R);

    REG_RETURN_INT(i2-i3);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  SUBI_REG_REG_INT  Convert and Subtract to Integer (op1=op2-op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(SUBI_REG_REG_INT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - SUBI_REG_REG_INT\n");

    CONV2INT(i2, op2R);

    REG_RETURN_INT(i2-op3I);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  MULTI_REG_REG_REG  Convert and Multiply to Integer (op1=op2*op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(MULTI_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - MULTI_REG_REG_REG\n");

    CONV2INT(i2, op2R);
    CONV2INT(i3, op3R);

    REG_RETURN_INT(i2*i3);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  MULTI_REG_REG_INT  Convert and Multiply to Integer (op1=op2*op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(MULTI_REG_REG_INT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - MULTI_REG_REG_INT\n");

    CONV2INT(i2, op2R);

    REG_RETURN_INT(i2*op3I);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DIVI_REG_REG_REG  Convert and Divide to Integer (op1=op2/op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DIVI_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DIVI_REG_REG_REG\n");

    CONV2INT(i2, op2R);
    CONV2INT(i3, op3R);

    REG_RETURN_INT(i2/i3);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DIVI_REG_REG_INT  Convert and Divide to Integer (op1=op2/op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DIVI_REG_REG_INT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DIVI_REG_REG_INT\n");

    CONV2INT(i2, op2R);

    REG_RETURN_INT(i2/op3I);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ADDF_REG_REG_FLOAT  Convert and Add to Float (op1=op2+op3)          pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(ADDF_REG_REG_FLOAT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - ADDF_REG_REG_FLOAT\n");

    CONV2FLOAT(f2, op2R)

    REG_RETURN_FLOAT(f2 + op3F);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ADDF_REG_REG_REG  Convert and Add to Float (op1=op2+op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(ADDF_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - ADDF_REG_REG_REG\n");

    CONV2FLOAT(f2, op2R)
    CONV2FLOAT(f3, op3R)

    REG_RETURN_FLOAT(f2 + f3);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  SUBF_REG_REG_REG  Convert and Subtract to Float (op1=op2-op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(SUBF_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - SUBF_REG_REG_REG\n");

    CONV2FLOAT(f2, op2R)
    CONV2FLOAT(f3, op3R)

    REG_RETURN_FLOAT(f2 - f3);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  SUBF_REG_REG_FLOAT  Convert and Subtract to Float (op1=op2-op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(SUBF_REG_REG_FLOAT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - SUBF_REG_REG_FLOAT\n");

    CONV2FLOAT(f2, op2R)

    REG_RETURN_FLOAT(f2 - op3F);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  SUBF_REG_FLOAT_REG  Convert and Subtract to Float (op1=op2-op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(SUBF_REG_FLOAT_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - SUBF_REG_FLOAT_REG\n");

    CONV2FLOAT(f3, op3R)

    REG_RETURN_FLOAT(op2F - f3);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  MULTF_REG_REG_REG  Convert and Multiply to Float (op1=op2*op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(MULTF_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - MULTF_REG_REG_REG\n");

    CONV2FLOAT(f2, op2R)
    f3 = op3F;

    REG_RETURN_FLOAT(f2 * op3RF);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  MULTF_REG_REG_FLOAT  Convert and Multiply to Float (op1=op2*op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(MULTF_REG_REG_FLOAT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - MULTF_REG_REG_FLOAT\n");

    CONV2FLOAT(f2, op2R)

    REG_RETURN_FLOAT(f2 * op3F);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DIVF_REG_REG_REG  Convert and Divide to Float (op1=op2/op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DIVF_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DIVF_REG_REG_REG\n");

    CONV2FLOAT(f2, op2R)
    CONV2FLOAT(f3, op3R)

    REG_RETURN_FLOAT(f2 / f3);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DIVF_REG_REG_FLOAT  Convert and Divide to Float (op1=op2/op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DIVF_REG_REG_FLOAT)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DIVF_REG_REG_FLOAT\n");

    CONV2FLOAT(f2, op2R)

    REG_RETURN_FLOAT(f2 / op3F);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DIVF_REG_FLOAT_REG  Convert and Divide to Float (op1=op2/op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DIVF_REG_FLOAT_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DIVF_REG_FLOAT_REG\n");

    CONV2FLOAT(f3, op3R)
    REG_RETURN_FLOAT(op2F / f3);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  AMAP_REG_REG  Int Prime op1              ???
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(AMAP_REG_REG)
    CALC_DISPATCH(2);
    DEBUG("TRACE - AMAP_REG_REG\n");

    op1R = current_frame->locals[op2RI + current_frame->module->globals + procedure->locals];

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  AMAP_REG_INT  Int Prime op1              ???
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(AMAP_REG_INT)
    CALC_DISPATCH(2);
    DEBUG("TRACE - AMAP_REG_INT\n");

    op1R = REG_VAL(op2I);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IPRIME_REG  Int Prime op1                                           pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(IPRIME_REG)
    CALC_DISPATCH(1);
    DEBUG("TRACE - IPRIME_REG\n");

    CONV2INT(i1, op1R);
    REG_RETURN_INT(i1);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FPRIME_REG  Float Prime op1                                         pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FPRIME_REG)
    CALC_DISPATCH(1);
    DEBUG("TRACE - FPRIME_REG\n");

    CONV2FLOAT(f1, op1R);
    REG_RETURN_FLOAT(f1);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  SPRIME_REG  String Prime op1                                        pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(SPRIME_REG)
    CALC_DISPATCH(1);
    DEBUG("TRACE - SPRIME_REG\n");

    prime_string(op1R);
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
    goto interprt_finished;

END_OF_INSTRUCTIONS;

    SIGNAL:
    printf("\n\nTRACE - Signal Received - aborting\n");
    goto interprt_finished;

    interprt_finished:

    /* Deallocate Frames */
    while (current_frame) {
        temp_frame = current_frame->parent;
        free_frame(current_frame);
        current_frame = temp_frame;
    }

    /* Deallocate Globals */
    for (mod_index=0; mod_index<num_modules; mod_index++) {
        for (i = 0; i < program[mod_index].segment.globals; i++ ) {
            free_value(program[mod_index].globals, program[mod_index].globals[i]);
        }
    }

#ifndef NDEBUG
    if (debug_mode) printf("Interpreter Finished\n");
#endif

    return 0;
}