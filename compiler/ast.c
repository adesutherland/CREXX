#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

/* Token Factory */
Token *token_f(Context *context, int type) {
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
/*    printf("(%d \"", token->token_type); */
    printf("(");
    print_unescaped(stdout, token->token_string,(int)token->length);
    printf(") ");
}

void free_tok(Context *context) {
    Token *t = context->token_head;
    Token *n;
    while (t) {
        n = t->token_next;
        free(t);
        t = n;
    }
}

/* ASTNode Factory - With node type*/
ASTNode *ast_ft(Context* context, NodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->parent = 0;
    node->child = 0;
    node->sibling = 0;
    node->token = 0;
    node->node_type = type;
    node->node_string = "";
    node->node_string_length = 0;
    node->free_list = context->free_list;
    if (node->free_list) node->node_number = node->free_list->node_number + 1;
    else node->node_number = 1;
    context->free_list = node;
    return node;
}

/* ASTNode Factory */
ASTNode *ast_f(Context* context, NodeType type, Token *token) {
    ASTNode *node = ast_ft(context, type);
    node->token = token;
    node->node_string = token->token_string;
    node->node_string_length = token->length;
    return node;
}

/* ASTNode Factory - With node type and string value */
ASTNode *ast_ftt(Context* context, NodeType type, const char *string) {
    ASTNode *node = ast_ft(context, type);
    node->node_string = string;
    node->node_string_length = strlen(string);
    return node;
}

/* ASTNode Factory - Error Node */
ASTNode *ast_error(Context* context, const char *error_string, Token *token) {
    ASTNode *errorAST = ast_ftt(context, ERROR, error_string);
    add_ast(errorAST, ast_f(context, TOKEN, token));
    return errorAST;
}

