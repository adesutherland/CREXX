/*
 * Private inline implementation fragment.
 * Included by rxcp_inline.c; not compiled separately.
 */

static int inline_eligibility_reject_is_scan_summary(InlineEligibilityReject reject) {
    return reject == INLINE_ELIGIBILITY_NODE_CUTOFF ||
           reject == INLINE_ELIGIBILITY_RETURN_COUNT_MISMATCH ||
           reject == INLINE_ELIGIBILITY_ASSEMBLER_ALIAS ||
           reject == INLINE_ELIGIBILITY_ASSEMBLER_EFFECT ||
           reject == INLINE_ELIGIBILITY_UNSUPPORTED_REFERENCE ||
           reject == INLINE_ELIGIBILITY_UNSUPPORTED_VARG_ACCESS;
}

static void inline_debug_log_eligibility_reject(Context *context,
                                               ASTNode *callable,
                                               Symbol *symbol,
                                               const InlineEligibility *eligibility) {
    if (!eligibility) {
        inline_debug_log(context, callable, symbol, "DEBUG_INLINE",
                         "reject: inline eligibility analysis failed");
        return;
    }

    switch (eligibility->reject) {
        case INLINE_ELIGIBILITY_MISSING_ARGS_OR_INSTRS:
        case INLINE_ELIGIBILITY_MISSING_INSTRS:
            inline_debug_log(context, callable, symbol, "DEBUG_INLINE",
                             "reject: procedure has no instruction list");
            return;
        case INLINE_ELIGIBILITY_RETURN_REFERENCE_CLASS:
            inline_debug_log(context, callable, symbol, "DEBUG_INLINE",
                             "reject: returns reference-bearing class");
            return;
        case INLINE_ELIGIBILITY_VARG_FORMAL_FOLLOWED:
            inline_debug_log(context, callable, symbol, "DEBUG_INLINE",
                             "reject: vararg formal is followed by additional formals");
            return;
        case INLINE_ELIGIBILITY_RETURN_SHAPE_FAILED:
            inline_debug_log(context, callable, symbol, "DEBUG_INLINE",
                             "reject: failed to analyse return shape");
            return;
        case INLINE_ELIGIBILITY_VALUE_NOT_FINAL_RETURN:
            inline_debug_log(context, callable, symbol, "DEBUG_INLINE",
                             "reject: value-returning procedure does not end in RETURN");
            return;
        case INLINE_ELIGIBILITY_VALUE_NO_RETURN:
            inline_debug_log(context, callable, symbol, "DEBUG_INLINE",
                             "reject: value-returning procedure has no RETURN");
            return;
        default:
            break;
    }

    if (inline_eligibility_reject_is_scan_summary(eligibility->reject)) {
        inline_debug_log(context, callable, symbol, "DEBUG_INLINE",
                         "reject: nodes=%d returns=%d final_return=%d assembler_alias=%d assembler_effect=%d unsupported_reference=%d class_attribute_write=%d unsupported_varg=%d cutoff=%d",
                         eligibility->check.node_count,
                         eligibility->check.return_count,
                         eligibility->return_shape.final_is_return,
                         eligibility->check.has_unsupported_assembler_alias,
                         eligibility->check.has_unsupported_assembler_effect,
                         eligibility->check.has_unsupported_reference,
                         eligibility->check.has_class_attribute_write,
                         eligibility->check.has_unsupported_varg_access,
                         INLINE_MAX_NODES);
        return;
    }

    inline_debug_log(context, callable, symbol, "DEBUG_INLINE",
                     "reject: inline eligibility analysis failed");
}

