/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "rxcp_val.h"
#include "rxcp_ast.h"
#include "rxcpbgmr.h"
#include "rxcpdary.h"
#include "rxcp_sym.h"
#include "rxcp_source_tree.h"
#include "rxcp_inline_internal.h"

static size_t inline_count_siblings(ASTNode *node);
static ASTNode *inline_clone_subtree(Context *context, ASTNode *node, InlineCloneState *state);
static ASTNode *inline_clone_subtree_in_scope(Context *context,
                                              ASTNode *node,
                                              InlineCloneState *state,
                                              Scope *current_scope);
static ASTNode *inline_create_temp_value_ref(Context *context,
                                             ASTNode *instr_list,
                                             Scope *inline_scope,
                                             ASTNode *source_node,
                                             InlineCloneState *state,
                                             const char *prefix,
                                             size_t suffix);
static int inline_capture_scoped_call_actuals(Context *context,
                                              ASTNode *instr_list,
                                              Scope *inline_scope,
                                              ASTNode *proc_def,
                                              ASTNode *call_node,
                                              InlineCloneState *clone_state,
                                              Symbol **captured_receiver_out,
                                              Symbol ***captured_symbols_out,
                                              size_t *captured_count_out);
static int inline_capture_varg_captured_actuals(Context *context,
                                                ASTNode *instr_list,
                                                Scope *inline_scope,
                                                ASTNode *varg_arg,
                                                ASTNode *actual_arg,
                                                InlineCloneState *state,
                                                Symbol **captured_symbols,
                                                size_t captured_count,
                                                size_t first_actual_index);
static int inline_initialise_varg_array(Context *context,
                                        ASTNode *instr_list,
                                        Scope *inline_scope,
                                        ASTNode *varg_arg,
                                        ASTNode *source_node,
                                        InlineCloneState *state);
static ASTNode *inline_find_varg_arg(ASTNode *proc_def);
static int inline_node_is_callable_def(ASTNode *node);
static ASTNode *inline_call_first_user_actual(ASTNode *call_node);
static ASTNode *inline_call_receiver(ASTNode *call_node);
static int inline_call_arity_matches(ASTNode *call_node, Symbol *proc_sym, size_t *varg_count_out);
static int inline_analyse_varg_usage(ASTNode *proc_def, int *unsupported_out, size_t *max_required_index_out);
static int inline_call_is_recursive(ASTNode *call_node, Symbol *proc_sym);
static int inline_numeric_context_compatible(const numeric_context *caller, const numeric_context *callee);
static int inline_analyse_return_shape(ASTNode *proc_def, InlineReturnShape *shape_out);
static int inline_method_writes_class_attribute(ASTNode *proc_def);
static Symbol *inline_find_instance_source_symbol(ASTNode *proc_def);
static int inline_callable_is_method(ASTNode *proc_def);
static int inline_symbol_writes_class_attribute(Symbol *symbol);
static int inline_subtree_reads_class_attribute(ASTNode *node);
static int inline_sibling_list_reads_class_attribute(ASTNode *node);
static int inline_assembler_has_unsupported_aliasing(ASTNode *node);
static int inline_assembler_has_unsupported_effect(ASTNode *node);
static int inline_proc_has_procedure_expose(ASTNode *node);
static int inline_count_return_nodes(ASTNode *node);
static int inline_rewrite_return_nodes(Context *context,
                                       ASTNode **node_ref,
                                       ASTNode *block_expr,
                                       Scope *inline_scope,
                                       int allow_dummy_return,
                                       ValueType proc_type,
                                       InlineCloneState *clone_state);

static const char *inline_debug_active_remap_rule_id = NULL;

const char *inline_debug_push_remap_rule(const char *rule_id) {
    const char *previous = inline_debug_active_remap_rule_id;
    inline_debug_active_remap_rule_id = rule_id;
    return previous;
}

void inline_debug_pop_remap_rule(const char *previous) {
    inline_debug_active_remap_rule_id = previous;
}

