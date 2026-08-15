/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

/** Gate F Level G task-expression dependency-plan lowering. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxcp_task_lower.h"
#include "rxcp_remap_build.h"
#include "rxcp_sym.h"
#include "rxcp_val.h"

typedef struct RxcpTaskPlan {
    Context *context;
    Scope *scope;
    ASTNode *instructions;
    const char *scope_name;
    size_t next_task;
    size_t next_temp;
    int failed;
} RxcpTaskPlan;

typedef struct RxcpConsumedTaskScopes {
    Symbol **symbols;
    size_t count;
    size_t capacity;
} RxcpConsumedTaskScopes;

typedef struct RxcpPendingTaskResult {
    Symbol *symbol;
    char *handle_name;
    const char *result_method;
    ASTNode *source_node;
    int materialized;
} RxcpPendingTaskResult;

typedef struct RxcpParallelPlan {
    RxcpTaskPlan task;
    ASTNode *parallel_node;
    RxcpPendingTaskResult *pending;
    size_t pending_count;
    size_t pending_capacity;
} RxcpParallelPlan;

static ASTNode *task_callable_definition(Symbol *symbol) {
    size_t i;

    if (!symbol || symbol->symbol_type != FUNCTION_SYMBOL) return 0;
    for (i = 0; i < sym_nond(symbol); i++) {
        SymbolNode *link = sym_trnd(symbol, i);
        ASTNode *definition = link ? link->node : 0;
        if (definition && definition->is_task_callable &&
            (definition->node_type == PROCEDURE ||
             definition->node_type == METHOD ||
             definition->node_type == FACTORY)) {
            return definition;
        }
    }
    return 0;
}

static int task_call_definition(ASTNode *node, ASTNode **definition_out) {
    ASTNode *definition;
    ASTNode *enclosing;
    Symbol *symbol;

    if (definition_out) *definition_out = 0;
    if (!node || (node->node_type != FUNCTION && node->node_type != MEMBER_CALL) ||
        !node->symbolNode || !(symbol = node->symbolNode->symbol)) return 0;
    definition = task_callable_definition(symbol);
    if (!definition) return 0;

    /* A task declaration is also the ordinary procedure body executed by a
     * worker.  Direct self-recursion stays in that worker and uses normal Rexx
     * call semantics; it must not submit a child to the bounded task pool and
     * then wait on itself.  Calls to any other task remain task calls and are
     * rejected from a task body by TASK_NESTED_WAIT below. */
    enclosing = ast_proc(node);
    if (enclosing == definition && enclosing->is_task_callable) return 0;
    if (definition_out) *definition_out = definition;
    return 1;
}

static int task_subtree_has_call(ASTNode *node) {
    ASTNode *child;

    if (!node) return 0;
    if (task_call_definition(node, 0)) return 1;
    if (node->node_type == BLOCK_EXPR ||
        node->node_type == PARALLEL_BLOCK_EXPR) return 0;
    if (node->node_type == PROCEDURE || node->node_type == METHOD ||
        node->node_type == FACTORY || node->node_type == MATCH) return 0;
    for (child = node->child; child; child = child->sibling) {
        if (task_subtree_has_call(child)) return 1;
    }
    return 0;
}

static int task_inside_parallel(ASTNode *node) {
    for (node = node ? node->parent : 0; node; node = node->parent) {
        if (node->node_type == PARALLEL_DO ||
            node->node_type == PARALLEL_BLOCK_EXPR) return 1;
        if (node->node_type == PROCEDURE || node->node_type == METHOD ||
            node->node_type == FACTORY) break;
    }
    return 0;
}

static int task_inside_task_callable(ASTNode *node) {
    ASTNode *procedure = ast_proc(node);
    return procedure && procedure->is_task_callable;
}

/* 1 = valid, 0 = wait for type convergence, -1 = diagnosed invalid. */
static int task_scope_expression_status(Context *context,
                                        ASTNode *expression) {
    if (!context || !expression) return -1;
    if (ast_chld(expression, ERROR, 0)) return -1;
    if (expression->value_type == TP_UNKNOWN) return 0;
    if (expression->value_type == TP_OBJECT && expression->value_class &&
        symbol_name_assignable_to(context, expression->value_class,
                                  "concurrency.taskscope")) {
        return 1;
    }
    if (!ast_chld(expression, ERROR, 0)) {
        mknd_err(expression, "PARALLEL_SCOPE_TYPE");
    }
    return -1;
}

static int task_consumed_scope_find(RxcpConsumedTaskScopes *state,
                                    Symbol *symbol) {
    size_t i;

    if (!state || !symbol) return 0;
    for (i = 0; i < state->count; i++) {
        if (state->symbols[i] == symbol) return 1;
    }
    return 0;
}

static void task_consumed_scope_add(RxcpConsumedTaskScopes *state,
                                    Symbol *symbol) {
    Symbol **grown;
    size_t capacity;

    if (!state || !symbol || task_consumed_scope_find(state, symbol)) return;
    if (state->count == state->capacity) {
        capacity = state->capacity ? state->capacity * 2u : 4u;
        grown = (Symbol **)realloc(state->symbols,
                                   capacity * sizeof(*state->symbols));
        if (!grown) return;
        state->symbols = grown;
        state->capacity = capacity;
    }
    state->symbols[state->count++] = symbol;
}

static void task_consumed_scope_remove(RxcpConsumedTaskScopes *state,
                                       Symbol *symbol) {
    size_t i;

    if (!state || !symbol) return;
    for (i = 0; i < state->count; i++) {
        if (state->symbols[i] == symbol) {
            state->symbols[i] = state->symbols[state->count - 1u];
            state->count--;
            return;
        }
    }
}

static void task_validate_scope_reuse_node(ASTNode *node,
                                           RxcpConsumedTaskScopes *state) {
    ASTNode *child;
    ASTNode *using_expression;

    if (!node || !state) return;
    if (node->node_type == PROCEDURE || node->node_type == METHOD ||
        node->node_type == FACTORY || node->node_type == MATCH) return;

    if (node->node_type == PARALLEL_DO ||
        node->node_type == PARALLEL_BLOCK_EXPR) {
        using_expression = node->child;
        if (using_expression && using_expression->node_type != INSTRUCTIONS &&
            using_expression->symbolNode) {
            Symbol *symbol = using_expression->symbolNode->symbol;
            if (task_consumed_scope_find(state, symbol)) {
                if (!ast_chld(using_expression, ERROR, 0)) {
                    mknd_err(using_expression, "PARALLEL_SCOPE_REUSED");
                }
            } else {
                task_consumed_scope_add(state, symbol);
            }
        }
    }

    for (child = node->child; child; child = child->sibling) {
        task_validate_scope_reuse_node(child, state);
    }

    if (node->node_type == ASSIGN) {
        ASTNode *target = ast_chdn(node, 0);
        if (target && target->symbolNode) {
            task_consumed_scope_remove(state, target->symbolNode->symbol);
        }
    }
}

