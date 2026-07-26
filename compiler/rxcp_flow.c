/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 *
 * Procedure-local typed control/data-flow analysis for rxc. The overlay is
 * deliberately rebuilt from the final typed AST and never becomes an RXAS
 * side channel. Unknown control shapes mark only the values they mention as
 * opaque, so unrelated values remain analysable.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "rxcpmain.h"
#include "rxcp_flow.h"

typedef struct FlowBits {
    uint64_t *words;
    size_t count;
} FlowBits;

typedef struct RxcpFlowValue {
    RxcpFlowValueKind kind;
    Symbol *symbol;
    ASTNode *temporary;
    ValueType type;
    ASTNode *first_read_anchor;
    ASTNode *first_write_anchor;
    ASTNode *last_use_anchor;
    unsigned int reads;
    unsigned int writes;
    unsigned int exposed : 1;
    unsigned int escaped : 1;
    unsigned int aliased : 1;
    unsigned int caller_owned : 1;
    unsigned int procedure_owned : 1;
    unsigned int single_use : 1;
} RxcpFlowValue;

typedef struct RxcpFlowBlock {
    int id;
    ASTNode *anchor;
    SourceNode *source_anchor;
    char source_provenance;
    int reachable;
    int contains_compiler_inserted_code;
    int *successors;
    size_t successor_count;
    size_t successor_capacity;
    int *predecessors;
    size_t predecessor_count;
    size_t predecessor_capacity;
    FlowBits uses;
    FlowBits defs;
    FlowBits safe_defs;
    FlowBits opaque;
    FlowBits live_in;
    FlowBits live_out;
    FlowBits definite_in;
    FlowBits definite_out;
    FlowBits reaching_gen;
    FlowBits reaching_kill;
    FlowBits reaching_in;
    FlowBits reaching_out;
} RxcpFlowBlock;

typedef struct RxcpFlowDefinition {
    size_t value;
    int block;
    ASTNode *anchor;
    ASTNode *constant_anchor;
    int copy_source;
} RxcpFlowDefinition;

typedef struct RxcpFlowProcedure {
    ASTNode *node;
    Scope *scope;
    numeric_context num_context;
    RxcpFlowValue *values;
    size_t value_count;
    size_t value_capacity;
    RxcpFlowBlock *blocks;
    size_t block_count;
    size_t block_capacity;
    RxcpFlowDefinition *definitions;
    size_t definition_count;
    int entry_block;
    int exit_block;
} RxcpFlowProcedure;

struct RxcpFlowProgram {
    RxcpFlowProcedure *procedures;
    size_t procedure_count;
    size_t procedure_capacity;
};

typedef struct FlowControl {
    ASTNode *owner;
    int break_block;
    int continue_block;
    const struct FlowControl *parent;
} FlowControl;

typedef struct FlowBuilder {
    RxcpFlowProcedure *procedure;
} FlowBuilder;

static int flow_copy_assignment_values(RxcpFlowProcedure *procedure,
                                       ASTNode *node,
                                       int *target_value,
                                       int *source_value);

static int bits_init(FlowBits *bits, size_t values) {
    bits->count = (values + 63u) / 64u;
    bits->words = bits->count ? calloc(bits->count, sizeof(uint64_t)) : 0;
    return bits->count == 0 || bits->words != 0;
}

static void bits_free(FlowBits *bits) {
    free(bits->words);
    bits->words = 0;
    bits->count = 0;
}

static void bits_clear(FlowBits *bits) {
    if (bits->words) memset(bits->words, 0, bits->count * sizeof(uint64_t));
}

static void bits_fill(FlowBits *bits, size_t values) {
    size_t i;
    for (i = 0; i < bits->count; i++) bits->words[i] = UINT64_MAX;
    if (bits->count && (values & 63u)) {
        bits->words[bits->count - 1] = (UINT64_C(1) << (values & 63u)) - 1u;
    }
}

static int bits_has(const FlowBits *bits, size_t bit) {
    size_t word = bit / 64u;
    if (!bits || word >= bits->count) return 0;
    return (bits->words[word] & (UINT64_C(1) << (bit & 63u))) != 0;
}

static void bits_set(FlowBits *bits, size_t bit) {
    size_t word = bit / 64u;
    if (bits && word < bits->count) bits->words[word] |= UINT64_C(1) << (bit & 63u);
}

static void bits_unset(FlowBits *bits, size_t bit) {
    size_t word = bit / 64u;
    if (bits && word < bits->count) bits->words[word] &= ~(UINT64_C(1) << (bit & 63u));
}

static int bits_equal(const FlowBits *left, const FlowBits *right) {
    if (left->count != right->count) return 0;
    return left->count == 0 || memcmp(left->words, right->words,
                                      left->count * sizeof(uint64_t)) == 0;
}

static void bits_copy(FlowBits *target, const FlowBits *source) {
    if (target->count && target->count == source->count) {
        memcpy(target->words, source->words, target->count * sizeof(uint64_t));
    }
}

static void bits_or(FlowBits *target, const FlowBits *source) {
    size_t i;
    for (i = 0; i < target->count; i++) target->words[i] |= source->words[i];
}

static void bits_and(FlowBits *target, const FlowBits *source) {
    size_t i;
    for (i = 0; i < target->count; i++) target->words[i] &= source->words[i];
}

static void bits_minus(FlowBits *target, const FlowBits *remove) {
    size_t i;
    for (i = 0; i < target->count; i++) target->words[i] &= ~remove->words[i];
}

static int flow_symbol_is_compiler_owned(const Symbol *symbol) {
    if (!symbol) return 0;
    if (symbol->name && strncmp(symbol->name, "__", 2) == 0) return 1;
    return symbol->creation_node && symbol->creation_node->is_compiler_added;
}

static int flow_add_value(RxcpFlowProcedure *procedure,
                          RxcpFlowValueKind kind,
                          Symbol *symbol,
                          ASTNode *temporary,
                          ValueType type) {
    RxcpFlowValue *value;
    size_t i;

    for (i = 0; i < procedure->value_count; i++) {
        if (symbol && procedure->values[i].symbol == symbol) return (int)i;
        if (temporary && procedure->values[i].temporary == temporary) return (int)i;
    }

    if (procedure->value_count == procedure->value_capacity) {
        size_t capacity = procedure->value_capacity ? procedure->value_capacity * 2u : 32u;
        RxcpFlowValue *values = realloc(procedure->values, capacity * sizeof(*values));
        if (!values) return -1;
        procedure->values = values;
        procedure->value_capacity = capacity;
    }

    value = &procedure->values[procedure->value_count];
    memset(value, 0, sizeof(*value));
    value->kind = kind;
    value->symbol = symbol;
    value->temporary = temporary;
    value->type = type;
    if (symbol) {
        value->exposed = symbol->exposed || symbol->is_global_var;
        value->escaped = symbol->has_reference_target || symbol->is_ref_arg;
        value->aliased = symbol->has_reference_target || symbol->is_ref_arg || symbol->exposed;
        value->caller_owned = symbol->is_arg || symbol->exposed || symbol->is_global_var;
        value->procedure_owned = !value->caller_owned;
    }
    return (int)procedure->value_count++;
}

static int flow_value_index_for_symbol(const RxcpFlowProcedure *procedure, const Symbol *symbol) {
    size_t i;
    if (!symbol) return -1;
    for (i = 0; i < procedure->value_count; i++) {
        if (procedure->values[i].symbol == symbol) return (int)i;
    }
    return -1;
}

static int flow_value_index_for_temp(const RxcpFlowProcedure *procedure, const ASTNode *node) {
    size_t i;
    if (!node) return -1;
    for (i = 0; i < procedure->value_count; i++) {
        if (procedure->values[i].temporary == node) return (int)i;
    }
    return -1;
}

static int flow_node_produces_temporary(const ASTNode *node) {
    if (!node || node->symbolNode || node->value_type == TP_UNKNOWN ||
        node->value_type == TP_VOID) return 0;
    switch (node->node_type) {
        case REXX_UNIVERSE: case PROGRAM_FILE: case IMPORTED_FILE:
        case NAMESPACE: case CLASS_DEF: case INTERFACE_DEF:
        case PROCEDURE: case METHOD: case FACTORY: case MATCH:
        case ARGS: case ARG: case INSTRUCTIONS: case ASSIGN: case DEFINE:
        case IF: case DO: case SIGNAL_BLOCK: case SIGNAL_HANDLER:
        case RETURN: case LEAVE: case ITERATE: case LEAVE_WITH:
        case SELECT: case SWITCH: case WHEN: case OTHERWISE:
            return 0;
        default:
            return 1;
    }
}

