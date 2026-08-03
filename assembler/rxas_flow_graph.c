/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

/* Immutable, per-procedure RXAS control-flow graph construction. */

#include "rxas_flow_graph_internal.h"
#include "rxas_flow_analysis.h"
#include "rxasassm.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static OperandType flow_graph_operand_type(const Assembler_Token *token) {
    if (!token) return OP_NONE;
    switch (token->token_type) {
        case ID: return OP_ID;
        case RREG:
        case GREG:
        case AREG: return OP_REG;
        case FUNC: return OP_FUNC;
        case INT: return OP_INT;
        case FLOAT: return OP_FLOAT;
        case CHAR: return OP_CHAR;
        case STRING: return OP_STRING;
        case DECIMAL: return OP_DECIMAL;
        case HEX: return OP_BINARY;
        default: return OP_NONE;
    }
}

static int flow_graph_mnemonic_matches(const char *mnemonic,
                                       const char *table_name) {
    size_t index;
    if (!mnemonic || !table_name) return 0;
    index = 0;
    while (mnemonic[index]) {
        if (toupper((unsigned char)mnemonic[index]) != table_name[index])
            return 0;
        index++;
    }
    return table_name[index] == 0 || table_name[index] == '_';
}

const OpInfo *rxas_flow_resolve_opcode(Assembler_Context *context,
                                       const instruction_queue *item) {
    const char *mnemonic;
    size_t operand_index;
    int table_index;
    int matches;

    if (!item || item->instrType != OP_CODE || !item->instrToken) return 0;
    mnemonic = (const char *)item->instrToken->token_value.string;
    for (table_index = 0; op_table[table_index].mnemonic; table_index++) {
        if (!rxop_is_source_mnemonic(op_table[table_index].mnemonic)) continue;
        if (!flow_graph_mnemonic_matches(mnemonic,
                                         op_table[table_index].mnemonic))
            continue;
        if (rxop_format_operand_count(op_table[table_index].format) !=
            item->operandCount)
            continue;
        matches = 1;
        for (operand_index = 0; operand_index < item->operandCount;
             operand_index++) {
            OperandType expected;
            OperandType actual;
            Assembler_Token *operand;
            size_t jump_table_cases;
            expected = rxop_format_operand_type(op_table[table_index].format,
                                                operand_index);
            operand = rxas_queue_operand(item, operand_index);
            actual = flow_graph_operand_type(operand);
            if (expected != actual &&
                !(expected == OP_BINARY && actual == OP_ID &&
                  rxas_jump_table_case_count(context, operand,
                                             &jump_table_cases))) {
                matches = 0;
                break;
            }
        }
        if (matches) return &op_table[table_index];
    }
    return 0;
}

static char *flow_graph_copy_string(const char *source) {
    char *copy;
    size_t length;
    if (!source) source = "(directives)";
    length = strlen(source);
    copy = malloc(length + 1);
    if (!copy) return 0;
    memcpy(copy, source, length + 1);
    return copy;
}

static size_t flow_graph_hash(const char *text) {
    size_t hash;
    const unsigned char *cursor;
    hash = (size_t)2166136261u;
    cursor = (const unsigned char *)text;
    while (*cursor) {
        hash ^= (size_t)*cursor++;
        hash *= (size_t)16777619u;
    }
    return hash;
}

static size_t flow_graph_label_capacity(size_t labels) {
    size_t capacity;
    capacity = 8;
    while (capacity < labels * 2 + 1) {
        if (capacity > ((size_t)-1) / 2) return 0;
        capacity *= 2;
    }
    return capacity;
}

static int flow_graph_insert_label(RxasFlowProcedure *procedure,
                                   const char *name, size_t record_id) {
    size_t slot;
    size_t mask;
    if (!procedure || !name || !procedure->label_capacity) return 0;
    mask = procedure->label_capacity - 1;
    slot = flow_graph_hash(name) & mask;
    while (procedure->labels[slot].name) {
        if (strcmp(procedure->labels[slot].name, name) == 0) return 0;
        slot = (slot + 1) & mask;
    }
    procedure->labels[slot].name = name;
    procedure->labels[slot].record_id = record_id;
    return 1;
}

static size_t flow_graph_find_label(const RxasFlowProcedure *procedure,
                                    const Assembler_Token *token) {
    const char *name;
    size_t slot;
    size_t start;
    size_t mask;
    if (!procedure || !token || !procedure->label_capacity ||
        (token->token_type != ID && token->token_type != LABEL))
        return RXAS_FLOW_ID_NONE;
    name = (const char *)token->token_value.string;
    mask = procedure->label_capacity - 1;
    slot = flow_graph_hash(name) & mask;
    start = slot;
    while (procedure->labels[slot].name) {
        if (strcmp(procedure->labels[slot].name, name) == 0)
            return procedure->labels[slot].record_id;
        slot = (slot + 1) & mask;
        if (slot == start) break;
    }
    return RXAS_FLOW_ID_NONE;
}

static size_t flow_graph_next_active(const RxasFlowProcedure *procedure,
                                     size_t record_id) {
    size_t index;
    if (!procedure) return RXAS_FLOW_ID_NONE;
    for (index = record_id; index < procedure->item_count; index++) {
        if (procedure->records[index].type != EMPTY) return index;
    }
    return RXAS_FLOW_ID_NONE;
}

