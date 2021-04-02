//
// Created by adria on 29/03/2021.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxas.h"
#include "operands.h"
#include "rx_vars.h"

#include <stdarg.h>

//#define NDEBUG

#ifndef NDEBUG
#define print_debug(...) printf_err(__VA_ARGS__)
#else
#define print_debug(...) (void)0
#endif

void printf_err(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

/* Signals an error - this function does not return */
void dosignal(int code) {
    printf("signal %d\n", code);
    // TODO longjmp()
    exit(-1);
}

typedef struct stack_frame stack_frame;

struct stack_frame {
    stack_frame *parent;
    void *return_inst;
    bin_code *return_pc;
    size_t number_locals;
//    var_pool pool;
    value *locals[1]; /* Must be last member */
};

/* Macros */
#define CALC_DISPATCH(n) {next_pc = pc + (n) + 1; next_inst = (next_pc)->impl_address;}
#define CALC_DISPATCH0 {next_pc = pc; next_inst = (next_pc)->impl_address;}
#define DISPATCH {pc = next_pc; goto *next_inst;}
#define REG_OP(n) current_frame->locals[(pc+(n))->index]
#define REG_IDX(n) (pc+(n))->index
#define INT_OP(n) (pc+(n))->iconst
#define CONSTSTRING_OP(n)  (string_constant *)(program->const_pool + (pc+(n))->index)
#define PROC_OP(n)  (proc_constant *)(program->const_pool + (pc+(n))->index)

/* Stack Frame Factory */
stack_frame *frame_f(bin_space *program, proc_constant *procedure, int no_args,
                     stack_frame *parent, bin_code *return_pc, void* return_inst) {
    stack_frame *this;
    int num_locals;

    num_locals = procedure->locals + program->globals + no_args + 1;
    this = (stack_frame*)calloc(1,sizeof(stack_frame)
            + (sizeof(value *) * (num_locals)));
    this->parent = parent;
    this->return_inst = return_inst;
    this->return_pc = return_pc;
    this->number_locals = num_locals;

    /* TODO Globals */
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
    stack_frame *current_frame = 0, *temp_frame;
    value *v1, *v2, *v3;
    long long i1, i2, i3;
    string_constant *s1, *s2, *s3;
    proc_constant *p1, *p2, *p3;

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
    else print_debug("Instruction SAY_REG not found\n");

    instruction = src_inst("exit", OP_NONE, OP_NONE, OP_NONE);
    if (instruction) address_map[instruction->opcode] = &&EXIT;
    else print_debug("Instruction EXIT not found\n");

    instruction = src_inst("load", OP_REG, OP_INT, OP_NONE);
    if (instruction) address_map[instruction->opcode] = &&LOAD_REG_INT;
    else print_debug("Instruction LOAD_REG_INT not found\n");

    instruction = src_inst("load", OP_REG, OP_STRING, OP_NONE);
    if (instruction) address_map[instruction->opcode] = &&LOAD_REG_STRING;
    else print_debug("Instruction LOAD_REG_STRING not found\n");

    instruction = src_inst("imult", OP_REG, OP_REG, OP_REG);
    if (instruction) address_map[instruction->opcode] = &&IMULT_REG_REG_REG;
    else print_debug("Instruction IMULT_REG_REG_REG not found\n");

    instruction = src_inst("imult", OP_REG, OP_REG, OP_INT);
    if (instruction) address_map[instruction->opcode] = &&IMULT_REG_REG_INT;
    else print_debug("Instruction IMULT_REG_REG_INT not found\n");

    instruction = src_inst("iadd", OP_REG, OP_REG, OP_REG);
    if (instruction) address_map[instruction->opcode] = &&IADD_REG_REG_REG;
    else print_debug("Instruction IADD_REG_REG_REG not found\n");

    instruction = src_inst("iadd", OP_REG, OP_REG, OP_INT);
    if (instruction) address_map[instruction->opcode] = &&IADD_REG_REG_INT;
    else print_debug("Instruction IADD_REG_REG_INT not found\n");

    instruction = src_inst("call", OP_REG, OP_FUNC, OP_NONE);
    if (instruction) address_map[instruction->opcode] = &&CALL_REG_FUNC;
    else print_debug("Instruction CALL_REG_FUNC not found\n");

    instruction = src_inst("call", OP_REG, OP_FUNC, OP_REG);
    if (instruction) address_map[instruction->opcode] = &&CALL_REG_FUNC_REG;
    else print_debug("Instruction CALL_REG_FUNC_REG not found\n");

    instruction = src_inst("call", OP_FUNC, OP_NONE, OP_NONE);
    if (instruction) address_map[instruction->opcode] = &&CALL_FUNC;
    else print_debug("Instruction CALL_FUNC not found\n");

    instruction = src_inst("move", OP_REG, OP_REG, OP_NONE);
    if (instruction) address_map[instruction->opcode] = &&MOVE_REG_REG;
    else print_debug("Instruction MOVE_REG_REG not found\n");

    instruction = src_inst("ret", OP_NONE, OP_NONE, OP_NONE);
    if (instruction) address_map[instruction->opcode] = &&RET;
    else print_debug("Instruction RET not found\n");

    instruction = src_inst("ret", OP_REG, OP_NONE, OP_NONE);
    if (instruction) address_map[instruction->opcode] = &&RET_REG;
    else print_debug("Instruction RET_REG not found\n");

    instruction = src_inst("ret", OP_INT, OP_NONE, OP_NONE);
    if (instruction) address_map[instruction->opcode] = &&RET_INT;
    else print_debug("Instruction RET_INT not found\n");

    instruction = src_inst("say", OP_STRING, OP_NONE, OP_NONE);
    if (instruction) address_map[instruction->opcode] = &&SAY_STRING;
    else print_debug("Instruction SAY_STRING not found\n");
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
        print_debug("main() not found\n");
        goto interprt_finished;
    }

    current_frame = frame_f(program, procedure, argc, 0, 0, 0);
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
        print_debug("TRACE - LOAD_REG_INT R%llu %llu\n", REG_IDX(1), INT_OP(2));
        v1 = REG_OP(1);
        i2 = INT_OP(2);
        if (v1) set_int(v1,i2);
        else REG_OP(1) = value_int_f(i2);
        DISPATCH;

    LOAD_REG_STRING:
        CALC_DISPATCH(2);
        print_debug("TRACE - LOAD_REG_STRING R%llu \"%.*s\"\n", REG_IDX(1), (CONSTSTRING_OP(2))->string_len, (CONSTSTRING_OP(2))->string);

        v1 = REG_OP(1);
        s1 = CONSTSTRING_OP(2);

        if(v1) set_conststring(v1, s1);
        else REG_OP(1) = value_conststring_f(s1);

        DISPATCH;

    SAY_REG:
        CALC_DISPATCH(1);
        print_debug("TRACE - SAY_REG R%llu\n", REG_IDX(1));
        v1 = REG_OP(1);
        if (!v1) {
            print_debug("register not initialised\n");
            goto SIGNAL;
        }
        prime_string(v1);
        printf("%.*s", v1->string_length, v1->string_value);
        DISPATCH;

    SAY_STRING:
        CALC_DISPATCH(1);
        print_debug("TRACE - SAY_STRING R%llu\n", REG_IDX(1));
        s1 = CONSTSTRING_OP(1);
        printf("%.*s", s1->string_len, s1->string);
        DISPATCH;

    IMULT_REG_REG_REG:
        CALC_DISPATCH(3);
        print_debug("TRACE - IMULT_REG_REG_REG R%llu R%llu R%llu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));

        v1 = REG_OP(1);
        v2 = REG_OP(2);
        v3 = REG_OP(3);

        if (!v2 || !v3) {
            print_debug("register not initialized\n");
            goto SIGNAL;
        }

        if (v1) set_int(v1, v2->int_value * v3->int_value);
        else REG_OP(1) = value_int_f(v2->int_value * v3->int_value);

        DISPATCH;

    IMULT_REG_REG_INT:
        CALC_DISPATCH(3);
        print_debug("TRACE - IMULT_REG_REG_INT R%llu R%llu %llu\n", REG_IDX(1), REG_IDX(2), INT_OP(3));

        v1 = REG_OP(1);
        v2 = REG_OP(2);
        i3 = INT_OP(3);

        if (!v2) {
            print_debug("register not initialized\n");
            goto SIGNAL;
        }

        if (v1) set_int(v1, v2->int_value * i3);
        else REG_OP(1) = value_int_f(v2->int_value * i3);

        DISPATCH;

    IADD_REG_REG_REG:
        CALC_DISPATCH(3);
        print_debug("TRACE - IADD_REG_REG_REG R%llu R%llu R%llu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));

        v1 = REG_OP(1);
        v2 = REG_OP(2);
        v3 = REG_OP(3);

        if (!v2 || !v3) {
            print_debug("register not initialized\n");
            goto SIGNAL;
        }

        if (v1) set_int(v1, v2->int_value + v3->int_value);
        else REG_OP(1) = value_int_f(v2->int_value + v3->int_value);

        DISPATCH;

    IADD_REG_REG_INT:
        CALC_DISPATCH(3);
        print_debug("TRACE - IADD_REG_REG_INT R%llu R%llu %llu\n", REG_IDX(1), REG_IDX(2), INT_OP(3));

        v1 = REG_OP(1);
        v2 = REG_OP(2);
        i3 = INT_OP(3);

        if (!v2) {
            print_debug("register not initialized\n");
            goto SIGNAL;
        }

        if (v1) set_int(v1, v2->int_value + i3);
        else REG_OP(1) = value_int_f(v2->int_value + i3);

        DISPATCH;

    CALL_FUNC:
        CALC_DISPATCH(1);
        p1 = PROC_OP(1);
        current_frame = frame_f(program, p1, 0, current_frame, next_pc, next_inst);
        print_debug("TRACE - CALL_FUNC %s\n",p1->name);
        /* Prepare dispatch to procedure as early as possible */
        pc = &(program->binary[p1->start]);
        CALC_DISPATCH0;
        /* Arguments - none */
        current_frame->locals[program->globals + p1->locals] = value_int_f(0);
        DISPATCH;

    RET:
        print_debug("TRACE - RET\n");
        next_pc = current_frame->return_pc;
        next_inst = current_frame->return_inst;
        temp_frame = current_frame;
        current_frame = current_frame->parent;
        free_frame(temp_frame);
        DISPATCH;

    CALL_REG_FUNC:
    CALL_REG_FUNC_REG:
    MOVE_REG_REG:
    RET_REG:
    RET_INT:
    UNKNOWN:
        print_debug("ERROR - Unimplemented instruction - aborting\n");
        goto interprt_finished;

    EXIT:
    print_debug("TRACE - EXIT\n");
    goto interprt_finished;

    SIGNAL:
        print_debug("TRACE - Signal Received - aborting\n");
        goto interprt_finished;

    interprt_finished:
    if (current_frame) free_frame(current_frame); // TODO need to delete all frames ...
    printf("Interpreter Finished\n");

 //   free_ops();

    return 0;
}