static void inline_debug_log(Context *context,
                             ASTNode *site,
                             Symbol *proc_sym,
                             const char *prefix,
                             const char *format,
                             ...) {
    Context *root;
    va_list args;

    root = context && context->master_context ? context->master_context : context;
    if (!root || root->debug_mode < 1 || !prefix || !format) return;

    fprintf(stderr, "%s", prefix);
    if (proc_sym && proc_sym->name) fprintf(stderr, " %s", proc_sym->name);
    if (inline_debug_active_remap_rule_id) fprintf(stderr, " rule=%s", inline_debug_active_remap_rule_id);
    if (site && site->file_name) {
        fprintf(stderr, " @ %s", site->file_name);
        if (site->line > 0) fprintf(stderr, ":%d", site->line);
        if (site->column > 0) fprintf(stderr, ":%d", site->column);
    }
    fprintf(stderr, " - ");

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
}

static void inline_debug_fail_closed(Context *context,
                                     ASTNode *site,
                                     Symbol *proc_sym,
                                     const char *format,
                                     ...) {
    Context *root;
    va_list args;

    root = context && context->master_context ? context->master_context : context;
    if (!root || root->debug_mode < 1 || !format) return;

    fprintf(stderr, "DEBUG_INLINE_FAILCLOSED");
    if (proc_sym && proc_sym->name) fprintf(stderr, " %s", proc_sym->name);
    if (inline_debug_active_remap_rule_id) fprintf(stderr, " rule=%s", inline_debug_active_remap_rule_id);
    if (site && site->file_name) {
        fprintf(stderr, " @ %s", site->file_name);
        if (site->line > 0) fprintf(stderr, ":%d", site->line);
        if (site->column > 0) fprintf(stderr, ":%d", site->column);
    }
    fprintf(stderr, " - ");

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
}

void inline_remap_debug_result(Context *context,
                               const RxcpRemapRule *rule,
                               ASTNode *site,
                               Symbol *proc_sym,
                               const char *result) {
    Context *root;

    root = context && context->master_context ? context->master_context : context;
    if (!root || root->debug_mode < 2 || !rule || !result) return;

    fprintf(stderr, "DEBUG_INLINE_REMAP");
    if (proc_sym && proc_sym->name) fprintf(stderr, " %s", proc_sym->name);
    if (site && site->file_name) {
        fprintf(stderr, " @ %s", site->file_name);
        if (site->line > 0) fprintf(stderr, ":%d", site->line);
        if (site->column > 0) fprintf(stderr, ":%d", site->column);
    }
    fprintf(stderr,
            " - %s: rule=%s phase=%s root=%s priority=%d\n",
            result,
            rule->id ? rule->id : "(unknown)",
            rule->phase ? rule->phase : "(unknown)",
            rule->root_shape ? rule->root_shape : "(unknown)",
            rule->priority);
}

static const char *inline_remap_hook_enter_rule(const char *rule_id, void *user_data) {
    (void)user_data;
    return inline_debug_push_remap_rule(rule_id);
}

static void inline_remap_hook_leave_rule(const char *previous_rule_id, void *user_data) {
    (void)user_data;
    inline_debug_pop_remap_rule(previous_rule_id);
}

static void inline_remap_hook_trace_result(Context *context,
                                           const RxcpRemapRule *rule,
                                           ASTNode *site,
                                           Symbol *symbol,
                                           const char *outcome,
                                           void *user_data) {
    (void)user_data;
    inline_remap_debug_result(context, rule, site, symbol, outcome);
}

const RxcpRemapHooks *rxcp_inline_remap_hooks(void) {
    static const RxcpRemapHooks hooks = {
        inline_remap_hook_enter_rule,
        inline_remap_hook_leave_rule,
        inline_remap_hook_trace_result,
        NULL
    };

    return &hooks;
}

const RxcpRemapHooks *rxcp_inline_remap_trace_hooks(void) {
    static const RxcpRemapHooks hooks = {
        NULL,
        NULL,
        inline_remap_hook_trace_result,
        NULL
    };

    return &hooks;
}

static void inline_export_debug_reject(Context *context,
                                       ASTNode *site,
                                       Symbol *proc_sym,
                                       const char *format,
                                       ...) {
    Context *root;
    va_list args;

    root = context && context->master_context ? context->master_context : context;
    if (!root || root->debug_mode < 2 || !format) return;

    fprintf(stderr, "DEBUG_INLINE_EXPORT");
    if (proc_sym && proc_sym->name) fprintf(stderr, " %s", proc_sym->name);
    if (site && site->file_name) {
        fprintf(stderr, " @ %s", site->file_name);
        if (site->line > 0) fprintf(stderr, ":%d", site->line);
        if (site->column > 0) fprintf(stderr, ":%d", site->column);
    }
    fprintf(stderr, " - reject: ");

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
}

