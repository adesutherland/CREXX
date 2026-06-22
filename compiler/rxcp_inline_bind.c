/*
 * Private inline implementation fragment.
 * Included by rxcp_inline.c; not compiled separately.
 */

static Symbol *inline_find_mapped_symbol(InlineCloneState *state, Symbol *old_symbol) {
    size_t i;

    if (!state || !old_symbol) return NULL;

    for (i = 0; i < state->symbol_count; i++) {
        if (state->symbol_entries[i].old_symbol == old_symbol) return state->symbol_entries[i].new_symbol;
    }

    return NULL;
}

static Scope *inline_find_mapped_scope(InlineCloneState *state, Scope *old_scope) {
    size_t i;

    if (!state || !old_scope) return NULL;

    for (i = 0; i < state->scope_count; i++) {
        if (state->scope_entries[i].old_scope == old_scope) return state->scope_entries[i].new_scope;
    }

    return NULL;
}

static ASTNode *inline_find_mapped_node(InlineCloneState *state, ASTNode *old_node) {
    size_t i;

    if (!state || !old_node) return NULL;

    for (i = 0; i < state->node_count; i++) {
        if (state->node_entries[i].old_node == old_node) return state->node_entries[i].new_node;
    }

    return NULL;
}

static int inline_append_symbol_map_entry(InlineCloneState *state, Symbol *old_symbol, Symbol *new_symbol) {
    InlineSymbolMapEntry *new_entries;

    if (!state || !old_symbol || !new_symbol) return 0;

    new_entries = realloc(state->symbol_entries,
                          sizeof(InlineSymbolMapEntry) * (state->symbol_count + 1));
    if (!new_entries) return 0;

    state->symbol_entries = new_entries;
    state->symbol_entries[state->symbol_count].old_symbol = old_symbol;
    state->symbol_entries[state->symbol_count].new_symbol = new_symbol;
    state->symbol_count++;
    return 1;
}

static ASTNode *inline_formal_target(ASTNode *param_arg) {
    return param_arg ? ast_chdn(param_arg, 0) : NULL;
}

static int inline_node_is_callable_def(ASTNode *node) {
    return node &&
           (node->node_type == PROCEDURE ||
            node->node_type == METHOD ||
            node->node_type == FACTORY);
}

static int inline_symbol_has_callable_template(Symbol *symbol) {
    return symbol &&
           symbol->ast_template &&
           inline_node_is_callable_def(symbol->ast_template);
}

static int inline_symbol_uses_imported_template(Symbol *symbol) {
    ASTNode *def_node;

    if (!symbol || !symbol->ast_template) return 0;
    def_node = sym_proc(symbol);
    return def_node && symbol->ast_template != def_node;
}

int inline_node_is_inlineable_call(ASTNode *node, Symbol **proc_sym_out) {
    Symbol *proc_sym;

    if (proc_sym_out) *proc_sym_out = NULL;
    if (!node ||
        (node->node_type != FUNCTION &&
         node->node_type != MEMBER_CALL &&
         node->node_type != FACTORY_CALL)) {
        return 0;
    }

    proc_sym = node->symbolNode ? node->symbolNode->symbol : NULL;
    if (!proc_sym || !proc_sym->is_inlinable || !inline_symbol_has_callable_template(proc_sym)) return 0;

    if (proc_sym_out) *proc_sym_out = proc_sym;
    return 1;
}

static ASTNode *inline_call_receiver(ASTNode *call_node) {
    if (!call_node || call_node->node_type != MEMBER_CALL) return NULL;
    return call_node->child;
}

static ASTNode *inline_call_first_user_actual(ASTNode *call_node) {
    if (!call_node) return NULL;
    if (call_node->node_type == MEMBER_CALL) {
        return call_node->child ? call_node->child->sibling : NULL;
    }
    return call_node->child;
}

static ASTNode *inline_formal_default(ASTNode *param_arg) {
    ASTNode *formal_target;

    formal_target = inline_formal_target(param_arg);
    return formal_target ? formal_target->sibling : NULL;
}

static int inline_is_missing_actual(ASTNode *actual_arg) {
    return actual_arg && actual_arg->node_type == NOVAL;
}

static int inline_is_supported_ref_actual(ASTNode *actual_arg) {
    if (!actual_arg) return 0;
    if (!actual_arg->symbolNode || !actual_arg->symbolNode->symbol) return 0;
    if (actual_arg->node_type != VAR_SYMBOL &&
        actual_arg->node_type != VAR_TARGET &&
        actual_arg->node_type != VAR_REFERENCE) return 0;

    return 1;
}

static InlineRefActualEntry *inline_find_ref_actual(InlineCloneState *state, Symbol *formal_symbol) {
    size_t i;

    if (!state || !formal_symbol) return NULL;

    for (i = 0; i < state->ref_count; i++) {
        if (state->ref_entries[i].formal_symbol == formal_symbol) return &state->ref_entries[i];
    }

    return NULL;
}

static InlineRefActualEntry *inline_find_ref_varg_actual(InlineCloneState *state, size_t index) {
    if (!state || !state->varg_ref_entries) return NULL;
    if (index < 1 || index > state->varg_count) return NULL;

    return &state->varg_ref_entries[index - 1];
}

static void inline_copy_replacement_semantics(ASTNode *replacement, ASTNode *replaced_node) {
    if (!replacement || !replaced_node) return;

    replacement->is_ref_arg = replaced_node->is_ref_arg;
    replacement->is_opt_arg = replaced_node->is_opt_arg;
    replacement->is_const_arg = replaced_node->is_const_arg;
    replacement->is_varg = replaced_node->is_varg;
    replacement->inherit_parent_reg_scope = replaced_node->inherit_parent_reg_scope;
    if (replacement->is_ref_arg && replacement->symbolNode) replacement->symbolNode->writeUsage = 1;

    ast_set_value_type(0,
                       replacement,
                       replaced_node->value_type,
                       replaced_node->value_dims,
                       replaced_node->value_dim_base,
                       replaced_node->value_dim_elements,
                       replaced_node->value_class);
    ast_set_target_type(0,
                        replacement,
                        replaced_node->target_type,
                        replaced_node->target_dims,
                        replaced_node->target_dim_base,
                        replaced_node->target_dim_elements,
                        replaced_node->target_class);
}

static int inline_node_has_array_shape(ASTNode *node) {
    if (!node) return 0;
    return node->value_dims > 0 || node->target_dims > 0;
}

static int inline_node_requires_local_scope(ASTNode *node) {
    if (!node) return 0;

    if (node->node_type == BLOCK_EXPR || node->node_type == DO) return 1;
    if (node->node_type != INSTRUCTIONS) return 0;
    if (node->force_local_scope) return 1;

    return node->parent &&
           (node->parent->node_type == IF || node->parent->node_type == INSTRUCTIONS);
}

static int inline_node_is_plain_object(ASTNode *node) {
    if (!node) return 0;
    return node->value_type == TP_OBJECT &&
           node->target_type == TP_OBJECT &&
           !inline_node_has_array_shape(node);
}

static int inline_is_direct_symbol_actual(ASTNode *node) {
    if (!node || !node->symbolNode || !node->symbolNode->symbol || node->child) return 0;

    return node->node_type == VAR_SYMBOL ||
           node->node_type == VAR_TARGET ||
           node->node_type == VAR_REFERENCE;
}

static int inline_symbol_is_class_attribute(Symbol *symbol) {
    return symbol &&
           symbol->symbol_type == VARIABLE_SYMBOL &&
           symbol->scope &&
           (symbol->scope->type == SCOPE_CLASS ||
            (symbol->scope->defining_node &&
             symbol->scope->defining_node->node_type == CLASS_DEF));
}

static Symbol *inline_resolve_class_symbol(Context *context, Scope *scope, const char *class_name) {
    Symbol *class_symbol;
    const char *lookup_name;
    Scope *namespace_scope;
    char *fq_name;

    if (!context || !context->ast || !class_name || !*class_name) return NULL;

    lookup_name = class_name;
    while (*lookup_name == '.') lookup_name++;

    class_symbol = sym_rfqn(context->ast, lookup_name);
    if (class_symbol) return class_symbol;

    if (strchr(lookup_name, '.')) return NULL;

    namespace_scope = scope;
    while (namespace_scope && namespace_scope->type != SCOPE_NAMESPACE) {
        namespace_scope = namespace_scope->parent;
    }
    if (!namespace_scope || !namespace_scope->name || !*namespace_scope->name) return NULL;

    fq_name = mprintf("%s.%s", namespace_scope->name, lookup_name);
    if (!fq_name) return NULL;
    class_symbol = sym_rfqn(context->ast, fq_name);
    free(fq_name);
    return class_symbol;
}

static int inline_class_has_reference_attribute(Context *context, Scope *scope, const char *class_name) {
    Symbol *class_symbol;
    Symbol **symbols;
    size_t i;
    int result = 0;

    class_symbol = inline_resolve_class_symbol(context, scope, class_name);
    if (!class_symbol || !class_symbol->defines_scope) return 0;

    symbols = scp_syms(class_symbol->defines_scope);
    if (!symbols) return 0;

    for (i = 0; symbols[i]; i++) {
        if (inline_symbol_is_class_attribute(symbols[i]) &&
            symbols[i]->type == TP_REFERENCE) {
            result = 1;
            break;
        }
    }

    free(symbols);
    return result;
}

static ASTNode *inline_class_attribute_register_view(Symbol *symbol) {
    int i;

    if (!symbol) return NULL;
    for (i = 0; i < (int)sym_nond(symbol); i++) {
        ASTNode *node = sym_trnd(symbol, i)->node;
        ASTNode *reg_node;
        ASTNode *child;

        if (!node || !node->parent || node->parent->node_type != DEFINE) continue;
        reg_node = ast_chld(node->parent, NODE_REGISTER, 0);
        if (!reg_node) continue;

        for (child = reg_node->child; child; child = child->sibling) {
            if (child->node_type == INTEGER || child->node_type == CONSTANT) continue;
            return child;
        }
    }

    return NULL;
}

static int inline_class_attribute_is_flag_view(Symbol *symbol) {
    ASTNode *view = inline_class_attribute_register_view(symbol);

    return view &&
           view->node_string &&
           view->node_string_length > 6 &&
           strncasecmp(view->node_string, "flags.", 6) == 0;
}

