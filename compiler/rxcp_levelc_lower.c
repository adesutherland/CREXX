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
 * expression operators, SAY, and local PROCEDURE EXPOSE over direct scalar or
 * stem names.
 * Everything else reports an unsupported-shape diagnostic until its lowering
 * and runtime contract are implemented.
 */

#include <ctype.h>
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
#define LEVELC_BIF_LENGTH_HELPER "rexxclassicbif_length"
#define LEVELC_BIF_DISPATCH_HELPER "rexxclassicbif_call"
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

typedef struct {
    ASTNode *instructions;
    ASTNode *main_first;
    ASTNode *main_end;
    LevelCProcedureSlice *procedures;
    size_t procedure_count;
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

static int levelc_is_short_circuit_operator(NodeType node_type) {
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
    if (node->node_type == INTEGER) return 1;
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
                                           const char **reason_out) {
    if (!stmt) return 1;
    if (stmt->node_type == REXX_OPTIONS) return 1;
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

    return rxcp_remap_create_member_call(context, source_node, receiver, "value", args, 1);
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

static ASTNode *levelc_rexxvalue_bool(Context *context,
                                      ASTNode *source_node,
                                      int value) {
    ASTNode *args[1];

    args[0] = rxcp_remap_create_integer_constant(context,
                                                 source_node,
                                                 value ? 1 : 0,
                                                 TP_INTEGER);
    if (!args[0]) return NULL;

    return rxcp_remap_create_factory_call(context,
                                          source_node,
                                          LEVELC_REXX_VALUE_CLASS,
                                          args,
                                          1);
}

static ASTNode *levelc_logical_value(Context *context,
                                     ASTNode *source_node,
                                     ASTNode *value) {
    if (!value) return NULL;
    return rxcp_remap_create_member_call(context,
                                         source_node,
                                         value,
                                         "logicalValue",
                                         NULL,
                                         0);
}

static ASTNode *levelc_logical_result(Context *context,
                                      ASTNode *source_node,
                                      ASTNode *value) {
    if (!value) return NULL;
    return rxcp_remap_create_member_call(context,
                                         source_node,
                                         value,
                                         "logicalResult",
                                         NULL,
                                         0);
}

static ASTNode *levelc_instruction_block(Context *context,
                                         ASTNode *source_node,
                                         ASTNode *prelude,
                                         ASTNode *statement) {
    ASTNode *instructions;

    if (!context || !source_node || !statement) return NULL;

    instructions = rxcp_remap_create_instruction_builder(context, source_node);
    if (!instructions) return NULL;

    if (prelude) rxcp_remap_append_builder_children(instructions, prelude);
    add_ast(instructions, statement);
    return rxcp_remap_create_do_block(context, source_node, instructions);
}

static ASTNode *levelc_short_circuit_result_assignment(Context *context,
                                                       ASTNode *source_node,
                                                       const char *result_name,
                                                       ASTNode *rhs) {
    if (!result_name || !rhs) return NULL;
    return rxcp_remap_create_named_assignment(context, source_node, result_name, rhs);
}

static ASTNode *levelc_lower_short_circuit(Context *context,
                                           ASTNode *expr,
                                           LevelCLowerPlan *plan,
                                           ASTNode *prelude) {
    char *result_name;
    ASTNode *left;
    ASTNode *right_prelude;
    ASTNode *right;
    ASTNode *condition;
    ASTNode *initial_assignment;
    ASTNode *then_assignment;
    ASTNode *else_assignment;
    ASTNode *then_block;
    ASTNode *else_block;
    ASTNode *if_statement;
    ASTNode *result_ref;

    if (!context || !expr || !prelude || !levelc_is_short_circuit_operator(expr->node_type)) {
        return NULL;
    }

    result_name = rxcp_remap_create_generated_node_name(LEVELC_EXPR_RESULT_PREFIX, expr);
    if (!result_name) return NULL;

    left = levelc_lower_expr(context, expr->child, plan, prelude);
    condition = levelc_logical_value(context, expr, left);
    if (!left || !condition) goto fail;

    initial_assignment = levelc_short_circuit_result_assignment(
            context,
            expr,
            result_name,
            levelc_rexxvalue_bool(context, expr, 0));
    if (!initial_assignment) goto fail;
    add_ast(prelude, initial_assignment);

    right_prelude = rxcp_remap_create_instruction_builder(context, expr->child->sibling);
    if (!right_prelude) goto fail;

    right = levelc_lower_expr(context, expr->child->sibling, plan, right_prelude);
    right = levelc_logical_result(context, expr->child->sibling, right);
    if (!right) goto fail;

    if (expr->node_type == OP_AND) {
        then_assignment = levelc_short_circuit_result_assignment(context,
                                                                 expr,
                                                                 result_name,
                                                                 right);
        else_assignment = levelc_short_circuit_result_assignment(
                context,
                expr,
                result_name,
                levelc_rexxvalue_bool(context, expr, 0));
    } else {
        then_assignment = levelc_short_circuit_result_assignment(
                context,
                expr,
                result_name,
                levelc_rexxvalue_bool(context, expr, 1));
        else_assignment = levelc_short_circuit_result_assignment(context,
                                                                 expr,
                                                                 result_name,
                                                                 right);
    }
    if (!then_assignment || !else_assignment) goto fail;

    if (expr->node_type == OP_AND) {
        then_block = levelc_instruction_block(context, expr, right_prelude, then_assignment);
        else_block = levelc_instruction_block(context, expr, NULL, else_assignment);
    } else {
        then_block = levelc_instruction_block(context, expr, NULL, then_assignment);
        else_block = levelc_instruction_block(context, expr, right_prelude, else_assignment);
    }
    if (!then_block || !else_block) goto fail;

    if_statement = rxcp_remap_create_if_statement(context,
                                                  expr,
                                                  condition,
                                                  then_block,
                                                  else_block);
    if (!if_statement) goto fail;
    add_ast(prelude, if_statement);

    result_ref = rxcp_remap_create_named_ref(context, expr, VAR_SYMBOL, result_name);
    free(result_name);
    return result_ref;

fail:
    free(result_name);
    return NULL;
}

static ASTNode *levelc_lower_bif_dispatch_call(Context *context,
                                               ASTNode *expr,
                                               const char *bif_name,
                                               LevelCLowerPlan *plan,
                                               ASTNode *prelude) {
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
    arg = expr->child;
    index = 1;
    while (index <= arg_count) {
        ASTNode *value_rhs;
        ASTNode *value_copy;
        int arg_provided;

        if (!arg) goto fail;
        arg_provided = levelc_argument_exists(arg);
        if (arg_provided) {
            value_rhs = levelc_lower_expr(context, arg, plan, prelude);
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
                                           LEVELC_BIF_DISPATCH_HELPER,
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
        return levelc_lower_bif_dispatch_call(context, expr, "SUBSTR", plan, prelude);
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
            return levelc_rexxvalue_from_literal(context, expr);
        case VAR_SYMBOL:
            return levelc_pool_value(context, expr);
        case FUNCTION:
            return levelc_lower_function_call(context, expr, plan, prelude);
        default:
            if (levelc_is_short_circuit_operator(expr->node_type)) {
                return levelc_lower_short_circuit(context, expr, plan, prelude);
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
                                          ASTNode *prelude) {
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
        args[2] = levelc_lower_expr(context, expr, plan, prelude);
        method_name = "setStemValue";
        arg_count = 3;
    } else {
        args[0] = levelc_name_string(context, target);
        args[1] = levelc_lower_expr(context, expr, plan, prelude);
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
    if (node->node_type == INTEGER) return levelc_rexxvalue_from_literal(context, node);
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

static ASTNode *levelc_build_options(Context *context, ASTNode *anchor_node) {
    ASTNode *options;
    ASTNode *levelb;
    ASTNode *comments_dash;
    ASTNode *numeric_classic;
    ASTNode *import_value;
    ASTNode *import_pool;
    ASTNode *import_bifs;

    options = ast_f(context, REXX_OPTIONS, anchor_node ? anchor_node->token : NULL);
    if (!options) return NULL;
    if (anchor_node) rxcp_remap_anchor_synthetic(options, anchor_node);

    levelb = rxcp_remap_create_literal(context, anchor_node ? anchor_node : options, "levelb");
    comments_dash = rxcp_remap_create_literal(context, anchor_node ? anchor_node : options, "comments_dash");
    numeric_classic = rxcp_remap_create_literal(context, anchor_node ? anchor_node : options, "numeric_classic");
    import_value = rxcp_remap_create_generated_import(context, anchor_node ? anchor_node : options, "rexxvalue");
    import_pool = rxcp_remap_create_generated_import(context, anchor_node ? anchor_node : options, "rexxpool");
    import_bifs = rxcp_remap_create_generated_import(context, anchor_node ? anchor_node : options, "rexxclassicbifs");
    if (!levelb || !comments_dash || !numeric_classic || !import_value || !import_pool || !import_bifs) return NULL;

    add_ast(options, levelb);
    add_ast(options, comments_dash);
    add_ast(options, numeric_classic);
    add_ast(options, import_value);
    add_ast(options, import_pool);
    add_ast(options, import_bifs);
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
        member_args[1] = arg_name ? rxcp_remap_create_named_ref(context,
                                                                target ? target : template_node,
                                                                VAR_SYMBOL,
                                                                arg_name) : NULL;
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

static int levelc_lower_main_statement(Context *context,
                                       ASTNode *instructions,
                                       ASTNode *stmt,
                                       LevelCLowerPlan *plan,
                                       const char **reason_out) {
    ASTNode *prelude;
    ASTNode *lowered;

    if (!stmt) return 1;

    prelude = rxcp_remap_create_instruction_builder(context, stmt);
    if (!prelude) {
        if (reason_out) *reason_out = "failed to create Level C statement prelude";
        return 0;
    }

    if (stmt->node_type == ASSIGN) {
        lowered = levelc_pool_set_statement(context, stmt, plan, prelude);
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
    if (stmt->node_type == LEVELC_ARG) {
        return levelc_append_arg_bindings(context, instructions, procedure, reason_out);
    }

    prelude = rxcp_remap_create_instruction_builder(context, stmt);
    if (!prelude) {
        if (reason_out) *reason_out = "failed to create Level C procedure statement prelude";
        return 0;
    }

    if (stmt->node_type == ASSIGN) {
        lowered = levelc_pool_set_statement(context, stmt, plan, prelude);
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
    return result;
}
