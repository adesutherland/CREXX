/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include <stdlib.h>
#include <string.h>
#include "rxcp_inline_internal.h"

static const RxcpRemapRule inline_remap_bind_actuals_rule = RXCP_REMAP_RULE_SERVICE(
    "inline.bind.actuals",
    "inline.expand",
    "callee-args+call-actuals",
    100
);

static const RxcpRemapRule inline_remap_structural_eligibility_rule = RXCP_REMAP_RULE_SERVICE(
    "inline.eligibility.structural",
    "inline.identify",
    "PROCEDURE|METHOD|FACTORY",
    0
);

const RxcpRemapRule *rxcp_inline_bind_actuals_rule(void) {
    return &inline_remap_bind_actuals_rule;
}

const RxcpRemapRule *rxcp_inline_structural_eligibility_rule(void) {
    return &inline_remap_structural_eligibility_rule;
}

static int inline_bind_rhs_eager_operator_proc_symbol(Context *context,
                                                      RxcpRemapMatch *match) {
    ASTNode *rhs;
    Symbol *proc_sym;

    (void)context;
    if (!match) return 0;

    rhs = rxcp_remap_capture_node(match, "rhs");
    if (!inline_node_is_inlineable_call(rhs, &proc_sym)) return 0;

    match->symbol = proc_sym;
    return 1;
}

static int inline_guard_rhs_eager_operator_capture_safe(Context *context,
                                                        const RxcpRemapMatch *match) {
    ASTNode *op;
    ASTNode *left;
    ASTNode *rhs;
    Scope *parent_scope;

    (void)context;
    if (!match) return 0;

    op = rxcp_remap_capture_node(match, "op");
    left = rxcp_remap_capture_node(match, "left");
    rhs = rxcp_remap_capture_node(match, "rhs");
    if (!op || !left || !rhs) return 0;
    if (!inline_rhs_eager_operator_needs_left_capture(rhs)) return 0;

    parent_scope = op->scope ? op->scope : (rhs->scope ? rhs->scope : left->scope);
    return parent_scope != NULL;
}

static int inline_rewrite_rhs_eager_operator_capture(Context *context,
                                                     const RxcpRemapMatch *match) {
    ASTNode *op;
    ASTNode *rhs;

    if (!match) return 0;

    op = rxcp_remap_capture_node(match, "op");
    rhs = rxcp_remap_capture_node(match, "rhs");
    if (!op || !rhs || !match->symbol) return 0;

    return ast_inline_rhs_eager_operator(context, op, rhs, match->symbol);
}

static int inline_bind_capture_proc_symbol(Context *context,
                                           RxcpRemapMatch *match,
                                           const char *capture_name) {
    ASTNode *call_node;
    Symbol *proc_sym;

    (void)context;
    if (!match || !capture_name) return 0;

    call_node = rxcp_remap_capture_node(match, capture_name);
    if (!inline_node_is_inlineable_call(call_node, &proc_sym)) return 0;

    match->symbol = proc_sym;
    return 1;
}

static int inline_bind_call_proc_symbol(Context *context, RxcpRemapMatch *match) {
    return inline_bind_capture_proc_symbol(context, match, "call");
}

static int inline_bind_rhs_proc_symbol(Context *context, RxcpRemapMatch *match) {
    return inline_bind_capture_proc_symbol(context, match, "rhs");
}

static int inline_guard_expression_block_not_rhs_capture(Context *context,
                                                         const RxcpRemapMatch *match) {
    ASTNode *call_node;

    (void)context;
    if (!match) return 0;

    call_node = rxcp_remap_capture_node(match, "call");
    return call_node && !inline_rhs_eager_operator_needs_left_capture(call_node);
}

static int inline_rewrite_expression_block(Context *context, const RxcpRemapMatch *match) {
    ASTNode *call_node;

    if (!match) return 0;

    call_node = rxcp_remap_capture_node(match, "call");
    if (!call_node || !match->symbol) return 0;

    return ast_inline_expression(context, call_node, match->symbol);
}

