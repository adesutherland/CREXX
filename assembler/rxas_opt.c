/*
 * RXAS - Keyhole Optimiser Logic
 */

#include "rxasassm.h"
#include "rxvminst.h"

#include "string.h"

typedef struct rule {
    char in_out;
    char* instruction;
    char optype1;
    size_t opnum1;
    char optype2;
    size_t opnum2;
    char optype3;
    size_t opnum3;
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

   /* Flags to show what maps are set */
   Token *reg_token[MAX_OP_MAP];
   Token *integer_token[MAX_OP_MAP];
   Token *string_token[MAX_OP_MAP];
   Token *character_token[MAX_OP_MAP];
   Token *real_token[MAX_OP_MAP];
   Token *func_token;

   /* Set if the number of rules used > MAX_OP_MAP - in which case the rule never matches */
   char overflowed;

   /* Instructions matched in the rules */
    char inst_mapped[OPTIMISER_QUEUE_SIZE];
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
                {'i', "swap", 'r', 0, 'r', 1, 0, 0},
                {'i', "swap", 'r', 0, 'r', 1, 0, 0},
                {'o', 0, 0, 0, 0, 0, 0, 0},

                /* Two swaps cancelling out: swap r0,r1; swap r1,r0 */
                {'i', "swap", 'r', 0, 'r', 1, 0, 0},
                {'i', "swap", 'r', 1, 'r', 0, 0, 0},
                {'o', 0, 0, 0, 0, 0, 0, 0},

                /* sconcat to sappend: sconcat r0,r0,r1 to sappend r0,r1 */
                {'i', "sconcat", 'r', 0, 'r', 0, 'r', 1},
                {'o', "sappend", 'r', 0, 'r', 1, 0, 0},

                /* concat to append: concat r0,r0,r1 to append r0,r1 */
                {'i', "concat", 'r', 0, 'r', 0, 'r', 1},
                {'o', "append", 'r', 0, 'r', 1, 0, 0},

                /* End of rules marker */
                {0, 0, 0, 0, 0, 0, 0, 0}
        };

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

/* Checks if a instrToken can map against an op_type and op_num
 * returns 1 if it can map, otherwise 0
 * NOTE: if it can be mapped the *map structure is updated */
static int can_map(op_map *map, Token *opToken, char op_type, size_t op_num) {
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

        case 'j': /* Jump, meaning an ID or FUNC - op_num is not used because the
                   * jump is a hazard instruction which will flush the instruction
                   * queue. Meaning there can only ever be one label in a rule */
            if ( !(opToken->token_type == FUNC ||
                   opToken->token_type == ID ) ) return 0; /* Wrong Type */
            else {
                map->func_token = opToken;
                return 1;
            }

        case 0:   /* Check nulls match */
            return 1;

        default: return 0; /* Something is wrong */
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

        case 'j': /* Jump, meaning an ID or FUNC - op_num is not used */
            return map->func_token;

        case 0:   /* Check nulls match */
        default:  /* Something wrong */
            return 0;
    }
}

/* Optimise a rule starting from a specific instruction
 * returns 1 if the rule was successfully applied */
