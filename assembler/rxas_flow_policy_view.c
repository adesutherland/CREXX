/* cREXX License (MIT). Exact declared-entry policy views, independent of SSA. */
#include "rxas_flow_graph_internal.h"
#include "rxasassm.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define POLICY_HALT 1u
#define POLICY_IGNORE 2u
#define POLICY_OTHER 4u
#define POLICY_TOP 7u
#define PROFILE_SEAL 0x52585056u

struct RxasFlowEntryProfile {
    const RxasFlowProcedure *owner;
    unsigned long epoch;
    RxasFlowEntryPolicy policy;
    unsigned int seal;
};

typedef struct PolicyViewWork {
    RxasFlowProcedure *view;
    const char **names;
    size_t *name_lengths;
    size_t names_count;
    size_t names_capacity;
    unsigned char *incoming;
    unsigned char *enabled;
    unsigned char *queued;
    size_t *queue;
    size_t *offsets;
    size_t *outgoing;
    size_t *resume_offsets;
    size_t *resume_targets;
    size_t work;
    size_t limit;
    RxasFlowPolicyViewStatus status;
} PolicyViewWork;

static int charge(PolicyViewWork *work, size_t amount) {
    if (amount > work->limit - work->work) {
        work->status = RXAS_FLOW_POLICY_VIEW_LIMIT; return 0;
    }
    work->work += amount; return 1;
}
static const Assembler_Token *operand(const instruction_queue *item, size_t index) {
    if (!item || index >= item->operandCount) return NULL;
    if (item->operandTokens) return item->operandTokens[index];
    if (index == 0) return item->operand1Token;
    if (index == 1) return item->operand2Token;
    if (index == 2) return item->operand3Token;
    return NULL;
}
static const char *policy_name(const RxasFlowProcedure *view, const RxasFlowInstruction *instruction) {
    const RxOpSignalContract *signal = &instruction->intrinsic_signal;
    if (signal->policy_source == RXOP_SIGNAL_SOURCE_STATIC_NAMES) return signal->policy_static_name;
    if (signal->policy_source == RXOP_SIGNAL_SOURCE_LITERAL_OPERAND) {
        const Assembler_Token *name = operand(view->records[instruction->record_id].queue_record, signal->policy_source_operand);
        if (name && name->token_type == STRING) return (const char *)name->token_value.string;
    }
    return NULL;
}
static int name_length(PolicyViewWork *work, const char *name, size_t *length) {
    size_t size = 0;
    for (;;) {
        if (!charge(work, 1)) return 0;
        if (!name[size]) { *length = size; return 1; }
        size++;
    }
}
static int name_index(PolicyViewWork *work, const char *name, size_t length, size_t *result) {
    size_t index;
    for (index = 1; index < work->names_count; index++) {
        size_t character;
        if (!charge(work, 1)) return 0;
        if (work->name_lengths[index] != length) continue;
        for (character = 0; character < length; character++) {
            if (!charge(work, 1)) return 0;
            if (toupper((unsigned char)work->names[index][character]) !=
                toupper((unsigned char)name[character])) break;
        }
        if (character == length) { *result = index; return 1; }
    }
    *result = 0; return 1; /* One class for every untouched signal name. */
}
static int normal_policy(PolicyViewWork *work, const RxasFlowInstruction *instruction,
                           unsigned char *state) {
    const RxOpSignalContract *signal = &instruction->intrinsic_signal;
    const char *name;
    size_t index, length;
    unsigned char effect;
    if (!instruction->op || (instruction->callback_obligations &&
        !rxop_callback_normal_preserves_frame(instruction->op->opcode))) {
        memset(state, POLICY_TOP, work->names_count); return 1;
    }
    if (!(signal->properties & RXOP_SIGNAL_PROP_POLICY_WRITE)) return 1;
    if (signal->policy_effect == RXOP_POLICY_EFFECT_PUSH) return 1;
    name = policy_name(work->view, instruction);
    if (!name || !*name) { memset(state, POLICY_TOP, work->names_count); return 1; }
    if (!name_length(work, name, &length) || !name_index(work, name, length, &index)) return 0;
    if (!index) { memset(state, POLICY_TOP, work->names_count); return 1; }
    switch (signal->policy_effect) {
        case RXOP_POLICY_EFFECT_HALT: case RXOP_POLICY_EFFECT_SILENT_HALT: effect = POLICY_HALT; break;
        case RXOP_POLICY_EFFECT_IGNORE: effect = POLICY_IGNORE; break;
        case RXOP_POLICY_EFFECT_BRANCH: case RXOP_POLICY_EFFECT_BRANCH_VALUE:
        case RXOP_POLICY_EFFECT_CALL: case RXOP_POLICY_EFFECT_CALL_ACTION:
        case RXOP_POLICY_EFFECT_CALL_BRANCH: case RXOP_POLICY_EFFECT_RETURN: effect = POLICY_OTHER; break;
        default: effect = POLICY_TOP; break;
    }
    state[index] = effect;
    return 1;
}
static int raised_policy(PolicyViewWork *work, const RxasFlowInstruction *instruction,
                          RxasFlowEdgeCause cause, const unsigned char *state, unsigned char *response) {
    const RxOpSignalContract *signal = &instruction->intrinsic_signal;
    unsigned char result = 0;
    size_t index;
    const char *start, *end;
    if (cause == RXAS_FLOW_CAUSE_CALLBACK || signal->source != RXOP_SIGNAL_SOURCE_STATIC_NAMES ||
        !signal->static_names || !*signal->static_names) {
        for (index = 0; index < work->names_count; index++) result |= state[index];
        *response = result; return 1;
    }
    start = signal->static_names;
    while (*start) {
        end = start;
        for (;;) {
            if (!charge(work, 1)) return 0;
            if (!*end || *end == '|') break;
            end++;
        }
        if (!name_index(work, start, (size_t)(end - start), &index)) return 0;
        result |= state[index];
        start = *end ? end + 1 : end;
    }
    *response = result; return 1;
}
static int append_edge(PolicyViewWork *work, size_t source, size_t target, RxasFlowEdgeKind kind,
                       RxasFlowEdgeCause cause, RxOpSignalPhase checkpoint,
                       RxasFlowEdgeResponse response, size_t producer) {
    RxasFlowProcedure *view = work->view;
    RxasFlowEdge *edges;
    size_t capacity;
    if (!charge(work, 1)) return 0;
    if (view->metrics.edges == view->edge_capacity) {
        capacity = view->edge_capacity ? view->edge_capacity * 2 : 32;
        if (capacity < view->edge_capacity || capacity > (size_t)PTRDIFF_MAX / sizeof(*edges)) return 0;
        edges = realloc(view->edges, capacity * sizeof(*edges));
        if (!edges) { work->status = RXAS_FLOW_POLICY_VIEW_MEMORY; return 0; }
        view->edges = edges; view->edge_capacity = capacity;
    }
    edges = &view->edges[view->metrics.edges++];
    edges->source = source; edges->target = target; edges->kind = kind;
    edges->cause = cause; edges->checkpoint = checkpoint;
    edges->response = response; edges->producer_instruction = producer;
    return 1;
}
static int add_cause(PolicyViewWork *work, const RxasFlowInstruction *instruction,
                     RxasFlowEdgeCause cause, RxOpSignalPhase checkpoint, unsigned int continuations) {
    RxasFlowProcedure *view = work->view;
    size_t source = instruction->block_id;
    size_t next = source + 1 < view->metrics.code_blocks ? source + 1 : view->normal_exit;
    size_t id = instruction->id;
    if (continuations & RXOP_SIGNAL_CONT_SKIP) {
        size_t cursor;
        int lexical = cause != RXAS_FLOW_CAUSE_CALLBACK || checkpoint != RXOP_SIGNAL_PHASE_AFTER_WRITES;
        if (lexical &&
            (!append_edge(work, source, next, RXAS_FLOW_EDGE_SIGNAL_SKIP, cause, checkpoint, RXAS_FLOW_RESPONSE_IGNORE, id) ||
             !append_edge(work, source, next, RXAS_FLOW_EDGE_SIGNAL_SKIP, cause, checkpoint, RXAS_FLOW_RESPONSE_HANDLED, id))) return 0;
        /* Dispatch polls the already selected next PC. A completed branch
         * resumes its normal target, not necessarily its lexical successor. */
        if (cause == RXAS_FLOW_CAUSE_CALLBACK)
            for (cursor = work->resume_offsets[source]; cursor < work->resume_offsets[source + 1]; cursor++) {
                size_t target = work->resume_targets[cursor];
                if (lexical && target == next) continue;
                if (!append_edge(work, source, target, RXAS_FLOW_EDGE_SIGNAL_SKIP, cause, checkpoint, RXAS_FLOW_RESPONSE_IGNORE, id) ||
                    !append_edge(work, source, target, RXAS_FLOW_EDGE_SIGNAL_SKIP, cause, checkpoint, RXAS_FLOW_RESPONSE_HANDLED, id)) return 0;
            }
    }
    if ((continuations & RXOP_SIGNAL_CONT_HANDLER) &&
        !append_edge(work, source, view->handler_root, RXAS_FLOW_EDGE_HANDLER, cause, checkpoint, RXAS_FLOW_RESPONSE_HANDLED, id)) return 0;
    if ((continuations & RXOP_SIGNAL_CONT_UNWIND) &&
        !append_edge(work, source, view->unwind_exit, RXAS_FLOW_EDGE_UNWIND, cause, checkpoint, RXAS_FLOW_RESPONSE_HANDLED, id)) return 0;
    if (continuations & RXOP_SIGNAL_CONT_TERMINAL) {
        if (!append_edge(work, source, view->terminal_exit, RXAS_FLOW_EDGE_TERMINAL, cause, checkpoint, RXAS_FLOW_RESPONSE_TERMINAL, id) ||
            !append_edge(work, source, view->terminal_exit, RXAS_FLOW_EDGE_TERMINAL, cause, checkpoint, RXAS_FLOW_RESPONSE_HANDLED, id)) return 0;
    }
    return 1;
}
static RxasFlowProcedure *copy_owner(const RxasFlowProcedure *owner, unsigned long epoch, size_t name_size) {
    RxasFlowProcedure *view = calloc(1, sizeof(*view));
    if (!view) return NULL;
    *view = *owner;
    view->epoch = epoch; view->analysis_manager = NULL;
    view->name = NULL; view->records = NULL; view->instructions = NULL;
    view->blocks = NULL; view->edges = NULL; view->reachable_blocks = NULL; view->labels = NULL;
    view->metrics.edges = 0; view->edge_capacity = 0;
#define COPY_FIELD(FIELD, COUNT) do { \
    if ((COUNT) > (size_t)PTRDIFF_MAX / sizeof(*view->FIELD)) goto failed; \
    view->FIELD = malloc(((COUNT) ? (COUNT) : 1) * sizeof(*view->FIELD)); \
    if (!view->FIELD) goto failed; \
    if (COUNT) memcpy(view->FIELD, owner->FIELD, (COUNT) * sizeof(*view->FIELD)); \
} while (0)
    COPY_FIELD(records, owner->metrics.records);
    COPY_FIELD(instructions, owner->metrics.instructions);
    COPY_FIELD(blocks, owner->metrics.blocks);
    COPY_FIELD(labels, owner->label_capacity);