static void task_validate_scope_reuse_callables(ASTNode *node) {
    ASTNode *child;

    if (!node) return;
    if (node->node_type == PROCEDURE || node->node_type == METHOD ||
        node->node_type == FACTORY || node->node_type == MATCH) {
        RxcpConsumedTaskScopes state = {0};
        for (child = node->child; child; child = child->sibling) {
            task_validate_scope_reuse_node(child, &state);
        }
        free(state.symbols);
        return;
    }
    for (child = node->child; child; child = child->sibling) {
        task_validate_scope_reuse_callables(child);
    }
}

void rxcp_validate_task_scope_reuse(Context *context) {
    if (context) task_validate_scope_reuse_callables(context->ast);
}

static char *task_plan_name(const char *kind, ASTNode *root, size_t index) {
    int length;
    char *name;

    length = snprintf(0, 0, "__task_%s_%d_%zu", kind,
                      root ? root->node_number : 0, index);
    if (length < 0) return 0;
    name = (char *)malloc((size_t)length + 1u);
    if (!name) return 0;
    snprintf(name, (size_t)length + 1u, "__task_%s_%d_%zu", kind,
             root ? root->node_number : 0, index);
    return name;
}

static ASTNode *task_named_factory(Context *context,
                                   ASTNode *anchor,
                                   const char *class_name,
                                   const char *factory_name,
                                   ASTNode **arguments,
                                   size_t argument_count) {
    ASTNode *node = rxcp_remap_create_factory_call(
            context, anchor, class_name, arguments, argument_count);
    ASTNode *association;

    if (!node || !factory_name) return node;
    association = rxcp_remap_create_named_ref(
            context, anchor, VAR_SYMBOL, factory_name);
    if (!association) return 0;
    node->association = association;
    return node;
}

static ASTNode *task_named_ref(RxcpTaskPlan *plan,
                               ASTNode *anchor,
                               const char *name,
                               NodeType type) {
    ASTNode *node = rxcp_remap_create_named_ref(
            plan->context, anchor, type, name);
    if (node) node->scope = plan->scope;
    return node;
}

static ASTNode *task_member_call(RxcpTaskPlan *plan,
                                 ASTNode *anchor,
                                 const char *receiver_name,
                                 const char *method,
                                 ASTNode **arguments,
                                 size_t argument_count) {
    ASTNode *receiver = task_named_ref(
            plan, anchor, receiver_name, VAR_SYMBOL);
    ASTNode *call;

    if (!receiver) return 0;
    call = rxcp_remap_create_member_call(
            plan->context, anchor, receiver, method,
            arguments, argument_count);
    if (call) call->scope = plan->scope;
    return call;
}

static int task_append_assignment(RxcpTaskPlan *plan,
                                  ASTNode *anchor,
                                  const char *name,
                                  ASTNode *value) {
    ASTNode *assignment;

    if (!plan || !anchor || !name || !value) return 0;
    assignment = rxcp_remap_create_named_assignment(
            plan->context, anchor, name, value);
    if (!assignment) return 0;
    assignment->scope = plan->scope;
    add_ast(plan->instructions, assignment);
    return 1;
}

static int task_append_member_statement(RxcpTaskPlan *plan,
                                        ASTNode *anchor,
                                        const char *receiver_name,
                                        const char *method) {
    ASTNode *call = task_member_call(
            plan, anchor, receiver_name, method, 0, 0);
    ASTNode *statement;

    if (!call) return 0;
    statement = rxcp_remap_create_call_statement(
            plan->context, anchor, call);
    if (!statement) return 0;
    statement->scope = plan->scope;
    add_ast(plan->instructions, statement);
    return 1;
}

static const char *task_argument_method(ASTNode *argument) {
    if (!argument || argument->value_dims || argument->value_reference_type != TP_UNKNOWN) {
        return 0;
    }
    switch (argument->value_type) {
        case TP_BOOLEAN: return "add_boolean";
        case TP_INTEGER: return "add_integer";
        case TP_STRING: return "add_string";
        case TP_BINARY: return "add_binary";
        default: return 0;
    }
}

static const char *task_result_method(ASTNode *call) {
    if (!call || call->value_dims || call->value_reference_type != TP_UNKNOWN) {
        return 0;
    }
    switch (call->value_type) {
        case TP_BOOLEAN:
        case TP_INTEGER: return "result_integer";
        case TP_STRING: return "result_string";
        case TP_BINARY: return "result_binary";
        default: return 0;
    }
}

static int task_formals_transferable(ASTNode *definition, ASTNode *diagnostic_site) {
    ASTNode *args;
    ASTNode *arg;

    if (!definition) return 0;
    args = ast_chld(definition, ARGS, 0);
    for (arg = args ? args->child : 0; arg; arg = arg->sibling) {
        ASTNode *formal = arg->child;
        if (formal && (formal->is_ref_arg || formal->is_varg ||
                       formal->value_reference_type != TP_UNKNOWN)) {
            if (!ast_chld(diagnostic_site, ERROR, 0)) {
                mknd_err(diagnostic_site, formal->is_ref_arg
                        ? "TASK_EXPOSED_ARGUMENT" : "TASK_REFERENCE_TYPE");
            }
            return 0;
        }
    }
    return 1;
}

static ASTNode *task_target_reference(RxcpTaskPlan *plan,
                                      ASTNode *call) {
    ASTNode *target;
    ASTNode *reference;
    Symbol *symbol = call && call->symbolNode ? call->symbolNode->symbol : 0;
    char *name;

    if (!plan || !call || !symbol) return 0;
    name = (char *)malloc(call->node_string_length + 1u);
    if (!name) return 0;
    memcpy(name, call->node_string, call->node_string_length);
    name[call->node_string_length] = 0;
    reference = rxcp_remap_create_named_ref(
            plan->context, call, FUNC_SYMBOL, name);
    free(name);
    if (!reference) return 0;
    reference->scope = plan->scope;
    sym_adnd(symbol, reference, 1, 0);
    target = ast_f(plan->context, TASK_TARGET, call->token);
    if (!target) return 0;
    target->scope = plan->scope;
    rxcp_remap_anchor_synthetic(target, call);
    add_ast(target, reference);
    return target;
}

static int task_process_expression(RxcpTaskPlan *plan, ASTNode *node);
static int task_lower_implicit_expression(Context *context, ASTNode *root);
static ASTNode *task_statement_expression(ASTNode *node);
static int task_isolate_shared_short_circuit_rhs(RxcpTaskPlan *plan,
                                                  ASTNode *node);

static int task_isolate_short_circuit_rhs(Context *context, ASTNode *node) {
    ASTNode *child;

    if (!context || !node || node->node_type == BLOCK_EXPR) return 1;
    for (child = node->child; child; child = child->sibling) {
        if (!task_isolate_short_circuit_rhs(context, child)) return 0;
    }
    if (node->node_type == OP_AND || node->node_type == OP_OR) {
        ASTNode *right = ast_chdn(node, 1);
        if (right && right->node_type != BLOCK_EXPR &&
            task_subtree_has_call(right) &&
            !task_lower_implicit_expression(context, right)) {
            return 0;
        }
    }
    return 1;
}