static int inline_rewrite_assignment_whole_rhs(Context *context, const RxcpRemapMatch *match) {
    ASTNode *assign;
    ASTNode *rhs;

    if (!match) return 0;

    assign = rxcp_remap_capture_node(match, "assign");
    rhs = rxcp_remap_capture_node(match, "rhs");
    if (!assign || !rhs || !match->symbol) return 0;

    return ast_inline_assignment(context, assign, rhs, match->symbol);
}

static int inline_rewrite_call_statement(Context *context, const RxcpRemapMatch *match) {
    ASTNode *stmt;
    ASTNode *call_node;

    if (!match) return 0;

    stmt = rxcp_remap_capture_node(match, "stmt");
    call_node = rxcp_remap_capture_node(match, "call");
    if (!stmt || !call_node || !match->symbol) return 0;

    return ast_inline_call(context, stmt, call_node, match->symbol);
}

static RxcpRemapResult inline_remap_apply_selector_rule(const RxcpRemapRule *rule,
                                                        Context *context,
                                                        InlineWalkerPayload *payload,
                                                        ASTNode *node) {
    RxcpRemapMatch match;
    ASTNode *debug_site;
    const char *previous_rule;
    int rewritten;
    size_t i;

    if (!rule || !rule->selector || !node) return RXCP_REMAP_NO_MATCH;
    if (!rxcp_remap_run_selector(rule->selector, node, &match)) return RXCP_REMAP_NO_MATCH;

    if (rule->binds) {
        for (i = 0; rule->binds[i].fn; i++) {
            if (!rule->binds[i].fn(context, &match)) {
                debug_site = rxcp_remap_debug_site(rule, &match);
                inline_remap_debug_result(context, rule, debug_site, match.symbol, "rejected");
                return RXCP_REMAP_REJECTED;
            }
        }
    }

    if (rule->guards) {
        for (i = 0; rule->guards[i].fn; i++) {
            if (!rule->guards[i].fn(context, &match)) {
                debug_site = rxcp_remap_debug_site(rule, &match);
                inline_remap_debug_result(context, rule, debug_site, match.symbol, "rejected");
                return RXCP_REMAP_REJECTED;
            }
        }
    }

    previous_rule = inline_debug_push_remap_rule(rule->id);
    rewritten = 0;
    if (rule->rewrites) {
        rewritten = 1;
        for (i = 0; rule->rewrites[i].fn; i++) {
            if (!rule->rewrites[i].fn(context, &match)) {
                rewritten = 0;
                break;
            }
        }
    }
    inline_debug_pop_remap_rule(previous_rule);

    debug_site = rxcp_remap_debug_site(rule, &match);

    if (rewritten) {
        if (payload) payload->changed = 1;
        inline_remap_debug_result(context, rule, debug_site, match.symbol, "applied");
        return RXCP_REMAP_APPLIED;
    }

    inline_remap_debug_result(context, rule, debug_site, match.symbol, "rejected");
    return RXCP_REMAP_REJECTED;
}

/* Rule model example:
 * - selector: only tree shape and capture names
 * - binds: derive semantic values from captures
 * - guards: prove this matched site is safe to rewrite
 * - rewrites: perform ordered tree surgery steps
 */
static const RxcpRemapSelectorStep inline_rhs_eager_operator_selector[] = {
    RXCP_REMAP_SEL_CLASS("op", RXCP_REMAP_NODECLASS_EAGER_OPERATOR),
    RXCP_REMAP_SEL_DOWN_FIRST(),
    RXCP_REMAP_SEL_ANY("left"),
    RXCP_REMAP_SEL_NEXT_SIBLING(),
    RXCP_REMAP_SEL_TYPESET("rhs", RXCP_REMAP_TYPESET_CALL_EXPR),
    RXCP_REMAP_SEL_ASSERT_NO_NEXT_SIBLING(),
    RXCP_REMAP_SEL_END()
};