#undef COPY_FIELD
    view->name = malloc(name_size + 1);
    view->reachable_blocks = calloc(owner->metrics.blocks ? owner->metrics.blocks : 1, 1);
    if (!view->name || !view->reachable_blocks) goto failed;
    memcpy(view->name, owner->name, name_size + 1);
    return view;
failed:
    rxas_flow_procedure_destroy(view); return NULL;
}
RxasFlowEntryProfile *rxas_flow_entry_profile_create(const RxasFlowProcedure *owner,
        unsigned long expected_epoch, RxasFlowEntryPolicy policy, int ordinary_activation) {
    RxasFlowEntryProfile *profile;
    if (!rxas_flow_procedure_epoch_matches(owner, expected_epoch) || owner->entry_policy || ordinary_activation != 1 ||
        (policy != RXAS_FLOW_ENTRY_POLICY_HALT && policy != RXAS_FLOW_ENTRY_POLICY_IGNORE)) return NULL;
    profile = calloc(1, sizeof(*profile)); if (!profile) return NULL;
    profile->owner = owner; profile->epoch = expected_epoch; profile->policy = policy; profile->seal = PROFILE_SEAL;
    return profile;
}
void rxas_flow_entry_profile_destroy(RxasFlowEntryProfile *profile) {
    if (profile) { memset(profile, 0, sizeof(*profile)); free(profile); }
}

