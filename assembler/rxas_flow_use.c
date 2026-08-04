/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

/* Cached sparse def-use and component/storage liveness for one immutable
 * RXAS procedure epoch.  Policy stays in the proof service: this module only
 * records observations and write-once value dependencies. */

#include "rxas_flow_use.h"
#include "rxas_flow_graph_internal.h"
#include "rxasassm.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define FLOW_USE_DUMP_ENTRY_LIMIT 256

static const unsigned int flow_use_components[] = {
    RXOP_COMPONENT_INTEGER,
    RXOP_COMPONENT_FLOAT,
    RXOP_COMPONENT_STRING,
    RXOP_COMPONENT_DECIMAL,
    RXOP_COMPONENT_BINARY,
    RXOP_COMPONENT_ATTRIBUTES,
    RXOP_COMPONENT_REFERENCE,
    RXOP_COMPONENT_NATIVE_PAYLOAD,
    RXOP_COMPONENT_ATTRIBUTE_COUNT
};

struct RxasFlowUseAnalysis {
    const RxasFlowProcedure *procedure;
    const RxasFlowSsaAnalysis *ssa;
    RxasFlowUseMetrics metrics;
    RxasFlowUse *uses;
    size_t use_count;
    size_t use_capacity;
    size_t value_count;
    size_t storage_count;
    size_t *value_use_offsets;
    size_t *value_use_ids;
    size_t *dependent_offsets;
    size_t *dependents;
    size_t *storage_use_offsets;
    size_t *storage_use_ids;
    size_t *storage_walk_stack;
    size_t storage_walk_capacity;
    size_t *storage_walk_marks;
    size_t storage_walk_mark_capacity;
    size_t storage_walk_generation;
    unsigned char *live_values;
};

static int flow_use_consume(RxasFlowUseAnalysis *analysis, size_t amount) {
    size_t remaining;
    if (!analysis ||
        analysis->metrics.status != RXAS_FLOW_ANALYSIS_AVAILABLE)
        return 0;
    remaining = analysis->metrics.work <= analysis->metrics.budget_limit
            ? analysis->metrics.budget_limit - analysis->metrics.work : 0;
    if (amount > remaining) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED;
        return 0;
    }
    analysis->metrics.work += amount;
    return 1;
}

static void *flow_use_calloc(RxasFlowUseAnalysis *analysis,
                             size_t count, size_t size) {
    void *memory;
    if (!count) count = 1;
    if (size && count > ((size_t)-1) / size) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
        return 0;
    }
    memory = calloc(count, size);
    if (!memory) analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
    return memory;
}

static int flow_use_grow(RxasFlowUseAnalysis *analysis) {
    size_t capacity;
    RxasFlowUse *uses;
    if (analysis->use_count < analysis->use_capacity) return 1;
    capacity = analysis->use_capacity ? analysis->use_capacity * 2 : 64;
    if (capacity < analysis->use_capacity ||
        capacity > ((size_t)-1) / sizeof(*uses)) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
        return 0;
    }
    uses = realloc(analysis->uses, capacity * sizeof(*uses));
    if (!uses) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
        return 0;
    }
    analysis->uses = uses;
    analysis->use_capacity = capacity;
    return 1;
}

static Assembler_Token *flow_use_operand(const instruction_queue *item,
                                         size_t operand_index) {
    if (!item || operand_index >= item->operandCount) return 0;
    if (item->operandTokens) return item->operandTokens[operand_index];
    switch (operand_index) {
        case 0: return item->operand1Token;
        case 1: return item->operand2Token;
        case 2: return item->operand3Token;
        case 3: return item->operand4Token;
        case 4: return item->operand5Token;
        case 5: return item->operand6Token;
        case 6: return item->operand7Token;
        case 7: return item->operand8Token;
        case 8: return item->operand9Token;
        case 9: return item->operand10Token;
    }
    return 0;
}

static int flow_use_register_from_token(const Assembler_Token *token,
                                        RxasFlowRegister *reg) {
    if (!token || !reg || token->token_value.integer < 0) return 0;
    if (token->token_type == RREG)
        reg->register_class = RXAS_FLOW_REGISTER_LOCAL;
    else if (token->token_type == AREG)
        reg->register_class = RXAS_FLOW_REGISTER_ARGUMENT;
    else if (token->token_type == GREG)
        reg->register_class = RXAS_FLOW_REGISTER_GLOBAL;
    else return 0;
    reg->number = (size_t)token->token_value.integer;
    return 1;
}

static int flow_use_add(RxasFlowUseAnalysis *analysis, size_t record_id,
                        size_t instruction_id, size_t operand_index,
                        RxasFlowRegister reg, unsigned int component,
                        unsigned int read_components,
                        size_t value_id, size_t storage_id,
                        RxasFlowUseKind kind) {
    RxasFlowUse *use;
    if (!flow_use_consume(analysis, 1) || !flow_use_grow(analysis)) return 0;
    use = &analysis->uses[analysis->use_count];
    memset(use, 0, sizeof(*use));
    use->id = analysis->use_count++;
    use->record_id = record_id;
    use->instruction_id = instruction_id;
    use->operand_index = operand_index;
    use->register_id = reg;
    use->component = component;
    use->read_components = read_components;
    use->value_id = value_id;
    use->storage_id = storage_id;
    use->kind = kind;
    switch (kind) {
        case RXAS_FLOW_USE_EXPLICIT_READ:
            analysis->metrics.explicit_reads++;
            break;
        case RXAS_FLOW_USE_EXPLICIT_READ_WRITE:
            analysis->metrics.read_write_uses++;
            break;
        case RXAS_FLOW_USE_EXPLICIT_WRITE:
            analysis->metrics.explicit_writes++;
            break;
        case RXAS_FLOW_USE_OPAQUE_WRITE:
            analysis->metrics.opaque_writes++;
            break;
        case RXAS_FLOW_USE_IMPLICIT_READ:
            analysis->metrics.implicit_reads++;
            break;
        case RXAS_FLOW_USE_METADATA_READ:
            analysis->metrics.metadata_reads++;
            break;
        case RXAS_FLOW_USE_TRACE_READ:
            analysis->metrics.trace_reads++;
            break;
        case RXAS_FLOW_USE_CURSOR_READ:
            analysis->metrics.cursor_reads++;
            break;
        case RXAS_FLOW_USE_CURSOR_WRITE:
            analysis->metrics.cursor_writes++;
            break;
        case RXAS_FLOW_USE_CALL_WINDOW_READ:
            analysis->metrics.call_window_reads++;
            break;
        case RXAS_FLOW_USE_OPAQUE_OBSERVATION:
            analysis->metrics.opaque_observations++;
            break;
    }
    if (component && value_id == RXAS_FLOW_ID_NONE &&
        kind != RXAS_FLOW_USE_EXPLICIT_WRITE &&
        kind != RXAS_FLOW_USE_OPAQUE_WRITE &&
        kind != RXAS_FLOW_USE_CALL_WINDOW_READ &&
        kind != RXAS_FLOW_USE_OPAQUE_OBSERVATION)
        analysis->metrics.unknown_values++;
    return 1;
}