static const RxcpRemapBindStep inline_rhs_eager_operator_binds[] = {
    RXCP_REMAP_BIND_STEP("bind-inlineable-rhs-proc-symbol", inline_bind_rhs_eager_operator_proc_symbol),
    RXCP_REMAP_BIND_END()
};

static const RxcpRemapGuardStep inline_rhs_eager_operator_guards[] = {
    RXCP_REMAP_GUARD_STEP("rhs-left-capture-is-safe", inline_guard_rhs_eager_operator_capture_safe),
    RXCP_REMAP_GUARD_END()
};

static const RxcpRemapRewriteStep inline_rhs_eager_operator_rewrites[] = {
    RXCP_REMAP_REWRITE_STEP("replace-op-with-left-capture-block", inline_rewrite_rhs_eager_operator_capture),
    RXCP_REMAP_REWRITE_END()
};

static const RxcpRemapSelectorStep inline_expression_block_selector[] = {
    RXCP_REMAP_SEL_TYPESET("call", RXCP_REMAP_TYPESET_CALL_EXPR),
    RXCP_REMAP_SEL_END()
};

static const RxcpRemapBindStep inline_call_binds[] = {
    RXCP_REMAP_BIND_STEP("bind-inlineable-call-proc-symbol", inline_bind_call_proc_symbol),
    RXCP_REMAP_BIND_END()
};

static const RxcpRemapGuardStep inline_expression_block_guards[] = {
    RXCP_REMAP_GUARD_STEP("not-rhs-left-capture-case", inline_guard_expression_block_not_rhs_capture),
    RXCP_REMAP_GUARD_END()
};

static const RxcpRemapRewriteStep inline_expression_block_rewrites[] = {
    RXCP_REMAP_REWRITE_STEP("replace-call-with-block-expr", inline_rewrite_expression_block),
    RXCP_REMAP_REWRITE_END()
};

static const RxcpRemapSelectorStep inline_assignment_whole_rhs_selector[] = {
    RXCP_REMAP_SEL_TYPE("assign", ASSIGN),
    RXCP_REMAP_SEL_DOWN_FIRST(),
    RXCP_REMAP_SEL_ANY("lhs"),
    RXCP_REMAP_SEL_NEXT_SIBLING(),
    RXCP_REMAP_SEL_TYPESET("rhs", RXCP_REMAP_TYPESET_CALL_EXPR),
    RXCP_REMAP_SEL_END()
};

static const RxcpRemapBindStep inline_rhs_binds[] = {
    RXCP_REMAP_BIND_STEP("bind-inlineable-rhs-proc-symbol", inline_bind_rhs_proc_symbol),
    RXCP_REMAP_BIND_END()
};

static const RxcpRemapRewriteStep inline_assignment_whole_rhs_rewrites[] = {
    RXCP_REMAP_REWRITE_STEP("replace-assignment-with-inline-block", inline_rewrite_assignment_whole_rhs),
    RXCP_REMAP_REWRITE_END()
};

static const RxcpRemapSelectorStep inline_call_statement_selector[] = {
    RXCP_REMAP_SEL_TYPE("stmt", CALL),
    RXCP_REMAP_SEL_DOWN_FIRST(),
    RXCP_REMAP_SEL_TYPESET("call", RXCP_REMAP_TYPESET_CALL_EXPR),
    RXCP_REMAP_SEL_END()
};

static const RxcpRemapRewriteStep inline_call_statement_rewrites[] = {
    RXCP_REMAP_REWRITE_STEP("replace-call-statement-with-inline-block", inline_rewrite_call_statement),
    RXCP_REMAP_REWRITE_END()
};

