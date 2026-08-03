/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxdefs.h"

typedef struct {
    int opcode;
    RxOpEffectState state;
    unsigned int reads;
    unsigned int writes;
    unsigned int kills;
    unsigned int branch_targets;
    const char *reads_signature;
    const char *writes_signature;
    const char *kills_signature;
    const char *branch_targets_signature;
    RxOpImplicitEffect implicit;
    unsigned int semantics;
    unsigned int cursor_reads;
    unsigned int cursor_writes;
    RxOpConstEvaluator const_evaluator;
} RxOpEffectSpec;

#define RXE(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS) \
    STATE, READS, WRITES, KILLS, BRANCH_TARGETS, NULL, NULL, NULL, NULL, IMPLICIT, SEMANTICS, \
    RXOP_OP_NONE, RXOP_OP_NONE, RXOP_CONST_EVAL_NONE
#define RXEV(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS) \
    STATE, RXOP_OP_NONE, RXOP_OP_NONE, RXOP_OP_NONE, RXOP_OP_NONE, \
    READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS, \
    RXOP_OP_NONE, RXOP_OP_NONE, RXOP_CONST_EVAL_NONE
#define RXEC(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS, \
             CURSOR_READS, CURSOR_WRITES, CONST_EVALUATOR) \
    STATE, READS, WRITES, KILLS, BRANCH_TARGETS, NULL, NULL, NULL, NULL, IMPLICIT, SEMANTICS, \
    CURSOR_READS, CURSOR_WRITES, CONST_EVALUATOR
#define RXOP_EFFECT(NAME, EFFECTS) { OP_##NAME, EFFECTS },
static const RxOpEffectSpec rxop_effect_specs[] = {
#include "rxopeffects.h"
};
#undef RXOP_EFFECT
#undef RXEC
#undef RXEV
#undef RXE

#define RXSC_NONE \
    RXOP_SIGNAL_STATE_NONE, RXOP_SIGNAL_PHASE_NONE, RXOP_SIGNAL_SOURCE_NONE, \
    SIZE_MAX, NULL, RXOP_OP_NONE, NULL, RXOP_COMPONENT_NONE, \
    RXOP_CONTEXT_NONE, RXOP_SIGNAL_DEP_NONE, RXOP_SIGNAL_CONT_NORMAL, \
    RXOP_SIGNAL_PROP_NONE, RXOP_POLICY_EFFECT_NONE, \
    RXOP_SIGNAL_SOURCE_NONE, SIZE_MAX, NULL
#define RXSC_NONE_STABLE(DEPENDENCIES) \
    RXOP_SIGNAL_STATE_NONE, RXOP_SIGNAL_PHASE_NONE, RXOP_SIGNAL_SOURCE_NONE, \
    SIZE_MAX, NULL, RXOP_OP_NONE, NULL, RXOP_COMPONENT_NONE, \
    RXOP_CONTEXT_NONE, DEPENDENCIES, RXOP_SIGNAL_CONT_NORMAL, \
    RXOP_SIGNAL_PROP_SUCCESS_STABLE, RXOP_POLICY_EFFECT_NONE, \
    RXOP_SIGNAL_SOURCE_NONE, SIZE_MAX, NULL
#define RXSC_NONE_POLICY(EFFECT, POLICY_SOURCE, POLICY_OPERAND, POLICY_NAME) \
    RXOP_SIGNAL_STATE_NONE, RXOP_SIGNAL_PHASE_NONE, RXOP_SIGNAL_SOURCE_NONE, \
    SIZE_MAX, NULL, RXOP_OP_NONE, NULL, RXOP_COMPONENT_NONE, \
    RXOP_CONTEXT_NONE, RXOP_SIGNAL_DEP_HANDLER_POLICY, \
    RXOP_SIGNAL_CONT_NORMAL, RXOP_SIGNAL_PROP_POLICY_WRITE, \
    EFFECT, POLICY_SOURCE, POLICY_OPERAND, POLICY_NAME