static int flow_use_add_opaque(RxasFlowUseAnalysis *analysis,
                               size_t record_id, size_t instruction_id) {
    RxasFlowRegister reg;
    reg.register_class = RXAS_FLOW_REGISTER_LOCAL;
    reg.number = 0;
    return flow_use_add(analysis, record_id, instruction_id,
                        RXAS_FLOW_ID_NONE, reg, RXOP_COMPONENT_NONE,
                        RXOP_COMPONENT_NONE,
                        RXAS_FLOW_ID_NONE, 0,
                        RXAS_FLOW_USE_OPAQUE_OBSERVATION);
}

static int flow_use_add_opaque_write(RxasFlowUseAnalysis *analysis,
                                     size_t record_id,
                                     size_t instruction_id) {
    RxasFlowRegister reg;
    reg.register_class = RXAS_FLOW_REGISTER_LOCAL;
    reg.number = 0;
    return flow_use_add(analysis, record_id, instruction_id,
                        RXAS_FLOW_ID_NONE, reg, RXOP_COMPONENT_NONE,
                        RXOP_COMPONENT_NONE,
                        RXAS_FLOW_ID_NONE, 0,
                        RXAS_FLOW_USE_OPAQUE_WRITE);
}

static int flow_use_add_call_window(RxasFlowUseAnalysis *analysis,
                                    size_t record_id,
                                    size_t instruction_id,
                                    size_t base_register) {
    RxasFlowRegister reg;
    reg.register_class = RXAS_FLOW_REGISTER_LOCAL;
    reg.number = base_register;
    return flow_use_add(analysis, record_id, instruction_id,
                        RXAS_FLOW_ID_NONE, reg, RXOP_COMPONENT_ALL,
                        RXOP_COMPONENT_ALL,
                        RXAS_FLOW_ID_NONE, 0,
                        RXAS_FLOW_USE_CALL_WINDOW_READ);
}

static int flow_use_add_components(RxasFlowUseAnalysis *analysis,
                                   size_t record_id, size_t instruction_id,
                                   size_t operand_index,
                                   RxasFlowRegister reg,
                                   unsigned int components,
                                   RxasFlowUseKind kind, int at_record) {
    size_t bit;
    for (bit = 0; bit < sizeof(flow_use_components) /
                            sizeof(flow_use_components[0]); bit++) {
        RxasFlowComponentFact fact;
        int available;
        if (!(components & flow_use_components[bit])) continue;
        if (!flow_use_consume(analysis, 1)) return 0;
        available = at_record
                ? rxas_flow_component_at_record(
                        analysis->ssa, analysis->metrics.epoch, record_id,
                        reg, flow_use_components[bit], &fact)
                : rxas_flow_component_at_instruction(
                        analysis->ssa, analysis->metrics.epoch,
                        instruction_id, 0, reg,
                        flow_use_components[bit], &fact);
        if (!available) {
                if (!flow_use_add(analysis, record_id, instruction_id,
                              operand_index, reg, flow_use_components[bit],
                              components,
                              RXAS_FLOW_ID_NONE, 0, kind))
                return 0;
        }
        else if (!flow_use_add(analysis, record_id, instruction_id,
                               operand_index, reg,
                               flow_use_components[bit], components,
                               fact.value_id,
                               fact.storage_id, kind))
            return 0;
    }
    return 1;
}

static int flow_use_add_cursor(RxasFlowUseAnalysis *analysis,
                               size_t record_id, size_t instruction_id,
                               size_t operand_index, RxasFlowRegister reg,
                               RxasFlowUseKind kind) {
    RxasFlowStorageFact fact;
    size_t storage_id;
    storage_id = 0;
    if (!flow_use_consume(analysis, 1)) return 0;
    if (rxas_flow_storage_at_instruction(
                analysis->ssa, analysis->metrics.epoch, instruction_id, 0,
                reg, &fact))
        storage_id = fact.storage_id;
    return flow_use_add(analysis, record_id, instruction_id, operand_index,
                        reg, RXOP_COMPONENT_NONE, RXOP_COMPONENT_NONE,
                        RXAS_FLOW_ID_NONE,
                        storage_id, kind);
}

static int flow_use_prepare_storage_walk(RxasFlowUseAnalysis *analysis) {
    size_t count;
    size_t old_capacity;
    size_t *resized;
    count = rxas_flow_storage_version_count(
            analysis->ssa, analysis->metrics.epoch);
    if (!count) return 0;
    if (analysis->storage_walk_capacity < count) {
        resized = realloc(analysis->storage_walk_stack,
                          count * sizeof(*resized));
        if (!resized) {
            analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
            return 0;
        }
        analysis->storage_walk_stack = resized;
        analysis->storage_walk_capacity = count;
    }
    if (analysis->storage_walk_mark_capacity < count) {
        old_capacity = analysis->storage_walk_mark_capacity;
        resized = realloc(analysis->storage_walk_marks,
                          count * sizeof(*resized));
        if (!resized) {
            analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
            return 0;
        }
        analysis->storage_walk_marks = resized;
        memset(analysis->storage_walk_marks + old_capacity, 0,
               (count - old_capacity) *
                    sizeof(*analysis->storage_walk_marks));
        analysis->storage_walk_mark_capacity = count;
    }
    analysis->storage_walk_generation++;
    if (!analysis->storage_walk_generation) {
        memset(analysis->storage_walk_marks, 0,
               analysis->storage_walk_mark_capacity *
                    sizeof(*analysis->storage_walk_marks));
        analysis->storage_walk_generation = 1;
    }
    return 1;
}

