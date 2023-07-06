#ifndef CREXX_RXVMINTP_H
#define CREXX_RXVMINTP_H

#include "rxas.h"
#include "rxbin.h"

#define rxversion "crexx f0019"

#define SMALLEST_STRING_BUFFER_LENGTH 32

typedef struct value value;

typedef union {
    /* todo - these flag definitions are not used and are not correct */
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
    char *binary_value;
    size_t binary_length;
    value **attributes;
    value **unlinked_attributes;
    value **attribute_buffers;
    size_t max_num_attributes;
    size_t num_attributes;
    size_t num_attribute_buffers;
    char small_string_buffer[SMALLEST_STRING_BUFFER_LENGTH];
};

/* Module Structure */
typedef struct module {
    bin_space segment;         /* Binary and Constant Pool */
    char *name;                /* Module Name */
    char *description;         /* Module Description */
    value **globals;           /* Globals registers array */
    int proc_head;             /* Offset to the head of the procs in the constant pool */
    int expose_head;           /* Offset to the head of the exposed procs in the constant pool */
    int meta_head;             /* Offset to the head of the meta data in the constant pool */
    char *globals_dont_free;   /* Indicates linked global value that should not be freed */
    size_t module_number;      /* Module Index - 1 base */
    size_t unresolved_symbols; /* Number of symbols not yet resolved by linking */
    size_t duplicated_symbols; /* Number of duplicated symbols ignored in module */
    module_file *file;         /* File section the module was loaded from */
} module;

struct stack_frame {
    stack_frame *prev_free;
    stack_frame *parent;
    proc_constant *procedure;
    void *return_inst;
    bin_code *return_pc;
    char is_interrupt;
    value *return_reg;
    size_t number_locals;
    size_t nominal_number_locals;
    size_t number_args;
    char interrupt_mask;
    value **baselocals; /* Initial / base / fixed local pointers */
    value **locals;   /* Locals pointer mapping (after swaps / links */
};

#ifdef NDEBUG  // RELEASE
    #define DEBUG(...) (void)0
#else          // DEBUG
    #define DEBUG(...) if (context->debug_mode) fprintf(stderr, __VA_ARGS__)
#endif

#define RXERROR(...)   { fprintf(stderr, __VA_ARGS__); goto SIGNAL; }
#define MAP_ADDR(instr, op1, op2, op3, target, msg)             \
                instruction = src_inst(instr, op1,op2,op3);     \
                address_map[instruction->opcode] = target;

#ifdef NTHREADED

#define START_OF_INSTRUCTIONS CASE_START:; switch ((enum instructions)(pc->instruction.opcode)) {
#define END_OF_INSTRUCTIONS default: goto UNKNOWN_INSTRUCTION; }
#define START_INSTRUCTION(inst) case INST_ ## inst:
#define START_BREAKPOINT BREAKPOINT:
#define END_BREAKPOINT goto CASE_START;
#define CALC_DISPATCH(n)           { next_pc = pc + (n) + 1; }
#define CALC_DISPATCH_MANUAL
#define DISPATCH                   { pc = next_pc; goto *(current_frame->interrupt_mask)?&&BREAKPOINT:&&CASE_START; }

#else

#define START_OF_INSTRUCTIONS
#define END_OF_INSTRUCTIONS
#define START_INSTRUCTION(inst) inst:
#define START_BREAKPOINT BREAKPOINT:
#define END_BREAKPOINT goto *next_inst;
#define CALC_DISPATCH(n)           { next_pc = pc + (n) + 1; next_inst = (next_pc)->impl_address; }
#define CALC_DISPATCH_MANUAL       { next_inst = (next_pc)->impl_address; }
#define DISPATCH                   { pc = next_pc; goto *(current_frame->interrupt_mask)?&&BREAKPOINT:next_inst; }
#endif

#define REG_OP(n)                    current_frame->locals[(pc+(n))->index]
#define REG_VAL(n)                   current_frame->locals[n]
#define REG_IDX(n)                   (pc+(n))->index
#define INT_OP(n)                    (pc+(n))->iconst
#define FLOAT_OP(n)                  (pc+(n))->fconst

