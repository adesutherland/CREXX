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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxcp_val.h"
#include "rxcp_ast.h"
#include "rxcp_sym.h"
#include "rxvml.h"

walker_result exit_dispatch_walker(walker_direction direction, ASTNode *node, void *payload) {
    Context *context = (Context *)payload;

    if (direction == in) {
        if (context->disable_exits) return result_normal;
        if (node->scope) {
            context->current_scope = node->scope;
        }
        if (node->node_type == COMPILER_ADDED_BLOCK) {
            return request_skip;
        }
        if (node->node_type == IMPLICIT_CMD || node->node_type == ADDRESS || node->node_type == EXIT_EXTENDED) {
            if (context->debug_mode >= 2) fprintf(stderr, "DEBUG_EXIT: exit_dispatch_walker hitting node type %d at line %d\n", node->node_type, node->line);
            int old_changed = context->changed_flags;
            int rc = rxcp_exit_bridge_invoke(context, node);
            if (rc < 0) {
                if (rc == -1) return request_skip; /* Node replaced */
                return result_error;
            }
            if (context->changed_flags > old_changed) {
                return request_skip;
            }
        }
    } else {
        if (node->scope && node->scope->parent) {
            context->current_scope = node->scope->parent;
        }
    }
    return result_normal;
}