/* A write through a storage phi can target any leaf selected on a feasible
 * path. Index every leaf once so later proofs do not mistake a conservative
 * mapping join for an absence of writes. */
static int flow_use_add_write_components(
        RxasFlowUseAnalysis *analysis, size_t record_id,
        size_t instruction_id, size_t operand_index,
        RxasFlowRegister reg, unsigned int components) {
    RxasFlowStorageFact fact;
    size_t head;
    size_t tail;
    if (!rxas_flow_storage_at_instruction(
                analysis->ssa, analysis->metrics.epoch,
                instruction_id, 0, reg, &fact) ||
        !fact.storage_id || !flow_use_prepare_storage_walk(analysis))
        return flow_use_add_opaque_write(
                analysis, record_id, instruction_id);
    head = 0;
    tail = 0;
    if (fact.storage_id > analysis->storage_walk_mark_capacity)
        return flow_use_add_opaque_write(
                analysis, record_id, instruction_id);
    analysis->storage_walk_marks[fact.storage_id - 1] =
            analysis->storage_walk_generation;
    analysis->storage_walk_stack[tail++] = fact.storage_id;
    while (head < tail) {
        RxasFlowStorageNode node;
        size_t bit;
        size_t input;
        size_t storage_id;
        storage_id = analysis->storage_walk_stack[head++];
        if (!flow_use_consume(analysis, 1) ||
            !rxas_flow_storage_node(
                    analysis->ssa, analysis->metrics.epoch,
                    storage_id, &node) || !node.id ||
            node.id > analysis->storage_walk_mark_capacity)
            return 0;
        if (node.kind == RXAS_FLOW_STORAGE_PHI) {
            for (input = 0; input < node.input_count; input++) {
                size_t input_id;
                input_id = rxas_flow_storage_input(
                        analysis->ssa, analysis->metrics.epoch,
                        node.id, input);
                if (!input_id ||
                    input_id > analysis->storage_walk_mark_capacity)
                    return flow_use_add_opaque_write(
                            analysis, record_id, instruction_id);
                if (analysis->storage_walk_marks[input_id - 1] ==
                            analysis->storage_walk_generation)
                    continue;
                if (tail >= analysis->storage_walk_capacity)
                    return flow_use_add_opaque_write(
                            analysis, record_id, instruction_id);
                analysis->storage_walk_marks[input_id - 1] =
                        analysis->storage_walk_generation;
                analysis->storage_walk_stack[tail++] = input_id;
            }
            continue;
        }
        if (node.kind == RXAS_FLOW_STORAGE_UNKNOWN)
            return flow_use_add_opaque_write(
                    analysis, record_id, instruction_id);
        for (bit = 0; bit < sizeof(flow_use_components) /
                                sizeof(flow_use_components[0]); bit++) {
            if (!(components & flow_use_components[bit])) continue;
            if (!flow_use_add(
                        analysis, record_id, instruction_id,
                        operand_index, reg, flow_use_components[bit],
                        components, RXAS_FLOW_ID_NONE, node.id,
                        RXAS_FLOW_USE_EXPLICIT_WRITE))
                return 0;
        }
    }
    return 1;
}

static int flow_use_add_numbered_components(
        RxasFlowUseAnalysis *analysis, size_t record_id,
        size_t instruction_id, size_t operand_index,
        RxasFlowRegisterClass reg_class, size_t number,
        unsigned int components, RxasFlowUseKind kind) {
    RxasFlowRegister reg;
    reg.register_class = reg_class;
    reg.number = number;
    return flow_use_add_components(
            analysis, record_id, instruction_id, operand_index, reg,
            components, kind, 0);
}

static int flow_use_add_implicit(RxasFlowUseAnalysis *analysis,
                                 const RxasFlowInstruction *instruction,
                                 const instruction_queue *item) {
    size_t number;
    size_t record_id;
    record_id = instruction->record_id;
    switch (instruction->effects.implicit) {
        case RXOP_IMPLICIT_LOCAL_R0_READ_WRITE:
        case RXOP_IMPLICIT_LOCAL_R1_READ_WRITE:
        case RXOP_IMPLICIT_LOCAL_R2_READ_WRITE:
            number = instruction->effects.implicit ==
                            RXOP_IMPLICIT_LOCAL_R0_READ_WRITE ? 0 :
                     instruction->effects.implicit ==
                            RXOP_IMPLICIT_LOCAL_R1_READ_WRITE ? 1 : 2;
            return flow_use_add_numbered_components(
                    analysis, record_id, instruction->id,
                    RXAS_FLOW_ID_NONE, RXAS_FLOW_REGISTER_LOCAL, number,
                    RXOP_COMPONENT_INTEGER,
                    RXAS_FLOW_USE_EXPLICIT_READ_WRITE);
        case RXOP_IMPLICIT_LOCAL_COPY:
            if (!item->operand2Token ||
                item->operand2Token->token_type != INT ||
                item->operand2Token->token_value.integer < 0)
                return flow_use_add_opaque(
                        analysis, record_id, instruction->id);
            return flow_use_add_numbered_components(
                    analysis, record_id, instruction->id,
                    RXAS_FLOW_ID_NONE, RXAS_FLOW_REGISTER_LOCAL,
                    (size_t)item->operand2Token->token_value.integer,
                    RXOP_COMPONENT_ALL, RXAS_FLOW_USE_IMPLICIT_READ);
        case RXOP_IMPLICIT_ARGUMENT_INDEX:
            if (!item->operand2Token ||
                item->operand2Token->token_type != INT ||
                item->operand2Token->token_value.integer < 0)
                return flow_use_add_opaque(
                        analysis, record_id, instruction->id);
            return flow_use_add_numbered_components(
                    analysis, record_id, instruction->id,
                    RXAS_FLOW_ID_NONE, RXAS_FLOW_REGISTER_ARGUMENT,
                    (size_t)item->operand2Token->token_value.integer,
                    RXOP_COMPONENT_ALL, RXAS_FLOW_USE_IMPLICIT_READ);
        case RXOP_IMPLICIT_LOCAL_RANGE_AFTER_OP3:
            if (!item->operand3Token ||
                item->operand3Token->token_type != RREG ||
                item->operand3Token->token_value.integer < 0)
                return flow_use_add_opaque(
                        analysis, record_id, instruction->id);
            number = (size_t)item->operand3Token->token_value.integer;
            /* Retain a call window as one range observation. Expanding every
             * possible local/component would recreate the dense matrix and
             * eagerly materialize otherwise-unused SSA phis. */
            return flow_use_add_call_window(
                    analysis, record_id, instruction->id, number);
        case RXOP_IMPLICIT_LOCAL_TARGET:
            return flow_use_add_opaque(analysis, record_id,
                                       instruction->id);
        case RXOP_IMPLICIT_NONE:
            return 1;
    }
    return flow_use_add_opaque(analysis, record_id, instruction->id);
}

