/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxcp_val.h"
#include "rxcp_ast.h"
#include "rxcpbgmr.h"
#include "rxcpdary.h"
#include "rxcp_sym.h"

#define INLINE_MAX_NODES 25

typedef struct {
    Symbol *old_symbol;
    Symbol *new_symbol;
} InlineSymbolMapEntry;

typedef struct {
    Scope *callee_scope;
    Scope *inline_scope;
    InlineSymbolMapEntry *entries;
    size_t count;
} InlineCloneState;

typedef struct {
    ASTNode *root_proc;
    int node_count;
    int has_nested_call;
    int has_nested_scope;
    int return_count;
} InlinableCheck;

typedef struct {
    ASTNode *return_target;
    Symbol *return_sink_symbol;
} InlineReturnPlan;

typedef enum {
    INLINE_EXPR_CONTEXT_NONE = 0,
    INLINE_EXPR_CONTEXT_EAGER_VALUE_CONSUMER,
    INLINE_EXPR_CONTEXT_EAGER_OPERATOR,
    INLINE_EXPR_CONTEXT_EAGER_CALL_ARGUMENT
} InlineExprContext;

static Symbol *inline_find_mapped_symbol(InlineCloneState *state, Symbol *old_symbol) {
    size_t i;

    if (!state || !old_symbol) return NULL;

    for (i = 0; i < state->count; i++) {
        if (state->entries[i].old_symbol == old_symbol) return state->entries[i].new_symbol;
    }

    return NULL;
}

static ASTNode *inline_clone_subtree(Context *context, ASTNode *node, InlineCloneState *state) {
    ASTNode *new_node;
    ASTNode *child;
    Symbol *mapped_symbol;

    if (!node) return NULL;

    new_node = ast_dup(context, node);
    new_node->scope = state ? state->inline_scope : node->scope;

    if (node->symbolNode && node->symbolNode->symbol) {
        mapped_symbol = inline_find_mapped_symbol(state, node->symbolNode->symbol);
        if (!mapped_symbol) mapped_symbol = node->symbolNode->symbol;
        sym_adnd(mapped_symbol, new_node, node->symbolNode->readUsage, node->symbolNode->writeUsage);
    }

    child = node->child;
    while (child) {
        add_ast(new_node, inline_clone_subtree(context, child, state));
        child = child->sibling;
    }

    return new_node;
}

static void inline_disconnect_subtree_symbols(ASTNode *node) {
    ASTNode *child;

    if (!node) return;

    child = node->child;
    while (child) {
        inline_disconnect_subtree_symbols(child);
        child = child->sibling;
    }

    if (node->symbolNode && node->symbolNode->symbol) {
        sym_dno(node->symbolNode->symbol, node);
    }
}

static size_t inline_count_siblings(ASTNode *node) {
    size_t count;

    count = 0;
    while (node) {
        count++;
        node = node->sibling;
    }

    return count;
}

static int inline_build_symbol_map(Scope *callee_scope,
                                   Scope *inline_scope,
                                   InlineCloneState *state) {
    Symbol **symbols;
    size_t count;
    size_t i;
    size_t out_index;

    if (!callee_scope || !inline_scope || !state) return 0;

    symbols = scp_syms(callee_scope);
    count = 0;
    while (symbols[count]) count++;

    state->entries = NULL;
    state->count = 0;
    state->callee_scope = callee_scope;
    state->inline_scope = inline_scope;

    if (!count) {
        free(symbols);
        return 1;
    }

    state->entries = malloc(sizeof(InlineSymbolMapEntry) * count);
    if (!state->entries) {
        free(symbols);
        return 0;
    }

    out_index = 0;
    for (i = 0; i < count; i++) {
        Symbol *old_symbol;
        Symbol *new_symbol;

        old_symbol = symbols[i];
        if (!old_symbol || old_symbol->symbol_type == FUNCTION_SYMBOL) continue;

        new_symbol = sym_dup(inline_scope, old_symbol);
        if (!new_symbol) continue;

        new_symbol->register_num = UNSET_REGISTER;
        new_symbol->register_type = 'r';
        new_symbol->meta_emitted = 0;
        new_symbol->init_emitted = 0;
        new_symbol->defines_scope = NULL;
        new_symbol->ast_template = NULL;
        new_symbol->is_inlinable = 0;

        state->entries[out_index].old_symbol = old_symbol;
        state->entries[out_index].new_symbol = new_symbol;
        out_index++;
    }

    state->count = out_index;
    free(symbols);
    return 1;
}

