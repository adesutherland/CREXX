/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * RXAS - Keyhole Optimiser Logic
 */

#include "rxasassm.h"
#include "rxdefs.h"

#include <ctype.h>
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
   Assembler_Token *literal_reg_token[MAX_OP_MAP];
   size_t literal_reg[MAX_OP_MAP];
   char literal_regtp[MAX_OP_MAP];

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
 *  op1/op2/op3 are the parameter types of the instruction, r: register capture, R/G/A: literal local/global/arg register,
 *                                                          l: label, b: branch, s: string, d: decimal,
 *                                                          h: hex (binary), c: character, f: float
 *  v1/v2/v3 is the temporary variable number in which the parameter content is kept, for the optimising statement.
 *     0:    parameter is not kept
 *     1-10: parameter is kept in the specified variable
 *     This number can be used in the optimised template and allows to merge parts of several input templates
 *     For R/G/A operands this is a literal register number, not a capture slot.
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
 * matched input instructions as long as they do not block the rule. Blocking
 * instructions include control-flow barriers, procedure calls, explicit
 * optimiser barriers from the instruction database, and instructions that use
 * a register involved in the rule. Again see examples!
 *
 * The optimiser gets control-flow and barrier metadata from rxops.h:
 * - non-FLOW_NEXT instructions are barriers
 * - FLG_OPT_BARRIER marks FLOW_NEXT instructions that must not be skipped
 * - FLG_IMPLICIT_REG_USE marks instructions such as inc0/dec0 whose register
 *   use is not visible as a normal operand
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
            /* Fixed-register arithmetic shortcuts */
            {NO_HAZARD, OP_CODE,"inc", 'R', 0, 0, 0, 0, 0,
                        OP_CODE,"inc0", 0, 0, 0, 0, 0, 0},
            {END_OF_RULE},
            {NO_HAZARD, OP_CODE,"dec", 'R', 0, 0, 0, 0, 0,
                        OP_CODE,"dec0", 0, 0, 0, 0, 0, 0},
            {END_OF_RULE},
            {NO_HAZARD, OP_CODE,"inc", 'R', 1, 0, 0, 0, 0,
                        OP_CODE,"inc1", 0, 0, 0, 0, 0, 0},
            {END_OF_RULE},
            {NO_HAZARD, OP_CODE,"dec", 'R', 1, 0, 0, 0, 0,
                        OP_CODE,"dec1", 0, 0, 0, 0, 0, 0},
            {END_OF_RULE},
            {NO_HAZARD, OP_CODE,"inc", 'R', 2, 0, 0, 0, 0,
                        OP_CODE,"inc2", 0, 0, 0, 0, 0, 0},
            {END_OF_RULE},
            {NO_HAZARD, OP_CODE,"dec", 'R', 2, 0, 0, 0, 0,
                        OP_CODE,"dec2", 0, 0, 0, 0, 0, 0},
            {END_OF_RULE},

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

static int literal_reg_type_matches(Assembler_Token *opToken, char op_type) {
    switch (op_type) {
        case 'R':
            return opToken->token_type == RREG;
        case 'G':
            return opToken->token_type == GREG;
        case 'A':
            return opToken->token_type == AREG;
        default:
            return 0;
    }
}

static char literal_reg_type(char op_type) {
    switch (op_type) {
        case 'R':
            return 'r';
        case 'G':
            return 'g';
        case 'A':
            return 'a';
        default:
            return 0;
    }
}

static int can_map_literal_register(Assembler_Token *opToken, char op_type, size_t op_num) {
    if (!literal_reg_type_matches(opToken, op_type)) return 0;
    return opToken->token_value.integer == op_num;
}

static int map_literal_register(op_map *map, Assembler_Token *opToken, char op_type, size_t op_num) {
    int i;
    char regtp;

    if (!can_map_literal_register(opToken, op_type, op_num)) return 0;

    regtp = literal_reg_type(op_type);
    for (i = 0; i < MAX_OP_MAP; i++) {
        if (map->literal_reg_token[i] &&
            map->literal_regtp[i] == regtp &&
            map->literal_reg[i] == op_num) {
            return 1;
        }
    }

    for (i = 0; i < MAX_OP_MAP; i++) {
        if (!map->literal_reg_token[i]) {
            map->literal_reg_token[i] = opToken;
            map->literal_regtp[i] = regtp;
            map->literal_reg[i] = op_num;
            return 1;
        }
    }

    return 0;
}

static OperandType operand_type(Assembler_Token *opToken) {
    if (!opToken) return OP_NONE;

    switch(opToken->token_type) {
        case ID:
            return OP_ID;
        case RREG:
        case GREG:
        case AREG:
            return OP_REG;
        case FUNC:
            return OP_FUNC;
        case INT:
            return OP_INT;
        case FLOAT:
            return OP_FLOAT;
        case CHAR:
            return OP_CHAR;
        case STRING:
            return OP_STRING;
        case DECIMAL:
            return OP_DECIMAL;
        case HEX:
            return OP_BINARY;
        default:
            return OP_NONE;
    }
}

