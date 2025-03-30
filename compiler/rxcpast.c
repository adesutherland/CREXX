#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "rxcpmain.h"
#include "rxcpbgmr.h"

static void print_error(ASTNode* node, FILE* stream, char* prefix) {
    /* Try and set error position if not already set */
    if (node->token) {
        if (node->line == -1) node->line = node->token->line;
        if (node->column == -1) node->column = node->token->column;
        if (!node->source_start) node->source_start = node->token->token_string;
        if (!node->source_end) node->source_end = node->token->token_string + node->token->length - 1;
    }
    if (node->child && node->child->token) {
        if (node->line == -1) node->line = node->child->token->line;
        if (node->column == -1) node->column = node->child->token->column;
        if (!node->source_start) node->source_start = node->child->token->token_string;
        if (!node->source_end) node->source_end = node->child->token->token_string + node->child->token->length - 1;
    }

    /* Print error - truncate source to one line */
    int len = (int) (node->source_end - node->source_start + 1);
    int i;
    for (i=0; i<len; i++) {
        if (!node->source_start || node->source_start[i] == '\n') {
            len = i;
            break;
        }
    }
    if (len) {
        fprintf(stream, "%s %s @ %d:%d - #%s, \"", prefix, node->file_name, node->line + 1,
                node->column + 1, node->node_string);
        prt_unex(stream, node->source_start, len);
        fprintf(stream, "\"\n");
    }
    else {
        fprintf(stream, "%s %s @ %d:%d - #%s\n", prefix, node->file_name, node->line + 1,
                node->column + 1, node->node_string);
    }
}

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

    if (token->token_type == TK_EOL) {
        /* EOL Special processing to get line / column number right */
        token->length = context->cursor - context->top;
        token->line = context->line - 1;
        token->column = context->top - context->prev_linestart + 1;
    }
    else {
        token->length = context->cursor - context->top;
        token->line = context->line;
        token->column = context->top - context->linestart + 1;
    }
    if (token->column < 0) token->column = 0;
    token->token_string = context->top;
    context->top = context->cursor;

    return token;
}

/* Split a token - returns the first token (token->token_next) points to the next twin; */
/* the first token has len characters, the second twin as the remaining characters.       */
/* The caller can then change the tokens' types as needed.                              */
Token *tok_splt(Context *context, Token *token, int len) {
    int n;
    Token *t;

    /* Copy token to make twin */
    Token *twin = malloc(sizeof(Token));
    memcpy(twin, token, sizeof(Token));

    /* Fix up linked list - the twin comes before token */
    twin->token_next = token;
    token->token_prev = twin;
    if (twin->token_prev) twin->token_prev->token_next = twin;
    else context->token_head = twin;

    /* Fix up token numbers */
    n = twin->token_number;
    t = twin->token_next;
    while (t) {
        t->token_number = ++n;
        t = t->token_next;
    }
    context->token_counter = n;

    /* Fix up token lengths / pos / string */
    twin->length = len;
    token->length -= len;
    token->column += len;
    token->token_string += len;

    return twin;
}

/* Remove the last (tail) token */
void token_r(Context *context) {
    Token *tail = context->token_tail;
    Token *new_tail;

    /* Unlink the tail token */
    if (tail) {
        new_tail = tail->token_prev;
        if (new_tail) {
            new_tail->token_next = 0;
            context->token_tail = new_tail;
        } else {
            context->token_head = 0;
            context->token_tail = 0;
        }
        free(tail);
        context->token_counter--;
    }
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
    context->token_head = 0;
    context->token_tail = 0;
    context->token_counter = 0;
}

