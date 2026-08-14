/*
 * Private inline implementation fragment.
 * Included by rxcp_inline.c; not compiled separately.
 */

static InlineExprContext inline_classify_expr_context(ASTNode *node) {
    ASTNode *parent;

    if (!node) return 0;

    parent = node->parent;
    if (!parent) return 0;

    switch (parent->node_type) {
        case ASSIGN:
        case CALL:
            /* These use dedicated statement-position rewrites instead of
             * replacing the child call with a BLOCK_EXPR. */
            return INLINE_EXPR_CONTEXT_NONE;

        case FUNCTION:
        case FACTORY_CALL:
        case MEMBER_CALL:
            return inline_node_is_call_argument(node) ?
                   INLINE_EXPR_CONTEXT_EAGER_CALL_ARGUMENT :
                   INLINE_EXPR_CONTEXT_NONE;

        case IF:
        case WHILE:
        case UNTIL:
        case FOR:
        case TO:
        case BY:
            return inline_is_direct_single_value_consumer(node) ?
                   INLINE_EXPR_CONTEXT_CONTROL_CONSUMER :
                   INLINE_EXPR_CONTEXT_NONE;

        case OP_AND:
        case OP_OR:
            return inline_parent_is_short_circuit_operator(parent) ?
                   INLINE_EXPR_CONTEXT_SHORT_CIRCUIT_OPERATOR :
                   INLINE_EXPR_CONTEXT_NONE;

        case OP_TYPE_CAST:
        case OP_TYPE_IS:
        case OP_TYPEOF:
            /*
             * Type operators are direct value consumers: the first child is
             * evaluated before the cast/test/typeof operation, while any type
             * descriptor child is metadata-only for register purposes.
             */
            return parent->child == node ?
                   INLINE_EXPR_CONTEXT_EAGER_VALUE_CONSUMER :
                   INLINE_EXPR_CONTEXT_NONE;

        default:
            if (inline_parent_is_eager_operator(parent)) {
                return inline_eager_operator_context_is_safe(node) ?
                       INLINE_EXPR_CONTEXT_EAGER_OPERATOR :
                       INLINE_EXPR_CONTEXT_NONE;
            }
            return inline_is_direct_single_value_consumer(node) ?
                   INLINE_EXPR_CONTEXT_EAGER_VALUE_CONSUMER :
                   INLINE_EXPR_CONTEXT_NONE;
    }
}

static Symbol *inline_symbol_from_proc_def(ASTNode *proc_def) {
    return proc_def && proc_def->symbolNode ? proc_def->symbolNode->symbol : NULL;
}

static int inline_symbol_in_list(Symbol **symbols, size_t count, Symbol *symbol) {
    size_t i;

    if (!symbols || !symbol) return 0;

    for (i = 0; i < count; i++) {
        if (symbols[i] == symbol) return 1;
    }

    return 0;
}

static int inline_append_symbol(Symbol ***symbols_out, size_t *count_out, Symbol *symbol) {
    Symbol **new_symbols;

    if (!symbols_out || !count_out) return 0;
    if (!symbol) return 1;
    if (inline_symbol_in_list(*symbols_out, *count_out, symbol)) return 1;

    new_symbols = realloc(*symbols_out, sizeof(Symbol *) * (*count_out + 1));
    if (!new_symbols) return 0;

    *symbols_out = new_symbols;
    (*symbols_out)[*count_out] = symbol;
    (*count_out)++;
    return 1;
}

static int inline_subtree_reaches_targets(ASTNode *node,
                                          Symbol **targets,
                                          size_t target_count,
                                          Symbol ***visited,
                                          size_t *visited_count);

static int inline_symbol_reaches_targets(Symbol *start,
                                         Symbol **targets,
                                         size_t target_count,
                                         Symbol ***visited,
                                         size_t *visited_count) {
    ASTNode *instrs;

    if (inline_symbol_in_list(targets, target_count, start)) return 1;
    if (!start || !inline_symbol_has_callable_template(start)) return 1;
    if (inline_symbol_in_list(*visited, *visited_count, start)) return 0;
    if (!inline_append_symbol(visited, visited_count, start)) return 0;

    instrs = ast_chld(start->ast_template, INSTRUCTIONS, 0);
    return inline_subtree_reaches_targets(instrs, targets, target_count, visited, visited_count);
}

static int inline_subtree_reaches_targets(ASTNode *node,
                                          Symbol **targets,
                                          size_t target_count,
                                          Symbol ***visited,
                                          size_t *visited_count) {
    ASTNode *child;
    Symbol *callee_symbol;

    if (!node) return 0;
    if (inline_node_is_callable_def(node)) return 0;

    if ((node->node_type == FUNCTION ||
         node->node_type == MEMBER_CALL ||
         node->node_type == FACTORY_CALL) &&
        node->symbolNode &&
        node->symbolNode->symbol &&
        node->symbolNode->symbol->is_inlinable &&
        inline_symbol_has_callable_template(node->symbolNode->symbol)) {
        callee_symbol = node->symbolNode->symbol;
        if (inline_symbol_reaches_targets(callee_symbol, targets, target_count, visited, visited_count)) {
            return 1;
        }
    }

    child = node->child;
    while (child) {
        if (inline_subtree_reaches_targets(child, targets, target_count, visited, visited_count)) return 1;
        child = child->sibling;
    }

    return 0;
}

static int inline_call_is_recursive(ASTNode *call_node, Symbol *proc_sym) {
    ASTNode *enclosing_proc;
    Scope *scope;
    Symbol **targets;
    Symbol **visited;
    size_t target_count;
    size_t visited_count;
    int is_recursive;

    if (!call_node || !proc_sym || !proc_sym->ast_template) return 1;

    targets = NULL;
    visited = NULL;
    target_count = 0;
    visited_count = 0;
    is_recursive = 0;

    enclosing_proc = ast_proc(call_node);
    if (!inline_append_symbol(&targets, &target_count, inline_symbol_from_proc_def(enclosing_proc))) {
        free(targets);
        return 1;
    }

    scope = call_node->scope;
    while (scope) {
        ASTNode *scope_node;
        Symbol *scope_symbol;

        scope_node = scope->defining_node;
        scope_symbol = NULL;
        if (scope_node &&
            scope_node->association &&
            inline_node_is_callable_def(scope_node->association)) {
            scope_symbol = inline_symbol_from_proc_def(scope_node->association);
        }

        if (scope_symbol && !inline_append_symbol(&targets, &target_count, scope_symbol)) {
            free(targets);
            free(visited);
            return 1;
        }

        scope = scope->parent;
    }

    if (target_count > 0) {
        is_recursive = inline_symbol_reaches_targets(proc_sym, targets, target_count, &visited, &visited_count);
    }

    free(targets);
    free(visited);
    return is_recursive;
}

static int inline_numeric_setting_compatible(int caller_value, int callee_value, int inherited_value) {
    if (callee_value == inherited_value) return 1;
    return caller_value == callee_value;
}

static int inline_numeric_context_compatible(const numeric_context *caller, const numeric_context *callee) {
    if (!caller || !callee) return 0;

    return inline_numeric_setting_compatible(caller->digits, callee->digits, -1) &&
           inline_numeric_setting_compatible(caller->fuzz, callee->fuzz, -1) &&
           inline_numeric_setting_compatible(caller->form, callee->form, NUMERIC_FORM_INHERIT) &&
           inline_numeric_setting_compatible(caller->casetype, callee->casetype, CASE_INHERIT) &&
           inline_numeric_setting_compatible(caller->standard, callee->standard, NUMERIC_STANDARD_INHERIT);
}

static int inline_callable_writes_class_attribute(Symbol *start,
                                                  Symbol ***visited,
                                                  size_t *visited_count);

static int inline_native_stem_call_mutates_class_attribute(ASTNode *node) {
    ASTNode *receiver;
    Symbol *method_symbol;
    char *scope_name;
    int matches;

    if (!node || node->node_type != MEMBER_CALL ||
        !node->node_string || node->node_string_length != 3 ||
        strncasecmp(node->node_string, "set", 3) != 0 ||
        !node->symbolNode || !node->symbolNode->symbol)
        return 0;
    receiver = node->child;
    if (!receiver || !receiver->symbolNode ||
        !inline_symbol_is_class_attribute(receiver->symbolNode->symbol))
        return 0;

    method_symbol = node->symbolNode->symbol;
    if (!method_symbol->scope) return 0;
    scope_name = scp_frnm(method_symbol->scope);
    if (!scope_name) return 0;
    matches = strcmp(scope_name, "rxfnsb.stem") == 0;
    free(scope_name);
    return matches;
}

static int inline_subtree_writes_class_attribute(ASTNode *node,
                                                 Symbol ***visited,
                                                 size_t *visited_count) {
    ASTNode *child;

    if (!node) return 0;
    if (inline_node_is_callable_def(node)) return 0;

    if ((node->node_type == VAR_TARGET || node->node_type == VAR_REFERENCE) &&
        node->symbolNode &&
        inline_symbol_is_class_attribute(node->symbolNode->symbol)) {
        return 1;
    }

    if (inline_native_stem_call_mutates_class_attribute(node)) return 1;

    if ((node->node_type == FUNCTION ||
         node->node_type == MEMBER_CALL ||
         node->node_type == FACTORY_CALL) &&
        node->symbolNode &&
        node->symbolNode->symbol &&
        node->symbolNode->symbol->ast_template &&
        inline_callable_writes_class_attribute(node->symbolNode->symbol, visited, visited_count)) {
        return 1;
    }

    child = node->child;
    while (child) {
        if (inline_subtree_writes_class_attribute(child, visited, visited_count)) return 1;
        child = child->sibling;
    }

    return 0;
}

