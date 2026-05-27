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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "rxcpmain.h"
#include "rxcpbgmr.h"
#include "rxcp_emit.h"
#include "rxcp_util.h"

static int flow_needs_attr_copy(ASTNode *node) {
    if (!node) return 0;

    if (node->value_dims || node->target_dims) return 1;

    return node->value_type == TP_STRING ||
           node->value_type == TP_OBJECT ||
           node->value_type == TP_BINARY ||
           node->target_type == TP_STRING ||
           node->target_type == TP_OBJECT ||
           node->target_type == TP_BINARY;
}

static const char *signal_block_catch_all_names[] = {
        "FAILURE", "ERROR", "OVERFLOW_UNDERFLOW", "DIVISION_BY_ZERO",
        "CONVERSION_ERROR", "INVALID_ARGUMENTS", "OUT_OF_RANGE", "UNICODE_ERROR",
        "UNKNOWN_INSTRUCTION", "FUNCTION_NOT_FOUND", "NOT_IMPLEMENTED",
        "INVALID_SIGNAL_CODE", "NOTREADY", "QUIT", "TERM", "POSIX_INT",
        "POSIX_HUP", "POSIX_USR1", "POSIX_USR2", "POSIX_CHLD", "OTHER", 0
};

typedef struct signal_emit_names {
    char **items;
    size_t count;
    size_t capacity;
} signal_emit_names;

static void signal_emit_names_add(signal_emit_names *names, const char *name) {
    if (!names || !name) return;
    if (names->count == names->capacity) {
        size_t new_capacity = names->capacity ? names->capacity * 2 : 8;
        char **new_items = realloc(names->items, sizeof(char*) * new_capacity);
        if (!new_items) return;
        names->items = new_items;
        names->capacity = new_capacity;
    }
    names->items[names->count++] = strdup(name);
}

static void signal_emit_names_free(signal_emit_names *names) {
    size_t i;
    if (!names) return;
    for (i = 0; i < names->count; i++) free(names->items[i]);
    free(names->items);
    names->items = 0;
    names->count = 0;
    names->capacity = 0;
}

static char *signal_emit_canonical_name(ASTNode *node) {
    char *name;
    size_t i;

    if (!node || !node->node_string || node->node_string_length == 0) return 0;
    name = malloc(node->node_string_length + 1);
    if (!name) return 0;
    for (i = 0; i < node->node_string_length; i++) {
        name[i] = (char)toupper((unsigned char)node->node_string[i]);
    }
    name[node->node_string_length] = 0;
    if (strcmp(name, "SYNTAX") == 0) {
        free(name);
        return strdup("ERROR");
    }
    return name;
}

static void signal_emit_collect_names(ASTNode *names_node, signal_emit_names *names) {
    ASTNode *name_node;
    const char **catch_all;

    if (!names_node || !names) return;
    if (!names_node->child) {
        for (catch_all = signal_block_catch_all_names; *catch_all; catch_all++) {
            signal_emit_names_add(names, *catch_all);
        }
        return;
    }

    name_node = names_node->child;
    while (name_node) {
        char *name = signal_emit_canonical_name(name_node);
        if (name) {
            signal_emit_names_add(names, name);
            free(name);
        }
        name_node = ast_nsib(name_node);
    }
}

static ASTNode *signal_handler_binding_target(ASTNode *handler) {
    ASTNode *binding;

    if (!handler) return 0;
    binding = ast_chdn(handler, 1);
    if (!binding || binding->node_type != DEFINE) return 0;
    return binding->child;
}

static void signal_emit_pop_names(OutputFragment *output, signal_emit_names *installed) {
    size_t i;
    char *line;

    if (!output || !installed) return;
    for (i = installed->count; i > 0; i--) {
        line = mprintf("   sigpop \"%s\"\n", installed->items[i - 1]);
        output_append_text(output, line);
        free(line);
    }
}

