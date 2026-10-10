/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#include <stdlib.h>
#include <string.h>
#include "rxcpmain.h"
#include "rxcp_contract.h"
#include "rxcp_val.h"
#include "rxcontract.h"
#include "rxcp_source_tree.h"

/* This adapter creates canonical declarations from independent metadata
 * records. No producer-language source is manufactured or parsed. */
static ASTNode *contract_node(Context *c, NodeType kind) {
    ASTNode *n = ast_ft(c, kind);
    n->is_metadata_declaration = 1;
    return n;
}
static ASTNode *text_node(Context *c, NodeType kind, const char *text) {
    ASTNode *n = contract_node(c, kind);
    ast_copy_str(n, (char *)text);
    return n;
}
static ASTNode *bound_node(Context *c, const char *text) {
    ASTNode *n;
    if (!text) return contract_node(c, NOVAL);
    if (*text != '-') return text_node(c, INTEGER, text);
    n = contract_node(c, OP_NEG);
    add_ast(n, text_node(c, INTEGER, text + 1));
    return n;
}
static ASTNode *type_node(Context *c, const rx_contract_type *type) {
    ASTNode *n;
    size_t i;
    if (type->referent) {
        n = contract_node(c, TYPE_REFERENCE);
        add_ast(n, type_node(c, type->referent));
        return n;
    }
    n = text_node(c, CLASS, type->name);
    if (nodeis(n, ".void")) n->node_type = VOID;
    for (i = 0; i < type->dimensions; ++i) {
        ASTNode *range = contract_node(c, RANGE);
        add_ast(range, bound_node(c, type->ranges[i].lower));
        add_ast(range, bound_node(c, type->ranges[i].upper));
        add_ast(n, range);
    }
    return n;
}
ASTNode *rxcp_contract_type(Context *c, const char *text) {
    rx_contract_type record;
    ASTNode *n;
    if (!text || !rx_contract_parse_type(text, strlen(text), &record, NULL)) return NULL;
    n = type_node(c, &record);
    rx_contract_type_free(&record);
    return n;
}
Context *rxcp_contract_context(Context *parent, const char *file, const char *ns) {
    Context *c = cntx_f();
    ASTNode *options, *namespace_node;
    c->master_context = parent->master_context ? parent->master_context : parent;
    c->location = parent->location;
    c->file_name = (char *)file;
    /* Canonical typed declarations use common validation; this is not the
     * producing language's source level or a source-parser selection. */
    c->level = LEVELB;
    c->debug_mode = parent->debug_mode;
    c->decimal_plugin = parent->decimal_plugin;
    c->disable_exits = parent->disable_exits;
    /* Class contracts are forward declarations until the module registry is
     * complete. Standalone callable imports explicitly select attachment below. */
    c->defer_imported_inline_attachments = 1;
    c->ast = contract_node(c, REXX_UNIVERSE);
    add_ast(c->ast, contract_node(c, PROGRAM_FILE));
    options = contract_node(c, REXX_OPTIONS);
    add_ast(options, text_node(c, LITERAL, "levelb"));
    namespace_node = contract_node(c, NAMESPACE);
    add_ast(namespace_node, text_node(c, LITERAL, ns));
    add_ast(options, namespace_node);
    add_ast(c->ast->child, options);
    add_ast(c->ast->child, contract_node(c, INSTRUCTIONS));
    return c;
}
ASTNode *rxcp_contract_class(Context *c, NodeType kind, const char *ns, const char *name) {
    rx_contract_class_header record;
    ASTNode *n;
    if ((kind != CLASS_DEF && kind != INTERFACE_DEF) ||
        !rx_contract_parse_class_header(ns, name, kind == INTERFACE_DEF, &record, NULL)) return NULL;
    n = text_node(c, kind, record.name);
    rx_contract_class_header_free(&record);
    return n;
}

/* Source providers can supply real declaration anchors. Copy the line into
 * the contract context; borrowing the provider's buffer would couple lifetimes. */
void rxcp_contract_source_anchor(ASTNode *n, ASTNode *source) {
    Context *c = n->context;
    const char *start = source->source_start, *end = source->source_end;
    const char *line_start, *line_end;
    SourceNode *anchor;
    size_t length;
    if (!start || !end || source->line < 0 || !source->context->buff_start ||
        start < source->context->buff_start || start >= source->context->buff_end) return;
    line_start = start;
    while (line_start > source->context->buff_start && line_start[-1] != '\n' && line_start[-1] != '\r') --line_start;
    line_end = start;
    while (line_end < source->context->buff_end && *line_end != '\n' && *line_end != '\r') ++line_end;
    length = (size_t)(line_end - line_start);
    anchor = calloc(1, sizeof(*anchor));
    if (!anchor) return;
    anchor->owned_file_name = strdup(source->file_name ? source->file_name : c->file_name);
    anchor->owned_source_text = malloc(length + 1);
    if (!anchor->owned_file_name || !anchor->owned_source_text) {
        free(anchor->owned_file_name); free(anchor->owned_source_text); free(anchor); return;
    }
    memcpy(anchor->owned_source_text, line_start, length); anchor->owned_source_text[length] = 0;
    anchor->owned_source_text_length = length;
    anchor->is_metadata_anchor = 1;
    anchor->context = c; anchor->node_type = n->node_type;
    anchor->file_name = anchor->owned_file_name;
    anchor->line = source->line; anchor->column = source->column;
    anchor->source_start = anchor->owned_source_text + (start - line_start);
    anchor->source_end = anchor->owned_source_text + ((end < line_end ? end : line_end - 1) - line_start);
    anchor->free_list = c->source_free_list;
    anchor->node_number = anchor->free_list ? anchor->free_list->node_number + 1 : 1;
    c->source_free_list = anchor;
    n->file_name = anchor->file_name; n->line = anchor->line; n->column = anchor->column;
    n->source_start = anchor->source_start; n->source_end = anchor->source_end;
    ast_set_primary_source_node(n, anchor, AST_SOURCE_EXACT);
}

