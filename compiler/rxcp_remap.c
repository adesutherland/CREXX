/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include <string.h>
#include "rxcp_remap.h"

static int rxcp_remap_store_capture(RxcpRemapMatch *match,
                                    const char *name,
                                    ASTNode *node) {
    size_t i;

    if (!match || !name) return 1;

    for (i = 0; i < match->capture_count; i++) {
        if (match->captures[i].name && strcmp(match->captures[i].name, name) == 0) {
            match->captures[i].node = node;
            return 1;
        }
    }

    if (match->capture_count >= RXCP_REMAP_MAX_CAPTURES) return 0;

    match->captures[match->capture_count].name = name;
    match->captures[match->capture_count].node = node;
    match->capture_count++;
    return 1;
}

ASTNode *rxcp_remap_capture_node(const RxcpRemapMatch *match, const char *name) {
    size_t i;

    if (!match || !name) return NULL;

    for (i = 0; i < match->capture_count; i++) {
        if (match->captures[i].name && strcmp(match->captures[i].name, name) == 0) {
            return match->captures[i].node;
        }
    }

    return NULL;
}

static int rxcp_remap_node_is_eager_operator(ASTNode *node) {
    if (!node) return 0;

    switch (node->node_type) {
        case OP_ADD:
        case OP_MINUS:
        case OP_MULT:
        case OP_DIV:
        case OP_IDIV:
        case OP_MOD:
        case OP_POWER:
        case OP_CONCAT:
        case OP_SCONCAT:
        case OP_COMPARE_EQUAL:
        case OP_COMPARE_NEQ:
        case OP_COMPARE_GT:
        case OP_COMPARE_LT:
        case OP_COMPARE_GTE:
        case OP_COMPARE_LTE:
        case OP_COMPARE_S_EQ:
        case OP_COMPARE_S_NEQ:
        case OP_COMPARE_S_GT:
        case OP_COMPARE_S_LT:
        case OP_COMPARE_S_GTE:
        case OP_COMPARE_S_LTE:
        case OP_NEG:
        case OP_PLUS:
        case OP_NOT:
        case OP_BIT_AND:
        case OP_BIT_OR:
        case OP_BIT_XOR:
        case OP_BIT_NOT:
        case OP_BIT_SHL:
        case OP_BIT_SHR:
        case OP_FLAG_HAS:
            return 1;

        default:
            return 0;
    }
}

static int rxcp_remap_node_matches_class(ASTNode *node,
                                         RxcpRemapNodeClass node_class) {
    if (!node) return 0;

    switch (node_class) {
        case RXCP_REMAP_NODECLASS_EAGER_OPERATOR:
            return rxcp_remap_node_is_eager_operator(node);
        default:
            return 0;
    }
}

static int rxcp_remap_node_matches_typeset(ASTNode *node,
                                           RxcpRemapTypeSet type_set) {
    if (!node) return 0;

    switch (type_set) {
        case RXCP_REMAP_TYPESET_CALL_EXPR:
            return node->node_type == FUNCTION ||
                   node->node_type == MEMBER_CALL ||
                   node->node_type == FACTORY_CALL;
        default:
            return 0;
    }
}

int rxcp_remap_run_selector(const RxcpRemapSelectorStep *selector,
                            ASTNode *root,
                            RxcpRemapMatch *match) {
    size_t i;

    if (!selector || !root || !match) return 0;

    memset(match, 0, sizeof(*match));
    match->root = root;
    match->current = root;

    for (i = 0; selector[i].op != RXCP_REMAP_SEL_END; i++) {
        const RxcpRemapSelectorStep *step = &selector[i];

        switch (step->op) {
            case RXCP_REMAP_SEL_MATCH_CLASS:
                if (!rxcp_remap_node_matches_class(match->current,
                                                   (RxcpRemapNodeClass)step->value)) {
                    return 0;
                }
                if (!rxcp_remap_store_capture(match, step->capture, match->current)) return 0;
                break;

            case RXCP_REMAP_SEL_MATCH_NODE_TYPE:
                if (!match->current || match->current->node_type != (NodeType)step->value) return 0;
                if (!rxcp_remap_store_capture(match, step->capture, match->current)) return 0;
                break;

            case RXCP_REMAP_SEL_MATCH_ANY:
                if (!match->current) return 0;
                if (!rxcp_remap_store_capture(match, step->capture, match->current)) return 0;
                break;

            case RXCP_REMAP_SEL_MATCH_TYPESET:
                if (!rxcp_remap_node_matches_typeset(match->current,
                                                     (RxcpRemapTypeSet)step->value)) {
                    return 0;
                }
                if (!rxcp_remap_store_capture(match, step->capture, match->current)) return 0;
                break;

            case RXCP_REMAP_SEL_DOWN_FIRST:
                if (!match->current || !match->current->child) return 0;
                match->current = match->current->child;
                break;

            case RXCP_REMAP_SEL_NEXT_SIBLING:
                if (!match->current || !match->current->sibling) return 0;
                match->current = match->current->sibling;
                break;

            case RXCP_REMAP_SEL_UP_PARENT:
                if (!match->current || !match->current->parent) return 0;
                match->current = match->current->parent;
                break;

            case RXCP_REMAP_SEL_ASSERT_NO_NEXT_SIBLING:
                if (!match->current || match->current->sibling) return 0;
                break;

            case RXCP_REMAP_SEL_END:
                return 1;
        }
    }

    return 1;
}

ASTNode *rxcp_remap_debug_site(const RxcpRemapRule *rule,
                               const RxcpRemapMatch *match) {
    ASTNode *site;

    if (!match) return NULL;
    if (!rule || !rule->debug_site_capture) return match->root;

    site = rxcp_remap_capture_node(match, rule->debug_site_capture);
    return site ? site : match->root;
}

size_t rxcp_remap_bind_step_count(const RxcpRemapRule *rule) {
    size_t count;

    count = 0;
    if (!rule || !rule->binds) return 0;
    while (rule->binds[count].fn) count++;
    return count;
}

size_t rxcp_remap_guard_step_count(const RxcpRemapRule *rule) {
    size_t count;

    count = 0;
    if (!rule || !rule->guards) return 0;
    while (rule->guards[count].fn) count++;
    return count;
}

size_t rxcp_remap_rewrite_step_count(const RxcpRemapRule *rule) {
    size_t count;

    count = 0;
    if (!rule || !rule->rewrites) return 0;
    while (rule->rewrites[count].fn) count++;
    return count;
}
