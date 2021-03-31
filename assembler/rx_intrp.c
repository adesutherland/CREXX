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
void signal(int code) {
    printf("signal %d\n", code);
    // TODO longjmp()
    exit(-1);
}

typedef struct stack_frame{


} stack_frame;

/* Dispatch Macros */
#define CALC_DISPATCH(n) {pc += (n) + 1; next_inst = (pc)->impl_address;}
#define CALC_DISPATCH0 next_inst = (pc)->impl_address
#define DISPATCH goto *next_inst

/* Interpreter */
int run(bin_space *program) {
    proc_constant *procedure;
    size_t i, j;
//    size_t pc;
    bin_code *pc;
    void* next_inst;

    /*
     * Temporary Solution to load Instruction database and instruction map
     * in future this will be via generated C source code - so instant
    */
    Instruction *instruction;
//    init_ops();
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

    /* Start */
    pc = &(program->binary[procedure->start]);
    CALC_DISPATCH0;
    DISPATCH;

    /* Instruction implementations */
    LOAD_REG_INT:
        CALC_DISPATCH(2);
        printf("TRACE - LOAD_REG_INT\n");
        DISPATCH;

    SAY_REG:
        CALC_DISPATCH(1);
        printf("TRACE - SAY_REG\n");
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
    printf("Interpreter Finished\n");

 //   free_ops();

    return 0;
}