static const RxcpRemapRule inline_remap_rules[] = {
    RXCP_REMAP_RULE_SELECTOR(
        "inline.rhs-eager-operator.capture-left",
        "inline.callsite",
        "eager-operator(rhs-inlineable-call)",
        10,
        inline_rhs_eager_operator_selector,
        inline_rhs_eager_operator_binds,
        inline_rhs_eager_operator_guards,
        inline_rhs_eager_operator_rewrites,
        "rhs"
    ),
    RXCP_REMAP_RULE_SELECTOR(
        "inline.expression.block",
        "inline.callsite",
        "FUNCTION|MEMBER_CALL|FACTORY_CALL",
        20,
        inline_expression_block_selector,
        inline_call_binds,
        inline_expression_block_guards,
        inline_expression_block_rewrites,
        "call"
    ),
    RXCP_REMAP_RULE_SELECTOR(
        "inline.assignment.whole-rhs",
        "inline.callsite",
        "ASSIGN(rhs-inlineable-call)",
        30,
        inline_assignment_whole_rhs_selector,
        inline_rhs_binds,
        NULL,
        inline_assignment_whole_rhs_rewrites,
        "rhs"
    ),
    RXCP_REMAP_RULE_SELECTOR(
        "inline.call.statement",
        "inline.callsite",
        "CALL(inlineable-call)",
        40,
        inline_call_statement_selector,
        inline_call_binds,
        NULL,
        inline_call_statement_rewrites,
        "call"
    )
};

RxcpRemapResult rxcp_inline_apply_remap_rules(Context *context,
                                              InlineWalkerPayload *payload,
                                              ASTNode *node) {
    size_t i;

    if (!context || !node) return RXCP_REMAP_NO_MATCH;

    for (i = 0; i < sizeof(inline_remap_rules) / sizeof(inline_remap_rules[0]); i++) {
        const RxcpRemapRule *rule = &inline_remap_rules[i];
        RxcpRemapResult result;

        if (rule->selector) {
            result = inline_remap_apply_selector_rule(rule, context, payload, node);
        } else if (rule->apply) {
            result = rule->apply(rule, context, payload, node);
        } else {
            result = RXCP_REMAP_NO_MATCH;
        }
        if (result != RXCP_REMAP_NO_MATCH) return result;
    }

    return RXCP_REMAP_NO_MATCH;
}

walker_result inline_procedure_walker(walker_direction direction, ASTNode *node, void *payload) {
    InlineWalkerPayload *inline_payload;
    Context *context;

    inline_payload = (InlineWalkerPayload *)payload;
    context = inline_payload ? inline_payload->context : NULL;

    if (direction == in) return result_normal;

    (void)rxcp_inline_apply_remap_rules(context, inline_payload, node);
    return result_normal;
}

static const char *inline_summary_selector_op_name(RxcpRemapSelectorOp op) {
    switch (op) {
        case RXCP_REMAP_SEL_END: return "end";
        case RXCP_REMAP_SEL_MATCH_CLASS: return "match-class";
        case RXCP_REMAP_SEL_MATCH_NODE_TYPE: return "match-node-type";
        case RXCP_REMAP_SEL_MATCH_ANY: return "match-any";
        case RXCP_REMAP_SEL_MATCH_TYPESET: return "match-typeset";
        case RXCP_REMAP_SEL_DOWN_FIRST: return "down-first";
        case RXCP_REMAP_SEL_NEXT_SIBLING: return "next-sibling";
        case RXCP_REMAP_SEL_UP_PARENT: return "up-parent";
        case RXCP_REMAP_SEL_ASSERT_NO_NEXT_SIBLING: return "assert-no-next-sibling";
        default: return "unknown";
    }
}

static const char *inline_summary_node_class_name(int value) {
    switch ((RxcpRemapNodeClass)value) {
        case RXCP_REMAP_NODECLASS_EAGER_OPERATOR: return "eager-operator";
        default: return "unknown-class";
    }
}

static const char *inline_summary_typeset_name(int value) {
    switch ((RxcpRemapTypeSet)value) {
        case RXCP_REMAP_TYPESET_CALL_EXPR: return "call-expression";
        default: return "unknown-typeset";
    }
}

