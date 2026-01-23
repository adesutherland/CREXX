/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2025 Adrian Sutherland, Peter Jacob, René Jansen
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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "rxcpmain.h"
#include "rxcpbgmr.h"
#include "rxcp_emit.h"

void emit_proc(ASTNode *node, void *pl) {
    walker_payload *payload = (walker_payload*) pl;
    ASTNode *child1, *child2, *child3, *n;
    char *temp1;
    char *comment_meta;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    child1 = node->child;
    if (child1) child2 = child1->sibling;
    else child2 = NULL;
    if (child2) child3 = child2->sibling;
    else child3 = NULL;

    switch (node->node_type) {
        case REXX_UNIVERSE:
        {
            char *buf = mprintf("/*\n"
                                " * cREXX COMPILER VERSION : %s\n"
                                " * SOURCE                 : %s\n"
                                " * BUILT                  : %d-%02d-%02d %02d:%02d:%02d\n"
                                " */\n"
                                "\n",
                                rxversion,
                                payload->context->file_name,
                                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

            if (node->output) output_prepend_text(buf, node->output);
            else node->output = output_fs(buf);
            free(buf);

            n = child1;
            while (n) {
                if (n->output) output_concat(node->output, n->output);
                if (n->cleanup) output_concat(node->output, n->cleanup);
                n = n->sibling;
            }

            print_output(payload->file, node->output);
        }
        break;

        case PROGRAM_FILE:
        {
            char *buf = mprintf(".srcfile=\"%s\"\n"
                                ".globals=%d\n",
                                payload->context->file_name,
                                payload->globals);

            if (node->output) output_prepend_text(buf, node->output);
            else node->output = output_fs(buf);
            free(buf);

            /* Add exposed global variables */
            add_exposed_global_variable(node);

            n = child1;
            while (n) {
                if (n->output) output_concat(node->output, n->output);
                if (n->cleanup) output_concat(node->output, n->cleanup);
                n = n->sibling;
            }
        }
        break;

        case PROCEDURE:
            if (ast_chld(node, INSTRUCTIONS, NOP)->node_type == NOP) {
                /* A declaration - external */
                char* type = ast_n2tp(ast_chld(node, CLASS, VOID));
                char* args = meta_narg(ast_chld(node, ARGS, 0));
                char* proc_symbol= sym_frnm(node->symbolNode->symbol);
                char* buf;
                if (node->symbolNode->symbol->exposed) {
                    buf = mprintf("\n%.*s() .expose=%s\n"
                                  "   .meta \"%s\"=\"b\" \"%s\" %.*s() \"%s\"\n",
                                  (int) node->node_string_length, node->node_string,
                                  proc_symbol, /* FQ Symbol Name */
                                  proc_symbol, /* FQ Symbol Name */
                                  type, /* Type */
                                  (int) node->node_string_length, node->node_string, /* Func Name */
                                  args /* Args */
                    );
                }
                else {
                    buf = mprintf("\n%.*s()\n"
                                  "   .meta \"%s\"=\"b\" \"%s\" %.*s() \"%s\"\n",
                                  (int) node->node_string_length, node->node_string,
                                  proc_symbol, /* FQ Symbol Name */
                                  type, /* Type */
                                  (int) node->node_string_length, node->node_string, /* Func Name */
                                  args /* Args */
                    );
                }
                if (node->output) output_prepend_text(buf, node->output);
                else node->output = output_fs(buf);
                free(type);
                free(args);
                free(buf);
                free(proc_symbol);
            }
            else {
                /* Definition */
                char* type = ast_n2tp(ast_chld(node, CLASS, VOID));
                char* args = meta_narg(ast_chld(node, ARGS, 0));
                char* proc_symbol= sym_frnm(node->symbolNode->symbol);
                char* buf;
                if (node->symbolNode->symbol->exposed) {
                    buf = mprintf("\n%.*s() .locals=%d .expose=%s\n"
                                  "   .meta \"%s\"=\"b\" \"%s\" %.*s() \"%s\" \"\"\n",
                                  (int) node->node_string_length, node->node_string, /* Function name */
                                  (int) node->scope->num_registers, /* Locals */
                                  proc_symbol, /* FQ Symbol name */
                                  proc_symbol, /* FQ Symbol Name */
                                  type, /* Return Type */
                                  (int) node->node_string_length, node->node_string, /* Function name */
                                  args /* Args */);
                }
                else {
                    buf = mprintf("\n%.*s() .locals=%d\n"
                                  "   .meta \"%s\"=\"b\" \"%s\" %.*s() \"%s\" \"\"\n",
                                  (int) node->node_string_length, node->node_string, /* Function name */
                                  (int) node->scope->num_registers, /* Locals */
                                  proc_symbol, /* FQ Symbol Name */
                                  type, /* Return Type */
                                  (int) node->node_string_length, node->node_string, /* Function name */
                                  args /* Args */);
                }
                if (node->output) output_prepend_text(buf, node->output);
                else node->output = output_fs(buf);
                free(type);
                free(args);
                free(buf);
                free(proc_symbol);

                /* Add source metadata */
                if (node->token) {
                    comment_meta = get_metaline_clause(node);
                    output_append_text(node->output, comment_meta);
                    free(comment_meta);
                }

                /* Add Global Variables */
                add_global_variable_metadata(node);

                /* If numeric options have non-inherited values, set them */
                if (node->scope->num_context.digits > -1) {
                    temp1 = mprintf("   setnumdgts %d\n", node->scope->num_context.digits);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
                if (node->scope->num_context.fuzz > -1) {
                    temp1 = mprintf("   setnumfuz %d\n", node->scope->num_context.fuzz);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
                if (node->scope->num_context.form > 0) {
                    /* 1 = SCIENTIFIC, 2 = ENGINEERING */
                    temp1 = mprintf("   setnumfrm %d\n", node->scope->num_context.form);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
                if (node->scope->num_context.casetype > 0) {
                    /* 1 = LOWER, 2 = UPPER */
                    temp1 = mprintf("   setnumcas %d\n", node->scope->num_context.casetype);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
                if (node->scope->num_context.standard > 0) {
                    /* 1 = COMMON, 2 = CLASSIC[REXX] */
                    temp1 = mprintf("   setnumstd %d\n", node->scope->num_context.standard);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }

                n = child2;
                while (n) {
                    if (n->output) output_concat(node->output, n->output);
                    if (n->cleanup) output_concat(node->output, n->cleanup);
                    n = n->sibling;
                }

                /* Clear all variable metadata */
                clear_variable_metadata(node);
                clear_global_variable_metadata(node);
            }
            break;
    }
}