RxasFlowProcedure *rxas_flow_procedure_entry_view(const RxasFlowProcedure *owner,
        unsigned long expected_epoch, const RxasFlowEntryProfile *profile,
        unsigned long view_epoch, size_t work_budget, RxasFlowPolicyViewStatus *status) {
    PolicyViewWork work;
    RxasFlowProcedure *view = NULL;
    size_t index, cells, head = 0, tail = 0, queued_count = 0, kept, owner_name_size;
    unsigned char *normal = NULL, *before_last = NULL, *out = NULL;
    memset(&work, 0, sizeof(work)); work.status = RXAS_FLOW_POLICY_VIEW_INVALID;
    if (!status) return NULL;
    *status = RXAS_FLOW_POLICY_VIEW_INVALID;
    if (!owner || !profile || profile->seal != PROFILE_SEAL || !view_epoch || owner->entry_policy) return NULL;
    if (!rxas_flow_procedure_epoch_matches(owner, expected_epoch) || profile->epoch != expected_epoch) {
        *status = RXAS_FLOW_POLICY_VIEW_STALE; return NULL;
    }
    if (profile->owner != owner) { *status = RXAS_FLOW_POLICY_VIEW_FOREIGN; return NULL; }
    if (!owner->metrics.complete_control_flow) { *status = RXAS_FLOW_POLICY_VIEW_INCOMPLETE; return NULL; }
    work.limit = work_budget ? work_budget : (size_t)-1;
    if (!charge(&work, owner->metrics.blocks) || !charge(&work, owner->metrics.edges) ||
        !charge(&work, owner->metrics.instructions) || !charge(&work, owner->metrics.records) ||
        !charge(&work, owner->label_capacity)) goto done;
    if (!name_length(&work, owner->name, &owner_name_size)) goto done;
    view = copy_owner(owner, view_epoch, owner_name_size);
    if (!view) { work.status = RXAS_FLOW_POLICY_VIEW_MEMORY; goto done; }
    work.view = view; work.status = RXAS_FLOW_POLICY_VIEW_MEMORY;
    if (owner->metrics.instructions == (size_t)-1) goto done;
    work.names_capacity = owner->metrics.instructions + 1;
    work.names = calloc(work.names_capacity, sizeof(*work.names));
    work.name_lengths = calloc(work.names_capacity, sizeof(*work.name_lengths));
    if (!work.names || !work.name_lengths) goto done;
    work.names[0] = ""; work.names_count = 1;
    for (index = 0; index < owner->metrics.instructions; index++) {
        const RxasFlowInstruction *instruction = &owner->instructions[index];
        const char *name = policy_name(owner, instruction);
        size_t length, found;
        if (!charge(&work, 1)) goto done;
        if (!name || !*name) continue;
        if (!name_length(&work, name, &length) || !name_index(&work, name, length, &found)) goto done;
        if (!found) {
            work.names[work.names_count] = name;
            work.name_lengths[work.names_count++] = length;
        }
    }
    if (!work_budget) {
        size_t scale = work.work;
        if (scale == (size_t)-1) goto done;
        scale++;
        if (scale > (size_t)-1 / (work.names_count + 1) / 64u) goto done;
        work.limit = scale * (work.names_count + 1) * 64u;
        if (work.work > work.limit) { work.status = RXAS_FLOW_POLICY_VIEW_LIMIT; goto done; }
    }
    /* Retain the raw graph, then create a separate cause-aware candidate. */
    for (index = 0; index < owner->metrics.edges; index++) {
        const RxasFlowEdge *edge = &owner->edges[index];
        if (edge->cause != RXAS_FLOW_CAUSE_COMBINED &&
            !append_edge(&work, edge->source, edge->target, edge->kind,
                    edge->cause, edge->checkpoint, edge->response, edge->producer_instruction)) goto done;
    }
    if (view->metrics.blocks >= (size_t)PTRDIFF_MAX / sizeof(*work.resume_offsets) ||
        view->metrics.edges > (size_t)PTRDIFF_MAX / sizeof(*work.resume_targets) ||
        !charge(&work, view->metrics.blocks) || !charge(&work, view->metrics.edges)) goto done;
    work.resume_offsets = calloc(view->metrics.blocks + 1, sizeof(*work.resume_offsets));
    work.resume_targets = calloc(view->metrics.edges ? view->metrics.edges : 1, sizeof(*work.resume_targets));
    if (!work.resume_offsets || !work.resume_targets) goto done;
    for (index = 0; index < view->metrics.edges; index++) {
        const RxasFlowEdge *edge = &view->edges[index];
        if (edge->kind == RXAS_FLOW_EDGE_NORMAL || edge->kind == RXAS_FLOW_EDGE_BRANCH)
            work.resume_offsets[edge->source + 1]++;
    }
    for (index = 1; index <= view->metrics.blocks; index++) work.resume_offsets[index] += work.resume_offsets[index - 1];
    {
        size_t *fill = calloc(view->metrics.blocks, sizeof(*fill));
        if (!fill) goto done;
        for (index = 0; index < view->metrics.edges; index++) {
            const RxasFlowEdge *edge = &view->edges[index];
            if (edge->kind == RXAS_FLOW_EDGE_NORMAL || edge->kind == RXAS_FLOW_EDGE_BRANCH)
                work.resume_targets[work.resume_offsets[edge->source] + fill[edge->source]++] = edge->target;
        }
        free(fill);
    }
    for (index = 0; index < owner->metrics.instructions; index++) {
        const RxasFlowInstruction *instruction = &owner->instructions[index];
        if (instruction->intrinsic_signal.state != RXOP_SIGNAL_STATE_NONE &&
            !add_cause(&work, instruction, RXAS_FLOW_CAUSE_INTRINSIC,
                       instruction->intrinsic_signal.phase, instruction->intrinsic_signal.continuations)) goto done;
        if (instruction->callback_obligations &&
            !add_cause(&work, instruction, RXAS_FLOW_CAUSE_CALLBACK,
                       instruction->op && rxop_callback_normal_preserves_frame(instruction->op->opcode)
                            ? RXOP_SIGNAL_PHASE_AFTER_WRITES : RXOP_SIGNAL_PHASE_UNKNOWN,
                       RXOP_SIGNAL_CONT_ALL)) goto done;
    }
    if (view->metrics.blocks > (size_t)-1 / work.names_count) goto done;
    cells = view->metrics.blocks * work.names_count;
    if (cells > (size_t)PTRDIFF_MAX ||
        view->metrics.blocks >= (size_t)PTRDIFF_MAX / sizeof(*work.offsets) ||
        view->metrics.edges > (size_t)PTRDIFF_MAX / sizeof(*work.outgoing)) goto done;
    if (!charge(&work, cells) || !charge(&work, view->metrics.edges)) goto done;
    work.incoming = calloc(cells ? cells : 1, 1);
    work.enabled = calloc(view->metrics.edges ? view->metrics.edges : 1, 1);
    work.queued = calloc(view->metrics.blocks, 1);
    work.queue = calloc(view->metrics.blocks, sizeof(*work.queue));
    work.offsets = calloc(view->metrics.blocks + 1, sizeof(*work.offsets));
    work.outgoing = calloc(view->metrics.edges ? view->metrics.edges : 1, sizeof(*work.outgoing));
    normal = calloc(work.names_count, 1); before_last = calloc(work.names_count, 1); out = calloc(work.names_count, 1);
    if (!work.incoming || !work.enabled || !work.queued || !work.queue || !work.offsets || !work.outgoing ||
        !normal || !before_last || !out) goto done;
    for (index = 0; index < view->metrics.edges; index++) work.offsets[view->edges[index].source + 1]++;
    for (index = 1; index <= view->metrics.blocks; index++) work.offsets[index] += work.offsets[index - 1];
    {
        size_t *fill = calloc(view->metrics.blocks, sizeof(*fill));
        if (!fill) goto done;
        for (index = 0; index < view->metrics.edges; index++) {
            size_t source = view->edges[index].source;
            work.outgoing[work.offsets[source] + fill[source]++] = index;
        }
        free(fill);
    }
#define ENQUEUE(BLOCK) do { if (!work.queued[(BLOCK)]) { \
    work.queue[tail] = (BLOCK); tail = (tail + 1) % view->metrics.blocks; \
    queued_count++; work.queued[(BLOCK)] = 1; } } while (0)
    memset(&work.incoming[view->entry_block * work.names_count],
           profile->policy == RXAS_FLOW_ENTRY_POLICY_HALT ? POLICY_HALT : POLICY_IGNORE, work.names_count);
    ENQUEUE(view->entry_block);
    while (queued_count) {
        size_t block_id = work.queue[head], record_id, cursor, name;
        const RxasFlowBlock *block = &view->blocks[block_id];
        unsigned char any = 0;
        head = (head + 1) % view->metrics.blocks; queued_count--; work.queued[block_id] = 0;
        if (!charge(&work, work.names_count)) goto done;
        memcpy(normal, &work.incoming[block_id * work.names_count], work.names_count);
        memcpy(before_last, normal, work.names_count);
        if (block->first_record != RXAS_FLOW_ID_NONE) {
            for (record_id = block->first_record; record_id <= block->last_record; record_id++) {
                size_t instruction_id = view->records[record_id].instruction_id;
                if (!charge(&work, 1)) goto done;
                if (instruction_id == RXAS_FLOW_ID_NONE) continue;
                if (!charge(&work, work.names_count)) goto done;
                if (instruction_id == block->last_instruction) memcpy(before_last, normal, work.names_count);
                if (!normal_policy(&work, &view->instructions[instruction_id], normal)) goto done;
            }
        }
        for (name = 0; name < work.names_count; name++) any |= normal[name];
        /* Unknown/installed handlers can also be entered asynchronously. The
         * ordinary exact root premise does not erase later handler ingress. */
        if (any & POLICY_OTHER) {
            int changed = 0;
            unsigned char *async_state = &work.incoming[view->async_root * work.names_count];
            for (name = 0; name < work.names_count; name++)
                if (async_state[name] != POLICY_TOP) { async_state[name] = POLICY_TOP; changed = 1; }
            if (changed) ENQUEUE(view->async_root);
        }
        for (cursor = work.offsets[block_id]; cursor < work.offsets[block_id + 1]; cursor++) {
            size_t edge_id = work.outgoing[cursor];
            const RxasFlowEdge *edge = &view->edges[edge_id];
            unsigned char *destination = &work.incoming[edge->target * work.names_count];
            int changed = 0, feasible = 1;
            if (!charge(&work, work.names_count)) goto done;
            memcpy(out, normal, work.names_count);
            if (edge->cause != RXAS_FLOW_CAUSE_NONE) {
                const RxasFlowInstruction *instruction = &view->instructions[edge->producer_instruction];
                unsigned char response;
                if (edge->checkpoint == RXOP_SIGNAL_PHASE_BEFORE_WRITES) memcpy(out, before_last, work.names_count);
                else if (edge->checkpoint != RXOP_SIGNAL_PHASE_AFTER_WRITES)
                    for (name = 0; name < work.names_count; name++) out[name] |= before_last[name];
                if (edge->cause == RXAS_FLOW_CAUSE_INTRINSIC && instruction->callback_obligations &&
                    (!instruction->op || !rxop_callback_normal_preserves_frame(instruction->op->opcode)))
                    memset(out, POLICY_TOP, work.names_count);
                if (!raised_policy(&work, instruction, edge->cause, out, &response)) goto done;
                if (edge->response == RXAS_FLOW_RESPONSE_IGNORE) feasible = (response & POLICY_IGNORE) != 0;
                else if (edge->response == RXAS_FLOW_RESPONSE_TERMINAL) feasible = (response & POLICY_HALT) != 0;
                else feasible = (response & POLICY_OTHER) != 0;
                if (edge->response == RXAS_FLOW_RESPONSE_HANDLED) memset(out, POLICY_TOP, work.names_count);
            }
            if (!feasible) continue;
            work.enabled[edge_id] = 1;
            for (name = 0; name < work.names_count; name++) {
                unsigned char merged = destination[name] | out[name];
                if (merged != destination[name]) { destination[name] = merged; changed = 1; }
            }
            if (changed) ENQUEUE(edge->target);
        }
    }
