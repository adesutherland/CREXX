/*
 * Private inline implementation fragment.
 * Included by rxcp_inline.c; not compiled separately.
 */

static int inline_append_scope_map_entry(InlineCloneState *state, Scope *old_scope, Scope *new_scope) {
    InlineScopeMapEntry *new_entries;

    if (!state || !old_scope || !new_scope) return 0;

    new_entries = realloc(state->scope_entries,
                          sizeof(InlineScopeMapEntry) * (state->scope_count + 1));
    if (!new_entries) return 0;

    state->scope_entries = new_entries;
    state->scope_entries[state->scope_count].old_scope = old_scope;
    state->scope_entries[state->scope_count].new_scope = new_scope;
    state->scope_count++;
    return 1;
}

static int inline_append_node_map_entry(InlineCloneState *state, ASTNode *old_node, ASTNode *new_node) {
    InlineNodeMapEntry *new_entries;

    if (!state || !old_node || !new_node) return 0;

    new_entries = realloc(state->node_entries,
                          sizeof(InlineNodeMapEntry) * (state->node_count + 1));
    if (!new_entries) return 0;

    state->node_entries = new_entries;
    state->node_entries[state->node_count].old_node = old_node;
    state->node_entries[state->node_count].new_node = new_node;
    state->node_count++;
    return 1;
}

static int inline_duplicate_scope_symbols(Scope *old_scope,
                                          Scope *new_scope,
                                          InlineCloneState *state) {
    Symbol **symbols;
    size_t i;

    if (!old_scope || !new_scope || !state) return 0;

    symbols = scp_syms(old_scope);
    if (!symbols) return 1;

    for (i = 0; symbols[i]; i++) {
        Symbol *old_symbol;
        Symbol *new_symbol;

        old_symbol = symbols[i];
        if (!old_symbol || old_symbol->symbol_type == FUNCTION_SYMBOL) continue;
        if (inline_find_mapped_symbol(state, old_symbol)) continue;

        new_symbol = sym_dup(new_scope, old_symbol);
        if (!new_symbol) {
            free(symbols);
            return 0;
        }

        new_symbol->register_num = UNSET_REGISTER;
        new_symbol->register_type = 'r';
        new_symbol->meta_emitted = 0;
        new_symbol->init_emitted = 0;
        new_symbol->needs_default_initiation = old_symbol->needs_default_initiation;
        new_symbol->defines_scope = NULL;
        new_symbol->ast_template = NULL;
        new_symbol->is_inlinable = 0;
        /*
         * Once a by-value formal has been cloned into an inline body it is
         * bound into local storage by inline_bind_call_arguments(); it is no
         * longer a VM argument slot. Keep `.ref` formals fenced because they
         * alias caller-visible storage.
         */
        if (old_symbol->is_arg && !old_symbol->is_ref_arg) {
            new_symbol->is_arg = 0;
        }

        if (!inline_append_symbol_map_entry(state, old_symbol, new_symbol)) {
            free(symbols);
            return 0;
        }
    }

    free(symbols);
    return 1;
}

static Scope *inline_clone_scope(Context *context,
                                 Scope *old_scope,
                                 Scope *new_parent,
                                 ASTNode *new_defining_node,
                                 InlineCloneState *state) {
    Scope *new_scope;

    if (!context || !old_scope || !state) return NULL;

    new_scope = rxcp_remap_create_scope(context,
                                        new_parent,
                                        new_defining_node,
                                        old_scope->type,
                                        old_scope,
                                        old_scope->name);
    if (!new_scope) return NULL;

    if (!inline_append_scope_map_entry(state, old_scope, new_scope)) return NULL;
    if (!inline_duplicate_scope_symbols(old_scope, new_scope, state)) return NULL;

    return new_scope;
}

static Scope *inline_prepare_cloned_node_scope(Context *context,
                                               ASTNode *old_node,
                                               ASTNode *new_node,
                                               Scope *current_scope,
                                               InlineCloneState *state) {
    Scope *node_scope;

    if (!new_node) return NULL;

    node_scope = current_scope ? current_scope : old_node->scope;

    if (old_node->inherit_parent_scope && old_node->scope) {
        Scope *mapped_scope;

        mapped_scope = inline_find_mapped_scope(state, old_node->scope);
        if (mapped_scope) node_scope = mapped_scope;
    }

    if (old_node->scope && old_node->scope->defining_node == old_node) {
        node_scope = inline_find_mapped_scope(state, old_node->scope);
        if (!node_scope) {
            node_scope = inline_clone_scope(context, old_node->scope, current_scope, new_node, state);
            if (!node_scope) return NULL;
        }
        new_node->scope = node_scope;
        if (!inline_append_node_map_entry(state, old_node, new_node)) return NULL;
        return node_scope;
    }

    if (inline_node_requires_local_scope(old_node)) {
        node_scope = rxcp_remap_create_local_scope(context, current_scope, new_node, NULL);
        if (!node_scope) return NULL;
    }

    new_node->scope = node_scope;
    return node_scope;
}