#define RXSC_UNKNOWN \
    RXOP_SIGNAL_STATE_UNKNOWN, RXOP_SIGNAL_PHASE_UNKNOWN, \
    RXOP_SIGNAL_SOURCE_UNKNOWN, SIZE_MAX, NULL, RXOP_OP_ALL, NULL, \
    RXOP_COMPONENT_ALL, RXOP_CONTEXT_NUMERIC, RXOP_SIGNAL_DEP_UNKNOWN, \
    RXOP_SIGNAL_CONT_ALL, \
    RXOP_SIGNAL_PROP_ADDRESS_OBSERVABLE | RXOP_SIGNAL_PROP_PAYLOAD_OBSERVABLE, \
    RXOP_POLICY_EFFECT_NONE, RXOP_SIGNAL_SOURCE_NONE, SIZE_MAX, NULL
#define RXSC_STATIC(PHASE, NAMES, FAILURE_WRITES, FAILURE_COMPONENTS, \
                    FAILURE_CONTEXT, DEPENDENCIES, PROPERTIES) \
    RXOP_SIGNAL_STATE_KNOWN, PHASE, RXOP_SIGNAL_SOURCE_STATIC_NAMES, SIZE_MAX, \
    NAMES, FAILURE_WRITES, NULL, FAILURE_COMPONENTS, FAILURE_CONTEXT, \
    DEPENDENCIES, RXOP_SIGNAL_CONT_ALL, \
    RXOP_SIGNAL_PROP_ADDRESS_OBSERVABLE | \
        RXOP_SIGNAL_PROP_PAYLOAD_OBSERVABLE | PROPERTIES, \
    RXOP_POLICY_EFFECT_NONE, RXOP_SIGNAL_SOURCE_NONE, SIZE_MAX, NULL
#define RXSC_STATIC_POLICY(PHASE, NAMES, FAILURE_WRITES, FAILURE_COMPONENTS, \
                    FAILURE_CONTEXT, DEPENDENCIES, PROPERTIES, EFFECT, \
                    POLICY_SOURCE, POLICY_OPERAND, POLICY_NAME) \
    RXOP_SIGNAL_STATE_KNOWN, PHASE, RXOP_SIGNAL_SOURCE_STATIC_NAMES, SIZE_MAX, \
    NAMES, FAILURE_WRITES, NULL, FAILURE_COMPONENTS, FAILURE_CONTEXT, \
    DEPENDENCIES, RXOP_SIGNAL_CONT_ALL, \
    RXOP_SIGNAL_PROP_ADDRESS_OBSERVABLE | \
        RXOP_SIGNAL_PROP_PAYLOAD_OBSERVABLE | PROPERTIES, \
    EFFECT, POLICY_SOURCE, POLICY_OPERAND, POLICY_NAME
#define RXSC_DYNAMIC(PHASE, SOURCE, SOURCE_OPERAND, DEPENDENCIES, PROPERTIES) \
    RXOP_SIGNAL_STATE_KNOWN, PHASE, SOURCE, SOURCE_OPERAND, NULL, \
    RXOP_OP_NONE, NULL, RXOP_COMPONENT_NONE, RXOP_CONTEXT_NONE, DEPENDENCIES, \
    RXOP_SIGNAL_CONT_ALL, RXOP_SIGNAL_PROP_ADDRESS_OBSERVABLE | \
        RXOP_SIGNAL_PROP_PAYLOAD_OBSERVABLE | PROPERTIES, \
    RXOP_POLICY_EFFECT_NONE, RXOP_SIGNAL_SOURCE_NONE, SIZE_MAX, NULL
#define RXSC_PLUGIN(PHASE, FAILURE_WRITES, FAILURE_COMPONENTS, DEPENDENCIES, \
                    PROPERTIES) \
    RXOP_SIGNAL_STATE_KNOWN, PHASE, RXOP_SIGNAL_SOURCE_PLUGIN, SIZE_MAX, NULL, \
    FAILURE_WRITES, NULL, FAILURE_COMPONENTS, RXOP_CONTEXT_NONE, DEPENDENCIES, \
    RXOP_SIGNAL_CONT_ALL, RXOP_SIGNAL_PROP_ADDRESS_OBSERVABLE | \
        RXOP_SIGNAL_PROP_PAYLOAD_OBSERVABLE | PROPERTIES, \
    RXOP_POLICY_EFFECT_NONE, RXOP_SIGNAL_SOURCE_NONE, SIZE_MAX, NULL
#define RXSC_DECIMAL_COMPARE \
    RXSC_PLUGIN(RXOP_SIGNAL_PHASE_AFTER_WRITES, RXOP_OP_1, \
                RXOP_COMPONENT_INTEGER, \
                RXOP_SIGNAL_DEP_NUMERIC_CONTEXT | RXOP_SIGNAL_DEP_PLUGIN, \
                RXOP_SIGNAL_PROP_SUCCESS_STABLE)