static void inline_free_symbol_map(InlineCloneState *state) {
    if (!state) return;
    if (state->entries) free(state->entries);
    state->entries = NULL;
    state->count = 0;
}

static void inline_copy_symbol_shape(Symbol *target, Symbol *source) {
    if (!target || !source) return;

    target->type = source->type;
    target->value_dims = source->value_dims;

    if (source->dim_base && source->value_dims) {
        target->dim_base = malloc(sizeof(int) * source->value_dims);
        memcpy(target->dim_base, source->dim_base, sizeof(int) * source->value_dims);
    }

    if (source->dim_elements && source->value_dims) {
        target->dim_elements = malloc(sizeof(int) * source->value_dims);
        memcpy(target->dim_elements, source->dim_elements, sizeof(int) * source->value_dims);
    }

    if (source->value_class) target->value_class = strdup(source->value_class);
}

static ASTNode *inline_create_sink_target(Context *context,
                                          Scope *inline_scope,
                                          ASTNode *source_node,
                                          Symbol *proc_sym) {
    char sink_name[64];
    ASTNode *sink_target;
    Symbol *sink_symbol;

    if (!context || !inline_scope || !source_node || !proc_sym) return NULL;

    snprintf(sink_name, sizeof(sink_name), "__inline_unused_%d", source_node->node_number);

    sink_symbol = sym_fn(inline_scope, sink_name, strlen(sink_name));
    if (!sink_symbol) return NULL;

    sink_symbol->symbol_type = VARIABLE_SYMBOL;
    sink_symbol->status = SYM_STATUS_LOCAL_VAR;
    sink_symbol->register_num = UNSET_REGISTER;
    sink_symbol->register_type = 'r';
    sink_symbol->meta_emitted = 0;
    sink_symbol->init_emitted = 0;
    inline_copy_symbol_shape(sink_symbol, proc_sym);

    sink_target = ast_ftt(context, VAR_TARGET, strdup(sink_symbol->name));
    sink_target->free_node_string = 1;
    sink_target->scope = inline_scope;
    sink_target->token = source_node->token;
    sink_target->line = source_node->line;
    sink_target->column = source_node->column;
    sink_target->source_start = source_node->source_start;
    sink_target->source_end = source_node->source_end;
    sink_target->token_start = source_node->token_start;
    sink_target->token_end = source_node->token_end;
    sym_adnd(sink_symbol, sink_target, 0, 1);
    ast_svtp(sink_target, sink_symbol);

    return sink_target;
}

static InlineExprContext inline_classify_expr_parent(ASTNode *node) {
    if (!node) return 0;

    switch (node->node_type) {
        case FUNCTION:
            return INLINE_EXPR_CONTEXT_EAGER_CALL_ARGUMENT;

        case SAY:
        case RETURN:
            return INLINE_EXPR_CONTEXT_EAGER_VALUE_CONSUMER;

        case OP_ADD:
        case OP_MINUS:
        case OP_MULT:
        case OP_DIV:
        case OP_IDIV:
        case OP_MOD:
        case OP_POWER:
        case OP_CONCAT:
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
        case OP_NEG:
        case OP_PLUS:
            return INLINE_EXPR_CONTEXT_EAGER_OPERATOR;

        default:
            return INLINE_EXPR_CONTEXT_NONE;
    }
}

static int inline_call_has_block_expr_arg(ASTNode *call_node) {
    ASTNode *arg;

    if (!call_node) return 0;

    arg = call_node->child;
    while (arg) {
        if (arg->node_type == BLOCK_EXPR) return 1;
        arg = arg->sibling;
    }

    return 0;
}

