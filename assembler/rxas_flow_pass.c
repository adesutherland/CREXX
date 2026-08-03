/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxas_flow_pass.h"

#include "rxas_flow_graph.h"
#include "rxdefs.h"
#include "rxasgrmr.h"

#include <string.h>

#define CAP_SEMANTIC_BASE \
    (RXAS_FLOW_CAP_CFG | RXAS_FLOW_CAP_DOMINANCE | RXAS_FLOW_CAP_SIGNAL | \
     RXAS_FLOW_CAP_STORAGE | RXAS_FLOW_CAP_VALUE)
#define CAP_SEMANTIC_USE (CAP_SEMANTIC_BASE | RXAS_FLOW_CAP_USE)

static const RxasOptimisationPassDescriptor pass_descriptors[] = {
    {RXAS_PASS_LOCAL_FIXED_REGISTER, "local-fixed-register", RXAS_OPT_OWNER_LOCAL, RXAS_FLOW_CAP_LOCAL_SCAN},
    {RXAS_PASS_LOCAL_CNOP, "local-cnop", RXAS_OPT_OWNER_LOCAL, RXAS_FLOW_CAP_LOCAL_SCAN},
    {RXAS_PASS_LOCAL_SWAP_PACK, "local-swap-pack", RXAS_OPT_OWNER_LOCAL, RXAS_FLOW_CAP_LOCAL_SCAN},
    {RXAS_PASS_LOCAL_CALL_PACK, "local-call-pack", RXAS_OPT_OWNER_LOCAL, RXAS_FLOW_CAP_LOCAL_SCAN},
    {RXAS_PASS_LOCAL_NULL_PACK, "local-null-pack", RXAS_OPT_OWNER_LOCAL, RXAS_FLOW_CAP_LOCAL_SCAN},
    {RXAS_PASS_LOCAL_CONCAT, "local-concat", RXAS_OPT_OWNER_LOCAL, RXAS_FLOW_CAP_LOCAL_SCAN},
    {RXAS_PASS_K06_STATUS_COPY, "K06-status-copy", RXAS_OPT_OWNER_LOCAL, RXAS_FLOW_CAP_LOCAL_SCAN},
    {RXAS_PASS_LOCAL_LOOP_BRANCH, "local-loop-branch", RXAS_OPT_OWNER_LOCAL, RXAS_FLOW_CAP_LOCAL_SCAN},
    {RXAS_PASS_LOCAL_SINGLE_USE_COPY, "local-single-use-copy", RXAS_OPT_OWNER_LOCAL, RXAS_FLOW_CAP_LOCAL_SCAN},
    {RXAS_PASS_M00_REACHABILITY, "M00-reachability", RXAS_OPT_OWNER_CFG, RXAS_FLOW_CAP_CFG},
    {RXAS_PASS_M01_DERIVATION, "M01-derivation", RXAS_OPT_OWNER_SSA, CAP_SEMANTIC_BASE},
    {RXAS_PASS_M02_CONSTANT, "M02-constant", RXAS_OPT_OWNER_SSA, CAP_SEMANTIC_BASE},
    {RXAS_PASS_M03_ABSENT, "M03-absent", RXAS_OPT_OWNER_SSA, CAP_SEMANTIC_BASE},
    {RXAS_PASS_M04_SELF_COPY, "M04-self-copy", RXAS_OPT_OWNER_SSA, CAP_SEMANTIC_BASE},
    {RXAS_PASS_M05_TYPED_COPY, "M05-typed-copy", RXAS_OPT_OWNER_SSA, CAP_SEMANTIC_USE},
    {RXAS_PASS_M06_PRODUCER_FORWARD, "M06-producer-forward", RXAS_OPT_OWNER_SSA, CAP_SEMANTIC_USE},
    {RXAS_PASS_M07_STORAGE_DIAGNOSTIC, "M07-storage-diagnostic",
     RXAS_OPT_OWNER_DIAGNOSTIC,
     RXAS_FLOW_CAP_CFG | RXAS_FLOW_CAP_SIGNAL | RXAS_FLOW_CAP_STORAGE},
    {RXAS_PASS_DIAGNOSTIC_FLOW_DUMP, "diagnostic-flow-dump",
     RXAS_OPT_OWNER_DIAGNOSTIC, CAP_SEMANTIC_USE},
    {RXAS_PASS_K01_STORAGE_PERMUTATION, "K01-storage-permutation", RXAS_OPT_OWNER_SSA, CAP_SEMANTIC_USE},
    {RXAS_PASS_K02_K03_LINKED_READ, "K02-K03-linked-read", RXAS_OPT_OWNER_SSA, CAP_SEMANTIC_USE},
    {RXAS_PASS_K04_COMPARE_BRANCH, "K04-compare-branch", RXAS_OPT_OWNER_SSA, CAP_SEMANTIC_USE},
    {RXAS_PASS_K05_BRANCH_THREAD, "K05-branch-thread", RXAS_OPT_OWNER_CFG, RXAS_FLOW_CAP_CFG}
};