static int inline_class_attribute_shape_is_portable(Symbol *symbol) {
    if (!inline_symbol_is_class_attribute(symbol)) return 1;
    if (symbol->is_this || symbol->is_factory) return 1;
    if (symbol->value_dims > 0) return 0;
    if (inline_class_attribute_is_flag_view(symbol)) return 0;

    switch (symbol->type) {
        case TP_INTEGER:
        case TP_BOOLEAN:
        case TP_FLOAT:
        case TP_STRING:
            return 1;
        default:
            return 0;
    }
}

static int inline_class_attribute_register_num(Symbol *symbol) {
    int i;

    if (!inline_symbol_is_class_attribute(symbol)) return symbol ? symbol->register_num : UNSET_REGISTER;

    for (i = 0; i < (int)sym_nond(symbol); i++) {
        ASTNode *node = sym_trnd(symbol, i)->node;
        ASTNode *reg_node;
        ASTNode *idx;

        if (!node || !node->parent || node->parent->node_type != DEFINE) continue;
        reg_node = ast_chld(node->parent, NODE_REGISTER, 0);
        if (!reg_node) continue;

        idx = ast_chld(reg_node, INTEGER, 0);
        if (idx) return node_to_integer(idx);
        if (reg_node->int_value) return (int)reg_node->int_value;
        if (reg_node->child && reg_node->child->token) {
            return (int)strtol(reg_node->child->token->token_string, NULL, 10);
        }
        if (reg_node->child && reg_node->child->node_string && reg_node->child->node_string_length) {
            char *buffer = malloc(reg_node->child->node_string_length + 1);
            int result;

            if (!buffer) return UNSET_REGISTER;
            memcpy(buffer, reg_node->child->node_string, reg_node->child->node_string_length);
            buffer[reg_node->child->node_string_length] = 0;
            result = (int)strtol(buffer, NULL, 10);
            free(buffer);
            return result;
        }
    }

    return symbol->register_num;
}

static int inline_is_direct_receiver_copyback_target(ASTNode *node) {
    if (!inline_is_direct_symbol_actual(node)) return 0;
    return !inline_symbol_is_class_attribute(node->symbolNode->symbol);
}

static int inline_is_locator_receiver_copyback_target(ASTNode *node) {
    if (!inline_is_supported_ref_actual(node)) return 0;
    if (!node->child) return 0;
    return !inline_symbol_is_class_attribute(node->symbolNode->symbol);
}

static int inline_is_supported_receiver_copyback_target(ASTNode *node) {
    return inline_is_direct_receiver_copyback_target(node) ||
           inline_is_locator_receiver_copyback_target(node);
}

static int inline_is_direct_single_value_consumer(ASTNode *node) {
    ASTNode *parent;

    if (!node) return 0;

    parent = node->parent;
    if (!parent || parent->child != node) return 0;

    switch (parent->node_type) {
        case SAY:
        case RETURN:
        case IF:
        case WHILE:
        case UNTIL:
        case FOR:
        case TO:
        case BY:
            return 1;

        default:
            return 0;
    }
}

static int inline_parent_is_eager_operator(ASTNode *parent) {
    if (!parent) return 0;

    switch (parent->node_type) {
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
        case OP_NOT:
        case OP_BIT_AND:
        case OP_BIT_OR:
        case OP_BIT_XOR:
        case OP_BIT_NOT:
        case OP_BIT_SHL:
        case OP_BIT_SHR:
        case OP_FLAG_HAS:
            return 1;

        default:
            return 0;
    }
}

static int inline_node_is_constant_literal(ASTNode *node) {
    if (!node) return 0;

    switch (node->node_type) {
        case CONSTANT:
        case CONST_SYMBOL:
        case STRING:
        case FLOAT:
        case DECIMAL:
        case BINARY:
        case INTEGER:
        case CLASS:
            return node->value_type == node->target_type;

        default:
            return 0;
    }
}

static int inline_node_is_plain_stable_value(ASTNode *node) {
    Symbol *symbol;

    if (!node || node->node_type != VAR_SYMBOL || node->child) return 0;
    if (!node->symbolNode || !node->symbolNode->symbol) return 0;

    symbol = node->symbolNode->symbol;
    if (symbol->symbol_type == FUNCTION_SYMBOL) return 0;
    return !inline_symbol_is_class_attribute(symbol);
}

static int inline_eager_operator_context_is_safe(ASTNode *node) {
    ASTNode *parent;

    if (!node) return 0;

    parent = node->parent;
    if (!inline_parent_is_eager_operator(parent)) return 0;

    if (parent->child == node) return 1;

    return inline_node_is_constant_literal(parent->child) ||
           inline_node_is_plain_stable_value(parent->child);
}

int inline_rhs_eager_operator_needs_left_capture(ASTNode *node) {
    ASTNode *parent;
    ASTNode *left;

    if (!node) return 0;

    parent = node->parent;
    if (!inline_parent_is_eager_operator(parent)) return 0;

    left = parent->child;
    if (!left || left == node) return 0;
    if (left->sibling != node || node->sibling) return 0;

    return !inline_eager_operator_context_is_safe(node);
}

static int inline_parent_is_short_circuit_operator(ASTNode *parent) {
    return parent &&
           (parent->node_type == OP_AND ||
            parent->node_type == OP_OR);
}

static int inline_node_is_call_argument(ASTNode *node) {
    ASTNode *parent;

    if (!node || !node->parent) return 0;

    parent = node->parent;
    switch (parent->node_type) {
        case FUNCTION:
        case FACTORY_CALL:
            return 1;

        case MEMBER_CALL:
            return parent->child != node;

        default:
            return 0;
    }
}

static int inline_node_needs_attr_copy(ASTNode *node) {
    if (!node) return 0;

    if (inline_node_has_array_shape(node)) return 1;

    return node->value_type == TP_OBJECT ||
           node->value_type == TP_BINARY ||
           node->value_type == TP_REFERENCE ||
           node->target_type == TP_OBJECT ||
           node->target_type == TP_BINARY ||
           node->target_type == TP_REFERENCE;
}

static int inline_formal_needs_isolated_copy(ASTNode *formal_target, ASTNode *param_arg) {
    if (!formal_target) return 0;
    if (inline_node_has_array_shape(formal_target)) return 1;
    if (formal_target->value_type == TP_BINARY || formal_target->target_type == TP_BINARY ||
        formal_target->value_type == TP_REFERENCE || formal_target->target_type == TP_REFERENCE) return 1;

    return inline_node_is_plain_object(formal_target) && !(param_arg && param_arg->is_const_arg);
}

static ASTNode *inline_create_register_copy_instr(Context *context,
                                                  Scope *scope,
                                                  const char *opcode,
                                                  ASTNode *lhs_node,
                                                  ASTNode *rhs_node) {
    ASTNode *instr;
    ASTNode *lhs_copy;
    ASTNode *rhs_copy;
    Symbol *lhs_symbol;
    Symbol *rhs_symbol;

    if (!context || !scope || !lhs_node || !rhs_node) return NULL;
    if (!lhs_node->symbolNode || !rhs_node->symbolNode) return NULL;

    lhs_symbol = lhs_node->symbolNode->symbol;
    rhs_symbol = rhs_node->symbolNode->symbol;
    if (!lhs_symbol || !rhs_symbol) return NULL;

    if (!opcode) return NULL;

    instr = ast_ftt(context, ASSEMBLER, strdup(opcode));
    if (!instr) return NULL;

    instr->free_node_string = 1;
    instr->scope = scope;
    rxcp_remap_anchor_synthetic(instr, lhs_node);

    lhs_copy = rxcp_remap_create_symbol_node(context, scope, lhs_node, lhs_symbol, VAR_TARGET, 0, 1);
    rhs_copy = rxcp_remap_create_symbol_node(context, scope, rhs_node, rhs_symbol, VAR_SYMBOL, 1, 0);
    if (!lhs_copy || !rhs_copy) return NULL;

    add_ast(instr, lhs_copy);
    add_ast(instr, rhs_copy);
    return instr;
}

static ASTNode *inline_materialize_capture_clone(Context *context,
                                                 ASTNode *source_node,
                                                 Scope *scope,
                                                 void *user_data) {
    InlineCloneState *state;

    (void)scope;
    state = (InlineCloneState *)user_data;
    return inline_clone_subtree(context, source_node, state);
}

static ASTNode *inline_create_temp_value_ref(Context *context,
                                             ASTNode *instr_list,
                                             Scope *inline_scope,
                                             ASTNode *source_node,
                                             InlineCloneState *state,
                                             const char *prefix,
                                             size_t suffix) {
    if (!context || !instr_list || !inline_scope || !source_node || !state || !prefix) return NULL;

    return rxcp_remap_capture_once(context,
                                   instr_list,
                                   inline_scope,
                                   source_node,
                                   prefix,
                                   suffix,
                                   inline_materialize_capture_clone,
                                   state,
                                   NULL,
                                   NULL);
}

static int inline_should_capture_scoped_actual(ASTNode *param_arg, ASTNode *actual_arg) {
    ASTNode *formal_target;

    if (!param_arg || !actual_arg) return 0;
    if (inline_is_missing_actual(actual_arg)) return 0;
    if (param_arg->is_ref_arg) return 0;
    if (!inline_subtree_reads_class_attribute(actual_arg)) return 0;

    formal_target = inline_formal_target(param_arg);
    if (inline_node_is_plain_object(formal_target) && param_arg->is_const_arg) return 0;

    return 1;
}

static int inline_subtree_reads_class_attribute(ASTNode *node) {
    ASTNode *child;

    if (!node) return 0;
    if (inline_node_is_callable_def(node)) return 0;

    if (node->node_type == VAR_SYMBOL &&
        node->symbolNode &&
        inline_symbol_is_class_attribute(node->symbolNode->symbol)) {
        return 1;
    }

    child = node->child;
    while (child) {
        if (inline_subtree_reads_class_attribute(child)) return 1;
        child = child->sibling;
    }

    return 0;
}

static int inline_sibling_list_reads_class_attribute(ASTNode *node) {
    while (node) {
        if (inline_subtree_reads_class_attribute(node)) return 1;
        node = node->sibling;
    }

    return 0;
}