static int flow_collect_temporaries(RxcpFlowProcedure *procedure, ASTNode *node) {
    ASTNode *child;
    if (!node) return 1;
    if (node != procedure->node &&
        (node->node_type == PROCEDURE || node->node_type == METHOD ||
         node->node_type == FACTORY || node->node_type == MATCH)) return 1;

    if (flow_node_produces_temporary(node) &&
        flow_add_value(procedure, RXCP_FLOW_AST_TEMPORARY, 0, node,
                       node->value_type) < 0) return 0;
    for (child = node->child; child; child = child->sibling) {
        if (!flow_collect_temporaries(procedure, child)) return 0;
    }
    return 1;
}

static int flow_collect_scope_values(RxcpFlowProcedure *procedure, Scope *scope) {
    Symbol **symbols;
    size_t i;

    if (!scope) return 1;
    symbols = scp_syms(scope);
    if (!symbols) return 0;
    for (i = 0; symbols[i]; i++) {
        Symbol *symbol = symbols[i];
        RxcpFlowValueKind kind;
        if (symbol->symbol_type != VARIABLE_SYMBOL) continue;
        kind = flow_symbol_is_compiler_owned(symbol) ? RXCP_FLOW_COMPILER_SYMBOL :
                                                       RXCP_FLOW_SOURCE_SYMBOL;
        if (flow_add_value(procedure, kind, symbol, 0, symbol->type) < 0) {
            free(symbols);
            return 0;
        }
    }
    free(symbols);

    for (i = 0; i < scp_noch(scope); i++) {
        Scope *child = scp_chd(scope, i);
        if (child && child->type != SCOPE_PROCEDURE &&
            !flow_collect_scope_values(procedure, child)) return 0;
    }
    return 1;
}

static int flow_block_bits_init(RxcpFlowBlock *block, size_t values) {
    if (bits_init(&block->uses, values) &&
        bits_init(&block->defs, values) &&
        bits_init(&block->safe_defs, values) &&
        bits_init(&block->opaque, values) &&
        bits_init(&block->live_in, values) &&
        bits_init(&block->live_out, values) &&
        bits_init(&block->definite_in, values) &&
        bits_init(&block->definite_out, values)) return 1;
    bits_free(&block->uses);
    bits_free(&block->defs);
    bits_free(&block->safe_defs);
    bits_free(&block->opaque);
    bits_free(&block->live_in);
    bits_free(&block->live_out);
    bits_free(&block->definite_in);
    bits_free(&block->definite_out);
    return 0;
}

static int flow_new_block(RxcpFlowProcedure *procedure, ASTNode *anchor) {
    RxcpFlowBlock *block;
    if (procedure->block_count == procedure->block_capacity) {
        size_t capacity = procedure->block_capacity ? procedure->block_capacity * 2u : 32u;
        RxcpFlowBlock *blocks = realloc(procedure->blocks, capacity * sizeof(*blocks));
        if (!blocks) return -1;
        procedure->blocks = blocks;
        procedure->block_capacity = capacity;
    }
    block = &procedure->blocks[procedure->block_count];
    memset(block, 0, sizeof(*block));
    block->id = (int)procedure->block_count;
    block->anchor = anchor;
    block->source_anchor = anchor ? anchor->source_node : 0;
    block->source_provenance = anchor ? anchor->source_provenance : AST_SOURCE_NONE;
    if (!flow_block_bits_init(block, procedure->value_count)) return -1;
    procedure->block_count++;
    return block->id;
}

static int flow_int_array_add(int **array, size_t *count, size_t *capacity, int value) {
    size_t i;
    for (i = 0; i < *count; i++) if ((*array)[i] == value) return 1;
    if (*count == *capacity) {
        size_t next = *capacity ? *capacity * 2u : 4u;
        int *items = realloc(*array, next * sizeof(*items));
        if (!items) return 0;
        *array = items;
        *capacity = next;
    }
    (*array)[(*count)++] = value;
    return 1;
}

static int flow_add_edge(RxcpFlowProcedure *procedure, int from, int to) {
    RxcpFlowBlock *source;
    RxcpFlowBlock *target;
    if (from < 0 || to < 0 || (size_t)from >= procedure->block_count ||
        (size_t)to >= procedure->block_count) return 0;
    source = &procedure->blocks[from];
    target = &procedure->blocks[to];
    return flow_int_array_add(&source->successors, &source->successor_count,
                              &source->successor_capacity, to) &&
           flow_int_array_add(&target->predecessors, &target->predecessor_count,
                              &target->predecessor_capacity, from);
}

static Symbol *flow_variable_symbol(ASTNode *node) {
    if (!node || !node->symbolNode || !node->symbolNode->symbol) return 0;
    return node->symbolNode->symbol->symbol_type == VARIABLE_SYMBOL ?
           node->symbolNode->symbol : 0;
}

static int flow_node_reads_variable(ASTNode *node) {
    if (!node || !node->symbolNode) return 0;
    /* Generated SELECT lowering can retain a semantic VAR_REFERENCE after the
     * connector's ordinary read flag has been cleared. The node is still a
     * machine read, irrespective of that bookkeeping flag. */
    return node->symbolNode->readUsage || node->node_type == VAR_REFERENCE;
}

static void flow_note_anchor(ASTNode **first, ASTNode **last, ASTNode *node) {
    if (!node) return;
    if (!*first || node->node_number < (*first)->node_number) *first = node;
    if (!*last || node->node_number > (*last)->node_number) *last = node;
}

static void flow_mark_node_effect(RxcpFlowProcedure *procedure,
                                  RxcpFlowBlock *block,
                                  ASTNode *node,
                                  int opaque) {
    Symbol *symbol = flow_variable_symbol(node);
    int value;
    if (symbol) {
        value = flow_value_index_for_symbol(procedure, symbol);
        if (value >= 0) {
            if (flow_node_reads_variable(node)) {
                bits_set(&block->uses, (size_t)value);
                procedure->values[value].reads++;
                flow_note_anchor(&procedure->values[value].first_read_anchor,
                                 &procedure->values[value].last_use_anchor, node);
            }
            if (node->symbolNode->writeUsage &&
                (!node->parent || (node->parent->node_type != DEFINE &&
                                   node->parent->node_type != ARG))) {
                bits_set(&block->defs, (size_t)value);
                /* A validated assembler destination whose refined connector
                 * is write-only kills the old value. On the normal successor
                 * path it is therefore a safe definition for default-init
                 * analysis; instructions with conservative read/write links
                 * retain the old fail-closed behaviour. */
                if (!node->symbolNode->readUsage && node->parent &&
                    node->parent->node_type == ASSEMBLER) {
                    bits_set(&block->safe_defs, (size_t)value);
                }
                procedure->values[value].writes++;
                flow_note_anchor(&procedure->values[value].first_write_anchor,
                                 &procedure->values[value].last_use_anchor, node);
            }
            if (opaque) bits_set(&block->opaque, (size_t)value);
        }
    } else {
        value = flow_value_index_for_temp(procedure, node);
        if (value >= 0) bits_set(&block->defs, (size_t)value);
    }
}

static void flow_collect_tree(RxcpFlowProcedure *procedure,
                              RxcpFlowBlock *block,
                              ASTNode *node,
                              int opaque) {
    ASTNode *child;
    if (!node) return;
    if (node->is_compiler_added || node->source_provenance == AST_SOURCE_SYNTHETIC ||
        node->source_provenance == AST_SOURCE_COMPOSITE) {
        block->contains_compiler_inserted_code = 1;
    }
    flow_mark_node_effect(procedure, block, node, opaque);
    for (child = node->child; child; child = child->sibling) {
        flow_collect_tree(procedure, block, child, opaque);
        {
            int value = flow_value_index_for_temp(procedure, child);
            if (value >= 0) bits_set(&block->uses, (size_t)value);
        }
    }
}

static int flow_scalar_type(ValueType type) {
    return type == TP_BOOLEAN || type == TP_INTEGER || type == TP_FLOAT ||
           type == TP_DECIMAL || type == TP_STRING;
}

static int flow_safe_rhs(ASTNode *node, ValueType target_type) {
    Symbol *symbol;
    if (!node || node->value_dims || node->target_dims) return 0;
    if (node->value_type != target_type && node->target_type != target_type) return 0;
    switch (node->node_type) {
        case INTEGER: case FLOAT: case DECIMAL: case STRING: case CONSTANT:
            return flow_scalar_type(target_type);
        case VAR_SYMBOL:
            symbol = flow_variable_symbol(node);
            return symbol && symbol->value_dims == 0 && symbol->type == target_type &&
                   flow_scalar_type(target_type);
        default:
            return 0;
    }
}