static int flow_use_collect_instruction(
        RxasFlowUseAnalysis *analysis, const RxasFlowRecord *record,
        const RxasFlowInstruction *instruction) {
    const instruction_queue *item;
    size_t operand;
    int implicit_complete;
    item = record->queue_record;
    if (!item || !instruction->op ||
        instruction->effects.state != RXOP_EFFECT_CLASSIFIED)
        return flow_use_add_opaque(analysis, record->id, instruction->id);
    for (operand = 0; operand < item->operandCount; operand++) {
        Assembler_Token *token;
        RxasFlowRegister reg;
        unsigned int components;
        RxasFlowUseKind kind;
        token = flow_use_operand(item, operand);
        if (!flow_use_register_from_token(token, &reg)) continue;
        if (rxop_effect_reads_operand(&instruction->effects, operand)) {
            components = rxop_component_reads(
                    instruction->op->opcode, operand);
            kind = rxop_effect_writes_operand(
                    &instruction->effects, operand)
                    ? RXAS_FLOW_USE_EXPLICIT_READ_WRITE
                    : RXAS_FLOW_USE_EXPLICIT_READ;
            if (!components) {
                if (!flow_use_add_opaque(
                            analysis, record->id, instruction->id))
                    return 0;
            }
            else if (!flow_use_add_components(
                         analysis, record->id, instruction->id, operand,
                         reg, components, kind, 0))
                return 0;
        }
        if (rxop_effect_writes_operand(&instruction->effects, operand) &&
            !rxas_flow_opcode_is_plain_mapping(
                    instruction->op->opcode)) {
            components = rxop_component_writes(
                    instruction->op->opcode, operand);
            /* Component metadata is intentionally sparse. A classified
             * register write without a narrower component contract is an
             * all-component write, not an opaque whole-procedure barrier. */
            if (!components) components = RXOP_COMPONENT_ALL;
            if (!flow_use_add_write_components(
                    analysis, record->id, instruction->id, operand,
                    reg, components))
                return 0;
        }
        if (rxop_effect_reads_cursor(&instruction->effects, operand) &&
            !flow_use_add_cursor(analysis, record->id, instruction->id,
                                 operand, reg,
                                 RXAS_FLOW_USE_CURSOR_READ))
            return 0;
        if (rxop_effect_writes_cursor(&instruction->effects, operand) &&
            !flow_use_add_cursor(analysis, record->id, instruction->id,
                                 operand, reg,
                                 RXAS_FLOW_USE_CURSOR_WRITE))
            return 0;
    }
    implicit_complete = flow_use_add_implicit(analysis, instruction, item);
    if (!implicit_complete) return 0;
    /* Classified fixed-arity and zero-argument calls expose their complete
     * caller interface through explicit operands.  Range calls are handled
     * by RXOP_IMPLICIT_LOCAL_RANGE_AFTER_OP3 above; no classified call form
     * has an additional unknown caller-local window. */
    return 1;
}

static unsigned int flow_use_trace_components(
        const instruction_queue *item) {
    int type;
    if (!item || !item->operand3Token ||
        item->operand3Token->token_type != STRING ||
        !item->operand3Token->token_value.string[0])
        return RXOP_COMPONENT_ALL;
    type = toupper((unsigned char)item->operand3Token->token_value.string[0]);
    if (type == 'B' || type == 'I') return RXOP_COMPONENT_INTEGER;
    if (type == 'F') return RXOP_COMPONENT_FLOAT;
    if (type == 'S') return RXOP_COMPONENT_STRING;
    if (type == 'D') return RXOP_COMPONENT_DECIMAL;
    if (type == 'X') return RXOP_COMPONENT_BINARY;
    if (type == 'R') return RXOP_COMPONENT_REFERENCE;
    return RXOP_COMPONENT_ALL;
}

static int flow_use_trace_register(const instruction_queue *item,
                                   RxasFlowRegister *reg) {
    int type;
    if (!item || !reg || !item->operand2Token ||
        item->operand2Token->token_type != STRING ||
        strcmp((const char *)item->operand2Token->token_value.string, "R") ||
        !item->operand4Token || item->operand4Token->token_type != STRING ||
        !item->operand4Token->token_value.string[0] ||
        !item->operand5Token || item->operand5Token->token_type != INT ||
        item->operand5Token->token_value.integer < 0)
        return 0;
    type = tolower((unsigned char)item->operand4Token->token_value.string[0]);
    if (type == 'r') reg->register_class = RXAS_FLOW_REGISTER_LOCAL;
    else if (type == 'a') reg->register_class = RXAS_FLOW_REGISTER_ARGUMENT;
    else if (type == 'g') reg->register_class = RXAS_FLOW_REGISTER_GLOBAL;
    else return 0;
    reg->number = (size_t)item->operand5Token->token_value.integer;
    return 1;
}