static Scope *inline_find_callsite_instance_scope(ASTNode *call_node) {
    ASTNode *node;

    node = call_node;
    while (node) {
        ASTNode *association;

        association = node->association;
        if (association &&
            (association->node_type == METHOD || association->node_type == FACTORY) &&
            node->scope) {
            ASTNode lookup_node;
            const char *name;

            name = association->node_type == FACTORY ? "\xc2\xa7" "factory" : "\xc2\xa7" "this";
            memset(&lookup_node, 0, sizeof(lookup_node));
            lookup_node.node_string = (char *)name;
            lookup_node.node_string_length = strlen(name);

            if (sym_lrsv(node->scope, &lookup_node)) return node->scope;
        }

        node = node->parent;
    }

    return call_node ? call_node->scope : NULL;
}

static ASTNode *inline_scope_callable_association(Scope *scope) {
    while (scope) {
        ASTNode *defining_node;

        defining_node = scope->defining_node;
        if (inline_node_is_callable_def(defining_node)) return defining_node;
        if (defining_node && inline_node_is_callable_def(defining_node->association)) {
            return defining_node->association;
        }

        scope = scope->parent;
    }

    return NULL;
}

static int inline_scoped_call_needs_actual_capture(ASTNode *proc_def, ASTNode *call_node) {
    ASTNode *param_list;
    ASTNode *param_arg;
    ASTNode *actual_arg;
    ASTNode *varg_arg;

    if (!proc_def || !call_node) return 0;
    if (proc_def->node_type != FACTORY && proc_def->node_type != METHOD) return 0;

    param_list = ast_chld(proc_def, ARGS, 0);
    param_arg = param_list ? param_list->child : NULL;
    actual_arg = inline_call_first_user_actual(call_node);
    varg_arg = inline_find_varg_arg(proc_def);

    while (param_arg && actual_arg) {
        if (param_arg == varg_arg) {
            if (param_arg->is_ref_arg) return 0;
            return inline_sibling_list_reads_class_attribute(actual_arg);
        }

        if (inline_should_capture_scoped_actual(param_arg, actual_arg)) return 1;

        param_arg = param_arg->sibling;
        actual_arg = actual_arg->sibling;
    }

    return 0;
}

static Symbol *inline_capture_method_receiver_for_scoped_args(Context *context,
                                                              ASTNode *instr_list,
                                                              Scope *caller_scope,
                                                              ASTNode *proc_def,
                                                              ASTNode *call_node,
                                                              InlineCloneState *clone_state) {
    ASTNode *receiver;
    Symbol *temp_symbol;
    ASTNode *capture_assign;
    ASTNode *capture_lhs;
    ASTNode *capture_rhs;

    if (!context || !instr_list || !caller_scope || !proc_def || !call_node || !clone_state) return NULL;
    if (proc_def->node_type != METHOD) return NULL;

    receiver = inline_call_receiver(call_node);
    if (!receiver) return NULL;

    temp_symbol = rxcp_remap_create_temp_symbol(context,
                                            caller_scope,
                                            receiver,
                                            "__inline_method_receiver",
                                            0);
    if (!temp_symbol) return NULL;

    capture_assign = rxcp_remap_create_assignment_node(context, caller_scope, receiver, receiver);
    if (!capture_assign) return NULL;
    capture_assign->association = inline_scope_callable_association(caller_scope);
    capture_assign->inherit_parent_scope = 1;

    capture_lhs = rxcp_remap_create_symbol_node(context,
                                            caller_scope,
                                            receiver,
                                            temp_symbol,
                                            VAR_TARGET,
                                            0,
                                            1);
    if (clone_state->method_receiver_uses_locator_copyback) {
        capture_rhs = inline_clone_captured_locator(context,
                                                    receiver,
                                                    caller_scope,
                                                    &clone_state->method_receiver_copyback_entry,
                                                    VAR_SYMBOL,
                                                    1,
                                                    0);
    } else {
        capture_rhs = inline_clone_subtree_in_scope(context, receiver, clone_state, caller_scope);
    }
    if (!capture_lhs || !capture_rhs) return NULL;

    add_ast(capture_assign, capture_lhs);
    add_ast(capture_assign, capture_rhs);
    add_ast(instr_list, capture_assign);

    return temp_symbol;
}

static int inline_capture_scoped_call_actuals(Context *context,
                                              ASTNode *instr_list,
                                              Scope *inline_scope,
                                              ASTNode *proc_def,
                                              ASTNode *call_node,
                                              InlineCloneState *clone_state,
                                              Symbol **captured_receiver_out,
                                              Symbol ***captured_symbols_out,
                                              size_t *captured_count_out) {
    Scope *caller_scope;
    ASTNode *param_list;
    ASTNode *param_arg;
    ASTNode *actual_arg;
    ASTNode *varg_arg;
    Symbol **captured_symbols;
    size_t actual_count;
    size_t actual_index;
    int capture_varg_actuals;

    if (captured_receiver_out) *captured_receiver_out = NULL;
    if (captured_symbols_out) *captured_symbols_out = NULL;
    if (captured_count_out) *captured_count_out = 0;

    if (!context || !instr_list || !inline_scope || !proc_def || !call_node || !clone_state ||
        !captured_receiver_out || !captured_symbols_out || !captured_count_out) {
        return 0;
    }
    if (proc_def->node_type != FACTORY && proc_def->node_type != METHOD) return 1;
    if (!inline_scoped_call_needs_actual_capture(proc_def, call_node)) return 1;

    actual_arg = inline_call_first_user_actual(call_node);
    actual_count = inline_count_siblings(actual_arg);
    if (actual_count == 0) return 1;

    caller_scope = inline_find_callsite_instance_scope(call_node);
    if (!caller_scope) caller_scope = call_node->scope ? call_node->scope : inline_scope->parent;
    if (!caller_scope) return 0;

    if (proc_def->node_type == METHOD) {
        *captured_receiver_out = inline_capture_method_receiver_for_scoped_args(context,
                                                                               instr_list,
                                                                               caller_scope,
                                                                               proc_def,
                                                                               call_node,
                                                                               clone_state);
        if (!*captured_receiver_out) return 0;
    }

    captured_symbols = calloc(actual_count, sizeof(Symbol *));
    if (!captured_symbols) return 0;

    param_list = ast_chld(proc_def, ARGS, 0);
    param_arg = param_list ? param_list->child : NULL;
    varg_arg = inline_find_varg_arg(proc_def);
    actual_index = 0;
    capture_varg_actuals = 0;

    while (param_arg && actual_arg) {
        ASTNode *capture_assign;
        ASTNode *capture_lhs;
        ASTNode *capture_rhs;
        Symbol *temp_symbol;

        if (param_arg == varg_arg && param_arg->is_ref_arg) break;

        if (param_arg != varg_arg && !inline_should_capture_scoped_actual(param_arg, actual_arg)) {
            param_arg = param_arg->sibling;
            actual_arg = actual_arg->sibling;
            actual_index++;
            continue;
        }
        if (param_arg == varg_arg && !capture_varg_actuals) {
            capture_varg_actuals = inline_sibling_list_reads_class_attribute(actual_arg);
        }
        if (param_arg == varg_arg &&
            (inline_is_missing_actual(actual_arg) || !capture_varg_actuals)) {
            actual_arg = actual_arg->sibling;
            actual_index++;
            continue;
        }

        temp_symbol = rxcp_remap_create_temp_symbol(context,
                                                caller_scope,
                                                actual_arg,
                                                "__inline_scoped_arg",
                                                actual_index);
        if (!temp_symbol) {
            free(captured_symbols);
            return 0;
        }

        capture_assign = rxcp_remap_create_assignment_node(context, caller_scope, actual_arg, actual_arg);
        if (!capture_assign) {
            free(captured_symbols);
            return 0;
        }
        capture_assign->association = inline_scope_callable_association(caller_scope);
        capture_assign->inherit_parent_scope = 1;

        capture_lhs = rxcp_remap_create_symbol_node(context,
                                                caller_scope,
                                                actual_arg,
                                                temp_symbol,
                                                VAR_TARGET,
                                                0,
                                                1);
        capture_rhs = inline_clone_subtree_in_scope(context, actual_arg, clone_state, caller_scope);
        if (!capture_lhs || !capture_rhs) {
            free(captured_symbols);
            return 0;
        }

        add_ast(capture_assign, capture_lhs);
        add_ast(capture_assign, capture_rhs);
        add_ast(instr_list, capture_assign);

        captured_symbols[actual_index] = temp_symbol;
        if (param_arg != varg_arg) param_arg = param_arg->sibling;
        actual_arg = actual_arg->sibling;
        actual_index++;
    }

    *captured_symbols_out = captured_symbols;
    *captured_count_out = actual_count;
    return 1;
}

static int inline_capture_varg_captured_actuals(Context *context,
                                                ASTNode *instr_list,
                                                Scope *inline_scope,
                                                ASTNode *varg_arg,
                                                ASTNode *actual_arg,
                                                InlineCloneState *state,
                                                Symbol **captured_symbols,
                                                size_t captured_count,
                                                size_t first_actual_index) {
    ASTNode *varg_type;
    ASTNode *source_template;
    size_t child_index;

    if (!context || !instr_list || !inline_scope || !varg_arg || !state) return 0;

    varg_type = inline_formal_default(varg_arg);
    source_template = varg_type ? varg_type : varg_arg;

    if (!actual_arg) {
        state->varg_symbols = NULL;
        state->varg_count = 0;
        return inline_initialise_varg_array(context, instr_list, inline_scope, varg_arg, source_template, state);
    }

    source_template = varg_type ? varg_type : actual_arg;
    state->varg_count = inline_count_siblings(actual_arg);
    state->varg_symbols = calloc(state->varg_count, sizeof(Symbol *));
    if (!state->varg_symbols) return 0;

    child_index = 0;
    while (actual_arg) {
        Symbol *captured_symbol;

        if (actual_arg->node_type == NOVAL) return 0;
        if (first_actual_index + child_index >= captured_count) return 0;

        captured_symbol = captured_symbols ? captured_symbols[first_actual_index + child_index] : NULL;
        if (!captured_symbol) return 0;

        state->varg_symbols[child_index] = captured_symbol;
        child_index++;
        actual_arg = actual_arg->sibling;
    }

    return inline_initialise_varg_array(context, instr_list, inline_scope, varg_arg, source_template, state);
}

