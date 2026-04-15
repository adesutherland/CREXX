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

void source_tree_free(Context *context) {
    SourceNode *node;
    SourceNode *next;

    if (!context) return;

    source_tree_clear_diagnostics(context);

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
