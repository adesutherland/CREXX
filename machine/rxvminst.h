// REXX Assembler
// Operands Structures
//

#ifndef CREXX_RXVMINST_H
#define CREXX_RXVMINST_H

#include "platform.h"

typedef enum OperandType
{
    OP_NONE = 0,
    OP_ID = 1,
    OP_REG = 2,
    OP_FUNC = 3,
    OP_INT = 4,
    OP_FLOAT = 5,
    OP_CHAR = 6,
    OP_STRING = 7
} OperandType;

typedef struct Instruction
{
    int opcode;
    char *instruction;
    char *desc;
    int operands;
    OperandType op1_type;
    OperandType op2_type;
    OperandType op3_type;
} Instruction;

// Load and initialise Instruction Database
void init_ops();

// Free Instruction Database
void free_ops();

// Add a instruction to the database
void instr_f(char* name, char *description,
             OperandType op1_type, OperandType op2_type, OperandType op3_type);

// Search for an instruction
Instruction *src_inst(char* name, OperandType op1_type,
                      OperandType op2_type, OperandType op3_type);

// Lookup an instruction by opcode
Instruction *get_inst(int opcode);

/* Get the first instruction with a specific instruction name (ignoring operands) */
/* returns null on no match */
Instruction *fst_inst(char* name);

/* returns the next instruction with the same instruction name (ignoring operands) */
/* returns null when there is not any more instructions with the same name */
Instruction *nxt_inst(Instruction *inst);

/* Returns the operand name as a string */
char* opd_name(OperandType type);

/* Returns the expected operands of a instruction in the provided buffer */
char* exp_opds(Instruction* inst, char* buffer, size_t buffer_len);

/* Prints the instruction database */
void prt_ops();

#endif //CREXX_RXVMINST_H