#define RXOP_SIGNAL(NAME, CONTRACT) { OP_##NAME, CONTRACT },
static const RxOpSignalContract rxop_signal_contracts[] = {
#include "rxopsignals.h"
};
#undef RXOP_SIGNAL
#undef RXSC_PLUGIN
#undef RXSC_DECIMAL_COMPARE
#undef RXSC_DYNAMIC
#undef RXSC_STATIC_POLICY
#undef RXSC_STATIC
#undef RXSC_UNKNOWN
#undef RXSC_NONE_POLICY
#undef RXSC_NONE_STABLE
#undef RXSC_NONE

RxOpEffects rxop_effects(int opcode) {
    RxOpEffects effects;
    const RxOpEffectSpec *spec;

    effects.opcode = opcode;
    effects.state = RXOP_EFFECT_CONSERVATIVE;
    effects.reads = RXOP_OP_ALL;
    effects.writes = RXOP_OP_ALL;
    effects.kills = RXOP_OP_NONE;
    effects.branch_targets = RXOP_OP_ALL;
    effects.reads_signature = NULL;
    effects.writes_signature = NULL;
    effects.kills_signature = NULL;
    effects.branch_targets_signature = NULL;
    effects.implicit = RXOP_IMPLICIT_NONE;
    effects.semantics = RXOP_SEM_OPAQUE;
    effects.cursor_reads = RXOP_OP_ALL;
    effects.cursor_writes = RXOP_OP_ALL;
    effects.const_evaluator = RXOP_CONST_EVAL_NONE;
    effects.flow = FLOW_TERM;
    effects.optimizer_barrier = 1;

    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS) return effects;
    if ((size_t)opcode >= rxop_effect_count()) return effects;

    spec = &rxop_effect_specs[opcode];
    if (spec->opcode != opcode) return effects;

    effects.state = spec->state;
    effects.reads = spec->reads;
    effects.writes = spec->writes;
    effects.kills = spec->kills;
    effects.branch_targets = spec->branch_targets;
    effects.reads_signature = spec->reads_signature;
    effects.writes_signature = spec->writes_signature;
    effects.kills_signature = spec->kills_signature;
    effects.branch_targets_signature = spec->branch_targets_signature;
    effects.implicit = spec->implicit;
    effects.semantics = spec->semantics;
    effects.cursor_reads = spec->cursor_reads;
    effects.cursor_writes = spec->cursor_writes;
    effects.const_evaluator = spec->const_evaluator;
    effects.flow = op_table[opcode].flow;
    effects.optimizer_barrier =
        spec->state != RXOP_EFFECT_CLASSIFIED ||
        (op_table[opcode].flags & FLG_OPT_BARRIER) != 0;

    return effects;
}

size_t rxop_effect_count(void) {
    return sizeof(rxop_effect_specs) / sizeof(rxop_effect_specs[0]);
}

static int rxop_effect_has_operand(unsigned int legacy_mask,
                                   const char *signature,
                                   size_t operand_index) {
    if (signature) {
        size_t length = strlen(signature);
        return operand_index < length && signature[operand_index] == '1';
    }
    if (legacy_mask == RXOP_OP_ALL) return 1;
    if (operand_index >= 3) return 0;
    return (legacy_mask & (1u << operand_index)) != 0;
}

int rxop_effect_reads_operand(const RxOpEffects *effects, size_t operand_index) {
    return effects && rxop_effect_has_operand(effects->reads, effects->reads_signature, operand_index);
}

int rxop_effect_writes_operand(const RxOpEffects *effects, size_t operand_index) {
    return effects && rxop_effect_has_operand(effects->writes, effects->writes_signature, operand_index);
}

int rxop_effect_kills_operand(const RxOpEffects *effects, size_t operand_index) {
    return effects && rxop_effect_has_operand(effects->kills, effects->kills_signature, operand_index);
}

int rxop_effect_branch_target_operand(const RxOpEffects *effects, size_t operand_index) {
    return effects && rxop_effect_has_operand(effects->branch_targets,
                                              effects->branch_targets_signature,
                                              operand_index);
}

int rxop_effect_reads_cursor(const RxOpEffects *effects, size_t operand_index) {
    return effects && rxop_effect_has_operand(effects->cursor_reads, NULL,
                                              operand_index);
}

