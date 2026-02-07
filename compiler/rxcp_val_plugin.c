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
#include "rxvml.h"

static rxvml_context* rxcp_get_rxvml_ctx(Context *context) {
    if (!context->rxvml_ctx) {
        /* Set debug mode for the bridge VM based on compiler debug level */
        int bridge_debug = (context->debug_mode >= 3) ? 1 : 0;
        context->rxvml_ctx = rxvml_create(NULL, bridge_debug);
        if (context->rxvml_ctx) {
            /* Try to load stub_plugin for now as per instructions/test.
             * Silence errors if not found to avoid polluting ctest output. */
            rxvml_load_module_file(context->rxvml_ctx, "stub_plugin");
        }
    }
    return (rxvml_context*)context->rxvml_ctx;
}

static ASTNode* get_instructions(ASTNode *temp_ast) {
    ASTNode *prog = ast_chld(temp_ast, PROGRAM_FILE, 0);
    if (prog) {
        ASTNode *main_proc = ast_chld(prog, PROCEDURE, 0);
        if (main_proc) {
            /* Find the INSTRUCTIONS node among the procedure's children */
            return ast_chld(main_proc, INSTRUCTIONS, 0);
        }
    }
    return NULL;
}

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
        } else if (node->node_type == IMPLICIT_CMD) {
            rxvml_context *rxctx = rxcp_get_rxvml_ctx(context);
            if (rxctx) {
                rxvml_value *tokens = rxcp_marshal_implicit_cmd(rxctx, node);
                rxvml_value *response = NULL;
                /* Call stub_plugin for now */
                if (rxvml_call_plugin(rxctx, "stub.stub_plugin", tokens, &response) == 0 && response) {
                    const char *code = NULL;
                    if (rxvml_to_str(rxctx, response, &code, NULL) == 0 && code) {
                        if (context->debug_mode >= 2) {
                            fprintf(stderr, "Plugin returned code: %s\n", code);
                        }
                        /* Prepend options levelb to the fragment to ensure keywords are recognized */
                        char *prefixed_code = malloc(strlen(code) + 32);
                        sprintf(prefixed_code, "options levelb\n%s", code);

                        Context *temp_ctx = rxcp_parse_buffer(prefixed_code, context->debug_mode);
                        free(prefixed_code);

                        if (temp_ctx && temp_ctx->ast) {
                            /* Run initial checks on the fragment to set up lines, columns, etc. */
                            ast_wlkr(temp_ctx->ast, initial_checks_walker, (void *) temp_ctx);

                            if (context->debug_mode >= 2) {
                                fprintf(stderr, "Plugin: Fragment AST after initial checks:\n");
                                rxcp_print_ast_recursive(temp_ctx->ast, 0);
                            }

                            ASTNode *instrs = get_instructions(temp_ctx->ast);
                            if (instrs) {
                                /* If the fragment resulted in a single instruction, graft it directly
                                 * instead of keeping the INSTRUCTIONS wrapper */
                                ASTNode *to_graft = instrs;
                                if (instrs->child && !instrs->child->sibling) {
                                    to_graft = instrs->child;
                                }

                                ASTNode *cloned = add_dast(node->parent, to_graft);
                                if (cloned) {
                                    if (context->debug_mode >= 2) {
                                        fprintf(stderr, "Plugin: Replacing node %p (type %d) with %p (type %d)\n",
                                                (void*)node, node->node_type, (void*)cloned, cloned->node_type);
                                    }
                                    ast_rpl(node, cloned);

                                    /* Hand over the temp_ctx to the master context to keep its buffer/tokens alive */
                                    Context *master = context->master_context ? context->master_context : context;
                                    master->fragment_contexts = realloc(master->fragment_contexts,
                                        sizeof(Context*) * (master->fragment_contexts_count + 1));
                                    master->fragment_contexts[master->fragment_contexts_count++] = temp_ctx;

                                    context->changed = 1;
                                } else {
                                    if (context->debug_mode >= 2) fprintf(stderr, "Plugin: Cloning failed\n");
                                    fre_cntx(temp_ctx);
                                }
                            } else {
                                if (context->debug_mode >= 2) fprintf(stderr, "Plugin: No instructions in fragment\n");
                                fre_cntx(temp_ctx);
                            }
                        } else {
                            if (context->debug_mode >= 2) fprintf(stderr, "Plugin: Parsing fragment failed\n");
                        }
                    }
                }
            }
        }
    }
    return result_normal;
}