static int inline_scope_is_within(Scope *scope, Scope *root) {
    if (!root) return 0;
    while (scope) {
        if (scope == root) return 1;
        scope = scope->parent;
    }
    return 0;
}

static int inline_expansion_plan_own_candidate(InlineExpansionPlan *plan,
                                               ASTNode *candidate_root,
                                               Scope *candidate_scope) {
    if (!plan || !candidate_root || !candidate_scope || plan->committed || plan->abandoned) return 0;

    /* A call-discard wrapper can own a BLOCK_EXPR nested below it.  Preserve
     * the first/root scope so one rollback removes the entire candidate tree. */
    if (!plan->candidate_root) plan->candidate_root = candidate_root;
    if (!plan->candidate_scope) plan->candidate_scope = candidate_scope;
    return 1;
}

static int inline_expansion_plan_record_external_symbol(InlineExpansionPlan *plan,
                                                        Symbol *symbol) {
    Symbol **symbols;
    size_t i;

    if (!plan || !symbol || plan->committed || plan->abandoned) return 0;
    if (inline_scope_is_within(symbol->scope, plan->candidate_scope)) return 1;

    for (i = 0; i < plan->external_symbol_count; i++) {
        if (plan->external_symbols[i] == symbol) return 1;
    }

    symbols = realloc(plan->external_symbols,
                      sizeof(Symbol *) * (plan->external_symbol_count + 1));
    if (!symbols) return 0;
    plan->external_symbols = symbols;
    plan->external_symbols[plan->external_symbol_count++] = symbol;
    return 1;
}

static Symbol *inline_create_temp_symbol(Context *context,
                                         Scope *scope,
                                         ASTNode *shape_source,
                                         const char *prefix,
                                         size_t suffix,
                                         InlineCloneState *state) {
    Symbol *symbol;

    symbol = rxcp_remap_create_temp_symbol(context, scope, shape_source, prefix, suffix);
    if (!symbol) return NULL;
    if (state && state->expansion_plan &&
        !inline_expansion_plan_record_external_symbol(state->expansion_plan, symbol)) {
        scp_rmsy(scope, symbol);
        free_sym(symbol);
        return NULL;
    }
    return symbol;
}

static void inline_expansion_plan_release_tracking(InlineExpansionPlan *plan) {
    if (!plan) return;
    free(plan->external_symbols);
    plan->external_symbols = NULL;
    plan->external_symbol_count = 0;
}

static void inline_expansion_plan_abort(Context *context, InlineExpansionPlan *plan) {
    ASTNode *node;
    Symbol *symbol;
    size_t i;

    if (!context || !plan || plan->committed || plan->abandoned) return;

    /* AST nodes are Context-arena owned.  Disconnect every node allocated by
     * this transaction, including partially-built nodes which were never
     * attached below candidate_root. */
    node = context->free_list;
    while (node && node != plan->allocation_mark) {
        if (node->symbolNode && node->symbolNode->symbol) {
            sym_dno(node->symbolNode->symbol, node);
        }
        node = node->free_list;
    }

    /* A few capture forms deliberately place compiler temporaries in the
     * caller scope.  They are transaction-owned and must not survive a reject. */
    for (i = 0; i < plan->external_symbol_count; i++) {
        symbol = plan->external_symbols[i];
        if (symbol && symbol->scope && sym_nond(symbol) == 0) {
            scp_rmsy(symbol->scope, symbol);
            free_sym(symbol);
        }
    }

    if (plan->candidate_scope) {
        (void)scp_abandon(context, plan->candidate_scope);
    }
    inline_expansion_plan_release_tracking(plan);
    plan->candidate_root = NULL;
    plan->candidate_scope = NULL;
    plan->abandoned = 1;
}

/*
 * These private implementation fragments are kept in the original translation
 * unit while the inline internals still share many static helpers. Promote a
 * fragment to an independently compiled source only after its dependency
 * boundary has been thinned deliberately.
 */
#include "rxcp_inline_bind.c"
#include "rxcp_inline_clone.c"
#include "rxcp_inline_rewrite.c"
#include "rxcp_inline_analysis.c"
#include "rxcp_inline_payload.c"
