/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * Immutable Source Tree Construction
 */

#include <stdlib.h>
#include <string.h>

#include "rxcpmain.h"
#include "rxcp_source_tree.h"

static void source_tree_clear_context_links(Context *context, ASTNode *node) {
    while (node) {
        node->is_source_diagnostic_recorded = 0;
        if (node->source_node && node->source_node->context == context) {
            ast_set_primary_source_node(node, 0, AST_SOURCE_NONE);
        }
        ast_clear_reporting_source_nodes(node);
        if (node->child) source_tree_clear_context_links(context, node->child);
        node = node->sibling;
    }
}

static void source_tree_clear_recorded_flags(ASTNode *node) {
    while (node) {
        node->is_source_diagnostic_recorded = 0;
        if (node->child) source_tree_clear_recorded_flags(node->child);
        node = node->sibling;
    }
}

static void source_tree_clear_detached_diagnostic_flags(Context *context) {
    ASTNode *diag;

    if (!context) return;

    diag = (ASTNode *)context->diagnostics_list;
    while (diag) {
        diag->is_source_diagnostic_recorded = 0;
        diag = diag->sibling;
    }
}

static void source_tree_clear_diagnostic_links(SourceNode *node) {
    while (node) {
        node->diagnostics = 0;
        if (node->child) source_tree_clear_diagnostic_links(node->child);
        node = node->sibling;
    }
}

static void source_semantic_free(SourceSemanticInfo *semantics) {
    if (!semantics) return;

    if (semantics->value_dim_base) free(semantics->value_dim_base);
    if (semantics->value_dim_elements) free(semantics->value_dim_elements);
    if (semantics->value_class) free(semantics->value_class);
    if (semantics->target_dim_base) free(semantics->target_dim_base);
    if (semantics->target_dim_elements) free(semantics->target_dim_elements);
    if (semantics->target_class) free(semantics->target_class);
    free(semantics);
}

static void source_tree_clear_semantic_links(SourceNode *node) {
    while (node) {
        if (node->semantics) {
            source_semantic_free(node->semantics);
            node->semantics = 0;
        }
        if (node->child) source_tree_clear_semantic_links(node->child);
        node = node->sibling;
    }
}

static SourceSemanticInfo *source_semantic_f(SourceNode *owner) {
    SourceSemanticInfo *semantics;

    if (!owner) return 0;
    if (owner->semantics) return owner->semantics;

    semantics = calloc(1, sizeof(SourceSemanticInfo));
    if (!semantics) return 0;

    semantics->owner = owner;
    semantics->symbol_type = UNKNOWN_SYMBOL;
    semantics->symbol_status = SYM_STATUS_UNRESOLVED;
    semantics->value_type = TP_UNKNOWN;
    semantics->target_type = TP_UNKNOWN;
    owner->semantics = semantics;
    return semantics;
}

static int source_semantic_type_specificity(ValueType type) {
    switch (type) {
        case TP_UNKNOWN: return 0;
        case TP_VOID: return 1;
        case TP_STRING: return 2;
        default: return 3;
    }
}

static void source_semantic_copy_dims(size_t dims,
                                      int **dest_base,
                                      int **dest_elements,
                                      int *src_base,
                                      int *src_elements) {
    size_t i;

    if (!dest_base || !dest_elements) return;

    if (*dest_base) free(*dest_base);
    if (*dest_elements) free(*dest_elements);
    *dest_base = 0;
    *dest_elements = 0;

    if (dims == 0) return;

    *dest_base = malloc(sizeof(int) * dims);
    *dest_elements = malloc(sizeof(int) * dims);
    if (!*dest_base || !*dest_elements) {
        if (*dest_base) free(*dest_base);
        if (*dest_elements) free(*dest_elements);
        *dest_base = 0;
        *dest_elements = 0;
        return;
    }

    if (src_base) memcpy(*dest_base, src_base, sizeof(int) * dims);
    else for (i = 0; i < dims; i++) (*dest_base)[i] = 1;
    if (src_elements) memcpy(*dest_elements, src_elements, sizeof(int) * dims);
    else for (i = 0; i < dims; i++) (*dest_elements)[i] = 0;
}

