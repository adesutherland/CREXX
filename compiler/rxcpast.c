#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxcpmain.h"

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
    prt_unex(stdout, token->token_string, (int) token->length);
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
    node->symbol = 0;
    node->scope = 0;
    node->output = 0;
    node->output2 = 0;
    node->output3 = 0;
    node->output4 = 0;
    node->node_type = type;
    node->value_type = TP_UNKNOWN;
    node->target_type = TP_UNKNOWN;
    node->node_string = "";
    node->node_string_length = 0;
    node->register_num = -1;
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
ASTNode *ast_err(Context* context, const char *error_string, Token *token) {
    ASTNode *errorAST = ast_ftt(context, ERROR, error_string);
    add_ast(errorAST, ast_f(context, TOKEN, token));
    return errorAST;
}

/* Turn a node to an ERROR */
void mknd_err(ASTNode* node, const char *error_string) {
    node->node_type = ERROR;
    node->node_string = error_string;
    node->node_string_length = strlen(error_string);
}

/* ASTNode Factory - Error at last Node */
ASTNode *ast_errh(Context* context, const char *error_string) {
    ASTNode *errorAST = ast_ftt(context, ERROR, error_string);
    add_ast(errorAST, ast_f(context, TOKEN, context->token_tail->token_prev));
    return errorAST;
}

const char *ast_ndtp(NodeType type) {
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
        case FLOAT:
            return "FLOAT";
        case INTEGER:
            return "INTEGER";
        case OP_ADD:
            return "OP_ADD";
        case OP_MINUS:
            return "OP_MINUS";
        case OP_AND:
            return "OP_AND";
        case OP_COMPARE_EQUAL:
            return "OP_COMPARE_EQUAL";
        case OP_COMPARE_NEQ:
            return "OP_COMPARE_NEQ";
        case OP_COMPARE_GT:
            return "OP_COMPARE_GT";
        case OP_COMPARE_LT:
            return "OP_COMPARE_LT";
        case OP_COMPARE_GTE:
            return "OP_COMPARE_GTE";
        case OP_COMPARE_LTE:
            return "OP_COMPARE_LTE";
        case OP_COMPARE_S_EQ:
            return "OP_COMPARE_S_EQ";
        case OP_COMPARE_S_NEQ:
            return "OP_COMPARE_S_NEQ";
        case OP_COMPARE_S_GT:
            return "OP_COMPARE_S_GT";
        case OP_COMPARE_S_LT:
            return "OP_COMPARE_S_LT";
        case OP_COMPARE_S_GTE:
            return "OP_COMPARE_S_GTE";
        case OP_COMPARE_S_LTE:
            return "OP_COMPARE_S_LTE";
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
        case VAR_TARGET:
            return "VAR_TARGET";

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
            printf("%s=", ast_ndtp(node->node_type));
            printf("\"");
            prt_unex(stdout, node->node_string,
                     (int) node->node_string_length);
            printf("\"");
        }
        else {
            printf("%s", ast_ndtp(node->node_type));
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
    ast_wlkr(node, prnt_walker_handler, NULL);
}

ASTNode *add_ast(ASTNode *parent, ASTNode *child) {
    if (child == 0) return child;
    ASTNode *s = parent->child;
    if (s) {
        while (s->sibling) s = s->sibling;
        s->sibling = child;
    } else parent->child = child;
    s = child;
    while (s) {
        s->parent = parent;
        s = s->sibling;
    }
    return child;
}

ASTNode *add_sbtr(ASTNode *older, ASTNode *younger) {
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
        if (t->scope) scp_free(t->scope);
        if (t->output) f_output(t->output);
        if (t->output2) f_output(t->output2);
        if (t->output3) f_output(t->output3);
        if (t->output4) f_output(t->output4);
        free(t);
        t = n;
    }
}