static int task_capture_ordinary_call(RxcpTaskPlan *plan,
                                      ASTNode *call,
                                      ASTNode **replacement_out) {
    char *name;
    ASTNode *replacement;
    ASTNode *assignment;

    if (replacement_out) *replacement_out = 0;
    name = task_plan_name("value", call, plan->next_temp++);
    if (!name) return 0;
    replacement = task_named_ref(plan, call, name, VAR_SYMBOL);
    if (!replacement) {
        free(name);
        return 0;
    }
    rxcp_remap_copy_node_semantics(replacement, call);
    ast_rpl(call, replacement);
    assignment = rxcp_remap_create_named_assignment(
            plan->context, call, name, call);
    free(name);
    if (!assignment) return 0;
    assignment->scope = plan->scope;
    add_ast(plan->instructions, assignment);
    if (replacement_out) *replacement_out = replacement;
    return 1;
}

static int task_lower_call(RxcpTaskPlan *plan,
                           ASTNode *call,
                           ASTNode *definition,
                           int materialize_result,
                           char **handle_name_out,
                           const char **result_method_out,
                           ASTNode **replacement_out) {
    size_t task_index = plan->next_task++;
    char *arguments_name = task_plan_name("arguments", call, task_index);
    char *handle_name = task_plan_name("handle", call, task_index);
    ASTNode *empty_arguments;
    ASTNode *argument;
    ASTNode *next;
    ASTNode *target;
    ASTNode *submit_arguments[2];
    ASTNode *submit_call;
    ASTNode *result_argument;
    ASTNode *result_call;
    const char *result_method;

    if (!arguments_name || !handle_name || !task_formals_transferable(definition, call)) {
        free(arguments_name);
        free(handle_name);
        return 0;
    }
    if (handle_name_out) *handle_name_out = 0;
    if (result_method_out) *result_method_out = 0;
    if (replacement_out) *replacement_out = 0;
    result_method = task_result_method(call);
    if (materialize_result && !result_method) {
        if (!ast_chld(call, ERROR, 0)) mknd_err(call, "TASK_NONTRANSFERABLE_TYPE");
        free(arguments_name);
        free(handle_name);
        return 0;
    }

    empty_arguments = task_named_factory(
            plan->context, call, "concurrency.taskarguments", "empty", 0, 0);
    if (!empty_arguments ||
        !task_append_assignment(plan, call, arguments_name, empty_arguments)) {
        free(arguments_name);
        free(handle_name);
        return 0;
    }

    argument = call->child;
    if (definition->node_type == METHOD) {
        ASTNode *encoded_receiver;
        ASTNode *method_argument[1];
        ASTNode *add_call;

        if (!argument) {
            if (!ast_chld(call, ERROR, 0)) {
                mknd_err(call, "TASK_NONTRANSFERABLE_RECEIVER");
            }
            free(arguments_name);
            free(handle_name);
            return 0;
        }
        next = argument->sibling;
        ast_del(argument);
        encoded_receiver = rxcp_remap_create_member_call(
                plan->context, call, argument, "to_channel", 0, 0);
        method_argument[0] = encoded_receiver;
        add_call = encoded_receiver
                ? task_member_call(plan, call, arguments_name,
                                   "add_value", method_argument, 1)
                : 0;
        if (!add_call ||
            !task_append_assignment(plan, call, arguments_name, add_call)) {
            free(arguments_name);
            free(handle_name);
            return 0;
        }
        argument = next;
    }
    while (argument) {
        const char *method;
        ASTNode *method_argument[1];
        ASTNode *add_call;

        next = argument->sibling;
        if (argument->node_type == NOVAL) {
            ast_del(argument);
            argument = next;
            continue;
        }
        method = task_argument_method(argument);
        if (!method) {
            if (!ast_chld(call, ERROR, 0)) mknd_err(call, "TASK_NONTRANSFERABLE_TYPE");
            free(arguments_name);
            free(handle_name);
            return 0;
        }
        ast_del(argument);
        method_argument[0] = argument;
        add_call = task_member_call(
                plan, call, arguments_name, method, method_argument, 1);
        if (!add_call ||
            !task_append_assignment(plan, call, arguments_name, add_call)) {
            free(arguments_name);
            free(handle_name);
            return 0;
        }
        argument = next;
    }

    target = task_target_reference(plan, call);
    submit_arguments[0] = target;
    submit_arguments[1] = task_named_ref(
            plan, call, arguments_name, VAR_SYMBOL);
    if (!target || !submit_arguments[1]) {
        free(arguments_name);
        free(handle_name);
        return 0;
    }
    submit_call = task_member_call(
            plan, call, plan->scope_name, "submit_arguments",
            submit_arguments, 2);
    if (!submit_call ||
        !task_append_assignment(plan, call, handle_name, submit_call)) {
        free(arguments_name);
        free(handle_name);
        return 0;
    }

    if (materialize_result) {
        result_argument = task_named_ref(plan, call, handle_name, VAR_SYMBOL);
        if (!result_argument) {
            free(arguments_name);
            free(handle_name);
            return 0;
        }
        {
            ASTNode *arguments[1];
            arguments[0] = result_argument;
            result_call = task_member_call(
                    plan, call, plan->scope_name, result_method, arguments, 1);
        }
        if (!result_call) {
            free(arguments_name);
            free(handle_name);
            return 0;
        }
        rxcp_remap_copy_node_semantics(result_call, call);
        ast_rpl(call, result_call);
        if (replacement_out) *replacement_out = result_call;
    }
    if (call->symbolNode) sym_dno(call->symbolNode->symbol, call);
    if (handle_name_out) {
        *handle_name_out = handle_name;
        handle_name = 0;
    }
    if (result_method_out) *result_method_out = result_method;
    free(arguments_name);
    free(handle_name);
    return 1;
}

static int task_process_non_task_expression(RxcpTaskPlan *plan,
                                            ASTNode **node_io) {
    ASTNode *child;
    ASTNode *next;
    ASTNode *node;
    ASTNode *replacement = 0;

    if (!plan || !node_io || !(node = *node_io)) return 0;
    if (node->node_type == BLOCK_EXPR ||
        node->node_type == PARALLEL_BLOCK_EXPR) return 1;
    child = node->child;
    while (child) {
        next = child->sibling;
        {
            ASTNode *current = child;
            if (!task_process_non_task_expression(plan, &current)) return 0;
        }
        child = next;
    }

    if (node->node_type == FUNCTION || node->node_type == MEMBER_CALL ||
        node->node_type == FACTORY_CALL) {
        if (!task_capture_ordinary_call(plan, node, &replacement)) return 0;
        *node_io = replacement;
    }
    return 1;
}

