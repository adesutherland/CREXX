/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXAS_FLOW_PASS_H
#define CREXX_RXAS_FLOW_PASS_H

#include <stddef.h>
#include <stdio.h>

#include "rxas.h"

/* Stable routing identities for every current RXAS optimisation family.
 * A route says which facts a consumer may request; it does not authorize a
 * rewrite.  Candidate counts are deliberately conservative demand filters. */
typedef enum RxasOptimisationPassId {
    RXAS_PASS_LOCAL_FIXED_REGISTER = 0,
    RXAS_PASS_LOCAL_CNOP,
    RXAS_PASS_LOCAL_SWAP_PACK,
    RXAS_PASS_LOCAL_CALL_PACK,
    RXAS_PASS_LOCAL_NULL_PACK,
    RXAS_PASS_LOCAL_CONCAT,
    RXAS_PASS_K06_STATUS_COPY,
    RXAS_PASS_LOCAL_LOOP_BRANCH,
    RXAS_PASS_LOCAL_SINGLE_USE_COPY,
    RXAS_PASS_M00_REACHABILITY,
    RXAS_PASS_M01_DERIVATION,
    RXAS_PASS_M02_CONSTANT,
    RXAS_PASS_M03_ABSENT,
    RXAS_PASS_M04_SELF_COPY,
    RXAS_PASS_M05_TYPED_COPY,
    RXAS_PASS_M06_PRODUCER_FORWARD,
    RXAS_PASS_X01_COMPONENT_PLACEMENT,
    RXAS_PASS_M07_STORAGE_DIAGNOSTIC,
    RXAS_PASS_DIAGNOSTIC_FLOW_DUMP,
    RXAS_PASS_K01_STORAGE_PERMUTATION,
    RXAS_PASS_K02_K03_LINKED_READ,
    RXAS_PASS_K04_COMPARE_BRANCH,
    RXAS_PASS_K05_BRANCH_THREAD,
    RXAS_PASS_H01_JOINED_KEY_REUSE,
    RXAS_PASS_H02_STRING_LITERAL_REUSE,
    RXAS_PASS_M08_SUCCESSFUL_GUARD,
    RXAS_PASS_COUNT
} RxasOptimisationPassId;

typedef enum RxasFlowCapability {
    RXAS_FLOW_CAP_LOCAL_SCAN = 1u << 0,
    RXAS_FLOW_CAP_CFG = 1u << 1,
    RXAS_FLOW_CAP_DOMINANCE = 1u << 2,
    RXAS_FLOW_CAP_SIGNAL = 1u << 3,
    RXAS_FLOW_CAP_STORAGE = 1u << 4,
    RXAS_FLOW_CAP_VALUE = 1u << 5,
    RXAS_FLOW_CAP_USE = 1u << 6,
    RXAS_FLOW_CAP_LOOPS = 1u << 7
} RxasFlowCapability;

typedef enum RxasOptimisationOwner {
    RXAS_OPT_OWNER_LOCAL = 0,
    RXAS_OPT_OWNER_CFG,
    RXAS_OPT_OWNER_SSA,
    RXAS_OPT_OWNER_DIAGNOSTIC
} RxasOptimisationOwner;

typedef struct RxasOptimisationPassDescriptor {
    RxasOptimisationPassId id;
    const char *name;
    RxasOptimisationOwner owner;
    unsigned int capabilities;
} RxasOptimisationPassDescriptor;

typedef struct RxasOptimisationCensus {
    size_t records;
    size_t instructions;
    size_t candidates[RXAS_PASS_COUNT];
    unsigned int requested_capabilities;
} RxasOptimisationCensus;

size_t rxas_optimisation_pass_count(void);
const RxasOptimisationPassDescriptor *rxas_optimisation_pass_descriptor(
        RxasOptimisationPassId id);

int rxas_optimisation_census(Assembler_Context *context,
                             const instruction_queue *items,
                             size_t item_count,
                             RxasOptimisationCensus *census);
int rxas_optimisation_has_candidates(const RxasOptimisationCensus *census,
                                     RxasOptimisationPassId id);
unsigned int rxas_optimisation_capabilities_for_owner(
        const RxasOptimisationCensus *census, RxasOptimisationOwner owner);
void rxas_optimisation_census_dump(const RxasOptimisationCensus *census,
                                   const char *procedure_name,
                                   FILE *stream);

#endif