static void source_semantic_replace_type(ValueType *dest_type,
                                         size_t *dest_dims,
                                         int **dest_base,
                                         int **dest_elements,
                                         char **dest_class,
                                         ValueType type,
                                         size_t dims,
                                         int *dim_base,
                                         int *dim_elements,
                                         const char *class_name) {
    if (!dest_type || !dest_dims || !dest_base || !dest_elements || !dest_class) return;

    *dest_type = type;
    *dest_dims = dims;
    source_semantic_copy_dims(dims, dest_base, dest_elements, dim_base, dim_elements);

    if (*dest_class) {
        free(*dest_class);
        *dest_class = 0;
    }
    if (class_name) *dest_class = strdup(class_name);
}

static void source_semantic_promote_type(ValueType *dest_type,
                                         size_t *dest_dims,
                                         int **dest_base,
                                         int **dest_elements,
                                         char **dest_class,
                                         ValueType type,
                                         size_t dims,
                                         int *dim_base,
                                         int *dim_elements,
                                         const char *class_name) {
    int current_spec;
    int new_spec;
    int promote;

    if (!dest_type || !dest_dims || !dest_base || !dest_elements || !dest_class) return;

    current_spec = source_semantic_type_specificity(*dest_type);
    new_spec = source_semantic_type_specificity(type);
    promote = 0;

    if (new_spec > current_spec) promote = 1;
    else if (new_spec == current_spec && type == *dest_type) {
        if (dims > *dest_dims) promote = 1;
        else if (dims == *dest_dims && class_name && !*dest_class) promote = 1;
    }

    if (promote) {
        source_semantic_replace_type(dest_type,
                                     dest_dims,
                                     dest_base,
                                     dest_elements,
                                     dest_class,
                                     type,
                                     dims,
                                     dim_base,
                                     dim_elements,
                                     class_name);
    }
}

static SourceSemanticKind source_semantic_kind_from_symbol_type(SymbolType symbol_type) {
    switch (symbol_type) {
        case VARIABLE_SYMBOL: return SOURCE_SEMANTIC_VARIABLE;
        case FUNCTION_SYMBOL: return SOURCE_SEMANTIC_FUNCTION;
        case CLASS_SYMBOL: return SOURCE_SEMANTIC_TYPE;
        case CONSTANT_SYMBOL: return SOURCE_SEMANTIC_CONSTANT;
        case NAMESPACE_SYMBOL: return SOURCE_SEMANTIC_NAMESPACE;
        default: return SOURCE_SEMANTIC_NONE;
    }
}

static SourceSemanticKind source_semantic_kind_from_node(ASTNode *node) {
    if (!node) return SOURCE_SEMANTIC_NONE;

    switch (node->node_type) {
        case PROCEDURE:
        case METHOD:
        case FACTORY:
        case FUNCTION:
        case FUNC_SYMBOL:
        case CALL:
        case MEMBER_CALL:
        case FACTORY_CALL:
            return SOURCE_SEMANTIC_FUNCTION;
        case CLASS:
        case CLASS_DEF:
            return SOURCE_SEMANTIC_TYPE;
        case CONST_SYMBOL:
            return SOURCE_SEMANTIC_CONSTANT;
        case NAMESPACE:
            return SOURCE_SEMANTIC_NAMESPACE;
        case VAR_SYMBOL:
        case VAR_TARGET:
        case VAR_REFERENCE:
            return SOURCE_SEMANTIC_VARIABLE;
        default:
            return SOURCE_SEMANTIC_NONE;
    }
}

typedef struct SourceSymbolIdMapEntry {
    Symbol *symbol;
    int identifier_id;
} SourceSymbolIdMapEntry;

typedef struct SourceSemanticSyncState {
    SourceSymbolIdMapEntry *entries;
    size_t count;
    size_t capacity;
    int next_identifier_id;
} SourceSemanticSyncState;

static int source_semantic_identifier_id(SourceSemanticSyncState *state, Symbol *symbol) {
    SourceSymbolIdMapEntry *new_entries;
    size_t new_capacity;
    size_t i;

    if (!state || !symbol) return 0;

    for (i = 0; i < state->count; i++) {
        if (state->entries[i].symbol == symbol) return state->entries[i].identifier_id;
    }

    if (state->count == state->capacity) {
        new_capacity = state->capacity ? state->capacity * 2 : 16;
        new_entries = realloc(state->entries, sizeof(SourceSymbolIdMapEntry) * new_capacity);
        if (!new_entries) return 0;
        state->entries = new_entries;
        state->capacity = new_capacity;
    }

    state->entries[state->count].symbol = symbol;
    state->entries[state->count].identifier_id = state->next_identifier_id++;
    state->count++;
    return state->entries[state->count - 1].identifier_id;
}

