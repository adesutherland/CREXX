/*
 * RXAS - Keyhole Optimiser Logic
 */

#include "rxasassm.h"
#include "rxvminst.h"

#include "string.h"

/* This defines an instruction to be searched for or as a template output */
typedef struct instruction_pattern {
    enum queue_item_type inst_type;
    char* instruction;
    char optype1;
    size_t opnum1;
    char optype2;
    size_t opnum2;
    char optype3;
    size_t opnum3;
} instruction_pattern;

/* This defines the rule flag */
typedef enum
{
    END_OF_RULE,    /* Marks the end of a rule                                            */
    NO_GAP,         /* No instruction between the last match and this instruction         */
    NO_HAZARD,      /* No hazard instruction (like a branch) before this instruction      */
                    /* This is used for non-branch rules                                  */
    ANY_GAP         /* Any instructions are allowed before this matched instruction       */
                    /* This is for branch rules                                           */
}   rule_flag;

typedef struct rule {
    rule_flag flag;
    instruction_pattern in;
    instruction_pattern out;
    instruction_pattern out2;
} rule;

/* We can only have 10 (0..9) operands of each type in our rules */
#define MAX_OP_MAP 10

/* Rules Operands Mapping */
typedef struct op_map {
   /* Holds the mapped values */
   size_t reg[MAX_OP_MAP];
   char regtp[MAX_OP_MAP]; /* register type: r,g or a */
   rxinteger integer[MAX_OP_MAP];
   unsigned char *string[MAX_OP_MAP];
   unsigned char character[MAX_OP_MAP];
   double real[MAX_OP_MAP];
   char* branch[MAX_OP_MAP]; /* Branch IDs */
   char* label[MAX_OP_MAP]; /* Branch LABELs */
   char* proc[MAX_OP_MAP]; /* Procedure IDs */

   /* Tokens to show what maps are set and the holding the defining token */
   Assembler_Token *reg_token[MAX_OP_MAP];
   Assembler_Token *integer_token[MAX_OP_MAP];
   Assembler_Token *string_token[MAX_OP_MAP];
   Assembler_Token *binary_token[MAX_OP_MAP];
   Assembler_Token *decimal_token[MAX_OP_MAP];
   Assembler_Token *character_token[MAX_OP_MAP];
   Assembler_Token *real_token[MAX_OP_MAP];
   Assembler_Token *branch_token[MAX_OP_MAP];
   Assembler_Token *label_token[MAX_OP_MAP];
   Assembler_Token *proc_token[MAX_OP_MAP];

   /* Instructions matched in the rules */
   rule *inst_mapped[OPTIMISER_TARGET_MAX_QUEUE_SIZE + OPTIMISER_QUEUE_EXTRA_BUFFER_SIZE];
} op_map;

