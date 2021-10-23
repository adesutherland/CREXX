/*
 * RXAS - Keyhole Optimiser Logic
 */

#include "rxasassm.h"
#include "rxvminst.h"

#include "string.h"

/* This defines an instruction to be searched for or as a template output */
typedef struct instruction_pattern {
    char inst_type; /* 0=nothing, 'l'=label, 'i'=instruction */
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
   char* proc[MAX_OP_MAP]; /* Procedure IDs */

   /* Tokens to show what maps are set and the holding the defining token */
   Token *reg_token[MAX_OP_MAP];
   Token *integer_token[MAX_OP_MAP];
   Token *string_token[MAX_OP_MAP];
   Token *character_token[MAX_OP_MAP];
   Token *real_token[MAX_OP_MAP];
   Token *proc_token[MAX_OP_MAP];
   Token *branch_token[MAX_OP_MAP];

   /* Instructions matched in the rules */
   rule *inst_mapped[OPTIMISER_QUEUE_SIZE];
} op_map;

/* The keyhole optimiser rules
 * Each rule is made up of a number of 'i' input instructions and a single 'o'
 * output instruction.
 *
 * The operand matching is done by mapping the actual register to the rules register number,
 * when that actual register is found again it keeps the same mapping. So each
 * input rule much match the instruction and operands. See examples!
 *
 * There may be other instructions in the source code between the matched input
 * instructions as long as they are 'irrelevant', meaning that they do not use
 * any of the register operands in the matching rules. Again see examples!
 *
 * If there is a match then each found in instruction is removed from the queue
 * and the output instruction is inserted in the queue where the last instruction
 * was found.
 *
 * If the out instruction is a null this means there is no output instruction to
 * be inserted
 *
 * Examples assume the following rules:
 *
 * Rule 1 - Two swaps cancelling out: swap r0,r1; swap r0,r1
 *    in_out instruction op1     op2     op3
 *  { 'i',   "swap",     'r', 0, 'r', 1, 0, 0 }
 *  { 'i',   "swap",     'r', 0, 'r', 1, 0, 0 }
 *  { 'o',   0,          0, 0,   0, 0,   0, 0 }
 *
 * Rule 2 -Two swaps cancelling out: swap r0,r1; swap r1,r0
 *    in_out instruction op1     op2     op3
 *  { 'i',   "swap",     'r', 0, 'r', 1, 0, 0 }
 *  { 'i',   "swap",     'r', 1, 'r', 0, 0, 0 }
 *  { 'o',   0,          0, 0,   0, 0,   0, 0 }
 *
 * Rule 3 - sconcat to sappend: sconcat r0,r0,r1 to sappend r0,r1
 *    in_out instruction op1     op2     op3
 *  { 'i',   "sconcat",  'r', 0, 'r', 0, 'r', 1 }
 *  { 'o',   "sappend",  'r', 0, 'r', 1, 0, 0 }
 *
 * Example 1
 *   swap r4,r8
 *   swap r4,r8
 *
 *   Rule 1 matches - rule r0 maps to register r4, and rule r1 maps to register r8
 *   No output - the swaps are removed
 *
 * Example 2
 *   swap r4,r8
 *   swap r8,r4
 *
 *   Rule 2 matches - rule r0 maps to register r4, and rule r1 maps to register r8
 *   This shows how the second rule matches rather than rule 1 because of the
 *   register order in the second input instruction
 *   No output - the swaps are removed
 *
 * Example 3
 *   swap r4,r8
 *   iadd r2,r3,r5
 *   swap r8,r4
 *
 *   Rule 2 matches - rule r0 maps to register r4, and rule r1 maps to register r8
 *   The iadd instruction is 'irrelevant' as it doesn't use r4 or r8
 *   No output - the swaps are removed
 *
 * Example 4
 *   swap r4,r8
 *   say r4
 *   swap r8,r4
 *
 *   Rule 2 does not match - although rule r0 maps to register r4, and rule r1 maps to register r8
 *   the say instruction uses r4. So no match, the swaps cannot be removed
 *   No output
 *
 * Example 5
 *   sconcat r4,r4,r8
 *
 *   Rule 3 matches - rule r0 maps to register r4, and rule r1 maps to register r8
 *   noting that the first two operands are the same register
 *   sconcat is removed and replaced with the faster sappend r4,r8
 *
 */