int rxop_effect_writes_cursor(const RxOpEffects *effects, size_t operand_index) {
    return effects && rxop_effect_has_operand(effects->cursor_writes, NULL,
                                              operand_index);
}

unsigned int rxop_component_reads(int opcode, size_t operand_index) {
    if (opcode == OP_COPY_REG_REG && operand_index == 1) return RXOP_COMPONENT_ALL;
    if (opcode == OP_ICOPY_REG_REG && operand_index == 1) return RXOP_COMPONENT_INTEGER;
    if (opcode == OP_FCOPY_REG_REG && operand_index == 1) return RXOP_COMPONENT_FLOAT;
    if (opcode == OP_SCOPY_REG_REG && operand_index == 1) return RXOP_COMPONENT_STRING;
    if (opcode == OP_DCOPY_REG_REG && operand_index == 1) return RXOP_COMPONENT_DECIMAL;
    if (opcode == OP_ACOPY_REG_REG && operand_index == 1) return RXOP_COMPONENT_ATTRIBUTES;
    if (opcode == OP_BCOPY_REG_REG && operand_index == 1)
        return RXOP_COMPONENT_BINARY | RXOP_COMPONENT_NATIVE_PAYLOAD;

    if (opcode >= OP_IADD_REG_REG_REG && opcode <= OP_DEC_REG)
        return RXOP_COMPONENT_INTEGER;
    if (opcode >= OP_IEQ_REG_REG_REG && opcode <= OP_ILTE_REG_INT_REG)
        return operand_index == 0 ? RXOP_COMPONENT_ALL : RXOP_COMPONENT_INTEGER;
    if (opcode == OP_BRT_ID_REG || opcode == OP_BRF_ID_REG ||
        opcode == OP_BRTF_ID_ID_REG || opcode == OP_BEQ_ID_REG_REG ||
        opcode == OP_BEQ_ID_REG_INT || opcode == OP_BNE_ID_REG_REG ||
        opcode == OP_BNE_ID_REG_INT)
        return RXOP_COMPONENT_INTEGER;
    if ((opcode == OP_SEQ_REG_REG_REG || opcode == OP_SEQ_REG_REG_STRING ||
         (opcode >= OP_SNE_REG_REG_REG && opcode <= OP_SLTE_REG_STRING_REG)) &&
        operand_index != 0)
        return RXOP_COMPONENT_STRING;
    if (opcode >= OP_FEQ_REG_REG_REG && opcode <= OP_FLTE_REG_FLOAT_REG)
        return operand_index == 0 ? RXOP_COMPONENT_ALL : RXOP_COMPONENT_FLOAT;
    if (opcode >= OP_DEQ_REG_REG_REG && opcode <= OP_DLTE_REG_DECIMAL_REG)
        return operand_index == 0 ? RXOP_COMPONENT_ALL : RXOP_COMPONENT_DECIMAL;
    if (opcode == OP_BINEQ_REG_REG_REG || opcode == OP_BINEQ_REG_REG_BINARY ||
        opcode == OP_BINNE_REG_REG_REG || opcode == OP_BINNE_REG_REG_BINARY)
        return operand_index == 0 ? RXOP_COMPONENT_ALL : RXOP_COMPONENT_BINARY;
    if (opcode == OP_BTOI_REG || opcode == OP_BTOD_REG ||
        opcode == OP_BTOF_REG || opcode == OP_BTOS_REG ||
        opcode == OP_ITOS_REG || opcode == OP_ITOF_REG ||
        opcode == OP_ITOB_REG || opcode == OP_ITOD_REG)
        return RXOP_COMPONENT_INTEGER;
    if (opcode == OP_FTOS_REG || opcode == OP_FTOI_REG ||
        opcode == OP_FTOB_REG || opcode == OP_FTOD_REG)
        return RXOP_COMPONENT_FLOAT;
    if (opcode == OP_STOB_REG || opcode == OP_STOF_REG ||
        opcode == OP_STOI_REG || opcode == OP_STOD_REG)
        return RXOP_COMPONENT_STRING;
    if (opcode == OP_DTOS_REG || opcode == OP_DTOI_REG ||
        opcode == OP_DTOB_REG || opcode == OP_DTOF_REG)
        return RXOP_COMPONENT_DECIMAL;
    if (opcode == OP_ITOF_REG_REG && operand_index == 1)
        return RXOP_COMPONENT_INTEGER;
    return RXOP_COMPONENT_ALL;
}