/* The COVID-Opt Keyhole Optimiser Rules
 *
 * Each rule set is made up of a number of instructions mappings (individual
 * rules). A rule set ends with a rule flagged END_OF_RULE. The rule sets as a
 * whole end with a rule set just made up of a rule flagged END_OF_RULE
 *
 * Each rule has
 * a) a flag indicating
 * - END_OF_RULE described above
 * - NO_GAP There can be no instructions before this rule in the ruleset
 * - NO_HAZARD There can be non-hazardous instructions before matching this rule
 * - ANY_GAP There can be any instructions before matching this rule
 *
 * b) The input mapping which is used to map a rule to an instruction.
 * c) 0,1, or 2 output template mappings that are used to replace the
 *    mapped instruction.
 * d) syntax of the mapping rules
 * *          *  flag type   type     inst    op1 v1  op2  v2 op3  v3
 * input    *  {NO_HAZARD, OP_CODE, "instruction", 'r', 0, 'r', 0, 'r', 1,
 * output 1 *              OP_CODE, "instruction", 'r', 0, 'r', 1,  0, 0},
 * output 2 *
 * input    *  {END_OF_RULE},  ...
 *
 *  op1/op2/op3 are the parameter types of the instruction, r: register, l: label, b: branch, s: string, d: decimal,
 *                                                          h: hex (binary), c: character, f: float
 *  v1/v2/v3 is the temporary variable number in which the parameter content is kept, for the optimising statement.
 *     0:    parameter is not kept
 *     1-10: parameter is kept in the specified variable
 *     This number can be used in the optimised template and allows to merge parts of several input templates
 * e) the variable must also be used to make sure that instructions belong together, assume we want to replace
 *    the 2 following instructions by a new instruction:
 *    1. we can optimise
 *       igt r0,r3,r1
 *       brt l15doend,r0
 *
 *    2. we can't optimise
 *       igt r99,r3,r1
 *       brt l15doend,r7
 *       as r99 is set in igt, but r7 is used to invoke the branch.
 *    We therefore need to setup the rule as:
 *        {ANY_GAP,   OP_CODE, "igt",   'r', 4, 'r', 1, 'r', 2},
 *        {NO_GAP,   OP_CODE, "brt",   'b', 3, 'r', 4,  0,  0,
 *                   OP_CODE, "igtbr", 'b', 3, 'r', 1, 'r', 2},
 *    we must save the boolean register (variable 4), which then is used in the brt rule to make sure the instruction
 *    sequence is matching to above sample 1
 *
 * All the rules of a ruleset need to map to instructions correctly. When they do
 * all the mapped instructions are replaced by the output templates.
 *
 * Each instruction mapping can have a type of
 * OP_CODE - INSTRUCTION (a normal instruction)
 * ASM_LABEL - LABEL (a label instruction)
 *
 * The operand matching is done by mapping the actual register to the rules register number,
 * when that actual register is found again it keeps the same mapping. So each
 * input rule much match the instruction and operands. See examples!
 *
 * NO_HAZARD
 * In this mode, there may be other instructions in the source code between the
 * matched input instructions as long as they are not hazardous including being
 * 'irrelevant', meaning that they do not use any of the register operands in
 * the matching rules. Again see examples!
 *
 * Hazardous instructions, change the data flow and include:
 * - labels (causes un-analysable flow control)
 * - branches (causes un-analysable flow control)
 * - function calls (these use dynamic registers)
 * - Procedure boundaries
 * - instructions not part of the ruleset but using registers used in the ruleset
 *
 * If there is a match then each found instruction is removed from the queue
 * and replaced with the output instruction templates (if any).
 *
 * NOTE that a branch to an unconditional branch is optimised later, as part of
 * the assembler branch address backpacking logic, so no rules are needed for
 * these scenarios
 *
 * Annotated Examples (see after this comment block for the actual rule declarations)
 *
 * 1. Rule for two swaps cancelling out: swap r0,r1; swap r0,r1
 *
 *          *  flag type   type      inst   op1     op2     op3
 * input    *  {NO_HAZARD, OP_CODE, "swap", 'r', 0, 'r', 1, 0, 0,
 * output 1 *               0},
 * output 2 *
 * input    *  {NO_HAZARD, OP_CODE, "swap", 'r', 0, 'r', 1, 0, 0,
 * output 1 *               0},
 * output 2 *
 * input    *  {END_OF_RULE},
 *
 * This rule helps the situation where there are two consecutive calls to procedures
 * and the compiler swaps back a register after the first call only to swap
 * it back for the next call. Note that a second ruleset is needed for the case
 * where the register numbers are reversed in the second swap (see the rules after
 * this comment block)
 *
 * NO_HAZARD is used as a branch in or out, or use of a relevant register would
 * invalidate the rule logic
 *
 * Example 1.1
 *   swap r4,r8
 *   swap r4,r8
 *
 * Rule matches - rule r0 maps to register r4, and rule r1 maps to register r8
 * No output - the swaps are removed
 *
 * Example 1.2
 *   swap r4,r8
 *   iadd r2,r3,r5
 *   swap r4,r8
 *
 * Rule matches - rule r0 maps to register r4, and rule r1 maps to register r8
 * The iadd instruction is 'irrelevant' as it doesn't use r4 or r8
 * Output - the swaps are removed
 *   iadd r2,r3,r5
 *
 * Example 1.3
 *   swap r4,r8
 *   say r4
 *   swap r4,r8
 *
 * Rule does not match - although rule r0 maps to register r4, and rule r1 maps
 * to register r8 the say instruction uses r4. So no match, the swaps cannot be
 * removed
 * Output same as input - no change
 *
 *
 * 2. Rule for converting a concat to a faster append
 *    concat r0,r0,r1 to append r0,r1
 *
 *          *  flag type   type     inst      op1     op2     op3
 * input    *  {NO_HAZARD, OP_CODE, "concat", 'r', 0, 'r', 0, 'r', 1,
 * output 1 *              OP_CODE, "append", 'r', 0, 'r', 1,  0, 0},
 * output 2 *
 * input    *  {END_OF_RULE},
 *
 * Example 2.1
 *   concat r4,r4,r8
 *
 * Rule matches - rule r0 maps to register r4, and rule r1 maps to register r8
 * noting that the first two operands are the same register
 * Output - concat is removed and replaced with the faster append
 *   append r4,r8
 *
 *
 * 3. Rule for optimising an unconditional branch (br) to a branch true (brt),
 *    converting this to a brtf, and reducing the number of branches program flow
 *    needs to go through
 *
 *    This is a complex ruleset and one of 3 rulesets (currently) designed to improve
 *    performance by reducing branches to branches
 *
 *          *  flag type   type     inst    op1     op2     op3
 * input    *  {ANY_GAP,   OP_CODE, "br",    'b', 0,  0,  0,  0,  0,
 * output 1 *              OP_CODE, "brtf",  'b', 1, 'b', 2, 'r', 0},
 * output 2 *
 * input    *  {ANY_GAP,   ASM_LABEL,0,       'l', 0,  0,  0,  0,  0,
 * output 1 *              ASM_LABEL,0,       'l', 0,  0,  0,  0,  0},
 * output 2 *
 * input    *  {NO_GAP,    OP_CODE, "brt",   'b', 1, 'r', 0,  0,  0,
 * output 1 *              OP_CODE, "brt",   'b', 1, 'r', 0,  0,  0,
 * output 2 *              ASM_LABEL,0,       'l', 2,  0,  0,  0,  0},
 * input    *  {END_OF_RULE},
 *
 * Especially with control statements (like IF) the compiler glues the
 * different logic blocks together with branches, this leads to scenarios
 * where a branch jumps directly to another branch.
 *
 * Rule 1 matches an unconditional branch which it proposes to change to a conditional
 *        direct branch to the two eventual destinations
 * Rule 2 matches to a label which is the destination of the previous matched br.
 *        ANY_GAP is used as intervening instruction can be safely ignored
 *        The output template shows this label is unchanged
 * Rule 3 matches the brt. The NO_GAP indicated that the brt must directly
 *        follow the label above. The output of this rule is the brt followed
 *        by a new label (to be used by the new brtf, from rule 1)
 *
 * The important details is the 'l' and 'b' mappings. These are intrinsically linked
 * ('b' 0  branches to label 'l' 0). So
 *   b0 (rule 1) maps to l0 (rule 2)
 *   b1 (rule 3) is the branch true target and is used in the rule 1 output
 *   b2 is special - there is no input b2 so the system creates a new unique
 *      label for it. Rule 1 uses this as the branch false destination, and rule
 *      3's output 2 makes the required label instruction.
 *
 * NOTE that by the nature of a keyhole optimiser this optimisation only works when
 * the branches are near (currently upto 20 instructions or so apart) each other.
 *
 * Example 3.1
 *
 *     br lb1
 *     ...
 *   lb1:
 *     brt lb2,r3
 *     instf
 *     ...
 *   lb2:
 *     instt
 *
 * As can be seen the control flow from "br lb1" either ends up at
 *   "instt" if r3 is true, or "instf" but requires two branches.
 *
 * The output from the ruleset is:
 *
 *     brtf lb2,lbnew,r3
 *     ...
 *   lb1:
 *     brt lb2,r3
 *   lbnew:
 *     instf
 *     ...
 *   lb2:
 *     instt
 *
 * As can be seen the branch now goes directly to the eventual destinations. The
 * rest of the code is unchanged as other logic may be branching to lb1 and lb2
 * by leaving this alone we know we are not breaking other areas. (Note that after
 * disassembly lp1 may be removed if it is not used by anu other code)
 *
 */
