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

#include "rxcpmain.h"
#include "rxcp_source_tree.h"

static void source_tree_clear_context_links(Context *context, ASTNode *node) {
    while (node) {
        if (node->source_node && node->source_node->context == context) {
            ast_set_primary_source_node(node, 0, AST_SOURCE_NONE);
        }
        if (node->child) source_tree_clear_context_links(context, node->child);
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

    source_node = source_node_f(context, node);
    ast_set_primary_source_node(node, source_node, AST_SOURCE_EXACT);

    last_child = 0;
    child = node->child;
    while (child) {
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

SourceNode *source_tree_build(Context *context, ASTNode *root) {
    if (!context || !root) return 0;

    source_tree_free(context);
    context->source_tree = source_tree_dup_node(context, root);
    return context->source_tree;
}

void source_tree_free(Context *context) {
    SourceNode *node;
    SourceNode *next;

    if (!context) return;

    if (context->ast) source_tree_clear_context_links(context, context->ast);
    if (context->work_ast && context->work_ast != context->ast) {
        source_tree_clear_context_links(context, context->work_ast);
    }

    node = context->source_free_list;
    while (node) {
        next = node->free_list;
        free(node);
        node = next;
    }

    context->source_tree = 0;
    context->source_free_list = 0;
}
