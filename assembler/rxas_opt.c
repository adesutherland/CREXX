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
#include <stdlib.h>
#include "string.h"

Assembler_Token *rxas_queue_operand(const instruction_queue *item, size_t operand_index) {
    if (!item) return 0;
    if (item->instrType == OP_CODE && item->operandTokens) {
        return operand_index < item->operandCount ? item->operandTokens[operand_index] : 0;
    }
    switch (operand_index) {
        case 0: return item->operand1Token;
        case 1: return item->operand2Token;
        case 2: return item->operand3Token;
        case 3: return item->operand4Token;
        case 4: return item->operand5Token;
        case 5: return item->operand6Token;
        case 6: return item->operand7Token;
        case 7: return item->operand8Token;
        case 8: return item->operand9Token;
        case 9: return item->operand10Token;
        default: return 0;
    }
}

void rxas_free_queue_item(instruction_queue *item) {
    if (!item) return;
    free(item->operandTokens);
    item->operandTokens = 0;
    item->operandCount = 0;
}

static void set_named_opcode_operands(instruction_queue *item) {
    item->operand1Token = rxas_queue_operand(item, 0);
    item->operand2Token = rxas_queue_operand(item, 1);
    item->operand3Token = rxas_queue_operand(item, 2);
    item->operand4Token = rxas_queue_operand(item, 3);
    item->operand5Token = rxas_queue_operand(item, 4);
    item->operand6Token = rxas_queue_operand(item, 5);
    item->operand7Token = rxas_queue_operand(item, 6);
    item->operand8Token = rxas_queue_operand(item, 7);
    item->operand9Token = rxas_queue_operand(item, 8);
    item->operand10Token = rxas_queue_operand(item, 9);
}

void rxas_set_queue_operands(Assembler_Context *context,
                             instruction_queue *item,
                             Assembler_Token *const *operandTokens,
                             size_t operandCount) {
    rxas_free_queue_item(item);
    if (operandCount) {
        item->operandTokens = malloc(operandCount * sizeof(*item->operandTokens));
        if (!item->operandTokens) {
            RX_PANIC_OOM("malloc rxas queued opcode operands",
                         operandCount * sizeof(*item->operandTokens),
                         context && context->file_name ? context->file_name : 0);
        }
        memcpy(item->operandTokens, operandTokens,
               operandCount * sizeof(*item->operandTokens));
    }
    item->operandCount = operandCount;
    set_named_opcode_operands(item);
}

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

typedef struct operand_pattern {
    char type;
    size_t number;
} operand_pattern;

typedef struct extended_instruction_pattern {
    const operand_pattern *operands;
    size_t operand_count;
} extended_instruction_pattern;

/* Existing rule initializers retain their three inline operand pairs.  A '*'
 * in the first pair selects a variable-length pattern from this side table. */
#define EXTENDED_PATTERN '*'

enum extended_pattern_id {
    EXT_WIDE_CNOP,
    EXT_RRRR,
    EXT_RRRRRR,
    EXT_RRRRRRRR,
    EXT_RIRI,
    EXT_RIRIR,
    EXT_RRRI,
    EXT_RRRIR,
    EXT_RIRR,
    EXT_RIRRR
};

static const operand_pattern wide_cnop_pattern[] = {
    {'r', 0}, {'r', 1}, {'r', 2}, {'r', 3}, {'r', 4},
    {'r', 5}, {'r', 6}, {'r', 7}, {'r', 8}
};
static const operand_pattern pattern_rrrr[] = {
    {'r', 0}, {'r', 1}, {'r', 2}, {'r', 3}
};
static const operand_pattern pattern_rrrrrr[] = {
    {'r', 0}, {'r', 1}, {'r', 2}, {'r', 3}, {'r', 4}, {'r', 5}
};
static const operand_pattern pattern_rrrrrrrr[] = {
    {'r', 0}, {'r', 1}, {'r', 2}, {'r', 3},
    {'r', 4}, {'r', 5}, {'r', 6}, {'r', 7}
};
static const operand_pattern pattern_riri[] = {
    {'r', 0}, {'i', 10}, {'r', 1}, {'i', 11}
};
static const operand_pattern pattern_ririr[] = {
    {'r', 0}, {'i', 10}, {'r', 1}, {'i', 11}, {'r', 2}
};
static const operand_pattern pattern_rrri[] = {
    {'r', 0}, {'r', 1}, {'r', 2}, {'i', 10}
};
static const operand_pattern pattern_rrrir[] = {
    {'r', 0}, {'r', 1}, {'r', 2}, {'i', 10}, {'r', 3}
};
static const operand_pattern pattern_rirr[] = {
    {'r', 0}, {'i', 10}, {'r', 1}, {'r', 2}
};
static const operand_pattern pattern_rirrr[] = {
    {'r', 0}, {'i', 10}, {'r', 1}, {'r', 2}, {'r', 3}
};

static const extended_instruction_pattern extended_patterns[] = {
    {wide_cnop_pattern, sizeof(wide_cnop_pattern) / sizeof(wide_cnop_pattern[0])},
    {pattern_rrrr, sizeof(pattern_rrrr) / sizeof(pattern_rrrr[0])},
    {pattern_rrrrrr, sizeof(pattern_rrrrrr) / sizeof(pattern_rrrrrr[0])},
    {pattern_rrrrrrrr, sizeof(pattern_rrrrrrrr) / sizeof(pattern_rrrrrrrr[0])},
    {pattern_riri, sizeof(pattern_riri) / sizeof(pattern_riri[0])},
    {pattern_ririr, sizeof(pattern_ririr) / sizeof(pattern_ririr[0])},
    {pattern_rrri, sizeof(pattern_rrri) / sizeof(pattern_rrri[0])},
    {pattern_rrrir, sizeof(pattern_rrrir) / sizeof(pattern_rrrir[0])},
    {pattern_rirr, sizeof(pattern_rirr) / sizeof(pattern_rirr[0])},
    {pattern_rirrr, sizeof(pattern_rirrr) / sizeof(pattern_rirrr[0])}
};

static size_t pattern_operand_count(const instruction_pattern *pattern) {
    if (pattern->optype1 == EXTENDED_PATTERN) {
        return extended_patterns[pattern->opnum1].operand_count;
    }
    if (pattern->optype3) return 3;
    if (pattern->optype2) return 2;
    if (pattern->optype1) return 1;
    return 0;
}

