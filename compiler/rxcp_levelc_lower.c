/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

/**
 * Level C Classic REXX lowering tracer.
 *
 * The active tracer slices deliberately accept only proven shapes: direct
 * scalar pool reads/writes, string and integer literals, binary addition, SAY,
 * and local PROCEDURE EXPOSE over direct scalar names. Everything else remains
 * behind the existing unsupported compile gate.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "rxcp_levelc_lower.h"
#include "rxcp_remap_build.h"
#include "rxcpcsym.h"

#define LEVELC_POOL_SYMBOL "__rxcp_levelc_pool"
#define LEVELC_PARENT_POOL_SYMBOL "__rxcp_levelc_parent_pool"
#define LEVELC_PARENT_POOL_REF_SYMBOL "__rxcp_levelc_parent_pool_ref"
#define LEVELC_PROC_PREFIX "__rxcp_levelc_proc_"

typedef struct {
    ASTNode *label;
    ASTNode *procedure;
    ASTNode *body_first;
    ASTNode *body_end;
    char *name;
} LevelCProcedureSlice;

typedef struct {
    ASTNode *instructions;
    ASTNode *main_first;
    ASTNode *main_end;
    LevelCProcedureSlice *procedures;
    size_t procedure_count;
} LevelCLowerPlan;

const char *rxcp_levelc_compile_unsupported_message(void) {
    return "REXX Level C (Classic REXX) is currently supported only for DSLSH syntax highlighting. Compilation is not supported yet";
}

static int levelc_node_has_diagnostic(ASTNode *node) {
    while (node) {
        if (node->node_type == ERROR || node->node_type == WARNING) return 1;
        if (node->child && levelc_node_has_diagnostic(node->child)) return 1;
        node = node->sibling;
    }
    return 0;
}

static ASTNode *levelc_program_file(Context *context) {
    ASTNode *root;

    if (!context || !context->ast) return NULL;
    root = context->ast;
    if (root->node_type != REXX_UNIVERSE) return NULL;
    if (!root->child || root->child->node_type != PROGRAM_FILE) return NULL;
    return root->child;
}

static ASTNode *levelc_instruction_list(ASTNode *program_file) {
    ASTNode *child;

    if (!program_file) return NULL;
    child = program_file->child;
    while (child) {
        if (child->node_type == INSTRUCTIONS) return child;
        child = child->sibling;
    }
    return NULL;
}

static void levelc_lower_plan_free(LevelCLowerPlan *plan) {
    size_t i;

    if (!plan) return;
    for (i = 0; i < plan->procedure_count; i++) {
        if (plan->procedures[i].name) free(plan->procedures[i].name);
    }
    if (plan->procedures) free(plan->procedures);
    memset(plan, 0, sizeof(*plan));
}

static int levelc_lower_plan_add_procedure(LevelCLowerPlan *plan,
                                           ASTNode *label,
                                           ASTNode *procedure,
                                           ASTNode *body_first,
                                           ASTNode *body_end,
                                           char *name) {
    LevelCProcedureSlice *procedures;

    if (!plan || !label || !procedure || !name) return 0;

    procedures = realloc(plan->procedures,
                         sizeof(LevelCProcedureSlice) * (plan->procedure_count + 1));
    if (!procedures) return 0;

    plan->procedures = procedures;
    plan->procedures[plan->procedure_count].label = label;
    plan->procedures[plan->procedure_count].procedure = procedure;
    plan->procedures[plan->procedure_count].body_first = body_first;
    plan->procedures[plan->procedure_count].body_end = body_end;
    plan->procedures[plan->procedure_count].name = name;
    plan->procedure_count++;
    return 1;
}

static int levelc_has_single_child(ASTNode *node) {
    return node && node->child && !node->child->sibling;
}

static int levelc_expr_supported(ASTNode *expr, const char **reason_out) {
    ASTNode *left;
    ASTNode *right;

    if (!expr) {
        if (reason_out) *reason_out = "missing expression";
        return 0;
    }

    switch (expr->node_type) {
        case STRING:
        case INTEGER:
        case VAR_SYMBOL:
            return 1;

        case OP_ADD:
            left = expr->child;
            right = left ? left->sibling : NULL;
            if (!left || !right || right->sibling) {
                if (reason_out) *reason_out = "unsupported add operand shape";
                return 0;
            }
            return levelc_expr_supported(left, reason_out) &&
                   levelc_expr_supported(right, reason_out);

        default:
            if (reason_out) *reason_out = "unsupported expression node";
            return 0;
    }
}

static int levelc_pool_statement_supported(ASTNode *stmt, const char **reason_out) {
    ASTNode *target;
    ASTNode *expr;

    if (!stmt) return 1;

    if (stmt->node_type == ASSIGN) {
        target = stmt->child;
        expr = target ? target->sibling : NULL;
        if (!target || target->node_type != VAR_TARGET || !expr || expr->sibling) {
            if (reason_out) *reason_out = "unsupported assignment shape";
            return 0;
        }
        return levelc_expr_supported(expr, reason_out);
    }

    if (stmt->node_type == SAY) {
        if (!levelc_has_single_child(stmt)) {
            if (reason_out) *reason_out = "unsupported SAY shape";
            return 0;
        }
        return levelc_expr_supported(stmt->child, reason_out);
    }

    if (reason_out) *reason_out = "unsupported statement node";
    return 0;
}

static char *levelc_node_text_copy(ASTNode *node) {
    char *copy;

    if (!node || !node->node_string) return NULL;
    copy = malloc(node->node_string_length + 1);
    if (!copy) return NULL;
    memcpy(copy, node->node_string, node->node_string_length);
    copy[node->node_string_length] = '\0';
    return copy;
}

static char *levelc_upper_name(ASTNode *node) {
    char *name;
    size_t i;

    if (!node) return NULL;
    if (node->token) {
        name = rxcp_levelc_upper_symbol_from_token(node->token, 0);
        if (name) return name;
    }

    name = levelc_node_text_copy(node);
    if (!name) return NULL;
    for (i = 0; name[i]; i++) {
        name[i] = (char)toupper((unsigned char)name[i]);
    }
    return name;
}

static char *levelc_upper_label_name(ASTNode *node) {
    char *name;
    size_t length;

    if (!node) return NULL;
    if (node->token) {
        name = rxcp_levelc_upper_symbol_from_token(node->token, 1);
        if (name) return name;
    }

    name = levelc_node_text_copy(node);
    if (!name) return NULL;
    length = strlen(name);
    if (length > 0 && name[length - 1] == ':') name[length - 1] = '\0';
    for (length = 0; name[length]; length++) {
        name[length] = (char)toupper((unsigned char)name[length]);
    }
    return name;
}

static int levelc_name_is_stem(const char *name) {
    size_t length;

    if (!name) return 0;
    length = strlen(name);
    return length > 0 && name[length - 1] == '.';
}

static char *levelc_generated_proc_name(const char *levelc_name, int with_colon) {
    size_t prefix_len;
    size_t name_len;
    char *result;

    if (!levelc_name || !levelc_name[0]) return NULL;
    prefix_len = strlen(LEVELC_PROC_PREFIX);
    name_len = strlen(levelc_name);
    result = malloc(prefix_len + name_len + (with_colon ? 1 : 0) + 1);
    if (!result) return NULL;
    memcpy(result, LEVELC_PROC_PREFIX, prefix_len);
    memcpy(result + prefix_len, levelc_name, name_len);
    if (with_colon) {
        result[prefix_len + name_len] = ':';
        result[prefix_len + name_len + 1] = '\0';
    } else {
        result[prefix_len + name_len] = '\0';
    }
    return result;
}

static ASTNode *levelc_named_ref(Context *context,
                                 ASTNode *source_node,
                                 NodeType node_type,
                                 const char *name) {
    ASTNode *node;

    if (!context || !source_node || !name) return NULL;

    node = ast_ftt(context, node_type, strdup(name));
    if (!node) return NULL;
    node->free_node_string = 1;
    rxcp_remap_anchor_synthetic(node, source_node);
    return node;
}

static ASTNode *levelc_pool_ref(Context *context, ASTNode *source_node, NodeType node_type) {
    return levelc_named_ref(context, source_node, node_type, LEVELC_POOL_SYMBOL);
}

static ASTNode *levelc_parent_pool_ref(Context *context, ASTNode *source_node, NodeType node_type) {
    return levelc_named_ref(context, source_node, node_type, LEVELC_PARENT_POOL_SYMBOL);
}

static ASTNode *levelc_parent_pool_ref_symbol(Context *context,
                                              ASTNode *source_node,
                                              NodeType node_type) {
    return levelc_named_ref(context, source_node, node_type, LEVELC_PARENT_POOL_REF_SYMBOL);
}

static ASTNode *levelc_name_string(Context *context, ASTNode *source_node) {
    char *name;
    ASTNode *node;

    name = levelc_upper_name(source_node);
    if (!name) return NULL;
    node = rxcp_remap_create_string_constant(context, source_node, name);
    free(name);
    return node;
}

static char *levelc_call_target_name(ASTNode *call_node) {
    ASTNode *target;

    if (!call_node || call_node->node_type != CALL) return NULL;
    target = call_node->child;
    if (!target || target->node_type != LITERAL || target->sibling) return NULL;
    return levelc_upper_name(target);
}

static LevelCProcedureSlice *levelc_find_procedure(LevelCLowerPlan *plan,
                                                   const char *name) {
    size_t i;

    if (!plan || !name) return NULL;
    for (i = 0; i < plan->procedure_count; i++) {
        if (plan->procedures[i].name && strcmp(plan->procedures[i].name, name) == 0) {
            return &plan->procedures[i];
        }
    }
    return NULL;
}

static int levelc_expose_list_supported(ASTNode *procedure_node,
                                        const char **reason_out) {
    ASTNode *child;
    ASTNode *args;
    ASTNode *arg;
    char *name;
    char *keyword;

    if (!procedure_node || procedure_node->node_type != LEVELC_PROCEDURE) {
        if (reason_out) *reason_out = "missing Level C PROCEDURE node";
        return 0;
    }

    child = procedure_node->child;
    if (!child) {
        if (reason_out) *reason_out = "plain PROCEDURE is outside slice";
        return 0;
    }
    if (child->node_type != LITERAL || !child->sibling ||
        child->sibling->node_type != ARGS || child->sibling->sibling) {
        if (reason_out) *reason_out = "unsupported PROCEDURE tail";
        return 0;
    }
    keyword = levelc_upper_name(child);
    if (!keyword || strcmp(keyword, "EXPOSE") != 0) {
        if (keyword) free(keyword);
        if (reason_out) *reason_out = "unsupported PROCEDURE keyword";
        return 0;
    }
    free(keyword);

    args = child->sibling;
    arg = args->child;
    if (!arg) {
        if (reason_out) *reason_out = "empty PROCEDURE EXPOSE list";
        return 0;
    }
    while (arg) {
        if (arg->node_type != VAR_TARGET) {
            if (reason_out) *reason_out = "unsupported PROCEDURE EXPOSE target";
            return 0;
        }
        name = levelc_upper_name(arg);
        if (!name || levelc_name_is_stem(name)) {
            if (name) free(name);
            if (reason_out) *reason_out = "stem PROCEDURE EXPOSE is outside slice";
            return 0;
        }
        free(name);
        arg = arg->sibling;
    }

    return 1;
}

static int levelc_main_statement_supported(ASTNode *stmt,
                                           LevelCLowerPlan *plan,
                                           const char **reason_out) {
    char *target_name;
    int supported;

    if (!stmt) return 1;
    if (stmt->node_type == REXX_OPTIONS) return 1;
    if (levelc_pool_statement_supported(stmt, reason_out)) return 1;

    if (stmt->node_type == CALL) {
        target_name = levelc_call_target_name(stmt);
        supported = target_name && levelc_find_procedure(plan, target_name) != NULL;
        if (target_name) free(target_name);
        if (!supported) {
            if (reason_out) *reason_out = "unsupported CALL target";
            return 0;
        }
        return 1;
    }

    if (stmt->node_type == EXIT) {
        if (stmt->child) {
            if (reason_out) *reason_out = "EXIT expression is outside slice";
            return 0;
        }
        return 1;
    }

    if (reason_out) *reason_out = "unsupported main statement";
    return 0;
}

static int levelc_proc_statement_supported(ASTNode *stmt,
                                           const char **reason_out) {
    if (!stmt) return 1;
    if (levelc_pool_statement_supported(stmt, reason_out)) return 1;
    if (stmt->node_type == RETURN) {
        if (stmt->child) {
            if (reason_out) *reason_out = "RETURN expression is outside slice";
            return 0;
        }
        return 1;
    }
    if (reason_out) *reason_out = "unsupported procedure statement";
    return 0;
}

static int levelc_plan_has_main_exit(LevelCLowerPlan *plan) {
    ASTNode *stmt;

    if (!plan) return 0;
    stmt = plan->main_first;
    while (stmt && stmt != plan->main_end) {
        if (stmt->node_type == EXIT && !stmt->child) return 1;
        stmt = stmt->sibling;
    }
    return 0;
}

static int levelc_collect_lower_plan(ASTNode *instructions,
                                     LevelCLowerPlan *plan,
                                     const char **reason_out) {
    ASTNode *stmt;
    ASTNode *label;
    ASTNode *procedure;
    ASTNode *body_first;
    ASTNode *body_end;
    ASTNode *body_stmt;
    ASTNode *last_body_stmt;
    char *name;
    int accepted_statement;

    if (reason_out) *reason_out = NULL;
    if (!instructions || instructions->node_type != INSTRUCTIONS || !plan) {
        if (reason_out) *reason_out = "missing top-level instruction list";
        return 0;
    }

    memset(plan, 0, sizeof(*plan));
    plan->instructions = instructions;

    stmt = instructions->child;
    while (stmt && stmt->node_type == REXX_OPTIONS) stmt = stmt->sibling;
    plan->main_first = stmt;
    while (stmt && stmt->node_type != LABEL) stmt = stmt->sibling;
    plan->main_end = stmt;

    while (stmt) {
        label = stmt;
        if (label->node_type != LABEL) {
            if (reason_out) *reason_out = "expected local routine label";
            return 0;
        }
        procedure = label->sibling;
        if (!procedure || procedure->node_type != LEVELC_PROCEDURE) {
            if (reason_out) *reason_out = "local routine label must be followed by PROCEDURE";
            return 0;
        }
        body_first = procedure->sibling;
        body_end = body_first;
        while (body_end && body_end->node_type != LABEL) body_end = body_end->sibling;

        if (!levelc_expose_list_supported(procedure, reason_out)) return 0;

        last_body_stmt = NULL;
        body_stmt = body_first;
        while (body_stmt && body_stmt != body_end) {
            if (!levelc_proc_statement_supported(body_stmt, reason_out)) return 0;
            last_body_stmt = body_stmt;
            body_stmt = body_stmt->sibling;
        }
        if (!last_body_stmt || last_body_stmt->node_type != RETURN) {
            if (reason_out) *reason_out = "procedure slice requires final RETURN";
            return 0;
        }

        name = levelc_upper_label_name(label);
        if (!name) {
            if (reason_out) *reason_out = "failed to normalize local routine label";
            return 0;
        }
        if (levelc_find_procedure(plan, name)) {
            free(name);
            if (reason_out) *reason_out = "duplicate local routine label";
            return 0;
        }
        if (!levelc_lower_plan_add_procedure(plan,
                                             label,
                                             procedure,
                                             body_first,
                                             body_end,
                                             name)) {
            free(name);
            if (reason_out) *reason_out = "failed to record local routine plan";
            return 0;
        }

        stmt = body_end;
    }

    accepted_statement = 0;
    stmt = plan->main_first;
    while (stmt && stmt != plan->main_end) {
        if (!levelc_main_statement_supported(stmt, plan, reason_out)) return 0;
        if (stmt->node_type != REXX_OPTIONS) accepted_statement = 1;
        stmt = stmt->sibling;
    }

    if (!accepted_statement) {
        if (reason_out) *reason_out = "no supported executable Level C statements";
        return 0;
    }

    if (plan->procedure_count > 0 && !levelc_plan_has_main_exit(plan)) {
        if (reason_out) *reason_out = "main slice must EXIT before local routines";
        return 0;
    }

    return 1;
}

static ASTNode *levelc_lower_expr(Context *context, ASTNode *expr);

static ASTNode *levelc_rexxvalue_from_literal(Context *context, ASTNode *source_node) {
    char *text;
    ASTNode *args[1];
    ASTNode *result;

    text = levelc_node_text_copy(source_node);
    if (!text) return NULL;

    args[0] = rxcp_remap_create_string_constant(context, source_node, text);
    free(text);
    if (!args[0]) return NULL;

    result = rxcp_remap_create_factory_call(context, source_node, "RexxValue", args, 1);
    return result;
}

static ASTNode *levelc_pool_value(Context *context, ASTNode *source_node) {
    ASTNode *args[1];
    ASTNode *receiver;

    receiver = levelc_pool_ref(context, source_node, VAR_SYMBOL);
    args[0] = levelc_name_string(context, source_node);
    if (!receiver || !args[0]) return NULL;

    return rxcp_remap_create_member_call(context, source_node, receiver, "value", args, 1);
}

static ASTNode *levelc_lower_add(Context *context, ASTNode *expr) {
    ASTNode *args[1];
    ASTNode *receiver;

    receiver = levelc_lower_expr(context, expr->child);
    args[0] = levelc_lower_expr(context, expr->child->sibling);
    if (!receiver || !args[0]) return NULL;

    return rxcp_remap_create_member_call(context, expr, receiver, "add", args, 1);
}

static ASTNode *levelc_unary_keyword_expr(Context *context,
                                          ASTNode *source_node,
                                          NodeType node_type,
                                          const char *keyword,
                                          ASTNode *operand) {
    ASTNode *node;

    if (!context || !source_node || !keyword || !operand) return NULL;

    node = ast_ftt(context, node_type, strdup(keyword));
    if (!node) return NULL;
    node->free_node_string = 1;
    rxcp_remap_anchor_synthetic(node, source_node);
    add_ast(node, operand);
    return node;
}

static ASTNode *levelc_reference_expr(Context *context,
                                      ASTNode *source_node,
                                      ASTNode *operand) {
    return levelc_unary_keyword_expr(context,
                                     source_node,
                                     OP_REFERENCE,
                                     "reference",
                                     operand);
}

static ASTNode *levelc_dereference_expr(Context *context,
                                        ASTNode *source_node,
                                        ASTNode *operand) {
    return levelc_unary_keyword_expr(context,
                                     source_node,
                                     OP_DEREFERENCE,
                                     "dereference",
                                     operand);
}

static ASTNode *levelc_function_call(Context *context,
                                     ASTNode *source_node,
                                     const char *function_name,
                                     ASTNode **args,
                                     size_t arg_count) {
    ASTNode *node;
    size_t i;

    if (!context || !source_node || !function_name) return NULL;

    node = ast_ftt(context, FUNCTION, strdup(function_name));
    if (!node) return NULL;
    node->free_node_string = 1;
    rxcp_remap_anchor_synthetic(node, source_node);

    if (arg_count == 0) {
        ASTNode *noval;

        noval = rxcp_remap_create_noval(context, source_node);
        if (!noval) return NULL;
        add_ast(node, noval);
    } else {
        if (!args) return NULL;
        for (i = 0; i < arg_count; i++) {
            if (!args[i]) return NULL;
            add_ast(node, args[i]);
        }
    }

    return node;
}

static ASTNode *levelc_return_statement(Context *context, ASTNode *source_node) {
    ASTNode *node;

    if (!context || !source_node) return NULL;

    node = ast_f(context, RETURN, source_node->token);
    if (!node) return NULL;
    rxcp_remap_anchor_synthetic(node, source_node);
    return node;
}

static ASTNode *levelc_lower_expr(Context *context, ASTNode *expr) {
    if (!context || !expr) return NULL;

    switch (expr->node_type) {
        case STRING:
        case INTEGER:
            return levelc_rexxvalue_from_literal(context, expr);
        case VAR_SYMBOL:
            return levelc_pool_value(context, expr);
        case OP_ADD:
            return levelc_lower_add(context, expr);
        default:
            return NULL;
    }
}

static ASTNode *levelc_pool_set_statement(Context *context, ASTNode *assign_node) {
    ASTNode *target;
    ASTNode *expr;
    ASTNode *receiver;
    ASTNode *args[2];
    ASTNode *member_call;

    target = assign_node->child;
    expr = target ? target->sibling : NULL;
    if (!target || !expr) return NULL;

    receiver = levelc_pool_ref(context, assign_node, VAR_SYMBOL);
    args[0] = levelc_name_string(context, target);
    args[1] = levelc_lower_expr(context, expr);
    if (!receiver || !args[0] || !args[1]) return NULL;

    member_call = rxcp_remap_create_member_call(context,
                                                assign_node,
                                                receiver,
                                                "setValue",
                                                args,
                                                2);
    if (!member_call) return NULL;

    return rxcp_remap_create_call_statement(context, assign_node, member_call);
}

static ASTNode *levelc_say_statement(Context *context, ASTNode *say_node) {
    ASTNode *lowered_expr;
    ASTNode *as_string;
    ASTNode *say;

    lowered_expr = levelc_lower_expr(context, say_node->child);
    if (!lowered_expr) return NULL;

    as_string = rxcp_remap_create_member_call(context,
                                             say_node,
                                             lowered_expr,
                                             "asString",
                                             NULL,
                                             0);
    if (!as_string) return NULL;

    say = ast_f(context, SAY, say_node->token);
    if (!say) return NULL;
    rxcp_remap_anchor_synthetic(say, say_node);
    add_ast(say, as_string);
    return say;
}

static ASTNode *levelc_pool_setup_statement(Context *context, ASTNode *anchor_node) {
    ASTNode *assign;
    ASTNode *lhs;
    ASTNode *rhs;

    assign = ast_f(context, ASSIGN, anchor_node ? anchor_node->token : NULL);
    if (!assign) return NULL;
    if (anchor_node) rxcp_remap_anchor_synthetic(assign, anchor_node);

    lhs = levelc_pool_ref(context, anchor_node ? anchor_node : assign, VAR_TARGET);
    rhs = rxcp_remap_create_factory_call(context,
                                         anchor_node ? anchor_node : assign,
                                         "RexxVariablePool",
                                         NULL,
                                         0);
    if (!lhs || !rhs) return NULL;

    add_ast(assign, lhs);
    add_ast(assign, rhs);
    return assign;
}

static ASTNode *levelc_parent_pool_setup_statement(Context *context,
                                                   ASTNode *anchor_node) {
    ASTNode *assign;
    ASTNode *lhs;
    ASTNode *rhs_operand;
    ASTNode *rhs;

    assign = ast_f(context, ASSIGN, anchor_node ? anchor_node->token : NULL);
    if (!assign) return NULL;
    if (anchor_node) rxcp_remap_anchor_synthetic(assign, anchor_node);

    lhs = levelc_parent_pool_ref(context, anchor_node ? anchor_node : assign, VAR_TARGET);
    rhs_operand = levelc_parent_pool_ref_symbol(context,
                                                anchor_node ? anchor_node : assign,
                                                VAR_SYMBOL);
    rhs = levelc_dereference_expr(context,
                                  anchor_node ? anchor_node : assign,
                                  rhs_operand);
    if (!lhs || !rhs_operand || !rhs) return NULL;

    add_ast(assign, lhs);
    add_ast(assign, rhs);
    return assign;
}

static ASTNode *levelc_call_local_procedure_statement(Context *context,
                                                      ASTNode *call_node,
                                                      LevelCLowerPlan *plan) {
    char *target_name;
    char *function_name;
    ASTNode *args[1];
    ASTNode *pool_symbol;
    ASTNode *call_expr;
    ASTNode *statement;

    target_name = levelc_call_target_name(call_node);
    if (!target_name || !levelc_find_procedure(plan, target_name)) {
        if (target_name) free(target_name);
        return NULL;
    }

    function_name = levelc_generated_proc_name(target_name, 0);
    free(target_name);
    if (!function_name) return NULL;

    pool_symbol = levelc_pool_ref(context, call_node, VAR_SYMBOL);
    args[0] = levelc_reference_expr(context, call_node, pool_symbol);
    if (!pool_symbol || !args[0]) {
        free(function_name);
        return NULL;
    }

    call_expr = levelc_function_call(context, call_node, function_name, args, 1);
    free(function_name);
    if (!call_expr) return NULL;

    statement = rxcp_remap_create_call_statement(context, call_node, call_expr);
    return statement;
}

static ASTNode *levelc_expose_value_statement(Context *context,
                                              ASTNode *procedure_node,
                                              ASTNode *expose_target) {
    ASTNode *receiver;
    ASTNode *args[3];
    ASTNode *parent_symbol;
    ASTNode *member_call;

    receiver = levelc_pool_ref(context, expose_target, VAR_SYMBOL);
    args[0] = levelc_name_string(context, expose_target);
    parent_symbol = levelc_parent_pool_ref(context, expose_target, VAR_SYMBOL);
    args[1] = levelc_reference_expr(context, expose_target, parent_symbol);
    args[2] = levelc_name_string(context, expose_target);
    if (!receiver || !args[0] || !parent_symbol || !args[1] || !args[2]) return NULL;

    member_call = rxcp_remap_create_member_call(context,
                                                procedure_node,
                                                receiver,
                                                "exposeValue",
                                                args,
                                                3);
    if (!member_call) return NULL;

    return rxcp_remap_create_call_statement(context, procedure_node, member_call);
}

static ASTNode *levelc_procedure_header(Context *context,
                                        LevelCProcedureSlice *procedure) {
    char *name;
    ASTNode *node;
    ASTNode *void_node;

    if (!context || !procedure || !procedure->name) return NULL;

    name = levelc_generated_proc_name(procedure->name, 1);
    if (!name) return NULL;

    node = ast_ftt(context, PROCEDURE, name);
    if (!node) {
        free(name);
        return NULL;
    }
    node->free_node_string = 1;
    rxcp_remap_anchor_synthetic(node, procedure->label);

    void_node = ast_ft(context, VOID);
    if (!void_node) return NULL;
    rxcp_remap_anchor_synthetic(void_node, procedure->procedure);
    add_ast(node, void_node);
    return node;
}

static ASTNode *levelc_procedure_args(Context *context,
                                      LevelCProcedureSlice *procedure) {
    ASTNode *args;
    ASTNode *arg;
    ASTNode *target;
    ASTNode *type_ref;
    ASTNode *class_node;

    if (!context || !procedure) return NULL;

    args = ast_ft(context, ARGS);
    arg = ast_ft(context, ARG);
    target = levelc_parent_pool_ref_symbol(context, procedure->procedure, VAR_TARGET);
    type_ref = ast_ftt(context, TYPE_REFERENCE, strdup("reference"));
    class_node = ast_ftt(context, CLASS, strdup(".RexxVariablePool"));
    if (!args || !arg || !target || !type_ref || !class_node) return NULL;

    rxcp_remap_anchor_synthetic(args, procedure->procedure);
    rxcp_remap_anchor_synthetic(arg, procedure->procedure);
    rxcp_remap_anchor_synthetic(type_ref, procedure->procedure);
    rxcp_remap_anchor_synthetic(class_node, procedure->procedure);
    type_ref->free_node_string = 1;
    class_node->free_node_string = 1;

    add_ast(type_ref, class_node);
    add_ast(arg, target);
    add_ast(arg, type_ref);
    add_ast(args, arg);
    return args;
}

static int levelc_append_procedure_exposes(Context *context,
                                           ASTNode *instructions,
                                           LevelCProcedureSlice *procedure,
                                           const char **reason_out) {
    ASTNode *child;
    ASTNode *args;
    ASTNode *expose_target;
    ASTNode *statement;

    if (!procedure || !procedure->procedure) return 1;
    child = procedure->procedure->child;
    if (!child) return 1;
    args = child->sibling;
    if (!args || args->node_type != ARGS) return 1;

    expose_target = args->child;
    while (expose_target) {
        statement = levelc_expose_value_statement(context,
                                                  procedure->procedure,
                                                  expose_target);
        if (!statement) {
            if (reason_out) *reason_out = "failed to create PROCEDURE EXPOSE statement";
            return 0;
        }
        add_ast(instructions, statement);
        expose_target = expose_target->sibling;
    }
    return 1;
}

static ASTNode *levelc_build_options(Context *context, ASTNode *anchor_node) {
    ASTNode *options;
    ASTNode *levelb;
    ASTNode *comments_dash;
    ASTNode *numeric_classic;
    ASTNode *import_value;
    ASTNode *import_pool;

    options = ast_f(context, REXX_OPTIONS, anchor_node ? anchor_node->token : NULL);
    if (!options) return NULL;
    if (anchor_node) rxcp_remap_anchor_synthetic(options, anchor_node);

    levelb = rxcp_remap_create_literal(context, anchor_node ? anchor_node : options, "levelb");
    comments_dash = rxcp_remap_create_literal(context, anchor_node ? anchor_node : options, "comments_dash");
    numeric_classic = rxcp_remap_create_literal(context, anchor_node ? anchor_node : options, "numeric_classic");
    import_value = rxcp_remap_create_import(context, anchor_node ? anchor_node : options, "rexxvalue");
    import_pool = rxcp_remap_create_import(context, anchor_node ? anchor_node : options, "rexxpool");
    if (!levelb || !comments_dash || !numeric_classic || !import_value || !import_pool) return NULL;

    add_ast(options, levelb);
    add_ast(options, comments_dash);
    add_ast(options, numeric_classic);
    add_ast(options, import_value);
    add_ast(options, import_pool);
    return options;
}

static ASTNode *levelc_lower_main_statement(Context *context,
                                            ASTNode *stmt,
                                            LevelCLowerPlan *plan) {
    if (!stmt) return NULL;

    if (stmt->node_type == ASSIGN) return levelc_pool_set_statement(context, stmt);
    if (stmt->node_type == SAY) return levelc_say_statement(context, stmt);
    if (stmt->node_type == CALL) return levelc_call_local_procedure_statement(context, stmt, plan);
    if (stmt->node_type == EXIT) return levelc_return_statement(context, stmt);
    return NULL;
}

static ASTNode *levelc_lower_proc_statement(Context *context, ASTNode *stmt) {
    if (!stmt) return NULL;

    if (stmt->node_type == ASSIGN) return levelc_pool_set_statement(context, stmt);
    if (stmt->node_type == SAY) return levelc_say_statement(context, stmt);
    if (stmt->node_type == RETURN) return levelc_return_statement(context, stmt);
    return NULL;
}

static int levelc_rewrite_program(Context *context,
                                  ASTNode *program_file,
                                  ASTNode *old_instructions,
                                  LevelCLowerPlan *plan,
                                  const char **reason_out) {
    ASTNode *anchor;
    ASTNode *options;
    ASTNode *instructions;
    ASTNode *pool_setup;
    ASTNode *stmt;
    size_t i;

    anchor = old_instructions && old_instructions->child ? old_instructions->child : program_file;
    options = levelc_build_options(context, anchor);
    instructions = ast_ft(context, INSTRUCTIONS);
    if (!options || !instructions) {
        if (reason_out) *reason_out = "failed to create Level C lowered program shell";
        return 0;
    }
    rxcp_remap_anchor_synthetic(instructions, anchor);

    pool_setup = levelc_pool_setup_statement(context, anchor);
    if (!pool_setup) {
        if (reason_out) *reason_out = "failed to create Level C pool setup";
        return 0;
    }
    add_ast(instructions, pool_setup);

    stmt = plan ? plan->main_first : old_instructions->child;
    while (stmt && (!plan || stmt != plan->main_end)) {
        ASTNode *lowered;

        if (stmt->node_type != REXX_OPTIONS) {
            lowered = levelc_lower_main_statement(context, stmt, plan);
            if (!lowered) {
                if (reason_out) *reason_out = "failed to lower supported Level C statement";
                return 0;
            }
            add_ast(instructions, lowered);
        }
        stmt = stmt->sibling;
    }

    if (plan) {
        for (i = 0; i < plan->procedure_count; i++) {
            ASTNode *procedure_header;
            ASTNode *procedure_args;
            ASTNode *parent_setup;
            ASTNode *procedure_pool_setup;

            procedure_header = levelc_procedure_header(context, &plan->procedures[i]);
            procedure_args = levelc_procedure_args(context, &plan->procedures[i]);
            parent_setup = levelc_parent_pool_setup_statement(context,
                                                              plan->procedures[i].procedure);
            procedure_pool_setup = levelc_pool_setup_statement(context,
                                                               plan->procedures[i].procedure);
            if (!procedure_header || !procedure_args || !parent_setup || !procedure_pool_setup) {
                if (reason_out) *reason_out = "failed to create Level C procedure shell";
                return 0;
            }

            add_ast(instructions, procedure_header);
            add_ast(instructions, procedure_args);
            add_ast(instructions, parent_setup);
            add_ast(instructions, procedure_pool_setup);
            if (!levelc_append_procedure_exposes(context,
                                                 instructions,
                                                 &plan->procedures[i],
                                                 reason_out)) {
                return 0;
            }

            stmt = plan->procedures[i].body_first;
            while (stmt && stmt != plan->procedures[i].body_end) {
                ASTNode *lowered;

                lowered = levelc_lower_proc_statement(context, stmt);
                if (!lowered) {
                    if (reason_out) *reason_out = "failed to lower supported Level C procedure statement";
                    return 0;
                }
                add_ast(instructions, lowered);
                stmt = stmt->sibling;
            }
        }
    }

    old_instructions->parent = NULL;
    old_instructions->sibling = NULL;

    program_file->child = options;
    options->parent = program_file;
    options->sibling = instructions->child;
    stmt = options->sibling;
    while (stmt) {
        stmt->parent = program_file;
        stmt = stmt->sibling;
    }
    instructions->child = NULL;
    instructions->parent = NULL;
    instructions->sibling = NULL;

    context->level = LEVELB;
    context->changed_flags |= FLAG_VAL_TRANS;
    return 1;
}

int rxcp_levelc_lower_slice1(Context *context, const char **reason_out) {
    ASTNode *program_file;
    ASTNode *instructions;
    LevelCLowerPlan plan;
    int result;

    if (reason_out) *reason_out = NULL;
    memset(&plan, 0, sizeof(plan));
    if (!context || !context->ast) {
        if (reason_out) *reason_out = "missing AST";
        return 0;
    }

    if (levelc_node_has_diagnostic(context->ast)) {
        if (reason_out) *reason_out = "source diagnostics present";
        return 0;
    }

    program_file = levelc_program_file(context);
    instructions = levelc_instruction_list(program_file);
    if (!program_file || !instructions) {
        if (reason_out) *reason_out = "unsupported Level C program shell";
        return 0;
    }

    if (!levelc_collect_lower_plan(instructions, &plan, reason_out)) {
        levelc_lower_plan_free(&plan);
        return 0;
    }
    result = levelc_rewrite_program(context, program_file, instructions, &plan, reason_out);
    levelc_lower_plan_free(&plan);
    return result;
}