static void inline_export_debug_eligibility_reject(Context *context,
                                                  ASTNode *callable,
                                                  Symbol *symbol,
                                                  const InlineEligibility *eligibility) {
    if (!eligibility) {
        inline_export_debug_reject(context, callable, symbol, "inline eligibility analysis failed");
        return;
    }

    switch (eligibility->reject) {
        case INLINE_ELIGIBILITY_MISSING_ARGS_OR_INSTRS:
        case INLINE_ELIGIBILITY_MISSING_INSTRS:
            inline_export_debug_reject(context, callable, symbol, "missing args or instruction list");
            return;
        case INLINE_ELIGIBILITY_RETURN_REFERENCE_CLASS:
            inline_export_debug_reject(context, callable, symbol, "returns reference-bearing class");
            return;
        case INLINE_ELIGIBILITY_VARG_FORMAL_FOLLOWED:
            inline_export_debug_reject(context, callable, symbol, "unsupported vararg formal shape");
            return;
        case INLINE_ELIGIBILITY_RETURN_SHAPE_FAILED:
            inline_export_debug_reject(context, callable, symbol, "failed to analyse return shape");
            return;
        case INLINE_ELIGIBILITY_VALUE_NOT_FINAL_RETURN:
            inline_export_debug_reject(context, callable, symbol, "value procedure lacks final RETURN");
            return;
        case INLINE_ELIGIBILITY_VALUE_NO_RETURN:
            inline_export_debug_reject(context, callable, symbol, "value procedure has no RETURN");
            return;
        case INLINE_ELIGIBILITY_NODE_CUTOFF:
            inline_export_debug_reject(context,
                                       callable,
                                       symbol,
                                       "node count %d exceeds cutoff %d",
                                       eligibility->check.node_count,
                                       INLINE_MAX_NODES);
            return;
        case INLINE_ELIGIBILITY_RETURN_COUNT_MISMATCH:
            inline_export_debug_reject(context, callable, symbol, "return-shape mismatch");
            return;
        case INLINE_ELIGIBILITY_ASSEMBLER_ALIAS:
            inline_export_debug_reject(context, callable, symbol, "assembler aliasing instruction");
            return;
        case INLINE_ELIGIBILITY_ASSEMBLER_EFFECT:
            inline_export_debug_reject(context, callable, symbol, "assembler stateful instruction");
            return;
        case INLINE_ELIGIBILITY_UNSUPPORTED_REFERENCE:
            inline_export_debug_reject(context, callable, symbol, "reference operation or type");
            return;
        case INLINE_ELIGIBILITY_UNSUPPORTED_VARG_ACCESS:
            inline_export_debug_reject(context, callable, symbol, "unsupported vararg access");
            return;
        case INLINE_ELIGIBILITY_UNPORTABLE_CLASS_ATTRIBUTE_SHAPE:
            inline_export_debug_reject(context, callable, symbol, "unportable class attribute shape");
            return;
        case INLINE_ELIGIBILITY_OK:
            break;
    }

    inline_export_debug_reject(context, callable, symbol, "inline eligibility analysis failed");
}

typedef struct {
    ASTNode *node;
    Symbol *symbol;
    int quiet;
} InlineStructuralEligibilityService;

static void inline_summary_body_symbol_usage(ASTNode *node,
                                             Symbol *symbol,
                                             int *read_out,
                                             int *write_out) {
    ASTNode *child;

    if (!node || !symbol) return;
    if (node->symbolNode && node->symbolNode->symbol == symbol) {
        if (node->symbolNode->readUsage && read_out) *read_out = 1;
        if (node->symbolNode->writeUsage && write_out) *write_out = 1;
    }
    child = node->child;
    while (child) {
        inline_summary_body_symbol_usage(child, symbol, read_out, write_out);
        child = child->sibling;
    }
}