static int op_operands_match(OpFormat format, OperandType t1, OperandType t2, OperandType t3, int actual_operands) {
    OperandType types[3];
    int expected_operands;

    expected_operands = rxbin_get_operand_types(format, types);
    if (expected_operands != actual_operands) return 0;
    if (actual_operands > 0 && types[0] != t1) return 0;
    if (actual_operands > 1 && types[1] != t2) return 0;
    if (actual_operands > 2 && types[2] != t3) return 0;
    return 1;
}

static int mnemonic_matches(const char *mnemonic, const char *table_name) {
    int i = 0;
    while (mnemonic[i]) {
        if (toupper((unsigned char)mnemonic[i]) != table_name[i]) return 0;
        i++;
    }
    if (table_name[i] == 0 || table_name[i] == '_') return 1;
    return 0;
}

static const OpInfo *find_optimiser_opcode(Assembler_Token *instrToken,
                                           Assembler_Token *operand1Token,
                                           Assembler_Token *operand2Token,
                                           Assembler_Token *operand3Token) {
    const char *mnemonic;
    OperandType t1;
    OperandType t2;
    OperandType t3;
    int actual_operands;
    int i;

    if (!instrToken) return 0;

    mnemonic = (const char *) instrToken->token_value.string;
    t1 = operand_type(operand1Token);
    t2 = operand_type(operand2Token);
    t3 = operand_type(operand3Token);
    actual_operands = 0;
    if (operand1Token) actual_operands = 1;
    if (operand2Token) actual_operands = 2;
    if (operand3Token) actual_operands = 3;

    for (i = 0; op_table[i].mnemonic != NULL; i++) {
        if (op_operands_match(op_table[i].format, t1, t2, t3, actual_operands) &&
            mnemonic_matches(mnemonic, op_table[i].mnemonic)) {
            return &op_table[i];
        }
    }

    return 0;
}

/* Check if a skipped instruction is a hard barrier for NO_HAZARD rules. */
static int is_rule_barrier(enum queue_item_type type, Assembler_Token *instrToken, Assembler_Token *operand1Token,
                           Assembler_Token *operand2Token, Assembler_Token *operand3Token) {
    const OpInfo *op_info;

    if (type == OP_CODE) {
        op_info = find_optimiser_opcode(instrToken, operand1Token, operand2Token, operand3Token);
        if (op_info) {
            if (op_info->flow != FLOW_NEXT) return 1;
            if (op_info->flags & FLG_OPT_BARRIER) return 1;
        }

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
    }

    else if (type == ASM_LABEL) return 1;

    return 0;
}

static int map_has_register(op_map *map, char reg_type, size_t reg_num) {
    int i;

    for (i = 0; i < MAX_OP_MAP; i++) {
        if (map->reg_token[i] &&
            map->regtp[i] == reg_type &&
            map->reg[i] == reg_num) {
            return 1;
        }
        if (map->literal_reg_token[i] &&
            map->literal_regtp[i] == reg_type &&
            map->literal_reg[i] == reg_num) {
            return 1;
        }
    }

    return 0;
}

static int implicit_int_register_relevant(op_map *map, Assembler_Token *token, char reg_type) {
    if (!token || token->token_type != INT) return 0;
    if (token->token_value.integer < 0) return 0;
    return map_has_register(map, reg_type, (size_t) token->token_value.integer);
}

static int implicit_register_relevant(op_map *map,
                                      const OpInfo *op_info,
                                      Assembler_Token *operand1Token,
                                      Assembler_Token *operand2Token,
                                      Assembler_Token *operand3Token) {
    if (!op_info || !(op_info->flags & FLG_IMPLICIT_REG_USE)) return 0;

    switch (op_info->opcode) {
        case OP_LOAD_INT_INT:
            return implicit_int_register_relevant(map, operand1Token, 'r') ||
                   implicit_int_register_relevant(map, operand2Token, 'r');
        case OP_LOAD_INT_REG:
            return implicit_int_register_relevant(map, operand1Token, 'r');
        case OP_INC0:
        case OP_DEC0:
            return map_has_register(map, 'r', 0);
        case OP_INC1:
        case OP_DEC1:
            return map_has_register(map, 'r', 1);
        case OP_INC2:
        case OP_DEC2:
            return map_has_register(map, 'r', 2);
        case OP_LINKARG_REG_INT:
            return implicit_int_register_relevant(map, operand2Token, 'a');
        default:
            return 1;
    }
}