static int flow_graph_collect_records(RxasFlowProcedure *procedure,
                                      Assembler_Context *context,
                                      const OpInfo *const *resolved_ops) {
    size_t index;
    size_t instruction_count;
    size_t label_count;
    size_t emitted_address;

    instruction_count = 0;
    label_count = 0;
    for (index = 0; index < procedure->item_count; index++) {
        if (procedure->items[index].instrType == OP_CODE) instruction_count++;
        if (procedure->items[index].instrType == ASM_LABEL) label_count++;
    }
    procedure->records = calloc(procedure->item_count ? procedure->item_count : 1,
                                sizeof(*procedure->records));
    procedure->instructions = calloc(instruction_count ? instruction_count : 1,
                                     sizeof(*procedure->instructions));
    procedure->label_capacity = flow_graph_label_capacity(label_count);
    if (!procedure->label_capacity) return 0;
    procedure->labels = calloc(procedure->label_capacity,
                               sizeof(*procedure->labels));
    if (!procedure->records || !procedure->instructions || !procedure->labels)
        return 0;

    emitted_address = context ? context->binary.inst_size : 0;
    instruction_count = 0;
    for (index = 0; index < procedure->item_count; index++) {
        const instruction_queue *item;
        RxasFlowRecord *record;
        item = &procedure->items[index];
        record = &procedure->records[index];
        record->id = index;
        record->instruction_id = RXAS_FLOW_ID_NONE;
        record->block_id = RXAS_FLOW_ID_NONE;
        record->emitted_address = emitted_address;
        record->type = item->instrType;
        record->queue_record = item;
        record->source_token = item->instrToken;
        if (item->instrType == ASM_LABEL && item->instrToken) {
            if (!flow_graph_insert_label(
                    procedure,
                    (const char *)item->instrToken->token_value.string,
                    index))
                procedure->metrics.complete_control_flow = 0;
        }
        if (item->instrType == OP_CODE) {
            RxasFlowInstruction *instruction;
            const OpInfo *op;
            instruction = &procedure->instructions[instruction_count];
            op = resolved_ops ? resolved_ops[index]
                              : rxas_flow_resolve_opcode(context, item);
            instruction->id = instruction_count;
            instruction->record_id = index;
            instruction->block_id = RXAS_FLOW_ID_NONE;
            instruction->emitted_address = emitted_address;
            instruction->op = op;
            if (op) {
                instruction->effects = rxop_effects(op->opcode);
                instruction->signal = rxop_signal_contract(op->opcode);
            }
            else {
                instruction->effects = rxop_effects(-1);
                instruction->signal = rxop_signal_contract(-1);
                procedure->metrics.complete_control_flow = 0;
            }
            record->instruction_id = instruction_count++;
            emitted_address += item->operandCount + 1;
        }
    }
    procedure->metrics.records = procedure->item_count;
    procedure->metrics.instructions = instruction_count;
    procedure->metrics.labels = label_count;
    return 1;
}

static Assembler_Token *flow_graph_indirect_table_operand(
        Assembler_Context *context, const RxasFlowInstruction *instruction,
        const instruction_queue *item, size_t *case_count) {
    size_t operand_index;
    if (!instruction || !instruction->op || !item) return 0;
    for (operand_index = 0; operand_index < item->operandCount;
         operand_index++) {
        Assembler_Token *operand;
        operand = rxas_queue_operand(item, operand_index);
        if (rxop_format_operand_type(instruction->op->format, operand_index) ==
                OP_BINARY &&
            operand && operand->token_type == ID &&
            rxas_jump_table_case_count(context, operand, case_count))
            return operand;
    }
    return 0;
}

static void flow_graph_mark_target_leader(RxasFlowProcedure *procedure,
                                          unsigned char *leaders,
                                          const Assembler_Token *target) {
    size_t target_record;
    target_record = flow_graph_find_label(procedure, target);
    if (target_record == RXAS_FLOW_ID_NONE) {
        procedure->metrics.complete_control_flow = 0;
        return;
    }
    leaders[target_record] = 1;
}

static int flow_graph_mark_leaders(RxasFlowProcedure *procedure,
                                   Assembler_Context *context,
                                   unsigned char *leaders) {
    size_t index;
    size_t first;
    first = flow_graph_next_active(procedure, 0);
    if (first != RXAS_FLOW_ID_NONE) leaders[first] = 1;
    for (index = 0; index < procedure->item_count; index++) {
        const RxasFlowRecord *record;
        const RxasFlowInstruction *instruction;
        const instruction_queue *item;
        size_t operand_index;
        size_t next;
        int terminates;
        record = &procedure->records[index];
        item = &procedure->items[index];
        if (record->type == ASM_LABEL) leaders[index] = 1;
        if (record->instruction_id == RXAS_FLOW_ID_NONE) continue;
        instruction = &procedure->instructions[record->instruction_id];
        if (!instruction->op) {
            next = flow_graph_next_active(procedure, index + 1);
            if (next != RXAS_FLOW_ID_NONE) leaders[next] = 1;
            continue;
        }
        for (operand_index = 0; operand_index < item->operandCount;
             operand_index++) {
            if (rxop_effect_branch_target_operand(&instruction->effects,
                                                  operand_index))
                flow_graph_mark_target_leader(
                        procedure, leaders,
                        rxas_queue_operand(item, operand_index));
        }
        if (instruction->effects.semantics & RXOP_SEM_INDIRECT_BRANCH) {
            Assembler_Token *table;
            size_t case_count;
            size_t case_index;
            table = flow_graph_indirect_table_operand(context, instruction,
                                                       item, &case_count);
            if (!table) procedure->metrics.complete_control_flow = 0;
            else {
                for (case_index = 0; case_index < case_count; case_index++)
                    flow_graph_mark_target_leader(
                            procedure, leaders,
                            rxas_jump_table_case_label(context, table,
                                                       case_index));
                    }
        }
        /* Make every potentially signalling instruction an exact CFG leader
         * so its failure-phase state and skip address are modelled at the
         * instruction rather than approximated at a larger block boundary. */
        if (instruction->signal.state != RXOP_SIGNAL_STATE_NONE) {
            size_t leader;
            size_t scan;
            leader = index;
            scan = index;
            /* Source/TRACE records between instructions describe the
             * following instruction and stay in its exact signal block. */
            while (scan > 0 &&
                   procedure->records[scan - 1].type != OP_CODE &&
                   procedure->records[scan - 1].type != ASM_LABEL) {
                scan--;
                if (procedure->records[scan].type != EMPTY) leader = scan;
            }
            leaders[leader] = 1;
        }
        terminates = instruction->op->flow != FLOW_NEXT ||
                     instruction->signal.state != RXOP_SIGNAL_STATE_NONE ||
                     (instruction->effects.semantics & RXOP_SEM_CALL) != 0;
        if (terminates) {
            next = flow_graph_next_active(procedure, index + 1);
            if (next != RXAS_FLOW_ID_NONE) leaders[next] = 1;
        }
    }
    return 1;
}