static int inline_callable_writes_class_attribute(Symbol *start,
                                                  Symbol ***visited,
                                                  size_t *visited_count) {
    ASTNode *instrs;

    if (!start) return 0;
    if (!inline_symbol_has_callable_template(start)) return 1;
    if (inline_symbol_in_list(*visited, *visited_count, start)) return 0;
    if (!inline_append_symbol(visited, visited_count, start)) return 1;

    instrs = ast_chld(start->ast_template, INSTRUCTIONS, 0);
    return inline_subtree_writes_class_attribute(instrs, visited, visited_count);
}

static int inline_method_writes_class_attribute(ASTNode *proc_def) {
    Symbol *proc_symbol;

    if (!proc_def || !inline_callable_is_method(proc_def)) return 0;

    proc_symbol = inline_symbol_from_proc_def(proc_def);
    if (!proc_symbol) return 0;

    return inline_symbol_writes_class_attribute(proc_symbol);
}

static int inline_symbol_writes_class_attribute(Symbol *symbol) {
    Symbol **visited;
    size_t visited_count;
    int result;

    if (!symbol) return 0;
    visited = NULL;
    visited_count = 0;
    result = inline_callable_writes_class_attribute(symbol, &visited, &visited_count);
    free(visited);
    return result;
}

static int inline_validate_call_site(Context *context,
                                     ASTNode *proc_def,
                                     ASTNode *call_node,
                                     Symbol *proc_sym) {
    int unsupported_varg_access;
    size_t max_required_varg_index;
    size_t varg_count;

    if (!proc_def || !call_node || !proc_sym) return 0;
    if (inline_call_is_recursive(call_node, proc_sym)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "recursive inline cycle detected");
        return 0;
    }
    if (!inline_call_arity_matches(call_node, proc_sym, &varg_count)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "call arity does not match formal arguments");
        return 0;
    }
    if (!call_node->scope ||
        !proc_def->scope ||
        !inline_numeric_context_compatible(&call_node->scope->num_context, &proc_def->scope->num_context)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "callee numeric context differs from caller context");
        return 0;
    }
    if (!proc_sym->has_vargs) return 1;

    if (!inline_analyse_varg_usage(proc_def, &unsupported_varg_access, &max_required_varg_index)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to analyse vararg usage");
        return 0;
    }
    if (unsupported_varg_access) {
        inline_debug_fail_closed(context, call_node, proc_sym, "unsupported vararg access in callee");
        return 0;
    }
    if (varg_count < max_required_varg_index) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "call provides %zu varargs but callee requires index %zu",
                                 varg_count, max_required_varg_index);
        return 0;
    }

    return 1;
}

static int inline_analyse_return_shape(ASTNode *proc_def, InlineReturnShape *shape_out) {
    ASTNode *instrs;
    ASTNode *instr;
    ASTNode *last_instr;
    InlineReturnShape shape;

    if (!proc_def) return 0;

    instrs = ast_chld(proc_def, INSTRUCTIONS, 0);
    if (!instrs) return 0;

    shape.return_count = 0;
    shape.top_level_return_count = 0;
    shape.final_is_return = 0;

    instr = instrs->child;
    last_instr = NULL;
    while (instr) {
        if (instr->node_type == RETURN) shape.top_level_return_count++;
        last_instr = instr;
        instr = instr->sibling;
    }

    shape.return_count = inline_count_return_nodes(instrs->child);
    shape.final_is_return = last_instr && last_instr->node_type == RETURN;

    if (shape_out) *shape_out = shape;
    return 1;
}

static int inline_count_return_nodes(ASTNode *node) {
    int count;

    count = 0;
    while (node) {
        if (node->node_type == RETURN) count++;
        if (node->child) count += inline_count_return_nodes(node->child);
        node = node->sibling;
    }

    return count;
}

static int inline_cost_is_branch(NodeType node_type) {
    switch (node_type) {
        case IF:
        case DO:
        case REPEAT:
        case SELECT:
        case SWITCH:
        case WHEN:
        case OPT_DISPATCH:
        case SIGNAL_BLOCK:
            return 1;
        default:
            return 0;
    }
}

static int inline_cost_is_call(NodeType node_type) {
    switch (node_type) {
        case FUNCTION:
        case MEMBER_CALL:
        case FACTORY_CALL:
        case INTRINSIC:
            return 1;
        default:
            return 0;
    }
}

static void inline_expansion_cost_walk(ASTNode *node, InlineExpansionCost *cost) {
    ASTNode *child;

    if (!node || !cost) return;

    cost->structural_nodes++;
    if (node->node_type == ASSIGN || node->node_type == DEFINE) cost->assignments++;
    if (inline_cost_is_branch(node->node_type)) cost->branches++;
    if (inline_cost_is_call(node->node_type)) cost->calls++;
    if (node->node_type == VAR_TARGET && node->symbolNode &&
        node->symbolNode->symbol && node->symbolNode->symbol->name &&
        strncmp(node->symbolNode->symbol->name, "__inline_", 9) == 0) {
        cost->inline_temp_definitions++;
    }

    if (node->node_type == PROCEDURE || node->node_type == METHOD ||
        node->node_type == FACTORY || node->node_type == MATCH) {
        return;
    }
    for (child = node->child; child; child = child->sibling) {
        inline_expansion_cost_walk(child, cost);
    }
}

static int inline_expansion_cost_collect(ASTNode *root, InlineExpansionCost *cost) {
    if (!root || !cost) return 0;

    memset(cost, 0, sizeof(*cost));
    inline_expansion_cost_walk(root, cost);
    cost->valid = cost->structural_nodes > 0;
    return cost->valid;
}

static int inline_expansion_cost_is_strict_improvement(const InlineExpansionCost *reference,
                                                       const InlineExpansionCost *candidate) {
    int strict;

    if (!reference || !candidate || !reference->valid || !candidate->valid) return 0;
    if (candidate->inline_temp_definitions > reference->inline_temp_definitions ||
        candidate->calls > reference->calls ||
        candidate->branches > reference->branches ||
        candidate->assignments > reference->assignments ||
        candidate->structural_nodes > reference->structural_nodes) {
        return 0;
    }

    strict = candidate->inline_temp_definitions < reference->inline_temp_definitions ||
             candidate->calls < reference->calls ||
             candidate->branches < reference->branches ||
             candidate->assignments < reference->assignments ||
             candidate->structural_nodes < reference->structural_nodes;
    return strict;
}

static void inline_expansion_plan_init(InlineExpansionPlan *plan,
                                       ASTNode *original_call,
                                       ASTNode *replacement_target,
                                       Scope *parent_scope,
                                       Symbol *callee_symbol,
                                       InlineExpansionKind kind) {
    if (!plan) return;

    memset(plan, 0, sizeof(*plan));
    plan->original_call = original_call;
    plan->replacement_target = replacement_target;
    plan->parent_scope = parent_scope;
    plan->callee_symbol = callee_symbol;
    plan->kind = kind;
    (void)inline_expansion_cost_collect(original_call, &plan->original_call_cost);
}

static int inline_expansion_plan_record_reference(Context *context,
                                                  InlineExpansionPlan *plan,
                                                  ASTNode *candidate_root,
                                                  int require_strict_improvement) {
    if (!plan || !candidate_root || plan->committed ||
        !inline_expansion_cost_collect(candidate_root, &plan->reference_candidate_cost)) {
        inline_debug_fail_closed(context,
                                 plan ? plan->original_call : NULL,
                                 plan ? plan->callee_symbol : NULL,
                                 "failed to record inline candidate profitability reference");
        return 0;
    }

    plan->profitability_required = require_strict_improvement;
    return 1;
}

static int inline_candidate_is_scalar_self_assignment(ASTNode *node) {
    ASTNode *lhs;
    ASTNode *rhs;
    Symbol *symbol;

    if (!node || node->node_type != ASSIGN || !node->is_compiler_added) return 0;
    lhs = node->child;
    rhs = lhs ? lhs->sibling : NULL;
    if (!lhs || !rhs || rhs->sibling || lhs->node_type != VAR_TARGET ||
        rhs->node_type != VAR_SYMBOL || !lhs->symbolNode || !rhs->symbolNode ||
        !lhs->symbolNode->symbol || lhs->symbolNode->symbol != rhs->symbolNode->symbol) {
        return 0;
    }

    symbol = lhs->symbolNode->symbol;
    if (symbol->value_dims || symbol->type == TP_OBJECT || symbol->type == TP_REFERENCE ||
        symbol->type == TP_BINARY || symbol->is_ref_arg || symbol->exposed ||
        symbol->has_reference_target) {
        return 0;
    }
    return lhs->target_type == rhs->value_type;
}

static size_t inline_candidate_cleanup_sequence(ASTNode *sequence,
                                                ASTNode *boundary_root) {
    ASTNode *node;
    ASTNode *next;
    size_t removed;

    if (!sequence || sequence->node_type != INSTRUCTIONS) return 0;
    removed = 0;
    node = sequence->child;
    while (node) {
        ASTNode *child;

        next = node->sibling;
        if (node->node_type != PROCEDURE && node->node_type != METHOD &&
            node->node_type != FACTORY && node->node_type != MATCH) {
            for (child = node->child; child; child = child->sibling) {
                if (child->node_type == INSTRUCTIONS) {
                    removed += inline_candidate_cleanup_sequence(child, boundary_root);
                }
            }
        }

        if (inline_candidate_is_scalar_self_assignment(node)) {
            ast_del(node);
            removed++;
            node = next;
            continue;
        }

        if (node->node_type == LEAVE_WITH && node->association == boundary_root) {
            ASTNode *dead;

            dead = next;
            while (dead) {
                ASTNode *dead_next = dead->sibling;
                ast_del(dead);
                removed++;
                dead = dead_next;
            }
            break;
        }
        node = next;
    }
    return removed;
}