static ASTNode *inline_clone_factory_selector_association(Context *context,
                                                          ASTNode *node,
                                                          Scope *node_scope) {
    ASTNode *selector;

    if (!context || !node || node->node_type != FACTORY_CALL || !node->association) return NULL;

    selector = ast_dup(context, node->association);
    if (!selector) return NULL;

    selector->scope = node_scope;
    return selector;
}

static ASTNode *inline_clone_subtree_in_scope(Context *context,
                                              ASTNode *node,
                                              InlineCloneState *state,
                                              Scope *current_scope) {
    ASTNode *new_node;
    ASTNode *child;
    Symbol *mapped_symbol;
    Scope *node_scope;
    ASTNode *mapped_association;

    if (!node) return NULL;

    if (state && node->symbolNode && node->symbolNode->symbol &&
        (node->node_type == VAR_SYMBOL || node->node_type == VAR_TARGET || node->node_type == VAR_REFERENCE)) {
        InlineRefActualEntry *ref_entry;

        ref_entry = inline_find_ref_actual(state, node->symbolNode->symbol);
        if (ref_entry) return inline_clone_ref_actual(context, node, current_scope, ref_entry, state);
    }

    if (state && node->node_type == OP_ARGS) {
        ASTNode *count_node;

        count_node = rxcp_remap_create_integer_constant(context, node, (int)state->varg_count, TP_INTEGER);
        if (count_node) count_node->scope = current_scope;
        return count_node;
    }

    if (state && node->node_type == OP_ARG_VALUE) {
        size_t index;
        ASTNode *replacement;
        InlineRefActualEntry *ref_varg_entry;

        if (!node->child) return NULL;
        if (!inline_varg_index_from_node(node->child, &index)) {
            return inline_build_dynamic_varg_value(context, node, current_scope, state);
        }
        ref_varg_entry = inline_find_ref_varg_actual(state, index);
        if (ref_varg_entry) {
            return inline_clone_ref_varg_actual(context, node, current_scope, ref_varg_entry, state);
        }
        if (index < 1 || index > state->varg_count || !state->varg_symbols || !state->varg_symbols[index - 1]) return NULL;

        replacement = rxcp_remap_create_symbol_node(context,
                                                current_scope,
                                                node,
                                                state->varg_symbols[index - 1],
                                                VAR_SYMBOL,
                                                1,
                                                0);
        if (!replacement) return NULL;
        ast_set_target_type(0,
                            replacement,
                            node->target_type,
                            node->target_dims,
                            node->target_dim_base,
                            node->target_dim_elements,
                            node->target_class);
        replacement->is_ref_arg = node->is_ref_arg;
        replacement->is_opt_arg = node->is_opt_arg;
        replacement->is_const_arg = node->is_const_arg;
        return replacement;
    }

    if (state && node->node_type == OP_ARG_IX_EXISTS) {
        size_t index;
        ASTNode *exists_node;

        if (!node->child) return NULL;
        if (!inline_varg_index_from_node(node->child, &index)) {
            return inline_build_dynamic_varg_exists(context, node, current_scope, state);
        }
        exists_node = rxcp_remap_create_integer_constant(context,
                                                     node,
                                                     index <= state->varg_count ? 1 : 0,
                                                     TP_BOOLEAN);
        if (exists_node) exists_node->scope = current_scope;
        return exists_node;
    }

    if (state && node->node_type == OP_ARG_EXISTS && node->symbolNode && node->symbolNode->symbol) {
        InlineOptionalPresenceEntry *entry;
        ASTNode *exists_node;

        entry = inline_find_optional_presence(state, node->symbolNode->symbol);
        if (entry) {
            exists_node = rxcp_remap_create_integer_constant(context,
                                                            node,
                                                            entry->present ? 1 : 0,
                                                            TP_BOOLEAN);
            if (exists_node) exists_node->scope = current_scope;
            return exists_node;
        }
    }

    new_node = ast_dup(context, node);
    if (!new_node) return NULL;

    node_scope = inline_prepare_cloned_node_scope(context, node, new_node, current_scope, state);
    if (!node_scope && inline_node_requires_local_scope(node)) return NULL;

    if (node->association) {
        mapped_association = inline_find_mapped_node(state, node->association);
        if (mapped_association) new_node->association = mapped_association;
        else if (node->node_type == FACTORY_CALL) {
            new_node->association = inline_clone_factory_selector_association(context, node, node_scope);
            if (!new_node->association) return NULL;
        } else {
            new_node->association = node->association;
        }
    }

    if (node->symbolNode && node->symbolNode->symbol) {
        mapped_symbol = inline_find_mapped_symbol(state, node->symbolNode->symbol);
        if (!mapped_symbol) mapped_symbol = node->symbolNode->symbol;
        sym_adnd(mapped_symbol, new_node, node->symbolNode->readUsage, node->symbolNode->writeUsage);
    }

    child = node->child;
    while (child) {
        ASTNode *cloned_child;

        cloned_child = inline_clone_subtree_in_scope(context, child, state, node_scope);
        if (!cloned_child) return NULL;
        add_ast(new_node, cloned_child);
        child = child->sibling;
    }

    return new_node;
}