int rxop_same_storage_copy_is_noop(int opcode) {
    switch (opcode) {
        case OP_COPY_REG_REG:
        case OP_ICOPY_REG_REG:
        case OP_FCOPY_REG_REG:
        case OP_SCOPY_REG_REG:
        case OP_DCOPY_REG_REG:
        case OP_ACOPY_REG_REG:
        case OP_BCOPY_REG_REG:
            return 1;
        default:
            return 0;
    }
}

/* NONE means the opcode-effects inventory proves a register write but the
 * component changed by that write is not yet exact. */
unsigned int rxop_component_writes(int opcode, size_t operand_index) {
    if (operand_index != 0) return RXOP_COMPONENT_NONE;
    if (opcode == OP_COPY_REG_REG || opcode == OP_NULL_REG)
        return RXOP_COMPONENT_ALL;
    if (opcode == OP_ICOPY_REG_REG || opcode == OP_LOAD_REG_INT)
        return RXOP_COMPONENT_INTEGER;
    if (opcode == OP_FCOPY_REG_REG || opcode == OP_LOAD_REG_FLOAT)
        return RXOP_COMPONENT_FLOAT;
    if (opcode == OP_SCOPY_REG_REG || opcode == OP_LOAD_REG_STRING)
        return RXOP_COMPONENT_STRING;
    if (opcode == OP_DCOPY_REG_REG || opcode == OP_LOAD_REG_DECIMAL)
        return RXOP_COMPONENT_DECIMAL;
    if (opcode == OP_ACOPY_REG_REG) return RXOP_COMPONENT_ATTRIBUTES;
    if (opcode == OP_BCOPY_REG_REG)
        return RXOP_COMPONENT_BINARY | RXOP_COMPONENT_NATIVE_PAYLOAD;
    if (opcode == OP_LOAD_REG_BINARY)
        return RXOP_COMPONENT_BINARY;
    if (opcode >= OP_IADD_REG_REG_REG && opcode <= OP_DEC_REG)
        return RXOP_COMPONENT_INTEGER;
    if (opcode >= OP_IEQ_REG_REG_REG && opcode <= OP_SLTE_REG_STRING_REG)
        return RXOP_COMPONENT_INTEGER;
    if (opcode >= OP_FEQ_REG_REG_REG && opcode <= OP_FLTE_REG_FLOAT_REG)
        return RXOP_COMPONENT_INTEGER;
    if (opcode >= OP_DEQ_REG_REG_REG && opcode <= OP_DLTE_REG_DECIMAL_REG)
        return RXOP_COMPONENT_INTEGER;
    if (opcode == OP_BINEQ_REG_REG_REG || opcode == OP_BINEQ_REG_REG_BINARY ||
        opcode == OP_BINNE_REG_REG_REG || opcode == OP_BINNE_REG_REG_BINARY)
        return RXOP_COMPONENT_INTEGER;
    if (opcode == OP_BTOS_REG || opcode == OP_ITOS_REG ||
        opcode == OP_FTOS_REG || opcode == OP_DTOS_REG)
        return RXOP_COMPONENT_STRING;
    if (opcode == OP_BTOF_REG || opcode == OP_ITOF_REG ||
        opcode == OP_STOF_REG || opcode == OP_DTOF_REG)
        return RXOP_COMPONENT_FLOAT;
    if (opcode == OP_BTOD_REG || opcode == OP_STOD_REG ||
        opcode == OP_ITOD_REG || opcode == OP_FTOD_REG)
        return RXOP_COMPONENT_DECIMAL;
    if (opcode == OP_BTOI_REG || opcode == OP_FTOI_REG ||
        opcode == OP_FTOB_REG || opcode == OP_ITOB_REG ||
        opcode == OP_STOB_REG || opcode == OP_STOI_REG ||
        opcode == OP_DTOI_REG || opcode == OP_DTOB_REG)
        return RXOP_COMPONENT_INTEGER;
    if (opcode == OP_ITOF_REG_REG)
        return RXOP_COMPONENT_INTEGER | RXOP_COMPONENT_FLOAT;
    return RXOP_COMPONENT_NONE;
}

