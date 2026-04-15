/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * AST Node Management and Memory Operations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "rxcpmain.h"
#include "rxcpbgmr.h"

void ast_set_primary_source_node(ASTNode *node, SourceNode *source_node, ASTSourceProvenance provenance) {
    if (!node) return;

    node->source_node = source_node;
    if (source_node && provenance != AST_SOURCE_NONE) node->source_provenance = (char)provenance;
    else node->source_provenance = AST_SOURCE_NONE;
}

void ast_copy_source_anchor(ASTNode *node, ASTNode *source_node, ASTSourceProvenance provenance) {
    if (!node || !source_node) return;

    node->file_name = source_node->file_name;
    node->token = source_node->token;
    node->line = source_node->line;
    node->column = source_node->column;
    node->token_start = source_node->token_start;
    node->token_end = source_node->token_end;
    node->source_start = source_node->source_start;
    node->source_end = source_node->source_end;

    if (source_node->source_node) ast_set_primary_source_node(node, source_node->source_node, provenance);
    else ast_set_primary_source_node(node, 0, AST_SOURCE_NONE);
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
    node->is_duplicate_warning = 0;
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
    node->decimal_value = 0; /* Decimal value as a string - malloced */
    node->exit_obj_reg = -1;
    node->register_num = -1;
    node->register_type = 'r';
    node->additional_registers = -1;
    node->num_additional_registers = 0;
    node->is_ref_arg = 0;
    node->is_const_arg = 0;
    node->is_opt_arg = 0;
    node->is_varg = 0;
    node->is_compiler_added = 0;
    node->force_local_scope = 0;
    node->inherit_parent_reg_scope = 0;
    node->suppress_shadow_warnings = 0;
    node->skip_exit_dispatch = 0;
    node->free_list = context->free_list;
    node->source_node = 0;
    node->source_provenance = AST_SOURCE_NONE;
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
    if (token) {
        node->node_string = token->token_string;
        node->node_string_length = token->length;
    } else {
        node->node_string = "";
        node->node_string_length = 0;
    }
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
    ASTNode *new_node = ast_ft(new_context, node->node_type);

    /* Context and Node Number already set by ast_ft */
    new_node->value_type = node->value_type;
    new_node->value_dims = node->value_dims;
    if (node->value_dim_base) {
        new_node->value_dim_base = malloc(sizeof(int) * node->value_dims);
        memcpy(new_node->value_dim_base, node->value_dim_base, sizeof(int) * node->value_dims);
    }
    if (node->value_dim_elements) {
        new_node->value_dim_elements = malloc(sizeof(int) * node->value_dims);
        memcpy(new_node->value_dim_elements, node->value_dim_elements, sizeof(int) * node->value_dims);
    }
    if (node->value_class) {
        new_node->value_class = malloc(strlen(node->value_class) + 1);
        strcpy(new_node->value_class, node->value_class);
    }

    new_node->target_type = node->target_type;
    new_node->target_dims = node->target_dims;
    if (node->target_dim_base) {
        new_node->target_dim_base = malloc(sizeof(int) * node->target_dims);
        memcpy(new_node->target_dim_base, node->target_dim_base, sizeof(int) * node->target_dims);
    }
    if (node->target_dim_elements) {
        new_node->target_dim_elements = malloc(sizeof(int) * node->target_dims);
        memcpy(new_node->target_dim_elements, node->target_dim_elements, sizeof(int) * node->target_dims);
    }
    if (node->target_class) {
        new_node->target_class = malloc(strlen(node->target_class) + 1);
        strcpy(new_node->target_class, node->target_class);
    }

    new_node->register_num = node->register_num;
    new_node->register_type = node->register_type;
    new_node->additional_registers = node->additional_registers;
    new_node->num_additional_registers = node->num_additional_registers;

    new_node->is_ref_arg = node->is_ref_arg;
    new_node->is_opt_arg = node->is_opt_arg;
    new_node->is_const_arg = node->is_const_arg;
    new_node->is_varg = node->is_varg;
    new_node->is_compiler_added = node->is_compiler_added;
    new_node->force_local_scope = node->force_local_scope;
    new_node->inherit_parent_reg_scope = node->inherit_parent_reg_scope;
    new_node->suppress_shadow_warnings = node->suppress_shadow_warnings;
    new_node->skip_exit_dispatch = node->skip_exit_dispatch;
    new_node->source_node = node->source_node;
    new_node->source_provenance = node->source_provenance;

    new_node->token = node->token;
    if (node->free_node_string) {
        new_node->node_string = malloc(node->node_string_length + 1);
        memcpy(new_node->node_string, node->node_string, node->node_string_length);
        new_node->node_string[node->node_string_length] = 0;
        new_node->node_string_length = node->node_string_length;
        new_node->free_node_string = 1;
    }
    else {
        new_node->node_string = node->node_string;
        new_node->node_string_length = node->node_string_length;
        new_node->free_node_string = 0;
    }

    new_node->int_value = node->int_value;
    new_node->bool_value = node->bool_value;
    new_node->float_value = node->float_value;
    if (node->decimal_value) {
        new_node->decimal_value = malloc(strlen(node->decimal_value) + 1);
        strcpy(new_node->decimal_value, node->decimal_value);
    }

    new_node->token_start = node->token_start;
    new_node->token_end = node->token_end;
    new_node->source_start = node->source_start;
    new_node->source_end = node->source_end;
    new_node->line = node->line;
    new_node->column = node->column;
    new_node->file_name = node->file_name;

    return new_node;
}

