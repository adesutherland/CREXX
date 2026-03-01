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

    /* If int_value is set, use it */
    if (node->int_value) return (int)node->int_value;

    char *s = node->node_string;
    size_t l = node->node_string_length;

    if (!s || !l) return 0;

    /* Skip leading dot or spaces */
    while (l && (*s == '.' || *s == ' ')) {
        s++;
        l--;
    }

    if (!l) return 0;

    char *buffer = malloc(l + 1);
    memcpy(buffer, s, l);
    buffer[l] = 0;

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
    size_t local_dims = 0;
    int *local_dim_base = 0;
    int *local_dim_elements = 0;
    int i;
    ASTNode* n;
    ASTNode* min;
    ASTNode* max;

    if (!node) {
        if (*dim_base) free(*dim_base); *dim_base = 0;
        if (*dim_elements) free(*dim_elements); *dim_elements = 0;
        *dims = 0;
        return;
    }

    local_dims = ast_nchd(node);
    if (local_dims) {
        local_dim_base = malloc(sizeof(int) * (local_dims));
        local_dim_elements = malloc(sizeof(int) * (local_dims));

        /* We are an array - determine the array bounds */
        if (node->node_type == CLASS) {
            /* Type definition - can have specific bounds */
            n = ast_chdn(node,0); /* n is a RANGE for that dimension */
            for (i = 0; i < (int)local_dims; i++) {
                /* Child 1 - n->child - is the base */
                min = ast_chdn(n,0);
                if (min->node_type == NOVAL) local_dim_base[i] = 1;
                else if (min->node_type == OP_NEG) local_dim_base[i] = -node_to_integer(ast_chdn(min, 0));
                else local_dim_base[i] = node_to_integer(min);

                /* Child 2 - n->child->sibling - is the max index value (which we convert to number of elements) */
                max = ast_chdn(n,1);
                if (max->node_type == NOVAL) local_dim_elements[i] = 0; /* Infinity */

                else if (local_dim_base[i] == 1 && max->node_type == INTEGER && node_to_integer(max) == 0)
                    local_dim_elements[i] = 0; /* Syntax candy to make 0 Infinity for base 1 */

                else {
                    if (max->node_type == OP_NEG)
                        local_dim_elements[i] =
                                -node_to_integer(ast_chdn(max, 0)) - local_dim_base[i] + 1;
                    else local_dim_elements[i] = node_to_integer(max) - local_dim_base[i] + 1;

                    if (local_dim_elements[i] < 1) {
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
            for (i = 0; i < (int)local_dims; i++) {
                local_dim_base[i] = 1;
                local_dim_elements[i] = 0; /* Infinity */
            }
        }
    }

    if (*dim_base) free(*dim_base);
    *dim_base = local_dim_base;

    if (*dim_elements) free(*dim_elements);
    *dim_elements = local_dim_elements;

    *dims = local_dims;
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
    size_t local_dims = 0;
    int *local_dim_base = 0;
    int *local_dim_elements = 0;
    char *local_class_name = 0;
    ValueType result;

    if (!node) {
        if (*class_name) free(*class_name); *class_name = 0;
        if (*dim_base) free(*dim_base); *dim_base = 0;
        if (*dim_elements) free(*dim_elements); *dim_elements = 0;
        *dims = 0;
        return TP_VOID;
    }

    if (node->value_type != TP_UNKNOWN) {
        /* The Node Type has already been determined */
        local_dims = node->value_dims;

        if (local_dims) {
            local_dim_base = malloc(sizeof(int) * (local_dims));
            memcpy(local_dim_base, node->value_dim_base, sizeof(int) * (local_dims));

            local_dim_elements = malloc(sizeof(int) * (local_dims));
            memcpy(local_dim_elements, node->value_dim_elements, sizeof(int) * (local_dims));
        }

        if (node->value_class) {
            local_class_name = malloc(strlen(node->value_class) + 1);
            strcpy(local_class_name, node->value_class);
        }
        result = node->value_type;
        goto exit;
    }

    /* If we don't know the node type, let's see if we can determine it from the symbol */
    if (node->node_type == FUNCTION) {
        if (node->symbolNode && node->symbolNode->symbol && node->symbolNode->symbol->type != TP_UNKNOWN) {
            Symbol *s = node->symbolNode->symbol;
            local_dims = s->value_dims;
            if (local_dims) {
                local_dim_base = malloc(sizeof(int) * (local_dims));
                memcpy(local_dim_base, s->dim_base, sizeof(int) * (local_dims));
                local_dim_elements = malloc(sizeof(int) * (local_dims));
                memcpy(local_dim_elements, s->dim_elements, sizeof(int) * (local_dims));
            }
            if (s->value_class) {
                local_class_name = malloc(strlen(s->value_class) + 1);
                strcpy(local_class_name, s->value_class);
            }
            result = s->type;
            goto exit;
        }
    }

    /* If we don't let's see if we can determine it now */
    if (node->node_type == CLASS) {
        /* Class and Class Arrays */
        local_dims = ast_nchd(node);
        if (local_dims) {
            local_dim_base = malloc(sizeof(int) * (local_dims));
            local_dim_elements = malloc(sizeof(int) * (local_dims));

            /* We are an array - determine the array bounds */
            /* Type definition - can have specific bounds */
            /* n is a RANGE for that dimension */
            n = ast_chdn(node, 0);
            for (i = 0; i < (int)local_dims; i++) {

                /* Child 1 - n->child - is the base */
                min = ast_chdn(n, 0);
                if (min->node_type == NOVAL) local_dim_base[i] = 1;
                else if (min->node_type == OP_NEG) local_dim_base[i] = -node_to_integer(ast_chdn(min, 0));
                else local_dim_base[i] = node_to_integer(min);

                /* Child 2 - n->child->sibling - is the max index value (which we convert to number of elements) */
                max = ast_chdn(n, 1);
                if (max->node_type == NOVAL) local_dim_elements[i] = 0; /* Infinity */

                else if (local_dim_base[i] == 1 && max->node_type == INTEGER && node_to_integer(max) == 0)
                    local_dim_elements[i] = 0; /* Syntax candy to make 0 Infinity for base 1 */

                else {
                    if (max->node_type == OP_NEG)
                        local_dim_elements[i] =
                                -node_to_integer(ast_chdn(max, 0)) - local_dim_base[i] + 1;
                    else local_dim_elements[i] = node_to_integer(max) - local_dim_base[i] + 1;

                    if (local_dim_elements[i] < 1) {
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

        if (is_node_string(node, ".int")) {
            result = TP_INTEGER;
            goto exit;
        }
        if (is_node_string(node, ".float")) {
            result = TP_FLOAT;
            goto exit;
        }
        if (is_node_string(node, ".decimal")) {
            result = TP_DECIMAL;
            goto exit;
        }
        if (is_node_string(node, ".string")) {
            result = TP_STRING;
            goto exit;
        }
        if (is_node_string(node, ".boolean")) {
            result = TP_BOOLEAN;
            goto exit;
        }
        if (is_node_string(node, ".binary")) {
            result = TP_BINARY;
            goto exit;
        }
        if (is_node_string(node, ".void")) {
            result = TP_VOID;
            goto exit;
        }

        /* TODO Class Support */
        if (node->node_string[0] == '.') {
            local_class_name = malloc(node->node_string_length);
            for (i = 1; i < (int)node->node_string_length; i++) {
                local_class_name[i-1] = (char)tolower(node->node_string[i]);
            }
            local_class_name[node->node_string_length - 1] = 0;
        } else {
            local_class_name = malloc(node->node_string_length + 1);
            for (i = 0; i < (int)node->node_string_length; i++) {
                local_class_name[i] = (char)tolower(node->node_string[i]);
            }
            local_class_name[node->node_string_length] = 0;
        }

        /* Try and import the class on-demand if it's not already known */
        if (context->ast && !sym_rvfn(context->ast, local_class_name)) {
            sym_imcls(context, node);
        }

        result = TP_OBJECT;
        goto exit;
    }

    local_dims = 0;
    switch (node->node_type) {
        case INTEGER:
        case OP_ARGS:
            result = TP_INTEGER;
            break;
        case FLOAT:
            result = TP_FLOAT;
            break;
        case DECIMAL:
            result = TP_DECIMAL;
            break;
        case STRING:
            result = TP_STRING;
            break;
        case BINARY:
            result = TP_BINARY;
            break;
        case VOID:
            result = TP_VOID;
            break;
        default:
            result = TP_UNKNOWN;
            break;
    }

exit:
    if (*class_name) free(*class_name);
    *class_name = local_class_name;

    if (*dim_base) free(*dim_base);
    *dim_base = local_dim_base;

    if (*dim_elements) free(*dim_elements);
    *dim_elements = local_dim_elements;

    *dims = local_dims;
    return result;
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

    /* AST fixups 
     * The Lemon parser only has a single lookahead and produces an unfinished flat AST.
     * These walkers restructure the AST into a proper logical hierarchy (e.g., nesting 
     * instructions under procedures and classes) and propagate source locations.
     */
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

    /* Pre-Loop Initialization: Prime the symbol table and imports */
    context->current_scope = 0;
    ast_wlkr(context->ast, build_symbols_walker, (void *) context);
    rxcp_scan_imports(context);

    /* fixed point validation - Converge all exits */
    context->iterations = 0;
    context->after_rewrite = 0;
    do {
        context->changed = 0;

        if (context->debug_mode && context == context->master_context) {
            rxcp_debug_header("STAGE_SYMBOLS", context->iterations);
            rxcp_print_ast_recursive(context->ast, 0);
            rxcp_print_symbol_table(context->ast->scope, 0);
        }

        /* Exit Dispatch
         * Progress: exit_dispatch_walker is idempotent. Verified by stress testing with 3x calls per iteration.
         */
        context->current_scope = 0;
        if (ast_wlkr(context->ast, exit_dispatch_walker, (void *) context) == result_error) break;
        if (context->debug_mode >= 2) rxcp_validate_ast_and_symbols(context->ast);
        if (context->debug_mode >= 3) {
            /* Stress test idempotency */
            ast_wlkr(context->ast, exit_dispatch_walker, (void *) context);
            rxcp_validate_ast_and_symbols(context->ast);
            ast_wlkr(context->ast, exit_dispatch_walker, (void *) context);
            rxcp_validate_ast_and_symbols(context->ast);
        }

        /* Re-write IMPLICIT_CMD Instructions
         * Progress: rewrite_implicit_cmd_walker is idempotent. Verified by stress testing with 3x calls per iteration.
         */
        context->current_scope = 0;
        ast_wlkr(context->ast, rewrite_implicit_cmd_walker, (void *) context);
        if (context->debug_mode >= 2) rxcp_validate_ast_and_symbols(context->ast);
        if (context->debug_mode >= 3) {
            /* Stress test idempotency */
            ast_wlkr(context->ast, rewrite_implicit_cmd_walker, (void *) context);
            rxcp_validate_ast_and_symbols(context->ast);
            ast_wlkr(context->ast, rewrite_implicit_cmd_walker, (void *) context);
            rxcp_validate_ast_and_symbols(context->ast);
        }

        /* Set Ordinals
         * Progress: set_node_ordinals_walker is idempotent. Recalculates from reset counter. Verified by stress testing.
         */
        ordinal_counter = 0;
        ast_wlkr(context->ast, set_node_ordinals_walker, (void *) &ordinal_counter);
        if (context->debug_mode >= 2) rxcp_validate_ast_and_symbols(context->ast);
        if (context->debug_mode >= 3) {
            /* Stress test idempotency */
            ordinal_counter = 0;
            ast_wlkr(context->ast, set_node_ordinals_walker, (void *) &ordinal_counter);
            rxcp_validate_ast_and_symbols(context->ast);
            ordinal_counter = 0;
            ast_wlkr(context->ast, set_node_ordinals_walker, (void *) &ordinal_counter);
            rxcp_validate_ast_and_symbols(context->ast);
        }

        /* Builds the Symbol Table
         * Progress: build_symbols_walker is idempotent. Scope creation is guarded; symbol associations use sym_adnd (idempotent).
         */
        context->current_scope = 0;
        ast_wlkr(context->ast, build_symbols_walker, (void *) context);
        if (context->debug_mode >= 2) rxcp_validate_ast_and_symbols(context->ast);
        if (context->debug_mode >= 3) {
            /* Stress test idempotency */
            context->current_scope = 0;
            ast_wlkr(context->ast, build_symbols_walker, (void *) context);
            rxcp_validate_ast_and_symbols(context->ast);
            context->current_scope = 0;
            ast_wlkr(context->ast, build_symbols_walker, (void *) context);
            rxcp_validate_ast_and_symbols(context->ast);
        }

        /* Scan imports now that namespaces are materialized; mark changed to rebuild symbols if any file loaded
         * Progress: rxcp_scan_imports is idempotent. Uses 'imported' flag in the global importable_file_list.
         */
        if (rxcp_scan_imports(context)) {
            context->changed = 1;
        }
        if (context->debug_mode >= 2) rxcp_validate_ast_and_symbols(context->ast);
        if (context->debug_mode >= 3) {
            /* Stress test idempotency */
            rxcp_scan_imports(context);
            rxcp_validate_ast_and_symbols(context->ast);
            rxcp_scan_imports(context);
            rxcp_validate_ast_and_symbols(context->ast);
        }

        /* Mainly resolve symbols - functions
         * Progress: resolve_functions_walker is idempotent. Symbol association is guarded by !node->symbolNode.
         */
        context->current_scope = 0;
        ast_wlkr(context->ast, resolve_functions_walker, (void *) context);
        if (context->debug_mode >= 2) rxcp_validate_ast_and_symbols(context->ast);
        if (context->debug_mode >= 3) {
            /* Stress test idempotency */
            context->current_scope = 0;
            ast_wlkr(context->ast, resolve_functions_walker, (void *) context);
            rxcp_validate_ast_and_symbols(context->ast);
            context->current_scope = 0;
            ast_wlkr(context->ast, resolve_functions_walker, (void *) context);
            rxcp_validate_ast_and_symbols(context->ast);
        }

        /* Resolve exposed symbols
         * Progress: exposed_symbols_walker is reviewed for idempotency.
         */
        context->current_scope = 0;
        ast_wlkr(context->ast, exposed_symbols_walker, (void *) context);
        if (context->debug_mode >= 2) rxcp_validate_ast_and_symbols(context->ast);
        if (context->debug_mode >= 3) {
            /* Stress test idempotency */
            context->current_scope = 0;
            ast_wlkr(context->ast, exposed_symbols_walker, (void *) context);
            rxcp_validate_ast_and_symbols(context->ast);
            context->current_scope = 0;
            ast_wlkr(context->ast, exposed_symbols_walker, (void *) context);
            rxcp_validate_ast_and_symbols(context->ast);
        }

        /* Validate Symbols
         * Progress: validate_symbols is idempotent. Symbols with types already resolved are skipped.
         */
        validate_symbols(context, context->ast->scope);
        if (context->debug_mode >= 2) rxcp_validate_ast_and_symbols(context->ast);
        if (context->debug_mode >= 3) {
            /* Stress test idempotency */
            validate_symbols(context, context->ast->scope);
            rxcp_validate_ast_and_symbols(context->ast);
            validate_symbols(context, context->ast->scope);
            rxcp_validate_ast_and_symbols(context->ast);
        }

        /* Set Node Types
         * Progress: set_node_types_walker is idempotent. Type setting is guarded by TP_UNKNOWN check.
         */
        context->current_scope = 0;
        ast_wlkr(context->ast, set_node_types_walker, (void *) context);
        if (context->debug_mode >= 2) rxcp_validate_ast_and_symbols(context->ast);
        if (context->debug_mode >= 3) {
            /* Stress test idempotency */
            context->current_scope = 0;
            ast_wlkr(context->ast, set_node_types_walker, (void *) context);
            rxcp_validate_ast_and_symbols(context->ast);
            context->current_scope = 0;
            ast_wlkr(context->ast, set_node_types_walker, (void *) context);
            rxcp_validate_ast_and_symbols(context->ast);
        }

        /* Re-write ADDRESS Instructions
         * Progress: rewrite_address_walker is idempotent. Mutates ADDRESS to ASSIGN.
         */
        context->current_scope = 0;
        ast_wlkr(context->ast, rewrite_address_walker, (void *) context);
        if (context->debug_mode >= 2) rxcp_validate_ast_and_symbols(context->ast);
        if (context->debug_mode >= 3) {
            /* Stress test idempotency */
            context->current_scope = 0;
            ast_wlkr(context->ast, rewrite_address_walker, (void *) context);
            rxcp_validate_ast_and_symbols(context->ast);
            context->current_scope = 0;
            ast_wlkr(context->ast, rewrite_address_walker, (void *) context);
            rxcp_validate_ast_and_symbols(context->ast);
        }

        /* Re-write EXIT Instructions
         * Progress: rewrite_exit_walker is idempotent. Mutates EXIT to CALL.
         */
        context->current_scope = 0;
        ast_wlkr(context->ast, rewrite_exit_walker, (void *) context);
        if (context->debug_mode >= 2) rxcp_validate_ast_and_symbols(context->ast);
        if (context->debug_mode >= 3) {
            /* Stress test idempotency */
            context->current_scope = 0;
            ast_wlkr(context->ast, rewrite_exit_walker, (void *) context);
            rxcp_validate_ast_and_symbols(context->ast);
            context->current_scope = 0;
            ast_wlkr(context->ast, rewrite_exit_walker, (void *) context);
            rxcp_validate_ast_and_symbols(context->ast);
        }

        context->iterations++;
        if (context->debug_mode >= 2) fprintf(stderr, "DEBUG: Iteration %d finished, changed=%d\n", context->iterations, context->changed);
        /* Incremental update of symbols - So walkers can avoid duplicate processing */
        if (context->iterations == 1) context->after_rewrite = 1;

    } while ((context->changed || (context->debug_mode >= 3 && context->iterations < 3)) && context->iterations < 16);

    /* Final pass to mutate remaining taken constants to STRING nodes */
    context->after_rewrite = 2;
    validate_symbols(context, context->ast->scope);

    /* Set Ordinals again after normalisation - not needed if normalisation is once at start */
    ordinal_counter = 0;
    ast_wlkr(context->ast, set_node_ordinals_walker, (void *) &ordinal_counter);

    /* Type Safety checks */
    context->current_scope = 0;
    ast_wlkr(context->ast, type_safety_walker, (void *)context);

    /* Type Safety for function arguments */
    context->current_scope = 0;
    ast_wlkr(context->ast, func_type_safety_walker, (void *)context);

    /* Set Scope Decimal parameters */
    context->current_scope = 0;
    ast_wlkr(context->ast, decimal_parameters_walker, (void *)context);

    if (context->ast->node_type == REXX_UNIVERSE) {
        context->ast->value_type = TP_VOID;
    }
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