static int inline_varg_actuals_are_captured(Symbol **captured_symbols,
                                            size_t captured_count,
                                            size_t first_actual_index,
                                            ASTNode *actual_arg) {
    size_t actual_index;

    if (!captured_symbols || !actual_arg) return 0;

    actual_index = first_actual_index;
    while (actual_arg) {
        if (actual_arg->node_type == NOVAL) return 0;
        if (actual_index >= captured_count || !captured_symbols[actual_index]) return 0;
        actual_arg = actual_arg->sibling;
        actual_index++;
    }

    return 1;
}

static ASTNode *inline_find_varg_arg(ASTNode *proc_def) {
    ASTNode *args;
    ASTNode *arg;

    if (!proc_def) return NULL;

    args = ast_chld(proc_def, ARGS, 0);
    arg = args ? args->child : NULL;
    while (arg) {
        if (arg->is_varg) return arg;
        arg = arg->sibling;
    }

    return NULL;
}

static int inline_call_arity_matches(ASTNode *call_node, Symbol *proc_sym, size_t *varg_count_out) {
    size_t actual_count;

    if (varg_count_out) *varg_count_out = 0;
    if (!call_node || !proc_sym) return 0;

    actual_count = inline_count_siblings(inline_call_first_user_actual(call_node));
    if (actual_count < proc_sym->fixed_args) return 0;
    if (!proc_sym->has_vargs && actual_count != proc_sym->fixed_args) return 0;

    if (varg_count_out && actual_count >= proc_sym->fixed_args) {
        *varg_count_out = actual_count - proc_sym->fixed_args;
    }

    return 1;
}

static int inline_varg_index_from_node(ASTNode *node, size_t *index_out) {
    int value;

    if (index_out) *index_out = 0;
    if (!node) return 0;

    switch (node->node_type) {
        case INTEGER:
        case CONSTANT:
            break;

        default:
            return 0;
    }

    value = node_to_integer(node);
    if (value < 1) return 0;

    if (index_out) *index_out = (size_t)value;
    return 1;
}

static walker_result inline_varg_usage_walker(walker_direction direction, ASTNode *node, void *payload) {
    InlinableCheck *check;
    size_t index;

    check = (InlinableCheck *)payload;
    if (!check || direction == in) return result_normal;

    if (node->node_type == OP_ARG_VALUE) {
        if (!node->child) {
            check->has_unsupported_varg_access = 1;
            return result_normal;
        }

        if (inline_varg_index_from_node(node->child, &index)) {
            if (index > check->max_required_varg_index) check->max_required_varg_index = index;
        } else {
            check->has_unsupported_varg_access = 1;
        }
    } else if (node->node_type == OP_ARG_IX_EXISTS) {
        if (!node->child) {
            check->has_unsupported_varg_access = 1;
        } else if (!inline_varg_index_from_node(node->child, &index)) {
            check->has_unsupported_varg_access = 1;
        }
    }

    return result_normal;
}

static int inline_analyse_varg_usage(ASTNode *proc_def, int *unsupported_out, size_t *max_required_index_out) {
    InlinableCheck check;
    ASTNode *varg_arg;

    if (unsupported_out) *unsupported_out = 0;
    if (max_required_index_out) *max_required_index_out = 0;
    if (!proc_def) return 0;

    memset(&check, 0, sizeof(check));
    varg_arg = inline_find_varg_arg(proc_def);
    check.ref_varg_mode = varg_arg && varg_arg->is_ref_arg;
    ast_wlkr(proc_def, inline_varg_usage_walker, &check);

    if (unsupported_out) *unsupported_out = check.has_unsupported_varg_access;
    if (max_required_index_out) *max_required_index_out = check.max_required_varg_index;

    return 1;
}

static ASTNode *inline_clone_ref_actual(Context *context,
                                        ASTNode *formal_node,
                                        Scope *current_scope,
                                        InlineRefActualEntry *ref_entry,
                                        InlineCloneState *state) {
    ASTNode *replacement;
    ASTNode *source_child;
    ASTNode *formal_child;
    size_t child_index;

    if (!context || !formal_node || !current_scope || !ref_entry || !ref_entry->actual_source) return NULL;

    replacement = ast_dup(context, ref_entry->actual_source);
    if (!replacement) return NULL;

    replacement->node_type = formal_node->node_type;
    replacement->scope = current_scope;

    if (ref_entry->actual_source->symbolNode && ref_entry->actual_source->symbolNode->symbol) {
        sym_adnd(ref_entry->actual_source->symbolNode->symbol,
                 replacement,
                 formal_node->symbolNode ? formal_node->symbolNode->readUsage : 0,
                 formal_node->symbolNode ? formal_node->symbolNode->writeUsage : 0);
    }

    source_child = ref_entry->actual_source->child;
    child_index = 0;
    while (source_child) {
        ASTNode *captured_ref;

        if (child_index >= ref_entry->captured_count || !ref_entry->captured_symbols[child_index]) return NULL;

        captured_ref = rxcp_remap_create_symbol_node(context,
                                                 current_scope,
                                                 source_child,
                                                 ref_entry->captured_symbols[child_index],
                                                 VAR_SYMBOL,
                                                 1,
                                                 0);
        if (!captured_ref) return NULL;

        add_ast(replacement, captured_ref);
        source_child = source_child->sibling;
        child_index++;
    }

    formal_child = formal_node->child;
    while (formal_child) {
        ASTNode *cloned_child;

        cloned_child = inline_clone_subtree_in_scope(context, formal_child, state, current_scope);
        if (!cloned_child) return NULL;

        add_ast(replacement, cloned_child);
        formal_child = formal_child->sibling;
    }

    inline_copy_replacement_semantics(replacement, formal_node);
    return replacement;
}

static ASTNode *inline_clone_ref_varg_actual(Context *context,
                                             ASTNode *source_node,
                                             Scope *current_scope,
                                             InlineRefActualEntry *ref_entry,
                                             InlineCloneState *state) {
    ASTNode *replacement;
    ASTNode *source_child;
    size_t child_index;

    if (!context || !source_node || !current_scope || !ref_entry || !ref_entry->actual_source) return NULL;

    replacement = ast_dup(context, ref_entry->actual_source);
    if (!replacement) return NULL;
    replacement->scope = current_scope;

    if (ref_entry->actual_source->symbolNode && ref_entry->actual_source->symbolNode->symbol) {
        sym_adnd(ref_entry->actual_source->symbolNode->symbol, replacement, 1, 0);
    }

    source_child = ref_entry->actual_source->child;
    child_index = 0;
    while (source_child) {
        ASTNode *captured_ref;

        if (child_index >= ref_entry->captured_count || !ref_entry->captured_symbols[child_index]) return NULL;

        captured_ref = rxcp_remap_create_symbol_node(context,
                                                 current_scope,
                                                 source_child,
                                                 ref_entry->captured_symbols[child_index],
                                                 VAR_SYMBOL,
                                                 1,
                                                 0);
        if (!captured_ref) return NULL;

        add_ast(replacement, captured_ref);
        source_child = source_child->sibling;
        child_index++;
    }

    inline_copy_replacement_semantics(replacement, source_node);
    /* Mark forwarded `.ref` vararg actuals so later call-site checks can keep
     * them as normal calls rather than recursively inlining aliasing through a
     * synthetic locator model. */
    replacement->is_varg = 1;
    replacement->is_compiler_added = 1;
    return replacement;
}

static ASTNode *inline_clone_captured_locator(Context *context,
                                              ASTNode *source_node,
                                              Scope *current_scope,
                                              InlineRefActualEntry *entry,
                                              NodeType node_type,
                                              unsigned int read_usage,
                                              unsigned int write_usage) {
    ASTNode *replacement;
    ASTNode *source_child;
    ASTNode *shape_source;
    size_t child_index;

    if (!context || !current_scope || !entry || !entry->actual_source) return NULL;

    shape_source = source_node ? source_node : entry->actual_source;
    replacement = ast_dup(context, entry->actual_source);
    if (!replacement) return NULL;

    replacement->node_type = node_type;
    replacement->scope = current_scope;

    if (entry->actual_source->symbolNode && entry->actual_source->symbolNode->symbol) {
        sym_adnd(entry->actual_source->symbolNode->symbol,
                 replacement,
                 read_usage,
                 write_usage);
    }

    source_child = entry->actual_source->child;
    child_index = 0;
    while (source_child) {
        ASTNode *captured_ref;

        if (child_index >= entry->captured_count || !entry->captured_symbols[child_index]) return NULL;

        captured_ref = rxcp_remap_create_symbol_node(context,
                                                 current_scope,
                                                 source_child,
                                                 entry->captured_symbols[child_index],
                                                 VAR_SYMBOL,
                                                 1,
                                                 0);
        if (!captured_ref) return NULL;

        add_ast(replacement, captured_ref);
        source_child = source_child->sibling;
        child_index++;
    }

    inline_copy_replacement_semantics(replacement, shape_source);
    return replacement;
}

static int inline_capture_ref_entry(Context *context,
                                    ASTNode *instr_list,
                                    Scope *inline_scope,
                                    ASTNode *actual_arg,
                                    InlineCloneState *state,
                                    InlineRefActualEntry *entry,
                                    const char *prefix) {
    ASTNode *child;
    size_t child_index;
    Symbol *formal_symbol;

    if (!context || !instr_list || !inline_scope || !actual_arg || !state || !entry || !prefix) return 0;
    if (!inline_is_supported_ref_actual(actual_arg)) return 0;

    formal_symbol = entry->formal_symbol;
    memset(entry, 0, sizeof(*entry));
    entry->formal_symbol = formal_symbol;
    entry->actual_source = actual_arg;
    entry->captured_count = inline_count_siblings(actual_arg->child);

    if (entry->captured_count) {
        entry->captured_symbols = calloc(entry->captured_count, sizeof(Symbol *));
        if (!entry->captured_symbols) return 0;
    }

    child = actual_arg->child;
    child_index = 0;
    while (child) {
        Symbol *temp_symbol;
        ASTNode *capture_assign;
        ASTNode *capture_lhs;
        ASTNode *capture_rhs;

        temp_symbol = rxcp_remap_create_temp_symbol(context, inline_scope, child, prefix, child_index);
        if (!temp_symbol) return 0;

        capture_assign = rxcp_remap_create_assignment_node(context, inline_scope, child, child);
        if (!capture_assign) return 0;
        capture_assign->target_type = child->value_type;

        capture_lhs = rxcp_remap_create_symbol_node(context,
                                                inline_scope,
                                                child,
                                                temp_symbol,
                                                VAR_TARGET,
                                                0,
                                                1);
        capture_rhs = inline_clone_subtree(context, child, state);
        if (!capture_lhs || !capture_rhs) return 0;

        add_ast(capture_assign, capture_lhs);
        add_ast(capture_assign, capture_rhs);
        add_ast(instr_list, capture_assign);

        entry->captured_symbols[child_index] = temp_symbol;
        child = child->sibling;
        child_index++;
    }

    return 1;
}