static size_t flow_graph_add_synthetic_block(RxasFlowProcedure *procedure,
                                             size_t block_id,
                                             RxasFlowBlockKind kind) {
    RxasFlowBlock *block;
    block = &procedure->blocks[block_id];
    block->id = block_id;
    block->kind = kind;
    block->first_record = RXAS_FLOW_ID_NONE;
    block->last_record = RXAS_FLOW_ID_NONE;
    block->first_instruction = RXAS_FLOW_ID_NONE;
    block->last_instruction = RXAS_FLOW_ID_NONE;
    return block_id;
}

static int flow_graph_form_blocks(RxasFlowProcedure *procedure,
                                  const unsigned char *leaders) {
    size_t index;
    size_t code_blocks;
    size_t current_block;
    size_t first_active;
    code_blocks = 0;
    for (index = 0; index < procedure->item_count; index++) {
        if (procedure->records[index].type != EMPTY && leaders[index])
            code_blocks++;
    }
    procedure->blocks = calloc(code_blocks + 7, sizeof(*procedure->blocks));
    if (!procedure->blocks) return 0;
    current_block = RXAS_FLOW_ID_NONE;
    for (index = 0; index < procedure->item_count; index++) {
        RxasFlowRecord *record;
        record = &procedure->records[index];
        if (record->type != EMPTY && leaders[index]) {
            RxasFlowBlock *block;
            if (current_block != RXAS_FLOW_ID_NONE)
                procedure->blocks[current_block].last_record = index - 1;
            current_block = current_block == RXAS_FLOW_ID_NONE
                    ? 0 : current_block + 1;
            block = &procedure->blocks[current_block];
            block->id = current_block;
            block->kind = RXAS_FLOW_BLOCK_CODE;
            block->first_record = index;
            block->last_record = procedure->item_count
                    ? procedure->item_count - 1 : RXAS_FLOW_ID_NONE;
            block->first_instruction = RXAS_FLOW_ID_NONE;
            block->last_instruction = RXAS_FLOW_ID_NONE;
        }
        if (current_block != RXAS_FLOW_ID_NONE) {
            record->block_id = current_block;
            if (record->instruction_id != RXAS_FLOW_ID_NONE) {
                RxasFlowBlock *block;
                RxasFlowInstruction *instruction;
                block = &procedure->blocks[current_block];
                instruction = &procedure->instructions[record->instruction_id];
                instruction->block_id = current_block;
                if (block->first_instruction == RXAS_FLOW_ID_NONE)
                    block->first_instruction = record->instruction_id;
                block->last_instruction = record->instruction_id;
            }
        }
    }
    first_active = flow_graph_next_active(procedure, 0);
    if (code_blocks && first_active != RXAS_FLOW_ID_NONE) {
        for (index = 0; index < first_active; index++)
            procedure->records[index].block_id = 0;
    }
    procedure->metrics.code_blocks = code_blocks;
    procedure->entry_block = flow_graph_add_synthetic_block(
            procedure, code_blocks, RXAS_FLOW_BLOCK_ENTRY);
    procedure->handler_root = flow_graph_add_synthetic_block(
            procedure, code_blocks + 1, RXAS_FLOW_BLOCK_HANDLER_ROOT);
    procedure->async_root = flow_graph_add_synthetic_block(
            procedure, code_blocks + 2, RXAS_FLOW_BLOCK_ASYNC_ROOT);
    procedure->normal_exit = flow_graph_add_synthetic_block(
            procedure, code_blocks + 3, RXAS_FLOW_BLOCK_NORMAL_EXIT);
    procedure->unwind_exit = flow_graph_add_synthetic_block(
            procedure, code_blocks + 4, RXAS_FLOW_BLOCK_UNWIND_EXIT);
    procedure->terminal_exit = flow_graph_add_synthetic_block(
            procedure, code_blocks + 5, RXAS_FLOW_BLOCK_TERMINAL_EXIT);
    procedure->unknown_exit = flow_graph_add_synthetic_block(
            procedure, code_blocks + 6, RXAS_FLOW_BLOCK_UNKNOWN_EXIT);
    procedure->metrics.blocks = code_blocks + 7;
    return 1;
}

static int flow_graph_add_edge(RxasFlowProcedure *procedure, size_t source,
                               size_t target, RxasFlowEdgeKind kind) {
    size_t new_capacity;
    RxasFlowEdge *new_edges;
    if (source >= procedure->metrics.blocks ||
        target >= procedure->metrics.blocks)
        return 0;
    if (procedure->metrics.edges == procedure->edge_capacity) {
        new_capacity = procedure->edge_capacity
                ? procedure->edge_capacity * 2 : 32;
        new_edges = realloc(procedure->edges,
                            new_capacity * sizeof(*new_edges));
        if (!new_edges) return 0;
        procedure->edges = new_edges;
        procedure->edge_capacity = new_capacity;
    }
    procedure->edges[procedure->metrics.edges].source = source;
    procedure->edges[procedure->metrics.edges].target = target;
    procedure->edges[procedure->metrics.edges].kind = kind;
    procedure->metrics.edges++;
    return 1;
}