/*
 * One readiness pass submits every leaf task whose task-valued inputs are
 * already represented by ordinary result expressions.  A task with a nested
 * task input is deliberately held for the next pass, while the same pass may
 * continue into later independent expression branches.  This gives
 * `third(first()) + second()` the required shape: first and second are
 * submitted before first is materialized and third is submitted.
 *
 * Ordinary calls are captured only where ordinary evaluation has reached
 * them.  In particular, calls after an unresolved task argument remain behind
 * that argument; ordinary work in a later operator branch may proceed while a
 * previously submitted task runs.
 */
static int task_schedule_ready_calls(RxcpTaskPlan *plan,
                                     ASTNode **node_io,
                                     int ordinary_blocked,
                                     int *scheduled,
                                     int *remaining) {
    ASTNode *node;
    ASTNode *definition = 0;
    ASTNode *child;
    ASTNode *next;
    ASTNode *replacement = 0;
    int is_task;
    int has_task_input = 0;
    int child_ordinary_blocked;

    if (!plan || !node_io || !(node = *node_io) ||
        !scheduled || !remaining) return 0;
    if (node->node_type == BLOCK_EXPR ||
        node->node_type == PARALLEL_BLOCK_EXPR) return 1;

    is_task = task_call_definition(node, &definition);
    for (child = node->child; child; child = child->sibling) {
        if (task_subtree_has_call(child)) {
            has_task_input = 1;
            break;
        }
    }

    if (is_task && !has_task_input) {
        child = node->child;
        while (child) {
            ASTNode *current = child;
            next = child->sibling;
            if (!task_process_non_task_expression(plan, &current)) return 0;
            child = next;
        }
        if (!task_lower_call(plan, node, definition, 1, 0, 0,
                             &replacement)) return 0;
        *node_io = replacement;
        (*scheduled)++;
        return 1;
    }

    if (is_task) {
        child_ordinary_blocked = ordinary_blocked;
        child = node->child;
        while (child) {
            ASTNode *current = child;
            int child_had_task = task_subtree_has_call(child);

            next = child->sibling;
            if (!task_schedule_ready_calls(plan, &current,
                                           child_ordinary_blocked,
                                           scheduled, remaining)) return 0;
            if (child_had_task) child_ordinary_blocked = 1;
            child = next;
        }
        *remaining = 1;
        return 1;
    }

    child = node->child;
    while (child) {
        ASTNode *current = child;
        next = child->sibling;
        if (!task_schedule_ready_calls(plan, &current, ordinary_blocked,
                                       scheduled, remaining)) return 0;
        child = next;
    }

    if (!ordinary_blocked && !has_task_input &&
        (node->node_type == FUNCTION || node->node_type == MEMBER_CALL ||
         node->node_type == FACTORY_CALL)) {
        if (!task_capture_ordinary_call(plan, node, &replacement)) return 0;
        *node_io = replacement;
    }
    return 1;
}

static int task_process_expression(RxcpTaskPlan *plan, ASTNode *node) {
    ASTNode *root = node;
    int scheduled;
    int remaining;

    if (!plan || !root) return 0;
    if (!task_isolate_shared_short_circuit_rhs(plan, root)) return 0;
    do {
        scheduled = 0;
        remaining = 0;
        if (!task_schedule_ready_calls(plan, &root, 0,
                                       &scheduled, &remaining)) return 0;
        if (remaining && !scheduled) {
            if (!ast_chld(root, ERROR, 0)) mknd_err(root, "TASK_RESULT_CYCLE");
            return 0;
        }
    } while (remaining);

    return task_process_non_task_expression(plan, &root);
}

/* Preserve Rexx short-circuiting without opening a second task scope.  The
 * right operand becomes an ordinary conditional block expression whose
 * generated submissions still reference the enclosing parallel scope. */
static int task_isolate_shared_short_circuit_rhs(RxcpTaskPlan *plan,
                                                  ASTNode *node) {
    ASTNode *child;
    ASTNode *next;

    if (!plan || !node) return 0;
    if (node->node_type == BLOCK_EXPR ||
        node->node_type == PARALLEL_BLOCK_EXPR) return 1;

    child = node->child;
    while (child) {
        next = child->sibling;
        if (!task_isolate_shared_short_circuit_rhs(plan, child)) return 0;
        child = next;
    }

    if (node->node_type == OP_AND || node->node_type == OP_OR) {
        ASTNode *right = ast_chdn(node, 1);

        if (right && right->node_type != BLOCK_EXPR &&
            task_subtree_has_call(right)) {
            ASTNode *block;
            ASTNode *instructions;
            ASTNode *leave;
            Scope *scope;
            RxcpTaskPlan nested = *plan;

            block = rxcp_remap_create_block_expr(
                    plan->context, plan->scope, right, 0,
                    &scope, &instructions);
            if (!block || !scope || !instructions) return 0;
            ast_rpl(right, block);
            leave = rxcp_remap_create_leave_with(
                    plan->context, scope, right, block, right);
            if (!leave) return 0;

            nested.scope = scope;
            nested.instructions = instructions;
            if (!task_process_expression(&nested, right)) return 0;
            add_ast(instructions, leave);
            plan->next_task = nested.next_task;
            plan->next_temp = nested.next_temp;
        }
    }
    return 1;
}

static int task_lower_implicit_call(Context *context,
                                    ASTNode *statement,
                                    ASTNode *call,
                                    ASTNode *definition) {
    ASTNode *instructions;
    ASTNode *scope_factory;
    RxcpTaskPlan plan;
    char *scope_name;

    if (!context || !statement || !call || !statement->parent || !statement->scope) return 0;
    scope_name = task_plan_name("scope", call, 0);
    if (!scope_name) return 0;
    instructions = ast_f(context, INSTRUCTIONS, statement->token);
    if (!instructions) {
        free(scope_name);
        return 0;
    }
    rxcp_remap_anchor_synthetic(instructions, statement);
    instructions->scope = statement->scope;
    instructions->inherit_parent_scope = 1;
    instructions->inherit_parent_reg_scope = 1;

    ast_rpl(statement, instructions);
    memset(&plan, 0, sizeof(plan));
    plan.context = context;
    plan.scope = statement->scope;
    plan.instructions = instructions;
    plan.scope_name = scope_name;

    scope_factory = task_named_factory(
            context, call, "concurrency.taskscope", "default", 0, 0);
    if (!scope_factory ||
        !task_append_assignment(&plan, call, scope_name, scope_factory) ||
        !task_lower_call(&plan, call, definition, 0, 0, 0, 0) ||
        !task_append_member_statement(&plan, call, scope_name, "finish")) {
        free(scope_name);
        return 0;
    }
    free(scope_name);
    context->changed_flags |= FLAG_VAL_TRANS | FLAG_ORCH | FLAG_VAL_SYM | FLAG_VAL_TYPE;
    return 1;
}