static int inline_prepare_method_receiver_copyback(Context *context,
                                                   ASTNode *instr_list,
                                                   Scope *inline_scope,
                                                   ASTNode *proc_def,
                                                   ASTNode *call_node,
                                                   InlineCloneState *clone_state) {
    ASTNode *receiver;

    if (!context || !instr_list || !inline_scope || !proc_def || !call_node || !clone_state) return 0;
    if (proc_def->node_type != METHOD) return 1;
    if (!inline_method_writes_class_attribute(proc_def)) return 1;

    receiver = inline_call_receiver(call_node);
    if (!receiver) return 0;
    if (inline_is_direct_receiver_copyback_target(receiver)) return 1;
    if (!inline_is_locator_receiver_copyback_target(receiver)) return 0;

    if (!inline_capture_ref_entry(context,
                                  instr_list,
                                  inline_scope,
                                  receiver,
                                  clone_state,
                                  &clone_state->method_receiver_copyback_entry,
                                  "__inline_receiver_ref")) {
        return 0;
    }

    clone_state->method_receiver_source_symbol = receiver->symbolNode->symbol;
    clone_state->method_receiver_uses_locator_copyback = 1;
    return 1;
}

static int inline_register_ref_actual(Context *context,
                                      ASTNode *instr_list,
                                      Scope *inline_scope,
                                      ASTNode *formal_target,
                                      ASTNode *actual_arg,
                                      InlineCloneState *state) {
    InlineRefActualEntry *new_entries;
    InlineRefActualEntry *entry;
    if (!context || !instr_list || !inline_scope || !formal_target || !actual_arg || !state) return 0;
    if (!formal_target->symbolNode || !formal_target->symbolNode->symbol) return 0;
    if (actual_arg->is_varg && actual_arg->is_compiler_added) return 0;
    if (!inline_is_supported_ref_actual(actual_arg)) return 0;

    entry = inline_find_ref_actual(state, formal_target->symbolNode->symbol);
    if (entry) return 1;

    new_entries = realloc(state->ref_entries, sizeof(InlineRefActualEntry) * (state->ref_count + 1));
    if (!new_entries) return 0;

    state->ref_entries = new_entries;
    entry = &state->ref_entries[state->ref_count];
    entry->formal_symbol = formal_target->symbolNode->symbol;
    if (!inline_capture_ref_entry(context,
                                  instr_list,
                                  inline_scope,
                                  actual_arg,
                                  state,
                                  entry,
                                  "__inline_ref")) {
        return 0;
    }

    state->ref_count++;
    return 1;
}

static int inline_capture_varg_actuals(Context *context,
                                       ASTNode *instr_list,
                                       Scope *inline_scope,
                                       ASTNode *varg_arg,
                                       ASTNode *actual_arg,
                                       InlineCloneState *state) {
    ASTNode *varg_type;
    ASTNode *source_template;
    size_t child_index;

    if (!context || !instr_list || !inline_scope || !varg_arg || !state) return 0;

    varg_type = inline_formal_default(varg_arg);
    source_template = varg_type ? varg_type : varg_arg;

    if (!actual_arg) {
        state->varg_symbols = NULL;
        state->varg_count = 0;
        return inline_initialise_varg_array(context, instr_list, inline_scope, varg_arg, source_template, state);
    }

    source_template = varg_type ? varg_type : actual_arg;
    state->varg_count = inline_count_siblings(actual_arg);
    state->varg_symbols = calloc(state->varg_count, sizeof(Symbol *));
    if (!state->varg_symbols) return 0;

    child_index = 0;
    while (actual_arg) {
        Symbol *temp_symbol;
        ASTNode *capture_assign;
        ASTNode *capture_lhs;
        ASTNode *capture_rhs;

        if (actual_arg->node_type == NOVAL) return 0;

        temp_symbol = rxcp_remap_create_temp_symbol(context,
                                                inline_scope,
                                                varg_type ? varg_type : actual_arg,
                                                "__inline_varg",
                                                child_index);
        if (!temp_symbol) return 0;

        capture_assign = rxcp_remap_create_assignment_node(context, inline_scope, actual_arg, actual_arg);
        if (!capture_assign) return 0;
        capture_assign->value_type = temp_symbol->type;
        capture_assign->target_type = temp_symbol->type;

        capture_lhs = rxcp_remap_create_symbol_node(context,
                                                inline_scope,
                                                actual_arg,
                                                temp_symbol,
                                                VAR_TARGET,
                                                0,
                                                1);
        capture_rhs = inline_clone_subtree(context, actual_arg, state);
        if (!capture_lhs || !capture_rhs) return 0;

        add_ast(capture_assign, capture_lhs);
        add_ast(capture_assign, capture_rhs);
        add_ast(instr_list, capture_assign);

        state->varg_symbols[child_index] = temp_symbol;
        child_index++;
        actual_arg = actual_arg->sibling;
    }

    return inline_initialise_varg_array(context, instr_list, inline_scope, varg_arg, source_template, state);
}

static int inline_capture_ref_varg_actuals(Context *context,
                                           ASTNode *instr_list,
                                           Scope *inline_scope,
                                           ASTNode *actual_arg,
                                           InlineCloneState *state) {
    size_t child_index;

    if (!context || !instr_list || !inline_scope || !state) return 0;

    if (!actual_arg) {
        state->varg_count = 0;
        state->varg_ref_entries = NULL;
        return 1;
    }

    state->varg_count = inline_count_siblings(actual_arg);
    state->varg_ref_entries = calloc(state->varg_count, sizeof(InlineRefActualEntry));
    if (!state->varg_ref_entries) return 0;

    child_index = 0;
    while (actual_arg) {
        if (actual_arg->node_type == NOVAL) return 0;
        if (actual_arg->symbolNode) actual_arg->symbolNode->writeUsage = 1;
        if (!inline_capture_ref_entry(context,
                                      instr_list,
                                      inline_scope,
                                      actual_arg,
                                      state,
                                      &state->varg_ref_entries[child_index],
                                      "__inline_ref_varg")) {
            return 0;
        }
        child_index++;
        actual_arg = actual_arg->sibling;
    }

    return 1;
}

static Symbol *inline_create_varg_array_symbol(Context *context,
                                               Scope *inline_scope,
                                               ASTNode *template_node,
                                               ASTNode *source_node,
                                               size_t count) {
    Symbol *array_symbol;
    int *old_base;
    int *old_elements;
    int old_dims;
    int new_dims;
    int i;
    char temp_name[80];

    if (!context || !inline_scope || !source_node) return NULL;

    snprintf(temp_name, sizeof(temp_name), "__inline_varg_array_%d", source_node->node_number);
    array_symbol = sym_fn(inline_scope, temp_name, strlen(temp_name));
    if (!array_symbol) return NULL;

    array_symbol->symbol_type = VARIABLE_SYMBOL;
    array_symbol->status = SYM_STATUS_LOCAL_VAR;
    array_symbol->register_num = UNSET_REGISTER;
    array_symbol->register_type = 'r';
    array_symbol->meta_emitted = 0;
    array_symbol->init_emitted = 0;
    array_symbol->needs_default_initiation = 1;

    if (!rxcp_remap_copy_node_value_shape_to_symbol(array_symbol, template_node ? template_node : source_node)) return NULL;

    old_base = array_symbol->dim_base;
    old_elements = array_symbol->dim_elements;
    old_dims = array_symbol->value_dims;
    new_dims = old_dims + 1;

    array_symbol->dim_base = calloc((size_t)new_dims, sizeof(int));
    array_symbol->dim_elements = calloc((size_t)new_dims, sizeof(int));
    if (!array_symbol->dim_base || !array_symbol->dim_elements) return NULL;

    array_symbol->value_dims = new_dims;
    array_symbol->dim_base[0] = 1;
    array_symbol->dim_elements[0] = (int)(count > 0 ? count : 1);

    for (i = 0; i < old_dims; i++) {
        array_symbol->dim_base[i + 1] = old_base ? old_base[i] : 1;
        array_symbol->dim_elements[i + 1] = old_elements ? old_elements[i] : 1;
    }

    free(old_base);
    free(old_elements);

    return array_symbol;
}

static ASTNode *inline_create_varg_array_slot(Context *context,
                                              Scope *scope,
                                              ASTNode *source_node,
                                              Symbol *array_symbol,
                                              ASTNode *index_node,
                                              NodeType node_type,
                                              ValueType element_type,
                                              int element_dims,
                                              int *element_base,
                                              int *element_elements,
                                              char *element_class) {
    ASTNode *slot_node;

    if (!context || !scope || !source_node || !array_symbol || !index_node) return NULL;

    slot_node = rxcp_remap_create_symbol_node(context, scope, source_node, array_symbol, node_type, 1, node_type == VAR_TARGET ? 1 : 0);
    if (!slot_node) return NULL;

    add_ast(slot_node, index_node);
    ast_set_value_type(0, slot_node, element_type, element_dims, element_base, element_elements, element_class);
    ast_set_target_type(0, slot_node, element_type, element_dims, element_base, element_elements, element_class);

    return slot_node;
}