/*
 * Return 1 if the instrToken is relevant.
 * Relevant means it uses a mapped register
 */
static int is_relevant(op_map *map, Assembler_Token *opToken) {
    char r_tp;

    if (!opToken) return 0;
    r_tp = reg_type(opToken);
    if (!r_tp) return 0;

    return map_has_register(map, r_tp, (size_t) opToken->token_value.integer);
}

static int instruction_is_relevant(op_map *map, instruction_queue *instruction) {
    const OpInfo *op_info;

    if (is_relevant(map, instruction->operand1Token) ||
        is_relevant(map, instruction->operand2Token) ||
        is_relevant(map, instruction->operand3Token)) {
        return 1;
    }

    if (instruction->instrType != OP_CODE) return 0;

    op_info = find_optimiser_opcode(instruction->instrToken,
                                    instruction->operand1Token,
                                    instruction->operand2Token,
                                    instruction->operand3Token);
    return implicit_register_relevant(map,
                                      op_info,
                                      instruction->operand1Token,
                                      instruction->operand2Token,
                                      instruction->operand3Token);
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

        case 'R': /* Literal local register */
        case 'G': /* Literal global register */
        case 'A': /* Literal argument register */
            return can_map_literal_register(opToken, op_type, op_num);

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

        case 'R': /* Literal local register */
        case 'G': /* Literal global register */
        case 'A': /* Literal argument register */
            return map_literal_register(map, opToken, op_type, op_num);

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
                        /* Is it a barrier instruction like a branch or procedure call? */
                        if (is_rule_barrier(
                                context->optimiser_queue[inst_no].instrType,
                                context->optimiser_queue[inst_no].instrToken,
                                context->optimiser_queue[inst_no].operand1Token,
                                context->optimiser_queue[inst_no].operand2Token,
                                context->optimiser_queue[inst_no].operand3Token))
                            return 0;

                        /* Is the instruction relevant (using some of the mapped registers) this
                         * also blocks the rule. */
                        if (instruction_is_relevant(map, &context->optimiser_queue[inst_no]))
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
                     item->operand3Token, item->operand4Token);
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
        case CLASS_META:
            /* Queue Class Metadata */
            rxasmeclss(context, item->instrToken, item->operand1Token, item->operand2Token);
            break;
        case ATTR_META:
            /* Queue Attribute Metadata */
            rxasmeattr(context, item->instrToken, item->operand1Token, item->operand2Token, item->operand3Token);
            break;
        case INTERFACE_META:
            rxasmeintf(context, item->instrToken, item->operand1Token, item->operand2Token);
            break;
        case IMPLEMENTS_META:
            rxasmeimpl(context, item->instrToken, item->operand1Token);
            break;
        case MEMBER_META:
            rxasmememb(context, item->instrToken, item->operand1Token, item->operand2Token, item->operand3Token, item->operand4Token);
            break;
        case INLINE_META:
            rxasmeil(context, item->instrToken, item->operand1Token, item->operand2Token);
            break;
        case SRC_STEP:
            /* Queue Source Step */
            rxasmestp(context, item->instrToken, item->operand1Token, item->operand2Token,
                      item->operand3Token, item->operand4Token, item->operand5Token,
                      item->operand6Token, item->operand7Token);
            break;
        case TRACE_EVENT:
            /* Queue Trace Event */
            rxasmete(context, item->instrToken, item->operand1Token, item->operand2Token,
                     item->operand3Token, item->operand4Token, item->operand5Token,
                     item->operand6Token, item->operand7Token, item->operand8Token,
                     item->operand9Token, item->operand10Token);
            break;
        default:;
    }
}

static void queue_instruction_ext_full(Assembler_Context *context, enum queue_item_type type,
                                       Assembler_Token *instrToken, Assembler_Token *operand1Token, Assembler_Token *operand2Token,
                                       Assembler_Token *operand3Token, Assembler_Token *operand4Token, Assembler_Token *operand5Token,
                                       Assembler_Token *operand6Token, Assembler_Token *operand7Token,
                                       Assembler_Token *operand8Token, Assembler_Token *operand9Token,
                                       Assembler_Token *operand10Token) {

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
    context->optimiser_queue[context->optimiser_queue_items].operand6Token = operand6Token;
    context->optimiser_queue[context->optimiser_queue_items].operand7Token = operand7Token;
    context->optimiser_queue[context->optimiser_queue_items].operand8Token = operand8Token;
    context->optimiser_queue[context->optimiser_queue_items].operand9Token = operand9Token;
    context->optimiser_queue[context->optimiser_queue_items].operand10Token = operand10Token;
    context->optimiser_queue_items++;

    /* Optimise */
    optimise(context);
}

