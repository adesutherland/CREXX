#ifndef CREXX_RX_INTRP_H
#define CREXX_RX_INTRP_H

#define ERROR(...) {fprintf(stderr, __VA_ARGS__); goto SIGNAL;}

#ifdef NDEBUG  // RELEASE
    #define DEBUG(...)
    #define MAP_ADDR(instr, op1, op2, op3, target, msg)         \
                instruction = src_inst(instr, op1,op2,op3);     \
                address_map[instruction->opcode] = target;
#else          // DEBUG
    #define DEBUG(...) fprintf(stderr, __VA_ARGS__)
    #define MAP_ADDR(instr, op1, op2, op3, target, msg)         \
                instruction = src_inst(instr, op1,op2,op3);     \
                if (instruction) {                              \
                    address_map[instruction->opcode] = target;  \
                } else {                                        \
                    DEBUG(msg);                                 \
                }
#endif

typedef struct bin_space program;

/* Signals an error - this function does not return */
void dosignal(int code);

int initialize();
int finalize();
int run(program *program, int argc, char *argv[]);

#endif //CREXX_RX_INTRP_H