static size_t flow_graph_target_block(const RxasFlowProcedure *procedure,
                                      const Assembler_Token *target) {
    size_t record_id;
    record_id = flow_graph_find_label(procedure, target);
    if (record_id == RXAS_FLOW_ID_NONE) return RXAS_FLOW_ID_NONE;
    return procedure->records[record_id].block_id;
}

static int flow_graph_add_label_edge(RxasFlowProcedure *procedure,
                                     size_t source,
                                     const Assembler_Token *target,
                                     RxasFlowEdgeKind kind,
                                     size_t *handler_targets,
                                     size_t *handler_count,
                                     unsigned char *handler_seen) {
    size_t target_block;
    target_block = flow_graph_target_block(procedure, target);
    if (target_block == RXAS_FLOW_ID_NONE) {
        procedure->metrics.unresolved_targets++;
        procedure->metrics.complete_control_flow = 0;
        return flow_graph_add_edge(procedure, source,
                                   procedure->unknown_exit,
                                   RXAS_FLOW_EDGE_UNKNOWN);
    }
    procedure->metrics.resolved_targets++;
    if (handler_targets && handler_count && handler_seen &&
        handler_seen[target_block])
        return 1;
    if (!flow_graph_add_edge(procedure, source, target_block, kind)) return 0;
    if (handler_targets && handler_count && handler_seen) {
        handler_seen[target_block] = 1;
        handler_targets[(*handler_count)++] = target_block;
    }
    return 1;
}

static int flow_graph_add_instruction_edges(
        RxasFlowProcedure *procedure, Assembler_Context *context,
        size_t block_id, const RxasFlowInstruction *instruction,
        size_t next_block, size_t *handler_targets, size_t *handler_count,
        unsigned char *handler_seen) {
    const instruction_queue *item;
    const OpInfo *op;
    unsigned int continuations;
    size_t operand_index;
    size_t target_count;
    int handler_registration;
    int indirect_branch;
    int add_fallthrough;

    item = &procedure->items[instruction->record_id];
    op = instruction->op;
    if (!op) {
        procedure->metrics.complete_control_flow = 0;
        if (!flow_graph_add_edge(procedure, block_id, next_block,
                                 RXAS_FLOW_EDGE_NORMAL) ||
            !flow_graph_add_edge(procedure, block_id, next_block,
                                 RXAS_FLOW_EDGE_SIGNAL_SKIP) ||
            !flow_graph_add_edge(procedure, block_id,
                                 procedure->handler_root,
                                 RXAS_FLOW_EDGE_HANDLER) ||
            !flow_graph_add_edge(procedure, block_id,
                                 procedure->unwind_exit,
                                 RXAS_FLOW_EDGE_UNWIND) ||
            !flow_graph_add_edge(procedure, block_id,
                                 procedure->terminal_exit,
                                 RXAS_FLOW_EDGE_TERMINAL) ||
            !flow_graph_add_edge(procedure, block_id,
                                 procedure->unknown_exit,
                                 RXAS_FLOW_EDGE_UNKNOWN))
            return 0;
        return 1;
    }
    continuations = instruction->signal.continuations;
    target_count = 0;
    indirect_branch =
            (instruction->effects.semantics & RXOP_SEM_INDIRECT_BRANCH) != 0;
    handler_registration =
            (instruction->signal.properties & RXOP_SIGNAL_PROP_POLICY_WRITE) != 0;
    for (operand_index = 0; operand_index < item->operandCount;
         operand_index++) {
        if (!rxop_effect_branch_target_operand(&instruction->effects,
                                               operand_index))
            continue;
        target_count++;
        if (handler_registration) {
            if (!flow_graph_add_label_edge(
                    procedure, procedure->handler_root,
                    rxas_queue_operand(item, operand_index),
                    RXAS_FLOW_EDGE_HANDLER, handler_targets,
                    handler_count, handler_seen))
                return 0;
        }
        else if ((op->flow == FLOW_JUMP || op->flow == FLOW_COND) &&
                 (continuations & RXOP_SIGNAL_CONT_NORMAL)) {
            if (!flow_graph_add_label_edge(
                    procedure, block_id,
                    rxas_queue_operand(item, operand_index),
                    RXAS_FLOW_EDGE_BRANCH, 0, 0, 0))
                return 0;
        }
    }
    if (indirect_branch && (continuations & RXOP_SIGNAL_CONT_NORMAL)) {
        Assembler_Token *table;
        size_t case_count;
        size_t case_index;
        table = flow_graph_indirect_table_operand(context, instruction, item,
                                                   &case_count);
        if (!table) {
            procedure->metrics.unresolved_targets++;
            procedure->metrics.complete_control_flow = 0;
            if (!flow_graph_add_edge(procedure, block_id,
                                     procedure->unknown_exit,
                                     RXAS_FLOW_EDGE_UNKNOWN))
                return 0;
        }
        else {
            for (case_index = 0; case_index < case_count; case_index++) {
                if (!flow_graph_add_label_edge(
                        procedure, block_id,
                        rxas_jump_table_case_label(context, table, case_index),
                        RXAS_FLOW_EDGE_BRANCH, 0, 0, 0))
                    return 0;
            }
            /* Packed jump-table dispatch always has a miss continuation.  It
             * is safe to model only when the following code block is still
             * inside this procedure; falling off the retained stream is an
             * incomplete target, not a normal procedure return. */
            if (next_block < procedure->metrics.code_blocks) {
                if (!flow_graph_add_edge(procedure, block_id, next_block,
                                         RXAS_FLOW_EDGE_NORMAL))
                    return 0;
            }
            else {
                procedure->metrics.unresolved_targets++;
                procedure->metrics.complete_control_flow = 0;
                if (!flow_graph_add_edge(procedure, block_id,
                                         procedure->unknown_exit,
                                         RXAS_FLOW_EDGE_UNKNOWN))
                    return 0;
            }
        }
    }

    add_fallthrough = !indirect_branch &&
                      (op->flow == FLOW_NEXT ||
                       (op->flow == FLOW_COND && target_count < 2));
    if ((continuations & RXOP_SIGNAL_CONT_NORMAL) && add_fallthrough) {
        if (!flow_graph_add_edge(procedure, block_id, next_block,
                                 RXAS_FLOW_EDGE_NORMAL))
            return 0;
    }
    if (op->flow == FLOW_TERM) {
        if (instruction->effects.semantics & RXOP_SEM_RETURN) {
            if (!flow_graph_add_edge(procedure, block_id,
                                     procedure->normal_exit,
                                     RXAS_FLOW_EDGE_NORMAL))
                return 0;
        }
        else if (!flow_graph_add_edge(procedure, block_id,
                                      procedure->terminal_exit,
                                      RXAS_FLOW_EDGE_TERMINAL))
            return 0;
    }
    if (instruction->signal.state != RXOP_SIGNAL_STATE_NONE) {
        if ((continuations & RXOP_SIGNAL_CONT_SKIP) &&
            !flow_graph_add_edge(procedure, block_id, next_block,
                                 RXAS_FLOW_EDGE_SIGNAL_SKIP))
            return 0;
        if ((continuations & RXOP_SIGNAL_CONT_HANDLER) &&
            !flow_graph_add_edge(procedure, block_id,
                                 procedure->handler_root,
                                 RXAS_FLOW_EDGE_HANDLER))
            return 0;
        if ((continuations & RXOP_SIGNAL_CONT_UNWIND) &&
            !flow_graph_add_edge(procedure, block_id,
                                 procedure->unwind_exit,
                                 RXAS_FLOW_EDGE_UNWIND))
            return 0;
        if ((continuations & RXOP_SIGNAL_CONT_TERMINAL) &&
            !flow_graph_add_edge(procedure, block_id,
                                 procedure->terminal_exit,
                                 RXAS_FLOW_EDGE_TERMINAL))
            return 0;
    }
    return 1;
}

