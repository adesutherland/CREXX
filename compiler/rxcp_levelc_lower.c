/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

/**
 * Level C Classic REXX lowering tracer.
 *
 * Slice 1 deliberately accepts only direct scalar assignment/read, string and
 * integer literals, binary addition, and SAY. Everything else remains behind
 * the existing unsupported compile gate.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "rxcp_levelc_lower.h"
#include "rxcp_remap_build.h"
#include "rxcpcsym.h"

#define LEVELC_POOL_SYMBOL "__rxcp_levelc_pool"

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

static int levelc_statement_supported(ASTNode *stmt, const char **reason_out) {
    ASTNode *target;
    ASTNode *expr;

    if (!stmt) return 1;

    if (stmt->node_type == REXX_OPTIONS) return 1;

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

static int levelc_accept_slice1(ASTNode *instructions, const char **reason_out) {
    ASTNode *stmt;
    int accepted_statement;

    if (!instructions || instructions->node_type != INSTRUCTIONS) {
        if (reason_out) *reason_out = "missing top-level instruction list";
        return 0;
    }

    accepted_statement = 0;
    stmt = instructions->child;
    while (stmt) {
        if (!levelc_statement_supported(stmt, reason_out)) return 0;
        if (stmt->node_type != REXX_OPTIONS) accepted_statement = 1;
        stmt = stmt->sibling;
    }

    if (!accepted_statement) {
        if (reason_out) *reason_out = "no supported executable Level C statements";
        return 0;
    }

    return 1;
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

static ASTNode *levelc_pool_ref(Context *context, ASTNode *source_node, NodeType node_type) {
    ASTNode *node;

    node = ast_ftt(context, node_type, strdup(LEVELC_POOL_SYMBOL));
    if (!node) return NULL;
    node->free_node_string = 1;
    rxcp_remap_anchor_synthetic(node, source_node);
    return node;
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

static ASTNode *levelc_lower_statement(Context *context, ASTNode *stmt) {
    if (!stmt) return NULL;

    if (stmt->node_type == ASSIGN) return levelc_pool_set_statement(context, stmt);
    if (stmt->node_type == SAY) return levelc_say_statement(context, stmt);
    return NULL;
}

static int levelc_rewrite_program(Context *context,
                                  ASTNode *program_file,
                                  ASTNode *old_instructions,
                                  const char **reason_out) {
    ASTNode *anchor;
    ASTNode *options;
    ASTNode *instructions;
    ASTNode *pool_setup;
    ASTNode *stmt;

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

    stmt = old_instructions->child;
    while (stmt) {
        ASTNode *lowered;

        if (stmt->node_type != REXX_OPTIONS) {
            lowered = levelc_lower_statement(context, stmt);
            if (!lowered) {
                if (reason_out) *reason_out = "failed to lower supported Level C statement";
                return 0;
            }
            add_ast(instructions, lowered);
        }
        stmt = stmt->sibling;
    }

    old_instructions->parent = NULL;
    old_instructions->sibling = NULL;

    program_file->child = options;
    options->parent = program_file;
    options->sibling = instructions;
    instructions->parent = program_file;
    instructions->sibling = NULL;

    context->level = LEVELB;
    context->changed_flags |= FLAG_VAL_TRANS;
    return 1;
}

int rxcp_levelc_lower_slice1(Context *context, const char **reason_out) {
    ASTNode *program_file;
    ASTNode *instructions;

    if (reason_out) *reason_out = NULL;
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

    if (!levelc_accept_slice1(instructions, reason_out)) return 0;
    return levelc_rewrite_program(context, program_file, instructions, reason_out);
}
