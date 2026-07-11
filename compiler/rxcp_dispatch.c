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
#include "../binutils/include/rxnumparse.h"
#include "rxcp_dispatch.h"
#include "rxcp_util.h"

#define RXCP_DISPATCH_MIN_INTEGER_CASES 8
#define RXCP_DISPATCH_MIN_EXACT_STRING_CASES 3
#define RXCP_DISPATCH_MIN_PADDED_STRING_CASES 2
#define RXCP_DISPATCH_MIN_NUMERIC_STRING_CASES 2
#define RXCP_DISPATCH_MIN_BINARY_CASES 3

typedef struct RxcpDispatchCase {
    ASTNode *if_node;
    ASTNode *selector;
    ASTNode *key;
    ASTNode *body;
    rxinteger key_value;
    unsigned char *canonical_key;
    size_t canonical_key_length;
    Symbol *selector_symbol;
    DispatchKind kind;
    int has_dispatch_key;
} RxcpDispatchCase;

typedef struct RxcpDispatchCandidate {
    RxcpDispatchCase *cases;
    size_t count;
    size_t capacity;
} RxcpDispatchCandidate;

typedef struct RxcpDispatchKeyView {
    const unsigned char *bytes;
    size_t length;
} RxcpDispatchKeyView;

static int dispatch_key_view_compare(const void *left, const void *right) {
    const RxcpDispatchKeyView *left_view = left;
    const RxcpDispatchKeyView *right_view = right;
    size_t common = left_view->length < right_view->length ?
                    left_view->length : right_view->length;
    int comparison = memcmp(left_view->bytes, right_view->bytes, common);

    if (comparison) return comparison;
    if (left_view->length < right_view->length) return -1;
    if (left_view->length > right_view->length) return 1;
    return 0;
}

static void dispatch_candidate_free(RxcpDispatchCandidate *candidate) {
    if (!candidate) return;
    size_t i;

    for (i = 0; i < candidate->count; i++) free(candidate->cases[i].canonical_key);
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
                                  unsigned char *canonical_key,
                                  size_t canonical_key_length,
                                  Symbol *selector_symbol,
                                  DispatchKind kind,
                                  int has_dispatch_key) {
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
    candidate->cases[candidate->count].canonical_key = canonical_key;
    candidate->cases[candidate->count].canonical_key_length = canonical_key_length;
    candidate->cases[candidate->count].selector_symbol = selector_symbol;
    candidate->cases[candidate->count].kind = kind;
    candidate->cases[candidate->count].has_dispatch_key = has_dispatch_key;
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
        rxinteger parsed;

        if (!node->node_string || !node->node_string_length) return 0;
        if (rxinteger_parse(node->node_string, &end, &parsed) ||
            end != node->node_string + node->node_string_length) return 0;
        if (value) *value = parsed;
        return 1;
    }

    if ((node->node_type == OP_NEG || node->node_type == OP_PLUS) && node->child) {
        rxinteger operand;
        static const char min_magnitude[] = "9223372036854775808";

        if (node->node_type == OP_NEG &&
            node->child->node_type == INTEGER &&
            node->child->node_string &&
            node->child->node_string_length == sizeof(min_magnitude) - 1 &&
            memcmp(node->child->node_string, min_magnitude, sizeof(min_magnitude) - 1) == 0) {
            if (value) *value = RXINTEGER_MIN;
            return 1;
        }
        if (!dispatch_integer_key(node->child, &operand)) return 0;
        if (node->node_type == OP_NEG) {
            if (!rxinteger_checked_neg(operand, &operand)) return 0;
        }
        if (value) *value = operand;
        return 1;
    }

    return 0;
}

static int dispatch_scalar_selector(ASTNode *node, ValueType type) {
    Symbol *symbol;

    if (!node ||
        (node->node_type != VAR_REFERENCE && node->node_type != VAR_SYMBOL) ||
        node->value_dims || node->target_dims) return 0;
    if (!node->symbolNode || !node->symbolNode->symbol) return 0;
    symbol = node->symbolNode->symbol;
    return node->value_type == type || node->target_type == type || symbol->type == type;
}

static int dispatch_stable_general_selector(ASTNode *node, ValueType type) {
    Symbol *symbol;

    if (!dispatch_scalar_selector(node, type)) return 0;
    symbol = node->symbolNode->symbol;
    if (symbol->symbol_type != VARIABLE_SYMBOL || symbol->value_dims) return 0;
    if (symbol->exposed || symbol->is_global_var || symbol->is_ref_arg ||
        symbol->has_reference_target || symbol->is_this || symbol->is_factory) return 0;
    if (symbol->scope &&
        (symbol->scope->type == SCOPE_CLASS ||
         (symbol->scope->defining_node &&
          symbol->scope->defining_node->node_type == CLASS_DEF))) return 0;
    return 1;
}