static void flow_graph_count_edges(RxasFlowProcedure *procedure) {
    size_t index;
    for (index = 0; index < procedure->metrics.edges; index++) {
        switch (procedure->edges[index].kind) {
            case RXAS_FLOW_EDGE_NORMAL: procedure->metrics.normal_edges++; break;
            case RXAS_FLOW_EDGE_BRANCH: procedure->metrics.branch_edges++; break;
            case RXAS_FLOW_EDGE_SIGNAL_SKIP:
                procedure->metrics.signal_skip_edges++;
                break;
            case RXAS_FLOW_EDGE_HANDLER: procedure->metrics.handler_edges++; break;
            case RXAS_FLOW_EDGE_TERMINAL: procedure->metrics.terminal_edges++; break;
            case RXAS_FLOW_EDGE_UNWIND: procedure->metrics.unwind_edges++; break;
            case RXAS_FLOW_EDGE_UNKNOWN: procedure->metrics.unknown_edges++; break;
        }
    }
}

static int flow_graph_mark_reachable(RxasFlowProcedure *procedure) {
    size_t *outgoing_counts;
    size_t *outgoing_offsets;
    size_t *outgoing_targets;
    size_t *queue;
    size_t head;
    size_t tail;
    size_t roots[3];
    size_t root_index;
    size_t edge_index;
    if (!procedure || !procedure->metrics.blocks) return 1;
    procedure->reachable_blocks = calloc(procedure->metrics.blocks, 1);
    outgoing_counts = calloc(procedure->metrics.blocks,
                             sizeof(*outgoing_counts));
    outgoing_offsets = malloc((procedure->metrics.blocks + 1) *
                              sizeof(*outgoing_offsets));
    outgoing_targets = malloc((procedure->metrics.edges
                               ? procedure->metrics.edges : 1) *
                              sizeof(*outgoing_targets));
    queue = malloc(procedure->metrics.blocks * sizeof(*queue));
    if (!procedure->reachable_blocks || !outgoing_counts ||
        !outgoing_offsets || !outgoing_targets || !queue) {
        free(outgoing_counts);
        free(outgoing_offsets);
        free(outgoing_targets);
        free(queue);
        return 0;
    }
    for (edge_index = 0; edge_index < procedure->metrics.edges;
         edge_index++) {
        const RxasFlowEdge *edge;
        edge = &procedure->edges[edge_index];
        if (edge->source < procedure->metrics.blocks &&
            edge->target < procedure->metrics.blocks)
            outgoing_counts[edge->source]++;
    }
    outgoing_offsets[0] = 0;
    for (root_index = 0; root_index < procedure->metrics.blocks;
         root_index++)
        outgoing_offsets[root_index + 1] = outgoing_offsets[root_index] +
                                            outgoing_counts[root_index];
    memset(outgoing_counts, 0,
           procedure->metrics.blocks * sizeof(*outgoing_counts));
    for (edge_index = 0; edge_index < procedure->metrics.edges;
         edge_index++) {
        const RxasFlowEdge *edge;
        edge = &procedure->edges[edge_index];
        if (edge->source < procedure->metrics.blocks &&
            edge->target < procedure->metrics.blocks)
            outgoing_targets[outgoing_offsets[edge->source] +
                             outgoing_counts[edge->source]++] = edge->target;
    }
    roots[0] = procedure->entry_block;
    roots[1] = procedure->handler_root;
    roots[2] = procedure->async_root;
    head = 0;
    tail = 0;
    for (root_index = 0; root_index < 3; root_index++) {
        size_t root;
        root = roots[root_index];
        if (root >= procedure->metrics.blocks ||
            procedure->reachable_blocks[root])
            continue;
        procedure->reachable_blocks[root] = 1;
        queue[tail++] = root;
    }
    while (head < tail) {
        size_t source;
        size_t outgoing_index;
        source = queue[head++];
        for (outgoing_index = outgoing_offsets[source];
             outgoing_index < outgoing_offsets[source + 1];
             outgoing_index++) {
            size_t target;
            target = outgoing_targets[outgoing_index];
            if (procedure->reachable_blocks[target])
                continue;
            procedure->reachable_blocks[target] = 1;
            queue[tail++] = target;
        }
    }
    free(outgoing_counts);
    free(outgoing_offsets);
    free(outgoing_targets);
    free(queue);
    return 1;
}