static int flow_use_collect_records(RxasFlowUseAnalysis *analysis) {
    const RxasFlowMetrics *metrics;
    size_t record_id;
    metrics = rxas_flow_procedure_metrics(
            analysis->procedure, analysis->metrics.epoch);
    if (!metrics || !metrics->complete_control_flow) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
        return 0;
    }
    for (record_id = 0; record_id < metrics->records; record_id++) {
        const RxasFlowRecord *record;
        const instruction_queue *item;
        RxasFlowRegister reg;
        if (!flow_use_consume(analysis, 1)) return 0;
        record = rxas_flow_procedure_record(
                analysis->procedure, analysis->metrics.epoch, record_id);
        if (!record) {
            analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
            return 0;
        }
        item = record->queue_record;
        if (record->instruction_id != RXAS_FLOW_ID_NONE) {
            const RxasFlowInstruction *instruction;
            instruction = rxas_flow_procedure_instruction(
                    analysis->procedure, analysis->metrics.epoch,
                    record->instruction_id);
            if (!instruction || !flow_use_collect_instruction(
                                        analysis, record, instruction))
                return 0;
        }
        else if (record->type == REG_META && item &&
                 flow_use_register_from_token(item->operand3Token, &reg)) {
            if (!flow_use_add_components(
                        analysis, record_id, RXAS_FLOW_ID_NONE,
                        RXAS_FLOW_ID_NONE, reg, RXOP_COMPONENT_ALL,
                        RXAS_FLOW_USE_METADATA_READ, 1))
                return 0;
        }
        else if (record->type == TRACE_EVENT && item &&
                 flow_use_trace_register(item, &reg)) {
            if (!flow_use_add_components(
                        analysis, record_id, RXAS_FLOW_ID_NONE,
                        RXAS_FLOW_ID_NONE, reg,
                        flow_use_trace_components(item),
                        RXAS_FLOW_USE_TRACE_READ, 1))
                return 0;
        }
    }
    return 1;
}

static int flow_use_build_value_index(RxasFlowUseAnalysis *analysis) {
    size_t *fill;
    size_t use_id;
    size_t value_id;
    analysis->value_count = rxas_flow_value_version_count(
            analysis->ssa, analysis->metrics.epoch);
    analysis->value_use_offsets = flow_use_calloc(
            analysis, analysis->value_count + 1, sizeof(size_t));
    if (!analysis->value_use_offsets) return 0;
    for (use_id = 0; use_id < analysis->use_count; use_id++) {
        value_id = analysis->uses[use_id].value_id;
        if (value_id < analysis->value_count)
            analysis->value_use_offsets[value_id + 1]++;
    }
    for (value_id = 1; value_id <= analysis->value_count; value_id++)
        analysis->value_use_offsets[value_id] +=
                analysis->value_use_offsets[value_id - 1];
    analysis->value_use_ids = flow_use_calloc(
            analysis,
            analysis->value_use_offsets[analysis->value_count],
            sizeof(size_t));
    fill = flow_use_calloc(
            analysis, analysis->value_count, sizeof(size_t));
    if (!analysis->value_use_ids || !fill) {
        free(fill);
        return 0;
    }
    for (value_id = 0; value_id < analysis->value_count; value_id++)
        fill[value_id] = analysis->value_use_offsets[value_id];
    for (use_id = 0; use_id < analysis->use_count; use_id++) {
        value_id = analysis->uses[use_id].value_id;
        if (value_id < analysis->value_count)
            analysis->value_use_ids[fill[value_id]++] = use_id;
    }
    free(fill);
    return flow_use_consume(
            analysis, analysis->value_count + analysis->use_count);
}

static int flow_use_build_storage_index(RxasFlowUseAnalysis *analysis) {
    size_t *fill;
    size_t use_id;
    size_t storage_id;
    analysis->storage_count = rxas_flow_storage_version_count(
            analysis->ssa, analysis->metrics.epoch);
    analysis->storage_use_offsets = flow_use_calloc(
            analysis, analysis->storage_count + 2, sizeof(size_t));
    if (!analysis->storage_use_offsets) return 0;
    for (use_id = 0; use_id < analysis->use_count; use_id++) {
        storage_id = analysis->uses[use_id].storage_id;
        if (storage_id && storage_id <= analysis->storage_count)
            analysis->storage_use_offsets[storage_id + 1]++;
    }
    for (storage_id = 1; storage_id <= analysis->storage_count + 1;
         storage_id++)
        analysis->storage_use_offsets[storage_id] +=
                analysis->storage_use_offsets[storage_id - 1];
    analysis->storage_use_ids = flow_use_calloc(
            analysis,
            analysis->storage_use_offsets[analysis->storage_count + 1],
            sizeof(size_t));
    fill = flow_use_calloc(
            analysis, analysis->storage_count + 1, sizeof(size_t));
    if (!analysis->storage_use_ids || !fill) {
        free(fill);
        return 0;
    }
    for (storage_id = 1; storage_id <= analysis->storage_count; storage_id++)
        fill[storage_id] = analysis->storage_use_offsets[storage_id];
    for (use_id = 0; use_id < analysis->use_count; use_id++) {
        storage_id = analysis->uses[use_id].storage_id;
        if (storage_id && storage_id <= analysis->storage_count)
            analysis->storage_use_ids[fill[storage_id]++] = use_id;
    }
    free(fill);
    return flow_use_consume(
            analysis, analysis->storage_count + analysis->use_count);
}

static int flow_use_build_dependencies(RxasFlowUseAnalysis *analysis) {
    size_t *fill;
    size_t value_id;
    size_t input_index;
    size_t input;
    RxasFlowValueNode node;
    analysis->dependent_offsets = flow_use_calloc(
            analysis, analysis->value_count + 1, sizeof(size_t));
    if (!analysis->dependent_offsets) return 0;
    for (value_id = 0; value_id < analysis->value_count; value_id++) {
        if (!flow_use_consume(analysis, 1)) return 0;
        if (!rxas_flow_value_node(
                    analysis->ssa, analysis->metrics.epoch, value_id, &node) ||
            node.id != value_id || node.kind != RXAS_FLOW_VALUE_PHI)
            continue;
        for (input_index = 0; input_index < node.input_count; input_index++) {
            input = rxas_flow_value_input(
                    analysis->ssa, analysis->metrics.epoch,
                    value_id, input_index);
            if (input < analysis->value_count && input != value_id)
                analysis->dependent_offsets[input + 1]++;
        }
    }
    for (value_id = 1; value_id <= analysis->value_count; value_id++)
        analysis->dependent_offsets[value_id] +=
                analysis->dependent_offsets[value_id - 1];
    analysis->metrics.phi_dependency_edges =
            analysis->dependent_offsets[analysis->value_count];
    analysis->dependents = flow_use_calloc(
            analysis, analysis->metrics.phi_dependency_edges,
            sizeof(size_t));
    fill = flow_use_calloc(
            analysis, analysis->value_count, sizeof(size_t));
    if (!analysis->dependents || !fill) {
        free(fill);
        return 0;
    }
    for (value_id = 0; value_id < analysis->value_count; value_id++)
        fill[value_id] = analysis->dependent_offsets[value_id];
    for (value_id = 0; value_id < analysis->value_count; value_id++) {
        if (!rxas_flow_value_node(
                    analysis->ssa, analysis->metrics.epoch, value_id, &node) ||
            node.id != value_id || node.kind != RXAS_FLOW_VALUE_PHI)
            continue;
        for (input_index = 0; input_index < node.input_count; input_index++) {
            input = rxas_flow_value_input(
                    analysis->ssa, analysis->metrics.epoch,
                    value_id, input_index);
            if (input < analysis->value_count && input != value_id)
                analysis->dependents[fill[input]++] = value_id;
        }
    }
    free(fill);
    return flow_use_consume(
            analysis, analysis->value_count +
                      analysis->metrics.phi_dependency_edges);
}

