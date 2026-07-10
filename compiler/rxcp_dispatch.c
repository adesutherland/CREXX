/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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
 * Compiler-owned SELECT dispatch lowering.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include "rxcp_dispatch.h"
#include "rxcp_util.h"

#define RXCP_DISPATCH_MIN_CASES 3

typedef struct RxcpDispatchCase {
    ASTNode *if_node;
    ASTNode *selector;
    ASTNode *key;
    ASTNode *body;
    rxinteger key_value;
} RxcpDispatchCase;

typedef struct RxcpDispatchCandidate {
    RxcpDispatchCase *cases;
    size_t count;
    size_t capacity;
    ASTNode *fallback;
    Symbol *selector_symbol;
} RxcpDispatchCandidate;

static void dispatch_candidate_free(RxcpDispatchCandidate *candidate) {
    if (!candidate) return;
    free(candidate->cases);
    candidate->cases = 0;
    candidate->count = 0;
    candidate->capacity = 0;
}

static int dispatch_candidate_add(RxcpDispatchCandidate *candidate,
                                  ASTNode *if_node,
                                  ASTNode *selector,
                                  ASTNode *key,
                                  ASTNode *body,
                                  rxinteger key_value) {
    RxcpDispatchCase *new_cases;

    if (candidate->count == candidate->capacity) {
        size_t new_capacity = candidate->capacity ? candidate->capacity * 2 : 4;
        new_cases = realloc(candidate->cases, sizeof(*new_cases) * new_capacity);
        if (!new_cases) return 0;
        candidate->cases = new_cases;
        candidate->capacity = new_capacity;
    }

    candidate->cases[candidate->count].if_node = if_node;
    candidate->cases[candidate->count].selector = selector;
    candidate->cases[candidate->count].key = key;
    candidate->cases[candidate->count].body = body;
    candidate->cases[candidate->count].key_value = key_value;
    candidate->count++;
    return 1;
}

static int dispatch_integer_key(ASTNode *node, rxinteger *value) {
    if (!node || node->value_dims || node->target_dims) return 0;

    if (node->node_type == CONSTANT) {
        if (node->value_type != TP_INTEGER && node->target_type != TP_INTEGER) return 0;
        if (value) *value = node->int_value;
        return 1;
    }

    if (node->node_type == INTEGER) {
        char *end;
        long long parsed;

        if (!node->node_string || !node->node_string_length) return 0;
        errno = 0;
        parsed = strtoll(node->node_string, &end, 10);
        if (errno || end != node->node_string + node->node_string_length) return 0;
        if ((long long)(rxinteger)parsed != parsed) return 0;
        if (value) *value = (rxinteger)parsed;
        return 1;
    }

    if ((node->node_type == OP_NEG || node->node_type == OP_PLUS) && node->child) {
        rxinteger operand;
        if (!dispatch_integer_key(node->child, &operand)) return 0;
        if (node->node_type == OP_NEG) {
            if (operand == (rxinteger)INT64_MIN) return 0;
            operand = (rxinteger)(-operand);
        }
        if (value) *value = operand;
        return 1;
    }

    return 0;
}

static int dispatch_has_duplicate_integer_key(RxcpDispatchCandidate *candidate,
                                              rxinteger key) {
    size_t i;

    for (i = 0; i < candidate->count; i++) {
        if (candidate->cases[i].key_value == key) return 1;
    }
    return 0;
}

static int collect_integer_dispatch(ASTNode *root, RxcpDispatchCandidate *candidate) {
    ASTNode *current = root;

    while (current && current->node_type == IF) {
        ASTNode *condition = ast_chdn(current, 0);
        ASTNode *body = ast_chdn(current, 1);
        ASTNode *otherwise = ast_chdn(current, 2);
        ASTNode *selector;
        ASTNode *key;
        Symbol *selector_symbol;
        rxinteger key_value;

        if (!condition || condition->node_type != OP_COMPARE_EQUAL || !body) return 0;
        selector = ast_chdn(condition, 0);
        key = ast_chdn(condition, 1);
        if (!selector || !key || selector->node_type != VAR_REFERENCE) return 0;
        if (!selector->symbolNode || !selector->symbolNode->symbol) return 0;
        if (selector->value_dims || selector->target_dims) return 0;
        if (selector->value_type != TP_INTEGER && selector->target_type != TP_INTEGER) return 0;
        if (!dispatch_integer_key(key, &key_value)) return 0;

        selector_symbol = selector->symbolNode->symbol;
        if (!candidate->selector_symbol) candidate->selector_symbol = selector_symbol;
        else if (candidate->selector_symbol != selector_symbol) return 0;

        /* RXAS rejects duplicate keys. Preserve first-match semantics by
         * retaining the original ladder until duplicate lowering is added. */
        if (dispatch_has_duplicate_integer_key(candidate, key_value)) return 0;
        if (!dispatch_candidate_add(candidate, current, selector, key, body, key_value)) return 0;

        if (otherwise && otherwise->node_type == IF) {
            current = otherwise;
        } else {
            candidate->fallback = otherwise;
            break;
        }
    }

    return candidate->count >= RXCP_DISPATCH_MIN_CASES;
}