rule rules[] =

        {

            /* Two swaps cancelling out: swap r0,r1; swap r0,r1 */
            {NO_HAZARD, OP_CODE,"swap", 'r', 0, 'r', 1, 0, 0,
                         0},
            {NO_HAZARD, OP_CODE,"swap", 'r', 0, 'r', 1, 0, 0,
                         0},
            {END_OF_RULE},

            /* Two swaps cancelling out: swap r0,r1; swap r1,r0 */
            {NO_HAZARD, OP_CODE,"swap", 'r', 0, 'r', 1, 0, 0,
                         0},
            {NO_HAZARD, OP_CODE,"swap", 'r', 1, 'r', 0, 0, 0,
                         0},
            {END_OF_RULE},

            /* sconcat to sappend: sconcat r0,r0,r1 to sappend r0,r1 */
            {NO_HAZARD, OP_CODE,"sconcat", 'r', 0, 'r', 0, 'r', 1,
                        OP_CODE,"sappend", 'r', 0, 'r', 1, 0, 0},
            {END_OF_RULE},

            /* concat to append: concat r0,r0,r1 to append r0,r1 */
            {NO_HAZARD, OP_CODE,"concat", 'r', 0, 'r', 0, 'r', 1,
                        OP_CODE,"append", 'r', 0, 'r', 1, 0, 0},
            {END_OF_RULE},

            /* Unconditional branch to branch true mapped to brtf*/
            {ANY_GAP,   OP_CODE,"br",  'b', 0,  0,  0,  0,  0,
                        OP_CODE,"brtf",'b', 1, 'b', 2, 'r', 0},
            {ANY_GAP,   ASM_LABEL,0,     'l', 0,  0,  0,  0,  0,
                        ASM_LABEL,0,     'l', 0,  0,  0,  0,  0},
            {NO_GAP,    OP_CODE,"brt", 'b', 1, 'r', 0,  0,  0,
                        OP_CODE,"brt", 'b', 1, 'r', 0,  0,  0,
                        ASM_LABEL,0,     'l', 2,  0,  0,  0,  0},
            {END_OF_RULE},

            /* Unconditional branch to branch false mapped to brtf*/
            {ANY_GAP,   OP_CODE,"br",  'b', 0,  0,  0,  0,  0,
                        OP_CODE,"brtf",'b', 2, 'b', 1, 'r', 0},
            {ANY_GAP,   ASM_LABEL,0,     'l', 0,  0,  0,  0,  0,
                        ASM_LABEL,0,     'l', 0,  0,  0,  0,  0},
            {NO_GAP,    OP_CODE,"brf", 'b', 1, 'r', 0,  0,  0,
                        OP_CODE,"brf", 'b', 1, 'r', 0,  0,  0,
                        ASM_LABEL,0,     'l', 2,  0,  0,  0,  0},
            {END_OF_RULE},

            /* Unconditional branch to branch true false to brtf*/
            {ANY_GAP,   OP_CODE,"br",  'b', 0,  0,  0,  0,  0,
                        OP_CODE,"brtf",'b', 1, 'b', 2, 'r', 0},
            {ANY_GAP,   ASM_LABEL,0,     'l', 0,  0,  0,  0,  0,
                        ASM_LABEL,0,     'l', 0,  0,  0,  0,  0},
            {NO_GAP,    OP_CODE,"brtf",'b', 1, 'b', 2, 'r', 0,
                        OP_CODE,"brtf",'b', 1, 'b', 2, 'r', 0},
            {END_OF_RULE},

            /* brt to brt with same condition */
            {ANY_GAP,   OP_CODE,"brt",  'b', 0, 'r', 0,  0,  0,
                        OP_CODE,"brt",  'b', 1, 'r', 0,  0,  0},
            {ANY_GAP,   ASM_LABEL,0,      'l', 0,  0,  0,  0,  0,
                        ASM_LABEL,0,      'l', 0,  0,  0,  0,  0},
            {NO_GAP,    OP_CODE,"brt",  'b', 1, 'r', 0,  0,  0,
                        OP_CODE,"brt",  'b', 1, 'r', 0,  0,  0},
            {END_OF_RULE},

            /* brf to brf with same condition */
            {ANY_GAP,   OP_CODE,"brf",  'b', 0, 'r', 0,  0,  0,
                        OP_CODE,"brf",  'b', 1, 'r', 0,  0,  0},
            {ANY_GAP,   ASM_LABEL,0,      'l', 0,  0,  0,  0,  0,
                        ASM_LABEL,0,      'l', 0,  0,  0,  0,  0},
            {NO_GAP,    OP_CODE,"brf",  'b', 1, 'r', 0,  0,  0,
                        OP_CODE,"brf",  'b', 1, 'r', 0,  0,  0},
            {END_OF_RULE},

            /* brt to brf with same condition */
            {ANY_GAP,   OP_CODE,"brt",  'b', 0, 'r', 0,  0,  0,
                        OP_CODE,"brt",  'b', 2, 'r', 0,  0,  0},
            {ANY_GAP,   ASM_LABEL,0,      'l', 0,  0,  0,  0,  0,
                        ASM_LABEL,0,      'l', 0,  0,  0,  0,  0},
            {NO_GAP,    OP_CODE,"brf",  'b', 1, 'r', 0,  0,  0,
                        OP_CODE,"brf",  'b', 1, 'r', 0,  0,  0,
                        ASM_LABEL,0,      'l', 2,  0,  0,  0,  0},
            {END_OF_RULE},

            /* brf to brt with same condition */
            {ANY_GAP,   OP_CODE,"brf",  'b', 0, 'r', 0,  0,  0,
                        OP_CODE,"brf",  'b', 2, 'r', 0,  0,  0},
            {ANY_GAP,   ASM_LABEL,0,      'l', 0,  0,  0,  0,  0,
                        ASM_LABEL,0,      'l', 0,  0,  0,  0,  0},
            {NO_GAP,    OP_CODE,"brt",  'b', 1, 'r', 0,  0,  0,
                        OP_CODE,"brt",  'b', 1, 'r', 0,  0,  0,
                        ASM_LABEL,0,      'l', 2,  0,  0,  0,  0},
            {END_OF_RULE},

            /* do loop improvement: replace igt/brt check by igtbr instruction:     igt r0,r3,r4; brt label,r0 => igtbr label,r3,r4  */
              {ANY_GAP,   OP_CODE, "igt",   'r', 4, 'r', 1, 'r', 2},
            {NO_GAP,   OP_CODE, "brt",   'b', 3, 'r', 4,  0,  0,
                                OP_CODE, "igtbr", 'b', 3, 'r', 1, 'r', 2},
            {END_OF_RULE},

            /* replace fgt/brf by igtbr instruction:     fgt r0,r3,r4; brf label,r0 => fgtbr label,r3,r4  */
            {ANY_GAP,   OP_CODE, "fgt",   'r', 4, 'r', 1, 'r', 2},
            {NO_GAP,   OP_CODE, "brf",   'b', 3, 'r', 4,  0,  0,
                    OP_CODE, "fgtbr", 'b', 3, 'r', 1, 'r', 2},
            {END_OF_RULE},

                /*  do loop increase ctr+1 and branch to start   inc rx; br dostart */
            {NO_GAP,   OP_CODE, "inc",   'r', 1, 0, 0, 0, 0},
            {NO_GAP,   OP_CODE, "br",   'b', 2, 0, 0,  0,  0,
                                OP_CODE, "bctp", 'b', 2, 'r', 1, 0, 0},
            {END_OF_RULE},
            /* NOTE Branch to unconditional branch is optimised later anyway so
             * no rule needed for these scenarios */

            /* End of all rules marker */
            {END_OF_RULE}
        };

