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
    int has_integer_key;
} RxcpDispatchCase;

typedef struct RxcpDispatchCandidate {
    RxcpDispatchCase *cases;
    size_t count;
    size_t capacity;
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
                                  rxinteger key_value,
                                  int has_integer_key) {
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
    candidate->cases[candidate->count].has_integer_key = has_integer_key;
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

static int dispatch_integer_selector(ASTNode *node) {
    Symbol *symbol;

    if (!node || node->node_type != VAR_REFERENCE || node->value_dims || node->target_dims) return 0;
    if (!node->symbolNode || !node->symbolNode->symbol) return 0;
    symbol = node->symbolNode->symbol;
    return node->value_type == TP_INTEGER ||
           node->target_type == TP_INTEGER ||
           symbol->type == TP_INTEGER;
}

static int dispatch_integer_key_compare(const void *left, const void *right) {
    rxinteger left_value = *(const rxinteger *)left;
    rxinteger right_value = *(const rxinteger *)right;

    if (left_value < right_value) return -1;
    if (left_value > right_value) return 1;
    return 0;
}

static int dispatch_run_has_duplicate_key(RxcpDispatchCandidate *candidate,
                                          size_t start,
                                          size_t end) {
    size_t count = end - start + 1;
    rxinteger *keys;
    size_t i;
    int has_duplicate = 0;

    if (count > SIZE_MAX / sizeof(*keys)) return 1;
    keys = malloc(sizeof(*keys) * count);
    if (!keys) return 1;

    for (i = 0; i < count; i++) keys[i] = candidate->cases[start + i].key_value;
    qsort(keys, count, sizeof(*keys), dispatch_integer_key_compare);
    for (i = 1; i < count; i++) {
        if (keys[i - 1] == keys[i]) {
            has_duplicate = 1;
            break;
        }
    }
    free(keys);
    return has_duplicate;
}

static int collect_dispatch_ladder(ASTNode *root, RxcpDispatchCandidate *candidate) {
    ASTNode *current = root;
    SourceNode *select_source = root ? root->source_node : 0;

    while (current && current->node_type == IF) {
        ASTNode *condition = ast_chdn(current, 0);
        ASTNode *body = ast_chdn(current, 1);
        ASTNode *otherwise = ast_chdn(current, 2);
        ASTNode *selector = 0;
        ASTNode *key = 0;
        rxinteger key_value = 0;
        int has_integer_key = 0;

        /* The generated IF nodes share the original SELECT source anchor. An
         * IF supplied as the user's OTHERWISE body is not part of this ladder. */
        if (select_source && current->source_node != select_source) break;
        if (!condition || !body) return 0;

        if (condition->node_type == OP_COMPARE_EQUAL) {
            Symbol *selector_symbol;

            selector = ast_chdn(condition, 0);
            key = ast_chdn(condition, 1);
            if (dispatch_integer_selector(selector) && key) {
                selector_symbol = selector->symbolNode->symbol;
                if (!candidate->selector_symbol) candidate->selector_symbol = selector_symbol;
                if (candidate->selector_symbol == selector_symbol) {
                    has_integer_key = dispatch_integer_key(key, &key_value);
                }
            }
        }

        if (!dispatch_candidate_add(candidate,
                                    current,
                                    selector,
                                    key,
                                    body,
                                    key_value,
                                    has_integer_key)) return 0;

        if (otherwise && otherwise->node_type == IF) {
            current = otherwise;
        } else {
            break;
        }
    }

    return candidate->count > 0;
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
                                     RxcpDispatchCandidate *candidate,
                                     size_t start,
                                     size_t end) {
    ASTNode *root = candidate->cases[start].if_node;
    ASTNode *dispatch;
    ASTNode *selector;
    ASTNode *fallback = ast_chdn(candidate->cases[end].if_node, 2);
    size_t i;

    dispatch = new_dispatch_node(context, OPT_DISPATCH, root, root->scope);

    selector = candidate->cases[start].selector;
    ast_del(selector);
    add_ast(dispatch, selector);

    for (i = start; i <= end; i++) {
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

    {
        ASTNode *dispatch_default;

        if (fallback) {
            ast_del(fallback);
            dispatch_default = new_dispatch_node(context,
                                                 OPT_DISPATCH_DEFAULT,
                                                 fallback,
                                                 root->scope);
            add_ast(dispatch_default, fallback);
            add_ast(dispatch, dispatch_default);
        }
    }

    ast_rpl(root, dispatch);
    ast_wlkr(root, detach_dispatch_symbols, 0);
}

static void lower_integer_dispatch_runs(Context *context,
                                        RxcpDispatchCandidate *candidate) {
    size_t position = candidate->count;

    /* Work backwards so each rewritten run is already attached to the
     * untouched miss path of the preceding run or residual comparison. */
    while (position > 0) {
        size_t end;
        size_t start;

        while (position > 0 && !candidate->cases[position - 1].has_integer_key) position--;
        if (!position) break;

        end = position - 1;
        start = end;
        while (start > 0 && candidate->cases[start - 1].has_integer_key) start--;

        if (end - start + 1 >= RXCP_DISPATCH_MIN_CASES &&
            !dispatch_run_has_duplicate_key(candidate, start, end)) {
            rewrite_integer_dispatch(context, candidate, start, end);
        }
        position = start;
    }
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

    if (collect_dispatch_ladder(root, &candidate)) {
        lower_integer_dispatch_runs(context, &candidate);
    }
    dispatch_candidate_free(&candidate);
    return result_normal;
}

void rxcp_lower_select_dispatch(Context *context) {
    if (!context || !context->ast) return;
    ast_wlkr(context->ast, select_dispatch_walker, context);
}