static void source_tree_sync_semantics_from_ast(SourceSemanticSyncState *state, ASTNode *node) {
    ASTNode *current;

    current = node;
    while (current) {
        SourceNode *owner = current->source_node;
        SourceSemanticKind fallback_kind = source_semantic_kind_from_node(current);
        int needs_semantics = 0;
        SourceSemanticInfo *semantics = 0;

        if (owner) {
            needs_semantics = current->symbolNode != 0 ||
                              current->value_type != TP_UNKNOWN ||
                              current->value_dims > 0 ||
                              current->value_class != 0 ||
                              current->target_type != TP_UNKNOWN ||
                              current->target_dims > 0 ||
                              current->target_class != 0 ||
                              current->is_ref_arg ||
                              current->is_opt_arg ||
                              current->is_const_arg ||
                              current->is_varg ||
                              fallback_kind != SOURCE_SEMANTIC_NONE;
        }

        if (needs_semantics) semantics = source_semantic_f(owner);

        if (semantics) {
            Symbol *symbol = current->symbolNode ? current->symbolNode->symbol : 0;
            SourceSemanticKind kind = SOURCE_SEMANTIC_NONE;

            if (current->value_type != TP_UNKNOWN || current->value_dims > 0 || current->value_class) {
                source_semantic_promote_type(&semantics->value_type,
                                             &semantics->value_dims,
                                             &semantics->value_dim_base,
                                             &semantics->value_dim_elements,
                                             &semantics->value_class,
                                             current->value_type,
                                             current->value_dims,
                                             current->value_dim_base,
                                             current->value_dim_elements,
                                             current->value_class);
            }

            if (current->target_type != TP_UNKNOWN || current->target_dims > 0 || current->target_class) {
                source_semantic_promote_type(&semantics->target_type,
                                             &semantics->target_dims,
                                             &semantics->target_dim_base,
                                             &semantics->target_dim_elements,
                                             &semantics->target_class,
                                             current->target_type,
                                             current->target_dims,
                                             current->target_dim_base,
                                             current->target_dim_elements,
                                             current->target_class);
            }

            semantics->is_ref_arg |= current->is_ref_arg;
            semantics->is_opt_arg |= current->is_opt_arg;
            semantics->is_const_arg |= current->is_const_arg;
            semantics->is_varg |= current->is_varg;

            if (symbol) {
                if (symbol->type != TP_UNKNOWN || symbol->value_dims > 0 || symbol->value_class) {
                    source_semantic_promote_type(&semantics->value_type,
                                                 &semantics->value_dims,
                                                 &semantics->value_dim_base,
                                                 &semantics->value_dim_elements,
                                                 &semantics->value_class,
                                                 symbol->type,
                                                 symbol->value_dims,
                                                 symbol->dim_base,
                                                 symbol->dim_elements,
                                                 symbol->value_class);
                }

                if (symbol->symbol_type != UNKNOWN_SYMBOL) semantics->symbol_type = symbol->symbol_type;
                if (symbol->status != SYM_STATUS_UNRESOLVED) semantics->symbol_status = symbol->status;
                if (current->symbolNode) {
                    semantics->read_usage |= current->symbolNode->readUsage;
                    semantics->write_usage |= current->symbolNode->writeUsage;
                }
                semantics->is_definition |= symbol->creation_node == current;
                semantics->is_global_var |= symbol->is_global_var;
                semantics->is_this |= symbol->is_this;
                semantics->is_factory |= symbol->is_factory;
                semantics->identifier_id = source_semantic_identifier_id(state, symbol);
                kind = source_semantic_kind_from_symbol_type(symbol->symbol_type);
            }

            if (kind == SOURCE_SEMANTIC_NONE) kind = fallback_kind;
            if (kind != SOURCE_SEMANTIC_NONE) semantics->identifier_kind = kind;

            if (!semantics->is_definition) {
                switch (current->node_type) {
                    case PROCEDURE:
                    case METHOD:
                    case FACTORY:
                    case CLASS_DEF:
                    case NAMESPACE:
                    case DEFINE:
                        semantics->is_definition = 1;
                        break;
                    default:
                        break;
                }
            }
        }

        if (current->child) source_tree_sync_semantics_from_ast(state, current->child);
        current = current->sibling;
    }
}