rule rules[] =
        {

            /* Two swaps cancelling out: swap r0,r1; swap r0,r1 */
            {NO_HAZARD, 'i',"swap", 'r', 0, 'r', 1, 0, 0,
                         0},
            {NO_HAZARD, 'i',"swap", 'r', 0, 'r', 1, 0, 0,
                         0},
            {END_OF_RULE},

            /* Two swaps cancelling out: swap r0,r1; swap r1,r0 */
            {NO_HAZARD, 'i',"swap", 'r', 0, 'r', 1, 0, 0,
                         0},
            {NO_HAZARD, 'i',"swap", 'r', 1, 'r', 0, 0, 0,
                         0},
            {END_OF_RULE},

            /* sconcat to sappend: sconcat r0,r0,r1 to sappend r0,r1 */
            {NO_HAZARD, 'i',"sconcat", 'r', 0, 'r', 0, 'r', 1,
                        'i',"sappend", 'r', 0, 'r', 1, 0, 0},
            {END_OF_RULE},

            /* concat to append: concat r0,r0,r1 to append r0,r1 */
            {NO_HAZARD, 'i',"concat", 'r', 0, 'r', 0, 'r', 1,
                        'i',"append", 'r', 0, 'r', 1, 0, 0},
            {END_OF_RULE},

            /* Unconditional branch to branch false */
            {ANY_GAP,   'i',"br",  'b', 0, 0, 0, 0, 0,
                        'i',"brf", 'b', 1, 'r',0, 0, 0},
            {ANY_GAP,   'l',0,     'l', 0, 0, 0, 0, 0,
                        'l',0,     'l', 0, 0, 0, 0, 0},
            {NO_GAP,    'i',"brf", 'b', 1, 'r', 0, 0, 0,
                        'i',"brf", 'b', 1, 'r',0, 0, 0},
            {END_OF_RULE},

            /* Unconditional branch to branch true */
            {ANY_GAP,   'i',"br",  'b', 0, 0, 0, 0, 0,
                        'i',"brt", 'b', 1, 'r',0, 0, 0},
            {ANY_GAP,   'l',0,     'l', 0, 0, 0, 0, 0,
                        'l',0,     'l', 0, 0, 0, 0, 0},
            {NO_GAP,    'i',"brt", 'b', 1, 'r', 0, 0, 0,
                        'i',"brt", 'b', 1, 'r',0, 0, 0},
            {END_OF_RULE},

            /* NOTE Branch to unconditional branch is optimised later - no rule needed for this */

            /* End of all rules marker */
            {END_OF_RULE}
        };

/* Check if a hazardous instruction is at the end of the queue. For
 * example a branch instruction (i.e. an instruction with a label or function
 * target changes the flow of control, breaking simple keyhole logic).
 * In which case we flush the queue before adding the next instruction */
static int is_hazardous(Token *instrToken, Token *operand1Token,
                              Token *operand2Token, Token *operand3Token) {

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

    if (instrToken->token_type == LABEL)
        return 1;

    /* Other hazardous instructions */
    /*
    else if (!strcmp((char*)(instrToken->token_value.string), "a_hazardous_instruction"))
        return 1;
    */
    return 0;
}

/*
 * Return 1 if the instrToken is relevant
 * Relevant means it uses a mapped register
 */
