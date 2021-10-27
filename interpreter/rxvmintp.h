#ifndef CREXX_RXVMINTP_H
#define CREXX_RXVMINTP_H

#include "rxas.h"

#define rxversion "cREXX I0159"

#define SMALLEST_STRING_BUFFER_LENGTH 32

typedef struct value value;

typedef union {
    struct {
        unsigned int type_object : 1;
        unsigned int type_string : 1;
        unsigned int type_decimal : 1;
        unsigned int type_float : 1;
        unsigned int type_int : 1;
    };
    unsigned int all_type_flags;
} value_type;

struct value {
    /* bit field to store value status - these are explicitly set (not automatic at all) */
    value_type status;

    /* Value */
    rxinteger int_value;
    double float_value;
    void *decimal_value; /* TODO */
    char *string_value;
    size_t string_length;
    size_t string_buffer_length;
    size_t string_pos;
#ifndef NUTF8
    size_t string_chars;
    size_t string_char_pos;
#endif
    void *object_value;

    /*
     * Each value can either be owned by a stack frame or a variable pool
     * The owner is responsible for freeing the value, but because value can
     * be linked other stack frames and variable pools it is important that
     * none of these free its memory when they are being freed themselves.
     * The owner member is only used by parents to see if they are the real
     * parent - if you like a paternity test!
     * This also allows a value to be adopted by another parent (e.g. for a
     * returned register from a procedure)
     */
    void *owner;
};

#ifdef NDEBUG  // RELEASE
    #define DEBUG(...) (void)0
#else          // DEBUG
    #define DEBUG(...) if (debug_mode) fprintf(stderr, __VA_ARGS__)
#endif

#define RXERROR(...)   { fprintf(stderr, __VA_ARGS__); goto SIGNAL; }
#define MAP_ADDR(instr, op1, op2, op3, target, msg)             \
                instruction = src_inst(instr, op1,op2,op3);     \
                address_map[instruction->opcode] = target;

#ifdef NTHREADED

#define START_OF_INSTRUCTIONS CASE_START:; switch ((enum instructions)(pc->instruction.opcode)) {
#define END_OF_INSTRUCTIONS }
#define START_INSTRUCTION(inst) case INST_ ## inst:
#define CALC_DISPATCH(n)           { next_pc = pc + (n) + 1; }
#define CALC_DISPATCH_MANUAL
#define DISPATCH                   { pc = next_pc; goto CASE_START; }

#else

#define START_OF_INSTRUCTIONS
#define END_OF_INSTRUCTIONS
#define START_INSTRUCTION(inst) inst:
#define CALC_DISPATCH(n)           { next_pc = pc + (n) + 1; next_inst = (next_pc)->impl_address; }
#define CALC_DISPATCH_MANUAL       { next_inst = (next_pc)->impl_address; }
#define DISPATCH                   { pc = next_pc; goto *next_inst; }

#endif

#define REG_OP(n)                    current_frame->locals[(pc+(n))->index]
#define REG_VAL(n)                   current_frame->locals[n]
#define REG_IDX(n)                   (pc+(n))->index
#define INT_OP(n)                    (pc+(n))->iconst
#define FLOAT_OP(n)                  (pc+(n))->fconst

#define CONSTSTRING_OP(n)            (string_constant *)(current_frame->module->const_pool + (pc+(n))->index)
#define PROC_OP(n)                   (proc_constant *)(current_frame->module->const_pool + (pc+(n))->index)
#define INT_VAL(vx)                  vx->int_value
#define FLOAT_VAL(vx)                vx->float_value

//
// PEJ Macros   April 2021

#define REG_RETURN_INT(val)        { set_int(REG_OP(1),val);}
#define REG_RETURN_FLOAT(val)      { set_float(REG_OP(1),val);  }
#define REG_RETURN_STRING(val)     { set_const_string(REG_OP(1), val);}

#define REG_RET_CHAR(val)          { v1=REG_OP(1); if (v1) set_char(v1,val);                               \
                                    else REG_OP(1) = value_char_f(current_frame,val); }
// TODO: String to integer just for real integers, or stop converting at "."
// maximum size of rxinteger is 20 digits plus sign
// maximum size of double is about 16 decimal digits plus sign

#define S2INT(t,s)                 { if ((s)->string_length>20)  goto convlength;                       \
                                    (s)->string_value[(s)->string_length]='\0';                         \
                                    (t) = strtol((s)->string_value, &converr, 0);                         \
                                    if (converr[0] != '\0' && converr[0]!='.') goto converror; }

#define S2FLOAT(t,s)              { if ((s)->string_length>16)  goto convlength;                       \
                                    (s)->string_value[(s)->string_length]='\0';                         \
                                    (t) = strtod((s)->string_value, &converr);                          \
                                    if (converr[0] != '\0') goto converror; }

#define CONV2INT(i,v)             { if ((v)->status.type_float)                                      \
                                    (i) = (rxinteger) (v)->float_value;                                 \
                                    else if ((v)->status.type_string) S2INT(i,v); }

#define CONV2FLOAT(i,v) if ((v)->status.type_int) (i) = (double) (v)->int_value;                      \
        else if ((v)->status.type_string) S2FLOAT(i,v);

// TODO PEJ what kind of checks must be performed in runtime/debug mode
#define REG_TEST(v)            { if (!(v)) goto notreg; }
#define op1R                     (REG_OP(1))
#define op2R                     (REG_OP(2))
#define op3R                     (REG_OP(3))
#define op1I                     (INT_OP(1))
#define op2I                     (INT_OP(2))
#define op3I                     (INT_OP(3))
#define op1F                     (FLOAT_OP(1))
#define op2F                     (FLOAT_OP(2))
#define op3F                     (FLOAT_OP(3))
#define op1S                     (CONSTSTRING_OP(1))
#define op2S                     (CONSTSTRING_OP(2))
#define op3S                     (CONSTSTRING_OP(3))
#define op1RI                    (INT_VAL(op1R))
#define op2RI                    (INT_VAL(op2R))
#define op3RI                    (INT_VAL(op3R))
#define op2RF                    (FLOAT_VAL(op2R))
#define op3RF                    (FLOAT_VAL(op3R))
#define REG_OP_TEST(v,n)        { (v) = REG_OP(n);}
//#define REG_OP_TEST_INT(v,n)   { (v) = REG_OP(n); REG_TEST(v);                                      \
//                                 if ((v)->status.type_int==0)  goto notint; }
//#define REG_OP_TEST_FLOAT(v,n)  { (v) = REG_OP(n); REG_TEST(v);                                      \
//                                   if ((v)->status.type_float==0)  goto notfloat; }
#define REG_OP_TEST_INT(v,n)    { (v) = REG_OP(n);}
#define REG_OP_TEST_FLOAT(v,n)  { (v) = REG_OP(n);}


/* Module Structure */
typedef struct module {
        bin_space segment;
        char *name;
        value **globals;
} module;

/* Signals an error - this function does not return */
void dosignal(int code);

int initialz();
int finalize();
int run(int num_modules, module *program, int argc, char *argv[], int debug_mode);

#endif //CREXX_RXVMINTP_H