static walker_result detach_dispatch_symbols(walker_direction direction,
                                             ASTNode *node,
                                             void *payload) {
    (void)payload;
    if (direction == in && node->symbolNode && node->symbolNode->symbol) {
        sym_dno(node->symbolNode->symbol, node);
    }
    return result_normal;
}

static ASTNode *new_dispatch_node(Context *context,
                                  NodeType type,
                                  ASTNode *source,
                                  Scope *scope) {
    ASTNode *node = ast_ft(context, type);

    node->scope = scope;
    node->value_type = TP_VOID;
    node->target_type = TP_VOID;
    node->is_compiler_added = 1;
    ast_copy_source_anchor(node, source, AST_SOURCE_SYNTHETIC);
    return node;
}

static void rewrite_integer_dispatch(Context *context,
                                     ASTNode *block,
                                     ASTNode *root,
                                     RxcpDispatchCandidate *candidate) {
    ASTNode *dispatch;
    ASTNode *selector;
    size_t i;

    dispatch = new_dispatch_node(context, OPT_DISPATCH, block, root->scope);

    selector = candidate->cases[0].selector;
    ast_del(selector);
    add_ast(dispatch, selector);

    for (i = 0; i < candidate->count; i++) {
        ASTNode *dispatch_case;
        ASTNode *key;
        ASTNode *body = candidate->cases[i].body;

        ast_del(body);
        key = new_dispatch_node(context,
                                CONSTANT,
                                candidate->cases[i].key,
                                root->scope);
        key->value_type = TP_INTEGER;
        key->target_type = TP_INTEGER;
        key->int_value = candidate->cases[i].key_value;
        {
            char *key_text = mprintf("%" PRIdMAX, (intmax_t)key->int_value);
            ast_sstr(key, key_text, strlen(key_text));
        }
        dispatch_case = new_dispatch_node(context,
                                          OPT_DISPATCH_CASE,
                                          candidate->cases[i].if_node,
                                          root->scope);
        add_ast(dispatch_case, key);
        add_ast(dispatch_case, body);
        add_ast(dispatch, dispatch_case);
    }

    if (candidate->fallback) {
        ASTNode *dispatch_default;
        ASTNode *fallback = candidate->fallback;

        ast_del(fallback);
        dispatch_default = new_dispatch_node(context,
                                             OPT_DISPATCH_DEFAULT,
                                             fallback,
                                             root->scope);
        add_ast(dispatch_default, fallback);
        add_ast(dispatch, dispatch_default);
    }

    ast_rpl(root, dispatch);
    ast_wlkr(root, detach_dispatch_symbols, 0);
}

static walker_result select_dispatch_walker(walker_direction direction,
                                            ASTNode *node,
                                            void *payload) {
    Context *context = payload;
    ASTNode *root;
    RxcpDispatchCandidate candidate = {0};

    if (direction != out || node->node_type != INSTRUCTIONS) return result_normal;
    if (!node->is_select_dispatch) return result_normal;

    root = node->child;
    while (root && root->node_type != IF) root = root->sibling;
    if (!root) return result_normal;

    if (collect_integer_dispatch(root, &candidate)) {
        rewrite_integer_dispatch(context, node, root, &candidate);
    }
    dispatch_candidate_free(&candidate);
    return result_normal;
}

void rxcp_lower_select_dispatch(Context *context) {
    if (!context || !context->ast) return;
    ast_wlkr(context->ast, select_dispatch_walker, context);
}