/* Assembler_Token to reg type */
static char reg_type(Assembler_Token *opToken) {
    switch(opToken->token_type) {
        case RREG:
            return 'r';
        case GREG:
            return 'g';
        case AREG:
            return 'a';
        default: return 0;
    }
}

/* Check if a hazardous instruction is at the end of the queue. For
 * example a branch instruction (i.e. an instruction with a label or function
 * target changes the flow of control, breaking simple keyhole logic).
 * In which case we flush the queue before adding the next instruction */
static int is_hazardous(enum queue_item_type type, Assembler_Token *instrToken, Assembler_Token *operand1Token,
                        Assembler_Token *operand2Token, Assembler_Token *operand3Token) {

    if (type == OP_CODE) {
        /* Calls or branches */
        if (operand1Token &&
            (operand1Token->token_type == ID || operand1Token->token_type == FUNC))
            return 1;

        if (operand2Token &&
            (operand2Token->token_type == ID || operand2Token->token_type == FUNC))
            return 1;

        if (operand3Token &&
            (operand3Token->token_type == ID || operand3Token->token_type == FUNC))
            return 1;

        /* Other hazardous instructions - hard coded */
        /*
        if (!strcmp((char*)(instrToken->token_value.string), "a_hazardous_instruction")) return 1;
        */
    }

    else if (type == ASM_LABEL) return 1;

    return 0;
}

/*
 * Return 1 if the instrToken is relevant.
 * Relevant means it uses a mapped register
 */
static int is_relevant(op_map *map, Assembler_Token *opToken) {
    int i;
    char r_tp;

    if (!opToken) return 0;
    r_tp = reg_type(opToken);
    if (!r_tp) return 0;

    for (i=0; i<MAX_OP_MAP; i++) {
        if (map->reg_token[i] &&
            map->regtp[i] == r_tp &&
            map->reg[i] == opToken->token_value.integer) return 1;
    }
    return 0;
}

/* Checks if a instrToken can map against an op_type
 * This checks the operand type and number is consistent with existing mapped operands
 * returns 1 if it can map, otherwise 0
 * NOTE: The *map structure is NOT updated
 * See map_operand() */
static int can_map_operand(op_map *map, Assembler_Token *opToken, char op_type, size_t op_num) {

    if (!opToken) {
        if (op_type) return 0;
        else return 1;
    }

    switch(op_type) {
        case 'r': /* Register */
            if ( !(opToken->token_type == RREG ||
                   opToken->token_type == GREG ||
                   opToken->token_type == AREG ) ) return 0; /* Wrong Type */

            if (map->reg_token[op_num]) { /* Already Mapped - checked consistent */
                if (map->regtp[op_num] != reg_type(opToken) ||
                    map->reg[op_num] != opToken->token_value.integer)
                    return 0; /* Wrong register */
            }
            return 1;

        case 'i': /* Integer */
            if (opToken->token_type != INT ) return 0; /* Wrong Type */
            if (map->integer_token[op_num]) { /* Already Mapped - checked consistent */
                if (map->integer[op_num] != opToken->token_value.integer)
                    return 0; /* Wrong value */
            }
            return 1;

        case 's': /* String */
            if (opToken->token_type != STRING ) return 0; /* Wrong Type */
            if (map->string_token[op_num]) { /* Already Mapped - checked consistent */
                if (map->string[op_num] != opToken->token_value.string) // TODO - Shouldn't this be strcmp?
                    return 0; /* Wrong value */
            }
            return 1;

        case 'h': /* Hex (Binary) */
            if (opToken->token_type != HEX ) return 0; /* Wrong Type */
            if (map->binary_token[op_num]) { /* Already Mapped - checked consistent */
                if (map->string[op_num] != opToken->token_value.string) // TODO - Shouldn't this be strcmp?
                    return 0; /* Wrong value */
            }
            return 1;

        case 'd': /* Decimal */
            if (opToken->token_type != DECIMAL ) return 0; /* Wrong Type */
            if (map->decimal_token[op_num]) { /* Already Mapped - checked consistent */
                if (map->string[op_num] != opToken->token_value.string) // TODO - Shouldn't this be strcmp?
                    return 0; /* Wrong value */
            }
            return 1;

        case 'c': /* Char */
            if (opToken->token_type != CHAR ) return 0; /* Wrong Type */
            if (map->character_token[op_num]) { /* Already Mapped - checked consistent */
                if (map->character[op_num] != opToken->token_value.character)
                    return 0; /* Wrong value */
            }
            return 1;

        case 'f': /* Float */
            if (opToken->token_type != FLOAT ) return 0; /* Wrong Type */
            if (map->real_token[op_num]) { /* Already Mapped - checked consistent */
                if (map->real[op_num] != opToken->token_value.real)
                    return 0; /* Wrong value */
            }
            return 1;

        case 'l': /* Label */
            /* The l and b tokens are intrinsically linked - "quantum!" - different
             * token types but have matching values */
            if (opToken->token_type != LABEL ) return 0; /* Wrong Type */
            if (map->label_token[op_num]) { /* Already Mapped - checked consistent */
                if (strcmp(map->label[op_num], (char*)opToken->token_value.string) != 0)
                    return 0; /* Wrong value */
            }
            else if (map->branch_token[op_num]) { /* Already Mapped branch - checked consistent */
                if (strcmp(map->branch[op_num], (char*)opToken->token_value.string) != 0)
                    return 0; /* Wrong value */
            }

            return 1;

        case 'b': /* BRANCH */
            /* The l and b tokens are intrinsically linked - "quantum!" - different
             * token types but have matching values */
            if (opToken->token_type != ID ) return 0; /* Wrong Type */
            if (map->branch_token[op_num]) { /* Already Mapped - checked consistent */
                if (strcmp(map->branch[op_num], (char*)opToken->token_value.string) != 0)
                    return 0; /* Wrong value */
            }
            else if (map->label_token[op_num]) { /* Already Mapped - checked consistent */
                if (strcmp(map->label[op_num], (char*)opToken->token_value.string) != 0)
                    return 0; /* Wrong value */
            }
            return 1;

        case 'p': /* PROCEDURE */
            if (opToken->token_type != FUNC ) return 0; /* Wrong Type */
            if (map->proc_token[op_num]) { /* Already Mapped - checked consistent */
                if (strcmp(map->proc[op_num], (char*)opToken->token_value.string) != 0)
                    return 0; /* Wrong value */
            }
            return 1;

        case 0:  /* Check nulls match - We know opToken is not null so not a match */
        default: /* Something is wrong */
            return 0;
    }
}