static int inline_initialise_varg_array(Context *context,
                                        ASTNode *instr_list,
                                        Scope *inline_scope,
                                        ASTNode *varg_arg,
                                        ASTNode *source_node,
                                        InlineCloneState *state) {
    ASTNode *template_node;
    size_t i;

    if (!context || !instr_list || !inline_scope || !source_node || !state) return 0;
    if (state->varg_array_symbol) return 1;

    template_node = inline_formal_default(varg_arg);
    if (!template_node) template_node = source_node;

    state->varg_array_symbol = inline_create_varg_array_symbol(context,
                                                               inline_scope,
                                                               template_node,
                                                               source_node,
                                                               state->varg_count);
    if (!state->varg_array_symbol) return 0;

    for (i = 0; i < state->varg_count; i++) {
        ASTNode *assign_node;
        ASTNode *lhs;
        ASTNode *rhs;
        ASTNode *index_node;

        if (!state->varg_symbols || !state->varg_symbols[i]) return 0;

        assign_node = rxcp_remap_create_assignment_node(context, inline_scope, source_node, source_node);
        if (!assign_node) return 0;
        assign_node->value_type = state->varg_symbols[i]->type;
        assign_node->target_type = state->varg_symbols[i]->type;

        index_node = rxcp_remap_create_integer_constant(context, source_node, (int)(i + 1), TP_INTEGER);
        rhs = rxcp_remap_create_symbol_node(context,
                                        inline_scope,
                                        source_node,
                                        state->varg_symbols[i],
                                        VAR_SYMBOL,
                                        1,
                                        0);
        if (!index_node || !rhs) return 0;
        index_node->scope = inline_scope;

        lhs = inline_create_varg_array_slot(context,
                                            inline_scope,
                                            source_node,
                                            state->varg_array_symbol,
                                            index_node,
                                            VAR_TARGET,
                                            state->varg_symbols[i]->type,
                                            state->varg_symbols[i]->value_dims,
                                            state->varg_symbols[i]->dim_base,
                                            state->varg_symbols[i]->dim_elements,
                                            state->varg_symbols[i]->value_class);
        if (!lhs) return 0;

        add_ast(assign_node, lhs);
        add_ast(assign_node, rhs);
        add_ast(instr_list, assign_node);
    }

    return 1;
}

static ASTNode *inline_create_assembler_instr(Context *context,
                                              Scope *scope,
                                              ASTNode *source_node,
                                              const char *opcode,
                                              ASTNode *arg1,
                                              ASTNode *arg2,
                                              ASTNode *arg3) {
    ASTNode *instr;

    if (!context || !scope || !source_node || !opcode) return NULL;

    instr = ast_ftt(context, ASSEMBLER, strdup(opcode));
    if (!instr) return NULL;

    instr->free_node_string = 1;
    instr->scope = scope;
    rxcp_remap_anchor_synthetic(instr, source_node);

    if (arg1) add_ast(instr, arg1);
    if (arg2) add_ast(instr, arg2);
    if (arg3) add_ast(instr, arg3);

    return instr;
}

static ASTNode *inline_create_string_constant(Context *context, ASTNode *source_node, const char *value) {
    ASTNode *node;

    if (!context || !source_node || !value) return NULL;

    node = ast_ft(context, STRING);
    if (!node) return NULL;

    ast_copy_str(node, (char *)value);
    rxcp_remap_anchor_synthetic(node, source_node);
    ast_set_value_type(0, node, TP_STRING, 0, 0, 0, 0);
    ast_set_target_type(0, node, TP_STRING, 0, 0, 0, 0);

    return node;
}

static Symbol *inline_find_instance_symbol(ASTNode *proc_def,
                                           InlineCloneState *state) {
    ASTNode lookup_node;
    const char *name;
    size_t name_len;
    Symbol *old_symbol;
    Symbol *new_symbol;

    if (!proc_def || !proc_def->scope || !state) return NULL;

    if (proc_def->node_type == METHOD) {
        name = "\xc2\xa7" "this";
    } else if (proc_def->node_type == FACTORY) {
        name = "\xc2\xa7" "factory";
    } else {
        return NULL;
    }

    name_len = strlen(name);
    memset(&lookup_node, 0, sizeof(lookup_node));
    lookup_node.node_string = (char *)name;
    lookup_node.node_string_length = name_len;

    old_symbol = sym_lrsv(proc_def->scope, &lookup_node);
    if (!old_symbol) return NULL;

    new_symbol = inline_find_mapped_symbol(state, old_symbol);
    if (!new_symbol) return NULL;

    return new_symbol;
}

static int inline_count_factory_attributes(ASTNode *factory_def) {
    ASTNode *class_node;
    int count;

    if (!factory_def) return 0;

    class_node = factory_def->parent;
    while (class_node && class_node->node_type != CLASS_DEF) class_node = class_node->parent;
    if (!class_node) return 0;

    {
        Scope *class_scope = 0;

        if (class_node->symbolNode && class_node->symbolNode->symbol) {
            class_scope = class_node->symbolNode->symbol->defines_scope;
        }
        if (!class_scope) class_scope = class_node->scope;

        if (class_scope) {
            Symbol **symbols = scp_syms(class_scope);
            if (symbols) {
                int i;

                count = 0;
                for (i = 0; symbols[i]; i++) {
                    Symbol *s = symbols[i];
                    int index;

                    if (s->symbol_type != VARIABLE_SYMBOL) continue;
                    index = inline_class_attribute_register_num(s);
                    if (index == 0) {
                        /* register.0 is the containing value, not a child slot. */
                    } else if (index >= count) count = index + 1;
                    else if (index == UNSET_REGISTER) count++;
                }
                free(symbols);
                return count;
            }
        }
    }

    count = 0;
    {
        ASTNode *attr = class_node->child;
        while (attr) {
            if (attr->node_type == DEFINE) {
                int index;
                ASTNode *nr;

                index = -1;
                nr = ast_chld(attr, NODE_REGISTER, 0);
                if (nr) {
                    ASTNode *idx;

                    idx = ast_chld(nr, INTEGER, 0);
                    if (idx) index = node_to_integer(idx);
                    else if (nr->int_value) index = (int)nr->int_value;
                    else if (nr->child && nr->child->token) index = (int)strtol(nr->child->token->token_string, NULL, 10);
                }

                if (index == 0) {
                    /* register.0 is the containing value, not a child slot. */
                } else if (index >= count) count = index + 1;
                else if (index == -1) count++;
            }
            attr = attr->sibling;
        }
    }

    return count;
}

static int inline_bind_method_receiver(Context *context,
                                       ASTNode *instr_list,
                                       Scope *inline_scope,
                                       ASTNode *proc_def,
                                       ASTNode *call_node,
                                       InlineCloneState *clone_state,
                                       Symbol *captured_receiver_symbol) {
    Symbol *this_symbol;
    ASTNode *receiver;
    ASTNode *assign_node;
    ASTNode *assign_lhs;
    ASTNode *assign_rhs;
    int method_needs_receiver_copyback;

    if (!context || !instr_list || !inline_scope || !proc_def || !call_node || !clone_state) return 0;
    if (proc_def->node_type != METHOD) return 1;

    receiver = inline_call_receiver(call_node);
    if (!receiver) return 0;

    this_symbol = inline_find_instance_symbol(proc_def, clone_state);
    if (!this_symbol) return 0;
    method_needs_receiver_copyback = inline_method_writes_class_attribute(proc_def);

    assign_node = rxcp_remap_create_assignment_node(context, inline_scope, receiver, receiver);
    if (!assign_node) return 0;

    assign_lhs = rxcp_remap_create_symbol_node(context,
                                           inline_scope,
                                           receiver,
                                           this_symbol,
                                           VAR_TARGET,
                                           0,
                                           1);
    if (captured_receiver_symbol) {
        assign_rhs = rxcp_remap_create_symbol_node(context,
                                               inline_scope,
                                               receiver,
                                               captured_receiver_symbol,
                                               VAR_SYMBOL,
                                               1,
                                               0);
    } else if (clone_state->method_receiver_uses_locator_copyback) {
        assign_rhs = inline_clone_captured_locator(context,
                                                   receiver,
                                                   inline_scope,
                                                   &clone_state->method_receiver_copyback_entry,
                                                   VAR_SYMBOL,
                                                   1,
                                                   0);
    } else {
        assign_rhs = inline_clone_subtree(context, receiver, clone_state);
    }
    if (!assign_lhs || !assign_rhs) return 0;

    add_ast(assign_node, assign_lhs);
    add_ast(assign_node, assign_rhs);
    add_ast(instr_list, assign_node);

    if (method_needs_receiver_copyback &&
        inline_is_direct_receiver_copyback_target(receiver) &&
        receiver->symbolNode &&
        receiver->symbolNode->symbol) {
        clone_state->method_receiver_source_symbol = receiver->symbolNode->symbol;
        clone_state->method_receiver_local_symbol = this_symbol;
        clone_state->method_receiver_needs_copyback = 1;
        clone_state->method_receiver_uses_locator_copyback = 0;
    } else if (method_needs_receiver_copyback &&
               clone_state->method_receiver_uses_locator_copyback &&
               receiver->symbolNode &&
               receiver->symbolNode->symbol) {
        clone_state->method_receiver_source_symbol = receiver->symbolNode->symbol;
        clone_state->method_receiver_local_symbol = this_symbol;
        clone_state->method_receiver_needs_copyback = 1;
    }

    return 1;
}