static RxcpPendingTaskResult *task_pending_find(RxcpParallelPlan *plan,
                                                Symbol *symbol) {
    size_t i;

    if (!plan || !symbol) return 0;
    for (i = 0; i < plan->pending_count; i++) {
        if (plan->pending[i].symbol == symbol && !plan->pending[i].materialized) {
            return &plan->pending[i];
        }
    }
    return 0;
}

static int task_pending_add(RxcpParallelPlan *plan,
                            Symbol *symbol,
                            char *handle_name,
                            const char *result_method,
                            ASTNode *source_node) {
    RxcpPendingTaskResult *pending;

    if (!plan || !symbol || !handle_name || !result_method) return 0;
    if (plan->pending_count == plan->pending_capacity) {
        size_t capacity = plan->pending_capacity ? plan->pending_capacity * 2u : 8u;
        pending = (RxcpPendingTaskResult *)realloc(
                plan->pending, capacity * sizeof(*pending));
        if (!pending) return 0;
        plan->pending = pending;
        plan->pending_capacity = capacity;
    }
    pending = &plan->pending[plan->pending_count++];
    memset(pending, 0, sizeof(*pending));
    pending->symbol = symbol;
    pending->handle_name = handle_name;
    pending->result_method = result_method;
    pending->source_node = source_node;
    return 1;
}

static int task_node_uses_symbol(ASTNode *node,
                                 Symbol *symbol,
                                 int reads,
                                 int writes) {
    ASTNode *child;

    if (!node || !symbol) return 0;
    if (node->symbolNode && node->symbolNode->symbol == symbol &&
        ((reads && node->symbolNode->readUsage) ||
         (writes && node->symbolNode->writeUsage))) return 1;
    if (node->node_type == PROCEDURE || node->node_type == METHOD ||
        node->node_type == FACTORY || node->node_type == MATCH) return 0;
    for (child = node->child; child; child = child->sibling) {
        if (task_node_uses_symbol(child, symbol, reads, writes)) return 1;
    }
    return 0;
}

static int task_node_forbids_pending_symbol(ASTNode *node,
                                            Symbol *symbol) {
    ASTNode *child;

    if (!node || !symbol) return 0;
    if (node->node_type == RETURN &&
        task_node_uses_symbol(node, symbol, 1, 0)) return 1;
    if ((node->node_type == OP_REFERENCE ||
         node->node_type == OP_DEREFERENCE ||
         node->node_type == EXPOSED) &&
        task_node_uses_symbol(node, symbol, 1, 1)) return 1;
    if (node->node_type == VAR_REFERENCE && node->symbolNode &&
        node->symbolNode->symbol == symbol) return 1;
    if (node->node_type == PROCEDURE || node->node_type == METHOD ||
        node->node_type == FACTORY || node->node_type == MATCH) return 0;
    for (child = node->child; child; child = child->sibling) {
        if (task_node_forbids_pending_symbol(child, symbol)) return 1;
    }
    return 0;
}

static int task_pending_materialize(RxcpParallelPlan *plan,
                                    RxcpPendingTaskResult *pending,
                                    ASTNode *anchor) {
    ASTNode *handle;
    ASTNode *arguments[1];
    ASTNode *result;
    ASTNode *assignment;

    if (!plan || !pending || pending->materialized) return 1;
    handle = task_named_ref(&plan->task, anchor, pending->handle_name, VAR_SYMBOL);
    if (!handle) return 0;
    arguments[0] = handle;
    result = task_member_call(&plan->task, anchor, plan->task.scope_name,
                              pending->result_method, arguments, 1);
    if (!result) return 0;
    assignment = rxcp_remap_create_assignment_to_symbol(
            plan->task.context, plan->task.scope, anchor, anchor,
            pending->symbol, result);
    if (!assignment) return 0;
    add_ast(plan->task.instructions, assignment);
    pending->materialized = 1;
    return 1;
}

static int task_pending_before_statement(RxcpParallelPlan *plan,
                                         ASTNode *statement) {
    size_t i;

    if (!plan || !statement) return 0;
    for (i = 0; i < plan->pending_count; i++) {
        RxcpPendingTaskResult *pending = &plan->pending[i];
        int writes;
        int reads;

        if (pending->materialized) continue;
        writes = task_node_uses_symbol(statement, pending->symbol, 0, 1);
        reads = task_node_uses_symbol(statement, pending->symbol, 1, 0);
        if (writes || task_node_forbids_pending_symbol(statement,
                                                       pending->symbol)) {
            if (!ast_chld(statement, ERROR, 0)) {
                mknd_err(statement, "PENDING_TASK_RESULT_MUTATION");
            }
            return 0;
        }
        if (reads && !task_pending_materialize(plan, pending, statement)) return 0;
    }
    return 1;
}

static int task_direct_pending_assignment(RxcpParallelPlan *plan,
                                          ASTNode *statement) {
    ASTNode *target;
    ASTNode *call;
    ASTNode *definition = 0;
    ASTNode *argument;
    ASTNode *next;
    Symbol *symbol;
    char *handle_name = 0;
    const char *result_method = 0;

    if (!plan || !statement || statement->node_type != ASSIGN) return 0;
    target = ast_chdn(statement, 0);
    call = ast_chdn(statement, 1);
    if (!target || target->node_type != VAR_TARGET || !target->symbolNode ||
        !call || !task_call_definition(call, &definition)) return 0;
    symbol = target->symbolNode->symbol;
    if (!symbol) return 0;
    if (task_pending_find(plan, symbol)) {
        if (!ast_chld(statement, ERROR, 0)) {
            mknd_err(statement, "PENDING_TASK_RESULT_MUTATION");
        }
        return -1;
    }
    if (!task_result_method(call)) return 0;

    argument = call->child;
    while (argument) {
        next = argument->sibling;
        if (!task_process_expression(&plan->task, argument)) return -1;
        argument = next;
    }
    if (!task_lower_call(&plan->task, call, definition, 0,
                         &handle_name, &result_method, 0) ||
        !task_pending_add(plan, symbol, handle_name, result_method, statement)) {
        free(handle_name);
        return -1;
    }
    rxcp_remap_disconnect_subtree_symbols(statement);
    return 1;
}

static int task_process_parallel_statement(RxcpParallelPlan *plan,
                                           ASTNode *statement,
                                           int allow_pending);

static int task_parallel_contains_node(ASTNode *parallel, ASTNode *node) {
    for (; node; node = node->parent) {
        if (node == parallel) return 1;
    }
    return 0;
}

static int task_parallel_statement_escapes(RxcpParallelPlan *plan,
                                           ASTNode *statement) {
    ASTNode *call;

    if (!plan || !statement) return 0;
    if (statement->node_type == RETURN || statement->node_type == EXIT) return 1;
    if (statement->node_type == CALL &&
        (call = ast_chdn(statement, 0)) != 0 &&
        call->node_type == FUNCTION && call->node_string &&
        call->node_string_length == 5 &&
        memcmp(call->node_string, "_exit", 5) == 0) return 1;
    if (statement->node_type == LEAVE || statement->node_type == LEAVE_WITH) {
        return !task_parallel_contains_node(plan->parallel_node,
                                            statement->association);
    }
    return 0;
}

