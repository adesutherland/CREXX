/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
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
 * Validation Pipeline Orchestrator
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "rxcp_val.h"
#include "rxcpdary.h"
#include "rxcp_source_tree.h"
#include "rxcp_emit.h"
#include "rxcp_util.h"
#include "rxcp_task_lower.h"
#include "rxcp_remap_build.h"

/* Suppress errors and warnings unless it is the final pass */
#undef mknd_err
#undef mknd_err_unique
#undef mknd_war
#define mknd_err(n, ...) ((!(context) || (context)->is_final_pass) ? (mknd_err)((n), __VA_ARGS__) : (n))
#define mknd_err1(n, ...) ((!(context) || (context)->is_final_pass) ? (mknd_err1)((n), __VA_ARGS__) : (n))
#define mknd_err2(n, ...) ((!(context) || (context)->is_final_pass) ? (mknd_err2)((n), __VA_ARGS__) : (n))
#define mknd_err3(n, ...) ((!(context) || (context)->is_final_pass) ? (mknd_err3)((n), __VA_ARGS__) : (n))
#define mknd_err5(n, ...) ((!(context) || (context)->is_final_pass) ? (mknd_err5)((n), __VA_ARGS__) : (n))
#define mknd_err_unique(n, ...) ((!(context) || (context)->is_final_pass) ? (mknd_err_unique)((n), __VA_ARGS__) : (n))
#define mknd_err_unique1(n, ...) ((!(context) || (context)->is_final_pass) ? (mknd_err_unique1)((n), __VA_ARGS__) : (n))
#define mknd_err_unique2(n, ...) ((!(context) || (context)->is_final_pass) ? (mknd_err_unique2)((n), __VA_ARGS__) : (n))
#define mknd_war(n, ...) ((!(context) || (context)->is_final_pass) ? (mknd_war)((n), __VA_ARGS__) : (n))
#define mknd_war1(n, ...) ((!(context) || (context)->is_final_pass) ? (mknd_war1)((n), __VA_ARGS__) : (n))
#define mknd_war2(n, ...) ((!(context) || (context)->is_final_pass) ? (mknd_war2)((n), __VA_ARGS__) : (n))
#define mknd_war3(n, ...) ((!(context) || (context)->is_final_pass) ? (mknd_war3)((n), __VA_ARGS__) : (n))

static int rxcp_program_has_import(ASTNode *program_file, const char *namespace_name) {
    ASTNode *child;

    if (!program_file || !namespace_name) return 0;

    child = program_file->child;
    while (child) {
        if (child->node_type == IMPORT &&
            child->child &&
            is_node_string(child->child, namespace_name)) {
            return 1;
        }
        child = child->sibling;
    }

    return 0;
}

static void rxcp_insert_program_import(Context *context, ASTNode *program_file, const char *namespace_name) {
    ASTNode *import_node;
    ASTNode *literal_node;
    ASTNode *child;
    ASTNode *insert_after;

    if (!context || !program_file || !namespace_name || !namespace_name[0]) return;

    import_node = ast_ft(context, IMPORT);
    literal_node = ast_ft(context, LITERAL);
    ast_copy_str(literal_node, (char*)namespace_name);
    add_ast(import_node, literal_node);
    import_node->is_compiler_added = 1;
    literal_node->is_compiler_added = 1;

    insert_after = 0;
    child = program_file->child;
    while (child &&
           (child->node_type == REXX_OPTIONS ||
            child->node_type == IMPORT ||
            child->node_type == NAMESPACE)) {
        insert_after = child;
        child = child->sibling;
    }

    if (insert_after) {
        import_node->sibling = insert_after->sibling;
        insert_after->sibling = import_node;
    } else {
        import_node->sibling = program_file->child;
        program_file->child = import_node;
    }
    import_node->parent = program_file;
}

static int rxcp_subtree_needs_concurrency(ASTNode *node) {
    ASTNode *child;

    if (!node) return 0;
    if (node->is_task_callable || node->node_type == TASK_DECL ||
        node->node_type == TASK_TARGET || node->node_type == PARALLEL_DO ||
        node->node_type == PARALLEL_BLOCK_EXPR) return 1;
    for (child = node->child; child; child = child->sibling) {
        if (rxcp_subtree_needs_concurrency(child)) return 1;
    }
    return 0;
}