static operand_pattern pattern_operand(const instruction_pattern *pattern,
                                       size_t operand_index) {
    operand_pattern operand = {0, 0};
    if (pattern->optype1 == EXTENDED_PATTERN) {
        const extended_instruction_pattern *extended =
                &extended_patterns[pattern->opnum1];
        if (operand_index < extended->operand_count) {
            return extended->operands[operand_index];
        }
        return operand;
    }
    switch (operand_index) {
        case 0: operand.type = pattern->optype1; operand.number = pattern->opnum1; break;
        case 1: operand.type = pattern->optype2; operand.number = pattern->opnum2; break;
        case 2: operand.type = pattern->optype3; operand.number = pattern->opnum3; break;
        default: break;
    }
    return operand;
}

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

typedef struct op_capture {
   size_t reg;
   char regtp;
   rxinteger integer;
   unsigned char *string;
   unsigned char character;
   double real;
   char *branch;
   char *label;
   char *proc;

   Assembler_Token *reg_token;
   Assembler_Token *integer_token;
   Assembler_Token *string_token;
   Assembler_Token *binary_token;
   Assembler_Token *decimal_token;
   Assembler_Token *character_token;
   Assembler_Token *real_token;
   Assembler_Token *branch_token;
   Assembler_Token *label_token;
   Assembler_Token *proc_token;
} op_capture;

typedef struct literal_register_capture {
   Assembler_Token *token;
   size_t reg;
   char regtp;
} literal_register_capture;

/* Rules Operands Mapping */
typedef struct op_map {
   op_capture *captures;
   size_t capture_count;
   size_t capture_capacity;
   literal_register_capture *literal_registers;
   size_t literal_register_count;
   size_t literal_register_capacity;

   /* Instructions matched in the rules */
   rule *inst_mapped[OPTIMISER_TARGET_MAX_QUEUE_SIZE + OPTIMISER_QUEUE_EXTRA_BUFFER_SIZE];
} op_map;

static op_capture *op_map_capture(op_map *map, size_t capture_number) {
    size_t required = capture_number + 1;
    if (required > map->capture_capacity) {
        size_t new_capacity = map->capture_capacity ? map->capture_capacity : 8;
        op_capture *new_captures;
        while (new_capacity < required) new_capacity *= 2;
        new_captures = realloc(map->captures, new_capacity * sizeof(*new_captures));
        if (!new_captures) {
            RX_PANIC_OOM("realloc rxas optimiser captures",
                         new_capacity * sizeof(*new_captures), 0);
        }
        memset(new_captures + map->capture_capacity, 0,
               (new_capacity - map->capture_capacity) * sizeof(*new_captures));
        map->captures = new_captures;
        map->capture_capacity = new_capacity;
    }
    if (required > map->capture_count) {
        memset(map->captures + map->capture_count, 0,
               (required - map->capture_count) * sizeof(*map->captures));
        map->capture_count = required;
    }
    return &map->captures[capture_number];
}

static void op_map_reset(op_map *map) {
    if (map->capture_count) {
        memset(map->captures, 0, map->capture_count * sizeof(*map->captures));
    }
    map->capture_count = 0;
    map->literal_register_count = 0;
    memset(map->inst_mapped, 0, sizeof(map->inst_mapped));
}

