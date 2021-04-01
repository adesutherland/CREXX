//
// Created by adria on 29/03/2021.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxas.h"
#include "operands.h"
#include "rx_vars.h"

/* Signals an error - this function does not return */
void dosignal(int code) {
    printf("signal %d\n", code);
    // TODO longjmp()
    exit(-1);
}

typedef struct stack_frame stack_frame;

struct stack_frame {
    stack_frame *parent;
    void* return_address;
    size_t number_locals;
//    var_pool pool;
    value *locals[1];
};

/* Macros */
#define CALC_DISPATCH(n) {next_pc = pc + (n) + 1; next_inst = (next_pc)->impl_address;}
#define CALC_DISPATCH0 {next_pc = pc; next_inst = (next_pc)->impl_address;}
#define DISPATCH {pc = next_pc; goto *next_inst;}
#define REG_OP(n) current_frame->locals[(pc+(n))->index]
#define INT_OP(n) (pc+(n))->iconst

/* Stack Frame Factory */
stack_frame *frame_f(bin_space *program, proc_constant *procedure, int no_args,
                     stack_frame *parent, void* return_address) {
    stack_frame *this;
    int num_locals;

    num_locals = procedure->locals + program->globals + no_args;
    this = (stack_frame*)calloc(1,sizeof(stack_frame)
            + (sizeof(value *) * num_locals));
    this->parent = parent;
    this->return_address = return_address;
    this->number_locals = num_locals + 1;
    return this;
}

/* Free Stack Frame */
void free_frame(stack_frame *frame) {
    /* TODO Free Variable Pool */
    int l;
    for (l=0; l<frame->number_locals; l++)
        if (frame->locals[l]) free(frame->locals[l]);
    free(frame);
}

/* Interpreter */
int run(bin_space *program, int argc, char *argv[]) {
    proc_constant *procedure;
    size_t i, j;
    bin_code *pc, *next_pc;
    void* next_inst;
    stack_frame *current_frame = 0;
    value *v1, *v2, *v3;
    long long i1, i2, i3;

    /*
     * Temporary Solution to load Instruction database and instruction map
     * in future this will be via generated C source code - so instant
    */
    Instruction *instruction;
    #define NO_OPCODES 200
    void* address_map[NO_OPCODES];
    for (i=0; i<NO_OPCODES; i++) address_map[i] = &&UNKNOWN;

    instruction = src_inst("say", OP_REG, OP_NONE, OP_NONE);
    if (instruction) address_map[instruction->opcode] = &&SAY_REG;
    else printf("Instruction SAY_REG not found\n");

    instruction = src_inst("exit", OP_NONE, OP_NONE, OP_NONE);
    if (instruction) address_map[instruction->opcode] = &&EXIT;
    else printf("Instruction EXIT not found\n");

    instruction = src_inst("load", OP_REG, OP_INT, OP_NONE);
    if (instruction) address_map[instruction->opcode] = &&LOAD_REG_INT;
    else printf("Instruction LOAD_REG_INT not found\n");

    instruction = src_inst("imult", OP_REG, OP_REG, OP_REG);
    if (instruction) address_map[instruction->opcode] = &&IMULT_REG_REG_REG;
    else printf("Instruction IMULT_REG_REG_REG not found\n");

    /* Finished making instruction map done  - temporary approach */

    /* Thread code - simples! */
    i = 0;
    while (i < program->inst_size) {
        j = i;
        i += program->binary[i].instruction.no_ops + 1;
        program->binary[j].impl_address =
                address_map[program->binary[j].instruction.opcode];
    }

    /* Find the programs entry point
     * TODO The assembler should save this in the binary structure */
    i = 0;
    while (i < program->const_size) {
        procedure = (proc_constant *) (program->const_pool + i);
        if (procedure->base.type == PROC_CONST &&
            strcmp(procedure->name, "main") == 0)
            break;
        procedure = 0;
    }

    if (!procedure) {
        printf("main() not found\n");
        goto interprt_finished;
    }

    current_frame = frame_f(program, procedure, argc, 0, 0);
    /* TODO Globals */
    /* Arguments */
    current_frame->locals[program->globals + procedure->locals] = value_int_f(argc);
    for (i = 0, j = program->globals + procedure->locals + 1; i<argc; i++, j++) {
        current_frame->locals[j] = value_nullstring_f(argv[i]);
    }

    /* Start */
    pc = &(program->binary[procedure->start]);
    CALC_DISPATCH0;
    DISPATCH;

    /* Instruction implementations */
    LOAD_REG_INT:
        CALC_DISPATCH(2);
        printf("TRACE - LOAD_REG_INT\n");
        v1 = REG_OP(1);
        i2 = INT_OP(2);
        if (v1) set_int(v1,i2);
        else REG_OP(1) = value_int_f(i2);
        DISPATCH;

    SAY_REG:
        CALC_DISPATCH(1);
        printf("TRACE - SAY_REG\n");
        v1 = REG_OP(1);
        if (!v1) {
            printf("register not initialised\n");
            goto SIGNAL;
        }
        prime_string(v1);
        printf("> %.*s\n", v1->string_length, v1->string_value);
        DISPATCH;

    IMULT_REG_REG_REG:
        CALC_DISPATCH(3);
        printf("TRACE - IMULT_REG_REG_REG\n");
        v1 = REG_OP(1);
        v2 = REG_OP(2);
        v3 = REG_OP(3);
        if (!v2 || !v3) {
            printf("register not initialized\n");
            goto SIGNAL;
        }

        REG_OP(1) = value_int_f(v2->int_value * v3->int_value);
        DISPATCH;

    EXIT:
        printf("TRACE - EXIT\n");
        goto interprt_finished;

    UNKNOWN:
        printf("ERROR - Unimplemented instruction - aborting\n");
        goto interprt_finished;

    SIGNAL:
        printf("TRACE - Signal Received - aborting\n");
        goto interprt_finished;


    interprt_finished:
    if (current_frame) free_frame(current_frame); // TODO need to delete all frames ...
    printf("Interpreter Finished\n");

 //   free_ops();

    return 0;
}
