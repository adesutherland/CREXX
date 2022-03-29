#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
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

    context->top = context->cursor;

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
    node->association = 0;
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
    node->free_node_string = 0;
    node->int_value = 0;
    node->bool_value = 0;
    node->float_value = 0;
    node->register_num = -1;
    node->register_type = 'r';
    node->additional_registers = -1;
    node->num_additional_registers = 0;
    node->is_ref_arg = 0;
    node->is_opt_arg = 0;
    node->free_list = context->free_list;
    if (node->free_list) node->node_number = node->free_list->node_number + 1;
    else node->node_number = 1;
    context->free_list = node;

    /* These values are normally set by the set_source_location walker
     *  However nodes (e.g. unspecified optional arguments) can be added after
     *  the walker has been run - so we have added logic to add_ast() and add_sbtr()
     *  to set these in this situation
     * */
    node->token_start = 0;
    node->token_end = 0;
    node->source_start = 0;
    node->source_end = 0;
    node->line = -1;
    node->column = -1;
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

/* ASTNode Factory - adds a STRING token removing the leading & trailing speech marks
 * and decoding / encoding the string nicely */
/* TODO - X and B suffix */
#define ADD_CHAR_TO_BUFFER(ch) { processed_length++; *(buffer++) = (ch); }
ASTNode *ast_fstr(Context* context, Token *token) {
    unsigned char separator;
    char* raw_string;
    size_t raw_length;
    char *processed_string;
    char *buffer;
    size_t processed_length;

    /* Make the token */
    ASTNode *node = ast_ft(context, STRING);
    node->token = token;

    /* Prepare for processing string */
    separator = token->token_string[0];
    raw_string = token->token_string + 1;
    if (token->token_string[token->length - 1] != separator)
        raw_length = token->length - 3; /* I.e. There "must" be an X or B suffix which we don't support yet */
    else
        raw_length = token->length - 2;
    processed_length = 0;
    processed_string = malloc(raw_length * 2); /* Worse case */

    /* Code String - basically RexxAssembler uses C type escapes */
    buffer = processed_string;
    while (raw_length) {
        /* Decode REXX style */
        if (*raw_string == separator) {
            /* Just skip to the repeated speech mark */
            raw_string++;
            raw_length--;
        }

        /* Encode C style */
        switch (*raw_string) {
            case '\\':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('\\');
                break;
            case '\n':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('n');
                break;
            case '\t':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('t');
                break;
            case '\a':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('a');
                break;
            case '\b':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('b');
                break;
            case '\f':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('f');
                break;
            case '\r':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('r');
                break;
            case '\v':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('v');
                break;
            case '\'':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('\'');
                break;
            case '\"':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('\"');
                break;
            case 0:
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('0');
                break;
            case '\?':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('?');
                break;
            default:
                ADD_CHAR_TO_BUFFER(*raw_string);
        }
        raw_string++;
        raw_length--;
    }

    /* Get rid of excess memory */
    processed_string = realloc(processed_string, processed_length);

    /* Fix up token */
    node->node_string = processed_string;
    node->node_string_length = processed_length;
    node->free_node_string = 1; /* So the malloced buffer is freed */
    return node;
}
#undef ADD_CHAR_TO_BUFFER

/* Set the string value of an ASTNode. string must be malloced. memory is
 * then managed by the AST Library (the caller must not free it) */
void ast_sstr(ASTNode *node, char* string, size_t length) {
    if (node->free_node_string) free(node->node_string);
    node->node_string = string;
    node->node_string_length = length;
    node->free_node_string = 1; /* So the malloced buffer is freed when cleaning up */
}

/* ASTNode Factory - With node type and string value */
ASTNode *ast_ftt(Context* context, NodeType type, char *string) {
    ASTNode *node = ast_ft(context, type);
    node->node_string = string;
    node->node_string_length = strlen(string);
    return node;
}

/* ASTNode Factory - With node type and string value copied from another node */
ASTNode *ast_fstk(Context* context, ASTNode *source_node) {
    ASTNode *node = ast_ft(context, source_node->node_type);
    node->node_string = source_node->node_string;
    node->node_string_length = source_node->node_string_length;
    return node;
}

