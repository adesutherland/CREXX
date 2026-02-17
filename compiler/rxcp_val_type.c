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
 * Validation Pass: Type Checking and Promotion
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "rxcp_val.h"

/* Validates a node promotion is correct for a call by reference (of symbols) adding error nodes if not */
void validate_node_promotion_for_ref(ASTNode* node) {
    size_t i;

    if (node->target_type == TP_UNKNOWN || node->value_type == TP_UNKNOWN) {
        mknd_err(node, "UNKNOWN_TYPE");
        return;
    }

    /* Ignore error nodes */
    if (node->node_type == ERROR) return;
    if (node->node_type == WARNING) return;

    if (node->value_dims != node->target_dims) mknd_err(node, "REFERENCE_TYPE_MISMATCH");
    else if (node->value_dims) {
        /* Check Dimension base/values */
        for (i = 0; i<node->value_dims; i++) {
            if (node->value_dim_base[i] != node->target_dim_base[i]) mknd_err(node, "REFERENCE_TYPE_MISMATCH");
            else if (node->value_dim_elements[i] != node->target_dim_elements[i]) mknd_err(node, "REFERENCE_TYPE_MISMATCH");
        }
    }

    if (node->value_type != node->target_type) mknd_err(node, "REFERENCE_TYPE_MISMATCH");

    /* Class / Object Support */
    if (node->value_type == TP_OBJECT || node->target_type == TP_OBJECT) {
        if (node->value_type != node->target_type) mknd_err(node, "REFERENCE_TYPE_MISMATCH");
        else if (node->value_class && node->target_class) {
            if (strcmp(node->value_class, node->target_class) != 0) mknd_err(node, "REFERENCE_TYPE_MISMATCH");
        }
        else if (node->value_class || node->target_class) mknd_err(node, "REFERENCE_TYPE_MISMATCH");
    }
}

/* Step 4
 * - Type Safety
 */

/* Type promotion matrix for numeric operators */
static const ValueType promotion[9][9] = {
/*                   TP_UNKNOWN, TP_VOID,    TP_BOOLEAN, TP_INTEGER, TP_FLOAT,   TP_DECIMAL, TP_STRING,  TP_BINARY,   TP_OBJECT */
/* TP_UNKNOWN */ {TP_UNKNOWN, TP_UNKNOWN, TP_UNKNOWN, TP_UNKNOWN, TP_UNKNOWN, TP_UNKNOWN, TP_UNKNOWN, TP_UNKNOWN,  TP_UNKNOWN},
/* TP_VOID    */ {TP_UNKNOWN, TP_VOID,    TP_BOOLEAN, TP_INTEGER, TP_FLOAT,   TP_DECIMAL, TP_FLOAT,   TP_BINARY,   TP_OBJECT},
/* TP_BOOLEAN */ {TP_UNKNOWN, TP_BOOLEAN, TP_BOOLEAN, TP_INTEGER, TP_FLOAT,   TP_DECIMAL, TP_FLOAT,   TP_UNKNOWN,  TP_OBJECT},
/* TP_INTEGER */ {TP_UNKNOWN, TP_INTEGER, TP_INTEGER, TP_INTEGER, TP_FLOAT,   TP_DECIMAL, TP_FLOAT,   TP_UNKNOWN,  TP_OBJECT},
/* TP_FLOAT   */ {TP_UNKNOWN, TP_FLOAT,   TP_FLOAT,   TP_FLOAT,   TP_FLOAT,   TP_DECIMAL, TP_FLOAT,   TP_UNKNOWN,  TP_OBJECT},
/* TP_DECIMAL */ {TP_UNKNOWN, TP_DECIMAL, TP_DECIMAL, TP_DECIMAL, TP_DECIMAL, TP_DECIMAL, TP_DECIMAL, TP_UNKNOWN,  TP_OBJECT},
/* TP_STRING  */ {TP_UNKNOWN, TP_FLOAT,   TP_FLOAT,   TP_FLOAT,   TP_FLOAT,   TP_DECIMAL, TP_FLOAT,   TP_UNKNOWN,  TP_OBJECT},
/* TP_BINARY  */ {TP_UNKNOWN, TP_BINARY,  TP_UNKNOWN, TP_UNKNOWN, TP_UNKNOWN, TP_UNKNOWN, TP_UNKNOWN, TP_UNKNOWN,  TP_OBJECT},
/* TP_OBJECT  */ {TP_UNKNOWN, TP_OBJECT,  TP_OBJECT,  TP_OBJECT,  TP_OBJECT,  TP_OBJECT,  TP_OBJECT,  TP_OBJECT,   TP_OBJECT},
};

/* Returns the value_type of a node - arrays changes to TP_OBJECT */
static ValueType node_type(ASTNode* node) {
    if (node->value_dims) return TP_OBJECT;
    if (node->value_type != TP_UNKNOWN) return node->value_type;
    if (node->node_type == INTEGER || node->node_type == OP_ARGS) return TP_INTEGER;
    if (node->node_type == FLOAT) return TP_FLOAT;
    if (node->node_type == DECIMAL) return TP_DECIMAL;
    if (node->node_type == STRING || node->node_type == CONSTANT) return TP_STRING;
    return TP_UNKNOWN;
}

/* Returns the highest value_type of the node's children nodes */
static ValueType max_type(ASTNode* node) {
    ASTNode *child;
    ValueType max_type = TP_UNKNOWN;

    child = node->child;
    while (child) {
        ValueType type = node_type(child);
        if (type > max_type) max_type = type;
        child = child->sibling;
    }

    return max_type;
}

/* Set the node value and target type to a simple type (not an array or class name) */
static void set_node_type(ASTNode* node, ValueType type) {
    node->value_type = type;
    node->value_dims = 0;
    if (node->value_class) {
        free(node->value_class);
        node->value_class = 0;
    }
    node->target_type = type;
    node->target_dims = 0;
    if (node->target_class) {
        free(node->target_class);
        node->target_class = 0;
    }
}

