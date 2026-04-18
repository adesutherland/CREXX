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
#include "rxcp_val.h"

/* Suppress errors and warnings unless it is the final pass */
#undef mknd_err
#undef mknd_err_unique
#undef mknd_war
#define mknd_err(n, ...) ((!(context) || (context)->is_final_pass) ? (mknd_err)((n), __VA_ARGS__) : (n))
#define mknd_err_unique(n, ...) ((!(context) || (context)->is_final_pass) ? (mknd_err_unique)((n), __VA_ARGS__) : (n))
#define mknd_war(n, ...) ((!(context) || (context)->is_final_pass) ? (mknd_war)((n), __VA_ARGS__) : (n))
#include "rxcp_util.h"
#include "rxbin.h" /* Needed for rxvmvars.h */
#include "rxvmvars.h"

static int origin_subtree_has_error(ASTNode *node) {
    ASTNode *current;

    current = node;
    while (current) {
        if (ast_hase(current)) return 1;
        switch (current->node_type) {
            case ASSIGN:
            case DEFINE:
            case REPEAT:
            case DO:
            case SAY:
            case CALL:
            case RETURN:
                return 0;
            default:
                current = current->parent;
        }
    }

    return 0;
}

/* Validates a node promotion is correct for a call by reference (of symbols) adding error nodes if not */
void validate_node_promotion_for_ref(Context *context, ASTNode* node) {
    size_t i;

    if (node->target_type == TP_UNKNOWN || node->value_type == TP_UNKNOWN) {
        return; /* Will be validated later or caught by type_safety_walker */
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
    ast_set_value_type(0, node, type, 0, 0, 0, 0);
    ast_set_target_type(0, node, type, 0, 0, 0, 0);
}

/* Set the target value to a simple target_type (not an array or class name)
 * and validates that it is convertable from the nodes value target_type */
static void set_node_target_type(Context* context, ASTNode* node, ValueType target_type) {
    ast_set_target_type(0, node, target_type, 0, 0, 0, 0);
    validate_node_promotion(context, node);
}

static ASTNode *find_enclosing_block_expr(ASTNode *node) {
    if (node) node = node->parent;
    while (node) {
        if (node->node_type == BLOCK_EXPR) return node;
        node = node->parent;
    }
    return 0;
}

static void copy_value_type(__attribute__((unused)) Context *context, ASTNode *dest, ASTNode *src) {
    ast_set_value_type(0, dest, src->value_type, src->value_dims,
                       src->value_dim_base, src->value_dim_elements, src->value_class);
    ast_set_target_type(0, dest, src->value_type, src->value_dims,
                        src->value_dim_base, src->value_dim_elements, src->value_class);
}

static int same_value_type(ASTNode *left, ASTNode *right) {
    size_t i;

    if (left->value_type != right->value_type || left->value_dims != right->value_dims) return 0;

    for (i = 0; i < left->value_dims; i++) {
        if (left->value_dim_base[i] != right->value_dim_base[i]) return 0;
        if (left->value_dim_elements[i] != right->value_dim_elements[i]) return 0;
    }

    if (left->value_class && right->value_class) {
        return strcmp(left->value_class, right->value_class) == 0;
    }

    return left->value_class == 0 && right->value_class == 0;
}

/* This walker does the basic value types of operations
 * No errors generated - just simple "guesses" as to types */
/* Propagate types from function signature to arguments and promote unknown symbols */
void infer_arguments(Context *context, ASTNode *node) {
    ASTNode *n1, *n2;
    int arg_num;

    /* Process all the arguments */
    if (node->node_type == MEMBER_CALL) n1 = node->child->sibling; /* Skip Instance */
    else n1 = node->child;

    if (node->symbolNode && sym_nond(node->symbolNode->symbol) > 0) {
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
        if (!n2) break;

        if (n2->child->node_type == VARG || n2->child->node_type == VARG_REFERENCE) {
            if (n1->node_type != NOVAL) {
                ast_sttn(n1, n2);
                promote_symbol_from_target(0, n1);
            }
        }
        else {
            /* Normal Argument */
            if (n1->node_type != NOVAL) {
                ast_sttn(n1, n2);
                promote_symbol_from_target(0, n1);
            }
            n2 = n2->sibling;
        }
        n1 = n1->sibling;
    }
}

/* Reset node types at the start of each validation pass so they can be re-evaluated on a clean slate */
walker_result clear_node_types_walker(walker_direction direction,
                                             ASTNode* node,
                                             __attribute__((unused)) void *payload) {
    if (direction == in) {
        if (node->node_type != INTEGER && node->node_type != FLOAT &&
            node->node_type != STRING && node->node_type != DECIMAL &&
            node->node_type != CONSTANT && node->node_type != CLASS &&
            node->node_type != OP_ARGS && node->node_type != VOID &&
            node->node_type != PROGRAM_FILE && node->node_type != PROCEDURE &&
            node->node_type != CLASS_DEF && node->node_type != METHOD) {
            
            node->value_type = TP_UNKNOWN;
            node->target_type = TP_UNKNOWN;
            if (node->value_class) { free(node->value_class); node->value_class = 0; }
            if (node->target_class) { free(node->target_class); node->target_class = 0; }
            node->value_dims = 0;
            node->target_dims = 0;
        }

        if (node->node_type == NOVAL) {
            node->is_opt_arg = 0;
            node->is_ref_arg = 0;
            node->is_const_arg = 0;
        }
    }
    return result_normal;
}

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
                    /* context->changed_flags |= FLAG_VAL_TYPE; */ set_node_type(node, TP_BOOLEAN);
                }
                break;

            case OP_CONCAT:
            case OP_SCONCAT:
                if (node->value_type == TP_UNKNOWN) {
                    /* context->changed_flags |= FLAG_VAL_TYPE; */ set_node_type(node, TP_STRING);
                }
                break;

            case OP_ADD:
            case OP_MINUS:
            case OP_MULT:
            case OP_POWER:
                if (node->value_type == TP_UNKNOWN) {
                    /* context->changed_flags |= FLAG_VAL_TYPE; */ ValueType type = promotion[node_type(child1)][node_type(child2)];
                    if (type == TP_UNKNOWN) type = TP_INTEGER; /* Default to integer */
                    set_node_type(node, type);
                }
                break;

            case OP_DIV:
                if (node->value_type == TP_UNKNOWN) {
                    /* Under OPTIONS NUMERIC_COMMON, `/` keeps integer semantics for int/int. */
                    ValueType type = promotion[node_type(child1)][node_type(child2)];
                    if (type == TP_INTEGER && node->context && !node->context->numeric_standard) {
                        set_node_type(node, TP_INTEGER);
                        break;
                    }
                    type = promotion[type][TP_FLOAT]; /* Ensure at least FLOAT otherwise */
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
                        /* context->changed_flags |= FLAG_VAL_TYPE; */ }
                }
                break;

            case OP_NOT:
                if (node->value_type == TP_UNKNOWN) {
                    /* context->changed_flags |= FLAG_VAL_TYPE; */ set_node_type(node, TP_BOOLEAN);
                }
                break;

            case OP_PLUS:
            case OP_NEG:
                if (node->value_type == TP_UNKNOWN) {
                    /* context->changed_flags |= FLAG_VAL_TYPE; */ set_node_type(node, promotion[node_type(child1)][TP_VOID]);
                }
                break;

            case FUNCTION:
                if (node->symbolNode) { /* Otherwise, an error node will have been added */
                    if (node->value_type == TP_UNKNOWN) {
                        /* context->changed_flags |= FLAG_VAL_TYPE; */ ast_svtp(node, node->symbolNode->symbol);
                    }
                    if (node->symbolNode->symbol && node->symbolNode->symbol->symbol_type == FUNCTION_SYMBOL) {
                        Symbol *fsym = node->symbolNode->symbol;
                        int is_method = 0;
                        if (sym_nond(fsym) > 0) {
                            SymbolNode *defsn = sym_trnd(fsym, 0);
                            if (defsn && defsn->node && (defsn->node->node_type == METHOD || defsn->node->node_type == FACTORY)) {
                                is_method = 1;
                            }
                        }
                        if (is_method) {
                            /* We are calling a method. If we are in a method/factory context and this is a simple FUNCTION node
                             * we must rewrite it to a MEMBER_CALL to implicitly pass §this */
                            ASTNode *this_node = ast_f(context, VAR_SYMBOL, node->token);
                            char *this_str = malloc(7);
                            strcpy(this_str, "\xc2\xa7" "this");
                            ast_sstr(this_node, this_str, 6);
                            Symbol *this_sym = sym_lrsv(context->current_scope, this_node);
                            if (this_sym) {
                                sym_adnd(this_sym, this_node, 1, 0);
                            }

                            node->node_type = MEMBER_CALL;

                            /* Insert this_node as the first child */
                            if (node->child) {
                                this_node->sibling = node->child;
                                node->child = this_node;
                                this_node->parent = node;
                            } else {
                                add_ast(node, this_node);
                            }

                            context->changed_flags |= FLAG_VAL_TYPE;
                            return result_normal;
                        }
                    }
                    infer_arguments(context, node);
                }
                break;

            case MEMBER_CALL:
                if (ast_chld(node, ERROR, 0)) break;
                if (node->value_type == TP_UNKNOWN) {
                    ASTNode *instance = ast_chdn(node, 0);
                    const char *cname = instance->value_class;
                    Symbol *class_sym = 0;
                    Symbol *method_sym = 0;
                    if (!cname && instance->symbolNode && instance->symbolNode->symbol) {
                        cname = instance->symbolNode->symbol->value_class;
                    }
                    if (instance->value_type == TP_OBJECT && cname) {
                        /* Resolve the Class Symbol from the instance's class name */
                        if (cname && cname[0] == '.') cname++;
                        ASTNode dummy = {0};
                        dummy.node_string = (char*)cname;
                        dummy.node_string_length = strlen(cname);
                        class_sym = sym_rvfc(context->ast, &dummy);
                        if (class_sym && class_sym->symbol_type == CLASS_SYMBOL) {
                            method_sym = sym_lrsv(class_sym->defines_scope, node);
                        } else if (!context->changed_flags || context->is_final_pass) {
                            if (ensure_class_imported(context, cname, strlen(cname))) {
                                class_sym = sym_rvfc(context->ast, &dummy);
                                if (class_sym && class_sym->symbol_type == CLASS_SYMBOL) {
                                    method_sym = sym_lrsv(class_sym->defines_scope, node);
                                }
                            }
                        }

                        if (method_sym && method_sym->symbol_type == FUNCTION_SYMBOL) {
                            if (!node->symbolNode || node->symbolNode->symbol != method_sym) {
                                sym_adnd(method_sym, node, 1, 0);
                                context->changed_flags |= FLAG_VAL_TYPE;
                            }
                            ast_svtp(node, method_sym);
                            infer_arguments(context, node);
                        } else if (!context->changed_flags || context->is_final_pass) {
                            /* DOT-AS-INDEX MUTATION: transform tokens.i -> tokens[i] */
                            Symbol *index_sym = sym_rslv_tiered(context->current_scope, node);
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

                                /* Link to the same symbol as instance if it has one */
                                if (instance->symbolNode) sym_adnd(instance->symbolNode->symbol, node, 1, 0);

                                /* Set scalar element type */
                                {
                                    ValueType new_type = (instance->symbolNode && instance->symbolNode->symbol) ? instance->symbolNode->symbol->type : instance->value_type;
                                    char *new_class = (instance->symbolNode && instance->symbolNode->symbol && instance->symbolNode->symbol->value_class) ? instance->symbolNode->symbol->value_class : instance->value_class;
                                    ast_set_value_type(0, node, new_type, 0, 0, 0, new_class);
                                    ast_set_target_type(0, node, new_type, 0, 0, 0, new_class);
                                }

                                /* Delete children (instance and any args) */
                                while (node->child) {
                                    ASTNode *c = node->child;
                                    if (c->symbolNode) sym_dno(c->symbolNode->symbol, c);
                                    ast_del(c);
                                }

                                /* Add the resolved variable as the index child (subscript) */
                                add_ast(node, new_index_node);

                                context->changed_flags |= FLAG_VAL_TYPE; return result_normal;
                            }

                            if (class_sym && class_sym->symbol_type == CLASS_SYMBOL) {
                                if (context->after_rewrite) {
                                    if (node->node_string && (strcmp(node->node_string, "get") == 0 || strcmp(node->node_string, "set") == 0)) {
                                        mknd_err(node, "INVALID_PUBLIC_ATTRIBUTE");
                                    } else {
                                        mknd_err(node, "METHOD_NOT_FOUND");
                                    }
                                }
                            } else {
                                /* Defer error if imports may provide class stubs */
                                int has_import = 0;
                                if (context->ast && context->ast->child && context->ast->child->node_type == PROGRAM_FILE) {
                                    ASTNode *pfch = context->ast->child->child;
                                    while (pfch) { if (pfch->node_type == IMPORT) { has_import = 1; break; } pfch = pfch->sibling; }
                                }
                                if (has_import && !context->after_rewrite) {
                                    /* defer error on first pass */
                                } else {
                                    mknd_err(node, "CLASS_NOT_FOUND");
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
                            if (!node->symbolNode || node->symbolNode->symbol != factory_sym) {
                                sym_adnd(factory_sym, node, 1, 0);
                                context->changed_flags |= FLAG_VAL_TYPE;
                            }
                            ast_set_value_type(0, node, TP_OBJECT, 0, 0, 0, class_sym->name);
                            ast_set_target_type(0, node, TP_OBJECT, 0, 0, 0, class_sym->name);
                            infer_arguments(context, node);
                        } else if (!context->changed_flags || context->is_final_pass) {
                            mknd_err(node, "FACTORY_NOT_FOUND");
                        }
                    } else {
                        /* Try and import the class */
                        Symbol *import_cls = ensure_class_imported(context, node->node_string, node->node_string_length);
                        if (import_cls) {
                            /* Resolve again - it should be found now */
                            class_sym = sym_rvfc(context->ast, node);
                            if (class_sym && class_sym->symbol_type == CLASS_SYMBOL) {
                                ASTNode star_node;
                                memset(&star_node, 0, sizeof(ASTNode));
                                star_node.node_string = "\xc2\xa7" "factory";
                                star_node.node_string_length = 9;
                                Symbol *factory_sym = sym_lrsv(class_sym->defines_scope, &star_node);
                                if (factory_sym && factory_sym->symbol_type == FUNCTION_SYMBOL) {
                                    if (!node->symbolNode || node->symbolNode->symbol != factory_sym) {
                                        sym_adnd(factory_sym, node, 1, 0);
                                        context->changed_flags |= FLAG_VAL_TYPE;
                                    }
                                    ast_set_value_type(0, node, TP_OBJECT, 0, 0, 0, class_sym->name);
                                    ast_set_target_type(0, node, TP_OBJECT, 0, 0, 0, class_sym->name);
                                    infer_arguments(context, node);
                                }
                            } else {
                                context->changed_flags |= FLAG_VAL_TYPE; 
                            }
                        } else {
                            /* Defer error if imports may provide class stubs */
                            int has_import = 0;
                            if (context->ast && context->ast->child && context->ast->child->node_type == PROGRAM_FILE) {
                                ASTNode *pfch = context->ast->child->child;
                                while (pfch) { if (pfch->node_type == IMPORT) { has_import = 1; break; } pfch = pfch->sibling; }
                            }
                            if (has_import && !context->after_rewrite) {
                                /* defer error on first pass */
                            } else {
                                mknd_err(node, "CLASS_NOT_FOUND");
                            }
                        }
                    }
                }
                break;

            case VAR_SYMBOL:
            case VAR_TARGET:
            case EXIT_TOKEN:
                if (node->value_type == TP_UNKNOWN && node->symbolNode) {
                    ValueType old_type = node->value_type;
                    int old_dims = node->value_dims;
                    
                    /* Prevent object property access (which will be rewritten to get()) from eagerly copying the object's type.
                     * If it has children (indices) and the symbol is a scalar object, leave it as TP_UNKNOWN.
                     * This prevents 'a = obj.bar' from inferring 'a' as TP_OBJECT on the first pass before the rewrite.
                     */
                    int skip_svtp = 0;
                    if ((node->node_type == VAR_SYMBOL || node->node_type == VAR_TARGET) && 
                        node->symbolNode->symbol->value_dims == 0 && 
                        child1 != NULL) {
                        skip_svtp = 1;
                    }
                    /* printf("DEBUG: VAR_SYMBOL '%s', skip_svtp=%d, type=%d, dims=%d, child1=%p\n", node->node_string, skip_svtp, node->symbolNode->symbol->type, node->symbolNode->symbol->value_dims, (void*)child1); */
                    
                    if (!skip_svtp) {
                        ast_svtp(node, node->symbolNode->symbol);
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
                            {
                                int new_dims = 0;
                                ValueType new_type = node->value_type;
                                char *new_class = node->value_class;
                                if (node->symbolNode && node->symbolNode->symbol) {
                                    if (node->symbolNode->symbol->value_dims > ast_nchd(node)) {
                                        new_dims = node->symbolNode->symbol->value_dims - ast_nchd(node);
                                    }
                                    if (!skip_svtp) {
                                        new_type = node->symbolNode->symbol->type;
                                        new_class = node->symbolNode->symbol->value_class;
                                    } else {
                                        new_type = TP_UNKNOWN;
                                        new_class = NULL;
                                    }
                                }
                                ast_set_value_type(0, node, new_type, new_dims, 0, 0, new_class);
                                ast_set_target_type(0, node, new_type, new_dims, 0, 0, new_class);
                            }
                        }
                    }

                    if (node->value_type != old_type || node->value_dims != old_dims) { /* context->changed_flags |= FLAG_VAL_TYPE; */ }
                }
                break;

            case ASSIGN:
                if (node->value_type == TP_UNKNOWN) {
                    set_node_type(node, TP_VOID);

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
                            /* context->changed_flags |= FLAG_VAL_TYPE; */ ast_svtp(child1, child1->symbolNode->symbol);
                        }
                    } else {
                        child1->symbolNode->symbol->type =
                                node_to_type(context, child2,
                                             &(child1->symbolNode->symbol->value_dims),
                                             &(child1->symbolNode->symbol->dim_base),
                                             &(child1->symbolNode->symbol->dim_elements),
                                             &(child1->symbolNode->symbol->value_class));

                        if (child1->symbolNode->symbol->value_dims == 0 && child2->node_type != CLASS)
                            node_to_dims(context, child1, &(child1->symbolNode->symbol->value_dims),
                                         &(child1->symbolNode->symbol->dim_base), &(child1->symbolNode->symbol->dim_elements));

                        if (child1->symbolNode->symbol->type != TP_UNKNOWN) {
                            /* context->changed_flags |= FLAG_VAL_TYPE; */ ast_svtp(child1, child1->symbolNode->symbol);
                        }
                    }
                }
                break;

            case CONST_SYMBOL:
                if (node->value_type == TP_UNKNOWN) {
                    /* context->changed_flags |= FLAG_VAL_TYPE; */ set_node_type(node, TP_STRING);
                }
                break;

            case INTEGER:
            case OP_ARGS:
                if (node->value_type == TP_UNKNOWN && node->parent->node_type != NODE_REGISTER) {
                    /* context->changed_flags |= FLAG_VAL_TYPE; */ set_node_type(node, TP_INTEGER);
                }
                break;

            case STRING:
                if (node->value_type == TP_UNKNOWN) {
                    /* context->changed_flags |= FLAG_VAL_TYPE; */ set_node_type(node, TP_STRING);
                }
                break;

            case FLOAT:
                if (node->value_type == TP_UNKNOWN) {
                    /* context->changed_flags |= FLAG_VAL_TYPE; */ set_node_type(node, TP_FLOAT);
                }
                break;

            case DECIMAL:
                if (node->value_type == TP_UNKNOWN) {
                    /* context->changed_flags |= FLAG_VAL_TYPE; */ set_node_type(node, TP_DECIMAL);
                }
                break;

            case VOID:
            case NOVAL:
            case RANGE:
                if (node->value_type == TP_UNKNOWN) {
                    /* context->changed_flags |= FLAG_VAL_TYPE; */ set_node_type(node, TP_VOID);
                }
                break;

            case CLASS:
                if (node->value_type == TP_UNKNOWN) {
                    ValueType nt = node_to_type(context, node, &(node->value_dims),
                                                    &(node->value_dim_base), &(node->value_dim_elements),
                                                    &(node->value_class));
                    if (nt != TP_UNKNOWN || node->value_dims > 0 || node->value_class) {
                        /* context->changed_flags |= FLAG_VAL_TYPE; */
                        node->value_type = nt;
                    }

                    /* Reset Node Target Type to be the same as the node value type */
                    ast_rttp(node);
                }
                break;

            case DEFINE:
                if (node->value_type == TP_UNKNOWN) {
                    set_node_type(node, TP_VOID);
                    
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
                        /* context->changed_flags |= FLAG_VAL_TYPE; */ ast_svtp(child1, child1->symbolNode->symbol);
                    }
                }
                break;

            case ARG:
                if (node->value_type == TP_UNKNOWN) {
                    if (node->child->node_type == VARG || node->child->node_type == VARG_REFERENCE) {
                        /* Ellipse */
                        /* context->changed_flags |= FLAG_VAL_TYPE; */ child1->value_type = node_to_type(context, child2,
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
                            /* context->changed_flags |= FLAG_VAL_TYPE; */ if (child1->symbolNode->symbol->type == TP_UNKNOWN) {
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
                    /* context->changed_flags |= FLAG_VAL_TYPE; */ }
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
                if (node->symbolNode && node->symbolNode->symbol && node->symbolNode->symbol->is_main) {
                    /* Validate main() return values */
                    if (node->value_type != TP_VOID && node->value_type != TP_INTEGER) {
                        /* Must be an string array */
                        ASTNode *cls = ast_chld(node, CLASS, 0);
                        if (cls) mknd_err(cls, "MAIN_RETURNS_INTEGER");
                        else mknd_err(node, "MAIN_RETURNS_INTEGER");
                    }
                }
                break;

            case OP_AND:
            case OP_OR:
                set_node_target_type(context, child1, TP_BOOLEAN);
                set_node_target_type(context, child2, TP_BOOLEAN);
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
                set_node_target_type(context, child1, max_type(node));
                set_node_target_type(context, child2, max_type(node));
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
                set_node_target_type(context, child1, node->value_type);
                set_node_target_type(context, child2, node->value_type);
                break;

            case OP_NOT:
                set_node_target_type(context, child1, node->value_type);
                break;

            case OP_PLUS:
            case OP_NEG:
                set_node_type(node, promotion[child1->value_type][TP_VOID]);
                set_node_target_type(context, child1, node->value_type);
                break;

            case VAR_SYMBOL:
                if (node->parent->node_type != NODE_REGISTER && node->parent->node_type != DEFINE) {
                    int skip_svtp = 0;
                    if (node->symbolNode && node->symbolNode->symbol) {
                        if (node->value_type == TP_UNKNOWN) {
                            if ((node->node_type == VAR_SYMBOL || node->node_type == VAR_TARGET) && 
                                node->symbolNode->symbol->value_dims == 0 && 
                                ast_chdn(node, 0) != NULL) {
                                skip_svtp = 1;
                            }
                            if (!skip_svtp) {
                                ast_svtp(node, node->symbolNode->symbol);
                            }
                        }
                    }
                    if (node->value_type == TP_UNKNOWN) {
                        /* Check if the variable was explicitly typed as .unknown at declaration */
                        int is_explicit_unknown = 0;
                        int has_creation_error = 0;
                        if (node->symbolNode && node->symbolNode->symbol && node->symbolNode->symbol->creation_node) {
                            ASTNode *creation = node->symbolNode->symbol->creation_node;
                            if (creation->node_type == DEFINE && ast_nchd(creation) > 1) {
                                ASTNode *type_node = ast_chdn(creation, 1);
                                if (type_node && type_node->node_type == CLASS && type_node->node_string && strcasecmp(type_node->node_string, ".unknown") == 0) {
                                    is_explicit_unknown = 1;
                                }
                            }
                            if (origin_subtree_has_error(creation)) has_creation_error = 1;
                        }
                        if (!is_explicit_unknown && !skip_svtp && !has_creation_error) {
                            mknd_err(node, "UNKNOWN_TYPE");
                        }
                    }
                }

                if (node->symbolNode && node->symbolNode->symbol && node->symbolNode->symbol->type != TP_UNKNOWN && ast_nchd(node) && !node->symbolNode->symbol->value_dims) {
                    /* Relax Type Checks for Unresolved Globals: Temporarily suppress #NOT_AN_ARRAY errors for symbols with status == SYM_STATUS_UNRESOLVED and exposed == 1 */
                    /* Also relax for exposed symbols in general as they might be arrays defined elsewhere */
                    /* Also relax for Object property access before syntax sugar transformation */
                    if (!((node->symbolNode->symbol->status == SYM_STATUS_UNRESOLVED || node->symbolNode->symbol->exposed) && node->symbolNode->symbol->exposed)) {
                        if (node->symbolNode->symbol->type != TP_OBJECT) {
                            mknd_err(node, "NOT_AN_ARRAY");
                        }
                    }
                }
                if (child1) {
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

                        set_node_target_type(context, n1, TP_INTEGER);

                        if (n1->node_type == STRING && n1->symbolNode && n1->symbolNode->symbol->symbol_type == CONSTANT_SYMBOL) {
                            /* Taken Constant used where integer required */
                            rxinteger val_int;
                            if (string2integer(&val_int, n1->node_string, n1->node_string_length)) {
                                mknd_err(n1, "BAD_CONVERSION");
                            }
                        }

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
                        if (node->symbolNode && node->symbolNode->symbol && node->symbolNode->symbol->value_dims < ast_nchd(node)) {
                            /* Relax Type Checks for Unresolved Globals: Temporarily suppress #ARRAY_DIMS_MISMATCH errors for symbols with status == SYM_STATUS_UNRESOLVED and exposed == 1 */
                            /* Also relax for exposed symbols in general */
                            if (!((node->symbolNode->symbol->status == SYM_STATUS_UNRESOLVED || node->symbolNode->symbol->exposed) && node->symbolNode->symbol->exposed))
                                mknd_err(node, "ARRAY_DIMS_MISMATCH");
                        }

                        set_node_type(node, TP_INTEGER);
                    }

                    else {
                        /* We are returning the array element */
                        if (node->symbolNode && node->symbolNode->symbol && node->symbolNode->symbol->value_dims < ast_nchd(node)) {
                            /* Relax Type Checks for Unresolved Globals: Temporarily suppress #ARRAY_DIMS_MISMATCH errors for symbols with status == SYM_STATUS_UNRESOLVED and exposed == 1 */
                            /* Also relax for exposed symbols in general */
                            if (!((node->symbolNode->symbol->status == SYM_STATUS_UNRESOLVED || node->symbolNode->symbol->exposed) && node->symbolNode->symbol->exposed))
                                mknd_err(node, "ARRAY_DIMS_MISMATCH");
                        }

                        {
                            int new_dims = 0;
                            ValueType new_type = node->value_type;
                            char *new_class = node->value_class;
                            if (node->symbolNode && node->symbolNode->symbol) {
                                if (node->symbolNode->symbol->value_dims > ast_nchd(node)) {
                                    new_dims = node->symbolNode->symbol->value_dims - ast_nchd(node);
                                }
                                new_type = node->symbolNode->symbol->type;
                                new_class = node->symbolNode->symbol->value_class;
                            }
                            ast_set_value_type(0, node, new_type, new_dims, 0, 0, new_class);
                            ast_set_target_type(0, node, new_type, new_dims, 0, 0, new_class);
                        }
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
                            set_node_target_type(context, n1, TP_VOID);
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

                    if (child1->symbolNode->symbol->type == TP_UNKNOWN) {
                        /* Only error if the RHS isn't explicitly '.unknown' */
                        int is_explicit_unknown = 0;
                        if (child2->node_type == CLASS && child2->node_string && strcasecmp(child2->node_string, ".unknown") == 0) {
                            is_explicit_unknown = 1;
                        }
                        if (!is_explicit_unknown && !ast_hase(node) && !ast_hase(child2)) {
                            mknd_err(node, "UNKNOWN_TYPE");
                        }
                    }
                }

                if (ast_nchd(child1)) {
                    /* We have unexpected array parameters */
                    mknd_err(ast_chdn(child1,0), "INVALID_LHS_ARRAY");
                }

                ast_sttn(child2, child1);
                validate_node_promotion(context, child2);
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
                            node_to_dims(context, child1, &(child1->symbolNode->symbol->value_dims),
                                         &(child1->symbolNode->symbol->dim_base), &(child1->symbolNode->symbol->dim_elements));
                    }
                    if (child1->symbolNode) {
                        int skip_svtp = 0;
                        if (child1->node_type == VAR_TARGET && 
                            child1->symbolNode->symbol->value_dims == 0 && 
                            ast_chdn(child1, 0) != NULL) {
                            skip_svtp = 1;
                        }
                        if (!skip_svtp) {
                            ast_svtp(child1, child1->symbolNode->symbol);
                        }
                    }

                    if (!child1->symbolNode || child1->value_type == TP_UNKNOWN) {
                        /* Check if the right-hand side is explicitly .unknown */
                        int is_explicit_unknown = 0;
                        if (child2->node_type == CLASS && child2->node_string && strcasecmp(child2->node_string, ".unknown") == 0) {
                            is_explicit_unknown = 1;
                        }
                        int skip_svtp = 0;
                        if (child1->node_type == VAR_TARGET && child1->symbolNode && child1->symbolNode->symbol &&
                            child1->symbolNode->symbol->value_dims == 0 && ast_chdn(child1, 0) != NULL) {
                            skip_svtp = 1;
                        }
                        if (!is_explicit_unknown && !skip_svtp && !ast_hase(node) && !ast_hase(child2)) {
                            mknd_err(node, "UNKNOWN_TYPE");
                        }
                    }

                    if (ast_nchd(child1)) {
                        /* We have array parameters */
                        /* Set array parameter type to integer */
                        n1 = child1->child;
                        while (n1) {
                            set_node_target_type(context, n1, TP_INTEGER);

                            if (n1->node_type == STRING && n1->symbolNode && n1->symbolNode->symbol->symbol_type == CONSTANT_SYMBOL) {
                                /* Taken Constant used where integer required */
                                rxinteger val_int;
                                if (string2integer(&val_int, n1->node_string, n1->node_string_length)) {
                                    mknd_err(n1, "BAD_CONVERSION");
                                }
                            }

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
                                                  n1->parent->symbolNode->symbol->dim_base[ix] +
                                                  n1->parent->symbolNode->symbol->dim_elements[ix] - 1)
                                            mknd_err(n1, "OUT_OF_RANGE");
                                    }
                                }
                            }

                            n1 = n1->sibling;
                        }

                        if (!child1->value_dims) {
                            /* Relax Type Checks for Unresolved Globals: Temporarily suppress #NOT_AN_ARRAY errors for symbols with status == SYM_STATUS_UNRESOLVED and exposed == 1 */
                            /* Also relax for exposed symbols in general */
                            /* Also relax for Object property access before syntax sugar transformation */
                            if (!((child1->symbolNode->symbol->status == SYM_STATUS_UNRESOLVED || child1->symbolNode->symbol->exposed) && child1->symbolNode->symbol->exposed)) {
                                if (child1->symbolNode->symbol->type != TP_OBJECT) {
                                    mknd_err(child1, "NOT_AN_ARRAY");
                                }
                            }
                        }
                        else if (child1->value_dims != ast_nchd(child1)) {
                            /* Relax Type Checks for Unresolved Globals: Temporarily suppress #ARRAY_DIMS_MISMATCH errors for symbols with status == SYM_STATUS_UNRESOLVED and exposed == 1 */
                            /* Also relax for exposed symbols in general */
                            /* Also relax for Object property access before syntax sugar transformation */
                            if (!((child1->symbolNode->symbol->status == SYM_STATUS_UNRESOLVED || child1->symbolNode->symbol->exposed) && child1->symbolNode->symbol->exposed)) {
                                if (child1->symbolNode->symbol->type != TP_OBJECT) {
                                    mknd_err(node, "ARRAY_DIMS_MISMATCH");
                                }
                            }
                        }

                        child1->value_dims = 0; /* We are a single value */
                        child1->target_dims = 0;
                    }

                    ast_sttn(child2, child1);
                    validate_node_promotion(context, child2);
                    ast_svtn(node, child1);
                }
                break;

            case ARGS:
                n1 = ast_proc(node);
                if (n1 && n1->symbolNode && n1->symbolNode->symbol && n1->symbolNode->symbol->is_main) {
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

                {
                    n1 = ast_proc(node);
                    ASTNode *inst = n1 ? ast_chld(n1, INSTRUCTIONS, NOP) : NULL;
                    if (inst && inst->node_type == INSTRUCTIONS) {
                        /* In a function implementation - in this case the optional flag '?' is invalid for a class type as a
                         * definition needs to know the default value that can only be defined by the expression on the
                         * right-hand-side */
                        if (child2->node_type == CLASS && node->is_opt_arg && !node->value_dims) {
                            /* Optional but the CLASS doesn't give the needed default value */
                            /* NOTE Arrays are an exception - their default value is a "blank" array */
                            mknd_err(node, "NO_DEFAULT_VALUE");
                        }
                    }
                }
                break;

            case OP_ARG_VALUE:
            case OP_ARG_IX_EXISTS:
                n1 = ast_proc(node);
                if (n1 && n1->symbolNode && n1->symbolNode->symbol) {
                    if (!n1->symbolNode->symbol->has_vargs) mknd_err(node,"NO_PROC_VARGS");
                }
                set_node_target_type(context, child1,TP_INTEGER);

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
                if (child1) set_node_target_type(context, child1, TP_STRING);
                break;

            case BLOCK_EXPR: {
                ASTNode *n = node->child;
                ASTNode *matched_leave = 0;
                ASTNode *first_typed_leave = 0;

                while (n) {
                    if (n->node_type == LEAVE_WITH && n->association == node) {
                        matched_leave = n;
                        if (n->value_type != TP_UNKNOWN) {
                            if (!first_typed_leave) {
                                first_typed_leave = n;
                            } else if (!same_value_type(first_typed_leave, n)) {
                                mknd_err(n, "TYPE_MISMATCH");
                            }
                        }
                    }

                    if (n->child && n->node_type != BLOCK_EXPR) {
                        n = n->child;
                    } else if (n == node) {
                        break;
                    } else if (n->sibling) {
                        n = n->sibling;
                    } else {
                        n = n->parent;
                        while (n && n != node && !n->sibling) n = n->parent;
                        if (!n || n == node) break;
                        n = n->sibling;
                    }
                }

                if (first_typed_leave) {
                    copy_value_type(context, node, first_typed_leave);
                } else if (!matched_leave && context->is_final_pass) {
                    mknd_err(node, "RETVAL_MISSING");
                    set_node_type(node, TP_VOID);
                }
                break;
            }

            case RETURN:
                /* Type is the procedure's return type */
                n1 = ast_proc(node);
                if (n1 && n1->symbolNode && n1->symbolNode->symbol) {
                    ast_svtp(node, n1->symbolNode->symbol);
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
                        validate_node_promotion(context, child1);
                    }
                    else mknd_err(node, "RETVAL_MISSING");
                }
                break;

            case LEAVE_WITH:
                node->association = find_enclosing_block_expr(node);
                if (!node->association) {
                    mknd_err(node, "NOT_IN_BLOCK_EXPR");
                }
                if (child1) {
                    copy_value_type(context, node, child1);
                    if (node->association && node->association->value_type != TP_UNKNOWN) {
                        ast_sttn(child1, node->association);
                        validate_node_promotion(context, child1);
                    }
                } else {
                    set_node_type(node, TP_VOID);
                }
                break;

            case IF:
                if (child1) set_node_target_type(context, child1, TP_BOOLEAN);
                break;

                /* Loops */
            case TO:
            case BY:
                /* The TO/BY value type needs to be the same as the assigment type */
                if (child1) set_node_target_type(context, child1, node->parent->child->value_type);
                set_node_type(node, node->parent->child->value_type);
                break;

            case FOR:
                set_node_target_type(context, child1, TP_INTEGER);
                set_node_type(node, TP_INTEGER);
                break;

            case UNTIL:
            case WHILE:
                if (child1) set_node_target_type(context, child1, TP_BOOLEAN);
                set_node_type(node, TP_BOOLEAN);
                break;

            case LEAVE:
            case ITERATE:
                /* Link to relevant DO */
                if (node->child) {
                    /* Symbol specified - so we need to find it */
                    n1 = ast_do(node);
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
                        n1 = ast_do(n1->parent);
                    }
                    found:;
                }
                else {
                    /* Symbol not specified - just find inner DO */
                    node->association = ast_do(node);
                    if (!node->association) {
                        if (node->node_type == LEAVE && find_enclosing_block_expr(node)) {
                            mknd_err(node, "LEAVE_WITH_REQUIRED");
                        } else {
                            mknd_err(node, "NOT_IN_LOOP"); /* 28.1, 28.2 */
                        }
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
                    if (n1->node_type == WARNING || n1->node_type == ERROR) {
                        n1 = n1->sibling;
                        continue;
                    }
                    arg_num++;
                    if (!n2) {
                        if (node->value_type == TP_UNKNOWN) {
                            break;
                        }
                        /* Its not an error for the first NOVAL argument */
                        if (arg_num > 1 || n1->node_type != NOVAL) mknd_err(n1, "UNEXPECTED_ARGUMENT, %d", arg_num);
                        else if (n1->node_type == NOVAL) {
                            /* Prune the unwanted NOVAL - the parser grammar just added it */
                            ASTNode *to_del = n1;
                            n1 = n1->sibling;
                            ast_del(to_del);
                            context->changed_flags |= FLAG_VAL_TYPE;
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
                                context->changed_flags |= FLAG_VAL_TYPE;
                            }
                        } else {
                            ast_sttn(n1, n2);
                            promote_symbol_from_target(0, n1);
                            validate_node_promotion(context, n1);


                            if (n2->child->node_type == VARG_REFERENCE) {
                                n1->is_ref_arg = 1;
                                if (n1->symbolNode) {
                                    validate_node_promotion_for_ref(context, n1);

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
                            promote_symbol_from_target(0, n1);
                            validate_node_promotion(context, n1);
                        }

                        if (n2->child->node_type == VAR_REFERENCE) {
                            n1->is_ref_arg = 1;
                            if (n1->symbolNode) {
                                validate_node_promotion_for_ref(context, n1);

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
                        context->changed_flags |= FLAG_VAL_TYPE;
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
                /* context->changed_flags |= FLAG_VAL_TYPE; */ node->node_type = DECIMAL;
                break;
            case CLASS:
                if (node->node_string_length == strlen(".float")) {
                    if (strncmp(node->node_string, ".float", node->node_string_length) == 0) {
                        /* This is a .float class - convert to decimal */
                        /* context->changed_flags |= FLAG_VAL_TYPE; */ ast_str(node, ".decimal");
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
                /* context->changed_flags |= FLAG_VAL_TYPE; */ node->node_type = FLOAT;
                break;
            case CLASS:
                if (node->node_string_length == strlen(".decimal")) {
                    if (strncmp(node->node_string, ".decimal", node->node_string_length) == 0) {
                        /* This is a .decimal class - convert to float */
                        /* context->changed_flags |= FLAG_VAL_TYPE; */ ast_str(node, ".float");
                    }
                }
                break;

            default:;
        }

        context->current_scope = node->scope;
    }

    return result_normal;
}