void ast_mark_compiler_generated_block(ASTNode *node) {
    if (!node) return;
    node->is_compiler_added = 1;
    node->force_local_scope = 1;
    node->inherit_parent_reg_scope = 1;
    node->suppress_shadow_warnings = 1;
    node->skip_exit_dispatch = 1;
    if (node->source_node) node->source_provenance = AST_SOURCE_SYNTHETIC;
}

/* Structure for add_dast() walker handler context */
struct add_dast_context {
    ASTNode *source;
    ASTNode *dest;
    ASTNode *insert_point;
};

walker_result add_dast_walker_handler1(walker_direction direction,
                                        ASTNode* node, void *payload) {
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
                new_node->scope = scp_f(new_node->context, new_node->parent->scope, new_node, new_symbol, node->scope->type);
                if (node->scope->type == SCOPE_NAMESPACE) {
                    new_symbol->symbol_type = NAMESPACE_SYMBOL;
                    new_symbol->status = SYM_STATUS_LOCAL_DEF;
                }
                else if (node->scope->type == SCOPE_CLASS) {
                    new_symbol->symbol_type = CLASS_SYMBOL;
                    new_symbol->status = SYM_STATUS_LOCAL_DEF;
                }
                else {
                    new_symbol->symbol_type = FUNCTION_SYMBOL;
                    new_symbol->status = SYM_STATUS_LOCAL_DEF;
                }
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
            if (new_symbol) {
                new_symbol->symbol_type = symbol->symbol_type;
                new_symbol->status = symbol->status;
                new_symbol->type = symbol->type;
                new_symbol->exposed = symbol->exposed;
                new_symbol->fixed_args = symbol->fixed_args;
                new_symbol->has_vargs = symbol->has_vargs;
                new_symbol->is_arg = symbol->is_arg;
                new_symbol->is_ref_arg = symbol->is_ref_arg;
                new_symbol->is_const_arg = symbol->is_const_arg;
                new_symbol->is_opt_arg = symbol->is_opt_arg;

                sym_adnd(new_symbol, new_node, node->symbolNode->readUsage, node->symbolNode->writeUsage);
            } else {
                fprintf(stderr, "INTERNAL ERROR: Duplicating AST - Could not find or create symbol %s\n", fqname);
                /* We can't really recover easily here without leaving the AST in a broken state, but let's at least not crash */
            }
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
    node = context.dest->child;
    if (node) {
        while (node->sibling) node = node->sibling;
    }
    return node;
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

/* ASTNode Factory - adds a DECIMAL token removing the trailing d if it exists */
ASTNode *ast_fdec(Context* context, Token *token) {
    /* Make the token */
    ASTNode *node = ast_f(context, DECIMAL, token);

    /* If the last character is a 'd' then remove it */
    if (node->node_string[node->node_string_length - 1] == 'd' ||
        node->node_string[node->node_string_length - 1] == 'D') {
        node->node_string_length--;
    }
    return node;
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
    ast_copy_source_anchor(node, source_node, AST_SOURCE_EXACT);
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
    ASTNode *target;

    char *buffer = malloc(buffer_size);

    /* Write to buffer as sized */
    va_start(argptr, error_string);
    needed = vsnprintf(buffer, buffer_size, error_string, argptr);
    va_end(argptr);

    /* Check if buffer was large enough, if not realloc and try again */
    if (needed >= buffer_size) {
        buffer_size = needed + 1;
        buffer = realloc(buffer, buffer_size);
        va_start(argptr, error_string);
        vsnprintf(buffer, buffer_size, error_string, argptr);
        va_end(argptr);
    }

    errNode = ast_ft(node->context, ERROR);
    errNode->node_string = buffer;
    errNode->node_string_length = strlen(buffer);
    errNode->free_node_string = 1;

    errNode->line = node->line;
    errNode->column = node->column;
    errNode->file_name = node->file_name;
    errNode->source_start = node->source_start;
    errNode->source_end = node->source_end;
    if (node->source_node) ast_set_primary_source_node(errNode, node->source_node, AST_SOURCE_INHERITED);

    add_ast(node, errNode);

    return node;
}

/* Add an ERROR node only if it doesn't already exist as a child with the same message */
ASTNode* mknd_err_unique(ASTNode* node, char *error_string, ...) {
    va_list argptr;
    size_t buffer_size = 200;
    size_t needed;
    ASTNode *errNode;

    char *buffer = malloc(buffer_size);

    /* Write to buffer as sized */
    va_start(argptr, error_string);
    needed = vsnprintf(buffer, buffer_size, error_string, argptr);
    va_end(argptr);

    /* Check if buffer was large enough, if not realloc and try again */
    if (needed >= buffer_size) {
        buffer_size = needed + 1;
        buffer = realloc(buffer, buffer_size);
        va_start(argptr, error_string);
        vsnprintf(buffer, buffer_size, error_string, argptr);
        va_end(argptr);
    }

    /* Check for duplicate */
    ASTNode *child = node->child;
    while (child) {
        if (child->node_type == ERROR && child->node_string_length == strlen(buffer) && 
            strncmp(child->node_string, buffer, child->node_string_length) == 0) {
            free(buffer);
            return node; /* Duplicate found */
        }
        child = child->sibling;
    }

    errNode = ast_ft(node->context, ERROR);
    errNode->node_string = buffer;
    errNode->node_string_length = strlen(buffer);
    errNode->free_node_string = 1;

    errNode->line = node->line;
    errNode->column = node->column;
    errNode->file_name = node->file_name;
    errNode->source_start = node->source_start;
    errNode->source_end = node->source_end;
    if (node->source_node) ast_set_primary_source_node(errNode, node->source_node, AST_SOURCE_INHERITED);

    add_ast(node, errNode);

    return node;
}

/* Add a WARNING node to a node - returns node for chaining */
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

    /* Check if buffer was large enough, if not realloc and try again */
    if (needed >= buffer_size) {
        buffer_size = needed + 1;
        buffer = realloc(buffer, buffer_size);
        va_start(argptr, error_string);
        vsnprintf(buffer, buffer_size, error_string, argptr);
        va_end(argptr);
    }

    warNode = ast_ft(node->context, WARNING);
    warNode->node_string = buffer;
    warNode->node_string_length = strlen(buffer);
    warNode->free_node_string = 1;

    warNode->line = node->line;
    warNode->column = node->column;
    warNode->file_name = node->file_name;
    warNode->source_start = node->source_start;
    warNode->source_end = node->source_end;
    if (node->source_node) ast_set_primary_source_node(warNode, node->source_node, AST_SOURCE_INHERITED);

    add_ast(node, warNode);

    return node;
}