/* Set the target value to a simple target_type (not an array or class name)
 * and validates that it is convertable from the nodes value target_type */
static void set_node_target_type(ASTNode* node, ValueType target_type) {
    node->target_type = target_type;
    node->target_dims = 0;
    if (node->target_class) {
        free(node->target_class);
        node->target_class = 0;
    }

    validate_node_promotion(node);
}

/* This walker does the basic value types of operations
 * No errors generated - just simple "guesses" as to types */
walker_result set_node_types_walker(walker_direction direction,
                                           ASTNode* node,
                                           void *payload) {

    Context *context = (Context*)payload;
    ASTNode *child1, *child2, *n1, *n2;
    int val, ix;

    if (direction == in) {
        /* IN - TOP DOWN */
        context->current_scope = node->scope;
    }
    else {
        /* OUT - BOTTOM UP */
        child1 = ast_chdn(node, 0);
        child2 = ast_chdn(node, 1);

        switch (node->node_type) {

            case OP_AND:
            case OP_OR:
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
            case OP_ARG_EXISTS:
            case OP_ARG_IX_EXISTS:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, TP_BOOLEAN);
                }
                break;

            case OP_CONCAT:
            case OP_SCONCAT:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, TP_STRING);
                }
                break;

            case OP_ADD:
            case OP_MINUS:
            case OP_MULT:
            case OP_POWER:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    ValueType type = promotion[node_type(child1)][node_type(child2)];
                    if (type == TP_UNKNOWN) type = TP_INTEGER; /* Default to integer */
                    set_node_type(node, type);
                }
                break;

            case OP_DIV:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    ValueType type = promotion[node_type(child1)][node_type(child2)];
                    type = promotion[type][TP_FLOAT]; /* Ensure at least FLOAT */
                    set_node_type(node, type);
                }
                break;

            case OP_MOD:
            case OP_IDIV:
                if (node->value_type == TP_UNKNOWN) {
                    ValueType type = promotion[node_type(child1)][node_type(child2)];
                    type = promotion[type][TP_INTEGER]; /* Ensure at least INTEGER */
                    if (type != TP_UNKNOWN) {
                        set_node_type(node, type);
                        context->changed = 1;
                    }
                }
                break;

            case OP_NOT:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, TP_BOOLEAN);
                }
                break;

            case OP_PLUS:
            case OP_NEG:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, promotion[node_type(child1)][TP_VOID]);
                }
                break;

            case FUNCTION:
                if (node->symbolNode) { /* Otherwise, an error node will have been added */
                    if (node->value_type == TP_UNKNOWN) {
                        context->changed = 1;
                        ast_svtp(node, node->symbolNode->symbol);
                    }
                }
                break;

            case MEMBER_CALL:
                if (ast_chld(node, ERROR, 0)) break;
                if (node->value_type == TP_UNKNOWN) {
                    ASTNode *instance = ast_chdn(node, 0);
                    if (instance->value_type == TP_OBJECT && instance->value_class) {
                        /* Resolve the Class Symbol from the instance's class name */
                        const char *cname = instance->value_class;
                        if (cname && cname[0] == '.') cname++;
                        Symbol *class_sym = sym_rvfn(context->ast, (char*)cname);
                        if (class_sym && class_sym->symbol_type == CLASS_SYMBOL) {
                            /* Lookup the Method Name in the Class Scope */
                            Symbol *method_sym = sym_lrsv(class_sym->defines_scope, node);
                            if (method_sym && method_sym->symbol_type == FUNCTION_SYMBOL) {
                                sym_adnd(method_sym, node, 1, 0);
                                ast_svtp(node, method_sym);
                                context->changed = 1;
                            } else if (!context->changed) {
                                /* DOT-AS-INDEX MUTATION: transform tokens.i -> tokens[i] */
                                Symbol *index_sym = sym_rslv(context->current_scope, node);
                                if (index_sym && index_sym->symbol_type == VARIABLE_SYMBOL &&
                                    instance->value_dims > 0) {

                                    ASTNode *new_index_node = ast_f(context, VAR_SYMBOL, node->token);
                                    {
                                        char *s = malloc(node->node_string_length + 1);
                                        memcpy(s, node->node_string, node->node_string_length);
                                        s[node->node_string_length] = 0;
                                        ast_sstr(new_index_node, s, node->node_string_length);
                                    }
                                    sym_adnd(index_sym, new_index_node, 1, 0);

                                    /* Transform current node (MEMBER_CALL) into the array's VAR_SYMBOL */
                                    node->node_type = VAR_SYMBOL;
                                    {
                                        char *s = malloc(instance->node_string_length + 1);
                                        memcpy(s, instance->node_string, instance->node_string_length);
                                        s[instance->node_string_length] = 0;
                                        ast_sstr(node, s, instance->node_string_length);
                                    }

                                    /* Disconnect from any previous symbols if any */
                                    if (node->symbolNode) sym_dno(node->symbolNode->symbol, node);

                                    /* Link to the same symbol as instance */
                                    sym_adnd(instance->symbolNode->symbol, node, 1, 0);

                                    /* Set scalar element type */
                                    node->value_type = instance->symbolNode->symbol->type;
                                    node->value_dims = 0;
                                    node->target_dims = 0;
                                    if (node->value_class) { free(node->value_class); node->value_class = 0; }
                                    if (instance->symbolNode->symbol->value_class) {
                                        node->value_class = malloc(strlen(instance->symbolNode->symbol->value_class) + 1);
                                        strcpy(node->value_class, instance->symbolNode->symbol->value_class);
                                    }
                                    ast_rttp(node);

                                    /* Delete children (instance and any args) */
                                    while (node->child) {
                                        ASTNode *c = node->child;
                                        if (c->symbolNode) sym_dno(c->symbolNode->symbol, c);
                                        ast_del(c);
                                    }

                                    /* Add the resolved variable as the index child (subscript) */
                                    add_ast(node, new_index_node);

                                    context->changed = 1;
                                    return result_normal;
                                } else if (!context->changed) {
                                    /* Try and import the class */
                                    Symbol *import_cls = sym_imcls(context, node);
                                    if (import_cls) {
                                        /* Resolve again - it should be found now */
                                        class_sym = sym_rvfc(context->ast, node);
                                    }
                                }
                                if (class_sym && class_sym->symbol_type == CLASS_SYMBOL) {
                                    /* continue with method resolution below */
                                } else if (!context->changed) {
                                    /* Defer error if imports may provide class stubs */
                                    int has_import = 0;
                                    if (context->ast && context->ast->child && context->ast->child->node_type == PROGRAM_FILE) {
                                        ASTNode *pfch = context->ast->child->child;
                                        while (pfch) { if (pfch->node_type == IMPORT) { has_import = 1; break; } pfch = pfch->sibling; }
                                    }
                                    if (has_import) {
                                        context->changed = 1;
                                    } else {
                                        mknd_err(node, "CLASS_NOT_FOUND");
                                    }
                                }
                            }
                        }
                    } else if (instance->value_type != TP_UNKNOWN) {
                        mknd_err(node, "NOT_AN_OBJECT");
                    }
                }
                break;

            case FACTORY_CALL:
                if (ast_chld(node, ERROR, 0)) break;
                if (node->value_type == TP_UNKNOWN) {
                    Symbol *class_sym = sym_rvfc(context->ast, node);
                    if (class_sym && class_sym->symbol_type == CLASS_SYMBOL) {
                        /* Resolve the Factory routine '§factory' within that class */
                        ASTNode star_node;
                        memset(&star_node, 0, sizeof(ASTNode));
                        star_node.node_string = "\xc2\xa7" "factory";
                        star_node.node_string_length = 9;
                        Symbol *factory_sym = sym_lrsv(class_sym->defines_scope, &star_node);
                        if (factory_sym && factory_sym->symbol_type == FUNCTION_SYMBOL) {
                            sym_adnd(factory_sym, node, 1, 0);
                            node->value_type = TP_OBJECT;
                            node->value_class = malloc(strlen(class_sym->name) + 1);
                            strcpy(node->value_class, class_sym->name);
                            ast_rttp(node);
                            context->changed = 1;
                        } else if (!context->changed) {
                            mknd_err(node, "FACTORY_NOT_FOUND");
                        }
                    } else if (!context->changed) {
                        /* Try and import the class */
                        Symbol *import_cls = sym_imcls(context, node);
                        if (import_cls) {
                            context->changed = 1;
                        } else {
                            /* Defer error if imports may provide class stubs */
                            int has_import = 0;
                            if (context->ast && context->ast->child && context->ast->child->node_type == PROGRAM_FILE) {
                                ASTNode *pfch = context->ast->child->child;
                                while (pfch) { if (pfch->node_type == IMPORT) { has_import = 1; break; } pfch = pfch->sibling; }
                            }
                            if (has_import) {
                                context->changed = 1;
                            } else {
                                mknd_err(node, "CLASS_NOT_FOUND");
                            }
                        }
                    }
                }
                break;

            case VAR_SYMBOL:
            case VAR_TARGET:
                if (node->value_type == TP_UNKNOWN && node->symbolNode) {
                    ast_svtp(node, node->symbolNode->symbol);
                    if (node->value_type != TP_UNKNOWN) {
                        context->changed = 1;
                    }

                    if (node->node_type == VAR_SYMBOL && child1) {
                        /* We have array parameters (subscript or stem-style) */
                        n1 = child1;
                        while (n1) {
                            if (n1->value_type == TP_VOID && !ast_nsib(n1)) {
                                /* The last parameter is VOID - this is a special case, we
                                 * are returning the number of elements as an integer */
                                break;
                            }

                            if (n1->node_type == INTEGER && !ast_nsib(n1) &&
                                node->symbolNode->symbol->dim_base &&
                                node->symbolNode->symbol->dim_base[ast_chdi(n1)] == 1 &&
                                node_to_integer(n1) == 0) {
                                /* Special case - last parameter and 1-base and is "0"
                                 * this is a syntax candy for VOID - returning the number of elements as an integer */
                                n1->value_type = TP_VOID;
                                n1->node_type = NOVAL;
                                break;
                            }

                            n1 = ast_nsib(n1);
                        }

                        if (n1 && n1->value_type == TP_VOID) {
                            /* The last parameter is VOID - this is a special case, we
                             * are returning the number of elements as an integer */
                            set_node_type(node, TP_INTEGER);
                        } else {
                            /* We are returning the array element */
                            /* Ensure the node reflects the element (scalar) type and class */
                            if (node->symbolNode && node->symbolNode->symbol->value_dims > ast_nchd(node)) {
                                node->value_dims = node->symbolNode->symbol->value_dims - ast_nchd(node);
                            } else {
                                node->value_dims = 0;
                            }
                            node->target_dims = node->value_dims;
                            ast_rttp(node);
                            /* Copy over element value type/class from the symbol if needed */
                            if (node->symbolNode && node->symbolNode->symbol) {
                                /* Keep the underlying element ValueType */
                                node->value_type = node->symbolNode->symbol->type;
                                if (node->value_class) { free(node->value_class); node->value_class = 0; }
                                if (node->symbolNode->symbol->value_class) {
                                    node->value_class = malloc(strlen(node->symbolNode->symbol->value_class) + 1);
                                    strcpy(node->value_class, node->symbolNode->symbol->value_class);
                                }
                                /* Make target match value */
                                ast_rttp(node);
                            } else {
                                /* Reset Node Target Type to be the same as the node value type */
                                ast_rttp(node);
                            }
                            context->changed = 1;
                        }
                    }
                }
                break;

            case ASSIGN:
                if (node->value_type == TP_UNKNOWN) {
                    set_node_type(node, TP_VOID);
                    context->changed = 1;
                }
                if (child1->symbolNode && child1->symbolNode->symbol->type == TP_UNKNOWN) {
                    /* If the symbol does not have a known type yet - then determine it */
                    if (node->parent->node_type == REPEAT) {
                        /* Special logic for LOOP Assignment - type must be numeric */
                        child1->symbolNode->symbol->value_dims = 0;
                        if (child1->symbolNode->symbol->value_class) free(child1->symbolNode->symbol->value_class);
                        child1->symbolNode->symbol->value_class = 0;
                        child1->symbolNode->symbol->type = promotion[child2->value_type][TP_INTEGER];
                        if (child1->symbolNode->symbol->type != TP_UNKNOWN) {
                            context->changed = 1;
                            ast_svtp(child1, child1->symbolNode->symbol);
                        }
                    } else {
                        child1->symbolNode->symbol->type =
                                node_to_type(context, child2,
                                             &(child1->symbolNode->symbol->value_dims),
                                             &(child1->symbolNode->symbol->dim_base),
                                             &(child1->symbolNode->symbol->dim_elements),
                                             &(child1->symbolNode->symbol->value_class));

                        if (child1->symbolNode->symbol->value_dims == 0 && child2->node_type != CLASS)
                            node_to_dims(child1, &(child1->symbolNode->symbol->value_dims),
                                         &(child1->symbolNode->symbol->dim_base), &(child1->symbolNode->symbol->dim_elements));

                        if (child1->symbolNode->symbol->type != TP_UNKNOWN) {
                            context->changed = 1;
                            ast_svtp(child1, child1->symbolNode->symbol);
                        }
                    }
                }
                break;

            case CONST_SYMBOL:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, TP_STRING);
                }
                break;

            case INTEGER:
            case OP_ARGS:
                if (node->value_type == TP_UNKNOWN && node->parent->node_type != NODE_REGISTER) {
                    context->changed = 1;
                    set_node_type(node, TP_INTEGER);
                }
                break;

            case STRING:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, TP_STRING);
                }
                break;

            case FLOAT:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, TP_FLOAT);
                }
                break;

            case DECIMAL:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, TP_DECIMAL);
                }
                break;

            case VOID:
            case NOVAL:
            case RANGE:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, TP_VOID);
                }
                break;

            case CLASS:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;

                    node->value_type = node_to_type(context, node, &(node->value_dims),
                                                    &(node->value_dim_base), &(node->value_dim_elements),
                                                    &(node->value_class));

                    /* Reset Node Target Type to be the same as the node value type */
                    ast_rttp(node);
                }
                break;

            case DEFINE:
                if (node->value_type == TP_UNKNOWN) {
                    set_node_type(node, TP_VOID);
                    context->changed = 1;
                }
                if (child1->symbolNode->symbol->type == TP_UNKNOWN) {
                    /* If the symbol does not have a known type yet - then determine it */
                    child1->symbolNode->symbol->type =
                            node_to_type(context, child2,
                                         &(child1->symbolNode->symbol->value_dims),
                                         &(child1->symbolNode->symbol->dim_base),
                                         &(child1->symbolNode->symbol->dim_elements),
                                         &(child1->symbolNode->symbol->value_class));
                    if (child1->symbolNode->symbol->type != TP_UNKNOWN) {
                        context->changed = 1;
                        ast_svtp(child1, child1->symbolNode->symbol);
                    }
                }
                break;

            case ARG:
                if (node->value_type == TP_UNKNOWN) {
                    if (node->child->node_type == VARG || node->child->node_type == VARG_REFERENCE) {
                        /* Ellipse */
                        context->changed = 1;
                        child1->value_type = node_to_type(context, child2,
                                                          &(child1->value_dims),
                                                          &(child1->value_dim_base),
                                                          &(child1->value_dim_elements),
                                                          &(child1->value_class));
                        ast_rttp(child1);
                        ast_svtn(node, child1);
                    }
                    else {
                        /* Normal Arg */
                        if (child1->symbolNode) {
                            context->changed = 1;
                            if (child1->symbolNode->symbol->type == TP_UNKNOWN) {
                                /* If the symbol does not have a known type yet */
                                child1->symbolNode->symbol->type = node_to_type(context, child2,
                                                                                &(child1->symbolNode->symbol->value_dims),
                                                                                &(child1->symbolNode->symbol->dim_base),
                                                                                &(child1->symbolNode->symbol->dim_elements),
                                                                                &(child1->symbolNode->symbol->value_class));
                            }
                            ast_svtp(child1, child1->symbolNode->symbol);
                            ast_svtn(node, child1);
                        }
                    }
                }
                break;

            case OP_ARG_VALUE:
                if (node->value_type == TP_UNKNOWN) {
                    /* Find the procedure ellipse type node */
                    n1 = ast_proc(node); /* Procedure node */
                    n1 = ast_chld(n1, ARGS, 0); /* ARGS Node */
                    if (ast_nchd(n1) == 0) break; /* No ARGS at all! */
                    n1 = ast_chdn(n1, ast_nchd(n1) - 1); /* Last ARG */
                    n1 = n1->child; /* The VARG */
                    if (n1->node_type != VARG) break; /* No VARGS */

                    n1 = n1->sibling; /* This is the CLASS of the VARGS */
                    ast_svtn(node, n1);
                    context->changed = 1;
                }
                break;

            default:;
        }

        context->current_scope = node->scope;
    }

    return result_normal;
}

