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
 * Compiler Plugin Interface
 */

#ifndef CREXX_RXCP_PLUGIN_H
#define CREXX_RXCP_PLUGIN_H

#include "rxcp_types.h"
#include "rxvml.h"

rxvml_value* rxcp_marshal_implicit_cmd(rxvml_context *ctx, ASTNode *cmd_node);

typedef enum PluginStatus {
    PLUGIN_CONTINUE,
    PLUGIN_DIRTY,
    PLUGIN_ERROR,
    PLUGIN_OK,
    PLUGIN_NEED_MORE
} PluginStatus;

typedef struct PluginContext {
    ASTNode *node;
    Scope *scope;
    Context *context;
    int iteration;
} PluginContext;

typedef PluginStatus (*PluginCallback)(PluginContext* pctx, ASTNode* node);

#endif //CREXX_RXCP_PLUGIN_H
