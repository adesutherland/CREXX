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
 * Validation Pipeline Orchestrator
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "rxcp_val.h"

/* Common Helpers */

/* Helper function to compare node string value with a string
 * Used for builtin class names (int, float etc) - so don't need to worry
 * about utf
 * NOTE value MUST be in lower case! */
int is_node_string(ASTNode* node, const char* value) {
    int i;
    /* If it is a different length it can't be the same! */
    if (strlen(value) != node->node_string_length) return 0;

    for (i=0; i < (int)node->node_string_length; i++) {
        if (tolower(node->node_string[i]) != value[i]) return 0;
    }
    return 1;
}

/* Convert a node (i.e. type INTEGER) to an integer - no error correction as the lexer will have done that */
int node_to_integer(ASTNode* node) {
    int result;
    if (!node) return 0;
    char *s = node->node_string;
    size_t l = node->node_string_length;

    /* Skip leading dot or spaces */
    while (l && (*s == '.' || *s == ' ')) {
        s++;
        l--;
    }

    char *buffer = malloc(l + 1);

    /* Null terminated buffer - {sigh} */
    buffer[l] = 0;
    memcpy(buffer, s, l);

    result = atoi(buffer);

    free(buffer);

    return result;
}

/*
 * Helper function to set dimensions based on a node
 * Sets dims to the number of dimensions (or 0 if not an array)
 *      dim_base and dim_elements are malloced and set as appropriately
 */
void node_to_dims(ASTNode* node, size_t *dims, int** dim_base, int** dim_elements) {
    *dims = 0;
    int i;
    ASTNode* n;
    ASTNode* temp;
    ASTNode* min;
    ASTNode* max;

    if (*dim_base) free(*dim_base);
    *dim_base = 0;

    if (*dim_elements) free(*dim_elements);
    *dim_elements = 0;

    if (!node) return;

    *dims = ast_nchd(node);
    if (*dims) {
        *dim_base = malloc(sizeof(int) * (*dims));
        *dim_elements = malloc(sizeof(int) * (*dims));

        /* We are an array - determine the array bounds */
        if (node->node_type == CLASS) {
            /* Type definition - can have specific bounds */
            n = ast_chdn(node,0); /* n is a RANGE for that dimension */
            for (i = 0; i < *dims; i++) {
                /* Child 1 - n->child - is the base */
                min = ast_chdn(n,0);
                if (min->node_type == NOVAL) (*dim_base)[i] = 1;
                else if (min->node_type == OP_NEG) (*dim_base)[i] = -node_to_integer(ast_chdn(min, 0));
                else (*dim_base)[i] = node_to_integer(min);

                /* Child 2 - n->child->sibling - is the max index value (which we convert to number of elements) */
                max = ast_chdn(n,1);
                if (max->node_type == NOVAL) (*dim_elements)[i] = 0; /* Infinity */

                else if ((*dim_base)[i] == 1 && max->node_type == INTEGER && node_to_integer(max) == 0)
                    (*dim_elements)[i] = 0; /* Syntax candy to make 0 Infinity for base 1 */

                else {
                    if (max->node_type == OP_NEG)
                        (*dim_elements)[i] =
                                -node_to_integer(ast_chdn(max, 0)) - (*dim_base)[i] + 1;
                    else (*dim_elements)[i] = node_to_integer(max) - (*dim_base)[i] + 1;

                    if ((*dim_elements)[i] < 1) {
                        if (max->node_type == OP_NEG) {
                            /* One child expected for the INTEGER otherwise an error MUST have been added already */
                            if (ast_nchd(max) == 1) mknd_err(max, "LESS_THAN_BASE");
                        } else {
                            /* No child expected otherwise an error MUST have been added already */
                            if (ast_nchd(max) == 0) mknd_err(max, "LESS_THAN_BASE");
                        }
                    }
                }

                n = ast_nsib(n);
            }
        } else {
            /* Implicit definition - default bounds */
            for (i = 0; i < *dims; i++) {
                (*dim_base)[i] = 1;
                (*dim_elements)[i] = 0; /* Infinity */
            }
        }
    }
}

/*
 * Helper function to set the arrays
 * Sets dims to the number of dimensions (or 0 if not an array)
 *      class_name is set to either zero of to a class name (if not an in built class)
 *                 So if it is not zero it needs to be free()d
 *      dim_base and dim_elements are malloced and set as appropriately
 */
