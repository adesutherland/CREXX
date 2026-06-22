/*
 * Private inline implementation fragment.
 * Included by rxcp_inline.c; not compiled separately.
 */

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
                                          ASTNode *shape_node) {
    char sink_name[64];
    ASTNode *sink_target;
    Symbol *sink_symbol;

    if (!context || !inline_scope || !source_node || !shape_node) return NULL;

    snprintf(sink_name, sizeof(sink_name), "__inline_unused_%d", source_node->node_number);

    sink_symbol = sym_fn(inline_scope, sink_name, strlen(sink_name));
    if (!sink_symbol) return NULL;

    sink_symbol->symbol_type = VARIABLE_SYMBOL;
    sink_symbol->status = SYM_STATUS_LOCAL_VAR;
    sink_symbol->register_num = UNSET_REGISTER;
    sink_symbol->register_type = 'r';
    sink_symbol->meta_emitted = 0;
    sink_symbol->init_emitted = 0;
    if (!inline_copy_node_shape(sink_symbol, shape_node)) return NULL;

    sink_target = ast_ftt(context, VAR_TARGET, strdup(sink_symbol->name));
    sink_target->free_node_string = 1;
    sink_target->scope = inline_scope;
    ast_copy_source_anchor(sink_target, source_node, AST_SOURCE_SYNTHETIC);
    sym_adnd(sink_symbol, sink_target, 0, 1);
    ast_svtp(sink_target, sink_symbol);

    return sink_target;
}

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

    if (!proc_def || proc_def->node_type != METHOD) return 0;

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
    ast_copy_source_anchor(wrapper, leave_node, AST_SOURCE_SYNTHETIC);
    ast_mark_compiler_generated_block(wrapper);
    wrapper->association = block_expr;
    wrapper->scope = inline_scope;
    wrapper->value_type = TP_VOID;
    wrapper->target_type = TP_VOID;

    leave_expr = leave_node->child;
    if (!leave_expr) return NULL;

    temp_symbol = inline_create_temp_symbol(context,
                                            inline_scope,
                                            leave_expr,
                                            "__inline_leave",
                                            (size_t)leave_node->node_number);
    if (!temp_symbol) return NULL;

    assign_node = ast_f(context, ASSIGN, leave_expr->token ? leave_expr->token : leave_node->token);
    if (!assign_node) return NULL;
    assign_node->scope = inline_scope;
    assign_node->value_type = leave_expr->value_type;
    assign_node->target_type = leave_expr->target_type;

    assign_lhs = inline_create_symbol_node(context,
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

    add_ast(assign_node, assign_lhs);
    add_ast(assign_node, leave_expr);
    add_ast(wrapper, assign_node);

    if (!inline_append_method_receiver_copyback(context,
                                                wrapper,
                                                inline_scope,
                                                leave_node,
                                                clone_state)) {
        return NULL;
    }

    temp_ref = inline_create_symbol_node(context,
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

        leave_expr = inline_create_integer_constant(context, node, 0, TP_INTEGER);
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

static ASTNode *inline_build_block_expr(Context *context,
                                        ASTNode *call_node,
                                        Symbol *proc_sym,
                                        Scope *parent_scope,
                                        int allow_dummy_return) {
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

    block_expr = ast_dup(context, call_node);
    if (!block_expr) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to duplicate call node for BLOCK_EXPR");
        return NULL;
    }

    block_expr->node_type = BLOCK_EXPR;
    ast_str(block_expr, "do");
    block_expr->association = proc_def;

    if (allow_dummy_return && proc_sym->type == TP_VOID) {
        ast_set_value_type(0, block_expr, TP_INTEGER, 0, 0, 0, 0);
        ast_set_target_type(0, block_expr, TP_INTEGER, 0, 0, 0, 0);
    }

    inline_scope = scp_f(context, parent_scope, block_expr, NULL, SCOPE_LOCAL);
    if (!inline_scope) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to create BLOCK_EXPR inline scope");
        return NULL;
    }
    block_expr->scope = inline_scope;

    instr_list = ast_f(context, INSTRUCTIONS, call_node->token);
    if (!instr_list) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to create inline instruction list");
        return NULL;
    }
    instr_list->scope = inline_scope;
    instr_list->value_type = TP_VOID;
    instr_list->target_type = TP_VOID;
    add_ast(block_expr, instr_list);

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

        cloned_instr = inline_clone_body_instruction(context, proc_instr, &clone_state);
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

    if (allow_dummy_return && proc_sym->type == TP_VOID &&
        inline_analyse_return_shape(proc_def, &return_shape) &&
        !return_shape.final_is_return) {
        ASTNode *leave_with;
        ASTNode *leave_expr;

        leave_expr = inline_create_integer_constant(context, call_node, 0, TP_INTEGER);
        if (!leave_expr) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to create dummy LEAVE_WITH expression");
            inline_free_symbol_map(&clone_state);
            return NULL;
        }
        leave_expr->scope = inline_scope;

        leave_with = ast_f(context, LEAVE_WITH, call_node->token);
        if (!leave_with) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to create dummy LEAVE_WITH node");
            inline_free_symbol_map(&clone_state);
            return NULL;
        }
        leave_with->scope = inline_scope;
        leave_with->association = block_expr;
        leave_with->value_type = leave_expr->value_type;
        leave_with->target_type = leave_expr->target_type;

        add_ast(leave_with, leave_expr);
        add_ast(instr_list, leave_with);
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

    if (!context || !statement_node || !call_node || !proc_sym || !proc_sym->ast_template) return 0;

    proc_def = proc_sym->ast_template;
    if (!proc_def || !proc_def->scope) {
        inline_debug_fail_closed(context, call_node, proc_sym, "callee has no inlineable procedure scope");
        return 0;
    }

    if (!inline_validate_call_site(context, proc_def, call_node, proc_sym)) return 0;

    block = ast_f(context, INSTRUCTIONS, call_node->token);
    if (!block) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to create compiler-generated statement block");
        return 0;
    }
    ast_copy_source_anchor(block, statement_node, AST_SOURCE_SYNTHETIC);
    ast_mark_compiler_generated_block(block);
    ast_enable_primary_reporting_anchor(block);
    block->association = proc_def;
    block->value_type = TP_VOID;
    block->target_type = TP_VOID;

    inline_scope = scp_f(context, statement_node->scope, block, NULL, SCOPE_LOCAL);
    if (!inline_scope) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to create compiler-generated statement scope");
        return 0;
    }
    instr_list = block;

    memset(&clone_state, 0, sizeof(clone_state));

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
                ret_lhs = inline_create_sink_target(context, inline_scope, proc_instr, proc_instr->child);
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

            if (inline_node_has_array_shape(ret_expr) ||
                (inline_node_needs_attr_copy(ret_expr) &&
                 (ret_expr->value_type == TP_BINARY || ret_expr->target_type == TP_BINARY))) {
                ASTNode *ret_copy;

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

                ret_copy = inline_create_register_copy_instr(context, inline_scope, "copy", ret_lhs, ret_rhs);
                if (!ret_copy) {
                    inline_debug_fail_closed(context, call_node, proc_sym, "failed to create aggregate return copy instructions");
                    inline_free_symbol_map(&clone_state);
                    return 0;
                }
                add_ast(instr_list, ret_copy);
            } else {
                ret_rhs = inline_clone_subtree(context, ret_expr, &clone_state);
                if (!ret_rhs) {
                    inline_debug_fail_closed(context, call_node, proc_sym, "failed to clone scalar return expression");
                    inline_free_symbol_map(&clone_state);
                    return 0;
                }
                add_ast(ret_assign, ret_lhs);
                add_ast(ret_assign, ret_rhs);
                add_ast(instr_list, ret_assign);
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

    if (!inline_append_method_receiver_copyback(context,
                                                instr_list,
                                                inline_scope,
                                                call_node,
                                                &clone_state)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to append method receiver copyback");
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    ast_rpl(statement_node, block);
    inline_disconnect_subtree_symbols(statement_node);
    inline_free_symbol_map(&clone_state);

    return 1;
}

int ast_inline_assignment(Context *context, ASTNode *assign_node, ASTNode *call_node, Symbol *proc_sym) {
    ASTNode *lhs;
    ASTNode *block_expr;
    ASTNode *proc_def;
    InlineReturnShape return_shape;
    InlineReturnPlan return_plan;
    int method_needs_receiver_copyback;

    if (!assign_node || !call_node) return 0;

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
    if (proc_def->node_type == METHOD &&
        inline_symbol_uses_imported_template(proc_sym) &&
        !inline_is_direct_symbol_actual(inline_call_receiver(call_node))) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "imported method assignment inline requires a direct receiver");
        return 0;
    }
    if (method_needs_receiver_copyback &&
        proc_def->node_type == METHOD &&
        !inline_is_direct_receiver_copyback_target(inline_call_receiver(call_node))) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "mutating method assignment inline requires a direct receiver copyback target");
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
        block_expr = inline_build_block_expr(context, call_node, proc_sym, assign_node->scope, 0);
        if (!block_expr) return 0;
        ast_rpl(call_node, block_expr);
        inline_disconnect_subtree_symbols(call_node);
        return 1;
    }
    if (return_shape.return_count != 1) {
        if (method_needs_receiver_copyback) {
            inline_debug_fail_closed(context, call_node, proc_sym,
                                     "mutating method multi-return assignment inline requires statement-position copyback");
            return 0;
        }
        block_expr = inline_build_block_expr(context, call_node, proc_sym, assign_node->scope, 0);
        if (!block_expr) return 0;
        ast_rpl(call_node, block_expr);
        inline_disconnect_subtree_symbols(call_node);
        return 1;
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
    int method_needs_receiver_copyback;

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
    if (proc_def->node_type == METHOD &&
        inline_symbol_uses_imported_template(proc_sym) &&
        !inline_is_direct_symbol_actual(inline_call_receiver(call_node))) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "imported method call inline requires a direct receiver");
        return 0;
    }
    if (method_needs_receiver_copyback &&
        proc_def->node_type == METHOD &&
        !inline_is_direct_receiver_copyback_target(inline_call_receiver(call_node))) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "mutating method call inline requires a direct receiver copyback target");
        return 0;
    }
    if ((proc_sym->type == TP_VOID && (return_shape.return_count != 1 || !return_shape.final_is_return)) ||
        (proc_sym->type != TP_VOID && return_shape.return_count != 1)) {
        if (method_needs_receiver_copyback) {
            inline_debug_fail_closed(context, call_node, proc_sym,
                                     "mutating method call inline requires statement-position copyback");
            return 0;
        }
        block = ast_f(context, INSTRUCTIONS, call_node->token);
        if (!block) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to create compiler-generated sink block");
            return 0;
        }
        ast_copy_source_anchor(block, call_stmt, AST_SOURCE_SYNTHETIC);
        ast_mark_compiler_generated_block(block);
        ast_enable_primary_reporting_anchor(block);
        block->association = proc_def;
        block->value_type = TP_VOID;
        block->target_type = TP_VOID;

        block_scope = scp_f(context, call_stmt->scope, block, NULL, SCOPE_LOCAL);
        if (!block_scope) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to create sink block scope");
            return 0;
        }

        block_expr = inline_build_block_expr(context, call_node, proc_sym, block_scope, 1);
        if (!block_expr) return 0;

        sink_assign = ast_f(context, ASSIGN, call_node->token);
        if (!sink_assign) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to create sink assignment");
            return 0;
        }
        sink_assign->scope = block_scope;
        sink_assign->value_type = block_expr->value_type;
        sink_assign->target_type = block_expr->target_type;

        sink_lhs = inline_create_sink_target(context, block_scope, call_node, block_expr);
        if (!sink_lhs) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to create unused return sink target");
            return 0;
        }

        add_ast(sink_assign, sink_lhs);
        add_ast(sink_assign, block_expr);
        add_ast(block, sink_assign);

        ast_rpl(call_stmt, block);
        inline_disconnect_subtree_symbols(call_stmt);
        return 1;
    }

    memset(&return_plan, 0, sizeof(return_plan));
    return_plan.return_sink_symbol = proc_sym;

    return ast_inline_statement(context, call_stmt, call_node, proc_sym, &return_plan);
}