static void flow_mark_safe_assignment(RxcpFlowProcedure *procedure,
                                      RxcpFlowBlock *block,
                                      ASTNode *node) {
    ASTNode *target;
    ASTNode *value_node;
    Symbol *symbol;
    int value;
    if (!node || node->node_type != ASSIGN) return;
    target = node->child;
    value_node = target ? target->sibling : 0;
    if (!target || target->child || !value_node) return;
    symbol = flow_variable_symbol(target);
    if (!symbol || symbol->value_dims || !flow_safe_rhs(value_node, symbol->type)) return;
    value = flow_value_index_for_symbol(procedure, symbol);
    if (value >= 0) bits_set(&block->safe_defs, (size_t)value);
}

static int flow_tree_has_nested_control(ASTNode *node, ASTNode *root) {
    ASTNode *child;
    if (!node) return 0;
    if (node != root &&
        (node->node_type == IF || node->node_type == DO ||
         node->node_type == SIGNAL_BLOCK || node->node_type == BLOCK_EXPR ||
         node->node_type == SELECT || node->node_type == SWITCH ||
         node->node_type == OPT_DISPATCH)) return 1;
    for (child = node->child; child; child = child->sibling) {
        if (flow_tree_has_nested_control(child, root)) return 1;
    }
    return 0;
}

static void flow_collect_statement(RxcpFlowProcedure *procedure,
                                   int block_id,
                                   ASTNode *node,
                                   int opaque) {
    RxcpFlowBlock *block = &procedure->blocks[block_id];
    int nested = flow_tree_has_nested_control(node, node);
    flow_collect_tree(procedure, block, node, opaque || nested);
    if (!opaque && !nested) flow_mark_safe_assignment(procedure, block, node);
}

static const FlowControl *flow_find_control(const FlowControl *control, ASTNode *owner) {
    const FlowControl *current;
    for (current = control; current; current = current->parent) {
        if (!owner || current->owner == owner) return current;
    }
    return 0;
}

static int flow_build_statement(FlowBuilder *builder,
                                ASTNode *node,
                                int next,
                                const FlowControl *control);

static int flow_build_sequence(FlowBuilder *builder,
                               ASTNode *first,
                               int next,
                               const FlowControl *control) {
    ASTNode **nodes = 0;
    ASTNode *node;
    size_t count = 0;
    size_t capacity = 0;
    int entry = next;
    size_t i;

    for (node = first; node; node = node->sibling) {
        if (count == capacity) {
            size_t new_capacity = capacity ? capacity * 2u : 16u;
            ASTNode **new_nodes = realloc(nodes, new_capacity * sizeof(*new_nodes));
            if (!new_nodes) {
                free(nodes);
                return -1;
            }
            nodes = new_nodes;
            capacity = new_capacity;
        }
        nodes[count++] = node;
    }
    for (i = count; i > 0; i--) {
        entry = flow_build_statement(builder, nodes[i - 1], entry, control);
        if (entry < 0) break;
    }
    free(nodes);
    return entry;
}

static int flow_build_if(FlowBuilder *builder,
                         ASTNode *node,
                         int next,
                         const FlowControl *control) {
    RxcpFlowProcedure *procedure = builder->procedure;
    ASTNode *condition = node->child;
    ASTNode *yes = condition ? condition->sibling : 0;
    ASTNode *no = yes ? yes->sibling : 0;
    int block = flow_new_block(procedure, condition ? condition : node);
    int yes_entry;
    int no_entry;
    if (block < 0) return -1;
    if (condition) flow_collect_statement(procedure, block, condition, 0);
    yes_entry = yes ? flow_build_statement(builder, yes, next, control) : next;
    no_entry = no ? flow_build_statement(builder, no, next, control) : next;
    if (yes_entry < 0 || no_entry < 0 ||
        !flow_add_edge(procedure, block, yes_entry) ||
        !flow_add_edge(procedure, block, no_entry)) return -1;
    return block;
}

static ASTNode *flow_direct_child(ASTNode *node, NodeType type) {
    ASTNode *child;
    for (child = node ? node->child : 0; child; child = child->sibling) {
        if (child->node_type == type) return child;
    }
    return 0;
}

static int flow_do_is_loop(ASTNode *node) {
    ASTNode *child;
    for (child = node ? node->child : 0; child; child = child->sibling) {
        if (child->node_type == REPEAT || child->node_type == WHILE ||
            child->node_type == UNTIL || child->node_type == FOR ||
            child->node_type == TO || child->node_type == BY) return 1;
    }
    return 0;
}

static void flow_collect_do_condition(RxcpFlowProcedure *procedure,
                                      int block_id,
                                      ASTNode *node) {
    RxcpFlowBlock *block = &procedure->blocks[block_id];
    ASTNode *child;
    for (child = node->child; child; child = child->sibling) {
        if (child->node_type == INSTRUCTIONS) continue;
        if (child->node_type == REPEAT) {
            ASTNode *part;
            for (part = child->child; part; part = part->sibling) {
                if (part == child->child && part->node_type == ASSIGN) continue;
                flow_collect_tree(procedure, block, part, 0);
            }
        } else {
            flow_collect_tree(procedure, block, child, 0);
        }
    }
}

static int flow_build_do(FlowBuilder *builder,
                         ASTNode *node,
                         int next,
                         const FlowControl *control) {
    RxcpFlowProcedure *procedure = builder->procedure;
    ASTNode *body = flow_direct_child(node, INSTRUCTIONS);
    ASTNode *repeat = flow_direct_child(node, REPEAT);
    ASTNode *initial = repeat && repeat->child && repeat->child->node_type == ASSIGN ?
                       repeat->child : 0;
    int condition_block;
    int latch_block;
    int body_entry;
    int initial_block;
    FlowControl loop_control;

    if (!body) {
        int block = flow_new_block(procedure, node);
        if (block < 0) return -1;
        flow_collect_statement(procedure, block, node, 1);
        return flow_add_edge(procedure, block, next) ? block : -1;
    }
    if (!flow_do_is_loop(node)) return flow_build_sequence(builder, body->child, next, control);

    condition_block = flow_new_block(procedure, node);
    if (condition_block < 0) return -1;
    flow_collect_do_condition(procedure, condition_block, node);

    latch_block = condition_block;
    if (initial) {
        Symbol *symbol = flow_variable_symbol(initial->child);
        int value = flow_value_index_for_symbol(procedure, symbol);
        latch_block = flow_new_block(procedure, repeat);
        if (latch_block < 0) return -1;
        if (value >= 0) {
            bits_set(&procedure->blocks[latch_block].uses, (size_t)value);
            bits_set(&procedure->blocks[latch_block].defs, (size_t)value);
        }
        if (!flow_add_edge(procedure, latch_block, condition_block)) return -1;
    }

    loop_control.owner = node;
    loop_control.break_block = next;
    loop_control.continue_block = latch_block;
    loop_control.parent = control;
    body_entry = flow_build_sequence(builder, body->child, latch_block, &loop_control);
    if (body_entry < 0 ||
        !flow_add_edge(procedure, condition_block, body_entry) ||
        !flow_add_edge(procedure, condition_block, next)) return -1;

    if (!initial) return condition_block;
    initial_block = flow_new_block(procedure, initial);
    if (initial_block < 0) return -1;
    flow_collect_statement(procedure, initial_block, initial, 0);
    return flow_add_edge(procedure, initial_block, condition_block) ? initial_block : -1;
}

static ASTNode *flow_handler_body(ASTNode *handler) {
    ASTNode *child;
    for (child = handler ? handler->child : 0; child; child = child->sibling) {
        if (child->node_type == INSTRUCTIONS) return child;
    }
    return 0;
}

static int flow_build_handler(FlowBuilder *builder,
                              ASTNode *handler,
                              int next,
                              const FlowControl *control) {
    RxcpFlowProcedure *procedure = builder->procedure;
    ASTNode *body = flow_handler_body(handler);
    int body_entry = body ? flow_build_sequence(builder, body->child, next, control) : next;
    int header = flow_new_block(procedure, handler);
    ASTNode *child;
    if (body_entry < 0 || header < 0) return -1;
    for (child = handler->child; child; child = child->sibling) {
        if (child != body) flow_collect_tree(procedure, &procedure->blocks[header], child, 0);
    }
    return flow_add_edge(procedure, header, body_entry) ? header : -1;
}