static int inline_initialise_factory_instance(Context *context,
                                              ASTNode *instr_list,
                                              Scope *inline_scope,
                                              ASTNode *proc_def,
                                              InlineCloneState *clone_state) {
    Symbol *factory_symbol;
    ASTNode *class_node;
    ASTNode *factory_target;
    ASTNode *attrs_count;
    ASTNode *setattrs_instr;
    ASTNode *setobjtype_target;
    ASTNode *class_name_node;
    ASTNode *setobjtype_instr;
    char *class_fq;

    if (!context || !instr_list || !inline_scope || !proc_def || !clone_state) return 0;
    if (proc_def->node_type != FACTORY) return 1;

    factory_symbol = inline_find_instance_symbol(proc_def, clone_state);
    if (!factory_symbol) return 0;

    factory_target = rxcp_remap_create_symbol_node(context,
                                               inline_scope,
                                               proc_def,
                                               factory_symbol,
                                               VAR_TARGET,
                                               0,
                                               1);
    attrs_count = rxcp_remap_create_integer_constant(context,
                                                 proc_def,
                                                 inline_count_factory_attributes(proc_def),
                                                 TP_INTEGER);
    if (!factory_target || !attrs_count) return 0;
    attrs_count->scope = inline_scope;

    setattrs_instr = inline_create_assembler_instr(context,
                                                   inline_scope,
                                                   proc_def,
                                                   "setattrs",
                                                   factory_target,
                                                   attrs_count,
                                                   NULL);
    if (!setattrs_instr) return 0;
    add_ast(instr_list, setattrs_instr);

    class_node = proc_def->parent;
    while (class_node && class_node->node_type != CLASS_DEF) class_node = class_node->parent;
    if (!class_node || !class_node->symbolNode || !class_node->symbolNode->symbol) return 1;

    class_fq = sym_frnm(class_node->symbolNode->symbol);
    if (!class_fq) return 0;

    setobjtype_target = rxcp_remap_create_symbol_node(context,
                                                  inline_scope,
                                                  proc_def,
                                                  factory_symbol,
                                                  VAR_TARGET,
                                                  0,
                                                  1);
    class_name_node = inline_create_string_constant(context, proc_def, class_fq);
    free(class_fq);
    if (!setobjtype_target || !class_name_node) return 0;
    class_name_node->scope = inline_scope;

    setobjtype_instr = inline_create_assembler_instr(context,
                                                     inline_scope,
                                                     proc_def,
                                                     "setobjtype",
                                                     setobjtype_target,
                                                     class_name_node,
                                                     NULL);
    if (!setobjtype_instr) return 0;
    add_ast(instr_list, setobjtype_instr);

    return 1;
}

typedef struct {
    ASTNode *instr_list;
    Scope *inline_scope;
    ASTNode *source_node;
    InlineCloneState *clone_state;
} InlineReceiverCopybackService;

static int inline_append_method_receiver_copyback_impl(Context *context,
                                                       ASTNode *instr_list,
                                                       Scope *inline_scope,
                                                       ASTNode *source_node,
                                                       InlineCloneState *clone_state) {
    ASTNode *copy_lhs;
    ASTNode *copy_rhs;
    ASTNode *value_copy;
    ASTNode *copy_assign;
    InlineRefActualEntry *receiver_entry;

    if (!context || !instr_list || !inline_scope || !source_node || !clone_state) return 0;
    if (!clone_state->method_receiver_needs_copyback) return 1;
    if (!clone_state->method_receiver_source_symbol || !clone_state->method_receiver_local_symbol) return 0;

    if (clone_state->method_receiver_uses_locator_copyback) {
        receiver_entry = &clone_state->method_receiver_copyback_entry;
        if (!receiver_entry->actual_source) return 0;

        copy_assign = rxcp_remap_create_assignment_node(context,
                                                        inline_scope,
                                                        receiver_entry->actual_source,
                                                        receiver_entry->actual_source);
        copy_lhs = inline_clone_captured_locator(context,
                                                 receiver_entry->actual_source,
                                                 inline_scope,
                                                 receiver_entry,
                                                 VAR_TARGET,
                                                 0,
                                                 1);
        copy_rhs = rxcp_remap_create_symbol_node(context,
                                             inline_scope,
                                             source_node,
                                             clone_state->method_receiver_local_symbol,
                                             VAR_SYMBOL,
                                             1,
                                             0);
        if (!copy_assign || !copy_lhs || !copy_rhs) return 0;

        add_ast(copy_assign, copy_lhs);
        add_ast(copy_assign, copy_rhs);
        add_ast(instr_list, copy_assign);
        return 1;
    }

    copy_lhs = rxcp_remap_create_symbol_node(context,
                                         inline_scope,
                                         source_node,
                                         clone_state->method_receiver_source_symbol,
                                         VAR_TARGET,
                                         0,
                                         1);
    copy_rhs = rxcp_remap_create_symbol_node(context,
                                         inline_scope,
                                         source_node,
                                         clone_state->method_receiver_local_symbol,
                                         VAR_SYMBOL,
                                         1,
                                         0);
    if (!copy_lhs || !copy_rhs) return 0;

    value_copy = inline_create_register_copy_instr(context, inline_scope, "copy", copy_lhs, copy_rhs);
    if (!value_copy) return 0;

    add_ast(instr_list, value_copy);
    return 1;
}

static int inline_receiver_copyback_service(Context *context, void *payload) {
    InlineReceiverCopybackService *service;

    service = (InlineReceiverCopybackService *)payload;
    if (!service) return 0;

    return inline_append_method_receiver_copyback_impl(context,
                                                       service->instr_list,
                                                       service->inline_scope,
                                                       service->source_node,
                                                       service->clone_state);
}

static int inline_append_method_receiver_copyback(Context *context,
                                                  ASTNode *instr_list,
                                                  Scope *inline_scope,
                                                  ASTNode *source_node,
                                                  InlineCloneState *clone_state) {
    InlineReceiverCopybackService service;
    RxcpRemapResult result;
    Symbol *symbol;

    service.instr_list = instr_list;
    service.inline_scope = inline_scope;
    service.source_node = source_node;
    service.clone_state = clone_state;
    symbol = clone_state ? clone_state->method_receiver_source_symbol : NULL;

    result = rxcp_remap_run_service(context,
                                    rxcp_inline_receiver_copyback_rule(),
                                    source_node,
                                    symbol,
                                    inline_receiver_copyback_service,
                                    &service,
                                    rxcp_inline_remap_trace_hooks());
    return result == RXCP_REMAP_APPLIED;
}

static ASTNode *inline_build_dynamic_varg_value(Context *context,
                                                ASTNode *node,
                                                Scope *current_scope,
                                                InlineCloneState *state) {
    ASTNode *block_expr;
    ASTNode *instr_list;
    Scope *inline_scope;
    Symbol *index_symbol;
    ASTNode *assign_node;
    ASTNode *assign_lhs;
    ASTNode *assign_rhs;
    ASTNode *leave_node;
    ASTNode *slot_node;
    ASTNode *index_ref;

    if (!context || !node || !current_scope || !state || !state->varg_array_symbol || !node->child) return NULL;

    block_expr = rxcp_remap_create_block_expr(context,
                                              current_scope,
                                              node,
                                              NULL,
                                              &inline_scope,
                                              &instr_list);
    if (!block_expr) return NULL;

    index_symbol = rxcp_remap_create_temp_symbol(context, inline_scope, node->child, "__inline_arg_ix", 0);
    if (!index_symbol) return NULL;

    assign_node = rxcp_remap_create_assignment_node(context, inline_scope, node, node->child);
    if (!assign_node) return NULL;
    assign_node->value_type = TP_INTEGER;
    assign_node->target_type = TP_INTEGER;

    assign_lhs = rxcp_remap_create_symbol_node(context, inline_scope, node->child, index_symbol, VAR_TARGET, 0, 1);
    assign_rhs = inline_clone_subtree_in_scope(context, node->child, state, inline_scope);
    if (!assign_lhs || !assign_rhs) return NULL;
    add_ast(assign_node, assign_lhs);
    add_ast(assign_node, assign_rhs);
    add_ast(instr_list, assign_node);

    index_ref = rxcp_remap_create_symbol_node(context, inline_scope, node, index_symbol, VAR_SYMBOL, 1, 0);
    if (!index_ref) return NULL;

    slot_node = inline_create_varg_array_slot(context,
                                              inline_scope,
                                              node,
                                              state->varg_array_symbol,
                                              index_ref,
                                              VAR_SYMBOL,
                                              node->value_type,
                                              node->value_dims,
                                              node->value_dim_base,
                                              node->value_dim_elements,
                                              node->value_class);
    if (!slot_node) return NULL;

    leave_node = rxcp_remap_create_leave_with(context, inline_scope, node, block_expr, slot_node);
    if (!leave_node) return NULL;
    add_ast(instr_list, leave_node);
    return block_expr;
}

static ASTNode *inline_build_dynamic_varg_exists(Context *context,
                                                 ASTNode *node,
                                                 Scope *current_scope,
                                                 InlineCloneState *state) {
    ASTNode *block_expr;
    ASTNode *instr_list;
    Scope *inline_scope;
    Symbol *index_symbol;
    ASTNode *assign_node;
    ASTNode *assign_lhs;
    ASTNode *assign_rhs;
    ASTNode *leave_node;
    ASTNode *index_ref;
    ASTNode *const_one;
    ASTNode *const_max;
    ASTNode *gte_node;
    ASTNode *lte_node;
    ASTNode *and_node;

    if (!context || !node || !current_scope || !state || !node->child) return NULL;

    block_expr = rxcp_remap_create_block_expr(context,
                                              current_scope,
                                              node,
                                              NULL,
                                              &inline_scope,
                                              &instr_list);
    if (!block_expr) return NULL;

    index_symbol = rxcp_remap_create_temp_symbol(context, inline_scope, node->child, "__inline_arg_ix", 1);
    if (!index_symbol) return NULL;

    assign_node = rxcp_remap_create_assignment_node(context, inline_scope, node, node->child);
    if (!assign_node) return NULL;
    assign_node->value_type = TP_INTEGER;
    assign_node->target_type = TP_INTEGER;

    assign_lhs = rxcp_remap_create_symbol_node(context, inline_scope, node->child, index_symbol, VAR_TARGET, 0, 1);
    assign_rhs = inline_clone_subtree_in_scope(context, node->child, state, inline_scope);
    if (!assign_lhs || !assign_rhs) return NULL;
    add_ast(assign_node, assign_lhs);
    add_ast(assign_node, assign_rhs);
    add_ast(instr_list, assign_node);

    index_ref = rxcp_remap_create_symbol_node(context, inline_scope, node, index_symbol, VAR_SYMBOL, 1, 0);
    const_one = rxcp_remap_create_integer_constant(context, node, 1, TP_INTEGER);
    const_max = rxcp_remap_create_integer_constant(context, node, (int)state->varg_count, TP_INTEGER);
    if (!index_ref || !const_one || !const_max) return NULL;
    index_ref->scope = inline_scope;
    const_one->scope = inline_scope;
    const_max->scope = inline_scope;

    gte_node = ast_f(context, OP_COMPARE_GTE, node->token);
    lte_node = ast_f(context, OP_COMPARE_LTE, node->token);
    and_node = ast_f(context, OP_AND, node->token);
    if (!gte_node || !lte_node || !and_node) return NULL;

    gte_node->scope = inline_scope;
    lte_node->scope = inline_scope;
    and_node->scope = inline_scope;

    ast_set_value_type(0, gte_node, TP_BOOLEAN, 0, 0, 0, 0);
    ast_set_target_type(0, gte_node, TP_BOOLEAN, 0, 0, 0, 0);
    ast_set_value_type(0, lte_node, TP_BOOLEAN, 0, 0, 0, 0);
    ast_set_target_type(0, lte_node, TP_BOOLEAN, 0, 0, 0, 0);
    ast_set_value_type(0, and_node, TP_BOOLEAN, 0, 0, 0, 0);
    ast_set_target_type(0, and_node, TP_BOOLEAN, 0, 0, 0, 0);

    add_ast(gte_node, index_ref);
    add_ast(gte_node, const_one);

    index_ref = rxcp_remap_create_symbol_node(context, inline_scope, node, index_symbol, VAR_SYMBOL, 1, 0);
    if (!index_ref) return NULL;
    add_ast(lte_node, index_ref);
    add_ast(lte_node, const_max);

    add_ast(and_node, gte_node);
    add_ast(and_node, lte_node);
    leave_node = rxcp_remap_create_leave_with(context, inline_scope, node, block_expr, and_node);
    if (!leave_node) return NULL;
    add_ast(instr_list, leave_node);

    return block_expr;
}