ValueType node_to_type(Context* context, ASTNode *node, size_t *dims, int **dim_base, int **dim_elements, char **class_name) {
    int i;
    ASTNode *n;
    ASTNode* min;
    ASTNode* max;

    *dims = 0;

    if (*class_name) free(*class_name);
    *class_name = 0;

    if (*dim_base) free(*dim_base);
    *dim_base = 0;

    if (*dim_elements) free(*dim_elements);
    *dim_elements = 0;

    if (!node) return TP_VOID;

    if (node->value_type != TP_UNKNOWN) {
        /* The Node Type has already been determined */
        *dims = node->value_dims;

        if (*dims) {
            *dim_base = malloc(sizeof(int) * (*dims));
            memcpy(*dim_base, node->value_dim_base, sizeof(int) * (*dims));

            *dim_elements = malloc(sizeof(int) * (*dims));
            memcpy(*dim_elements, node->value_dim_elements, sizeof(int) * (*dims));
        }

        if (node->value_class) {
            *class_name = malloc(strlen(node->value_class) + 1);
            strcpy(*class_name, node->value_class);
        }
        return node->value_type;
    }

    /* If we don't let's see if we can determine it now */
    if (node->node_type == CLASS) {
        /* Class and Class Arrays */
        *dims = ast_nchd(node);
        if (*dims) {
            *dim_base = malloc(sizeof(int) * (*dims));
            *dim_elements = malloc(sizeof(int) * (*dims));

            /* We are an array - determine the array bounds */
            /* Type definition - can have specific bounds */
            /* n is a RANGE for that dimension */
            n = ast_chdn(node, 0);
            for (i = 0; i < *dims; i++) {

                /* Child 1 - n->child - is the base */
                min = ast_chdn(n, 0);
                if (min->node_type == NOVAL) (*dim_base)[i] = 1;
                else if (min->node_type == OP_NEG) (*dim_base)[i] = -node_to_integer(ast_chdn(min, 0));
                else (*dim_base)[i] = node_to_integer(min);

                /* Child 2 - n->child->sibling - is the max index value (which we convert to number of elements) */
                max = ast_chdn(n, 1);
                if (max->node_type == NOVAL) (*dim_elements)[i] = 0; /* Infinity */

                else if ((*dim_base)[i] == 1 && max->node_type == INTEGER && node_to_integer(max) == 0)
                    (*dim_elements)[i] = 0; /* Syntax candy to make 0 Infinity for base 1 */

                else {
                    if (max->node_type == OP_NEG)
                        (*dim_elements)[i] =
                                -node_to_integer(ast_chdn(max, 0)) - (*dim_base)[i] + 1;
                    else (*dim_elements)[i] = node_to_integer(max) - (*dim_base)[i] + 1;

                    if ((*dim_elements)[i] < 1) {
                        if (max->node_type == OP_NEG) {
                            /* One child expected for the INTEGER otherwise an error MUST have been added already */
                            if (ast_nchd(max) == 1) mknd_err(max, "LESS_THAN_BASE");
                        } else {
                            /* No child expected otherwise an error MUST have been added already */
                            if (ast_nchd(max) == 0) mknd_err(max, "LESS_THAN_BASE");
                        }
                    }
                }

                n = ast_nsib(n);
            }
        }

        if (is_node_string(node, ".int")) return TP_INTEGER;
        if (is_node_string(node, ".float")) return TP_FLOAT;
        if (is_node_string(node, ".decimal")) return TP_DECIMAL;
        if (is_node_string(node, ".string")) return TP_STRING;
        if (is_node_string(node, ".boolean")) return TP_BOOLEAN;
        if (is_node_string(node, ".binary")) return TP_BINARY;
        if (is_node_string(node, ".void")) return TP_VOID;

        /* TODO Class Support */
        if (node->node_string[0] == '.') {
            *class_name = malloc(node->node_string_length);
            for (i = 1; i < (int)node->node_string_length; i++) {
                (*class_name)[i-1] = (char)tolower(node->node_string[i]);
            }
            (*class_name)[node->node_string_length - 1] = 0;
        } else {
            *class_name = malloc(node->node_string_length + 1);
            for (i = 0; i < (int)node->node_string_length; i++) {
                (*class_name)[i] = (char)tolower(node->node_string[i]);
            }
            (*class_name)[node->node_string_length] = 0;
        }

        /* Try and import the class on-demand if it's not already known */
        if (context->ast && !sym_rvfn(context->ast, *class_name)) {
            sym_imcls(context, node);
        }

        return TP_OBJECT;
    }

    switch (node->node_type) {
        case INTEGER:
        case OP_ARGS:
            return TP_INTEGER;
        case FLOAT:
            return TP_FLOAT;
        case DECIMAL:
            return TP_DECIMAL;
        case STRING:
            return TP_STRING;
        case BINARY:
            return TP_BINARY;
        case VOID:
            return TP_VOID;
        default:
            return TP_UNKNOWN;
    }
}