static int inline_build_callable_summary(ASTNode *callable,
                                         Symbol *symbol,
                                         const InlineEligibility *eligibility) {
    InlineCallableSummary summary;
    InlineExpansionCost cost;
    ASTNode *arg;
    size_t formal_count;
    size_t formal_index;

    if (!callable || !symbol || !eligibility || !eligibility->args || !eligibility->instrs) return 0;

    memset(&summary, 0, sizeof(summary));
    summary.schema_version = RXCP_INLINE_CALLABLE_SUMMARY_SCHEMA;
    formal_count = inline_count_siblings(eligibility->args->child);
    summary.formal_count = formal_count;
    if (formal_count) {
        summary.formals = calloc(formal_count, sizeof(InlineFormalSummary));
        if (!summary.formals) return 0;
    }

    arg = eligibility->args->child;
    formal_index = 0;
    while (arg) {
        ASTNode *formal_target;
        Symbol *formal_symbol;
        InlineFormalSummary *formal;
        int read;
        int write;

        formal_target = inline_formal_target(arg);
        formal_symbol = formal_target && formal_target->symbolNode ?
                        formal_target->symbolNode->symbol : NULL;
        if (!formal_target || formal_index >= formal_count) {
            free(summary.formals);
            return 0;
        }

        formal = &summary.formals[formal_index];
        formal->type = formal_target->value_type;
        formal->dims = formal_target->value_dims;
        if (arg->is_ref_arg) formal->flags |= RXCP_INLINE_FORMAL_BY_REF;
        if (arg->is_opt_arg) formal->flags |= RXCP_INLINE_FORMAL_OPTIONAL;
        if (arg->is_opt_arg && inline_formal_default(arg)) {
            formal->flags |= RXCP_INLINE_FORMAL_HAS_DEFAULT;
        }
        if (arg->is_varg) formal->flags |= RXCP_INLINE_FORMAL_VARG;
        read = 0;
        write = 0;
        /* Derive use facts from this callable's validated body, not from the
         * symbol's connector-array order. Imported declarations can retain
         * definition connectors outside the reconstructed template; those
         * are call-entry bindings, not body writes. */
        inline_summary_body_symbol_usage(eligibility->instrs,
                                         formal_symbol,
                                         &read,
                                         &write);
        if (read) formal->flags |= RXCP_INLINE_FORMAL_READ;
        if (write) formal->flags |= RXCP_INLINE_FORMAL_WRITTEN;
        /* Derive the proof from validated symbol use, rather than copying the
         * older emitter hint.  This lets a versioned summary open the binding
         * only when the body independently proves that the by-value formal is
         * not written. */
        if (!arg->is_ref_arg && formal_symbol && !write) {
            formal->flags |= RXCP_INLINE_FORMAL_READ_ONLY;
        }
        if (arg->is_ref_arg || !formal_symbol || formal_symbol->has_reference_target ||
            formal_target->value_dims || formal_target->value_type == TP_OBJECT ||
            formal_target->value_type == TP_REFERENCE) {
            formal->flags |= RXCP_INLINE_FORMAL_ESCAPES;
        }
        if (formal_target->value_type != TP_UNKNOWN &&
            formal_target->value_type != TP_OBJECT &&
            formal_target->value_type != TP_REFERENCE) {
            formal->flags |= RXCP_INLINE_FORMAL_EXACT_SHAPE;
        }

        arg = arg->sibling;
        formal_index++;
    }

    summary.result_type = callable->value_type;
    summary.result_dims = callable->value_dims;
    if (!eligibility->return_shape.final_is_return) {
        summary.result_flags |= RXCP_INLINE_RESULT_FALLTHROUGH;
    }
    if (eligibility->return_shape.return_count > 1) {
        summary.result_flags |= RXCP_INLINE_RESULT_MULTIPLE;
    }
    if (callable->value_dims) summary.result_flags |= RXCP_INLINE_RESULT_AGGREGATE;
    else if (callable->value_type == TP_REFERENCE || callable->value_type == TP_OBJECT) {
        summary.result_flags |= RXCP_INLINE_RESULT_REFERENCE;
    } else if (callable->value_type != TP_UNKNOWN) {
        summary.result_flags |= RXCP_INLINE_RESULT_EXACT_SCALAR;
    }
    summary.context_flags = RXCP_INLINE_CONTEXT_SOURCE_IDENTITY |
                            RXCP_INLINE_CONTEXT_TRACE_IDENTITY |
                            RXCP_INLINE_CONTEXT_NUMERIC;

    if (inline_callable_is_method(callable)) {
        Symbol **mutation_visited = NULL;
        size_t mutation_visited_count = 0;
        int receiver_mutates;

        summary.control_flags |= RXCP_INLINE_CONTROL_METHOD_RECEIVER;
        /* Carry transitive receiver mutation through residual callable
         * dependencies as part of the immutable imported-body proof.  Walk
         * the body being summarized directly: a reconstructed callable's
         * owning symbol can still carry the imported summary that this pass
         * is independently validating. */
        receiver_mutates = inline_subtree_writes_class_attribute(
                eligibility->instrs,
                &mutation_visited,
                &mutation_visited_count);
        free(mutation_visited);
        if (receiver_mutates) {
            summary.control_flags |= RXCP_INLINE_CONTROL_RECEIVER_ATTRIBUTE_WRITE;
        }
    }

    if (!inline_expansion_cost_collect(eligibility->instrs, &cost)) {
        free(summary.formals);
        return 0;
    }
    summary.structural_nodes = cost.structural_nodes;
    summary.assignments = cost.assignments;
    summary.branches = cost.branches;
    summary.calls = cost.calls;
    summary.inline_temp_definitions = cost.inline_temp_definitions;

    if (!sym_copy_inline_summary(symbol, &summary)) {
        free(summary.formals);
        return 0;
    }
    free(summary.formals);
    return 1;
}