static int dispatch_hex_value(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    return -1;
}

static unsigned char *dispatch_decode_string(ASTNode *node, size_t *length_out) {
    unsigned char *buffer;
    size_t i;
    size_t out = 0;

    if (!node || node->value_type != TP_STRING ||
        (node->node_type != STRING && node->node_type != CONSTANT)) return 0;
    buffer = malloc(node->node_string_length ? node->node_string_length : 1);
    if (!buffer) return 0;

    for (i = 0; i < node->node_string_length; i++) {
        unsigned char byte = (unsigned char)node->node_string[i];
        if (byte == '\\' && i + 1 < node->node_string_length) {
            char esc = node->node_string[++i];
            switch (esc) {
                case '\\': byte = '\\'; break;
                case 'n': byte = '\n'; break;
                case 't': byte = '\t'; break;
                case 'a': byte = '\a'; break;
                case 'b': byte = '\b'; break;
                case 'f': byte = '\f'; break;
                case 'r': byte = '\r'; break;
                case 'v': byte = '\v'; break;
                case '\'': byte = '\''; break;
                case '"': byte = '"'; break;
                case '0': byte = '\0'; break;
                case '?': byte = '?'; break;
                case 'x':
                    if (i + 2 < node->node_string_length) {
                        int high = dispatch_hex_value(node->node_string[i + 1]);
                        int low = dispatch_hex_value(node->node_string[i + 2]);
                        if (high >= 0 && low >= 0) {
                            byte = (unsigned char)((high << 4) | low);
                            i += 2;
                            break;
                        }
                    }
                    buffer[out++] = '\\';
                    byte = (unsigned char)esc;
                    break;
                default:
                    buffer[out++] = '\\';
                    byte = (unsigned char)esc;
                    break;
            }
        }
        buffer[out++] = byte;
    }
    *length_out = out;
    return buffer;
}

static unsigned char *dispatch_decode_binary(ASTNode *node, size_t *length_out) {
    const char *text;
    size_t length;
    size_t hex_length;
    unsigned char *bytes;
    size_t i;

    if (!node || node->value_type != TP_BINARY ||
        (node->node_type != BINARY && node->node_type != CONSTANT)) return 0;
    text = node->node_string;
    length = node->node_string_length;
    if (length < 2 || text[0] != '0' || (text[1] != 'x' && text[1] != 'X')) return 0;
    hex_length = length - 2;
    if (hex_length % 2) return 0;
    bytes = malloc(hex_length ? hex_length / 2 : 1);
    if (!bytes) return 0;
    for (i = 0; i < hex_length; i += 2) {
        int high = dispatch_hex_value(text[i + 2]);
        int low = dispatch_hex_value(text[i + 3]);
        if (high < 0 || low < 0) {
            free(bytes);
            return 0;
        }
        bytes[i / 2] = (unsigned char)((high << 4) | low);
    }
    *length_out = hex_length / 2;
    return bytes;
}

static int dispatch_classify_key(ASTNode *condition,
                                 ASTNode *selector,
                                 ASTNode *key,
                                 DispatchKind *kind_out,
                                 rxinteger *integer_out,
                                 unsigned char **canonical_out,
                                 size_t *canonical_length_out) {
    Symbol *symbol;
    unsigned char *bytes = 0;
    size_t length = 0;

    if (!selector || !selector->symbolNode || !selector->symbolNode->symbol || !key) return 0;
    symbol = selector->symbolNode->symbol;

    if (symbol->type == TP_INTEGER && dispatch_integer_key(key, integer_out)) {
        uint64_t value = (uint64_t)(int64_t)*integer_out;
        size_t i;

        bytes = malloc(8);
        if (!bytes) return 0;
        for (i = 0; i < 8; i++) bytes[i] = (unsigned char)((value >> (i * 8)) & 0xffu);
        *kind_out = DISPATCH_INTEGER;
        *canonical_out = bytes;
        *canonical_length_out = 8;
        return 1;
    }

    if (symbol->type == TP_STRING) {
        bytes = dispatch_decode_string(key, &length);
        if (!bytes) return 0;
        if (condition->node_type == OP_COMPARE_S_EQ) {
            *kind_out = DISPATCH_STRING_EXACT;
        } else {
            unsigned char numeric_key[RX_NUMERIC_KEY_SIZE];
            int is_nan = 0;

            int is_numeric = rx_numeric_key_from_text(numeric_key,
                                                      (const char *)bytes,
                                                      length,
                                                      &is_nan);

            if (is_numeric && is_nan) {
                free(bytes);
                return 0;
            }
            if (is_numeric) {
                free(bytes);
                bytes = malloc(sizeof(numeric_key));
                if (!bytes) return 0;
                memcpy(bytes, numeric_key, sizeof(numeric_key));
                length = sizeof(numeric_key);
                *kind_out = DISPATCH_STRING_NUMERIC;
            } else {
                while (length > 0 && bytes[length - 1] == ' ') length--;
                *kind_out = DISPATCH_STRING_PADDED;
            }
        }
        *canonical_out = bytes;
        *canonical_length_out = length;
        return 1;
    }

    if (symbol->type == TP_BINARY && condition->node_type == OP_COMPARE_EQUAL) {
        bytes = dispatch_decode_binary(key, &length);
        if (!bytes) return 0;
        *kind_out = DISPATCH_BINARY_EXACT;
        *canonical_out = bytes;
        *canonical_length_out = length;
        return 1;
    }
    return 0;
}

