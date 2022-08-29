// REXX Assembler
// Error Library

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxasgrmr.h"
#include "rxas.h"

/* Compares the position (line / column) of a and b
 * return <0 if a<b, >0 if a>=b */
static int compare_error(Assembler_Error *a, Assembler_Error *b) {
    if (a->line != b->line) return a->line - b->line;
    if (a->column != b->column) return a->column - b->column;
    return 1;
}

/* Error Factory */
Assembler_Error* rxaserrf(Assembler_Context* context, int line, int column,
                          int severity, char *message) {

    Assembler_Error *error = malloc(sizeof(Assembler_Error));
    Assembler_Error *i, *j;

    /* Set data */
    error->line = line;
    error->column = column;
    error->severity = severity;
    strncpy(error->message, message, MAX_ERROR_LENGTH);
    error->message[MAX_ERROR_LENGTH-1] = 0;
    error->next_error = 0; /* rxasperr() sets and uses this */
    if (severity > context->severity) context->severity = severity;

    /* Link it up */
    if (context->error_tail) {
        if (compare_error(error,context->error_tail) > 0) {
            error->prev_error = context->error_tail;
            context->error_tail = error;
        }
        else {
            j = context->error_tail;
            i = (context->error_tail)->prev_error;
            while (i && compare_error(error,i) < 0) {
                j = i;
                i = i->prev_error;
            }
            j->prev_error = error;
            error->prev_error = i;
         }
    }
    else {
        error->prev_error = 0;
        context->error_tail = error;
    }

    return error;
}

void rxasperr(Assembler_Context* context) {
    /* Find first Error and set next_error pointers */
    Assembler_Error *e = context->error_tail;
    Assembler_Error *p;
    while (e) {
        p = e->prev_error;
        if (p) {
            p->next_error = e;
            e = p;
        }
        else break;
    }

    /* Print Errors - e is now the first error */
    if (context->severity != 0) {
        printf("\nError Severity %d in file %s\n", context->severity, context->file_name);
        while (e) {
            printf("%d:%d - %s\n", e->line, e->column, e->message);
            e = e->next_error;
        }
    }
}

void rxasfrer(Assembler_Context* context) {
    Assembler_Error *e = context->error_tail;
    Assembler_Error *p;
    while (e) {
        p = e->prev_error;
        free(e);
        e = p;
    }
}

void rxaseaft(Assembler_Context* context, Assembler_Token* after_token, char* message) {
    char buffer[MAX_ERROR_LENGTH];
    if (after_token->optimised) {
        snprintf(buffer, MAX_ERROR_LENGTH, "INTERNAL ERROR after \"%.*s\" optimised to \"%s\", %s",
                 (int) after_token->length, after_token->token_source, after_token->token_value.string, message);
        rxaserrf(context, (int) after_token->line, (int) after_token->column, 2, buffer);
    }
    else {

        snprintf(buffer, MAX_ERROR_LENGTH, "Error after \"%.*s\", %s",
                 (int) after_token->length, after_token->token_source, message);
        rxaserrf(context, (int) after_token->line,
                 (int) (after_token->column + after_token->length), 1, buffer);
    }
}

void rxaserat(Assembler_Context* context, Assembler_Token* after_token, char* message) {
    char buffer[MAX_ERROR_LENGTH];
    if (after_token->optimised) {
        snprintf(buffer, MAX_ERROR_LENGTH, "INTERNAL ERROR at \"%.*s\" optimised to \"%s\", %s",
                 (int) after_token->length, after_token->token_source, after_token->token_value.string, message);
        rxaserrf(context, (int) after_token->line, (int) after_token->column, 2, buffer);
    }
    else {
        snprintf(buffer, MAX_ERROR_LENGTH, "Error at \"%.*s\", %s",
                 (int) after_token->length, after_token->token_source, message);
        rxaserrf(context, (int) after_token->line, (int) after_token->column, 1, buffer);
    }
}