static int flow_build_signal_block(FlowBuilder *builder,
                                   ASTNode *node,
                                   int next,
                                   const FlowControl *control) {
    RxcpFlowProcedure *procedure = builder->procedure;
    ASTNode *try_body = node->child;
    ASTNode *handler;
    int *handlers = 0;
    size_t handler_count = 0;
    size_t handler_capacity = 0;
    size_t try_first;
    size_t try_end;
    size_t i;
    int try_entry;

    for (handler = try_body ? try_body->sibling : 0; handler; handler = handler->sibling) {
        int entry;
        if (handler->node_type != SIGNAL_HANDLER) continue;
        entry = flow_build_handler(builder, handler, next, control);
        if (entry < 0 || !flow_int_array_add(&handlers, &handler_count,
                                             &handler_capacity, entry)) {
            free(handlers);
            return -1;
        }
    }

    try_first = procedure->block_count;
    if (try_body && try_body->node_type == INSTRUCTIONS) {
        try_entry = flow_build_sequence(builder, try_body->child, next, control);
    } else if (try_body && try_body->node_type != NOP) {
        try_entry = flow_build_statement(builder, try_body, next, control);
    } else {
        try_entry = next;
    }
    try_end = procedure->block_count;
    if (try_entry < 0) {
        free(handlers);
        return -1;
    }

    for (i = try_first; i < try_end; i++) {
        size_t h;
        for (h = 0; h < handler_count; h++) {
            if (!flow_add_edge(procedure, (int)i, handlers[h])) {
                free(handlers);
                return -1;
            }
        }
    }
    free(handlers);
    return try_entry;
}

static int flow_build_statement(FlowBuilder *builder,
                                ASTNode *node,
                                int next,
                                const FlowControl *control) {
    RxcpFlowProcedure *procedure = builder->procedure;
    int block;
    const FlowControl *target;
    if (!node) return next;
    switch (node->node_type) {
        case INSTRUCTIONS:
            return flow_build_sequence(builder, node->child, next, control);
        case IF:
            return flow_build_if(builder, node, next, control);
        case DO:
            return flow_build_do(builder, node, next, control);
        case SIGNAL_BLOCK:
            return flow_build_signal_block(builder, node, next, control);
        case RETURN:
            block = flow_new_block(procedure, node);
            if (block < 0) return -1;
            flow_collect_statement(procedure, block, node, 0);
            return flow_add_edge(procedure, block, procedure->exit_block) ? block : -1;
        case LEAVE:
            block = flow_new_block(procedure, node);
            if (block < 0) return -1;
            flow_collect_statement(procedure, block, node, 0);
            target = flow_find_control(control, node->association);
            return flow_add_edge(procedure, block,
                                 target ? target->break_block : procedure->exit_block) ? block : -1;
        case ITERATE:
            block = flow_new_block(procedure, node);
            if (block < 0) return -1;
            flow_collect_statement(procedure, block, node, 0);
            target = flow_find_control(control, node->association);
            return flow_add_edge(procedure, block,
                                 target ? target->continue_block : procedure->exit_block) ? block : -1;
        case SELECT: case SWITCH: case OPT_DISPATCH: case BLOCK_EXPR:
            block = flow_new_block(procedure, node);
            if (block < 0) return -1;
            flow_collect_statement(procedure, block, node, 1);
            return flow_add_edge(procedure, block, next) ? block : -1;
        default:
            block = flow_new_block(procedure, node);
            if (block < 0) return -1;
            flow_collect_statement(procedure, block, node, 0);
            return flow_add_edge(procedure, block, next) ? block : -1;
    }
}

static int flow_mark_reachable(RxcpFlowProcedure *procedure) {
    int *stack;
    unsigned char *enqueued;
    size_t size = 0;
    size_t capacity = procedure->block_count ? procedure->block_count : 1;
    stack = malloc(capacity * sizeof(*stack));
    enqueued = calloc(capacity, 1);
    if (!stack || !enqueued) {
        free(stack);
        free(enqueued);
        return 0;
    }
    stack[size++] = procedure->entry_block;
    enqueued[procedure->entry_block] = 1;
    while (size) {
        int id = stack[--size];
        RxcpFlowBlock *block;
        size_t i;
        if (id < 0 || (size_t)id >= procedure->block_count) continue;
        block = &procedure->blocks[id];
        if (block->reachable) continue;
        block->reachable = 1;
        for (i = 0; i < block->successor_count; i++) {
            int successor = block->successors[i];
            if (!enqueued[successor]) {
                enqueued[successor] = 1;
                stack[size++] = successor;
            }
        }
    }
    free(enqueued);
    free(stack);
    return 1;
}

static int flow_run_liveness(RxcpFlowProcedure *procedure) {
    FlowBits new_in = {0};
    FlowBits new_out = {0};
    FlowBits effective_defs = {0};
    int changed;
    size_t i;
    if (!bits_init(&new_in, procedure->value_count) ||
        !bits_init(&new_out, procedure->value_count) ||
        !bits_init(&effective_defs, procedure->value_count)) {
        bits_free(&new_in);
        bits_free(&new_out);
        bits_free(&effective_defs);
        return 0;
    }
    do {
        changed = 0;
        for (i = procedure->block_count; i > 0; i--) {
            RxcpFlowBlock *block = &procedure->blocks[i - 1];
            size_t s;
            if (!block->reachable) continue;
            bits_clear(&new_out);
            for (s = 0; s < block->successor_count; s++) {
                bits_or(&new_out, &procedure->blocks[block->successors[s]].live_in);
            }
            bits_copy(&new_in, &new_out);
            bits_copy(&effective_defs, &block->defs);
            if (block->anchor && block->anchor->flow_skip_assignment_store) {
                int skipped_target = -1;
                if (flow_copy_assignment_values(procedure, block->anchor,
                                                &skipped_target, 0) &&
                    skipped_target >= 0) {
                    /* A skipped exact x=y store does not define x's physical
                     * register. Other RHS shapes may already emit directly
                     * into x and therefore retain their definition here. */
                    bits_unset(&effective_defs, (size_t)skipped_target);
                }
            }
            bits_minus(&new_in, &effective_defs);
            bits_or(&new_in, &block->uses);
            if (!bits_equal(&new_in, &block->live_in) ||
                !bits_equal(&new_out, &block->live_out)) {
                bits_copy(&block->live_in, &new_in);
                bits_copy(&block->live_out, &new_out);
                changed = 1;
            }
        }
    } while (changed);
    bits_free(&new_in);
    bits_free(&new_out);
    bits_free(&effective_defs);
    return 1;
}

static int flow_run_definite_assignment(RxcpFlowProcedure *procedure) {
    FlowBits incoming;
    FlowBits outgoing;
    int changed;
    size_t i;
    if (!bits_init(&incoming, procedure->value_count) ||
        !bits_init(&outgoing, procedure->value_count)) return 0;

    for (i = 0; i < procedure->block_count; i++) {
        if ((int)i != procedure->entry_block && procedure->blocks[i].reachable) {
            bits_fill(&procedure->blocks[i].definite_in, procedure->value_count);
            bits_fill(&procedure->blocks[i].definite_out, procedure->value_count);
        }
    }
    for (i = 0; i < procedure->value_count; i++) {
        Symbol *symbol = procedure->values[i].symbol;
        if (symbol && (symbol->is_arg || symbol->needs_default_initiation)) {
            bits_set(&procedure->blocks[procedure->entry_block].definite_out, i);
        }
    }

    do {
        changed = 0;
        for (i = 0; i < procedure->block_count; i++) {
            RxcpFlowBlock *block = &procedure->blocks[i];
            size_t p;
            int have_predecessor = 0;
            if (!block->reachable || (int)i == procedure->entry_block) continue;
            bits_fill(&incoming, procedure->value_count);
            for (p = 0; p < block->predecessor_count; p++) {
                RxcpFlowBlock *pred = &procedure->blocks[block->predecessors[p]];
                if (!pred->reachable) continue;
                if (!have_predecessor) {
                    bits_copy(&incoming, &pred->definite_out);
                    have_predecessor = 1;
                } else {
                    bits_and(&incoming, &pred->definite_out);
                }
            }
            if (!have_predecessor) bits_clear(&incoming);
            bits_copy(&outgoing, &incoming);
            bits_or(&outgoing, &block->defs);
            if (!bits_equal(&incoming, &block->definite_in) ||
                !bits_equal(&outgoing, &block->definite_out)) {
                bits_copy(&block->definite_in, &incoming);
                bits_copy(&block->definite_out, &outgoing);
                changed = 1;
            }
        }
    } while (changed);
    bits_free(&incoming);
    bits_free(&outgoing);
    return 1;
}