static int flow_use_mark_live(RxasFlowUseAnalysis *analysis) {
    size_t *stack;
    size_t stack_count;
    size_t value_id;
    size_t input_index;
    size_t input;
    RxasFlowValueNode node;
    analysis->live_values = flow_use_calloc(
            analysis, analysis->value_count, 1);
    stack = flow_use_calloc(
            analysis, analysis->value_count, sizeof(size_t));
    if (!analysis->live_values || !stack) {
        free(stack);
        return 0;
    }
    stack_count = 0;
    for (value_id = 0; value_id < analysis->value_count; value_id++) {
        if (analysis->value_use_offsets[value_id] ==
            analysis->value_use_offsets[value_id + 1])
            continue;
        analysis->live_values[value_id] = 1;
        stack[stack_count++] = value_id;
        analysis->metrics.live_values++;
    }
    while (stack_count) {
        value_id = stack[--stack_count];
        if (!flow_use_consume(analysis, 1)) {
            free(stack);
            return 0;
        }
        if (!rxas_flow_value_node(
                    analysis->ssa, analysis->metrics.epoch, value_id, &node) ||
            node.kind != RXAS_FLOW_VALUE_PHI)
            continue;
        for (input_index = 0; input_index < node.input_count; input_index++) {
            input = rxas_flow_value_input(
                    analysis->ssa, analysis->metrics.epoch,
                    node.id, input_index);
            if (input >= analysis->value_count ||
                analysis->live_values[input])
                continue;
            analysis->live_values[input] = 1;
            stack[stack_count++] = input;
            analysis->metrics.live_values++;
        }
    }
    free(stack);
    return 1;
}

static void flow_use_set_retained_bytes(RxasFlowUseAnalysis *analysis) {
    if (!analysis) return;
    analysis->metrics.retained_bytes = sizeof(*analysis) +
            analysis->use_capacity * sizeof(*analysis->uses) +
            (analysis->value_count + 1) *
                    sizeof(*analysis->value_use_offsets) +
            analysis->value_use_offsets[analysis->value_count] *
                    sizeof(*analysis->value_use_ids) +
            (analysis->value_count + 1) *
                    sizeof(*analysis->dependent_offsets) +
            analysis->metrics.phi_dependency_edges *
                    sizeof(*analysis->dependents) +
            (analysis->storage_count + 2) *
                    sizeof(*analysis->storage_use_offsets) +
            analysis->storage_use_offsets[analysis->storage_count + 1] *
                    sizeof(*analysis->storage_use_ids) +
            analysis->storage_walk_capacity *
                    sizeof(*analysis->storage_walk_stack) +
            analysis->storage_walk_mark_capacity *
                    sizeof(*analysis->storage_walk_marks) +
            analysis->value_count;
}

static void flow_use_free(RxasFlowUseAnalysis *analysis) {
    if (!analysis) return;
    free(analysis->uses);
    free(analysis->value_use_offsets);
    free(analysis->value_use_ids);
    free(analysis->dependent_offsets);
    free(analysis->dependents);
    free(analysis->storage_use_offsets);
    free(analysis->storage_use_ids);
    free(analysis->storage_walk_stack);
    free(analysis->storage_walk_marks);
    free(analysis->live_values);
    free(analysis);
}

void rxas_flow_use_analysis_destroy(struct RxasFlowUseAnalysis *analysis) {
    flow_use_free(analysis);
}

static RxasFlowUseAnalysis *flow_use_build(
        RxasFlowProcedure *procedure, unsigned long epoch, size_t budget) {
    RxasFlowUseAnalysis *analysis;
    analysis = calloc(1, sizeof(*analysis));
    if (!analysis) return 0;
    analysis->procedure = procedure;
    analysis->metrics.status = RXAS_FLOW_ANALYSIS_AVAILABLE;
    analysis->metrics.epoch = epoch;
    analysis->metrics.budget_limit = budget;
    analysis->ssa = rxas_flow_require_ssa_analysis(procedure, epoch, 0);
    if (!analysis->ssa) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
        return analysis;
    }
    if (!flow_use_collect_records(analysis) ||
        !rxas_flow_ssa_metrics(analysis->ssa, epoch) ||
        !flow_use_build_value_index(analysis) ||
        !flow_use_build_storage_index(analysis) ||
        !flow_use_build_dependencies(analysis) ||
        !flow_use_mark_live(analysis))
        return analysis;
    analysis->metrics.uses = analysis->use_count;
    flow_use_set_retained_bytes(analysis);
    return analysis;
}

static size_t flow_use_default_budget(
        const RxasFlowMetrics *metrics, const RxasFlowSsaMetrics *ssa) {
    size_t scale;
    if (!metrics || !ssa) return 0;
    if (metrics->records > ((size_t)-1) / 128) return (size_t)-1;
    scale = metrics->records * 128;
    if (ssa->value_versions > (((size_t)-1) - scale) / 48)
        return (size_t)-1;
    scale += ssa->value_versions * 48;
    if (scale > (size_t)-1 - 8192) return (size_t)-1;
    return scale + 8192;
}

