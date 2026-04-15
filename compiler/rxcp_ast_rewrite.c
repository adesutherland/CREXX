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

#include "rxcp_ast_rewrite.h"
#include <stdlib.h>
#include <string.h>

ASTRewriteTemplate* ast_rw_new(NodeType type, const char* str) {
    ASTRewriteTemplate* tmpl = malloc(sizeof(ASTRewriteTemplate));
    tmpl->action = RW_NEW_NODE;
    tmpl->node_type = type;
    tmpl->string_val = str ? strdup(str) : NULL;
    tmpl->reused_node = NULL;
    tmpl->source_loc_node = NULL;
    tmpl->children = NULL;
    tmpl->num_children = 0;
    return tmpl;
}

ASTRewriteTemplate* ast_rw_reuse(ASTNode* node) {
    if (!node) return NULL;
    ASTRewriteTemplate* tmpl = malloc(sizeof(ASTRewriteTemplate));
    tmpl->action = RW_REUSE_NODE;
    tmpl->reused_node = node;
    tmpl->source_loc_node = NULL;
    tmpl->string_val = NULL;
    tmpl->children = NULL;
    tmpl->num_children = 0;
    return tmpl;
}

ASTRewriteTemplate* ast_rw_add(ASTRewriteTemplate* parent, ASTRewriteTemplate* child) {
    if (!parent || !child) return parent;
    parent->num_children++;
    parent->children = realloc(parent->children, parent->num_children * sizeof(ASTRewriteTemplate*));
    parent->children[parent->num_children - 1] = child;
    return parent;
}

ASTRewriteTemplate* ast_rw_move_children(ASTRewriteTemplate* parent, ASTNode* source_node) {
    return ast_rw_move_children_replace(parent, source_node, NULL, NULL);
}

ASTRewriteTemplate* ast_rw_move_children_replace(ASTRewriteTemplate* parent, ASTNode* source_node, ASTNode* replace_child, ASTRewriteTemplate* replace_tmpl) {
    if (!parent || !source_node) return parent;
    ASTNode *c = source_node->child;
    while (c) {
        ASTNode *next = c->sibling;
        if (replace_child && c == replace_child) {
            if (replace_tmpl) ast_rw_add(parent, replace_tmpl);
        } else {
            /* Detach to prevent cyclic operations before reuse */
            c->sibling = NULL;
            c->parent = NULL;
            ast_rw_add(parent, ast_rw_reuse(c));
        }
        c = next;
    }
    return parent;
}

ASTRewriteTemplate* ast_rw_loc(ASTRewriteTemplate* tmpl, ASTNode* loc_node) {
    if (!tmpl) return NULL;
    tmpl->source_loc_node = loc_node;
    return tmpl;
}

ASTRewriteTemplate* ast_rw_children(void) {
    ASTRewriteTemplate* tmpl = malloc(sizeof(ASTRewriteTemplate));
    tmpl->action = RW_CHILDREN;
    tmpl->node_type = 0;
    tmpl->string_val = NULL;
    tmpl->reused_node = NULL;
    tmpl->source_loc_node = NULL;
    tmpl->children = NULL;
    tmpl->num_children = 0;
    return tmpl;
}

static void free_rw_template(ASTRewriteTemplate* tmpl) {
    if (!tmpl) return;
    size_t i;
    for (i = 0; i < tmpl->num_children; i++) {
        free_rw_template(tmpl->children[i]);
    }
    if (tmpl->children) free(tmpl->children);
    if (tmpl->string_val) free((void*)tmpl->string_val);
    free(tmpl);
}

/* Recursive builder */
static ASTNode* execute_rewrite_recursive(Context* ctx, ASTRewriteTemplate* tmpl, ASTNode* target_to_replace) {
    if (!tmpl) return NULL;
    ASTNode *result = NULL;
    size_t i;

    if (tmpl->action == RW_NEW_NODE) {
        result = ast_ft(ctx, tmpl->node_type);
        if (tmpl->string_val) {
            ast_sstr(result, strdup(tmpl->string_val), strlen(tmpl->string_val));
            result->free_node_string = 1;
        }
        /* Inherit source tracking from explicitly assigned loc_node or target */
        ASTNode* loc_node = tmpl->source_loc_node ? tmpl->source_loc_node : target_to_replace;
        if (loc_node) {
            result->file_name = loc_node->file_name;
            result->token = loc_node->token;
            result->line = loc_node->line;
            result->column = loc_node->column;
            result->token_start = loc_node->token_start;
            result->source_start = loc_node->source_start;
            result->source_end = loc_node->source_end;
            if (loc_node->source_node) {
                ast_set_primary_source_node(result, loc_node->source_node, AST_SOURCE_SYNTHETIC);
            }
        }
    } 
    else if (tmpl->action == RW_REUSE_NODE) {
        result = tmpl->reused_node;
        
        /* 1. Detach from old parent cleanly */
        if (result->parent) {
            ASTNode *p = result->parent->child;
            if (p == result) {
                result->parent->child = result->sibling;
            } else {
                while (p && p->sibling != result) p = p->sibling;
                if (p) p->sibling = result->sibling;
            }
            result->parent = NULL;
        }
        result->sibling = NULL;
        
        /* 2. Decouple Symbol - it will be re-resolved */
        if (result->symbolNode && result->symbolNode->symbol) {
            sym_dno(result->symbolNode->symbol, result);
        }
    }
    
    /* Build Children */
    ASTNode *first_child = NULL;
    ASTNode *last_child = NULL;

    for (i = 0; i < tmpl->num_children; i++) {
        ASTNode *child_node = execute_rewrite_recursive(ctx, tmpl->children[i], target_to_replace);
        if (child_node) {
            if (result) {
                add_ast(result, child_node);
            } else {
                /* If this is an RW_CHILDREN wrapper, link them as siblings */
                if (!first_child) {
                    first_child = child_node;
                    last_child = child_node;
                } else {
                    last_child->sibling = child_node;
                    child_node->parent = first_child->parent;
                    while (last_child->sibling) last_child = last_child->sibling;
                }
            }
        }
    }

    if (!result) result = first_child;
    return result;
}

ASTNode* ast_execute_rewrite(Context *ctx, ASTNode *target_to_replace, ASTRewriteTemplate *tmpl) {
    if (!target_to_replace || !tmpl) return NULL;
    
    /* Build new tree */
    ASTNode *new_root = execute_rewrite_recursive(ctx, tmpl, target_to_replace);
    
    if (new_root) {
        /* Physically swap in tree */
        ast_rpl(target_to_replace, new_root);

        /* Restore the sibling pointer on the replaced node so that ast_wlkr's
         * 'node = node->sibling' loop can continue walking the rest of the tree
         * if the walker handler returns result_normal. */
        target_to_replace->sibling = new_root->sibling;

        ctx->changed_flags |= FLAG_VAL_TRANS;
    }
    
    free_rw_template(tmpl);
    return new_root;
}