static int flow_build_definitions(RxcpFlowProcedure *procedure) {
    size_t block_index;
    size_t value;
    size_t count = 0;
    size_t definition;

    for (block_index = 0; block_index < procedure->block_count; block_index++) {
        for (value = 0; value < procedure->value_count; value++) {
            if (bits_has(&procedure->blocks[block_index].defs, value)) count++;
        }
    }
    procedure->definitions = count ? calloc(count, sizeof(*procedure->definitions)) : 0;
    if (count && !procedure->definitions) return 0;
    procedure->definition_count = count;
    definition = 0;
    for (block_index = 0; block_index < procedure->block_count; block_index++) {
        for (value = 0; value < procedure->value_count; value++) {
            if (!bits_has(&procedure->blocks[block_index].defs, value)) continue;
            procedure->definitions[definition].value = value;
            procedure->definitions[definition].block = (int)block_index;
            procedure->definitions[definition].anchor = procedure->blocks[block_index].anchor;
            procedure->definitions[definition].copy_source = -1;
            if (procedure->blocks[block_index].anchor &&
                procedure->blocks[block_index].anchor->node_type == ASSIGN) {
                ASTNode *target = procedure->blocks[block_index].anchor->child;
                ASTNode *rhs = target ? target->sibling : 0;
                Symbol *target_symbol = flow_variable_symbol(target);
                if (target_symbol &&
                    flow_value_index_for_symbol(procedure, target_symbol) == (int)value && rhs) {
                    if (rhs->node_type == INTEGER || rhs->node_type == FLOAT ||
                        rhs->node_type == DECIMAL || rhs->node_type == STRING ||
                        rhs->node_type == CONSTANT) {
                        procedure->definitions[definition].constant_anchor = rhs;
                    } else if (rhs->node_type == VAR_SYMBOL) {
                        procedure->definitions[definition].copy_source =
                                flow_value_index_for_symbol(procedure,
                                                            flow_variable_symbol(rhs));
                    }
                }
            }
            definition++;
        }
    }

    for (block_index = 0; block_index < procedure->block_count; block_index++) {
        RxcpFlowBlock *block = &procedure->blocks[block_index];
        if (!bits_init(&block->reaching_gen, count) ||
            !bits_init(&block->reaching_kill, count) ||
            !bits_init(&block->reaching_in, count) ||
            !bits_init(&block->reaching_out, count)) return 0;
        for (definition = 0; definition < count; definition++) {
            size_t def_value = procedure->definitions[definition].value;
            if (!bits_has(&block->defs, def_value)) continue;
            if (procedure->definitions[definition].block == (int)block_index) {
                bits_set(&block->reaching_gen, definition);
            } else {
                bits_set(&block->reaching_kill, definition);
            }
        }
    }
    return 1;
}

static int flow_run_reaching_definitions(RxcpFlowProcedure *procedure) {
    FlowBits incoming;
    FlowBits outgoing;
    int changed;
    size_t i;
    if (!bits_init(&incoming, procedure->definition_count) ||
        !bits_init(&outgoing, procedure->definition_count)) return 0;
    do {
        changed = 0;
        for (i = 0; i < procedure->block_count; i++) {
            RxcpFlowBlock *block = &procedure->blocks[i];
            size_t p;
            if (!block->reachable) continue;
            bits_clear(&incoming);
            for (p = 0; p < block->predecessor_count; p++) {
                RxcpFlowBlock *pred = &procedure->blocks[block->predecessors[p]];
                if (pred->reachable) bits_or(&incoming, &pred->reaching_out);
            }
            bits_copy(&outgoing, &incoming);
            bits_minus(&outgoing, &block->reaching_kill);
            bits_or(&outgoing, &block->reaching_gen);
            if (!bits_equal(&incoming, &block->reaching_in) ||
                !bits_equal(&outgoing, &block->reaching_out)) {
                bits_copy(&block->reaching_in, &incoming);
                bits_copy(&block->reaching_out, &outgoing);
                changed = 1;
            }
        }
    } while (changed);
    bits_free(&incoming);
    bits_free(&outgoing);
    return 1;
}

static int flow_must_safe_write_before_read(RxcpFlowProcedure *procedure,
                                            size_t value) {
    unsigned char *in_state;
    unsigned char *out_state;
    int changed;
    size_t i;
    size_t iterations = 0;
    size_t limit = procedure->block_count * 4u + 4u;

    in_state = calloc(procedure->block_count, 1);
    out_state = calloc(procedure->block_count, 1);
    if (!in_state || !out_state) {
        free(in_state);
        free(out_state);
        return 0;
    }
    /* Must facts use intersection at joins, so reachable non-entry blocks
     * start at lattice top. Starting a loop at bottom would never discover a
     * safe definition that dominates its preheader. */
    for (i = 0; i < procedure->block_count; i++) {
        if (procedure->blocks[i].reachable && (int)i != procedure->entry_block) {
            in_state[i] = 1;
            out_state[i] = 1;
        }
    }
    do {
        changed = 0;
        for (i = 0; i < procedure->block_count; i++) {
            RxcpFlowBlock *block = &procedure->blocks[i];
            unsigned char incoming = 0;
            unsigned char outgoing;
            size_t p;
            int have_predecessor = 0;
            if (!block->reachable) continue;
            if ((int)i != procedure->entry_block) {
                incoming = 1;
                for (p = 0; p < block->predecessor_count; p++) {
                    RxcpFlowBlock *pred = &procedure->blocks[block->predecessors[p]];
                    if (!pred->reachable) continue;
                    incoming = (unsigned char)(incoming && out_state[pred->id]);
                    have_predecessor = 1;
                }
                if (!have_predecessor) incoming = 0;
            }
            outgoing = (unsigned char)(incoming || bits_has(&block->safe_defs, value));
            if (in_state[i] != incoming || out_state[i] != outgoing) {
                in_state[i] = incoming;
                out_state[i] = outgoing;
                changed = 1;
            }
        }
        iterations++;
    } while (changed && iterations < limit);

    if (changed) {
        free(in_state);
        free(out_state);
        return 0;
    }
    for (i = 0; i < procedure->block_count; i++) {
        RxcpFlowBlock *block = &procedure->blocks[i];
        if (!block->reachable) continue;
        if (bits_has(&block->opaque, value) ||
            (bits_has(&block->uses, value) && !in_state[i])) {
            free(in_state);
            free(out_state);
            return 0;
        }
    }
    free(in_state);
    free(out_state);
    return 1;
}

static int flow_symbol_has_dereference_alias(Symbol *symbol) {
    size_t i;
    if (!symbol || !symbol->scope) return 0;
    for (i = 0; i < scp_dereference_symbol_count(symbol->scope); i++) {
        if (scp_dereference_symbol_at(symbol->scope, i) == symbol) return 1;
    }
    return 0;
}

static int flow_default_candidate(Symbol *symbol) {
    if (!symbol || !symbol->needs_default_initiation ||
        symbol->symbol_type != VARIABLE_SYMBOL || !symbol->scope) return 0;
    if (symbol->scope->type != SCOPE_PROCEDURE && symbol->scope->type != SCOPE_LOCAL) return 0;
    if (symbol->is_arg || symbol->is_ref_arg || symbol->is_this || symbol->is_factory ||
        symbol->exposed || symbol->is_global_var || symbol->has_reference_target ||
        symbol->value_dims || flow_symbol_is_compiler_owned(symbol) ||
        flow_symbol_has_dereference_alias(symbol)) return 0;
    return flow_scalar_type(symbol->type);
}

static int flow_arg_candidate(ASTNode *arg, Symbol *symbol) {
    if (!arg || !symbol || arg->node_type != ARG || arg->is_ref_arg ||
        arg->is_opt_arg || arg->is_const_arg) return 0;
    if (!symbol->is_arg || symbol->is_ref_arg || symbol->exposed ||
        symbol->has_reference_target || symbol->value_dims ||
        flow_symbol_is_compiler_owned(symbol) || flow_symbol_has_dereference_alias(symbol)) return 0;
    return symbol->type == TP_BOOLEAN || symbol->type == TP_INTEGER ||
           symbol->type == TP_FLOAT || symbol->type == TP_DECIMAL;
}

#define FLOW_COPY_NONE (-1)
#define FLOW_COPY_TOP  (-2)

static int flow_copy_type(ValueType type) {
    return type == TP_BOOLEAN || type == TP_INTEGER || type == TP_FLOAT;
}