/* Maps an instrToken against an op_type and op_num
 * This checks the operand is consistent with existing mapped operands
 * returns 1 if is does mapped successfully, otherwise 0
 * NOTE: if it does map the *map structure is updated
 * See can_map_operand() */
static int map_operand(op_map *map, Assembler_Token *opToken, char op_type, size_t op_num) {

    if (!opToken) {
        if (op_type) return 0;
        else return 1;
    }

    switch(op_type) {
        case 'r': /* Register */
            if ( !(opToken->token_type == RREG ||
                   opToken->token_type == GREG ||
                   opToken->token_type == AREG ) ) return 0; /* Wrong Type */

            if (map->reg_token[op_num]) { /* Already Mapped - checked consistent */
                if (map->regtp[op_num] != reg_type(opToken) ||
                    map->reg[op_num] != opToken->token_value.integer)
                    return 0; /* Wrong register */
            }
            else { /* Not mapped yet - map it */
                map->reg_token[op_num] = opToken;
                map->regtp[op_num] = reg_type(opToken);
                map->reg[op_num] = opToken->token_value.integer;
            }
            return 1;

        case 'i': /* Integer */
            if (opToken->token_type != INT ) return 0; /* Wrong Type */
            if (map->integer_token[op_num]) { /* Already Mapped - checked consistent */
                if (map->integer[op_num] != opToken->token_value.integer)
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                map->integer_token[op_num] = opToken;
                map->integer[op_num] = opToken->token_value.integer;
            }
            return 1;

        case 's': /* String */
            if (opToken->token_type != STRING ) return 0; /* Wrong Type */
            if (map->string_token[op_num]) { /* Already Mapped - checked consistent */
                if (map->string[op_num] != opToken->token_value.string) // TODO - Shouldn't this be strcmp?
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                map->string_token[op_num] = opToken;
                map->string[op_num] = opToken->token_value.string;
            }
            return 1;

        case 'h': /* Hex (Binary) */
            if (opToken->token_type != HEX ) return 0; /* Wrong Type */
            if (map->binary_token[op_num]) { /* Already Mapped - checked consistent */
                if (map->string[op_num] != opToken->token_value.string) // TODO - Shouldn't this be strcmp?
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                map->binary_token[op_num] = opToken;
                map->string[op_num] = opToken->token_value.string;
            }
            return 1;

        case 'd': /* Decimal */
            if (opToken->token_type != DECIMAL ) return 0; /* Wrong Type */
            if (map->decimal_token[op_num]) { /* Already Mapped - checked consistent */
                if (map->string[op_num] != opToken->token_value.string) // TODO - Shouldn't this be strcmp?
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                map->decimal_token[op_num] = opToken;
                map->string[op_num] = opToken->token_value.string;
            }
            return 1;

        case 'c': /* Char */
            if (opToken->token_type != CHAR ) return 0; /* Wrong Type */
            if (map->character_token[op_num]) { /* Already Mapped - checked consistent */
                if (map->character[op_num] != opToken->token_value.character)
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                map->character_token[op_num] = opToken;
                map->character[op_num] = opToken->token_value.character;
            }
            return 1;

        case 'f': /* Float */
            if (opToken->token_type != FLOAT ) return 0; /* Wrong Type */
            if (map->real_token[op_num]) { /* Already Mapped - checked consistent */
                if (map->real[op_num] != opToken->token_value.real)
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                map->real_token[op_num] = opToken;
                map->real[op_num] = opToken->token_value.real;
            }
            return 1;

        case 'l': /* Label */
            /* The l and b tokens are intrinsically linked - "quantum!" - different
             * token types but have matching values */
            if (opToken->token_type != LABEL ) return 0; /* Wrong Type */
            if (map->label_token[op_num]) { /* Already Mapped - checked consistent */
                if (strcmp(map->label[op_num], (char*)opToken->token_value.string) != 0)
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                if (map->branch_token[op_num]) { /* Already Mapped branch - checked consistent */
                    if (strcmp(map->branch[op_num], (char*)opToken->token_value.string) != 0)
                        return 0; /* Wrong value */
                }
                map->label_token[op_num] = opToken;
                map->label[op_num] = (char*)opToken->token_value.string;
            }
            return 1;

        case 'b': /* BRANCH */
            /* The l and b tokens are intrinsically linked - "quantum!" - different
             * token types but have matching values */
            if (opToken->token_type != ID ) return 0; /* Wrong Type */
            if (map->branch_token[op_num]) { /* Already Mapped - checked consistent */
                if (strcmp(map->branch[op_num], (char*)opToken->token_value.string) != 0)
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                if (map->label_token[op_num]) { /* Already Mapped - checked consistent */
                    if (strcmp(map->label[op_num], (char*)opToken->token_value.string) != 0)
                        return 0; /* Wrong value */
                }
                map->branch_token[op_num] = opToken;
                map->branch[op_num] = (char*)opToken->token_value.string;
            }
            return 1;

        case 'p': /* PROCEDURE */
            if (opToken->token_type != FUNC ) return 0; /* Wrong Type */
            if (map->proc_token[op_num]) { /* Already Mapped - checked consistent */
                if (strcmp(map->proc[op_num], (char*)opToken->token_value.string) != 0)
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                map->proc_token[op_num] = opToken;
                map->proc[op_num] = (char*)opToken->token_value.string;
            }
            return 1;

        case 0:  /* Check nulls match - We know opToken is not null so not a match */
        default: /* Something is wrong */
            return 0;
    }
}

/* Checks if an instruction can map a rule
 * return 1 if it can map, 0 otherwise */