void rxcp_contract_copy_anchor(ASTNode *n, const ASTNode *source) {
    SourceNode *old = source->source_node, *anchor;
    Context *c = n->context;
    size_t length;
    ASTSourceProvenance provenance = (ASTSourceProvenance)n->source_provenance;
    if (!old || !old->is_metadata_anchor || old->context == c) return;
    /* Metadata anchors have owned text/file identities. Each receiving
     * Context must own its copy; class registries can die in either order. */
    ast_set_primary_source_node(n, NULL, AST_SOURCE_NONE);
    n->source_start = n->source_end = NULL;
    n->file_name = c->file_name; n->line = n->column = -1;
    if (!old->owned_source_text) return;
    length = old->owned_source_text_length;
    anchor = calloc(1, sizeof(*anchor));
    if (!anchor) { mknd_err(n, "OUT_OF_MEMORY"); return; }
    anchor->owned_file_name = strdup(old->file_name ? old->file_name : c->file_name);
    anchor->owned_source_text = malloc(length + 1);
    if (!anchor->owned_file_name || !anchor->owned_source_text) {
        free(anchor->owned_file_name); free(anchor->owned_source_text); free(anchor);
        mknd_err(n, "OUT_OF_MEMORY"); return;
    }
    memcpy(anchor->owned_source_text, old->owned_source_text, length + 1);
    anchor->owned_source_text_length = length;
    anchor->is_metadata_anchor = 1;
    anchor->context = c; anchor->node_type = old->node_type;
    anchor->file_name = anchor->owned_file_name;
    anchor->line = old->line; anchor->column = old->column;
    anchor->source_start = anchor->owned_source_text + (old->source_start - old->owned_source_text);
    anchor->source_end = anchor->owned_source_text + (old->source_end - old->owned_source_text);
    anchor->free_list = c->source_free_list;
    anchor->node_number = anchor->free_list ? anchor->free_list->node_number + 1 : 1;
    c->source_free_list = anchor;
    n->file_name = anchor->file_name; n->line = anchor->line; n->column = anchor->column;
    n->source_start = anchor->source_start; n->source_end = anchor->source_end;
    ast_set_primary_source_node(n, anchor, provenance);
}
ASTNode *rxcp_contract_callable(Context *c, NodeType kind, const char *name,
                              const char *result, const char *args, int task) {
    rx_contract_callable record;
    rx_contract_callable_kind record_kind;
    ASTNode *n, *arguments;
    size_t i;
    if (kind == PROCEDURE) record_kind = RX_CONTRACT_PROCEDURE;
    else if (kind == METHOD) record_kind = RX_CONTRACT_METHOD;
    else if (kind == FACTORY) record_kind = RX_CONTRACT_FACTORY;
    else return NULL;
    if (!rx_contract_parse_callable(record_kind, name, result, args, task, &record, NULL)) return NULL;
    n = text_node(c, kind, record.name);
    n->is_task_callable = record.task;
    add_ast(n, type_node(c, &record.result));
    arguments = contract_node(c, ARGS);
    for (i = 0; i < record.arg_count; ++i) {
        rx_contract_arg *a = &record.args[i];
        ASTNode *arg = contract_node(c, ARG);
        ASTNode *shape;
        arg->is_ref_arg = a->by_reference; arg->is_opt_arg = a->optional; arg->is_varg = a->vararg;
        add_ast(arg, text_node(c, a->vararg ? (a->by_reference ? VARG_REFERENCE : VARG) :
                              (a->by_reference ? VAR_REFERENCE : VAR_TARGET), a->name));
        if (a->default_kind == RX_CONTRACT_INTEGER_DEFAULT) {
            shape = text_node(c, INTEGER, a->default_literal);
            shape->value_type = TP_INTEGER;
        } else shape = type_node(c, &a->type);
        add_ast(arg, shape);
        add_ast(arguments, arg);
    }
    rx_contract_callable_free(&record);
    /* Shared structural validation attaches the following ARG instruction. */
    add_sbtr(n, arguments);
    return n;
}
ASTNode *rxcp_contract_attribute(Context *c, const char *name, const char *type,
                               int reg, const char *view) {
    rx_contract_attribute record;
    ASTNode *n;
    if (!rx_contract_parse_attribute(name, type, reg, view, &record, NULL)) return NULL;
    n = contract_node(c, DEFINE);
    add_ast(n, text_node(c, VAR_TARGET, record.name));
    add_ast(n, type_node(c, &record.type));
    if (record.register_index >= 0) {
        ASTNode *mapping = contract_node(c, NODE_REGISTER);
        char number[32];
        snprintf(number, sizeof(number), "%d", record.register_index);
        add_ast(mapping, text_node(c, INTEGER, number));
        if (record.view) add_ast(mapping, text_node(c, VAR_SYMBOL, record.view));
        add_ast(n, mapping);
    }
    rx_contract_attribute_free(&record);
    return n;
}
void rxcp_contract_finish(Context *context) { rxcp_bvl(context); }