typedef struct InlineDefaultResultProof {
    ASTNode *boundary_root;
    Symbol *symbol;
    size_t leave_count;
    int invalid;
} InlineDefaultResultProof;

static void inline_find_default_result_symbol(ASTNode *node,
                                              InlineDefaultResultProof *proof) {
    ASTNode *child;
    Symbol *symbol;

    if (!node || !proof || proof->invalid) return;
    if (node != proof->boundary_root &&
        (node->node_type == PROCEDURE || node->node_type == METHOD ||
         node->node_type == FACTORY || node->node_type == MATCH)) return;

    if (node->node_type == LEAVE_WITH && node->association == proof->boundary_root) {
        child = node->child;
        proof->leave_count++;
        if (!child || child->sibling || child->node_type != VAR_SYMBOL || child->child ||
            child->flow_substitute_symbol || !child->symbolNode ||
            !(symbol = child->symbolNode->symbol)) {
            proof->invalid = 1;
            return;
        }
        if (proof->symbol && proof->symbol != symbol) {
            proof->invalid = 1;
            return;
        }
        proof->symbol = symbol;
        return;
    }

    for (child = node->child; child; child = child->sibling) {
        inline_find_default_result_symbol(child, proof);
    }
}

static void inline_statement_result_effects(ASTNode *node,
                                            Symbol *symbol,
                                            int *reads,
                                            int *safe_assembler_write) {
    ASTNode *child;

    if (!node || !symbol) return;
    if (node->symbolNode && node->symbolNode->symbol == symbol &&
        (!node->parent || (node->parent->node_type != DEFINE &&
                           node->parent->node_type != ARG))) {
        if (node->symbolNode->readUsage && reads) *reads = 1;
        if (node->symbolNode->writeUsage && !node->symbolNode->readUsage &&
            node->parent && node->parent->node_type == ASSEMBLER &&
            safe_assembler_write) {
            *safe_assembler_write = 1;
        }
    }
    for (child = node->child; child; child = child->sibling) {
        inline_statement_result_effects(child, symbol, reads, safe_assembler_write);
    }
}

/* BLOCK_EXPR is intentionally opaque to the whole-procedure flow consumer.
 * Recover only the bounded F03 case proved inside one inline candidate: one
 * scalar local result, linear control, a classified write-only assembler
 * destination before its first read, and one final ownership-transferring
 * return. Source metadata remains; only the redundant physical default is
 * omitted. */
static void inline_candidate_prove_owned_result_default(ASTNode *candidate_root) {
    InlineDefaultResultProof proof;
    ASTNode *sequence;
    ASTNode *statement;
    Symbol *symbol;
    int safely_defined;

    if (!candidate_root || candidate_root->node_type != BLOCK_EXPR ||
        !candidate_root->association || !candidate_root->scope) return;

    memset(&proof, 0, sizeof(proof));
    proof.boundary_root = candidate_root;
    inline_find_default_result_symbol(candidate_root, &proof);
    if (proof.invalid || proof.leave_count != 1 || !(symbol = proof.symbol)) return;
    if (!symbol->needs_default_initiation || symbol->scope != candidate_root->scope ||
        symbol->symbol_type != VARIABLE_SYMBOL || symbol->inline_value_alias ||
        symbol->exposed || symbol->is_global_var || symbol->is_arg || symbol->is_ref_arg ||
        symbol->is_this || symbol->is_factory || symbol->has_reference_target ||
        symbol->value_dims || symbol->type == TP_OBJECT || symbol->type == TP_REFERENCE ||
        symbol->type == TP_BINARY) return;

    sequence = candidate_root->child;
    if (!sequence || sequence->node_type != INSTRUCTIONS) return;
    safely_defined = 0;
    for (statement = sequence->child; statement; statement = statement->sibling) {
        int reads = 0;
        int safe_assembler_write = 0;

        if (statement->node_type == IF || statement->node_type == DO ||
            statement->node_type == SIGNAL_BLOCK || statement->node_type == BLOCK_EXPR ||
            statement->node_type == SELECT || statement->node_type == SWITCH ||
            statement->node_type == OPT_DISPATCH) return;

        inline_statement_result_effects(statement,
                                        symbol,
                                        &reads,
                                        &safe_assembler_write);
        if (reads && !safely_defined) return;
        if (safe_assembler_write) safely_defined = 1;
        if (statement->node_type == LEAVE_WITH &&
            statement->association == candidate_root) break;
    }

    if (safely_defined) symbol->inline_skip_default_initiation = 1;
}

static int inline_candidate_cleanup_fixed_point(Context *context,
                                                InlineExpansionPlan *plan,
                                                ASTNode *candidate_root,
                                                InlineCloneState *clone_state) {
    InlineExpansionCost initial_cost;
    size_t bound;
    size_t iteration;
    size_t rewrites;

    if (!plan || !candidate_root || !clone_state ||
        !inline_expansion_plan_record_reference(context, plan, candidate_root, 0)) {
        return 0;
    }

    initial_cost = plan->reference_candidate_cost;
    if (clone_state->cleanup_coalesced_bindings) {
        size_t count = clone_state->cleanup_coalesced_bindings;
        /* The binding remains as a source/TRACE event, but same-register
         * lowering makes its physical assignment free. Model only that
         * emitted operation in the candidate-local profitability delta. */
        plan->reference_candidate_cost.assignments += count;
    }

    bound = initial_cost.structural_nodes + 1;
    rewrites = clone_state->cleanup_coalesced_bindings;
    for (iteration = 0; iteration < bound; iteration++) {
        ASTNode *sequence;
        size_t changed;

        sequence = candidate_root->node_type == BLOCK_EXPR ?
                   candidate_root->child : candidate_root;
        changed = inline_candidate_cleanup_sequence(sequence, candidate_root);
        rewrites += changed;
        if (!changed) break;
    }
    if (iteration == bound) {
        inline_debug_fail_closed(context,
                                 plan->original_call,
                                 plan->callee_symbol,
                                 "inline candidate cleanup did not converge within %zu iterations",
                                 bound);
        return 0;
    }

    if (rewrites) {
        InlineExpansionCost cleaned_cost;

        if (!inline_expansion_cost_collect(candidate_root, &cleaned_cost) ||
            !inline_expansion_cost_is_strict_improvement(&plan->reference_candidate_cost,
                                                         &cleaned_cost)) {
            inline_debug_fail_closed(context,
                                     plan->original_call,
                                     plan->callee_symbol,
                                     "candidate-local cleanup did not produce a strict multi-metric improvement");
            return 0;
        }
        memset(&plan->cleanup_delta, 0, sizeof(plan->cleanup_delta));
        plan->cleanup_delta.structural_nodes =
            plan->reference_candidate_cost.structural_nodes - cleaned_cost.structural_nodes;
        plan->cleanup_delta.assignments =
            plan->reference_candidate_cost.assignments - cleaned_cost.assignments;
        plan->cleanup_delta.branches =
            plan->reference_candidate_cost.branches - cleaned_cost.branches;
        plan->cleanup_delta.calls =
            plan->reference_candidate_cost.calls - cleaned_cost.calls;
        plan->cleanup_delta.inline_temp_definitions =
            plan->reference_candidate_cost.inline_temp_definitions - cleaned_cost.inline_temp_definitions;
        plan->cleanup_delta.valid = 1;
        plan->profitability_required = 1;
    }
    inline_candidate_prove_owned_result_default(candidate_root);
    return 1;
}

static int inline_expansion_plan_commit(Context *context,
                                        InlineExpansionPlan *plan,
                                        ASTNode *candidate_root) {
    if (!plan || !plan->original_call || !plan->replacement_target ||
        !candidate_root || plan->committed) {
        inline_debug_fail_closed(context,
                                 plan ? plan->original_call : NULL,
                                 plan ? plan->callee_symbol : NULL,
                                 "invalid inline expansion transaction commit");
        return 0;
    }

    if (!plan->reference_candidate_cost.valid &&
        !inline_expansion_plan_record_reference(context, plan, candidate_root, 0)) {
        return 0;
    }
    if (!inline_expansion_cost_collect(candidate_root, &plan->final_candidate_cost)) {
        inline_debug_fail_closed(context,
                                 plan->original_call,
                                 plan->callee_symbol,
                                 "failed to cost final inline candidate");
        return 0;
    }
    if (plan->profitability_required && plan->cleanup_delta.valid) {
        plan->reference_candidate_cost = plan->final_candidate_cost;
        plan->reference_candidate_cost.structural_nodes += plan->cleanup_delta.structural_nodes;
        plan->reference_candidate_cost.assignments += plan->cleanup_delta.assignments;
        plan->reference_candidate_cost.branches += plan->cleanup_delta.branches;
        plan->reference_candidate_cost.calls += plan->cleanup_delta.calls;
        plan->reference_candidate_cost.inline_temp_definitions +=
            plan->cleanup_delta.inline_temp_definitions;
        plan->reference_candidate_cost.valid = 1;
    }
    if (plan->profitability_required &&
        !inline_expansion_cost_is_strict_improvement(&plan->reference_candidate_cost,
                                                     &plan->final_candidate_cost)) {
        inline_debug_fail_closed(context,
                                 plan->original_call,
                                 plan->callee_symbol,
                                 "inline candidate is not a strict multi-metric improvement "
                                 "(nodes %zu/%zu, assignments %zu/%zu, branches %zu/%zu, "
                                 "calls %zu/%zu, inline temps %zu/%zu)",
                                 plan->final_candidate_cost.structural_nodes,
                                 plan->reference_candidate_cost.structural_nodes,
                                 plan->final_candidate_cost.assignments,
                                 plan->reference_candidate_cost.assignments,
                                 plan->final_candidate_cost.branches,
                                 plan->reference_candidate_cost.branches,
                                 plan->final_candidate_cost.calls,
                                 plan->reference_candidate_cost.calls,
                                 plan->final_candidate_cost.inline_temp_definitions,
                                 plan->reference_candidate_cost.inline_temp_definitions);
        return 0;
    }

    if (plan->profitability_required && candidate_root->node_type == BLOCK_EXPR) {
        candidate_root->is_inline_pruned = 1;
    }
    plan->candidate_root = candidate_root;
    rxcp_remap_replace_node(plan->replacement_target, candidate_root);
    plan->committed = 1;
    return 1;
}