static int can_map_instruction(op_map *map, instruction_queue *instruction, rule *rule) {

    switch (rule->in.inst_type) {
        case OP_CODE:
            if (instruction->instrType != OP_CODE)
                return 0; /* Not a normal instruction */

            if (strcmp((char *) (instruction->instrToken->token_value.string), rule->in.instruction) != 0)
                return 0; /* Not the right instruction */

            if (!can_map_operand(map, instruction->operand1Token, rule->in.optype1, rule->in.opnum1))
                return 0; /* Not the right operand (or mapping inconsistency */

            if (!can_map_operand(map, instruction->operand2Token, rule->in.optype2, rule->in.opnum2))
                return 0;

            if (!can_map_operand(map, instruction->operand3Token, rule->in.optype3, rule->in.opnum3))
                return 0;

            return 1;

        case ASM_LABEL:
            if (instruction->instrType != ASM_LABEL)
                return 0; /* Not a label */

            if (!can_map_operand(map, instruction->instrToken,
                             'l', rule->in.opnum1))
                return 0;
            return 1;

        default:
            return 0;
    }
}

/* Maps an instruction against a rule
 * Returns 1 on success, 0 if the map fails. *map may be changed on either case */
static int map_instruction(op_map *map, instruction_queue *instruction, rule *rule) {

    switch (rule->in.inst_type) {
        case OP_CODE:
            if (instruction->instrType != OP_CODE)
                return 0; /* Not a normal instruction */

            if (strcmp((char *) (instruction->instrToken->token_value.string), rule->in.instruction) != 0)
                return 0; /* Not the right instruction */

            if (!map_operand(map, instruction->operand1Token, rule->in.optype1, rule->in.opnum1))
                return 0; /* Not the right operand (or mapping inconsistency */

            if (!map_operand(map, instruction->operand2Token, rule->in.optype2, rule->in.opnum2))
                return 0;

            if (!map_operand(map, instruction->operand3Token, rule->in.optype3, rule->in.opnum3))
                return 0;

            return 1;

        case ASM_LABEL:
            if (instruction->instrType != ASM_LABEL)
                return 0; /* Not a label */

            if (!map_operand(map, instruction->instrToken,
                                 'l', rule->in.opnum1))
                return 0;
            return 1;

        default:
            return 0;
    }
}

/* Returns the mapped token for a rule */
static Assembler_Token* mapped_token(Assembler_Context *context, op_map *map, char op_type, size_t op_num) {
    Assembler_Token *t;
    char buffer[20];

    switch(op_type) {
        case 'r': /* Register */
            return map->reg_token[op_num];

        case 'i': /* Integer */
            return map->integer_token[op_num];

        case 's': /* String */
            return map->string_token[op_num];

        case 'h': /* Hex (Binary) */
            return map->binary_token[op_num];

        case 'd': /* Decimal */
            return map->decimal_token[op_num];

        case 'c': /* Char */
            return map->character_token[op_num];

        case 'f': /* Float */
            return map->real_token[op_num];

        case 'l': /* Label == Branch */
            t = map->label_token[op_num];
            if (t == 0) {
                /* Special functionality for labels / branches
                 * if label has not been defined in an input rule then a
                 * unique label token is created */

                /* If the intrinsically linked branch id is set make a pair */
                t = map->branch_token[op_num];
                if (t) t = rxas_tid(context, t, (char *) t->token_value.string);

                    /* Otherwise make a unique label - note that as it does not start with
                     * a letter, it cannot be a duplicate of a label from the
                     * rxas source file */
                else {
                    snprintf(buffer, 20, "%d", context->optimiser_counter++);
                    t = rxas_tid(context, NULL, buffer);
                }
                /* Store the created token */
                t->token_type = LABEL;
                map->label_token[op_num] = t;
            }
            return t;

        case 'b': /* Branch == Label */
            t = map->branch_token[op_num];
            if (t == 0) {
                /* Special functionality for labels / branches
                 * if branch id has not been defined in an input rule then a
                 * unique branch token is created */

                /* If the intrinsically linked label is set make a pair */
                t = map->label_token[op_num];
                if (t) t = rxas_tid(context, t, (char *) t->token_value.string);

                /* Otherwise make a unique label - note that as it does not start with
                 * a letter, it cannot be a duplicate of a label from the
                 * rxas source file */
                else {
                    snprintf(buffer, 20, "%d", context->optimiser_counter++);
                    t = rxas_tid(context, NULL, buffer);
                }
                /* Store the created token */
                map->branch_token[op_num] = t;
            }
            return t;

        case 'p': /* Procedure */
            return map->proc_token[op_num];

        case 0:   /* Check nulls match */
        default:  /* Something wrong */
            return 0;
    }
}

/* Optimise a rule starting from a specific instruction
 * returns 1 if the rule was successfully applied */