walker_result type_safety_walker(walker_direction direction,
                                        ASTNode* node,
                                        void *payload) {

    Context *context = (Context*)payload;
    ASTNode *child1, *child2, *n1, *n2;
    int val, ix;

    if (direction == in) {
        /* IN - TOP DOWN */
        context->current_scope = node->scope;
    }
    else {
        /* OUT - BOTTOM UP */
        child1 = ast_chdn(node, 0);
        child2 = ast_chdn(node, 1);

        switch (node->node_type) {
            case PROCEDURE:
                if (strcmp(node->symbolNode->symbol->name,"main") == 0) {
                    /* Validate main() return values */
                    if (node->value_type != TP_VOID && node->value_type != TP_INTEGER) {
                        /* Must be an string array */
                        mknd_err(ast_chld(node,CLASS,0),"MAIN_RETURNS_INTEGER");
                    }
                }
                break;

            case OP_AND:
            case OP_OR:
                set_node_target_type(child1, TP_BOOLEAN);
                set_node_target_type(child2, TP_BOOLEAN);
                break;

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
                set_node_target_type(child1, max_type(node));
                set_node_target_type(child2, max_type(node));
                break;

            case OP_CONCAT:
            case OP_SCONCAT:
            case OP_ADD:
            case OP_MINUS:
            case OP_MULT:
            case OP_POWER:
            case OP_DIV:
            case OP_IDIV:
            case OP_MOD:
                set_node_target_type(child1, node->value_type);
                set_node_target_type(child2, node->value_type);
                break;

            case OP_NOT:
                set_node_target_type(child1, node->value_type);
                break;

            case OP_PLUS:
            case OP_NEG:
                set_node_type(node, promotion[child1->value_type][TP_VOID]);
                set_node_target_type(child1, node->value_type);
                break;

            case VAR_SYMBOL:
                if (node->parent->node_type != NODE_REGISTER) {
                    if (node->symbolNode && node->symbolNode->symbol) {
                        if (node->value_type == TP_UNKNOWN) ast_svtp(node, node->symbolNode->symbol);
                    }
                    if (node->value_type == TP_UNKNOWN) mknd_err(node, "UNKNOWN_TYPE");
                }

                if (node->symbolNode && node->symbolNode->symbol && node->symbolNode->symbol->type != TP_UNKNOWN && ast_nchd(node) && !node->symbolNode->symbol->value_dims) {
                    mknd_err(node, "NOT_AN_ARRAY");
                }
                else if (child1) {
                    /* We have array parameters */

                    /* Set array parameter type to integer */
                    n1 = child1;
                    while (n1) {
                        if (n1->value_type == TP_VOID && !ast_nsib(n1)) {
                            /* The last parameter is VOID - this is a special case, we
                             * are returning the number of elements as an integer */
                            break;
                        }

                        if (n1->node_type == INTEGER && !ast_nsib(n1) &&
                            node->symbolNode && node->symbolNode->symbol &&
                            node->symbolNode->symbol->dim_base &&
                            node->symbolNode->symbol->dim_base[ast_chdi(n1)] == 1 &&
                            node_to_integer(n1) == 0) {
                            /* Special case - last parameter and 1-base and is "0"
                             * this is a syntax candy for VOID - returning the number of elements as an integer */
                            n1->value_type = TP_VOID;
                            break;
                        }

                        set_node_target_type(n1, TP_INTEGER);

                        if (n1->node_type == INTEGER && n1->parent->symbolNode && n1->parent->symbolNode->symbol && n1->parent->symbolNode->symbol->dim_base) {
                            /* As a constant integer we can check it is in range */
                            val = node_to_integer(n1);
                            ix = ast_chdi(n1);

                            if (val < n1->parent->symbolNode->symbol->dim_base[ix])
                                mknd_err(n1, "OUT_OF_RANGE");

                            else if (n1->parent->symbolNode->symbol->dim_elements[ix]) {
                                /* There is a max number of elements - so check it */
                                if (val > n1->parent->symbolNode->symbol->dim_base[ix] +
                                                      n1->parent->symbolNode->symbol->dim_elements[ix] - 1)
                                    mknd_err(n1, "OUT_OF_RANGE");
                            }
                        }

                        n1 = ast_nsib(n1);
                    }

                    if (n1 && n1->value_type == TP_VOID) {
                        /* The last parameter is VOID - this is a special case, we
                         * are returning the number of elements as an integer */

                        /* We can have fewer parameters when we are getting the number of elements */
                        if (node->symbolNode && node->symbolNode->symbol && node->symbolNode->symbol->value_dims < ast_nchd(node))
                            mknd_err(node, "ARRAY_DIMS_MISMATCH");

                        set_node_type(node, TP_INTEGER);
                    }

                    else {
                        /* We are returning the array element */
                        if (node->symbolNode && node->symbolNode->symbol && node->symbolNode->symbol->value_dims < ast_nchd(node))
                            mknd_err(node, "ARRAY_DIMS_MISMATCH");

                        if (node->symbolNode && node->symbolNode->symbol && node->symbolNode->symbol->value_dims > ast_nchd(node)) {
                            node->value_dims = node->symbolNode->symbol->value_dims - ast_nchd(node);
                        } else {
                            node->value_dims = 0;
                        }
                        node->target_dims = node->value_dims;
                        ast_rttp(node);
                    }
                }
                break;

            case CLASS:
                if (node->value_dims) {
                    /* We are an array */
                    if (node->parent->node_type == PROCEDURE) {
                        /* In a procedure definition the array params should be null */
                        n1 = child1;
                        while (n1) {
                            set_node_target_type(n1, TP_VOID);
                            n1 = ast_nsib(n1);
                        }
                    }
                }
                // else we are a class TODO

                break;

            case DEFINE:
                if (child1->symbolNode->symbol->type == TP_UNKNOWN) {
                    /* If the symbol does not have a known type yet - then determine it */
                    child1->symbolNode->symbol->type =
                            node_to_type(context, child2,
                                         &(child1->symbolNode->symbol->value_dims),
                                         &(child1->symbolNode->symbol->dim_base),
                                         &(child1->symbolNode->symbol->dim_elements),
                                         &(child1->symbolNode->symbol->value_class));
                    ast_svtp(child1, child1->symbolNode->symbol);

                    if (child1->symbolNode->symbol->type == TP_UNKNOWN) mknd_err(node, "UNKNOWN_TYPE");
                }

                if (ast_nchd(child1)) {
                    /* We have unexpected array parameters */
                    mknd_err(ast_chdn(child1,0), "INVALID_LHS_ARRAY");
                }

                ast_sttn(child2, child1);
                validate_node_promotion(child2);
                ast_svtn(node, child1);
                ast_rttp(node);
                break;

            case ASSIGN:
                if (child2->value_type == TP_VOID) {
                    mknd_err(child2, "RETURNS_VOID");
                }
                else {
                    if (child1->symbolNode && child1->symbolNode->symbol->type == TP_UNKNOWN) {
                        /* If the symbol does not have a known type yet - then determine it */
                        child1->symbolNode->symbol->type =
                                node_to_type(context, child2,
                                             &(child1->symbolNode->symbol->value_dims),
                                             &(child1->symbolNode->symbol->dim_base),
                                             &(child1->symbolNode->symbol->dim_elements),
                                             &(child1->symbolNode->symbol->value_class));

                        if (child1->symbolNode->symbol->value_dims == 0 && child2->node_type != CLASS)
                            node_to_dims(child1, &(child1->symbolNode->symbol->value_dims),
                                         &(child1->symbolNode->symbol->dim_base), &(child1->symbolNode->symbol->dim_elements));
                    }
                    if (child1->symbolNode) ast_svtp(child1, child1->symbolNode->symbol);

                    if (!child1->symbolNode || child1->symbolNode->symbol->type == TP_UNKNOWN) mknd_err(node, "UNKNOWN_TYPE");

                    if (ast_nchd(child1)) {
                        /* We have array parameters */
                        /* Set array parameter type to integer */
                        n1 = child1->child;
                        while (n1) {
                            set_node_target_type(n1, TP_INTEGER);

                            if (n1->node_type == INTEGER) {
                                /* As a constant integer we can check it is in range */
                                val = node_to_integer(n1);
                                ix = ast_chdi(n1);

                                if (ix < (int)child1->value_dims) {
                                    if (val < n1->parent->symbolNode->symbol->dim_base[ix])
                                        mknd_err(n1, "OUT_OF_RANGE");

                                    else if (n1->parent->symbolNode->symbol->dim_elements[ix]) {
                                        /* There is a max number of elements - so check it */
                                        if (val > n1->parent->symbolNode->symbol->dim_base[ix] +
                                                  n1->parent->symbolNode->symbol->dim_elements[ix] - 1)
                                            mknd_err(n1, "OUT_OF_RANGE");
                                    }
                                }
                            }

                            n1 = n1->sibling;
                        }

                        if (!child1->value_dims)
                            mknd_err(child1, "NOT_AN_ARRAY");
                        else if (child1->value_dims != ast_nchd(child1)) mknd_err(node, "ARRAY_DIMS_MISMATCH");

                        child1->value_dims = 0; /* We are a single value */
                        child1->target_dims = 0;
                    }

                    ast_sttn(child2, child1);
                    validate_node_promotion(child2);
                    ast_svtn(node, child1);
                }
                break;

            case ARGS:
                if (strcmp(node->parent->symbolNode->symbol->name,"main") == 0) {
                    /* Validate the signature of the main() functions */
                    if (ast_nchd(node) == 0) break; /* A main() can ignore arguments */
                    if (ast_nchd(node) != 1) {
                        /* Should only have 1 argument */
                        mknd_err(node,"INVALID_MAIN_ARGS");
                        break;
                    }
                    if (child1->value_dims != 1) {
                        /* Must be a 1 dimensional array */
                        mknd_err(node,"INVALID_MAIN_ARGS");
                        break;
                    }
                    if (child1->value_dim_elements[0] != 0 ) {
                        /* Must be a dynamic array */
                        mknd_err(node,"INVALID_MAIN_ARGS");
                        break;
                    }
                    if (child1->value_type != TP_STRING ) {
                        /* Must be an string array */
                        mknd_err(node,"INVALID_MAIN_ARGS");
                        break;
                    }
                }
                break;

            case ARG:
                if (child1->symbolNode && child1->symbolNode->symbol->type == TP_UNKNOWN) {
                    /* If the symbol does not have a known type yet - then determine it */
                    child1->symbolNode->symbol->type =
                            node_to_type(context, child2,
                                         &(child1->symbolNode->symbol->value_dims),
                                         &(child1->symbolNode->symbol->dim_base),
                                         &(child1->symbolNode->symbol->dim_elements),
                                         &(child1->symbolNode->symbol->value_class));
                }
                if (child1->symbolNode) ast_svtp(child1, child1->symbolNode->symbol);

                ast_svtn(node, child1);
                ast_rttp(node);

                if (ast_chld(node->parent->parent, INSTRUCTIONS, NOP)->node_type == INSTRUCTIONS) {
                    /* In a function implementation - in this case the optional flag '?' is invalid for a class type as a
                     * definition needs to know the default value that can only be defined by the expression on the
                     * right-hand-side */
                    if (child2->node_type == CLASS && node->is_opt_arg && !node->value_dims) {
                        /* Optional but the CLASS doesn't give the needed default value */
                        /* NOTE Arrays are an exception - their default value is a "blank" array */
                        mknd_err(node, "NO_DEFAULT_VALUE");
                    }
                }
                break;

            case OP_ARG_VALUE:
            case OP_ARG_IX_EXISTS:
                n1 = ast_proc(node);
                if (n1 && n1->symbolNode && n1->symbolNode->symbol) {
                    if (!n1->symbolNode->symbol->has_vargs) mknd_err(node,"NO_PROC_VARGS");
                }
                set_node_target_type(child1,TP_INTEGER);

                if (child1->node_type == INTEGER) {
                    /* As a constant integer we can check it is in range
                     * In fact this never works as the node will be OP_NEG (but leaving it here in case it is needed in the future) */
                    val = node_to_integer(child1);
                    if (val < 1)
                        mknd_err(child1, "OUT_OF_RANGE");
                }
                break;

            case OP_ARGS:
                n1 = ast_proc(node);
                if (n1 && n1->symbolNode && n1->symbolNode->symbol) {
                    if (!n1->symbolNode->symbol->has_vargs) mknd_err(node,"NO_PROC_VARGS");
                }
                break;

            case SAY:
                if (child1) set_node_target_type(child1, TP_STRING);
                break;

            case RETURN:
                /* Type is the scope > procedure > type */
                if (context->current_scope->defining_node && context->current_scope->defining_node->symbolNode) {
                    ast_svtp(node, context->current_scope->defining_node->symbolNode->symbol);
                } else {
                    /* Fallback to VOID if no defining node or symbol */
                    set_node_type(node, TP_VOID);
                }
                if (node->value_type == TP_VOID) {
                    if (child1) mknd_err(child1, "EXTRANEOUS_RETVAL");
                }
                else {
                    if (child1) {
                        ast_sttn(child1, node);
                        validate_node_promotion(child1);
                    }
                    else mknd_err(node, "RETVAL_MISSING");
                }
                break;

            case IF:
                if (child1) set_node_target_type(child1, TP_BOOLEAN);
                break;

                /* Loops */
            case TO:
            case BY:
                /* The TO/BY value type needs to be the same as the assigment type */
                if (child1) set_node_target_type(child1, node->parent->child->value_type);
                set_node_type(node, node->parent->child->value_type);
                break;

            case FOR:
                set_node_target_type(child1, TP_INTEGER);
                set_node_type(node, TP_INTEGER);
                break;

            case UNTIL:
            case WHILE:
                if (child1) set_node_target_type(child1, TP_BOOLEAN);
                set_node_type(node, TP_BOOLEAN);
                break;

            case LEAVE:
            case ITERATE:
                /* Link to relevant DO */
                if (node->child) {
                    /* Symbol specified - so we need to find it */
                    n1 = node->parent;
                    while (1) {
                        if (!n1) {
                            mknd_err(node, "INVALID_CONTROL_VARIABLE"); /* 28.3 */
                            break;
                        }
                        else if (n1->node_type == DO) {
                            /* Find the ASSIGN */
                            n2 = n1->child; /* REPEAT */
                            n2 = n2->child; /* First child of REPEAT */
                            while (n2) {
                                if (n2->node_type == ASSIGN) {
                                    /* Same Symbol? */
                                    if (n2->child->symbolNode && node->child->symbolNode &&
                                        n2->child->symbolNode->symbol == node->child->symbolNode->symbol) {
                                        node->association = n1;
                                        goto found;
                                    }
                                    else break;
                                }
                                n2 = n2->sibling; /* Next REPEAT Child */
                            }
                        }
                        n1 = n1->parent;
                    }
                    found:;
                }
                else {
                    /* Symbol not specified - just find inner DO */
                    n1 = node->parent;
                    while (1) {
                        if (!n1) {
                            mknd_err(node, "NOT_IN_LOOP"); /* 28.1, 28.2 */
                            break;
                        }
                        else if (n1->node_type == DO) {
                            node->association = n1;
                            break;
                        }
                        n1 = n1->parent;
                    }
                }
                break;


            case STRING:
                if (ast_nchd(node)) mknd_err(ast_chdn(node,0), "TAKENCONSTANT_ARRAY");
                break;

            default:;
        }

        context->current_scope = node->scope;
    }

    return result_normal;
}