static int flow_graph_build_edges(RxasFlowProcedure *procedure,
                                  Assembler_Context *context) {
    size_t block_id;
    size_t *handler_targets;
    unsigned char *handler_seen;
    size_t handler_count;
    handler_targets = calloc(procedure->metrics.labels
                                     ? procedure->metrics.labels : 1,
                             sizeof(*handler_targets));
    handler_seen = calloc(procedure->metrics.code_blocks
                                  ? procedure->metrics.code_blocks : 1,
                          1);
    if (!handler_targets || !handler_seen) {
        free(handler_targets);
        free(handler_seen);
        return 0;
    }
    handler_count = 0;
    if (procedure->metrics.code_blocks) {
        if (!flow_graph_add_edge(procedure, procedure->entry_block, 0,
                                 RXAS_FLOW_EDGE_NORMAL)) {
            free(handler_targets);
            free(handler_seen);
            return 0;
        }
    }
    else if (!flow_graph_add_edge(procedure, procedure->entry_block,
                                  procedure->normal_exit,
                                  RXAS_FLOW_EDGE_NORMAL)) {
        free(handler_targets);
        free(handler_seen);
        return 0;
    }
    for (block_id = 0; block_id < procedure->metrics.code_blocks;
         block_id++) {
        const RxasFlowBlock *block;
        size_t next_block;
        block = &procedure->blocks[block_id];
        next_block = block_id + 1 < procedure->metrics.code_blocks
                ? block_id + 1 : procedure->normal_exit;
        if (block->last_instruction == RXAS_FLOW_ID_NONE) {
            if (!flow_graph_add_edge(procedure, block_id, next_block,
                                     RXAS_FLOW_EDGE_NORMAL)) {
                free(handler_targets);
                free(handler_seen);
                return 0;
            }
        }
        else if (!flow_graph_add_instruction_edges(
                    procedure, context, block_id,
                    &procedure->instructions[block->last_instruction],
                    next_block, handler_targets, &handler_count,
                    handler_seen)) {
            free(handler_targets);
            free(handler_seen);
            return 0;
        }
    }
    for (block_id = 0; block_id < handler_count; block_id++) {
        if (!flow_graph_add_edge(procedure, procedure->async_root,
                                 handler_targets[block_id],
                                 RXAS_FLOW_EDGE_HANDLER)) {
            free(handler_targets);
            free(handler_seen);
            return 0;
        }
    }
    if (!flow_graph_add_edge(procedure, procedure->handler_root,
                             procedure->unwind_exit,
                             RXAS_FLOW_EDGE_UNWIND)) {
        free(handler_targets);
        free(handler_seen);
        return 0;
    }
    free(handler_targets);
    free(handler_seen);
    flow_graph_count_edges(procedure);
    return 1;
}

static RxasFlowProcedure *flow_graph_build_procedure(
        Assembler_Context *context, const instruction_queue *items,
        size_t item_count, unsigned long epoch,
        const OpInfo *const *resolved_ops) {
    RxasFlowProcedure *procedure;
    unsigned char *leaders;
    if (!context || (!items && item_count) || !epoch) return 0;
    procedure = calloc(1, sizeof(*procedure));
    if (!procedure) return 0;
    procedure->epoch = epoch;
    procedure->items = items;
    procedure->item_count = item_count;
    procedure->local_count = context->current_locals > 0
            ? (size_t)context->current_locals : 0;
    procedure->global_count = context->binary.globals > 0
            ? (size_t)context->binary.globals : 0;
    procedure->metrics.complete_control_flow = 1;
    procedure->name = flow_graph_copy_string(context->current_proc_name);
    if (!procedure->name ||
        !flow_graph_collect_records(procedure, context, resolved_ops)) {
        rxas_flow_procedure_destroy(procedure);
        return 0;
    }
    leaders = calloc(item_count ? item_count : 1, 1);
    if (!leaders) {
        rxas_flow_procedure_destroy(procedure);
        return 0;
    }
    if (!flow_graph_mark_leaders(procedure, context, leaders) ||
        !flow_graph_form_blocks(procedure, leaders) ||
        !flow_graph_build_edges(procedure, context) ||
        !flow_graph_mark_reachable(procedure)) {
        free(leaders);
        rxas_flow_procedure_destroy(procedure);
        return 0;
    }
    free(leaders);
    return procedure;
}

RxasFlowProcedure *rxas_flow_procedure_build(Assembler_Context *context,
                                              const instruction_queue *items,
                                              size_t item_count,
                                              unsigned long epoch) {
    return flow_graph_build_procedure(context, items, item_count, epoch, 0);
}

RxasFlowProcedure *rxas_flow_procedure_build_resolved(
        Assembler_Context *context, const instruction_queue *items,
        size_t item_count, unsigned long epoch,
        const OpInfo *const *resolved_ops) {
    if (!resolved_ops && item_count) return 0;
    return flow_graph_build_procedure(context, items, item_count, epoch,
                                      resolved_ops);
}

