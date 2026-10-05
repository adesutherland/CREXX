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
 * bounded DO forms, and local PROCEDURE EXPOSE over ordered variable lists.
 * Everything else reports an unsupported-shape diagnostic until its lowering
 * and runtime contract are implemented.
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NUTF8
#include "utf.h"
#endif

#include "rxcp_levelc_lower.h"
#include "rxcpbgmr.h"
#include "rxcp_remap_build.h"
#include "rxcp_util.h"
#include "rxcpcsym.h"

#define LEVELC_POOL_SYMBOL "__rxcp_levelc_pool"
#define LEVELC_CONFIG_SYMBOL "__rxcp_levelc_config"
#define LEVELC_CONFIG_REF_SYMBOL "__rxcp_levelc_config_ref"
#define LEVELC_OPTIONS_CONFIG_PREFIX "__rxcp_levelc_options_config_"
#define LEVELC_PARENT_POOL_SYMBOL "__rxcp_levelc_parent_pool"
#define LEVELC_PARENT_POOL_REF_SYMBOL "__rxcp_levelc_parent_pool_ref"
#define LEVELC_BODY_NAME "__rxcp_levelc_body"
#define LEVELC_ENTRY_SYMBOL "__rxcp_levelc_entry"
#define LEVELC_PRIVATE_POOL_PREFIX "__rxcp_levelc_private_pool_"
#define LEVELC_ACTIVATION_SYMBOL "__rxcp_levelc_activation"
#define LEVELC_CALL_ACTIVATION_PREFIX "__rxcp_levelc_call_activation_"
#define LEVELC_MAIN_ARG_INDEX_PREFIX "__rxcp_levelc_main_arg_index_"
#define LEVELC_ACTIVATION_CLASS "RexxActivationArguments"
#define LEVELC_ACTIVATION_CLASS_TYPE ".RexxActivationArguments"
#define LEVELC_BIF_ARGS_PREFIX "__rxcp_levelc_bif_args_"
#define LEVELC_BIF_EXISTS_PREFIX "__rxcp_levelc_bif_exists_"
#define LEVELC_BIF_CONTEXT_PREFIX "__rxcp_levelc_bif_context_"
#define LEVELC_EXPR_RESULT_PREFIX "__rxcp_levelc_expr_"
#define LEVELC_SIGNAL_TARGET_PREFIX "__rxcp_levelc_signal_target_"
#define LEVELC_SIGNAL_DETAIL_PREFIX "__rxcp_levelc_signal_detail_"
#define LEVELC_SIGNAL_EVENT_SYMBOL "__rxcp_levelc_signal_event"
#define LEVELC_SIGNAL_KIND_SYMBOL "__rxcp_levelc_signal_kind"
#define LEVELC_SIGNAL_SELECTED_SYMBOL "__rxcp_levelc_signal_selected"
#define LEVELC_NOVALUE_EVENT_PREFIX "__rxcp_levelc_novalue_event_"
#define LEVELC_PARSE_FIELDS_PREFIX "__rxcp_levelc_parse_fields_"
#define LEVELC_PARSE_SOURCE_PREFIX "__rxcp_levelc_parse_source_"
#define LEVELC_LOOP_PREFIX "__rxcp_levelc_loop_"
#define LEVELC_DO_STATE_PREFIX "__rxcp_levelc_do_state_"
#define LEVELC_BIF_TRANSLATE_HELPER "rexxclassicbif_translate"
#define LEVELC_BIF_CONTEXT_CLASS "RexxBifCallContext"
#define LEVELC_REXX_VALUE_CLASS "RexxValue"
#define LEVELC_REXX_VALUE_CLASS_TYPE ".RexxValue"
#define LEVELC_INT_CLASS_TYPE ".int"

typedef struct {
    ASTNode *label;
    ASTNode *frame_label;
    ASTNode *body_first;
    ASTNode *body_end;
    char *name;
} LevelCProcedureSlice;

typedef struct LevelCLoopBinding {
    ASTNode *source_do;
    const char *control_name;
    struct LevelCLoopBinding *previous;
} LevelCLoopBinding;

typedef enum {
    LEVELC_DO_GROUP,
    LEVELC_DO_COUNTED,
    LEVELC_DO_FOREVER,
    LEVELC_DO_CONTROLLED,
    LEVELC_DO_CONDITIONAL
} LevelCDoKind;

typedef struct {
    ASTNode *source_do;
    ASTNode *repeat;
    ASTNode *condition;
    ASTNode *body;
    ASTNode *assign;
    ASTNode *target;
    ASTNode *start;
    ASTNode *to;
    ASTNode *by;
    ASTNode *for_clause;
    LevelCDoKind kind;
} LevelCDoHeader;

typedef struct {
    const char *name;
    const char *module;
    const char *entry;
} LevelCBifEntry;

typedef struct {
    ASTNode *source;
    ASTNode *trampoline;
    ASTNode *destination;
    char *condition;
    char *target_name;
    int condition_id;
} LevelCSignalHandler;

enum {
    LEVELC_SIGNAL_SYNTAX_ID = 1,
    LEVELC_SIGNAL_ERROR_ID = 2,
    LEVELC_SIGNAL_FAILURE_ID = 3,
    LEVELC_SIGNAL_HALT_ID = 4,
    LEVELC_SIGNAL_NOTREADY_ID = 5,
    LEVELC_SIGNAL_NOVALUE_ID = 6,
    LEVELC_SIGNAL_LOSTDIGITS_ID = 7
};

static int levelc_signal_condition_id(const char *name) {
    if (!name) return 0;
    if (strcmp(name, "SYNTAX") == 0) return LEVELC_SIGNAL_SYNTAX_ID;
    if (strcmp(name, "ERROR") == 0) return LEVELC_SIGNAL_ERROR_ID;
    if (strcmp(name, "FAILURE") == 0) return LEVELC_SIGNAL_FAILURE_ID;
    if (strcmp(name, "HALT") == 0) return LEVELC_SIGNAL_HALT_ID;
    if (strcmp(name, "NOTREADY") == 0) return LEVELC_SIGNAL_NOTREADY_ID;
    if (strcmp(name, "NOVALUE") == 0) return LEVELC_SIGNAL_NOVALUE_ID;
    if (strcmp(name, "LOSTDIGITS") == 0) return LEVELC_SIGNAL_LOSTDIGITS_ID;
    return 0;
}

#define LEVELC_DIRECT_BIF(name, module, entry) \
    {name, module, module "." entry}
static const LevelCBifEntry levelc_direct_bifs[] = {
    LEVELC_DIRECT_BIF("ABBREV", "rexxclassicbifabbrev", "rexxclassicbif_abbrev"),
    LEVELC_DIRECT_BIF("ABS", "rexxclassicbifabs", "rexxclassicbif_abs"),
    LEVELC_DIRECT_BIF("ADDRESS", "rexxclassicbifaddress", "rexxclassicbif_address"),
    LEVELC_DIRECT_BIF("ARG", "rexxclassicbifarg", "rexxclassicbif_arg"),
    LEVELC_DIRECT_BIF("B2X", "rexxclassicbifb2x", "rexxclassicbif_b2x"),
    {"BITAND", NULL, "rexxclassicbifs.rexxclassicbif_bitand"},
    {"BITOR", NULL, "rexxclassicbifs.rexxclassicbif_bitor"},
    {"BITXOR", NULL, "rexxclassicbifs.rexxclassicbif_bitxor"},
    LEVELC_DIRECT_BIF("C2D", "rexxclassicbifc2d", "rexxclassicbif_c2d"),
    LEVELC_DIRECT_BIF("C2X", "rexxclassicbifc2x", "rexxclassicbif_c2x"),
    LEVELC_DIRECT_BIF("CENTER", "rexxclassicbifcenter", "rexxclassicbif_center"),
    LEVELC_DIRECT_BIF("CENTRE", "rexxclassicbifcenter", "rexxclassicbif_centre"),
    LEVELC_DIRECT_BIF("CHANGESTR", "rexxclassicbifchangestr", "rexxclassicbif_changestr"),
    LEVELC_DIRECT_BIF("COMPARE", "rexxclassicbifcompare", "rexxclassicbif_compare"),
    LEVELC_DIRECT_BIF("CONDITION", "rexxclassicbifcondition", "rexxclassicbif_condition"),
    LEVELC_DIRECT_BIF("COPIES", "rexxclassicbifcopies", "rexxclassicbif_copies"),
    LEVELC_DIRECT_BIF("COUNTSTR", "rexxclassicbifcountstr", "rexxclassicbif_countstr"),
    LEVELC_DIRECT_BIF("D2C", "rexxclassicbifd2c", "rexxclassicbif_d2c"),
    LEVELC_DIRECT_BIF("D2X", "rexxclassicbifd2x", "rexxclassicbif_d2x"),
    LEVELC_DIRECT_BIF("DATATYPE", "rexxclassicbifdatatype", "rexxclassicbif_datatype"),
    LEVELC_DIRECT_BIF("DATE", "rexxclassicbifdate", "rexxclassicbif_date"),
    LEVELC_DIRECT_BIF("DELSTR", "rexxclassicbifdelstr", "rexxclassicbif_delstr"),
    LEVELC_DIRECT_BIF("DELWORD", "rexxclassicbifdelword", "rexxclassicbif_delword"),
    LEVELC_DIRECT_BIF("DIGITS", "rexxclassicbifnumeric", "rexxclassicbif_digits"),
    LEVELC_DIRECT_BIF("FORM", "rexxclassicbifnumeric", "rexxclassicbif_form"),
    LEVELC_DIRECT_BIF("FORMAT", "rexxclassicbifformat", "rexxclassicbif_format"),
    LEVELC_DIRECT_BIF("FUZZ", "rexxclassicbifnumeric", "rexxclassicbif_fuzz"),
    LEVELC_DIRECT_BIF("INSERT", "rexxclassicbifinsert", "rexxclassicbif_insert"),
    LEVELC_DIRECT_BIF("LASTPOS", "rexxclassicbiflastpos", "rexxclassicbif_lastpos"),
    LEVELC_DIRECT_BIF("LEFT", "rexxclassicbifleft", "rexxclassicbif_left"),
    LEVELC_DIRECT_BIF("LENGTH", "rexxclassicbiflength", "rexxclassicbif_length"),
    {"LOWER", NULL, "rexxclassicbifs.rexxclassicbif_lower"},
    LEVELC_DIRECT_BIF("MAX", "rexxclassicbifmax", "rexxclassicbif_max"),
    LEVELC_DIRECT_BIF("MIN", "rexxclassicbifmin", "rexxclassicbif_min"),
    LEVELC_DIRECT_BIF("OVERLAY", "rexxclassicbifoverlay", "rexxclassicbif_overlay"),
    LEVELC_DIRECT_BIF("POS", "rexxclassicbifpos", "rexxclassicbif_pos"),
    LEVELC_DIRECT_BIF("RANDOM", "rexxclassicbifrandom", "rexxclassicbif_random"),
    LEVELC_DIRECT_BIF("REVERSE", "rexxclassicbifreverse", "rexxclassicbif_reverse"),
    LEVELC_DIRECT_BIF("RIGHT", "rexxclassicbifright", "rexxclassicbif_right"),
    LEVELC_DIRECT_BIF("SIGN", "rexxclassicbifsign", "rexxclassicbif_sign"),
    LEVELC_DIRECT_BIF("SPACE", "rexxclassicbifspace", "rexxclassicbif_space"),
    LEVELC_DIRECT_BIF("STRIP", "rexxclassicbifstrip", "rexxclassicbif_strip"),
    LEVELC_DIRECT_BIF("SUBSTR", "rexxclassicbifsubstr", "rexxclassicbif_substr"),
    LEVELC_DIRECT_BIF("SUBWORD", "rexxclassicbifsubword", "rexxclassicbif_subword"),
    LEVELC_DIRECT_BIF("SYMBOL", "rexxclassicbifsymbol", "rexxclassicbif_symbol"),
    LEVELC_DIRECT_BIF("TIME", "rexxclassicbiftime", "rexxclassicbif_time"),
    LEVELC_DIRECT_BIF("TRACE", "rexxclassicbiftrace", "rexxclassicbif_trace"),
    LEVELC_DIRECT_BIF("TRANSLATE", "rexxclassicbiftranslate", "rexxclassicbif_translate"),
    LEVELC_DIRECT_BIF("TRUNC", "rexxclassicbiftrunc", "rexxclassicbif_trunc"),
    {"UPPER", NULL, "rexxclassicbifs.rexxclassicbif_upper"},
    LEVELC_DIRECT_BIF("VALUE", "rexxclassicbifvalue", "rexxclassicbif_value"),
    LEVELC_DIRECT_BIF("VERIFY", "rexxclassicbifverify", "rexxclassicbif_verify"),
    LEVELC_DIRECT_BIF("WORD", "rexxclassicbifword", "rexxclassicbif_word"),
    LEVELC_DIRECT_BIF("WORDINDEX", "rexxclassicbifwordindex", "rexxclassicbif_wordindex"),
    LEVELC_DIRECT_BIF("WORDLENGTH", "rexxclassicbifwordlength", "rexxclassicbif_wordlength"),
    LEVELC_DIRECT_BIF("WORDPOS", "rexxclassicbifwordpos", "rexxclassicbif_wordpos"),
    LEVELC_DIRECT_BIF("WORDS", "rexxclassicbifwords", "rexxclassicbif_words"),
    LEVELC_DIRECT_BIF("X2B", "rexxclassicbifx2b", "rexxclassicbif_x2b"),
    LEVELC_DIRECT_BIF("X2C", "rexxclassicbifx2c", "rexxclassicbif_x2c"),
    LEVELC_DIRECT_BIF("X2D", "rexxclassicbifx2d", "rexxclassicbif_x2d"),
    LEVELC_DIRECT_BIF("XRANGE", "rexxclassicbifxrange", "rexxclassicbif_xrange")
};
#undef LEVELC_DIRECT_BIF
#define LEVELC_DIRECT_BIF_COUNT (sizeof(levelc_direct_bifs) / sizeof(levelc_direct_bifs[0]))
_Static_assert(LEVELC_DIRECT_BIF_COUNT <= 64, "Level C BIF use mask is too small");

typedef struct {
    ASTNode *instructions;
    ASTNode *main_first;
    ASTNode *main_end;
    LevelCProcedureSlice *procedures;
    size_t procedure_count;
    LevelCDoHeader *do_headers;
    size_t do_header_count;
    LevelCSignalHandler *signal_handlers;
    size_t signal_handler_count;
    ASTNode *classic_condition_dispatch;
    ASTNode *classic_condition_source;
    int has_novalue_on;
    LevelCLoopBinding *active_loop;
    uint64_t used_direct_bifs;
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
        case LEVELC_SIGNAL_VALUE:
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
    for (i = 0; i < plan->signal_handler_count; i++) {
        free(plan->signal_handlers[i].condition);
        free(plan->signal_handlers[i].target_name);
    }
    if (plan->procedures) free(plan->procedures);
    free(plan->do_headers);
    free(plan->signal_handlers);
    memset(plan, 0, sizeof(*plan));
}

static int levelc_lower_plan_add_procedure(LevelCLowerPlan *plan,
                                           ASTNode *label,
                                           ASTNode *body_first,
                                           ASTNode *body_end,
                                           char *name) {
    LevelCProcedureSlice *procedures;

    if (!plan || !label || !name) return 0;

    procedures = realloc(plan->procedures,
                         sizeof(LevelCProcedureSlice) * (plan->procedure_count + 1));
    if (!procedures) return 0;

    plan->procedures = procedures;
    plan->procedures[plan->procedure_count].label = label;
    plan->procedures[plan->procedure_count].frame_label = NULL;
    plan->procedures[plan->procedure_count].body_first = body_first;
    plan->procedures[plan->procedure_count].body_end = body_end;
    plan->procedures[plan->procedure_count].name = name;
    plan->procedure_count++;
    return 1;
}

static char *levelc_upper_name(ASTNode *node);
static LevelCProcedureSlice *levelc_find_procedure(LevelCLowerPlan *plan,
                                                   const char *name);
static ASTNode *levelc_frame_node(Context *context, ASTNode *source,
                                  NodeType type, ASTNode *target);
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

static const LevelCBifEntry *levelc_find_direct_bif(const char *name,
                                                    size_t *index_out) {
    size_t index;
    if (!name) return NULL;
    for (index = 0; index < LEVELC_DIRECT_BIF_COUNT; index++) {
        if (strcmp(name, levelc_direct_bifs[index].name) == 0) {
            if (index_out) *index_out = index;
            return &levelc_direct_bifs[index];
        }
    }
    return NULL;
}

