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
    if (instructions) return; /* Stops double initialising */
    /* STARTINSTRUCTION */
    instr_f("iadd", "Integer Add (op1=op2+op3)", OP_REG, OP_REG, OP_REG);
    instr_f("iadd", "Integer Add (op1=op2+op3)", OP_REG, OP_REG, OP_INT);
    instr_f("addi", "Convert and Add to Integer (op1=op2+op3) (Deprecated)", OP_REG, OP_REG, OP_REG);
    instr_f("addi", "Convert and Add to Integer (op1=op2+op3) (Deprecated)", OP_REG, OP_REG, OP_INT);

    instr_f("isub", "Integer Subtract (op1=op2-op3)", OP_REG, OP_REG, OP_REG);
    instr_f("isub", "Integer Subtract (op1=op2-op3)", OP_REG, OP_REG, OP_INT);
    instr_f("isub", "Integer Subtract (op1=op2-op3)", OP_REG, OP_INT, OP_REG);

    instr_f("subi", "Convert and Subtract to Integer (op1=op2-op3) (Deprecated)", OP_REG, OP_REG, OP_REG);
    instr_f("subi", "Convert and Subtract to Integer (op1=op2-op3) (Deprecated)", OP_REG, OP_REG, OP_INT);

    instr_f("imult", "Integer Multiply (op1=op2*op3)", OP_REG, OP_REG, OP_REG);
    instr_f("imult", "Integer Multiply (op1=op2*op3)", OP_REG, OP_REG, OP_INT);
    instr_f("multi", "Convert and Multiply to Integer (op1=op2*op3) (Deprecated)", OP_REG, OP_REG, OP_REG);
    instr_f("multi", "Convert and Multiply to Integer (op1=op2*op3) (Deprecated)", OP_REG, OP_REG, OP_INT);

    instr_f("idiv", "Integer Divide (op1=op2/op3)", OP_REG, OP_REG, OP_REG);
    instr_f("idiv", "Integer Divide (op1=op2/op3)", OP_REG, OP_REG, OP_INT);
    instr_f("idiv", "Integer Divide (op1=op2/op3)", OP_REG, OP_INT, OP_REG);

    instr_f("fidiv", "Integer Divide with Floats (op1=op2/op3)", OP_REG, OP_REG, OP_REG);
    instr_f("fidiv", "Integer Divide with Floats (op1=op2/op3)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("fidiv", "Integer Divide with Floats (op1=op2/op3)", OP_REG, OP_FLOAT, OP_REG);

    instr_f("didiv", "Integer Divide with Decimals (op1=op2/op3)", OP_REG, OP_REG, OP_REG);
    instr_f("didiv", "Integer Divide with Decimals (op1=op2/op3)", OP_REG, OP_REG, OP_DECIMAL);
    instr_f("didiv", "Integer Divide with Decimals (op1=op2/op3)", OP_REG, OP_DECIMAL, OP_REG);

    instr_f("divi", "Convert and Divide to Integer (op1=op2/op3) (Deprecated)", OP_REG, OP_REG, OP_REG);
    instr_f("divi", "Convert and Divide to Integer (op1=op2/op3) (Deprecated)", OP_REG, OP_REG, OP_INT);

    instr_f("imod", "Integer Modulo (op1=op2%op3)", OP_REG, OP_REG, OP_REG);
    instr_f("imod", "Integer Modulo (op1=op2%op3)", OP_REG, OP_REG, OP_INT);
    instr_f("imod", "Integer Modulo (op1=op2&op3)", OP_REG, OP_INT, OP_REG);

    instr_f("fmod", "Float Modulo (op1=op2%op3)", OP_REG, OP_REG, OP_REG);
    instr_f("fmod", "Float Modulo (op1=op2%op3)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("fmod", "Float Modulo (op1=op2&op3)", OP_REG, OP_FLOAT, OP_REG);

    instr_f("dmod", "Decimal Modulo (op1=op2%op3)", OP_REG, OP_REG, OP_REG);
    instr_f("dmod", "Decimal Modulo (op1=op2%op3)", OP_REG, OP_REG, OP_DECIMAL);
    instr_f("dmod", "Decimal Modulo (op1=op2&op3)", OP_REG, OP_DECIMAL, OP_REG);

    instr_f("fadd", "Float Add (op1=op2+op3)", OP_REG, OP_REG, OP_REG);
    instr_f("fadd", "Float Add (op1=op2+op3)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("addf", "Convert and Add to Float (op1=op2+op3) (Deprecated)", OP_REG, OP_REG, OP_REG);
    instr_f("addf", "Convert and Add to Float (op1=op2+op3) (Deprecated)", OP_REG, OP_REG, OP_FLOAT);

    instr_f("fsub", "Float Subtract (op1=op2-op3)", OP_REG, OP_REG, OP_REG);
    instr_f("fsub", "Float Subtract (op1=op2-op3)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("fsub", "Float Subtract (op1=op2-op3)", OP_REG, OP_FLOAT, OP_REG);
    instr_f("subf", "Convert and Subtract to Float (op1=op2-op3) (Deprecated)", OP_REG, OP_REG, OP_REG);
    instr_f("subf", "Convert and Subtract to Float (op1=op2-op3) (Deprecated)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("subf", "Convert and Subtract to Float (op1=op2-op3) (Deprecated)", OP_REG, OP_FLOAT, OP_REG);

    instr_f("fmult", "Float Multiply (op1=op2*op3)", OP_REG, OP_REG, OP_REG);
    instr_f("fmult", "Float Multiply (op1=op2*op3)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("multf", "Convert and Multiply to Float (op1=op2*op3) (Deprecated)", OP_REG, OP_REG, OP_REG);
    instr_f("multf", "Convert and Multiply to Float (op1=op2*op3) (Deprecated)", OP_REG, OP_REG, OP_FLOAT);

    instr_f("fdiv", "Float Divide (op1=op2/op3)", OP_REG, OP_REG, OP_REG);
    instr_f("fdiv", "Float Divide (op1=op2/op3)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("fdiv", "Float Divide (op1=op2/op3)", OP_REG, OP_FLOAT, OP_REG);
    instr_f("divf", "Convert and Divide to Float (op1=op2/op3) (Deprecated)", OP_REG, OP_REG, OP_REG);
    instr_f("divf", "Convert and Divide to Float (op1=op2/op3) (Deprecated)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("divf", "Convert and Divide to Float (op1=op2/op3) (Deprecated)", OP_REG, OP_FLOAT, OP_REG);

    instr_f("inc", "Increment Int (op1++)", OP_REG, OP_NONE, OP_NONE);
    instr_f("dec", "Decrement Int (op1--)", OP_REG, OP_NONE, OP_NONE);
    instr_f("inc0", "Increment R0++ Int", OP_NONE, OP_NONE, OP_NONE);
    instr_f("dec0", "Decrement R0-- Int", OP_NONE, OP_NONE, OP_NONE);
    instr_f("inc1", "Increment R1++ Int", OP_NONE, OP_NONE, OP_NONE);
    instr_f("dec1", "Decrement R1-- Int", OP_NONE, OP_NONE, OP_NONE);
    instr_f("inc2", "Increment R2++ Int", OP_NONE, OP_NONE, OP_NONE);
    instr_f("dec2", "Decrement R2-- Int", OP_NONE, OP_NONE, OP_NONE);

    instr_f("iand", "bit wise and of 2 integers (op1=op2&op3)", OP_REG, OP_REG, OP_REG);
    instr_f("iand", "bit wise and of 2 integers (op1=op2&op3)", OP_REG, OP_REG, OP_INT);
    instr_f("ior", "bit wise or of 2 integers (op1=op2|op3)", OP_REG, OP_REG, OP_REG);
    instr_f("ior", "bit wise or of 2 integers (op1=op2|op3)", OP_REG, OP_REG, OP_INT);
    instr_f("ixor", "bit wise exclusive OR of 2 integers (op1=op2^op3)", OP_REG, OP_REG, OP_REG);
    instr_f("ixor", "bit wise exclusive OR of 2 integers (op1=op2^op3)", OP_REG, OP_REG, OP_INT);
    instr_f("ishl", "bit wise shift logical left of integer (op1=op2<<op3)", OP_REG, OP_REG, OP_REG);
    instr_f("ishl", "bit wise shift logical left of integer (op1=op2<<op3)", OP_REG, OP_REG, OP_INT);
    instr_f("ishr", "bit wise shift logical right of integer (op1=op2>>op3)", OP_REG, OP_REG, OP_REG);
    instr_f("ishr", "bit wise shift logical right of integer (op1=op2>>op3)", OP_REG, OP_REG, OP_INT);
    instr_f("inot", "inverts all bits of an integer (op1=~op2)", OP_REG, OP_REG, OP_NONE);
    instr_f("inot", "inverts all bits of an integer (op1=~op2)", OP_REG, OP_INT, OP_NONE);

    instr_f("sconcat", "String Concat with space (op1=op2||op3)", OP_REG, OP_REG, OP_REG);
    instr_f("sconcat", "String Concat with space (op1=op2||op3)", OP_REG, OP_REG, OP_STRING);
    instr_f("sconcat", "String Concat with space (op1=op2||op3)", OP_REG, OP_STRING, OP_REG);
    instr_f("concat", "String Concat (op1=op2||op3)", OP_REG, OP_REG, OP_REG);
    instr_f("concat", "String Concat (op1=op2||op3)", OP_REG, OP_REG, OP_STRING);
    instr_f("concat", "String Concat (op1=op2||op3)", OP_REG, OP_STRING, OP_REG);
    instr_f("appendchar", "Append Concat Char op2 (as int) on op1", OP_REG, OP_REG, OP_NONE);
    instr_f("concchar", "Concat Char op1 from op2 position op3", OP_REG, OP_REG, OP_REG);
    instr_f("sappend", "String Append with space (op1=op1||op2)", OP_REG, OP_REG, OP_NONE);
    instr_f("append", "String Append (op1=op1||op2)", OP_REG, OP_REG, OP_NONE);

    instr_f("triml", "Trim String (op1) from Left by (op2) Chars", OP_REG, OP_REG, OP_NONE);
    instr_f("trimr", "Trim String (op1) from Right by (op2) Chars", OP_REG, OP_REG, OP_NONE);
    instr_f("trunc", "Trunc String (op1) to (op2) Chars", OP_REG, OP_REG, OP_NONE);
    instr_f("triml", "Trim String (op2) from Left by (op3) Chars into op1", OP_REG, OP_REG, OP_REG);
    instr_f("trimr", "Trim String (op2) from Right by (op3) Chars into op1", OP_REG, OP_REG, OP_REG);
    instr_f("trunc", "Trunc String (op2) to (op3) Chars into op1", OP_REG, OP_REG, OP_REG);
    instr_f("strlen", "String Length op1 = length(op2)", OP_REG, OP_REG, OP_NONE);
    instr_f("strchar", "op1 (as int) = op2[op3]", OP_REG, OP_REG, OP_REG);
    instr_f("strchar", "op1 (as int) = op2[charpos]", OP_REG, OP_REG, OP_NONE);
    instr_f("hexchar", "op1 (as hex) = op2[op3]", OP_REG, OP_REG, OP_REG);
    instr_f("poschar", "op1 = position of op3 in op2", OP_REG, OP_REG, OP_REG);

    instr_f("setstrpos", "Set String (op1) charpos set to op2", OP_REG, OP_REG, OP_NONE);
    instr_f("getstrpos", "Get String (op2) charpos into op1", OP_REG, OP_REG, OP_NONE);
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
    instr_f("igtbr", "Int Greater than if (op2>op3) goto op1", OP_ID, OP_REG, OP_REG);
    instr_f("iltbr", "Int Less than if (op2<op3) goto op1", OP_ID, OP_REG, OP_REG);

    instr_f("feq", "Float Equals op1=(op2==op3)", OP_REG, OP_REG, OP_REG);
    instr_f("feq", "Float Equals op1=(op2==op3)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("fne", "Float Not equals op1=(op2!=op3)", OP_REG, OP_REG, OP_REG);
    instr_f("fne", "Float Not equals op1=(op2!=op3)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("fgt", "Float Greater than op1=(op2>op3)", OP_REG, OP_REG, OP_REG);
    instr_f("fgt", "Float Greater than op1=(op2>op3)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("fgt", "Float Greater than op1=(op2>op3)", OP_REG, OP_FLOAT, OP_REG);
    instr_f("fgte", "Float Greater than equals op1=(op2>=op3)", OP_REG, OP_REG, OP_REG);
    instr_f("fgte", "Float Greater than equals op1=(op2>=op3)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("fgte", "Float Greater than equals op1=(op2>=op3)", OP_REG, OP_FLOAT, OP_REG);
    instr_f("flt", "Float Less than op1=(op2<op3)", OP_REG, OP_REG, OP_REG);
    instr_f("flt", "Float Less than op1=(op2<op3)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("flt", "Float Less than op1=(op2<op3)", OP_REG, OP_FLOAT, OP_REG);
    instr_f("flte", "Float Less than equals op1=(op2<=op3)", OP_REG, OP_REG, OP_REG);
    instr_f("flte", "Float Less than equals op1=(op2<=op3)", OP_REG, OP_REG, OP_FLOAT);
    instr_f("flte", "Float Less than equals op1=(op2<=op3)", OP_REG, OP_FLOAT, OP_REG);
    instr_f("fgtbr", "Float Greater than if (op2>op3) goto op1", OP_ID, OP_REG, OP_REG);
    instr_f("fltbr", "Float Less than if (op2<op3) goto op1", OP_ID, OP_REG, OP_REG);


    instr_f("seq", "String Equals op1=(op2==op3)", OP_REG, OP_REG, OP_REG);
    instr_f("seq", "String Equals op1=(op2==op3)", OP_REG, OP_REG, OP_STRING);
    instr_f("rseq", "non strict String Equals op1=(op2=op3)", OP_REG, OP_REG, OP_REG);
    instr_f("rseq", "non strict String Equals op1=(op2=op3)", OP_REG, OP_REG, OP_STRING);
    instr_f("sne", "String Not equals op1=(op2!=op3)", OP_REG, OP_REG, OP_REG);
    instr_f("sne", "String Not equals op1=(op2!=op3)", OP_REG, OP_REG, OP_STRING);
    instr_f("sgt", "String Greater than op1=(op2>op3)", OP_REG, OP_REG, OP_REG);
    instr_f("sgt", "String Greater than op1=(op2>op3)", OP_REG, OP_REG, OP_STRING);
    instr_f("sgt", "String Greater than op1=(op2>op3)", OP_REG, OP_STRING, OP_REG);
    instr_f("sgte", "String Greater than equals op1=(op2>=op3)", OP_REG, OP_REG, OP_REG);
    instr_f("sgte", "String Greater than equals op1=(op2>=op3)", OP_REG, OP_REG, OP_STRING);
    instr_f("sgte", "String Greater than equals op1=(op2>=op3)", OP_REG, OP_STRING, OP_REG);
    instr_f("slt", "String Less than op1=(op2<op3)", OP_REG, OP_REG, OP_REG);
    instr_f("slt", "String Less than op1=(op2<op3)", OP_REG, OP_REG, OP_STRING);
    instr_f("slt", "String Less than op1=(op2<op3)", OP_REG, OP_STRING, OP_REG);
    instr_f("slte", "String Less than equals op1=(op2<=op3)", OP_REG, OP_REG, OP_REG);
    instr_f("slte", "String Less than equals op1=(op2<=op3)", OP_REG, OP_REG, OP_STRING);
    instr_f("slte", "String Less than equals op1=(op2<=op3)", OP_REG, OP_STRING, OP_REG);

    instr_f("and", "Logical (int) and op1=(op2 && op3)", OP_REG, OP_REG, OP_REG);
    instr_f("or", "Logical (int) or op1=(op2 || op3)", OP_REG, OP_REG, OP_REG);
    instr_f("not", "Logical (int) not op1=!op2", OP_REG, OP_REG, OP_NONE);

    instr_f("time", "Put time into op1", OP_REG, OP_NONE, OP_NONE);
    instr_f("mtime", "Put time in microseconds into op1", OP_REG, OP_NONE, OP_NONE);
    instr_f("xtime", "put special time properties into op1", OP_REG, OP_STRING, OP_NONE);

    instr_f("map", "Map op1 to var name in op2", OP_REG, OP_REG, OP_NONE);
    instr_f("map", "Map op1 to var name op2", OP_REG, OP_STRING, OP_NONE);
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
    instr_f("dcall", "Dynamic call procedure (op1=op2(op3...) )", OP_REG, OP_REG, OP_REG);

    instr_f("ret", "Return VOID", OP_NONE, OP_NONE, OP_NONE);
    instr_f("ret", "Return op1", OP_REG, OP_NONE, OP_NONE);
    instr_f("ret", "Return op1", OP_INT, OP_NONE, OP_NONE);
    instr_f("ret", "Return op1", OP_FLOAT, OP_NONE, OP_NONE);
    instr_f("ret", "Return op1", OP_CHAR, OP_NONE, OP_NONE);
    instr_f("ret", "Return op1", OP_STRING, OP_NONE, OP_NONE);

    instr_f("br", "Branch to op1", OP_ID, OP_NONE, OP_NONE);
    instr_f("brt", "Branch to op1 if op2 true", OP_ID, OP_REG, OP_NONE);
    instr_f("brf", "Branch to op1 if op2 false", OP_ID, OP_REG, OP_NONE);
    instr_f("brtf", "Branch to op1 if op3 true, otherwise branch to op2", OP_ID, OP_ID, OP_REG);

    instr_f("move", "Move op2 to op1", OP_REG, OP_REG, OP_NONE);
    instr_f("swap", "Swap op1 and op2", OP_REG, OP_REG, OP_NONE);
    instr_f("copy", "Copy op2 to op1", OP_REG, OP_REG, OP_NONE);
    instr_f("icopy", "Copy Integer op2 to op1", OP_REG, OP_REG, OP_NONE);
    instr_f("fcopy", "Copy Float op2 to op1", OP_REG, OP_REG, OP_NONE);
    instr_f("scopy", "Copy String op2 to op1", OP_REG, OP_REG, OP_NONE);
    instr_f("acopy", "Copy status Attributes op2 to op1", OP_REG, OP_REG, OP_NONE);

    instr_f("linkarg", "Link args[op2] to op1", OP_REG, OP_INT, OP_NONE);
    instr_f("linkarg", "Link args[op2+op3] to op1", OP_REG, OP_REG, OP_INT);
    instr_f("linkattr", "Link attribute op3 of op2 to op1", OP_REG, OP_REG, OP_REG);
    instr_f("linkattr", "Link attribute op3 of op2 to op1", OP_REG, OP_REG, OP_INT);
    instr_f("linkattr1", "Link attribute op3 (1 base) of op2 to op1", OP_REG, OP_REG, OP_REG);
    instr_f("linkattr1", "Link attribute op3 (1 base) of op2 to op1", OP_REG, OP_REG, OP_INT);

    instr_f("linktoattr", "Link op3 to attribute op1 of op2 ", OP_REG, OP_REG, OP_REG);
    instr_f("linktoattr", "Link op3 to attribute op1 of op2 ", OP_INT, OP_REG, OP_REG);
    instr_f("linktoattr1", "Link op3 to attribute op1 (1 base) of op2 ", OP_REG, OP_REG, OP_REG);
    instr_f("linktoattr1", "Link op3 to attribute op1 (1 base) of op2 ", OP_INT, OP_REG, OP_REG);
    instr_f("unlinkattr", "Unlink attribute op1 of op2", OP_REG, OP_REG, OP_NONE);
    instr_f("unlinkattr", "Unlink attribute op1 of op2", OP_INT, OP_REG, OP_NONE);
    instr_f("unlinkattr1", "Unlink attribute op1 (1 base) of op2", OP_REG, OP_REG, OP_NONE);
    instr_f("unlinkattr1", "Unlink attribute op1 (1 base) of op2", OP_INT, OP_REG, OP_NONE);

    instr_f("link", "Link op2 to op1", OP_REG, OP_REG, OP_NONE);
    instr_f("unlink", "Unlink op1", OP_REG, OP_NONE, OP_NONE);
    instr_f("null", "Null op1", OP_REG, OP_NONE, OP_NONE);

    instr_f("load", "Load op1 with op2", OP_REG, OP_INT, OP_NONE);
    instr_f("load", "Load op1 with op2", OP_REG, OP_FLOAT, OP_NONE);
    instr_f("load", "Load op1 with op2", OP_REG, OP_STRING, OP_NONE);
    instr_f("load", "Load op1 with op2", OP_REG, OP_CHAR, OP_NONE);
    instr_f("load", "Load op1 with op2", OP_REG, OP_REG, OP_NONE);
    instr_f("load", "Load op1 with op2", OP_REG, OP_DECIMAL, OP_NONE);
    instr_f("load", "Load op1 with op2 (non symbolic registers)", OP_INT, OP_INT, OP_NONE);
    instr_f("load", "Load op1 with op2 (non symbolic registers)", OP_INT, OP_REG, OP_NONE);

    instr_f("say", "Say op1", OP_REG, OP_NONE, OP_NONE);
    instr_f("sayx", "Say op1 without line feed", OP_REG, OP_NONE, OP_NONE);
    instr_f("sayx", "Say op1 (as string) without line feed", OP_STRING, OP_NONE, OP_NONE);

    instr_f("say", "Say op1", OP_INT, OP_NONE, OP_NONE);
    instr_f("say", "Say op1", OP_FLOAT, OP_NONE, OP_NONE);
    instr_f("say", "Say op1", OP_STRING, OP_NONE, OP_NONE);
    instr_f("say", "Say op1", OP_CHAR, OP_NONE, OP_NONE);

    instr_f("readline", "Read Line to op1", OP_REG, OP_NONE, OP_NONE);

    instr_f("exit", "Exit", OP_NONE, OP_NONE, OP_NONE);
    instr_f("exit", "Exit op1", OP_REG, OP_NONE, OP_NONE);
    instr_f("exit", "Exit op1", OP_INT, OP_NONE, OP_NONE);

    instr_f("btoi", "Set register integer value from its boolean value", OP_REG, OP_NONE, OP_NONE);
    instr_f("btod", "Set register decimal value from its boolean value", OP_REG, OP_NONE, OP_NONE);
    instr_f("btof", "Set register float value from its boolean value", OP_REG, OP_NONE, OP_NONE);
    instr_f("btos", "Set register string value from its boolean value", OP_REG, OP_NONE, OP_NONE);
    instr_f("itos", "Set register string value from its int value", OP_REG, OP_NONE, OP_NONE);
    instr_f("ftos", "Set register string value from its float value", OP_REG, OP_NONE, OP_NONE);
    instr_f("itof", "Set register float value from its int value", OP_REG, OP_NONE, OP_NONE);
    instr_f("ftoi", "Set register int value from its float value", OP_REG, OP_NONE, OP_NONE);
    instr_f("ftob", "Set register boolean (int 1 or 0) value from its float value", OP_REG, OP_NONE, OP_NONE);
    instr_f("itob", "Set register boolean (int 1 or 0) value from its integer value", OP_REG, OP_NONE, OP_NONE);
    instr_f("stob", "Set register boolean (int 1 or 0) value from its string value", OP_REG, OP_NONE, OP_NONE);
    instr_f("stof", "Set register float value from its string value", OP_REG, OP_NONE, OP_NONE);
    instr_f("stoi", "Set register int value from its string value", OP_REG, OP_NONE, OP_NONE);

    instr_f("fformat", "DEPRECATED use fextr. Set string value from float value using a format string", OP_REG, OP_REG, OP_REG);
    instr_f("fextr", "op3 float extracted to op1 string coefficient and op2 int decimal exponent", OP_REG, OP_REG, OP_REG);

    instr_f("strlower", "Set string to lower case value", OP_REG, OP_REG, OP_NONE);
    instr_f("strupper", "Set string to upper case value", OP_REG, OP_REG, OP_NONE);
    instr_f("transchar", "replace op1 if it is in op3-list by char in op2-list", OP_REG, OP_REG, OP_REG);
    instr_f("dropchar", "set op1 from op2 after dropping all chars from op3", OP_REG, OP_REG, OP_REG);
    instr_f("substring", "set op1=substr(op2,op3) remaining string", OP_REG, OP_REG, OP_REG);
    instr_f("substcut", "set op1=substr(op1,,op2) cuts off op1 after position op3", OP_REG, OP_REG, OP_NONE);
    instr_f("padstr", "set op1=op2[repeated op3 times]", OP_REG, OP_REG, OP_REG);
    instr_f("strpos", "op1 is position of op2 in op3", OP_REG, OP_REG, OP_REG);

    instr_f("getbyte", "get byte  (op1=op2(op3)", OP_REG, OP_REG, OP_REG);
    instr_f("cnop", "no operation", OP_NONE, OP_NONE, OP_NONE);

    instr_f("ipow", "op1=op2**op3", OP_REG, OP_REG, OP_REG);
    instr_f("ipow", "op1=op2**op3", OP_REG, OP_REG, OP_INT);
    instr_f("ipow", "op1=op2**op3", OP_REG, OP_INT, OP_REG);

    instr_f("fpow", "op1=op2**op3", OP_REG, OP_REG, OP_REG);
    instr_f("fpow", "op1=op2**op3", OP_REG, OP_REG, OP_FLOAT);
    instr_f("fpow", "op1=op2**op3", OP_REG, OP_FLOAT, OP_REG);

    instr_f("bct", "dec op2; if op2>0; goto op1(if true)", OP_ID, OP_REG,OP_NONE);
    instr_f("bct", "dec op2; inc op3, if op2>0; goto op1(if true)", OP_ID,OP_REG, OP_REG);
    instr_f("bctnm", "dec op2; if op2>=0; goto op1(if true)", OP_ID, OP_REG,OP_NONE);
    instr_f("bctnm", "dec op2; inc op3, if op2>=0; goto op1(if true)", OP_ID,OP_REG, OP_REG);
    instr_f("bctp", "inc op2; goto op1", OP_ID, OP_REG,OP_NONE);

    instr_f("bcf", "if op2=0 goto op1(if false) else dec op2", OP_ID, OP_REG,OP_NONE);
    instr_f("bcf", "if op2=0 goto op1(if false) else dec op2 and inc op3", OP_ID,OP_REG, OP_REG);

    instr_f("bgt", "if op2>op3 then goto op1", OP_ID,OP_REG, OP_REG);
    instr_f("bgt", "if op2>op3 then goto op1", OP_ID,OP_REG, OP_INT);
    instr_f("bge", "if op2>=op3 then goto op1", OP_ID,OP_REG, OP_REG);
    instr_f("bge", "if op2>=op3 then goto op1", OP_ID,OP_REG, OP_INT);
    instr_f("blt", "if op2<op3 then goto op1", OP_ID,OP_REG, OP_REG);
    instr_f("blt", "if op2<op3 then goto op1", OP_ID,OP_REG, OP_INT);
    instr_f("ble", "if op2<=op3 then goto op1", OP_ID,OP_REG, OP_REG);
    instr_f("ble", "if op2<=op3 then goto op1", OP_ID,OP_REG, OP_INT);
    instr_f("bne", "if op2!=op3 then goto op1", OP_ID,OP_REG, OP_REG);
    instr_f("bne", "if op2!=op3 then goto op1", OP_ID,OP_REG, OP_INT);
    instr_f("beq", "if op2==op3 then goto op1", OP_ID,OP_REG, OP_REG);
    instr_f("beq", "if op2==op3 then goto op1", OP_ID,OP_REG, OP_INT);

    instr_f("fndblnk", "op1 = find next blank in op2[op3] and behind", OP_REG,OP_REG, OP_REG);
    instr_f("fndnblnk", "op1 = find next next non blank in op2[op3] and behind", OP_REG,OP_REG, OP_REG);

    instr_f("isex", "dec op1 = -op1 (sign change)", OP_REG, OP_NONE,OP_NONE);
    instr_f("fsex", "float op1 = -op1 (sign change)", OP_REG, OP_NONE,OP_NONE);

    instr_f("gettp", "gets the register type flag (op1 = op2.typeflag)", OP_REG, OP_REG,OP_NONE);
    instr_f("settp", "sets the register type flag (op1.typeflag = op2)", OP_REG, OP_INT,OP_NONE);

    instr_f("loadsettp", "load register and sets the register type flag load op1=op2 (op1.typeflag = op3)", OP_REG, OP_INT,OP_INT);
    instr_f("loadsettp", "load register and sets the register type flag load op1=op2 (op1.typeflag = op3)", OP_REG, OP_FLOAT,OP_INT);
    instr_f("loadsettp", "load register and sets the register type flag load op1=op2 (op1.typeflag = op3)", OP_REG, OP_STRING,OP_INT);

    instr_f("setortp", "or the register type flag (op1.typeflag = op1.typeflag | op2)", OP_REG, OP_INT,OP_NONE);
    instr_f("getandtp", "get the register type flag with mask (op1(int) = op2.typeflag & op3)", OP_REG, OP_REG,OP_INT);

    instr_f("brtpt", "if op2.typeflag true then goto op1", OP_ID, OP_REG,OP_NONE);
    instr_f("brtpandt", "if op2.typeflag && op3 true then goto op1", OP_ID, OP_REG,OP_INT);

    instr_f("opendll", "open DLL", OP_REG, OP_REG, OP_REG);
    instr_f("dllparms", "fetches parms for DLL call ", OP_REG, OP_REG, OP_REG);

    instr_f("irand", "random number random, op1=irand(op2)", OP_REG, OP_REG,OP_NONE);
    instr_f("irand", "random number random, op1=irand(op2)", OP_REG, OP_INT,OP_NONE);
    instr_f("rxvers", "get version, op1=version details", OP_REG, OP_NONE,OP_NONE);
    instr_f("getenv", "get environment variable, op1=env[op2]", OP_REG, OP_REG,OP_NONE);
    instr_f("getenv", "get environment variable, op1=env[op2]", OP_REG, OP_STRING,OP_NONE);

    instr_f("rxhash", "returns hash value, etc, op1=hash(op2,len(op3))", OP_REG, OP_REG,OP_REG);

    instr_f("getattrs", "get number attributes op1 = op2.num_attributes", OP_REG, OP_REG,OP_NONE);
    instr_f("getattrs", "get number attributes op1 = op2.num_attributes + op3", OP_REG, OP_REG,OP_INT);

    instr_f("setattrs", "set number attributes op1.num_attributes = op2", OP_REG, OP_REG,OP_NONE);
    instr_f("setattrs", "set number attributes op1.num_attributes = op2", OP_REG, OP_INT,OP_NONE);
    instr_f("setattrs", "set number attributes op1.num_attributes = op2 + op3", OP_REG, OP_REG,OP_INT);
    instr_f("setattrs", "set number attributes op1.num_attributes = op2 + op3", OP_REG, OP_INT,OP_INT);

    instr_f("getabufs", "get attribute buffer size op1 = op2.max_attributes", OP_REG, OP_REG,OP_NONE);

    instr_f("minattrs", "ensure min number attributes op1.num_attributes >= op2", OP_REG, OP_REG,OP_NONE);
    instr_f("minattrs", "ensure min number attributes op1.num_attributes >= op2", OP_REG, OP_INT,OP_NONE);
    instr_f("minattrs", "ensure min number attributes op1.num_attributes >= op2 + op3", OP_REG, OP_REG,OP_INT);
    instr_f("minattrs", "ensure min number attributes op1.num_attributes >= op2 + op3", OP_REG, OP_INT,OP_INT);

    instr_f("erase", "erases register contents", OP_REG, OP_NONE, OP_NONE);

    // IO Instructions
    instr_f("spawn", "Spawn Process op1 = exec op2 redirect op3", OP_REG, OP_REG, OP_REG);
    instr_f("redir2str", "Redirect op1 -> string op2", OP_REG, OP_REG, OP_NONE);
    instr_f("redir2arr", "Redirect op1 -> array op2", OP_REG, OP_REG, OP_NONE);
    instr_f("str2redir", "Redirect op1 <- string op2", OP_REG, OP_REG, OP_NONE);
    instr_f("arr2redir", "Redirect op1 <- array op2", OP_REG, OP_REG, OP_NONE);
    instr_f("nullredir", "Redirect op1 = to/from null", OP_REG, OP_NONE, OP_NONE);

    instr_f("fopen","op1 file*(int) = fopen filename op2(string) mode op3(string)", OP_REG, OP_REG, OP_REG);
    instr_f("fclose","op1 rc(int) = fclose op2 file*(int)", OP_REG, OP_REG, OP_NONE);
    instr_f("fflush","op1 rc(int) = fflush op2 file*(int)", OP_REG, OP_REG, OP_NONE);
    instr_f("freadb","op1(binary) = fread op2 file*(int) op3 bytes(int)", OP_REG, OP_REG, OP_REG);
    instr_f("freadline","op1 (string) = read until newline op2 file*(int)", OP_REG, OP_REG, OP_NONE);
    instr_f("freadbyte","op1 (int) = read byte op2 file*(int)", OP_REG, OP_REG, OP_NONE);
    instr_f("freadcdpt","op1 (string and int) = read codepoint op2 file*(int)", OP_REG, OP_REG, OP_NONE);
    instr_f("fwrite","fwrite to op1 file*(int) from op2(string)", OP_REG, OP_REG, OP_NONE);
    instr_f("fwriteb","fwrite to op1 file*(int) from op2(binary)", OP_REG, OP_REG, OP_NONE);
    instr_f("fwritebyte","write byte to op1 file*(int) op2 source(int)", OP_REG, OP_REG, OP_NONE);
    instr_f("fwritecdpt","write codepoint to op1 file*(int) op2 source(int)", OP_REG, OP_REG, OP_NONE);
    instr_f("fclearerr", "clearerr op1 file*(int)", OP_REG, OP_NONE, OP_NONE);
    instr_f("feof", "op1 rc(int) = feof op2 file*(int)", OP_REG, OP_REG, OP_NONE);
    instr_f("ferror", "op1 rc(int) = ferror op2 file*(int)", OP_REG, OP_REG, OP_NONE);

    // Signal Instructions
    instr_f("signal",  "Signal type op1", OP_STRING, OP_NONE, OP_NONE);
    instr_f("signalt", "Signal type op1 if op2 true", OP_STRING, OP_REG, OP_NONE);
    instr_f("signalf", "Signal type op1 if op2 false", OP_STRING, OP_REG, OP_NONE);
    instr_f("signal",  "Signal type op1 (message op2)", OP_STRING, OP_STRING, OP_NONE);
    instr_f("signal",  "Signal type op1 (payload op2)", OP_STRING, OP_REG, OP_NONE);
    instr_f("signalt", "Signal type op1 (message op2) if op3 true", OP_STRING, OP_STRING, OP_REG);
    instr_f("signalf", "Signal type op1 (message op2) if op3 false", OP_STRING, OP_STRING, OP_REG);

    instr_f("bpon",    "Enable Breakpoints with op1 handler", OP_FUNC, OP_NONE, OP_NONE);
    instr_f("bpon",    "Enable Breakpoints with existing handler", OP_NONE, OP_NONE, OP_NONE);
    instr_f("bpoff",   "Disable Breakpoints", OP_NONE, OP_NONE, OP_NONE);

    instr_f("sigignore", "Set Signal op1 Handle to Ignore", OP_STRING, OP_NONE, OP_NONE);
    instr_f("sighalt",   "Set Signal op1 Handle to Halt", OP_STRING, OP_NONE, OP_NONE);
    instr_f("sigshalt",  "Set Signal op1 Handle to Silent Halt", OP_STRING, OP_NONE, OP_NONE);
    instr_f("sigbr",     "Set Signal op2 Handle to Branch to op1", OP_ID, OP_STRING, OP_NONE);
    instr_f("sigcall",   "Set Signal op2 Handle to Call op1", OP_FUNC, OP_STRING, OP_NONE);
    instr_f("sigcallbr", "Set Signal op3 Handle to Call op2 returning to op1", OP_ID, OP_FUNC, OP_STRING);
    instr_f("sigret",    "Set Signal op1 Handle to Return", OP_STRING, OP_NONE, OP_NONE);

    instr_f("ichkrng", "if op1<op2 | op1>op3 signal OUT_OF_RANGE", OP_REG, OP_INT, OP_INT);
    instr_f("ichkrng", "if op1<op2 | op1>op3 signal OUT_OF_RANGE", OP_REG, OP_INT, OP_REG);
    instr_f("ichkrng", "if op1<op2 | op1>op3 signal OUT_OF_RANGE", OP_REG, OP_REG, OP_REG);
    instr_f("ichkrng", "if op1<op2 | op1>op3 signal OUT_OF_RANGE", OP_INT, OP_INT, OP_REG);
    instr_f("ichkrng", "if op1<op2 | op1>op3 signal OUT_OF_RANGE", OP_INT, OP_REG, OP_REG);

    // -------------------------------------------------------------------------
    // Decimal Plugin Library Instructions
    // -------------------------------------------------------------------------
    instr_f("decplnm", "Get Decimal Plugin Name op1=name op2=description op3=version", OP_REG, OP_REG, OP_REG);
    instr_f("setdgts", "Set Decimal Digits digits=op1", OP_REG, OP_NONE, OP_NONE);
    instr_f("setdgts", "Set Decimal Digits digits=op1", OP_INT, OP_NONE, OP_NONE);
    instr_f("getdgts", "Get Decimal Digits op1=digits", OP_REG, OP_NONE, OP_NONE);
    instr_f("stod", "Convert Decimal String to Decimal Number op1=s2dec(op2)", OP_REG, OP_NONE, OP_NONE);
    instr_f("dtos", "Convert Decimal Number to Decimal String op1=dec2s(op2)", OP_REG, OP_NONE, OP_NONE);
    instr_f("dtoi", "Convert Decimal Number to Integer op1=dec2s(op2)", OP_REG, OP_NONE, OP_NONE);
    instr_f("dtob", "Convert Decimal Number to Boolean op1=dec2s(op2)", OP_REG, OP_NONE, OP_NONE);
    instr_f("itod", "Convert Integer to Decimal Number op1=s2dec(op2)", OP_REG, OP_NONE, OP_NONE);
    instr_f("ftod", "Convert Float to Decimal Number op1=f2dec(op2)", OP_REG, OP_NONE, OP_NONE);
    instr_f("dtof", "Convert Decimal Number to Float op1=f2dec(op2)", OP_REG, OP_NONE, OP_NONE);
    instr_f("dsub", "Decimal Subtract (op1=op2-op3)", OP_REG, OP_REG, OP_REG);
    instr_f("dsub", "Decimal Subtract (op1=op2-op3)", OP_REG, OP_REG, OP_DECIMAL);
    instr_f("dsub", "Decimal Subtract (op1=op2-op3)", OP_REG, OP_DECIMAL, OP_REG);
    instr_f("dadd", "Decimal Add (op1=op2-op3)", OP_REG, OP_REG, OP_REG);
    instr_f("dadd", "Decimal Add (op1=op2-op3)", OP_REG, OP_REG, OP_DECIMAL);
    instr_f("dmult", "Decimal Multiply (op1=op2*op3)", OP_REG, OP_REG, OP_REG);
    instr_f("dmult", "Decimal Multiply (op1=op2*op3)", OP_REG, OP_REG, OP_DECIMAL);
    instr_f("ddiv", "Decimal Divide (op1=op2/op3)", OP_REG, OP_REG, OP_REG);
    instr_f("ddiv", "Decimal Divide (op1=op2/op3)", OP_REG, OP_REG, OP_DECIMAL);
    instr_f("ddiv", "Decimal Divide (op1=op2/op3)", OP_REG, OP_DECIMAL, OP_REG);
    instr_f("deq", "Decimal Equals op1=(op2==op3)", OP_REG, OP_REG, OP_REG);
    instr_f("deq", "Decimal Equals op1=(op2==op3)", OP_REG, OP_REG, OP_DECIMAL);
    instr_f("dne", "Decimal Not equals op1=(op2!=op3)", OP_REG, OP_REG, OP_REG);
    instr_f("dne", "Decimal Not equals op1=(op2!=op3)", OP_REG, OP_REG, OP_DECIMAL);
    instr_f("dgt", "Decimal Greater than op1=(op2>op3)", OP_REG, OP_REG, OP_REG);
    instr_f("dgt", "Decimal Greater than op1=(op2>op3)", OP_REG, OP_REG, OP_DECIMAL);
    instr_f("dgt", "Decimal Greater than op1=(op2>op3)", OP_REG, OP_DECIMAL , OP_REG);
    instr_f("dgte", "Decimal Greater than equals op1=(op2>=op3)", OP_REG, OP_REG, OP_REG);
    instr_f("dgte", "Decimal Greater than equals op1=(op2>=op3)", OP_REG, OP_REG, OP_DECIMAL);
    instr_f("dgte", "Decimal Greater than equals op1=(op2>=op3)", OP_REG, OP_DECIMAL, OP_REG);
    instr_f("dlt", "Decimal Less than op1=(op2<op3)", OP_REG, OP_REG, OP_REG);
    instr_f("dlt", "Decimal Less than op1=(op2<op3)", OP_REG, OP_REG, OP_DECIMAL);
    instr_f("dlt", "Decimal Less than op1=(op2<op3)", OP_REG, OP_DECIMAL, OP_REG);
    instr_f("dlte", "Decimal Less than equals op1=(op2<=op3)", OP_REG, OP_REG, OP_REG);
    instr_f("dlte", "Decimal Less than equals op1=(op2<=op3)", OP_REG, OP_REG, OP_DECIMAL);
    instr_f("dlte", "Decimal Less than equals op1=(op2<=op3)", OP_REG, OP_DECIMAL, OP_REG);
    instr_f("dgtbr", "Decimal Greater than if (op2>op3) goto op1", OP_ID, OP_REG, OP_REG);
    instr_f("dltbr", "Decimal Less than if (op2<op3) goto op1", OP_ID, OP_REG, OP_REG);
    instr_f("deqbr", "Decimal Equal if (op2=op3) goto op1", OP_ID, OP_REG, OP_REG);
    instr_f("dcopy", "Copy Decimal op2 to op1", OP_REG, OP_REG, OP_NONE);
    instr_f("dextr", "op3 decimal extracted to op1 string coefficient and op2 int decimal exponent", OP_REG, OP_REG, OP_REG);
    instr_f("dpow", "op1=op2**op3", OP_REG, OP_REG, OP_REG);
    instr_f("dpow", "op1=op2**op3", OP_REG, OP_REG, OP_DECIMAL);
    instr_f("dpow", "op1=op2**op3", OP_REG, OP_DECIMAL, OP_REG);
    instr_f("dsex", "Decimal op1 = -op1 (sign change)", OP_REG, OP_NONE,OP_NONE);
    // ---------------------------------------------------------------------------------------
    // end of decimal instructions
    // ---------------------------------------------------------------------------------------

    // META Instructions to support the RXVM Metadata
    instr_f("metaloadmodule", "Load Module (op1 = module num of last loaded module in rxbin op2)", OP_REG, OP_REG, OP_NONE);
    instr_f("metaloadedmodules", "Loaded Modules (op1 = array loaded modules)", OP_REG, OP_NONE, OP_NONE);
    instr_f("metaloadedprocs", "Loaded Procedures (op1 = array procedures in module op2)", OP_REG, OP_REG, OP_NONE);
    instr_f("metaloadedeprocs", "Loaded Exposed Procedures (op1 = array procedures in module op2)", OP_REG, OP_REG, OP_NONE);
    instr_f("metaloadinst", "Load Instruction Code (op1 = (inst)op2[op3])", OP_REG, OP_REG, OP_REG);
    instr_f("metadecodeinst", "Decode opcode (op1 decoded op2)", OP_REG, OP_REG, OP_NONE);
    instr_f("metaloadioperand", "Load Integer/Index Operand (op1 = (int)op2[op3])", OP_REG, OP_REG, OP_REG);
    instr_f("metaloadfoperand", "Load Float Operand (op1 = (float)op2[op3])", OP_REG, OP_REG, OP_REG);
    instr_f("metaloadsoperand", "Load String Operand (op1 = (string)op2[op3])", OP_REG, OP_REG, OP_REG);
    instr_f("metaloadpoperand", "Load Procedure Operand (op1 = (proc)op2[op3])", OP_REG, OP_REG, OP_REG);
    instr_f("metaloaddata", "Load Metadata (op1 = (metadata)op2[op3])", OP_REG, OP_REG, OP_REG);
    instr_f("metalinkpreg", "Link parent-frame-register[op2] to op1", OP_REG, OP_REG, OP_NONE);
    instr_f("metaloadcalleraddr", "Load caller address object to op1", OP_REG, OP_NONE, OP_NONE);

    /* ENDINSTRUCTIONS */

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

/* Print Instruction Database */
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
        case OP_DECIMAL: return "DECIMAL";
        case OP_BINARY: return "BINARY";
        case OP_NONE:
        default:
            return "NONE";
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