static int pass_is_copy_candidate(int opcode) {
    return opcode == OP_ICOPY_REG_REG || opcode == OP_FCOPY_REG_REG ||
           opcode == OP_SCOPY_REG_REG;
}

static int pass_is_linked_read_candidate(int opcode) {
    return opcode == OP_LINK_REG_REG ||
           opcode == OP_LINKATTR1_REG_REG_INT;
}

static int pass_is_branch_thread_candidate(int opcode) {
    return opcode == OP_BR_ID || opcode == OP_BRT_ID_REG ||
           opcode == OP_BRF_ID_REG || opcode == OP_BRTF_ID_ID_REG;
}

static int pass_is_compare_candidate(int opcode) {
    return rxop_compare_branch_fusion(opcode, OP_BRT_ID_REG, 0) ||
           rxop_compare_branch_fusion(opcode, OP_BRF_ID_REG, 0);
}

static int pass_is_scalar_constant_candidate(
        int opcode, const instruction_queue *item) {
    const Assembler_Token *constant;
    unsigned int component;
    if (!item || item->operandCount != 2) return 0;
    constant = rxas_queue_operand(item, 1);
    component = rxop_component_writes(opcode, 0);
    return constant &&
           ((component == RXOP_COMPONENT_INTEGER &&
             constant->token_type == INT) ||
            (component == RXOP_COMPONENT_FLOAT &&
             constant->token_type == FLOAT));
}

static void pass_add_candidate(RxasOptimisationCensus *census,
                               RxasOptimisationPassId id) {
    const RxasOptimisationPassDescriptor *descriptor;
    if (!census || id < 0 || id >= RXAS_PASS_COUNT) return;
    census->candidates[id]++;
    descriptor = rxas_optimisation_pass_descriptor(id);
    if (descriptor)
        census->requested_capabilities |= descriptor->capabilities;
}

size_t rxas_optimisation_pass_count(void) {
    return sizeof(pass_descriptors) / sizeof(pass_descriptors[0]);
}

const RxasOptimisationPassDescriptor *rxas_optimisation_pass_descriptor(
        RxasOptimisationPassId id) {
    if (id < 0 || id >= RXAS_PASS_COUNT ||
        (size_t)id >= rxas_optimisation_pass_count())
        return 0;
    return &pass_descriptors[id];
}