int ast_inline_expression(Context *context, ASTNode *call_node, Symbol *proc_sym) {
    ASTNode *block_expr;
    InlineExprContext expr_context;
    InlineReturnShape return_shape;

    if (!context || !call_node || !proc_sym || !proc_sym->ast_template) return 0;

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
        proc_sym->ast_template->node_type == METHOD &&
        !inline_is_direct_receiver_copyback_target(inline_call_receiver(call_node))) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "mutating method expression inline requires a direct receiver copyback target");
        return 0;
    }
    if (proc_sym->ast_template->node_type == METHOD &&
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

    block_expr = inline_build_block_expr(context, call_node, proc_sym, call_node->scope, 0);
    if (!block_expr) return 0;

    ast_rpl(call_node, block_expr);
    inline_disconnect_subtree_symbols(call_node);

    return 1;
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

    block_expr = ast_dup(context, op_node);
    if (!block_expr) {
        inline_debug_fail_closed(context, rhs_call, proc_sym,
                                 "failed to create RHS eager-operator BLOCK_EXPR");
        return 0;
    }
    block_expr->node_type = BLOCK_EXPR;
    ast_str(block_expr, "do");

    inline_scope = scp_f(context, parent_scope, block_expr, NULL, SCOPE_LOCAL);
    if (!inline_scope) {
        inline_debug_fail_closed(context, rhs_call, proc_sym,
                                 "failed to create RHS eager-operator inline scope");
        return 0;
    }
    block_expr->scope = inline_scope;

    instr_list = ast_f(context, INSTRUCTIONS, op_node->token);
    if (!instr_list) {
        inline_debug_fail_closed(context, rhs_call, proc_sym,
                                 "failed to create RHS eager-operator instruction list");
        return 0;
    }
    instr_list->scope = inline_scope;
    instr_list->value_type = TP_VOID;
    instr_list->target_type = TP_VOID;
    add_ast(block_expr, instr_list);

    left_symbol = inline_create_temp_symbol(context,
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

    assign_node = ast_f(context, ASSIGN, left->token ? left->token : op_node->token);
    if (!assign_node) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }
    assign_node->scope = inline_scope;
    assign_node->value_type = assign_rhs->value_type;
    assign_node->target_type = assign_rhs->target_type;

    assign_lhs = inline_create_symbol_node(context,
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

    add_ast(assign_node, assign_lhs);
    add_ast(assign_node, assign_rhs);
    add_ast(instr_list, assign_node);

    op_expr = ast_dup(context, op_node);
    if (!op_expr) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }
    op_expr->scope = inline_scope;

    temp_ref = inline_create_symbol_node(context,
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

    leave_node = ast_f(context, LEAVE_WITH, op_node->token);
    if (!leave_node) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }
    leave_node->scope = inline_scope;
    leave_node->association = block_expr;
    leave_node->value_type = op_expr->value_type;
    leave_node->target_type = op_expr->target_type;

    add_ast(leave_node, op_expr);
    add_ast(instr_list, leave_node);

    ast_rpl(op_node, block_expr);
    inline_disconnect_subtree_symbols(op_node);
    inline_free_symbol_map(&clone_state);

    return 1;
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

    if (eligibility->check.node_count > INLINE_MAX_NODES) {
        eligibility->reject = INLINE_ELIGIBILITY_NODE_CUTOFF;
    } else if (eligibility->check.return_count != eligibility->return_shape.return_count) {
        eligibility->reject = INLINE_ELIGIBILITY_RETURN_COUNT_MISMATCH;
    } else if (eligibility->check.has_unsupported_assembler_alias) {
        eligibility->reject = INLINE_ELIGIBILITY_ASSEMBLER_ALIAS;
    } else if (eligibility->check.has_unsupported_assembler_effect) {
        eligibility->reject = INLINE_ELIGIBILITY_ASSEMBLER_EFFECT;
    } else if (eligibility->check.has_unsupported_reference) {
        eligibility->reject = INLINE_ELIGIBILITY_UNSUPPORTED_REFERENCE;
    } else if (eligibility->check.has_unsupported_varg_access) {
        eligibility->reject = INLINE_ELIGIBILITY_UNSUPPORTED_VARG_ACCESS;
    } else if (reject_unportable_class_attribute_shape &&
               (callable->node_type == METHOD || callable->node_type == FACTORY) &&
               eligibility->check.has_unportable_class_attribute_shape) {
        eligibility->reject = INLINE_ELIGIBILITY_UNPORTABLE_CLASS_ATTRIBUTE_SHAPE;
    }

    return eligibility->reject;
}