static SourceNode *source_node_f(Context *context, ASTNode *node) {
    SourceNode *source_node;

    source_node = calloc(1, sizeof(SourceNode));
    source_node->context = context;
    source_node->node_type = node->node_type;
    source_node->file_name = node->file_name;
    source_node->token = node->token;
    source_node->token_start = node->token_start;
    source_node->token_end = node->token_end;
    source_node->node_string = node->node_string;
    source_node->node_string_length = node->node_string_length;
    source_node->source_start = node->source_start;
    source_node->source_end = node->source_end;
    source_node->line = node->line;
    source_node->column = node->column;
    source_node->free_list = context->source_free_list;
    if (source_node->free_list) source_node->node_number = source_node->free_list->node_number + 1;
    else source_node->node_number = 1;
    context->source_free_list = source_node;

    return source_node;
}

static SourceNode *source_tree_dup_node(Context *context, ASTNode *node) {
    SourceNode *source_node;
    ASTNode *child;
    SourceNode *last_child;
    SourceNode *source_child;

    if (!node) return 0;
    if (node->node_type == ERROR || node->node_type == WARNING) return 0;

    source_node = source_node_f(context, node);
    ast_set_primary_source_node(node, source_node, AST_SOURCE_EXACT);

    last_child = 0;
    child = node->child;
    while (child) {
        if (child->node_type == ERROR || child->node_type == WARNING) {
            ast_set_primary_source_node(child, source_node, AST_SOURCE_INHERITED);
            source_tree_record_diagnostic(context, child);
            child = child->sibling;
            continue;
        }
        source_child = source_tree_dup_node(context, child);
        if (source_child) {
            source_child->parent = source_node;
            if (last_child) last_child->sibling = source_child;
            else source_node->child = source_child;
            last_child = source_child;
        }
        child = child->sibling;
    }

    return source_node;
}

static SourceNode *source_tree_resolve_owner(Context *context, ASTNode *diag) {
    ASTNode *current;

    current = diag;
    while (current) {
        if (current->source_node) return current->source_node;
        current = current->parent;
    }

    return context ? context->source_tree : 0;
}

static void source_tree_append_diagnostic(Context *context, ASTNode *diag) {
    SourceDiagnostic *source_diag;
    SourceDiagnostic *tail;
    SourceNode *owner;
    const char *message;
    int line;
    int column;
    SourceDiagnosticSeverity severity;
    SourceDiagnostic *existing;

    if (!context || !diag) return;
    if (diag->node_type != ERROR && diag->node_type != WARNING) return;
    if (diag->is_duplicate_warning) return;
    if (diag->is_source_diagnostic_recorded) return;

    owner = source_tree_resolve_owner(context, diag);
    if (!owner) return;

    message = diag->node_string ? diag->node_string : "Syntax Error";
    line = diag->line >= 0 ? diag->line : owner->line;
    column = diag->column >= 0 ? diag->column : owner->column;
    severity = diag->node_type == WARNING ? SOURCE_DIAG_WARNING : SOURCE_DIAG_ERROR;

    existing = owner->diagnostics;
    while (existing) {
        if (existing->severity == severity &&
            existing->is_internal == diag->is_internal_diagnostic &&
            existing->line == line &&
            existing->column == column &&
            existing->message &&
            strcmp(existing->message, message) == 0) {
            diag->is_source_diagnostic_recorded = 1;
            return;
        }
        existing = existing->next_on_source;
    }

    source_diag = calloc(1, sizeof(SourceDiagnostic));
    if (!source_diag) return;

    source_diag->owner = owner;
    source_diag->message = strdup(message);
    source_diag->message_length = strlen(message);
    source_diag->file_name = diag->file_name ? diag->file_name : owner->file_name;
    source_diag->source_start = diag->source_start ? diag->source_start : owner->source_start;
    source_diag->source_end = diag->source_end ? diag->source_end : owner->source_end;
    source_diag->line = line;
    source_diag->column = column;
    source_diag->severity = severity;
    source_diag->is_internal = diag->is_internal_diagnostic;
    diag->is_source_diagnostic_recorded = 1;

    source_diag->next_on_source = owner->diagnostics;
    owner->diagnostics = source_diag;

    if (!context->source_diagnostics_list) {
        context->source_diagnostics_list = source_diag;
        context->source_diagnostics_free_list = source_diag;
        return;
    }

    tail = context->source_diagnostics_list;
    while (tail->next_in_context) tail = tail->next_in_context;
    tail->next_in_context = source_diag;
}