static int task_parallel_result_leave(RxcpParallelPlan *plan,
                                      ASTNode *statement) {
    return plan && statement &&
           statement->node_type == LEAVE_WITH &&
           statement->association == plan->parallel_node &&
           plan->parallel_node->node_type == PARALLEL_BLOCK_EXPR;
}

/* Task lowering can run on the first type-convergence iteration, before an
 * otherwise untyped LEAVE WITH has acquired its ordinary BLOCK_EXPR
 * association.  Bind syntactically enclosed leaves here, while leaving nested
 * block expressions to their own association/lowering pass. */
static void task_bind_parallel_result_leaves(ASTNode *node,
                                             ASTNode *parallel) {
    ASTNode *child;

    if (!node || !parallel) return;
    if (node != parallel &&
        (node->node_type == BLOCK_EXPR ||
         node->node_type == PARALLEL_BLOCK_EXPR)) return;
    if (node->node_type == LEAVE_WITH && !node->association) {
        node->association = parallel;
    }
    for (child = node->child; child; child = child->sibling) {
        task_bind_parallel_result_leaves(child, parallel);
    }
}

static int task_append_parallel_abort(RxcpParallelPlan *plan,
                                      ASTNode *anchor,
                                      const char *message) {
    ASTNode *text;
    ASTNode *reason_args[1];
    ASTNode *reason;
    ASTNode *abort_args[1];
    ASTNode *abort_call;
    ASTNode *statement;

    if (!plan || !anchor || !message) return 0;
    text = rxcp_remap_create_string_constant(plan->task.context, anchor, message);
    reason_args[0] = text;
    reason = task_named_factory(plan->task.context, anchor,
                                "concurrency.channelvalue", "string_value",
                                reason_args, 1);
    abort_args[0] = reason;
    abort_call = task_member_call(&plan->task, anchor, plan->task.scope_name,
                                  "abort", abort_args, 1);
    statement = rxcp_remap_create_call_statement(
            plan->task.context, anchor, abort_call);
    if (!text || !reason || !abort_call || !statement) return 0;
    statement->scope = plan->task.scope;
    add_ast(plan->task.instructions, statement);
    return 1;
}

static int task_process_parallel_instruction_list(RxcpParallelPlan *plan,
                                                  ASTNode *instructions,
                                                  int allow_pending) {
    ASTNode *statement;
    ASTNode *next;
    ASTNode *saved_instructions;

    if (!plan || !instructions || instructions->node_type != INSTRUCTIONS) return 0;
    saved_instructions = plan->task.instructions;
    plan->task.instructions = instructions;
    statement = instructions->child;
    instructions->child = 0;
    while (statement) {
        next = statement->sibling;
        statement->parent = 0;
        statement->sibling = 0;
        if (!task_process_parallel_statement(plan, statement, allow_pending)) {
            plan->task.instructions = saved_instructions;
            return 0;
        }
        statement = next;
    }
    plan->task.instructions = saved_instructions;
    return 1;
}

static int task_process_parallel_nested_lists(RxcpParallelPlan *plan,
                                              ASTNode *node) {
    ASTNode *child;
    ASTNode *next;

    if (!plan || !node) return 0;
    child = node->child;
    while (child) {
        next = child->sibling;
        if (child->node_type == INSTRUCTIONS) {
            if (!task_process_parallel_instruction_list(plan, child, 0)) return 0;
        } else if (child->node_type != BLOCK_EXPR &&
                   child->node_type != PARALLEL_BLOCK_EXPR &&
                   !task_process_parallel_nested_lists(plan, child)) {
            return 0;
        }
        child = next;
    }
    return 1;
}

static int task_process_parallel_statement(RxcpParallelPlan *plan,
                                           ASTNode *statement,
                                           int allow_pending) {
    ASTNode *expression;
    ASTNode *definition = 0;
    int pending_result;

    if (!plan || !statement) return 0;
    if (!task_pending_before_statement(plan, statement)) {
        add_ast(plan->task.instructions, statement);
        return 1;
    }
    if (allow_pending) {
        pending_result = task_direct_pending_assignment(plan, statement);
        if (pending_result < 0) return 0;
        if (pending_result > 0) return 1;
    }

    expression = task_statement_expression(statement);
    if (expression && task_subtree_has_call(expression)) {
        if (statement->node_type == CALL && task_call_definition(expression, &definition)) {
            ASTNode *argument = expression->child;
            ASTNode *next;
            while (argument) {
                next = argument->sibling;
                if (!task_process_expression(&plan->task, argument)) return 0;
                argument = next;
            }
            if (!task_lower_call(&plan->task, expression, definition,
                                 0, 0, 0, 0)) return 0;
            rxcp_remap_disconnect_subtree_symbols(statement);
            return 1;
        }
        if (statement->node_type == CALL) {
            ASTNode *argument = expression->child;
            ASTNode *next;
            while (argument) {
                next = argument->sibling;
                if (!task_process_expression(&plan->task, argument)) return 0;
                argument = next;
            }
        } else if (!task_process_expression(&plan->task, expression)) {
            return 0;
        }
    }
    if (!task_process_parallel_nested_lists(plan, statement)) return 0;
    if (task_parallel_result_leave(plan, statement) &&
        !task_append_member_statement(&plan->task, statement,
                                      plan->task.scope_name, "finish")) {
        return 0;
    }
    if (task_parallel_statement_escapes(plan, statement) &&
        !task_append_parallel_abort(plan, statement,
                                    "parallel controller control exit")) return 0;
    add_ast(plan->task.instructions, statement);
    return 1;
}

static int task_materialize_all_pending(RxcpParallelPlan *plan,
                                        ASTNode *anchor) {
    size_t i;

    if (!plan) return 0;
    for (i = 0; i < plan->pending_count; i++) {
        if (!task_pending_materialize(plan, &plan->pending[i], anchor)) return 0;
    }
    return 1;
}