void rxas_flow_procedure_destroy(RxasFlowProcedure *procedure) {
    if (!procedure) return;
    rxas_flow_analysis_manager_destroy(procedure);
    free(procedure->name);
    free(procedure->records);
    free(procedure->instructions);
    free(procedure->blocks);
    free(procedure->edges);
    free(procedure->reachable_blocks);
    free(procedure->labels);
    memset(procedure, 0, sizeof(*procedure));
    free(procedure);
}

unsigned long rxas_flow_procedure_epoch(const RxasFlowProcedure *procedure) {
    return procedure ? procedure->epoch : 0;
}

int rxas_flow_procedure_epoch_matches(const RxasFlowProcedure *procedure,
                                      unsigned long expected_epoch) {
    return procedure && expected_epoch && procedure->epoch == expected_epoch;
}

const RxasFlowMetrics *rxas_flow_procedure_metrics(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch) {
    if (!rxas_flow_procedure_epoch_matches(procedure, expected_epoch)) return 0;
    return &procedure->metrics;
}

const RxasFlowRecord *rxas_flow_procedure_record(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t record_id) {
    if (!rxas_flow_procedure_epoch_matches(procedure, expected_epoch) ||
        record_id >= procedure->metrics.records)
        return 0;
    return &procedure->records[record_id];
}

int rxas_flow_procedure_pin_queue_record(
        RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t record_id, const instruction_queue *epoch_item) {
    if (!procedure || procedure->epoch != expected_epoch || !epoch_item ||
        record_id >= procedure->item_count)
        return 0;
    procedure->records[record_id].queue_record = epoch_item;
    return 1;
}

int rxas_flow_procedure_rebind_queue_records(
        RxasFlowProcedure *procedure, unsigned long expected_epoch,
        const instruction_queue *items, size_t item_count) {
    size_t record_id;
    if (!procedure || procedure->epoch != expected_epoch || !items ||
        item_count != procedure->item_count)
        return 0;
    for (record_id = 0; record_id < item_count; record_id++)
        procedure->records[record_id].queue_record = &items[record_id];
    return 1;
}

size_t rxas_flow_procedure_label_record(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch,
        const Assembler_Token *label) {
    if (!rxas_flow_procedure_epoch_matches(procedure, expected_epoch))
        return RXAS_FLOW_ID_NONE;
    return flow_graph_find_label(procedure, label);
}

const RxasFlowInstruction *rxas_flow_procedure_instruction(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t instruction_id) {
    if (!rxas_flow_procedure_epoch_matches(procedure, expected_epoch) ||
        instruction_id >= procedure->metrics.instructions)
        return 0;
    return &procedure->instructions[instruction_id];
}

const RxasFlowBlock *rxas_flow_procedure_block(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t block_id) {
    if (!rxas_flow_procedure_epoch_matches(procedure, expected_epoch) ||
        block_id >= procedure->metrics.blocks)
        return 0;
    return &procedure->blocks[block_id];
}

const RxasFlowEdge *rxas_flow_procedure_edge(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t edge_id) {
    if (!rxas_flow_procedure_epoch_matches(procedure, expected_epoch) ||
        edge_id >= procedure->metrics.edges)
        return 0;
    return &procedure->edges[edge_id];
}

int rxas_flow_procedure_block_reachable(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t block_id) {
    return rxas_flow_procedure_epoch_matches(procedure, expected_epoch) &&
           procedure->reachable_blocks && block_id < procedure->metrics.blocks &&
           procedure->reachable_blocks[block_id] != 0;
}

int rxas_flow_procedure_record_reachable(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t record_id) {
    const RxasFlowRecord *record;
    record = rxas_flow_procedure_record(
            procedure, expected_epoch, record_id);
    return record && record->block_id != RXAS_FLOW_ID_NONE &&
           rxas_flow_procedure_block_reachable(
                   procedure, expected_epoch, record->block_id);
}

#define RXAS_FLOW_SPECIAL_ACCESSOR(NAME, FIELD) \
size_t NAME(const RxasFlowProcedure *procedure, unsigned long expected_epoch) { \
    if (!rxas_flow_procedure_epoch_matches(procedure, expected_epoch)) \
        return RXAS_FLOW_ID_NONE; \
    return procedure->FIELD; \
}

RXAS_FLOW_SPECIAL_ACCESSOR(rxas_flow_procedure_entry_block, entry_block)
RXAS_FLOW_SPECIAL_ACCESSOR(rxas_flow_procedure_handler_root, handler_root)
RXAS_FLOW_SPECIAL_ACCESSOR(rxas_flow_procedure_async_root, async_root)
RXAS_FLOW_SPECIAL_ACCESSOR(rxas_flow_procedure_normal_exit, normal_exit)
RXAS_FLOW_SPECIAL_ACCESSOR(rxas_flow_procedure_unwind_exit, unwind_exit)
RXAS_FLOW_SPECIAL_ACCESSOR(rxas_flow_procedure_terminal_exit, terminal_exit)
RXAS_FLOW_SPECIAL_ACCESSOR(rxas_flow_procedure_unknown_exit, unknown_exit)

#undef RXAS_FLOW_SPECIAL_ACCESSOR

static const char *flow_graph_block_kind_name(RxasFlowBlockKind kind) {
    switch (kind) {
        case RXAS_FLOW_BLOCK_CODE: return "code";
        case RXAS_FLOW_BLOCK_ENTRY: return "entry";
        case RXAS_FLOW_BLOCK_HANDLER_ROOT: return "handler-root";
        case RXAS_FLOW_BLOCK_ASYNC_ROOT: return "async-root";
        case RXAS_FLOW_BLOCK_NORMAL_EXIT: return "normal-exit";
        case RXAS_FLOW_BLOCK_UNWIND_EXIT: return "unwind-exit";
        case RXAS_FLOW_BLOCK_TERMINAL_EXIT: return "terminal-exit";
        case RXAS_FLOW_BLOCK_UNKNOWN_EXIT: return "unknown-exit";
    }
    return "invalid";
}