static ASTNode *inline_create_receiver_copyback_leave_wrapper(Context *context,
                                                              ASTNode *leave_node,
                                                              ASTNode *block_expr,
                                                              Scope *inline_scope,
                                                              InlineCloneState *clone_state) {
    ASTNode *wrapper;
    ASTNode *leave_expr;
    ASTNode *assign_node;
    ASTNode *assign_lhs;
    ASTNode *temp_ref;
    Symbol *temp_symbol;

    if (!context || !leave_node || !block_expr || !inline_scope || !clone_state) return NULL;
    if (leave_node->node_type != LEAVE_WITH) return NULL;
    if (!clone_state->method_receiver_needs_copyback) return NULL;

    wrapper = ast_f(context, INSTRUCTIONS, leave_node->token);
    if (!wrapper) return NULL;
    rxcp_remap_anchor_synthetic(wrapper, leave_node);
    rxcp_remap_mark_generated_block(wrapper, 0);
    wrapper->association = block_expr;
    wrapper->scope = inline_scope;
    wrapper->value_type = TP_VOID;
    wrapper->target_type = TP_VOID;

    leave_expr = leave_node->child;
    if (!leave_expr) return NULL;

    temp_symbol = rxcp_remap_create_temp_symbol(context,
                                            inline_scope,
                                            leave_expr,
                                            "__inline_leave",
                                            (size_t)leave_node->node_number);
    if (!temp_symbol) return NULL;

    assign_node = rxcp_remap_create_assignment_node(context,
                                                    inline_scope,
                                                    leave_expr->token ? leave_expr : leave_node,
                                                    leave_expr);
    if (!assign_node) return NULL;

    assign_lhs = rxcp_remap_create_symbol_node(context,
                                           inline_scope,
                                           leave_expr,
                                           temp_symbol,
                                           VAR_TARGET,
                                           0,
                                           1);
    if (!assign_lhs) return NULL;

    leave_node->child = NULL;
    leave_expr->parent = NULL;
    leave_expr->sibling = NULL;

    rxcp_remap_append_assignment_node(wrapper, assign_node, assign_lhs, leave_expr);

    if (!inline_append_method_receiver_copyback(context,
                                                wrapper,
                                                inline_scope,
                                                leave_node,
                                                clone_state)) {
        return NULL;
    }

    temp_ref = rxcp_remap_create_symbol_node(context,
                                         inline_scope,
                                         leave_node,
                                         temp_symbol,
                                         VAR_SYMBOL,
                                         1,
                                         0);
    if (!temp_ref) return NULL;

    add_ast(leave_node, temp_ref);
    return wrapper;
}

typedef struct {
    ASTNode **node_ref;
    ASTNode *block_expr;
    Scope *inline_scope;
    int allow_dummy_return;
    ValueType proc_type;
    InlineCloneState *clone_state;
} InlineReturnRewriteService;

static int inline_rewrite_return_nodes_impl(Context *context,
                                            ASTNode **node_ref,
                                            ASTNode *block_expr,
                                            Scope *inline_scope,
                                            int allow_dummy_return,
                                            ValueType proc_type,
                                            InlineCloneState *clone_state) {
    ASTNode *node;
    ASTNode *child;
    ASTNode *next_child;
    ASTNode *leave_expr;

    if (!node_ref || !*node_ref) return 1;

    node = *node_ref;

    child = node->child;
    while (child) {
        next_child = child->sibling;
        if (!inline_rewrite_return_nodes_impl(context,
                                              &child,
                                              block_expr,
                                              inline_scope,
                                              allow_dummy_return,
                                              proc_type,
                                              clone_state)) {
            return 0;
        }
        child = next_child;
    }

    if (node->node_type != RETURN) return 1;

    if (!node->child) {
        if (!(allow_dummy_return && proc_type == TP_VOID)) return 0;

        leave_expr = rxcp_remap_create_integer_constant(context, node, 0, TP_INTEGER);
        if (!leave_expr) return 0;
        leave_expr->scope = node->scope ? node->scope : inline_scope;
        add_ast(node, leave_expr);
    }

    node->node_type = LEAVE_WITH;
    ast_str(node, "leave");
    node->association = block_expr;
    node->value_type = node->child ? node->child->value_type : TP_VOID;
    node->target_type = node->child ? node->child->target_type : TP_VOID;

    if (clone_state && clone_state->method_receiver_needs_copyback) {
        ASTNode *wrapper;

        wrapper = inline_create_receiver_copyback_leave_wrapper(context,
                                                                node,
                                                                block_expr,
                                                                inline_scope,
                                                                clone_state);
        if (!wrapper) return 0;

        if (node->parent) ast_rpl(node, wrapper);
        else *node_ref = wrapper;
        add_ast(wrapper, node);
    }

    return 1;
}

static int inline_rewrite_return_nodes_service(Context *context, void *payload) {
    InlineReturnRewriteService *service;

    service = (InlineReturnRewriteService *)payload;
    if (!service) return 0;

    return inline_rewrite_return_nodes_impl(context,
                                            service->node_ref,
                                            service->block_expr,
                                            service->inline_scope,
                                            service->allow_dummy_return,
                                            service->proc_type,
                                            service->clone_state);
}

static int inline_rewrite_return_nodes(Context *context,
                                       ASTNode **node_ref,
                                       ASTNode *block_expr,
                                       Scope *inline_scope,
                                       int allow_dummy_return,
                                       ValueType proc_type,
                                       InlineCloneState *clone_state) {
    InlineReturnRewriteService service;
    RxcpRemapResult result;
    ASTNode *site;

    service.node_ref = node_ref;
    service.block_expr = block_expr;
    service.inline_scope = inline_scope;
    service.allow_dummy_return = allow_dummy_return;
    service.proc_type = proc_type;
    service.clone_state = clone_state;
    site = node_ref ? *node_ref : NULL;

    result = rxcp_remap_run_service(context,
                                    rxcp_inline_return_rewrite_rule(),
                                    site,
                                    NULL,
                                    inline_rewrite_return_nodes_service,
                                    &service,
                                    rxcp_inline_remap_trace_hooks());
    return result == RXCP_REMAP_APPLIED;
}

static Symbol *inline_detached_receiver_guard_assignment(ASTNode *node,
                                                         ASTNode **rhs_out) {
    ASTNode *lhs;
    ASTNode *rhs;
    Symbol *symbol;

    if (rhs_out) *rhs_out = NULL;
    if (!node || node->node_type != ASSIGN) return NULL;
    lhs = node->child;
    rhs = lhs ? lhs->sibling : NULL;
    if (!lhs || !rhs || rhs->sibling || lhs->node_type != VAR_TARGET || lhs->child ||
        !lhs->symbolNode || !(symbol = lhs->symbolNode->symbol) || !symbol->name ||
        strncmp(symbol->name, "__inline_receiver_guard_", 24) != 0) {
        return NULL;
    }
    if (rhs_out) *rhs_out = rhs;
    return symbol;
}

static ASTNode *inline_clone_detached_receiver_guard(Context *context,
                                                     ASTNode *instr_list,
                                                     ASTNode *source_if,
                                                     InlineCloneState *clone_state) {
    ASTNode *temp_ref;
    ASTNode *cloned_if;
    ASTNode *cloned_condition;
    size_t suffix;

    if (!context || !instr_list || !source_if || !clone_state ||
        !clone_state->method_receiver_detach_guards ||
        !inline_receiver_guard_if_shape(source_if)) {
        return NULL;
    }

    suffix = clone_state->method_receiver_detached_guard_materialized;
    temp_ref = inline_create_temp_value_ref(context,
                                            instr_list,
                                            clone_state->inline_scope,
                                            source_if->child,
                                            clone_state,
                                            "__inline_receiver_guard",
                                            suffix);
    if (!temp_ref || temp_ref->node_type != VAR_SYMBOL || temp_ref->child ||
        temp_ref->value_type != TP_BOOLEAN || temp_ref->value_dims != 0) {
        return NULL;
    }

    cloned_if = inline_clone_body_instruction(context, source_if, clone_state);
    if (!cloned_if || !inline_receiver_guard_if_shape(cloned_if)) return NULL;
    cloned_condition = cloned_if->child;
    if (!rxcp_remap_replace_node(cloned_condition, temp_ref)) return NULL;

    clone_state->method_receiver_detached_guard_materialized++;
    return cloned_if;
}