static void op_map_free(op_map *map) {
    free(map->captures);
    free(map->literal_registers);
    memset(map, 0, sizeof(*map));
}

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
 * Cancelling SWAP pairs are no longer a rule-table authority. K01 sends the
 * complete procedure to the immutable storage-permutation proof service, which
 * verifies restored bindings plus every intervening observation and write.
 *
 * 1. Rule for converting a concat to a faster append
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
 * 2. Rule for optimising an unconditional branch (br) to a branch true (brt),
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

            /* A following zero-operand CNOP is redundant.  This harmless rule
             * is also the rule-engine regression for wide input/output maps. */
            {NO_GAP, OP_CODE,"cnop", EXTENDED_PATTERN, EXT_WIDE_CNOP, 0, 0, 0, 0,
                     OP_CODE,"cnop", EXTENDED_PATTERN, EXT_WIDE_CNOP, 0, 0, 0, 0},
            {NO_GAP, OP_CODE,"cnop", 0, 0, 0, 0, 0, 0,
                     0},
            {END_OF_RULE},

            /* NR-09 Class 1: collect adjacent independent swaps. */
            {NO_GAP, OP_CODE,"swap", 'r', 0, 'r', 1, 0, 0,
                         0},
            {NO_GAP, OP_CODE,"swap", 'r', 2, 'r', 3, 0, 0,
                         OP_CODE,"swapn", EXTENDED_PATTERN, EXT_RRRR, 0, 0, 0, 0},
            {END_OF_RULE},

            {NO_GAP, OP_CODE,"swapn", EXTENDED_PATTERN, EXT_RRRR, 0, 0, 0, 0,
                         0},
            {NO_GAP, OP_CODE,"swap", 'r', 4, 'r', 5, 0, 0,
                         OP_CODE,"swapn", EXTENDED_PATTERN, EXT_RRRRRR, 0, 0, 0, 0},
            {END_OF_RULE},

            {NO_GAP, OP_CODE,"swapn", EXTENDED_PATTERN, EXT_RRRRRR, 0, 0, 0, 0,
                         0},
            {NO_GAP, OP_CODE,"swap", 'r', 6, 'r', 7, 0, 0,
                         OP_CODE,"swapn", EXTENDED_PATTERN, EXT_RRRRRRRR, 0, 0, 0, 0},
            {END_OF_RULE},

            /* NR-09 Class 1: call-window preparation. */
            {NO_GAP, OP_CODE,"settp", 'r', 0, 'i', 10, 0, 0,
                         0},
            {NO_GAP, OP_CODE,"swap", 'r', 1, 'r', 0, 0, 0,
                         OP_CODE,"settpswap", 'r', 0, 'i', 10, 'r', 1},
            {END_OF_RULE},

            {NO_GAP, OP_CODE,"load", 'r', 0, 'i', 10, 0, 0,
                         0},
            {NO_GAP, OP_CODE,"settp", 'r', 1, 'i', 11, 0, 0,
                         OP_CODE,"loadsettp2", EXTENDED_PATTERN, EXT_RIRI, 0, 0, 0, 0},
            {END_OF_RULE},

            {NO_GAP, OP_CODE,"loadsettp2", EXTENDED_PATTERN, EXT_RIRI, 0, 0, 0, 0,
                         0},
            {NO_GAP, OP_CODE,"swap", 'r', 2, 'r', 1, 0, 0,
                         OP_CODE,"loadsettpswap", EXTENDED_PATTERN, EXT_RIRIR, 0, 0, 0, 0},
            {END_OF_RULE},

            {NO_GAP, OP_CODE,"swap", 'r', 0, 'r', 1, 0, 0,
                         0},
            {NO_GAP, OP_CODE,"settp", 'r', 2, 'i', 10, 0, 0,
                         OP_CODE,"swapsettp", EXTENDED_PATTERN, EXT_RRRI, 0, 0, 0, 0},
            {END_OF_RULE},

            {NO_GAP, OP_CODE,"swapsettp", EXTENDED_PATTERN, EXT_RRRI, 0, 0, 0, 0,
                         0},
            {NO_GAP, OP_CODE,"swap", 'r', 3, 'r', 2, 0, 0,
                         OP_CODE,"swapsettpswap", EXTENDED_PATTERN, EXT_RRRIR, 0, 0, 0, 0},
            {END_OF_RULE},

            {NO_GAP, OP_CODE,"settpswap", 'r', 0, 'i', 10, 'r', 1,
                         0},
            {NO_GAP, OP_CODE,"settpswap", 'r', 2, 'i', 10, 'r', 3,
                         OP_CODE,"settpswapsettpswap", EXTENDED_PATTERN, EXT_RIRRR, 0, 0, 0, 0},
            {END_OF_RULE},

            /* NR-09 Class 1: collect clears and constant loads. */
            {NO_GAP, OP_CODE,"null", 'r', 0, 0, 0, 0, 0,
                         0},
            {NO_GAP, OP_CODE,"null", 'r', 1, 0, 0, 0, 0,
                         OP_CODE,"nulln", 'r', 0, 'r', 1, 0, 0},
            {END_OF_RULE},

            {NO_GAP, OP_CODE,"nulln", 'r', 0, 'r', 1, 0, 0,
                         0},
            {NO_GAP, OP_CODE,"null", 'r', 2, 0, 0, 0, 0,
                         OP_CODE,"nulln", 'r', 0, 'r', 1, 'r', 2},
            {END_OF_RULE},

            {NO_GAP, OP_CODE,"nulln", 'r', 0, 'r', 1, 'r', 2,
                         0},
            {NO_GAP, OP_CODE,"null", 'r', 3, 0, 0, 0, 0,
                         OP_CODE,"nulln", EXTENDED_PATTERN, EXT_RRRR, 0, 0, 0, 0},
            {END_OF_RULE},

            /* Full copy already copies status flags; drop redundant acopy. */
            {NO_GAP, OP_CODE,"copy", 'r', 0, 'r', 1, 0, 0,
                        OP_CODE,"copy", 'r', 0, 'r', 1, 0, 0},
            {NO_GAP, OP_CODE,"acopy", 'r', 0, 'r', 1, 0, 0,
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

            /* Integer compare followed by branch: materialise no boolean temp. */
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

/* The rule array predates stable diagnostic identities.  Hash the complete
 * input/output shape of one rule set so a retained baseline can identify the
 * same tactical rule even after unrelated rules are migrated or reordered.
 * This is diagnostic identity only; it never participates in matching. */
static unsigned int rule_signature_uint(unsigned int hash,
                                        unsigned int value) {
    size_t byte_index;
    for (byte_index = 0; byte_index < 4; byte_index++) {
        hash ^= (value >> (byte_index * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static unsigned int rule_signature_text(unsigned int hash,
                                        const char *text) {
    if (!text) return rule_signature_uint(hash, 0u);
    while (*text) {
        hash ^= (unsigned char)*text++;
        hash *= 16777619u;
    }
    return rule_signature_uint(hash, 0xffu);
}

static unsigned int rule_signature_pattern(
        unsigned int hash, const instruction_pattern *pattern) {
    size_t operand_index;
    size_t operand_count;
    hash = rule_signature_uint(hash, (unsigned int)pattern->inst_type);
    hash = rule_signature_text(hash, pattern->instruction);
    operand_count = pattern_operand_count(pattern);
    hash = rule_signature_uint(hash, (unsigned int)operand_count);
    for (operand_index = 0; operand_index < operand_count; operand_index++) {
        operand_pattern operand;
        operand = pattern_operand(pattern, operand_index);
        hash = rule_signature_uint(hash, (unsigned int)(unsigned char)operand.type);
        hash = rule_signature_uint(hash, (unsigned int)operand.number);
    }
    return hash;
}

static unsigned int rule_signature(const rule *start) {
    const rule *entry;
    unsigned int hash;
    hash = 2166136261u;
    for (entry = start; ; entry++) {
        hash = rule_signature_uint(hash, (unsigned int)entry->flag);
        if (entry->flag == END_OF_RULE) break;
        hash = rule_signature_pattern(hash, &entry->in);
        hash = rule_signature_pattern(hash, &entry->out);
        hash = rule_signature_pattern(hash, &entry->out2);
    }
    return hash;
}

static void debug_rule_accept(Assembler_Context *context, const op_map *map,
                              const rule *start) {
    size_t index;
    int first;
    if (!context || !context->debug_mode || !map || !start) return;
    fprintf(stderr,
            "PERF3 legacy-rule procedure=%s signature=%08x first=%s records=",
            context->current_proc_name ? context->current_proc_name
                                       : "(directives)",
            rule_signature(start),
            start->in.instruction ? start->in.instruction : "label");
    first = 1;
    for (index = 0; index < context->optimiser_queue_items; index++) {
        if (!map->inst_mapped[index]) continue;
        if (!first) fputc(',', stderr);
        fprintf(stderr, "%llu", (unsigned long long)index);
        first = 0;
    }
    fputc('\n', stderr);
}

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
    size_t i;
    char regtp;

    if (!can_map_literal_register(opToken, op_type, op_num)) return 0;

    regtp = literal_reg_type(op_type);
    for (i = 0; i < map->literal_register_count; i++) {
        if (map->literal_registers[i].regtp == regtp &&
            map->literal_registers[i].reg == op_num) {
            return 1;
        }
    }

    if (map->literal_register_count == map->literal_register_capacity) {
        size_t new_capacity = map->literal_register_capacity
                ? map->literal_register_capacity * 2 : 8;
        literal_register_capture *new_registers = realloc(
                map->literal_registers, new_capacity * sizeof(*new_registers));
        if (!new_registers) {
            RX_PANIC_OOM("realloc rxas optimiser literal registers",
                         new_capacity * sizeof(*new_registers), 0);
        }
        map->literal_registers = new_registers;
        map->literal_register_capacity = new_capacity;
    }
    map->literal_registers[map->literal_register_count].token = opToken;
    map->literal_registers[map->literal_register_count].regtp = regtp;
    map->literal_registers[map->literal_register_count].reg = op_num;
    map->literal_register_count++;
    return 1;
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

static int op_operands_match(OpFormat format,
                             Assembler_Token *const *operandTokens,
                             size_t operandCount) {
    size_t i;

    if (rxop_format_operand_count(format) != operandCount) return 0;
    for (i = 0; i < operandCount; i++) {
        if (rxop_format_operand_type(format, i) != operand_type(operandTokens[i])) return 0;
    }
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

static const OpInfo *find_optimiser_opcode(const instruction_queue *instruction) {
    const char *mnemonic;
    int i;

    if (!instruction || !instruction->instrToken || instruction->instrType != OP_CODE) return 0;

    mnemonic = (const char *) instruction->instrToken->token_value.string;

    for (i = 0; op_table[i].mnemonic != NULL; i++) {
        if (!rxop_is_source_mnemonic(op_table[i].mnemonic)) continue;
        if (op_operands_match(op_table[i].format,
                              instruction->operandTokens,
                              instruction->operandCount) &&
            mnemonic_matches(mnemonic, op_table[i].mnemonic)) {
            return &op_table[i];
        }
    }

    return 0;
}

/* Check if a skipped instruction is a hard barrier for NO_HAZARD rules. */
static int is_rule_barrier(const instruction_queue *instruction) {
    const OpInfo *op_info;
    size_t i;

    if (instruction->instrType == OP_CODE) {
        op_info = find_optimiser_opcode(instruction);
        if (op_info) {
            if (op_info->flow != FLOW_NEXT) return 1;
            if (op_info->flags & FLG_OPT_BARRIER) return 1;
        }

        /* Calls or branches */
        for (i = 0; i < instruction->operandCount; i++) {
            Assembler_Token *operand = rxas_queue_operand(instruction, i);
            if (operand && (operand->token_type == ID || operand->token_type == FUNC)) return 1;
        }
    }

    else if (instruction->instrType == ASM_LABEL) return 1;

    return 0;
}

static int map_has_register(op_map *map, char reg_type, size_t reg_num) {
    size_t i;

    for (i = 0; i < map->capture_count; i++) {
        if (map->captures[i].reg_token &&
            map->captures[i].regtp == reg_type &&
            map->captures[i].reg == reg_num) {
            return 1;
        }
    }
    for (i = 0; i < map->literal_register_count; i++) {
        if (map->literal_registers[i].regtp == reg_type &&
            map->literal_registers[i].reg == reg_num) {
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

static int implicit_register_range_relevant(op_map *map, Assembler_Token *base_token) {
    char base_type;
    size_t base_number;
    size_t i;

    if (!base_token) return 0;
    base_type = reg_type(base_token);
    if (!base_type) return 0;
    base_number = (size_t)base_token->token_value.integer;

    for (i = 0; i < map->capture_count; i++) {
        if (map->captures[i].reg_token &&
            map->captures[i].regtp == base_type &&
            map->captures[i].reg > base_number) {
            return 1;
        }
    }
    for (i = 0; i < map->literal_register_count; i++) {
        if (map->literal_registers[i].regtp == base_type &&
            map->literal_registers[i].reg > base_number) {
            return 1;
        }
    }

    return 0;
}

static int implicit_register_relevant(op_map *map,
                                      const OpInfo *op_info,
                                      Assembler_Token *operand1Token,
                                      Assembler_Token *operand2Token,
                                      Assembler_Token *operand3Token) {
    RxOpEffects effects;

    if (!op_info) return 1;
    effects = rxop_effects(op_info->opcode);
    if (effects.state != RXOP_EFFECT_CLASSIFIED) return 1;
    if (effects.implicit == RXOP_IMPLICIT_NONE) return 0;

    switch (effects.implicit) {
        case RXOP_IMPLICIT_LOCAL_COPY:
            return implicit_int_register_relevant(map, operand1Token, 'r') ||
                   implicit_int_register_relevant(map, operand2Token, 'r');
        case RXOP_IMPLICIT_LOCAL_TARGET:
            return implicit_int_register_relevant(map, operand1Token, 'r');
        case RXOP_IMPLICIT_LOCAL_R0_READ_WRITE:
            return map_has_register(map, 'r', 0);
        case RXOP_IMPLICIT_LOCAL_R1_READ_WRITE:
            return map_has_register(map, 'r', 1);
        case RXOP_IMPLICIT_LOCAL_R2_READ_WRITE:
            return map_has_register(map, 'r', 2);
        case RXOP_IMPLICIT_ARGUMENT_INDEX:
            return implicit_int_register_relevant(map, operand2Token, 'a');
        case RXOP_IMPLICIT_LOCAL_RANGE_AFTER_OP3:
            return implicit_register_range_relevant(map, operand3Token);
        case RXOP_IMPLICIT_NONE:
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

static int string_token_equals(const Assembler_Token *token, const char *value) {
    return token &&
           token->token_type == STRING &&
           strcmp((const char *)token->token_value.string, value) == 0;
}

static int string_token_has_single_char(const Assembler_Token *token) {
    return token &&
           token->token_type == STRING &&
           token->token_value.string[0] &&
           token->token_value.string[1] == 0;
}

static int instruction_is_relevant(op_map *map, instruction_queue *instruction) {
    const OpInfo *op_info;
    Assembler_Token *value_source;
    Assembler_Token *register_type_token;
    Assembler_Token *value_ref;
    char register_type;

    if (instruction->instrType == TRACE_EVENT) {
        value_source = instruction->operand2Token;
        register_type_token = instruction->operand4Token;
        value_ref = instruction->operand5Token;

        if (string_token_equals(value_source, "R") &&
            string_token_has_single_char(register_type_token) &&
            value_ref &&
            value_ref->token_type == INT &&
            value_ref->token_value.integer >= 0) {
            register_type = (char)tolower((unsigned char)register_type_token->token_value.string[0]);
            if (map_has_register(map, register_type, (size_t)value_ref->token_value.integer)) {
                return 1;
            }
        }
    }

    if (instruction->instrType == OP_CODE) {
        size_t i;
        for (i = 0; i < instruction->operandCount; i++) {
            if (is_relevant(map, rxas_queue_operand(instruction, i))) return 1;
        }
    } else if (is_relevant(map, instruction->operand1Token) ||
               is_relevant(map, instruction->operand2Token) ||
               is_relevant(map, instruction->operand3Token)) {
        return 1;
    }

    if (instruction->instrType != OP_CODE) return 0;

    op_info = find_optimiser_opcode(instruction);
    return implicit_register_relevant(map,
                                      op_info,
                                      instruction->operand1Token,
                                      instruction->operand2Token,
                                      instruction->operand3Token);
}

static int trace_event_matches_mapped_register(op_map *map,
                                               instruction_queue *instruction,
                                               char op_type,
                                               size_t op_num) {
    Assembler_Token *value_source;
    Assembler_Token *register_type_token;
    Assembler_Token *value_ref;
    char register_type;
    op_capture *capture;

    if (instruction->instrType != TRACE_EVENT) return 0;
    if (op_type == 0) return 1;
    if (op_type != 'r') return 0;
    capture = op_map_capture(map, op_num);
    if (!capture->reg_token) return 0;

    value_source = instruction->operand2Token;
    register_type_token = instruction->operand4Token;
    value_ref = instruction->operand5Token;

    if (!string_token_equals(value_source, "R") ||
        !string_token_has_single_char(register_type_token) ||
        !value_ref ||
        value_ref->token_type != INT ||
        value_ref->token_value.integer < 0) {
        return 0;
    }

    register_type = (char)tolower((unsigned char)register_type_token->token_value.string[0]);
    return capture->regtp == register_type &&
           capture->reg == (size_t)value_ref->token_value.integer;
}

/* Checks if a instrToken can map against an op_type
 * This checks the operand type and number is consistent with existing mapped operands
 * returns 1 if it can map, otherwise 0
 * NOTE: The *map structure is NOT updated
 * See map_operand() */
static int can_map_operand(op_map *map, Assembler_Token *opToken, char op_type, size_t op_num) {
    op_capture *capture;

    if (!opToken) {
        if (op_type) return 0;
        else return 1;
    }
    capture = op_map_capture(map, op_num);

    switch(op_type) {
        case 'r': /* Register */
            if ( !(opToken->token_type == RREG ||
                   opToken->token_type == GREG ||
                   opToken->token_type == AREG ) ) return 0; /* Wrong Type */

            if (capture->reg_token) { /* Already Mapped - checked consistent */
                if (capture->regtp != reg_type(opToken) ||
                    capture->reg != opToken->token_value.integer)
                    return 0; /* Wrong register */
            }
            return 1;

        case 'R': /* Literal local register */
        case 'G': /* Literal global register */
        case 'A': /* Literal argument register */
            return can_map_literal_register(opToken, op_type, op_num);

        case 'i': /* Integer */
            if (opToken->token_type != INT ) return 0; /* Wrong Type */
            if (capture->integer_token) { /* Already Mapped - checked consistent */
                if (capture->integer != opToken->token_value.integer)
                    return 0; /* Wrong value */
            }
            return 1;

        case 's': /* String */
            if (opToken->token_type != STRING ) return 0; /* Wrong Type */
            if (capture->string_token) { /* Already Mapped - checked consistent */
                if (capture->string != opToken->token_value.string) // TODO - Shouldn't this be strcmp?
                    return 0; /* Wrong value */
            }
            return 1;

        case 'h': /* Hex (Binary) */
            if (opToken->token_type != HEX ) return 0; /* Wrong Type */
            if (capture->binary_token) { /* Already Mapped - checked consistent */
                if (capture->string != opToken->token_value.string) // TODO - Shouldn't this be strcmp?
                    return 0; /* Wrong value */
            }
            return 1;

        case 'd': /* Decimal */
            if (opToken->token_type != DECIMAL ) return 0; /* Wrong Type */
            if (capture->decimal_token) { /* Already Mapped - checked consistent */
                if (capture->string != opToken->token_value.string) // TODO - Shouldn't this be strcmp?
                    return 0; /* Wrong value */
            }
            return 1;

        case 'c': /* Char */
            if (opToken->token_type != CHAR ) return 0; /* Wrong Type */
            if (capture->character_token) { /* Already Mapped - checked consistent */
                if (capture->character != opToken->token_value.character)
                    return 0; /* Wrong value */
            }
            return 1;

        case 'f': /* Float */
            if (opToken->token_type != FLOAT ) return 0; /* Wrong Type */
            if (capture->real_token) { /* Already Mapped - checked consistent */
                if (capture->real != opToken->token_value.real)
                    return 0; /* Wrong value */
            }
            return 1;

        case 'l': /* Label */
            /* The l and b tokens are intrinsically linked - "quantum!" - different
             * token types but have matching values */
            if (opToken->token_type != LABEL ) return 0; /* Wrong Type */
            if (capture->label_token) { /* Already Mapped - checked consistent */
                if (strcmp(capture->label, (char*)opToken->token_value.string) != 0)
                    return 0; /* Wrong value */
            }
            else if (capture->branch_token) { /* Already Mapped branch - checked consistent */
                if (strcmp(capture->branch, (char*)opToken->token_value.string) != 0)
                    return 0; /* Wrong value */
            }

            return 1;

        case 'b': /* BRANCH */
            /* The l and b tokens are intrinsically linked - "quantum!" - different
             * token types but have matching values */
            if (opToken->token_type != ID ) return 0; /* Wrong Type */
            if (capture->branch_token) { /* Already Mapped - checked consistent */
                if (strcmp(capture->branch, (char*)opToken->token_value.string) != 0)
                    return 0; /* Wrong value */
            }
            else if (capture->label_token) { /* Already Mapped - checked consistent */
                if (strcmp(capture->label, (char*)opToken->token_value.string) != 0)
                    return 0; /* Wrong value */
            }
            return 1;

        case 'p': /* PROCEDURE */
            if (opToken->token_type != FUNC ) return 0; /* Wrong Type */
            if (capture->proc_token) { /* Already Mapped - checked consistent */
                if (strcmp(capture->proc, (char*)opToken->token_value.string) != 0)
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
    op_capture *capture;

    if (!opToken) {
        if (op_type) return 0;
        else return 1;
    }
    capture = op_map_capture(map, op_num);

    switch(op_type) {
        case 'r': /* Register */
            if ( !(opToken->token_type == RREG ||
                   opToken->token_type == GREG ||
                   opToken->token_type == AREG ) ) return 0; /* Wrong Type */

            if (capture->reg_token) { /* Already Mapped - checked consistent */
                if (capture->regtp != reg_type(opToken) ||
                    capture->reg != opToken->token_value.integer)
                    return 0; /* Wrong register */
            }
            else { /* Not mapped yet - map it */
                capture->reg_token = opToken;
                capture->regtp = reg_type(opToken);
                capture->reg = opToken->token_value.integer;
            }
            return 1;

        case 'R': /* Literal local register */
        case 'G': /* Literal global register */
        case 'A': /* Literal argument register */
            return map_literal_register(map, opToken, op_type, op_num);

        case 'i': /* Integer */
            if (opToken->token_type != INT ) return 0; /* Wrong Type */
            if (capture->integer_token) { /* Already Mapped - checked consistent */
                if (capture->integer != opToken->token_value.integer)
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                capture->integer_token = opToken;
                capture->integer = opToken->token_value.integer;
            }
            return 1;

        case 's': /* String */
            if (opToken->token_type != STRING ) return 0; /* Wrong Type */
            if (capture->string_token) { /* Already Mapped - checked consistent */
                if (capture->string != opToken->token_value.string) // TODO - Shouldn't this be strcmp?
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                capture->string_token = opToken;
                capture->string = opToken->token_value.string;
            }
            return 1;

        case 'h': /* Hex (Binary) */
            if (opToken->token_type != HEX ) return 0; /* Wrong Type */
            if (capture->binary_token) { /* Already Mapped - checked consistent */
                if (capture->string != opToken->token_value.string) // TODO - Shouldn't this be strcmp?
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                capture->binary_token = opToken;
                capture->string = opToken->token_value.string;
            }
            return 1;

        case 'd': /* Decimal */
            if (opToken->token_type != DECIMAL ) return 0; /* Wrong Type */
            if (capture->decimal_token) { /* Already Mapped - checked consistent */
                if (capture->string != opToken->token_value.string) // TODO - Shouldn't this be strcmp?
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                capture->decimal_token = opToken;
                capture->string = opToken->token_value.string;
            }
            return 1;

        case 'c': /* Char */
            if (opToken->token_type != CHAR ) return 0; /* Wrong Type */
            if (capture->character_token) { /* Already Mapped - checked consistent */
                if (capture->character != opToken->token_value.character)
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                capture->character_token = opToken;
                capture->character = opToken->token_value.character;
            }
            return 1;

        case 'f': /* Float */
            if (opToken->token_type != FLOAT ) return 0; /* Wrong Type */
            if (capture->real_token) { /* Already Mapped - checked consistent */
                if (capture->real != opToken->token_value.real)
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                capture->real_token = opToken;
                capture->real = opToken->token_value.real;
            }
            return 1;

        case 'l': /* Label */
            /* The l and b tokens are intrinsically linked - "quantum!" - different
             * token types but have matching values */
            if (opToken->token_type != LABEL ) return 0; /* Wrong Type */
            if (capture->label_token) { /* Already Mapped - checked consistent */
                if (strcmp(capture->label, (char*)opToken->token_value.string) != 0)
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                if (capture->branch_token) { /* Already Mapped branch - checked consistent */
                    if (strcmp(capture->branch, (char*)opToken->token_value.string) != 0)
                        return 0; /* Wrong value */
                }
                capture->label_token = opToken;
                capture->label = (char*)opToken->token_value.string;
            }
            return 1;

        case 'b': /* BRANCH */
            /* The l and b tokens are intrinsically linked - "quantum!" - different
             * token types but have matching values */
            if (opToken->token_type != ID ) return 0; /* Wrong Type */
            if (capture->branch_token) { /* Already Mapped - checked consistent */
                if (strcmp(capture->branch, (char*)opToken->token_value.string) != 0)
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                if (capture->label_token) { /* Already Mapped - checked consistent */
                    if (strcmp(capture->label, (char*)opToken->token_value.string) != 0)
                        return 0; /* Wrong value */
                }
                capture->branch_token = opToken;
                capture->branch = (char*)opToken->token_value.string;
            }
            return 1;

        case 'p': /* PROCEDURE */
            if (opToken->token_type != FUNC ) return 0; /* Wrong Type */
            if (capture->proc_token) { /* Already Mapped - checked consistent */
                if (strcmp(capture->proc, (char*)opToken->token_value.string) != 0)
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                capture->proc_token = opToken;
                capture->proc = (char*)opToken->token_value.string;
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
    size_t operand_count;
    size_t operand_index;

    switch (rule->in.inst_type) {
        case OP_CODE:
            if (instruction->instrType != OP_CODE)
                return 0; /* Not a normal instruction */

            if (strcmp((char *) (instruction->instrToken->token_value.string), rule->in.instruction) != 0)
                return 0; /* Not the right instruction */

            operand_count = pattern_operand_count(&rule->in);
            if (instruction->operandCount != operand_count) return 0;
            for (operand_index = 0; operand_index < operand_count; operand_index++) {
                operand_pattern operand = pattern_operand(&rule->in, operand_index);
                if (!can_map_operand(map,
                                     rxas_queue_operand(instruction, operand_index),
                                     operand.type, operand.number)) return 0;
            }

            return 1;

        case ASM_LABEL:
            if (instruction->instrType != ASM_LABEL)
                return 0; /* Not a label */

            if (!can_map_operand(map, instruction->instrToken,
                             'l', rule->in.opnum1))
                return 0;
            return 1;

        case TRACE_EVENT:
            return trace_event_matches_mapped_register(map, instruction, rule->in.optype1, rule->in.opnum1);

        default:
            return 0;
    }
}

/* Maps an instruction against a rule
 * Returns 1 on success, 0 if the map fails. *map may be changed on either case */
static int map_instruction(op_map *map, instruction_queue *instruction, rule *rule) {
    size_t operand_count;
    size_t operand_index;

    switch (rule->in.inst_type) {
        case OP_CODE:
            if (instruction->instrType != OP_CODE)
                return 0; /* Not a normal instruction */

            if (strcmp((char *) (instruction->instrToken->token_value.string), rule->in.instruction) != 0)
                return 0; /* Not the right instruction */

            operand_count = pattern_operand_count(&rule->in);
            if (instruction->operandCount != operand_count) return 0;
            for (operand_index = 0; operand_index < operand_count; operand_index++) {
                operand_pattern operand = pattern_operand(&rule->in, operand_index);
                if (!map_operand(map,
                                 rxas_queue_operand(instruction, operand_index),
                                 operand.type, operand.number)) return 0;
            }

            return 1;

        case ASM_LABEL:
            if (instruction->instrType != ASM_LABEL)
                return 0; /* Not a label */

            if (!map_operand(map, instruction->instrToken,
                                 'l', rule->in.opnum1))
                return 0;
            return 1;

        case TRACE_EVENT:
            return trace_event_matches_mapped_register(map, instruction, rule->in.optype1, rule->in.opnum1);

        default:
            return 0;
    }
}

/* Returns the mapped token for a rule */
static Assembler_Token* mapped_token(Assembler_Context *context, op_map *map, char op_type, size_t op_num) {
    Assembler_Token *t;
    op_capture *capture = op_map_capture(map, op_num);
    char buffer[20];

    switch(op_type) {
        case 'r': /* Register */
            return capture->reg_token;

        case 'i': /* Integer */
            return capture->integer_token;

        case 's': /* String */
            return capture->string_token;

        case 'h': /* Hex (Binary) */
            return capture->binary_token;

        case 'd': /* Decimal */
            return capture->decimal_token;

        case 'c': /* Char */
            return capture->character_token;

        case 'f': /* Float */
            return capture->real_token;

        case 'l': /* Label == Branch */
            t = capture->label_token;
            if (t == 0) {
                /* Special functionality for labels / branches
                 * if label has not been defined in an input rule then a
                 * unique label token is created */

                /* If the intrinsically linked branch id is set make a pair */
                t = capture->branch_token;
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
                capture->label_token = t;
            }
            return t;

        case 'b': /* Branch == Label */
            t = capture->branch_token;
            if (t == 0) {
                /* Special functionality for labels / branches
                 * if branch id has not been defined in an input rule then a
                 * unique branch token is created */

                /* If the intrinsically linked label is set make a pair */
                t = capture->label_token;
                if (t) t = rxas_tid(context, t, (char *) t->token_value.string);

                /* Otherwise make a unique label - note that as it does not start with
                 * a letter, it cannot be a duplicate of a label from the
                 * rxas source file */
                else {
                    snprintf(buffer, 20, "%d", context->optimiser_counter++);
                    t = rxas_tid(context, NULL, buffer);
                }
                /* Store the created token */
                capture->branch_token = t;
            }
            return t;

        case 'p': /* Procedure */
            return capture->proc_token;

        case 0:   /* Check nulls match */
        default:  /* Something wrong */
            return 0;
    }
}

static Assembler_Token **mapped_pattern_tokens(Assembler_Context *context,
                                               op_map *map,
                                               const instruction_pattern *pattern,
                                               size_t *operand_count_out) {
    size_t operand_count = pattern_operand_count(pattern);
    Assembler_Token **tokens = 0;
    size_t i;

    if (operand_count) {
        tokens = malloc(operand_count * sizeof(*tokens));
        if (!tokens) {
            RX_PANIC_OOM("malloc rxas optimiser output operands",
                         operand_count * sizeof(*tokens), 0);
        }
        for (i = 0; i < operand_count; i++) {
            operand_pattern operand = pattern_operand(pattern, i);
            tokens[i] = mapped_token(context, map, operand.type, operand.number);
        }
    }
    *operand_count_out = operand_count;
    return tokens;
}

/* Optimise a rule starting from a specific instruction
 * returns 1 if the rule was successfully applied */
static int optimise_rule(Assembler_Context *context, op_map *map, rule *r, int inst_no) {
    int inst_no2;
    rule *start_rule;

    start_rule = r;

    /* Clear Map */
    op_map_reset(map);

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
                if ((r->flag == NO_GAP || r->flag == NO_HAZARD) &&
                    instruction_is_relevant(map, &context->optimiser_queue[inst_no])) {
                    return 0;
                }

                if (    context->optimiser_queue[inst_no].instrType == OP_CODE ||
                        context->optimiser_queue[inst_no].instrType == ASM_LABEL ) {
                    /* Non-value metadata is ignored; relevant metadata was handled above. */

                    if (r->flag == NO_GAP) return 0; /* No gap allowed! */
                    if (r->flag == NO_HAZARD) {
                        /* Is it a barrier instruction like a branch or procedure call? */
                        if (is_rule_barrier(&context->optimiser_queue[inst_no]))
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
        debug_rule_accept(context, map, start_rule);

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
                    {
                        Assembler_Token **replacementOperands;
                        size_t replacementOperandCount;
                        context->optimiser_queue[inst_no].instrType = OP_CODE;
                        context->optimiser_queue[inst_no].instrToken =
                                rxas_tid(context,
                                         context->optimiser_queue[inst_no].instrToken,
                                         r->out.instruction);
                        replacementOperands = mapped_pattern_tokens(
                                context, map, &r->out, &replacementOperandCount);
                        rxas_set_queue_operands(context,
                                                &context->optimiser_queue[inst_no],
                                                replacementOperands,
                                                replacementOperandCount);
                        free(replacementOperands);
                        break;
                    }

                    case ASM_LABEL:
                    {
                        Assembler_Token *labelToken = mapped_token(context, map, 'l', r->out.opnum1);
                        rxas_free_queue_item(&context->optimiser_queue[inst_no]);
                        memset(&context->optimiser_queue[inst_no], 0, sizeof(instruction_queue));
                        context->optimiser_queue[inst_no].instrType = ASM_LABEL;
                        context->optimiser_queue[inst_no].instrToken = labelToken;
                        break;
                    }

                    default:
                        /* No - output rule so remove instruction from the queue */
                        rxas_free_queue_item(&context->optimiser_queue[inst_no]);
                        if ((int)context->optimiser_queue_items - (int)inst_no - 1 > 0) {
                            memmove(&context->optimiser_queue[inst_no],
                                    &context->optimiser_queue[inst_no + 1],
                                    sizeof(instruction_queue) *
                                    (context->optimiser_queue_items - inst_no - 1));
                        }
                        /* One less instruction in the queue */
                        context->optimiser_queue_items--;
                        memset(&context->optimiser_queue[context->optimiser_queue_items], 0,
                               sizeof(instruction_queue));
                }

                /* Secondary output instruction */
                switch (r->out2.inst_type) {
                    case OP_CODE:
                    {
                        Assembler_Token *baseToken = context->optimiser_queue[inst_no].instrToken;
                        Assembler_Token **replacementOperands;
                        size_t replacementOperandCount;
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
                        memset(&context->optimiser_queue[inst_no2], 0, sizeof(instruction_queue));
                        context->optimiser_queue[inst_no2].instrType = OP_CODE;
                        context->optimiser_queue[inst_no2].instrToken =
                                rxas_tid(context, baseToken, r->out2.instruction);
                        replacementOperands = mapped_pattern_tokens(
                                context, map, &r->out2, &replacementOperandCount);
                        rxas_set_queue_operands(context,
                                                &context->optimiser_queue[inst_no2],
                                                replacementOperands,
                                                replacementOperandCount);
                        free(replacementOperands);
                        break;
                    }

                    case ASM_LABEL:
                    {
                        Assembler_Token *labelToken = mapped_token(context, map, 'l', r->out2.opnum1);
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
                        memset(&context->optimiser_queue[inst_no2], 0, sizeof(instruction_queue));
                        context->optimiser_queue[inst_no2].instrType = ASM_LABEL;
                        context->optimiser_queue[inst_no2].instrToken = labelToken;
                        break;
                    }

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
    op_map map = {0};
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

    op_map_free(&map);
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
            rxasgenv(context, item->instrToken, item->operandTokens, item->operandCount);
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

static void reserve_procedure_queue(Assembler_Context *context, size_t required) {
    size_t new_capacity;
    instruction_queue *new_queue;

    if (required <= context->procedure_queue_capacity) return;
    new_capacity = context->procedure_queue_capacity
            ? context->procedure_queue_capacity : 64;
    while (new_capacity < required) new_capacity *= 2;
    new_queue = realloc(context->procedure_queue,
                        new_capacity * sizeof(*new_queue));
    if (!new_queue) {
        RX_PANIC_OOM("realloc rxas procedure optimiser stream",
                     new_capacity * sizeof(*new_queue),
                     context && context->file_name ? context->file_name : 0);
    }
    memset(new_queue + context->procedure_queue_capacity, 0,
           (new_capacity - context->procedure_queue_capacity) * sizeof(*new_queue));
    context->procedure_queue = new_queue;
    context->procedure_queue_capacity = new_capacity;
}

/* Move the stable head of the bounded local peephole into the transient
 * procedure stream. The variable operand vector moves with the record. */
static void retire_oldest_queue_item(Assembler_Context *context) {
    instruction_queue *destination;

    if (!context->optimiser_queue_items) return;
    reserve_procedure_queue(context, context->procedure_queue_items + 1);
    destination = &context->procedure_queue[context->procedure_queue_items++];
    *destination = context->optimiser_queue[0];

    if (context->optimiser_queue_items > 1) {
        memmove(&context->optimiser_queue[0],
                &context->optimiser_queue[1],
                sizeof(instruction_queue) * (context->optimiser_queue_items - 1));
    }
    context->optimiser_queue_items--;
    memset(&context->optimiser_queue[context->optimiser_queue_items], 0,
           sizeof(instruction_queue));
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
        retire_oldest_queue_item(context);
    }

    /* Add to the end of the queue */
    memset(&context->optimiser_queue[context->optimiser_queue_items], 0,
           sizeof(instruction_queue));
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

static void queue_opcode(Assembler_Context *context,
                         Assembler_Token *instrToken,
                         Assembler_Token *const *operandTokens,
                         size_t operandCount) {
    instruction_queue *item;

    while (context->optimiser_queue_items >= OPTIMISER_TARGET_MAX_QUEUE_SIZE) {
        retire_oldest_queue_item(context);
    }

    item = &context->optimiser_queue[context->optimiser_queue_items++];
    memset(item, 0, sizeof(*item));
    item->instrType = OP_CODE;
    item->instrToken = instrToken;
    rxas_set_queue_operands(context, item, operandTokens, operandCount);
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
void rxasquev(Assembler_Context *context, Assembler_Token *instrToken,
              Assembler_Token *const *operandTokens, size_t operandCount) {
    promote_floats_to_decimalsv(instrToken, operandTokens, operandCount);
    if (context->optimise) {
        queue_opcode(context, instrToken, operandTokens, operandCount);
    } else {
        rxasgenv(context, instrToken, operandTokens, operandCount);
    }
}

void rxasque_span(Assembler_Context *context, Assembler_Token *instrToken,
                  Assembler_Token *lastOperandToken) {
    Assembler_Token **operands = 0;
    Assembler_Token *operand = instrToken ? instrToken->token_next : 0;
    size_t operandCount = 0;
    size_t capacity = 0;

    while (operand) {
        if (operandCount == capacity) {
            size_t newCapacity = capacity ? capacity * 2 : 4;
            Assembler_Token **newOperands = realloc(operands,
                                                     newCapacity * sizeof(*newOperands));
            if (!newOperands) {
                free(operands);
                RX_PANIC_OOM("realloc rxas parsed operands",
                             newCapacity * sizeof(*newOperands),
                             context && context->file_name ? context->file_name : 0);
            }
            operands = newOperands;
            capacity = newCapacity;
        }
        operands[operandCount++] = operand;
        if (operand == lastOperandToken) break;
        operand = operand->token_next;
        if (operand) operand = operand->token_next; /* Skip the comma. */
    }

    rxasquev(context, instrToken, operands, operandCount);
    free(operands);
}

/* Queue opcode  */
void rxasque0(Assembler_Context *context, Assembler_Token *instrToken) {
    rxasquev(context, instrToken, 0, 0);
}

/* Queue opcode  */
void rxasque1(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token) {
    Assembler_Token *operands[] = {operand1Token};
    rxasquev(context, instrToken, operands, 1);
}

/* Queue opcode  */
void rxasque2(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token,
              Assembler_Token *operand2Token) {
    Assembler_Token *operands[] = {operand1Token, operand2Token};
    rxasquev(context, instrToken, operands, 2);
}

/* Queue opcode  */
void rxasque3(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token,
              Assembler_Token *operand2Token, Assembler_Token *operand3Token) {
    Assembler_Token *operands[] = {operand1Token, operand2Token, operand3Token};
    rxasquev(context, instrToken, operands, 3);
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
        while (context->optimiser_queue_items) {
            retire_oldest_queue_item(context);
        }

        rxas_flow_optimise(context,
                           context->procedure_queue,
                           context->procedure_queue_items);

        /* Emit the analysed stream through the unchanged assembler path.
         * EMPTY records are deliberate whole-procedure removals. */
        for (i = 0; i < context->procedure_queue_items; i++) {
            executeQueuedItem(context, context->procedure_queue + i);
            rxas_free_queue_item(context->procedure_queue + i);
            memset(context->procedure_queue + i, 0, sizeof(instruction_queue));
        }
        context->procedure_queue_items = 0;
    }
}