/* ASTNode Factory - Error Node */
ASTNode *ast_err(Context* context, char *error_string, Token *token) {
    ASTNode *errorAST = ast_ftt(context, ERROR, error_string);
    add_ast(errorAST, ast_f(context, TOKEN, token));
    return errorAST;
}

/* Turn a node to an ERROR */
void mknd_err(ASTNode* node, char *error_string, ...) {
    va_list argptr;
    size_t buffer_size = 200;
    size_t needed;
    char *buffer = malloc(buffer_size);

    /* Reset Node */
    node->node_type = ERROR;
    if (node->free_node_string) {
        free(node->node_string);
        node->free_node_string = 0;
    }

    /* Write to buffer as sized */
    va_start(argptr, error_string);
    needed = vsnprintf(buffer, buffer_size, error_string, argptr);
    va_end(argptr);
    if (needed < 0) {
        /* Error - bail */
        fprintf(stderr,"Internal Error: First vsnprintf() failed in mknd_err()\n");
        exit(1);
    }
    needed++; /* Null terminator */
    buffer = realloc(buffer,needed); /* Trim or grow */

    if (needed > buffer_size) {
        /* If grow redo vsnprintf */
        va_start(argptr, error_string);
        needed = vsnprintf(buffer, needed, error_string, argptr);
        va_end(argptr);
        if (needed < 0) {
            /* Error - bail */
            fprintf(stderr,"Internal Error: Second vsnprintf() failed in mknd_err()\n");
            exit(1);
        }
    }

    node->node_string = buffer;
    node->node_string_length = needed - 1;
    node->free_node_string = 1;
}

/* Set a node string to a static value (i.e. the node isn't responsible for
 * freeing it). See also ast_sstr() */
void ast_str(ASTNode* node, char *string) {
    if (node->free_node_string) {
        free(node->node_string);
        node->free_node_string = 0;
    }
    node->node_string = string;
    node->node_string_length = strlen(string);
}