static int inline_structural_eligibility_service(Context *context, void *payload) {
    InlineStructuralEligibilityService *service;
    InlineEligibility eligibility;
    ASTNode *node;
    Symbol *sym;

    service = (InlineStructuralEligibilityService *)payload;
    if (!service || !service->node || !service->symbol) return 0;

    node = service->node;
    sym = service->symbol;
    sym_clear_inline_summary(sym);

    if (inline_proc_has_procedure_expose(node)) {
        if (!service->quiet) {
            inline_debug_log(context, node, sym, "DEBUG_INLINE",
                             "reject: procedure-level EXPOSE is not inlineable");
        }
        sym->is_inlinable = 0;
        return 0;
    }

    if (inline_analyse_callable_eligibility(context, node, sym, 0, 0, &eligibility) != INLINE_ELIGIBILITY_OK) {
        if (!service->quiet) {
            inline_debug_log_eligibility_reject(context, node, sym, &eligibility);
        }
        sym->is_inlinable = 0;
        return 0;
    }

    if (!inline_build_callable_summary(node, sym, &eligibility)) {
        if (!service->quiet) {
            inline_debug_log(context, node, sym, "DEBUG_INLINE",
                             "reject: callable summary construction failed");
        }
        sym->is_inlinable = 0;
        return 0;
    }

    if (!service->quiet) {
        inline_debug_log(context, node, sym, "DEBUG_INLINE",
                         "accept: nodes=%d returns=%d final_return=%d cutoff=%d",
                         eligibility.check.node_count,
                         eligibility.check.return_count,
                         eligibility.return_shape.final_is_return,
                         INLINE_MAX_NODES);
    }
    sym->is_inlinable = 1;
    sym->ast_template = node;
    return 1;
}

/* Walker to identify inlinable procedures */
walker_result identify_inlinable_walker(walker_direction direction, ASTNode *node, void *payload) {
    InlineEligibilityWalkerPayload *eligibility_payload;
    Context *context;

    eligibility_payload = (InlineEligibilityWalkerPayload *)payload;
    context = eligibility_payload ? eligibility_payload->context : NULL;

    if (direction == in) return result_normal;

    if (node->node_type == PROCEDURE ||
        node->node_type == METHOD ||
        node->node_type == FACTORY) {
        Symbol *sym;
        InlineStructuralEligibilityService service;

        sym = node->symbolNode ? node->symbolNode->symbol : NULL;
        if (sym && sym->is_inlinable && inline_symbol_has_callable_template(sym) &&
            inline_symbol_uses_imported_template(sym) && sym->inline_summary &&
            sym->inline_summary->schema_version == RXCP_INLINE_CALLABLE_SUMMARY_SCHEMA) {
            return result_normal;
        }

        if (!sym || sym->is_main || !sym->scope ||
            (node->node_type == PROCEDURE && sym->scope->type == SCOPE_CLASS) ||
            ((inline_callable_is_method(node) || node->node_type == FACTORY) &&
             (!node->parent || node->parent->node_type != CLASS_DEF))) {
            if (sym) {
                sym->is_inlinable = 0;
                sym_clear_inline_summary(sym);
            }
            return result_normal;
        }

        service.node = node;
        service.symbol = sym;
        service.quiet = eligibility_payload ? eligibility_payload->quiet : 0;
        (void)rxcp_remap_run_service(context,
                                     rxcp_inline_structural_eligibility_rule(),
                                     node,
                                     sym,
                                     inline_structural_eligibility_service,
                                     &service,
                                     rxcp_inline_remap_trace_hooks());
    }
    return result_normal;
}

static void rxcp_inline_prepare_kind(Context *context, int quiet) {
    InlineEligibilityWalkerPayload payload;

    if (!context || !context->ast) return;

    rxcp_inline_maybe_print_rule_summary(context);

    payload.context = context;
    payload.quiet = quiet;
    context->current_scope = 0;
    ast_wlkr(context->ast, identify_inlinable_walker, (void *)&payload);
}

void rxcp_inline_prepare(Context *context) {
    rxcp_inline_prepare_kind(context, 0);
}

void rxcp_inline_prepare_quiet(Context *context) {
    rxcp_inline_prepare_kind(context, 1);
}

static int rxcp_inline_pass_kind(Context *context, int exact_scalar_accessors_only) {
    InlineWalkerPayload payload;

    if (!context || !context->ast) return 0;

    memset(&payload, 0, sizeof(payload));
    payload.context = context;
    payload.exact_scalar_accessors_only = exact_scalar_accessors_only;

    context->current_scope = 0;
    ast_wlkr(context->ast, inline_procedure_walker, (void *)&payload);

    return payload.changed;
}

int rxcp_inline_scalar_accessor_pass(Context *context) {
    return rxcp_inline_pass_kind(context, 1);
}

int rxcp_inline_prepared_pass(Context *context) {
    return rxcp_inline_pass_kind(context, 0);
}

int rxcp_inline_pass(Context *context) {
    rxcp_inline_prepare(context);
    return rxcp_inline_prepared_pass(context);
}