static int inline_validate_detached_receiver_guard_clone(ASTNode *instr_list,
                                                         ASTNode *block_expr,
                                                         InlineCloneState *clone_state) {
    ASTNode *statement;
    size_t guard_count;

    if (!clone_state || !clone_state->method_receiver_detach_guards) return 1;
    if (!instr_list || instr_list->node_type != INSTRUCTIONS || !block_expr ||
        clone_state->method_receiver_detached_guard_expected == 0 ||
        clone_state->method_receiver_detached_guard_materialized !=
            clone_state->method_receiver_detached_guard_expected) {
        return 0;
    }

    statement = instr_list->child;
    while (statement && !inline_detached_receiver_guard_assignment(statement, NULL)) {
        statement = statement->sibling;
    }

    guard_count = 0;
    while (statement && guard_count < clone_state->method_receiver_detached_guard_expected) {
        ASTNode *rhs;
        ASTNode *guard_if;
        ASTNode *condition;
        ASTNode *leave;
        Symbol *guard_symbol;
        size_t attribute_reads;

        guard_symbol = inline_detached_receiver_guard_assignment(statement, &rhs);
        guard_if = statement->sibling;
        condition = guard_if ? guard_if->child : NULL;
        leave = condition ? condition->sibling : NULL;
        attribute_reads = 0;
        if (!guard_symbol || !rhs ||
            !inline_receiver_guard_condition_walk(rhs, &attribute_reads) ||
            attribute_reads != 1 || !guard_if || guard_if->node_type != IF ||
            !condition || condition->node_type != VAR_SYMBOL || condition->child ||
            condition->value_type != TP_BOOLEAN || condition->value_dims != 0 ||
            !condition->symbolNode || condition->symbolNode->symbol != guard_symbol ||
            !leave || leave->node_type != LEAVE_WITH || leave->sibling ||
            leave->association != block_expr || leave->value_type != TP_BOOLEAN ||
            leave->value_dims != 0 || !leave->child || leave->child->sibling) {
            return 0;
        }
        guard_count++;
        statement = guard_if->sibling;
    }

    return guard_count == clone_state->method_receiver_detached_guard_expected &&
           statement && statement->node_type == LEAVE_WITH &&
           statement->association == block_expr && statement->value_type == TP_BOOLEAN &&
           statement->value_dims == 0 && statement->child &&
           !statement->child->sibling && !statement->sibling;
}

static ASTNode *inline_build_block_expr(Context *context,
                                        ASTNode *call_node,
                                        Symbol *proc_sym,
                                        Scope *parent_scope,
                                        int allow_dummy_return,
                                        InlineExpansionPlan *expansion_plan) {
    ASTNode *proc_def;
    ASTNode *proc_instrs;
    ASTNode *block_expr;
    ASTNode *instr_list;
    ASTNode *proc_instr;
    Scope *inline_scope;
    InlineCloneState clone_state;
    InlineReturnShape return_shape;

    if (!context || !call_node || !proc_sym || !proc_sym->ast_template || !parent_scope) return NULL;

    proc_def = proc_sym->ast_template;
    if (!proc_def || !proc_def->scope) {
        inline_debug_fail_closed(context, call_node, proc_sym, "callee has no inlineable procedure scope");
        return NULL;
    }

    if (!inline_validate_call_site(context, proc_def, call_node, proc_sym)) return NULL;

    block_expr = rxcp_remap_create_block_expr(context,
                                              parent_scope,
                                              call_node,
                                              proc_def,
                                              &inline_scope,
                                              &instr_list);
    if (!block_expr) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to create BLOCK_EXPR inline scaffold");
        return NULL;
    }

    if (allow_dummy_return && proc_sym->type == TP_VOID) {
        ast_set_value_type(0, block_expr, TP_INTEGER, 0, 0, 0, 0);
        ast_set_target_type(0, block_expr, TP_INTEGER, 0, 0, 0, 0);
    }

    memset(&clone_state, 0, sizeof(clone_state));

    if (!inline_build_symbol_map(proc_def->scope, inline_scope, &clone_state)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to build inline symbol/scope map");
        inline_free_symbol_map(&clone_state);
        return NULL;
    }

    if (!inline_bind_call_arguments(context, instr_list, inline_scope, proc_def, call_node, proc_sym, &clone_state)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to bind inline call arguments");
        inline_free_symbol_map(&clone_state);
        return NULL;
    }

    proc_instrs = ast_chld(proc_def, INSTRUCTIONS, 0);
    if (!proc_instrs) {
        inline_debug_fail_closed(context, call_node, proc_sym, "callee has no instruction list");
        inline_free_symbol_map(&clone_state);
        return NULL;
    }

    proc_instr = proc_instrs->child;
    while (proc_instr) {
        ASTNode *cloned_instr;

        if (clone_state.method_receiver_detach_guards && proc_instr->node_type == IF) {
            cloned_instr = inline_clone_detached_receiver_guard(context,
                                                                instr_list,
                                                                proc_instr,
                                                                &clone_state);
        } else {
            cloned_instr = inline_clone_body_instruction(context, proc_instr, &clone_state);
        }
        if (!cloned_instr) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to clone callee instruction subtree");
            inline_free_symbol_map(&clone_state);
            return NULL;
        }
        if (!inline_rewrite_return_nodes(context,
                                         &cloned_instr,
                                         block_expr,
                                         inline_scope,
                                         allow_dummy_return,
                                         proc_sym->type,
                                         &clone_state)) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to rewrite return nodes for BLOCK_EXPR inline");
            inline_free_symbol_map(&clone_state);
            return NULL;
        }
        add_ast(instr_list, cloned_instr);

        proc_instr = proc_instr->sibling;
    }

    if (!inline_validate_detached_receiver_guard_clone(instr_list,
                                                       block_expr,
                                                       &clone_state)) {
        inline_debug_fail_closed(context,
                                 call_node,
                                 proc_sym,
                                 "detached receiver-guard clone failed post-rewrite validation");
        inline_free_symbol_map(&clone_state);
        return NULL;
    }

    if (allow_dummy_return && proc_sym->type == TP_VOID &&
        inline_analyse_return_shape(proc_def, &return_shape) &&
        !return_shape.final_is_return) {
        ASTNode *leave_with;
        ASTNode *leave_expr;

        leave_expr = rxcp_remap_create_integer_constant(context, call_node, 0, TP_INTEGER);
        if (!leave_expr) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to create dummy LEAVE_WITH expression");
            inline_free_symbol_map(&clone_state);
            return NULL;
        }
        leave_expr->scope = inline_scope;

        leave_with = rxcp_remap_append_leave_with(context,
                                                  instr_list,
                                                  inline_scope,
                                                  call_node,
                                                  block_expr,
                                                  leave_expr);
        if (!leave_with) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to create dummy LEAVE_WITH node");
            inline_free_symbol_map(&clone_state);
            return NULL;
        }
    }

    if (!inline_candidate_cleanup_fixed_point(context,
                                              expansion_plan,
                                              block_expr,
                                              &clone_state)) {
        inline_free_symbol_map(&clone_state);
        return NULL;
    }
    inline_free_symbol_map(&clone_state);
    return block_expr;
}