static int ast_inline_statement(Context *context,
                                ASTNode *statement_node,
                                ASTNode *call_node,
                                Symbol *proc_sym,
                                InlineReturnPlan *return_plan) {
    ASTNode *proc_def;
    ASTNode *param_list;
    ASTNode *proc_instrs;
    ASTNode *param_arg;
    ASTNode *actual_arg;
    ASTNode *block;
    ASTNode *instr_list;
    ASTNode *proc_instr;
    Scope *inline_scope;
    InlineCloneState clone_state;

    if (!context || !statement_node || !call_node || !proc_sym || !proc_sym->ast_template) return 0;
    if (inline_call_has_block_expr_arg(call_node)) return 0;

    proc_def = proc_sym->ast_template;
    if (!proc_def || !proc_def->scope) return 0;

    if (call_node->child && inline_count_siblings(call_node->child) != proc_sym->fixed_args) return 0;
    if (!call_node->child && proc_sym->fixed_args != 0) return 0;

    block = ast_f(context, INSTRUCTIONS, call_node->token);
    ast_mark_compiler_generated_block(block);
    block->value_type = TP_VOID;
    block->target_type = TP_VOID;

    inline_scope = scp_f(context, statement_node->scope, block, NULL, SCOPE_LOCAL);
    instr_list = block;

    memset(&clone_state, 0, sizeof(clone_state));
    if (!inline_build_symbol_map(proc_def->scope, inline_scope, &clone_state)) {
        return 0;
    }

    param_list = ast_chld(proc_def, ARGS, 0);
    param_arg = param_list ? param_list->child : NULL;
    actual_arg = call_node->child;

    while (param_arg) {
        ASTNode *formal_target;
        ASTNode *bind_assign;
        ASTNode *bind_lhs;
        ASTNode *bind_rhs;

        formal_target = ast_chdn(param_arg, 0);
        if (!formal_target || !actual_arg) {
            inline_free_symbol_map(&clone_state);
            return 0;
        }

        bind_assign = ast_f(context, ASSIGN, formal_target->token);
        bind_assign->scope = inline_scope;

        bind_lhs = inline_clone_subtree(context, formal_target, &clone_state);
        bind_rhs = inline_clone_subtree(context, actual_arg, &clone_state);

        add_ast(bind_assign, bind_lhs);
        add_ast(bind_assign, bind_rhs);
        add_ast(instr_list, bind_assign);

        param_arg = param_arg->sibling;
        actual_arg = actual_arg->sibling;
    }

    if (actual_arg) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    proc_instrs = ast_chld(proc_def, INSTRUCTIONS, 0);
    if (!proc_instrs) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    proc_instr = proc_instrs->child;
    while (proc_instr) {
        if (proc_instr->node_type == RETURN) {
            ASTNode *ret_expr;
            ASTNode *ret_assign;
            ASTNode *ret_lhs;
            ASTNode *ret_rhs;

            ret_expr = proc_instr->child;
            if (!ret_expr) {
                proc_instr = proc_instr->sibling;
                continue;
            }

            ret_assign = ast_f(context, ASSIGN, proc_instr->token);
            ret_assign->scope = inline_scope;

            if (return_plan && return_plan->return_target) {
                ret_lhs = inline_clone_subtree(context, return_plan->return_target, &clone_state);
            } else if (return_plan && return_plan->return_sink_symbol) {
                ret_lhs = inline_create_sink_target(context, inline_scope, proc_instr, return_plan->return_sink_symbol);
            } else {
                inline_free_symbol_map(&clone_state);
                return 0;
            }

            if (!ret_lhs) {
                inline_free_symbol_map(&clone_state);
                return 0;
            }

            ret_rhs = inline_clone_subtree(context, ret_expr, &clone_state);

            add_ast(ret_assign, ret_lhs);
            add_ast(ret_assign, ret_rhs);
            add_ast(instr_list, ret_assign);
        } else {
            ASTNode *cloned_instr;

            cloned_instr = inline_clone_subtree(context, proc_instr, &clone_state);
            add_ast(instr_list, cloned_instr);
        }

        proc_instr = proc_instr->sibling;
    }

    ast_rpl(statement_node, block);
    inline_disconnect_subtree_symbols(statement_node);
    inline_free_symbol_map(&clone_state);

    return 1;
}

static int ast_inline_assignment(Context *context, ASTNode *assign_node, ASTNode *call_node, Symbol *proc_sym) {
    ASTNode *lhs;
    InlineReturnPlan return_plan;

    if (!assign_node || !call_node) return 0;

    lhs = assign_node->child;
    if (!lhs || lhs->node_type != VAR_TARGET || lhs->child) return 0;

    memset(&return_plan, 0, sizeof(return_plan));
    return_plan.return_target = lhs;

    return ast_inline_statement(context, assign_node, call_node, proc_sym, &return_plan);
}

static int ast_inline_call(Context *context, ASTNode *call_stmt, ASTNode *call_node, Symbol *proc_sym) {
    InlineReturnPlan return_plan;

    memset(&return_plan, 0, sizeof(return_plan));
    return_plan.return_sink_symbol = proc_sym;

    return ast_inline_statement(context, call_stmt, call_node, proc_sym, &return_plan);
}