static int optimise_rule(Assembler_Context *context, op_map *map, rule *r, int inst_no) {

    /* Clear Map */
    memset(map, 0, sizeof(*map));

    /* Check the instruction maps */
    if (!can_map(map, context->optimiser_queue[inst_no].operand1Token, r->optype1, r->opnum1))
        return 0;
    if (!can_map(map, context->optimiser_queue[inst_no].operand2Token, r->optype2, r->opnum2))
        return 0;
    if (!can_map(map, context->optimiser_queue[inst_no].operand3Token, r->optype3, r->opnum3))
        return 0;
    r++;

    map->inst_mapped[inst_no] = 1;

    /* Process next input rules */
    while (r->in_out == 'i' && inst_no < context->optimiser_queue_items) {
        for (inst_no++; inst_no < context->optimiser_queue_items; inst_no++) {
            if (!strcmp((char*)(context->optimiser_queue[inst_no].instrToken->token_value.string), r->instruction)) {
                /* Could be the instruction we are looking for */
                /* First see if the instruction is Relevant */
                if ( !( !is_relevant(map, context->optimiser_queue[inst_no].operand1Token) &&
                        !is_relevant(map, context->optimiser_queue[inst_no].operand2Token) &&
                        !is_relevant(map, context->optimiser_queue[inst_no].operand3Token) &&
                        context->optimiser_queue[inst_no].operand1Token ) ) {
                    /* Relevant - so check if it is a match */
                    if ( can_map(map, context->optimiser_queue[inst_no].operand1Token, r->optype1, r->opnum1) &&
                         can_map(map, context->optimiser_queue[inst_no].operand2Token, r->optype2, r->opnum2) &&
                         can_map(map, context->optimiser_queue[inst_no].operand3Token, r->optype3, r->opnum3) ) {

                        map->inst_mapped[inst_no] = 1;
                        r++;
                        break;
                    }
                    else return 0; /* Match Failed */
                }
            }
            else {
                /* Not the instruction we are looking for - make sure its irrelevant */
                if (is_relevant(map, context->optimiser_queue[inst_no].operand1Token))
                    return 0; /* If it is relevant we can't match the rule */
                if (is_relevant(map, context->optimiser_queue[inst_no].operand2Token))
                    return 0; /* If it is relevant we can't match the rule */
                if (is_relevant(map, context->optimiser_queue[inst_no].operand3Token))
                    return 0; /* If it is relevant we can't match the rule */
            }
        }
    }

    if (r->in_out == 'o') {
        /* A match! We need to apply the output rule */
        if (r->instruction) {
            Token
                    *new_instruction =
                    token_id(context,
                             context->optimiser_queue[inst_no].instrToken,
                             r->instruction);
            context->optimiser_queue[inst_no].instrToken = new_instruction;

            context->optimiser_queue[inst_no].operand1Token =
                    mapped_token(map, r->optype1, r->opnum1);

            context->optimiser_queue[inst_no].operand2Token =
                    mapped_token(map, r->optype2, r->opnum2);

            context->optimiser_queue[inst_no].operand3Token =
                    mapped_token(map, r->optype3, r->opnum3);

            inst_no--;
        }

        /* Now we need to delete the other matched instructions */
        while (inst_no >= 0) {
            if (map->inst_mapped[inst_no]) {
                /* Remove from the queue */
                if (OPTIMISER_QUEUE_SIZE - inst_no - 1 > 0 ) {
                    memmove(&context->optimiser_queue[inst_no],
                            &context->optimiser_queue[inst_no + 1],
                            sizeof(instruction_queue) *
                            (OPTIMISER_QUEUE_SIZE - inst_no - 1));
                }
                /* One less instruction in the queue */
                context->optimiser_queue_items--;
            }
            inst_no--;
        }
        return 1;
    }

    return 0;
}

/* Optimise a rule across the queue of instructions
 * Updates the rule pointer to the start of the next rule
 * returns 1 if the rule was successfully applied */
static int optimise_rule_in_queue(Assembler_Context *context, rule **r ) {
    size_t i;
    op_map map;
    int result = 0;

    for (i=0; i<context->optimiser_queue_items; i++) {
        if (!strcmp((char*)(context->optimiser_queue[i].instrToken->token_value.string), (*r)->instruction)) {
           if (optimise_rule(context, &map, *r, i)) {
               result = 1;
               break;
           }
        }
    }

    while ((*r)->in_out == 'i') (*r)++;
    if ((*r)->in_out == 'o') (*r)++;

    return result;
}

static void optimise(Assembler_Context *context) {
    rule *r;
    int changed;
    do {
        changed = 0;
        r = &rules[0];
        while (r->in_out)
            if (optimise_rule_in_queue(context, &r)) changed = 1;
    } while (changed);
}

static void queue_instruction(Assembler_Context *context, Token *instrToken, Token *operand1Token,
                              Token *operand2Token, Token *operand3Token) {
    /* Check if a hazardous instruction is at the end of the queue. For
     * example a branch instruction (i.e. an instruction with a label or function
     * target changes the flow of control, breaking simple keyhole logic).
     * In which case we flush the queue before adding the next instruction */

    /* Calls or branches */
    if (operand1Token &&
            (operand1Token->token_type == ID || operand1Token->token_type == FUNC))
        flushopt(context);
    else if (operand2Token &&
            (operand2Token->token_type == ID || operand2Token->token_type == FUNC))
        flushopt(context);
    else if (operand3Token &&
            (operand3Token->token_type == ID || operand3Token->token_type == FUNC))
        flushopt(context);

    /* Other hazardous instructions */
    /*
    else if (!strcmp((char*)(instrToken->token_value.string), "a_hazardous_instruction"))
        flushopt(context);
    */

        /* Check if the queue is full */
    if (context->optimiser_queue_items >= OPTIMISER_QUEUE_SIZE) {
        /* Write the oldest instruction */
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

/* Flush the optimiser queue */
void flushopt(Assembler_Context *context) {
    size_t i;
    if (context->optimise) {
        /* Output the queue */
        for (i=0; i<context->optimiser_queue_items; i++)
            rxasgen(context, context->optimiser_queue[i].instrToken,
                    context->optimiser_queue[i].operand1Token,
                    context->optimiser_queue[i].operand2Token,
                    context->optimiser_queue[i].operand3Token);
        context->optimiser_queue_items = 0;
    }
}