static int ast_inline_statement(Context *context,
                                ASTNode *statement_node,
                                ASTNode *call_node,
                                Symbol *proc_sym,
                                InlineReturnPlan *return_plan) {
    ASTNode *proc_def;
    ASTNode *proc_instrs;
    ASTNode *block;
    ASTNode *instr_list;
    ASTNode *proc_instr;
    Scope *inline_scope;
    InlineCloneState clone_state;
    InlineExpansionPlan expansion_plan;
    int receiver_copyback_appended;

    if (!context || !statement_node || !call_node || !proc_sym || !proc_sym->ast_template) return 0;

    inline_expansion_plan_init(&expansion_plan,
                               call_node,
                               statement_node,
                               statement_node->scope,
                               proc_sym,
                               INLINE_EXPANSION_STATEMENT);

    proc_def = proc_sym->ast_template;
    if (!proc_def || !proc_def->scope) {
        inline_debug_fail_closed(context, call_node, proc_sym, "callee has no inlineable procedure scope");
        return 0;
    }

    if (!inline_validate_call_site(context, proc_def, call_node, proc_sym)) return 0;

    block = rxcp_remap_create_generated_instruction_block(context,
                                                          statement_node->scope,
                                                          call_node,
                                                          statement_node,
                                                          proc_def,
                                                          RXCP_REMAP_GENERATED_BLOCK_PRIMARY_REPORTING,
                                                          &inline_scope);
    if (!block) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to create compiler-generated statement block");
        return 0;
    }
    instr_list = block;

    memset(&clone_state, 0, sizeof(clone_state));
    receiver_copyback_appended = 0;

    if (!inline_build_symbol_map(proc_def->scope, inline_scope, &clone_state)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to build inline symbol/scope map");
        return 0;
    }

    if (!inline_bind_call_arguments(context, instr_list, inline_scope, proc_def, call_node, proc_sym, &clone_state)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to bind inline call arguments");
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    proc_instrs = ast_chld(proc_def, INSTRUCTIONS, 0);
    if (!proc_instrs) {
        inline_debug_fail_closed(context, call_node, proc_sym, "callee has no instruction list");
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

            if (return_plan && return_plan->return_target) {
                Scope *caller_scope;

                caller_scope = call_node->scope ? call_node->scope :
                               (return_plan->return_target->scope ?
                                return_plan->return_target->scope :
                                inline_scope);
                ret_assign->scope = caller_scope;
                ret_lhs = inline_clone_subtree_in_scope(context,
                                                        return_plan->return_target,
                                                        &clone_state,
                                                        caller_scope);
            } else if (return_plan && return_plan->return_sink_symbol) {
                ret_assign->scope = inline_scope;
                ret_lhs = rxcp_remap_create_sink_target(context,
                                                        inline_scope,
                                                        proc_instr,
                                                        proc_instr->child,
                                                        "__inline_unused");
            } else {
                inline_debug_fail_closed(context, call_node, proc_sym, "missing return target/sink during statement inline");
                inline_free_symbol_map(&clone_state);
                return 0;
            }

            if (!ret_lhs) {
                inline_debug_fail_closed(context, call_node, proc_sym, "failed to build return assignment target");
                inline_free_symbol_map(&clone_state);
                return 0;
            }

            if (clone_state.method_receiver_needs_copyback) {
                ret_rhs = inline_create_temp_value_ref(context,
                                                       instr_list,
                                                       inline_scope,
                                                       ret_expr,
                                                       &clone_state,
                                                       "__inline_ret",
                                                       (size_t)proc_instr->node_number);
                if (!ret_rhs) {
                    inline_debug_fail_closed(context, call_node, proc_sym, "failed to capture return value before receiver copyback");
                    inline_free_symbol_map(&clone_state);
                    return 0;
                }

                if (!inline_append_method_receiver_copyback(context,
                                                            instr_list,
                                                            inline_scope,
                                                            call_node,
                                                            &clone_state)) {
                    inline_debug_fail_closed(context, call_node, proc_sym, "failed to append method receiver copyback before return assignment");
                    inline_free_symbol_map(&clone_state);
                    return 0;
                }
                receiver_copyback_appended = 1;
            } else if (inline_node_has_array_shape(ret_expr) ||
                (inline_node_needs_attr_copy(ret_expr) &&
                 (ret_expr->value_type == TP_BINARY || ret_expr->target_type == TP_BINARY))) {
                ret_rhs = inline_clone_subtree(context, ret_expr, &clone_state);
                if (!ret_rhs) {
                    inline_debug_fail_closed(context, call_node, proc_sym, "failed to clone aggregate return expression");
                    inline_free_symbol_map(&clone_state);
                    return 0;
                }

                if (!inline_is_direct_symbol_actual(ret_expr)) {
                    ret_rhs = inline_create_temp_value_ref(context,
                                                           instr_list,
                                                           inline_scope,
                                                           ret_expr,
                                                           &clone_state,
                                                           "__inline_ret",
                                                           0);
                }

                if (!ret_rhs) {
                    inline_debug_fail_closed(context, call_node, proc_sym, "failed to materialise aggregate return temp");
                    inline_free_symbol_map(&clone_state);
                    return 0;
                }
            } else {
                ret_rhs = inline_clone_subtree(context, ret_expr, &clone_state);
                if (!ret_rhs) {
                    inline_debug_fail_closed(context, call_node, proc_sym, "failed to clone scalar return expression");
                    inline_free_symbol_map(&clone_state);
                    return 0;
                }
            }

            /* Aggregate caller locals can use the direct register-copy form.
             * A class attribute is receiver-owned storage, so keep the
             * inlined body but route its result through the ordinary ASSIGN
             * emitter; that emitter materialises link/copy/unlink for the
             * receiver slot. */
            if ((inline_node_has_array_shape(ret_rhs) ||
                 (inline_node_needs_attr_copy(ret_rhs) &&
                  (ret_rhs->value_type == TP_BINARY || ret_rhs->target_type == TP_BINARY))) &&
                !(ret_lhs->symbolNode && ret_lhs->symbolNode->symbol &&
                  inline_symbol_is_class_attribute(ret_lhs->symbolNode->symbol))) {
                ASTNode *ret_copy;

                ret_copy = rxcp_remap_create_register_copy_instr(context, inline_scope, "copy", ret_lhs, ret_rhs);
                if (!ret_copy) {
                    inline_debug_fail_closed(context, call_node, proc_sym, "failed to create return copy instructions");
                    inline_free_symbol_map(&clone_state);
                    return 0;
                }
                add_ast(instr_list, ret_copy);
            } else {
                rxcp_remap_append_assignment_node(instr_list, ret_assign, ret_lhs, ret_rhs);
            }
        } else {
            ASTNode *cloned_instr;

            cloned_instr = inline_clone_body_instruction(context, proc_instr, &clone_state);
            if (!cloned_instr) {
                inline_debug_fail_closed(context, call_node, proc_sym, "failed to clone statement instruction subtree");
                inline_free_symbol_map(&clone_state);
                return 0;
            }
            add_ast(instr_list, cloned_instr);
        }

        proc_instr = proc_instr->sibling;
    }

    if (!receiver_copyback_appended) {
        if (!inline_append_method_receiver_copyback(context,
                                                    instr_list,
                                                    inline_scope,
                                                    call_node,
                                                    &clone_state)) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to append method receiver copyback");
            inline_free_symbol_map(&clone_state);
            return 0;
        }
    }

    if (!inline_candidate_cleanup_fixed_point(context,
                                              &expansion_plan,
                                              block,
                                              &clone_state)) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }
    if (!inline_expansion_plan_commit(context, &expansion_plan, block)) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }
    inline_free_symbol_map(&clone_state);

    return 1;
}

int ast_inline_assignment(Context *context, ASTNode *assign_node, ASTNode *call_node, Symbol *proc_sym) {
    ASTNode *lhs;
    ASTNode *block_expr;
    ASTNode *proc_def;
    InlineReturnShape return_shape;
    InlineReturnPlan return_plan;
    InlineExpansionPlan expansion_plan;
    int method_needs_receiver_copyback;

    if (!assign_node || !call_node) return 0;

    inline_expansion_plan_init(&expansion_plan,
                               call_node,
                               call_node,
                               assign_node->scope,
                               proc_sym,
                               INLINE_EXPANSION_ASSIGNMENT_EXPRESSION);

    lhs = assign_node->child;
    if (!lhs || lhs->node_type != VAR_TARGET) {
        inline_debug_fail_closed(context, call_node, proc_sym, "assignment inline requires a plain VAR_TARGET lhs");
        return 0;
    }
    memset(&return_plan, 0, sizeof(return_plan));
    return_plan.return_target = lhs;

    proc_def = proc_sym ? proc_sym->ast_template : NULL;
    if (!proc_def || !inline_analyse_return_shape(proc_def, &return_shape)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to analyse callee return shape for assignment inline");
        return 0;
    }
    method_needs_receiver_copyback = inline_method_writes_class_attribute(proc_def);
    if (inline_callable_is_method(proc_def) &&
        inline_symbol_uses_imported_template(proc_sym) &&
        !inline_is_direct_symbol_actual(inline_call_receiver(call_node))) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "imported method assignment inline requires a direct receiver");
        return 0;
    }
    if (method_needs_receiver_copyback &&
        inline_callable_is_method(proc_def) &&
        !inline_is_supported_receiver_copyback_target(inline_call_receiver(call_node))) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "mutating method assignment inline requires a supported receiver copyback target");
        return 0;
    }
    if (!return_shape.final_is_return || return_shape.return_count == 0) {
        inline_debug_fail_closed(context, call_node, proc_sym, "assignment inline requires a final value RETURN");
        return 0;
    }
    if ((assign_node->parent && assign_node->parent->node_type == REPEAT) ||
        lhs->child ||
        (proc_sym && proc_sym->value_dims > 0)) {
        if (method_needs_receiver_copyback) {
            inline_debug_fail_closed(context, call_node, proc_sym,
                                     "mutating method assignment inline requires statement-position copyback");
            return 0;
        }
        block_expr = inline_build_block_expr(context,
                                             call_node,
                                             proc_sym,
                                             assign_node->scope,
                                             0,
                                             &expansion_plan);
        if (!block_expr) return 0;
        return inline_expansion_plan_commit(context, &expansion_plan, block_expr);
    }
    if (return_shape.return_count != 1) {
        if (method_needs_receiver_copyback) {
            inline_debug_fail_closed(context, call_node, proc_sym,
                                     "mutating method multi-return assignment inline requires statement-position copyback");
            return 0;
        }
        block_expr = inline_build_block_expr(context,
                                             call_node,
                                             proc_sym,
                                             assign_node->scope,
                                             0,
                                             &expansion_plan);
        if (!block_expr) return 0;
        return inline_expansion_plan_commit(context, &expansion_plan, block_expr);
    }

    return ast_inline_statement(context, assign_node, call_node, proc_sym, &return_plan);
}

int ast_inline_call(Context *context, ASTNode *call_stmt, ASTNode *call_node, Symbol *proc_sym) {
    ASTNode *proc_def;
    ASTNode *block;
    Scope *block_scope;
    ASTNode *block_expr;
    ASTNode *sink_assign;
    ASTNode *sink_lhs;
    InlineReturnShape return_shape;
    InlineReturnPlan return_plan;
    InlineExpansionPlan expansion_plan;
    int method_needs_receiver_copyback;

    inline_expansion_plan_init(&expansion_plan,
                               call_node,
                               call_stmt,
                               call_stmt ? call_stmt->scope : NULL,
                               proc_sym,
                               INLINE_EXPANSION_CALL_EXPRESSION);

    proc_def = proc_sym ? proc_sym->ast_template : NULL;
    if (!proc_def || !inline_analyse_return_shape(proc_def, &return_shape)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to analyse callee return shape for call inline");
        return 0;
    }
    if (return_shape.return_count == 0) {
        if (proc_sym->type != TP_VOID) {
            inline_debug_fail_closed(context, call_node, proc_sym, "value-returning callee has no RETURN");
            return 0;
        }
    } else if (!return_shape.final_is_return) {
        if (proc_sym->type != TP_VOID) {
            inline_debug_fail_closed(context, call_node, proc_sym, "call inline requires a final RETURN for value-producing callees");
            return 0;
        }
    }

    method_needs_receiver_copyback = inline_method_writes_class_attribute(proc_def);
    if (inline_callable_is_method(proc_def) &&
        inline_symbol_uses_imported_template(proc_sym) &&
        !inline_is_direct_symbol_actual(inline_call_receiver(call_node))) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "imported method call inline requires a direct receiver");
        return 0;
    }
    if (method_needs_receiver_copyback &&
        inline_callable_is_method(proc_def) &&
        !inline_is_supported_receiver_copyback_target(inline_call_receiver(call_node))) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "mutating method call inline requires a supported receiver copyback target");
        return 0;
    }
    if ((proc_sym->type == TP_VOID && (return_shape.return_count != 1 || !return_shape.final_is_return)) ||
        (proc_sym->type != TP_VOID && return_shape.return_count != 1)) {
        if (method_needs_receiver_copyback) {
            inline_debug_fail_closed(context, call_node, proc_sym,
                                     "mutating method call inline requires statement-position copyback");
            return 0;
        }
        block = rxcp_remap_create_generated_instruction_block(context,
                                                              call_stmt->scope,
                                                              call_node,
                                                              call_stmt,
                                                              proc_def,
                                                              RXCP_REMAP_GENERATED_BLOCK_PRIMARY_REPORTING,
                                                              &block_scope);
        if (!block) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to create compiler-generated sink block");
            return 0;
        }

        block_expr = inline_build_block_expr(context,
                                             call_node,
                                             proc_sym,
                                             block_scope,
                                             1,
                                             &expansion_plan);
        if (!block_expr) return 0;

        sink_assign = rxcp_remap_create_assignment_node(context,
                                                        block_scope,
                                                        call_node,
                                                        block_expr);
        if (!sink_assign) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to create sink assignment");
            return 0;
        }

        sink_lhs = rxcp_remap_create_sink_target(context,
                                                 block_scope,
                                                 call_node,
                                                 block_expr,
                                                 "__inline_unused");
        if (!sink_lhs) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to create unused return sink target");
            return 0;
        }

        rxcp_remap_append_assignment_node(block, sink_assign, sink_lhs, block_expr);

        return inline_expansion_plan_commit(context, &expansion_plan, block);
    }

    memset(&return_plan, 0, sizeof(return_plan));
    return_plan.return_sink_symbol = proc_sym;

    return ast_inline_statement(context, call_stmt, call_node, proc_sym, &return_plan);
}