static int ast_inline_expression(Context *context, ASTNode *call_node, Symbol *proc_sym) {
    ASTNode *proc_def;
    ASTNode *param_list;
    ASTNode *proc_instrs;
    ASTNode *param_arg;
    ASTNode *actual_arg;
    ASTNode *block_expr;
    ASTNode *instr_list;
    ASTNode *proc_instr;
    Scope *inline_scope;
    InlineCloneState clone_state;

    InlineExprContext expr_context;

    if (!context || !call_node || !proc_sym || !proc_sym->ast_template) return 0;
    if (inline_call_has_block_expr_arg(call_node)) return 0;

    expr_context = inline_classify_expr_parent(call_node->parent);
    if (expr_context == INLINE_EXPR_CONTEXT_NONE) return 0;

    proc_def = proc_sym->ast_template;
    if (!proc_def || !proc_def->scope) return 0;

    if (call_node->child && inline_count_siblings(call_node->child) != proc_sym->fixed_args) return 0;
    if (!call_node->child && proc_sym->fixed_args != 0) return 0;

    block_expr = ast_dup(context, call_node);
    block_expr->node_type = BLOCK_EXPR;
    block_expr->node_string = "do";
    block_expr->node_string_length = 2;
    block_expr->free_node_string = 0;

    inline_scope = scp_f(context, call_node->scope, block_expr, NULL, SCOPE_LOCAL);
    block_expr->scope = inline_scope;

    instr_list = ast_f(context, INSTRUCTIONS, call_node->token);
    instr_list->scope = inline_scope;
    instr_list->value_type = TP_VOID;
    instr_list->target_type = TP_VOID;
    add_ast(block_expr, instr_list);

    memset(&clone_state, 0, sizeof(clone_state));
    if (!inline_build_symbol_map(proc_def->scope, inline_scope, &clone_state)) {
        return 0;
    }

    param_list = ast_chld(proc_def, ARGS, 0);
    param_arg = param_list ? param_list->child : NULL;
    actual_arg = call_node->child;

    while (param_arg) {
        ASTNode *formal_target;
        ASTNode *bind_assign;
        ASTNode *bind_lhs;
        ASTNode *bind_rhs;

        formal_target = ast_chdn(param_arg, 0);
        if (!formal_target || !actual_arg) {
            inline_free_symbol_map(&clone_state);
            return 0;
        }

        bind_assign = ast_f(context, ASSIGN, formal_target->token);
        bind_assign->scope = inline_scope;
        bind_assign->value_type = formal_target->value_type;
        bind_assign->target_type = formal_target->target_type;

        bind_lhs = inline_clone_subtree(context, formal_target, &clone_state);
        bind_rhs = inline_clone_subtree(context, actual_arg, &clone_state);

        add_ast(bind_assign, bind_lhs);
        add_ast(bind_assign, bind_rhs);
        add_ast(instr_list, bind_assign);

        param_arg = param_arg->sibling;
        actual_arg = actual_arg->sibling;
    }

    if (actual_arg) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    proc_instrs = ast_chld(proc_def, INSTRUCTIONS, 0);
    if (!proc_instrs) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    proc_instr = proc_instrs->child;
    while (proc_instr) {
        if (proc_instr->node_type == RETURN) {
            ASTNode *ret_expr;
            ASTNode *leave_with;
            ASTNode *leave_expr;

            ret_expr = proc_instr->child;
            if (!ret_expr) {
                inline_free_symbol_map(&clone_state);
                return 0;
            }

            leave_with = ast_f(context, LEAVE_WITH, proc_instr->token);
            leave_with->scope = inline_scope;
            leave_with->association = block_expr;
            leave_with->value_type = ret_expr->value_type;
            leave_with->target_type = ret_expr->target_type;

            leave_expr = inline_clone_subtree(context, ret_expr, &clone_state);

            add_ast(leave_with, leave_expr);
            add_ast(instr_list, leave_with);
        } else {
            ASTNode *cloned_instr;

            cloned_instr = inline_clone_subtree(context, proc_instr, &clone_state);
            add_ast(instr_list, cloned_instr);
        }

        proc_instr = proc_instr->sibling;
    }

    ast_rpl(call_node, block_expr);
    inline_disconnect_subtree_symbols(call_node);
    inline_free_symbol_map(&clone_state);

    return 1;
}

