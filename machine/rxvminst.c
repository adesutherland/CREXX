// REXX Assembler
// Operation List

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "rxvminst.h"
#include "../avl_tree/avl_tree.h"


/* Internal Tree node structure */
struct instruction_wrapper {
    Instruction* data;
    struct avl_tree_node index_node;
};

static struct avl_tree_node *instruction_root = 0;
static Instruction** instructions = 0;
static int no_instructions = 0;

// Load and initialise Instruction Database
void init_ops() {
    instr_f("iadd", "Integer Add (op1=op2+op3)", OP_REG, OP_REG, OP_REG);
    instr_f("iadd", "Integer Add (op1=op2+op3)", OP_REG, OP_REG, OP_INT);
    instr_f("addi", "Convert and Add to Integer (op1=op2+op3)", OP_REG, OP_REG, OP_REG);
    instr_f("addi", "Convert and Add to Integer (op1=op2+op3)", OP_REG, OP_REG, OP_INT);

    instr_f("isub", "Integer Subtract (op1=op2-op3)", OP_REG, OP_REG, OP_REG);
    instr_f("isub", "Integer Subtract (op1=op2-op3)", OP_REG, OP_REG, OP_INT);
    instr_f("subi", "Convert and Subtract to Integer (op1=op2-op3)", OP_REG, OP_REG, OP_REG);
    instr_f("subi", "Convert and Subtract to Integer (op1=op2-op3)", OP_REG, OP_REG, OP_INT);

    instr_f("imult", "Integer Multiply (op1=op2*op3)", OP_REG, OP_REG, OP_REG);
    instr_f("imult", "Integer Multiply (op1=op2*op3)", OP_REG, OP_REG, OP_INT);
    instr_f("multi", "Convert and Multiply to Integer (op1=op2*op3)", OP_REG, OP_REG, OP_REG);
    instr_f("multi", "Convert and Multiply to Integer (op1=op2*op3)", OP_REG, OP_REG, OP_INT);

    instr_f("idiv", "Integer Divide (op1=op2/op3)", OP_REG, OP_REG, OP_REG);
    instr_f("idiv", "Integer Divide (op1=op2/op3)", OP_REG, OP_REG, OP_INT);
    instr_f("divi", "Convert and Divide to Integer (op1=op2/op3)", OP_REG, OP_REG, OP_REG);
    instr_f("divi", "Convert and Divide to Integer (op1=op2/op3)", OP_REG, OP_REG, OP_INT);

    instr_f("fadd", "Float Add (op1=op2+op3)", OP_REG, OP_REG, OP_REG);
    instr_f("fadd", "Float Add (op1=op2+op3)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("addf", "Convert and Add to Float (op1=op2+op3)", OP_REG, OP_REG, OP_REG);
    instr_f("addf", "Convert and Add to Float (op1=op2+op3)", OP_REG, OP_REG, OP_FLOAT);

    instr_f("fsub", "Float Subtract (op1=op2-op3)", OP_REG, OP_REG, OP_REG);
    instr_f("fsub", "Float Subtract (op1=op2-op3)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("fsub", "Float Subtract (op1=op2-op3)", OP_REG, OP_FLOAT, OP_REG);
    instr_f("subf", "Convert and Subtract to Float (op1=op2-op3)", OP_REG, OP_REG, OP_REG);
    instr_f("subf", "Convert and Subtract to Float (op1=op2-op3)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("subf", "Convert and Subtract to Float (op1=op2-op3)", OP_REG, OP_FLOAT, OP_REG);

    instr_f("fmult", "Float Multiply (op1=op2*op3)", OP_REG, OP_REG, OP_REG);
    instr_f("fmult", "Float Multiply (op1=op2*op3)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("multf", "Convert and Multiply to Float (op1=op2*op3)", OP_REG, OP_REG, OP_REG);
    instr_f("multf", "Convert and Multiply to Float (op1=op2*op3)", OP_REG, OP_REG, OP_FLOAT);

    instr_f("fdiv", "Float Divide (op1=op2/op3)", OP_REG, OP_REG, OP_REG);
    instr_f("fdiv", "Float Divide (op1=op2/op3)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("fdiv", "Float Divide (op1=op2/op3)", OP_REG, OP_FLOAT, OP_REG);
    instr_f("divf", "Convert and Divide to Float (op1=op2/op3)", OP_REG, OP_REG, OP_REG);
    instr_f("divf", "Convert and Divide to Float (op1=op2/op3)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("divf", "Convert and Divide to Float (op1=op2/op3)", OP_REG, OP_FLOAT, OP_REG);

    instr_f("inc", "Increment Int (op1++)", OP_REG, OP_NONE, OP_NONE);
    instr_f("dec", "Decrement Int (op1--)", OP_REG, OP_NONE, OP_NONE);
    instr_f("inc0", "Increment R0++ Int", OP_NONE, OP_NONE, OP_NONE);
    instr_f("dec0", "Decrement R0-- Int", OP_NONE, OP_NONE, OP_NONE);
    instr_f("inc1", "Increment R0++ Int", OP_NONE, OP_NONE, OP_NONE);
    instr_f("dec1", "Decrement R0-- Int", OP_NONE, OP_NONE, OP_NONE);
    instr_f("inc2", "Increment R0++ Int", OP_NONE, OP_NONE, OP_NONE);
    instr_f("dec2", "Decrement R0-- Int", OP_NONE, OP_NONE, OP_NONE);

    instr_f("sconcat", "String Concat with space (op1=op2||op3)", OP_REG, OP_REG, OP_REG);
    instr_f("sconcat", "String Concat with space (op1=op2||op3)", OP_REG, OP_REG, OP_STRING);
    instr_f("sconcat", "String Concat with space (op1=op2||op3)", OP_REG, OP_STRING, OP_REG);
    instr_f("concat", "String Concat (op1=op2||op3)", OP_REG, OP_REG, OP_REG);
    instr_f("concat", "String Concat (op1=op2||op3)", OP_REG, OP_REG, OP_STRING);
    instr_f("concat", "String Concat (op1=op2||op3)", OP_REG, OP_STRING, OP_REG);
    instr_f("appendchar", "Append Concat Char op2 (as int) on op1", OP_REG, OP_REG, OP_NONE);

    instr_f("triml", "Trim String (op1) from Left by (op2) Chars", OP_REG, OP_REG, OP_NONE);
    instr_f("trimr", "Trim String (op1) from Right by (op2) Chars", OP_REG, OP_REG, OP_NONE);
    instr_f("trunc", "Trunc String (op1) to (op2) Chars", OP_REG, OP_REG, OP_NONE);
    instr_f("triml", "Trim String (op2) from Left by (op3) Chars into op1", OP_REG, OP_REG, OP_REG);
    instr_f("trimr", "Trim String (op2) from Right by (op3) Chars into op1", OP_REG, OP_REG, OP_REG);
    instr_f("trunc", "Trunc String (op2) to (op3) Chars into op1", OP_REG, OP_REG, OP_REG);
    instr_f("strlen", "String Length op1 = length(op2)", OP_REG, OP_REG, OP_NONE);
    instr_f("strchar", "Int op1 (as int) = op2[op3]", OP_REG, OP_REG, OP_REG);
    instr_f("strchar", "Int op1 (as int) = op2[charpos]", OP_REG, OP_REG, OP_NONE);
    instr_f("strpos", "Set String (op1) charpos set to op2", OP_REG, OP_REG, OP_NONE);
    instr_f("substr", "op1 = op2[charpos]...op2[charpos+op3-1]", OP_REG, OP_REG, OP_REG);

    instr_f("ieq", "Int Equals op1=(op2==op3)", OP_REG, OP_REG, OP_REG);
    instr_f("ieq", "Int Equals op1=(op2==op3)", OP_REG, OP_REG, OP_INT);
    instr_f("ine", "Int Not equals op1=(op2!=op3)", OP_REG, OP_REG, OP_REG);
    instr_f("ine", "Int Not equals op1=(op2!=op3)", OP_REG, OP_REG, OP_INT);
    instr_f("igt", "Int Greater than op1=(op2>op3)", OP_REG, OP_REG, OP_REG);
    instr_f("igt", "Int Greater than op1=(op2>op3)", OP_REG, OP_REG, OP_INT);
    instr_f("igt", "Int Greater than op1=(op2>op3)", OP_REG, OP_INT, OP_REG);
    instr_f("igte", "Int Greater than equals op1=(op2>=op3)", OP_REG, OP_REG, OP_REG);
    instr_f("igte", "Int Greater than equals op1=(op2>=op3)", OP_REG, OP_REG, OP_INT);
    instr_f("igte", "Int Greater than equals op1=(op2>=op3)", OP_REG, OP_INT, OP_REG);
    instr_f("ilt", "Int Less than op1=(op2<op3)", OP_REG, OP_REG, OP_REG);
    instr_f("ilt", "Int Less than op1=(op2<op3)", OP_REG, OP_REG, OP_INT);
    instr_f("ilt", "Int Less than op1=(op2<op3)", OP_REG, OP_INT, OP_REG);
    instr_f("ilte", "Int Less than equals op1=(op2<=op3)", OP_REG, OP_REG, OP_REG);
    instr_f("ilte", "Int Less than equals op1=(op2<=op3)", OP_REG, OP_REG, OP_INT);
    instr_f("ilte", "Int Less than equals op1=(op2<=op3)", OP_REG, OP_INT, OP_REG);

    instr_f("seq", "String Equals op1=(op2==op3)", OP_REG, OP_REG, OP_REG);
    instr_f("seq", "String Equals op1=(op2==op3)", OP_REG, OP_REG, OP_STRING);
    instr_f("sne", "String Not equals op1=(op2!=op3)", OP_REG, OP_REG, OP_REG);
    instr_f("sne", "String Not equals op1=(op2!=op3)", OP_REG, OP_REG, OP_STRING);

    instr_f("time", "Put time into op1", OP_REG, OP_NONE, OP_NONE);

    instr_f("map", "Map op1 to var name in op2", OP_REG, OP_REG, OP_NONE);
    instr_f("map", "Map op1 to var name op2", OP_REG, OP_STRING, OP_NONE);
    instr_f("amap", "Map op1 to arg register index in op2", OP_REG, OP_REG, OP_NONE);
    instr_f("amap", "Map op1 to arg register index  op2", OP_REG, OP_INT, OP_NONE);
    instr_f("pmap", "Map op1 to parent var name in op2", OP_REG, OP_REG, OP_NONE);
    instr_f("pmap", "Map op1 to parent var name op2", OP_REG, OP_STRING, OP_NONE);
    instr_f("gmap", "Map op1 to global var name in op2", OP_REG, OP_REG, OP_NONE);
    instr_f("gmap", "Map op1 to global var name op2", OP_REG, OP_STRING, OP_NONE);
    instr_f("nsmap", "Map op1 to namespace in op2 var name in op3", OP_REG, OP_REG, OP_REG);
    instr_f("nsmap", "Map op1 to namespace in op2 var name op3", OP_REG, OP_REG, OP_STRING);
    instr_f("nsmap", "Map op1 to namespace op2 var name op3", OP_REG, OP_STRING, OP_STRING);
    instr_f("nsmap", "Map op1 to namespace op2 var name in op3", OP_REG, OP_STRING, OP_REG);
    instr_f("unmap", "Unmap op1", OP_REG, OP_NONE, OP_NONE);


    instr_f("call", "Call procedure (op1())", OP_FUNC, OP_NONE, OP_NONE);
    instr_f("call", "Call procedure (op1=op2())", OP_REG, OP_FUNC, OP_NONE);
    instr_f("call", "Call procedure (op1=op2(op3...) )", OP_REG, OP_FUNC, OP_REG);

    instr_f("ret", "Return NULL", OP_NONE, OP_NONE, OP_NONE);
    instr_f("ret", "Return op1", OP_REG, OP_NONE, OP_NONE);
    instr_f("ret", "Return op1", OP_INT, OP_NONE, OP_NONE);
    instr_f("ret", "Return op1", OP_FLOAT, OP_NONE, OP_NONE);
    instr_f("ret", "Return op1", OP_CHAR, OP_NONE, OP_NONE);
    instr_f("ret", "Return op1", OP_STRING, OP_NONE, OP_NONE);

    instr_f("br", "Branch to op1", OP_ID, OP_NONE, OP_NONE);
    instr_f("brt", "Branch to op1 if op2 true", OP_ID, OP_REG, OP_NONE);
    instr_f("brf", "Branch to op1 if op2 false", OP_ID, OP_REG, OP_NONE);

    instr_f("move", "Move op2 to op1", OP_REG, OP_REG, OP_NONE);
    instr_f("copy", "Copy op2 to op1", OP_REG, OP_REG, OP_NONE);
    instr_f("link", "Link op2 to op1", OP_REG, OP_REG, OP_NONE);
    instr_f("unlink", "Unlink op1", OP_REG, OP_NONE, OP_NONE);
    instr_f("null", "Null op1", OP_REG, OP_NONE, OP_NONE);

    instr_f("load", "Load op1 with op2", OP_REG, OP_INT, OP_NONE);
    instr_f("load", "Load op1 with op2", OP_REG, OP_FLOAT, OP_NONE);
    instr_f("load", "Load op1 with op2", OP_REG, OP_STRING, OP_NONE);
    instr_f("load", "Load op1 with op2", OP_REG, OP_CHAR, OP_NONE);

    instr_f("ssay", "String Say op1", OP_REG, OP_NONE, OP_NONE);
    instr_f("say", "Say op1", OP_INT, OP_NONE, OP_NONE);
    instr_f("say", "Say op1", OP_FLOAT, OP_NONE, OP_NONE);
    instr_f("say", "Say op1", OP_STRING, OP_NONE, OP_NONE);
    instr_f("say", "Say op1", OP_CHAR, OP_NONE, OP_NONE);

    instr_f("exit", "Exit", OP_NONE, OP_NONE, OP_NONE);
    instr_f("exit", "Exit op1", OP_REG, OP_NONE, OP_NONE);
    instr_f("exit", "Exit op1", OP_INT, OP_NONE, OP_NONE);

    instr_f("itos", "Set register string value from its int value", OP_REG, OP_NONE, OP_NONE);
    instr_f("ftos", "Set register string value from its float value", OP_REG, OP_NONE, OP_NONE);

    /* Space for the instructions plus instructions[0] and null termination */
    struct instruction_wrapper *i = 0;
    instructions = malloc(sizeof(Instruction) * (no_instructions + 2));
    instructions[0] = 0;
    instructions[no_instructions+1] = 0;

    /* Now populate instructions[] */
    i = 0;
    { /* In a block because the macro redefines a variable - C90 issue {sigh} */
        avl_tree_for_each_in_order(i, instruction_root,
                                   struct instruction_wrapper, index_node) {
            instructions[i->data->opcode] = i->data;
        }
    }
}

static int compare(Instruction *n1, Instruction *n2) {
    int c;

    c = strcmp(n1->instruction, n2->instruction);
    if (c) return c;

    c = (int)n1->op1_type - (int)n2->op1_type;
    if (c) return c;

    c = (int)n1->op2_type - (int)n2->op2_type;
    if (c) return c;

    return (int)n1->op3_type - (int)n2->op3_type;
}

#define GET_DATA(i) avl_tree_entry((i), struct instruction_wrapper, index_node)->data

static int compare_nn(const struct avl_tree_node *node1,
			    const struct avl_tree_node *node2)
{
    Instruction* n1 = GET_DATA(node1);
    Instruction* n2 = GET_DATA(node2);
    return compare(n1,n2);
}

static int compare_nv(const void *instptr,
                        const struct avl_tree_node *nodeptr) {
    Instruction* n1 = (Instruction*)instptr;
    Instruction* n2 = GET_DATA(nodeptr);
    return compare(n1,n2);
}

/* Only Compares the name */
static int comp_nm_nv(const void *instptr,
                      const struct avl_tree_node *nodeptr) {
    Instruction* n1 = (Instruction*)instptr;
    Instruction* n2 = GET_DATA(nodeptr);
    return strcmp(n1->instruction, n2->instruction);
}

// Add a instruction to the database
void instr_f(char *name, char *description,
             OperandType op1_type, OperandType op2_type, OperandType op3_type) {

    Instruction *data = malloc(sizeof(Instruction));
    no_instructions++; /* First Instruction has opcode 1 */
    data->instruction = name;
    data->desc = description;
    data->op1_type = op1_type;
    data->op2_type = op2_type;
    data->op3_type = op3_type;
    if (op1_type == OP_NONE) data->operands = 0;
    else if (op2_type == OP_NONE) data->operands = 1;
    else if (op3_type == OP_NONE) data->operands = 2;
    else data->operands = 3;
    data->opcode = no_instructions;

    struct instruction_wrapper *i = malloc(sizeof(struct instruction_wrapper));
    i->data = data;
    if (avl_tree_insert(&instruction_root, &i->index_node, compare_nn)) {
        // Duplicate.
        free(i);
        free(data);
        fprintf(stderr, "Duplicate Instruction %s\n", name);
    }
}

// Search for an instruction
Instruction *src_inst(char* name, OperandType op1_type,
                      OperandType op2_type, OperandType op3_type) {
    struct avl_tree_node *result;
    Instruction search_for;
    search_for.instruction = name;
    search_for.op1_type = op1_type;
    search_for.op2_type = op2_type;
    search_for.op3_type = op3_type;

    result = avl_tree_lookup(instruction_root, &search_for, compare_nv);

    if (result) return avl_tree_entry(result, struct instruction_wrapper, index_node)->data;
    else return 0;
}

/* Get the first instruction with a specific instruction name (ignoring operands) */
/* returns null on no match */
Instruction *fst_inst(char* name) {
    struct avl_tree_node *result;
    struct avl_tree_node *prev;
    Instruction *found;
    Instruction search_for;
    search_for.instruction = name;

    result = avl_tree_lookup(instruction_root, &search_for, comp_nm_nv);

    if (result) {
        /* result might very well not be the first instruction so we just
         * work backwards across the tree to find the real first one
         */
        prev = avl_pino(result);
        while (prev) {
            found = avl_tree_entry(prev, struct instruction_wrapper, index_node)->data;
            if (strcmp(found->instruction, name) != 0)
                    break;
            result = prev;
            prev = avl_pino(result);
        }
        found = avl_tree_entry(result, struct instruction_wrapper, index_node)->data;
        return found;
    }

    return 0;
}

/* returns the next instruction with the same instruction name (ignoring operands) */
/* returns null when there is not any more instructions with the same name */
Instruction *nxt_inst(Instruction *inst) {
    struct avl_tree_node *result;
    struct avl_tree_node *next;
    Instruction *found = 0;

    result = avl_tree_lookup(instruction_root, inst, compare_nv);

    if (result) {
        next = avl_nino(result);
        if (next) {
            found = avl_tree_entry(next, struct instruction_wrapper, index_node)->data;
            if (strcmp(found->instruction, inst->instruction) != 0)
                found = 0;
        }
    }

    return found;
}

// Lookup an instruction by opcode
Instruction *get_inst(int opcode) {
    if (opcode <=0) return 0;
    if (opcode > no_instructions) return 0;
    return instructions[opcode];
}

/* Free Instruction Database */
void free_ops() {
    struct instruction_wrapper *i;

    /* This walks the tree in post order which allows each node be freed */
    avl_tree_for_each_in_postorder(i, instruction_root,
                                   struct instruction_wrapper, index_node) {
        free(i->data);
        free(i);
    }

    free(instructions);
    instructions = 0;
    instruction_root = 0;
    no_instructions = 0;
}

/* Free Instruction Database */
void prt_ops() {
    struct instruction_wrapper *i;
    char buffer[100];

    printf("\n* REXX Assembly Instruction List\n");
    /* This walks the tree in post order which allows each node be freed */
    avl_tree_for_each_in_order(i, instruction_root, struct instruction_wrapper,
                               index_node) {

        exp_opds(i->data, buffer, 100);
        printf("0x%.4x %-10s %-20s %s\n",
               i->data->opcode, i->data->instruction, buffer, i->data->desc);
    }
    printf("\n");
}

char* opd_name(OperandType type) {
    switch(type) {
        case OP_ID: return "ID";
        case OP_REG: return "REG";
        case OP_FUNC: return "FUNC";
        case OP_INT: return "INT";
        case OP_FLOAT: return "FLOAT";
        case OP_CHAR: return "CHAR";
        case OP_STRING: return "STRING";
        case OP_NONE: return "NONE";
    }
}

char* exp_opds(Instruction* inst, char* buffer, size_t buffer_len) {
    switch (inst->operands) {
        case 0:
            snprintf(buffer,buffer_len,"%s","no operand");
            break;
        case 1:
            snprintf(buffer, buffer_len, "{%s}", opd_name(inst->op1_type));
            break;
        case 2:
            snprintf(buffer, buffer_len, "{%s,%s}", opd_name(inst->op1_type),
                     opd_name(inst->op2_type));
            break;
        case 3:
            snprintf(buffer, buffer_len, "{%s,%s,%s}",
                     opd_name(inst->op1_type), opd_name(inst->op2_type),
                     opd_name(inst->op3_type));
            break;
        default:
            snprintf(buffer,buffer_len,"{%s}","*Internal Error*");
    }
    return buffer;
}
