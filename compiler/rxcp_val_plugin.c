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
#include "rxcp_plugin.h"
#include "rxcp_ast.h"
#include "rxcp_sym.h"

walker_result plugin_dispatch_walker(walker_direction direction, ASTNode *node, void *payload) {
    Context *context = (Context *)payload;

    if (direction == in) {
        if (node->node_type == FUNCTION) {
            /* Check if symbol is resolved */
            Symbol *symbol = NULL;
            if (node->symbolNode && node->symbolNode->symbol) {
                symbol = node->symbolNode->symbol;
            }

            if (symbol && symbol->compiler_plugin) {
                PluginContext pctx;
                pctx.node = node;
                pctx.scope = node->scope ? node->scope : context->current_scope;
                pctx.context = context;
                pctx.iteration = context->iterations;

                PluginStatus status = symbol->compiler_plugin(&pctx, node);

                if (status == PLUGIN_DIRTY) {
                    context->changed = 1;
                } else if (status == PLUGIN_ERROR) {
                    mknd_err(node, "PLUGIN_ERROR");
                    return result_error;
                }
            }
        }
    }
    return result_normal;
}