unsigned int rxop_component_clears(int opcode, size_t operand_index) {
    if (operand_index != 0) return RXOP_COMPONENT_NONE;
    /* set_int() and set_float() release a reference payload and finalize any
     * host-owned native payload before assigning the scalar field.  Ordinary
     * binary data is intentionally a separate component and is not cleared. */
    if (opcode == OP_LOAD_REG_INT || opcode == OP_LOAD_REG_FLOAT)
        return RXOP_COMPONENT_REFERENCE |
               RXOP_COMPONENT_NATIVE_PAYLOAD;
    return RXOP_COMPONENT_NONE;
}

RxOpValueDerivation rxop_value_derivation(int opcode) {
    if (opcode == OP_BTOI_REG) return RXOP_DERIVATION_BOOLEAN_TO_INTEGER;
    if (opcode == OP_BTOD_REG) return RXOP_DERIVATION_BOOLEAN_TO_DECIMAL;
    if (opcode == OP_BTOF_REG) return RXOP_DERIVATION_BOOLEAN_TO_FLOAT;
    if (opcode == OP_BTOS_REG) return RXOP_DERIVATION_BOOLEAN_TO_STRING;
    if (opcode == OP_ITOS_REG) return RXOP_DERIVATION_INTEGER_TO_STRING;
    if (opcode == OP_FTOS_REG) return RXOP_DERIVATION_FLOAT_TO_STRING;
    if (opcode == OP_DTOS_REG) return RXOP_DERIVATION_DECIMAL_TO_STRING;
    if (opcode == OP_ITOF_REG || opcode == OP_ITOF_REG_REG)
        return RXOP_DERIVATION_INTEGER_TO_FLOAT;
    if (opcode == OP_FTOI_REG) return RXOP_DERIVATION_FLOAT_TO_INTEGER;
    if (opcode == OP_FTOB_REG) return RXOP_DERIVATION_FLOAT_TO_BOOLEAN;
    if (opcode == OP_ITOB_REG) return RXOP_DERIVATION_INTEGER_TO_BOOLEAN;
    if (opcode == OP_STOB_REG) return RXOP_DERIVATION_STRING_TO_BOOLEAN;
    if (opcode == OP_STOF_REG) return RXOP_DERIVATION_STRING_TO_FLOAT;
    if (opcode == OP_STOI_REG) return RXOP_DERIVATION_STRING_TO_INTEGER;
    if (opcode == OP_STOD_REG) return RXOP_DERIVATION_STRING_TO_DECIMAL;
    if (opcode == OP_DTOI_REG) return RXOP_DERIVATION_DECIMAL_TO_INTEGER;
    if (opcode == OP_DTOB_REG) return RXOP_DERIVATION_DECIMAL_TO_BOOLEAN;
    if (opcode == OP_ITOD_REG)
        return RXOP_DERIVATION_INTEGER_TO_DECIMAL;
    if (opcode == OP_FTOD_REG) return RXOP_DERIVATION_FLOAT_TO_DECIMAL;
    if (opcode == OP_DTOF_REG) return RXOP_DERIVATION_DECIMAL_TO_FLOAT;
    return RXOP_DERIVATION_NONE;
}

size_t rxop_derivation_source_operand(int opcode) {
    if (rxop_value_derivation(opcode) == RXOP_DERIVATION_NONE)
        return SIZE_MAX;
    return opcode == OP_ITOF_REG_REG ? 1 : 0;
}

unsigned int rxop_derivation_source_component(int opcode) {
    switch (rxop_value_derivation(opcode)) {
        case RXOP_DERIVATION_INTEGER_TO_FLOAT:
        case RXOP_DERIVATION_INTEGER_TO_STRING:
        case RXOP_DERIVATION_INTEGER_TO_DECIMAL:
        case RXOP_DERIVATION_INTEGER_TO_BOOLEAN:
        case RXOP_DERIVATION_BOOLEAN_TO_INTEGER:
        case RXOP_DERIVATION_BOOLEAN_TO_DECIMAL:
        case RXOP_DERIVATION_BOOLEAN_TO_FLOAT:
        case RXOP_DERIVATION_BOOLEAN_TO_STRING:
            return RXOP_COMPONENT_INTEGER;
        case RXOP_DERIVATION_FLOAT_TO_STRING:
        case RXOP_DERIVATION_FLOAT_TO_INTEGER:
        case RXOP_DERIVATION_FLOAT_TO_BOOLEAN:
        case RXOP_DERIVATION_FLOAT_TO_DECIMAL:
            return RXOP_COMPONENT_FLOAT;
        case RXOP_DERIVATION_DECIMAL_TO_STRING:
        case RXOP_DERIVATION_DECIMAL_TO_INTEGER:
        case RXOP_DERIVATION_DECIMAL_TO_BOOLEAN:
        case RXOP_DERIVATION_DECIMAL_TO_FLOAT:
            return RXOP_COMPONENT_DECIMAL;
        case RXOP_DERIVATION_STRING_TO_BOOLEAN:
        case RXOP_DERIVATION_STRING_TO_FLOAT:
        case RXOP_DERIVATION_STRING_TO_INTEGER:
        case RXOP_DERIVATION_STRING_TO_DECIMAL:
            return RXOP_COMPONENT_STRING;
        case RXOP_DERIVATION_NONE:
            break;
    }
    return RXOP_COMPONENT_NONE;
}

