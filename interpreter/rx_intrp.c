//
// Created by adria on 29/03/2021.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
    value **return_reg;
    size_t number_locals;
//    var_pool pool;
    value *locals[1]; /* Must be last member */
};
/* Macros */

#define CALC_DISPATCH(n) {next_pc = pc + (n) + 1; next_inst = (next_pc)->impl_address;}
#define CALC_DISPATCH_MANUAL {next_inst = (next_pc)->impl_address;}
#define DISPATCH {pc = next_pc; goto *next_inst;}
#define REG_OP(n) current_frame->locals[(pc+(n))->index]

#define REG_VAL(n) current_frame->locals[n]
#define REG_IDX(n) (pc+(n))->index
#define INT_OP(n) (pc+(n))->iconst
#define FLOAT_OP(n) (pc +(n))->fconst;

#define CONSTSTRING_OP(n)  (string_constant *)(program->const_pool + (pc+(n))->index)
#define PROC_OP(n)  (proc_constant *)(program->const_pool + (pc+(n))->index)
#define INT_VAL(vx) vx->int_value

//
// PEJ Macros   April 2021
#define print_Error(ms){print_debug(ms); goto SIGNAL;}
#define REG_RET_INT(val) {v1=REG_OP(1);if (v1) set_int(v1,val); else REG_OP(1) = value_int_f(current_frame,val);}
#define REG_RET_FLOAT(val) {v1=REG_OP(1);if (v1) set_float(v1,val); else REG_OP(1) = value_float_f(current_frame, val);}
#define REG_RET_STRING(val) {v1=REG_OP(1);if (v1) set_conststring(value *val); else REG_OP(1) = value_fFstringfloat_f(current_frame,val);}

#define REG_RET_CHAR(val) {v1=REG_OP(1);if (v1) set_char(v1,val); else REG_OP(1) = value_char_f(current_frame,val);}
// TODO: String to integer just for real integers, or stop converting at "."
// maximum size of long long is 20 digits plus sign
// maximum size of double is about 16 decimal digits plus sign
#define S2INT(t,s) {if (s->string_length>20)  goto convlength; \
        s->string_value[s->string_length]='\0'; t = strtol(s->string_value, &converr, 0); \
        if (converr[0] != '\0' && converr[0]!='.') goto converror;}
#define S2FLOAT(t,s) {if (s->string_length>16)  goto convlength; \
        s->string_value[s->string_length]='\0'; t = strtod(s->string_value, &converr); \
        if (converr[0] != '\0') goto converror;}
#define CONV2INT(i,v) if (v->status.primed_float) i=(long long) v->float_value; \
        else if (v->status.primed_string) S2INT(i,v);
#define CONV2FLOAT(i,v) if (v->status.primed_int) i=(double) v->int_value; \
        else if (v->status.primed_string) S2FLOAT(i,v);

// TODO PEJ what kind of checks must be performed in runtime/debug mode
#define REG_TEST(v) if (!v) goto notreg;
#define REG_OP_TEST(v,n) v = REG_OP(n); REG_TEST(v);
#define REG_OP_TEST_INT(v,n) v = REG_OP(n); REG_TEST(v);   \
        if (v->status.primed_int==0)  goto notint;
#define REG_OP_TEST_FLOAT(v,n) v = REG_OP(n); REG_TEST(v); \
        if (v->status.primed_float==0)  goto notfloat;