int ast_inline_expression(Context *context, ASTNode *call_node, Symbol *proc_sym) {
    ASTNode *block_expr;
    InlineExpansionPlan expansion_plan;
    InlineExprContext expr_context;
    InlineReturnShape return_shape;

    if (!context || !call_node || !proc_sym || !proc_sym->ast_template) return 0;

    inline_expansion_plan_init(&expansion_plan,
                               call_node,
                               call_node,
                               call_node->scope,
                               proc_sym,
                               INLINE_EXPANSION_VALUE_EXPRESSION);

    expr_context = inline_classify_expr_context(call_node);
    if (expr_context == INLINE_EXPR_CONTEXT_NONE) {
        ASTNode *parent;

        parent = call_node->parent;
        if (parent && (parent->node_type == ASSIGN || parent->node_type == CALL)) {
            inline_debug_fail_closed(context, call_node, proc_sym,
                                     "expression context belongs to a dedicated statement rewrite");
        } else {
            inline_debug_fail_closed(context, call_node, proc_sym,
                                     "BLOCK_EXPR expression inline requires a direct single-value consumer");
        }
        return 0;
    }
    if (!inline_analyse_return_shape(proc_sym->ast_template, &return_shape)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to analyse callee return shape for expression inline");
        return 0;
    }
    if (inline_method_writes_class_attribute(proc_sym->ast_template) &&
        inline_callable_is_method(proc_sym->ast_template) &&
        !inline_is_direct_receiver_copyback_target(inline_call_receiver(call_node))) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "mutating method expression inline requires a direct receiver copyback target");
        return 0;
    }
    if (inline_callable_is_method(proc_sym->ast_template) &&
        inline_symbol_uses_imported_template(proc_sym) &&
        !inline_is_direct_symbol_actual(inline_call_receiver(call_node))) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "imported method expression inline requires a direct receiver");
        return 0;
    }
    if (!return_shape.final_is_return || return_shape.return_count == 0) {
        inline_debug_fail_closed(context, call_node, proc_sym, "expression inline requires a final value RETURN");
        return 0;
    }

    block_expr = inline_build_block_expr(context,
                                         call_node,
                                         proc_sym,
                                         call_node->scope,
                                         0,
                                         &expansion_plan);
    if (!block_expr) return 0;

    return inline_expansion_plan_commit(context, &expansion_plan, block_expr);
}