/* Fix up types for function arguments and OP_ARG_VALUE nodes
 * Needs to be done after the procedure arguments have been processed */
walker_result func_type_safety_walker(walker_direction direction,
                                             ASTNode* node,
                                             void *payload) {

    Context *context = (Context*)payload;
    ASTNode *n1, *n2;
    int arg_num;

    if (direction == in) {
        /* IN - TOP DOWN */
        context->current_scope = node->scope;
    }
    else {
        /* OUT - BOTTOM UP */
        switch (node->node_type) {

            case FUNCTION:
            case MEMBER_CALL:
            case FACTORY_CALL:
                /* Process all the arguments */
                if (node->node_type == MEMBER_CALL) n1 = node->child->sibling; /* Skip Instance */
                else n1 = node->child;

                if  (node->symbolNode && sym_nond(node->symbolNode->symbol) > 0) {
                    n2 = sym_trnd(node->symbolNode->symbol, 0)->node;
                    /* n2 is PROCEDURE/METHOD/FACTORY. Go to the first arg */
                    if (n2 && (n2->node_type == PROCEDURE || n2->node_type == METHOD || n2->node_type == FACTORY)) {
                        n2 = ast_chld(n2, ARGS, 0);
                        if (n2) n2 = n2->child;
                    } else n2 = 0;
                }
                else n2 = 0;
                /* Check each argument */
                arg_num = 0;
                while (n1) {
                    arg_num++;
                    if (!n2) {
                        /* Its not an error for the first NOVAL argument */
                        if (arg_num > 1 || n1->node_type != NOVAL) mknd_err(n1, "UNEXPECTED_ARGUMENT, %d", arg_num);
                        else if (n1->node_type == NOVAL) {
                            /* Prune the unwanted NOVAL - the parser grammar just added it */
                            ASTNode *to_del = n1;
                            n1 = n1->sibling;
                            ast_del(to_del);
                            break;
                        }
                        break;
                    }

                    if (n2->child->node_type == VARG || n2->child->node_type == VARG_REFERENCE) {
                        /* Last ellipsis */
                        n1->is_opt_arg = 0;

                        if (n1->node_type == NOVAL) {
                            if (n1->sibling) {
                                /* If n1 is not the last argument then it can't be NOVAL */
                                mknd_err(n1, "ARGUMENT_REQUIRED, %d, \"...\"", arg_num);
                            }
                            else {
                                /* Prune the unwanted NOVAL - the parser grammar just added it */
                                ast_del(n1);
                            }
                        } else {
                            ast_sttn(n1, n2);
                            validate_node_promotion(n1);


                            if (n2->child->node_type == VAR_REFERENCE) {
                                n1->is_ref_arg = 1;
                                if (n1->symbolNode) {
                                    validate_node_promotion_for_ref(n1);

                                    /* Mark as write access for the optimiser */
                                    n1->symbolNode->writeUsage = 1;
                                }
                            }
                        }
                    }

                    else {
                        /* Normal Argument */
                        n1->is_opt_arg = n2->is_opt_arg;
                        if (n1->node_type == NOVAL) {
                            ast_svtn(n1, n2);
                            if (!n1->is_opt_arg) {
                                mknd_err(n1, "ARGUMENT_REQUIRED, %d, \"%s\"", arg_num,
                                         n2->child->symbolNode->symbol->name);
                            }
                        } else {
                            ast_sttn(n1, n2);
                            validate_node_promotion(n1);
                        }

                        if (n2->child->node_type == VAR_REFERENCE) {
                            n1->is_ref_arg = 1;
                            if (n1->symbolNode) {
                                validate_node_promotion_for_ref(n1);

                                /* Mark as write access for the optimiser */
                                n1->symbolNode->writeUsage = 1;
                            }
                        }
                        n2 = n2->sibling;
                    }
                    n1 = n1->sibling;
                }

                while (n2) {
                    /* Skip an ellipse - should be the last argument, but this does not assume it */
                    if (n2->child->node_type == VARG || n2->child->node_type == VARG_REFERENCE) {
                        n2 = n2->sibling;
                        arg_num++;
                    }
                    else {
                        arg_num++;
                        n1 = ast_ft(context, NOVAL);
                        ast_svtn(n1, n2);
                        add_ast(node, n1);
                        n1->is_opt_arg = n2->is_opt_arg;
                        n1->is_ref_arg = n2->is_ref_arg;
                        n1->is_const_arg = n2->is_const_arg;
                        if (!n1->is_opt_arg) {
                            mknd_err(n1, "ARGUMENT_REQUIRED, %d, \"%s\"", arg_num, n2->child->symbolNode->symbol->name);
                        }
                        n2 = n2->sibling;
                    }
                }
                break;

            case REXX_UNIVERSE:
            case PROGRAM_FILE:
            case IMPORTED_FILE:
            case INSTRUCTIONS:
            case PROCEDURE:
            case EXPOSED:
            case SAY:
            case RETURN:
            case EXIT:
            case IF:
            case DO:
            case FOR:
            case WHILE:
            case UNTIL:
            case REPEAT:
            case ITERATE:
            case LEAVE:
            case NOP:
            case OPTIONS:
            case REXX_OPTIONS:
            case IMPORT:
            case NAMESPACE:
            case PARSE:
            case UPPER:
            case PULL:
            case ADDRESS:
            case ENVIRONMENT:
            case DEC_DIGITS:
            case DEC_FORM:
            case DEC_FUZZ:
            case DEC_CASE:
            case DEC_STANDARD:
            case CLASS_DEF:
            case METHOD:
            case FACTORY:
                if (node->value_type == TP_UNKNOWN) {
                    set_node_type(node, TP_VOID);
                    context->changed = 1;
                }
                break;

            default:;
        }

        context->current_scope = node->scope;
    }

    return result_normal;
}