static ASTNode *dispatch_constant_payload(ASTNode *node, ValueType expected_type) {
    ASTNode *value;

    if (!node || node->node_type != OP_TYPE_CAST) return node;
    value = ast_chdn(node, 0);
    if (!value || value->value_type != expected_type) return node;
    return value;
}

static int dispatch_run_has_duplicate_key(RxcpDispatchCandidate *candidate,
                                          size_t start,
                                          size_t end) {
    size_t count = end - start + 1;
    RxcpDispatchKeyView *views;
    size_t i;

    if (count > SIZE_MAX / sizeof(*views)) return 1;
    views = malloc(count * sizeof(*views));
    if (!views) return 1;
    for (i = 0; i < count; i++) {
        views[i].bytes = candidate->cases[start + i].canonical_key;
        views[i].length = candidate->cases[start + i].canonical_key_length;
    }
    qsort(views, count, sizeof(*views), dispatch_key_view_compare);
    for (i = 1; i < count; i++) {
        if (dispatch_key_view_compare(&views[i - 1], &views[i]) == 0) {
            free(views);
            return 1;
        }
    }
    free(views);
    return 0;
}

static int collect_dispatch_ladder(ASTNode *root,
                                   RxcpDispatchCandidate *candidate,
                                   int lock_source_anchor,
                                   int require_stable_selector) {
    ASTNode *current = root;
    SourceNode *select_source = lock_source_anchor && root ? root->source_node : 0;

    while (current && current->node_type == IF) {
        ASTNode *condition = ast_chdn(current, 0);
        ASTNode *body = ast_chdn(current, 1);
        ASTNode *otherwise = ast_chdn(current, 2);
        ASTNode *selector = 0;
        ASTNode *key = 0;
        rxinteger key_value = 0;
        unsigned char *canonical_key = 0;
        size_t canonical_key_length = 0;
        Symbol *selector_symbol = 0;
        DispatchKind kind = DISPATCH_NONE;
        int has_dispatch_key = 0;

        /* The generated IF nodes share the original SELECT source anchor. An
         * IF supplied as the user's OTHERWISE body is not part of this ladder. */
        if (select_source && current->source_node != select_source) break;
        if (!condition || !body) return 0;

        if (condition->node_type == OP_COMPARE_EQUAL ||
            condition->node_type == OP_COMPARE_S_EQ) {
            selector = ast_chdn(condition, 0);
            key = ast_chdn(condition, 1);
            if (selector && selector->symbolNode && selector->symbolNode->symbol) {
                selector_symbol = selector->symbolNode->symbol;
                key = dispatch_constant_payload(key, selector_symbol->type);
                if ((selector_symbol->type == TP_INTEGER ||
                     selector_symbol->type == TP_STRING ||
                     selector_symbol->type == TP_BINARY) &&
                    dispatch_scalar_selector(selector, selector_symbol->type) &&
                    (!require_stable_selector ||
                     dispatch_stable_general_selector(selector, selector_symbol->type))) {
                    has_dispatch_key = dispatch_classify_key(condition,
                                                             selector,
                                                             key,
                                                             &kind,
                                                             &key_value,
                                                             &canonical_key,
                                                             &canonical_key_length);
                }
            }
        }

        if (!dispatch_candidate_add(candidate,
                                    current,
                                    selector,
                                    key,
                                    body,
                                    key_value,
                                    canonical_key,
                                    canonical_key_length,
                                    selector_symbol,
                                    kind,
                                    has_dispatch_key)) {
            free(canonical_key);
            return 0;
        }

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

static ValueType dispatch_key_type(DispatchKind kind) {
    switch (kind) {
        case DISPATCH_INTEGER: return TP_INTEGER;
        case DISPATCH_BINARY_EXACT: return TP_BINARY;
        case DISPATCH_STRING_EXACT:
        case DISPATCH_STRING_PADDED:
        case DISPATCH_STRING_NUMERIC:
            return TP_STRING;
        default: return TP_UNKNOWN;
    }
}

static size_t dispatch_minimum_cases(DispatchKind kind) {
    switch (kind) {
        case DISPATCH_INTEGER:
            return RXCP_DISPATCH_MIN_INTEGER_CASES;
        case DISPATCH_STRING_EXACT:
            return RXCP_DISPATCH_MIN_EXACT_STRING_CASES;
        case DISPATCH_STRING_PADDED:
            return RXCP_DISPATCH_MIN_PADDED_STRING_CASES;
        case DISPATCH_STRING_NUMERIC:
            return RXCP_DISPATCH_MIN_NUMERIC_STRING_CASES;
        case DISPATCH_BINARY_EXACT:
            return RXCP_DISPATCH_MIN_BINARY_CASES;
        default:
            return SIZE_MAX;
    }
}

static void rewrite_dispatch(Context *context,
                             RxcpDispatchCandidate *candidate,
                             size_t start,
                             size_t end) {
    ASTNode *root = candidate->cases[start].if_node;
    ASTNode *dispatch;
    ASTNode *selector;
    ASTNode *fallback = ast_chdn(candidate->cases[end].if_node, 2);
    size_t i;

    dispatch = new_dispatch_node(context, OPT_DISPATCH, root, root->scope);
    dispatch->dispatch_kind = candidate->cases[start].kind;

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
        key->value_type = dispatch_key_type(candidate->cases[i].kind);
        key->target_type = key->value_type;
        if (candidate->cases[i].kind == DISPATCH_INTEGER) {
            char *key_text;
            key->int_value = candidate->cases[i].key_value;
            key_text = mprintf("%" PRIdMAX, (intmax_t)key->int_value);
            ast_sstr(key, key_text, strlen(key_text));
        } else {
            ASTNode *source_key = candidate->cases[i].key;
            char *key_text = malloc(source_key->node_string_length + 1);
            if (!key_text) {
                RX_PANIC_OOM("malloc rxc dispatch key",
                             source_key->node_string_length + 1,
                             context->file_name);
            }
            if (source_key->node_string_length) {
                memcpy(key_text, source_key->node_string, source_key->node_string_length);
            }
            key_text[source_key->node_string_length] = 0;
            ast_sstr(key, key_text, source_key->node_string_length);
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

static void lower_dispatch_runs(Context *context,
                                RxcpDispatchCandidate *candidate) {
    size_t position = candidate->count;

    /* Work backwards so each rewritten run is already attached to the
     * untouched miss path of the preceding run or residual comparison. */
    while (position > 0) {
        size_t end;
        size_t start;

        while (position > 0 && !candidate->cases[position - 1].has_dispatch_key) position--;
        if (!position) break;

        end = position - 1;
        start = end;
        while (start > 0 && candidate->cases[start - 1].has_dispatch_key &&
               candidate->cases[start - 1].kind == candidate->cases[end].kind &&
               candidate->cases[start - 1].selector_symbol ==
                   candidate->cases[end].selector_symbol) {
            start--;
        }

        if (end - start + 1 >= dispatch_minimum_cases(candidate->cases[end].kind) &&
            !dispatch_run_has_duplicate_key(candidate, start, end)) {
            rewrite_dispatch(context, candidate, start, end);
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

    if (collect_dispatch_ladder(root, &candidate, 1, 0)) {
        lower_dispatch_runs(context, &candidate);
    }
    dispatch_candidate_free(&candidate);
    return result_normal;
}

static int dispatch_is_else_if_child(ASTNode *node) {
    return node && node->parent && node->parent->node_type == IF &&
           ast_chdn(node->parent, 2) == node;
}

static int dispatch_has_explicit_select_owner(ASTNode *node) {
    ASTNode *current = node;

    while (current) {
        if (current->node_type == INSTRUCTIONS && current->is_select_dispatch) return 1;
        current = current->parent;
    }
    return 0;
}

static walker_result general_dispatch_walker(walker_direction direction,
                                             ASTNode *node,
                                             void *payload) {
    Context *context = payload;
    RxcpDispatchCandidate candidate = {0};

    if (direction != out || node->node_type != IF) return result_normal;
    if (dispatch_is_else_if_child(node) || dispatch_has_explicit_select_owner(node)) {
        return result_normal;
    }

    if (collect_dispatch_ladder(node, &candidate, 0, 1)) {
        lower_dispatch_runs(context, &candidate);
    }
    dispatch_candidate_free(&candidate);
    return result_normal;
}

void rxcp_lower_select_dispatch(Context *context) {
    if (!context || !context->ast) return;
    ast_wlkr(context->ast, select_dispatch_walker, context);
    if (context->optimise) ast_wlkr(context->ast, general_dispatch_walker, context);
}