/* ASTNode Factory - Error at last Node */
ASTNode *ast_errh(Context* context, char *error_string) {
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
        case ARGS:
            return "ARGS";
        case ASSIGN:
            return "ASSIGN";
        case ASSEMBLER:
            return "ASSEMBLER";
        case BY:
            return "BY";
        case CALL:
            return "CALL";
        case CLASS:
            return "CLASS";
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
       case WHILE:
            return "WHILE";
        case UNTIL:
            return "UNTIL";
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
        case NOP:
            return "NOP";
        case NOVAL:
            return "NOVAL";
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
        case OP_NOT:
            return "OP_NOT";
        case OP_PLUS:
            return "OP_PLUS";
        case OP_NEG:
            return "OP_NEG";
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
        case VAR_REFERENCE:
            return "VAR_REFERENCE";
        case VAR_SYMBOL:
            return "VAR_SYMBOL";
        case VAR_TARGET:
            return "VAR_TARGET";
        case VOID:
            return "VOID";
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



static walker_result print_error_walker(walker_direction direction,
                                  ASTNode* node,
                                  __attribute__((unused)) void *payload) {

    int *errors = (int*)payload;

    if (direction == in) {
        if (node->node_type == ERROR) {
            /* Print error - truncate source to one line */
            int len = (int) (node->source_end - node->source_start + 1);
            int i;
            for (i=0; i<len; i++) {
                if (node->source_start[i] == '\n') {
                    len = i;
                    break;
                }
            }
            fprintf(stderr,"Error @ %d:%d - #%s, \"", node->line+1, node->column+1, node->node_string);
            prt_unex(stderr, node->source_start, len);
            fprintf(stderr,"\"\n");
            (*errors)++;
        }
    }
    return result_normal;
}

/* Prints errors and returns the number of errors in the AST Tree */
int prnterrs(Context *context) {
    int errors = 0;
    ast_wlkr(context->ast, print_error_walker, &errors);
    return errors;
}

void prnt_ast(ASTNode *node) {
    ast_wlkr(node, prnt_walker_handler, NULL);
}


/*
 * ASTNode Line and Column numbers are normally set by the set_source_location walker
 * However nodes (e.g. unspecified optional arguments) can be added after
 * the walker has been run - so we have added logic to add_ast() and add_sbtr()
 * to set these in this situation
 * */
static void fix_ast_line_number(ASTNode *node) {
    ASTNode *older = 0;
    ASTNode *n;

    /* If the node has already got a line number there is nothing needed */
    if (node->line != -1) return;

    /* If the parent has not got a line number then the walker has not been run
     * so nothing is needed */
    if (!node->parent || node->parent->line == -1) return;

    /* We need to fix up the line number etc. */
    /* If we have a token then use it */
    if (node->token) {
        node->token_start = node->token;
        node->token_end = node->token;
        node->source_start = node->token_start->token_string;
        node->source_end = node->token_end->token_string +
                           node->token_end->length - 1;
        node->line = node->token_start->line;
        node->column = node->token_start->column;
        return;
    }

    /* Older Sibling ? */
    n = node->parent->child;
    while (n != node) {
        if (!n) {
            /* Internal Error - bail */
            fprintf(stderr, "Internal Error: Node is not one of its parent's children\n");
            exit(1);
        }
        older = n;
        n = n->sibling;
    }
    if (older && older->line != -1) { /* Check If the older has valid line number (it should!) */
        node->token_start = 0;
        node->token_end = 0;
        node->source_start = older->source_end + 1;
        node->source_end = node->source_start - 1;
        node->line = older->line;
        node->column = older->column + (int)(older->source_end - older->source_start) + 1;
    }
    else {
        /* No older sibling - use the parent */
        node->token_start = 0;
        node->token_end = 0;
        node->source_start = node->parent->source_end + 1;
        node->source_end = node->source_start - 1;
        node->line = node->parent->line;
        node->column = node->parent->column + (int)(node->parent->source_end - node->parent->source_start) + 1;
    }
}

/* Add Child - Returns child for chaining */
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
    fix_ast_line_number(child);
    return child;
}

/* Add sibling - Returns younger for chaining */
ASTNode *add_sbtr(ASTNode *older, ASTNode *younger) {
    if (younger == 0 || older == 0) return younger;
    ASTNode *parent = older->parent;
    while (older->sibling) older = older->sibling;
    older->sibling = younger;
    younger->parent = parent;
    fix_ast_line_number(younger);
    return younger;
}

/* Replace replaced_node with new_node in the tree
 * note that replaced_node should not be a descendant or direct relation of
 * new_node (else we might get a loop in the tree)! */
void ast_rpl(ASTNode* replaced_node, ASTNode* new_node) {
    ASTNode *younger_sibling;

    /* Make the new node point to the right parent and younger sibling */
    new_node->sibling = replaced_node->sibling;
    new_node->parent = replaced_node->parent;

    /* Update younger sibling */
    younger_sibling = replaced_node->parent->child;
    while (younger_sibling) {
        if (younger_sibling->sibling == replaced_node) {
            younger_sibling->sibling = new_node;
            break;
        }
        younger_sibling = younger_sibling->sibling;
    }

    /* Update Parent */
    if (replaced_node->parent->child == replaced_node)
        replaced_node->parent->child = new_node;

    /* Orphan the replaced node */
    replaced_node->parent = NULL;
    replaced_node->sibling = NULL;
}

/* Delete / Remove node (i.e. the whole subtree) from the tree */
void ast_del(ASTNode* node) {
    ASTNode *younger_sibling;

    /* Update younger sibling */
    younger_sibling = node->parent->child;
    while (younger_sibling) {
        if (younger_sibling->sibling == node) {
            younger_sibling->sibling = node->sibling;
            break;
        }
        younger_sibling = younger_sibling->sibling;
    }

    /* Update Parent */
    if (node->parent->child == node)
        node->parent->child = node->sibling;

    /* Orphan the deleted node */
    node->parent = NULL;
    node->sibling = NULL;
}