/* Stack Frame Factory */
stack_frame *frame_f(bin_space *program, proc_constant *procedure, int no_args,
                     stack_frame *parent, bin_code *return_pc, void* return_inst,
                     value **return_reg) {
    stack_frame *this;
    int num_locals;

    num_locals = procedure->locals + program->globals + no_args + 1;
    this = (stack_frame*)calloc(1,sizeof(stack_frame)
            + (sizeof(value *) * (num_locals)));
    this->parent = parent;
    this->return_inst = return_inst;
    this->return_pc = return_pc;
    this->number_locals = num_locals;
    this->return_reg = return_reg;

    /* TODO Globals */
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
int run(bin_space *program, int argc, char *argv[]) {
    proc_constant *procedure;
    size_t i, j;
    bin_code *pc, *next_pc;
    void* next_inst;
    stack_frame *current_frame = 0, *temp_frame;
    value *v1, *v2, *v3;
    long long i1, i2, i3;
    double f1,f2,f3;
    char *converr;
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
/* ----------------------------------------------------------------------------
 * load instruction code generated from the instruction table pej 8. April 2021
 * ----------------------------------------------------------------------------
 */
    #include "instrset.h"

    /* Finished making instruction map done  - temporary approach */

    /* Thread code - simples! */
    i = 0;
    while (i < program->inst_size) {
        j = i;
        i += program->binary[i].instruction.no_ops + 1;
        program->binary[j].impl_address =
                address_map[program->binary[j].instruction.opcode];
    }

    /* Find the program's entry point
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

    current_frame = frame_f(program, procedure, argc, 0, 0, 0, 0);
    /* Arguments */
    current_frame->locals[program->globals + procedure->locals] = value_int_f(current_frame, argc);
    for (i = 0, j = program->globals + procedure->locals + 1; i<argc; i++, j++) {
        current_frame->locals[j] = value_nullstring_f(current_frame, argv[i]);
    }

    /* Start */
    next_pc = &(program->binary[procedure->start]);
    CALC_DISPATCH_MANUAL;
    DISPATCH;

    /* Instruction implementations */
    LOAD_REG_INT:
        CALC_DISPATCH(2);
        print_debug("TRACE - LOAD_REG_INT R%llu %llu\n", REG_IDX(1), INT_OP(2));
        v1 = REG_OP(1);
        i2 = INT_OP(2);
        if (v1) set_int(v1,i2);
        else REG_OP(1) = value_int_f(current_frame, i2);
        DISPATCH;

    LOAD_REG_STRING:
        CALC_DISPATCH(2);
        print_debug("TRACE - LOAD_REG_STRING R%llu \"%.*s\"\n", REG_IDX(1), (CONSTSTRING_OP(2))->string_len, (CONSTSTRING_OP(2))->string);

        v1 = REG_OP(1);
        s1 = CONSTSTRING_OP(2);

        if(v1) set_conststring(v1, s1);
        else REG_OP(1) = value_conststring_f(current_frame, s1);

        DISPATCH;

    SAY_REG:
        CALC_DISPATCH(1);
        print_debug("TRACE - SAY_REG R%llu\n", REG_IDX(1));
        v1 = REG_OP(1);
        if (!v1) print_Error("register not initialised\n");
        prime_string(v1);
        printf("%.*s", v1->string_length, v1->string_value);
        DISPATCH;

    SAY_STRING:
        CALC_DISPATCH(1);
        print_debug("TRACE - SAY_STRING constant index 0x%x\n", (pc+1)->index);
        s1 = CONSTSTRING_OP(1);
        printf("%.*s", s1->string_len, s1->string);
        DISPATCH;

    IMULT_REG_REG_REG:
        CALC_DISPATCH(3);
        print_debug("TRACE - IMULT_REG_REG_REG R%llu R%llu R%llu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));

        v1 = REG_OP(1);
        v2 = REG_OP(2);
        v3 = REG_OP(3);

        if (!v2 || !v3) print_Error("register not initialized\n");

        if (v1) set_int(v1, v2->int_value * v3->int_value);
        else REG_OP(1) = value_int_f(current_frame, v2->int_value * v3->int_value);

        DISPATCH;

    IMULT_REG_REG_INT:
    {
        CALC_DISPATCH(3);
        print_debug("TRACE - IMULT_REG_REG_INT R%llu R%llu %llu\n", REG_IDX(1), REG_IDX(2), INT_OP(3));

        v1 = REG_OP(1);
        v2 = REG_OP(2);
        i3 = INT_OP(3);
        if (!v2) print_Error("register not initialized\n");

        if (v1) set_int(v1, v2->int_value * i3);
        else
            REG_OP(1) = value_int_f(current_frame, v2->int_value * i3);

        DISPATCH;
    }
    IADD_REG_REG_REG:
        CALC_DISPATCH(3);
        print_debug("TRACE - IADD_REG_REG_REG R%llu R%llu R%llu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));

        v1 = REG_OP(1);
        v2 = REG_OP(2);
        v3 = REG_OP(3);

        if (!v2 || !v3) print_Error("register not initialized\n");

        if (v1) set_int(v1, v2->int_value + v3->int_value);
        else REG_OP(1) = value_int_f(current_frame, v2->int_value + v3->int_value);

        DISPATCH;

    ISUB_REG_REG_REG:
        CALC_DISPATCH(3);
        print_debug("TRACE - ISUB_REG_REG_REG R%llu R%llu R%llu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));

        v1 = REG_OP(1);
        v2 = REG_OP(2);
        v3 = REG_OP(3);

        if (!v2 || !v3) print_Error("register not initialized\n");

        if (v1) set_int(v1, v2->int_value - v3->int_value);
        else REG_OP(1) = value_int_f(current_frame, v2->int_value - v3->int_value);
        DISPATCH;

    IADD_REG_REG_INT:
        CALC_DISPATCH(3);
        print_debug("TRACE - IADD_REG_REG_INT R%llu R%llu %llu\n", REG_IDX(1), REG_IDX(2), INT_OP(3));

        v1 = REG_OP(1);
        v2 = REG_OP(2);
        i3 = INT_OP(3);

        if (!v2) print_Error("register not initialized\n");

        if (v1) set_int(v1, v2->int_value + i3);
        else REG_OP(1) = value_int_f(current_frame, v2->int_value + i3);

        DISPATCH;

    CALL_FUNC:
        CALC_DISPATCH(1);
        p1 = PROC_OP(1); /* This is the target */

        /* New stackframe */
        current_frame = frame_f(program, p1, 0, current_frame, next_pc,
                                next_inst, 0);
        print_debug("TRACE - CALL_FUNC %s()\n",p1->name);
        /* Prepare dispatch to procedure as early as possible */
        next_pc = &(program->binary[p1->start]);
        CALC_DISPATCH_MANUAL;

        /* Arguments - none */
        current_frame->locals[program->globals + p1->locals] = value_int_f(current_frame, 0);
        /* This gotos the start of the called proceure */
        DISPATCH;

    CALL_REG_FUNC:
        CALC_DISPATCH(2);
        v1 = REG_OP(1);
        p2 = PROC_OP(2); /* This is the target */

        /* Clear target return value register */
        free_value(current_frame, v1);
        REG_OP(1) = 0;

        /* New stackframe */
        current_frame = frame_f(program, p2, 0, current_frame, next_pc,
                                next_inst, &(REG_OP(1)));
        print_debug("TRACE - CALL_REG_FUNC R%llu=%s()\n", REG_IDX(1), p2->name);

        /* Prepare dispatch to procedure as early as possible */
        next_pc = &(program->binary[p2->start]);
        CALC_DISPATCH_MANUAL;

        /* Arguments - none */
        current_frame->locals[program->globals + p2->locals] = value_int_f(current_frame, 0);
        /* This gotos the start of the called procedure */
        DISPATCH;

    CALL_REG_FUNC_REG:
        CALC_DISPATCH(3);
        v1 = REG_OP(1);
        p2 = PROC_OP(2); /* This is the target */
        v3 = REG_OP(3);

        if (!v3 || !v3->status.primed_int) print_Error("ERROR: CALL_REG_FUNC_REG Arg Reg not an integer");

        /* Clear target return value register */
        free_value(current_frame, v1);
        REG_OP(1) = 0;

        /* New stackframe */
        current_frame = frame_f(program, p2, v3->int_value, current_frame, next_pc,
                                next_inst, &(REG_OP(1)));
        print_debug("TRACE - CALL_REG_FUNC_REG R%llu=%s(R%llu...)\n", REG_IDX(1),
                    p2->name, REG_IDX(3));

        /* Prepare dispatch to procedure as early as possible */
        next_pc = &(program->binary[p2->start]);
        CALC_DISPATCH_MANUAL;

        /* Arguments - complex lets never have to change this code! */
        current_frame->locals[program->globals + p2->locals] =
                current_frame->parent->locals[(pc + (3))->index];
        for (i=0; i<v3->int_value; i++) {
            current_frame->locals[program->globals + p2->locals + i + 1] =
                    current_frame->parent->locals[(pc + (3))->index + i + 1];
        }

        /* This gotos the start of the called procedure */
        DISPATCH;

    RET:
        CALC_DISPATCH(0);
        print_debug("TRACE - RET\n");
        /* Where we return to */
        next_pc = current_frame->return_pc;
        next_inst = current_frame->return_inst;
        /* back to the parents stack frame */
        temp_frame = current_frame;
        current_frame = current_frame->parent;
        if (!current_frame) print_Error("ERROR - Past top of procedure stack - aborting\n");

       free_frame(temp_frame);
        DISPATCH;

    RET_REG:
        CALC_DISPATCH(1);
        print_debug("TRACE - RET_REG\n");
        v1 = REG_OP(1);
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
        if (!current_frame) print_Error("ERROR - Past top of procedure stack - aborting\n");

        free_frame(temp_frame);
        DISPATCH;

    RET_INT:
        CALC_DISPATCH(1);
        print_debug("TRACE - RET_INT\n");
        i1 = INT_OP(1);
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
        if (!current_frame) print_Error("ERROR - Past top of procedure stack - aborting\n");

        free_frame(temp_frame);
        DISPATCH;
/* ------------------------------------------------------------------------------------
 *  RET_FLOAT                                                        pej 12. April 2021
 *  -----------------------------------------------------------------------------------
 */
    RET_FLOAT:
    CALC_DISPATCH(1);
    print_debug("TRACE - RET_FLOAT\n");

    f1 = FLOAT_OP(1);
    /* Where we return to */
    next_pc = current_frame->return_pc;
    next_inst = current_frame->return_inst;
    /* Set the result register */
    if (current_frame->return_reg)
        *(current_frame->return_reg) = value_float_f(current_frame->parent,f1);
    /* back to the parents stack frame */
    temp_frame = current_frame;
    current_frame = current_frame->parent;
    if (!current_frame) print_Error("ERROR - Past top of procedure stack - aborting\n");

    free_frame(temp_frame);
    DISPATCH;
    /* ------------------------------------------------------------------------------------
    *  RET_FLOAT                                                        pej 12. April 2021
    *  -----------------------------------------------------------------------------------
    */
    RET_STRING:
    CALC_DISPATCH(1);
    print_debug("TRACE - RET_FLOAT\n");

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
    if (!current_frame) print_Error("ERROR - Past top of procedure stack - aborting\n");

    free_frame(temp_frame);
    DISPATCH;


    MOVE_REG_REG:
        CALC_DISPATCH(2);
        print_debug("TRACE - MOVE_REG_REG R%llu R%llu\n", REG_IDX(1), REG_IDX(2));

        v1 = REG_OP(1);

        /* v1 needs to be deallocated */
        free_value(current_frame, v1);

        /* Now move the register; if op2 is null, well so be it, no harm done */
        REG_OP(1) = REG_OP(2);
        REG_OP(2) = 0;

        DISPATCH;

    DEC0:
        /* TODO This is really idec0 - i.e. it does not prime the int */
        CALC_DISPATCH(0);
        print_debug("TRACE - DEC0\n", REG_IDX(0));
        (current_frame->locals[0]->int_value)--;
        (current_frame->locals[0]->status.all_flags) = 0;
        (current_frame->locals[0]->status.primed_int) = 1;
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DEC1   R1--                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
    DEC1:
    /* TODO This is really idec1 - i.e. it does not prime the int */
        CALC_DISPATCH(0);
        print_debug("TRACE - DEC1\n", REG_IDX(1));
        (current_frame->locals[1]->int_value)--;
        (current_frame->locals[1]->status.all_flags) = 0;
        (current_frame->locals[1]->status.primed_int) = 1;
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DEC2   R2--                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
    DEC2:
    /* TODO This is really idec2 - i.e. it does not prime the int */
        CALC_DISPATCH(0);
        print_debug("TRACE - DEC2\n", REG_IDX(2));
        (current_frame->locals[2]->int_value)--;
        (current_frame->locals[2]->status.all_flags) = 0;
        (current_frame->locals[2]->status.primed_int) = 1;
    DISPATCH;

    DEC_REG:
        /* TODO This is really idec reg - i.e. it does not prime the int */
        CALC_DISPATCH(1);
        print_debug("TRACE - DEC_REG R%llu\n", REG_IDX(1));
        (current_frame->locals[REG_IDX(1)]->int_value)--;
        (current_frame->locals[REG_IDX(1)]->status.all_flags) = 0;
        (current_frame->locals[REG_IDX(1)]->status.primed_int) = 1;

    DISPATCH;

    BR_ID:
        next_pc = program->binary + REG_IDX(1);
        CALC_DISPATCH_MANUAL;
        DISPATCH;

    /* For these we optimise for condition to NOT be met because in a loop
     * these ae used to jump out of the loop when the end condition it met
     * (and every little bit helps to improve performance!)
     */
    BRT_ID_REG:
        CALC_DISPATCH(2); /* i.e. if the condition is not met - this helps the
                                the real CPUs branch prediction (in theory) */
        if (REG_OP(2)->int_value) {
            next_pc = program->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL;
        }
        DISPATCH;

    BRF_ID_REG:
        CALC_DISPATCH(2); /* i.e. if the condition is not met - this helps the
                                  the real CPUs branch prediction (in theory) */
        print_debug("TRACE - BRF_ID_REG\n", REG_IDX(1));
        if (!(REG_OP(2)->int_value)) {
            next_pc = program->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL;
        }
        DISPATCH;

    IMASTER_REG:
        CALC_DISPATCH(1);
        print_debug("TRACE - IMASTER_REG R%llu\n", REG_IDX(1));
        v1 = REG_OP(1);
        master_int(v1);
        DISPATCH;

    TIME_REG:
        CALC_DISPATCH(1);
        print_debug("TRACE - TIME R%llu\n", REG_IDX(1));
        v1 = REG_OP(1);
        if (v1) set_int(v1,time(NULL));
        else REG_OP(1) = value_int_f(current_frame, time(NULL));
        DISPATCH;
/* ------------------------------------------------------------------------------------
 *  TRIMR  Trim right                                                 pej 7. April 2021
 * ------------------------------------------------------------------------------------
 */
    TRIMR_REG_REG:
      CALC_DISPATCH(2);
        print_debug("TRACE - TRIMR_REG_REG\n") ;
        v1 = REG_OP(1);
        REG_TEST(v1);
        prime_string(v1);

        i = v1->string_length - 1;
        while (i >= 0 && v1->string_value[i] == ' ') {
            v1->string_value[i] = '\0';
            i--;
        }
        v1->string_length=i+1;
        REG_OP(1)=v1;
      DISPATCH;
/* ------------------------------------------------------------------------------------
 *  TRIML  Trim left                                                  pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
    TRIML_REG_REG:
      CALC_DISPATCH(2);
        print_debug("TRACE - TRIML_REG_REG\n") ;
        v1 = REG_OP(1);
        REG_TEST(v1);
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
   INC0:
      CALC_DISPATCH(0);
        print_debug("TRACE - INC0\n");
        REG_VAL(0)->int_value++;
        (current_frame->locals[0]->int_value)++;
        (current_frame->locals[0]->status.all_flags) = 0;
        (current_frame->locals[0]->status.primed_int) = 1;

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  INC1   R1++                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
  INC1:
    CALC_DISPATCH(0);
      print_debug("TRACE - INC1\n", REG_IDX(1));
      (current_frame->locals[1]->int_value)++;
      (current_frame->locals[1]->status.all_flags) = 0;
      (current_frame->locals[1]->status.primed_int) = 1;
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  INC2   R2++                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
  INC2:
    CALC_DISPATCH(0);
      print_debug("TRACE - INC2\n", REG_IDX(2));
      (current_frame->locals[2]->int_value)++;
      (current_frame->locals[2]->status.all_flags) = 0;
      (current_frame->locals[2]->status.primed_int) = 1;
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ISUB_REG_REG_INT: Integer Subtract (op1=op2-op3)               pej 8. April 2021
 *  -----------------------------------------------------------------------------------
 */
  ISUB_REG_REG_INT:
    CALC_DISPATCH(3);
    print_debug("TRACE - ISUB_REG_REG_INT\n") ;
    REG_OP_TEST(v2,2);     // value in v2
    i3 = INT_OP(3);

    REG_RET_INT(v2->int_value+i3);  // return in first register
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IEQ_REG_REG_REG  Int Equals op1=(op2==op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
  IEQ_REG_REG_REG:
   CALC_DISPATCH(3);
    print_debug("TRACE - ISUB_REG_REG_REG\n") ;
    REG_OP_TEST(v2,2);
    REG_OP_TEST(v3,3);

     if (INT_VAL(v2)==INT_VAL(v3)) i=1;
        else i=0;
     REG_RET_INT(i);  // return in first register
   DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IEQ_REG_REG_INT  Int Equals op1=(op2==op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    IEQ_REG_REG_INT: // label not yet defined
    CALC_DISPATCH(3);
    print_debug("TRACE - IEQ_REG_REG_INT\n") ;

    REG_OP_TEST(v2,2);
    i3 = INT_OP(3);

    if (INT_VAL(v3)==i3) i=1;
    else i=0;
    REG_RET_INT(i);  // return in first register
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  INE_REG_REG_REG  Int Equals op1=(op2!=op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    INE_REG_REG_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - INE_REG_REG_REG\n") ;
    REG_OP_TEST(v2,2);
    REG_OP_TEST(v3,3);

    if (INT_VAL(v2)!=INT_VAL(v3)) i=0;
    else i=1;
    REG_RET_INT(i);  // return in first register
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  INE_REG_REG_INT  Int Equals op1=(op2!=op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    INE_REG_REG_INT:
    CALC_DISPATCH(3);
    print_debug("TRACE - INE_REG_REG_INT\n") ;
    v2 = REG_OP_TEST(v2,2);
    i3 = INT_OP(3);
    if (INT_VAL(v3)!=i3) i=0;
    else i=1;
    REG_RET_INT(i);  // return in first register
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IGT_REG_REG_REG  Int Greater than op1=(op2>op3)                      pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    IGT_REG_REG_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - IGT_REG_REG_REG\n") ;

    REG_OP_TEST(v2,2);
    REG_OP_TEST(v3,3);

    if (INT_VAL(v2)>INT_VAL(v3)) i=1;
    else i=0;
    REG_RET_INT(i);  // return in first register
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IGT_REG_REG_INT  Int Greater than op1=(op2>op3)                      pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    IGT_REG_REG_INT:
    CALC_DISPATCH(3);
    print_debug("TRACE - IGT_REG_REG_INT\n") ;

    REG_OP_TEST(v2,2);
    i3 = INT_OP(3);

    if (INT_VAL(v2)>i3) i=1;
    else i=0;
    REG_RET_INT(i);  // return in first register
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IGT_REG_INT_REG  Int Greater than op1=(op2>op3)                      pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    IGT_REG_INT_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - IGT_REG_INT_REG\n") ;

    i2 = INT_OP(2);
    REG_OP_TEST(v3,3);

    if (i2>INT_VAL(v3)) i=1;
    else i=0;
    REG_RET_INT(i);  // return in first register
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ILT_REG_REG_REG  Int Less than op1=(op2<op3)                         pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    ILT_REG_REG_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - ILT_REG_REG_REG\n") ;

    REG_OP_TEST(v2,2);
    REG_OP_TEST(v3,3);

    if (INT_VAL(v2)<INT_VAL(v3)) i=1;
    else i=0;
    REG_RET_INT(i);  // return in first register
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ILT_REG_REG_INT  Int Less than op1=(op2<op3)                         pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    ILT_REG_REG_INT:
    CALC_DISPATCH(3);
    print_debug("TRACE - ILT_REG_REG_INT\n") ;

    REG_OP_TEST(v2,2);
    i3 = INT_OP(3);

    if (INT_VAL(v2)<i3) i=1;
    else i=0;
    REG_RET_INT(i);  // return in first register
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ILT_REG_INT_REG  Int Less than op1=(op2<op3)                         pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    ILT_REG_INT_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - ILT_REG_INT_REG\n") ;

    i2 = INT_OP(2);
    REG_OP_TEST(v3,3);

    if (i2<INT_VAL(v3)) i=1;
    else i=0;
    REG_RET_INT(i);  // return in first register
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IGTE_REG_REG_REG  Int Greater Equal than op1=(op2>=op3)              pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    IGTE_REG_REG_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - IGTE_REG_REG_REG\n") ;

    REG_OP_TEST(v2,2);
    REG_OP_TEST(v3,3);

    if (INT_VAL(v2)>=INT_VAL(v3)) i=1;
    else i=0;
    REG_RET_INT(i);  // return in first register
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IGTE_REG_REG_INT  Int Greater Equal than op1=(op2>=op3)              pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    IGTE_REG_REG_INT:
    CALC_DISPATCH(3);
    print_debug("TRACE - IGTE_REG_REG_INT\n") ;

    REG_OP_TEST(v2,2);
    i3 = INT_OP(3);

    if (INT_VAL(v2)>=i3) i=1;
    else i=0;
    REG_RET_INT(i);  // return in first register
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IGTE_REG_INT_REG  Int Greater Equal than op1=(op2>=op3)              pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    IGTE_REG_INT_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - IGTE_REG_INT_REG\n") ;

    i2 = INT_OP(2);
    REG_OP_TEST(v3,3);

    if (i2>=INT_VAL(v3)) i=1;
    else i=0;
    REG_RET_INT(i);  // return in first register
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ILTE_REG_REG_REG  Int Less Equal than op1=(op2<=op3)                 pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    ILTE_REG_REG_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - ILTE_REG_REG_REG\n") ;

    REG_OP_TEST(v2,2);
    REG_OP_TEST(v3,3);

    if (INT_VAL(v2)<=INT_VAL(v3)) i=1;
    else i=0;
    REG_RET_INT(i);  // return in first register
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ILTE_REG_REG_INT  Int Less Equal than op1=(op2<=op3)                 pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    ILTE_REG_REG_INT:
    CALC_DISPATCH(3);
    print_debug("TRACE - ILTE_REG_REG_INT\n") ;

    REG_OP_TEST(v2,2);
    i3 = INT_OP(3);

    if (INT_VAL(v2)<=i3) i=1;
    else i=0;
    REG_RET_INT(i);  // return in first register
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ILTE_REG_INT_REG  Int Less Equal than op1=(op2<=op3)                 pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    ILTE_REG_INT_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - ILTE_REG_INT_REG\n") ;

    i2 = INT_OP(2);
    REG_OP_TEST(v3,3);

    if (i2<=INT_VAL(v3)) i=1;
    else i=0;
    REG_RET_INT(i);  // return in first register
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  COPY_REG_REG  Copy op2 to op1                                       pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    COPY_REG_REG: // label not yet defined
    CALC_DISPATCH(2);
    REG_OP(1) = value_f(current_frame);
    v1 = REG_OP(1);
    v2 = REG_OP(2);
    memcpy(v1, v2, sizeof(value));
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  INC_REG  Increment Int (op1++)                                      pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    INC_REG: // label not yet defined
    CALC_DISPATCH(1);
    print_debug("TRACE - INC_REG\n") ;
    (current_frame->locals[REG_IDX(1)]->int_value)++;
    (current_frame->locals[REG_IDX(1)]->status.all_flags) = 0;
    (current_frame->locals[REG_IDX(1)]->status.primed_int) = 1;

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  IDIV_REG_REG_INT  Integer Divide (op1=op2/op3)                      pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    IDIV_REG_REG_INT:
    CALC_DISPATCH(3);
    print_debug("TRACE - IDIV_REG_REG_INT\n") ;

    REG_OP_TEST_INT(v2,2);
    i3 = INT_OP(3);
    v2->int_value=v2->int_value/i3;
    REG_RET_INT(v2->int_value);
    DISPATCH;
 /* -----------------------------------------------------------------------------------
 *  IDIV_REG_REG_REG  Integer Divide (op1=op2/op3)                      pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    IDIV_REG_REG_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - IDIV_REG_REG_REG\n") ;

    REG_OP_TEST_INT(v2,2);
    REG_OP_TEST_INT(v3,3);
    v2->int_value=v2->int_value/v3->int_value;
    REG_RET_INT(v2->int_value);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  SAY_INT  Say op1                                                    pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    SAY_INT:
    CALC_DISPATCH(1);
    print_debug("TRACE - SAY_INT\n") ;

    printf("%d", INT_OP(1));
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  SAY_CHAR  Say op1                                                   pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    SAY_CHAR:
    CALC_DISPATCH(1);
    print_debug("TRACE - SAY_INT\n") ;

    printf("%c", (pc+(1))->cconst);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  SAY_FLOAT  Say op1                                                  pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    SAY_FLOAT:
    CALC_DISPATCH(1);
    print_debug("TRACE - SAY_FLOAT\n") ;

    f1=FLOAT_OP(1);
    printf("%g", f1);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  LOAD_REG_FLOAT  Load op1 with op2                                   pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    LOAD_REG_FLOAT:
    CALC_DISPATCH(2);
    print_debug("TRACE - LOAD_REG_FLOAT\n") ;

    f2 = FLOAT_OP(2);
    REG_RET_FLOAT(f2);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FADD_REG_REG_REG  Float Add (op1=op2+op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    FADD_REG_REG_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - FADD_REG_REG_REG\n") ;

    REG_OP_TEST_FLOAT(v2,2);
    REG_OP_TEST_FLOAT(v3,3);

    REG_RET_FLOAT(v2->float_value+v3->float_value);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FSUB_REG_REG_REG  Float Sub (op1=op2-op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    FSUB_REG_REG_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - FSUB_REG_REG_REG\n") ;

    REG_OP_TEST_FLOAT(v2,2);
    REG_OP_TEST_FLOAT(v3,3);
    REG_RET_FLOAT(v2->float_value-v3->float_value);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FDIV_REG_REG_REG  Float Div (op1=op2/op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    FDIV_REG_REG_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - FDIV_REG_REG_REG\n") ;

    REG_OP_TEST_FLOAT(v2,2);
    REG_OP_TEST_FLOAT(v3,3);
    REG_RET_FLOAT(v2->float_value/v3->float_value);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FMULT_REG_REG_REG  Float Mult (op1=op2/op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    FMULT_REG_REG_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - FMULT_REG_REG_REG\n") ;

    REG_OP_TEST_FLOAT(v2,2);
    REG_OP_TEST_FLOAT(v3,3);
    REG_RET_FLOAT(v2->float_value*v3->float_value);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FADD_REG_REG_FLOAT  Float Add (op1=op2+op3)                          pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    FADD_REG_REG_FLOAT:
    CALC_DISPATCH(3);
    print_debug("TRACE - FADD_REG_REG_FLOAT\n") ;

    REG_OP_TEST_FLOAT(v2,2);
    f3 = FLOAT_OP(3);
    REG_RET_FLOAT(v2->float_value+f3);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FSUB_REG_REG_FLOAT  Float Sub (op1=op2-op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    FSUB_REG_REG_FLOAT:
    CALC_DISPATCH(3);
    print_debug("TRACE - FSUB_REG_REG_FLOAT\n") ;

    REG_OP_TEST_FLOAT(v2,2);
    f3 = FLOAT_OP(3)
    REG_RET_FLOAT(v2->float_value-f3);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FDIV_REG_REG_FLOAT  Float Div (op1=op2/op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    FDIV_REG_REG_FLOAT:
    CALC_DISPATCH(3);
    print_debug("TRACE - FDIV_REG_REG_FLOAT\n") ;

    REG_OP_TEST_FLOAT(v2,2);
    f3 = FLOAT_OP(3);
    REG_RET_FLOAT(v2->float_value/f3);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FMULT_REG_REG_FLOAT  Float Mult (op1=op2/op3)                       pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    FMULT_REG_REG_FLOAT:
    CALC_DISPATCH(3);
    print_debug("TRACE - FMULT_REG_REG_FLOAT\n") ;

    REG_OP_TEST_FLOAT(v2,2);
    f3 = FLOAT_OP(3);
    REG_RET_FLOAT(v2->float_value*f3);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FSUB_REG_FLOAT_REG  Float Sub (op1=op2-op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    FSUB_REG_FLOAT_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - FSUB_REG_FLOAT_REG\n") ;

    f2 = FLOAT_OP(2);
    REG_OP_TEST_FLOAT(v3,2);
    REG_RET_FLOAT(f2-v3->float_value);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  FDIV_REG_FLOAT_REG  Float Div (op1=op2/op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    FDIV_REG_FLOAT_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - FDIV_REG_FLOAT_REG\n") ;

    f2 = FLOAT_OP(2);
    REG_OP_TEST_FLOAT(v3,2);
    REG_RET_FLOAT(f2/v3->float_value);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  STR2INT_REG_REG_REG  String to Int op1 = op2[op3]                   pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    STR2INT_REG_REG_REG: // label not yet defined
    CALC_DISPATCH(3);
    print_debug("TRACE - STRINT_REG_REG_REG\n") ;

    v2 = REG_OP(2);
    REG_TEST(v2);
    REG_OP_TEST_INT(v3,3);
    i1=v2->string_value[v3->int_value-1]-'0';

    REG_RET_INT(i1);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ADDI_REG_REG_INT  Convert and Add to Integer (op1=op2+op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    ADDI_REG_REG_INT:
    CALC_DISPATCH(3);
    print_debug("TRACE - ADDI_REG_REG_INT");

    REG_OP_TEST(v2,2);
    CONV2INT(i2,v2)
    i3 = INT_OP(3);

    REG_RET_INT(i2+i3);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ADDI_REG_REG_REG  Convert and Add to Integer (op1=op2+op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    ADDI_REG_REG_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - ADDI_REG_REG_REG");

    REG_OP_TEST(v2,2);
    REG_OP_TEST(v3,3);
    CONV2INT(i2,v2)
    CONV2INT(i3,v3)

    REG_RET_INT(i2+i3);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  SUBI_REG_REG_REG  Convert and Subtract to Integer (op1=op2-op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    SUBI_REG_REG_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - SUBI_REG_REG_REG");

    REG_OP_TEST(v2,2);
    REG_OP_TEST(v3,3);
    CONV2INT(i2,v2)
    CONV2INT(i3,v3)

    REG_RET_INT(i2-i3);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  SUBI_REG_REG_INT  Convert and Subtract to Integer (op1=op2-op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    SUBI_REG_REG_INT:
    CALC_DISPATCH(3);
    print_debug("TRACE - SUBI_REG_REG_INT");

    REG_OP_TEST(v2,2);
    CONV2INT(i2,v2)
    i3 = INT_OP(3);

    REG_RET_INT(i2-i3);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  MULTI_REG_REG_REG  Convert and Multiply to Integer (op1=op2*op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    MULTI_REG_REG_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - MULTI_REG_REG_REG");

    REG_OP_TEST(v2,2);
    REG_OP_TEST(v3,3);
    CONV2INT(i2,v2)
    CONV2INT(i3,v3)

    REG_RET_INT(i2*i3);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  MULTI_REG_REG_INT  Convert and Multiply to Integer (op1=op2*op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    MULTI_REG_REG_INT:
    CALC_DISPATCH(3);
    print_debug("TRACE - MULTI_REG_REG_INT");

    REG_OP_TEST(v2,2);
    CONV2INT(i2,v2)
    i3 = INT_OP(3);

    REG_RET_INT(i2*i3);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DIVI_REG_REG_REG  Convert and Divide to Integer (op1=op2/op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    DIVI_REG_REG_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - DIVI_REG_REG_REG");

    REG_OP_TEST(v2,2);
    REG_OP_TEST(v3,3);
    CONV2INT(i2,v2)
    CONV2INT(i3,v3)

    REG_RET_INT(i2/i3);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DIVI_REG_REG_INT  Convert and Divide to Integer (op1=op2/op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    DIVI_REG_REG_INT:
    CALC_DISPATCH(3);
    print_debug("TRACE - DIVI_REG_REG_INT");

    REG_OP_TEST(v2,2);
    CONV2INT(i2,v2)
    i3 = INT_OP(3);

    REG_RET_INT(i2/i3);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ADDF_REG_REG_FLOAT  Convert and Add to Float (op1=op2+op3)          pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    ADDF_REG_REG_FLOAT:
    CALC_DISPATCH(3);
    print_debug("TRACE - ADDF_REG_REG_FLOAT");

    REG_OP_TEST(v2,2);
    CONV2FLOAT(f2,v2)
    f3 = FLOAT_OP(3);

    REG_RET_FLOAT(f2+f3);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  ADDF_REG_REG_REG  Convert and Add to Float (op1=op2+op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    ADDF_REG_REG_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - ADDF_REG_REG_REG");

    REG_OP_TEST(v2,2);
    REG_OP_TEST(v3,3);
    CONV2FLOAT(f2,v2)
    CONV2FLOAT(f3,v3)

    REG_RET_FLOAT(f2+f3);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  SUBF_REG_REG_REG  Convert and Subtract to Float (op1=op2-op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    SUBF_REG_REG_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - SUBF_REG_REG_REG");

    REG_OP_TEST(v2,2);
    REG_OP_TEST(v3,3);
    CONV2FLOAT(f2,v2)
    CONV2FLOAT(f3,v3)

    REG_RET_FLOAT(f2-f3);
    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  SUBF_REG_REG_FLOAT  Convert and Subtract to Float (op1=op2-op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    SUBF_REG_REG_FLOAT:
    CALC_DISPATCH(3);
    print_debug("TRACE - SUBF_REG_REG_FLOAT");

    REG_OP_TEST(v2,2);
    CONV2FLOAT(f2,v2)
    f3 = FLOAT_OP(3);

    REG_RET_FLOAT(f2-f3);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  SUBF_REG_FLOAT_REG  Convert and Subtract to Float (op1=op2-op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    SUBF_REG_FLOAT_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - SUBF_REG_FLOAT_REG");

    f2 = FLOAT_OP(2);
    REG_OP_TEST(v3,3);
    CONV2FLOAT(f3,v3)

    REG_RET_FLOAT(f2-f3);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  MULTF_REG_REG_REG  Convert and Multiply to Float (op1=op2*op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    MULTF_REG_REG_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - MULTF_REG_REG_REG");

    REG_OP_TEST(v2,2);
    CONV2FLOAT(f2,v2)
    f3 = FLOAT_OP(3);

    REG_RET_FLOAT(f2*f3);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  MULTF_REG_REG_FLOAT  Convert and Multiply to Float (op1=op2*op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    MULTF_REG_REG_FLOAT:
    CALC_DISPATCH(3);
    print_debug("TRACE - MULTF_REG_REG_FLOAT");

    REG_OP_TEST(v2,2);
    CONV2FLOAT(f2,v2)
    f3 = FLOAT_OP(3);

    REG_RET_FLOAT(f2*f3);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DIVF_REG_REG_REG  Convert and Divide to Float (op1=op2/op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    DIVF_REG_REG_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - DIVF_REG_REG_REG");

    REG_OP_TEST(v2,2);
    REG_OP_TEST(v3,3);
    CONV2FLOAT(f2,v2)
    CONV2FLOAT(f3,v3)

    REG_RET_FLOAT(f2/f3);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DIVF_REG_REG_FLOAT  Convert and Divide to Float (op1=op2/op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    DIVF_REG_REG_FLOAT:
    CALC_DISPATCH(3);
    print_debug("TRACE - DIVF_REG_REG_FLOAT");

    REG_OP_TEST(v2,2);
    CONV2FLOAT(f2,v2)
    f3 = FLOAT_OP(3);

    REG_RET_FLOAT(f2/f3);

    DISPATCH;
/* ------------------------------------------------------------------------------------
 *  DIVF_REG_FLOAT_REG  Convert and Divide to Float (op1=op2/op3)              pej 14 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
    DIVF_REG_FLOAT_REG:
    CALC_DISPATCH(3);
    print_debug("TRACE - DIVF_REG_FLOAT_REG");

    f2 = FLOAT_OP(2);
    REG_OP_TEST(v3,3);
    CONV2FLOAT(f3,v3)
    REG_RET_FLOAT(f2/f3);

    DISPATCH;

    AMAP_REG_REG:
    CALC_DISPATCH(2);
    print_debug("TRACE - AMAP_REG_REG");

    REG_OP(1) = current_frame->locals[REG_OP(2)->int_value + program->globals + procedure->locals];

    DISPATCH;

    AMAP_REG_INT:
    CALC_DISPATCH(2);
    print_debug("TRACE - AMAP_REG_INT");

    REG_OP(1) = REG_VAL(INT_OP(2));

    DISPATCH;
/* ---------------------------------------------------------------------------
 * load instructions not yet implemented generated from the instruction table
 *      and scan of this module                              pej 8. April 2021
 * ---------------------------------------------------------------------------
 */
#include "instrmiss.h"


UNKNOWN:
        printf("ERROR - Unimplemented instruction - aborting\n");
        goto interprt_finished;
notreg:
        print_debug("register not initialised\n");
        goto SIGNAL;
notint:
        print_Error("Parameter is not an integer\n");
        goto SIGNAL;
notfloat:
        print_Error("Parameter is not a float\n");
        goto SIGNAL;
convlength:
        print_debug("maximum string length exceeded\n");
        goto SIGNAL;
converror:
        print_debug("Conversion error occurred: %d\n");
        goto SIGNAL;


EXIT:
        printf("TRACE - EXIT\n");
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