walker_result float2decimal_walker(walker_direction direction,
                                           ASTNode* node,
                                           void *payload) {

    Context *context = (Context*)payload;

    if (direction == in) {
        /* IN - TOP DOWN */
        context->current_scope = node->scope;
    }
    else {
        /* OUT - BOTTOM UP */
        switch (node->node_type) {
            case FLOAT:
                context->changed = 1;
                node->node_type = DECIMAL;
                break;
            case CLASS:
                if (node->node_string_length == strlen(".float")) {
                    if (strncmp(node->node_string, ".float", node->node_string_length) == 0) {
                        /* This is a .float class - convert to decimal */
                        context->changed = 1;
                        ast_str(node, ".decimal");
                    }
                }
                break;

            default:;
        }

        context->current_scope = node->scope;
    }

    return result_normal;
}

walker_result decimal2float_walker(walker_direction direction,
                                           ASTNode* node,
                                           void *payload) {

    Context *context = (Context*)payload;

    if (direction == in) {
        /* IN - TOP DOWN */
        context->current_scope = node->scope;
    }
    else {
        /* OUT - BOTTOM UP */
        switch (node->node_type) {
            /* TODO remove digits instructons -> NOP as these are irrelevant for float - consider a warning */
            case DECIMAL:
                context->changed = 1;
                node->node_type = FLOAT;
                break;
            case CLASS:
                if (node->node_string_length == strlen(".decimal")) {
                    if (strncmp(node->node_string, ".decimal", node->node_string_length) == 0) {
                        /* This is a .decimal class - convert to float */
                        context->changed = 1;
                        ast_str(node, ".float");
                    }
                }
                break;

            default:;
        }

        context->current_scope = node->scope;
    }

    return result_normal;
}