static void queue_instruction_ext(Assembler_Context *context, enum queue_item_type type,
                                  Assembler_Token *instrToken, Assembler_Token *operand1Token, Assembler_Token *operand2Token,
                                  Assembler_Token *operand3Token, Assembler_Token *operand4Token, Assembler_Token *operand5Token,
                                  Assembler_Token *operand6Token, Assembler_Token *operand7Token) {
    queue_instruction_ext_full(context, type, instrToken, operand1Token, operand2Token,
                               operand3Token, operand4Token, operand5Token,
                               operand6Token, operand7Token, 0, 0, 0);
}

static void queue_trace_event(Assembler_Context *context,
                              Assembler_Token *kind, Assembler_Token *mode_mask,
                              Assembler_Token *value_source, Assembler_Token *value_type,
                              Assembler_Token *register_type, Assembler_Token *value_ref,
                              Assembler_Token *source_step, Assembler_Token *clause,
                              Assembler_Token *flags, Assembler_Token *symbol,
                              Assembler_Token *resolved_name) {
    queue_instruction_ext_full(context, TRACE_EVENT, kind, mode_mask, value_source, value_type,
                               register_type, value_ref, source_step, clause,
                               flags, symbol, resolved_name);
}

static void queue_instruction(Assembler_Context *context, enum queue_item_type type,
                              Assembler_Token *instrToken, Assembler_Token *operand1Token, Assembler_Token *operand2Token,
                              Assembler_Token *operand3Token, Assembler_Token *operand4Token, Assembler_Token *operand5Token) {
    queue_instruction_ext(context, type, instrToken, operand1Token, operand2Token,
                          operand3Token, operand4Token, operand5Token, 0, 0);
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

/* Queue Source Step */
void rxasqmstp(Assembler_Context *context, Assembler_Token *step, Assembler_Token *clause, Assembler_Token *flags,
               Assembler_Token *file, Assembler_Token *line, Assembler_Token *start, Assembler_Token *end,
               Assembler_Token *source) {
    if (context->optimise) {
        queue_instruction_ext(context, SRC_STEP, step, clause, flags, file, line, start, end, source);
    }
    else rxasmestp(context, step, clause, flags, file, line, start, end, source);
}

/* Queue Trace Event */
void rxasqmte(Assembler_Context *context, Assembler_Token *kind, Assembler_Token *mode_mask,
              Assembler_Token *value_source, Assembler_Token *value_type, Assembler_Token *register_type,
              Assembler_Token *value_ref, Assembler_Token *source_step, Assembler_Token *clause,
              Assembler_Token *flags, Assembler_Token *symbol, Assembler_Token *resolved_name) {
    if (context->optimise) {
        queue_trace_event(context, kind, mode_mask, value_source, value_type, register_type, value_ref,
                          source_step, clause, flags, symbol, resolved_name);
    }
    else rxasmete(context, kind, mode_mask, value_source, value_type, register_type, value_ref,
                  source_step, clause, flags, symbol, resolved_name);
}

/* Queue Function Metadata */
void rxasqmfu(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *func, Assembler_Token *args) {
    if (context->optimise) {
        queue_instruction(context, FUNC_META, symbol, option, type, func, args, 0);
    }
    else rxasmefu(context, symbol, option, type, func, args);
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

/* Queue Class Metadata */
void rxasqmclss(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type) {
    if (context->optimise) {
        queue_instruction(context, CLASS_META, symbol, option, type, 0, 0, 0);
    }
    else rxasmeclss(context, symbol, option, type);
}

/* Queue Attribute Metadata */
void rxasqmattr(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *reg) {
    if (context->optimise) {
        queue_instruction(context, ATTR_META, symbol, option, type, reg, 0, 0);
    }
    else rxasmeattr(context, symbol, option, type, reg);
}

void rxasqmintf(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type) {
    if (context->optimise) {
        queue_instruction(context, INTERFACE_META, symbol, option, type, 0, 0, 0);
    }
    else rxasmeintf(context, symbol, option, type);
}

void rxasqmimpl(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *interface_symbol) {
    if (context->optimise) {
        queue_instruction(context, IMPLEMENTS_META, symbol, interface_symbol, 0, 0, 0, 0);
    }
    else rxasmeimpl(context, symbol, interface_symbol);
}

void rxasqmmemb(Assembler_Context *context, Assembler_Token *owner, Assembler_Token *kind, Assembler_Token *member, Assembler_Token *type, Assembler_Token *args) {
    if (context->optimise) {
        queue_instruction(context, MEMBER_META, owner, kind, member, type, args, 0);
    }
    else rxasmememb(context, owner, kind, member, type, args);
}

/* Queue Inline Metadata */
void rxasqmil(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *payload) {
    if (context->optimise) {
        queue_instruction(context, INLINE_META, symbol, option, payload, 0, 0, 0);
    }
    else rxasmeil(context, symbol, option, payload);
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