void free_ast(Context *context) {
    ASTNode *t = context->free_list;
    ASTNode *n;
    while (t) {
        n = t->free_list;
        if (t->free_node_string) free(t->node_string);
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

/* Prints to dot file one symbol */
void pdot_scope(Symbol *symbol, void *payload) {
    char reg[20];
    if (symbol->register_num >= 0)
        sprintf(reg,"%c%d",symbol->register_type,symbol->register_num);
    else
        reg[0] = 0;

    if (symbol->is_function) {
        fprintf((FILE*)payload,
                "s%d_%s[style=filled fillcolor=pink shape=box label=\"%s\\n(%s)\\n%s\"]\n",
                symbol->scope->defining_node->node_number,
                symbol->name,
                symbol->name,
                type_nm(symbol->type),
                reg);
    }
    else {
        fprintf((FILE*)payload,
                "s%d_%s[style=filled fillcolor=cyan shape=box label=\"%s\\n(%s)\\n%s\"]\n",
                symbol->scope->defining_node->node_number,
                symbol->name,
                symbol->name,
                type_nm(symbol->type),
                reg);
    }
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
            case CALL:
            case ENVIRONMENT:
            case FOR:
            case WHILE:
            case UNTIL:
            case ITERATE:
            case LEAVE:
            case NOP:
            case OPTIONS:
            case PULL:
            case REPEAT:
            case RETURN:
            case SAY:
            case UPPER:
            case PARSE:
                attributes = "color=green4";
                only_type = 1;
                break;

            case ASSEMBLER:
                attributes = "color=green4";
                break;

            case FUNCTION:
            case PROCEDURE:
                attributes = "color=pink";
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
            case OP_NOT:
            case OP_PLUS:
            case OP_NEG:
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
            case NOVAL:
                attributes = "color=darkcyan";
                only_type = 1;
                break;

            case ARG:
            case ARGS:
            case PATTERN:
            case CLASS:
            case VOID:
            case REL_POS:
            case ABS_POS:
            case SIGN:
            case TARGET:
            case TEMPLATES:
                attributes = "color=green";
                break;

            case VAR_SYMBOL:
            case VAR_TARGET:
            case VAR_REFERENCE:
            case CONST_SYMBOL:
                attributes = "color=cyan3 shape=cds";
//                only_label = 1;
                break;

            case STRING:
            case INTEGER:
            case FLOAT:
            case CONSTANT:
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

        if (node->register_num >= 0) {
            if (node->num_additional_registers)
                sprintf(value_type_buffer,"\n%c%d, r%d-r%d",
                        node->register_type, node->register_num,
                        node->additional_registers,
                        node->additional_registers + node->num_additional_registers - 1);
            else
                sprintf(value_type_buffer,"\n%c%d", node->register_type, node->register_num);
        }
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

        /* Link to Associated Node */
        if (node->association) {
            fprintf(output,"n%d -> n%d [color=red dir=\"forward\"]\n",
                    node->node_number,
                    node->association->node_number);
        }

        /* Link to Symbol */
        if (node->symbol) {
            if (node->symbol->writeUsage && node->symbol->readUsage) {
                fprintf(output,"n%d -> s%d_%s [color=cyan dir=\"both\"]\n",
                        node->node_number,
                        node->symbol->symbol->scope->defining_node->node_number,
                        node->symbol->symbol->name);
            }
            else if (node->symbol->writeUsage) {
                fprintf(output,"n%d -> s%d_%s [color=cyan dir=\"forward\"]\n",
                        node->node_number,
                        node->symbol->symbol->scope->defining_node->node_number,
                        node->symbol->symbol->name);
            }
            else if (node->symbol->readUsage) {
                fprintf(output,"n%d -> s%d_%s [color=cyan dir=\"back\"]\n",
                        node->node_number,
                        node->symbol->symbol->scope->defining_node->node_number,
                        node->symbol->symbol->name);
            }
            else {
                fprintf(output,"n%d -> s%d_%s [color=cyan dir=\"none\"]\n",
                        node->node_number,
                        node->symbol->symbol->scope->defining_node->node_number,
                        node->symbol->symbol->name);
            }
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