const RxasFlowUseAnalysis *rxas_flow_require_use_analysis(
        RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t work_budget) {
    struct RxasFlowAnalysisManager *manager;
    const RxasFlowMetrics *metrics;
    const RxasFlowSsaAnalysis *ssa;
    const RxasFlowSsaMetrics *ssa_metrics;
    size_t expanded_ssa_budget;
    size_t requested;
    if (!rxas_flow_procedure_epoch_matches(procedure, expected_epoch)) return 0;
    metrics = rxas_flow_procedure_metrics(procedure, expected_epoch);
    ssa = rxas_flow_require_ssa_analysis(procedure, expected_epoch, 0);
    ssa_metrics = ssa ? rxas_flow_ssa_metrics(ssa, expected_epoch) : 0;
    if (!metrics || !ssa_metrics) return 0;
    expanded_ssa_budget = ssa_metrics->budget_limit <= (size_t)-1 / 8
            ? ssa_metrics->budget_limit * 8 : (size_t)-1;
    ssa = rxas_flow_require_ssa_analysis(
            procedure, expected_epoch, expanded_ssa_budget);
    ssa_metrics = ssa ? rxas_flow_ssa_metrics(ssa, expected_epoch) : 0;
    if (!ssa_metrics) return 0;
    requested = work_budget ? work_budget
                            : flow_use_default_budget(metrics, ssa_metrics);
    manager = procedure->analysis_manager;
    if (manager && manager->epoch == expected_epoch && manager->use) {
        if (manager->use->metrics.status == RXAS_FLOW_ANALYSIS_AVAILABLE)
            return manager->use;
        if (requested <= manager->use_budget) return 0;
        flow_use_free(manager->use);
        manager->use = 0;
    }
    if (!manager) {
        manager = calloc(1, sizeof(*manager));
        if (!manager) return 0;
        procedure->analysis_manager = manager;
    }
    manager->epoch = expected_epoch;
    manager->use_budget = requested;
    manager->use = flow_use_build(procedure, expected_epoch, requested);
    if (!manager->use || manager->use->metrics.status !=
                                RXAS_FLOW_ANALYSIS_AVAILABLE)
        return 0;
    return manager->use;
}

const RxasFlowUseMetrics *rxas_flow_last_use_metrics(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch) {
    const struct RxasFlowAnalysisManager *manager;
    if (!rxas_flow_procedure_epoch_matches(procedure, expected_epoch) ||
        !procedure->analysis_manager)
        return 0;
    manager = procedure->analysis_manager;
    if (manager->epoch != expected_epoch || !manager->use) return 0;
    return &manager->use->metrics;
}

static int flow_use_valid(const RxasFlowUseAnalysis *analysis,
                          unsigned long epoch) {
    return analysis && epoch && analysis->metrics.epoch == epoch &&
           analysis->metrics.status == RXAS_FLOW_ANALYSIS_AVAILABLE &&
           rxas_flow_procedure_epoch_matches(analysis->procedure, epoch);
}

static size_t flow_use_canonical_value(
        const RxasFlowUseAnalysis *analysis, unsigned long epoch,
        size_t value_id) {
    RxasFlowValueNode node;
    if (value_id == RXAS_FLOW_ID_NONE ||
        !rxas_flow_value_node(analysis->ssa, epoch, value_id, &node))
        return RXAS_FLOW_ID_NONE;
    return node.id;
}

static size_t flow_use_canonical_storage(
        const RxasFlowUseAnalysis *analysis, unsigned long epoch,
        size_t storage_id) {
    RxasFlowStorageNode node;
    if (!storage_id ||
        !rxas_flow_storage_node(analysis->ssa, epoch, storage_id, &node))
        return 0;
    return node.id;
}

const RxasFlowUseMetrics *rxas_flow_use_metrics(
        const RxasFlowUseAnalysis *analysis, unsigned long expected_epoch) {
    if (!flow_use_valid(analysis, expected_epoch)) return 0;
    return &analysis->metrics;
}

size_t rxas_flow_use_count(const RxasFlowUseAnalysis *analysis,
                           unsigned long expected_epoch) {
    return flow_use_valid(analysis, expected_epoch) ? analysis->use_count : 0;
}

const RxasFlowUse *rxas_flow_use(const RxasFlowUseAnalysis *analysis,
                                 unsigned long expected_epoch,
                                 size_t use_id) {
    if (!flow_use_valid(analysis, expected_epoch) ||
        use_id >= analysis->use_count)
        return 0;
    return &analysis->uses[use_id];
}

size_t rxas_flow_value_direct_use_count(
        const RxasFlowUseAnalysis *analysis, unsigned long expected_epoch,
        size_t value_id) {
    if (!flow_use_valid(analysis, expected_epoch)) return 0;
    value_id = flow_use_canonical_value(
            analysis, expected_epoch, value_id);
    if (
        value_id >= analysis->value_count)
        return 0;
    return analysis->value_use_offsets[value_id + 1] -
           analysis->value_use_offsets[value_id];
}

const RxasFlowUse *rxas_flow_value_direct_use(
        const RxasFlowUseAnalysis *analysis, unsigned long expected_epoch,
        size_t value_id, size_t use_index) {
    size_t start;
    size_t end;
    if (!flow_use_valid(analysis, expected_epoch)) return 0;
    value_id = flow_use_canonical_value(
            analysis, expected_epoch, value_id);
    if (
        value_id >= analysis->value_count)
        return 0;
    start = analysis->value_use_offsets[value_id];
    end = analysis->value_use_offsets[value_id + 1];
    if (use_index >= end - start) return 0;
    return &analysis->uses[analysis->value_use_ids[start + use_index]];
}

size_t rxas_flow_value_dependent_count(
        const RxasFlowUseAnalysis *analysis, unsigned long expected_epoch,
        size_t value_id) {
    if (!flow_use_valid(analysis, expected_epoch)) return 0;
    value_id = flow_use_canonical_value(
            analysis, expected_epoch, value_id);
    if (
        value_id >= analysis->value_count)
        return 0;
    return analysis->dependent_offsets[value_id + 1] -
           analysis->dependent_offsets[value_id];
}

size_t rxas_flow_value_dependent(
        const RxasFlowUseAnalysis *analysis, unsigned long expected_epoch,
        size_t value_id, size_t dependent_index) {
    size_t start;
    size_t end;
    if (!flow_use_valid(analysis, expected_epoch)) return RXAS_FLOW_ID_NONE;
    value_id = flow_use_canonical_value(
            analysis, expected_epoch, value_id);
    if (value_id >= analysis->value_count)
        return RXAS_FLOW_ID_NONE;
    start = analysis->dependent_offsets[value_id];
    end = analysis->dependent_offsets[value_id + 1];
    if (dependent_index >= end - start) return RXAS_FLOW_ID_NONE;
    return analysis->dependents[start + dependent_index];
}

