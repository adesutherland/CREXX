/*
 * RXAS - Keyhole Optimiser Logic
 */

#include "rxasassm.h"
#include "string.h"

typedef struct rule {
    char in_out;
    char* instruction;
    char optype1;
    int opnum1;
    char optype2;
    int opnum2;
    char optype3;
    int opnum3;
} rule;

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
 * and the output instruction is inserted in the queue where the first instruction
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
                {'i', "sconcat", 'r', 0, 'r', 0, 'r', 1},
                {'o', "sappend", 'r', 0, 'r', 1, 0, 0},

                /* End of rules marker */
                {0, 0, 0, 0, 0, 0, 0, 0}
        };

static rule* optimise_rule(Assembler_Context *context, rule *r ) {
    char *inst = r->instruction;
    size_t i;

    for (i=0; i<context->optimiser_queue_items; i++) {
        if (!strcmp((char*)(context->optimiser_queue[i].instrToken->token_value.string), inst)) {
            /*
           printf("inst=%s %s %d\n",
                  inst,
                  (char*)(context->optimiser_queue[i].instrToken->token_value.string),
                  (int)context->optimiser_queue[i].instrToken->line);
           */

        }
    }

    while (r->in_out == 'i') r++;
    if (r->in_out == 'o') r++;

    return r;
}

static void optimise(Assembler_Context *context) {
    rule *r = &rules[0];

    while (r->in_out)
        r = optimise_rule(context, r);
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