int ast_inline_rhs_eager_operator(Context *context,
                                  ASTNode *op_node,
                                  ASTNode *rhs_call,
                                  Symbol *proc_sym) {
    ASTNode *left;
    ASTNode *block_expr;
    ASTNode *instr_list;
    ASTNode *assign_node;
    ASTNode *assign_lhs;
    ASTNode *assign_rhs;
    ASTNode *leave_node;
    ASTNode *op_expr;
    ASTNode *temp_ref;
    ASTNode *rhs_expr;
    Scope *parent_scope;
    Scope *inline_scope;
    Symbol *left_symbol;
    InlineCloneState clone_state;
    InlineExpansionPlan expansion_plan;

    if (!context || !op_node || !rhs_call) return 0;
    if (!inline_parent_is_eager_operator(op_node)) return 0;

    left = op_node->child;
    if (!left || left->sibling != rhs_call || rhs_call->sibling) return 0;

    parent_scope = op_node->scope ? op_node->scope :
                   (rhs_call->scope ? rhs_call->scope : left->scope);
    if (!parent_scope) {
        inline_debug_fail_closed(context, rhs_call, proc_sym,
                                 "RHS eager-operator inline requires a parent scope");
        return 0;
    }

    inline_expansion_plan_init(&expansion_plan,
                               rhs_call,
                               op_node,
                               parent_scope,
                               proc_sym,
                               INLINE_EXPANSION_EAGER_OPERATOR);

    block_expr = rxcp_remap_create_block_expr(context,
                                              parent_scope,
                                              op_node,
                                              NULL,
                                              &inline_scope,
                                              &instr_list);
    if (!block_expr) {
        inline_debug_fail_closed(context, rhs_call, proc_sym,
                                 "failed to create RHS eager-operator BLOCK_EXPR");
        return 0;
    }

    left_symbol = rxcp_remap_create_temp_symbol(context,
                                            inline_scope,
                                            left,
                                            "__inline_lhs",
                                            (size_t)op_node->node_number);
    if (!left_symbol) {
        inline_debug_fail_closed(context, rhs_call, proc_sym,
                                 "failed to create RHS eager-operator left temp");
        return 0;
    }

    memset(&clone_state, 0, sizeof(clone_state));
    clone_state.inline_scope = inline_scope;

    assign_rhs = inline_clone_subtree_in_scope(context, left, &clone_state, inline_scope);
    if (!assign_rhs) {
        inline_debug_fail_closed(context, rhs_call, proc_sym,
                                 "failed to clone RHS eager-operator left operand");
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    rhs_expr = inline_clone_subtree_in_scope(context, rhs_call, &clone_state, inline_scope);
    if (!rhs_expr) {
        inline_debug_fail_closed(context, rhs_call, proc_sym,
                                 "failed to clone RHS eager-operator right operand");
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    assign_node = rxcp_remap_create_assignment_node(context,
                                                    inline_scope,
                                                    left,
                                                    assign_rhs);
    if (!assign_node) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    assign_lhs = rxcp_remap_create_symbol_node(context,
                                           inline_scope,
                                           left,
                                           left_symbol,
                                           VAR_TARGET,
                                           0,
                                           1);
    if (!assign_lhs) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    rxcp_remap_append_assignment_node(instr_list, assign_node, assign_lhs, assign_rhs);

    op_expr = ast_dup(context, op_node);
    if (!op_expr) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }
    op_expr->scope = inline_scope;

    temp_ref = rxcp_remap_create_symbol_node(context,
                                         inline_scope,
                                         left,
                                         left_symbol,
                                         VAR_SYMBOL,
                                         1,
                                         0);
    if (!temp_ref) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    add_ast(op_expr, temp_ref);
    add_ast(op_expr, rhs_expr);

    leave_node = rxcp_remap_append_leave_with(context,
                                              instr_list,
                                              inline_scope,
                                              op_node,
                                              block_expr,
                                              op_expr);
    if (!leave_node) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    inline_free_symbol_map(&clone_state);
    return inline_expansion_plan_commit(context, &expansion_plan, block_expr);
}

static walker_result inlinable_check_walker(walker_direction direction, ASTNode *node, void *payload) {
    InlinableCheck *check;

    check = (InlinableCheck *)payload;

    if (direction == in) {
        check->node_count++;

        if (node->node_type == RETURN) {
            check->return_count++;
        }

        if (inline_assembler_has_unsupported_aliasing(node)) {
            check->has_unsupported_assembler_alias = 1;
        }
        if (inline_assembler_has_unsupported_effect(node)) {
            check->has_unsupported_assembler_effect = 1;
        }

        if (node->node_type == OP_REFERENCE ||
            node->node_type == OP_DEREFERENCE ||
            node->node_type == OP_SNAPSHOT ||
            node->node_type == OP_REFVALID ||
            node->node_type == TYPE_REFERENCE ||
            node->value_type == TP_REFERENCE ||
            node->target_type == TP_REFERENCE ||
            (node->value_type == TP_OBJECT &&
             inline_class_has_reference_attribute(check->context, node->scope, node->value_class)) ||
            (node->target_type == TP_OBJECT &&
             inline_class_has_reference_attribute(check->context, node->scope, node->target_class)) ||
            (node->symbolNode && node->symbolNode->symbol &&
             (node->symbolNode->symbol->type == TP_REFERENCE ||
              (node->symbolNode->symbol->type == TP_OBJECT &&
               inline_class_has_reference_attribute(check->context,
                                                    node->symbolNode->symbol->scope,
                                                    node->symbolNode->symbol->value_class))))) {
            check->has_unsupported_reference = 1;
        }

        if (node->symbolNode &&
            node->symbolNode->symbol &&
            (node->node_type == VAR_TARGET || node->node_type == VAR_REFERENCE) &&
            inline_symbol_is_class_attribute(node->symbolNode->symbol)) {
            check->has_class_attribute_write = 1;
        }

        if (node->symbolNode &&
            node->symbolNode->symbol &&
            !inline_class_attribute_shape_is_portable(node->symbolNode->symbol)) {
            check->has_unportable_class_attribute_shape = 1;
        }

        if (node->node_type == OP_ARG_VALUE) {
            size_t index;

            if (!node->child) {
                check->has_unsupported_varg_access = 1;
            } else if (inline_varg_index_from_node(node->child, &index)) {
                if (index > check->max_required_varg_index) check->max_required_varg_index = index;
            } else {
                check->has_unsupported_varg_access = 1;
            }
        } else if (node->node_type == OP_ARG_IX_EXISTS) {
            if (!node->child) {
                check->has_unsupported_varg_access = 1;
            } else if (!inline_varg_index_from_node(node->child, NULL)) {
                check->has_unsupported_varg_access = 1;
            }
        }
    }
    return result_normal;
}

/* Reference values are weak alias descriptors.  The general reference AST
 * surface still needs full lifetime/alias proof, but these two method bodies
 * have no such ambiguity: the getter copies one receiver-owned descriptor to
 * the return path, while the setter copies one required by-value descriptor
 * into one receiver-owned attribute.  The existing inline transaction keeps
 * receiver evaluation, formal capture, copyback and source/TRACE ordering.
 */
static InlineReferenceAccessorKind inline_exact_reference_accessor_kind(ASTNode *callable,
                                                                        ASTNode *args,
                                                                        ASTNode *instrs) {
    ASTNode *first;

    if (!callable || !args || !instrs || !inline_callable_is_method(callable)) {
        return INLINE_REFERENCE_ACCESSOR_NONE;
    }

    first = instrs->child;
    if (!first) return INLINE_REFERENCE_ACCESSOR_NONE;

    if (!args->child && callable->value_type == TP_REFERENCE && callable->value_dims == 0) {
        ASTNode *result;
        Symbol *attribute;

        if (first->node_type != RETURN || first->sibling) return INLINE_REFERENCE_ACCESSOR_NONE;
        result = first->child;
        if (!result || result->sibling || result->child ||
            result->node_type != VAR_SYMBOL || result->value_type != TP_REFERENCE ||
            result->value_dims != 0 || !result->symbolNode) {
            return INLINE_REFERENCE_ACCESSOR_NONE;
        }
        attribute = result->symbolNode->symbol;
        if (!attribute || attribute->type != TP_REFERENCE || attribute->value_dims != 0 ||
            !inline_symbol_is_class_attribute(attribute)) {
            return INLINE_REFERENCE_ACCESSOR_NONE;
        }
        return INLINE_REFERENCE_ACCESSOR_GETTER;
    }

    if (callable->value_type == TP_VOID && callable->value_dims == 0 &&
        args->child && !args->child->sibling) {
        ASTNode *arg;
        ASTNode *formal_target;
        ASTNode *assignment;
        ASTNode *final_return;
        ASTNode *lhs;
        ASTNode *rhs;
        Symbol *formal_symbol;
        Symbol *attribute;

        arg = args->child;
        formal_target = inline_formal_target(arg);
        if (arg->is_ref_arg || arg->is_opt_arg || arg->is_varg ||
            !formal_target || formal_target->value_type != TP_REFERENCE ||
            formal_target->value_dims != 0 || !formal_target->symbolNode) {
            return INLINE_REFERENCE_ACCESSOR_NONE;
        }

        assignment = first;
        final_return = assignment->sibling;
        if (assignment->node_type != ASSIGN || !final_return || final_return->sibling ||
            final_return->node_type != RETURN || final_return->child) {
            return INLINE_REFERENCE_ACCESSOR_NONE;
        }

        lhs = assignment->child;
        rhs = lhs ? lhs->sibling : NULL;
        if (!lhs || !rhs || rhs->sibling || lhs->child || rhs->child ||
            lhs->node_type != VAR_TARGET || rhs->node_type != VAR_SYMBOL ||
            lhs->value_type != TP_REFERENCE || rhs->value_type != TP_REFERENCE ||
            lhs->value_dims != 0 || rhs->value_dims != 0 ||
            !lhs->symbolNode || !rhs->symbolNode) {
            return INLINE_REFERENCE_ACCESSOR_NONE;
        }

        formal_symbol = formal_target->symbolNode->symbol;
        attribute = lhs->symbolNode->symbol;
        if (!formal_symbol || !attribute || rhs->symbolNode->symbol != formal_symbol ||
            formal_symbol->type != TP_REFERENCE || formal_symbol->value_dims != 0 ||
            attribute->type != TP_REFERENCE || attribute->value_dims != 0 ||
            !inline_symbol_is_class_attribute(attribute)) {
            return INLINE_REFERENCE_ACCESSOR_NONE;
        }
        return INLINE_REFERENCE_ACCESSOR_SETTER;
    }

    return INLINE_REFERENCE_ACCESSOR_NONE;
}

static InlineEligibilityReject inline_analyse_callable_eligibility(Context *context,
                                                                   ASTNode *callable,
                                                                   Symbol *symbol,
                                                                   int require_args,
                                                                   int reject_unportable_class_attribute_shape,
                                                                   InlineEligibility *eligibility) {
    ASTNode *arg;
    ASTNode *varg_arg;

    if (!eligibility) return INLINE_ELIGIBILITY_MISSING_INSTRS;

    memset(eligibility, 0, sizeof(*eligibility));
    eligibility->reject = INLINE_ELIGIBILITY_OK;

    eligibility->args = ast_chld(callable, ARGS, 0);
    eligibility->instrs = ast_chld(callable, INSTRUCTIONS, 0);
    if (require_args && !eligibility->args) {
        eligibility->reject = INLINE_ELIGIBILITY_MISSING_ARGS_OR_INSTRS;
        return eligibility->reject;
    }
    if (!eligibility->instrs) {
        eligibility->reject = require_args ?
                              INLINE_ELIGIBILITY_MISSING_ARGS_OR_INSTRS :
                              INLINE_ELIGIBILITY_MISSING_INSTRS;
        return eligibility->reject;
    }

    if (symbol &&
        symbol->type == TP_OBJECT &&
        inline_class_has_reference_attribute(context, symbol->scope, symbol->value_class)) {
        eligibility->reject = INLINE_ELIGIBILITY_RETURN_REFERENCE_CLASS;
        return eligibility->reject;
    }

    if (eligibility->args) {
        arg = eligibility->args->child;
        while (arg) {
            if (arg->is_varg && arg->sibling) {
                eligibility->reject = INLINE_ELIGIBILITY_VARG_FORMAL_FOLLOWED;
                return eligibility->reject;
            }
            arg = arg->sibling;
        }
    }

    if (!inline_analyse_return_shape(callable, &eligibility->return_shape)) {
        eligibility->reject = INLINE_ELIGIBILITY_RETURN_SHAPE_FAILED;
        return eligibility->reject;
    }
    if (symbol &&
        !eligibility->return_shape.final_is_return &&
        symbol->type != TP_VOID) {
        eligibility->reject = INLINE_ELIGIBILITY_VALUE_NOT_FINAL_RETURN;
        return eligibility->reject;
    }
    if (symbol &&
        symbol->type != TP_VOID &&
        eligibility->return_shape.return_count == 0) {
        eligibility->reject = INLINE_ELIGIBILITY_VALUE_NO_RETURN;
        return eligibility->reject;
    }

    memset(&eligibility->check, 0, sizeof(eligibility->check));
    eligibility->check.root_proc = callable;
    eligibility->check.context = context;
    varg_arg = inline_find_varg_arg(callable);
    eligibility->check.ref_varg_mode = eligibility->args && varg_arg && varg_arg->is_ref_arg;
    ast_wlkr(callable, inlinable_check_walker, &eligibility->check);
    eligibility->reference_accessor_kind = inline_exact_reference_accessor_kind(callable,
                                                                                eligibility->args,
                                                                                eligibility->instrs);

    if (eligibility->check.node_count > INLINE_MAX_NODES) {
        eligibility->reject = INLINE_ELIGIBILITY_NODE_CUTOFF;
    } else if (eligibility->check.return_count != eligibility->return_shape.return_count) {
        eligibility->reject = INLINE_ELIGIBILITY_RETURN_COUNT_MISMATCH;
    } else if (eligibility->check.has_unsupported_assembler_alias) {
        eligibility->reject = INLINE_ELIGIBILITY_ASSEMBLER_ALIAS;
    } else if (eligibility->check.has_unsupported_assembler_effect) {
        eligibility->reject = INLINE_ELIGIBILITY_ASSEMBLER_EFFECT;
    } else if (eligibility->check.has_unsupported_reference &&
               eligibility->reference_accessor_kind == INLINE_REFERENCE_ACCESSOR_NONE) {
        eligibility->reject = INLINE_ELIGIBILITY_UNSUPPORTED_REFERENCE;
    } else if (eligibility->check.has_unsupported_varg_access) {
        eligibility->reject = INLINE_ELIGIBILITY_UNSUPPORTED_VARG_ACCESS;
    /* I6 already transports the complete indexed Boolean shape used by the
     * bounded receiver-guard proof.  Revalidate that exact template on both
     * export and import instead of opening array attributes generally. */
    } else if (reject_unportable_class_attribute_shape &&
               (inline_callable_is_method(callable) || callable->node_type == FACTORY) &&
               eligibility->check.has_unportable_class_attribute_shape &&
               eligibility->reference_accessor_kind == INLINE_REFERENCE_ACCESSOR_NONE &&
               !inline_receiver_guard_template_shape(callable, NULL)) {
        eligibility->reject = INLINE_ELIGIBILITY_UNPORTABLE_CLASS_ATTRIBUTE_SHAPE;
    }

    return eligibility->reject;
}
