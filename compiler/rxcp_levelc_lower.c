/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

/**
 * Level C Classic REXX lowering tracer.
 *
 * The active tracer slices deliberately accept only proven shapes: direct
 * scalar and compound pool reads/writes, string and integer literals, proven
 * expression operators, SAY, NOP, direct scalar DROP, nested IF, SELECT and
 * bounded DO forms, and local PROCEDURE EXPOSE over direct scalar or stem names.
 * Everything else reports an unsupported-shape diagnostic until its lowering
 * and runtime contract are implemented.
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxcp_levelc_lower.h"
#include "rxcp_remap_build.h"
#include "rxcpcsym.h"

#define LEVELC_POOL_SYMBOL "__rxcp_levelc_pool"
#define LEVELC_PARENT_POOL_SYMBOL "__rxcp_levelc_parent_pool"
#define LEVELC_PARENT_POOL_REF_SYMBOL "__rxcp_levelc_parent_pool_ref"
#define LEVELC_PROC_PREFIX "__rxcp_levelc_proc_"
#define LEVELC_PROC_ARG_PREFIX "__rxcp_levelc_arg_"
#define LEVELC_BIF_ARGS_PREFIX "__rxcp_levelc_bif_args_"
#define LEVELC_BIF_EXISTS_PREFIX "__rxcp_levelc_bif_exists_"
#define LEVELC_BIF_CONTEXT_PREFIX "__rxcp_levelc_bif_context_"
#define LEVELC_COMPOUND_TAIL_PREFIX "__rxcp_levelc_tail_"
#define LEVELC_EXPR_RESULT_PREFIX "__rxcp_levelc_expr_"
#define LEVELC_PARSE_FIELDS_PREFIX "__rxcp_levelc_parse_fields_"
#define LEVELC_LOOP_PREFIX "__rxcp_levelc_loop_"
#define LEVELC_START_VALUE_PREFIX "__rxcp_levelc_start_"
#define LEVELC_TO_LIMIT_PREFIX "__rxcp_levelc_to_"
#define LEVELC_BY_STEP_PREFIX "__rxcp_levelc_by_"
#define LEVELC_FOR_COUNT_PREFIX "__rxcp_levelc_for_"
#define LEVELC_BIF_LENGTH_HELPER "rexxclassicbif_length"
#define LEVELC_BIF_DISPATCH_HELPER "rexxclassicbif_call"
#define LEVELC_BIF_TRANSLATE_HELPER "rexxclassicbif_translate"
#define LEVELC_BIF_CONTEXT_CLASS "RexxBifCallContext"
#define LEVELC_REXX_VALUE_CLASS "RexxValue"
#define LEVELC_REXX_VALUE_CLASS_TYPE ".RexxValue"
#define LEVELC_INT_CLASS_TYPE ".int"

typedef struct {
    ASTNode *label;
    ASTNode *procedure;
    ASTNode *arg_statement;
    ASTNode *body_first;
    ASTNode *body_end;
    char *name;
    size_t arg_count;
    int returns_value;
} LevelCProcedureSlice;

typedef struct LevelCLoopBinding {
    ASTNode *source_do;
    const char *control_name;
    struct LevelCLoopBinding *previous;
} LevelCLoopBinding;

typedef struct {
    ASTNode *instructions;
    ASTNode *main_first;
    ASTNode *main_end;
    LevelCProcedureSlice *procedures;
    size_t procedure_count;
    LevelCLoopBinding *active_loop;
} LevelCLowerPlan;

typedef enum {
    LEVELC_VAR_NAME_INVALID = 0,
    LEVELC_VAR_NAME_SCALAR,
    LEVELC_VAR_NAME_STEM,
    LEVELC_VAR_NAME_COMPOUND
} LevelCVariableNameKind;

const char *rxcp_levelc_compile_unsupported_message(void) {
    return "REXX Level C (Classic REXX) compilation does not yet support this program shape";
}

static int levelc_node_has_diagnostic(ASTNode *node) {
    while (node) {
        if (node->node_type == ERROR || node->node_type == WARNING) return 1;
        if (node->child && levelc_node_has_diagnostic(node->child)) return 1;
        node = node->sibling;
    }
    return 0;
}

static int levelc_node_is_source_only(NodeType type) {
    switch (type) {
        case LEVELC_ADDRESS:
        case LEVELC_ARG:
        case LEVELC_DROP:
        case LEVELC_INTERPRET:
        case LEVELC_NUMERIC:
        case LEVELC_PROCEDURE:
        case LEVELC_PUSH:
        case LEVELC_QUEUE:
        case LEVELC_SIGNAL:
        case LEVELC_TRACE:
        case PARSE:
        case PULL:
        case TEMPLATES:
            return 1;
        default:
            return 0;
    }
}

static int levelc_verify_lowered_chain(ASTNode *node,
                                       ASTNode *expected_parent,
                                       const char **reason_out) {
    ASTNode *slow = node;
    ASTNode *fast = node;

    while (fast && fast->sibling) {
        slow = slow->sibling;
        fast = fast->sibling->sibling;
        if (slow == fast) {
            if (reason_out) *reason_out = "lowered tree has a sibling cycle";
            return 0;
        }
    }

    while (node) {
        if (node->parent != expected_parent) {
            if (reason_out) *reason_out = "lowered tree has inconsistent parent ownership";
            return 0;
        }
        if (levelc_node_is_source_only(node->node_type)) {
            if (reason_out) *reason_out = "lowered tree retains a Level C-only node";
            return 0;
        }
        if (!levelc_verify_lowered_chain(node->child, node, reason_out)) return 0;
        node = node->sibling;
    }
    return 1;
}

int rxcp_levelc_verify_lowered_tree(ASTNode *root, const char **reason_out) {
    if (!root) {
        if (reason_out) *reason_out = "lowered tree has no root";
        return 0;
    }
    return levelc_verify_lowered_chain(root, NULL, reason_out);
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
                                           ASTNode *arg_statement,
                                           ASTNode *body_first,
                                           ASTNode *body_end,
                                           char *name,
                                           size_t arg_count,
                                           int returns_value) {
    LevelCProcedureSlice *procedures;

    if (!plan || !label || !procedure || !name) return 0;

    procedures = realloc(plan->procedures,
                         sizeof(LevelCProcedureSlice) * (plan->procedure_count + 1));
    if (!procedures) return 0;

    plan->procedures = procedures;
    plan->procedures[plan->procedure_count].label = label;
    plan->procedures[plan->procedure_count].procedure = procedure;
    plan->procedures[plan->procedure_count].arg_statement = arg_statement;
    plan->procedures[plan->procedure_count].body_first = body_first;
    plan->procedures[plan->procedure_count].body_end = body_end;
    plan->procedures[plan->procedure_count].name = name;
    plan->procedures[plan->procedure_count].arg_count = arg_count;
    plan->procedures[plan->procedure_count].returns_value = returns_value;
    plan->procedure_count++;
    return 1;
}

static char *levelc_upper_name(ASTNode *node);
static LevelCProcedureSlice *levelc_find_procedure(LevelCLowerPlan *plan,
                                                   const char *name);
static int levelc_expr_supported(ASTNode *expr,
                                 LevelCLowerPlan *plan,
                                 const char **reason_out);
static int levelc_variable_value_supported(ASTNode *node,
                                           const char **reason_out);
static int levelc_assignment_target_supported(ASTNode *node,
                                              const char **reason_out);

static const char *levelc_binary_operator_method(NodeType node_type) {
    switch (node_type) {
        case OP_ADD:
            return "add";
        case OP_MINUS:
            return "subtract";
        case OP_MULT:
            return "multiply";
        case OP_DIV:
            return "divide";
        case OP_IDIV:
            return "integerDivide";
        case OP_MOD:
            return "remainder";
        case OP_POWER:
            return "power";
        case OP_CONCAT:
            return "concat";
        case OP_SCONCAT:
            return "spaceConcat";
        case OP_COMPARE_EQUAL:
            return "compareEqual";
        case OP_COMPARE_NEQ:
            return "compareNotEqual";
        case OP_COMPARE_GT:
            return "compareGreaterThan";
        case OP_COMPARE_LT:
            return "compareLessThan";
        case OP_COMPARE_GTE:
            return "compareGreaterOrEqual";
        case OP_COMPARE_LTE:
            return "compareLessOrEqual";
        case OP_COMPARE_S_EQ:
            return "strictCompareEqual";
        case OP_COMPARE_S_NEQ:
            return "strictCompareNotEqual";
        case OP_COMPARE_S_GT:
            return "strictCompareGreaterThan";
        case OP_COMPARE_S_LT:
            return "strictCompareLessThan";
        case OP_COMPARE_S_GTE:
            return "strictCompareGreaterOrEqual";
        case OP_COMPARE_S_LTE:
            return "strictCompareLessOrEqual";
        case OP_XOR:
            return "logicalXor";
        default:
            return NULL;
    }
}

static const char *levelc_unary_operator_method(NodeType node_type) {
    switch (node_type) {
        case OP_NEG:
            return "negate";
        case OP_PLUS:
            return "positive";
        case OP_NOT:
            return "logicalNot";
        default:
            return NULL;
    }
}

static int levelc_is_classic_logical_binary(NodeType node_type) {
    return node_type == OP_AND || node_type == OP_OR;
}

static int levelc_has_single_child(ASTNode *node) {
    return node && node->child && !node->child->sibling;
}

static size_t levelc_function_argument_count(ASTNode *expr) {
    ASTNode *arg;
    size_t count;

    if (!expr || expr->node_type != FUNCTION) return 0;
    arg = expr->child;
    if (!arg) return 0;
    if (arg->node_type == NOVAL && !arg->sibling) return 0;

    count = 0;
    while (arg) {
        count++;
        arg = arg->sibling;
    }
    return count;
}

static int levelc_argument_exists(ASTNode *arg) {
    return arg && arg->node_type != NOVAL;
}

static int levelc_function_name_is(ASTNode *expr, const char *expected_name) {
    char *name;
    int matched;

    if (!expr || expr->node_type != FUNCTION || !expected_name) return 0;

    name = levelc_upper_name(expr);
    matched = name && strcmp(name, expected_name) == 0;
    if (name) free(name);
    return matched;
}

static int levelc_length_function_supported(ASTNode *expr,
                                            LevelCLowerPlan *plan,
                                            const char **reason_out) {
    ASTNode *arg;

    (void)plan;

    if (!levelc_function_name_is(expr, "LENGTH")) {
        if (reason_out) *reason_out = "unsupported Level C function call";
        return 0;
    }

    arg = expr->child;
    if (!arg || arg->node_type == NOVAL || arg->sibling) {
        if (reason_out) *reason_out = "unsupported LENGTH argument shape";
        return 0;
    }

    return levelc_expr_supported(arg, plan, reason_out);
}

static int levelc_substr_function_supported(ASTNode *expr,
                                            LevelCLowerPlan *plan,
                                            const char **reason_out) {
    ASTNode *arg;
    size_t index;
    size_t arg_count;

    if (!levelc_function_name_is(expr, "SUBSTR")) {
        if (reason_out) *reason_out = "unsupported Level C function call";
        return 0;
    }

    arg_count = levelc_function_argument_count(expr);
    if (arg_count < 2 || arg_count > 4) {
        if (reason_out) *reason_out = "unsupported SUBSTR argument count";
        return 0;
    }

    arg = expr->child;
    index = 1;
    while (arg) {
        if ((index == 1 || index == 2) && !levelc_argument_exists(arg)) {
            if (reason_out) *reason_out = "missing required SUBSTR argument";
            return 0;
        }
        if (levelc_argument_exists(arg) &&
            !levelc_expr_supported(arg, plan, reason_out)) {
            return 0;
        }
        arg = arg->sibling;
        index++;
    }

    return 1;
}

static int levelc_local_function_supported(ASTNode *expr,
                                           LevelCLowerPlan *plan,
                                           const char **reason_out) {
    char *name;
    LevelCProcedureSlice *procedure;
    ASTNode *arg;

    if (!expr || expr->node_type != FUNCTION || !plan) {
        if (reason_out) *reason_out = "unsupported local function call";
        return 0;
    }

    name = levelc_upper_name(expr);
    procedure = name ? levelc_find_procedure(plan, name) : NULL;
    if (name) free(name);
    if (!procedure) {
        if (reason_out) *reason_out = "unsupported Level C function call";
        return 0;
    }
    if (!procedure->returns_value) {
        if (reason_out) *reason_out = "local function has no return value";
        return 0;
    }
    if (levelc_function_argument_count(expr) != procedure->arg_count) {
        if (reason_out) *reason_out = "local function argument count mismatch";
        return 0;
    }

    arg = expr->child;
    if (arg && arg->node_type == NOVAL && !arg->sibling) arg = NULL;
    while (arg) {
        if (!levelc_argument_exists(arg)) {
            if (reason_out) *reason_out = "omitted local function argument is outside slice";
            return 0;
        }
        if (!levelc_expr_supported(arg, plan, reason_out)) return 0;
        arg = arg->sibling;
    }

    return 1;
}

static int levelc_expr_supported(ASTNode *expr,
                                 LevelCLowerPlan *plan,
                                 const char **reason_out) {
    ASTNode *left;
    ASTNode *right;
    const char *method;

    if (!expr) {
        if (reason_out) *reason_out = "missing expression";
        return 0;
    }

    switch (expr->node_type) {
        case STRING:
        case INTEGER:
        case DECIMAL:
            return 1;

        case VAR_SYMBOL:
            return levelc_variable_value_supported(expr, reason_out);

        case OP_AND:
        case OP_OR:
            left = expr->child;
            right = left ? left->sibling : NULL;
            if (!left || !right || right->sibling) {
                if (reason_out) *reason_out = "unsupported short-circuit operand shape";
                return 0;
            }
            return levelc_expr_supported(left, plan, reason_out) &&
                   levelc_expr_supported(right, plan, reason_out);

        case FUNCTION:
            return levelc_length_function_supported(expr, plan, reason_out) ||
                   levelc_substr_function_supported(expr, plan, reason_out) ||
                   levelc_local_function_supported(expr, plan, reason_out);

        default:
            method = levelc_binary_operator_method(expr->node_type);
            if (method) {
                left = expr->child;
                right = left ? left->sibling : NULL;
                if (!left || !right || right->sibling) {
                    if (reason_out) *reason_out = "unsupported binary operand shape";
                    return 0;
                }
                return levelc_expr_supported(left, plan, reason_out) &&
                       levelc_expr_supported(right, plan, reason_out);
            }

            method = levelc_unary_operator_method(expr->node_type);
            if (method) {
                left = expr->child;
                if (!left || left->sibling) {
                    if (reason_out) *reason_out = "unsupported unary operand shape";
                    return 0;
                }
                return levelc_expr_supported(left, plan, reason_out);
            }

            if (reason_out) *reason_out = "unsupported expression node";
            return 0;
    }
}

static int levelc_pool_statement_supported(ASTNode *stmt,
                                           LevelCLowerPlan *plan,
                                           const char **reason_out) {
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
        if (!levelc_assignment_target_supported(target, reason_out)) return 0;
        return levelc_expr_supported(expr, plan, reason_out);
    }

    if (stmt->node_type == SAY) {
        if (!levelc_has_single_child(stmt)) {
            if (reason_out) *reason_out = "unsupported SAY shape";
            return 0;
        }
        return levelc_expr_supported(stmt->child, plan, reason_out);
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

static LevelCVariableNameKind levelc_variable_name_kind(const char *name) {
    const char *dot;
    size_t length;

    if (!name || !name[0]) return LEVELC_VAR_NAME_INVALID;

    dot = strchr(name, '.');
    if (!dot) return LEVELC_VAR_NAME_SCALAR;

    length = strlen(name);
    if (length > 0 && name[length - 1] == '.') return LEVELC_VAR_NAME_STEM;
    if (dot[1] != '\0') return LEVELC_VAR_NAME_COMPOUND;
    return LEVELC_VAR_NAME_INVALID;
}

static int levelc_name_is_stem(const char *name) {
    return levelc_variable_name_kind(name) == LEVELC_VAR_NAME_STEM;
}

static int levelc_name_is_compound(const char *name) {
    return levelc_variable_name_kind(name) == LEVELC_VAR_NAME_COMPOUND;
}

static char *levelc_compound_stem_name(const char *name) {
    const char *dot;
    size_t length;
    char *stem;

    if (!name || !levelc_name_is_compound(name)) return NULL;

    dot = strchr(name, '.');
    if (!dot) return NULL;

    length = (size_t)(dot - name) + 1;
    stem = malloc(length + 1);
    if (!stem) return NULL;
    memcpy(stem, name, length);
    stem[length] = '\0';
    return stem;
}

static char *levelc_compound_tail_name(const char *name) {
    const char *dot;
    char *tail;

    if (!name || !levelc_name_is_compound(name)) return NULL;

    dot = strchr(name, '.');
    if (!dot || !dot[1]) return NULL;

    tail = strdup(dot + 1);
    return tail;
}

static int levelc_tail_is_numeric_literal(const char *tail) {
    size_t i;

    if (!tail || !tail[0]) return 0;
    for (i = 0; tail[i]; i++) {
        if (!isdigit((unsigned char)tail[i])) return 0;
    }
    return 1;
}

static int levelc_compound_tail_supported(const char *name) {
    char *tail;
    int supported;

    tail = levelc_compound_tail_name(name);
    if (!tail) return 0;

    supported = tail[0] != '\0' && strchr(tail, '.') == NULL;
    free(tail);
    return supported;
}

static char *levelc_generated_proc_name(const char *levelc_name, int with_colon) {
    if (!levelc_name || !levelc_name[0]) return NULL;
    return rxcp_remap_create_prefixed_name(LEVELC_PROC_PREFIX,
                                           levelc_name,
                                           with_colon ? ":" : NULL);
}

static ASTNode *levelc_pool_ref(Context *context, ASTNode *source_node, NodeType node_type) {
    return rxcp_remap_create_named_ref(context, source_node, node_type, LEVELC_POOL_SYMBOL);
}

static ASTNode *levelc_parent_pool_ref(Context *context, ASTNode *source_node, NodeType node_type) {
    return rxcp_remap_create_named_ref(context, source_node, node_type, LEVELC_PARENT_POOL_SYMBOL);
}

static ASTNode *levelc_parent_pool_ref_symbol(Context *context,
                                              ASTNode *source_node,
                                              NodeType node_type) {
    return rxcp_remap_create_named_ref(context, source_node, node_type, LEVELC_PARENT_POOL_REF_SYMBOL);
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

static int levelc_variable_value_supported(ASTNode *node,
                                           const char **reason_out) {
    char *name;
    LevelCVariableNameKind kind;
    int supported;

    name = levelc_upper_name(node);
    if (!name) {
        if (reason_out) *reason_out = "failed to normalize variable name";
        return 0;
    }

    kind = levelc_variable_name_kind(name);
    supported = 0;
    switch (kind) {
        case LEVELC_VAR_NAME_SCALAR:
            supported = 1;
            break;
        case LEVELC_VAR_NAME_COMPOUND:
            supported = levelc_compound_tail_supported(name);
            if (!supported && reason_out) *reason_out = "compound tail shape is outside slice";
            break;
        case LEVELC_VAR_NAME_STEM:
            if (reason_out) *reason_out = "bare stem value is outside slice";
            break;
        default:
            if (reason_out) *reason_out = "unsupported variable name shape";
            break;
    }

    free(name);
    return supported;
}

static int levelc_assignment_target_supported(ASTNode *node,
                                              const char **reason_out) {
    char *name;
    LevelCVariableNameKind kind;
    int supported;

    name = levelc_upper_name(node);
    if (!name) {
        if (reason_out) *reason_out = "failed to normalize assignment target";
        return 0;
    }

    kind = levelc_variable_name_kind(name);
    supported = 0;
    switch (kind) {
        case LEVELC_VAR_NAME_SCALAR:
            supported = 1;
            break;
        case LEVELC_VAR_NAME_COMPOUND:
            supported = levelc_compound_tail_supported(name);
            if (!supported && reason_out) *reason_out = "compound assignment tail shape is outside slice";
            break;
        case LEVELC_VAR_NAME_STEM:
            if (reason_out) *reason_out = "bare stem assignment is outside slice";
            break;
        default:
            if (reason_out) *reason_out = "unsupported assignment target shape";
            break;
    }

    free(name);
    return supported;
}

static char *levelc_call_target_name(ASTNode *call_node) {
    ASTNode *target;

    if (!call_node || call_node->node_type != CALL) return NULL;
    target = call_node->child;
    if (!target || target->node_type != LITERAL) return NULL;
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

static int levelc_procedure_tail_supported(ASTNode *procedure_node,
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
    if (!child) return 1;
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
        if (!name) {
            if (reason_out) *reason_out = "failed to normalize PROCEDURE EXPOSE target";
            return 0;
        }
        if (levelc_name_is_compound(name)) {
            free(name);
            if (reason_out) *reason_out = "compound PROCEDURE EXPOSE is outside slice";
            return 0;
        }
        free(name);
        arg = arg->sibling;
    }

    return 1;
}

static ASTNode *levelc_arg_template_target(ASTNode *template_node) {
    ASTNode *target;

    if (!template_node || template_node->node_type != TEMPLATES) return NULL;
    target = template_node->child;
    if (!target || target->node_type != TARGET || target->sibling) return NULL;
    return target;
}

static int levelc_arg_statement_supported(ASTNode *stmt,
                                          size_t *arg_count_out,
                                          const char **reason_out) {
    ASTNode *templates;
    ASTNode *template_node;
    ASTNode *target;
    char *name;
    size_t arg_count;

    if (arg_count_out) *arg_count_out = 0;
    if (!stmt || stmt->node_type != LEVELC_ARG) {
        if (reason_out) *reason_out = "missing ARG statement";
        return 0;
    }

    templates = stmt->child;
    if (!templates) return 1;
    if (templates->node_type != TEMPLATES) {
        if (reason_out) *reason_out = "unsupported ARG template list";
        return 0;
    }

    arg_count = 0;
    template_node = templates->child;
    while (template_node) {
        target = levelc_arg_template_target(template_node);
        if (!target) {
            if (reason_out) *reason_out = "unsupported ARG template";
            return 0;
        }
        name = levelc_upper_name(target);
        if (!name) {
            if (reason_out) *reason_out = "failed to normalize ARG template";
            return 0;
        }
        if (levelc_variable_name_kind(name) != LEVELC_VAR_NAME_SCALAR) {
            free(name);
            if (reason_out) *reason_out = "non-scalar ARG template is outside slice";
            return 0;
        }
        free(name);
        arg_count++;
        template_node = template_node->sibling;
    }

    if (arg_count_out) *arg_count_out = arg_count;
    return 1;
}

static int levelc_call_tail_value_supported(ASTNode *node,
                                            const char **reason_out) {
    if (!node) return 0;
    if (node->node_type == INTEGER || node->node_type == DECIMAL) return 1;
    if (node->node_type == LITERAL) {
        return levelc_variable_value_supported(node, reason_out);
    }
    return 0;
}

static int levelc_call_tail_supported(ASTNode *args,
                                      size_t *arg_count_out,
                                      const char **reason_out) {
    ASTNode *node;
    int expect_value;
    size_t arg_count;

    if (arg_count_out) *arg_count_out = 0;
    if (!args) return 1;
    if (args->node_type != ARGS) {
        if (reason_out) *reason_out = "unsupported CALL argument tail";
        return 0;
    }

    expect_value = 1;
    arg_count = 0;
    node = args->child;
    while (node) {
        if (node->node_type == TOKEN && node->node_string &&
            strcmp(node->node_string, ",") == 0) {
            if (expect_value) {
                if (reason_out) *reason_out = "omitted CALL arguments are outside slice";
                return 0;
            }
            expect_value = 1;
        } else if (levelc_call_tail_value_supported(node, reason_out)) {
            if (!expect_value) {
                if (reason_out) *reason_out = "CALL arguments must be comma separated";
                return 0;
            }
            expect_value = 0;
            arg_count++;
        } else {
            if (reason_out) *reason_out = "unsupported CALL argument expression";
            return 0;
        }
        node = node->sibling;
    }

    if (expect_value && arg_count > 0) {
        if (reason_out) *reason_out = "trailing CALL comma is outside slice";
        return 0;
    }

    if (arg_count_out) *arg_count_out = arg_count;
    return 1;
}

static int levelc_call_statement_supported(ASTNode *stmt,
                                           LevelCLowerPlan *plan,
                                           const char **reason_out) {
    char *target_name;
    LevelCProcedureSlice *procedure;
    ASTNode *args;
    size_t arg_count;

    target_name = levelc_call_target_name(stmt);
    procedure = target_name ? levelc_find_procedure(plan, target_name) : NULL;
    if (target_name) free(target_name);
    if (!procedure) {
        if (reason_out) *reason_out = "unsupported CALL target";
        return 0;
    }

    args = stmt->child ? stmt->child->sibling : NULL;
    if (!levelc_call_tail_supported(args, &arg_count, reason_out)) return 0;
    if (arg_count != procedure->arg_count) {
        if (reason_out) *reason_out = "CALL argument count mismatch";
        return 0;
    }

    return 1;
}

static int levelc_main_statement_supported(ASTNode *stmt,
                                           LevelCLowerPlan *plan,
                                           const char **reason_out);
static int levelc_proc_statement_supported(ASTNode *stmt,
                                           LevelCLowerPlan *plan,
                                           const char **reason_out);

static int levelc_select_statement_supported(ASTNode *stmt,
                                             LevelCLowerPlan *plan,
                                             int in_procedure,
                                             const char **reason_out) {
    ASTNode *list = stmt ? stmt->child : NULL;
    ASTNode *clause;
    int saw_when = 0;
    int saw_otherwise = 0;

    if (!list || list->node_type != INSTRUCTIONS || list->sibling) goto invalid;
    clause = list->child;
    while (clause) {
        ASTNode *condition;
        ASTNode *body;
        if (clause->node_type == WHEN && !saw_otherwise) {
            condition = clause->child;
            body = condition ? condition->sibling : NULL;
            if (!condition || !body || body->sibling) goto invalid;
            if (!levelc_expr_supported(condition, plan, reason_out)) return 0;
            if (in_procedure) {
                if (body->node_type == LEVELC_ARG) goto invalid;
                if (!levelc_proc_statement_supported(body, plan, reason_out)) return 0;
            } else if (!levelc_main_statement_supported(body, plan, reason_out)) return 0;
            saw_when = 1;
        } else if (clause->node_type == OTHERWISE && saw_when && !saw_otherwise &&
                   !clause->sibling) {
            ASTNode *part = clause->child;
            ASTNode *body_list = part && part->node_type == INSTRUCTIONS ? part :
                                 part ? part->sibling : NULL;
            if (!body_list || body_list->node_type != INSTRUCTIONS || body_list->sibling) goto invalid;
            if (part != body_list) {
                if (in_procedure) {
                    if (part->node_type == LEVELC_ARG) goto invalid;
                    if (!levelc_proc_statement_supported(part, plan, reason_out)) return 0;
                } else if (!levelc_main_statement_supported(part, plan, reason_out)) return 0;
            }
            part = body_list->child;
            while (part) {
                if (in_procedure) {
                    if (part->node_type == LEVELC_ARG) goto invalid;
                    if (!levelc_proc_statement_supported(part, plan, reason_out)) return 0;
                } else if (!levelc_main_statement_supported(part, plan, reason_out)) return 0;
                part = part->sibling;
            }
            saw_otherwise = 1;
        } else goto invalid;
        clause = clause->sibling;
    }
    if (saw_when) return 1;
invalid:
    if (reason_out) *reason_out = "unsupported SELECT statement shape";
    return 0;
}

static int levelc_scalar_drop_supported(ASTNode *stmt,
                                        const char **reason_out) {
    ASTNode *list = stmt ? stmt->child : NULL;
    ASTNode *target;

    if (!list || list->node_type != ARGS || list->sibling || !list->child) {
        if (reason_out) *reason_out = "unsupported DROP list";
        return 0;
    }

    target = list->child;
    while (target) {
        char *name;
        int scalar;

        if (target->node_type != VAR_TARGET || target->child) {
            if (reason_out) *reason_out = "indirect DROP is outside slice";
            return 0;
        }
        name = levelc_upper_name(target);
        scalar = name && levelc_variable_name_kind(name) == LEVELC_VAR_NAME_SCALAR;
        free(name);
        if (!scalar) {
            if (reason_out) *reason_out = "stem or compound DROP is outside slice";
            return 0;
        }
        target = target->sibling;
    }
    return 1;
}

static int levelc_literal_repeat_count(ASTNode *repeat,
                                       int *count_out,
                                       const char **reason_out) {
    ASTNode *for_node = repeat ? repeat->child : NULL;
    ASTNode *literal = for_node ? for_node->child : NULL;
    char *text;
    char *end;
    long value;
    size_t i;

    if (!repeat || repeat->node_type != REPEAT || !for_node ||
        for_node->node_type != FOR || for_node->sibling || !literal ||
        literal->node_type != INTEGER || literal->child || literal->sibling) goto unsupported;
    text = levelc_node_text_copy(literal);
    if (!text || !text[0]) {
        free(text);
        goto unsupported;
    }
    for (i = 0; text[i]; i++) {
        if (!isdigit((unsigned char)text[i])) {
            free(text);
            goto unsupported;
        }
    }
    errno = 0;
    value = strtol(text, &end, 10);
    if (errno == ERANGE || *end || value > INT_MAX) {
        free(text);
        goto unsupported;
    }
    free(text);
    if (count_out) *count_out = (int)value;
    return 1;

unsupported:
    if (reason_out) *reason_out = "unsupported DO repetition header";
    return 0;
}

static int levelc_repetition_supported(ASTNode *repeat,
                                       int *count_out,
                                       int *forever_out,
                                       const char **reason_out) {
    ASTNode *for_node = repeat ? repeat->child : NULL;
    ASTNode *expression = for_node ? for_node->child : NULL;

    if (repeat && repeat->node_type == REPEAT && !repeat->child &&
        nodeis(repeat, "forever")) {
        if (count_out) *count_out = 0;
        if (forever_out) *forever_out = 1;
        return 1;
    }
    if (forever_out) *forever_out = 0;
    if (levelc_literal_repeat_count(repeat, count_out, reason_out)) return 1;
    if (!repeat || repeat->node_type != REPEAT || !for_node ||
        for_node->node_type != FOR || for_node->sibling ||
        !expression || expression->sibling) return 0;
    if (count_out) *count_out = -1;
    if (reason_out) *reason_out = NULL;
    return 1;
}

static int levelc_nonnegative_integer_literal(ASTNode *node) {
    char *text;
    size_t i;
    int valid;

    if (!node || node->node_type != INTEGER || node->child || node->sibling) return 0;
    text = levelc_node_text_copy(node);
    valid = text && text[0];
    for (i = 0; valid && text[i]; i++) {
        if (!isdigit((unsigned char)text[i])) valid = 0;
    }
    free(text);
    return valid;
}

static int levelc_signed_integer_literal(ASTNode *node) {
    if (levelc_nonnegative_integer_literal(node)) return 1;
    return node && node->node_type == OP_NEG && !node->sibling &&
           levelc_nonnegative_integer_literal(node->child);
}

static int levelc_bounded_nonnegative_integer_literal(ASTNode *node,
                                                      int *value_out) {
    char *text;
    char *end;
    long value;

    if (!levelc_nonnegative_integer_literal(node)) return 0;
    text = levelc_node_text_copy(node);
    if (!text) return 0;
    errno = 0;
    value = strtol(text, &end, 10);
    if (errno == ERANGE || *end || value > INT_MAX) {
        free(text);
        return 0;
    }
    free(text);
    if (value_out) *value_out = (int)value;
    return 1;
}

static int levelc_controlled_header_supported(ASTNode *repeat,
                                               LevelCLowerPlan *plan,
                                               ASTNode **to_out,
                                               ASTNode **by_out,
                                               ASTNode **for_out,
                                               const char **reason_out) {
    ASTNode *assign = repeat ? repeat->child : NULL;
    ASTNode *target = assign ? assign->child : NULL;
    ASTNode *start = target ? target->sibling : NULL;
    ASTNode *clause = assign ? assign->sibling : NULL;
    ASTNode *to = NULL;
    ASTNode *by = NULL;
    ASTNode *for_clause = NULL;
    char *name;
    int scalar;

    if (!repeat || repeat->node_type != REPEAT || !assign ||
        assign->node_type != ASSIGN || !target ||
        target->node_type != VAR_TARGET || !start ||
        !(levelc_nonnegative_integer_literal(start) ||
          levelc_expr_supported(start, plan, reason_out))) goto unsupported;
    while (clause) {
        if (clause->node_type == TO && !to && clause->child &&
            !clause->child->sibling &&
            (levelc_nonnegative_integer_literal(clause->child) ||
             levelc_expr_supported(clause->child, plan, reason_out))) {
            to = clause;
        } else if (clause->node_type == BY && !by && clause->child &&
                   !clause->child->sibling &&
                   (levelc_signed_integer_literal(clause->child) ||
                    levelc_expr_supported(clause->child, plan, reason_out))) {
            by = clause;
        } else if (clause->node_type == FOR && !for_clause && clause->child &&
                   !clause->child->sibling &&
                   (levelc_bounded_nonnegative_integer_literal(clause->child, NULL) ||
                    levelc_expr_supported(clause->child, plan, reason_out))) {
            for_clause = clause;
        } else goto unsupported;
        clause = clause->sibling;
    }
    name = levelc_upper_name(target);
    scalar = name && levelc_variable_name_kind(name) == LEVELC_VAR_NAME_SCALAR;
    free(name);
    if (scalar) {
        if (to_out) *to_out = to;
        if (by_out) *by_out = by;
        if (for_out) *for_out = for_clause;
        return 1;
    }

unsupported:
    if (reason_out) *reason_out = "unsupported controlled DO header";
    return 0;
}

static int levelc_do_condition_supported(ASTNode *condition,
                                         LevelCLowerPlan *plan,
                                         const char **reason_out) {
    if (!condition ||
        (condition->node_type != WHILE && condition->node_type != UNTIL) ||
        !levelc_has_single_child(condition)) {
        if (reason_out) *reason_out = "unsupported DO condition header";
        return 0;
    }
    return levelc_expr_supported(condition->child, plan, reason_out);
}

static ASTNode *levelc_nearest_source_repetitive_do(ASTNode *stmt) {
    ASTNode *ancestor;

    for (ancestor = stmt ? stmt->parent : NULL; ancestor; ancestor = ancestor->parent) {
        if (ancestor->node_type == DO && ancestor->child &&
            (ancestor->child->node_type == REPEAT ||
             ancestor->child->node_type == WHILE ||
             ancestor->child->node_type == UNTIL)) return ancestor;
    }
    return NULL;
}

static ASTNode *levelc_named_source_repetitive_do(ASTNode *stmt) {
    ASTNode *ancestor;
    char *requested_name;

    if (!stmt || !stmt->child || stmt->child->node_type != VAR_SYMBOL ||
        stmt->child->child || stmt->child->sibling) return NULL;
    requested_name = levelc_upper_name(stmt->child);
    if (!requested_name) return NULL;
    for (ancestor = stmt->parent; ancestor; ancestor = ancestor->parent) {
        ASTNode *repeat = ancestor->node_type == DO ? ancestor->child : NULL;
        ASTNode *assign = repeat && repeat->node_type == REPEAT ? repeat->child : NULL;
        ASTNode *control = assign && assign->node_type == ASSIGN ? assign->child : NULL;
        char *control_name;
        int matches;

        if (!control || control->node_type != VAR_TARGET) continue;
        control_name = levelc_upper_name(control);
        matches = control_name && strcmp(requested_name, control_name) == 0;
        free(control_name);
        if (matches) {
            free(requested_name);
            return ancestor;
        }
    }
    free(requested_name);
    return NULL;
}

static int levelc_transfer_supported(ASTNode *stmt,
                                     LevelCLowerPlan *plan,
                                     const char **reason_out) {
    ASTNode *target;

    if (!stmt) {
        if (reason_out) *reason_out = "unsupported LEAVE/ITERATE shape";
        return 0;
    }
    if (stmt->child) {
        target = levelc_named_source_repetitive_do(stmt);
        if (target && levelc_controlled_header_supported(
                target->child, plan, NULL, NULL, NULL, reason_out)) return 1;
        if (reason_out && !*reason_out) *reason_out = "named LEAVE/ITERATE requires a supported controlled DO";
        return 0;
    }
    target = levelc_nearest_source_repetitive_do(stmt);
    if (target) {
        if (target->child->node_type == WHILE ||
            target->child->node_type == UNTIL) return 1;
        if (levelc_controlled_header_supported(target->child, plan, NULL, NULL, NULL, NULL)) return 1;
        return levelc_repetition_supported(target->child, NULL, NULL, reason_out);
    }
    if (reason_out) *reason_out = "LEAVE/ITERATE requires a supported repetitive DO";
    return 0;
}

static int levelc_do_supported(ASTNode *stmt,
                              LevelCLowerPlan *plan,
                              int in_procedure,
                              const char **reason_out) {
    ASTNode *body = stmt ? stmt->child : NULL;
    ASTNode *body_statement;

    if (body && body->node_type == REPEAT) {
        int count = 0;
        ASTNode *repeat = body;
        if (repeat->child && repeat->child->node_type == ASSIGN) {
            if (!levelc_controlled_header_supported(repeat, plan, NULL, NULL, NULL, reason_out)) return 0;
        } else {
            if (!levelc_repetition_supported(repeat, &count, NULL, reason_out)) return 0;
            if (count < 0 && !levelc_expr_supported(repeat->child->child, plan, reason_out)) return 0;
        }
        body = body->sibling;
        if (body && (body->node_type == WHILE || body->node_type == UNTIL)) {
            if (!levelc_do_condition_supported(body, plan, reason_out)) return 0;
            body = body->sibling;
        }
    } else if (body && (body->node_type == WHILE || body->node_type == UNTIL)) {
        if (!levelc_do_condition_supported(body, plan, reason_out)) return 0;
        body = body->sibling;
    }
    if (!body || body->node_type != INSTRUCTIONS || body->sibling) {
        if (reason_out) *reason_out = "unsupported DO header";
        return 0;
    }

    body_statement = body->child;
    while (body_statement) {
        if (in_procedure) {
            if (body_statement->node_type == LEVELC_ARG) {
                if (reason_out) *reason_out = "ARG must be first in procedure slice";
                return 0;
            }
            if (!levelc_proc_statement_supported(body_statement, plan, reason_out)) return 0;
        } else if (!levelc_main_statement_supported(body_statement, plan, reason_out)) {
            return 0;
        }
        body_statement = body_statement->sibling;
    }
    return 1;
}

static int levelc_if_statement_supported(ASTNode *stmt,
                                         LevelCLowerPlan *plan,
                                         int in_procedure,
                                         const char **reason_out) {
    ASTNode *condition = stmt ? stmt->child : NULL;
    ASTNode *then_statement = condition ? condition->sibling : NULL;
    ASTNode *else_statement = then_statement ? then_statement->sibling : NULL;

    if (!condition || !then_statement || (else_statement && else_statement->sibling)) {
        if (reason_out) *reason_out = "unsupported IF statement shape";
        return 0;
    }
    if (!levelc_expr_supported(condition, plan, reason_out)) return 0;
    if (in_procedure && (then_statement->node_type == LEVELC_ARG ||
                         (else_statement && else_statement->node_type == LEVELC_ARG))) {
        if (reason_out) *reason_out = "ARG must be first in procedure slice";
        return 0;
    }
    if (in_procedure) {
        if (!levelc_proc_statement_supported(then_statement, plan, reason_out)) return 0;
        return !else_statement || levelc_proc_statement_supported(else_statement, plan, reason_out);
    }
    if (!levelc_main_statement_supported(then_statement, plan, reason_out)) return 0;
    return !else_statement || levelc_main_statement_supported(else_statement, plan, reason_out);
}

static int levelc_direct_parse_shape(ASTNode *stmt,
                                     LevelCLowerPlan *plan,
                                     ASTNode **source_out,
                                     ASTNode **target_out,
                                     size_t *target_count_out,
                                     int *is_value_out,
                                     int *upper_out,
                                     const char **reason_out) {
    ASTNode *source;
    ASTNode *templates;
    ASTNode *template_node;
    ASTNode *target;
    ASTNode *cursor;
    char *name;
    size_t target_count;
    int upper = 0;
    int is_value = 0;

    if (!stmt || stmt->node_type != PARSE) return 0;
    source = stmt->child;
    if (source && source->node_type == OPTIONS) {
        ASTNode *option = source->child;
        name = option && !option->sibling ? levelc_upper_name(option) : NULL;
        upper = name && strcmp(name, "UPPER") == 0;
        free(name);
        if (!upper) goto unsupported;
        source = source->sibling;
    }
    templates = source ? source->sibling : NULL;
    template_node = templates && templates->node_type == TEMPLATES ? templates->child : NULL;
    target = template_node && template_node->node_type == TEMPLATES
        ? template_node->child : NULL;
    if (!source || !templates || templates->sibling || !template_node ||
        template_node->sibling || !target) goto unsupported;

    if (source->node_type == VAR_REFERENCE) {
        name = levelc_upper_name(source);
        if (!source->child || source->child->node_type != TOKEN ||
            source->child->sibling || !name ||
            levelc_variable_name_kind(name) != LEVELC_VAR_NAME_SCALAR) {
            free(name);
            goto unsupported;
        }
        free(name);
    } else if (source->node_type == LITERAL) {
        ASTNode *expr = source->child;
        name = levelc_upper_name(source);
        is_value = name && strcmp(name, "VALUE") == 0;
        free(name);
        if (!is_value || !expr || expr->sibling ||
            !levelc_expr_supported(expr, plan, reason_out)) goto unsupported;
    } else goto unsupported;

    target_count = 0;
    for (cursor = target; cursor; cursor = cursor->sibling) {
        if (cursor->node_type != TARGET || cursor->child) goto unsupported;
        name = levelc_upper_name(cursor);
        if (!name || levelc_variable_name_kind(name) != LEVELC_VAR_NAME_SCALAR) {
            free(name);
            goto unsupported;
        }
        free(name);
        target_count++;
    }
    if (target_count != 1 && target_count != 3) goto unsupported;
    if (source_out) *source_out = source;
    if (target_out) *target_out = target;
    if (target_count_out) *target_count_out = target_count;
    if (is_value_out) *is_value_out = is_value;
    if (upper_out) *upper_out = upper;
    return 1;

unsupported:
    if (reason_out) *reason_out = "unsupported PARSE source or template shape";
    return 0;
}

static int levelc_main_statement_supported(ASTNode *stmt,
                                           LevelCLowerPlan *plan,
                                           const char **reason_out) {
    if (!stmt) return 1;
    if (stmt->node_type == REXX_OPTIONS) return 1;
    if (stmt->node_type == NOP) return stmt->child == NULL;
    if (stmt->node_type == LEAVE || stmt->node_type == ITERATE)
        return levelc_transfer_supported(stmt, plan, reason_out);
    if (stmt->node_type == LEVELC_DROP) return levelc_scalar_drop_supported(stmt, reason_out);
    if (stmt->node_type == IF) return levelc_if_statement_supported(stmt, plan, 0, reason_out);
    if (stmt->node_type == DO) return levelc_do_supported(stmt, plan, 0, reason_out);
    if (stmt->node_type == SELECT) return levelc_select_statement_supported(stmt, plan, 0, reason_out);
    if (stmt->node_type == PARSE)
        return levelc_direct_parse_shape(stmt, plan, NULL, NULL, NULL, NULL, NULL, reason_out);
    if (levelc_pool_statement_supported(stmt, plan, reason_out)) return 1;

    if (stmt->node_type == CALL) {
        return levelc_call_statement_supported(stmt, plan, reason_out);
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
                                           LevelCLowerPlan *plan,
                                           const char **reason_out) {
    if (!stmt) return 1;
    if (stmt->node_type == LEVELC_ARG) return levelc_arg_statement_supported(stmt, NULL, reason_out);
    if (stmt->node_type == NOP) return stmt->child == NULL;
    if (stmt->node_type == LEAVE || stmt->node_type == ITERATE)
        return levelc_transfer_supported(stmt, plan, reason_out);
    if (stmt->node_type == LEVELC_DROP) return levelc_scalar_drop_supported(stmt, reason_out);
    if (stmt->node_type == IF) return levelc_if_statement_supported(stmt, plan, 1, reason_out);
    if (stmt->node_type == DO) return levelc_do_supported(stmt, plan, 1, reason_out);
    if (stmt->node_type == SELECT) return levelc_select_statement_supported(stmt, plan, 1, reason_out);
    if (stmt->node_type == PARSE)
        return levelc_direct_parse_shape(stmt, plan, NULL, NULL, NULL, NULL, NULL, reason_out);
    if (levelc_pool_statement_supported(stmt, plan, reason_out)) return 1;
    if (stmt->node_type == RETURN) {
        if (stmt->child) return levelc_expr_supported(stmt->child, plan, reason_out);
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
    ASTNode *arg_statement;
    char *name;
    size_t arg_count;
    size_t i;
    int accepted_statement;
    int returns_value;

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

        if (!levelc_procedure_tail_supported(procedure, reason_out)) return 0;

        arg_statement = NULL;
        arg_count = 0;
        returns_value = 0;
        last_body_stmt = NULL;
        body_stmt = body_first;
        while (body_stmt && body_stmt != body_end) {
            if (body_stmt->node_type == LEVELC_ARG) {
                if (body_stmt != body_first || arg_statement) {
                    if (reason_out) *reason_out = "ARG must be first in procedure slice";
                    return 0;
                }
                if (!levelc_arg_statement_supported(body_stmt, &arg_count, reason_out)) {
                    return 0;
                }
                arg_statement = body_stmt;
            }
            if (body_stmt->node_type == RETURN && body_stmt->child) returns_value = 1;
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
                                             arg_statement,
                                             body_first,
                                             body_end,
                                             name,
                                             arg_count,
                                             returns_value)) {
            free(name);
            if (reason_out) *reason_out = "failed to record local routine plan";
            return 0;
        }

        stmt = body_end;
    }

    for (i = 0; i < plan->procedure_count; i++) {
        body_stmt = plan->procedures[i].body_first;
        while (body_stmt && body_stmt != plan->procedures[i].body_end) {
            if (!levelc_proc_statement_supported(body_stmt, plan, reason_out)) return 0;
            body_stmt = body_stmt->sibling;
        }
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

static ASTNode *levelc_lower_expr(Context *context,
                                  ASTNode *expr,
                                  LevelCLowerPlan *plan,
                                  ASTNode *prelude);

static char *levelc_generated_arg_name(size_t index) {
    return rxcp_remap_create_generated_indexed_name(LEVELC_PROC_ARG_PREFIX, index);
}

static ASTNode *levelc_rexxvalue_from_text(Context *context,
                                           ASTNode *source_node,
                                           const char *text) {
    ASTNode *args[1];
    ASTNode *result;

    if (!text) return NULL;

    args[0] = rxcp_remap_create_string_constant(context, source_node, text);
    if (!args[0]) return NULL;

    result = rxcp_remap_create_factory_call(context,
                                            source_node,
                                            LEVELC_REXX_VALUE_CLASS,
                                            args,
                                            1);
    return result;
}

static ASTNode *levelc_rexxvalue_from_literal(Context *context, ASTNode *source_node) {
    char *text;
    ASTNode *result;

    if (source_node && source_node->node_type == STRING &&
        source_node->node_string_length == 0)
        return levelc_rexxvalue_from_text(context, source_node, "");

    text = levelc_node_text_copy(source_node);
    if (!text) return NULL;
    result = levelc_rexxvalue_from_text(context, source_node, text);
    free(text);
    return result;
}

static ASTNode *levelc_blank_rexxvalue(Context *context, ASTNode *source_node) {
    return levelc_rexxvalue_from_text(context, source_node, "");
}

static ASTNode *levelc_scalar_pool_value_by_name(Context *context,
                                                 ASTNode *source_node,
                                                 const char *name) {
    ASTNode *args[1];
    ASTNode *receiver;

    receiver = levelc_pool_ref(context, source_node, VAR_SYMBOL);
    args[0] = rxcp_remap_create_string_constant(context, source_node, name);
    if (!receiver || !args[0]) return NULL;

    return rxcp_remap_create_member_call(context, source_node, receiver, "symbolValue", args, 1);
}

static ASTNode *levelc_compound_tail_expr(Context *context,
                                          ASTNode *source_node,
                                          const char *name) {
    char *tail;
    ASTNode *value;
    ASTNode *as_string;

    tail = levelc_compound_tail_name(name);
    if (!tail) return NULL;

    if (levelc_tail_is_numeric_literal(tail)) {
        ASTNode *literal_tail;

        literal_tail = rxcp_remap_create_string_constant(context, source_node, tail);
        free(tail);
        return literal_tail;
    }

    value = levelc_scalar_pool_value_by_name(context, source_node, tail);
    free(tail);
    if (!value) return NULL;

    as_string = rxcp_remap_create_member_call(context,
                                             source_node,
                                             value,
                                             "asString",
                                             NULL,
                                             0);
    return as_string;
}

static ASTNode *levelc_compound_stem_string(Context *context,
                                            ASTNode *source_node,
                                            const char *name) {
    char *stem;
    ASTNode *node;

    stem = levelc_compound_stem_name(name);
    if (!stem) return NULL;

    node = rxcp_remap_create_string_constant(context, source_node, stem);
    free(stem);
    return node;
}

static ASTNode *levelc_compound_pool_value_by_name(Context *context,
                                                  ASTNode *source_node,
                                                  const char *name,
                                                  ASTNode *tail_expr) {
    ASTNode *args[2];
    ASTNode *receiver;

    receiver = levelc_pool_ref(context, source_node, VAR_SYMBOL);
    args[0] = levelc_compound_stem_string(context, source_node, name);
    args[1] = tail_expr ? tail_expr : levelc_compound_tail_expr(context, source_node, name);
    if (!receiver || !args[0] || !args[1]) return NULL;

    return rxcp_remap_create_member_call(context, source_node, receiver, "stemValue", args, 2);
}

static ASTNode *levelc_pool_value(Context *context, ASTNode *source_node) {
    char *name;
    ASTNode *value;

    name = levelc_upper_name(source_node);
    if (!name) return NULL;

    if (levelc_variable_name_kind(name) == LEVELC_VAR_NAME_COMPOUND) {
        value = levelc_compound_pool_value_by_name(context, source_node, name, NULL);
    } else {
        value = levelc_scalar_pool_value_by_name(context, source_node, name);
    }

    free(name);
    return value;
}

static ASTNode *levelc_copy_rexxvalue(Context *context,
                                      ASTNode *source_node,
                                      ASTNode *value_expr) {
    ASTNode *factory_args[1];

    if (!context || !source_node || !value_expr) return NULL;

    factory_args[0] = rxcp_remap_create_member_call(context,
                                                    source_node,
                                                    value_expr,
                                                    "asString",
                                                    NULL,
                                                    0);
    if (!factory_args[0]) return NULL;

    return rxcp_remap_create_factory_call(context,
                                          source_node,
                                          LEVELC_REXX_VALUE_CLASS,
                                          factory_args,
                                          1);
}

static ASTNode *levelc_lower_binary_method(Context *context,
                                           ASTNode *expr,
                                           LevelCLowerPlan *plan,
                                           ASTNode *prelude,
                                           const char *method_name) {
    ASTNode *args[1];
    ASTNode *receiver;

    if (!method_name) return NULL;
    receiver = levelc_lower_expr(context, expr->child, plan, prelude);
    args[0] = levelc_lower_expr(context, expr->child->sibling, plan, prelude);
    if (!receiver || !args[0]) return NULL;

    return rxcp_remap_create_member_call(context, expr, receiver, method_name, args, 1);
}

static ASTNode *levelc_lower_unary_method(Context *context,
                                          ASTNode *expr,
                                          LevelCLowerPlan *plan,
                                          ASTNode *prelude,
                                          const char *method_name) {
    ASTNode *receiver;

    if (!method_name) return NULL;
    receiver = levelc_lower_expr(context, expr->child, plan, prelude);
    if (!receiver) return NULL;

    return rxcp_remap_create_member_call(context, expr, receiver, method_name, NULL, 0);
}

static ASTNode *levelc_if_logical_value(Context *context,
                                        ASTNode *source_node,
                                        ASTNode *value) {
    if (!value) return NULL;
    return rxcp_remap_create_member_call(context,
                                         source_node,
                                         value,
                                         "logicalIfValue",
                                         NULL,
                                         0);
}

static ASTNode *levelc_do_condition_logical_value(Context *context,
                                                  ASTNode *source_node,
                                                  ASTNode *value) {
    if (!value) return NULL;
    return rxcp_remap_create_member_call(context,
                                         source_node,
                                         value,
                                         source_node->node_type == WHILE
                                             ? "logicalWhileValue"
                                             : "logicalUntilValue",
                                         NULL,
                                         0);
}

static ASTNode *levelc_controlled_while_entry(Context *context,
                                               ASTNode *condition_node,
                                               LevelCLowerPlan *plan,
                                               ASTNode *to_guard) {
    ASTNode *prelude = rxcp_remap_create_instruction_builder(context, condition_node);
    ASTNode *condition_value = prelude
        ? levelc_lower_expr(context, condition_node->child, plan, prelude) : NULL;
    ASTNode *then_instructions;
    ASTNode *else_instructions;
    ASTNode *block_instructions;
    ASTNode *true_leave;
    ASTNode *false_leave;
    ASTNode *then_block;
    ASTNode *else_block;
    ASTNode *branch;
    ASTNode *block;

    condition_value = levelc_do_condition_logical_value(context, condition_node,
                                                        condition_value);
    if (!condition_value) return NULL;
    if (!to_guard) {
        return prelude->child
            ? rxcp_remap_create_prelude_block_expr(context, condition_node,
                                                   prelude, condition_value)
            : condition_value;
    }

    then_instructions = rxcp_remap_create_instruction_builder(context, condition_node);
    else_instructions = rxcp_remap_create_instruction_builder(context, condition_node);
    block_instructions = rxcp_remap_create_instruction_builder(context, condition_node);
    true_leave = ast_f(context, LEAVE_WITH, condition_node->token);
    false_leave = ast_f(context, LEAVE_WITH, condition_node->token);
    block = ast_f(context, BLOCK_EXPR, condition_node->token);
    if (!then_instructions || !else_instructions || !block_instructions ||
        !true_leave || !false_leave || !block) return NULL;
    rxcp_remap_anchor_synthetic(true_leave, condition_node);
    rxcp_remap_anchor_synthetic(false_leave, condition_node);
    rxcp_remap_anchor_synthetic(block, condition_node);
    rxcp_remap_append_builder_children(then_instructions, prelude);
    add_ast(true_leave, condition_value);
    add_ast(then_instructions, true_leave);
    add_ast(false_leave, rxcp_remap_create_integer_constant(context, condition_node,
                                                             0, TP_BOOLEAN));
    add_ast(else_instructions, false_leave);
    then_block = rxcp_remap_create_do_block(context, condition_node, then_instructions);
    else_block = rxcp_remap_create_do_block(context, condition_node, else_instructions);
    branch = then_block && else_block
        ? rxcp_remap_create_if_statement(context, condition_node, to_guard,
                                         then_block, else_block) : NULL;
    if (!branch) return NULL;
    add_ast(block_instructions, branch);
    add_ast(block, block_instructions);
    return block;
}

static ASTNode *levelc_controlled_until_end(Context *context,
                                             ASTNode *condition_node,
                                             LevelCLowerPlan *plan,
                                             ASTNode *step_prelude) {
    ASTNode *condition_prelude = rxcp_remap_create_instruction_builder(context,
                                                                        condition_node);
    ASTNode *condition_value = condition_prelude
        ? levelc_lower_expr(context, condition_node->child, plan,
                            condition_prelude) : NULL;
    ASTNode *end_instructions;
    ASTNode *step_instructions;
    ASTNode *true_leave;
    ASTNode *false_leave;
    ASTNode *step_block;
    ASTNode *branch;
    ASTNode *block;

    condition_value = levelc_do_condition_logical_value(context, condition_node,
                                                        condition_value);
    if (!condition_value || !step_prelude || !step_prelude->child) return NULL;
    end_instructions = rxcp_remap_create_instruction_builder(context, condition_node);
    step_instructions = rxcp_remap_create_instruction_builder(context, condition_node);
    true_leave = ast_f(context, LEAVE_WITH, condition_node->token);
    false_leave = ast_f(context, LEAVE_WITH, condition_node->token);
    block = ast_f(context, BLOCK_EXPR, condition_node->token);
    if (!end_instructions || !step_instructions || !true_leave ||
        !false_leave || !block) return NULL;
    rxcp_remap_anchor_synthetic(true_leave, condition_node);
    rxcp_remap_anchor_synthetic(false_leave, condition_node);
    rxcp_remap_anchor_synthetic(block, condition_node);
    add_ast(true_leave, rxcp_remap_create_integer_constant(context, condition_node,
                                                            1, TP_BOOLEAN));
    rxcp_remap_append_builder_children(step_instructions, step_prelude);
    add_ast(false_leave, rxcp_remap_create_integer_constant(context, condition_node,
                                                             0, TP_BOOLEAN));
    add_ast(step_instructions, false_leave);
    step_block = rxcp_remap_create_do_block(context, condition_node,
                                           step_instructions);
    branch = step_block
        ? rxcp_remap_create_if_statement(context, condition_node,
                                         condition_value, true_leave, step_block)
        : NULL;
    if (!branch) return NULL;
    rxcp_remap_append_builder_children(end_instructions, condition_prelude);
    add_ast(end_instructions, branch);
    add_ast(block, end_instructions);
    return block;
}

static char *levelc_capture_control_clause(Context *context,
                                            ASTNode *clause,
                                            LevelCLowerPlan *plan,
                                            ASTNode *header_setup,
                                            const char *name_prefix,
                                            const char *validation_method) {
    char *name;
    ASTNode *value;
    ASTNode *checked;
    ASTNode *capture;

    if (!context || !clause || !header_setup) return NULL;
    name = rxcp_remap_create_generated_node_name(name_prefix, clause);
    if (!name) return NULL;
    value = levelc_lower_expr(context, clause->child, plan, header_setup);
    checked = value
        ? rxcp_remap_create_member_call(context, clause, value,
                                        validation_method, NULL, 0)
        : NULL;
    capture = checked && name
        ? rxcp_remap_create_named_assignment(context, clause, name, checked)
        : NULL;
    if (!capture) {
        free(name);
        return NULL;
    }
    add_ast(header_setup, capture);
    return name;
}

static ASTNode *levelc_lower_classic_logical_binary(Context *context,
                                                    ASTNode *expr,
                                                    LevelCLowerPlan *plan,
                                                    ASTNode *prelude) {
    char *left_name;
    ASTNode *left;
    ASTNode *left_copy;
    ASTNode *left_assignment;
    ASTNode *left_ref;
    ASTNode *right;
    ASTNode *args[1];
    ASTNode *result;

    if (!context || !expr || !prelude ||
        !levelc_is_classic_logical_binary(expr->node_type)) return NULL;

    left_name = rxcp_remap_create_generated_node_name(LEVELC_EXPR_RESULT_PREFIX, expr);
    if (!left_name) return NULL;

    left = levelc_lower_expr(context, expr->child, plan, prelude);
    left_copy = levelc_copy_rexxvalue(context, expr->child, left);
    left_assignment = left_copy
        ? rxcp_remap_create_named_assignment(context, expr, left_name, left_copy)
        : NULL;
    if (!left_assignment) goto fail;
    add_ast(prelude, left_assignment);

    right = levelc_lower_expr(context, expr->child->sibling, plan, prelude);
    left_ref = rxcp_remap_create_named_ref(context, expr, VAR_SYMBOL, left_name);
    if (!right || !left_ref) goto fail;

    args[0] = right;
    result = rxcp_remap_create_member_call(context,
                                          expr,
                                          left_ref,
                                          expr->node_type == OP_AND
                                              ? "logicalAnd" : "logicalOr",
                                          args,
                                          1);
    free(left_name);
    return result;

fail:
    free(left_name);
    return NULL;
}

static ASTNode *levelc_lower_bif_dispatch_call(Context *context,
                                               ASTNode *expr,
                                               const char *bif_name,
                                               LevelCLowerPlan *plan,
                                               ASTNode *prelude,
                                               const char *callee_name,
                                               ASTNode *single_value_override) {
    char *args_name;
    char *exists_name;
    char *context_name;
    ASTNode *statement;
    ASTNode *receiver;
    ASTNode *member_args[2];
    ASTNode *call_args[1];
    ASTNode *function_args[1];
    ASTNode *arg;
    size_t arg_count;
    size_t index;

    if (!context || !expr || !bif_name || !prelude) return NULL;

    args_name = rxcp_remap_create_generated_node_name(LEVELC_BIF_ARGS_PREFIX, expr);
    exists_name = rxcp_remap_create_generated_node_name(LEVELC_BIF_EXISTS_PREFIX, expr);
    context_name = rxcp_remap_create_generated_node_name(LEVELC_BIF_CONTEXT_PREFIX, expr);
    if (!args_name || !exists_name || !context_name) {
        if (args_name) free(args_name);
        if (exists_name) free(exists_name);
        if (context_name) free(context_name);
        return NULL;
    }

    {
        RxcpRemapArgumentFrameSpec frame;

        frame.values_name = args_name;
        frame.values_class_name = LEVELC_REXX_VALUE_CLASS_TYPE;
        frame.provided_name = exists_name;
        frame.provided_class_name = LEVELC_INT_CLASS_TYPE;
        if (!rxcp_remap_begin_argument_frame(context, prelude, expr, &frame)) goto fail;
    }

    arg_count = levelc_function_argument_count(expr);
    if (single_value_override && arg_count != 1) goto fail;
    arg = expr->child;
    index = 1;
    while (index <= arg_count) {
        ASTNode *value_rhs;
        ASTNode *value_copy;
        int arg_provided;

        if (!arg) goto fail;
        arg_provided = levelc_argument_exists(arg);
        if (arg_provided) {
            value_rhs = single_value_override ? single_value_override
                                              : levelc_lower_expr(context, arg, plan, prelude);
            value_copy = levelc_copy_rexxvalue(context, arg, value_rhs);
        } else {
            value_rhs = levelc_blank_rexxvalue(context, arg);
            value_copy = value_rhs;
        }
        if (!value_rhs || !value_copy) goto fail;

        {
            RxcpRemapArgumentFrameSpec frame;

            frame.values_name = args_name;
            frame.values_class_name = LEVELC_REXX_VALUE_CLASS_TYPE;
            frame.provided_name = exists_name;
            frame.provided_class_name = LEVELC_INT_CLASS_TYPE;
            if (!rxcp_remap_append_argument_frame_slot(context,
                                                       prelude,
                                                       arg,
                                                       &frame,
                                                       (int)index,
                                                       value_copy,
                                                       arg_provided)) {
                goto fail;
            }
        }

        arg = arg->sibling;
        index++;
    }

    call_args[0] = rxcp_remap_create_string_constant(context, expr, bif_name);
    statement = rxcp_remap_create_named_assignment(
            context,
            expr,
            context_name,
            rxcp_remap_create_factory_call(context,
                                           expr,
                                           LEVELC_BIF_CONTEXT_CLASS,
                                           call_args,
                                           1));
    if (!statement) goto fail;
    add_ast(prelude, statement);

    receiver = rxcp_remap_create_named_ref(context, expr, VAR_SYMBOL, context_name);
    member_args[0] = rxcp_remap_create_named_ref(context, expr, VAR_SYMBOL, args_name);
    member_args[1] = rxcp_remap_create_named_ref(context, expr, VAR_SYMBOL, exists_name);
    statement = rxcp_remap_create_member_call_statement(context,
                                                        expr,
                                                        receiver,
                                                        "setArguments",
                                                        member_args,
                                                        2);
    if (!statement) goto fail;
    add_ast(prelude, statement);

    receiver = rxcp_remap_create_named_ref(context, expr, VAR_SYMBOL, context_name);
    member_args[0] = rxcp_remap_create_reference_expr(
            context,
            expr,
            levelc_pool_ref(context, expr, VAR_SYMBOL));
    statement = rxcp_remap_create_member_call_statement(context,
                                                        expr,
                                                        receiver,
                                                        "setCallerPool",
                                                        member_args,
                                                        1);
    if (!statement) goto fail;
    add_ast(prelude, statement);

    function_args[0] = rxcp_remap_create_reference_expr(
            context,
            expr,
            rxcp_remap_create_named_ref(context, expr, VAR_SYMBOL, context_name));
    if (!function_args[0]) goto fail;

    free(args_name);
    free(exists_name);
    free(context_name);
    return rxcp_remap_create_function_call(context,
                                           expr,
                                           callee_name,
                                           function_args,
                                           1);

fail:
    free(args_name);
    free(exists_name);
    free(context_name);
    return NULL;
}

static ASTNode *levelc_lower_local_function_call(Context *context,
                                                 ASTNode *expr,
                                                 LevelCLowerPlan *plan,
                                                 ASTNode *prelude) {
    char *target_name;
    char *function_name;
    LevelCProcedureSlice *procedure;
    ASTNode **args;
    ASTNode *pool_symbol;
    ASTNode *arg;
    ASTNode *call;
    size_t index;

    if (!context || !expr || !plan) return NULL;

    target_name = levelc_upper_name(expr);
    procedure = target_name ? levelc_find_procedure(plan, target_name) : NULL;
    if (!procedure || !procedure->returns_value ||
        levelc_function_argument_count(expr) != procedure->arg_count) {
        if (target_name) free(target_name);
        return NULL;
    }

    function_name = levelc_generated_proc_name(target_name, 0);
    free(target_name);
    if (!function_name) return NULL;

    args = calloc(procedure->arg_count + 1, sizeof(ASTNode *));
    if (!args) {
        free(function_name);
        return NULL;
    }

    pool_symbol = levelc_pool_ref(context, expr, VAR_SYMBOL);
    args[0] = rxcp_remap_create_reference_expr(context, expr, pool_symbol);
    if (!pool_symbol || !args[0]) {
        free(function_name);
        free(args);
        return NULL;
    }

    arg = expr->child;
    if (arg && arg->node_type == NOVAL && !arg->sibling) arg = NULL;
    index = 1;
    while (arg) {
        ASTNode *actual_value;

        actual_value = levelc_lower_expr(context, arg, plan, prelude);
        args[index] = levelc_copy_rexxvalue(context, arg, actual_value);
        if (!actual_value || !args[index]) {
            free(function_name);
            free(args);
            return NULL;
        }
        arg = arg->sibling;
        index++;
    }

    call = rxcp_remap_create_function_call(context,
                                           expr,
                                           function_name,
                                           args,
                                           procedure->arg_count + 1);
    free(function_name);
    free(args);
    return call;
}

static ASTNode *levelc_lower_function_call(Context *context,
                                           ASTNode *expr,
                                           LevelCLowerPlan *plan,
                                           ASTNode *prelude) {
    char *name;
    ASTNode *args[1];
    ASTNode *call;

    if (!context || !expr || expr->node_type != FUNCTION) return NULL;

    name = levelc_upper_name(expr);
    if (!name) return NULL;

    if (strcmp(name, "LENGTH") == 0 && expr->child && !expr->child->sibling) {
        args[0] = levelc_lower_expr(context, expr->child, plan, prelude);
        free(name);
        if (!args[0]) return NULL;

        call = rxcp_remap_create_function_call(context,
                                               expr,
                                               LEVELC_BIF_LENGTH_HELPER,
                                               args,
                                               1);
        return call;
    }

    if (strcmp(name, "SUBSTR") == 0) {
        free(name);
        return levelc_lower_bif_dispatch_call(context, expr, "SUBSTR", plan,
                                              prelude, LEVELC_BIF_DISPATCH_HELPER, NULL);
    }

    free(name);
    return levelc_lower_local_function_call(context, expr, plan, prelude);
}

static ASTNode *levelc_materialise_compound_tail(Context *context,
                                                 ASTNode *source_node,
                                                 const char *name,
                                                 ASTNode *prelude) {
    char *tail;
    char *tail_name;
    ASTNode *tail_expr;
    ASTNode *tail_ref;
    ASTNode *statement;

    if (!context || !source_node || !name || !prelude) return NULL;

    tail = levelc_compound_tail_name(name);
    if (!tail) return NULL;
    if (levelc_tail_is_numeric_literal(tail)) {
        ASTNode *literal_tail;

        literal_tail = rxcp_remap_create_string_constant(context, source_node, tail);
        free(tail);
        return literal_tail;
    }
    free(tail);

    tail_expr = levelc_compound_tail_expr(context, source_node, name);
    tail_name = rxcp_remap_create_generated_node_name(LEVELC_COMPOUND_TAIL_PREFIX, source_node);
    if (!tail_expr || !tail_name) {
        if (tail_name) free(tail_name);
        return NULL;
    }

    statement = rxcp_remap_create_named_assignment(context,
                                                   source_node,
                                                   tail_name,
                                                   tail_expr);
    if (!statement) {
        free(tail_name);
        return NULL;
    }
    tail_ref = rxcp_remap_create_named_ref(context, source_node, VAR_SYMBOL, tail_name);
    free(tail_name);
    if (!tail_ref) return NULL;

    add_ast(prelude, statement);
    return tail_ref;
}

static ASTNode *levelc_lower_expr(Context *context,
                                  ASTNode *expr,
                                  LevelCLowerPlan *plan,
                                  ASTNode *prelude) {
    const char *method;

    if (!context || !expr) return NULL;

    switch (expr->node_type) {
        case STRING:
        case INTEGER:
        case DECIMAL:
            return levelc_rexxvalue_from_literal(context, expr);
        case VAR_SYMBOL:
            return levelc_pool_value(context, expr);
        case FUNCTION:
            return levelc_lower_function_call(context, expr, plan, prelude);
        default:
            if (levelc_is_classic_logical_binary(expr->node_type)) {
                return levelc_lower_classic_logical_binary(context, expr, plan, prelude);
            }
            method = levelc_binary_operator_method(expr->node_type);
            if (method) {
                return levelc_lower_binary_method(context, expr, plan, prelude, method);
            }
            method = levelc_unary_operator_method(expr->node_type);
            if (method) {
                return levelc_lower_unary_method(context, expr, plan, prelude, method);
            }
            return NULL;
    }
}

static ASTNode *levelc_pool_set_statement(Context *context,
                                          ASTNode *assign_node,
                                          LevelCLowerPlan *plan,
                                          ASTNode *prelude,
                                          ASTNode *value_override) {
    ASTNode *target;
    ASTNode *expr;
    ASTNode *receiver;
    ASTNode *args[3];
    char *target_name;
    LevelCVariableNameKind target_kind;
    const char *method_name;
    size_t arg_count;

    target = assign_node->child;
    expr = target ? target->sibling : NULL;
    if (!target || !expr) return NULL;

    target_name = levelc_upper_name(target);
    if (!target_name) return NULL;
    target_kind = levelc_variable_name_kind(target_name);

    receiver = levelc_pool_ref(context, assign_node, VAR_SYMBOL);
    if (!receiver) {
        free(target_name);
        return NULL;
    }

    if (target_kind == LEVELC_VAR_NAME_COMPOUND) {
        args[0] = levelc_compound_stem_string(context, target, target_name);
        args[1] = levelc_materialise_compound_tail(context, target, target_name, prelude);
        args[2] = value_override ? value_override
                                 : levelc_lower_expr(context, expr, plan, prelude);
        method_name = "setStemValue";
        arg_count = 3;
    } else {
        args[0] = levelc_name_string(context, target);
        args[1] = value_override ? value_override
                                 : levelc_lower_expr(context, expr, plan, prelude);
        args[2] = NULL;
        method_name = "setValue";
        arg_count = 2;
    }
    free(target_name);

    if (!args[0] || !args[1] || (arg_count == 3 && !args[2])) return NULL;

    return rxcp_remap_create_member_call_statement(context,
                                                   assign_node,
                                                   receiver,
                                                   method_name,
                                                   args,
                                                   arg_count);
}

static ASTNode *levelc_say_statement(Context *context,
                                     ASTNode *say_node,
                                     LevelCLowerPlan *plan,
                                     ASTNode *prelude) {
    ASTNode *lowered_expr;
    ASTNode *as_string;
    ASTNode *say;

    lowered_expr = levelc_lower_expr(context, say_node->child, plan, prelude);
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
    rhs = rxcp_remap_create_dereference_expr(context,
                                             anchor_node ? anchor_node : assign,
                                             rhs_operand);
    if (!lhs || !rhs_operand || !rhs) return NULL;

    add_ast(assign, lhs);
    add_ast(assign, rhs);
    return assign;
}

static ASTNode *levelc_lower_call_tail_value(Context *context,
                                             ASTNode *node,
                                             LevelCLowerPlan *plan,
                                             ASTNode *prelude) {
    (void)plan;
    (void)prelude;

    if (!node) return NULL;
    if (node->node_type == LITERAL) return levelc_pool_value(context, node);
    if (node->node_type == INTEGER || node->node_type == DECIMAL)
        return levelc_rexxvalue_from_literal(context, node);
    return NULL;
}

static ASTNode *levelc_call_local_procedure_statement(Context *context,
                                                      ASTNode *call_node,
                                                      LevelCLowerPlan *plan,
                                                      ASTNode *prelude) {
    char *target_name;
    char *function_name;
    LevelCProcedureSlice *procedure;
    ASTNode **args;
    ASTNode *tail;
    ASTNode *tail_node;
    ASTNode *pool_symbol;
    ASTNode *call_expr;
    ASTNode *statement;
    size_t arg_index;

    target_name = levelc_call_target_name(call_node);
    procedure = target_name ? levelc_find_procedure(plan, target_name) : NULL;
    if (!target_name || !procedure) {
        if (target_name) free(target_name);
        return NULL;
    }

    function_name = levelc_generated_proc_name(target_name, 0);
    free(target_name);
    if (!function_name) return NULL;

    args = calloc(procedure->arg_count + 1, sizeof(ASTNode *));
    if (!args) {
        free(function_name);
        return NULL;
    }

    pool_symbol = levelc_pool_ref(context, call_node, VAR_SYMBOL);
    args[0] = rxcp_remap_create_reference_expr(context, call_node, pool_symbol);
    if (!pool_symbol || !args[0]) {
        free(function_name);
        free(args);
        return NULL;
    }

    tail = call_node->child ? call_node->child->sibling : NULL;
    tail_node = tail ? tail->child : NULL;
    arg_index = 1;
    while (tail_node) {
        if (tail_node->node_type == TOKEN && tail_node->node_string &&
            strcmp(tail_node->node_string, ",") == 0) {
            tail_node = tail_node->sibling;
            continue;
        }
        if (arg_index > procedure->arg_count) {
            free(function_name);
            free(args);
            return NULL;
        }
        {
            ASTNode *actual_value;

            actual_value = levelc_lower_call_tail_value(context,
                                                        tail_node,
                                                        plan,
                                                        prelude);
            args[arg_index] = levelc_copy_rexxvalue(context,
                                                    tail_node,
                                                    actual_value);
            if (!actual_value || !args[arg_index]) {
                free(function_name);
                free(args);
                return NULL;
            }
        }
        arg_index++;
        tail_node = tail_node->sibling;
    }

    call_expr = rxcp_remap_create_function_call(context,
                                                call_node,
                                                function_name,
                                                args,
                                                procedure->arg_count + 1);
    free(function_name);
    free(args);
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
    char *name;
    const char *method_name;

    name = levelc_upper_name(expose_target);
    if (!name) return NULL;

    receiver = levelc_pool_ref(context, expose_target, VAR_SYMBOL);
    if (levelc_name_is_stem(name)) {
        method_name = "exposeStem";
        args[0] = rxcp_remap_create_string_constant(context, expose_target, name);
    } else {
        method_name = "exposeValue";
        args[0] = levelc_name_string(context, expose_target);
    }
    parent_symbol = levelc_parent_pool_ref(context, expose_target, VAR_SYMBOL);
    args[1] = rxcp_remap_create_reference_expr(context, expose_target, parent_symbol);
    args[2] = rxcp_remap_create_string_constant(context, expose_target, name);
    free(name);
    if (!receiver || !args[0] || !parent_symbol || !args[1] || !args[2]) return NULL;

    return rxcp_remap_create_member_call_statement(context,
                                                   procedure_node,
                                                   receiver,
                                                   method_name,
                                                   args,
                                                   3);
}

static ASTNode *levelc_procedure_header(Context *context,
                                        LevelCProcedureSlice *procedure) {
    char *name;
    ASTNode *node;
    ASTNode *return_type;

    if (!context || !procedure || !procedure->name) return NULL;

    name = levelc_generated_proc_name(procedure->name, 1);
    if (!name) return NULL;

    if (procedure->returns_value) {
        return_type = rxcp_remap_create_class_type(context,
                                                   procedure->procedure,
                                                   LEVELC_REXX_VALUE_CLASS_TYPE);
    } else {
        return_type = rxcp_remap_create_void_type(context, procedure->procedure);
    }
    node = return_type ? rxcp_remap_create_procedure_header(context,
                                                           procedure->label,
                                                           name,
                                                           return_type) : NULL;
    free(name);
    return node;
}

static ASTNode *levelc_procedure_args(Context *context,
                                      LevelCProcedureSlice *procedure) {
    ASTNode *args;
    ASTNode *arg;
    ASTNode *target;
    ASTNode *type_ref;
    ASTNode *class_node;
    size_t index;

    if (!context || !procedure) return NULL;

    args = rxcp_remap_create_args_builder(context, procedure->procedure);
    target = levelc_parent_pool_ref_symbol(context, procedure->procedure, VAR_TARGET);
    type_ref = rxcp_remap_create_reference_type(context,
                                                procedure->procedure,
                                                ".RexxVariablePool");
    arg = rxcp_remap_create_arg(context, procedure->procedure, target, type_ref);
    if (!args || !arg) return NULL;

    add_ast(args, arg);

    for (index = 1; index <= procedure->arg_count; index++) {
        char *arg_name;

        arg_name = levelc_generated_arg_name(index);
        target = arg_name ? rxcp_remap_create_named_ref(context,
                                                        procedure->procedure,
                                                        VAR_TARGET,
                                                        arg_name) : NULL;
        class_node = rxcp_remap_create_class_type(context,
                                                  procedure->procedure,
                                                  LEVELC_REXX_VALUE_CLASS_TYPE);
        if (arg_name) free(arg_name);
        arg = rxcp_remap_create_arg(context, procedure->procedure, target, class_node);
        if (!arg) return NULL;

        add_ast(args, arg);
    }

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

static ASTNode *levelc_build_options(Context *context,
                                     ASTNode *anchor_node,
                                     int needs_translate) {
    ASTNode *options;
    ASTNode *levelb;
    ASTNode *comments_dash;
    ASTNode *numeric_classic;
    ASTNode *import_value;
    ASTNode *import_pool;
    ASTNode *import_bifs;
    ASTNode *import_translate;

    options = ast_f(context, REXX_OPTIONS, anchor_node ? anchor_node->token : NULL);
    if (!options) return NULL;
    if (anchor_node) rxcp_remap_anchor_synthetic(options, anchor_node);

    levelb = rxcp_remap_create_literal(context, anchor_node ? anchor_node : options, "levelb");
    comments_dash = rxcp_remap_create_literal(context, anchor_node ? anchor_node : options, "comments_dash");
    numeric_classic = rxcp_remap_create_literal(context, anchor_node ? anchor_node : options, "numeric_classic");
    import_value = rxcp_remap_create_generated_import(context, anchor_node ? anchor_node : options, "rexxvalue");
    import_pool = rxcp_remap_create_generated_import(context, anchor_node ? anchor_node : options, "rexxpool");
    import_bifs = rxcp_remap_create_generated_import(context, anchor_node ? anchor_node : options, "rexxclassicbifs");
    import_translate = needs_translate
        ? rxcp_remap_create_generated_import(context,
                                             anchor_node ? anchor_node : options,
                                             "rexxclassicbiftranslate")
        : NULL;
    if (!levelb || !comments_dash || !numeric_classic || !import_value ||
        !import_pool || !import_bifs || (needs_translate && !import_translate)) return NULL;

    add_ast(options, levelb);
    add_ast(options, comments_dash);
    add_ast(options, numeric_classic);
    add_ast(options, import_value);
    add_ast(options, import_pool);
    add_ast(options, import_bifs);
    if (import_translate) add_ast(options, import_translate);
    return options;
}

static int levelc_append_arg_bindings(Context *context,
                                      ASTNode *instructions,
                                      LevelCProcedureSlice *procedure,
                                      const char **reason_out) {
    ASTNode *templates;
    ASTNode *template_node;
    ASTNode *target;
    ASTNode *receiver;
    ASTNode *member_args[2];
    ASTNode *statement;
    ASTNode *function;
    ASTNode *function_arg;
    ASTNode *arg_value;
    size_t index;

    if (!procedure || !procedure->arg_statement) return 1;

    templates = procedure->arg_statement->child;
    template_node = templates ? templates->child : NULL;
    index = 1;
    while (template_node) {
        char *arg_name;

        target = levelc_arg_template_target(template_node);
        arg_name = levelc_generated_arg_name(index);
        receiver = levelc_pool_ref(context, target ? target : template_node, VAR_SYMBOL);
        member_args[0] = target ? levelc_name_string(context, target) : NULL;
        function = target ? ast_f(context, FUNCTION, target->token) : NULL;
        function_arg = target ? ast_f(context, VAR_SYMBOL, target->token) : NULL;
        if (function && function_arg) add_ast(function, function_arg);
        arg_value = arg_name && target
            ? rxcp_remap_create_named_ref(context, target, VAR_SYMBOL, arg_name)
            : NULL;
        member_args[1] = function && function_arg && arg_value
            ? levelc_lower_bif_dispatch_call(context, function, "TRANSLATE", NULL,
                                             instructions,
                                             LEVELC_BIF_TRANSLATE_HELPER, arg_value)
            : NULL;
        if (arg_name) free(arg_name);
        if (!receiver || !member_args[0] || !member_args[1]) {
            if (reason_out) *reason_out = "failed to create ARG binding";
            return 0;
        }

        statement = rxcp_remap_create_member_call_statement(context,
                                                            procedure->arg_statement,
                                                            receiver,
                                                            "setValue",
                                                            member_args,
                                                            2);
        if (!statement) {
            if (reason_out) *reason_out = "failed to create ARG binding statement";
            return 0;
        }
        add_ast(instructions, statement);

        template_node = template_node->sibling;
        index++;
    }

    return 1;
}

static ASTNode *levelc_proc_return_statement(Context *context,
                                             ASTNode *stmt,
                                             LevelCLowerPlan *plan,
                                             LevelCProcedureSlice *procedure,
                                             ASTNode *prelude) {
    ASTNode *return_stmt;
    ASTNode *return_value;

    return_stmt = rxcp_remap_create_return_statement(context, stmt);
    if (!return_stmt) return NULL;

    if (stmt->child) {
        return_value = levelc_lower_expr(context, stmt->child, plan, prelude);
        if (!return_value) return NULL;
        add_ast(return_stmt, return_value);
    } else if (procedure && procedure->returns_value) {
        return_value = levelc_blank_rexxvalue(context, stmt);
        if (!return_value) return NULL;
        add_ast(return_stmt, return_value);
    }

    return return_stmt;
}

static int levelc_lower_if_statement(Context *context,
                                    ASTNode *instructions,
                                    ASTNode *stmt,
                                    LevelCLowerPlan *plan,
                                    LevelCProcedureSlice *procedure,
                                    int in_procedure,
                                    const char **reason_out);
static int levelc_lower_do(Context *context,
                           ASTNode *instructions,
                           ASTNode *stmt,
                           LevelCLowerPlan *plan,
                           LevelCProcedureSlice *procedure,
                           int in_procedure,
                           const char **reason_out);
static int levelc_lower_select_statement(Context *context,
                                         ASTNode *instructions,
                                         ASTNode *stmt,
                                         LevelCLowerPlan *plan,
                                         LevelCProcedureSlice *procedure,
                                         int in_procedure,
                                         const char **reason_out);

static int levelc_lower_nop(Context *context,
                            ASTNode *instructions,
                            ASTNode *stmt,
                            const char **reason_out) {
    ASTNode *lowered = ast_f(context, NOP, stmt->token);

    if (!lowered) {
        if (reason_out) *reason_out = "failed to lower Level C NOP";
        return 0;
    }
    rxcp_remap_anchor_synthetic(lowered, stmt);
    add_ast(instructions, lowered);
    return 1;
}

static int levelc_lower_transfer(Context *context,
                                ASTNode *instructions,
                                ASTNode *stmt,
                                LevelCLowerPlan *plan,
                                const char **reason_out) {
    LevelCLoopBinding *binding = plan ? plan->active_loop : NULL;
    ASTNode *lowered;
    ASTNode *target;
    ASTNode *source_target = stmt->child
        ? levelc_named_source_repetitive_do(stmt)
        : levelc_nearest_source_repetitive_do(stmt);

    while (binding && binding->source_do != source_target) binding = binding->previous;
    if (!source_target || !binding) {
        if (reason_out) *reason_out = "LEAVE/ITERATE lost its source loop binding";
        return 0;
    }
    lowered = ast_f(context, stmt->node_type, stmt->token);
    target = rxcp_remap_create_named_ref(context, stmt, VAR_SYMBOL,
                                         binding->control_name);
    if (!lowered || !target) {
        if (reason_out) *reason_out = "failed to lower Level C loop transfer";
        return 0;
    }
    rxcp_remap_anchor_synthetic(lowered, stmt);
    add_ast(lowered, target);
    add_ast(instructions, lowered);
    return 1;
}

static int levelc_lower_scalar_drop(Context *context,
                                    ASTNode *instructions,
                                    ASTNode *stmt,
                                    const char **reason_out) {
    ASTNode *target = stmt->child->child;

    while (target) {
        ASTNode *receiver = levelc_pool_ref(context, target, VAR_SYMBOL);
        ASTNode *args[1] = {levelc_name_string(context, target)};
        ASTNode *lowered;

        if (!receiver || !args[0]) goto fail;
        lowered = rxcp_remap_create_member_call_statement(context, target,
                                                            receiver, "drop", args, 1);
        if (!lowered) goto fail;
        add_ast(instructions, lowered);
        target = target->sibling;
    }
    return 1;

fail:
    if (reason_out) *reason_out = "failed to lower Level C DROP";
    return 0;
}

static int levelc_lower_direct_parse(Context *context,
                                     ASTNode *instructions,
                                     ASTNode *stmt,
                                     LevelCLowerPlan *plan,
                                     const char **reason_out) {
    ASTNode *source;
    ASTNode *target;
    ASTNode *prelude;
    ASTNode *value;
    ASTNode *function;
    ASTNode *receiver;
    ASTNode *args[2];
    ASTNode *lowered;
    size_t target_count;
    size_t index;
    int is_value;
    int upper;

    if (!levelc_direct_parse_shape(stmt, plan, &source, &target, &target_count,
                                   &is_value, &upper, reason_out)) return 0;
    prelude = rxcp_remap_create_instruction_builder(context, stmt);
    if (!prelude) goto fail;
    value = is_value ? levelc_lower_expr(context, source->child, plan, prelude)
                     : levelc_pool_value(context, source);
    if (!value) goto fail;
    if (upper) {
        function = ast_f(context, FUNCTION, target->token);
        if (!function) goto fail;
        add_ast(function, ast_f(context, VAR_SYMBOL, target->token));
        value = levelc_lower_bif_dispatch_call(context, function, "TRANSLATE", NULL,
                                               prelude, LEVELC_BIF_TRANSLATE_HELPER,
                                               value);
        if (!value) goto fail;
    }

    if (target_count == 3) {
        char *fields_name = rxcp_remap_create_generated_node_name(
            LEVELC_PARSE_FIELDS_PREFIX, stmt);
        ASTNode *fields_define = fields_name
            ? rxcp_remap_create_array_define(context, stmt, fields_name,
                                             LEVELC_REXX_VALUE_CLASS_TYPE)
            : NULL;
        ASTNode *split = rxcp_remap_create_member_call(context, stmt, value,
                                                        "parseThreeWords", NULL, 0);
        ASTNode *capture = fields_name && split
            ? rxcp_remap_create_named_assignment(context, stmt, fields_name, split)
            : NULL;
        if (!fields_define || !capture) {
            free(fields_name);
            goto fail;
        }
        add_ast(prelude, fields_define);
        add_ast(prelude, capture);
        rxcp_remap_append_builder_children(instructions, prelude);
        for (index = 1; index <= 3; index++, target = target->sibling) {
            receiver = levelc_pool_ref(context, target, VAR_SYMBOL);
            args[0] = levelc_name_string(context, target);
            args[1] = rxcp_remap_create_indexed_ref(context, target, VAR_SYMBOL,
                                                    fields_name, (int)index);
            lowered = receiver && args[0] && args[1]
                ? rxcp_remap_create_member_call_statement(context, target,
                                                          receiver, "setValue", args, 2)
                : NULL;
            if (!lowered) {
                free(fields_name);
                goto fail;
            }
            add_ast(instructions, lowered);
        }
        free(fields_name);
        return 1;
    }

    receiver = levelc_pool_ref(context, target, VAR_SYMBOL);
    args[0] = levelc_name_string(context, target);
    args[1] = value;
    lowered = receiver && args[0]
        ? rxcp_remap_create_member_call_statement(context, stmt, receiver,
                                                  "setValue", args, 2)
        : NULL;
    if (!lowered) goto fail;
    rxcp_remap_append_builder_children(instructions, prelude);
    add_ast(instructions, lowered);
    return 1;

fail:
    if (reason_out) *reason_out = "failed to lower supported PARSE shape";
    return 0;
}

static int levelc_lower_main_statement(Context *context,
                                       ASTNode *instructions,
                                       ASTNode *stmt,
                                       LevelCLowerPlan *plan,
                                       const char **reason_out) {
    ASTNode *prelude;
    ASTNode *lowered;

    if (!stmt) return 1;
    if (stmt->node_type == NOP) return levelc_lower_nop(context, instructions, stmt, reason_out);
    if (stmt->node_type == LEAVE || stmt->node_type == ITERATE)
        return levelc_lower_transfer(context, instructions, stmt, plan, reason_out);
    if (stmt->node_type == LEVELC_DROP) {
        return levelc_lower_scalar_drop(context, instructions, stmt, reason_out);
    }
    if (stmt->node_type == PARSE)
        return levelc_lower_direct_parse(context, instructions, stmt, plan, reason_out);
    if (stmt->node_type == IF) {
        return levelc_lower_if_statement(context, instructions, stmt, plan, NULL, 0, reason_out);
    }
    if (stmt->node_type == DO) {
        return levelc_lower_do(context, instructions, stmt, plan, NULL, 0, reason_out);
    }
    if (stmt->node_type == SELECT) {
        return levelc_lower_select_statement(context, instructions, stmt, plan, NULL, 0, reason_out);
    }

    prelude = rxcp_remap_create_instruction_builder(context, stmt);
    if (!prelude) {
        if (reason_out) *reason_out = "failed to create Level C statement prelude";
        return 0;
    }

    if (stmt->node_type == ASSIGN) {
        lowered = levelc_pool_set_statement(context, stmt, plan, prelude, NULL);
    } else if (stmt->node_type == SAY) {
        lowered = levelc_say_statement(context, stmt, plan, prelude);
    } else if (stmt->node_type == CALL) {
        lowered = levelc_call_local_procedure_statement(context, stmt, plan, prelude);
    } else if (stmt->node_type == EXIT) {
        lowered = rxcp_remap_create_return_statement(context, stmt);
    } else {
        lowered = NULL;
    }

    if (!lowered) {
        if (reason_out) *reason_out = "failed to lower supported Level C statement";
        return 0;
    }

    rxcp_remap_append_builder_children(instructions, prelude);
    add_ast(instructions, lowered);
    return 1;
}

static int levelc_lower_proc_statement(Context *context,
                                       ASTNode *instructions,
                                       ASTNode *stmt,
                                       LevelCLowerPlan *plan,
                                       LevelCProcedureSlice *procedure,
                                       const char **reason_out) {
    ASTNode *prelude;
    ASTNode *lowered;

    if (!stmt) return 1;
    if (stmt->node_type == NOP) return levelc_lower_nop(context, instructions, stmt, reason_out);
    if (stmt->node_type == LEAVE || stmt->node_type == ITERATE)
        return levelc_lower_transfer(context, instructions, stmt, plan, reason_out);
    if (stmt->node_type == LEVELC_DROP) {
        return levelc_lower_scalar_drop(context, instructions, stmt, reason_out);
    }
    if (stmt->node_type == LEVELC_ARG) {
        return levelc_append_arg_bindings(context, instructions, procedure, reason_out);
    }
    if (stmt->node_type == PARSE)
        return levelc_lower_direct_parse(context, instructions, stmt, plan, reason_out);
    if (stmt->node_type == IF) {
        return levelc_lower_if_statement(context, instructions, stmt, plan,
                                        procedure, 1, reason_out);
    }
    if (stmt->node_type == DO) {
        return levelc_lower_do(context, instructions, stmt, plan,
                               procedure, 1, reason_out);
    }
    if (stmt->node_type == SELECT) {
        return levelc_lower_select_statement(context, instructions, stmt, plan,
                                             procedure, 1, reason_out);
    }

    prelude = rxcp_remap_create_instruction_builder(context, stmt);
    if (!prelude) {
        if (reason_out) *reason_out = "failed to create Level C procedure statement prelude";
        return 0;
    }

    if (stmt->node_type == ASSIGN) {
        lowered = levelc_pool_set_statement(context, stmt, plan, prelude, NULL);
    } else if (stmt->node_type == SAY) {
        lowered = levelc_say_statement(context, stmt, plan, prelude);
    } else if (stmt->node_type == RETURN) {
        lowered = levelc_proc_return_statement(context, stmt, plan, procedure, prelude);
    } else {
        lowered = NULL;
    }

    if (!lowered) {
        if (reason_out) *reason_out = "failed to lower supported Level C procedure statement";
        return 0;
    }

    rxcp_remap_append_builder_children(instructions, prelude);
    add_ast(instructions, lowered);
    return 1;
}

static int levelc_lower_if_statement(Context *context,
                                    ASTNode *instructions,
                                    ASTNode *stmt,
                                    LevelCLowerPlan *plan,
                                    LevelCProcedureSlice *procedure,
                                    int in_procedure,
                                    const char **reason_out) {
    ASTNode *condition_node = stmt->child;
    ASTNode *then_node = condition_node->sibling;
    ASTNode *else_node = then_node->sibling;
    ASTNode *prelude = rxcp_remap_create_instruction_builder(context, stmt);
    ASTNode *then_instructions = rxcp_remap_create_instruction_builder(context, then_node);
    ASTNode *else_instructions = else_node
        ? rxcp_remap_create_instruction_builder(context, else_node) : NULL;
    ASTNode *condition;
    ASTNode *then_block;
    ASTNode *else_block = NULL;
    ASTNode *lowered;

    if (!prelude || !then_instructions || (else_node && !else_instructions)) goto fail;
    condition = levelc_lower_expr(context, condition_node, plan, prelude);
    condition = levelc_if_logical_value(context, condition_node, condition);
    if (!condition) goto fail;

    if (in_procedure) {
        if (!levelc_lower_proc_statement(context, then_instructions, then_node,
                                        plan, procedure, reason_out)) return 0;
    } else if (!levelc_lower_main_statement(context, then_instructions, then_node,
                                            plan, reason_out)) return 0;
    then_block = rxcp_remap_create_do_block(context, then_node, then_instructions);
    if (!then_block) goto fail;

    if (else_node) {
        if (in_procedure) {
            if (!levelc_lower_proc_statement(context, else_instructions, else_node,
                                            plan, procedure, reason_out)) return 0;
        } else if (!levelc_lower_main_statement(context, else_instructions, else_node,
                                                plan, reason_out)) return 0;
        else_block = rxcp_remap_create_do_block(context, else_node, else_instructions);
        if (!else_block) goto fail;
    }

    lowered = rxcp_remap_create_if_statement(context, stmt, condition,
                                             then_block, else_block);
    if (!lowered) goto fail;
    rxcp_remap_append_builder_children(instructions, prelude);
    add_ast(instructions, lowered);
    return 1;

fail:
    if (reason_out) *reason_out = "failed to lower supported Level C IF";
    return 0;
}

static int levelc_lower_do(Context *context,
                           ASTNode *instructions,
                           ASTNode *stmt,
                           LevelCLowerPlan *plan,
                           LevelCProcedureSlice *procedure,
                           int in_procedure,
                           const char **reason_out) {
    ASTNode *body = stmt->child;
    ASTNode *repeat = body && body->node_type == REPEAT ? body : NULL;
    ASTNode *possible_condition = repeat ? repeat->sibling : body;
    ASTNode *condition_node = possible_condition &&
        (possible_condition->node_type == WHILE ||
         possible_condition->node_type == UNTIL) ? possible_condition : NULL;
    ASTNode *body_statement;
    ASTNode *lowered_body = rxcp_remap_create_instruction_builder(context, stmt);
    ASTNode *lowered = NULL;
    ASTNode *initial_statement = NULL;
    ASTNode *header_setup = NULL;
    LevelCLoopBinding binding;
    LevelCLoopBinding *previous = plan ? plan->active_loop : NULL;
    char *control_name = NULL;
    int loop_active = 0;
    int count = 1;
    int forever = 0;
    int controlled = repeat && repeat->child && repeat->child->node_type == ASSIGN;

    if (!lowered_body) goto fail;
    if (repeat || condition_node) {
        ASTNode *count_node;
        ASTNode *condition_value = NULL;

        if (!plan) goto fail;
        if (controlled) {
            ASTNode *assign = repeat->child;
            ASTNode *target = assign->child;
            ASTNode *to = NULL;
            ASTNode *by = NULL;
            ASTNode *for_clause = NULL;
            ASTNode *control_anchor;
            ASTNode *initial_prelude;
            ASTNode *step_prelude;
            ASTNode *end_prelude;
            ASTNode *current_value;
            ASTNode *while_value;
            ASTNode *step_value;
            ASTNode *until_value;
            ASTNode *while_source;
            ASTNode *until_source;
            ASTNode *pool_receiver;
            ASTNode *set_args[2];
            ASTNode *set_statement;
            ASTNode *clause;
            ASTNode *captured_to_ref = NULL;
            ASTNode *captured_by_check_ref = NULL;
            ASTNode *captured_by_step_ref = NULL;
            ASTNode *captured_for_ref = NULL;
            ASTNode *captured_start_ref = NULL;
            ASTNode *start_value = NULL;
            ASTNode *start_copy = NULL;
            ASTNode *start_capture = NULL;
            ASTNode *checked_start = NULL;
            int literal_start;
            char *start_name = NULL;
            char *to_name = NULL;
            char *by_name = NULL;
            char *for_name = NULL;
            char *target_name;
            ASTNode *limit_args[2];
            ASTNode *step_args[1];
            int for_count = 0;
            int literal_for = 0;

            if (!levelc_controlled_header_supported(repeat, plan, &to, &by,
                                                       &for_clause, reason_out)) goto fail;
            literal_start = levelc_nonnegative_integer_literal(target->sibling);
            control_anchor = to ? to : (for_clause ? for_clause : (by ? by : assign));
            if (for_clause)
                literal_for = levelc_bounded_nonnegative_integer_literal(
                        for_clause->child, &for_count);
            header_setup = rxcp_remap_create_instruction_builder(context, assign);
            if (!header_setup) goto fail;
            if (!literal_start) {
                ASTNode *start = target->sibling;
                start_name = rxcp_remap_create_generated_node_name(
                        LEVELC_START_VALUE_PREFIX, start);
                start_value = levelc_lower_expr(context, start, plan, header_setup);
                start_copy = levelc_copy_rexxvalue(context, start, start_value);
                start_capture = start_name && start_copy
                    ? rxcp_remap_create_named_assignment(context, start,
                                                         start_name, start_copy)
                    : NULL;
                if (!start_capture) {
                    free(start_name);
                    goto fail;
                }
                add_ast(header_setup, start_capture);
                captured_start_ref = rxcp_remap_create_named_ref(
                        context, start, VAR_SYMBOL, start_name);
                free(start_name);
                if (!captured_start_ref) goto fail;
            }
            for (clause = assign->sibling; clause; clause = clause->sibling) {
                if (clause == to &&
                    !levelc_nonnegative_integer_literal(clause->child)) {
                    to_name = levelc_capture_control_clause(
                            context, clause, plan, header_setup,
                            LEVELC_TO_LIMIT_PREFIX, "controlToValue");
                    if (!to_name) {
                        free(by_name);
                        free(for_name);
                        goto fail;
                    }
                } else if (clause == by &&
                           !levelc_signed_integer_literal(clause->child)) {
                    by_name = levelc_capture_control_clause(
                            context, clause, plan, header_setup,
                            LEVELC_BY_STEP_PREFIX, "controlByValue");
                    if (!by_name) {
                        free(to_name);
                        free(for_name);
                        goto fail;
                    }
                } else if (clause == for_clause && !literal_for) {
                    for_name = levelc_capture_control_clause(
                            context, clause, plan, header_setup,
                            LEVELC_FOR_COUNT_PREFIX, "controlForCountValue");
                    if (!for_name) {
                        free(to_name);
                        free(by_name);
                        goto fail;
                    }
                }
            }
            if (to_name)
                captured_to_ref = rxcp_remap_create_named_ref(
                        context, to, VAR_SYMBOL, to_name);
            if (by_name) {
                if (to)
                    captured_by_check_ref = rxcp_remap_create_named_ref(
                            context, by, VAR_SYMBOL, by_name);
                captured_by_step_ref = rxcp_remap_create_named_ref(
                        context, by, VAR_SYMBOL, by_name);
            }
            if (for_name)
                captured_for_ref = rxcp_remap_create_named_ref(
                        context, for_clause, VAR_SYMBOL, for_name);
            free(to_name);
            free(by_name);
            free(for_name);
            if ((to && !levelc_nonnegative_integer_literal(to->child) &&
                 !captured_to_ref) ||
                (by && !levelc_signed_integer_literal(by->child) &&
                 (!captured_by_step_ref || (to && !captured_by_check_ref))) ||
                (for_clause && !literal_for && !captured_for_ref)) goto fail;
            if (!literal_start) {
                checked_start = rxcp_remap_create_member_call(
                        context, target->sibling, captured_start_ref,
                        "controlStartValue", NULL, 0);
                if (!checked_start) goto fail;
            }
            initial_prelude = rxcp_remap_create_instruction_builder(context, assign);
            initial_statement = initial_prelude
                ? levelc_pool_set_statement(context, assign, plan,
                                            initial_prelude, checked_start)
                : NULL;
            if (!initial_statement || initial_prelude->child) goto fail;
            target_name = levelc_upper_name(target);
            if (!target_name) goto fail;
            while_value = NULL;
            if (to) {
                current_value = levelc_scalar_pool_value_by_name(context, target, target_name);
                step_prelude = rxcp_remap_create_instruction_builder(context, by ? by : to);
                limit_args[0] = captured_to_ref ? captured_to_ref
                    : levelc_rexxvalue_from_literal(context, to->child);
                limit_args[1] = by && step_prelude
                    ? captured_by_check_ref ? captured_by_check_ref
                        : levelc_lower_expr(context, by->child, plan, step_prelude)
                    : NULL;
                if (!step_prelude || step_prelude->child || (by && !limit_args[1])) {
                    free(target_name);
                    goto fail;
                }
                while_value = current_value && limit_args[0]
                    ? rxcp_remap_create_member_call(context, to, current_value,
                                                    by ? "controlToContinueBy"
                                                       : "controlToContinue",
                                                    limit_args, by ? 2 : 1)
                    : NULL;
            }
            if (condition_node && condition_node->node_type == WHILE) {
                while_value = levelc_controlled_while_entry(context,
                                                             condition_node,
                                                             plan, while_value);
                if (!while_value) {
                    free(target_name);
                    goto fail;
                }
            }
            current_value = levelc_scalar_pool_value_by_name(context, target, target_name);
            end_prelude = rxcp_remap_create_instruction_builder(context,
                                                                by ? by : control_anchor);
            step_args[0] = by && end_prelude
                ? captured_by_step_ref ? captured_by_step_ref
                    : levelc_lower_expr(context, by->child, plan, end_prelude)
                : NULL;
            if (!end_prelude || end_prelude->child || (by && !step_args[0])) {
                free(target_name);
                goto fail;
            }
            step_value = current_value
                ? rxcp_remap_create_member_call(context, target, current_value,
                                                by ? "controlStepBy" : "controlStepByOne",
                                                by ? step_args : NULL, by ? 1 : 0)
                : NULL;
            pool_receiver = levelc_pool_ref(context, target, VAR_SYMBOL);
            set_args[0] = levelc_name_string(context, target);
            set_args[1] = step_value;
            set_statement = pool_receiver && set_args[0] && set_args[1]
                ? rxcp_remap_create_member_call_statement(context, target,
                                                          pool_receiver, "setValue",
                                                          set_args, 2)
                : NULL;
            if (end_prelude && set_statement) add_ast(end_prelude, set_statement);
            until_value = condition_node && condition_node->node_type == UNTIL
                ? levelc_controlled_until_end(context, condition_node, plan,
                                               end_prelude)
                : end_prelude && set_statement
                    ? rxcp_remap_create_prelude_block_expr(
                        context, control_anchor, end_prelude,
                        rxcp_remap_create_integer_constant(context, control_anchor,
                                                           0, TP_BOOLEAN))
                    : NULL;
            while_source = condition_node && condition_node->node_type == WHILE
                ? condition_node
                : to ? ast_f(context, WHILE, to->token) : NULL;
            until_source = condition_node && condition_node->node_type == UNTIL
                ? condition_node : ast_f(context, UNTIL, control_anchor->token);
            if (while_source && to &&
                (!condition_node || condition_node->node_type != WHILE))
                rxcp_remap_anchor_synthetic(while_source, to);
            if (until_source && (!condition_node || condition_node->node_type != UNTIL))
                rxcp_remap_anchor_synthetic(until_source, control_anchor);
            body = condition_node ? condition_node->sibling : repeat->sibling;
            control_name = rxcp_remap_create_generated_node_name(LEVELC_LOOP_PREFIX, stmt);
            count_node = for_clause
                ? literal_for
                    ? rxcp_remap_create_integer_constant(context, for_clause->child,
                                                         for_count, TP_INTEGER)
                    : captured_for_ref
                : NULL;
            lowered = (!(to || (condition_node && condition_node->node_type == WHILE)) ||
                       (while_value && while_source)) && until_value &&
                      until_source && control_name &&
                      (!for_clause || count_node)
                ? rxcp_remap_create_controlled_do(
                    context, stmt, lowered_body, control_name, count_node,
                    while_source, while_value, until_source, until_value)
                : NULL;
            free(target_name);
            if (!lowered) goto fail;
        } else if (repeat) {
            if (!levelc_repetition_supported(repeat, &count, &forever,
                                             reason_out)) goto fail;
            body = repeat->sibling;
        }
        if (condition_node && !controlled) {
            ASTNode *condition_prelude = rxcp_remap_create_instruction_builder(
                    context, condition_node);
            if (!condition_prelude) goto fail;
            condition_value = levelc_lower_expr(context, condition_node->child,
                                                plan, condition_prelude);
            condition_value = levelc_do_condition_logical_value(
                    context, condition_node, condition_value);
            if (!condition_value) goto fail;
            if (condition_prelude->child) {
                condition_value = rxcp_remap_create_prelude_block_expr(
                        context, condition_node, condition_prelude, condition_value);
                if (!condition_value) goto fail;
            }
            body = condition_node->sibling;
        }
        if (!controlled) {
            control_name = rxcp_remap_create_generated_node_name(LEVELC_LOOP_PREFIX, stmt);
            count_node = NULL;
            if (repeat && !forever) {
                if (count >= 0) {
                    count_node = rxcp_remap_create_integer_constant(
                            context, repeat->child->child, count, TP_INTEGER);
                } else {
                    ASTNode *count_source = repeat->child->child;
                    ASTNode *count_prelude = rxcp_remap_create_instruction_builder(
                            context, count_source);
                    ASTNode *count_value = count_prelude
                            ? levelc_lower_expr(context, count_source, plan, count_prelude)
                            : NULL;
                    count_node = count_value ? rxcp_remap_create_member_call(
                            context, count_source, count_value,
                            "repeatCountValue", NULL, 0) : NULL;
                    if (count_node && count_prelude->child) {
                        count_node = rxcp_remap_create_prelude_block_expr(
                                context, count_source, count_prelude, count_node);
                    }
                }
            }
            lowered = rxcp_remap_create_controlled_do(
                    context, stmt, lowered_body, control_name, count_node,
                    condition_node, condition_value, NULL, NULL);
            if (!control_name || (repeat && !forever && !count_node) || !lowered) goto fail;
        }
        binding.source_do = stmt;
        binding.control_name = control_name;
        binding.previous = previous;
        plan->active_loop = &binding;
        loop_active = 1;
    }
    if (!body) goto fail;
    body_statement = body->child;
    while (body_statement) {
        if (in_procedure) {
            if (!levelc_lower_proc_statement(context, lowered_body, body_statement,
                                            plan, procedure, reason_out)) goto fail;
        } else if (!levelc_lower_main_statement(context, lowered_body, body_statement,
                                                plan, reason_out)) goto fail;
        body_statement = body_statement->sibling;
    }

    if (!repeat && !condition_node) {
        lowered = rxcp_remap_create_do_block(context, stmt, lowered_body);
    }
    if (!lowered) goto fail;
    if (loop_active) plan->active_loop = previous;
    if (header_setup) rxcp_remap_append_builder_children(instructions, header_setup);
    if (initial_statement) add_ast(instructions, initial_statement);
    add_ast(instructions, lowered);
    free(control_name);
    return 1;

fail:
    if (loop_active) plan->active_loop = previous;
    free(control_name);
    if (reason_out && !*reason_out) *reason_out = "failed to lower supported Level C DO";
    return 0;
}

static int levelc_lower_select_body_statement(Context *context,
                                               ASTNode *instructions,
                                               ASTNode *statement,
                                               LevelCLowerPlan *plan,
                                               LevelCProcedureSlice *procedure,
                                               int in_procedure,
                                               const char **reason_out) {
    if (in_procedure) {
        return levelc_lower_proc_statement(context, instructions, statement,
                                           plan, procedure, reason_out);
    }
    return levelc_lower_main_statement(context, instructions, statement, plan, reason_out);
}

static int levelc_lower_select_statement(Context *context,
                                         ASTNode *instructions,
                                         ASTNode *stmt,
                                         LevelCLowerPlan *plan,
                                         LevelCProcedureSlice *procedure,
                                         int in_procedure,
                                         const char **reason_out) {
    ASTNode *clause;
    ASTNode *otherwise = NULL;
    ASTNode *fallback;
    ASTNode **whens;
    size_t count = 0;
    size_t index = 0;

    for (clause = stmt->child->child; clause; clause = clause->sibling) {
        if (clause->node_type == WHEN) count++;
        else otherwise = clause;
    }
    whens = calloc(count, sizeof(*whens));
    if (!whens) goto fail;
    for (clause = stmt->child->child; clause && clause->node_type == WHEN;
         clause = clause->sibling) whens[index++] = clause;

    fallback = rxcp_remap_create_instruction_builder(context, stmt);
    if (!fallback) goto fail_free;
    if (otherwise) {
        ASTNode *part = otherwise->child;
        ASTNode *list = part->node_type == INSTRUCTIONS ? part : part->sibling;
        if (part != list &&
            !levelc_lower_select_body_statement(context, fallback, part, plan,
                                                 procedure, in_procedure, reason_out)) goto fail_free;
        for (part = list->child; part; part = part->sibling) {
            if (!levelc_lower_select_body_statement(context, fallback, part, plan,
                                                    procedure, in_procedure, reason_out)) goto fail_free;
        }
    } else {
        char line[32];
        ASTNode *args[1];
        ASTNode *call;
        snprintf(line, sizeof(line), "%d", stmt->token ? stmt->token->line + 1 : 0);
        args[0] = rxcp_remap_create_string_constant(context, stmt, line);
        call = args[0] ? rxcp_remap_create_function_call(context, stmt,
                                                        "rexxvalue_select_missing", args, 1) : NULL;
        call = call ? rxcp_remap_create_call_statement(context, stmt, call) : NULL;
        if (!call) goto fail_free;
        add_ast(fallback, call);
    }

    while (index > 0) {
        ASTNode *when = whens[--index];
        ASTNode *condition_node = when->child;
        ASTNode *body_node = condition_node->sibling;
        ASTNode *prelude = rxcp_remap_create_instruction_builder(context, when);
        ASTNode *then_instructions = rxcp_remap_create_instruction_builder(context, body_node);
        ASTNode *next = rxcp_remap_create_instruction_builder(context, when);
        ASTNode *condition;
        ASTNode *then_block;
        ASTNode *else_block;
        ASTNode *lowered;

        if (!prelude || !then_instructions || !next) goto fail_free;
        condition = levelc_lower_expr(context, condition_node, plan, prelude);
        condition = condition ? rxcp_remap_create_member_call(context, condition_node,
                                                               condition, "logicalWhenValue", NULL, 0) : NULL;
        if (!condition) goto fail_free;
        if (!levelc_lower_select_body_statement(context, then_instructions, body_node,
                                                plan, procedure, in_procedure, reason_out)) goto fail_free;
        then_block = rxcp_remap_create_do_block(context, body_node, then_instructions);
        else_block = rxcp_remap_create_do_block(context, when, fallback);
        lowered = then_block && else_block
            ? rxcp_remap_create_if_statement(context, when, condition, then_block, else_block)
            : NULL;
        if (!lowered) goto fail_free;
        rxcp_remap_append_builder_children(next, prelude);
        add_ast(next, lowered);
        fallback = next;
    }

    rxcp_remap_append_builder_children(instructions, fallback);
    free(whens);
    return 1;

fail_free:
    free(whens);
fail:
    if (reason_out && !*reason_out) *reason_out = "failed to lower supported Level C SELECT";
    return 0;
}

static int levelc_tree_contains_parse_upper(ASTNode *node) {
    while (node) {
        if (node->node_type == PARSE && node->child &&
            node->child->node_type == OPTIONS) return 1;
        if (levelc_tree_contains_parse_upper(node->child)) return 1;
        node = node->sibling;
    }
    return 0;
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
    int needs_translate = 0;

    anchor = old_instructions && old_instructions->child ? old_instructions->child : program_file;
    needs_translate = levelc_tree_contains_parse_upper(old_instructions);
    for (i = 0; plan && i < plan->procedure_count; i++) {
        if (plan->procedures[i].arg_statement && plan->procedures[i].arg_count > 0)
            needs_translate = 1;
    }
    options = levelc_build_options(context, anchor, needs_translate);
    instructions = rxcp_remap_create_instruction_builder(context, anchor);
    if (!options || !instructions) {
        if (reason_out) *reason_out = "failed to create Level C lowered program shell";
        return 0;
    }

    pool_setup = levelc_pool_setup_statement(context, anchor);
    if (!pool_setup) {
        if (reason_out) *reason_out = "failed to create Level C pool setup";
        return 0;
    }
    add_ast(instructions, pool_setup);

    stmt = plan ? plan->main_first : old_instructions->child;
    while (stmt && (!plan || stmt != plan->main_end)) {
        if (stmt->node_type != REXX_OPTIONS) {
            if (!levelc_lower_main_statement(context,
                                             instructions,
                                             stmt,
                                             plan,
                                             reason_out)) {
                return 0;
            }
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
                if (!levelc_lower_proc_statement(context,
                                                instructions,
                                                stmt,
                                                plan,
                                                &plan->procedures[i],
                                                reason_out)) {
                    return 0;
                }
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

int rxcp_levelc_lower_to_canonical(Context *context, const char **reason_out) {
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
    if (result) result = rxcp_levelc_verify_lowered_tree(context->ast, reason_out);
    return result;
}