#undef ENQUEUE
    kept = 0;
    view->metrics.normal_edges = view->metrics.branch_edges = view->metrics.signal_skip_edges = 0;
    view->metrics.handler_edges = view->metrics.terminal_edges = view->metrics.unwind_edges = view->metrics.unknown_edges = 0;
    for (index = 0; index < view->metrics.blocks; index++)
        view->reachable_blocks[index] = work.incoming[index * work.names_count] != 0;
    for (index = 0; index < view->metrics.edges; index++) {
        RxasFlowEdge edge = view->edges[index];
        if (!work.enabled[index] || !view->reachable_blocks[edge.source]) continue;
        view->edges[kept++] = edge;
        switch (edge.kind) {
            case RXAS_FLOW_EDGE_NORMAL: view->metrics.normal_edges++; break;
            case RXAS_FLOW_EDGE_BRANCH: view->metrics.branch_edges++; break;
            case RXAS_FLOW_EDGE_SIGNAL_SKIP: view->metrics.signal_skip_edges++; break;
            case RXAS_FLOW_EDGE_HANDLER: view->metrics.handler_edges++; break;
            case RXAS_FLOW_EDGE_TERMINAL: view->metrics.terminal_edges++; break;
            case RXAS_FLOW_EDGE_UNWIND: view->metrics.unwind_edges++; break;
            case RXAS_FLOW_EDGE_UNKNOWN: view->metrics.unknown_edges++; break;
        }
    }
    view->metrics.edges = kept; view->entry_policy = profile->policy;
    view->profile_source_epoch = expected_epoch; view->policy_view_work = work.work;
    work.status = RXAS_FLOW_POLICY_VIEW_OK;
done:
    free(work.names); free(work.name_lengths); free(work.incoming); free(work.enabled); free(work.queued);
    free(work.queue); free(work.offsets); free(work.outgoing); free(normal); free(before_last); free(out);
    free(work.resume_offsets); free(work.resume_targets);
    *status = work.status;
    if (work.status != RXAS_FLOW_POLICY_VIEW_OK) { rxas_flow_procedure_destroy(view); return NULL; }
    return view;
}