int rxas_optimisation_census(Assembler_Context *context,
                             const instruction_queue *items,
                             size_t item_count,
                             RxasOptimisationCensus *census) {
    const instruction_queue *item;
    const OpInfo *op;
    size_t index;
    int opcode;
    if (!items || !census) return 0;
    memset(census, 0, sizeof(*census));
    census->records = item_count;
    for (index = 0; index < item_count; index++) {
        item = &items[index];
        if (item->instrType != OP_CODE) continue;
        census->instructions++;
        op = rxas_flow_resolve_opcode(context, item);
        if (!op) continue;
        opcode = op->opcode;

        /* M00 is a procedure-level structural consumer.  Counting every
         * executable record deliberately overselects rather than risking an
         * unreachable block being hidden by an incomplete demand filter. */
        pass_add_candidate(census, RXAS_PASS_M00_REACHABILITY);

        if (opcode == OP_INC_REG || opcode == OP_DEC_REG)
            pass_add_candidate(census, RXAS_PASS_LOCAL_FIXED_REGISTER);
        if (opcode == OP_CNOP)
            pass_add_candidate(census, RXAS_PASS_LOCAL_CNOP);
        if (opcode == OP_SWAP_REG_REG)
            pass_add_candidate(census, RXAS_PASS_LOCAL_SWAP_PACK);
        if (opcode == OP_SETTP_REG_INT || opcode == OP_LOAD_REG_INT)
            pass_add_candidate(census, RXAS_PASS_LOCAL_CALL_PACK);
        if (opcode == OP_NULL_REG)
            pass_add_candidate(census, RXAS_PASS_LOCAL_NULL_PACK);
        if (opcode == OP_CONCAT_REG_REG_REG ||
            opcode == OP_SCONCAT_REG_REG_REG)
            pass_add_candidate(census, RXAS_PASS_LOCAL_CONCAT);
        if (opcode == OP_COPY_REG_REG || opcode == OP_ACOPY_REG_REG)
            pass_add_candidate(census, RXAS_PASS_K06_STATUS_COPY);
        if (opcode == OP_INC_REG || opcode == OP_BR_ID)
            pass_add_candidate(census, RXAS_PASS_LOCAL_LOOP_BRANCH);

        if (rxop_value_derivation(opcode) != RXOP_DERIVATION_NONE)
            pass_add_candidate(census, RXAS_PASS_M01_DERIVATION);
        if (pass_is_scalar_constant_candidate(opcode, item))
            pass_add_candidate(census, RXAS_PASS_M02_CONSTANT);
        if (opcode == OP_NULL_REG)
            pass_add_candidate(census, RXAS_PASS_M03_ABSENT);
        if (rxop_same_storage_copy_is_noop(opcode))
            pass_add_candidate(census, RXAS_PASS_M04_SELF_COPY);
        if (pass_is_copy_candidate(opcode)) {
            pass_add_candidate(census, RXAS_PASS_LOCAL_SINGLE_USE_COPY);
            pass_add_candidate(census, RXAS_PASS_M05_TYPED_COPY);
        }
        if (opcode == OP_ICOPY_REG_REG || opcode == OP_FCOPY_REG_REG)
            pass_add_candidate(census, RXAS_PASS_M06_PRODUCER_FORWARD);
        if (context && context->debug_mode) {
            pass_add_candidate(census, RXAS_PASS_M07_STORAGE_DIAGNOSTIC);
            pass_add_candidate(census, RXAS_PASS_DIAGNOSTIC_FLOW_DUMP);
        }
        if (opcode == OP_SWAP_REG_REG ||
            opcode == OP_SWAPN_REG_REG_REG_REG)
            pass_add_candidate(census, RXAS_PASS_K01_STORAGE_PERMUTATION);
        if (pass_is_linked_read_candidate(opcode))
            pass_add_candidate(census, RXAS_PASS_K02_K03_LINKED_READ);
        if (pass_is_compare_candidate(opcode))
            pass_add_candidate(census, RXAS_PASS_K04_COMPARE_BRANCH);
        if (pass_is_branch_thread_candidate(opcode))
            pass_add_candidate(census, RXAS_PASS_K05_BRANCH_THREAD);
    }
    return 1;
}

int rxas_optimisation_has_candidates(const RxasOptimisationCensus *census,
                                     RxasOptimisationPassId id) {
    return census && id >= 0 && id < RXAS_PASS_COUNT &&
           census->candidates[id] != 0;
}

unsigned int rxas_optimisation_capabilities_for_owner(
        const RxasOptimisationCensus *census, RxasOptimisationOwner owner) {
    unsigned int capabilities;
    size_t index;
    if (!census || owner < RXAS_OPT_OWNER_LOCAL ||
        owner > RXAS_OPT_OWNER_DIAGNOSTIC)
        return 0;
    capabilities = 0;
    for (index = 0; index < rxas_optimisation_pass_count(); index++)
        if (pass_descriptors[index].owner == owner &&
            census->candidates[index])
            capabilities |= pass_descriptors[index].capabilities;
    return capabilities;
}

void rxas_optimisation_census_dump(const RxasOptimisationCensus *census,
                                   const char *procedure_name,
                                   FILE *stream) {
    size_t index;
    if (!census || !stream) return;
    fprintf(stream,
            "PERF3 optimisation-census procedure=%s records=%llu "
            "instructions=%llu capabilities=0x%x",
            procedure_name ? procedure_name : "(directives)",
            (unsigned long long)census->records,
            (unsigned long long)census->instructions,
            census->requested_capabilities);
    for (index = 0; index < rxas_optimisation_pass_count(); index++)
        if (census->candidates[index])
            fprintf(stream, " %s=%llu", pass_descriptors[index].name,
                    (unsigned long long)census->candidates[index]);
    fputc('\n', stream);
}