static int flow_copy_value_candidate(const RxcpFlowValue *value) {
    Symbol *symbol;
    if (!value || !(symbol = value->symbol) || !flow_copy_type(value->type) ||
        symbol->symbol_type != VARIABLE_SYMBOL || !symbol->scope ||
        (symbol->scope->type != SCOPE_PROCEDURE && symbol->scope->type != SCOPE_LOCAL) ||
        symbol->is_ref_arg || symbol->is_this || symbol->is_factory ||
        symbol->exposed || symbol->is_global_var || symbol->has_reference_target ||
        symbol->value_dims || flow_symbol_has_dereference_alias(symbol)) return 0;
    return 1;
}

static int flow_copy_assignment_values(RxcpFlowProcedure *procedure,
                                       ASTNode *node,
                                       int *target_value,
                                       int *source_value) {
    ASTNode *target;
    ASTNode *source;
    Symbol *target_symbol;
    Symbol *source_symbol;
    int target_index;
    int source_index;
    if (target_value) *target_value = FLOW_COPY_NONE;
    if (source_value) *source_value = FLOW_COPY_NONE;
    if (!node || node->node_type != ASSIGN) return 0;
    target = node->child;
    source = target ? target->sibling : 0;
    if (!target || target->child || !source || source->child ||
        source->node_type != VAR_SYMBOL) return 0;
    target_symbol = flow_variable_symbol(target);
    source_symbol = flow_variable_symbol(source);
    target_index = flow_value_index_for_symbol(procedure, target_symbol);
    source_index = flow_value_index_for_symbol(procedure, source_symbol);
    if (target_index < 0 || source_index < 0 || target_index == source_index ||
        !flow_copy_value_candidate(&procedure->values[target_index]) ||
        !flow_copy_value_candidate(&procedure->values[source_index]) ||
        target_symbol->type != source_symbol->type) return 0;
    if (target_value) *target_value = target_index;
    if (source_value) *source_value = source_index;
    return 1;
}

static void flow_copy_kill(int *state, size_t count, int value) {
    size_t i;
    for (i = 0; i < count; i++) {
        if ((int)i == value || state[i] == value) state[i] = FLOW_COPY_NONE;
    }
}

static void flow_copy_transfer(RxcpFlowProcedure *procedure,
                               RxcpFlowBlock *block,
                               const int *input,
                               int *output) {
    int target = FLOW_COPY_NONE;
    int source = FLOW_COPY_NONE;
    int skipped_copy;
    size_t i;
    memcpy(output, input, procedure->value_count * sizeof(*output));
    (void)flow_copy_assignment_values(procedure, block->anchor, &target, &source);
    skipped_copy = block->anchor && block->anchor->flow_skip_assignment_store &&
                   target >= 0 && source >= 0;
    for (i = 0; i < procedure->value_count; i++) {
        if (bits_has(&block->defs, i) || bits_has(&block->opaque, i)) {
            if (skipped_copy && (int)i == target &&
                !bits_has(&block->opaque, i)) continue;
            flow_copy_kill(output, procedure->value_count, (int)i);
        }
    }
    if (!skipped_copy && target >= 0 && source >= 0 &&
        !bits_has(&block->opaque, (size_t)target) &&
        !bits_has(&block->opaque, (size_t)source)) {
        /* Keep a stable direct equality rather than canonicalising chains in
         * the transfer function. Canonicalisation can oscillate around copy
         * cycles; direct x==y facts form the normal finite must-copy lattice. */
        output[target] = source;
    }
}

static int flow_copy_meet(int left, int right) {
    if (left == FLOW_COPY_TOP) return right;
    if (right == FLOW_COPY_TOP) return left;
    return left == right ? left : FLOW_COPY_NONE;
}

static int flow_resolve_copy_source(const RxcpFlowProcedure *procedure,
                                    const int *state,
                                    int value) {
    int source;
    size_t steps = 0;
    if (value < 0) return FLOW_COPY_NONE;
    source = state[value];
    while (source >= 0 && state[source] >= 0 &&
           state[source] != source && steps++ < procedure->value_count) {
        source = state[source];
    }
    if (steps >= procedure->value_count ||
        (source >= 0 && state[source] == value)) return FLOW_COPY_NONE;
    return source;
}

static size_t flow_count_register_reads(RxcpFlowProcedure *procedure,
                                        ASTNode *root,
                                        const int *state,
                                        int source) {
    ASTNode *child;
    Symbol *symbol;
    int value;
    int resolved;
    size_t count = 0;
    if (!root) return 0;
    if (root != procedure->node &&
        (root->node_type == PROCEDURE || root->node_type == METHOD ||
         root->node_type == FACTORY || root->node_type == MATCH)) return 0;
    symbol = flow_variable_symbol(root);
    if (symbol && flow_node_reads_variable(root)) {
        value = flow_value_index_for_symbol(procedure, symbol);
        resolved = flow_resolve_copy_source(procedure, state, value);
        if (resolved < 0) resolved = value;
        if (resolved == source) count++;
    }
    for (child = root->child; child; child = child->sibling) {
        count += flow_count_register_reads(procedure, child, state, source);
    }
    return count;
}

static int flow_copy_read_can_share_register(RxcpFlowProcedure *procedure,
                                             RxcpFlowBlock *block,
                                             ASTNode *node,
                                             const int *state,
                                             int source) {
    ValueType target_type;
    if (!node) return 0;
    target_type = node->target_type;
    if (target_type == TP_UNKNOWN || target_type == node->value_type) return 1;

    /* Scalar promotions are emitted in place. Sharing is therefore legal only
     * when this is the sole read of the resolved physical register in the
     * statement and its old value is dead afterwards. This rejects one use,
     * not the equality or the surrounding block/procedure. */
    if (bits_has(&block->live_out, (size_t)source)) return 0;
    return flow_count_register_reads(procedure, block->anchor, state, source) == 1;
}

static int flow_mark_copy_reads(RxcpFlowProcedure *procedure,
                                RxcpFlowBlock *block,
                                ASTNode *node,
                                const int *state) {
    ASTNode *child;
    Symbol *symbol;
    int value;
    int source;
    int changed = 0;
    if (!node) return 0;
    if (node != block->anchor &&
        (node->node_type == PROCEDURE || node->node_type == METHOD ||
         node->node_type == FACTORY || node->node_type == MATCH)) return 0;
    symbol = flow_variable_symbol(node);
    if (symbol && flow_node_reads_variable(node) && !node->symbolNode->writeUsage &&
        !node->child) {
        value = flow_value_index_for_symbol(procedure, symbol);
        source = flow_resolve_copy_source(procedure, state, value);
        /* Resolve stable direct equalities only at the use. A cycle is not a
         * usable canonical register and therefore rejects this one rewrite. */
        if (source >= 0 && source != value &&
            !bits_has(&block->opaque, (size_t)value) &&
            !bits_has(&block->opaque, (size_t)source) &&
            flow_copy_value_candidate(&procedure->values[value]) &&
            flow_copy_value_candidate(&procedure->values[source]) &&
            flow_copy_read_can_share_register(procedure, block, node, state, source)) {
            Symbol *replacement = procedure->values[source].symbol;
            if (node->flow_substitute_symbol != replacement) {
                node->flow_substitute_symbol = replacement;
                changed = 1;
            }
        }
    }
    for (child = node->child; child; child = child->sibling) {
        changed |= flow_mark_copy_reads(procedure, block, child, state);
    }
    return changed;
}

static int flow_mark_block_copy_reads(RxcpFlowProcedure *procedure,
                                      RxcpFlowBlock *block,
                                      const int *state) {
    ASTNode *child;
    int changed = 0;
    if (!block || !block->anchor) return 0;
    if (block->anchor->node_type == REPEAT) {
        /* The synthetic counted-loop latch has no standalone AST read to
         * retarget. Its implicit control-variable read/write is modelled in
         * the block facts and emitted by the existing DO lowering. */
        return 0;
    }
    if (block->anchor->node_type != DO) {
        return flow_mark_copy_reads(procedure, block, block->anchor, state);
    }

    /* A DO header block owns only its condition/control expressions. The
     * body and counted-loop initializer have distinct CFG blocks and must be
     * visited with their own incoming copy states. */
    for (child = block->anchor->child; child; child = child->sibling) {
        if (child->node_type == INSTRUCTIONS) continue;
        if (child->node_type == REPEAT) {
            ASTNode *part;
            for (part = child->child; part; part = part->sibling) {
                if (part == child->child && part->node_type == ASSIGN) continue;
                changed |= flow_mark_copy_reads(procedure, block, part, state);
            }
        } else {
            changed |= flow_mark_copy_reads(procedure, block, child, state);
        }
    }
    return changed;
}