/* Validates a node promotion is correct adding error nodes if not */
void validate_node_promotion(ASTNode* node) {
    size_t i;
    if (node->target_type == TP_UNKNOWN) return; /* Can't validate yet - will be done later after the target is set */
    if (node->value_type == TP_UNKNOWN) return; /* Can't validate yet - will be done later after the value is set */

    /* Ignore error nodes */
    if (node->node_type == ERROR) return;
    if (node->node_type == WARNING) return;

    if (node->value_dims != node->target_dims) {
        if (!node->value_dims) mknd_err(node, "EXPECTING_ARRAY");
        else if (!node->target_dims)
            mknd_err(node, "UNEXPECTED_ARRAY");
        else mknd_err(node, "ARRAY_DIMS_MISMATCH");
    }
    else if (node->value_dims) {
        /* Check Dimension base/values */
        for (i = 0; i<node->value_dims; i++) {
            if (node->value_dim_base[i] != node->target_dim_base[i])
                mknd_err(node, "INCOMPATIBLE_DIM_BASE dim=%d from=%d to=%d",
                         (int)i + 1, node->value_dim_base[i], node->target_dim_base[i]);
            else if (node->value_dim_elements[i] != node->target_dim_elements[i] && node->target_dim_elements[i])
                mknd_err(node, "INCOMPATIBLE_DIM_SIZE dim=%d from=%d to=%d",
                         (int)i + 1, node->value_dim_elements[i], node->target_dim_elements[i]);
        }
    }

    if (node->value_dims && node->value_type != node->target_type) mknd_err(node, "ARRAY_ELEMENT_TYPE_MISMATCH");

    if (node->value_type == TP_VOID && node->target_type != TP_VOID) mknd_err(node, "MISSING_VALUE");

    /* Binary cant to cast */
    if (node->value_type != node->target_type &&
        node->value_type == TP_BINARY &&
        node->target_type != TP_BINARY) mknd_err(node, "CANNOT_CAST_BINARY");

    if (node->value_type != TP_VOID && node->target_type == TP_VOID) mknd_err(node, "UNEXPECTED_VALUE");

    /* Class / Object Support */
    if (node->value_type == TP_OBJECT || node->target_type == TP_OBJECT) {
        if (node->value_type != node->target_type) {
            mknd_err(node, "TYPE_MISMATCH");
        }
        else if (node->value_class && node->target_class) {
            if (strcmp(node->value_class, node->target_class) != 0) {
                mknd_err(node, "TYPE_MISMATCH");
            }
        }
        else if (node->value_class || node->target_class) {
            mknd_err(node, "TYPE_MISMATCH");
        }
    }
}

/*
 * Sets the ordinal value of each node in the tree
 */
walker_result set_node_ordinals_walker(walker_direction direction,
                                              ASTNode* node,
                                              void *payload) {
    int* ordinal_counter = (int*)payload;

    if (direction == out) {
        /* BOTTOM-UP */
        node->high_ordinal = (*ordinal_counter)++;

        if (node->child) node->low_ordinal = node->child->low_ordinal;
        else node->low_ordinal = node->high_ordinal;
    }
    return result_normal;
}