void rxcp_inject_concurrency_imports(Context *context) {
    ASTNode *program_file;

    if (!context || !context->ast) return;
    for (program_file = context->ast->child;
         program_file;
         program_file = program_file->sibling) {
        if (program_file->node_type == PROGRAM_FILE &&
            rxcp_subtree_needs_concurrency(program_file) &&
            !rxcp_program_has_import(program_file, "concurrency")) {
            rxcp_insert_program_import(context, program_file, "concurrency");
        }
    }
}

/*
 * Resolve the source-facing `task name` expression through the ordinary
 * callable symbol table, then lower it to the private Level B binding factory.
 * Keeping this after resolve_functions_walker is important: task targets obey
 * exactly the same namespace, import and shadowing rules as normal calls.
 */
static ASTNode *rxcp_task_callable_definition(Symbol *symbol) {
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

static ASTNode *rxcp_task_binding_argument(Context *context,
                                           NodeType type,
                                           const char *value,
                                           ASTNode *anchor) {
    ASTNode *argument;
    char *copy;

    if (!context || !value) return 0;
    copy = strdup(value);
    if (!copy) return 0;
    argument = ast_ftt(context, type, copy);
    if (!argument) {
        free(copy);
        return 0;
    }
    argument->free_node_string = 1;
    if (anchor) ast_copy_source_anchor(argument, anchor, AST_SOURCE_INHERITED);
    return argument;
}

static ASTNode *rxcp_task_factory_definition(Symbol *symbol) {
    size_t i;

    if (!symbol || symbol->symbol_type != FUNCTION_SYMBOL) return 0;
    for (i = 0; i < sym_nond(symbol); i++) {
        SymbolNode *link = sym_trnd(symbol, i);
        ASTNode *definition = link ? link->node : 0;
        if (definition && definition->node_type == FACTORY) return definition;
    }
    return 0;
}

static int rxcp_taskwork_factory_valid(Context *context,
                                       ASTNode *definition) {
    ASTNode *class_node;
    char *class_name;
    int valid;

    if (!context || !definition || definition->node_type != FACTORY ||
        !(class_node = definition->parent) ||
        class_node->node_type != CLASS_DEF ||
        !class_node->symbolNode || !class_node->symbolNode->symbol) {
        return 0;
    }
    class_name = sym_frnm(class_node->symbolNode->symbol);
    if (!class_name) return 0;
    valid = symbol_name_assignable_to(
            context, class_name, "concurrency.taskwork");
    free(class_name);
    return valid;
}

/* NULL means either not yet typed or not transferable; final-pass callers
 * distinguish those cases before issuing the stable transfer diagnostic. */
static const char *rxcp_task_factory_argument_method(Context *context,
                                                      ASTNode *argument) {
    if (!context || !argument || argument->value_dims ||
        argument->value_reference_type != TP_UNKNOWN) return 0;
    switch (argument->value_type) {
        case TP_BOOLEAN: return "add_boolean";
        case TP_INTEGER: return "add_integer";
        case TP_STRING: return "add_string";
        case TP_BINARY: return "add_binary";
        case TP_OBJECT:
            if (argument->value_class &&
                symbol_name_assignable_to(context, argument->value_class,
                                          "concurrency.channelvalue")) {
                return "add_value";
            }
            return 0;
        default:
            return 0;
    }
}

static ASTNode *rxcp_task_factory_argument_values(Context *context,
                                                   ASTNode *target,
                                                   ASTNode *factory) {
    ASTNode *builder;
    ASTNode *association;
    ASTNode *argument;

    builder = rxcp_remap_create_factory_call(
            context, target, "concurrency.taskarguments", 0, 0);
    association = rxcp_remap_create_named_ref(
            context, target, VAR_SYMBOL, "empty");
    if (!builder || !association) return 0;
    builder->association = association;
    builder->scope = target->scope;

    while ((argument = factory->child) != 0) {
        const char *method;
        ASTNode *arguments[1];
        ASTNode *next;

        if (argument->node_type == NOVAL) {
            ast_del(argument);
            continue;
        }
        method = rxcp_task_factory_argument_method(context, argument);
        if (!method) return 0;
        ast_del(argument);
        arguments[0] = argument;
        next = rxcp_remap_create_member_call(
                context, target, builder, method, arguments, 1u);
        if (!next) return 0;
        next->scope = target->scope;
        builder = next;
    }
    builder = rxcp_remap_create_member_call(
            context, target, builder, "values", 0, 0);
    if (builder) builder->scope = target->scope;
    return builder;
}

walker_result rxcp_task_target_lowering_walker(walker_direction direction,
                                                       ASTNode *node,
                                                       void *payload) {
    Context *context = (Context *)payload;
    ASTNode *reference;
    ASTNode *definition;
    ASTNode *name_argument;
    ASTNode *signature_argument;
    ASTNode *binding_argument;
    ASTNode *factory_arguments = 0;
    ASTNode *association;
    Symbol *symbol;
    char *fqn;
    char *return_type;
    char *arguments;
    char *signature;
    char *placeholder;

    if (direction != out || !node || node->node_type != TASK_TARGET) {
        return result_normal;
    }
    if (ast_chld(node, ERROR, 0)) return result_normal;

    reference = node->child;
    if (!reference) return result_normal;
    if (reference->node_type == FACTORY_CALL) {
        ASTNode *argument;

        symbol = reference->symbolNode ? reference->symbolNode->symbol : 0;
        definition = rxcp_task_factory_definition(symbol);
        if (!definition || !rxcp_taskwork_factory_valid(context, definition)) {
            if (context->is_final_pass && !ast_chld(node, ERROR, 0)) {
                mknd_err(node, "TASKWORK_ADAPTER_REQUIRED");
            }
            return result_normal;
        }
        for (argument = reference->child; argument; argument = argument->sibling) {
            if (argument->node_type == NOVAL) continue;
            if (argument->value_type == TP_UNKNOWN) return result_normal;
            if (!rxcp_task_factory_argument_method(context, argument)) {
                if (context->is_final_pass && !ast_chld(node, ERROR, 0)) {
                    mknd_err(node, "TASK_NONTRANSFERABLE_TYPE");
                }
                return result_normal;
            }
        }
        factory_arguments = rxcp_task_factory_argument_values(
                context, node, reference);
        if (!factory_arguments) {
            if (context->is_final_pass && !ast_chld(node, ERROR, 0)) {
                mknd_err(node, "OUT_OF_MEMORY");
            }
            return result_normal;
        }
    } else if (reference->node_type == FUNC_SYMBOL) {
        symbol = reference->symbolNode ? reference->symbolNode->symbol : 0;
        definition = rxcp_task_callable_definition(symbol);
        if (!definition) {
            if (context->is_final_pass && !ast_chld(node, ERROR, 0)) {
                mknd_err(node, "TASK_TARGET_REQUIRED");
            }
            return result_normal;
        }
    } else {
        if (context->is_final_pass && !ast_chld(node, ERROR, 0)) {
            mknd_err(node, "TASK_DYNAMIC_TARGET");
        }
        return result_normal;
    }

    fqn = sym_frnm(symbol);
    return_type = callable_effective_return_type(definition);
    arguments = meta_narg(ast_chld(definition, ARGS, 0));
    signature = mprintf("%s %s(%s)", return_type, fqn, arguments);
    placeholder = rxcp_task_placeholder_hex(fqn);
    name_argument = rxcp_task_binding_argument(context, STRING, fqn, node);
    signature_argument = rxcp_task_binding_argument(context, STRING, signature, node);
    binding_argument = rxcp_task_binding_argument(context, BINARY, placeholder, node);
    association = rxcp_task_binding_argument(context, VAR_SYMBOL, "binding", node);

    free(return_type);
    free(arguments);
    free(signature);
    free(placeholder);
    free(fqn);

    if (!name_argument || !signature_argument || !binding_argument || !association) {
        mknd_err(node, "OUT_OF_MEMORY");
        return result_normal;
    }

    if (reference->symbolNode) sym_dno(reference->symbolNode->symbol, reference);
    while (node->child) ast_del(node->child);

    node->node_type = FACTORY_CALL;
    ast_sstr(node, strdup("concurrency.tasktarget"),
             strlen("concurrency.tasktarget"));
    node->association = association;
    add_ast(node, name_argument);
    add_ast(node, signature_argument);
    add_ast(node, binding_argument);
    if (factory_arguments) add_ast(node, factory_arguments);
    context->changed_flags |= FLAG_VAL_SYM | FLAG_VAL_TYPE | FLAG_ORCH;
    return result_normal;
}

/* Convert a node (i.e. type INTEGER) to an integer - no error correction as the lexer will have done that */
