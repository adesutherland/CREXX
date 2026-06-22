/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXCP_REMAP_H
#define CREXX_RXCP_REMAP_H

#include <stddef.h>
#include <stdio.h>
#include "rxcp_types.h"
#include "rxcp_ast.h"

typedef enum {
    RXCP_REMAP_NO_MATCH = 0,
    RXCP_REMAP_REJECTED,
    RXCP_REMAP_APPLIED
} RxcpRemapResult;

typedef enum {
    RXCP_REMAP_SEL_END = 0,
    RXCP_REMAP_SEL_MATCH_CLASS,
    RXCP_REMAP_SEL_MATCH_NODE_TYPE,
    RXCP_REMAP_SEL_MATCH_ANY,
    RXCP_REMAP_SEL_MATCH_TYPESET,
    RXCP_REMAP_SEL_DOWN_FIRST,
    RXCP_REMAP_SEL_NEXT_SIBLING,
    RXCP_REMAP_SEL_UP_PARENT,
    RXCP_REMAP_SEL_ASSERT_NO_NEXT_SIBLING
} RxcpRemapSelectorOp;

typedef enum {
    RXCP_REMAP_NODECLASS_EAGER_OPERATOR = 1
} RxcpRemapNodeClass;

typedef enum {
    RXCP_REMAP_TYPESET_CALL_EXPR = 1
} RxcpRemapTypeSet;

typedef struct {
    RxcpRemapSelectorOp op;
    const char *capture;
    int value;
} RxcpRemapSelectorStep;

#define RXCP_REMAP_SEL_CLASS(capture_name, node_class) \
    { RXCP_REMAP_SEL_MATCH_CLASS, (capture_name), (node_class) }
#define RXCP_REMAP_SEL_TYPE(capture_name, node_type) \
    { RXCP_REMAP_SEL_MATCH_NODE_TYPE, (capture_name), (node_type) }
#define RXCP_REMAP_SEL_ANY(capture_name) \
    { RXCP_REMAP_SEL_MATCH_ANY, (capture_name), 0 }
#define RXCP_REMAP_SEL_TYPESET(capture_name, type_set) \
    { RXCP_REMAP_SEL_MATCH_TYPESET, (capture_name), (type_set) }
#define RXCP_REMAP_SEL_DOWN_FIRST() \
    { RXCP_REMAP_SEL_DOWN_FIRST, NULL, 0 }
#define RXCP_REMAP_SEL_NEXT_SIBLING() \
    { RXCP_REMAP_SEL_NEXT_SIBLING, NULL, 0 }
#define RXCP_REMAP_SEL_UP_PARENT() \
    { RXCP_REMAP_SEL_UP_PARENT, NULL, 0 }
#define RXCP_REMAP_SEL_ASSERT_NO_NEXT_SIBLING() \
    { RXCP_REMAP_SEL_ASSERT_NO_NEXT_SIBLING, NULL, 0 }
#define RXCP_REMAP_SEL_END() \
    { RXCP_REMAP_SEL_END, NULL, 0 }

#define RXCP_REMAP_MAX_CAPTURES 8

typedef struct {
    const char *name;
    ASTNode *node;
} RxcpRemapCapture;

typedef struct {
    ASTNode *root;
    ASTNode *current;
    RxcpRemapCapture captures[RXCP_REMAP_MAX_CAPTURES];
    size_t capture_count;
    Symbol *symbol;
    void *user_data;
} RxcpRemapMatch;

struct RxcpRemapRule;
typedef RxcpRemapResult (*RxcpRemapApplyFn)(const struct RxcpRemapRule *rule,
                                            Context *context,
                                            void *payload,
                                            ASTNode *node);
typedef int (*RxcpRemapBindFn)(Context *context, RxcpRemapMatch *match);
typedef int (*RxcpRemapGuardFn)(Context *context, const RxcpRemapMatch *match);
typedef int (*RxcpRemapRewriteFn)(Context *context, const RxcpRemapMatch *match);

typedef struct {
    const char *id;
    RxcpRemapBindFn fn;
} RxcpRemapBindStep;

typedef struct {
    const char *id;
    RxcpRemapGuardFn fn;
} RxcpRemapGuardStep;

typedef struct {
    const char *id;
    RxcpRemapRewriteFn fn;
} RxcpRemapRewriteStep;

#define RXCP_REMAP_BIND_STEP(step_id, step_fn) \
    { (step_id), (step_fn) }
#define RXCP_REMAP_BIND_END() \
    { NULL, NULL }
#define RXCP_REMAP_GUARD_STEP(step_id, step_fn) \
    { (step_id), (step_fn) }
#define RXCP_REMAP_GUARD_END() \
    { NULL, NULL }
#define RXCP_REMAP_REWRITE_STEP(step_id, step_fn) \
    { (step_id), (step_fn) }
#define RXCP_REMAP_REWRITE_END() \
    { NULL, NULL }

#define RXCP_REMAP_RULE_SELECTOR(rule_id, rule_phase, rule_root, rule_priority, rule_selector, rule_binds, rule_guards, rule_rewrites, rule_debug_capture) \
    { \
        (rule_id), \
        (rule_phase), \
        (rule_root), \
        (rule_priority), \
        (rule_selector), \
        (rule_binds), \
        (rule_guards), \
        (rule_rewrites), \
        (rule_debug_capture), \
        NULL \
    }

#define RXCP_REMAP_RULE_CALLBACK(rule_id, rule_phase, rule_root, rule_priority, rule_apply) \
    { \
        (rule_id), \
        (rule_phase), \
        (rule_root), \
        (rule_priority), \
        NULL, \
        NULL, \
        NULL, \
        NULL, \
        NULL, \
        (rule_apply) \
    }

#define RXCP_REMAP_RULE_SERVICE(rule_id, rule_phase, rule_root, rule_priority) \
    { \
        (rule_id), \
        (rule_phase), \
        (rule_root), \
        (rule_priority), \
        NULL, \
        NULL, \
        NULL, \
        NULL, \
        NULL, \
        NULL \
    }

typedef struct RxcpRemapRule {
    const char *id;
    const char *phase;
    const char *root_shape;
    int priority;
    const RxcpRemapSelectorStep *selector;
    const RxcpRemapBindStep *binds;
    const RxcpRemapGuardStep *guards;
    const RxcpRemapRewriteStep *rewrites;
    const char *debug_site_capture;
    RxcpRemapApplyFn apply;
} RxcpRemapRule;

int rxcp_remap_run_selector(const RxcpRemapSelectorStep *selector,
                            ASTNode *root,
                            RxcpRemapMatch *match);
ASTNode *rxcp_remap_capture_node(const RxcpRemapMatch *match, const char *name);
ASTNode *rxcp_remap_debug_site(const RxcpRemapRule *rule, const RxcpRemapMatch *match);
size_t rxcp_remap_bind_step_count(const RxcpRemapRule *rule);
size_t rxcp_remap_guard_step_count(const RxcpRemapRule *rule);
size_t rxcp_remap_rewrite_step_count(const RxcpRemapRule *rule);

#endif
