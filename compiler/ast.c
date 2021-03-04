#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rexxgrmr.h"
#include "compiler.h"

/* Token Factory */
Token *token_f(Scanner *context, int type) {
    Token *token = malloc(sizeof(Token));
    token->token_type = type;

    /* Link it up */
    if (context->token_tail) {
        token->token_next = 0;
        token->token_prev = context->token_tail;
        context->token_tail->token_next = token;
        context->token_tail = token;
    } else {
        context->token_head = token;
        context->token_tail = token;
        token->token_next = 0;
        token->token_prev = 0;
    }
    token->token_number = ++(context->token_counter);
    token->token_subtype = 0; /* TODO */
    token->length = context->cursor - context->top;
    token->line = context->line;
    token->column = context->top - context->linestart + 1;
    token->token_string = context->top;

    return token;
}

void prnt_tok(Token *token) {
/*
    printf("%d.%d %s \"%.*s\"",token->line+1,token->column+1,
           token_type_name(token->token_type),token->length,token->token_string);
*/
    printf("%s \"%.*s\"",
           token_type_name(token->token_type), (int) token->length,
           token->token_string);

}

void free_tok(Scanner *context) {
    Token *t = context->token_head;
    Token *n;
    while (t) {
        n = t->token_next;
        free(t);
        t = n;
    }
}

/* ASTNode Factory */
ASTNode *ast_f(Scanner* context, NodeType type, Token *token) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->parent = 0;
    node->child = 0;
    node->sibling = 0;
    node->token = token;
    node->node_type = type;
    node->context = context;
    node->node_string = token->token_string;
    node->length = token->length;
    context->last_node = node;
    return node;
}

/* ASTNode Factory - With node type*/
ASTNode *ast_ft(Scanner* context, NodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->parent = 0;
    node->child = 0;
    node->sibling = 0;
    node->token = 0;
    node->node_type = type;
    node->context = context;
    node->node_string = "";
    node->length = 0;
    context->last_node = node;
    return node;
}

/* ASTNode Factory - With node type and string value */
ASTNode *ast_ftt(Scanner* context, NodeType type, char *string) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->parent = 0;
    node->child = 0;
    node->sibling = 0;
    node->token = 0;
    node->node_type = type;
    node->context = context;
    node->node_string = string;
    node->length = strlen(string);
    context->last_node = node;
    return node;
}

/* ASTNode Factory - Error Node */
ASTNode *ast_error(Scanner* context, char *error_string, Token *token) {
    ASTNode *errorAST = ast_ftt(context, ERROR, error_string);
    add_ast(errorAST, ast_f(context, TOKEN, token));
    return errorAST;
}

const char *ast_nodetype(NodeType type) {
    switch (type) {
        case ABS_POS:
            return "ABS_POS";
        case ADDRESS:
            return "ADDRESS";
        case ARG:
            return "ARG";
        case ASSIGN:
            return "ASSIGN";
        case BY:
            return "BY";
        case CALL:
            return "CALL";
        case CONST_SYMBOL:
            return "CONST_SYMBOL";
        case DO:
            return "DO";
        case ENVIRONMENT:
            return "ENVIRONMENT";
        case ERROR:
            return "ERROR";
        case FOR:
            return "FOR";
        case FUNCTION:
            return "FUNCTION";
        case IF:
            return "IF";
        case INSTRUCTIONS:
            return "INSTRUCTIONS";
        case ITERATE:
            return "ITERATE";
        case LABEL:
            return "LABEL";
        case LEAVE:
            return "LEAVE";
        case NUMBER:
            return "NUMBER";
        case OP_ADD:
            return "OP_ADD";
        case OP_AND:
            return "OP_AND";
        case OP_COMPARE:
            return "OP_COMPARE";
        case OP_CONCAT:
            return "OP_CONCAT";
        case OP_MULT:
            return "OP_MULT";
        case OP_OR:
            return "OP_OR";
        case OP_POWER:
            return "OP_POWER";
        case OP_PREFIX:
            return "OP_PREFIX";
        case OP_SCONCAT:
            return "OP_SCONCAT";
        case OPTIONS:
            return "OPTIONS";
        case PARSE:
            return "PARSE";
        case PATTERN:
            return "PATTERN";
        case PROCEDURE:
            return "PROCEDURE";
        case PROGRAM_FILE:
            return "PROGRAM_FILE";
        case PULL:
            return "PULL";
        case REL_POS:
            return "REL_POS";
        case REPEAT:
            return "REPEAT";
        case RETURN:
            return "RETURN";
        case REXX:
            return "REXX";
        case SAY:
            return "SAY";
        case SIGN:
            return "SIGN";
        case STRING:
            return "STRING";
        case TARGET:
            return "TARGET";
        case TEMPLATES:
            return "TEMPLATES";
        case TO:
            return "TO";
        case TOKEN:
            return "TOKEN";
        case UPPER:
            return "UPPER";
        case VAR_SYMBOL:
            return "VAR_SYMBOL";
        default: return "*UNKNOWN*";
    }
}

void prnt_ast(ASTNode *node) {
    printf("[");
    printf("%s:", ast_nodetype(node->node_type));
    printf("\"%.*s\"", (int) node->length, node->node_string);
    if (node->child) {
        printf(" (");
        prnt_ast(node->child);
        printf(")");
    }
    printf("]");
    if (node->sibling) {
        printf(" ");
        prnt_ast(node->sibling);
    }
}

ASTNode *add_ast(ASTNode *parent, ASTNode *child) {
    if (child == 0) return child;
    ASTNode *s = parent->child;
    if (s) {
        while (s->sibling) s = s->sibling;
        s->sibling = child;
    } else parent->child = child;
    child->parent = parent;
    return child;
}

void free_ast(ASTNode *node) {
    if (node->child) free_ast(node->child);
    if (node->sibling) free_ast(node->sibling);
    free(node);
}

void print_unescaped(char *ptr, int len) {
    int i;
    if (!ptr) return;
    for (i = 0; i < len; i++, ptr++) {
        switch (*ptr) {
            case '\0':
                printf("\\0");
                break;
            case '\a':
                printf("\\a");
                break;
            case '\b':
                printf("\\b");
                break;
            case '\f':
                printf("\\f");
                break;
            case '\n':
                printf("\\n");
                break;
            case '\r':
                printf("\\r");
                break;
            case '\t':
                printf("\\t");
                break;
            case '\v':
                printf("\\v");
                break;
            case '\\':
                printf("\\\\");
                break;
            case '\?':
                printf("\\\?");
                break;
            case '\'':
                printf("\\\'");
                break;
            case '\"':
                printf("\\\"");
                break;
            default:
                printf("%c", *ptr);
        }
    }
}

void pdot_ast(ASTNode *node, int parent, int *counter) {
    int me = *counter;

    printf("n%d[label=\"%s\\n", *counter, ast_nodetype(node->node_type));
    print_unescaped(node->token->token_string, node->token->length);
    printf("\"]\n");

    if (node->child) {
        (*counter)++;
        printf("n%d -> n%d\n", me, *counter);
        pdot_ast(node->child, me, counter);
    }
    if (node->sibling) {
        (*counter)++;
        printf("n%d -> n%d\n", parent, *counter);
        pdot_ast(node->sibling, parent, counter);
    }
}