static int is_relevant(op_map *map, Token *opToken) {
    int i;
    char r_tp;

    if (!opToken) return 0;

    switch(opToken->token_type) {
        case RREG:
            r_tp = 'r';
            break;
        case GREG:
            r_tp = 'g';
            break;
        case AREG:
            r_tp = 'a';
            break;
        default: return 0;
    }
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
static int can_map_operand(op_map *map, Token *opToken, char op_type, size_t op_num) {
    int i;

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
                if (map->regtp[op_num] != op_type ||
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
                if (map->string[op_num] != opToken->token_value.string)
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
            if (opToken->token_type != LABEL ) return 0; /* Wrong Type */
            if (map->branch_token[op_num]) { /* Already Mapped - checked consistent */
                if (strcmp(map->branch[op_num], (char*)opToken->token_value.string) != 0)
                    return 0; /* Wrong value */
            }
            return 1;

        case 'b': /* BRANCH */
            if (opToken->token_type != ID ) return 0; /* Wrong Type */
            if (map->branch_token[op_num]) { /* Already Mapped - checked consistent */
                if (strcmp(map->branch[op_num], (char*)opToken->token_value.string) != 0)
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
static int map_operand(op_map *map, Token *opToken, char op_type, size_t op_num) {
    int i;

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
                if (map->regtp[op_num] != op_type ||
                    map->reg[op_num] != opToken->token_value.integer)
                    return 0; /* Wrong register */
            }
            else { /* Not mapped yet - map it */
                map->reg_token[op_num] = opToken;
                map->regtp[op_num] = op_type;
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
                if (map->string[op_num] != opToken->token_value.string)
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                map->string_token[op_num] = opToken;
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

        case 'l': /* LABEL */
            if (opToken->token_type != LABEL ) return 0; /* Wrong Type */
            if (map->branch_token[op_num]) { /* Already Mapped - checked consistent */
                if (strcmp(map->branch[op_num], (char*)opToken->token_value.string) != 0)
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
                map->branch_token[op_num] = opToken;
                map->branch[op_num] = (char*)opToken->token_value.string;
            }
            return 1;

        case 'b': /* BRANCH */
            if (opToken->token_type != ID ) return 0; /* Wrong Type */
            if (map->branch_token[op_num]) { /* Already Mapped - checked consistent */
                if (strcmp(map->branch[op_num], (char*)opToken->token_value.string) != 0)
                    return 0; /* Wrong value */
            }
            else { /* Not mapped yet - map it */
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
        case 'i':
            if (instruction->instrToken->token_type != ID)
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

        case 'l':
            if (instruction->instrToken->token_type != LABEL)
                return 0; /* Not a label */

            if (!can_map_operand(map, instruction->instrToken,
                             'l', rule->in.opnum1))
                return 0;
            return 1;

        default:
            return 0;
    }
}

/* Maps an instruction can map a rule
 * Returns 1 on success, 0 if the map fails. *map may be changed on either case */
static int map_instruction(op_map *map, instruction_queue *instruction, rule *rule) {

    switch (rule->in.inst_type) {
        case 'i':
            if (instruction->instrToken->token_type != ID)
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

        case 'l':
            if (instruction->instrToken->token_type != LABEL)
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
static Token* mapped_token(op_map *map, char op_type, size_t op_num) {
    switch(op_type) {
        case 'r': /* Register */
            return map->reg_token[op_num];

        case 'i': /* Integer */
            return map->integer_token[op_num];

        case 's': /* String */
            return map->string_token[op_num];

        case 'c': /* Char */
            return map->character_token[op_num];

        case 'f': /* Float */
            return map->real_token[op_num];

        case 'l': /* Label == Branch */
            return map->branch_token[op_num];

        case 'b': /* Branch */
            return map->branch_token[op_num];

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
    Token *new_instruction;

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
                if (r->flag == NO_GAP) return 0; /* No gap allowed! */
                if (r->flag == NO_HAZARD) {
                    /* Is it a hazardous instruction like a branch? */
                    if (is_hazardous(
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

    if (r->flag == END_OF_RULE) {
        /* A match! We need to apply the output rule */
        /* Make sure inst_no is in range */
        if (inst_no >= context->optimiser_queue_items)
            inst_no = context->optimiser_queue_items - 1;
        while (inst_no >= 0) {
            r = map->inst_mapped[inst_no];
            if (r) {
                switch (r->out.inst_type) {
                    case 'i':
                        context->optimiser_queue[inst_no].instrToken =
                                token_id(context,
                                         context->optimiser_queue[inst_no].instrToken,
                                         r->out.instruction);
                        context->optimiser_queue[inst_no].operand1Token =
                                mapped_token(map, r->out.optype1, r->out.opnum1);
                        context->optimiser_queue[inst_no].operand2Token =
                                mapped_token(map, r->out.optype2, r->out.opnum2);
                        context->optimiser_queue[inst_no].operand3Token =
                                mapped_token(map, r->out.optype3, r->out.opnum3);
                        break;

                    case 'l':
                        context->optimiser_queue[inst_no].instrToken =
                                mapped_token(map, 'l', r->out.opnum1);
                        context->optimiser_queue[inst_no].instrToken->token_type = LABEL;
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

static void queue_instruction(Assembler_Context *context, Token *instrToken, Token *operand1Token,
                              Token *operand2Token, Token *operand3Token) {

    /* Check if the queue is full */
    if (context->optimiser_queue_items >= OPTIMISER_QUEUE_SIZE) {
        /* Write the oldest instruction */
        if (context->optimiser_queue[0].instrToken->token_type == LABEL)
            rxaslabl(context, context->optimiser_queue[0].instrToken);
        else
            rxasgen(context, context->optimiser_queue[0].instrToken,
                context->optimiser_queue[0].operand1Token,
                context->optimiser_queue[0].operand2Token,
                context->optimiser_queue[0].operand3Token);

        /* Move the queue */
        memmove(&context->optimiser_queue[0],
                &context->optimiser_queue[1],
                sizeof(instruction_queue) * (OPTIMISER_QUEUE_SIZE - 1));

        /* One less instruction in the queue */
        context->optimiser_queue_items--;
    }

    /* Add to the end of the queue */
    context->optimiser_queue[context->optimiser_queue_items].instrToken = instrToken;
    context->optimiser_queue[context->optimiser_queue_items].operand1Token = operand1Token;
    context->optimiser_queue[context->optimiser_queue_items].operand2Token = operand2Token;
    context->optimiser_queue[context->optimiser_queue_items].operand3Token = operand3Token;
    context->optimiser_queue_items++;

    /* Optimise */
    optimise(context);
}

/** Queue code for the keyhole optimiser */
void rxasque0(Assembler_Context *context, Token *instrToken) {
    if (context->optimise) {
        queue_instruction(context, instrToken, 0, 0, 0);
    }
    else rxasgen0(context, instrToken);
}

void rxasque1(Assembler_Context *context, Token *instrToken, Token *operand1Token) {
    if (context->optimise) {
        queue_instruction(context, instrToken, operand1Token, 0, 0);
    }
    else rxasgen1(context, instrToken, operand1Token);
}

void rxasque2(Assembler_Context *context, Token *instrToken, Token *operand1Token,
              Token *operand2Token) {
    if (context->optimise) {
        queue_instruction(context, instrToken, operand1Token, operand2Token, 0);
    }
    else rxasgen2(context, instrToken, operand1Token, operand2Token);
}

void rxasque3(Assembler_Context *context, Token *instrToken, Token *operand1Token,
              Token *operand2Token, Token *operand3Token) {
    if (context->optimise) {
        queue_instruction(context, instrToken, operand1Token, operand2Token, operand3Token);
    }
    else rxasgen3(context, instrToken, operand1Token, operand2Token, operand3Token);
}

void rxasqlbl(Assembler_Context *context, Token *labelToken) {
    if (context->optimise) {
        queue_instruction(context, labelToken, 0, 0, 0);
    }
    else rxaslabl(context, labelToken);
}

/* Flush the optimiser queue */
void flushopt(Assembler_Context *context) {
    size_t i;
    if (context->optimise) {
        /* Output the queue */
        for (i=0; i<context->optimiser_queue_items; i++) {
            if (context->optimiser_queue[i].instrToken->token_type == LABEL)
                rxaslabl(context, context->optimiser_queue[i].instrToken);
            else
                rxasgen(context, context->optimiser_queue[i].instrToken,
                        context->optimiser_queue[i].operand1Token,
                        context->optimiser_queue[i].operand2Token,
                        context->optimiser_queue[i].operand3Token);
        }
        context->optimiser_queue_items = 0;
    }
}