static int optimise_rule(Assembler_Context *context, op_map *map, rule *r, int inst_no) {
    int inst_no2;

    /* Clear Map */
    memset(map, 0, sizeof(*map));

    /* First check if the current instruction maps to the first rule  */
    if (map_instruction(map, &context->optimiser_queue[inst_no], r) != 1)
        return 0;

    map->inst_mapped[inst_no] = r;
    r++;

    /* Process next input rules */
    while (r->flag != END_OF_RULE && inst_no < context->optimiser_queue_items) {
        for (inst_no++; inst_no < context->optimiser_queue_items; inst_no++) {
            /* Check if it can match */
            if (can_map_instruction(map, &context->optimiser_queue[inst_no],r) == 1) {
                /* Can match - so make it so! */
                map_instruction(map, &context->optimiser_queue[inst_no], r);
                map->inst_mapped[inst_no] = r;
                r++;
            } else {
                /* Not a match - we need to check that skipping the instruction does not break the rule */
                if (    context->optimiser_queue[inst_no].instrType == OP_CODE ||
                        context->optimiser_queue[inst_no].instrType == ASM_LABEL ) {
                    /* Meta entries are always ignored */

                    if (r->flag == NO_GAP) return 0; /* No gap allowed! */
                    if (r->flag == NO_HAZARD) {
                        /* Is it a hazardous instruction like a branch? */
                        if (is_hazardous(
                                context->optimiser_queue[inst_no].instrType,
                                context->optimiser_queue[inst_no].instrToken,
                                context->optimiser_queue[inst_no].operand1Token,
                                context->optimiser_queue[inst_no].operand2Token,
                                context->optimiser_queue[inst_no].operand3Token))
                            return 0;

                        /* Is the instruction relevant (using some of the mapped registers) this
                         * is also a hazard */
                        if (is_relevant(map, context->optimiser_queue[inst_no].operand1Token) ||
                            is_relevant(map, context->optimiser_queue[inst_no].operand2Token) ||
                            is_relevant(map, context->optimiser_queue[inst_no].operand3Token))
                            return 0;
                    }
                }
            }
        }
    }

    if (r->flag == END_OF_RULE) {
        /* A match! We need to apply the output rule */
        /* Make sure inst_no is in range */
        if (inst_no >= context->optimiser_queue_items)
            inst_no = (int)context->optimiser_queue_items - 1;
        while (inst_no >= 0) {
            r = map->inst_mapped[inst_no];
            if (r) {
                /* Main output instruction */
                /*   r->in.instruction    which will be replaced
                 *   r->out.instruction   replacement instruction(s)
                 */
                switch (r->out.inst_type) {
                    case OP_CODE:
                        context->optimiser_queue[inst_no].instrType = OP_CODE;
                        context->optimiser_queue[inst_no].instrToken =
                                rxas_tid(context,
                                         context->optimiser_queue[inst_no].instrToken,
                                         r->out.instruction);
                        context->optimiser_queue[inst_no].operand1Token =
                                mapped_token(context, map, r->out.optype1, r->out.opnum1);
                        context->optimiser_queue[inst_no].operand2Token =
                                mapped_token(context, map, r->out.optype2, r->out.opnum2);
                        context->optimiser_queue[inst_no].operand3Token =
                                mapped_token(context, map, r->out.optype3, r->out.opnum3);
                        break;

                    case ASM_LABEL:
                        context->optimiser_queue[inst_no].instrType = ASM_LABEL;
                        context->optimiser_queue[inst_no].instrToken =
                                mapped_token(context, map, 'l', r->out.opnum1);
                        context->optimiser_queue[inst_no].operand1Token = 0;
                        context->optimiser_queue[inst_no].operand2Token = 0;
                        context->optimiser_queue[inst_no].operand3Token = 0;
                        break;

                    default:
                        /* No - output rule so remove instruction from the queue */
                        if ((int)context->optimiser_queue_items - (int)inst_no - 1 > 0) {
                            memmove(&context->optimiser_queue[inst_no],
                                    &context->optimiser_queue[inst_no + 1],
                                    sizeof(instruction_queue) *
                                    (context->optimiser_queue_items - inst_no - 1));
                        }
                        /* One less instruction in the queue */
                        context->optimiser_queue_items--;
                }

                /* Secondary output instruction */
                switch (r->out2.inst_type) {
                    case OP_CODE:
                        /* Insert instruction in the queue */
                        inst_no2 = inst_no + 1;
                        if ((int)context->optimiser_queue_items - inst_no2 > 0) {
                            memmove(&context->optimiser_queue[inst_no2 + 1],
                                    &context->optimiser_queue[inst_no2],
                                    sizeof(instruction_queue) *
                                    (context->optimiser_queue_items - inst_no2));
                        }
                        context->optimiser_queue_items++;

                        /* Add the instruction */
                        context->optimiser_queue[inst_no2].instrType = OP_CODE;
                        context->optimiser_queue[inst_no2].instrToken =
                                rxas_tid(context,
                                         context->optimiser_queue[inst_no2].instrToken,
                                         r->out2.instruction);
                        context->optimiser_queue[inst_no2].operand1Token =
                                mapped_token(context, map, r->out2.optype1, r->out2.opnum1);
                        context->optimiser_queue[inst_no2].operand2Token =
                                mapped_token(context, map, r->out2.optype2, r->out2.opnum2);
                        context->optimiser_queue[inst_no2].operand3Token =
                                mapped_token(context, map, r->out2.optype3, r->out2.opnum3);
                        break;

                    case ASM_LABEL:
                        /* Insert instruction in the queue */
                        inst_no2 = inst_no + 1;
                        if ((int)context->optimiser_queue_items - inst_no2 > 0) {
                            memmove(&context->optimiser_queue[inst_no2 + 1],
                                    &context->optimiser_queue[inst_no2],
                                    sizeof(instruction_queue) *
                                    (context->optimiser_queue_items - inst_no2));
                        }
                        context->optimiser_queue_items++;

                        /* Add the instruction */
                        context->optimiser_queue[inst_no2].instrType = ASM_LABEL;
                        context->optimiser_queue[inst_no2].instrToken =
                                mapped_token(context, map, 'l', r->out2.opnum1);
                        context->optimiser_queue[inst_no2].operand1Token = 0;
                        context->optimiser_queue[inst_no2].operand2Token = 0;
                        context->optimiser_queue[inst_no2].operand3Token = 0;
                        break;

                    default: ; /* No secondary instruction - nothing to be done */
                }

            }
            inst_no--;
        }
        return 1;
    }

    return 0; /* Not a match - ran out of instructions */
}

/* Optimise a rule across the queue of instructions
 * Updates the rule pointer to the start of the next rule
 * returns 1 if the rule was successfully applied */
static int optimise_rule_in_queue(Assembler_Context *context, rule **r ) {
    int i;
    op_map map;
    int result = 0;

    /* Work down each instruction as a start point */
    for (i=0; i<context->optimiser_queue_items; i++) {
        if (!strcmp((char*)(context->optimiser_queue[i].instrToken->token_value.string),
                    (*r)->in.instruction)) {
           if (optimise_rule(context, &map, *r, i)) {
               result = 1;
               break;
           }
        }
    }

    /* Skip to the next rule start */
    while ((*r)->flag != END_OF_RULE) (*r)++;
    (*r)++;

    return result;
}

static void optimise(Assembler_Context *context) {
    rule *r;
    int changed;
    do {
        changed = 0;
        r = &rules[0];
        while (r->flag != END_OF_RULE)
            if (optimise_rule_in_queue(context, &r)) changed = 1;
    } while (changed);
}

