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

#ifndef CREXX_RXCP_AST_REWRITE_H
#define CREXX_RXCP_AST_REWRITE_H

#include "rxcpmain.h"

/*
 * Declarative AST Rewriting Utility
 *
 * Provides a safer alternative to manual AST manipulation inside validation walkers.
 * It manages:
 * - Clean detachment and symbol-table decoupling (`sym_dno`) of reused nodes.
 * - Safe propagation of source location (line, column, token).
 */

typedef enum {
    RW_NEW_NODE,     /* Create a brand new node */
    RW_REUSE_NODE,   /* Safely detach and reuse an existing ASTNode */
    RW_CHILDREN      /* A structural wrapper for a list of child nodes */
} ASTRewriteAction;

typedef struct ASTRewriteTemplate {
    ASTRewriteAction action;
    NodeType node_type;         /* Used for RW_NEW_NODE */
    const char* string_val;     /* Used for RW_NEW_NODE */
    ASTNode* reused_node;       /* Used for RW_REUSE_NODE */
    struct ASTRewriteTemplate** children; 
    size_t num_children;
} ASTRewriteTemplate;

/* Builders */
ASTRewriteTemplate* ast_rw_new(NodeType type, const char* str);
ASTRewriteTemplate* ast_rw_reuse(ASTNode* node);
ASTRewriteTemplate* ast_rw_add(ASTRewriteTemplate* parent, ASTRewriteTemplate* child);
ASTRewriteTemplate* ast_rw_children(void);

/* Execution Engine */
/* Builds the new AST tree described by the template, correctly mapping source pointers 
 * from the target_to_replace or reused nodes.
 * Replaces the target_to_replace with the newly built tree and frees the old nodes.
 * Cleans up the template memory.
 * Returns the newly created node that took target_to_replace's place, or NULL on error.
 */
ASTNode* ast_execute_rewrite(Context *ctx, ASTNode *target_to_replace, ASTRewriteTemplate *template);

#endif //CREXX_RXCP_AST_REWRITE_H