static ASTNode *task_parallel_signal_handler(RxcpParallelPlan *plan,
                                             ASTNode *anchor) {
    ASTNode *handler;
    ASTNode *names;
    ASTNode *binding;
    ASTNode *binding_target;
    ASTNode *binding_type;
    ASTNode *instructions;
    ASTNode *reason_text;
    ASTNode *reason;
    ASTNode *reason_args[1];
    ASTNode *abort_args[1];
    ASTNode *abort_call;
    ASTNode *abort_statement;
    ASTNode *name_receiver;
    ASTNode *message_receiver;
    ASTNode *name_call;
    ASTNode *message_call;
    ASTNode *raise;
    char *problem_name;

    if (!plan || !anchor) return 0;
    problem_name = task_plan_name("problem", anchor, 0);
    if (!problem_name) return 0;
    handler = ast_f(plan->task.context, SIGNAL_HANDLER, anchor->token);
    names = ast_f(plan->task.context, SIGNAL_NAMES, anchor->token);
    binding = ast_f(plan->task.context, DEFINE, anchor->token);
    binding_target = rxcp_remap_create_named_ref(
            plan->task.context, anchor, VAR_TARGET, problem_name);
    binding_type = rxcp_remap_create_class_type(
            plan->task.context, anchor, ".signal");
    instructions = ast_f(plan->task.context, INSTRUCTIONS, anchor->token);
    if (!handler || !names || !binding || !binding_target ||
        !binding_type || !instructions) {
        free(problem_name);
        return 0;
    }
    handler->is_compiler_added = 1;
    names->is_compiler_added = 1;
    binding->is_compiler_added = 1;
    instructions->is_compiler_added = 1;
    handler->scope = plan->task.scope;
    names->scope = plan->task.scope;
    binding->scope = plan->task.scope;
    binding_target->scope = plan->task.scope;
    binding_type->scope = plan->task.scope;
    instructions->scope = plan->task.scope;
    instructions->inherit_parent_scope = 1;
    instructions->inherit_parent_reg_scope = 1;
    add_ast(binding, binding_target);
    add_ast(binding, binding_type);

    reason_text = rxcp_remap_create_string_constant(
            plan->task.context, anchor, "parallel controller signal");
    reason_args[0] = reason_text;
    reason = task_named_factory(plan->task.context, anchor,
                                "concurrency.channelvalue", "string_value",
                                reason_args, 1);
    abort_args[0] = reason;
    abort_call = task_member_call(&plan->task, anchor, plan->task.scope_name,
                                  "abort", abort_args, 1);
    abort_statement = rxcp_remap_create_call_statement(
            plan->task.context, anchor, abort_call);
    if (!reason_text || !reason || !abort_call || !abort_statement) {
        free(problem_name);
        return 0;
    }
    abort_statement->scope = plan->task.scope;
    add_ast(instructions, abort_statement);

    name_receiver = task_named_ref(&plan->task, anchor, problem_name, VAR_SYMBOL);
    message_receiver = task_named_ref(&plan->task, anchor, problem_name, VAR_SYMBOL);
    name_call = rxcp_remap_create_member_call(
            plan->task.context, anchor, name_receiver, "name", 0, 0);
    message_call = rxcp_remap_create_member_call(
            plan->task.context, anchor, message_receiver, "message", 0, 0);
    if (name_call) name_call->scope = plan->task.scope;
    if (message_call) message_call->scope = plan->task.scope;
    raise = rxcp_remap_create_assembler_instr(
            plan->task.context, plan->task.scope, anchor, "signal",
            name_call, message_call, 0);
    if (!name_receiver || !message_receiver || !name_call ||
        !message_call || !raise) {
        free(problem_name);
        return 0;
    }
    raise->is_compiler_added = 1;
    add_ast(instructions, raise);
    add_ast(handler, names);
    add_ast(handler, binding);
    add_ast(handler, instructions);
    free(problem_name);
    return handler;
}

static int task_lower_parallel_do(Context *context, ASTNode *node) {
    ASTNode *using_expression = 0;
    ASTNode *source_instructions;
    ASTNode *outer;
    ASTNode *protected_block;
    ASTNode *scope_value;
    ASTNode *handler;
    Scope *scope;
    RxcpParallelPlan plan;
    char *scope_name;
    size_t i;

    if (!context || !node || node->node_type != PARALLEL_DO || !node->scope) return 0;
    source_instructions = node->child;
    if (source_instructions && source_instructions->node_type != INSTRUCTIONS) {
        using_expression = source_instructions;
        source_instructions = source_instructions->sibling;
    }
    if (!source_instructions || source_instructions->node_type != INSTRUCTIONS) return 0;

    if (using_expression) {
        int using_status = task_scope_expression_status(context,
                                                        using_expression);
        if (using_status <= 0) return 1;
    }

    scope_name = task_plan_name("scope", node, 0);
    if (!scope_name) return 0;
    scope = rxcp_remap_create_local_scope(context, node->scope,
                                          node, node->scope);
    outer = ast_f(context, INSTRUCTIONS, node->token);
    protected_block = ast_f(context, SIGNAL_BLOCK, node->token);
    if (!scope || !outer || !protected_block) {
        free(scope_name);
        return 0;
    }
    rxcp_remap_anchor_synthetic(outer, node);
    rxcp_remap_anchor_synthetic(protected_block, node);
    outer->scope = scope;
    outer->inherit_parent_scope = 1;
    outer->inherit_parent_reg_scope = 1;
    protected_block->scope = scope;
    protected_block->is_compiler_added = 1;

    memset(&plan, 0, sizeof(plan));
    plan.parallel_node = node;
    plan.task.context = context;
    plan.task.scope = scope;
    plan.task.instructions = outer;
    plan.task.scope_name = scope_name;

    if (using_expression) {
        ast_del(using_expression);
        scope_value = using_expression;
    } else {
        scope_value = task_named_factory(
                context, node, "concurrency.taskscope", "default", 0, 0);
    }
    if (!scope_value ||
        !task_append_assignment(&plan.task, node, scope_name, scope_value)) goto failed;

    ast_del(source_instructions);
    if (!task_process_parallel_instruction_list(&plan, source_instructions, 1)) goto failed;
    plan.task.instructions = source_instructions;
    if (!task_append_member_statement(&plan.task, node, scope_name, "finish") ||
        !task_materialize_all_pending(&plan, node)) goto failed;
    handler = task_parallel_signal_handler(&plan, node);
    if (!handler) goto failed;

    add_ast(protected_block, source_instructions);
    add_ast(protected_block, handler);

    node->node_type = INSTRUCTIONS;
    node->is_compiler_added = 1;
    node->force_local_scope = 1;
    node->inherit_parent_reg_scope = 1;
    node->scope = scope;
    node->child = 0;
    while (outer->child) {
        ASTNode *statement = outer->child;
        ast_del(statement);
        add_ast(node, statement);
    }
    add_ast(node, protected_block);
    for (i = 0; i < plan.pending_count; i++) free(plan.pending[i].handle_name);
    free(plan.pending);
    free(scope_name);
    context->changed_flags |= FLAG_VAL_TRANS | FLAG_ORCH | FLAG_VAL_SYM | FLAG_VAL_TYPE;
    return 1;

failed:
    for (i = 0; i < plan.pending_count; i++) free(plan.pending[i].handle_name);
    free(plan.pending);
    free(scope_name);
    return 0;
}

