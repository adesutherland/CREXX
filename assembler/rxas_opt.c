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

rule rules[] =
        {
                {'i', "swap", 'r', 0, 'r', 1, 0, 0},
                {'i', "swap", 'r', 0, 'r', 1, 0, 0},
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
           /* printf("inst=%s\n", inst); */


        }
    }

    while (r->in_out && !strcmp(r->instruction,inst)) r++;
    return r;
}

static void optimise(Assembler_Context *context) {
    rule *r = &rules[0];

    while (r->in_out)
        r = optimise_rule(context, r);
}

static void queue_instruction(Assembler_Context *context, Token *instrToken, Token *operand1Token,
Token *operand2Token, Token *operand3Token) {
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
