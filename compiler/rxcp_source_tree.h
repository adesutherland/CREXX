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
 * Immutable Source Tree Definitions
 */

#ifndef CREXX_RXCP_SOURCE_TREE_H
#define CREXX_RXCP_SOURCE_TREE_H

#include "rxcp_types.h"
#include "rxcp_token.h"

typedef enum SourceDiagnosticSeverity {
    SOURCE_DIAG_ERROR = 0,
    SOURCE_DIAG_WARNING = 1
} SourceDiagnosticSeverity;

struct SourceDiagnostic {
    SourceDiagnostic *next_in_context;
    SourceDiagnostic *next_on_source;
    SourceNode *owner;
    char *message;
    size_t message_length;
    char *file_name;
    char *source_start;
    char *source_end;
    int line;
    int column;
    SourceDiagnosticSeverity severity;
    char is_internal;
};

struct SourceNode {
    Context *context;
    int node_number;
    NodeType node_type;
    char *file_name;
    Token *token;
    Token *token_start;
    Token *token_end;
    char *node_string;
    size_t node_string_length;
    char *source_start;
    char *source_end;
    int line;
    int column;
    SourceNode *parent;
    SourceNode *child;
    SourceNode *sibling;
    SourceDiagnostic *diagnostics;
    SourceNode *free_list;
};

SourceNode *source_tree_build(Context *context, ASTNode *root);
void source_tree_clear_diagnostics(Context *context);
void source_tree_record_diagnostic(Context *context, ASTNode *diag);
void source_tree_sync_diagnostics(Context *context);
void source_tree_free(Context *context);

#endif