typedef struct {
    ASTNode *instr_list;
    Scope *inline_scope;
    ASTNode *proc_def;
    ASTNode *call_node;
    Symbol *proc_sym;
    InlineCloneState *clone_state;
} InlineBindActualsService;

static int inline_bind_call_arguments_impl(Context *context,
                                           ASTNode *instr_list,
                                           Scope *inline_scope,
                                           ASTNode *proc_def,
                                           ASTNode *call_node,
                                           Symbol *proc_sym,
                                           InlineCloneState *clone_state) {
    ASTNode *param_list;
    ASTNode *param_arg;
    ASTNode *actual_arg;
    ASTNode *varg_arg;
    Symbol *captured_method_receiver;
    Symbol **captured_scoped_actuals;
    size_t captured_scoped_actual_count;
    size_t actual_index;

#define INLINE_BIND_RETURN(value) do { \
    free(captured_scoped_actuals); \
    return (value); \
} while (0)

    if (!context || !instr_list || !inline_scope || !proc_def || !call_node || !proc_sym || !clone_state) return 0;

    captured_scoped_actuals = NULL;
    captured_method_receiver = NULL;
    captured_scoped_actual_count = 0;
    actual_index = 0;

    if (!inline_call_arity_matches(call_node, proc_sym, NULL)) INLINE_BIND_RETURN(0);
    if (!inline_prepare_method_receiver_copyback(context,
                                                 instr_list,
                                                 inline_scope,
                                                 proc_def,
                                                 call_node,
                                                 clone_state)) {
        INLINE_BIND_RETURN(0);
    }
    if (!inline_capture_scoped_call_actuals(context,
                                            instr_list,
                                            inline_scope,
                                            proc_def,
                                            call_node,
                                            clone_state,
                                            &captured_method_receiver,
                                            &captured_scoped_actuals,
                                            &captured_scoped_actual_count)) {
        INLINE_BIND_RETURN(0);
    }
    if (!inline_initialise_factory_instance(context, instr_list, inline_scope, proc_def, clone_state)) INLINE_BIND_RETURN(0);
    if (!inline_bind_method_receiver(context,
                                     instr_list,
                                     inline_scope,
                                     proc_def,
                                     call_node,
                                     clone_state,
                                     captured_method_receiver)) {
        INLINE_BIND_RETURN(0);
    }

    param_list = ast_chld(proc_def, ARGS, 0);
    param_arg = param_list ? param_list->child : NULL;
    actual_arg = inline_call_first_user_actual(call_node);
    varg_arg = inline_find_varg_arg(proc_def);

    while (param_arg) {
        ASTNode *formal_target;
        ASTNode *formal_default;
        ASTNode *bind_assign;
        ASTNode *bind_lhs;
        ASTNode *bind_rhs;
        ASTNode *bind_source;
        Symbol *captured_actual_symbol;

        if (param_arg == varg_arg) break;

        formal_target = inline_formal_target(param_arg);
        formal_default = inline_formal_default(param_arg);
        captured_actual_symbol = actual_index < captured_scoped_actual_count ?
                                 captured_scoped_actuals[actual_index] : NULL;
        if (!formal_target || !actual_arg) INLINE_BIND_RETURN(0);

        if (param_arg->is_ref_arg && !inline_is_missing_actual(actual_arg)) {
            if (!inline_register_ref_actual(context, instr_list, inline_scope, formal_target, actual_arg, clone_state)) {
                INLINE_BIND_RETURN(0);
            }
            param_arg = param_arg->sibling;
            actual_arg = actual_arg->sibling;
            actual_index++;
            continue;
        }

        if (inline_is_missing_actual(actual_arg)) {
            if (!param_arg->is_opt_arg || !formal_default) INLINE_BIND_RETURN(0);
        }

        if (inline_node_is_plain_object(formal_target) &&
            param_arg->is_const_arg &&
            !inline_is_missing_actual(actual_arg)) {
            if (!inline_register_ref_actual(context, instr_list, inline_scope, formal_target, actual_arg, clone_state)) {
                INLINE_BIND_RETURN(0);
            }
            param_arg = param_arg->sibling;
            actual_arg = actual_arg->sibling;
            actual_index++;
            continue;
        }

        bind_assign = rxcp_remap_create_assignment_node(context, inline_scope, formal_target, formal_target);
        if (!bind_assign) INLINE_BIND_RETURN(0);

        bind_lhs = inline_clone_subtree(context, formal_target, clone_state);
        if (inline_is_missing_actual(actual_arg)) {
            bind_source = formal_default;
        } else if (captured_actual_symbol) {
            bind_source = actual_arg;
        } else {
            bind_source = actual_arg;
        }

        if (captured_actual_symbol) {
            bind_rhs = rxcp_remap_create_symbol_node(context,
                                                 inline_scope,
                                                 actual_arg,
                                                 captured_actual_symbol,
                                                 VAR_SYMBOL,
                                                 1,
                                                 0);
        } else if (inline_formal_needs_isolated_copy(formal_target, param_arg) &&
            !inline_is_direct_symbol_actual(bind_source)) {
            bind_rhs = inline_create_temp_value_ref(context,
                                                    instr_list,
                                                    inline_scope,
                                                    bind_source,
                                                    clone_state,
                                                    "__inline_bind",
                                                    0);
        } else if (inline_is_missing_actual(actual_arg)) {
            bind_rhs = inline_clone_subtree(context, formal_default, clone_state);
            bind_source = formal_default;
        } else {
            bind_rhs = inline_clone_subtree(context, actual_arg, clone_state);
            bind_source = actual_arg;
        }

        if (!bind_lhs || !bind_rhs) INLINE_BIND_RETURN(0);

        if (inline_formal_needs_isolated_copy(formal_target, param_arg)) {
            ASTNode *bind_copy;

            bind_copy = inline_create_register_copy_instr(context, inline_scope, "copy", bind_lhs, bind_rhs);
            if (!bind_copy) INLINE_BIND_RETURN(0);

            add_ast(instr_list, bind_copy);
        } else {
            add_ast(bind_assign, bind_lhs);
            add_ast(bind_assign, bind_rhs);
            add_ast(instr_list, bind_assign);
        }

        param_arg = param_arg->sibling;
        actual_arg = actual_arg->sibling;
        actual_index++;
    }

    if (varg_arg) {
        if (varg_arg->is_ref_arg) {
            if (!inline_capture_ref_varg_actuals(context, instr_list, inline_scope, actual_arg, clone_state)) {
                INLINE_BIND_RETURN(0);
            }
        } else if ((proc_def->node_type == FACTORY || proc_def->node_type == METHOD) &&
                   inline_varg_actuals_are_captured(captured_scoped_actuals,
                                                    captured_scoped_actual_count,
                                                    actual_index,
                                                    actual_arg)) {
            if (!inline_capture_varg_captured_actuals(context,
                                                      instr_list,
                                                      inline_scope,
                                                      varg_arg,
                                                      actual_arg,
                                                      clone_state,
                                                      captured_scoped_actuals,
                                                      captured_scoped_actual_count,
                                                      actual_index)) {
                INLINE_BIND_RETURN(0);
            }
        } else {
            if (!inline_capture_varg_actuals(context, instr_list, inline_scope, varg_arg, actual_arg, clone_state)) {
                INLINE_BIND_RETURN(0);
            }
        }
        actual_arg = NULL;
        param_arg = varg_arg ? varg_arg->sibling : param_arg;
    }

    if (actual_arg || param_arg) INLINE_BIND_RETURN(0);

    INLINE_BIND_RETURN(1);
#undef INLINE_BIND_RETURN
}

static int inline_bind_call_arguments_service(Context *context, void *payload) {
    InlineBindActualsService *service;

    service = (InlineBindActualsService *)payload;
    if (!service) return 0;

    return inline_bind_call_arguments_impl(context,
                                           service->instr_list,
                                           service->inline_scope,
                                           service->proc_def,
                                           service->call_node,
                                           service->proc_sym,
                                           service->clone_state);
}

static int inline_bind_call_arguments(Context *context,
                                      ASTNode *instr_list,
                                      Scope *inline_scope,
                                      ASTNode *proc_def,
                                      ASTNode *call_node,
                                      Symbol *proc_sym,
                                      InlineCloneState *clone_state) {
    InlineBindActualsService service;
    RxcpRemapResult result;

    service.instr_list = instr_list;
    service.inline_scope = inline_scope;
    service.proc_def = proc_def;
    service.call_node = call_node;
    service.proc_sym = proc_sym;
    service.clone_state = clone_state;

    result = rxcp_remap_run_service(context,
                                    rxcp_inline_bind_actuals_rule(),
                                    call_node,
                                    proc_sym,
                                    inline_bind_call_arguments_service,
                                    &service,
                                    rxcp_inline_remap_trace_hooks());
    return result == RXCP_REMAP_APPLIED;
}