int rxas_flow_value_is_live(const RxasFlowUseAnalysis *analysis,
                            unsigned long expected_epoch, size_t value_id) {
    if (!flow_use_valid(analysis, expected_epoch)) return 0;
    value_id = flow_use_canonical_value(
            analysis, expected_epoch, value_id);
    if (value_id >= analysis->value_count)
        return 0;
    return analysis->live_values[value_id] != 0;
}

size_t rxas_flow_storage_use_count(
        const RxasFlowUseAnalysis *analysis, unsigned long expected_epoch,
        size_t storage_id) {
    if (!flow_use_valid(analysis, expected_epoch)) return 0;
    storage_id = flow_use_canonical_storage(
            analysis, expected_epoch, storage_id);
    if (!storage_id ||
        storage_id > analysis->storage_count)
        return 0;
    return analysis->storage_use_offsets[storage_id + 1] -
           analysis->storage_use_offsets[storage_id];
}

const RxasFlowUse *rxas_flow_storage_use(
        const RxasFlowUseAnalysis *analysis, unsigned long expected_epoch,
        size_t storage_id, size_t use_index) {
    size_t start;
    size_t end;
    if (!flow_use_valid(analysis, expected_epoch)) return 0;
    storage_id = flow_use_canonical_storage(
            analysis, expected_epoch, storage_id);
    if (!storage_id ||
        storage_id > analysis->storage_count)
        return 0;
    start = analysis->storage_use_offsets[storage_id];
    end = analysis->storage_use_offsets[storage_id + 1];
    if (use_index >= end - start) return 0;
    return &analysis->uses[analysis->storage_use_ids[start + use_index]];
}

static const char *flow_use_kind_name(RxasFlowUseKind kind) {
    switch (kind) {
        case RXAS_FLOW_USE_EXPLICIT_READ: return "explicit-read";
        case RXAS_FLOW_USE_EXPLICIT_READ_WRITE: return "read-write";
        case RXAS_FLOW_USE_EXPLICIT_WRITE: return "explicit-write";
        case RXAS_FLOW_USE_OPAQUE_WRITE: return "opaque-write";
        case RXAS_FLOW_USE_IMPLICIT_READ: return "implicit-read";
        case RXAS_FLOW_USE_METADATA_READ: return "metadata-read";
        case RXAS_FLOW_USE_TRACE_READ: return "trace-read";
        case RXAS_FLOW_USE_CURSOR_READ: return "cursor-read";
        case RXAS_FLOW_USE_CURSOR_WRITE: return "cursor-write";
        case RXAS_FLOW_USE_CALL_WINDOW_READ: return "call-window-read";
        case RXAS_FLOW_USE_OPAQUE_OBSERVATION: return "opaque";
    }
    return "invalid";
}

int rxas_flow_use_dump(const RxasFlowUseAnalysis *analysis,
                       unsigned long expected_epoch, FILE *stream) {
    size_t use_id;
    if (!stream || !flow_use_valid(analysis, expected_epoch)) return 0;
    fprintf(stream,
            "PERF3 flow-use epoch=%lu status=available uses=%llu "
            "explicit=%llu read-write=%llu writes=%llu opaque-writes=%llu "
            "implicit=%llu "
            "metadata=%llu "
            "trace=%llu cursor-read=%llu cursor-write=%llu "
            "call-window=%llu opaque=%llu unknown=%llu "
            "phi-edges=%llu live-values=%llu budget=%llu work=%llu "
            "retained-bytes=%llu\n",
            analysis->metrics.epoch,
            (unsigned long long)analysis->metrics.uses,
            (unsigned long long)analysis->metrics.explicit_reads,
            (unsigned long long)analysis->metrics.read_write_uses,
            (unsigned long long)analysis->metrics.explicit_writes,
            (unsigned long long)analysis->metrics.opaque_writes,
            (unsigned long long)analysis->metrics.implicit_reads,
            (unsigned long long)analysis->metrics.metadata_reads,
            (unsigned long long)analysis->metrics.trace_reads,
            (unsigned long long)analysis->metrics.cursor_reads,
            (unsigned long long)analysis->metrics.cursor_writes,
            (unsigned long long)analysis->metrics.call_window_reads,
            (unsigned long long)analysis->metrics.opaque_observations,
            (unsigned long long)analysis->metrics.unknown_values,
            (unsigned long long)analysis->metrics.phi_dependency_edges,
            (unsigned long long)analysis->metrics.live_values,
            (unsigned long long)analysis->metrics.budget_limit,
            (unsigned long long)analysis->metrics.work,
            (unsigned long long)analysis->metrics.retained_bytes);
    for (use_id = 0; use_id < analysis->use_count &&
                     use_id < FLOW_USE_DUMP_ENTRY_LIMIT; use_id++) {
        const RxasFlowUse *use;
        const char *reg_class;
        use = &analysis->uses[use_id];
        reg_class = use->register_id.register_class ==
                            RXAS_FLOW_REGISTER_ARGUMENT ? "a" :
                    use->register_id.register_class ==
                            RXAS_FLOW_REGISTER_GLOBAL ? "g" : "r";
        fprintf(stream,
                "PERF3 flow-use-entry id=%llu record=%llu instruction=%llu "
                "operand=%llu register=%s%llu component=0x%x value=%llu "
                "read-components=0x%x storage=%llu kind=%s\n",
                (unsigned long long)use->id,
                (unsigned long long)use->record_id,
                (unsigned long long)use->instruction_id,
                (unsigned long long)use->operand_index,
                reg_class, (unsigned long long)use->register_id.number,
                use->component, (unsigned long long)use->value_id,
                use->read_components,
                (unsigned long long)use->storage_id,
                flow_use_kind_name(use->kind));
    }
    if (analysis->use_count > FLOW_USE_DUMP_ENTRY_LIMIT)
        fprintf(stream,
                "PERF3 flow-use-entries-truncated shown=%u omitted=%llu\n",
                (unsigned int)FLOW_USE_DUMP_ENTRY_LIMIT,
                (unsigned long long)(analysis->use_count -
                                      FLOW_USE_DUMP_ENTRY_LIMIT));
    return 1;
}
