// REXX Assembler
// Token Library

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "platform.h"
#include "rxasgrmr.h"
#include "rxas.h"


/* Token Factory */
Token* token_f(Assembler_Context* context, int type) {
    size_t extra_for_value;

    /* This is a good place to process token values (e.g. lowercase, type
     * conversion). First step is to work out the size so we can malloc
     * enough memory  */
    switch (type) {
        case LABEL:
        case ID:
        case FUNC:
        case STRING:
            extra_for_value = context->cursor - context->top - sizeof(((Token*)0)->token_value) + 1;
            if (extra_for_value < 0) extra_for_value = 0;
            break;
        default:
            extra_for_value = 0;
    }

    Token* token = malloc(sizeof(Token) + extra_for_value);
    token->token_type = type;

    /* Link it up */
    if (context->token_tail) {
        token->token_next = 0;
        token->token_prev = context->token_tail;
        context->token_tail->token_next = token;
        context->token_tail = token;
    }
    else {
        context->token_head = token;
        context->token_tail = token;
        token->token_next = 0;
        token->token_prev = 0;
    }
    token->token_number = ++(context->token_counter);
    token->token_subtype = 0;
    token->length = context->cursor - context->top;
    token->line = context->line;
    token->column = context->top - context->linestart + 1;
    token->token_source = context->top;

    /* Now we work out the useful token value */
    char *buffer;
    char *c;
    switch (type) {
        case INT:
            /* Need to null terminate - sigh */
            buffer = malloc(token->length + 1);
            memcpy(buffer, token->token_source, token->length);
            buffer[token->length] = 0;
#ifdef __32BIT__
            token->token_value.integer = atol(buffer);
#else
            token->token_value.integer = atoll(buffer);
#endif
            free(buffer);
            break;
        case FLOAT:
            /* Need to null terminate */
            buffer = malloc(token->length + 1);
            memcpy(buffer, token->token_source, token->length);
            buffer[token->length] = 0;
            token->token_value.real = atof(buffer);
            free(buffer);
            break;
        case LABEL:
            memcpy(token->token_value.string, token->token_source, token->length);
            token->token_value.string[token->length-1] = 0; /* Remove the ":" */
            for (c = token->token_value.string; *c; ++c) *c = (char) tolower(*c);
            break;
        case ID:
            memcpy(token->token_value.string, token->token_source, token->length);
            token->token_value.string[token->length] = 0;
            for (c = token->token_value.string; *c; ++c) *c = (char) tolower(*c);
            break;
        case RREG:
        case GREG:
        case AREG:
            /* Need to null terminate */
            buffer = malloc(token->length);
            memcpy(buffer, token->token_source + 1, token->length - 1);
            buffer[token->length - 1] = 0;
            token->token_value.integer = atoi(buffer);
            free(buffer);
            /* Subtype = type of register */
            token->token_subtype = tolower(token->token_source[0]);
            break;
        case FUNC:
            memcpy(token->token_value.string, token->token_source, token->length);
            token->token_value.string[token->length-2] = 0; /* Remove the "()" */
            for (c = token->token_value.string; *c; ++c) *c = (char) tolower(*c);
            break;
        case CHAR:
            /* TODO escape chars */
            token->token_value.character = token->token_source[1];
            break;
        case STRING:
            /* Remove the "s */
            memcpy(token->token_value.string, token->token_source + 1, token->length - 2);
            token->token_value.string[token->length-2] = 0;
            break;
        default:
            token->token_value.integer = 0;
    }

    context->top = context->cursor;

    return token;
}

void prnt_tok(Token* token) {
/*
    printf("%d:%d %s \"%.*s\"", (int)token->line, (int)token->column,
           token_type_name(token->token_type),
           (int)token->length,token->token_source);
*/
    printf("%s", tk_tp_nm(token->token_type));
    switch (token->token_type) {
        case INT:
            printf("[%d] ", (int) token->token_value.integer);
            break;
        case FLOAT:
            printf("[%f] ", token->token_value.real);
            break;
        case CHAR:
            printf("[%c] ", token->token_value.character);
            break;
        case RREG:
        case AREG:
        case GREG:
            printf("[%c%d] ", token->token_subtype, (int) token->token_value.integer);
            break;
        case LABEL:
        case ID:
        case FUNC:
        case STRING:
            printf("[%s] ", token->token_value.string);
            break;
        default:
            printf(" ");
    }
}

void free_tok(Assembler_Context* context) {
    Token *t = context->token_head;
    Token *n;
    while (t) {
        n = t->token_next;
        free(t);
        t = n;
    }
}