static void inline_summary_print_selector(FILE *out, const RxcpRemapRule *rule) {
    size_t i;

    if (!rule || !rule->selector) {
        fprintf(out, "    selector: <%s>\n", rule && rule->apply ? "callback" : "service");
        return;
    }

    fprintf(out, "    selector:\n");
    for (i = 0; rule->selector[i].op != RXCP_REMAP_SEL_END; i++) {
        const RxcpRemapSelectorStep *step = &rule->selector[i];

        fprintf(out, "      - %s", inline_summary_selector_op_name(step->op));
        if (step->capture) fprintf(out, " capture=%s", step->capture);
        if (step->op == RXCP_REMAP_SEL_MATCH_CLASS) {
            fprintf(out, " class=%s", inline_summary_node_class_name(step->value));
        } else if (step->op == RXCP_REMAP_SEL_MATCH_NODE_TYPE) {
            fprintf(out, " node-type=%s", ast_ndtp((NodeType)step->value));
        } else if (step->op == RXCP_REMAP_SEL_MATCH_TYPESET) {
            fprintf(out, " typeset=%s", inline_summary_typeset_name(step->value));
        }
        fputc('\n', out);
    }
}

static void inline_summary_print_steps(FILE *out,
                                       const char *label,
                                       const RxcpRemapBindStep *binds,
                                       const RxcpRemapGuardStep *guards,
                                       const RxcpRemapRewriteStep *rewrites) {
    size_t i;

    fprintf(out, "    %s:", label);
    if (binds) {
        for (i = 0; binds[i].fn; i++) fprintf(out, " %s", binds[i].id ? binds[i].id : "(unnamed)");
    } else if (guards) {
        for (i = 0; guards[i].fn; i++) fprintf(out, " %s", guards[i].id ? guards[i].id : "(unnamed)");
    } else if (rewrites) {
        for (i = 0; rewrites[i].fn; i++) fprintf(out, " %s", rewrites[i].id ? rewrites[i].id : "(unnamed)");
    }
    fputc('\n', out);
}

static void inline_summary_print_rule(FILE *out, const RxcpRemapRule *rule) {
    const char *kind;

    if (!rule) return;

    kind = rule->selector ? "selector" : (rule->apply ? "callback" : "service");
    fprintf(out, "  - %s\n", rule->id ? rule->id : "(unnamed)");
    fprintf(out, "    phase: %s\n", rule->phase ? rule->phase : "(unknown)");
    fprintf(out, "    root: %s\n", rule->root_shape ? rule->root_shape : "(unknown)");
    fprintf(out, "    priority: %d\n", rule->priority);
    fprintf(out, "    kind: %s\n", kind);
    if (rule->debug_site_capture) fprintf(out, "    debug-capture: %s\n", rule->debug_site_capture);
    inline_summary_print_selector(out, rule);
    inline_summary_print_steps(out, "binds", rule->binds, NULL, NULL);
    inline_summary_print_steps(out, "guards", NULL, rule->guards, NULL);
    inline_summary_print_steps(out, "rewrites", NULL, NULL, rule->rewrites);
}

void rxcp_inline_print_rule_summary(FILE *out) {
    size_t i;

    if (!out) out = stderr;

    fprintf(out, "RXCP_INLINE_RULE_SUMMARY\n");
    fprintf(out, "  service-boundaries:\n");
    inline_summary_print_rule(out, &inline_remap_structural_eligibility_rule);
    inline_summary_print_rule(out, &inline_remap_bind_actuals_rule);
    fprintf(out, "  callsite-rules:\n");
    for (i = 0; i < sizeof(inline_remap_rules) / sizeof(inline_remap_rules[0]); i++) {
        inline_summary_print_rule(out, &inline_remap_rules[i]);
    }
}

void rxcp_inline_maybe_print_rule_summary(Context *context) {
    static int printed = 0;
    Context *root;
    const char *enabled;

    root = context && context->master_context ? context->master_context : context;
    enabled = getenv("RXCP_INLINE_RULE_SUMMARY");
    if (printed || !root || !enabled || !*enabled || strcmp(enabled, "0") == 0) return;

    printed = 1;
    rxcp_inline_print_rule_summary(stderr);
}