/* ASTNode Factory - With node type*/
ASTNode *ast_ft(Context* context, NodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->context = context;
    node->file_name = context->file_name;
    node->parent = 0;
    node->child = 0;
    node->sibling = 0;
    node->association = 0;
    node->token = 0;
    node->symbolNode = 0;
    node->scope = 0;
    node->output = 0;
    node->cleanup = 0;
    node->loopstartchecks = 0;
    node->loopinc = 0;
    node->loopendchecks = 0;
    node->node_type = type;
    node->value_type = TP_UNKNOWN;
    node->value_dims = 0;
    node->value_dim_base = 0;
    node->value_dim_elements = 0;
    node->value_class = 0;
    node->target_type = TP_UNKNOWN;
    node->target_dims = 0;
    node->target_dim_base = 0;
    node->target_dim_elements = 0;
    node->target_class = 0;
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
    node->is_const_arg = 0;
    node->is_opt_arg = 0;
    node->is_varg = 0;
    node->free_list = context->free_list;
    if (node->free_list) node->node_number = node->free_list->node_number + 1;
    else node->node_number = 1;
    context->free_list = node;

    /*  Note that ordinal is only set before optimisation - new nodes have value -1 */
    node->high_ordinal = -1;
    node->low_ordinal = -1;

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

/* Factory to create a duplicated AST node into a new context
 * - context is the target context
 * - node is the node to be duplicated
 *
 * A number of aspects are NOT copied
 * - Symbols
 * - Scope
 * - Associated Nodes
 * - Tree position (child, sibling, parent)
 * - Emitter output fragments
 * - Ordinals
 */
ASTNode *ast_dup(Context* new_context, ASTNode *node) {
    ASTNode *new_node = malloc(sizeof(ASTNode));
    new_node->context = new_context;
    new_node->token = node->token;
    new_node->node_type = node->node_type;
    new_node->value_type = node->value_type;
    new_node->value_dims = node->value_dims;
    if (new_node->value_dims) {
        new_node->value_dim_base = malloc(sizeof(int) * new_node->value_dims);
        memcpy(new_node->value_dim_base, node->value_dim_base, sizeof(int) * new_node->value_dims);
        new_node->value_dim_elements = malloc(sizeof(int) * new_node->value_dims);
        memcpy(new_node->value_dim_elements, node->value_dim_elements, sizeof(int) * new_node->value_dims);
    }
    else {
        new_node->value_dim_base = 0;
        new_node->value_dim_elements = 0;
    }
    if (node->value_class) {
        new_node->value_class = malloc(strlen(node->value_class) + 1);
        strcpy(new_node->value_class,node->value_class);
    } else new_node->value_class = 0;
    new_node->target_type = node->target_type;
    new_node->target_dims = node->target_dims;
    if (new_node->target_dims) {
        new_node->target_dim_base = malloc(sizeof(int) * new_node-> target_dims);
        memcpy(new_node->target_dim_base, node->target_dim_base, sizeof(int) * new_node->target_dims);
        new_node->target_dim_elements = malloc(sizeof(int) * new_node->target_dims);
        memcpy(new_node->target_dim_elements, node->target_dim_elements, sizeof(int) * new_node->target_dims);
    }
    else {
        new_node->target_dim_base = 0;
        new_node->target_dim_elements = 0;
    }
    if (node->target_class) {
        new_node->target_class = malloc(strlen(node->target_class) + 1);
        strcpy(new_node->target_class,node->target_class);
    } else new_node->target_class = 0;
    new_node->file_name = node->file_name;
    new_node->int_value = node->int_value;
    new_node->bool_value = node->bool_value;
    new_node->float_value = node->float_value;
    new_node->register_num = node->register_num;
    new_node->register_type = node->register_type;
    new_node->additional_registers = node->additional_registers;
    new_node->num_additional_registers = node->num_additional_registers;
    new_node->is_ref_arg = node->is_ref_arg;
    new_node->is_const_arg = node->is_const_arg;
    new_node->is_opt_arg = node->is_opt_arg;
    new_node->token_start = node->token_start;
    new_node->token_end = node->token_end;
    new_node->source_start = node->source_start;
    new_node->source_end = node->source_end;
    new_node->line = node->line;
    new_node->column = node->column;

    /* Node String - need to malloc or just point to existing buffer */
    new_node->free_node_string = node->free_node_string;
    new_node->node_string_length = node->node_string_length;
    if (node->free_node_string) {
        new_node->node_string = malloc(new_node->node_string_length);
        memcpy(new_node->node_string, node->node_string, new_node->node_string_length);
    }
    else {
        new_node->node_string = node->node_string;
    }

    /* Scope / Symbol not copied */
    new_node->scope = 0;
    new_node->symbolNode = 0;

    /* Association not copied */
    new_node->association = 0;

    /* Position in new tree not defined / copied */
    new_node->parent = 0;
    new_node->child = 0;
    new_node->sibling = 0;

    /* These are only set when emitting - not copied */
    new_node->output = 0;
    new_node->cleanup = 0;
    new_node->loopstartchecks = 0;
    new_node->loopinc = 0;
    new_node->loopendchecks = 0;

    /*  Note that ordinal is only set just before optimisation - new nodes have value -1 */
    new_node->high_ordinal = -1;
    new_node->low_ordinal = -1;

    /* Add new node to context free list */
    new_node->free_list = new_context->free_list;
    if (new_node->free_list) new_node->node_number = new_node->free_list->node_number + 1;
    else new_node->node_number = 1;
    new_context->free_list = new_node;

    return new_node;
}

/* Structure for add_dast() walker handler context */
struct add_dast_context {
    ASTNode *source;
    ASTNode *dest;
    ASTNode *insert_point;
};

walker_result add_dast_walker_handler1(walker_direction direction,
                                  ASTNode* node,
                                  void *payload) {
    ASTNode* new_node;
    struct add_dast_context *context = (struct add_dast_context *)payload;
    char *fqname;
    Symbol *new_symbol;
    Symbol *symbol;

    if (direction == in) {
        /* Top Down */
        new_node = ast_dup(context->dest->context, node);
        if (node == context->source) {
            /* Top node */
            add_ast(context->dest, new_node);
            context->insert_point = new_node;
        }
        else {
            add_ast(context->insert_point, new_node);
            context->insert_point = new_node;
        }

        /* Duplicate Scope */
        if (node->scope) {
            fqname = scp_frnm(node->scope);
            new_symbol = sym_rfqn(context->dest, fqname);
            if (new_symbol) {
                if (!new_symbol->defines_scope) {
                    fprintf(stderr, "INTERNAL ERROR: Duplicating AST - Found Symbol is a not a Scope\n");
                    exit(100);
                }
                new_node->scope = new_symbol->defines_scope;
            }
            else {
                /* Need to make a new symbol */
                new_symbol = sym_afqn(context->dest, fqname);
                new_node->scope = scp_f(new_node->parent->scope, new_node, new_symbol);
                new_symbol->symbol_type = NAMESPACE_SYMBOL;
                new_symbol->defines_scope = new_node->scope;
                //new_symbol->scope->defining_node
            }
            free(fqname);
        }

        /* Duplicate Linked Symbol */
        if (node->symbolNode) {
            symbol = node->symbolNode->symbol;
            fqname = sym_frnm(symbol);

            new_symbol = sym_rfqn(context->dest, fqname);
            if (!new_symbol) {
                new_symbol = sym_afqn(context->dest, fqname);
            }
            new_symbol->symbol_type = symbol->symbol_type;
            new_symbol->type = symbol->type;
            new_symbol->exposed = symbol->exposed;
            new_symbol->fixed_args = symbol->fixed_args;
            new_symbol->has_vargs = symbol->has_vargs;
            new_symbol->is_arg = symbol->is_arg;
            new_symbol->is_ref_arg = symbol->is_ref_arg;
            new_symbol->is_const_arg = symbol->is_const_arg;
            new_symbol->is_opt_arg = symbol->is_opt_arg;

            sym_adnd(new_symbol, new_node, node->symbolNode->readUsage, node->symbolNode->writeUsage);
            free(fqname);
        }
    }
    else {
        /* Bottom Up */
        context->insert_point = context->insert_point->parent;
    }

    return result_normal;
}

/* Add a duplicate of the tree headed by the source node as a child to dest
 * This handles the nodes, scopes and symbols, and associated nodes
 * Note that symbols and associated nodes out of the scope of the original child tree are removed */
/* TODO Associated nodes */
ASTNode *add_dast(ASTNode *dest, ASTNode *source) {
    struct add_dast_context context;
    ASTNode *node;

    context.dest = dest;
    context.source = source;

    ast_wlkr(source, add_dast_walker_handler1, &context);

    /* Return the added child (the last sibling) */
    node = context.dest;
    while (node->child) node = node->child;
    return node;
}

/* Convert one hex digit to an int (-1 = error)*/
static int hexchar2int(char hexbyte) {
    int val = -1;

    // transform hex character to the 4bit equivalent number
    if (hexbyte >= '0' && hexbyte <= '9') val = hexbyte - '0';
    else if (hexbyte >= 'a' && hexbyte <='f') val = hexbyte - 'a' + 10;
    else if (hexbyte >= 'A' && hexbyte <='F') val = hexbyte - 'A' + 10;

    return val;
}

/* Helper Function to turn 4 binary bits to an int (hex) character */
static int binchar2int(const char* bin) {
    int result = 0;

    if (bin[0] == '1') result += 8;
    if (bin[1] == '1') result += 4;
    if (bin[2] == '1') result += 2;
    if (bin[3] == '1') result++;

    return result;
}

/*
 * Escape a character
 * input - character to be escaped
 * Returns a static buffer with the escaped character string
 */
static char* escape_character(unsigned char c) {
    static char buffer[5];
    buffer[0] = '\\';
    buffer[2] = 0;
    /* Encode C style */
    switch (c) {
        case '\\':
            buffer[1] = '\\';
            break;
        case '\n':
            buffer[1] = 'n';
            break;
        case '\t':
            buffer[1] = 't';
            break;
        case '\a':
            buffer[1] = 'a';
            break;
        case '\b':
            buffer[1] = 'b';
            break;
        case '\f':
            buffer[1] = 'f';
            break;
        case '\r':
            buffer[1] = 'r';
            break;
        case '\v':
            buffer[1] = 'v';
            break;
        case '\'':
            buffer[1] = '\'';
            break;
        case '\"':
            buffer[1] = '\"';
            break;
        case 0:
            buffer[1] = '0';
            break;
        case '\?':
            buffer[1] = '\?';
            break;
        default:
            /* Should we escape this character? */
            if (isprint(c)) {
                buffer[0] = (char)c;
                buffer[1] = 0;
            } else {
                /* Escape as a hex character */
                buffer[1] = 'x';
                snprintf(buffer + 2, 3, "%02x", c);
            }
            break;
    }
    return buffer;
}

/* ASTNode Factory - adds a STRING token removing the leading & trailing speech marks
 * and decoding / encoding the string nicely - or converting to an error string */
#define ADD_CHAR_TO_BUFFER(ch) { processed_length++; *(buffer++) = (ch); }
ASTNode *ast_fstr(Context* context, Token *token) {
    unsigned char separator;
    char* raw_string;
    size_t raw_length;
    size_t hex_bin_length;
    char *processed_string;
    char *buffer;
    char c;
    size_t i;
    size_t processed_length;
    char hex_bin_buffer[9];
    size_t hex_buffer_len;
    char string_type;
    char *escaped_char;

    /* Make the token */
    ASTNode *node = ast_ft(context, STRING);
    node->token = token;

    /* Prepare for processing string */
    separator = token->token_string[0];
    raw_string = token->token_string + 1;
    if (token->token_string[token->length - 1] != separator) {
        if ( token->token_string[token->length - 1] == 'x' ||
             token->token_string[token->length - 1] == 'X') string_type = 'x'; /* Hex */
        else string_type = 'b'; /* Binary */
        raw_length = token->length - 3; /* I.e. There "must" be an X or B suffix */
    }
    else {
        string_type = 'n'; /* Normal */
        raw_length = token->length - 2;
    }

    if (!raw_length) {
        /* Zero Length String */
        node->node_string = 0;
        node->node_string_length = 0;
        node->free_node_string = 0;
        return node;
    }

    /* Code String - basically RexxAssembler uses C type escapes */
    if (string_type == 'n') { /* Normal String */
        processed_length = 0;
        processed_string = malloc(raw_length * 4); /* Worse case */
        buffer = processed_string;

        while (raw_length) {
            /* Decode REXX style */
            if (*raw_string == separator) {
                /* Just skip to the repeated speech mark */
                raw_string++;
                raw_length--;
            }

            /* Encode C style */
            escaped_char = escape_character(*raw_string);
            while (*escaped_char) ADD_CHAR_TO_BUFFER(*(escaped_char++));
            raw_string++;
            raw_length--;
        }
    }

    else if (string_type == 'x') { /* Hex String */
        /* Validate hex string and work out length */
        for (hex_bin_length = 0, i = 0; i < raw_length; i++) {
            c = raw_string[i];
            if (c == ' ') continue;
            if ( !( (c >= '0' && c <= '9') ||
                    (c >= 'a' && c <= 'f') ||
                    (c >= 'A' && c <= 'F') ) ) {
                mknd_err(node, "INVALID_HEX");
                return node;
            }
            hex_bin_length++;
        }

        processed_length = 0;
        processed_string = malloc(raw_length * 4); /* Worse case */
        buffer = processed_string;

        /* Output the hex string */
        if (hex_bin_length % 2) { /* Odd number of digits - add a leading zero */
            hex_buffer_len = 1;
            hex_bin_buffer[0] = '0';
        }
        else hex_buffer_len = 0;

        while (raw_length) {
            if (*raw_string != ' ') {
                hex_bin_buffer[hex_buffer_len] = (char)tolower(*raw_string);
                hex_buffer_len++;
                if (hex_buffer_len == 2) {

                    escaped_char = escape_character(
                            (hexchar2int(hex_bin_buffer[0]) * 16) +
                            hexchar2int(hex_bin_buffer[1]));

                    while (*escaped_char) ADD_CHAR_TO_BUFFER(*(escaped_char++));
                    hex_buffer_len = 0;
                }
            }
            raw_string++;
            raw_length--;
        }
    }

    else  { /* Binary String */
        /* Validate binary string and work out length */
        for (hex_bin_length = 0, i = 0; i < raw_length; i++) {
            c = raw_string[i];
            if (c == ' ') continue;
            if (c != '0' && c != '1') {
                mknd_err(node, "INVALID_BIN");
                return node;
            }
            hex_bin_length++;
        }

        processed_length = 0;
        processed_string = malloc(raw_length * 4); /* Worse case */
        buffer = processed_string;

        /* Output the bin string */
        /* Add leaving '0's */
        hex_buffer_len = (hex_bin_length % 8) ? 8 - (int)(hex_bin_length % 8) : 0;
        for (i=0;i<hex_buffer_len;i++) hex_bin_buffer[i]='0';

        while (raw_length) {
            if (*raw_string != ' ') {
                hex_bin_buffer[hex_buffer_len] = (char)tolower(*raw_string);
                hex_buffer_len++;
                if (hex_buffer_len == 8) {
                    escaped_char = escape_character(
                            (binchar2int(hex_bin_buffer) * 16) +
                            binchar2int(hex_bin_buffer + 4));

                    while (*escaped_char) ADD_CHAR_TO_BUFFER(*(escaped_char++));
                    hex_buffer_len = 0;
                }
            }
            raw_string++;
            raw_length--;
        }
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
ASTNode * ast_ftt(Context* context, NodeType type, char *string) {
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

/* Add error node to parent node */
ASTNode *ast_err(Context* context, char *error_string, Token *token) {
    ASTNode *newNode = ast_f(context, TOKEN, token);
    mknd_err(newNode, error_string);
    return newNode;
}

/* Add warning node to parent node */
ASTNode *ast_war(Context* context, char *warning_string, Token *token) {
    ASTNode *newNode = ast_f(context, TOKEN, token);
    mknd_war(newNode, warning_string);
    return newNode;
}

/* Add an ERROR node to a node - returns node for chaining */
ASTNode* mknd_err(ASTNode* node, char *error_string, ...) {
    va_list argptr;
    size_t buffer_size = 200;
    size_t needed;
    ASTNode *errNode;
    char *buffer = malloc(buffer_size);

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

    /* Child node */
    errNode = ast_ftt(node->context, ERROR, buffer);
    add_ast(node,  errNode);
    errNode->token = node->token;
    errNode->free_node_string = 1;
    errNode->source_start = node->source_start;
    errNode->source_end = node->source_end;
    errNode->line = node->line;
    errNode->column = node->column;
    if (node->context->debug_mode) print_error(errNode, stdout, "DEBUG: Error in");

    return node;
}

/* Add a warning child node - returns node for chaining */
ASTNode* mknd_war(ASTNode* node, char *error_string, ...) {
    va_list argptr;
    size_t buffer_size = 200;
    size_t needed;
    ASTNode *warNode;
    char *buffer = malloc(buffer_size);

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

    /* Child node */
    warNode = ast_ftt(node->context, WARNING, buffer);
    add_ast(node,  warNode);
    warNode->token = node->token;
    warNode->free_node_string = 1;
    warNode->source_start = node->source_start;
    warNode->source_end = node->source_end;
    warNode->line = node->line;
    warNode->column = node->column;
    if (node->context->debug_mode) print_error(warNode, stdout, "DEBUG: Error in");

    return node;
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
    add_ast(errorAST, ast_f(context, TOKEN, context->token_tail->token_prev->token_prev));

    if (context->debug_mode) print_error(errorAST, stdout, "DEBUG: Error in");
    return errorAST;
}

/* Returns the number of children of a node */
size_t ast_nchd(ASTNode* node) {
    size_t n = 0;
    ASTNode* c;

    if (!node) return 0;

    c = node->child;
    while (c) {
        if (c->node_type != ERROR && c->node_type != WARNING) n++;
        c = c->sibling;
    }

    return n;
}

/* Returns the PROCEDURE ASTNode procedure of an AST node */
ASTNode* ast_proc(ASTNode *node) {
    while (node) {
        if (node->node_type == PROCEDURE) return node;
        node = node->parent;
    }
    return 0;
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
        case DEFINE:
            return "DEFINE";
        case DO:
            return "DO";
        case ENVIRONMENT:
            return "ENVIRONMENT";
        case ERROR:
            return "ERROR";
        case EXPOSED:
            return "EXPOSED";
        case FOR:
            return "FOR";
        case WARNING:
            return "WARNING";
        case WHILE:
            return "WHILE";
        case UNTIL:
            return "UNTIL";
        case FUNCTION:
            return "FUNCTION";
        case FUNC_SYMBOL:
            return "FUNC_SYMBOL";
        case IF:
            return "IF";
        case IMPORT:
            return "IMPORT";
        case IMPORTED_FILE:
            return "IMPORTED_FILE";
        case INSTRUCTIONS:
            return "INSTRUCTIONS";
        case ITERATE:
            return "ITERATE";
        case LABEL:
            return "LABEL";
        case LEAVE:
            return "LEAVE";
        case LITERAL:
            return "LITERAL";
        case FLOAT:
            return "FLOAT";
        case INTEGER:
            return "INTEGER";
        case OP_MAKE_ARRAY:
            return "OP_MAKE_ARRAY";
        case NAMESPACE:
            return "NAMESPACE";
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
        case OP_ARGS:
            return "OP_ARGS";
        case OP_ARG_VALUE:
            return "OP_ARG_VALUE";
        case OP_ARG_EXISTS:
            return "OP_ARG_EXISTS";
        case OP_ARG_IX_EXISTS:
            return "OP_ARG_IX_EXISTS";
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
        case RANGE:
            return "RANGE";
        case REL_POS:
            return "REL_POS";
        case REPEAT:
            return "REPEAT";
        case REDIRECT_IN:
            return "REDIRECT_IN";
        case REDIRECT_OUT:
            return "REDIRECT_OUT";
        case REDIRECT_ERROR:
            return "REDIRECT_ERROR";
        case REDIRECT_EXPOSE:
            return "REDIRECT_EXPOSE";
        case RETURN:
            return "RETURN";
        case EXIT:
            return "EXIT";
        case REXX_OPTIONS:
            return "REXX_OPTIONS";
        case REXX_UNIVERSE:
            return "REXX_UNIVERSE";
        case SAY:
            return "SAY";
        case SIGN:
            return "SIGN";
        case STRING:
            return "STRING";
        case BINARY:
            return "BINARY";
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
        case VARG:
            return "VARG";
        case VARG_REFERENCE:
            return "VARG_REFERENCE";
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
            print_error(node, stderr, "Error in");
            (*errors)++;
        }
    }
    return result_normal;
}

static walker_result print_warning_walker(walker_direction direction,
                                        ASTNode* node,
                                        __attribute__((unused)) void *payload) {

    int *errors = (int*)payload;

    if (direction == in) {
        if (node->node_type == WARNING) {
            print_error(node, stderr, "Warning in");
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

/* Prints errors and returns the number of errors in the AST Tree */
int prntwars(Context *context) {
    int errors = 0;
    ast_wlkr(context->ast, print_warning_walker, &errors);
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
        node->source_end = node->source_start ? (node->source_start - 1) : 0;
        node->line = older->line;
        node->column = older->column + (int)(older->source_end - older->source_start) + 1;
    }
    else {
        /* No older sibling - use the parent */
        node->token_start = 0;
        node->token_end = 0;
        node->source_start = node->parent->source_end + 1;
        node->source_end = node->source_start ? (node->source_start - 1) : 0;
        node->line = node->parent->line;
        node->column = node->parent->column + (int)(node->parent->source_end - node->parent->source_start) + 1;
    }
}

/* Add Child - Returns child for chaining
 * Note - This assumes the child has only got younger siblings and moved them as well
 *        older siblings are left hanging ... */
ASTNode *add_ast(ASTNode *parent, ASTNode *child) {
    if (child == 0) return child;

    /* Adds as the youngest sibling */
    ASTNode *s = parent->child;
    if (s) {
        while (s->sibling) s = s->sibling;
        s->sibling = child;
    } else parent->child = child;

    /* Sets the parent of the child and any younger siblings of the child */
    s = child;
    while (s) {
        s->parent = parent;
        s = s->sibling;
    }

    /* Fix line numbers */
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
        if (t->value_dim_base) free(t->value_dim_base);
        if (t->value_dim_elements) free(t->value_dim_elements);
        if (t->value_class) free(t->value_class);
        if (t->target_dim_base) free(t->target_dim_base);
        if (t->target_dim_elements) free(t->target_dim_elements);
        if (t->target_class) free(t->target_class);
        if (t->output) f_output(t->output);
        if (t->cleanup) f_output(t->cleanup);
        if (t->loopstartchecks) f_output(t->loopstartchecks);
        if (t->loopinc) f_output(t->loopinc);
        if (t->loopendchecks) f_output(t->loopendchecks);
        free(t);
        t = n;
    }
    context->ast = 0;
    context->free_list = 0;
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
                /* Should we escape this character? */
                if (isprint(*ptr)) {
                    fprintf(output, "%c", *ptr);
                }
                else {
                    /* Escape as a hex character */
                    fprintf(output, "\\x%02x", *ptr);
                }
        }
    }
}

/* Returns a malloced string of the array part of a symbols/type
 * (it returns a null terminated string if there is no array part - still needs a free() */
char *ast_astr(size_t dims, int* base, int* num_elements) {
    char* result;
    int i, c, x;
    if (!dims) {
        result = malloc(1);
        result[0] = 0;
        return result;
    }
    /* Each dim could be "xxxx..xx to xxxx..xx," (each number upto 11 chars) = 11+11+5 = 27 chars plus 3 for the [, ], and terminator */
    result = malloc((dims * 27) + 3);

    result[0] = '[';
    c = 1;
    for (i=0; i<dims; i++) {

        if (base[i] == 1 && num_elements[i] == 0) {
            result[c++] = '*';
        }
        else if (base[i] == 1) {
            c += sprintf(result + c, "%d", num_elements[i]);
        }
        else if (num_elements[i] == 0) {
            c += sprintf(result + c, "%d to *", base[i]);
        }
        else {
            c += sprintf(result + c, "%d to %d", base[i], base[i] + num_elements[i] - 1);
        }

        if (i + 1 != dims) {
            result[c++] = ',';
        }
    }
    result[c++] = ']';
    result[c++] = 0;

    return result;
}

/* Returns the type of a node as a text string in a malloced buffer */
char* ast_n2tp(ASTNode *node) {
    char *buffer = 0;
    char *array;
    char *result;
    ValueType type = node->value_type;

    if (type == TP_OBJECT) {
        buffer = ast_nsrc(node);
        if (buffer[0] == 0) {  /* I.e. an empty line */
            free(buffer); /* set to .OBJECT below */
            buffer = 0;
        }
    }
    if (!buffer) {
        buffer = malloc(sizeof(".BOOLEAN") + 1); /* Make it long enough for the longest option */
        switch (type) {
            case TP_BOOLEAN:
                strcpy(buffer, ".boolean");
                break;
            case TP_INTEGER:
                strcpy(buffer, ".int");
                break;
            case TP_FLOAT:
                strcpy(buffer, ".float");
                break;
            case TP_STRING:
                strcpy(buffer, ".string");
                break;
            case TP_BINARY:
                strcpy(buffer, ".binary");
                break;
            case TP_OBJECT:
                strcpy(buffer, ".object");
                break;
            case TP_VOID:
                strcpy(buffer, ".void");
                break;
            default:
                strcpy(buffer, ".unknown");
        }
    }

    array = ast_astr(node->value_dims, node->value_dim_base, node->value_dim_elements);

    result = malloc(strlen(buffer) + strlen(array) + 1);
    strcpy(result, buffer);
    strcat(result, array);
    free(buffer);
    free(array);

    return result;
}

/* Returns the source code of a node in a malloced buffer with formatting removed / cleaned */
char *ast_nsrc(ASTNode *node) {
    ASTNode *n;
    Token *t;
    size_t buffer_len;
    char *buffer, *b;
    size_t i;

    /* Calculate required buffer length */
    buffer_len = 0;
    for  (t = node->token_start; t; t = t->token_next) {
        buffer_len += t->length + 1; /* +1 for space */
        if (t == node->token_end) break;
    }

    /* Empty Source Line */
    if (!buffer_len) {
        buffer = malloc(1);
        buffer[0] = 0;
        return buffer;
    }

    /* Create and write to buffer */
    b = buffer = malloc(buffer_len);
    for  (t = node->token_start; t; t = t->token_next) {
        if (t->token_type != TK_STRING)  {
            /* Lower case it */
            for (i = 0; i < t->length; i++) {
                *(b++) = (char)tolower(t->token_string[i]);
            }
        }
        else {
            memcpy(b, t->token_string, t->length);
            b += t->length;
        }
        *(b++) = ' '; /* Add Space */
        if (t == node->token_end) break;
    }

    /* Turn the last space to a terminating null */
    *(--b) = 0;

    return buffer;
}

/* Prints to dot file one symbol */
void pdot_scope(Symbol *symbol, void *payload) {
    char reg[20];
    char *arr;
    char *name;
    char *clas;

    if (symbol->register_num >= 0)
        sprintf(reg,"%c%d",symbol->register_type,symbol->register_num);
    else
        reg[0] = 0;

    name = sym_frnm(symbol);

    arr = ast_astr(symbol->value_dims, symbol->dim_base, symbol->dim_elements);

    if (symbol->value_class)
        clas = symbol->value_class;
    else
        clas = type_nm(symbol->type);

    switch (symbol->symbol_type) {
        case CLASS_SYMBOL:
        case NAMESPACE_SYMBOL:
            fprintf((FILE *) payload,
                    "\"s%p%d_%s\"[style=filled fillcolor=green shape=box label=\"%s\"]\n",
                    (void*)symbol->scope->defining_node->context,
                    symbol->scope->defining_node->node_number,
                    symbol->name,
                    name);
            break;

        case FUNCTION_SYMBOL:
            fprintf((FILE *) payload,
                    "\"s%p%d_%s\"[style=filled fillcolor=pink shape=box label=\"%s\\n(%s%s)\\n%s\"]\n",
                    (void*)symbol->scope->defining_node->context,
                    symbol->scope->defining_node->node_number,
                    symbol->name,
                    name,
                    clas, arr,
                    reg);
            break;
        default:
            fprintf((FILE *) payload,
                    "\"s%p%d_%s\"[style=filled fillcolor=cyan shape=box label=\"%s\\n(%s%s)\\n%s\"]\n",
                    (void*)symbol->scope->defining_node->context,
                    symbol->scope->defining_node->node_number,
                    symbol->name,
                    name,
                    clas, arr,
                    reg);
    }
    free(arr);
    free(name);
}

/* Get the child node of a certain type1 or type2 (or null) */
ASTNode * ast_chld(ASTNode *parent, NodeType type1, NodeType type2) {
    ASTNode *n = parent->child;
    while (n) {
        if (n->node_type == type1) return n;
        if (type2 && n->node_type == type2) return n;
        n = n->sibling;
    }
    return 0;
}

/* Returns 1 if the node is an error or warning node, or has any descendant error or warning node */
int ast_hase(ASTNode *node) {
    if (node->node_type == ERROR || node->node_type == WARNING) return 1;

    ASTNode *n = node->child;
    while (n) {
        if (ast_hase(n)) return 1;
        n = n->sibling;
    }
    return 0;
}

/* Prune all nodes except ERRORs and WARNINGs */
void ast_prun(ASTNode *node) {
    ASTNode *n = node->child;
    ASTNode *next;
    while (n) {
        next = n->sibling;
        if (ast_hase(n)) ast_prun(n); /* If it has error nodes just prune it (recursive) */
        else ast_del(n); /* Else delete it */
        n = next;
    }
    if (!node->child && node->node_type != ERROR && node->node_type != WARNING) ast_del(node);
}

/* Prune all children nodes except ERRORs and WARNINGs */
void ast_prnc(ASTNode *node) {
    ASTNode *n = node->child;
    ASTNode *next;
    while (n) {
        next = n->sibling;
        if (ast_hase(n)) ast_prun(n); /* If it has error nodes just prune it (recursive) */
        else ast_del(n); /* Else delete it */
        n = next;
    }
}

walker_result pdot_walker_handler(walker_direction direction,
                                  ASTNode* node, void *payload) {
    FILE* output = (FILE*)payload;

    char *attributes;
    int only_type = 0;
    int only_label = 0;
    int child_index;
    ASTNode *first_node;
    char value_type_buffer[200];
    char* varr;
    char* vclas;
    char* tarr;
    char* tclas;

    if (direction == in) {
        /* IN - TOP DOWN */

        child_index = ast_chdi(node);

        /* Scope == DOT Subgraph */
        if (!node->parent || node->scope != node->parent->scope) {
            if (node->scope) fprintf(output, "subgraph scope_%p{\n", (void*)node->scope);
        }

        /* Attributes */
        switch (node->node_type) {

            /* Groupings */
            case REXX_UNIVERSE:
            case PROGRAM_FILE:
            case IMPORTED_FILE:
            case INSTRUCTIONS:
            case DO:
            case BY:
            case IF:
            case REXX_OPTIONS:
            case IMPORT:
            case NAMESPACE:
            case EXPOSED:
            case TO:
                attributes = "color=blue";
                only_type = 1;
                break;

            case ASSIGN:
            case CALL:
            case DEFINE:
            case ENVIRONMENT:
            case FOR:
            case WHILE:
            case UNTIL:
            case ITERATE:
            case LEAVE:
            case NOP:
            case OPTIONS:
            case PULL:
            case RANGE:
            case REPEAT:
            case REDIRECT_IN:
            case REDIRECT_OUT:
            case REDIRECT_ERROR:
            case REDIRECT_EXPOSE:
            case RETURN:
            case EXIT:
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
            case FUNC_SYMBOL:
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
            case OP_ARGS:
            case OP_ARG_VALUE:
            case OP_ARG_EXISTS:
            case OP_ARG_IX_EXISTS:
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
            case OP_MAKE_ARRAY:
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
            case VARG_REFERENCE:
            case VARG:
            case CONST_SYMBOL:
            case LITERAL:
                attributes = "color=cyan3 shape=cds";
//                only_label = 1;
                break;

            case STRING:
            case BINARY:
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

        varr = ast_astr(node->value_dims, node->value_dim_base, node->value_dim_elements);
        tarr = ast_astr(node->target_dims, node->target_dim_base, node->target_dim_elements);

        if (node->value_class) vclas = node->value_class;
        else vclas = type_nm(node->value_type);

        if (node->target_class) tclas = node->target_class;
        else if (node->target_type == TP_OBJECT && node->value_class) tclas = node->value_class;
        else tclas = type_nm(node->target_type);

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

        if (node->value_type != TP_UNKNOWN || node->value_dims ||
            node->target_type != TP_UNKNOWN || node->target_dims) {
            if (node->value_type == node->target_type && strcmp(vclas,tclas) == 0 && strcmp(varr,tarr) == 0) {
                strcat(value_type_buffer, "\n(");
                strcat(value_type_buffer, vclas);
                strcat(value_type_buffer, varr);
                strcat(value_type_buffer, ")");
            } else {
                strcat(value_type_buffer, "\n(");
                strcat(value_type_buffer, vclas);
                strcat(value_type_buffer, varr);
                strcat(value_type_buffer, "->");
                strcat(value_type_buffer, tclas);
                strcat(value_type_buffer, tarr);
                strcat(value_type_buffer, ")");
            }
        }

        free(varr);
        free(tarr);

        if (only_type) {
            fprintf(output, "n%p%d[ordering=\"out\" label=\"%s%s", (void*)node->context,node->node_number,
                    ast_ndtp(node->node_type), value_type_buffer);
        } else if (only_label) {
            fprintf(output, "n%p%d[ordering=\"out\" label=\"", (void*)node->context,node->node_number);
            prt_unex(output, node->node_string,
                     (int) node->node_string_length);
            fprintf(output, "%s", value_type_buffer);
        } else {
            fprintf(output, "n%p%d[ordering=\"out\" label=\"%s\\n", (void*)node->context,node->node_number,
                    ast_ndtp(node->node_type));
            prt_unex(output, node->node_string,
                     (int) node->node_string_length);
            fprintf(output, "%s", value_type_buffer);
        }
        fprintf(output, "\" %s]\n", attributes);

        /* Link to Parent */
        if (node->parent) {
            fprintf(output,"n%p%d -> n%p%d [xlabel=\"%d\"]\n",
                    (void*)node->parent->context,  node->parent->node_number,
                    (void*)node->context,node->node_number, child_index);
        }

        /* Link to Associated Node */
        if (node->association) {
            fprintf(output,"n%p%d -> n%p%d [color=red dir=\"forward\"]\n",
                    (void*)node->context,node->node_number,
                    (void*)node->association->context,node->association->node_number);
        }

        /* Link to Symbol */
        if (node->symbolNode) {
            if (node->symbolNode->writeUsage && node->symbolNode->readUsage) {
                fprintf(output,"n%p%d -> \"s%p%d_%s\" [color=cyan dir=\"both\"]\n",
                        (void*)node->context,node->node_number,
                        (void*)node->symbolNode->symbol->scope->defining_node->context,node->symbolNode->symbol->scope->defining_node->node_number,
                        node->symbolNode->symbol->name);
            }
            else if (node->symbolNode->writeUsage) {
                fprintf(output,"n%p%d -> \"s%p%d_%s\" [color=cyan dir=\"forward\"]\n",
                        (void*)node->context,node->node_number,
                        (void*)node->symbolNode->symbol->scope->defining_node->context,node->symbolNode->symbol->scope->defining_node->node_number,
                        node->symbolNode->symbol->name);
            }
            else if (node->symbolNode->readUsage) {
                fprintf(output,"n%p%d -> \"s%p%d_%s\" [color=cyan dir=\"back\"]\n",
                        (void*)node->context,node->node_number,
                        (void*)node->symbolNode->symbol->scope->defining_node->context,node->symbolNode->symbol->scope->defining_node->node_number,
                        node->symbolNode->symbol->name);
            }
            else {
                fprintf(output,"n%p%d -> \"s%p%d_%s\" [color=cyan dir=\"none\"]\n",
                        (void*)node->context,node->node_number,
                        (void*)node->symbolNode->symbol->scope->defining_node->context,node->symbolNode->symbol->scope->defining_node->node_number,
                        node->symbolNode->symbol->name);
            }
        }
    }

    else {
        /* OUT - Bottom Up */
        /* Scope Symbols */
        if (!node->parent || node->scope != node->parent->scope) {
            if (node->scope) scp_4all(node->scope, pdot_scope, output);
        }

        /* Scope == DOT Subgraph */
        if (!node->parent || node->scope != node->parent->scope) {
            if (node->scope) fprintf(output, "}\n");
        }
    }

    return result_normal;
}

void pdot_tree(ASTNode *tree, char* output_file, char* prefix) {
    char dot_filename[250];
    char command[250];
    FILE *output;

    snprintf(dot_filename, 250, "%s.%s.dot", prefix, output_file);
    output = fopen(dot_filename, "w");

    if (tree) {
        fprintf(output, "digraph REXXAST { pad=0.25\n");
        ast_wlkr(tree, pdot_walker_handler, (void *) output);
        fprintf(output, "\n}\n");
    }
    fclose(output);

    /* Get dot from https://graphviz.org/download/ */
    snprintf(command, 250, "dot %s.%s.dot -Tpng -o %s.%s.png", prefix, output_file, prefix, output_file);
    system(command);
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

/* Set Node Value and Target Type from Symbol */
void ast_svtp(ASTNode* node, Symbol* symbol) {
    node->value_type = symbol->type;
    node->value_dims = symbol->value_dims;
    node->target_type = symbol->type;
    node->target_dims = symbol->value_dims;

    if (node->value_dim_base) free(node->value_dim_base);
    if (node->value_dim_elements) free(node->value_dim_elements);
    if (node->value_dims) {
        node->value_dim_base = malloc(sizeof(int) * node->value_dims);
        memcpy(node->value_dim_base, symbol->dim_base, sizeof(int) * node->value_dims);
        node->value_dim_elements = malloc(sizeof(int) * node->target_dims);
        memcpy(node->value_dim_elements, symbol->dim_elements, sizeof(int) * node->value_dims);
    }
    else {
        node->value_dim_base = 0;
        node->value_dim_elements = 0;
    }

    if (node->target_dim_base) free(node->target_dim_base);
    if (node->target_dim_elements) free(node->target_dim_elements);
    if (node->target_dims) {
        node->target_dim_base = malloc(sizeof(int) * node->target_dims);
        memcpy(node->target_dim_base, symbol->dim_base, sizeof(int) * node->target_dims);

        node->target_dim_elements = malloc(sizeof(int) * node->target_dims);
        memcpy(node->target_dim_elements, symbol->dim_elements, sizeof(int) * node->target_dims);
    }
    else {
        node->target_dim_base = 0;
        node->target_dim_elements = 0;
    }

    if (node->value_class) free(node->value_class);
    if (symbol->value_class) {
        node->value_class = malloc(strlen(symbol->value_class) + 1);
        strcpy(node->value_class, symbol->value_class);
    } else node->value_class = 0;

    if (node->target_class) {
        free(node->target_class);
        node->target_class = 0;
    }
}

/* Set Node Target Value Type from Symbol */
/* Note: Does not validate promotion */
void ast_sttp(ASTNode* node, Symbol* symbol) {
    node->target_type = symbol->type;
    node->target_dims = symbol->value_dims;

    if (node->target_dim_base) free(node->target_dim_base);
    if (node->target_dim_elements) free(node->target_dim_elements);
    if (node->target_dims) {
        node->target_dim_base = malloc(sizeof(int) * node->target_dims);
        memcpy(node->target_dim_base, symbol->dim_base, sizeof(int) * node->target_dims);

        node->target_dim_elements = malloc(sizeof(int) * node->target_dims);
        memcpy(node->target_dim_elements, symbol->dim_elements, sizeof(int) * node->target_dims);
    }
    else {
        node->target_dim_base = 0;
        node->target_dim_elements = 0;
    }

    if (node->target_class) free(node->target_class);
    if (symbol->value_class) {
        node->target_class = malloc(strlen(symbol->value_class) + 1);
        strcpy(node->target_class, symbol->value_class);
    } else node->target_class = 0;
}

/* Set Node Target Value Type from Target Type of from_node */
/* Note: Does not validate promotion */
void ast_sttn(ASTNode* node, ASTNode* from_node) {
    node->target_type = from_node->target_type;
    node->target_dims = from_node->target_dims;

    if (node->target_dim_base) free(node->target_dim_base);
    if (node->target_dim_elements) free(node->target_dim_elements);
    if (node->target_dims) {
        node->target_dim_base = malloc(sizeof(int) * node->target_dims);
        memcpy(node->target_dim_base, from_node->target_dim_base, sizeof(int) * node->target_dims);

        node->target_dim_elements = malloc(sizeof(int) * node->target_dims);
        memcpy(node->target_dim_elements, from_node->target_dim_elements, sizeof(int) * node->target_dims);
    }
    else {
        node->target_dim_base = 0;
        node->target_dim_elements = 0;
    }

    if (node->target_class) free(node->target_class);
    if (from_node->target_class) {
        node->target_class = malloc(strlen(from_node->target_class) + 1);
        strcpy(node->target_class, from_node->target_class);
    } else node->target_class = 0;
}

/* Set Node Value (and Target) Type from the from_node target type */
void ast_svtn(ASTNode* node, ASTNode* from_node) {
    node->value_type = from_node->target_type;
    node->value_dims = from_node->target_dims;
    node->target_type = from_node->target_type;
    node->target_dims = from_node->target_dims;

    if (node->value_dim_base) free(node->value_dim_base);
    if (node->value_dim_elements) free(node->value_dim_elements);
    if (node->value_dims) {
        node->value_dim_base = malloc(sizeof(int) * node->value_dims);
        memcpy(node->value_dim_base, from_node->target_dim_base, sizeof(int) * node->value_dims);
        node->value_dim_elements = malloc(sizeof(int) * node->target_dims);
        memcpy(node->value_dim_elements, from_node->target_dim_elements, sizeof(int) * node->value_dims);
    }
    else {
        node->value_dim_base = 0;
        node->value_dim_elements = 0;
    }

    if (node->target_dim_base) free(node->target_dim_base);
    if (node->target_dim_elements) free(node->target_dim_elements);
    if (node->target_dims) {
        node->target_dim_base = malloc(sizeof(int) * node->target_dims);
        memcpy(node->target_dim_base, from_node->target_dim_base, sizeof(int) * node->target_dims);

        node->target_dim_elements = malloc(sizeof(int) * node->target_dims);
        memcpy(node->target_dim_elements, from_node->target_dim_elements, sizeof(int) * node->target_dims);
    }
    else {
        node->target_dim_base = 0;
        node->target_dim_elements = 0;
    }

    if (node->value_class) free(node->value_class);
    if (from_node->target_class) {
        node->value_class = malloc(strlen(from_node->target_class) + 1);
        strcpy(node->value_class, from_node->target_class);
    } else node->value_class = 0;
    if (node->target_class) {
        free(node->target_class);
        node->target_class = 0;
    }
}

/* Reset Node Target Type to be the same as the node value type */
void ast_rttp(ASTNode* node) {
    node->target_type = node->value_type;
    node->target_dims = node->value_dims;

    if (node->target_dim_base) free(node->target_dim_base);
    if (node->target_dim_elements) free(node->target_dim_elements);
    if (node->target_dims) {
        node->target_dim_base = malloc(sizeof(int) * node->target_dims);
        memcpy(node->target_dim_base, node->value_dim_base, sizeof(int) * node->target_dims);

        node->target_dim_elements = malloc(sizeof(int) * node->target_dims);
        memcpy(node->target_dim_elements, node->value_dim_elements, sizeof(int) * node->target_dims);
    }
    else {
        node->target_dim_base = 0;
        node->target_dim_elements = 0;
    }

    if (node->target_class) {
        free(node->target_class);
        node->target_class = 0; /* I.e. assumed to be the same as the value type */
    }
}

/* Returns the index number of a child of its parent (or -1 on error) */
int ast_chdi(ASTNode* node) {
    int i;
    ASTNode* n;

    if (!node->parent) return -1;

    for (i = 0, n = node->parent->child; n; n = n->sibling) {
        if (n == node) return i;
        if (n->node_type != ERROR && n->node_type != WARNING) i++;
    }
    return -1;
}

/* Returns the nth child of a parent (or 0 on error), skipping ERROR/WARNING nodes */
ASTNode* ast_chdn(ASTNode* parent, size_t n) {
    ASTNode* node = parent->child;
    size_t i = 0;
    while (node) {
        if (node->node_type != ERROR && node->node_type != WARNING) {
            if (i == n) return node;
            i++;
        }
        node = node->sibling;
    }
    return 0;
}

/* Returns the next sibling a node (or 0 on error), skipping ERROR/WARNING nodes */
ASTNode* ast_nsib(ASTNode* node) {
    while (node) {
        node = node->sibling;
        if (node && node->node_type != ERROR && node->node_type != WARNING) return node;
    }
    return 0;
}