/* Walker to find statement-shaped call sites and inline them */
walker_result inline_procedure_walker(walker_direction direction, ASTNode *node, void *payload) {
    Context *context;
    ASTNode *call_node;
    ASTNode *lhs;
    ASTNode *rhs;
    Symbol *proc_sym;

    context = (Context *)payload;

    if (direction == in) return result_normal;

    if (node->node_type == FUNCTION) {
        proc_sym = node->symbolNode ? node->symbolNode->symbol : NULL;
        if (proc_sym && proc_sym->is_inlinable && proc_sym->ast_template) {
            ast_inline_expression(context, node, proc_sym);
        }
        return result_normal;
    }

    if (node->node_type == ASSIGN) {
        lhs = node->child;
        rhs = lhs ? lhs->sibling : NULL;

        if (!lhs || !rhs) return result_normal;
        if (rhs->node_type != FUNCTION) return result_normal;

        proc_sym = rhs->symbolNode ? rhs->symbolNode->symbol : NULL;
        if (proc_sym && proc_sym->is_inlinable && proc_sym->ast_template) {
            ast_inline_assignment(context, node, rhs, proc_sym);
        }
        return result_normal;
    }

    if (node->node_type != CALL) return result_normal;

    call_node = node->child;
    if (!call_node || call_node->node_type != FUNCTION) return result_normal;

    proc_sym = call_node->symbolNode ? call_node->symbolNode->symbol : NULL;
    if (proc_sym && proc_sym->is_inlinable && proc_sym->ast_template) {
        ast_inline_call(context, node, call_node, proc_sym);
    }

    return result_normal;
}

static walker_result inlinable_check_walker(walker_direction direction, ASTNode *node, void *payload) {
    InlinableCheck *check;

    check = (InlinableCheck *)payload;

    if (direction == in) {
        check->node_count++;

        if (node != check->root_proc &&
            node->scope &&
            node->scope->defining_node == node) {
            check->has_nested_scope = 1;
        }

        if (node != check->root_proc &&
            (node->node_type == CALL || node->node_type == FUNCTION)) {
            check->has_nested_call = 1;
        }

        if (node->node_type == RETURN) {
            check->return_count++;
        }
    }
    return result_normal;
}

/* Walker to identify inlinable procedures */
walker_result identify_inlinable_walker(walker_direction direction, ASTNode *node, void *payload) {
    (void)payload;

    if (direction == in) return result_normal;

    if (node->node_type == PROCEDURE) {
        Symbol *sym;
        ASTNode *args;
        ASTNode *arg;
        ASTNode *formal_target;
        Symbol *formal_symbol;
        ASTNode *instrs;
        ASTNode *instr;
        ASTNode *last_instr;
        int return_count;
        InlinableCheck check;

        sym = node->symbolNode ? node->symbolNode->symbol : NULL;
        if (!sym || sym->is_main || !sym->scope || sym->scope->type == SCOPE_CLASS) {
            if (sym) sym->is_inlinable = 0;
            return result_normal;
        }

        args = ast_chld(node, ARGS, 0);
        if (args) {
            arg = args->child;
            while (arg) {
                if (arg->is_ref_arg || arg->is_opt_arg || arg->is_varg) {
                    sym->is_inlinable = 0;
                    return result_normal;
                }

                formal_target = ast_chdn(arg, 0);
                formal_symbol = formal_target && formal_target->symbolNode ? formal_target->symbolNode->symbol : NULL;
                if (formal_symbol &&
                    (formal_symbol->value_class ||
                     formal_symbol->value_dims > 0 ||
                     formal_symbol->type == TP_OBJECT)) {
                    sym->is_inlinable = 0;
                    return result_normal;
                }

                arg = arg->sibling;
            }
        }

        if (sym->value_class || sym->value_dims > 0 || sym->type == TP_OBJECT) {
            sym->is_inlinable = 0;
            return result_normal;
        }

        instrs = ast_chld(node, INSTRUCTIONS, 0);
        if (!instrs) {
            sym->is_inlinable = 0;
            return result_normal;
        }

        instr = instrs->child;
        last_instr = NULL;
        return_count = 0;
        while (instr) {
            if (instr->node_type == RETURN) return_count++;
            last_instr = instr;
            instr = instr->sibling;
        }

        if (return_count != 1 || !last_instr || last_instr->node_type != RETURN) {
            sym->is_inlinable = 0;
            return result_normal;
        }

        memset(&check, 0, sizeof(check));
        check.root_proc = node;
        ast_wlkr(node, inlinable_check_walker, &check);

        if (check.node_count > INLINE_MAX_NODES ||
            check.has_nested_call ||
            check.has_nested_scope ||
            check.return_count != 1) {
            sym->is_inlinable = 0;
            return result_normal;
        }

        sym->is_inlinable = 1;
        sym->ast_template = node;
    }
    return result_normal;
}

void rxcp_inline_prune(Context *context, ASTNode *tree) {
    (void)context;
    (void)tree;
}
