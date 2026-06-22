/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXCP_INLINE_INTERNAL_H
#define CREXX_RXCP_INLINE_INTERNAL_H

#include <stdio.h>
#include "rxcp_val.h"
#include "rxcp_remap.h"
#include "rxcp_remap_build.h"

/*
 * Production inlining size policy.  This is a profitability/metadata-size
 * cutoff, not a semantic safety boundary; semantic gates still decide whether
 * a body is safe to inline.
 */
#define INLINE_MAX_NODES 300
#define INLINE_META_MAX_SOURCE_SPAN 512

typedef struct {
    Symbol *old_symbol;
    Symbol *new_symbol;
} InlineSymbolMapEntry;

typedef struct {
    Scope *old_scope;
    Scope *new_scope;
} InlineScopeMapEntry;

typedef struct {
    ASTNode *old_node;
    ASTNode *new_node;
} InlineNodeMapEntry;

typedef struct {
    Symbol *formal_symbol;
    ASTNode *actual_source;
    Symbol **captured_symbols;
    size_t captured_count;
} InlineRefActualEntry;

typedef struct {
    Scope *callee_scope;
    Scope *inline_scope;
    InlineSymbolMapEntry *symbol_entries;
    size_t symbol_count;
    InlineScopeMapEntry *scope_entries;
    size_t scope_count;
    InlineNodeMapEntry *node_entries;
    size_t node_count;
    InlineRefActualEntry *ref_entries;
    size_t ref_count;
    InlineRefActualEntry *varg_ref_entries;
    Symbol **varg_symbols;
    size_t varg_count;
    Symbol *varg_array_symbol;
    Symbol *method_receiver_source_symbol;
    Symbol *method_receiver_local_symbol;
    int method_receiver_needs_copyback;
} InlineCloneState;

typedef struct {
    ASTNode *root_proc;
    Context *context;
    int node_count;
    int return_count;
    int has_unsupported_varg_access;
    int has_unsupported_assembler_alias;
    int has_unsupported_assembler_effect;
    int has_unsupported_reference;
    int has_class_attribute_write;
    int has_unportable_class_attribute_shape;
    size_t max_required_varg_index;
    int ref_varg_mode;
} InlinableCheck;

typedef struct {
    ASTNode *return_target;
    Symbol *return_sink_symbol;
} InlineReturnPlan;

typedef struct {
    int return_count;
    int top_level_return_count;
    int final_is_return;
} InlineReturnShape;

typedef enum {
    INLINE_ELIGIBILITY_OK = 0,
    INLINE_ELIGIBILITY_MISSING_ARGS_OR_INSTRS,
    INLINE_ELIGIBILITY_MISSING_INSTRS,
    INLINE_ELIGIBILITY_RETURN_REFERENCE_CLASS,
    INLINE_ELIGIBILITY_VARG_FORMAL_FOLLOWED,
    INLINE_ELIGIBILITY_RETURN_SHAPE_FAILED,
    INLINE_ELIGIBILITY_VALUE_NOT_FINAL_RETURN,
    INLINE_ELIGIBILITY_VALUE_NO_RETURN,
    INLINE_ELIGIBILITY_NODE_CUTOFF,
    INLINE_ELIGIBILITY_RETURN_COUNT_MISMATCH,
    INLINE_ELIGIBILITY_ASSEMBLER_ALIAS,
    INLINE_ELIGIBILITY_ASSEMBLER_EFFECT,
    INLINE_ELIGIBILITY_UNSUPPORTED_REFERENCE,
    INLINE_ELIGIBILITY_UNSUPPORTED_VARG_ACCESS,
    INLINE_ELIGIBILITY_UNPORTABLE_CLASS_ATTRIBUTE_SHAPE
} InlineEligibilityReject;

typedef struct {
    ASTNode *args;
    ASTNode *instrs;
    InlineReturnShape return_shape;
    InlinableCheck check;
    InlineEligibilityReject reject;
} InlineEligibility;

typedef enum {
    INLINE_EXPR_CONTEXT_NONE = 0,
    INLINE_EXPR_CONTEXT_EAGER_VALUE_CONSUMER,
    INLINE_EXPR_CONTEXT_EAGER_OPERATOR,
    INLINE_EXPR_CONTEXT_EAGER_CALL_ARGUMENT,
    INLINE_EXPR_CONTEXT_SHORT_CIRCUIT_OPERATOR,
    INLINE_EXPR_CONTEXT_CONTROL_CONSUMER
} InlineExprContext;

typedef struct {
    Context *context;
    int changed;
} InlineWalkerPayload;

const char *inline_debug_push_remap_rule(const char *rule_id);
void inline_debug_pop_remap_rule(const char *previous);
void inline_remap_debug_result(Context *context,
                               const RxcpRemapRule *rule,
                               ASTNode *site,
                               Symbol *proc_sym,
                               const char *outcome);
const RxcpRemapHooks *rxcp_inline_remap_hooks(void);
const RxcpRemapHooks *rxcp_inline_remap_trace_hooks(void);

int inline_node_is_inlineable_call(ASTNode *node, Symbol **proc_sym_out);
int inline_rhs_eager_operator_needs_left_capture(ASTNode *node);

int ast_inline_assignment(Context *context, ASTNode *assign_node, ASTNode *call_node, Symbol *proc_sym);
int ast_inline_call(Context *context, ASTNode *call_stmt, ASTNode *call_node, Symbol *proc_sym);
int ast_inline_expression(Context *context, ASTNode *call_node, Symbol *proc_sym);
int ast_inline_rhs_eager_operator(Context *context,
                                  ASTNode *op_node,
                                  ASTNode *call_node,
                                  Symbol *proc_sym);

RxcpRemapResult rxcp_inline_apply_remap_rules(Context *context,
                                              InlineWalkerPayload *payload,
                                              ASTNode *node);
const RxcpRemapRule *rxcp_inline_bind_actuals_rule(void);
const RxcpRemapRule *rxcp_inline_structural_eligibility_rule(void);
const RxcpRemapRule *rxcp_inline_clone_body_rule(void);
const RxcpRemapRule *rxcp_inline_return_rewrite_rule(void);
const RxcpRemapRule *rxcp_inline_receiver_copyback_rule(void);
void rxcp_inline_print_rule_summary(FILE *out);
void rxcp_inline_maybe_print_rule_summary(Context *context);

#endif