static void flow_rebuild_effective_uses(RxcpFlowProcedure *procedure,
                                        RxcpFlowBlock *block,
                                        ASTNode *node) {
    ASTNode *child;
    Symbol *symbol;
    int value;
    if (!node) return;
    if (node != block->anchor &&
        (node->node_type == PROCEDURE || node->node_type == METHOD ||
         node->node_type == FACTORY || node->node_type == MATCH)) return;
    symbol = flow_variable_symbol(node);
    if (symbol && flow_node_reads_variable(node)) {
        Symbol *effective = node->flow_substitute_symbol ?
                            node->flow_substitute_symbol : symbol;
        value = flow_value_index_for_symbol(procedure, effective);
        if (value >= 0) bits_set(&block->uses, (size_t)value);
    }
    for (child = node->child; child; child = child->sibling) {
        flow_rebuild_effective_uses(procedure, block, child);
        value = flow_value_index_for_temp(procedure, child);
        if (value >= 0) bits_set(&block->uses, (size_t)value);
    }
}

static void flow_rebuild_block_effective_uses(RxcpFlowProcedure *procedure,
                                              RxcpFlowBlock *block) {
    ASTNode *child;
    if (!block || !block->anchor) return;
    if (block->anchor->node_type == REPEAT) {
        ASTNode *initial = block->anchor->child;
        Symbol *symbol = initial && initial->node_type == ASSIGN ?
                         flow_variable_symbol(initial->child) : 0;
        int value = flow_value_index_for_symbol(procedure, symbol);
        if (value >= 0) bits_set(&block->uses, (size_t)value);
        return;
    }
    if (block->anchor->node_type != DO) {
        flow_rebuild_effective_uses(procedure, block, block->anchor);
        return;
    }

    for (child = block->anchor->child; child; child = child->sibling) {
        if (child->node_type == INSTRUCTIONS) continue;
        if (child->node_type == REPEAT) {
            ASTNode *part;
            for (part = child->child; part; part = part->sibling) {
                if (part == child->child && part->node_type == ASSIGN) continue;
                flow_rebuild_effective_uses(procedure, block, part);
            }
        } else {
            flow_rebuild_effective_uses(procedure, block, child);
        }
    }
}

static int flow_apply_copy_propagation(RxcpFlowProcedure *procedure,
                                       int *markers_changed) {
    int *in_state;
    int *out_state;
    int *incoming;
    int *outgoing;
    size_t cells;
    size_t i;
    size_t b;
    size_t iterations = 0;
    size_t limit;
    int changed;
    int marked = 0;
    if (markers_changed) *markers_changed = 0;
    if (!procedure->value_count || !procedure->block_count) return 1;
    if (procedure->block_count > SIZE_MAX / procedure->value_count) return 0;
    cells = procedure->block_count * procedure->value_count;
    in_state = malloc(cells * sizeof(*in_state));
    out_state = malloc(cells * sizeof(*out_state));
    incoming = malloc(procedure->value_count * sizeof(*incoming));
    outgoing = malloc(procedure->value_count * sizeof(*outgoing));
    if (!in_state || !out_state || !incoming || !outgoing) {
        free(in_state); free(out_state); free(incoming); free(outgoing);
        return 0;
    }
    for (i = 0; i < cells; i++) in_state[i] = out_state[i] = FLOW_COPY_TOP;
    for (i = 0; i < procedure->value_count; i++) {
        in_state[(size_t)procedure->entry_block * procedure->value_count + i] = FLOW_COPY_NONE;
        out_state[(size_t)procedure->entry_block * procedure->value_count + i] = FLOW_COPY_NONE;
    }
    limit = procedure->block_count * (procedure->value_count + 1u) * 4u + 4u;
    do {
        changed = 0;
        for (b = 0; b < procedure->block_count; b++) {
            RxcpFlowBlock *block = &procedure->blocks[b];
            int *block_in = in_state + b * procedure->value_count;
            int *block_out = out_state + b * procedure->value_count;
            size_t p;
            int have_predecessor = 0;
            if (!block->reachable) continue;
            for (i = 0; i < procedure->value_count; i++) {
                incoming[i] = (int)b == procedure->entry_block ?
                              FLOW_COPY_NONE : FLOW_COPY_TOP;
            }
            if ((int)b != procedure->entry_block) {
                for (p = 0; p < block->predecessor_count; p++) {
                    RxcpFlowBlock *pred = &procedure->blocks[block->predecessors[p]];
                    int *pred_out;
                    if (!pred->reachable) continue;
                    pred_out = out_state + (size_t)pred->id * procedure->value_count;
                    for (i = 0; i < procedure->value_count; i++) {
                        incoming[i] = flow_copy_meet(incoming[i], pred_out[i]);
                    }
                    have_predecessor = 1;
                }
                if (!have_predecessor) {
                    for (i = 0; i < procedure->value_count; i++) incoming[i] = FLOW_COPY_NONE;
                }
            }
            flow_copy_transfer(procedure, block, incoming, outgoing);
            if (memcmp(block_in, incoming,
                       procedure->value_count * sizeof(*incoming)) != 0 ||
                memcmp(block_out, outgoing,
                       procedure->value_count * sizeof(*outgoing)) != 0) {
                memcpy(block_in, incoming, procedure->value_count * sizeof(*incoming));
                memcpy(block_out, outgoing, procedure->value_count * sizeof(*outgoing));
                changed = 1;
            }
        }
        iterations++;
    } while (changed && iterations < limit);
    if (changed) {
        free(in_state); free(out_state); free(incoming); free(outgoing);
        /* This optional consumer fails closed without invalidating the base
         * flow overlay or the compilation. No AST marker has been applied. */
        return 1;
    }
    for (b = 0; b < procedure->block_count; b++) {
        RxcpFlowBlock *block = &procedure->blocks[b];
        if (!block->reachable || (int)b == procedure->entry_block ||
            (int)b == procedure->exit_block) continue;
        marked |= flow_mark_block_copy_reads(procedure, block,
                                             in_state + b * procedure->value_count);
    }
    for (b = 0; b < procedure->block_count; b++) {
        RxcpFlowBlock *block = &procedure->blocks[b];
        bits_clear(&block->uses);
        bits_clear(&block->live_in);
        bits_clear(&block->live_out);
        if (block->reachable && (int)b != procedure->entry_block &&
            (int)b != procedure->exit_block) {
            flow_rebuild_block_effective_uses(procedure, block);
        }
    }
    free(in_state); free(out_state); free(incoming); free(outgoing);
    if (markers_changed) *markers_changed = marked;
    return flow_run_liveness(procedure);
}

static int flow_assignment_store_candidate(RxcpFlowProcedure *procedure,
                                           ASTNode *node,
                                           int *value_out) {
    ASTNode *target;
    Symbol *symbol;
    int value;
    if (value_out) *value_out = FLOW_COPY_NONE;
    if (!node || node->node_type != ASSIGN || !node->parent ||
        node->parent->node_type == REPEAT) return 0;
    target = node->child;
    if (!target || target->child || !target->sibling) return 0;
    symbol = flow_variable_symbol(target);
    value = flow_value_index_for_symbol(procedure, symbol);
    if (value < 0 || !flow_copy_value_candidate(&procedure->values[value])) return 0;
    if (value_out) *value_out = value;
    return 1;
}

static int flow_apply_dead_assignment_stores(RxcpFlowProcedure *procedure) {
    size_t b;
    int changed = 0;
    for (b = 0; b < procedure->block_count; b++) {
        RxcpFlowBlock *block = &procedure->blocks[b];
        int value;
        if (!block->reachable ||
            !flow_assignment_store_candidate(procedure, block->anchor, &value)) continue;
        if (!bits_has(&block->live_out, (size_t)value) &&
            !bits_has(&block->opaque, (size_t)value)) {
            if (!block->anchor->flow_skip_assignment_store) {
                block->anchor->flow_skip_assignment_store = 1;
                changed = 1;
            }
        }
    }
    return changed;
}

static size_t flow_count_symbol_writes(ASTNode *node, Symbol *symbol) {
    ASTNode *child;
    size_t count = 0;
    if (!node) return 0;
    if (flow_variable_symbol(node) == symbol && node->symbolNode->writeUsage) count++;
    for (child = node->child; child; child = child->sibling) {
        count += flow_count_symbol_writes(child, symbol);
    }
    return count;
}

