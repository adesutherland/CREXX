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
} InlineStructuralEligibilityService;

static int inline_structural_eligibility_service(Context *context, void *payload) {
    InlineStructuralEligibilityService *service;
    InlineEligibility eligibility;
    ASTNode *node;
    Symbol *sym;

    service = (InlineStructuralEligibilityService *)payload;
    if (!service || !service->node || !service->symbol) return 0;

    node = service->node;
    sym = service->symbol;

    if (inline_proc_has_procedure_expose(node)) {
        inline_debug_log(context, node, sym, "DEBUG_INLINE",
                         "reject: procedure-level EXPOSE is not inlineable");
        sym->is_inlinable = 0;
        return 0;
    }

    if (inline_analyse_callable_eligibility(context, node, sym, 0, 0, &eligibility) != INLINE_ELIGIBILITY_OK) {
        inline_debug_log_eligibility_reject(context, node, sym, &eligibility);
        sym->is_inlinable = 0;
        return 0;
    }

    inline_debug_log(context, node, sym, "DEBUG_INLINE",
                     "accept: nodes=%d returns=%d final_return=%d cutoff=%d",
                     eligibility.check.node_count,
                     eligibility.check.return_count,
                     eligibility.return_shape.final_is_return,
                     INLINE_MAX_NODES);
    sym->is_inlinable = 1;
    sym->ast_template = node;
    return 1;
}

/* Walker to identify inlinable procedures */
walker_result identify_inlinable_walker(walker_direction direction, ASTNode *node, void *payload) {
    Context *context = (Context *)payload;

    if (direction == in) return result_normal;

    if (node->node_type == PROCEDURE ||
        node->node_type == METHOD ||
        node->node_type == FACTORY) {
        Symbol *sym;
        InlineStructuralEligibilityService service;

        sym = node->symbolNode ? node->symbolNode->symbol : NULL;
        if (sym && sym->is_inlinable && inline_symbol_has_callable_template(sym) &&
            inline_symbol_uses_imported_template(sym)) {
            return result_normal;
        }

        if (!sym || sym->is_main || !sym->scope ||
            (node->node_type == PROCEDURE && sym->scope->type == SCOPE_CLASS) ||
            ((node->node_type == METHOD || node->node_type == FACTORY) &&
             (!node->parent || node->parent->node_type != CLASS_DEF))) {
            if (sym) sym->is_inlinable = 0;
            return result_normal;
        }

        service.node = node;
        service.symbol = sym;
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

int rxcp_inline_pass(Context *context) {
    InlineWalkerPayload payload;

    if (!context || !context->ast) return 0;

    rxcp_inline_maybe_print_rule_summary(context);

    context->current_scope = 0;
    ast_wlkr(context->ast, identify_inlinable_walker, (void *)context);

    memset(&payload, 0, sizeof(payload));
    payload.context = context;

    context->current_scope = 0;
    ast_wlkr(context->ast, inline_procedure_walker, (void *)&payload);

    return payload.changed;
}