static void source_tree_collect_detached_diagnostics(Context *context) {
    ASTNode *diag;

    diag = (ASTNode *)context->diagnostics_list;
    while (diag) {
        source_tree_append_diagnostic(context, diag);
        diag = diag->sibling;
    }
}

static void source_tree_collect_ast_diagnostics(Context *context, ASTNode *node) {
    ASTNode *current;

    current = node;
    while (current) {
        if (current->node_type == ERROR || current->node_type == WARNING) {
            source_tree_append_diagnostic(context, current);
        }
        if (current->child) source_tree_collect_ast_diagnostics(context, current->child);
        current = current->sibling;
    }
}

static int source_tree_has_diagnostics(SourceNode *node) {
    while (node) {
        if (node->diagnostics) return 1;
        if (node->child && source_tree_has_diagnostics(node->child)) return 1;
        node = node->sibling;
    }
    return 0;
}

static void source_tree_rebuild_context_diagnostics(Context *context,
                                                    SourceNode *node,
                                                    SourceDiagnostic **tail) {
    SourceDiagnostic *diag;

    while (node) {
        diag = node->diagnostics;
        while (diag) {
            diag->next_in_context = 0;
            if (!context->source_diagnostics_list) context->source_diagnostics_list = diag;
            else if (*tail) (*tail)->next_in_context = diag;
            *tail = diag;
            diag = diag->next_on_source;
        }

        if (node->child) source_tree_rebuild_context_diagnostics(context, node->child, tail);
        node = node->sibling;
    }
}

SourceNode *source_tree_build(Context *context, ASTNode *root) {
    if (!context || !root) return 0;

    source_tree_free(context);
    context->source_tree = source_tree_dup_node(context, root);
    return context->source_tree;
}

void source_tree_clear_diagnostics(Context *context) {
    SourceDiagnostic *diag;
    SourceDiagnostic *next;

    if (!context) return;

    if (context->ast) source_tree_clear_recorded_flags(context->ast);
    if (context->work_ast && context->work_ast != context->ast) {
        source_tree_clear_recorded_flags(context->work_ast);
    }
    source_tree_clear_detached_diagnostic_flags(context);

    if (context->source_tree) source_tree_clear_diagnostic_links(context->source_tree);

    diag = context->source_diagnostics_free_list;
    while (diag) {
        next = diag->next_in_context;
        if (diag->message) free(diag->message);
        free(diag);
        diag = next;
    }

    context->source_diagnostics_list = 0;
    context->source_diagnostics_free_list = 0;
}

void source_tree_clear_semantics(Context *context) {
    if (!context || !context->source_tree) return;
    source_tree_clear_semantic_links(context->source_tree);
}

void source_tree_record_diagnostic(Context *context, ASTNode *diag) {
    if (!context || !context->source_tree) return;
    source_tree_append_diagnostic(context, diag);
}

void source_tree_sync_diagnostics(Context *context) {
    SourceDiagnostic *tail;

    if (!context || !context->source_tree) return;

    source_tree_clear_diagnostics(context);

    if (context->ast) source_tree_collect_ast_diagnostics(context, context->ast);
    if (context->diagnostics_list && !source_tree_has_diagnostics(context->source_tree)) {
        source_tree_collect_detached_diagnostics(context);
    }

    context->source_diagnostics_list = 0;
    tail = 0;
    source_tree_rebuild_context_diagnostics(context, context->source_tree, &tail);

    context->source_diagnostics_free_list = context->source_diagnostics_list;
}

void source_tree_sync_semantics(Context *context) {
    SourceSemanticSyncState state;
    ASTNode *root;

    if (!context || !context->source_tree) return;

    source_tree_clear_semantics(context);

    memset(&state, 0, sizeof(state));
    state.next_identifier_id = 1;

    root = context->work_ast ? context->work_ast : context->ast;
    if (root) source_tree_sync_semantics_from_ast(&state, root);

    if (state.entries) free(state.entries);
}

void source_tree_free(Context *context) {
    SourceNode *node;
    SourceNode *next;

    if (!context) return;

    source_tree_clear_diagnostics(context);
    source_tree_clear_semantics(context);

    if (context->ast) source_tree_clear_context_links(context, context->ast);
    if (context->work_ast && context->work_ast != context->ast) {
        source_tree_clear_context_links(context, context->work_ast);
    }
    source_tree_clear_detached_diagnostic_flags(context);

    node = context->source_free_list;
    while (node) {
        next = node->free_list;
        free(node);
        node = next;
    }

    context->source_tree = 0;
    context->source_free_list = 0;
}