/* Execute Queued Item */
static void executeQueuedItem(Assembler_Context *context, instruction_queue *item) {

    switch  (item->instrType) {
        case ASM_LABEL:
            rxaslabl(context, item->instrToken);
            break;
        case OP_CODE:
            rxasgen(context, item->instrToken,
                    item->operand1Token,
                    item->operand2Token,
                    item->operand3Token);
            break;
        case FUNC_META:
            /* Queue Function Metadata */
            rxasmefu(context, item->instrToken, item->operand1Token, item->operand2Token,
                     item->operand3Token, item->operand4Token, item->operand5Token);
            break;
        case CONST_META:
            /* Queue Constant Metadata */
            rxasmect(context, item->instrToken, item->operand1Token, item->operand2Token,
                     item->operand3Token);
            break;
        case CLEAR_META:
            /* Queue Clear Metadata */
            rxasmecl(context, item->instrToken);
            break;
        case REG_META:
            /* Queue Register Metadata */
            rxasmere(context, item->instrToken, item->operand1Token, item->operand2Token,
                     item->operand3Token);
            break;
        case SRC_LINE:
            /* Queue Source Line */
            rxasmesr(context, item->instrToken, item->operand1Token, item->operand2Token);
            break;
        case SRC_FILE:
            /* Source File */
            rxasmefl(context, item->instrToken);
            break;
        default:;
    }
}

static void queue_instruction(Assembler_Context *context, enum queue_item_type type,
                              Assembler_Token *instrToken, Assembler_Token *operand1Token, Assembler_Token *operand2Token,
                              Assembler_Token *operand3Token, Assembler_Token *operand4Token, Assembler_Token *operand5Token) {

    /* Remove old instructions to get queue down to the target length */
    /* Note that instruction rules can add instructions to the queue  */
    while (context->optimiser_queue_items >= OPTIMISER_TARGET_MAX_QUEUE_SIZE) {
        executeQueuedItem(context, context->optimiser_queue);

        /* Move the queue */
        memmove(&context->optimiser_queue[0],
                &context->optimiser_queue[1],
                sizeof(instruction_queue) * (context->optimiser_queue_items - 1));

        /* One less instruction in the queue */
        context->optimiser_queue_items--;
    }

    /* Add to the end of the queue */
    context->optimiser_queue[context->optimiser_queue_items].instrType = type;
    context->optimiser_queue[context->optimiser_queue_items].instrToken = instrToken;
    context->optimiser_queue[context->optimiser_queue_items].operand1Token = operand1Token;
    context->optimiser_queue[context->optimiser_queue_items].operand2Token = operand2Token;
    context->optimiser_queue[context->optimiser_queue_items].operand3Token = operand3Token;
    context->optimiser_queue[context->optimiser_queue_items].operand4Token = operand4Token;
    context->optimiser_queue[context->optimiser_queue_items].operand5Token = operand5Token;
    context->optimiser_queue_items++;

    /* Optimise */
    optimise(context);
}

/* Queue code for the keyhole optimiser */
/* Queue opcode  */
void rxasque0(Assembler_Context *context, Assembler_Token *instrToken) {
    if (context->optimise) {
        queue_instruction( context, OP_CODE, instrToken, 0, 0, 0, 0, 0);
    }
    else rxasgen0(context, instrToken);
}

/* Queue opcode  */
void rxasque1(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token) {
    // Convert any FLOATS to DECIMALS
    promote_floats_to_decimals(instrToken, operand1Token, 0, 0);
    if (context->optimise) {
        queue_instruction(context, OP_CODE, instrToken, operand1Token, 0, 0, 0, 0);
    }
    else rxasgen1(context, instrToken, operand1Token);
}

/* Queue opcode  */
void rxasque2(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token,
              Assembler_Token *operand2Token) {
    // Convert any FLOATS to DECIMALS
    promote_floats_to_decimals(instrToken, operand1Token, operand2Token, 0);
    if (context->optimise) {
        queue_instruction(context, OP_CODE, instrToken, operand1Token, operand2Token, 0, 0, 0);
    }
    else rxasgen2(context, instrToken, operand1Token, operand2Token);
}

/* Queue opcode  */
void rxasque3(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token,
              Assembler_Token *operand2Token, Assembler_Token *operand3Token) {
    // Convert any FLOATS to DECIMALS
    promote_floats_to_decimals(instrToken, operand1Token, operand2Token, operand3Token);
    if (context->optimise) {
        queue_instruction(context, OP_CODE, instrToken, operand1Token, operand2Token, operand3Token, 0, 0);
    }
    else rxasgen3(context, instrToken, operand1Token, operand2Token, operand3Token);
}

/* Queue Label */
void rxasqlbl(Assembler_Context *context, Assembler_Token *labelToken) {
    if (context->optimise) {
        queue_instruction(context, ASM_LABEL, labelToken, 0, 0, 0, 0, 0);
    }
    else rxaslabl(context, labelToken);
}

/* Queue Source filename */
void rxasqmfl(Assembler_Context *context, Assembler_Token *file) {
    if (context->optimise) {
        queue_instruction(context, SRC_FILE, file, 0, 0, 0, 0, 0);
    }
    else rxasmefl(context, file);
}

/* Queue Source Line */
void rxasqmsr(Assembler_Context *context, Assembler_Token *line, Assembler_Token *column, Assembler_Token *source) {
    if (context->optimise) {
        queue_instruction(context, SRC_LINE, line, column, source, 0, 0, 0);
    }
    else rxasmesr(context, line, column, source);
}

/* Queue Function Metadata */
void rxasqmfu(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *func, Assembler_Token *args, Assembler_Token *inliner) {
    if (context->optimise) {
        queue_instruction(context, FUNC_META, symbol, option, type, func, args, inliner);
    }
    else rxasmefu(context, symbol, option, type, func, args, inliner);
}

/* Queue Register Metadata */
void rxasqmre(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *reg) {
    if (context->optimise) {
        queue_instruction(context, REG_META, symbol, option, type, reg, 0, 0);
    }
    else rxasmere(context, symbol, option, type, reg);
}

/* Queue Constant Metadata */
void rxasqmct(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *constant) {
    if (context->optimise) {
        queue_instruction(context, CONST_META, symbol, option, type, constant, 0, 0);
    }
    else rxasmect(context, symbol, option, type, constant);
}

/* Queue Clear Metadata */
void rxasqmcl(Assembler_Context *context, Assembler_Token *symbol) {
    if (context->optimise) {
        queue_instruction(context, CLEAR_META, symbol, 0, 0, 0, 0, 0);
    }
    else rxasmecl(context, symbol);
}

/* Flush the optimiser queue */
void flushopt(Assembler_Context *context) {
    size_t i;
    if (context->optimise) {
        /* Output the queue */
        for (i=0; i<context->optimiser_queue_items; i++) {
            executeQueuedItem(context, context->optimiser_queue +  i);
        }
        context->optimiser_queue_items = 0;
    }
}