/* ASTNode Factory - Error at last Node */
ASTNode *ast_error_here(Context* context, const char *error_string) {
    ASTNode *errorAST = ast_ftt(context, ERROR, error_string);
    add_ast(errorAST, ast_f(context, TOKEN, context->token_tail->token_prev));
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
        case OP_MINUS:
            return "OP_MINUS";
        case OP_AND:
            return "OP_AND";
        case OP_COMPARE:
            return "OP_COMPARE";
        case OP_CONCAT:
            return "OP_CONCAT";
        case OP_MULT:
            return "OP_MULT";
        case OP_DIV:
            return "OP_DIV";
        case OP_IDIV:
            return "OP_IDIV";
        case OP_MOD:
            return "OP_MOD";
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
        case REXX_OPTIONS:
            return "REXX_OPTIONS";
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

walker_result prnt_walker_handler(walker_direction direction,
                                        ASTNode* node,
                                  __attribute__((unused)) void *payload) {
    if (direction == in) {
        if (node->child) { /* Non-terminal node */
            printf(" ^(");
        } else printf(" ");
        if (node->node_string_length) {
            printf("%s=", ast_nodetype(node->node_type));
            printf("\"");
            print_unescaped(stdout, node->node_string,
                            (int)node->node_string_length);
            printf("\"");
        }
        else {
            printf("%s", ast_nodetype(node->node_type));
        }
    }
    else {
        if (node->child) { /* Non-terminal node */
            printf(")");
        }
    }
    return result_normal;
}

void prnt_ast(ASTNode *node) {
    ast_walker(node, prnt_walker_handler, NULL);
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

ASTNode *add_sibling_ast(ASTNode *older, ASTNode *younger) {
    if (younger == 0 || older == 0) return younger;
    ASTNode *parent = older->parent;
    while (older->sibling) older = older->sibling;
    older->sibling = younger;
    younger->parent = parent;
    return younger;
}

void free_ast(Context *context) {
    ASTNode *t = context->free_list;
    ASTNode *n;
    while (t) {
        n = t->free_list;
        free(t);
        t = n;
    }
}

void print_unescaped(FILE* output, const char *ptr, int len) {
    int i;
    if (!ptr) return;
    for (i = 0; i < len; i++, ptr++) {
        switch (*ptr) {
            case '\0':
                fprintf(output, "\\0");
                break;
            case '\a':
                fprintf(output, "\\a");
                break;
            case '\b':
                fprintf(output, "\\b");
                break;
            case '\f':
                fprintf(output, "\\f");
                break;
            case '\n':
                fprintf(output, "\\n");
                break;
            case '\r':
                fprintf(output, "\\r");
                break;
            case '\t':
                fprintf(output, "\\t");
                break;
            case '\v':
                fprintf(output, "\\v");
                break;
            case '\\':
                fprintf(output, "\\\\");
                break;
            case '\?':
                fprintf(output, "\\?");
                break;
            case '\'':
                fprintf(output, "\\'");
                break;
            case '\"':
                fprintf(output, "\\\"");
                break;
            default:
                fprintf(output, "%c", *ptr);
        }
    }
}

walker_result pdot_walker_handler(walker_direction direction,
                                  ASTNode* node, void *payload) {
    FILE* output = (FILE*)payload;

    char *attributes;
    int only_type = 0;
    int only_label = 0;

    if (direction == in) {
        /* Attributes */
        switch (node->node_type) {

            /* Groupings */
            case PROGRAM_FILE:
            case INSTRUCTIONS:
            case DO:
            case BY:
            case IF:
            case REXX_OPTIONS:
            case TO:
                attributes = "color=blue";
                only_type = 1;
                break;

            case ASSIGN:
            case ARG:
            case CALL:
            case ENVIRONMENT:
            case FOR:
            case FUNCTION:
            case ITERATE:
            case LEAVE:
            case OPTIONS:
            case PROCEDURE:
            case PULL:
            case REPEAT:
            case RETURN:
            case SAY:
            case UPPER:
            case PARSE:
                attributes = "color=green4";
                only_type = 1;
                break;

                /* Address is often a sign of a parsing error */
            case ADDRESS:
                attributes = "style=filled fillcolor=orange";
                only_type = 1;
                break;

            case OP_ADD:
            case OP_MINUS:
            case OP_AND:
            case OP_COMPARE:
            case OP_CONCAT:
            case OP_MULT:
            case OP_DIV:
            case OP_IDIV:
            case OP_MOD:
            case OP_OR:
            case OP_POWER:
            case OP_PREFIX:
            case OP_SCONCAT:
                attributes = "color=darkcyan";
                only_type = 1;
                break;

            case PATTERN:
            case REL_POS:
            case ABS_POS:
            case SIGN:
            case TARGET:
            case TEMPLATES:
                attributes = "color=green";
                break;

            case VAR_SYMBOL:
            case CONST_SYMBOL:
                attributes = "color=cyan3 shape=cds";
                only_label = 1;
                break;

            case STRING:
            case NUMBER:
                attributes = "color=cyan3 shape=box";
                only_label = 1;
                break;

            case LABEL:
                attributes = "color=green4";
                break;

                /* Errors */
            case TOKEN:
                attributes = "style=filled fillcolor=indianred1";
                only_label = 1;
                break;

//            case ERROR:
//                attributes = "style=filled fillcolor=indianred1";
//                //           only_type = 1;
//                break;

            default:
                attributes = "style=filled fillcolor=indianred1";
                break;
        }

        if (only_type) {
            fprintf(output, "n%d[label=\"%s", node->node_number,
                    ast_nodetype(node->node_type));
        } else if (only_label) {
            fprintf(output, "n%d[label=\"", node->node_number);
            print_unescaped(output, node->node_string,
                            (int)node->node_string_length);
        } else {
            fprintf(output, "n%d[label=\"%s\\n", node->node_number,
                    ast_nodetype(node->node_type));
            print_unescaped(output, node->node_string,
                            (int)node->node_string_length);
        }
        fprintf(output, "\" %s]\n", attributes);

        /* Link to Parent */
        if (node->parent) {
            fprintf(output,"n%d -> n%d\n", node->parent->node_number, node->node_number);
        }
    }

    return result_normal;
}

void pdot_tree(ASTNode *tree, char* output_file) {
    FILE *output;

    if (output_file) output = fopen(output_file, "w");
    else output = stdout;

    if (tree) {
        fprintf(output, "digraph REXXAST {\n");
        ast_walker(tree, pdot_walker_handler, (void*)output);
        fprintf(output, "\n}\n");
    }
    if (output_file) fclose(output);
}

/* AST Walker
 * It returns
 *     result_normal - All OK - normal processing
 *     result_abort - Walk Aborted by handler
 *     result_error - error condition
 */
walker_result ast_walker(ASTNode *tree, walker_handler handler, void *payload) {
    walker_result r;
    ASTNode *child;

    if (!tree) return result_error;
    r = handler(in, tree, payload);
    if (r == result_abort || r == result_error) return r;
    else if (r == request_skip) return result_normal;

    if ( (child = tree->child) ) {
        r = ast_walker(child, handler, payload);
        if (r == result_abort || r == result_error) return r;

        while ( (child = child->sibling) ) {
            r = ast_walker(child, handler, payload);
            if (r == result_abort || r == result_error) return r;
        }
    }

    r = handler(out, tree, payload);
    if (r == result_abort || r == result_error) return r;

    return result_normal;
};