static int task_lower_parallel_block_expression(Context *context,
                                                ASTNode *node) {
    ASTNode *using_expression = 0;
    ASTNode *source_instructions;
    ASTNode *outer;
    ASTNode *protected_block;
    ASTNode *scope_value;
    ASTNode *handler;
    Scope *scope;
    RxcpParallelPlan plan;
    char *scope_name;
    size_t i;

    if (!context || !node || node->node_type != PARALLEL_BLOCK_EXPR ||
        !node->scope) return 0;
    source_instructions = node->child;
    if (source_instructions && source_instructions->node_type != INSTRUCTIONS) {
        using_expression = source_instructions;
        source_instructions = source_instructions->sibling;
    }
    if (!source_instructions ||
        source_instructions->node_type != INSTRUCTIONS) return 0;

    if (using_expression) {
        int using_status = task_scope_expression_status(context,
                                                        using_expression);
        if (using_status <= 0) return 1;
    }

    scope_name = task_plan_name("scope", node, 0);
    if (!scope_name) return 0;
    scope = rxcp_remap_create_local_scope(context, node->scope,
                                          node, node->scope);
    outer = ast_f(context, INSTRUCTIONS, node->token);
    protected_block = ast_f(context, SIGNAL_BLOCK, node->token);
    if (!outer || !protected_block) {
        free(scope_name);
        return 0;
    }
    rxcp_remap_anchor_synthetic(outer, node);
    rxcp_remap_anchor_synthetic(protected_block, node);
    outer->scope = scope;
    outer->inherit_parent_scope = 1;
    outer->inherit_parent_reg_scope = 1;
    protected_block->scope = scope;
    protected_block->is_compiler_added = 1;

    memset(&plan, 0, sizeof(plan));
    plan.parallel_node = node;
    plan.task.context = context;
    plan.task.scope = scope;
    plan.task.instructions = outer;
    plan.task.scope_name = scope_name;

    if (using_expression) {
        ast_del(using_expression);
        scope_value = using_expression;
    } else {
        scope_value = task_named_factory(
                context, node, "concurrency.taskscope", "default", 0, 0);
    }
    if (!scope_value ||
        !task_append_assignment(&plan.task, node,
                                scope_name, scope_value)) goto failed;

    task_bind_parallel_result_leaves(source_instructions, node);
    ast_del(source_instructions);
    if (!task_process_parallel_instruction_list(&plan,
                                                source_instructions, 1)) {
        goto failed;
    }
    handler = task_parallel_signal_handler(&plan, node);
    if (!handler) goto failed;

    add_ast(protected_block, source_instructions);
    add_ast(protected_block, handler);
    add_ast(outer, protected_block);

    node->node_type = BLOCK_EXPR;
    node->is_compiler_added = 1;
    node->scope = scope;
    node->child = 0;
    add_ast(node, outer);
    for (i = 0; i < plan.pending_count; i++) {
        free(plan.pending[i].handle_name);
    }
    free(plan.pending);
    free(scope_name);
    context->changed_flags |= FLAG_VAL_TRANS | FLAG_ORCH |
                              FLAG_VAL_SYM | FLAG_VAL_TYPE;
    return 1;

failed:
    for (i = 0; i < plan.pending_count; i++) {
        free(plan.pending[i].handle_name);
    }
    free(plan.pending);
    free(scope_name);
    return 0;
}

static int task_lower_implicit_expression(Context *context, ASTNode *root) {
    ASTNode *block;
    ASTNode *instructions;
    ASTNode *leave;
    ASTNode *scope_factory;
    Scope *scope;
    RxcpTaskPlan plan;
    char *scope_name;

    if (!context || !root || !root->parent || !root->scope) return 0;
    if (!task_isolate_short_circuit_rhs(context, root)) return 0;
    scope_name = task_plan_name("scope", root, 0);
    if (!scope_name) return 0;
    block = rxcp_remap_create_block_expr(
            context, root->scope, root, 0, &scope, &instructions);
    if (!block || !scope || !instructions) {
        free(scope_name);
        return 0;
    }
    ast_rpl(root, block);
    leave = rxcp_remap_create_leave_with(
            context, scope, root, block, root);
    if (!leave) {
        free(scope_name);
        return 0;
    }

    memset(&plan, 0, sizeof(plan));
    plan.context = context;
    plan.scope = scope;
    plan.instructions = instructions;
    plan.scope_name = scope_name;

    scope_factory = task_named_factory(
            context, root, "concurrency.taskscope", "default", 0, 0);
    if (!scope_factory ||
        !task_append_assignment(&plan, root, scope_name, scope_factory) ||
        !task_process_expression(&plan, root) ||
        !task_append_member_statement(&plan, root, scope_name, "finish")) {
        free(scope_name);
        return 0;
    }
    add_ast(instructions, leave);
    free(scope_name);
    context->changed_flags |= FLAG_ORCH | FLAG_VAL_SYM | FLAG_VAL_TYPE;
    return 1;
}

static ASTNode *task_statement_expression(ASTNode *node) {
    if (!node) return 0;
    switch (node->node_type) {
        case ASSIGN:
        case DEFINE:
            return ast_chdn(node, 1);
        case RETURN:
        case SAY:
        case LEAVE_WITH:
            return ast_chdn(node, 0);
        case IF:
        case WHILE:
        case UNTIL:
            return ast_chdn(node, 0);
        case CALL:
            return ast_chdn(node, 0);
        default:
            return 0;
    }
}

walker_result rxcp_task_calls_walker(walker_direction direction,
                                     ASTNode *node,
                                     void *payload) {
    Context *context = (Context *)payload;
    ASTNode *expression;

    if (direction != out || !context || !node) return result_normal;
    if ((node->node_type == PARALLEL_DO ||
         node->node_type == PARALLEL_BLOCK_EXPR) &&
        ast_chld(node, ERROR, 0)) return result_normal;
    expression = task_statement_expression(node);
    if (node->node_type == PARALLEL_BLOCK_EXPR) {
        if (!task_lower_parallel_block_expression(context, node) &&
            !ast_chld(node, ERROR, 0)) {
            mknd_err(node, "TASK_LOWERING_FAILED");
        }
        return result_normal;
    }
    if (node->node_type == PARALLEL_DO) {
        if (!task_lower_parallel_do(context, node) && !ast_chld(node, ERROR, 0)) {
            mknd_err(node, "TASK_LOWERING_FAILED");
        }
        return result_normal;
    }
    if (!expression || expression->node_type == BLOCK_EXPR ||
        !task_subtree_has_call(expression)) return result_normal;

    if (task_inside_task_callable(expression)) {
        if (!ast_chld(expression, ERROR, 0)) mknd_err(expression, "TASK_NESTED_WAIT");
        return result_normal;
    }
    if (task_inside_parallel(expression)) return result_normal;
    if (node->node_type == CALL && task_call_definition(expression, 0)) {
        ASTNode *definition = 0;
        task_call_definition(expression, &definition);
        if (!task_lower_implicit_call(context, node, expression, definition) &&
            !ast_chld(expression, ERROR, 0)) {
            mknd_err(expression, "TASK_LOWERING_FAILED");
        }
        return result_normal;
    }
    if (!task_lower_implicit_expression(context, expression)) {
        if (!ast_chld(expression, ERROR, 0)) mknd_err(expression, "TASK_LOWERING_FAILED");
    }
    return result_normal;
}
