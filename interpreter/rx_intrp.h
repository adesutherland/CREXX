#ifndef CREXX_RX_INTRP_H
#define CREXX_RX_INTRP_H

#ifdef NDEBUG  // RELEASE
    #define DEBUG(...) (void)0
#else          // DEBUG
    #define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#endif

#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); goto SIGNAL; } while(0)
#define MAP_ADDR(instr, op1, op2, op3, target, msg)             \
                instruction = src_inst(instr, op1,op2,op3);     \
                address_map[instruction->opcode] = target;

#define CALC_DISPATCH(n)        do { next_pc = pc + (n) + 1; next_inst = (next_pc)->impl_address; }     while(0)
#define CALC_DISPATCH_MANUAL    do { next_inst = (next_pc)->impl_address; }                             while(0)
#define DISPATCH                do { pc = next_pc; goto *next_inst; }                                   while(0)
#define REG_OP(n)                    current_frame->locals[(pc+(n))->index]
#define REG_VAL(n)                   current_frame->locals[n]
#define REG_IDX(n)                   (pc+(n))->index
#define INT_OP(n)                    (pc+(n))->iconst
#define FLOAT_OP(n)                  (pc +(n))->fconst

#define CONSTSTRING_OP(n)            (string_constant *)(program->const_pool + (pc+(n))->index)
#define PROC_OP(n)                   (proc_constant *)(program->const_pool + (pc+(n))->index)
#define INT_VAL(vx)                  vx->int_value

//
// PEJ Macros   April 2021
#define REG_RET_INT(val)        do { v1=REG_OP(1); if (v1) set_int(v1,val);                             \
                                    else REG_OP(1) = value_int_f(current_frame,val); }                  while(0)

#define REG_RET_FLOAT(val)      do { v1=REG_OP(1); if (v1) set_float(v1,val);                           \
                                    else REG_OP(1) = value_float_f(current_frame, val); }               while(0)

#define REG_RET_STRING(val)     do { v1=REG_OP(1); if (v1) set_conststring(value val);                  \
                                    else REG_OP(1) = value_fFstringfloat_f(current_frame,val); }        while(0)

#define REG_RET_CHAR(val)       do { v1=REG_OP(1); if (v1) set_char(v1,val);                            \
                                    else REG_OP(1) = value_char_f(current_frame,val); }                 while(0)

// TODO: String to integer just for real integers, or stop converting at "."
// maximum size of long long is 20 digits plus sign
// maximum size of double is about 16 decimal digits plus sign

#define S2INT(t,s)              do { if ((s)->string_length>20)  goto convlength;                       \
                                    (s)->string_value[(s)->string_length]='\0';                         \
                                    (t) = strtol((s)->string_value, &converr, 0);                         \
                                    if (converr[0] != '\0' && converr[0]!='.') goto converror; }        while(0)

#define S2FLOAT(t,s)            do { if ((s)->string_length>16)  goto convlength;                       \
                                    (s)->string_value[(s)->string_length]='\0';                         \
                                    (t) = strtod((s)->string_value, &converr);                          \
                                    if (converr[0] != '\0') goto converror; }                           while(0)

#define CONV2INT(i,v)           do { if ((v)->status.primed_float)                                      \
                                    (i) = (long long) (v)->float_value;                                 \
                                    else if ((v)->status.primed_string) S2INT(i,v); }                   while(0)

#define CONV2FLOAT(i,v) if ((v)->status.primed_int) (i) = (double) (v)->int_value;                      \
        else if ((v)->status.primed_string) S2FLOAT(i,v);

// TODO PEJ what kind of checks must be performed in runtime/debug mode
#define REG_TEST(v)             do { if (!(v)) goto notreg; }                                           while(0)
#define REG_OP_TEST(v,n)        do { (v) = REG_OP(n); REG_TEST(v); }                                    while(0)
#define REG_OP_TEST_INT(v,n)    do { (v) = REG_OP(n); REG_TEST(v);                                      \
                                    if ((v)->status.primed_int==0)  goto notint; }                      while(0)
#define REG_OP_TEST_FLOAT(v,n)  do { (v) = REG_OP(n); REG_TEST(v);                                      \
                                    if ((v)->status.primed_float==0)  goto notfloat; }                  while(0)



typedef struct bin_space program;

/* Signals an error - this function does not return */
void dosignal(int code);

int initialize();
int finalize();
int run(program *program, int argc, char *argv[]);

#endif //CREXX_RX_INTRP_H