static const char *flow_graph_edge_kind_name(RxasFlowEdgeKind kind) {
    switch (kind) {
        case RXAS_FLOW_EDGE_NORMAL: return "normal";
        case RXAS_FLOW_EDGE_BRANCH: return "branch";
        case RXAS_FLOW_EDGE_SIGNAL_SKIP: return "signal-skip";
        case RXAS_FLOW_EDGE_HANDLER: return "handler";
        case RXAS_FLOW_EDGE_TERMINAL: return "terminal";
        case RXAS_FLOW_EDGE_UNWIND: return "unwind";
        case RXAS_FLOW_EDGE_UNKNOWN: return "unknown";
    }
    return "invalid";
}

static const char *flow_graph_record_kind_name(enum queue_item_type type) {
    switch (type) {
        case EMPTY: return "empty";
        case ASM_LABEL: return "label";
        case OP_CODE: return "instruction";
        case SRC_STEP: return "source-step";
        case FUNC_META: return "function-meta";
        case REG_META: return "register-meta";
        case CONST_META: return "constant-meta";
        case CLEAR_META: return "clear-meta";
        case CLASS_META: return "class-meta";
        case ATTR_META: return "attribute-meta";
        case INTERFACE_META: return "interface-meta";
        case IMPLEMENTS_META: return "implements-meta";
        case MEMBER_META: return "member-meta";
        case INLINE_META: return "inline-meta";
        case TRACE_EVENT: return "trace-event";
    }
    return "invalid";
}

static void flow_graph_dump_id(FILE *stream, size_t id) {
    if (id == RXAS_FLOW_ID_NONE) fputs("-", stream);
    else fprintf(stream, "%llu", (unsigned long long)id);
}

int rxas_flow_procedure_dump(const RxasFlowProcedure *procedure,
                             unsigned long expected_epoch,
                             FILE *stream) {
    size_t index;
    if (!stream ||
        !rxas_flow_procedure_epoch_matches(procedure, expected_epoch))
        return 0;
    fprintf(stream,
            "PERF3 flow-graph procedure=%s epoch=%lu records=%llu "
            "instructions=%llu code-blocks=%llu blocks=%llu edges=%llu "
            "normal=%llu branch=%llu skip=%llu handler=%llu "
            "terminal=%llu unwind=%llu unknown=%llu complete=%d\n",
            procedure->name, procedure->epoch,
            (unsigned long long)procedure->metrics.records,
            (unsigned long long)procedure->metrics.instructions,
            (unsigned long long)procedure->metrics.code_blocks,
            (unsigned long long)procedure->metrics.blocks,
            (unsigned long long)procedure->metrics.edges,
            (unsigned long long)procedure->metrics.normal_edges,
            (unsigned long long)procedure->metrics.branch_edges,
            (unsigned long long)procedure->metrics.signal_skip_edges,
            (unsigned long long)procedure->metrics.handler_edges,
            (unsigned long long)procedure->metrics.terminal_edges,
            (unsigned long long)procedure->metrics.unwind_edges,
            (unsigned long long)procedure->metrics.unknown_edges,
            procedure->metrics.complete_control_flow);
    for (index = 0; index < procedure->metrics.blocks; index++) {
        const RxasFlowBlock *block;
        block = &procedure->blocks[index];
        fprintf(stream, "PERF3 flow-block id=%llu kind=%s records=",
                (unsigned long long)block->id,
                flow_graph_block_kind_name(block->kind));
        flow_graph_dump_id(stream, block->first_record);
        fputc(':', stream);
        flow_graph_dump_id(stream, block->last_record);
        fputs(" instructions=", stream);
        flow_graph_dump_id(stream, block->first_instruction);
        fputc(':', stream);
        flow_graph_dump_id(stream, block->last_instruction);
        fputc('\n', stream);
    }
    for (index = 0; index < procedure->metrics.records; index++) {
        const RxasFlowRecord *record;
        record = &procedure->records[index];
        fprintf(stream,
                "PERF3 flow-record id=%llu kind=%s block=",
                (unsigned long long)record->id,
                flow_graph_record_kind_name(record->type));
        flow_graph_dump_id(stream, record->block_id);
        fputs(" instruction=", stream);
        flow_graph_dump_id(stream, record->instruction_id);
        fprintf(stream, " address=%llu\n",
                (unsigned long long)record->emitted_address);
    }
    for (index = 0; index < procedure->metrics.instructions; index++) {
        const RxasFlowInstruction *instruction;
        instruction = &procedure->instructions[index];
        fprintf(stream,
                "PERF3 flow-instruction id=%llu record=%llu block=%llu "
                "address=%llu opcode=%s signal=%d phase=%d\n",
                (unsigned long long)instruction->id,
                (unsigned long long)instruction->record_id,
                (unsigned long long)instruction->block_id,
                (unsigned long long)instruction->emitted_address,
                instruction->op ? instruction->op->mnemonic : "unknown",
                (int)instruction->signal.state,
                (int)instruction->signal.phase);
    }
    for (index = 0; index < procedure->metrics.edges; index++) {
        const RxasFlowEdge *edge;
        edge = &procedure->edges[index];
        fprintf(stream,
                "PERF3 flow-edge from=%llu to=%llu kind=%s\n",
                (unsigned long long)edge->source,
                (unsigned long long)edge->target,
                flow_graph_edge_kind_name(edge->kind));
    }
    return 1;
}