unsigned int rxop_derivation_context_reads(int opcode) {
    RxOpSignalContract signal;
    if (rxop_value_derivation(opcode) == RXOP_DERIVATION_NONE)
        return RXOP_CONTEXT_NONE;
    signal = rxop_signal_contract(opcode);
    return signal.dependencies & RXOP_SIGNAL_DEP_NUMERIC_CONTEXT
            ? RXOP_CONTEXT_NUMERIC : RXOP_CONTEXT_NONE;
}

unsigned int rxop_context_writes(int opcode) {
    if (opcode == OP_SETNUMDGTS_REG || opcode == OP_SETNUMDGTS_INT ||
        opcode == OP_SETNUMFUZ_REG || opcode == OP_SETNUMFUZ_INT ||
        opcode == OP_SETNUMFRM_REG || opcode == OP_SETNUMFRM_INT ||
        opcode == OP_SETNUMCAS_REG || opcode == OP_SETNUMCAS_INT ||
        opcode == OP_SETNUMSTD_REG || opcode == OP_SETNUMSTD_INT ||
        opcode == OP_NUMSCI_INT_INT_INT || opcode == OP_NUMENG_INT_INT_INT)
        return RXOP_CONTEXT_NUMERIC;
    return RXOP_CONTEXT_NONE;
}

RxOpSignalContract rxop_signal_contract(int opcode) {
    RxOpSignalContract contract;

    contract.opcode = opcode;
    contract.state = RXOP_SIGNAL_STATE_UNKNOWN;
    contract.phase = RXOP_SIGNAL_PHASE_UNKNOWN;
    contract.source = RXOP_SIGNAL_SOURCE_UNKNOWN;
    contract.source_operand = SIZE_MAX;
    contract.static_names = NULL;
    contract.failure_writes = RXOP_OP_ALL;
    contract.failure_writes_signature = NULL;
    contract.failure_component_writes = RXOP_COMPONENT_ALL;
    contract.failure_context_writes = RXOP_CONTEXT_NUMERIC;
    contract.dependencies = RXOP_SIGNAL_DEP_UNKNOWN;
    contract.continuations = RXOP_SIGNAL_CONT_ALL;
    contract.properties = RXOP_SIGNAL_PROP_ADDRESS_OBSERVABLE |
                          RXOP_SIGNAL_PROP_PAYLOAD_OBSERVABLE;
    contract.policy_effect = RXOP_POLICY_EFFECT_UNKNOWN;
    contract.policy_source = RXOP_SIGNAL_SOURCE_UNKNOWN;
    contract.policy_source_operand = SIZE_MAX;
    contract.policy_static_name = NULL;

    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS) return contract;
    if ((size_t)opcode >= rxop_signal_contract_count()) return contract;
    if (rxop_signal_contracts[opcode].opcode != opcode) return contract;
    return rxop_signal_contracts[opcode];
}

size_t rxop_signal_contract_count(void) {
    return sizeof(rxop_signal_contracts) / sizeof(rxop_signal_contracts[0]);
}

int rxop_can_signal(int opcode) {
    return rxop_signal_contract(opcode).state != RXOP_SIGNAL_STATE_NONE;
}

int rxop_signal_failure_writes_operand(const RxOpSignalContract *contract,
                                       size_t operand_index) {
    if (!contract) return 1;
    return rxop_effect_has_operand(contract->failure_writes,
                                   contract->failure_writes_signature,
                                   operand_index);
}

RxOpSignalPhase rxop_signal_phase(int opcode) {
    return rxop_signal_contract(opcode).phase;
}