/* Validate AST */
void validate_ast(Context *context) {
    int ordinal_counter;

    /* AST fixups */
    context->current_scope = 0;
    context->in_factory = 0;
    ast_wlkr(context->ast, initial_checks_walker, (void *) context);
    ast_wlkr(context->ast, rxcp_fixup_walker, (void *) context);

    // Initial checks walker will have set the options
    if (context->floats_decimal)  {
        /* decimal option set - this walker converts flaat node types to decimal */
        ast_wlkr(context->ast, float2decimal_walker, (void *) context);
    } else if (context->floats_binary) {
        /* binary option set - this walker converts decimal node types to binary */
        ast_wlkr(context->ast, decimal2float_walker, (void *) context);
    }

    /* Adds rxsysb library (e.g. for ADDRESS and EXIT) */
    context->current_scope = 0;
    context->need_rxsysb = 0;
    ast_wlkr(context->ast, needs_rxsysb_walker, (void *) context);
    if (context->need_rxsysb) {
        context->current_scope = 0;
        context->has_rxsysb = 0;
        ast_wlkr(context->ast, add_rxsysb_walker, (void *) context);
    }

    if (context->debug_mode && context == context->master_context) {
        rxcp_debug_header("STAGE_FIXUP", -1);
        rxcp_print_ast_recursive(context->ast, 0);
    }

    /* Fixed Point Iteration Loop */
    context->iterations = 0;
    context->after_rewrite = 0;
    do {
        context->changed = 0;

        if (context->debug_mode && context == context->master_context) {
            rxcp_debug_header("STAGE_SYMBOLS", context->iterations);
            rxcp_print_ast_recursive(context->ast, 0);
            rxcp_print_symbol_table(context->ast->scope, 0);
        }

        /* Exit Dispatch */
        context->current_scope = 0;
        ast_wlkr(context->ast, exit_dispatch_walker, (void *) context);

        /* Re-write IMPLICIT_CMD Instructions */
        context->current_scope = 0;
        ast_wlkr(context->ast, rewrite_implicit_cmd_walker, (void *) context);

        /* Set Ordinals */
        ordinal_counter = 0;
        ast_wlkr(context->ast, set_node_ordinals_walker, (void *) &ordinal_counter);

        /* Builds the Symbol Table */
        context->current_scope = 0;
        ast_wlkr(context->ast, build_symbols_walker, (void *) context);

        /* Scan imports now that namespaces are materialized; mark changed to rebuild symbols if any file loaded */
        if (rxcp_scan_imports(context)) {
            context->changed = 1;
        }

        /* Mainly resolve symbols - functions */
        context->current_scope = 0;
        ast_wlkr(context->ast, resolve_functions_walker, (void *) context);

        /* Resolve exposed symbols */
        context->current_scope = 0;
        ast_wlkr(context->ast, exposed_symbols_walker, (void *) context);

        /* Validate Symbols */
        validate_symbols(context, context->ast->scope);

        /* Exit Dispatch */
        context->current_scope = 0;
        ast_wlkr(context->ast, exit_dispatch_walker, (void *) context);

        /* Set Node Types */
        context->current_scope = 0;
        ast_wlkr(context->ast, set_node_types_walker, (void *) context);

        /* Re-write ADDRESS Instructions */
        context->current_scope = 0;
        ast_wlkr(context->ast, rewrite_address_walker, (void *) context);

        /* Re-write EXIT Instructions */
        context->current_scope = 0;
        ast_wlkr(context->ast, rewrite_exit_walker, (void *) context);

        context->iterations++;
        /* Incremental update of symbols - So walkers can avoid duplicate processing */
        if (context->iterations == 1) context->after_rewrite = 1;

    } while (context->changed && context->iterations < 16);

    /* Type Safety checks */
    context->current_scope = 0;
    ast_wlkr(context->ast, type_safety_walker, (void *)context);

    /* Type Safety for function arguments */
    context->current_scope = 0;
    ast_wlkr(context->ast, func_type_safety_walker, (void *)context);

    /* Set Scope Decimal parameters */
    context->current_scope = 0;
    ast_wlkr(context->ast, decimal_parameters_walker, (void *)context);
}

void rxcp_val(Context *context) {
    validate_ast(context);
}

/* Basic validation for an AST (typically the AST will be attached to a main AST as part of
 * function import) */
void rxcp_bvl(Context *context) {
    int ordinal_counter = 0;

    /* Step 1
     * - Sets the source start / finish for eac node
     * - Fixes SCONCAT to CONCAT
     * - Other AST fixups (TBC)
     */
    ast_wlkr(context->ast, initial_checks_walker, (void *) context);

    /* 1b - set node ordinal values */
    ast_wlkr(context->ast, set_node_ordinals_walker, (void *) &ordinal_counter);

    /* Step 2
     * - Builds the Symbol Table
     */
    /* Mainly build symbols - procedures, members */
    context->current_scope = 0;
    ast_wlkr(context->ast, build_symbols_walker, (void *) context);
}