static int flow_arg_can_share_input(RxcpFlowProcedure *procedure,
                                    ASTNode *arg,
                                    Symbol *symbol,
                                    int value) {
    size_t b;
    int saw_write = 0;
    if (!arg || !symbol || value < 0 || arg->is_ref_arg || arg->is_opt_arg ||
        arg->is_const_arg || !flow_copy_value_candidate(&procedure->values[value])) return 0;
    for (b = 0; b < procedure->block_count; b++) {
        RxcpFlowBlock *block = &procedure->blocks[b];
        int target = FLOW_COPY_NONE;
        if (!block->reachable) continue;
        if (bits_has(&block->opaque, (size_t)value)) return 0;
        if (!bits_has(&block->defs, (size_t)value)) continue;
        saw_write = 1;
        if (!block->anchor || !block->anchor->flow_skip_assignment_store ||
            !flow_copy_assignment_values(procedure, block->anchor, &target, 0) ||
            target != value || flow_count_symbol_writes(block->anchor, symbol) != 1) {
            return 0;
        }
    }
    return saw_write;
}

static void flow_reset_node_marks(RxcpFlowProcedure *procedure, ASTNode *node) {
    ASTNode *child;
    if (!node) return;
    if (node != procedure->node &&
        (node->node_type == PROCEDURE || node->node_type == METHOD ||
         node->node_type == FACTORY || node->node_type == MATCH)) return;
    node->flow_skip_assignment_store = 0;
    node->flow_substitute_symbol = 0;
    for (child = node->child; child; child = child->sibling) {
        flow_reset_node_marks(procedure, child);
    }
}

static void flow_prepare_values(RxcpFlowProcedure *procedure) {
    ASTNode *args;
    ASTNode *arg;
    size_t i;
    for (i = 0; i < procedure->value_count; i++) {
        RxcpFlowValue *value = &procedure->values[i];
        value->single_use = value->reads == 1;
        if (value->symbol) value->symbol->flow_skip_default_initiation = 0;
    }
    args = flow_direct_child(procedure->node, ARGS);
    for (arg = args ? args->child : 0; arg; arg = arg->sibling) {
        arg->flow_skip_arg_copy = 0;
        arg->flow_share_arg_input = 0;
    }
    flow_reset_node_marks(procedure, procedure->node);
}

static int flow_apply_transforms(RxcpFlowProcedure *procedure) {
    ASTNode *args;
    ASTNode *arg;
    size_t i;
    size_t copy_pass = 0;
    size_t copy_limit = procedure->block_count + 2u;
    int markers_changed;
    int stores_changed;

    /* Copy-use rewrites can expose a dead exact copy store; omitting that
     * physical write can in turn make an earlier equality usable. Iterate the
     * two monotone marker sets to a bounded fixed point. */
    do {
        markers_changed = 0;
        if (!flow_apply_copy_propagation(procedure, &markers_changed)) return 0;
        stores_changed = flow_apply_dead_assignment_stores(procedure);
        copy_pass++;
    } while ((markers_changed || stores_changed) && copy_pass < copy_limit);

    for (i = 0; i < procedure->value_count; i++) {
        RxcpFlowValue *value = &procedure->values[i];
        Symbol *symbol = value->symbol;
        if (!flow_default_candidate(symbol)) continue;
        if (flow_must_safe_write_before_read(procedure, i)) {
            symbol->flow_skip_default_initiation = 1;
        }
    }

    args = flow_direct_child(procedure->node, ARGS);
    for (arg = args ? args->child : 0; arg; arg = arg->sibling) {
        ASTNode *target = arg->child;
        Symbol *symbol = flow_variable_symbol(target);
        int value = flow_value_index_for_symbol(procedure, symbol);
        if (value >= 0 && flow_arg_candidate(arg, symbol)) {
            if (flow_arg_can_share_input(procedure, arg, symbol, value)) {
                arg->flow_share_arg_input = 1;
            } else if (flow_must_safe_write_before_read(procedure, (size_t)value)) {
                arg->flow_skip_arg_copy = 1;
            }
        }
    }
    return 1;
}

static void flow_block_free(RxcpFlowBlock *block) {
    free(block->successors);
    free(block->predecessors);
    bits_free(&block->uses);
    bits_free(&block->defs);
    bits_free(&block->safe_defs);
    bits_free(&block->opaque);
    bits_free(&block->live_in);
    bits_free(&block->live_out);
    bits_free(&block->definite_in);
    bits_free(&block->definite_out);
    bits_free(&block->reaching_gen);
    bits_free(&block->reaching_kill);
    bits_free(&block->reaching_in);
    bits_free(&block->reaching_out);
}

static void flow_procedure_free(RxcpFlowProcedure *procedure) {
    size_t i;
    for (i = 0; i < procedure->block_count; i++) flow_block_free(&procedure->blocks[i]);
    free(procedure->blocks);
    free(procedure->values);
    free(procedure->definitions);
    memset(procedure, 0, sizeof(*procedure));
}

void rxcp_flow_free(Context *context) {
    RxcpFlowProgram *program;
    size_t i;
    if (!context || !context->flow_program) return;
    program = context->flow_program;
    for (i = 0; i < program->procedure_count; i++) {
        flow_procedure_free(&program->procedures[i]);
    }
    free(program->procedures);
    free(program);
    context->flow_program = 0;
}

static RxcpFlowProcedure *flow_program_add_procedure(RxcpFlowProgram *program,
                                                     ASTNode *node) {
    RxcpFlowProcedure *procedure;
    if (program->procedure_count == program->procedure_capacity) {
        size_t capacity = program->procedure_capacity ? program->procedure_capacity * 2u : 32u;
        RxcpFlowProcedure *procedures = realloc(program->procedures,
                                                capacity * sizeof(*procedures));
        if (!procedures) return 0;
        program->procedures = procedures;
        program->procedure_capacity = capacity;
    }
    procedure = &program->procedures[program->procedure_count++];
    memset(procedure, 0, sizeof(*procedure));
    procedure->node = node;
    procedure->scope = node->scope;
    if (node->scope) procedure->num_context = node->scope->num_context;
    return procedure;
}

static int flow_build_procedure(RxcpFlowProcedure *procedure, int apply_transforms) {
    FlowBuilder builder;
    ASTNode *body;
    int body_entry;

    if (!procedure->scope ||
        !flow_collect_scope_values(procedure, procedure->scope) ||
        !flow_collect_temporaries(procedure, procedure->node)) return 0;

    procedure->entry_block = flow_new_block(procedure, procedure->node);
    procedure->exit_block = flow_new_block(procedure, procedure->node);
    if (procedure->entry_block < 0 || procedure->exit_block < 0) return 0;
    body = flow_direct_child(procedure->node, INSTRUCTIONS);
    builder.procedure = procedure;
    body_entry = body ? flow_build_sequence(&builder, body->child,
                                            procedure->exit_block, 0) :
                        procedure->exit_block;
    if (body_entry < 0 || !flow_add_edge(procedure, procedure->entry_block, body_entry)) return 0;

    if (!flow_mark_reachable(procedure) ||
        !flow_run_liveness(procedure) ||
        !flow_run_definite_assignment(procedure) ||
        !flow_build_definitions(procedure) ||
        !flow_run_reaching_definitions(procedure)) return 0;
    flow_prepare_values(procedure);
    if (apply_transforms && !flow_apply_transforms(procedure)) return 0;
    return 1;
}

static int flow_walk_procedures(RxcpFlowProgram *program,
                                ASTNode *node,
                                int apply_transforms) {
    ASTNode *child;
    if (!node) return 1;
    if (node->node_type == PROCEDURE || node->node_type == METHOD ||
        node->node_type == FACTORY || node->node_type == MATCH) {
        ASTNode *body = flow_direct_child(node, INSTRUCTIONS);
        if (body) {
            RxcpFlowProcedure *procedure = flow_program_add_procedure(program, node);
            if (!procedure || !flow_build_procedure(procedure, apply_transforms)) return 0;
        }
        return 1;
    }
    for (child = node->child; child; child = child->sibling) {
        if (!flow_walk_procedures(program, child, apply_transforms)) return 0;
    }
    return 1;
}

int rxcp_flow_analyze(Context *context, int apply_transforms) {
    RxcpFlowProgram *program;
    if (!context || !context->ast) return 0;
    rxcp_flow_free(context);
    program = calloc(1, sizeof(*program));
    if (!program) return 0;
    context->flow_program = program;
    if (!flow_walk_procedures(program, context->ast, apply_transforms)) {
        rxcp_flow_free(context);
        return 0;
    }
    return 1;
}