static ASTNode *inline_clone_subtree(Context *context, ASTNode *node, InlineCloneState *state) {
    if (!state) return ast_dup_subtree(context, node);
    return inline_clone_subtree_in_scope(context, node, state, state->inline_scope);
}

typedef struct {
    ASTNode *source;
    InlineCloneState *clone_state;
    ASTNode *cloned;
} InlineCloneBodyService;

static int inline_clone_body_instruction_service(Context *context, void *payload) {
    InlineCloneBodyService *service;

    service = (InlineCloneBodyService *)payload;
    if (!service || !service->source || !service->clone_state) return 0;

    service->cloned = inline_clone_subtree(context, service->source, service->clone_state);
    return service->cloned != NULL;
}

static ASTNode *inline_clone_body_instruction(Context *context,
                                              ASTNode *source,
                                              InlineCloneState *clone_state) {
    InlineCloneBodyService service;
    RxcpRemapResult result;

    service.source = source;
    service.clone_state = clone_state;
    service.cloned = NULL;

    result = rxcp_remap_run_service(context,
                                    rxcp_inline_clone_body_rule(),
                                    source,
                                    NULL,
                                    inline_clone_body_instruction_service,
                                    &service,
                                    rxcp_inline_remap_trace_hooks());
    return result == RXCP_REMAP_APPLIED ? service.cloned : NULL;
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
    if (!callee_scope || !inline_scope || !state) return 0;

    if (!state->symbol_entries) state->symbol_count = 0;
    if (!state->scope_entries) state->scope_count = 0;
    if (!state->node_entries) state->node_count = 0;
    state->callee_scope = callee_scope;
    state->inline_scope = inline_scope;
    rxcp_remap_copy_numeric_context(inline_scope, callee_scope);

    if (!inline_append_scope_map_entry(state, callee_scope, inline_scope)) return 0;
    return inline_duplicate_scope_symbols(callee_scope, inline_scope, state);
}

static void inline_free_symbol_map(InlineCloneState *state) {
    size_t i;

    if (!state) return;
    if (state->symbol_entries) free(state->symbol_entries);
    if (state->scope_entries) free(state->scope_entries);
    if (state->node_entries) free(state->node_entries);
    if (state->ref_entries) {
        for (i = 0; i < state->ref_count; i++) {
            rxcp_remap_free_captured_locator(&state->ref_entries[i].locator);
        }
        free(state->ref_entries);
    }
    if (state->optional_presence_entries) free(state->optional_presence_entries);
    if (state->varg_ref_entries) {
        for (i = 0; i < state->varg_count; i++) {
            rxcp_remap_free_captured_locator(&state->varg_ref_entries[i].locator);
        }
        free(state->varg_ref_entries);
    }
    rxcp_remap_free_captured_locator(&state->method_receiver_copyback_locator);
    if (state->varg_symbols) free(state->varg_symbols);
    state->symbol_entries = NULL;
    state->symbol_count = 0;
    state->scope_entries = NULL;
    state->scope_count = 0;
    state->node_entries = NULL;
    state->node_count = 0;
    state->ref_entries = NULL;
    state->ref_count = 0;
    state->optional_presence_entries = NULL;
    state->optional_presence_count = 0;
    state->varg_ref_entries = NULL;
    state->varg_symbols = NULL;
    state->varg_count = 0;
    state->method_receiver_source_symbol = NULL;
    state->method_receiver_local_symbol = NULL;
    state->method_receiver_needs_copyback = 0;
    state->method_receiver_uses_locator_copyback = 0;
}