#define CONSTSTRING_OP(n)            ((string_constant *)(current_frame->procedure->binarySpace->const_pool + (pc+(n))->index))
#define PROC_OP(n)                   ((proc_constant *)(current_frame->procedure->binarySpace->const_pool + (pc+(n))->index))
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
        else if ((v)->status.type_string) S2FLOAT(i,v);                                               \
                                                                                                      \
// Get Character
#ifndef NUTF8
  #define GETSTRCHAR(c,v,p) {string_set_byte_pos((v),(p)); utf8codepoint((v)->string_value+(v)->string_pos, &(c));}
#else
  #define GETSTRCHAR(c,v,p) {c=(v)->string_value[(p)]; }
#endif

#ifndef NUTF8
  #define GETSTRLEN(i,v)   { i = (rxinteger) v->string_chars; }
#else
  #define GETSTRLEN(i,v)   { i = (rxinteger) v->string_length; }
#endif

#ifndef NUTF8
  #define PUTSTRLEN(v,i)   { v->string_length=i; v->string_chars=i;}
#else
  #define PUTSTRLEN(v,i)   { v->string_length=i; }
#endif



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

/* Runtime context */
typedef struct rxvm_context {
    char *location;
    size_t num_modules;
    size_t module_buffer_size;
    module **modules;
    struct avl_tree_node *exposed_proc_tree;
    struct avl_tree_node *exposed_reg_tree;
    char debug_mode;
} rxvm_context;

/* Signals an error - this function does not return */
void dosignal(int code);

int initialz();
int finalize();
int run(rxvm_context *context, int argc, char *argv[]);

/* Initialise modules context */
void rxinimod(rxvm_context *context);

/* Free Module Context */
void rxfremod(rxvm_context *context);

/* Loads a new module
 * returns 0  - Error
 *         >0 - Last Module Number loaded (1 based) (more than one might have been loaded ...)  */
int rxldmod(rxvm_context *context, char *new_module_file);

/* Loads a module from a memory buffer
 * returns 0  - Error
 *         >0 - Last Module Number loaded (1 based) (more than one might have been loaded ...)  */
int rxldmodm(rxvm_context *context, char *buffer_start, size_t buffer_length);

/* Private structure for output to string thread */
typedef struct redirect REDIRECT;

/* Get Environment Value
 * Sets value (null terminated) (and a handle) from env variable name length name_length (not null terminated)
 * Value can be set to point to a zero length string (if the variable is not set)
 *
 * Returns 1 if value should bee free()d
 * Otherwise returns 0
 */
int getEnvVal(char **value, char *name, size_t name_length);

/*
 * - A pin, pout or perr does not need to be specified ... in this case the std streams are used.
 * - Command contains the commands string to execute
 * - rc will contain the return code from the command
 * - errorText contains a descriptive text of any error in the spawn
 *   (i.e. NOT from the executed child process). This is set if this returns
 *   a non-zero return code.
 *
 * Return codes
 *  0 - SHELLSPAWN_OK         - All OK
 *  4 - SHELLSPAWN_NOFOUND    - The command was not found
 *  5 - SHELLSPAWN_FAILURE    - Spawn failed unexpectedly (see error text for details)
*/
int shellspawn(const char *command,
               REDIRECT* pIn,
               REDIRECT* pOut,
               REDIRECT* pErr,
               value* variables,
               int *rc,
               char **errorText);

// SPAWN Error codes
#define SHELLSPAWN_OK         0
#define SHELLSPAWN_NOFOUND    4
#define SHELLSPAWN_FAILURE    5

/* Create a redirect pipe to string */
/* the redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void redr2str(value* redirect_reg, value* string_reg);

/* Create a redirect pipe to string */
/* the redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void redr2arr(value* redirect_reg, value* string_reg);

/* Create a redirect pipe from a string */
/* the redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void str2redr(value* redirect_reg, value* string_reg);

/* Create a redirect pipe from an array */
/* the redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void arr2redr(value* redirect_reg, value* string_reg);

/* Create a null redirect pipe */
/* In general, he redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void nullredr(value* redirect_reg);

#endif //CREXX_RXVMINTP_H