void ast_str(ASTNode* node, char *string) {
    if (node->free_node_string) {
        free(node->node_string);
    }
    node->node_string = string;
    node->free_node_string = 0;
    node->node_string_length = strlen(string);
}

void ast_sstr(ASTNode *node, char* string, size_t length) {
    if (node->free_node_string) {
        free(node->node_string);
    }
    node->node_string = string;
    node->free_node_string = 1; /* So the malloced buffer is freed when cleaning up */
    node->node_string_length = length;
}

void ast_copy_str(ASTNode* node, char *string) {
    if (node->free_node_string) {
        free(node->node_string);
    }
    node->node_string = strdup(string);
    node->free_node_string = 1; /* So the malloced buffer is freed when cleaning up */
    node->node_string_length = strlen(string);
}

/* ASTNode Factory - Error at last Node */
ASTNode *ast_errh(Context* context, char *error_string) {
    ASTNode *errorAST = ast_ftt(context, ERROR, error_string);
    add_ast(errorAST, ast_f(context, TOKEN, context->token_tail->token_prev->token_prev));

    if (context->debug_mode >= 2) print_error(errorAST, stdout, "DEBUG: Error in");
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

/* Returns the PROCEDURE, METHOD or FACTORY ASTNode of an AST node */
ASTNode* ast_proc(ASTNode *node) {
    /* Prefer Scope hierarchy if available and linked to a defining node */
    if (node && node->scope) {
        Scope *s = node->scope;
        while (s) {
            if (s->type == SCOPE_PROCEDURE) return s->defining_node;
            s = s->parent;
        }
    }
    /* Fallback to AST hierarchy */
    while (node) {
        if (node->node_type == PROCEDURE || node->node_type == METHOD || node->node_type == FACTORY) return node;
        node = node->parent;
    }
    return 0;
}

/* Returns the nearest enclosing CLASS ASTNode */
ASTNode* ast_class(ASTNode *node) {
    if (node && node->scope) {
        Scope *s = node->scope;
        while (s) {
            if (s->type == SCOPE_CLASS) return s->defining_node;
            s = s->parent;
        }
    }
    while (node) {
        if (node->node_type == CLASS_DEF) return node;
        node = node->parent;
    }
    return 0;
}

/* Returns the nearest enclosing NAMESPACE ASTNode */
ASTNode* ast_ns(ASTNode *node) {
    if (node && node->scope) {
        Scope *s = node->scope;
        while (s) {
            if (s->type == SCOPE_NAMESPACE) return s->defining_node;
            s = s->parent;
        }
    }
    while (node) {
        if (node->node_type == NAMESPACE || node->node_type == PROGRAM_FILE) return node;
        node = node->parent;
    }
    return 0;
}

/* Returns the nearest enclosing DO ASTNode */
ASTNode* ast_do(ASTNode *node) {
    while (node) {
        if (node->node_type == DO) return node;
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
        case IMPLICIT_CMD:
            return "IMPLICIT_CMD";
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
        case DEC_CASE:
            return "DEC_CASE";
        case DEC_DIGITS:
            return "DEC_DIGITS";
        case DEC_FUZZ:
            return "DEC_FUZZ";
        case DEC_FORM:
            return "DEC_FORM";
        case DEC_STANDARD:
            return "DEC_STANDARD";
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
        case DECIMAL:
            return "DECIMAL";
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
        case OP_NEG:
            return "OP_NEG";
        case OP_PLUS:
            return "OP_PLUS";
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
        case RANGE:
            return "RANGE";
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
        case CONSTANT:
            return "CONSTANT";
        case VOID:
            return "VOID";
        case FACTORY:
            return "FACTORY";
        case METHOD:
            return "METHOD";
        case WITH:
            return "WITH";
        case NODE_REGISTER:
            return "NODE_REGISTER";
        case OF:
            return "OF";
        case CLASS_DEF:
            return "CLASS_DEF";
        case MEMBER_CALL:
            return "MEMBER_CALL";
        case FACTORY_CALL:
            return "FACTORY_CALL";
        case BLOCK_EXPR:
            return "BLOCK_EXPR";
        case LEAVE_WITH:
            return "LEAVE_WITH";
        case EXIT_EXTENDED:
            return "EXIT_EXTENDED";
        case EXIT_TOKEN:
            return "EXIT_TOKEN";
        case SELECT:
            return "SELECT";
        case SWITCH:
            return "SWITCH";
        case WHEN:
            return "WHEN";
        case OTHERWISE:
            return "OTHERWISE";
        default: return "*UNKNOWN*";
    }
}

static void fix_ast_line_number(ASTNode *node) {
    ASTNode *older = 0;
    ASTNode *n;

    /* If the node has already got a line number there is nothing needed */
    if (node->line != -1) return;

    /* If the parent has not got a line number then the walker has not been run
     * so nothing is needed */
    if (!node->parent || node->parent->line == -1) return;

    /* We need to fix up the line number etc. */
    if (node->node_type == ERROR || node->node_type == WARNING) {
        node->line = node->parent->line;
        node->column = node->parent->column;
        node->source_start = node->parent->source_start;
        node->source_end = node->parent->source_end;
        node->token_start = node->parent->token_start;
        node->token_end = node->parent->token_end;
        return;
    }

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
    ASTNode *c;

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

    c = child;
    while (c) {
        if (!c->source_node && parent->source_node) {
            ast_set_primary_source_node(c, parent->source_node, AST_SOURCE_INHERITED);
        }
        c = c->sibling;
    }

    /* Fix line numbers */
    fix_ast_line_number(child);

    return child;
}

/* Add sibling - Returns younger for chaining */
ASTNode *add_sbtr(ASTNode *older, ASTNode *younger) {
    ASTNode *n;

    if (younger == 0 || older == 0) return younger;
    ASTNode *parent = older->parent;
    while (older->sibling) older = older->sibling;
    older->sibling = younger;
    n = younger;
    while (n) {
        n->parent = parent;
        if (!n->source_node && older->source_node) {
            ast_set_primary_source_node(n, older->source_node, AST_SOURCE_INHERITED);
        }
        n = n->sibling;
    }
    fix_ast_line_number(younger);
    return younger;
}

/* Replace replaced_node with new_node in the tree
 * note that replaced_node should not be a descendant or direct relation of
 * new_node (else we might get a loop in the tree)! */
void ast_rpl(ASTNode* replaced_node, ASTNode* new_node) {
    ASTNode *younger_sibling;

    if (!new_node->source_node && replaced_node->source_node) {
        ast_set_primary_source_node(new_node, replaced_node->source_node, AST_SOURCE_INHERITED);
    }

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
        if (t->decimal_value) free(t->decimal_value);
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

/* Returns a malloced string of the array part of a symbols/type
 * (it returns a null terminated string if there is no array part - still needs a free() */
char *ast_astr(size_t dims, int* base, int* num_elements) {
    char* result;
    int i, c;
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
    int dims = node->value_dims;
    int* dim_base = node->value_dim_base;
    int* dim_elements = node->value_dim_elements;

    if (type == TP_UNKNOWN && node->symbolNode && node->symbolNode->symbol) {
        Symbol *s = node->symbolNode->symbol;
        if (s->type != TP_UNKNOWN) {
            type = s->type;
            dims = s->value_dims;
            dim_base = s->dim_base;
            dim_elements = s->dim_elements;
        }
    }

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
            case TP_DECIMAL:
                strcpy(buffer, ".decimal");
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

    array = ast_astr(dims, dim_base, dim_elements);

    result = malloc(strlen(buffer) + strlen(array) + 1);
    strcpy(result, buffer);
    strcat(result, array);
    free(buffer);
    free(array);

    return result;
}

/* Returns the source code of a node in a malloced buffer with formatting removed / cleaned */
char *ast_nsrc(ASTNode *node) {
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

/* Utility to check is a node (typically an IDENTIFIER) is a certain value */
/* Case-insensitive and only checks the first 14 characters of the value   */
/* Returns 1 if it is, 0 if not */
int nodeis(ASTNode *node, const char* value) {
    char text_buffer[15];
    int val;
    int i;

    if (!node || !node->token || !node->token->token_string || !value) return 0;

    val = (int)strlen(value);
    if (val > 14) val = 14;
    strncpy(text_buffer, node->token->token_string, val);
    // lowercase the buffer
    for (i = 0; i < val; i++) {
        text_buffer[i] = (char)tolower(text_buffer[i]);
    }
    text_buffer[val] = 0;

    if (strcmp(text_buffer, value) == 0) return 1;
    return 0;
}

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

    if (node->target_class) free(node->target_class);
    if (symbol->value_class) {
        node->target_class = malloc(strlen(symbol->value_class) + 1);
        strcpy(node->target_class, symbol->value_class);
    } else node->target_class = 0;
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
    if (node->target_class) free(node->target_class);
    if (from_node->target_class) {
        node->value_class = malloc(strlen(from_node->target_class) + 1);
        strcpy(node->value_class, from_node->target_class);
        node->target_class = malloc(strlen(from_node->target_class) + 1);
        strcpy(node->target_class, from_node->target_class);
    } else {
        node->value_class = 0;
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

    if (node->target_class) free(node->target_class);
    if (node->value_class) {
        node->target_class = malloc(strlen(node->value_class) + 1);
        strcpy(node->target_class, node->value_class);
    } else node->target_class = 0;
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
    size_t i;
    ASTNode* c;

    if (!parent) return 0;

    c = parent->child;
    for (i = 0; c; c = c->sibling) {
        if (c->node_type != ERROR && c->node_type != WARNING) {
            if (i == n) return c;
            i++;
        }
    }

    return 0;
}

/* Returns the next sibling a node (or 0 on error), skipping ERROR/WARNING nodes */
ASTNode* ast_nsib(ASTNode* node) {
    ASTNode* s;

    if (!node) return 0;

    s = node->sibling;
    while (s) {
        if (s->node_type != ERROR && s->node_type != WARNING) return s;
        s = s->sibling;
    }

    return 0;
}


void ast_set_file_name(Context *context, char *file_name) {
    ASTNode *t = context->free_list;
    while (t) {
        t->file_name = file_name;
        t = t->free_list;
    }
}
ASTNode *ast_dup_subtree(Context* new_context, ASTNode *node) {
    if (!node) return NULL;
    ASTNode *new_node = ast_dup(new_context, node);
    ASTNode *child = node->child;
    while (child) {
        add_ast(new_node, ast_dup_subtree(new_context, child));
        child = child->sibling;
    }
    return new_node;
}

static void ast_dup_subtree_with_symbols_recursive(Context* new_context, ASTNode *node, ASTNode *new_node, Scope *current_new_scope) {
    ASTNode *child = node->child;
    ASTNode *new_child;
    Scope *node_new_scope = current_new_scope;

    /* If the node has a scope, duplicate it */
    if (node->scope && node->scope->defining_node == node) {
        node_new_scope = scp_dup(new_context, node->scope, current_new_scope, new_node);
        new_node->scope = node_new_scope;
    } else if (node->scope) {
        /* This node is in a scope but doesn't define it. 
         * We need to find the duplicated version of this scope.
         * For simplicity in inlining, we assume we only duplicate subtrees that are within one procedure.
         */
        new_node->scope = current_new_scope;
    }

    /* Link symbols */
    if (node->symbolNode && node->symbolNode->symbol) {
        Symbol *old_sym = node->symbolNode->symbol;
        Symbol *new_sym = NULL;
        
        /* Look for the symbol in the new scope hierarchy */
        char *fqn = sym_frnm(old_sym);
        new_sym = sym_rfqn(new_context->ast, fqn); /* Search from root of new tree */
        free(fqn);

        if (new_sym) {
            sym_adnd(new_sym, new_node, node->symbolNode->readUsage, node->symbolNode->writeUsage);
        } else {
            /* If not found in new tree, it might be a global/external symbol. Link to original. */
            sym_adnd(old_sym, new_node, node->symbolNode->readUsage, node->symbolNode->writeUsage);
        }
    }

    while (child) {
        new_child = ast_dup(new_context, child);
        add_ast(new_node, new_child);
        ast_dup_subtree_with_symbols_recursive(new_context, child, new_child, node_new_scope);
        child = child->sibling;
    }
}

ASTNode *ast_dup_subtree_with_symbols(Context* new_context, ASTNode *node, Scope *new_parent_scope) {
    if (!node) return NULL;
    ASTNode *new_node = ast_dup(new_context, node);
    ast_dup_subtree_with_symbols_recursive(new_context, node, new_node, new_parent_scope);
    return new_node;
}