void prt_unex(FILE* output, const char *ptr, int len) {
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

void pdot_scope(Symbol *symbol, void *payload) {
    char reg[20];
    if (symbol->register_num != -1)
        sprintf(reg,"R%d",symbol->register_num);
    else
        reg[0] = 0;

    fprintf((FILE*)payload,
            "s%d_%s[style=filled fillcolor=cyan shape=box label=\"%s\\n(%s)\\n%s\"]\n",
            symbol->scope->defining_node->node_number,
            symbol->name,
            symbol->name,
            type_nm(symbol->type),
            reg);
}

/* Works out the which child index a child has */
static int get_child_index(ASTNode *node) {
    int i = 1;
    ASTNode *n = node->parent;
    if (!n) return 0;
    n = n->child;
    while (n != node) {
        n = n->sibling;
        i++;
    }
    return i;
}

walker_result pdot_walker_handler(walker_direction direction,
                                  ASTNode* node, void *payload) {
    FILE* output = (FILE*)payload;

    char *attributes;
    int only_type = 0;
    int only_label = 0;
    int child_index;
    ASTNode *first_node;
    char value_type_buffer[40];

    if (direction == in) {
        /* IN - TOP DOWN */

        child_index = get_child_index(node);

        /* Scope == DOT Subgraph */
        if (node->scope) {
            fprintf(output, "subgraph scope_%d {\n", node->node_number);
        }

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
            case OP_CONCAT:
            case OP_MULT:
            case OP_DIV:
            case OP_IDIV:
            case OP_MOD:
            case OP_OR:
            case OP_POWER:
            case OP_PREFIX:
            case OP_SCONCAT:
            case OP_COMPARE_EQUAL:
            case OP_COMPARE_NEQ:
            case OP_COMPARE_GT:
            case OP_COMPARE_LT:
            case OP_COMPARE_GTE:
            case OP_COMPARE_LTE:
            case OP_COMPARE_S_EQ:
            case OP_COMPARE_S_NEQ:
            case OP_COMPARE_S_GT:
            case OP_COMPARE_S_LT:
            case OP_COMPARE_S_GTE:
            case OP_COMPARE_S_LTE:
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
            case VAR_TARGET:
            case CONST_SYMBOL:
                attributes = "color=cyan3 shape=cds";
                only_label = 1;
                break;

            case STRING:
            case INTEGER:
            case FLOAT:
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

        if (node->register_num != -1)
            sprintf(value_type_buffer,"\nR%d",node->register_num);
        else
            value_type_buffer[0] = 0;

        if (node->value_type != TP_UNKNOWN ||
            node->target_type != TP_UNKNOWN) {
            if (node->value_type == node->target_type) {
                strcat(value_type_buffer, "\n(");
                strcat(value_type_buffer, type_nm(node->value_type));
                strcat(value_type_buffer, ")");
            } else {
                strcat(value_type_buffer, "\n(");
                strcat(value_type_buffer, type_nm(node->value_type));
                strcat(value_type_buffer, "->");
                strcat(value_type_buffer, type_nm(node->target_type));
                strcat(value_type_buffer, ")");
            }
        }

        if (only_type) {
            fprintf(output, "n%d[ordering=\"out\" label=\"%s%s", node->node_number,
                    ast_ndtp(node->node_type), value_type_buffer);
        } else if (only_label) {
            fprintf(output, "n%d[ordering=\"out\" label=\"", node->node_number);
            prt_unex(output, node->node_string,
                     (int) node->node_string_length);
            fprintf(output, "%s", value_type_buffer);
        } else {
            fprintf(output, "n%d[ordering=\"out\" label=\"%s\\n", node->node_number,
                    ast_ndtp(node->node_type));
            prt_unex(output, node->node_string,
                     (int) node->node_string_length);
            fprintf(output, "%s", value_type_buffer);
        }
        fprintf(output, "\" %s]\n", attributes);

        /* Link to Parent */
        if (node->parent) {
            fprintf(output,"n%d -> n%d [xlabel=\"%d\"]\n",
                    node->parent->node_number,
                    node->node_number, child_index);
        }

        /* Link to Symbol */
        if (node->symbol) {
            if (node->node_type == VAR_TARGET)
                fprintf(output,"n%d -> s%d_%s [color=cyan]\n",
                        node->node_number,
                        node->symbol->scope->defining_node->node_number,
                        node->symbol->name);
            else
                fprintf(output,"n%d -> s%d_%s [color=cyan dir=\"back\"]\n",
                        node->node_number,
                        node->symbol->scope->defining_node->node_number,
                        node->symbol->name);
        }
    }

    else {
        /* OUT - Bottom Up */
        /* Scope == DOT Subgraph */
        if (node->scope) {
            scp_4all(node->scope, pdot_scope, output);
            fprintf(output, "}\n");
        }
    }

    return result_normal;
}

void pdot_tree(ASTNode *tree, char* output_file) {
    FILE *output;

    if (output_file) output = fopen(output_file, "w");
    else output = stdout;

    if (tree) {
        fprintf(output, "digraph REXXAST { pad=0.25\n");
        ast_wlkr(tree, pdot_walker_handler, (void *) output);
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
walker_result ast_wlkr(ASTNode *tree, walker_handler handler, void *payload) {
    walker_result r;
    ASTNode *child;

    if (!tree) return result_error;
    r = handler(in, tree, payload);
    if (r == result_abort || r == result_error) return r;
    else if (r == request_skip) return result_normal;

    if ( (child = tree->child) ) {
        r = ast_wlkr(child, handler, payload);
        if (r == result_abort || r == result_error) return r;

        while ( (child = child->sibling) ) {
            r = ast_wlkr(child, handler, payload);
            if (r == result_abort || r == result_error) return r;
        }
    }

    r = handler(out, tree, payload);
    if (r == result_abort || r == result_error) return r;

    return result_normal;
};