void emit_flow(ASTNode *node, void *pl) {
    walker_payload *payload = (walker_payload*) pl;
    ASTNode *child1, *child2, *child3, *n;
    char *tp_prefix;
    char *temp1;
    char *temp2;
    char *comment_meta;
    char *op;
    int j;
    unsigned int trace_step_id;
    unsigned int trace_clause_id;

    child1 = node->child;
    if (child1) child2 = child1->sibling;
    else child2 = NULL;
    if (child2) child3 = child2->sibling;
    else child3 = NULL;

    if (node->value_dims) tp_prefix = "";
    else tp_prefix = type_to_prefix(node->value_type);

    if (tp_prefix[0] == 's' && (node->node_type == TO || node->node_type == BY)) tp_prefix = "f";

    switch (node->node_type) {

        case ARGS:
            if (!node->output) node->output = output_f();
            n = child1;
            while (n) {
                if (n->output) output_concat(node->output, n->output);
                if (n->cleanup) output_concat(node->output, n->cleanup);
                n = ast_nsib(n);
            }
            break;

        case INSTRUCTIONS:
            if (!node->output) node->output = output_f();
            n = child1;
            while (n) {
                if (n->output) output_concat(node->output, n->output);
                if (n->cleanup) output_concat(node->output, n->cleanup);
                n = ast_nsib(n);
            }
            comment_meta = get_reporting_metalines(node);
            if (comment_meta[0]) output_prepend_text(comment_meta, node->output);
            free(comment_meta);
            break;

        case SIGNAL_BLOCK: {
            signal_emit_names installed = {0};
            ASTNode *handler;
            size_t handler_index;

            if (!node->output) node->output = output_f();

            comment_meta = get_metaline_token_at(node);
            output_append_text(node->output, comment_meta);
            free(comment_meta);

            handler = child2;
            handler_index = 0;
            while (handler) {
                signal_emit_names handler_names = {0};
                ASTNode *binding = signal_handler_binding_target(handler);

                signal_emit_collect_names(handler->child, &handler_names);
                for (j = 0; j < (int)handler_names.count; j++) {
                    int binding_register = binding ? binding->register_num : UNSET_REGISTER;
                    char binding_register_type = binding ? binding->register_type : 'r';

                    if (binding && binding->symbolNode && binding->symbolNode->symbol) {
                        binding_register = binding->symbolNode->symbol->register_num;
                        binding_register_type = binding->symbolNode->symbol->register_type;
                    }

                    temp1 = mprintf("   sigpush \"%s\"\n", handler_names.items[j]);
                    output_append_text(node->output, temp1);
                    free(temp1);
                    signal_emit_names_add(&installed, handler_names.items[j]);

                    if (binding) {
                        temp1 = mprintf("   sigbrv l%dsignalhandler%zu,%c%d,\"%s\"\n",
                                        node->node_number,
                                        handler_index,
                                        binding_register_type,
                                        binding_register,
                                        handler_names.items[j]);
                    } else {
                        temp1 = mprintf("   sigbr l%dsignalhandler%zu,\"%s\"\n",
                                        node->node_number,
                                        handler_index,
                                        handler_names.items[j]);
                    }
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
                signal_emit_names_free(&handler_names);
                handler = ast_nsib(handler);
                handler_index++;
            }

            if (child1 && child1->output) output_concat(node->output, child1->output);
            if (child1 && child1->cleanup) output_concat(node->output, child1->cleanup);

            signal_emit_pop_names(node->output, &installed);
            temp1 = mprintf("   br l%dsignalend\n", node->node_number);
            output_append_text(node->output, temp1);
            free(temp1);

            handler = child2;
            handler_index = 0;
            while (handler) {
                ASTNode *handler_body = ast_chdn(handler, 2);

                temp1 = mprintf("l%dsignalhandler%zu:\n", node->node_number, handler_index);
                output_append_text(node->output, temp1);
                free(temp1);

                signal_emit_pop_names(node->output, &installed);
                if (handler_body && handler_body->output) output_concat(node->output, handler_body->output);
                if (handler_body && handler_body->cleanup) output_concat(node->output, handler_body->cleanup);

                temp1 = mprintf("   br l%dsignalend\n", node->node_number);
                output_append_text(node->output, temp1);
                free(temp1);

                handler = ast_nsib(handler);
                handler_index++;
            }

            temp1 = mprintf("l%dsignalend:\n", node->node_number);
            output_append_text(node->output, temp1);
            free(temp1);
            signal_emit_names_free(&installed);
            break;
        }

        case NOP:
            if (!node->output) node->output = output_f();
            break;

        case SAY:
            /* Add source metadata */
            comment_meta = get_metaline(node);
            trace_step_id = trace_source_step_id_from_metaline(comment_meta);
            trace_clause_id = trace_clause_id_from_metaline(comment_meta);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            /* Add Variable Metadata */
            add_variable_metadata(node);

            if (child1->register_num == DONT_ASSIGN_REGISTER) {
                /* If the register is not set then the child is a constant
                 * which we SAY directly. Get the constant string - target type */
                temp2 = format_constant(child1->target_type, child1);
                temp1 = mprintf("   say %s\n", temp2);
                free(temp2);
            }
            else {
                output_concat(node->output, child1->output);
                output_apply_trace_source_ids(node->output, trace_step_id, trace_clause_id);
                temp1 = mprintf("   say %c%d\n",
                                child1->register_type,
                                child1->register_num);
            }
            output_append_text(node->output, temp1);
            free(temp1);

            /* Cleanup child */
            if (child1->cleanup) output_concat(node->output, child1->cleanup);
            break;

        case RETURN:
            /* Add source metadata */
            comment_meta = get_metaline(node);
            trace_step_id = trace_source_step_id_from_metaline(comment_meta);
            trace_clause_id = trace_clause_id_from_metaline(comment_meta);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            /* Add Variable Metadata */
            add_variable_metadata(node);

            if (child1 == 0) {
                temp1 = mprintf("   ret\n");
            }
            else if (child1->register_num == DONT_ASSIGN_REGISTER) {
                /* If the register is not set then the child is a constant
                 * which we RET directly. Get the constant string - target type */
                temp2 = format_constant(child1->target_type, child1);
                temp1 = mprintf("   ret %s\n", temp2);
                free(temp2);
            }
            else {
                output_concat(node->output, child1->output);
                output_apply_trace_source_ids(node->output, trace_step_id, trace_clause_id);
                temp1 = mprintf("   ret %c%d\n",
                                child1->register_type,
                                child1->register_num);
                // TODO - Test array element as we have not unlinked
            }
            output_append_text(node->output, temp1);
            free(temp1);
            break;

        case IF:
            /* Add source metadata */
            comment_meta = get_metaline_range(node, child1);
            trace_step_id = trace_source_step_id_from_metaline(comment_meta);
            trace_clause_id = trace_clause_id_from_metaline(comment_meta);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            if (child1->output) output_concat(node->output, child1->output);
            output_apply_trace_source_ids(node->output, trace_step_id, trace_clause_id);
            comment_meta = get_metaline_token_after(child1);
            temp1 = mprintf("   brf l%diffalse,%c%d\n%s",
                            node->node_number,
                            node->register_type,
                            node->register_num,
                            comment_meta);
            output_append_text(node->output, temp1);
            free(temp1);
            free(comment_meta);
            output_concat(node->output, child2->output);
            if (child3) {
                comment_meta = get_metaline_token_after(child2);
                temp1 = mprintf("   br l%difend\n%sl%diffalse:\n",
                                node->node_number,
                                comment_meta,
                                node->node_number);
                output_append_text(node->output, temp1);
                free(temp1);
                free(comment_meta);
                output_concat(node->output, child3->output);

                temp1 = mprintf("l%difend:\n",
                                node->node_number);
                output_append_text(node->output, temp1);
                free(temp1);
                if (child1->cleanup) output_concat(node->output, child1->cleanup);
                if (child2->cleanup) output_concat(node->output, child2->cleanup);
                if (child3->cleanup) output_concat(node->output, child3->cleanup);
            }
            else {
                temp1 = mprintf("l%diffalse:\n",
                                node->node_number);
                output_append_text(node->output, temp1);
                free(temp1);
                if (child1->cleanup) output_concat(node->output, child1->cleanup);
                if (child2->cleanup) output_concat(node->output, child2->cleanup);
            }
            break;

        case DO: /* DO LOOP */
            /* Loop Assignments REPEAT->output */

            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */

            /* child1 is the REPEAT node, child2 is the INSTRUCTIONS */

            comment_meta = get_metaline_token_at(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            /* Init */
            output_concat(node->output, child1->output);

            /* Loop Start */
            temp1 = mprintf("l%ddostart:\n",
                            node->node_number);
            output_append_text(node->output, temp1);
            free(temp1);

            /* Loop Begin Checks REPEAT->loopstartchecks */
            output_concat(node->output, child1->loopstartchecks);

            /* Loop Body - instructions */
            output_concat(node->output, child2->output);

            /* Loop End Checks REPEAT->loopendchecks */
            temp1 = mprintf("l%ddoinc:\n",
                            node->node_number);
            output_append_text(node->output, temp1);
            free(temp1);
            output_concat(node->output, child1->loopendchecks);

            /* Loop increments REPEAT->loopinc */
            output_concat(node->output, child1->loopinc);

            /* Loop End */
            comment_meta = get_metaline_token_after(child2);

            output_append_text(node->output, comment_meta);
            temp1 = mprintf("   br l%ddostart\nl%ddoend:\n",
                            node->node_number, node->node_number);
            output_append_text(node->output, temp1);
            if (child1->cleanup) output_concat(node->output, child1->cleanup);
            free(temp1);
            free(comment_meta);
            break;

        case REPEAT:
            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */
            if (!node->output) node->output = output_f(); /* Assign / init instruction */
            node->loopstartchecks = output_f(); /* Begin Loop exit checks */
            node->loopinc = output_f(); /* Loop increments */
            node->loopendchecks = output_f(); /* End Loop exit checks */
            while (child1) {
                if (child1->node_type == ASSIGN) {
                    /* Only output is valid - does not follow convention */
                    if (child1->output) output_concat(node->output, child1->output);
                }
                else {
                    if (child1->output)
                        output_concat(node->output, child1->output);
                    if (child1->loopstartchecks)
                        output_concat(node->loopstartchecks, child1->loopstartchecks);
                    if (child1->loopinc)
                        output_concat(node->loopinc, child1->loopinc);
                    if (child1->loopendchecks)
                        output_concat(node->loopendchecks, child1->loopendchecks);
                }
                child1 = child1->sibling;
            }
            /* Output Cleanups */
            child1 = node->child;
            while (child1) {
                if (child1->cleanup) {
                    if (!node->cleanup) node->cleanup = output_f();
                    output_concat(node->cleanup, child1->cleanup);
                }
                child1 = child1->sibling;
            }

            break;

        case FOR:
            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */
            comment_meta = get_metaline(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            output_concat(node->output, child1->output);
            if (child1->register_num != node->register_num ||
                child1->register_type != node->register_type) {
                temp1 = mprintf("   icopy %c%d,%c%d\n",
                                node->register_type,
                                node->register_num,
                                child1->register_type,
                                child1->register_num);
                output_append_text(node->output, temp1);
                free(temp1);
            }
            node->loopstartchecks = output_fs(comment_meta);
            temp1 = mprintf("   bcf l%ddoend,%c%d\n",
                            node->parent->parent->node_number,
                            node->register_type,
                            node->register_num);
            output_append_text(node->loopstartchecks, temp1);
            if (child1->cleanup) {
                if (!node->cleanup) node->cleanup = output_f();
                output_concat(node->cleanup, child1->cleanup);
            }
            free(comment_meta);
            free(temp1);
            break;

        case TO:
            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */

            comment_meta = get_metaline(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            output_concat(node->output, child1->output);

            /* Need to determine the sign of the BY */
            /* Find the BY */
            j = 1; /* J is the sign: 1 (default)=positive, -1=negative, 0=dynamic */
            n = node->parent->child; /* First sibling */
            while (n) {
                if (n->node_type == BY) {
                    if (ast_chdn(n, 0)) {
                        if (is_constant(ast_chdn(n, 0))) {
                            if (ast_chdn(n, 0)->value_type == ast_chdn(n, 0)->target_type) {
                                /* Is a constant */
                                if (ast_chdn(n, 0)->value_type == TP_INTEGER) {
                                    if (ast_chdn(n, 0)->int_value >= 0) j = 1;
                                    else j = -1;
                                }
                                else if (ast_chdn(n, 0)->value_type == TP_FLOAT) {
                                    if (ast_chdn(n, 0)->float_value >= 0.0) j = 1;
                                    else j = -1;
                                }
                                else if (ast_chdn(n, 0)->value_type == TP_DECIMAL) {
                                    if (ast_chdn(n, 0)->decimal_value[0] == '-') j = -1;
                                    else j = 1;
                                }
                                else j = 1;
                            }
                            else j = 0; /* Not a constant */
                        }
                        else j = 0; /* Not a constant */
                    }
                    else j = 1; /* Implicit by */
                    break;
                }
                n = n->sibling;
            }
            /* n is set by the BY node */

            /* If the REPEAT has a TO it has an ASSIGN and its register
             * number will have been set to the ASSIGN Variable */
            node->loopstartchecks = output_fs(comment_meta);
            switch (j) {
                case 1: /* Positive */
                    temp1 = mprintf("   %sgt r%d,%c%d,%c%d\n   brt l%ddoend,r%d\n",
                                    tp_prefix,
                                    node->additional_registers,
                                    node->parent->register_type,
                                    node->parent->register_num,
                                    child1->register_type,
                                    child1->register_num,
                                    node->parent->parent->node_number,
                                    node->additional_registers);
                    break;
                case -1: /* Negative */
                    temp1 = mprintf("   %slt r%d,%c%d,%c%d\n   brt l%ddoend,r%d\n",
                                    tp_prefix,
                                    node->additional_registers,
                                    node->parent->register_type,
                                    node->parent->register_num,
                                    child1->register_type,
                                    child1->register_num,
                                    node->parent->parent->node_number,
                                    node->additional_registers);
                    break;
                default: /* Dynamic by value */
                    /* We need a zero (int or flaot */
                    if (*tp_prefix == 'i') op = "0";
                    else op = "0.0";
                    temp1 = mprintf(
                            "   %slt r%d,%c%d,%s\n" /* Check the by value sign */
                            "   brt l%ddoneg1,r%d\n"    /* JMP to Negative BY */

                            "   %sgt r%d,%c%d,%c%d\n"   /* Pos BY */
                            "   brtf l%ddoend,l%ddoneg2,r%d\n"

                            "l%ddoneg1:\n"
                            "   %slt r%d,%c%d,%c%d\n"   /* Neg BY */
                            "   brt l%ddoend,r%d\n"

                            "l%ddoneg2:\n",

                            tp_prefix,
                            node->additional_registers,
                            ast_chdn(n, 0)->register_type,
                            ast_chdn(n, 0)->register_num,
                            op,
                            node->parent->parent->node_number,
                            node->additional_registers,

                            tp_prefix,
                            node->additional_registers,
                            node->parent->register_type,
                            node->parent->register_num,
                            child1->register_type,
                            child1->register_num,
                            node->parent->parent->node_number,
                            node->parent->parent->node_number,
                            node->additional_registers,

                            node->parent->parent->node_number,
                            tp_prefix,
                            node->additional_registers,
                            node->parent->register_type,
                            node->parent->register_num,
                            child1->register_type,
                            child1->register_num,
                            node->parent->parent->node_number,
                            node->additional_registers,

                            node->parent->parent->node_number);
            }
            output_append_text(node->loopstartchecks, temp1);
            free(temp1);
            free(comment_meta);
            if (child1->cleanup) {
                if (!node->cleanup) node->cleanup = output_f();
                output_concat(node->cleanup, child1->cleanup);
            }
            break;

        case BY:
            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */

            /* If the REPEAT has a BY it has an ASSIGN and its register
             * number will have been set to the ASSIGN Variable */

            if (child1) {
                /* BY explicitly stated */
                comment_meta = get_metaline(node);
                if (node->output) output_prepend_text(comment_meta, node->output);
                else node->output = output_fs(comment_meta);
                output_concat(node->output, child1->output);

                node->loopinc = output_fs(comment_meta);
                temp1 = mprintf(" %sadd %c%d,%c%d,%c%d\n",
                                tp_prefix,
                                node->parent->register_type,
                                node->parent->register_num,
                                node->child->register_type,
                                node->child->register_num,
                                node->parent->register_type,
                                node->parent->register_num);
                output_append_text(node->loopinc, temp1);
                free(comment_meta);
                free(temp1);
                if (child1->cleanup) {
                    if (!node->cleanup) node->cleanup = output_f();
                    output_concat(node->cleanup, child1->cleanup);
                }
            }
            else {
                /* BY Added implicitly - increment by 1 */
                /* For the source we can only reference the symbol in the loop assignment node */
//                    comment_meta = get_comment_line_number_only(node->parent, "{Implicit \"BY 1\"}");
                comment_meta = get_metaline_token_at(node->parent->child);
                node->loopinc = output_fs(comment_meta);
                free(comment_meta);

                if (*tp_prefix == 'i') {
                    temp1 = mprintf("   inc %c%d\n",
                                    node->parent->register_type,
                                    node->parent->register_num);
                }
                else {
                    temp1 = mprintf("   %sadd %c%d,%c%d,1.0\n",
                                    tp_prefix,
                                    node->parent->register_type,
                                    node->parent->register_num,
                                    node->parent->register_type,
                                    node->parent->register_num);
                }
                output_append_text(node->loopinc, temp1);
                free(temp1);
            }
            break;

        case WHILE:
            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */
            comment_meta = get_metaline(node);
            node->loopstartchecks = output_fs(comment_meta);
            free(comment_meta);
            output_concat(node->loopstartchecks, child1->output);
            temp1 = mprintf("   brf l%ddoend,%c%d\n",
                            node->parent->parent->node_number,
                            node->register_type,
                            node->register_num);
            output_append_text(node->loopstartchecks, temp1);
            free(temp1);
            if (child1->cleanup) {
                if (!node->cleanup) node->cleanup = output_f();
                output_concat(node->cleanup, child1->cleanup);
            }
            break;

        case UNTIL:
            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */
            comment_meta = get_metaline(node);
            node->loopendchecks = output_fs(comment_meta);
            free(comment_meta);
            output_concat(node->loopendchecks, child1->output);
            temp1 = mprintf("   brt l%ddoend,%c%d\n",
                            node->parent->parent->node_number,
                            node->register_type,
                            node->register_num);
            output_append_text(node->loopendchecks, temp1);
            free(temp1);
            if (child1->cleanup) {
                if (!node->cleanup) node->cleanup = output_f();
                output_concat(node->cleanup, child1->cleanup);
            }
            break;

        case LEAVE:
            /* Leave Loop */
            /* Add source metadata */
            comment_meta = get_metaline(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            /* Add Variable Metadata */
            add_variable_metadata(node);

            temp1 = mprintf("   br l%ddoend\n",
                            node->association->node_number);
            output_append_text(node->output, temp1);
            free(temp1);
            break;

        case LEAVE_WITH:
            /* Leave BLOCK_EXPR with value */
            comment_meta = get_metaline(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            /* Add Variable Metadata */
            add_variable_metadata(node);

            if (child1) {
                char *result_prefix;

                if (child1->output) output_concat(node->output, child1->output);

                if (node->association) {
                    if (node->association->value_dims) result_prefix = "";
                    else result_prefix = type_to_prefix(node->association->value_type);

                    if (child1->register_num == DONT_ASSIGN_REGISTER) {
                        temp2 = format_constant(child1->target_type, child1);
                        temp1 = mprintf("   load %c%d,%s\n",
                                        node->association->register_type,
                                        node->association->register_num,
                                        temp2);
                        output_append_text(node->output, temp1);
                        free(temp1);
                        free(temp2);
                    }
                    else if (child1->register_num != node->association->register_num ||
                             child1->register_type != node->association->register_type) {
                        temp1 = mprintf("   %scopy %c%d,%c%d\n",
                                        result_prefix,
                                        node->association->register_type,
                                        node->association->register_num,
                                        child1->register_type,
                                        child1->register_num);
                        output_append_text(node->output, temp1);
                        free(temp1);

                        if (flow_needs_attr_copy(node->association)) {
                            temp1 = mprintf("   acopy %c%d,%c%d\n",
                                            node->association->register_type,
                                            node->association->register_num,
                                            child1->register_type,
                                            child1->register_num);
                            output_append_text(node->output, temp1);
                            free(temp1);
                        }
                    }
                }

                if (child1->cleanup) output_concat(node->output, child1->cleanup);
            }

            if (node->association) {
                temp1 = mprintf("   br l%dbexprend\n",
                                node->association->node_number);
                output_append_text(node->output, temp1);
                free(temp1);
            }
            break;

        case ITERATE:
            /* Iterate Loop */
            /* Add source metadata */
            comment_meta = get_metaline(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            /* Add Variable Metadata */
            add_variable_metadata(node);

            temp1 = mprintf("   br l%ddoinc\n",
                            node->association->node_number);
            output_append_text(node->output, temp1);
            free(temp1);
            break;

        default:
            break;
    }
}