static int levelc_direct_bif_supported(ASTNode *expr,
                                      LevelCLowerPlan *plan,
                                      const char **reason_out) {
    ASTNode *arg;
    char *name;
    const LevelCBifEntry *bif;
    size_t bif_index;

    name = levelc_upper_name(expr);
    bif = levelc_find_direct_bif(name, &bif_index);
    if (name) free(name);
    if (!bif) {
        if (reason_out) *reason_out = "unsupported Level C function call";
        return 0;
    }
    arg = expr->child;
    while (arg) {
        if (levelc_argument_exists(arg) &&
            !levelc_expr_supported(arg, plan, reason_out)) {
            return 0;
        }
        arg = arg->sibling;
    }
    if (plan) plan->used_direct_bifs |= UINT64_C(1) << bif_index;
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
    arg = expr->child;
    if (arg && arg->node_type == NOVAL && !arg->sibling) arg = NULL;
    while (arg) {
        if (levelc_argument_exists(arg) &&
            !levelc_expr_supported(arg, plan, reason_out)) return 0;
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
        case BINARY:
        case INTEGER:
        case DECIMAL:
        case CONST_SYMBOL:
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
            {
                char *name = levelc_upper_name(expr);
                int is_local = name && plan && levelc_find_procedure(plan, name);
                if (name) free(name);
                if (is_local) return levelc_local_function_supported(expr, plan, reason_out);
                return levelc_direct_bif_supported(expr, plan, reason_out);
            }

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
        if (!target || target->node_type != VAR_TARGET || (expr && expr->sibling)) {
            if (reason_out) *reason_out = "unsupported assignment shape";
            return 0;
        }
        if (!levelc_assignment_target_supported(target, reason_out)) return 0;
        return !expr || levelc_expr_supported(expr, plan, reason_out);
    }

    if (stmt->node_type == SAY) {
        if (!stmt->child) return 1;
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

static Token *levelc_literal_last_token(ASTNode *node) {
    Token *first;
    Token *last;
    if (!node || !node->token) return NULL;
    first = node->token;
    last = first;
    if (first->token_string && first->length >= 3) {
        char suffix = (char)tolower((unsigned char)first->token_string[first->length - 1]);
        if (suffix == 'x' || suffix == 'b') return first;
    }
    if (first->token_subtype) {
        Token *cursor = first;
        while (cursor && cursor->token_number <= first->token_subtype) {
            if (cursor->token_type == TK_STRING ||
                cursor->token_type == TK_STRING_CONTINUATION) last = cursor;
            cursor = cursor->token_next;
        }
    }
    return last;
}

static char levelc_byte_literal_kind(ASTNode *node) {
    Token *last;
    char suffix;
    if (!node || (node->node_type != STRING && node->node_type != BINARY &&
                  node->node_type != PATTERN)) return 0;
    last = levelc_literal_last_token(node);
    if (!last || !last->token_string || last->length < 3) return 0;
    suffix = (char)tolower((unsigned char)last->token_string[last->length - 1]);
    return suffix == 'x' || suffix == 'b' ? suffix : 0;
}

/* Read source byte ordinals once, independent of their generic STRING/BINARY AST type. */
static int levelc_byte_literal_bytes(ASTNode *node,
                                     unsigned char **bytes_out,
                                     size_t *length_out) {
    Token *first = node ? node->token : NULL;
    Token *last = levelc_literal_last_token(node);
    Token *cursor;
    char kind = levelc_byte_literal_kind(node);
    size_t digits = 0;
    size_t count;
    size_t output_index = 0;
    unsigned int pending = 0;
    size_t used;
    size_t group = kind == 'x' ? 2 : 8;
    unsigned char *bytes;
    if (!first || !last || !kind || !bytes_out || !length_out) return 0;
    for (cursor = first; cursor; cursor = cursor->token_next) {
        size_t body_length;
        size_t index;
        if (cursor->token_type == TK_STRING ||
            cursor->token_type == TK_STRING_CONTINUATION) {
            if (cursor->length < (cursor == last ? 3 : 2)) return 0;
            body_length = (size_t)cursor->length - (cursor == last ? 3 : 2);
            for (index = 0; index < body_length; index++) {
                unsigned char ch = (unsigned char)cursor->token_string[index + 1];
                if (isspace(ch)) continue;
                if (kind == 'x' ? !isxdigit(ch) : ch != '0' && ch != '1') return 0;
                digits++;
            }
        }
        if (cursor == last) break;
    }
    if (cursor != last || digits > SIZE_MAX - (group - 1)) return 0;
    count = (digits + group - 1) / group;
    bytes = malloc(count ? count : 1);
    if (!bytes) return 0;
    used = (group - digits % group) % group;
    for (cursor = first; cursor; cursor = cursor->token_next) {
        size_t body_length;
        size_t index;
        if (cursor->token_type == TK_STRING ||
            cursor->token_type == TK_STRING_CONTINUATION) {
            body_length = (size_t)cursor->length - (cursor == last ? 3 : 2);
            for (index = 0; index < body_length; index++) {
                unsigned char ch = (unsigned char)cursor->token_string[index + 1];
                if (isspace(ch)) continue;
                pending = kind == 'x'
                    ? (pending << 4) | (unsigned int)hexchar2int((char)ch)
                    : (pending << 1) | (unsigned int)(ch - '0');
                if (++used == group) {
                    bytes[output_index++] = (unsigned char)pending;
                    pending = 0;
                    used = 0;
                }
            }
        }
        if (cursor == last) break;
    }
    *bytes_out = bytes;
    *length_out = output_index;
    if (output_index == count) return 1;
    free(bytes);
    *bytes_out = NULL;
    return 0;
}

static unsigned char *levelc_latin1_utf8(const unsigned char *ordinals,
                                         size_t count,
                                         size_t *length_out) {
    unsigned char *result;
    size_t length = 0;
    size_t index;
    if (!length_out || count > (SIZE_MAX - 1) / 2) return NULL;
    result = malloc(count * 2 + 1);
    if (!result) return NULL;
    for (index = 0; index < count; index++) {
        unsigned char ordinal = ordinals[index];
        if (ordinal < 128) result[length++] = ordinal;
        else {
            result[length++] = (unsigned char)(0xc0 | (ordinal >> 6));
            result[length++] = (unsigned char)(0x80 | (ordinal & 0x3f));
        }
    }
    result[length] = 0;
    *length_out = length;
    return result;
}

static char *levelc_upper_name(ASTNode *node) {
    char *name;
    char *upper;

    if (!node) return NULL;
    if (node->token) {
        name = rxcp_levelc_upper_symbol_from_token(node->token, 0);
        if (name) return name;
    }

    name = levelc_node_text_copy(node);
    if (!name) return NULL;
    upper = rxcp_levelc_upper_text(name, strlen(name));
    free(name);
    return upper;
}

static char *levelc_upper_label_name(ASTNode *node) {
    char *name;
    char *upper;
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
    length = strlen(name);
    upper = rxcp_levelc_upper_text(name, length);
    free(name);
    return upper;
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

static ASTNode *levelc_pool_ref(Context *context, ASTNode *source_node, NodeType node_type) {
    return rxcp_remap_create_named_ref(context, source_node, node_type, LEVELC_POOL_SYMBOL);
}

static ASTNode *levelc_config_ref(Context *context, ASTNode *source_node,
                                  NodeType node_type) {
    return rxcp_remap_create_named_ref(context, source_node, node_type,
                                       LEVELC_CONFIG_REF_SYMBOL);
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
    int supported;

    name = levelc_upper_name(node);
    if (!name) {
        if (reason_out) *reason_out = "failed to normalize variable name";
        return 0;
    }

    supported = levelc_variable_name_kind(name) != LEVELC_VAR_NAME_INVALID;
    if (!supported && reason_out) *reason_out = "unsupported variable name shape";

    free(name);
    return supported;
}

static int levelc_assignment_target_supported(ASTNode *node,
                                              const char **reason_out) {
    char *name;
    int supported;

    name = levelc_upper_name(node);
    if (!name) {
        if (reason_out) *reason_out = "failed to normalize assignment target";
        return 0;
    }

    supported = levelc_variable_name_kind(name) != LEVELC_VAR_NAME_INVALID;
    if (!supported && reason_out) *reason_out = "unsupported assignment target shape";

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
        if (arg->node_type != VAR_TARGET && arg->node_type != VAR_REFERENCE) {
            if (reason_out) *reason_out = "unsupported PROCEDURE EXPOSE target";
            return 0;
        }
        name = levelc_upper_name(arg);
        if (!name || levelc_variable_name_kind(name) == LEVELC_VAR_NAME_INVALID) {
            free(name);
            if (reason_out) *reason_out = "failed to normalize PROCEDURE EXPOSE target";
            return 0;
        }
        free(name);
        arg = arg->sibling;
    }

    return 1;
}

static int levelc_template_segment_supported(ASTNode *segment,
                                             const char **reason_out) {
    ASTNode *item;
    char *name;

    if (!segment || segment->node_type != TEMPLATES) goto unsupported;
    for (item = segment->child; item; item = item->sibling) {
        if (item->node_type == TARGET && !item->child) {
            name = levelc_upper_name(item);
            if (!name || (strcmp(name, ".") != 0 &&
                          levelc_variable_name_kind(name) == LEVELC_VAR_NAME_INVALID)) {
                free(name);
                goto unsupported;
            }
            free(name);
        } else if (item->node_type == PATTERN && !item->child && item->token) {
            continue;
        } else if (item->node_type == PATTERN && item->child &&
                   item->child->node_type == VAR_REFERENCE &&
                   !item->child->sibling) {
            name = levelc_upper_name(item->child);
            if (!name || levelc_variable_name_kind(name) == LEVELC_VAR_NAME_INVALID) {
                free(name);
                goto unsupported;
            }
            free(name);
        } else if (item->node_type == ABS_POS || item->node_type == REL_POS) {
            ASTNode *position = item->child ? item->child : item;
            if (item->child && item->child->sibling) goto unsupported;
            if (position->node_type != INTEGER && position->node_type != ABS_POS &&
                position->node_type != VAR_REFERENCE)
                goto unsupported;
            if (position->node_type == VAR_REFERENCE) {
                name = levelc_upper_name(position);
                if (!name || levelc_variable_name_kind(name) == LEVELC_VAR_NAME_INVALID) {
                    free(name);
                    goto unsupported;
                }
                free(name);
            }
        } else {
            goto unsupported;
        }
    }
    return 1;

unsupported:
    if (reason_out) *reason_out = "unsupported PARSE/ARG template item";
    return 0;
}

static int levelc_arg_statement_supported(ASTNode *stmt,
                                          const char **reason_out) {
    ASTNode *templates;
    ASTNode *template_node;
    if (!stmt || stmt->node_type != LEVELC_ARG) {
        if (reason_out) *reason_out = "missing ARG statement";
        return 0;
    }

    templates = stmt->child;
    if (!templates) return 1;
    if (templates->node_type != TEMPLATES || templates->sibling) {
        if (reason_out) *reason_out = "unsupported ARG template list";
        return 0;
    }

    template_node = templates->child;
    while (template_node) {
        if (!levelc_template_segment_supported(template_node, reason_out))
            return 0;
        template_node = template_node->sibling;
    }
    return 1;
}

static int levelc_call_statement_supported(ASTNode *stmt,
                                           LevelCLowerPlan *plan,
                                           const char **reason_out) {
    char *target_name;
    LevelCProcedureSlice *procedure;
    ASTNode *args;
    ASTNode *arg;

    target_name = levelc_call_target_name(stmt);
    procedure = target_name ? levelc_find_procedure(plan, target_name) : NULL;
    if (target_name) free(target_name);
    if (!procedure) {
        if (reason_out) *reason_out = "unsupported CALL target";
        return 0;
    }

    args = stmt->child ? stmt->child->sibling : NULL;
    if (!args) return 1;
    if (args->node_type != ARGS || args->sibling || !args->child) {
        if (reason_out) *reason_out = "unsupported CALL argument list";
        return 0;
    }
    for (arg = args->child; arg; arg = arg->sibling) {
        if (levelc_argument_exists(arg) &&
            !levelc_expr_supported(arg, plan, reason_out)) return 0;
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
        if (clause->node_type == LABEL && !saw_otherwise) {
            clause = clause->sibling;
            continue;
        }
        if (clause->node_type == WHEN && !saw_otherwise) {
            condition = clause->child;
            body = condition ? condition->sibling : NULL;
            if (!condition || !body || body->sibling) goto invalid;
            if (!levelc_expr_supported(condition, plan, reason_out)) return 0;
            if (in_procedure) {
                if (!levelc_proc_statement_supported(body, plan, reason_out)) return 0;
            } else if (!levelc_main_statement_supported(body, plan, reason_out)) return 0;
            saw_when = 1;
        } else if (clause->node_type == OTHERWISE && saw_when && !saw_otherwise &&
                   !clause->sibling) {
            ASTNode *body_list = clause->child;
            ASTNode *part;
            if (!body_list || body_list->node_type != INSTRUCTIONS || body_list->sibling) goto invalid;
            part = body_list->child;
            while (part) {
                if (in_procedure) {
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

static int levelc_drop_supported(ASTNode *stmt,
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
        LevelCVariableNameKind kind;
        int supported;

        if (!((target->node_type == VAR_TARGET && !target->child) ||
              (target->node_type == VAR_REFERENCE && target->child &&
               target->child->node_type == TOKEN && !target->child->sibling))) {
            if (reason_out) *reason_out = "unsupported DROP variable reference";
            return 0;
        }
        name = levelc_upper_name(target);
        kind = levelc_variable_name_kind(name);
        supported = kind != LEVELC_VAR_NAME_INVALID;
        free(name);
        if (!supported) {
            if (reason_out) *reason_out = "unsupported DROP name";
            return 0;
        }
        target = target->sibling;
    }
    return 1;
}

static int levelc_controlled_header_supported(LevelCDoHeader *header,
                                               LevelCLowerPlan *plan,
                                               const char **reason_out) {
    ASTNode *repeat = header ? header->repeat : NULL;
    ASTNode *assign = repeat ? repeat->child : NULL;
    ASTNode *target = assign ? assign->child : NULL;
    ASTNode *start = target ? target->sibling : NULL;
    ASTNode *clause = assign ? assign->sibling : NULL;
    char *name;
    LevelCVariableNameKind kind;

    if (!header) goto unsupported;
    if (!repeat || repeat->node_type != REPEAT || !assign ||
        assign->node_type != ASSIGN || !target ||
        target->node_type != VAR_TARGET || !start ||
        start->sibling) goto unsupported;
    header->assign = assign;
    header->target = target;
    header->start = start;
    if (!levelc_expr_supported(start, plan, reason_out)) goto unsupported;
    while (clause) {
        if (!clause->child || clause->child->sibling) goto unsupported;
        if (clause->node_type == TO && !header->to) {
            header->to = clause;
        } else if (clause->node_type == BY && !header->by) {
            header->by = clause;
        } else if (clause->node_type == FOR && !header->for_clause) {
            header->for_clause = clause;
        } else goto unsupported;
        if (!levelc_expr_supported(clause->child, plan, reason_out)) goto unsupported;
        clause = clause->sibling;
    }
    name = levelc_upper_name(target);
    kind = levelc_variable_name_kind(name);
    free(name);
    if (kind == LEVELC_VAR_NAME_SCALAR ||
        kind == LEVELC_VAR_NAME_COMPOUND) return 1;

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

static LevelCDoHeader *levelc_find_do_header(LevelCLowerPlan *plan,
                                              ASTNode *source_do) {
    size_t i;

    if (!plan || !source_do) return NULL;
    for (i = 0; i < plan->do_header_count; i++) {
        if (plan->do_headers[i].source_do == source_do) return &plan->do_headers[i];
    }
    return NULL;
}

static LevelCDoHeader *levelc_record_do_header(ASTNode *stmt,
                                                LevelCLowerPlan *plan,
                                                const char **reason_out) {
    LevelCDoHeader header = {0};
    LevelCDoHeader *grown;
    ASTNode *cursor;

    if (!stmt || !plan) return NULL;
    grown = levelc_find_do_header(plan, stmt);
    if (grown) return grown;

    header.source_do = stmt;
    cursor = stmt->child;
    if (cursor && cursor->node_type == REPEAT) {
        header.repeat = cursor;
        if (cursor->child && cursor->child->node_type == ASSIGN) {
            header.kind = LEVELC_DO_CONTROLLED;
            if (!levelc_controlled_header_supported(&header, plan, reason_out))
                return NULL;
        } else if (!cursor->child && nodeis(cursor, "forever")) {
            header.kind = LEVELC_DO_FOREVER;
        } else {
            ASTNode *for_node = cursor->child;
            ASTNode *expression = for_node ? for_node->child : NULL;
            if (!for_node || for_node->node_type != FOR || for_node->sibling ||
                !expression || expression->sibling ||
                !levelc_expr_supported(expression, plan, reason_out)) {
                if (reason_out && !*reason_out)
                    *reason_out = "unsupported DO repetition header";
                return NULL;
            }
            header.kind = LEVELC_DO_COUNTED;
        }
        cursor = cursor->sibling;
    }
    if (cursor && (cursor->node_type == WHILE || cursor->node_type == UNTIL)) {
        if (!levelc_do_condition_supported(cursor, plan, reason_out)) return NULL;
        header.condition = cursor;
        if (!header.repeat) header.kind = LEVELC_DO_CONDITIONAL;
        cursor = cursor->sibling;
    }
    if (!cursor || cursor->node_type != INSTRUCTIONS || cursor->sibling) {
        if (reason_out) *reason_out = "unsupported DO header";
        return NULL;
    }
    header.body = cursor;
    if (plan->do_header_count >= SIZE_MAX / sizeof(*grown)) return NULL;
    grown = realloc(plan->do_headers,
                    (plan->do_header_count + 1) * sizeof(*grown));
    if (!grown) return NULL;
    plan->do_headers = grown;
    plan->do_headers[plan->do_header_count] = header;
    return &plan->do_headers[plan->do_header_count++];
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
    LevelCDoHeader *header;

    if (!stmt) {
        if (reason_out) *reason_out = "unsupported LEAVE/ITERATE shape";
        return 0;
    }
    if (stmt->child) {
        if (stmt->child->node_type != VAR_SYMBOL || stmt->child->child ||
            stmt->child->sibling) {
            if (reason_out) *reason_out = "unsupported named LEAVE/ITERATE shape";
            return 0;
        }
        target = levelc_named_source_repetitive_do(stmt);
        if (!target && stmt->context && stmt->context->levelc_strict_classic)
            return 1;
        header = levelc_find_do_header(plan, target);
        if (header && header->kind == LEVELC_DO_CONTROLLED) return 1;
        if (reason_out && !*reason_out) *reason_out = "named LEAVE/ITERATE requires a supported controlled DO";
        return 0;
    }
    target = levelc_nearest_source_repetitive_do(stmt);
    if (!target && stmt->context && stmt->context->levelc_strict_classic)
        return 1;
    header = levelc_find_do_header(plan, target);
    if (header && header->kind != LEVELC_DO_GROUP) return 1;
    if (reason_out) *reason_out = "LEAVE/ITERATE requires a supported repetitive DO";
    return 0;
}

static int levelc_do_supported(ASTNode *stmt,
                              LevelCLowerPlan *plan,
                              int in_procedure,
                              const char **reason_out) {
    LevelCDoHeader *header = levelc_record_do_header(stmt, plan, reason_out);
    ASTNode *body_statement;

    if (!header) return 0;

    body_statement = header->body->child;
    while (body_statement) {
        if (in_procedure) {
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
    if (in_procedure) {
        if (!levelc_proc_statement_supported(then_statement, plan, reason_out)) return 0;
        return !else_statement || levelc_proc_statement_supported(else_statement, plan, reason_out);
    }
    if (!levelc_main_statement_supported(then_statement, plan, reason_out)) return 0;
    return !else_statement || levelc_main_statement_supported(else_statement, plan, reason_out);
}

static int levelc_parse_shape(ASTNode *stmt,
                              LevelCLowerPlan *plan,
                              ASTNode **source_out,
                              ASTNode **segment_out,
                              int *is_value_out,
                              int *upper_out,
                              const char **reason_out) {
    ASTNode *source;
    ASTNode *templates;
    ASTNode *template_node;
    char *name;
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
    if (!source || !templates || templates->sibling || !template_node ||
        template_node->sibling || !template_node->child) goto unsupported;

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

    if (!levelc_template_segment_supported(template_node, reason_out)) return 0;
    if (source_out) *source_out = source;
    if (segment_out) *segment_out = template_node;
    if (is_value_out) *is_value_out = is_value;
    if (upper_out) *upper_out = upper;
    return 1;

unsupported:
    if (reason_out) *reason_out = "unsupported PARSE source or template shape";
    return 0;
}

static int levelc_statement_supported(ASTNode *stmt,
                                      LevelCLowerPlan *plan,
                                      int in_procedure,
                                      const char **reason_out) {
    if (!stmt) return 1;
    if (stmt->node_type == REXX_OPTIONS)
        return !stmt->child || levelc_expr_supported(stmt->child, plan, reason_out);
    if (stmt->node_type == LEVELC_ARG) return levelc_arg_statement_supported(stmt, reason_out);
    if (stmt->node_type == LEVELC_SIGNAL && stmt->child && !stmt->child->sibling) {
        if (stmt->child->node_type == LITERAL || stmt->child->node_type == STRING)
            return 1;
        if (stmt->child->node_type == LEVELC_SIGNAL_VALUE &&
            stmt->child->child && !stmt->child->child->sibling)
            return levelc_expr_supported(stmt->child->child, plan, reason_out);
    }
    if (stmt->node_type == LEVELC_SIGNAL && stmt->child &&
        stmt->child->node_type == LITERAL) {
        ASTNode *mode = stmt->child;
        ASTNode *condition = mode->sibling;
        ASTNode *name_clause = condition ? condition->sibling : NULL;
        char *mode_name = levelc_upper_name(mode);
        char *condition_name = condition && condition->node_type == LITERAL
            ? levelc_upper_name(condition) : NULL;
        int supported = mode_name && condition_name &&
            levelc_signal_condition_id(condition_name) != 0 &&
            ((strcmp(mode_name, "OFF") == 0 && !name_clause) ||
             (strcmp(mode_name, "ON") == 0 &&
              (!name_clause ||
               (name_clause->node_type == LITERAL && !name_clause->sibling &&
                name_clause->child && !name_clause->child->sibling &&
                (name_clause->child->node_type == LITERAL ||
                 name_clause->child->node_type == STRING)))));
        free(mode_name);
        free(condition_name);
        if (supported) return 1;
    }
    if (in_procedure && stmt->node_type == LEVELC_PROCEDURE)
        return levelc_procedure_tail_supported(stmt, reason_out);
    if (stmt->node_type == NOP) return stmt->child == NULL;
    if (stmt->node_type == LEAVE || stmt->node_type == ITERATE)
        return levelc_transfer_supported(stmt, plan, reason_out);
    if (stmt->node_type == LEVELC_DROP) return levelc_drop_supported(stmt, reason_out);
    if (stmt->node_type == IF)
        return levelc_if_statement_supported(stmt, plan, in_procedure, reason_out);
    if (stmt->node_type == DO)
        return levelc_do_supported(stmt, plan, in_procedure, reason_out);
    if (stmt->node_type == SELECT)
        return levelc_select_statement_supported(stmt, plan, in_procedure, reason_out);
    if (stmt->node_type == PARSE)
        return levelc_parse_shape(stmt, plan, NULL, NULL, NULL, NULL, reason_out);
    if (levelc_pool_statement_supported(stmt, plan, reason_out)) return 1;

    if (stmt->node_type == CALL) {
        return levelc_call_statement_supported(stmt, plan, reason_out);
    }

    if (!in_procedure && stmt->node_type == EXIT) {
        if (stmt->child) {
            if (reason_out) *reason_out = "EXIT expression is outside slice";
            return 0;
        }
        return 1;
    }

    if (in_procedure && stmt->node_type == RETURN) {
        if (stmt->child) return levelc_expr_supported(stmt->child, plan, reason_out);
        return 1;
    }
    if (reason_out) *reason_out = in_procedure
        ? "unsupported procedure statement" : "unsupported main statement";
    return 0;
}

static int levelc_main_statement_supported(ASTNode *stmt,
                                           LevelCLowerPlan *plan,
                                           const char **reason_out) {
    return levelc_statement_supported(stmt, plan, 0, reason_out);
}

static int levelc_proc_statement_supported(ASTNode *stmt,
                                           LevelCLowerPlan *plan,
                                           const char **reason_out) {
    return levelc_statement_supported(stmt, plan, 1, reason_out);
}

static int levelc_collect_lower_plan(ASTNode *instructions,
                                     LevelCLowerPlan *plan,
                                     const char **reason_out) {
    ASTNode *stmt;
    ASTNode *label;
    ASTNode *body_first;
    ASTNode *body_end;
    ASTNode *body_stmt;
    char *name;
    size_t i;

    if (reason_out) *reason_out = NULL;
    if (!instructions || instructions->node_type != INSTRUCTIONS || !plan) {
        if (reason_out) *reason_out = "missing top-level instruction list";
        return 0;
    }

    memset(plan, 0, sizeof(*plan));
    plan->instructions = instructions;

    stmt = instructions->child;
    plan->main_first = stmt;
    while (stmt && stmt->node_type != LABEL) stmt = stmt->sibling;
    plan->main_end = stmt;

    while (stmt) {
        label = stmt;
        if (label->node_type != LABEL) {
            if (reason_out) *reason_out = "expected local routine label";
            return 0;
        }
        body_first = label->sibling;
        body_end = body_first;
        while (body_end && body_end->node_type != LABEL) body_end = body_end->sibling;

        name = levelc_upper_label_name(label);
        if (!name) {
            if (reason_out) *reason_out = "failed to normalize local routine label";
            return 0;
        }
        if (!levelc_lower_plan_add_procedure(plan,
                                             label,
                                             body_first,
                                             body_end,
                                             name)) {
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

    stmt = plan->main_first;
    while (stmt && stmt != plan->main_end) {
        if (!levelc_main_statement_supported(stmt, plan, reason_out)) return 0;
        stmt = stmt->sibling;
    }

    if (!plan->main_first && plan->procedure_count == 0) {
        if (reason_out) *reason_out = "no supported executable Level C statements";
        return 0;
    }

    return 1;
}

static ASTNode *levelc_lower_expr(Context *context,
                                  ASTNode *expr,
                                  LevelCLowerPlan *plan,
                                  ASTNode *prelude);

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

static ASTNode *levelc_rexxvalue_from_byte_literal(Context *context,
                                                   ASTNode *source_node) {
    unsigned char *ordinals = NULL;
    unsigned char *utf8 = NULL;
    char *escaped = NULL;
    size_t count = 0;
    size_t utf8_length = 0;
    size_t escaped_length = 0;
    size_t index;
    ASTNode *result = NULL;
    if (!levelc_byte_literal_bytes(source_node, &ordinals, &count)) goto done;
    utf8 = levelc_latin1_utf8(ordinals, count, &utf8_length);
    if (!utf8 || utf8_length > (SIZE_MAX - 1) / 4) goto done;
    escaped = malloc(utf8_length * 4 + 1);
    if (!escaped) goto done;
    for (index = 0; index < utf8_length; index++) {
        const char *part = escape_character(utf8[index]);
        size_t part_length = strlen(part);
        memcpy(escaped + escaped_length, part, part_length);
        escaped_length += part_length;
    }
    escaped[escaped_length] = '\0';
    result = levelc_rexxvalue_from_text(context, source_node, escaped);
done:
    free(escaped);
    free(utf8);
    free(ordinals);
    return result;
}

static ASTNode *levelc_rexxvalue_from_literal(Context *context, ASTNode *source_node) {
    char *text;
    char *cursor;
    ASTNode *result;

    if (levelc_byte_literal_kind(source_node))
        return levelc_rexxvalue_from_byte_literal(context, source_node);

    if (source_node && source_node->node_type == STRING &&
        source_node->node_string_length == 0)
        return levelc_rexxvalue_from_text(context, source_node, "");

    text = levelc_node_text_copy(source_node);
    if (!text) return NULL;
    if (source_node->node_type == DECIMAL || source_node->node_type == CONST_SYMBOL) {
        for (cursor = text; *cursor; cursor++) {
            if (source_node->node_type == CONST_SYMBOL)
                *cursor = (char)toupper((unsigned char)*cursor);
            else if (*cursor == 'e') *cursor = 'E';
        }
    }
    result = levelc_rexxvalue_from_text(context, source_node, text);
    free(text);
    return result;
}

static ASTNode *levelc_blank_rexxvalue(Context *context, ASTNode *source_node) {
    return levelc_rexxvalue_from_text(context, source_node, "");
}

static ASTNode *levelc_symbol_pool_value_by_name(Context *context,
                                                 ASTNode *source_node,
                                                 const char *name) {
    ASTNode *args[1];
    ASTNode *receiver;

    receiver = levelc_pool_ref(context, source_node, VAR_SYMBOL);
    args[0] = rxcp_remap_create_string_constant(context, source_node, name);
    if (!receiver || !args[0]) return NULL;

    return rxcp_remap_create_member_call(context, source_node, receiver, "symbolValue", args, 1);
}

static int levelc_append_novalue_guard(Context *context,
                                      ASTNode *prelude,
                                      ASTNode *source_node,
                                      const char *name) {
    ASTNode *activation = rxcp_remap_create_named_ref(
        context, source_node, VAR_SYMBOL, LEVELC_ACTIVATION_SYMBOL);
    ASTNode *policy_args[1] = {rxcp_remap_create_integer_constant(
        context, source_node, LEVELC_SIGNAL_NOVALUE_ID, TP_INTEGER)};
    ASTNode *policy = activation && policy_args[0]
        ? rxcp_remap_create_member_call(context, source_node, activation,
                                        "signalPolicy", policy_args, 1)
        : NULL;
    ASTNode *enabled = ast_f(context, OP_COMPARE_GT, source_node->token);
    ASTNode *pool_for_check = levelc_pool_ref(context, source_node, VAR_SYMBOL);
    ASTNode *check_args[1] = {rxcp_remap_create_string_constant(
        context, source_node, name)};
    ASTNode *has_value = pool_for_check && check_args[0]
        ? rxcp_remap_create_member_call(context, source_node, pool_for_check,
                                        "symbolHasValue", check_args, 1)
        : NULL;
    ASTNode *missing = ast_f(context, OP_COMPARE_EQUAL, source_node->token);
    ASTNode *pool_for_detail = levelc_pool_ref(context, source_node, VAR_SYMBOL);
    ASTNode *detail_args[1] = {rxcp_remap_create_string_constant(
        context, source_node, name)};
    ASTNode *detail = pool_for_detail && detail_args[0]
        ? rxcp_remap_create_member_call(context, source_node, pool_for_detail,
                                        "resolveSymbolName", detail_args, 1)
        : NULL;
    ASTNode *event_args[2] = {
        rxcp_remap_create_integer_constant(context, source_node,
                                           LEVELC_SIGNAL_NOVALUE_ID, TP_INTEGER),
        detail
    };
    ASTNode *event = event_args[0] && event_args[1]
        ? rxcp_remap_create_factory_call(context, source_node,
                                         "RexxClassicConditionEvent",
                                         event_args, 2)
        : NULL;
    char *event_name = rxcp_remap_create_generated_node_name(
        LEVELC_NOVALUE_EVENT_PREFIX, source_node);
    ASTNode *event_assignment = event && event_name
        ? rxcp_remap_create_named_assignment(context, source_node,
                                             event_name, event)
        : NULL;
    ASTNode *signal = ast_ftt(context, ASSEMBLER, strdup("signal"));
    ASTNode *missing_instructions = rxcp_remap_create_instruction_builder(
        context, source_node);
    ASTNode *missing_block;
    ASTNode *missing_if;
    ASTNode *enabled_instructions = rxcp_remap_create_instruction_builder(
        context, source_node);
    ASTNode *enabled_block;
    ASTNode *enabled_if;

    if (!prelude || !policy || !enabled || !has_value || !missing ||
        !event_assignment || !signal || !missing_instructions ||
        !enabled_instructions) {
        free(event_name);
        return 0;
    }
    rxcp_remap_anchor_synthetic(enabled, source_node);
    add_ast(enabled, policy);
    add_ast(enabled, rxcp_remap_create_integer_constant(
        context, source_node, 0, TP_INTEGER));
    rxcp_remap_anchor_synthetic(missing, source_node);
    add_ast(missing, has_value);
    add_ast(missing, rxcp_remap_create_integer_constant(
        context, source_node, 0, TP_BOOLEAN));
    signal->free_node_string = 1;
    signal->is_compiler_added = 1;
    rxcp_remap_anchor_synthetic(signal, source_node);
    add_ast(signal, rxcp_remap_create_string_constant(
        context, source_node, "CLASSIC_CONDITION"));
    add_ast(signal, rxcp_remap_create_named_ref(
        context, source_node, VAR_SYMBOL, event_name));
    add_ast(missing_instructions, event_assignment);
    add_ast(missing_instructions, signal);
    missing_block = rxcp_remap_create_do_block(
        context, source_node, missing_instructions);
    missing_if = missing_block ? rxcp_remap_create_if_statement(
        context, source_node, missing, missing_block, NULL) : NULL;
    if (!missing_if) {
        free(event_name);
        return 0;
    }
    add_ast(enabled_instructions, missing_if);
    enabled_block = rxcp_remap_create_do_block(
        context, source_node, enabled_instructions);
    enabled_if = enabled_block ? rxcp_remap_create_if_statement(
        context, source_node, enabled, enabled_block, NULL) : NULL;
    free(event_name);
    if (!enabled_if) return 0;
    add_ast(prelude, enabled_if);
    return 1;
}

static ASTNode *levelc_pool_value(Context *context,
                                  ASTNode *source_node,
                                  LevelCLowerPlan *plan,
                                  ASTNode *prelude) {
    char *name;
    ASTNode *value;

    name = levelc_upper_name(source_node);
    if (!name) return NULL;

    if (plan && plan->has_novalue_on &&
        !levelc_append_novalue_guard(context, prelude, source_node, name)) {
        free(name);
        return NULL;
    }
    value = levelc_symbol_pool_value_by_name(context, source_node, name);

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

static char *levelc_begin_call_activation(Context *context,
                                          ASTNode *call_node,
                                          ASTNode *prelude,
                                          int is_function) {
    char *name;
    ASTNode *factory;
    ASTNode *assignment;
    ASTNode *receiver;
    ASTNode *parent;
    ASTNode *inherit;
    ASTNode *inherit_args[1];
    ASTNode *begin;

    if (!context || !call_node || !prelude) return NULL;
    name = rxcp_remap_create_generated_node_name(
        LEVELC_CALL_ACTIVATION_PREFIX, call_node);
    factory = name ? rxcp_remap_create_factory_call(
        context, call_node, LEVELC_ACTIVATION_CLASS, NULL, 0) : NULL;
    assignment = factory ? rxcp_remap_create_named_assignment(
        context, call_node, name, factory) : NULL;
    receiver = assignment ? rxcp_remap_create_named_ref(
        context, call_node, VAR_SYMBOL, name) : NULL;
    parent = rxcp_remap_create_named_ref(
        context, call_node, VAR_SYMBOL, LEVELC_ACTIVATION_SYMBOL);
    inherit_args[0] = parent ? rxcp_remap_create_reference_expr(
        context, call_node, parent) : NULL;
    inherit = receiver && inherit_args[0] ? rxcp_remap_create_member_call_statement(
        context, call_node, receiver, "inheritSignalPolicy", inherit_args, 1)
        : NULL;
    receiver = assignment ? rxcp_remap_create_named_ref(
        context, call_node, VAR_SYMBOL, name) : NULL;
    begin = receiver ? rxcp_remap_create_member_call_statement(
        context, call_node, receiver,
        is_function ? "beginInternalFunction" : "beginInternalCall",
        NULL, 0) : NULL;
    if (!assignment || !inherit || !begin) {
        free(name);
        return NULL;
    }
    add_ast(prelude, assignment);
    add_ast(prelude, inherit);
    add_ast(prelude, begin);
    return name;
}

static int levelc_append_activation_method(Context *context,
                                           ASTNode *instructions,
                                           ASTNode *anchor,
                                           const char *method) {
    ASTNode *receiver = rxcp_remap_create_named_ref(
        context, anchor, VAR_SYMBOL, LEVELC_ACTIVATION_SYMBOL);
    ASTNode *call = receiver ? rxcp_remap_create_member_call_statement(
        context, anchor, receiver, method, NULL, 0) : NULL;
    if (!call) return 0;
    add_ast(instructions, call);
    return 1;
}

static int levelc_append_classic_error_if_expr(Context *context,
                                               ASTNode *instructions,
                                               ASTNode *anchor,
                                               ASTNode *condition,
                                               ASTNode *signal_detail) {
    ASTNode *signal = ast_ftt(context, ASSEMBLER, strdup("signal"));
    ASTNode *signal_name = rxcp_remap_create_string_constant(
        context, anchor, "CLASSIC_SYNTAX");
    ASTNode *then_instructions = rxcp_remap_create_instruction_builder(
        context, anchor);
    ASTNode *then_block;
    ASTNode *branch;

    if (!instructions || !anchor || !condition || !signal || !signal_name ||
        !signal_detail || !then_instructions) return 0;
    signal->free_node_string = 1;
    signal->is_compiler_added = 1;
    rxcp_remap_anchor_synthetic(signal, anchor);
    add_ast(signal, signal_name);
    add_ast(signal, signal_detail);
    add_ast(then_instructions, signal);
    then_block = rxcp_remap_create_do_block(context, anchor, then_instructions);
    branch = then_block ? rxcp_remap_create_if_statement(
        context, anchor, condition, then_block, NULL) : NULL;
    if (!branch) return 0;
    add_ast(instructions, branch);
    return 1;
}

static int levelc_append_classic_error_if(Context *context,
                                          ASTNode *instructions,
                                          ASTNode *anchor,
                                          ASTNode *condition,
                                          const char *detail) {
    ASTNode *signal_detail = rxcp_remap_create_string_constant(
        context, anchor, detail);
    return signal_detail && levelc_append_classic_error_if_expr(
        context, instructions, anchor, condition, signal_detail);
}

static int levelc_append_procedure_entry(Context *context,
                                         ASTNode *instructions,
                                         ASTNode *stmt) {
    ASTNode *receiver = rxcp_remap_create_named_ref(
        context, stmt, VAR_SYMBOL, LEVELC_ACTIVATION_SYMBOL);
    ASTNode *eligible = receiver ? rxcp_remap_create_member_call(
        context, stmt, receiver, "procedureEligible", NULL, 0) : NULL;
    ASTNode *condition = ast_f(context, OP_COMPARE_EQUAL, stmt->token);
    ASTNode *zero = rxcp_remap_create_integer_constant(
        context, stmt, 0, TP_BOOLEAN);
    if (!eligible || !condition || !zero) return 0;
    rxcp_remap_anchor_synthetic(condition, stmt);
    add_ast(condition, eligible);
    add_ast(condition, zero);
    if (!levelc_append_classic_error_if(context, instructions, stmt, condition,
            "RXC-LC-17.1: PROCEDURE is valid only as the first instruction of an internal call"))
        return 0;
    return levelc_append_activation_method(context, instructions, stmt,
                                           "enterProcedure");
}

static int levelc_append_call_argument(Context *context,
                                       ASTNode *source_node,
                                       ASTNode *prelude,
                                       const char *frame_name,
                                       ASTNode *value,
                                       int exists) {
    ASTNode *receiver;
    ASTNode *args[2];
    ASTNode *statement;

    if (!context || !source_node || !prelude || !frame_name) return 0;
    receiver = rxcp_remap_create_named_ref(context, source_node,
                                           VAR_SYMBOL, frame_name);
    args[0] = exists ? levelc_copy_rexxvalue(context, source_node, value)
                     : levelc_blank_rexxvalue(context, source_node);
    args[1] = rxcp_remap_create_integer_constant(context, source_node,
                                                   exists, TP_INTEGER);
    statement = receiver && args[0] && args[1]
        ? rxcp_remap_create_member_call_statement(context, source_node,
                                                  receiver, "append", args, 2)
        : NULL;
    if (!statement) return 0;
    add_ast(prelude, statement);
    return 1;
}

static int levelc_append_call_actuals(Context *context,
                                      ASTNode *actual,
                                      LevelCLowerPlan *plan,
                                      ASTNode *prelude,
                                      const char *frame_name) {
    while (actual) {
        int exists = levelc_argument_exists(actual);
        ASTNode *value = exists
            ? levelc_lower_expr(context, actual, plan, prelude) : NULL;
        if ((exists && !value) ||
            !levelc_append_call_argument(context, actual, prelude, frame_name,
                                         value, exists)) return 0;
        actual = actual->sibling;
    }
    return 1;
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
    char *result_name;
    ASTNode *statement;
    ASTNode *receiver;
    ASTNode *member_args[2];
    ASTNode *call_args[1];
    ASTNode *function_args[2];
    ASTNode *direct_call;
    ASTNode *condition;
    ASTNode *detail;
    ASTNode *result;
    ASTNode *arg;
    size_t arg_count;
    size_t index;
    int activation_bif;

    if (!context || !expr || !bif_name || !prelude) return NULL;
    activation_bif = strcmp(bif_name, "ARG") == 0 ||
                     strcmp(bif_name, "CONDITION") == 0;

    args_name = rxcp_remap_create_generated_node_name(LEVELC_BIF_ARGS_PREFIX, expr);
    exists_name = rxcp_remap_create_generated_node_name(LEVELC_BIF_EXISTS_PREFIX, expr);
    context_name = rxcp_remap_create_generated_node_name(LEVELC_BIF_CONTEXT_PREFIX, expr);
    result_name = rxcp_remap_create_generated_node_name(LEVELC_EXPR_RESULT_PREFIX, expr);
    if (!args_name || !exists_name || !context_name || !result_name) {
        if (args_name) free(args_name);
        if (exists_name) free(exists_name);
        if (context_name) free(context_name);
        if (result_name) free(result_name);
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

    receiver = rxcp_remap_create_named_ref(context, expr, VAR_SYMBOL, context_name);
    member_args[0] = levelc_config_ref(context, expr, VAR_SYMBOL);
    statement = rxcp_remap_create_member_call_statement(context,
                                                        expr,
                                                        receiver,
                                                        "setConfig",
                                                        member_args,
                                                        1);
    if (!statement) goto fail;
    add_ast(prelude, statement);

    function_args[0] = rxcp_remap_create_reference_expr(
            context,
            expr,
            rxcp_remap_create_named_ref(context, expr, VAR_SYMBOL, context_name));
    if (!function_args[0]) goto fail;

    if (activation_bif) {
        function_args[1] = rxcp_remap_create_reference_expr(
            context, expr,
            rxcp_remap_create_named_ref(context, expr, VAR_SYMBOL,
                                         LEVELC_ACTIVATION_SYMBOL));
        if (!function_args[1]) goto fail;
    }
    direct_call = rxcp_remap_create_function_call(context,
                                                  expr,
                                                  callee_name,
                                                  function_args,
                                                  activation_bif ? 2 : 1);
    statement = direct_call ? rxcp_remap_create_named_assignment(
        context, expr, result_name, direct_call) : NULL;
    if (!statement) goto fail;
    add_ast(prelude, statement);

    receiver = rxcp_remap_create_named_ref(context, expr, VAR_SYMBOL, context_name);
    condition = receiver ? rxcp_remap_create_member_call(
        context, expr, receiver, "hasError", NULL, 0) : NULL;
    receiver = rxcp_remap_create_named_ref(context, expr, VAR_SYMBOL, context_name);
    detail = receiver ? rxcp_remap_create_member_call(
        context, expr, receiver, "errorDetail", NULL, 0) : NULL;
    if (!condition || !detail ||
        !levelc_append_classic_error_if_expr(context, prelude, expr,
                                             condition, detail)) goto fail;

    result = rxcp_remap_create_named_ref(context, expr, VAR_SYMBOL, result_name);
    if (!result) goto fail;

    free(args_name);
    free(exists_name);
    free(context_name);
    free(result_name);
    return result;

fail:
    free(args_name);
    free(exists_name);
    free(context_name);
    free(result_name);
    return NULL;
}

static int levelc_append_function_result_guard(Context *context,
                                               ASTNode *prelude,
                                               ASTNode *call_node,
                                               const char *frame_name,
                                               const char *target_name) {
    static const char prefix[] = "RXC-LC-44.1: No data returned from function ";
    ASTNode *receiver = rxcp_remap_create_named_ref(
        context, call_node, VAR_SYMBOL, frame_name);
    ASTNode *present = receiver ? rxcp_remap_create_member_call(
        context, call_node, receiver, "hasReturnValue", NULL, 0) : NULL;
    ASTNode *condition = ast_f(context, OP_COMPARE_EQUAL, call_node->token);
    ASTNode *zero = rxcp_remap_create_integer_constant(
        context, call_node, 0, TP_BOOLEAN);
    size_t length = strlen(prefix) + strlen(target_name) + 1;
    char *detail = malloc(length);
    int result;

    if (!present || !condition || !zero || !detail) {
        free(detail);
        return 0;
    }
    snprintf(detail, length, "%s%s", prefix, target_name);
    rxcp_remap_anchor_synthetic(condition, call_node);
    add_ast(condition, present);
    add_ast(condition, zero);
    result = levelc_append_classic_error_if(
        context, prelude, call_node, condition, detail);
    free(detail);
    return result;
}

static ASTNode *levelc_lower_local_function_call(Context *context,
                                                 ASTNode *expr,
                                                 LevelCLowerPlan *plan,
                                                 ASTNode *prelude) {
    char *target_name;
    LevelCProcedureSlice *procedure;
    ASTNode *args[4];
    ASTNode *pool_symbol;
    ASTNode *arg;
    ASTNode *call;
    ASTNode *call_stmt;
    ASTNode *result_receiver;
    char *frame_name;
    size_t entry;

    if (!context || !expr || !plan) return NULL;

    target_name = levelc_upper_name(expr);
    procedure = target_name ? levelc_find_procedure(plan, target_name) : NULL;
    if (!procedure) {
        if (target_name) free(target_name);
        return NULL;
    }

    entry = (size_t)(procedure - plan->procedures) + 1;
    if (entry > INT_MAX) {
        free(target_name);
        return NULL;
    }

    frame_name = levelc_begin_call_activation(context, expr, prelude, 1);
    if (!frame_name) {
        free(target_name);
        return NULL;
    }

    pool_symbol = levelc_pool_ref(context, expr, VAR_SYMBOL);
    args[0] = rxcp_remap_create_reference_expr(context, expr, pool_symbol);
    args[1] = levelc_config_ref(context, expr, VAR_SYMBOL);
    if (!pool_symbol || !args[0] || !args[1]) {
        free(frame_name);
        free(target_name);
        return NULL;
    }

    arg = expr->child;
    if (arg && arg->node_type == NOVAL && !arg->sibling) arg = NULL;
    if (!levelc_append_call_actuals(context, arg, plan, prelude, frame_name)) {
        free(frame_name);
        free(target_name);
        return NULL;
    }

    args[2] = rxcp_remap_create_named_ref(context, expr,
                                          VAR_SYMBOL, frame_name);
    args[3] = rxcp_remap_create_integer_constant(context, expr, (int)entry,
                                                 TP_INTEGER);
    if (!args[2] || !args[3]) {
        free(frame_name);
        free(target_name);
        return NULL;
    }

    call = rxcp_remap_create_function_call(context,
                                           expr,
                                           LEVELC_BODY_NAME,
                                           args,
                                           4);
    call_stmt = call ? rxcp_remap_create_call_statement(context, expr, call) : NULL;
    if (!call_stmt) {
        free(frame_name);
        free(target_name);
        return NULL;
    }
    add_ast(prelude, call_stmt);
    if (!levelc_append_function_result_guard(
            context, prelude, expr, frame_name, target_name)) {
        free(frame_name);
        free(target_name);
        return NULL;
    }
    result_receiver = rxcp_remap_create_named_ref(context, expr,
                                                  VAR_SYMBOL, frame_name);
    free(frame_name);
    free(target_name);
    return result_receiver ? rxcp_remap_create_member_call(
        context, expr, result_receiver, "returnValue", NULL, 0) : NULL;
}

static ASTNode *levelc_lower_function_call(Context *context,
                                           ASTNode *expr,
                                           LevelCLowerPlan *plan,
                                           ASTNode *prelude) {
    char *name;
    const LevelCBifEntry *bif;
    LevelCProcedureSlice *procedure;

    if (!context || !expr || expr->node_type != FUNCTION) return NULL;

    name = levelc_upper_name(expr);
    if (!name) return NULL;

    procedure = plan ? levelc_find_procedure(plan, name) : NULL;
    bif = procedure ? NULL : levelc_find_direct_bif(name, NULL);
    if (bif) {
        ASTNode *call = levelc_lower_bif_dispatch_call(context, expr, name, plan,
                                                       prelude, bif->entry, NULL);
        free(name);
        return call;
    }
    free(name);
    return levelc_lower_local_function_call(context, expr, plan, prelude);
}

static ASTNode *levelc_lower_expr(Context *context,
                                  ASTNode *expr,
                                  LevelCLowerPlan *plan,
                                  ASTNode *prelude) {
    const char *method;

    if (!context || !expr) return NULL;

    switch (expr->node_type) {
        case STRING:
        case BINARY:
        case INTEGER:
        case DECIMAL:
        case CONST_SYMBOL:
            return levelc_rexxvalue_from_literal(context, expr);
        case VAR_SYMBOL:
            return levelc_pool_value(context, expr, plan, prelude);
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
    ASTNode *args[2];

    target = assign_node->child;
    expr = target ? target->sibling : NULL;
    if (!target) return NULL;

    receiver = levelc_pool_ref(context, assign_node, VAR_SYMBOL);
    args[0] = levelc_name_string(context, target);
    args[1] = value_override ? value_override
                             : expr ? levelc_lower_expr(context, expr, plan, prelude)
                                    : levelc_blank_rexxvalue(context, assign_node);
    if (!receiver || !args[0] || !args[1]) return NULL;

    return rxcp_remap_create_member_call_statement(context,
                                                   assign_node,
                                                   receiver,
                                                   "setSymbolValue",
                                                   args,
                                                   2);
}

static ASTNode *levelc_say_statement(Context *context,
                                     ASTNode *say_node,
                                     LevelCLowerPlan *plan,
                                     ASTNode *prelude) {
    ASTNode *lowered_expr;
    ASTNode *as_string;
    ASTNode *say;

    if (say_node->child) {
        lowered_expr = levelc_lower_expr(context, say_node->child, plan, prelude);
        if (!lowered_expr) return NULL;
        as_string = rxcp_remap_create_member_call(context,
                                                 say_node,
                                                 lowered_expr,
                                                 "asString",
                                                 NULL,
                                                 0);
    } else {
        as_string = rxcp_remap_create_string_constant(context, say_node, "");
    }
    if (!as_string) return NULL;

    say = ast_f(context, SAY, say_node->token);
    if (!say) return NULL;
    rxcp_remap_anchor_synthetic(say, say_node);
    add_ast(say, as_string);
    return say;
}

static ASTNode *levelc_options_statement(Context *context,
                                          ASTNode *options_node,
                                          LevelCLowerPlan *plan,
                                          ASTNode *prelude) {
    ASTNode *config_ref;
    ASTNode *config_assignment;
    ASTNode *config;
    ASTNode *args[1];
    char *config_name;

    config_name = rxcp_remap_create_generated_node_name(
        LEVELC_OPTIONS_CONFIG_PREFIX, options_node);
    config_ref = levelc_config_ref(context, options_node, VAR_SYMBOL);
    config_assignment = config_name && config_ref
        ? rxcp_remap_create_named_assignment(
              context, options_node, config_name,
              rxcp_remap_create_dereference_expr(context, options_node,
                                                config_ref))
        : NULL;
    config = config_name ? rxcp_remap_create_named_ref(
                               context, options_node, VAR_SYMBOL, config_name)
                         : NULL;
    args[0] = options_node->child
        ? levelc_lower_expr(context, options_node->child, plan, prelude)
        : levelc_blank_rexxvalue(context, options_node);
    if (!config_assignment || !config || !args[0]) {
        free(config_name);
        return NULL;
    }
    add_ast(prelude, config_assignment);
    free(config_name);
    return rxcp_remap_create_member_call_statement(context,
                                                    options_node,
                                                    config,
                                                    "applyOptions",
                                                    args,
                                                    1);
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

static ASTNode *levelc_config_setup_statement(Context *context,
                                               ASTNode *anchor_node,
                                               int make_reference) {
    ASTNode *value;
    ASTNode *args[1];
    if (make_reference) {
        value = rxcp_remap_create_reference_expr(
                context, anchor_node,
                rxcp_remap_create_named_ref(context, anchor_node, VAR_SYMBOL,
                                            LEVELC_CONFIG_SYMBOL));
        return value ? rxcp_remap_create_named_assignment(
                context, anchor_node, LEVELC_CONFIG_REF_SYMBOL, value) : NULL;
    }
    args[0] = rxcp_remap_create_string_constant(context, anchor_node, "UTF8");
    value = args[0] ? rxcp_remap_create_factory_call(context, anchor_node,
                                                     "RexxClassicConfig", args, 1)
                    : NULL;
    return value ? rxcp_remap_create_named_assignment(
            context, anchor_node, LEVELC_CONFIG_SYMBOL, value) : NULL;
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

static ASTNode *levelc_call_local_procedure_statement(Context *context,
                                                      ASTNode *call_node,
                                                      LevelCLowerPlan *plan,
                                                      ASTNode *prelude) {
    char *target_name;
    char *frame_name;
    LevelCProcedureSlice *procedure;
    ASTNode *args[4];
    ASTNode *actual;
    ASTNode *pool_symbol;
    ASTNode *call_expr;
    ASTNode *statement;
    size_t entry;

    target_name = levelc_call_target_name(call_node);
    procedure = target_name ? levelc_find_procedure(plan, target_name) : NULL;
    if (!target_name || !procedure) {
        if (target_name) free(target_name);
        return NULL;
    }

    free(target_name);
    entry = (size_t)(procedure - plan->procedures) + 1;
    if (entry > INT_MAX) return NULL;

    frame_name = levelc_begin_call_activation(context, call_node, prelude, 0);
    if (!frame_name) return NULL;

    pool_symbol = levelc_pool_ref(context, call_node, VAR_SYMBOL);
    args[0] = rxcp_remap_create_reference_expr(context, call_node, pool_symbol);
    args[1] = levelc_config_ref(context, call_node, VAR_SYMBOL);
    if (!pool_symbol || !args[0] || !args[1]) {
        free(frame_name);
        return NULL;
    }

    actual = call_node->child ? call_node->child->sibling : NULL;
    actual = actual ? actual->child : NULL;
    if (!levelc_append_call_actuals(context, actual, plan, prelude, frame_name)) {
        free(frame_name);
        return NULL;
    }

    args[2] = rxcp_remap_create_named_ref(context, call_node,
                                          VAR_SYMBOL, frame_name);
    args[3] = rxcp_remap_create_integer_constant(context, call_node, (int)entry,
                                                 TP_INTEGER);
    if (!args[2] || !args[3]) {
        free(frame_name);
        return NULL;
    }

    call_expr = rxcp_remap_create_function_call(context,
                                                call_node,
                                                LEVELC_BODY_NAME,
                                                args,
                                                4);
    free(frame_name);
    if (!call_expr) return NULL;

    statement = rxcp_remap_create_call_statement(context, call_node, call_expr);
    return statement;
}

static ASTNode *levelc_expose_value_statement(Context *context,
                                              ASTNode *expose_target) {
    ASTNode *receiver;
    ASTNode *args[3];
    ASTNode *parent_symbol;
    const char *method_name;
    int arg_count;

    receiver = levelc_pool_ref(context, expose_target, VAR_SYMBOL);
    args[0] = levelc_name_string(context, expose_target);
    parent_symbol = levelc_parent_pool_ref(context, expose_target, VAR_SYMBOL);
    args[1] = rxcp_remap_create_reference_expr(context, expose_target, parent_symbol);
    if (expose_target->node_type == VAR_REFERENCE) {
        method_name = "exposeIndirect";
        args[2] = levelc_config_ref(context, expose_target, VAR_SYMBOL);
        arg_count = 3;
    } else {
        method_name = "exposeSymbol";
        arg_count = 2;
    }
    if (!receiver || !args[0] || !parent_symbol || !args[1] ||
        (arg_count == 3 && !args[2])) return NULL;

    return rxcp_remap_create_member_call_statement(context,
                                                   expose_target,
                                                   receiver,
                                                   method_name,
                                                   args,
                                                   arg_count);
}

static ASTNode *levelc_body_header(Context *context, ASTNode *anchor) {
    ASTNode *return_type;

    if (!context || !anchor) return NULL;
    return_type = rxcp_remap_create_void_type(context, anchor);
    return return_type ? rxcp_remap_create_procedure_header(
        context, anchor, LEVELC_BODY_NAME ":", return_type) : NULL;
}

static ASTNode *levelc_body_args(Context *context, ASTNode *anchor) {
    ASTNode *args;
    ASTNode *arg;
    ASTNode *target;
    ASTNode *type_ref;
    ASTNode *class_node;

    if (!context || !anchor) return NULL;

    args = rxcp_remap_create_args_builder(context, anchor);
    target = levelc_parent_pool_ref_symbol(context, anchor, VAR_TARGET);
    type_ref = rxcp_remap_create_reference_type(context,
                                                anchor,
                                                ".RexxVariablePool");
    arg = rxcp_remap_create_arg(context, anchor, target, type_ref);
    if (!args || !arg) return NULL;

    add_ast(args, arg);

    target = levelc_config_ref(context, anchor, VAR_TARGET);
    type_ref = rxcp_remap_create_reference_type(context,
                                                anchor,
                                                ".RexxClassicConfig");
    arg = rxcp_remap_create_arg(context, anchor, target, type_ref);
    if (!arg) return NULL;
    add_ast(args, arg);

    target = rxcp_remap_create_named_ref(context, anchor,
                                         VAR_TARGET, LEVELC_ACTIVATION_SYMBOL);
    class_node = rxcp_remap_create_class_type(context,
                                               anchor,
                                               LEVELC_ACTIVATION_CLASS_TYPE);
    arg = target && class_node
        ? rxcp_remap_create_arg(context, anchor, target, class_node)
        : NULL;
    if (!arg) return NULL;
    add_ast(args, arg);

    target = rxcp_remap_create_named_ref(context, anchor, VAR_TARGET,
                                         LEVELC_ENTRY_SYMBOL);
    type_ref = rxcp_remap_create_class_type(context, anchor, LEVELC_INT_CLASS_TYPE);
    arg = target && type_ref ? rxcp_remap_create_arg(context, anchor, target, type_ref) : NULL;
    if (!arg) return NULL;
    add_ast(args, arg);

    return args;
}

static int levelc_append_procedure_exposes(Context *context,
                                           ASTNode *instructions,
                                           ASTNode *procedure_node,
                                           const char **reason_out) {
    ASTNode *child;
    ASTNode *args;
    ASTNode *expose_target;
    ASTNode *statement;

    if (!procedure_node) return 1;
    child = procedure_node->child;
    if (!child) return 1;
    args = child->sibling;
    if (!args || args->node_type != ARGS) return 1;

    expose_target = args->child;
    while (expose_target) {
        statement = levelc_expose_value_statement(context,
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
                                     int needs_translate,
                                     int needs_signal_policy,
                                     const LevelCLowerPlan *plan) {
    ASTNode *options;
    ASTNode *levelb;
    ASTNode *comments_dash;
    ASTNode *numeric_classic;
    ASTNode *import_value;
    ASTNode *import_pool;
    ASTNode *import_activation;
    ASTNode *import_do;
    ASTNode *import_config;
    ASTNode *import_bifs;
    ASTNode *import_translate;
    ASTNode *import_signal;
    ASTNode *import_condition_event;
    size_t i;
    int needs_do_state = context->levelc_strict_classic;

    options = ast_f(context, REXX_OPTIONS, anchor_node ? anchor_node->token : NULL);
    if (!options) return NULL;
    if (anchor_node) rxcp_remap_anchor_synthetic(options, anchor_node);

    levelb = rxcp_remap_create_literal(context, anchor_node ? anchor_node : options, "levelb");
    comments_dash = rxcp_remap_create_literal(context, anchor_node ? anchor_node : options, "comments_dash");
    numeric_classic = rxcp_remap_create_literal(context, anchor_node ? anchor_node : options, "numeric_classic");
    import_value = rxcp_remap_create_generated_import(context, anchor_node ? anchor_node : options, "rexxvalue");
    import_pool = rxcp_remap_create_generated_import(context, anchor_node ? anchor_node : options, "rexxpool");
    import_activation = rxcp_remap_create_generated_import(context,
        anchor_node ? anchor_node : options, "rexxactivation");
    for (i = 0; plan && i < plan->do_header_count; i++) {
        if (plan->do_headers[i].kind == LEVELC_DO_COUNTED ||
            plan->do_headers[i].kind == LEVELC_DO_CONTROLLED) {
            needs_do_state = 1;
            break;
        }
    }
    import_do = needs_do_state
        ? rxcp_remap_create_generated_import(context,
                                             anchor_node ? anchor_node : options,
                                             "rexxdostate")
        : NULL;
    import_config = rxcp_remap_create_generated_import(context, anchor_node ? anchor_node : options, "rexxclassicconfig");
    import_bifs = rxcp_remap_create_generated_import(context, anchor_node ? anchor_node : options, "rexxclassicbifs");
    import_translate = needs_translate
        ? rxcp_remap_create_generated_import(context,
                                             anchor_node ? anchor_node : options,
                                             "rexxclassicbiftranslate")
        : NULL;
    import_signal = needs_signal_policy
        ? rxcp_remap_create_generated_import(context,
                                             anchor_node ? anchor_node : options,
                                             "rxfnsb")
        : NULL;
    import_condition_event = plan && plan->has_novalue_on
        ? rxcp_remap_create_generated_import(context,
                                             anchor_node ? anchor_node : options,
                                             "rexxclassicconditionevent")
        : NULL;
    if (!levelb || !comments_dash || !numeric_classic || !import_value ||
        !import_pool || !import_activation || (needs_do_state && !import_do) ||
        !import_config || !import_bifs ||
        (needs_translate && !import_translate) ||
        (needs_signal_policy && !import_signal) ||
        (plan && plan->has_novalue_on && !import_condition_event)) return NULL;

    add_ast(options, levelb);
    add_ast(options, comments_dash);
    add_ast(options, numeric_classic);
    add_ast(options, import_value);
    add_ast(options, import_pool);
    add_ast(options, import_activation);
    if (import_do) add_ast(options, import_do);
    add_ast(options, import_config);
    add_ast(options, import_bifs);
    if (import_translate) add_ast(options, import_translate);
    if (import_signal) add_ast(options, import_signal);
    if (import_condition_event) add_ast(options, import_condition_event);
    for (i = 0; plan && i < LEVELC_DIRECT_BIF_COUNT; i++) {
        const char *module;
        ASTNode *import;
        size_t previous;
        if (!(plan->used_direct_bifs & (UINT64_C(1) << i))) continue;
        module = levelc_direct_bifs[i].module;
        if (!module || (needs_translate &&
                        strcmp(module, "rexxclassicbiftranslate") == 0)) continue;
        for (previous = 0; previous < i; previous++) {
            if ((plan->used_direct_bifs & (UINT64_C(1) << previous)) &&
                levelc_direct_bifs[previous].module &&
                strcmp(levelc_direct_bifs[previous].module, module) == 0) break;
        }
        if (previous < i) continue;
        import = rxcp_remap_create_generated_import(
                context, anchor_node ? anchor_node : options, module);
        if (!import) return NULL;
        add_ast(options, import);
    }
    return options;
}

static ASTNode *levelc_proc_return_statement(Context *context,
                                             ASTNode *stmt,
                                             LevelCLowerPlan *plan,
                                             LevelCProcedureSlice *procedure,
                                             ASTNode *prelude) {
    ASTNode *return_stmt;
    ASTNode *return_value;
    ASTNode *receiver;
    ASTNode *function_receiver;
    ASTNode *function_call;
    ASTNode *function_condition;
    ASTNode *one;
    ASTNode *args[1];
    ASTNode *record;

    (void)procedure;

    return_stmt = rxcp_remap_create_return_statement(context, stmt);
    if (!return_stmt) return NULL;

    receiver = rxcp_remap_create_named_ref(context, stmt, VAR_SYMBOL,
                                            LEVELC_ACTIVATION_SYMBOL);
    if (!receiver) return NULL;
    if (stmt->child) {
        return_value = levelc_lower_expr(context, stmt->child, plan, prelude);
        if (!return_value) return NULL;
        args[0] = return_value;
        record = rxcp_remap_create_member_call_statement(context, stmt, receiver,
                                                         "setReturnValue", args, 1);
    } else {
        function_receiver = rxcp_remap_create_named_ref(
            context, stmt, VAR_SYMBOL, LEVELC_ACTIVATION_SYMBOL);
        function_call = function_receiver ? rxcp_remap_create_member_call(
            context, stmt, function_receiver, "isFunctionCall", NULL, 0) : NULL;
        function_condition = ast_f(context, OP_COMPARE_EQUAL, stmt->token);
        one = rxcp_remap_create_integer_constant(context, stmt, 1, TP_BOOLEAN);
        if (!function_call || !function_condition || !one) return NULL;
        rxcp_remap_anchor_synthetic(function_condition, stmt);
        add_ast(function_condition, function_call);
        add_ast(function_condition, one);
        if (!levelc_append_classic_error_if(
                context, prelude, stmt, function_condition,
                "RXC-LC-45.1: Data expected on RETURN instruction because this routine was called as a function"))
            return NULL;
        record = rxcp_remap_create_member_call_statement(context, stmt, receiver,
                                                         "clearReturnValue", NULL, 0);
    }
    if (!record) return NULL;
    add_ast(prelude, record);

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
static int levelc_append_private_pool_transition(Context *context,
                                                  ASTNode *instructions,
                                                  ASTNode *procedure_node);

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
        if (context->levelc_strict_classic && !source_target) {
            const int has_loop = levelc_nearest_source_repetitive_do(stmt) != NULL;
            const char *code = stmt->node_type == LEAVE
                ? (has_loop ? "28.3" : "28.1")
                : (has_loop ? "28.4" : "28.2");
            char *name = stmt->child ? levelc_upper_name(stmt->child) : NULL;
            ASTNode *args[2];
            ASTNode *call;

            args[0] = rxcp_remap_create_string_constant(context, stmt, code);
            args[1] = rxcp_remap_create_string_constant(context, stmt,
                                                        name ? name : "");
            free(name);
            call = args[0] && args[1]
                ? rxcp_remap_create_function_call(context, stmt,
                                                   "rexxdostate_invalid_transfer", args, 2)
                : NULL;
            lowered = call ? rxcp_remap_create_call_statement(context, stmt, call) : NULL;
            if (!lowered) {
                if (reason_out) *reason_out = "failed to lower Classic loop transfer error";
                return 0;
            }
            add_ast(instructions, lowered);
            return 1;
        }
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

static int levelc_lower_drop(Context *context,
                             ASTNode *instructions,
                             ASTNode *stmt,
                             LevelCLowerPlan *plan,
                             const char **reason_out) {
    ASTNode *target = stmt->child->child;

    while (target) {
        ASTNode *receiver = levelc_pool_ref(context, target, VAR_SYMBOL);
        ASTNode *args[2] = {NULL, NULL};
        ASTNode *lowered;
        const char *method;
        int arg_count = 1;

        if (target->node_type == VAR_REFERENCE) {
            ASTNode *value = levelc_pool_value(context, target, plan,
                                               instructions);
            args[0] = value
                ? rxcp_remap_create_member_call(context, target, value,
                                                "asString", NULL, 0)
                : NULL;
            args[1] = levelc_config_ref(context, target, VAR_SYMBOL);
            arg_count = 2;
            method = "dropIndirectList";
        } else {
            args[0] = levelc_name_string(context, target);
            method = "dropSymbol";
        }

        if (!receiver || !args[0] || (arg_count == 2 && !args[1])) goto fail;
        lowered = rxcp_remap_create_member_call_statement(context, target,
                                                            receiver, method,
                                                            args, arg_count);
        if (!lowered) goto fail;
        add_ast(instructions, lowered);
        target = target->sibling;
    }
    return 1;

fail:
    if (reason_out) *reason_out = "failed to lower Level C DROP";
    return 0;
}

typedef struct {
    unsigned char *bytes;
    size_t length;
    size_t capacity;
    unsigned int item_count;
    unsigned int result_count;
    ASTNode **external_operands;
    unsigned int external_count;
    unsigned int external_capacity;
} LevelCParsePlanBytes;

/* parseplan item kinds, shared with the VM and Level B exit.
 * The descriptor is emitted as escaped bytes in an AST STRING operand because
 * the canonical assembler signature for parseplan requires a string constant. */
enum {
    LEVELC_PARSEPLAN_TARGET = 1,
    LEVELC_PARSEPLAN_LITERAL = 2,
    LEVELC_PARSEPLAN_ABSOLUTE = 3,
    LEVELC_PARSEPLAN_RELATIVE_PLUS = 4,
    LEVELC_PARSEPLAN_RELATIVE_MINUS = 5,
    LEVELC_PARSEPLAN_IMPLICIT_WORD = 6,
    LEVELC_PARSEPLAN_DYNAMIC_LITERAL = 7,
    LEVELC_PARSEPLAN_DYNAMIC_ABSOLUTE = 8,
    LEVELC_PARSEPLAN_DYNAMIC_PLUS = 9,
    LEVELC_PARSEPLAN_DYNAMIC_MINUS = 10
};

static int levelc_parseplan_number(LevelCParsePlanBytes *plan,
                                   uint64_t value, unsigned int width);
static int levelc_parseplan_item(LevelCParsePlanBytes *plan,
                                 unsigned int kind, unsigned int flags);

/* A dynamic name reads a completed result if a preceding control closed that
 * target; otherwise it reads the visible pool before parseplan starts. */
static int levelc_parseplan_completed_target(ASTNode *first,
                                             ASTNode *current,
                                             unsigned int completed_count,
                                             const char *name) {
    ASTNode *cursor;
    unsigned int result_index = 0;
    int matched = 0;
    for (cursor = first; cursor && cursor != current; cursor = cursor->sibling) {
        char *target_name;
        if (cursor->node_type != TARGET) continue;
        target_name = levelc_upper_name(cursor);
        if (!target_name) return -1;
        if (strcmp(target_name, ".") != 0) {
            result_index++;
            if (result_index <= completed_count && strcmp(target_name, name) == 0)
                matched = (int)result_index;
        }
        free(target_name);
    }
    return matched;
}

static int levelc_parseplan_dynamic_item(LevelCParsePlanBytes *plan,
                                         ASTNode *first,
                                         ASTNode *current,
                                         ASTNode *operand,
                                         unsigned int completed_count,
                                         unsigned int kind) {
    ASTNode **grown;
    char *name = levelc_upper_name(operand);
    int captured;
    unsigned int index;
    if (!name) return 0;
    captured = levelc_parseplan_completed_target(first, current, completed_count, name);
    free(name);
    if (captured < 0) return 0;
    if (captured) {
        index = (unsigned int)captured - 1;
    } else {
        if (plan->external_count == UINT16_MAX) return 0;
        if (plan->external_count == plan->external_capacity) {
            unsigned int capacity = plan->external_capacity ? plan->external_capacity * 2 : 8;
            if (capacity <= plan->external_capacity) return 0;
            grown = realloc(plan->external_operands, capacity * sizeof(*grown));
            if (!grown) return 0;
            plan->external_operands = grown;
            plan->external_capacity = capacity;
        }
        index = plan->external_count;
        plan->external_operands[plan->external_count++] = operand;
    }
    return levelc_parseplan_item(plan, kind, captured ? 1 : 0) &&
           levelc_parseplan_number(plan, index, 2);
}

static int levelc_parseplan_byte(LevelCParsePlanBytes *plan, unsigned int value) {
    unsigned char *grown;
    size_t capacity;
    if (plan->length == plan->capacity) {
        capacity = plan->capacity ? plan->capacity * 2 : 64;
        if (capacity <= plan->capacity) return 0;
        grown = realloc(plan->bytes, capacity);
        if (!grown) return 0;
        plan->bytes = grown;
        plan->capacity = capacity;
    }
    plan->bytes[plan->length++] = (unsigned char)value;
    return 1;
}

static int levelc_parseplan_number(LevelCParsePlanBytes *plan,
                                   uint64_t value,
                                   unsigned int width) {
    unsigned int index;
    for (index = 0; index < width; index++) {
        if (!levelc_parseplan_byte(plan, (unsigned int)(value & 0xff))) return 0;
        value >>= 8;
    }
    return 1;
}

static int levelc_parseplan_item(LevelCParsePlanBytes *plan,
                                 unsigned int kind,
                                 unsigned int flags) {
    if (plan->item_count == UINT16_MAX) return 0;
    if (!levelc_parseplan_byte(plan, kind) ||
        !levelc_parseplan_byte(plan, flags)) return 0;
    plan->item_count++;
    return 1;
}

static int levelc_parseplan_position_value(ASTNode *position, uint64_t *out) {
    char *text;
    char *end;
    unsigned long long value;
    if (!position || !out) return 0;
    text = levelc_node_text_copy(position);
    if (!text || !*text || !isdigit((unsigned char)*text)) {
        free(text);
        return 0;
    }
    errno = 0;
    value = strtoull(text, &end, 10);
    if (errno || *end || value > INT64_MAX) {
        free(text);
        return 0;
    }
    *out = (uint64_t)value;
    free(text);
    return 1;
}

static int levelc_parseplan_literal(ASTNode *pattern,
                                    unsigned char **bytes_out,
                                    size_t *length_out,
                                    size_t *chars_out) {
    const char *raw;
    size_t raw_length;
    size_t close_index;
    size_t index;
    size_t count = 0;
    unsigned char *bytes;
    if (levelc_byte_literal_kind(pattern)) {
        unsigned char *ordinals = NULL;
        size_t ordinal_count = 0;
        if (!levelc_byte_literal_bytes(pattern, &ordinals, &ordinal_count))
            return 0;
        bytes = levelc_latin1_utf8(ordinals, ordinal_count, length_out);
        free(ordinals);
        if (!bytes) return 0;
        *bytes_out = bytes;
        *chars_out = ordinal_count;
        return 1;
    }
    if (!pattern || !pattern->token || !bytes_out || !length_out || !chars_out)
        return 0;
    raw = pattern->token->token_string;
    raw_length = pattern->token->length;
    if (!raw || raw_length < 2 || (raw[0] != '\'' && raw[0] != '"') ||
        raw[raw_length - 1] != raw[0]) return 0;
    close_index = raw_length - 1;
    bytes = malloc(raw_length + 1);
    if (!bytes) return 0;
    for (index = 1; index < close_index; index++) {
        if (raw[index] == raw[0] && index + 1 < close_index &&
            raw[index + 1] == raw[0]) index++;
        bytes[count++] = (unsigned char)raw[index];
    }
    bytes[count] = 0;
#ifndef NUTF8
    if (utf8nvalid_count(bytes, count, chars_out)) {
        free(bytes);
        return 0;
    }
#else
    *chars_out = count;
#endif
    *bytes_out = bytes;
    *length_out = count;
    return 1;
}

static char *levelc_static_signal_name(ASTNode *target) {
    unsigned char *decoded = NULL;
    size_t length = 0;
    size_t chars = 0;
    char *name;
    if (!target) return NULL;
    if (target->node_type != STRING) return levelc_upper_name(target);
    name = levelc_parseplan_literal(target, &decoded, &length, &chars)
        ? rxcp_levelc_upper_text((const char *)decoded, length) : NULL;
    free(decoded);
    return name;
}

static ASTNode *levelc_parseplan_descriptor(Context *context,
                                            ASTNode *stmt,
                                            ASTNode *first,
                                            unsigned int *result_count_out,
                                            ASTNode ***external_operands_out,
                                            unsigned int *external_count_out) {
    LevelCParsePlanBytes plan = {0};
    ASTNode *cursor;
    ASTNode *prev1 = NULL;
    ASTNode *prev2 = NULL;
    ASTNode *prev3 = NULL;
    ASTNode *descriptor = NULL;
    char *escaped_text;
    size_t index;
    unsigned int header_size = 8;
    for (cursor = first; cursor; cursor = cursor->sibling) {
        if ((cursor->node_type == PATTERN && cursor->child) ||
            ((cursor->node_type == ABS_POS || cursor->node_type == REL_POS) &&
             cursor->child && cursor->child->node_type == VAR_REFERENCE)) {
            header_size = 12;
            break;
        }
    }
    for (index = 0; index < header_size; index++) {
        if (!levelc_parseplan_byte(&plan, 0)) goto done;
    }
    for (cursor = first; cursor; cursor = cursor->sibling) {
        if (cursor->node_type == TARGET) {
            char *name = levelc_upper_name(cursor);
            int is_dot = name && strcmp(name, ".") == 0;
            free(name);
            if (!plan.item_count &&
                (!levelc_parseplan_item(&plan, LEVELC_PARSEPLAN_ABSOLUTE, 0) ||
                 !levelc_parseplan_number(&plan, 1, 8))) goto done;
            if (prev1 && prev1->node_type == TARGET &&
                !levelc_parseplan_item(&plan, LEVELC_PARSEPLAN_IMPLICIT_WORD, 0)) goto done;
            if (!levelc_parseplan_item(&plan, LEVELC_PARSEPLAN_TARGET, is_dot ? 0 : 1)) goto done;
            if (!is_dot) {
                if (plan.result_count == UINT16_MAX) goto done;
                plan.result_count++;
            }
        } else if (cursor->node_type == PATTERN) {
            unsigned char *bytes = NULL;
            size_t length;
            size_t chars;
            if (cursor->child) {
                unsigned int completed = prev1 && prev1->node_type == TARGET
                    ? plan.result_count > 0 ? plan.result_count - 1 : 0
                    : plan.result_count;
                if (!levelc_parseplan_dynamic_item(&plan, first, cursor,
                        cursor->child, completed,
                        LEVELC_PARSEPLAN_DYNAMIC_LITERAL)) goto done;
                goto next_item;
            }
            if (!levelc_parseplan_literal(cursor, &bytes, &length, &chars)) goto done;
            if (length > UINT32_MAX || chars > UINT32_MAX ||
                !levelc_parseplan_item(&plan, LEVELC_PARSEPLAN_LITERAL, 0) ||
                !levelc_parseplan_number(&plan, length, 4) ||
                !levelc_parseplan_number(&plan, chars, 4)) {
                free(bytes);
                goto done;
            }
            for (index = 0; index < length; index++) {
                if (!levelc_parseplan_byte(&plan, bytes[index])) {
                    free(bytes);
                    goto done;
                }
            }
            free(bytes);
        } else {
            ASTNode *position = cursor->child ? cursor->child : cursor;
            uint64_t value;
            unsigned int kind = cursor->node_type == ABS_POS ? LEVELC_PARSEPLAN_ABSOLUTE
                : cursor->node_string && cursor->node_string[0] == '+' ?
                    LEVELC_PARSEPLAN_RELATIVE_PLUS : LEVELC_PARSEPLAN_RELATIVE_MINUS;
            unsigned int flags = 0;
            if (position->node_type == VAR_REFERENCE) {
                unsigned int completed = prev1 && prev1->node_type == TARGET
                    ? plan.result_count > 0 ? plan.result_count - 1 : 0
                    : plan.result_count;
                if (!levelc_parseplan_dynamic_item(&plan, first, cursor,
                        position, completed, kind + 5)) goto done;
                goto next_item;
            }
            if (!levelc_parseplan_position_value(position, &value)) goto done;
            if (kind == LEVELC_PARSEPLAN_ABSOLUTE && prev3 && prev3->node_type == ABS_POS &&
                prev2 && prev2->node_type == TARGET &&
                prev1 && prev1->node_type == PATTERN &&
                cursor->sibling && cursor->sibling->node_type == TARGET) {
                ASTNode *earlier_position = prev3->child ? prev3->child : prev3;
                char *earlier_text = levelc_node_text_copy(earlier_position);
                char *current_text = levelc_node_text_copy(position);
                if (!earlier_text || !current_text) {
                    free(earlier_text);
                    free(current_text);
                    goto done;
                }
                flags = strcmp(earlier_text, current_text) == 0;
                free(earlier_text);
                free(current_text);
            }
            if (!levelc_parseplan_item(&plan, kind, flags) ||
                !levelc_parseplan_number(&plan, value, 8)) goto done;
        }
next_item:
        prev3 = prev2;
        prev2 = prev1;
        prev1 = cursor;
    }
    plan.bytes[0] = 'P';
    plan.bytes[1] = header_size == 8 ? 1 : 2;
    plan.bytes[2] = (unsigned char)header_size;
    plan.bytes[4] = (unsigned char)(plan.item_count & 0xff);
    plan.bytes[5] = (unsigned char)(plan.item_count >> 8);
    plan.bytes[6] = (unsigned char)(plan.result_count & 0xff);
    plan.bytes[7] = (unsigned char)(plan.result_count >> 8);
    if (header_size == 12) {
        plan.bytes[8] = (unsigned char)(plan.external_count & 0xff);
        plan.bytes[9] = (unsigned char)(plan.external_count >> 8);
        plan.bytes[10] = 1; /* Classic numeric-position error policy. */
    }
    if (plan.length > (SIZE_MAX - 1) / 4) goto done;
    escaped_text = malloc(plan.length * 4 + 1);
    if (!escaped_text) goto done;
    size_t escaped_length = 0;
    for (index = 0; index < plan.length; index++) {
        const char *escaped = escape_character(plan.bytes[index]);
        size_t byte_length = strlen(escaped);
        memcpy(escaped_text + escaped_length, escaped, byte_length);
        escaped_length += byte_length;
    }
    escaped_text[escaped_length] = '\0';
    descriptor = ast_ft(context, STRING);
    if (descriptor) ast_sstr(descriptor, escaped_text, escaped_length);
    else free(escaped_text);
    if (descriptor) {
        rxcp_remap_anchor_synthetic(descriptor, stmt);
        if (result_count_out) *result_count_out = plan.result_count;
        if (external_operands_out) {
            *external_operands_out = plan.external_operands;
            plan.external_operands = NULL;
        }
        if (external_count_out) *external_count_out = plan.external_count;
    }
done:
    free(plan.bytes);
    free(plan.external_operands);
    return descriptor;
}

static int levelc_lower_template_segment(Context *context,
                                         ASTNode *instructions,
                                         ASTNode *stmt,
                                         ASTNode *segment,
                                         ASTNode *value,
                                         ASTNode *prelude,
                                         LevelCLowerPlan *plan,
                                         int upper,
                                         const char **reason_out) {
    char *source_name = NULL;
    char *fields_name = NULL;
    ASTNode *source_string;
    ASTNode *source_capture;
    ASTNode *fields_define;
    ASTNode *descriptor;
    ASTNode *fields_ref;
    ASTNode *source_ref;
    ASTNode *parse_instr;
    ASTNode *target;
    ASTNode **external_operands = NULL;
    unsigned int result_count = 0;
    unsigned int result_index = 0;
    unsigned int external_count = 0;
    unsigned int external_index;

    if (!context || !instructions || !stmt || !segment || !prelude || !value)
        goto fail;
    if (!segment->child) {
        rxcp_remap_append_builder_children(instructions, prelude);
        return 1;
    }
    if (upper) {
        ASTNode *function = ast_f(context, FUNCTION, stmt->token);
        ASTNode *function_arg = ast_f(context, VAR_SYMBOL, stmt->token);
        if (!function || !function_arg) goto fail;
        add_ast(function, function_arg);
        value = levelc_lower_bif_dispatch_call(context, function, "TRANSLATE", NULL,
                                               prelude, LEVELC_BIF_TRANSLATE_HELPER,
                                               value);
        if (!value) goto fail;
    }

    source_name = rxcp_remap_create_generated_node_name(
        LEVELC_PARSE_SOURCE_PREFIX, segment);
    fields_name = rxcp_remap_create_generated_node_name(
        LEVELC_PARSE_FIELDS_PREFIX, segment);
    source_string = rxcp_remap_create_member_call(
        context, stmt, value, "asString", NULL, 0);
    source_capture = source_name && source_string
        ? rxcp_remap_create_named_assignment(context, stmt,
                                             source_name, source_string)
        : NULL;
    fields_define = fields_name
        ? rxcp_remap_create_array_define(context, stmt, fields_name, ".string")
        : NULL;
    descriptor = levelc_parseplan_descriptor(context, stmt, segment->child,
                                              &result_count, &external_operands,
                                              &external_count);
    fields_ref = fields_name
        ? rxcp_remap_create_named_ref(context, stmt, VAR_SYMBOL, fields_name)
        : NULL;
    source_ref = source_name
        ? rxcp_remap_create_named_ref(context, stmt, VAR_SYMBOL, source_name)
        : NULL;
    parse_instr = descriptor && fields_ref && source_ref
        ? ast_ftt(context, ASSEMBLER, strdup("parseplan")) : NULL;
    if (!source_capture || !fields_define || !parse_instr) goto fail;
    parse_instr->free_node_string = 1;
    rxcp_remap_anchor_synthetic(parse_instr, stmt);
    add_ast(parse_instr, fields_ref);
    add_ast(parse_instr, source_ref);
    add_ast(parse_instr, descriptor);
    add_ast(prelude, source_capture);
    add_ast(prelude, fields_define);
    for (external_index = 0; external_index < external_count; external_index++) {
        ASTNode *operand = external_operands[external_index];
        ASTNode *pool_value = levelc_pool_value(context, operand, plan,
                                                prelude);
        ASTNode *as_string = pool_value
            ? rxcp_remap_create_member_call(context, operand, pool_value,
                                            "asString", NULL, 0) : NULL;
        ASTNode *assignment = as_string &&
            result_count + external_index + 1 <= INT_MAX
            ? rxcp_remap_create_indexed_assignment(context, operand,
                fields_name, (int)(result_count + external_index + 1), as_string)
            : NULL;
        if (!assignment) goto fail;
        add_ast(prelude, assignment);
    }
    add_ast(prelude, parse_instr);
    rxcp_remap_append_builder_children(instructions, prelude);

    for (target = segment->child; target; target = target->sibling) {
        char *target_name;
        int is_dot;
        ASTNode *field;
        ASTNode *value_args[1];
        ASTNode *receiver;
        ASTNode *args[2];
        ASTNode *lowered;
        if (target->node_type != TARGET) continue;
        target_name = levelc_upper_name(target);
        is_dot = target_name && strcmp(target_name, ".") == 0;
        free(target_name);
        if (is_dot) continue;
        result_index++;
        field = rxcp_remap_create_indexed_ref(context, target, VAR_SYMBOL,
                                              fields_name, (int)result_index);
        value_args[0] = field;
        receiver = levelc_pool_ref(context, target, VAR_SYMBOL);
        args[0] = levelc_name_string(context, target);
        args[1] = field
            ? rxcp_remap_create_factory_call(context, target,
                                             LEVELC_REXX_VALUE_CLASS,
                                             value_args, 1)
            : NULL;
        lowered = receiver && args[0] && args[1]
            ? rxcp_remap_create_member_call_statement(context, target,
                                                       receiver, "setSymbolValue",
                                                       args, 2)
            : NULL;
        if (!lowered) goto fail;
        add_ast(instructions, lowered);
    }
    if (result_index != result_count) goto fail;
    free(source_name);
    free(fields_name);
    free(external_operands);
    return 1;

fail:
    free(source_name);
    free(fields_name);
    free(external_operands);
    if (reason_out) *reason_out = "failed to lower PARSE/ARG template";
    return 0;
}

static int levelc_lower_direct_parse(Context *context,
                                     ASTNode *instructions,
                                     ASTNode *stmt,
                                     LevelCLowerPlan *plan,
                                     const char **reason_out) {
    ASTNode *source;
    ASTNode *segment;
    ASTNode *prelude;
    ASTNode *value;
    int is_value;
    int upper;

    if (!levelc_parse_shape(stmt, plan, &source, &segment,
                            &is_value, &upper, reason_out)) return 0;
    prelude = rxcp_remap_create_instruction_builder(context, stmt);
    if (!prelude) goto fail;
    value = is_value ? levelc_lower_expr(context, source->child, plan, prelude)
                     : levelc_pool_value(context, source, plan, prelude);
    if (!value) goto fail;
    return levelc_lower_template_segment(context, instructions, stmt, segment,
                                          value, prelude, plan, upper, reason_out);

fail:
    if (reason_out) *reason_out = "failed to lower supported PARSE shape";
    return 0;
}

static int levelc_lower_arg_instruction(Context *context,
                                        ASTNode *instructions,
                                        ASTNode *stmt,
                                        LevelCLowerPlan *plan,
                                        const char **reason_out) {
    ASTNode *templates = stmt ? stmt->child : NULL;
    ASTNode *segment = templates ? templates->child : NULL;
    size_t index = 1;

    while (segment) {
        ASTNode *prelude;
        ASTNode *activation;
        ASTNode *args[1];
        ASTNode *value;
        if (index > INT_MAX) goto fail;
        if (!segment->child) {
            segment = segment->sibling;
            index++;
            continue;
        }
        prelude = rxcp_remap_create_instruction_builder(context, stmt);
        activation = rxcp_remap_create_named_ref(context, stmt, VAR_SYMBOL,
                                                  LEVELC_ACTIVATION_SYMBOL);
        args[0] = rxcp_remap_create_integer_constant(context, stmt,
                                                      (int)index, TP_INTEGER);
        value = activation && args[0]
            ? rxcp_remap_create_member_call(context, stmt, activation,
                                            "argument", args, 1)
            : NULL;
        if (!prelude || !value ||
            !levelc_lower_template_segment(context, instructions, stmt, segment,
                                            value, prelude, plan, 1, reason_out))
            return 0;
        segment = segment->sibling;
        index++;
    }
    return 1;

fail:
    if (reason_out) *reason_out = "ARG template position exceeds compiler index range";
    return 0;
}

static int levelc_append_signal_sigl(Context *context,
                                     ASTNode *instructions,
                                     ASTNode *stmt) {
    ASTNode *line_value;
    ASTNode *line_text;
    ASTNode *receiver;
    ASTNode *args[2];
    ASTNode *set_sigl;
    char line[32];

    snprintf(line, sizeof(line), "%d", stmt->token ? stmt->token->line + 1 : 0);
    line_text = rxcp_remap_create_string_constant(context, stmt, line);
    args[0] = rxcp_remap_create_string_constant(context, stmt, "SIGL");
    {
        ASTNode *value_args[1] = {line_text};
        line_value = line_text ? rxcp_remap_create_factory_call(
            context, stmt, LEVELC_REXX_VALUE_CLASS, value_args, 1) : NULL;
    }
    args[1] = line_value;
    receiver = levelc_pool_ref(context, stmt, VAR_SYMBOL);
    set_sigl = receiver && args[0] && args[1]
        ? rxcp_remap_create_member_call_statement(context, stmt, receiver,
                                                  "setSymbolValue", args, 2)
        : NULL;
    if (!set_sigl) return 0;
    add_ast(instructions, set_sigl);
    return 1;
}

static int levelc_signal_mode(ASTNode *stmt, const char *expected) {
    char *mode = stmt && stmt->child ? levelc_upper_name(stmt->child) : NULL;
    int equal = mode && strcmp(mode, expected) == 0;
    free(mode);
    return equal;
}

static ASTNode *levelc_signal_handler_node(Context *context,
                                            ASTNode *source,
                                            NodeType type,
                                            char *condition,
                                            ASTNode *destination) {
    ASTNode *node = levelc_frame_node(context, source, type, destination);
    if (!node) return NULL;
    ast_copy_str(node, condition);
    return node;
}

static int levelc_append_signal_policy_index(Context *context,
                                              ASTNode *instructions,
                                              ASTNode *source,
                                              int condition_id,
                                              int handler_index) {
    ASTNode *receiver = rxcp_remap_create_named_ref(
        context, source, VAR_SYMBOL, LEVELC_ACTIVATION_SYMBOL);
    ASTNode *args[2];
    ASTNode *statement;
    args[0] = rxcp_remap_create_integer_constant(
        context, source, condition_id, TP_INTEGER);
    args[1] = rxcp_remap_create_integer_constant(
        context, source, handler_index, TP_INTEGER);
    statement = receiver && args[0] && args[1]
        ? rxcp_remap_create_member_call_statement(
            context, source, receiver, "setSignalPolicy", args, 2)
        : NULL;
    if (!statement) return 0;
    add_ast(instructions, statement);
    return 1;
}

static int levelc_lower_signal_policy(Context *context,
                                       ASTNode *instructions,
                                       ASTNode *stmt,
                                       LevelCLowerPlan *plan,
                                       const char **reason_out) {
    ASTNode *condition_node = stmt->child->sibling;
    char *condition = levelc_upper_name(condition_node);
    ASTNode *operation;
    int condition_id = levelc_signal_condition_id(condition);

    if (!condition_id) goto fail;
    if (levelc_signal_mode(stmt, "OFF")) {
        if (condition_id == LEVELC_SIGNAL_SYNTAX_ID) {
            operation = levelc_signal_handler_node(context, stmt,
                                                    FRAME_HANDLER_OFF, condition,
                                                    NULL);
            if (!operation) goto fail;
            add_ast(instructions, operation);
        }
        free(condition);
        return levelc_append_signal_policy_index(
            context, instructions, stmt, condition_id, 0);
    }
    if (levelc_signal_mode(stmt, "ON")) {
        LevelCSignalHandler *handlers;
        LevelCSignalHandler *handler;
        ASTNode *target = condition_node->sibling;
        char *name = target ? levelc_static_signal_name(target->child)
                            : strdup(condition);
        LevelCProcedureSlice *destination;
        ASTNode *binding;
        if (!name) {
            free(condition);
            goto fail;
        }
        if (plan->signal_handler_count >= INT_MAX) {
            free(condition);
            free(name);
            goto fail;
        }
        destination = levelc_find_procedure(plan, name);
        handlers = realloc(plan->signal_handlers,
                           sizeof(*handlers) * (plan->signal_handler_count + 1));
        if (!handlers) {
            free(condition);
            free(name);
            goto fail;
        }
        plan->signal_handlers = handlers;
        handler = &handlers[plan->signal_handler_count++];
        handler->source = stmt;
        handler->trampoline = levelc_frame_node(context, stmt, FRAME_LABEL, NULL);
        handler->destination = destination ? destination->frame_label : NULL;
        handler->condition = condition;
        handler->target_name = name;
        handler->condition_id = condition_id;
        if (condition_id != LEVELC_SIGNAL_SYNTAX_ID &&
            !plan->classic_condition_dispatch) {
            plan->classic_condition_dispatch = levelc_frame_node(
                context, stmt, FRAME_LABEL, NULL);
            plan->classic_condition_source = stmt;
            if (!plan->classic_condition_dispatch) goto fail;
        }
        operation = condition_id == LEVELC_SIGNAL_SYNTAX_ID && handler->trampoline
            ? levelc_signal_handler_node(context, stmt, FRAME_HANDLER_ON,
                                         condition, handler->trampoline)
            : NULL;
        binding = operation ? rxcp_remap_create_named_ref(
            context, stmt, VAR_TARGET, LEVELC_SIGNAL_EVENT_SYMBOL) : NULL;
        if (condition_id == LEVELC_SIGNAL_SYNTAX_ID) {
            if (!operation || !binding) goto fail;
            add_ast(operation, binding);
            add_ast(instructions, operation);
        }
        return levelc_append_signal_policy_index(
            context, instructions, stmt, handler->condition_id,
            (int)plan->signal_handler_count);
    }
    free(condition);

fail:
    if (reason_out) *reason_out = "failed to lower Level C SIGNAL condition policy";
    return 0;
}

static int levelc_lower_direct_signal(Context *context,
                                      ASTNode *instructions,
                                      ASTNode *stmt,
                                      LevelCLowerPlan *plan,
                                      const char **reason_out) {
    ASTNode *target = stmt->child;
    ASTNode *branch;
    LevelCProcedureSlice *destination;
    char *name;

    /* Decode quoted source text before matching the Unicode frame label. */
    name = levelc_static_signal_name(target);
    if (!name) goto fail;

    if (!levelc_append_signal_sigl(context, instructions, stmt)) {
        free(name);
        goto fail;
    }

    destination = levelc_find_procedure(plan, name);
    if (destination && destination->frame_label) {
        branch = levelc_frame_node(context, stmt, FRAME_BRANCH,
                                   destination->frame_label);
    } else {
        size_t length = strlen(name) + 64;
        char *detail = malloc(length);
        ASTNode *condition = rxcp_remap_create_integer_constant(
            context, stmt, 1, TP_BOOLEAN);
        if (!detail || !condition) {
            free(detail);
            free(name);
            goto fail;
        }
        snprintf(detail, length, "RXC-LC-16.1: Label not found: %s", name);
        if (!levelc_append_classic_error_if(context, instructions, stmt,
                                             condition, detail)) {
            free(detail);
            free(name);
            goto fail;
        }
        free(detail);
        branch = NULL;
    }
    free(name);
    if (branch) add_ast(instructions, branch);
    return 1;

fail:
    if (reason_out) *reason_out = "failed to lower direct Level C SIGNAL";
    return 0;
}

static int levelc_lower_value_signal(Context *context,
                                     ASTNode *instructions,
                                     ASTNode *stmt,
                                     LevelCLowerPlan *plan,
                                     const char **reason_out) {
    ASTNode *value_form = stmt->child;
    ASTNode *expression = value_form->child;
    ASTNode *prelude = rxcp_remap_create_instruction_builder(context, stmt);
    ASTNode *value;
    ASTNode *function;
    ASTNode *function_arg;
    ASTNode *upper;
    ASTNode *as_string;
    ASTNode *capture;
    char *target_name = rxcp_remap_create_generated_node_name(
        LEVELC_SIGNAL_TARGET_PREFIX, stmt);
    size_t i;

    if (!prelude || !target_name) goto fail;
    value = levelc_lower_expr(context, expression, plan, prelude);
    function = ast_f(context, FUNCTION, value_form->token);
    function_arg = ast_f(context, VAR_SYMBOL, value_form->token);
    if (!value || !function || !function_arg) goto fail;
    add_ast(function, function_arg);
    upper = levelc_lower_bif_dispatch_call(context, function, "TRANSLATE", plan,
                                            prelude, LEVELC_BIF_TRANSLATE_HELPER,
                                            value);
    as_string = upper ? rxcp_remap_create_member_call(
        context, stmt, upper, "asString", NULL, 0) : NULL;
    capture = as_string ? rxcp_remap_create_named_assignment(
        context, stmt, target_name, as_string) : NULL;
    if (!capture) goto fail;
    add_ast(prelude, capture);
    rxcp_remap_append_builder_children(instructions, prelude);
    if (!levelc_append_signal_sigl(context, instructions, stmt)) goto fail;

    for (i = 0; i < plan->procedure_count; i++) {
        ASTNode *source = plan->procedures[i].label;
        ASTNode *condition = ast_f(context, OP_COMPARE_EQUAL, stmt->token);
        ASTNode *branch = levelc_frame_node(
            context, stmt, FRAME_BRANCH, plan->procedures[i].frame_label);
        ASTNode *then_instructions = rxcp_remap_create_instruction_builder(context, stmt);
        ASTNode *then_block;
        ASTNode *if_statement;
        if (!condition || !branch || !then_instructions) goto fail;
        rxcp_remap_anchor_synthetic(condition, stmt);
        add_ast(condition, rxcp_remap_create_named_ref(
            context, stmt, VAR_SYMBOL, target_name));
        add_ast(condition, rxcp_remap_create_string_constant(
            context, source, plan->procedures[i].name));
        add_ast(then_instructions, branch);
        then_block = rxcp_remap_create_do_block(context, stmt, then_instructions);
        if_statement = then_block ? rxcp_remap_create_if_statement(
            context, stmt, condition, then_block, NULL) : NULL;
        if (!if_statement) goto fail;
        add_ast(instructions, if_statement);
    }
    {
        ASTNode *detail = ast_f(context, OP_CONCAT, stmt->token);
        ASTNode *signal = ast_ftt(context, ASSEMBLER, strdup("signal"));
        if (!detail || !signal) goto fail;
        rxcp_remap_anchor_synthetic(detail, stmt);
        add_ast(detail, rxcp_remap_create_string_constant(
            context, stmt, "RXC-LC-16.1: Label not found: "));
        add_ast(detail, rxcp_remap_create_named_ref(
            context, stmt, VAR_SYMBOL, target_name));
        signal->free_node_string = 1;
        signal->is_compiler_added = 1;
        rxcp_remap_anchor_synthetic(signal, stmt);
        add_ast(signal, rxcp_remap_create_string_constant(
            context, stmt, "CLASSIC_SYNTAX"));
        add_ast(signal, detail);
        add_ast(instructions, signal);
    }
    free(target_name);
    return 1;

fail:
    free(target_name);
    if (reason_out) *reason_out = "failed to lower evaluated Level C SIGNAL";
    return 0;
}

static int levelc_lower_statement(Context *context,
                                  ASTNode *instructions,
                                  ASTNode *stmt,
                                  LevelCLowerPlan *plan,
                                  LevelCProcedureSlice *procedure,
                                  int in_procedure,
                                  const char **reason_out) {
    ASTNode *prelude;
    ASTNode *lowered;

    if (!stmt) return 1;
    if (stmt->node_type == NOP) return levelc_lower_nop(context, instructions, stmt, reason_out);
    if (stmt->node_type == LEAVE || stmt->node_type == ITERATE)
        return levelc_lower_transfer(context, instructions, stmt, plan, reason_out);
    if (stmt->node_type == LEVELC_DROP) {
        return levelc_lower_drop(context, instructions, stmt, plan, reason_out);
    }
    if (stmt->node_type == LEVELC_ARG) {
        return levelc_lower_arg_instruction(context, instructions, stmt,
                                            plan, reason_out);
    }
    if (stmt->node_type == LEVELC_SIGNAL) {
        if (stmt->child && stmt->child->node_type == LEVELC_SIGNAL_VALUE)
            return levelc_lower_value_signal(context, instructions, stmt, plan,
                                             reason_out);
        if (levelc_signal_mode(stmt, "ON") || levelc_signal_mode(stmt, "OFF"))
            return levelc_lower_signal_policy(context, instructions, stmt,
                                              plan, reason_out);
        return levelc_lower_direct_signal(context, instructions, stmt, plan,
                                          reason_out);
    }
    if (in_procedure && stmt->node_type == LEVELC_PROCEDURE) {
        if (!levelc_append_procedure_entry(context, instructions, stmt)) {
            if (reason_out) *reason_out = "failed to validate PROCEDURE activation";
            return 0;
        }
        if (!levelc_append_private_pool_transition(context, instructions, stmt)) {
            if (reason_out) *reason_out = "failed to create PROCEDURE pool transition";
            return 0;
        }
        return levelc_append_procedure_exposes(context, instructions, stmt, reason_out);
    }
    if (stmt->node_type == PARSE)
        return levelc_lower_direct_parse(context, instructions, stmt, plan, reason_out);
    if (stmt->node_type == IF) {
        return levelc_lower_if_statement(context, instructions, stmt, plan,
                                         procedure, in_procedure, reason_out);
    }
    if (stmt->node_type == DO) {
        return levelc_lower_do(context, instructions, stmt, plan,
                               procedure, in_procedure, reason_out);
    }
    if (stmt->node_type == SELECT) {
        return levelc_lower_select_statement(context, instructions, stmt, plan,
                                             procedure, in_procedure, reason_out);
    }

    prelude = rxcp_remap_create_instruction_builder(context, stmt);
    if (!prelude) {
        if (reason_out) *reason_out = "failed to create Level C statement prelude";
        return 0;
    }

    if (stmt->node_type == ASSIGN) {
        lowered = levelc_pool_set_statement(context, stmt, plan, prelude, NULL);
    } else if (stmt->node_type == REXX_OPTIONS) {
        lowered = levelc_options_statement(context, stmt, plan, prelude);
    } else if (stmt->node_type == SAY) {
        lowered = levelc_say_statement(context, stmt, plan, prelude);
    } else if (stmt->node_type == CALL) {
        lowered = levelc_call_local_procedure_statement(context, stmt, plan, prelude);
    } else if (!in_procedure && stmt->node_type == EXIT) {
        lowered = rxcp_remap_create_return_statement(context, stmt);
    } else if (in_procedure && stmt->node_type == RETURN) {
        lowered = levelc_proc_return_statement(context, stmt, plan, procedure, prelude);
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

static int levelc_lower_main_statement(Context *context,
                                       ASTNode *instructions,
                                       ASTNode *stmt,
                                       LevelCLowerPlan *plan,
                                       const char **reason_out) {
    return levelc_lower_statement(context, instructions, stmt, plan,
                                  NULL, 0, reason_out);
}

static int levelc_lower_proc_statement(Context *context,
                                       ASTNode *instructions,
                                       ASTNode *stmt,
                                       LevelCLowerPlan *plan,
                                       LevelCProcedureSlice *procedure,
                                       const char **reason_out) {
    return levelc_lower_statement(context, instructions, stmt, plan,
                                  procedure, 1, reason_out);
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

static ASTNode *levelc_do_state_ref(Context *context,
                                    ASTNode *source,
                                    const char *state_name) {
    return rxcp_remap_create_named_ref(context, source, VAR_SYMBOL, state_name);
}

static ASTNode *levelc_do_state_statement(Context *context,
                                          ASTNode *source,
                                          const char *state_name,
                                          const char *method,
                                          ASTNode *value) {
    ASTNode *receiver = levelc_do_state_ref(context, source, state_name);
    ASTNode *args[1] = {value};

    return receiver
        ? rxcp_remap_create_member_call_statement(context, source, receiver,
                                                  method, value ? args : NULL,
                                                  value ? 1 : 0)
        : NULL;
}

static int levelc_do_state_operand(Context *context,
                                   ASTNode *source,
                                   LevelCLowerPlan *plan,
                                   ASTNode *setup,
                                   const char *state_name,
                                   const char *method) {
    ASTNode *value = levelc_lower_expr(context, source, plan, setup);
    ASTNode *call = value
        ? levelc_do_state_statement(context, source, state_name, method, value)
        : NULL;

    if (!call) return 0;
    add_ast(setup, call);
    return 1;
}

static ASTNode *levelc_lower_state_do(Context *context,
                                      ASTNode *instructions,
                                      ASTNode *stmt,
                                      LevelCDoHeader *header,
                                      LevelCLowerPlan *plan,
                                      ASTNode *lowered_body,
                                      char **loop_name_out) {
    ASTNode *setup;
    ASTNode *factory_args[2];
    ASTNode *factory;
    ASTNode *state_assignment;
    ASTNode *entry;
    ASTNode *end_setup;
    ASTNode *until_value;
    ASTNode *while_source;
    ASTNode *until_source;
    ASTNode *lowered;
    ASTNode *clause;
    ASTNode *condition = header->condition;
    ASTNode *anchor = header->repeat ? header->repeat : stmt;
    char *state_name = rxcp_remap_create_generated_node_name(
            LEVELC_DO_STATE_PREFIX, stmt);
    char *loop_name = rxcp_remap_create_generated_node_name(
            LEVELC_LOOP_PREFIX, stmt);

    if (!state_name || !loop_name) goto fail;
    setup = rxcp_remap_create_instruction_builder(context, stmt);
    factory_args[0] = rxcp_remap_create_reference_expr(
            context, stmt, levelc_pool_ref(context, stmt, VAR_SYMBOL));
    factory_args[1] = header->kind == LEVELC_DO_CONTROLLED
        ? levelc_name_string(context, header->target)
        : rxcp_remap_create_string_constant(context, stmt, "");
    factory = factory_args[0] && factory_args[1]
        ? rxcp_remap_create_factory_call(context, stmt, "RexxDoState",
                                         factory_args, 2)
        : NULL;
    state_assignment = factory
        ? rxcp_remap_create_named_assignment(context, stmt, state_name, factory)
        : NULL;
    if (!setup || !state_assignment) goto fail;
    add_ast(setup, state_assignment);

    if (header->kind == LEVELC_DO_COUNTED) {
        if (!levelc_do_state_operand(context, header->repeat->child->child,
                                     plan, setup, state_name, "setRepeatCount"))
            goto fail;
    } else {
        if (!levelc_do_state_operand(context, header->start, plan, setup,
                                     state_name, "setStart")) goto fail;
        for (clause = header->assign->sibling; clause; clause = clause->sibling) {
            const char *method = clause->node_type == TO ? "setTo"
                               : clause->node_type == BY ? "setBy"
                               : "setForCount";
            if (!levelc_do_state_operand(context, clause->child, plan, setup,
                                         state_name, method)) goto fail;
        }
        state_assignment = levelc_do_state_statement(
                context, header->assign, state_name, "startControl", NULL);
        if (!state_assignment) goto fail;
        add_ast(setup, state_assignment);
    }

    entry = rxcp_remap_create_member_call(
            context, anchor, levelc_do_state_ref(context, anchor, state_name),
            "entryAllowed", NULL, 0);
    if (!entry) goto fail;
    if (condition && condition->node_type == WHILE) {
        entry = levelc_controlled_while_entry(context, condition, plan, entry);
        if (!entry) goto fail;
    }

    end_setup = rxcp_remap_create_instruction_builder(context, anchor);
    state_assignment = end_setup
        ? levelc_do_state_statement(context, anchor, state_name, "advance", NULL)
        : NULL;
    if (!state_assignment) goto fail;
    add_ast(end_setup, state_assignment);
    until_value = condition && condition->node_type == UNTIL
        ? levelc_controlled_until_end(context, condition, plan, end_setup)
        : rxcp_remap_create_prelude_block_expr(
                context, anchor, end_setup,
                rxcp_remap_create_integer_constant(context, anchor, 0, TP_BOOLEAN));
    if (!until_value) goto fail;

    while_source = condition && condition->node_type == WHILE
        ? condition : ast_f(context, WHILE, anchor->token);
    until_source = condition && condition->node_type == UNTIL
        ? condition : ast_f(context, UNTIL, anchor->token);
    if (!while_source || !until_source) goto fail;
    if (while_source != condition) rxcp_remap_anchor_synthetic(while_source, anchor);
    if (until_source != condition) rxcp_remap_anchor_synthetic(until_source, anchor);
    lowered = rxcp_remap_create_controlled_do(
            context, stmt, lowered_body, loop_name, NULL,
            while_source, entry, until_source, until_value);
    if (!lowered) goto fail;
    rxcp_remap_append_builder_children(instructions, setup);
    *loop_name_out = loop_name;
    free(state_name);
    return lowered;

fail:
    free(state_name);
    free(loop_name);
    return NULL;
}

static int levelc_lower_do(Context *context,
                           ASTNode *instructions,
                           ASTNode *stmt,
                           LevelCLowerPlan *plan,
                           LevelCProcedureSlice *procedure,
                           int in_procedure,
                           const char **reason_out) {
    LevelCDoHeader *header = levelc_find_do_header(plan, stmt);
    ASTNode *body_statement;
    ASTNode *lowered_body = rxcp_remap_create_instruction_builder(context, stmt);
    ASTNode *lowered = NULL;
    LevelCLoopBinding binding;
    LevelCLoopBinding *previous = plan ? plan->active_loop : NULL;
    char *control_name = NULL;
    int loop_active = 0;

    if (!header || !lowered_body) goto fail;
    if (header->kind == LEVELC_DO_COUNTED ||
        header->kind == LEVELC_DO_CONTROLLED) {
        lowered = levelc_lower_state_do(context, instructions, stmt, header,
                                        plan, lowered_body, &control_name);
        if (!lowered) goto fail;
    } else if (header->kind != LEVELC_DO_GROUP) {
        ASTNode *condition = header->condition;
        ASTNode *condition_value = NULL;

        if (condition) {
            ASTNode *prelude = rxcp_remap_create_instruction_builder(context,
                                                                      condition);
            if (!prelude) goto fail;
            condition_value = levelc_lower_expr(context, condition->child,
                                                plan, prelude);
            condition_value = levelc_do_condition_logical_value(
                    context, condition, condition_value);
            if (!condition_value) goto fail;
            if (prelude->child) {
                condition_value = rxcp_remap_create_prelude_block_expr(
                        context, condition, prelude, condition_value);
                if (!condition_value) goto fail;
            }
        }
        control_name = rxcp_remap_create_generated_node_name(LEVELC_LOOP_PREFIX,
                                                              stmt);
        lowered = control_name
            ? rxcp_remap_create_controlled_do(
                    context, stmt, lowered_body, control_name, NULL,
                    condition, condition_value, NULL, NULL)
            : NULL;
        if (!lowered) goto fail;
    }

    if (header->kind != LEVELC_DO_GROUP) {
        binding.source_do = stmt;
        binding.control_name = control_name;
        binding.previous = previous;
        plan->active_loop = &binding;
        loop_active = 1;
    }
    for (body_statement = header->body->child; body_statement;
         body_statement = body_statement->sibling) {
        if (in_procedure) {
            if (!levelc_lower_proc_statement(context, lowered_body, body_statement,
                                             plan, procedure, reason_out)) goto fail;
        } else if (!levelc_lower_main_statement(context, lowered_body,
                                                 body_statement, plan, reason_out)) {
            goto fail;
        }
    }
    if (header->kind == LEVELC_DO_GROUP)
        lowered = rxcp_remap_create_do_block(context, stmt, lowered_body);
    if (!lowered) goto fail;

    if (loop_active) plan->active_loop = previous;
    add_ast(instructions, lowered);
    free(control_name);
    return 1;

fail:
    if (loop_active) plan->active_loop = previous;
    free(control_name);
    if (reason_out && !*reason_out)
        *reason_out = "failed to lower supported Level C DO";
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
        else if (clause->node_type == OTHERWISE) otherwise = clause;
    }
    whens = calloc(count, sizeof(*whens));
    if (!whens) goto fail;
    for (clause = stmt->child->child; clause; clause = clause->sibling)
        if (clause->node_type == WHEN) whens[index++] = clause;

    fallback = rxcp_remap_create_instruction_builder(
        context, otherwise ? otherwise : whens[count - 1]);
    if (!fallback) goto fail_free;
    if (otherwise) {
        ASTNode *list = otherwise->child;
        ASTNode *part;
        for (part = list->child; part; part = part->sibling) {
            if (!levelc_lower_select_body_statement(context, fallback, part, plan,
                                                    procedure, in_procedure, reason_out)) goto fail_free;
        }
    } else {
        char line[32];
        ASTNode *args[1];
        ASTNode *call;
        ASTNode *last_when = whens[count - 1];
        snprintf(line, sizeof(line), "%d", stmt->token ? stmt->token->line + 1 : 0);
        args[0] = rxcp_remap_create_string_constant(context, last_when, line);
        call = args[0] ? rxcp_remap_create_function_call(context, last_when,
                                                        "rexxvalue_select_missing", args, 1) : NULL;
        call = call ? rxcp_remap_create_call_statement(context, last_when, call) : NULL;
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

static int levelc_tree_contains_arg(ASTNode *node) {
    while (node) {
        if (node->node_type == LEVELC_ARG ||
            levelc_tree_contains_arg(node->child)) return 1;
        node = node->sibling;
    }
    return 0;
}

static int levelc_tree_contains_procedure(ASTNode *node) {
    while (node) {
        if (node->node_type == LEVELC_PROCEDURE ||
            levelc_tree_contains_procedure(node->child)) return 1;
        node = node->sibling;
    }
    return 0;
}

static int levelc_tree_contains_signal_value(ASTNode *node) {
    while (node) {
        if (node->node_type == LEVELC_SIGNAL_VALUE ||
            levelc_tree_contains_signal_value(node->child)) return 1;
        node = node->sibling;
    }
    return 0;
}

static int levelc_tree_contains_signal_on(ASTNode *node) {
    while (node) {
        if (node->node_type == LEVELC_SIGNAL &&
            levelc_signal_mode(node, "ON")) return 1;
        if (levelc_tree_contains_signal_on(node->child)) return 1;
        node = node->sibling;
    }
    return 0;
}

static int levelc_tree_contains_novalue_on(ASTNode *node) {
    while (node) {
        if (node->node_type == LEVELC_SIGNAL &&
            levelc_signal_mode(node, "ON") && node->child &&
            node->child->sibling &&
            node->child->sibling->node_type == LITERAL) {
            char *name = levelc_upper_name(node->child->sibling);
            int is_novalue = name && strcmp(name, "NOVALUE") == 0;
            free(name);
            if (is_novalue) return 1;
        }
        if (levelc_tree_contains_novalue_on(node->child)) return 1;
        node = node->sibling;
    }
    return 0;
}

static int levelc_append_signal_trampolines(Context *context,
                                             ASTNode *instructions,
                                             LevelCLowerPlan *plan,
                                             const char **reason_out) {
    size_t i;
    for (i = 0; i < plan->signal_handler_count; i++) {
        LevelCSignalHandler *handler = &plan->signal_handlers[i];
        ASTNode *off = handler->condition_id == LEVELC_SIGNAL_SYNTAX_ID
            ? levelc_signal_handler_node(context, handler->source,
                                         FRAME_HANDLER_OFF,
                                         handler->condition, NULL)
            : NULL;
        ASTNode *pool = levelc_pool_ref(context, handler->source, VAR_SYMBOL);
        ASTNode *args[3];
        ASTNode *record;
        ASTNode *branch;
        if (!handler->trampoline ||
            (handler->condition_id == LEVELC_SIGNAL_SYNTAX_ID && !off) ||
            !pool) goto fail;
        add_ast(instructions, handler->trampoline);
        if (off) add_ast(instructions, off);
        if (!levelc_append_signal_policy_index(
                context, instructions, handler->source,
                handler->condition_id, 0)) goto fail;
        args[0] = rxcp_remap_create_reference_expr(
            context, handler->source, pool);
        args[1] = rxcp_remap_create_reference_expr(
            context, handler->source,
            rxcp_remap_create_named_ref(
                context, handler->source, VAR_SYMBOL,
                LEVELC_ACTIVATION_SYMBOL));
        args[2] = rxcp_remap_create_named_ref(
            context, handler->source, VAR_SYMBOL,
            LEVELC_SIGNAL_EVENT_SYMBOL);
        record = args[0] && args[1] && args[2] ? rxcp_remap_create_function_call(
            context, handler->source,
            "rexxclassicbifs.rexxclassic_signal_record", args, 3) : NULL;
        record = record ? rxcp_remap_create_call_statement(
            context, handler->source, record) : NULL;
        if (!record) goto fail;
        add_ast(instructions, record);
        if (handler->destination) {
            branch = levelc_frame_node(context, handler->source,
                                       FRAME_BRANCH, handler->destination);
            if (!branch) goto fail;
            add_ast(instructions, branch);
        } else {
            size_t length = strlen(handler->target_name) + 64;
            char *detail = malloc(length);
            char *detail_name = rxcp_remap_create_generated_node_name(
                LEVELC_SIGNAL_DETAIL_PREFIX, handler->source);
            ASTNode *detail_assignment;
            ASTNode *signal;
            if (!detail || !detail_name) {
                free(detail);
                free(detail_name);
                goto fail;
            }
            snprintf(detail, length, "RXC-LC-16.1: Label not found: %s",
                     handler->target_name);
            detail_assignment = rxcp_remap_create_named_assignment(
                context, handler->source, detail_name,
                rxcp_remap_create_string_constant(
                    context, handler->source, detail));
            signal = ast_ftt(context, ASSEMBLER, strdup("signalorigin"));
            if (signal) {
                signal->free_node_string = 1;
                signal->is_compiler_added = 1;
                rxcp_remap_anchor_synthetic(signal, handler->source);
                add_ast(signal, rxcp_remap_create_string_constant(
                    context, handler->source, "CLASSIC_SYNTAX"));
                add_ast(signal, rxcp_remap_create_named_ref(
                    context, handler->source, VAR_SYMBOL, detail_name));
                add_ast(signal, rxcp_remap_create_named_ref(
                    context, handler->source, VAR_SYMBOL,
                    LEVELC_SIGNAL_EVENT_SYMBOL));
            }
            free(detail);
            free(detail_name);
            if (!detail_assignment || !signal) goto fail;
            add_ast(instructions, detail_assignment);
            add_ast(instructions, signal);
        }
    }
    return 1;

fail:
    if (reason_out) *reason_out = "failed to append Level C SIGNAL handler entries";
    return 0;
}

static int levelc_append_classic_condition_dispatch(Context *context,
                                                     ASTNode *instructions,
                                                     LevelCLowerPlan *plan) {
    ASTNode *source = plan->classic_condition_source;
    ASTNode *event;
    ASTNode *kind_args[1];
    ASTNode *kind_call;
    ASTNode *kind_assignment;
    ASTNode *receiver;
    ASTNode *selected_args[1];
    ASTNode *selected_call;
    ASTNode *selected_assignment;
    size_t i;

    if (!plan->classic_condition_dispatch) return 1;
    add_ast(instructions, plan->classic_condition_dispatch);
    event = rxcp_remap_create_named_ref(
        context, source, VAR_SYMBOL, LEVELC_SIGNAL_EVENT_SYMBOL);
    kind_args[0] = event;
    kind_call = event ? rxcp_remap_create_function_call(
        context, source, "rexxclassicbifs.rexxclassic_signal_event_id",
        kind_args, 1) : NULL;
    kind_assignment = kind_call ? rxcp_remap_create_named_assignment(
        context, source, LEVELC_SIGNAL_KIND_SYMBOL, kind_call) : NULL;
    if (!kind_assignment) return 0;
    add_ast(instructions, kind_assignment);

    receiver = rxcp_remap_create_named_ref(
        context, source, VAR_SYMBOL, LEVELC_ACTIVATION_SYMBOL);
    selected_args[0] = rxcp_remap_create_named_ref(
        context, source, VAR_SYMBOL, LEVELC_SIGNAL_KIND_SYMBOL);
    selected_call = receiver && selected_args[0]
        ? rxcp_remap_create_member_call(context, source, receiver,
                                        "signalPolicy", selected_args, 1)
        : NULL;
    selected_assignment = selected_call ? rxcp_remap_create_named_assignment(
        context, source, LEVELC_SIGNAL_SELECTED_SYMBOL, selected_call) : NULL;
    if (!selected_assignment) return 0;
    add_ast(instructions, selected_assignment);

    for (i = 0; i < plan->signal_handler_count; i++) {
        LevelCSignalHandler *handler = &plan->signal_handlers[i];
        ASTNode *condition;
        ASTNode *selected;
        ASTNode *index;
        ASTNode *branch;
        ASTNode *then_instructions;
        ASTNode *then_block;
        ASTNode *guard;
        if (handler->condition_id == LEVELC_SIGNAL_SYNTAX_ID) continue;
        condition = ast_f(context, OP_COMPARE_EQUAL, handler->source->token);
        selected = rxcp_remap_create_named_ref(
            context, handler->source, VAR_SYMBOL,
            LEVELC_SIGNAL_SELECTED_SYMBOL);
        index = rxcp_remap_create_integer_constant(
            context, handler->source, (int)i + 1, TP_INTEGER);
        branch = levelc_frame_node(context, handler->source, FRAME_BRANCH,
                                   handler->trampoline);
        then_instructions = rxcp_remap_create_instruction_builder(
            context, handler->source);
        if (!condition || !selected || !index || !branch ||
            !then_instructions) return 0;
        rxcp_remap_anchor_synthetic(condition, handler->source);
        add_ast(condition, selected);
        add_ast(condition, index);
        add_ast(then_instructions, branch);
        then_block = rxcp_remap_create_do_block(
            context, handler->source, then_instructions);
        guard = then_block ? rxcp_remap_create_if_statement(
            context, handler->source, condition, then_block, NULL) : NULL;
        if (!guard) return 0;
        add_ast(instructions, guard);
    }

    /* A malformed or disabled typed event must not loop back into this handler. */
    {
        ASTNode *off = levelc_signal_handler_node(
            context, source, FRAME_HANDLER_OFF, "CLASSIC_CONDITION", NULL);
        ASTNode *signal = ast_ftt(context, ASSEMBLER, strdup("signal"));
        if (!off || !signal) return 0;
        signal->free_node_string = 1;
        signal->is_compiler_added = 1;
        rxcp_remap_anchor_synthetic(signal, source);
        add_ast(signal, rxcp_remap_create_string_constant(
            context, source, "INVALID_ARGUMENTS"));
        add_ast(signal, rxcp_remap_create_string_constant(
            context, source, "Unmatched Classic condition event"));
        add_ast(instructions, off);
        add_ast(instructions, signal);
    }
    return 1;
}

static int levelc_insert_inherited_signal_handlers(Context *context,
                                                     ASTNode *instructions,
                                                     ASTNode *event_setup,
                                                     LevelCLowerPlan *plan) {
    ASTNode *builder = rxcp_remap_create_instruction_builder(context, event_setup);
    ASTNode *first;
    ASTNode *tail;
    ASTNode *next;
    ASTNode *node;
    size_t i;
    if (!builder) return 0;
    if (plan->classic_condition_dispatch) {
        ASTNode *on = levelc_signal_handler_node(
            context, plan->classic_condition_source, FRAME_HANDLER_ON,
            "CLASSIC_CONDITION", plan->classic_condition_dispatch);
        ASTNode *binding = on ? rxcp_remap_create_named_ref(
            context, plan->classic_condition_source, VAR_TARGET,
            LEVELC_SIGNAL_EVENT_SYMBOL) : NULL;
        if (!on || !binding) return 0;
        add_ast(on, binding);
        add_ast(builder, on);
    }
    for (i = 0; i < plan->signal_handler_count; i++) {
        LevelCSignalHandler *handler = &plan->signal_handlers[i];
        if (handler->condition_id != LEVELC_SIGNAL_SYNTAX_ID) continue;
        ASTNode *receiver = rxcp_remap_create_named_ref(
            context, handler->source, VAR_SYMBOL, LEVELC_ACTIVATION_SYMBOL);
        ASTNode *args[1] = { rxcp_remap_create_integer_constant(
            context, handler->source, handler->condition_id, TP_INTEGER) };
        ASTNode *selected = receiver && args[0]
            ? rxcp_remap_create_member_call(
                context, handler->source, receiver, "signalPolicy", args, 1)
            : NULL;
        ASTNode *condition = ast_f(context, OP_COMPARE_EQUAL,
                                    handler->source->token);
        ASTNode *on = levelc_signal_handler_node(
            context, handler->source, FRAME_HANDLER_ON,
            handler->condition, handler->trampoline);
        ASTNode *binding = on ? rxcp_remap_create_named_ref(
            context, handler->source, VAR_TARGET,
            LEVELC_SIGNAL_EVENT_SYMBOL) : NULL;
        ASTNode *then_instructions = rxcp_remap_create_instruction_builder(
            context, handler->source);
        ASTNode *then_block;
        ASTNode *branch;
        if (!selected || !condition || !on || !binding || !then_instructions)
            return 0;
        rxcp_remap_anchor_synthetic(condition, handler->source);
        add_ast(condition, selected);
        add_ast(condition, rxcp_remap_create_integer_constant(
            context, handler->source, (int)i + 1, TP_INTEGER));
        add_ast(on, binding);
        add_ast(then_instructions, on);
        then_block = rxcp_remap_create_do_block(
            context, handler->source, then_instructions);
        branch = then_block ? rxcp_remap_create_if_statement(
            context, handler->source, condition, then_block, NULL) : NULL;
        if (!branch) return 0;
        add_ast(builder, branch);
    }

    first = builder->child;
    if (!first) return 1;
    tail = first;
    while (tail->sibling) tail = tail->sibling;
    next = event_setup->sibling;
    tail->sibling = next;
    event_setup->sibling = first;
    for (node = first; node != next; node = node->sibling)
        node->parent = instructions;
    builder->child = NULL;
    return 1;
}

static int levelc_append_main_activation(Context *context,
                                         ASTNode *instructions,
                                         ASTNode *anchor) {
    char *index_name;
    ASTNode *factory;
    ASTNode *assignment;
    ASTNode *body;
    ASTNode *receiver;
    ASTNode *source;
    ASTNode *count;
    ASTNode *args[1];
    ASTNode *append;
    ASTNode *index_initial;
    ASTNode *next_index;
    ASTNode *index_increment;
    ASTNode *loop;

    index_name = rxcp_remap_create_generated_node_name(
        LEVELC_MAIN_ARG_INDEX_PREFIX, anchor);
    factory = rxcp_remap_create_factory_call(context, anchor,
                                             LEVELC_ACTIVATION_CLASS, NULL, 0);
    assignment = factory ? rxcp_remap_create_named_assignment(
        context, anchor, LEVELC_ACTIVATION_SYMBOL, factory) : NULL;
    body = rxcp_remap_create_instruction_builder(context, anchor);
    receiver = rxcp_remap_create_named_ref(context, anchor, VAR_SYMBOL,
                                            LEVELC_ACTIVATION_SYMBOL);
    source = ast_ftt(context, OP_ARG_VALUE, strdup("arg"));
    count = ast_ftt(context, OP_ARGS, strdup("arg"));
    if (source) {
        source->free_node_string = 1;
        rxcp_remap_anchor_synthetic(source, anchor);
        if (index_name) add_ast(source, rxcp_remap_create_named_ref(
            context, anchor, VAR_SYMBOL, index_name));
    }
    if (count) {
        count->free_node_string = 1;
        rxcp_remap_anchor_synthetic(count, anchor);
    }
    args[0] = source;
    append = receiver && source
        ? rxcp_remap_create_member_call_statement(context, anchor, receiver,
                                                   "appendText", args, 1)
        : NULL;
    index_initial = index_name ? rxcp_remap_create_named_assignment(
        context, anchor, index_name,
        rxcp_remap_create_integer_constant(context, anchor, 1, TP_INTEGER))
        : NULL;
    next_index = ast_f(context, OP_ADD, anchor->token);
    if (next_index && index_name) {
        rxcp_remap_anchor_synthetic(next_index, anchor);
        add_ast(next_index, rxcp_remap_create_named_ref(
            context, anchor, VAR_SYMBOL, index_name));
        add_ast(next_index, rxcp_remap_create_integer_constant(
            context, anchor, 1, TP_INTEGER));
    }
    index_increment = index_name && next_index
        ? rxcp_remap_create_named_assignment(context, anchor,
                                              index_name, next_index)
        : NULL;
    if (body && append && index_increment) {
        add_ast(body, append);
        add_ast(body, index_increment);
    }
    loop = index_initial && body && append && index_increment && count
        ? rxcp_remap_create_do_with_count(context, anchor, body, count)
        : NULL;
    free(index_name);
    if (!assignment || !index_initial || !loop) return 0;
    add_ast(instructions, assignment);
    add_ast(instructions, index_initial);
    add_ast(instructions, loop);
    return 1;
}

static ASTNode *levelc_frame_node(Context *context, ASTNode *source,
                                  NodeType type, ASTNode *target) {
    ASTNode *node = ast_f(context, type, source ? source->token : NULL);
    if (!node) return NULL;
    if (source) rxcp_remap_anchor_synthetic(node, source);
    node->association = target;
    return node;
}

static int levelc_append_entry_dispatch(Context *context,
                                        ASTNode *instructions,
                                        LevelCLowerPlan *plan) {
    size_t i;
    for (i = 0; i < plan->procedure_count; i++) {
        ASTNode *source = plan->procedures[i].label;
        ASTNode *condition = ast_f(context, OP_COMPARE_EQUAL, source->token);
        ASTNode *branch = levelc_frame_node(context, source, FRAME_BRANCH,
                                             plan->procedures[i].frame_label);
        ASTNode *then_instructions = rxcp_remap_create_instruction_builder(context, source);
        ASTNode *then_block;
        ASTNode *if_statement;
        if (!condition || !branch || !then_instructions || i >= INT_MAX) return 0;
        rxcp_remap_anchor_synthetic(condition, source);
        add_ast(condition, rxcp_remap_create_named_ref(
            context, source, VAR_SYMBOL, LEVELC_ENTRY_SYMBOL));
        add_ast(condition, rxcp_remap_create_integer_constant(
            context, source, (int)i + 1, TP_INTEGER));
        add_ast(then_instructions, branch);
        then_block = rxcp_remap_create_do_block(context, source, then_instructions);
        if_statement = then_block ? rxcp_remap_create_if_statement(
            context, source, condition, then_block, NULL) : NULL;
        if (!if_statement) return 0;
        add_ast(instructions, if_statement);
    }
    return 1;
}

static ASTNode *levelc_inherited_pool_statement(Context *context,
                                                 ASTNode *anchor) {
    ASTNode *parent_ref = levelc_parent_pool_ref_symbol(context, anchor, VAR_SYMBOL);
    ASTNode *parent = parent_ref ? rxcp_remap_create_dereference_expr(
        context, anchor, parent_ref) : NULL;
    return parent ? rxcp_remap_create_named_assignment(
        context, anchor, LEVELC_POOL_SYMBOL, parent) : NULL;
}

static int levelc_append_private_pool_transition(Context *context,
                                                 ASTNode *instructions,
                                                 ASTNode *procedure_node) {
    char *private_name = rxcp_remap_create_generated_node_name(
        LEVELC_PRIVATE_POOL_PREFIX, procedure_node);
    ASTNode *factory = rxcp_remap_create_factory_call(
        context, procedure_node, "RexxVariablePool", NULL, 0);
    ASTNode *private_setup = private_name && factory
        ? rxcp_remap_create_named_assignment(context, procedure_node,
                                              private_name, factory) : NULL;
    ASTNode *private_symbol = private_name
        ? rxcp_remap_create_named_ref(context, procedure_node,
                                      VAR_SYMBOL, private_name) : NULL;
    ASTNode *private_reference = private_symbol
        ? rxcp_remap_create_reference_expr(context, procedure_node,
                                            private_symbol) : NULL;
    ASTNode *private_value = private_reference
        ? rxcp_remap_create_dereference_expr(context, procedure_node,
                                              private_reference) : NULL;
    ASTNode *activate = private_value
        ? rxcp_remap_create_named_assignment(context, procedure_node,
                                              LEVELC_POOL_SYMBOL, private_value) : NULL;
    free(private_name);
    if (!private_setup || !activate) return 0;
    add_ast(instructions, private_setup);
    add_ast(instructions, activate);
    return 1;
}

static ASTNode *levelc_main_body_call(Context *context, ASTNode *anchor) {
    ASTNode *args[4];
    ASTNode *pool = levelc_pool_ref(context, anchor, VAR_SYMBOL);
    ASTNode *call;
    args[0] = pool ? rxcp_remap_create_reference_expr(context, anchor, pool) : NULL;
    args[1] = levelc_config_ref(context, anchor, VAR_SYMBOL);
    args[2] = rxcp_remap_create_named_ref(context, anchor, VAR_SYMBOL,
                                          LEVELC_ACTIVATION_SYMBOL);
    args[3] = rxcp_remap_create_integer_constant(context, anchor, 0, TP_INTEGER);
    if (!args[0] || !args[1] || !args[2] || !args[3]) return NULL;
    call = rxcp_remap_create_function_call(context, anchor, LEVELC_BODY_NAME,
                                           args, 4);
    return call ? rxcp_remap_create_call_statement(context, anchor, call) : NULL;
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
    ASTNode *config_setup;
    ASTNode *config_ref_setup;
    ASTNode *body_header;
    ASTNode *body_args;
    ASTNode *parent_setup;
    ASTNode *inherited_pool;
    ASTNode *event_setup = NULL;
    ASTNode *body_call;
    ASTNode *return_stmt;
    ASTNode *stmt;
    size_t i;
    int needs_translate = 0;
    int needs_signal_policy = 0;
    int has_procedure;

    anchor = old_instructions && old_instructions->child ? old_instructions->child : program_file;
    needs_translate = levelc_tree_contains_parse_upper(old_instructions) ||
                      levelc_tree_contains_arg(old_instructions) ||
                      levelc_tree_contains_signal_value(old_instructions);
    needs_signal_policy = levelc_tree_contains_signal_on(old_instructions);
    if (plan) plan->has_novalue_on = levelc_tree_contains_novalue_on(old_instructions);
    has_procedure = levelc_tree_contains_procedure(old_instructions);
    options = levelc_build_options(context, anchor, needs_translate,
                                   needs_signal_policy, plan);
    instructions = rxcp_remap_create_instruction_builder(context, anchor);
    if (!options || !instructions) {
        if (reason_out) *reason_out = "failed to create Level C lowered program shell";
        return 0;
    }

    pool_setup = levelc_pool_setup_statement(context, anchor);
    config_setup = levelc_config_setup_statement(context, anchor, 0);
    config_ref_setup = levelc_config_setup_statement(context, anchor, 1);
    if (!pool_setup || !config_setup || !config_ref_setup) {
        if (reason_out) *reason_out = "failed to create Level C pool setup";
        return 0;
    }
    add_ast(instructions, pool_setup);
    add_ast(instructions, config_setup);
    add_ast(instructions, config_ref_setup);
    /* Only this wrapper reads the VM's hidden command-line argv. */
    if (!levelc_append_main_activation(context, instructions, anchor)) {
        if (reason_out) *reason_out = "failed to capture main ARG activation";
        return 0;
    }
    body_call = levelc_main_body_call(context, anchor);
    return_stmt = rxcp_remap_create_return_statement(context, anchor);
    if (!body_call || !return_stmt) {
        if (reason_out) *reason_out = "failed to create Level C main body call";
        return 0;
    }
    add_ast(instructions, body_call);
    add_ast(instructions, return_stmt);

    body_header = levelc_body_header(context, anchor);
    body_args = levelc_body_args(context, anchor);
    parent_setup = levelc_parent_pool_setup_statement(context, anchor);
    inherited_pool = levelc_inherited_pool_statement(context, anchor);
    if (!body_header || !body_args || !parent_setup || !inherited_pool) {
        if (reason_out) *reason_out = "failed to create Level C callable body";
        return 0;
    }
    add_ast(instructions, body_header);
    add_ast(instructions, body_args);
    add_ast(instructions, parent_setup);
    add_ast(instructions, inherited_pool);
    if (needs_signal_policy) {
        ASTNode *event_args[1] = {
            rxcp_remap_create_string_constant(context, anchor, "")
        };
        ASTNode *factory = rxcp_remap_create_factory_call(
            context, anchor, "runtime_signal", event_args, 1);
        event_setup = factory ? rxcp_remap_create_named_assignment(
            context, anchor, LEVELC_SIGNAL_EVENT_SYMBOL, factory) : NULL;
        if (!event_setup) {
            if (reason_out) *reason_out = "failed to create SIGNAL event storage";
            return 0;
        }
        add_ast(instructions, event_setup);
    }
    for (i = 0; plan && i < plan->procedure_count; i++) {
        plan->procedures[i].frame_label = levelc_frame_node(
            context, plan->procedures[i].label, FRAME_LABEL, NULL);
        if (!plan->procedures[i].frame_label) {
            if (reason_out) *reason_out = "failed to create local frame label";
            return 0;
        }
    }
    if (plan && !levelc_append_entry_dispatch(context, instructions, plan)) {
        if (reason_out) *reason_out = "failed to create local entry dispatch";
        return 0;
    }

    stmt = plan ? plan->main_first : old_instructions->child;
    while (stmt && (!plan || stmt != plan->main_end)) {
        if (!levelc_lower_main_statement(context,
                                         instructions,
                                         stmt,
                                         plan,
                                         reason_out)) {
            return 0;
        }
        stmt = stmt->sibling;
    }

    if (plan) {
        for (i = 0; i < plan->procedure_count; i++) {
            add_ast(instructions, plan->procedures[i].frame_label);

            stmt = plan->procedures[i].body_first;
            if (has_procedure && stmt && stmt != plan->procedures[i].body_end &&
                stmt->node_type != LEVELC_PROCEDURE &&
                !levelc_append_activation_method(context, instructions, stmt,
                                                  "consumeFirstInstruction")) {
                if (reason_out) *reason_out = "failed to consume PROCEDURE entry window";
                return 0;
            }
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

    return_stmt = rxcp_remap_create_return_statement(context, anchor);
    if (!return_stmt) return 0;
    add_ast(instructions, return_stmt);
    if (plan && !levelc_append_signal_trampolines(context, instructions,
                                                  plan, reason_out)) return 0;
    if (plan && !levelc_append_classic_condition_dispatch(
            context, instructions, plan)) {
        if (reason_out) *reason_out = "failed to dispatch Classic condition event";
        return 0;
    }
    if (plan && event_setup && !levelc_insert_inherited_signal_handlers(
            context, instructions, event_setup, plan)) {
        if (reason_out) *reason_out = "failed to restore inherited SIGNAL handlers";
        return 0;
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
