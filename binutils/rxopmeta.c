/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxdefs.h"
#include "rxflags.h"

#include <string.h>

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
    RxOpConstEvaluator const_evaluator;
} RxOpEffectSpec;

#define RXE(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS) \
    STATE, READS, WRITES, KILLS, BRANCH_TARGETS, NULL, NULL, NULL, NULL, IMPLICIT, SEMANTICS, \
    RXOP_CONST_EVAL_NONE
#define RXEV(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS) \
    STATE, RXOP_OP_NONE, RXOP_OP_NONE, RXOP_OP_NONE, RXOP_OP_NONE, \
    READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS, \
    RXOP_CONST_EVAL_NONE
#define RXEVC(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS, \
              CONST_EVALUATOR) \
    STATE, RXOP_OP_NONE, RXOP_OP_NONE, RXOP_OP_NONE, RXOP_OP_NONE, \
    READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS, \
    CONST_EVALUATOR
#define RXEC(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS, \
             CONST_EVALUATOR) \
    STATE, READS, WRITES, KILLS, BRANCH_TARGETS, NULL, NULL, NULL, NULL, IMPLICIT, SEMANTICS, \
    CONST_EVALUATOR
#define RXOP_EFFECT(NAME, EFFECTS) { OP_##NAME, EFFECTS },
static const RxOpEffectSpec rxop_effect_specs[] = {
#include "rxopeffects.h"
};
#undef RXOP_EFFECT
#undef RXEC
#undef RXEVC
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
#define RXSC_DYNAMIC_CHECKED(PHASE, SOURCE, SOURCE_OPERAND, NAMES, DEPENDENCIES, PROPERTIES) \
    RXOP_SIGNAL_STATE_KNOWN, PHASE, SOURCE, SOURCE_OPERAND, NAMES, \
    RXOP_OP_NONE, NULL, RXOP_COMPONENT_NONE, RXOP_CONTEXT_NONE, DEPENDENCIES, \
    RXOP_SIGNAL_CONT_ALL, RXOP_SIGNAL_PROP_ADDRESS_OBSERVABLE | \
        RXOP_SIGNAL_PROP_PAYLOAD_OBSERVABLE | PROPERTIES, \
    RXOP_POLICY_EFFECT_NONE, RXOP_SIGNAL_SOURCE_NONE, SIZE_MAX, NULL
#define RXSC_DYNAMIC(PHASE, SOURCE, SOURCE_OPERAND, DEPENDENCIES, PROPERTIES) \
    RXSC_DYNAMIC_CHECKED(PHASE, SOURCE, SOURCE_OPERAND, NULL, DEPENDENCIES, PROPERTIES)
#define RXSC_PLUGIN(PHASE, FAILURE_WRITES, FAILURE_COMPONENTS, DEPENDENCIES, \
                    PROPERTIES) \
    RXOP_SIGNAL_STATE_KNOWN, PHASE, RXOP_SIGNAL_SOURCE_PLUGIN, SIZE_MAX, NULL, \
    FAILURE_WRITES, NULL, FAILURE_COMPONENTS, RXOP_CONTEXT_NONE, DEPENDENCIES, \
    RXOP_SIGNAL_CONT_ALL, RXOP_SIGNAL_PROP_ADDRESS_OBSERVABLE | \
        RXOP_SIGNAL_PROP_PAYLOAD_OBSERVABLE | PROPERTIES, \
    RXOP_POLICY_EFFECT_NONE, RXOP_SIGNAL_SOURCE_NONE, SIZE_MAX, NULL
#define RXSC_CALL(FAILURE_WRITES, FAILURE_SIGNATURE, FAILURE_COMPONENTS) \
    RXOP_SIGNAL_STATE_KNOWN, RXOP_SIGNAL_PHASE_PARTIAL_WRITES, \
    RXOP_SIGNAL_SOURCE_PROPAGATED_CALL, SIZE_MAX, NULL, FAILURE_WRITES, \
    FAILURE_SIGNATURE, FAILURE_COMPONENTS, RXOP_CONTEXT_NONE, \
    RXOP_SIGNAL_DEP_UNKNOWN, RXOP_SIGNAL_CONT_ALL, \
    RXOP_SIGNAL_PROP_ADDRESS_OBSERVABLE | \
        RXOP_SIGNAL_PROP_PAYLOAD_OBSERVABLE, \
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
#undef RXSC_CALL
#undef RXSC_DECIMAL_COMPARE
#undef RXSC_DYNAMIC
#undef RXSC_DYNAMIC_CHECKED
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

static int rxop_normal_target_count(int opcode) {
    RxOpEffects effects = rxop_effects(opcode);
    size_t i, operands;
    int targets = 0;
    if (opcode >= 0 && opcode < OP_MAX_INSTRUCTIONS &&
        effects.state == RXOP_EFFECT_CONSERVATIVE && effects.flow == FLOW_NEXT)
        return 0;
    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS ||
        (effects.state != RXOP_EFFECT_CLASSIFIED &&
         (effects.state != RXOP_EFFECT_CONSERVATIVE || !effects.branch_targets_signature)) ||
        (effects.semantics & RXOP_SEM_INDIRECT_BRANCH)) return -1;
    operands = rxop_format_operand_count(op_table[opcode].format);
    for (i = 0; i < operands; ++i) {
        if (!rxop_effect_branch_target_operand(&effects, i)) continue;
        if (rxop_format_operand_type(op_table[opcode].format, i) != OP_ID)
            return -1;
        ++targets;
    }
    return targets;
}

int rxop_normal_successors(int opcode) {
    RxOpEffects effects = rxop_effects(opcode);
    int targets = rxop_normal_target_count(opcode);
    if (targets < 0) return -2;
    switch (effects.flow) {
        case FLOW_NEXT: return -1;
        case FLOW_TERM: return targets == 0 ? 0 : -2;
        case FLOW_JUMP: return targets == 1 ? 1 : -2;
        case FLOW_COND: return targets == 1 || targets == 2 ? 2 : -2;
    }
    return -2;
}

int rxop_normal_fallthrough(int opcode) {
    RxOpEffects effects = rxop_effects(opcode);
    return effects.flow == FLOW_COND && rxop_normal_target_count(opcode) == 1;
}

RxOpBindingAction rxop_binding_action(int opcode) {
    RxOpEffects effects = rxop_effects(opcode);
    switch (opcode) {
        case OP_LOAD_INT_INT:
        case OP_LOAD_INT_REG: return RXOP_BINDING_COMPOUND;
        case OP_LOAD_REG_REG:
        case OP_LINK_REG_REG: return RXOP_BINDING_LINK;
        case OP_UNLINK_REG: return RXOP_BINDING_UNLINK;
        case OP_SWAP_REG_REG: return RXOP_BINDING_SWAP;
        case OP_SWAPN_REG_REG_REG_REG:
        case OP_SWAPN_REG_REG_REG_REG_REG_REG:
        case OP_SWAPN_REG_REG_REG_REG_REG_REG_REG_REG:
            return RXOP_BINDING_COMPOUND;
        case OP_LINKARG_REG_INT:
        case OP_LINKARG_REG_REG_INT:
            return RXOP_BINDING_ARGUMENT;
        case OP_ENDLIFE_REG: return RXOP_BINDING_LIFETIME;
        case OP_SETTPSWAP_REG_INT_REG:
        case OP_LOADSETTPSWAP_REG_INT_REG_INT_REG:
        case OP_SWAPSETTP_REG_REG_REG_INT:
        case OP_SWAPSETTPSWAP_REG_REG_REG_INT_REG:
        case OP_SETTPSWAPSETTPSWAP_REG_INT_REG_REG_REG:
        case OP_SWAPCALL_REG_FUNC_REG_REG_REG:
        case OP_SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG:
            return RXOP_BINDING_COMPOUND;
        default: break;
    }
    if (effects.state == RXOP_EFFECT_CLASSIFIED &&
        (effects.semantics & (RXOP_SEM_ALIAS_CREATE | RXOP_SEM_ALIAS_RELEASE)))
        return RXOP_BINDING_COMPOUND;
    return RXOP_BINDING_NONE;
}

unsigned int rxop_activation_effects(int opcode) {
    RxOpEffects effects = rxop_effects(opcode);
    RxOpSignalContract signal = rxop_signal_contract(opcode);
    unsigned int result = RXOP_ACTIVATION_NONE;
    if (effects.state != RXOP_EFFECT_CLASSIFIED) return RXOP_ACTIVATION_UNKNOWN;
    if (rxop_binding_action(opcode) != RXOP_BINDING_NONE)
        result |= RXOP_ACTIVATION_BINDING;
    if (effects.implicit != RXOP_IMPLICIT_NONE)
        result |= RXOP_ACTIVATION_ARGUMENTS;
    if (effects.semantics & RXOP_SEM_CALL) result |= RXOP_ACTIVATION_CALL;
    if (effects.semantics & RXOP_SEM_RETURN) result |= RXOP_ACTIVATION_RETURN;
    if (effects.semantics & (RXOP_SEM_REFERENCE_CREATE | RXOP_SEM_REFERENCE_READ |
                            RXOP_SEM_REFERENCE_WRITE | RXOP_SEM_REFERENCE_RELEASE))
        result |= RXOP_ACTIVATION_REFERENCE;
    if (effects.semantics & RXOP_SEM_LIFETIME_END)
        result |= RXOP_ACTIVATION_LIFETIME;
    if (rxop_context_writes(opcode)) result |= RXOP_ACTIVATION_NUMERIC;
    if (signal.policy_effect != RXOP_POLICY_EFFECT_NONE &&
        signal.policy_effect != RXOP_POLICY_EFFECT_UNKNOWN)
        result |= RXOP_ACTIVATION_SIGNAL;
    /* These are observations of the removed activation or its executable
     * image, rather than ordinary opaque external effects such as I/O/time.
     * Preserve the distinction even where optimizer effects remain opaque. */
    switch (opcode) {
        case OP_GETNUMDGTS_REG:
        case OP_GETNUMFUZ_REG:
        case OP_GETNUMFRM_REG:
        case OP_GETNUMCAS_REG:
        case OP_GETNUMSTD_REG:
            result |= RXOP_ACTIVATION_NUMERIC; break;
        case OP_METALINKPREG_REG_REG:
        case OP_METALOADCALLERADDR_REG:
            result |= RXOP_ACTIVATION_FRAME; break;
        case OP_METALOADMODULE_REG_REG:
            result |= RXOP_ACTIVATION_MODULE; break;
        case OP_METALOADINST_REG_REG_REG:
        case OP_METADECODEINST_REG_REG:
        case OP_METALOADIOPERAND_REG_REG_REG:
        case OP_METALOADFOPERAND_REG_REG_REG:
        case OP_METALOADSOPERAND_REG_REG_REG:
        case OP_METALOADPOPERAND_REG_REG_REG:
        case OP_METALOADDATA_REG_REG_REG:
            result |= RXOP_ACTIVATION_CODE; break;
        case OP_BPON_FUNC:
        case OP_BPON:
        case OP_BPOFF:
            result |= RXOP_ACTIVATION_DEBUG; break;
        default: break;
    }
    return result;
}

RxOpCloneCategory rxop_clone_category(int opcode) {
    RxOpEffects effects = rxop_effects(opcode);
    unsigned int activation = rxop_activation_effects(opcode);
    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS ||
        !rxop_is_source_mnemonic(op_table[opcode].mnemonic) ||
        effects.state != RXOP_EFFECT_CLASSIFIED)
        return RXOP_CLONE_MISSING;
    if (activation & (RXOP_ACTIVATION_UNKNOWN | RXOP_ACTIVATION_FRAME |
                      RXOP_ACTIVATION_MODULE | RXOP_ACTIVATION_CODE |
                      RXOP_ACTIVATION_DEBUG)) return RXOP_CLONE_RETAINED;
    if (activation & RXOP_ACTIVATION_CALL) return RXOP_CLONE_CALL;
    if (activation & RXOP_ACTIVATION_RETURN) return RXOP_CLONE_RETURN;
    if (activation & RXOP_ACTIVATION_BINDING)
        return rxop_binding_action(opcode) == RXOP_BINDING_COMPOUND
                ? RXOP_CLONE_RETAINED : RXOP_CLONE_BINDING;
    if (activation & (RXOP_ACTIVATION_NUMERIC | RXOP_ACTIVATION_SIGNAL))
        return RXOP_CLONE_CONTEXT;
    if (activation != RXOP_ACTIVATION_NONE || rxop_normal_successors(opcode) == -2 ||
        (effects.semantics & (RXOP_SEM_REFERENCE_CREATE | RXOP_SEM_REFERENCE_READ |
                             RXOP_SEM_REFERENCE_WRITE | RXOP_SEM_REFERENCE_RELEASE |
                             RXOP_SEM_LIFETIME_END | RXOP_SEM_INDIRECT_WRITE |
                             RXOP_SEM_INDIRECT_BRANCH))) return RXOP_CLONE_RETAINED;
    return RXOP_CLONE_ORDINARY;
}

int rxop_component_write_is_known(int opcode, size_t operand_index) {
    RxOpEffects effects = rxop_effects(opcode);
    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS ||
        effects.state != RXOP_EFFECT_CLASSIFIED ||
        operand_index >= rxop_format_operand_count(op_table[opcode].format))
        return 0;
    if (!rxop_effect_writes_operand(&effects, operand_index)) return 1;
    /* The component helpers intentionally have an unknown-write default of
     * NONE. Do not confuse that absent projection with a proved no-op. */
    return rxop_component_writes(opcode, operand_index) != RXOP_COMPONENT_NONE;
}

unsigned int rxop_component_reads(int opcode, size_t operand_index) {
    if ((opcode == OP_SAY_REG || opcode == OP_SAYX_REG) && operand_index == 0)
        return RXOP_COMPONENT_STRING;
    if ((opcode == OP_SETOBJTYPE_REG_STRING ||
         opcode == OP_ASSERTTYPE_REG_STRING ||
         opcode == OP_ASSERTINITIALIZED_REG) && operand_index == 0)
        return RXOP_COMPONENT_ATTRIBUTES;
    if (opcode >= OP_ICHKRNG_REG_INT_INT &&
        opcode <= OP_ICHKRNG_INT_REG_REG)
        return RXOP_COMPONENT_INTEGER;
    if (opcode == OP_BCHECKRANGE_REG_REG_REG)
        return operand_index == 0 ? RXOP_COMPONENT_BINARY
                                  : RXOP_COMPONENT_INTEGER;
    if (opcode == OP_PGETI_REG_REG_REG || opcode == OP_PGETF_REG_REG_REG)
        return operand_index == 1 ? RXOP_COMPONENT_BINARY
                                  : RXOP_COMPONENT_INTEGER;
    if (opcode == OP_PSETI_REG_REG_REG)
        return operand_index == 0 ? RXOP_COMPONENT_BINARY
                                  : RXOP_COMPONENT_INTEGER;
    if (opcode == OP_PSETF_REG_REG_REG) {
        if (operand_index == 0) return RXOP_COMPONENT_BINARY;
        if (operand_index == 1) return RXOP_COMPONENT_INTEGER;
        return RXOP_COMPONENT_FLOAT;
    }
    if (opcode == OP_CHANOPEN_REG_REG_REG_REG_REG) {
        if (operand_index == 4) return RXOP_COMPONENT_BINARY;
        if (operand_index >= 2) return RXOP_COMPONENT_INTEGER;
        return RXOP_COMPONENT_ALL;
    }
    if (opcode == OP_CHANSTART_REG_REG_REG_REG_REG) {
        if (operand_index == 3) return RXOP_COMPONENT_BINARY;
        if (operand_index >= 2) return RXOP_COMPONENT_INTEGER;
        return RXOP_COMPONENT_ALL;
    }
    if (opcode == OP_CHANWAIT_REG_REG_REG_REG) {
        if (operand_index >= 2) return RXOP_COMPONENT_INTEGER;
        return RXOP_COMPONENT_ALL;
    }
    if (opcode == OP_CHANCANCEL_REG_REG_REG_REG) {
        if (operand_index == 3) return RXOP_COMPONENT_BINARY;
        if (operand_index >= 1) return RXOP_COMPONENT_INTEGER;
        return RXOP_COMPONENT_ALL;
    }
    if (opcode == OP_CHANCLOSE_REG_REG_REG ||
        opcode == OP_CHANRELEASE_REG_REG_REG) {
        if (operand_index >= 1) return RXOP_COMPONENT_INTEGER;
        return RXOP_COMPONENT_ALL;
    }
    if (opcode == OP_COPY_REG_REG && operand_index == 1) return RXOP_COMPONENT_ALL;
    if (opcode == OP_ICOPY_REG_REG && operand_index == 1) return RXOP_COMPONENT_INTEGER;
    if (opcode == OP_FCOPY_REG_REG && operand_index == 1) return RXOP_COMPONENT_FLOAT;
    if (opcode == OP_SCOPY_REG_REG && operand_index == 1) return RXOP_COMPONENT_STRING;
    if (opcode == OP_DCOPY_REG_REG && operand_index == 1) return RXOP_COMPONENT_DECIMAL;
    if (opcode == OP_ACOPY_REG_REG && operand_index == 1) return RXOP_COMPONENT_ATTRIBUTES;
    if (opcode == OP_BCOPY_REG_REG && operand_index == 1)
        return RXOP_COMPONENT_BINARY | RXOP_COMPONENT_NATIVE_PAYLOAD;
    if (opcode == OP_IGETUNLINK_REG_REG && operand_index == 1)
        return RXOP_COMPONENT_INTEGER;

    if (opcode == OP_GETATTRS_REG_REG ||
        opcode == OP_GETATTRS_REG_REG_INT)
        return operand_index == 1 ? RXOP_COMPONENT_ATTRIBUTE_COUNT
                                  : RXOP_COMPONENT_ALL;
    if (opcode == OP_SETATTRS_REG_REG ||
        opcode == OP_SETATTRS_REG_REG_INT)
        return operand_index == 0 ? RXOP_COMPONENT_ATTRIBUTE_COUNT
                                  : RXOP_COMPONENT_INTEGER;
    if (opcode == OP_SETATTRS_REG_INT ||
        opcode == OP_SETATTRS_REG_INT_INT)
        return RXOP_COMPONENT_ATTRIBUTE_COUNT;
    if (opcode == OP_MINATTRS_REG_REG ||
        opcode == OP_MINATTRS_REG_REG_INT)
        return operand_index == 0 ? RXOP_COMPONENT_ATTRIBUTE_COUNT
                                  : RXOP_COMPONENT_INTEGER;
    if (opcode == OP_MINATTRS_REG_INT ||
        opcode == OP_MINATTRS_REG_INT_INT)
        return RXOP_COMPONENT_ATTRIBUTE_COUNT;
    if (opcode == OP_LINKATTR_REG_REG_REG ||
        opcode == OP_LINKATTR1_REG_REG_REG)
        return operand_index == 1 ? RXOP_COMPONENT_ATTRIBUTE_COUNT
                                  : RXOP_COMPONENT_INTEGER;
    if (opcode == OP_LINKATTR_REG_REG_INT ||
        opcode == OP_LINKATTR1_REG_REG_INT)
        return RXOP_COMPONENT_ATTRIBUTE_COUNT;

    if (opcode == OP_STRLEN_REG_REG && operand_index == 1)
        return RXOP_COMPONENT_STRING;
    if (opcode == OP_STRPOS_REG_REG_REG)
        return operand_index == 0 ? RXOP_COMPONENT_INTEGER
                                  : RXOP_COMPONENT_STRING;
    if (opcode == OP_SUBSTRING_REG_REG_REG_REG)
        return operand_index == 1 ? RXOP_COMPONENT_STRING
                                  : RXOP_COMPONENT_INTEGER;
    if (opcode == OP_BSLICE_REG_REG_REG_REG)
        return operand_index == 1 ? RXOP_COMPONENT_BINARY
                                  : RXOP_COMPONENT_INTEGER;
    if (opcode >= OP_CONCAT_REG_REG_REG &&
        opcode <= OP_SCONCAT_REG_STRING_REG)
        return RXOP_COMPONENT_STRING;

    if (opcode >= OP_DADD_REG_REG_REG &&
        opcode <= OP_DMOD_REG_DECIMAL_REG)
        return RXOP_COMPONENT_DECIMAL;
    if (opcode >= OP_DPOW_REG_REG_REG &&
        opcode <= OP_DSEX_REG)
        return RXOP_COMPONENT_DECIMAL;

    /* Native stem keys and stored values are strings.  The receiver remains
     * a whole native value; GET destinations are success-path writes and are
     * classified separately by rxop_component_writes(). */
    if (opcode == OP_STEMGET_REG_REG_REG)
        return operand_index == 2 ? RXOP_COMPONENT_STRING
                                  : RXOP_COMPONENT_ALL;
    if (opcode == OP_STEMSET_REG_REG_REG)
        return operand_index >= 1 ? RXOP_COMPONENT_STRING
                                  : RXOP_COMPONENT_ALL;
    if (opcode == OP_STEMRESET_REG_REG)
        return operand_index == 1 ? RXOP_COMPONENT_STRING
                                  : RXOP_COMPONENT_ALL;
    if (opcode == OP_STEMGET2_REG_REG_REG_REG)
        return operand_index >= 2 ? RXOP_COMPONENT_STRING
                                  : RXOP_COMPONENT_ALL;
    if (opcode == OP_STEMSET2_REG_REG_REG_REG)
        return operand_index >= 1 ? RXOP_COMPONENT_STRING
                                  : RXOP_COMPONENT_ALL;

    if ((opcode >= OP_IADD_REG_REG_REG && opcode <= OP_DEC_REG) ||
        (opcode >= OP_IPOW_REG_REG_REG && opcode <= OP_ISEX_REG))
        return RXOP_COMPONENT_INTEGER;
    /* The bitwise and logical integer family has one typed result and only
     * integer inputs. The effects inventory decides which operand is read;
     * this projection must not turn those reads into whole-value unknowns. */
    if (opcode >= OP_IAND_REG_REG_REG && opcode <= OP_NOT_REG_REG)
        return RXOP_COMPONENT_INTEGER;
    if (opcode >= OP_IEQ_REG_REG_REG && opcode <= OP_ILTE_REG_INT_REG)
        return operand_index == 0 ? RXOP_COMPONENT_ALL : RXOP_COMPONENT_INTEGER;
    if (opcode == OP_BRT_ID_REG || opcode == OP_BRF_ID_REG ||
        opcode == OP_BRTF_ID_ID_REG || opcode == OP_BEQ_ID_REG_REG ||
        opcode == OP_BEQ_ID_REG_INT || opcode == OP_BNE_ID_REG_REG ||
        opcode == OP_BNE_ID_REG_INT || opcode == OP_BGT_ID_REG_REG ||
        opcode == OP_BGT_ID_REG_INT || opcode == OP_BGE_ID_REG_REG ||
        opcode == OP_BGE_ID_REG_INT || opcode == OP_BLT_ID_REG_REG ||
        opcode == OP_BLT_ID_REG_INT || opcode == OP_BLE_ID_REG_REG ||
        opcode == OP_BLE_ID_REG_INT)
        return RXOP_COMPONENT_INTEGER;
    if ((opcode == OP_SEQ_REG_REG_REG || opcode == OP_SEQ_REG_REG_STRING ||
         (opcode >= OP_SNE_REG_REG_REG && opcode <= OP_SLTE_REG_STRING_REG)) &&
        operand_index != 0)
        return RXOP_COMPONENT_STRING;
    if (opcode >= OP_REQ_REG_REG_REG &&
        opcode <= OP_RLTE_REG_STRING_REG && operand_index != 0)
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

typedef enum RxOpIntegerRelation {
    RXOP_INT_RELATION_NONE = 0,
    RXOP_INT_RELATION_EQ,
    RXOP_INT_RELATION_NE,
    RXOP_INT_RELATION_GT,
    RXOP_INT_RELATION_GE,
    RXOP_INT_RELATION_LT,
    RXOP_INT_RELATION_LE
} RxOpIntegerRelation;

static RxOpIntegerRelation rxop_integer_relation_complement(
        RxOpIntegerRelation relation) {
    switch (relation) {
        case RXOP_INT_RELATION_EQ: return RXOP_INT_RELATION_NE;
        case RXOP_INT_RELATION_NE: return RXOP_INT_RELATION_EQ;
        case RXOP_INT_RELATION_GT: return RXOP_INT_RELATION_LE;
        case RXOP_INT_RELATION_GE: return RXOP_INT_RELATION_LT;
        case RXOP_INT_RELATION_LT: return RXOP_INT_RELATION_GE;
        case RXOP_INT_RELATION_LE: return RXOP_INT_RELATION_GT;
        default: return RXOP_INT_RELATION_NONE;
    }
}

static RxOpIntegerRelation rxop_integer_relation_reverse(
        RxOpIntegerRelation relation) {
    switch (relation) {
        case RXOP_INT_RELATION_EQ:
        case RXOP_INT_RELATION_NE:
            return relation;
        case RXOP_INT_RELATION_GT: return RXOP_INT_RELATION_LT;
        case RXOP_INT_RELATION_GE: return RXOP_INT_RELATION_LE;
        case RXOP_INT_RELATION_LT: return RXOP_INT_RELATION_GT;
        case RXOP_INT_RELATION_LE: return RXOP_INT_RELATION_GE;
        default: return RXOP_INT_RELATION_NONE;
    }
}

static int rxop_integer_fused_branch(RxOpIntegerRelation relation,
                                     int immediate) {
    switch (relation) {
        case RXOP_INT_RELATION_EQ:
            return immediate ? OP_BEQ_ID_REG_INT : OP_BEQ_ID_REG_REG;
        case RXOP_INT_RELATION_NE:
            return immediate ? OP_BNE_ID_REG_INT : OP_BNE_ID_REG_REG;
        case RXOP_INT_RELATION_GT:
            return immediate ? OP_BGT_ID_REG_INT : OP_BGT_ID_REG_REG;
        case RXOP_INT_RELATION_GE:
            return immediate ? OP_BGE_ID_REG_INT : OP_BGE_ID_REG_REG;
        case RXOP_INT_RELATION_LT:
            return immediate ? OP_BLT_ID_REG_INT : OP_BLT_ID_REG_REG;
        case RXOP_INT_RELATION_LE:
            return immediate ? OP_BLE_ID_REG_INT : OP_BLE_ID_REG_REG;
        default:
            return -1;
    }
}

int rxop_compare_branch_fusion(int compare_opcode, int branch_opcode,
                               RxOpCompareBranchFusion *fusion) {
    RxOpCompareBranchFusion local_fusion;
    RxOpIntegerRelation relation;
    int immediate;
    int reverse;
    int fused_opcode;
    if (!fusion) fusion = &local_fusion;
    memset(fusion, 0, sizeof(*fusion));
    relation = RXOP_INT_RELATION_NONE;
    immediate = 0;
    reverse = 0;
    switch (compare_opcode) {
        case OP_IEQ_REG_REG_REG: relation = RXOP_INT_RELATION_EQ; break;
        case OP_IEQ_REG_REG_INT:
            relation = RXOP_INT_RELATION_EQ; immediate = 1; break;
        case OP_INE_REG_REG_REG: relation = RXOP_INT_RELATION_NE; break;
        case OP_INE_REG_REG_INT:
            relation = RXOP_INT_RELATION_NE; immediate = 1; break;
        case OP_IGT_REG_REG_REG: relation = RXOP_INT_RELATION_GT; break;
        case OP_IGT_REG_REG_INT:
            relation = RXOP_INT_RELATION_GT; immediate = 1; break;
        case OP_IGT_REG_INT_REG:
            relation = RXOP_INT_RELATION_GT; immediate = 1; reverse = 1; break;
        case OP_IGTE_REG_REG_REG: relation = RXOP_INT_RELATION_GE; break;
        case OP_IGTE_REG_REG_INT:
            relation = RXOP_INT_RELATION_GE; immediate = 1; break;
        case OP_IGTE_REG_INT_REG:
            relation = RXOP_INT_RELATION_GE; immediate = 1; reverse = 1; break;
        case OP_ILT_REG_REG_REG: relation = RXOP_INT_RELATION_LT; break;
        case OP_ILT_REG_REG_INT:
            relation = RXOP_INT_RELATION_LT; immediate = 1; break;
        case OP_ILT_REG_INT_REG:
            relation = RXOP_INT_RELATION_LT; immediate = 1; reverse = 1; break;
        case OP_ILTE_REG_REG_REG: relation = RXOP_INT_RELATION_LE; break;
        case OP_ILTE_REG_REG_INT:
            relation = RXOP_INT_RELATION_LE; immediate = 1; break;
        case OP_ILTE_REG_INT_REG:
            relation = RXOP_INT_RELATION_LE; immediate = 1; reverse = 1; break;
        default:
            return 0;
    }
    if (branch_opcode == OP_BRF_ID_REG)
        relation = rxop_integer_relation_complement(relation);
    else if (branch_opcode != OP_BRT_ID_REG)
        return 0;
    if (reverse) relation = rxop_integer_relation_reverse(relation);
    fused_opcode = rxop_integer_fused_branch(relation, immediate);
    if (fused_opcode < 0) return 0;
    fusion->compare_opcode = compare_opcode;
    fusion->branch_opcode = branch_opcode;
    fusion->fused_opcode = fused_opcode;
    fusion->left_source_operand = reverse ? 2 : 1;
    fusion->right_source_operand = reverse ? 1 : 2;
    return 1;
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
    if (opcode == OP_CHANOPEN_REG_REG_REG_REG_REG ||
        opcode == OP_CHANSTART_REG_REG_REG_REG_REG)
        return operand_index < 2 ? RXOP_COMPONENT_INTEGER
                                 : RXOP_COMPONENT_NONE;
    if (opcode == OP_CHANWAIT_REG_REG_REG_REG) {
        if (operand_index == 0) return RXOP_COMPONENT_INTEGER;
        if (operand_index == 1) return RXOP_COMPONENT_BINARY;
        return RXOP_COMPONENT_NONE;
    }
    if (opcode == OP_CHANCANCEL_REG_REG_REG_REG ||
        opcode == OP_CHANCLOSE_REG_REG_REG ||
        opcode == OP_CHANRELEASE_REG_REG_REG)
        return operand_index == 0 ? RXOP_COMPONENT_INTEGER
                                  : RXOP_COMPONENT_NONE;
    if (opcode == OP_PGETI_REG_REG_REG)
        return operand_index == 0 ? RXOP_COMPONENT_INTEGER
                                  : RXOP_COMPONENT_NONE;
    if (opcode == OP_PGETF_REG_REG_REG)
        return operand_index == 0 ? RXOP_COMPONENT_FLOAT
                                  : RXOP_COMPONENT_NONE;
    if (opcode == OP_PSETI_REG_REG_REG || opcode == OP_PSETF_REG_REG_REG)
        return operand_index == 0 ? RXOP_COMPONENT_BINARY
                                  : RXOP_COMPONENT_NONE;
    if (opcode == OP_NULLN_REG_REG || opcode == OP_NULLN_REG_REG_REG ||
        opcode == OP_NULLN_REG_REG_REG_REG)
        return RXOP_COMPONENT_ALL;
    if (operand_index != 0) return RXOP_COMPONENT_NONE;
    if (opcode == OP_COPY_REG_REG || opcode == OP_NULL_REG)
        return RXOP_COMPONENT_ALL;
    if (opcode == OP_ICOPY_REG_REG || opcode == OP_LOAD_REG_INT ||
        opcode == OP_STRLEN_REG_REG || opcode == OP_STRPOS_REG_REG_REG ||
        opcode == OP_GETTP_REG_REG || opcode == OP_GETANDTP_REG_REG_INT ||
        opcode == OP_GETNUMDGTS_REG || opcode == OP_GETNUMFUZ_REG ||
        opcode == OP_GETNUMFRM_REG || opcode == OP_GETNUMCAS_REG ||
        opcode == OP_GETNUMSTD_REG)
        return RXOP_COMPONENT_INTEGER;
    if (opcode == OP_FCOPY_REG_REG || opcode == OP_LOAD_REG_FLOAT)
        return RXOP_COMPONENT_FLOAT;
    if (opcode == OP_SCOPY_REG_REG || opcode == OP_LOAD_REG_STRING)
        return RXOP_COMPONENT_STRING;
    if (opcode >= OP_CONCAT_REG_REG_REG &&
        opcode <= OP_SCONCAT_REG_STRING_REG)
        return RXOP_COMPONENT_STRING;
    if (opcode == OP_SUBSTRING_REG_REG_REG_REG)
        return RXOP_COMPONENT_STRING;
    if (opcode == OP_DCOPY_REG_REG || opcode == OP_LOAD_REG_DECIMAL)
        return RXOP_COMPONENT_DECIMAL;
    if (opcode >= OP_DADD_REG_REG_REG &&
        opcode <= OP_DMOD_REG_DECIMAL_REG)
        return RXOP_COMPONENT_DECIMAL;
    if (opcode >= OP_DPOW_REG_REG_REG && opcode <= OP_DSEX_REG)
        return RXOP_COMPONENT_DECIMAL;
    if (opcode == OP_ACOPY_REG_REG) return RXOP_COMPONENT_ATTRIBUTES;
    if (opcode == OP_SETOBJTYPE_REG_STRING)
        return RXOP_COMPONENT_ATTRIBUTES;
    if (opcode == OP_ISETATTR1_REG_INT_REG)
        return RXOP_COMPONENT_INTEGER;
    if (opcode == OP_IGETUNLINK_REG_REG)
        return RXOP_COMPONENT_INTEGER;
    if (opcode == OP_BCOPY_REG_REG)
        return RXOP_COMPONENT_BINARY | RXOP_COMPONENT_NATIVE_PAYLOAD;
    if (opcode == OP_SETATTRS_REG_REG ||
        opcode == OP_SETATTRS_REG_INT ||
        opcode == OP_SETATTRS_REG_REG_INT ||
        opcode == OP_SETATTRS_REG_INT_INT ||
        opcode == OP_MINATTRS_REG_REG ||
        opcode == OP_MINATTRS_REG_INT ||
        opcode == OP_MINATTRS_REG_REG_INT ||
        opcode == OP_MINATTRS_REG_INT_INT)
        return RXOP_COMPONENT_ATTRIBUTE_COUNT;
    if (opcode == OP_LOAD_REG_BINARY)
        return RXOP_COMPONENT_BINARY;
    if (opcode == OP_BSLICE_REG_REG_REG_REG)
        return RXOP_COMPONENT_BINARY;
    if (opcode == OP_STEMGET_REG_REG_REG ||
        opcode == OP_STEMGET2_REG_REG_REG_REG ||
        opcode == OP_STEMKEYAT_REG_REG_REG ||
        opcode == OP_STEMVALUEAT_REG_REG_REG)
        return RXOP_COMPONENT_STRING;
    if (opcode == OP_STEMSIZE_REG_REG)
        return RXOP_COMPONENT_INTEGER;
    if ((opcode >= OP_IADD_REG_REG_REG && opcode <= OP_DEC_REG) ||
        (opcode >= OP_IPOW_REG_REG_REG && opcode <= OP_ISEX_REG))
        return RXOP_COMPONENT_INTEGER;
    if (opcode >= OP_IAND_REG_REG_REG && opcode <= OP_NOT_REG_REG)
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
    if (opcode >= OP_REQ_REG_REG_REG && opcode <= OP_RLTE_REG_STRING_REG)
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
    if (opcode == OP_CHANOPEN_REG_REG_REG_REG_REG ||
        opcode == OP_CHANSTART_REG_REG_REG_REG_REG ||
        opcode == OP_CHANWAIT_REG_REG_REG_REG ||
        opcode == OP_CHANCANCEL_REG_REG_REG_REG ||
        opcode == OP_CHANCLOSE_REG_REG_REG ||
        opcode == OP_CHANRELEASE_REG_REG_REG) {
        if (rxop_component_writes(opcode, operand_index) != RXOP_COMPONENT_NONE)
            return RXOP_COMPONENT_REFERENCE |
                   RXOP_COMPONENT_NATIVE_PAYLOAD;
        return RXOP_COMPONENT_NONE;
    }
    if (operand_index != 0) return RXOP_COMPONENT_NONE;
    /* set_int(), set_float(), and native-stem string extraction release a
     * reference payload and finalize any host-owned native payload before
     * assigning the result component.  Typed copies deliberately do not.
     * Ordinary binary data is a separate component and is not cleared. */
    if (opcode == OP_LOAD_REG_INT || opcode == OP_LOAD_REG_FLOAT ||
        opcode == OP_LOAD_REG_STRING ||
        (opcode >= OP_IADD_REG_REG_REG && opcode <= OP_IMOD_REG_INT_REG) ||
        (opcode >= OP_IEQ_REG_REG_REG &&
         opcode <= OP_SLTE_REG_STRING_REG) ||
        (opcode >= OP_FEQ_REG_REG_REG &&
         opcode <= OP_FLTE_REG_FLOAT_REG) ||
        (opcode >= OP_DEQ_REG_REG_REG &&
         opcode <= OP_DLTE_REG_DECIMAL_REG) ||
        opcode == OP_BINEQ_REG_REG_REG ||
        opcode == OP_BINEQ_REG_REG_BINARY ||
        opcode == OP_BINNE_REG_REG_REG ||
        opcode == OP_BINNE_REG_REG_BINARY ||
        opcode == OP_STEMGET_REG_REG_REG ||
        opcode == OP_STEMGET2_REG_REG_REG_REG ||
        opcode == OP_STEMKEYAT_REG_REG_REG ||
        opcode == OP_STEMVALUEAT_REG_REG_REG ||
        opcode == OP_STEMSIZE_REG_REG ||
        opcode == OP_PGETI_REG_REG_REG ||
        opcode == OP_PGETF_REG_REG_REG ||
        (opcode >= OP_REQ_REG_REG_REG &&
         opcode <= OP_RLTE_REG_STRING_REG))
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

RxOpCallContract rxop_call_contract(int opcode) {
    RxOpCallContract call = { RXOP_CALL_NONE, 0, 0, 0, 0, "", "", 1, 1 };
    switch (opcode) {
        /* Logical activation has typed capture selectors but is not another
         * ordinary CALL site. Scope-aware clients use its recipe/continuation. */
        case OP_INLENTER_REG_FUNC_STRING_ID:
            call.target_operand = 2; call.result_operand = 1; break;
        case OP_CALL_FUNC:
            call.kind = RXOP_CALL_DIRECT; call.target_operand = 1; break;
        case OP_CALL_REG_FUNC:
            call.kind = RXOP_CALL_DIRECT; call.target_operand = 2;
            call.result_operand = 1; call.prefix = "clear:1"; break;
        case OP_CALL_REG_FUNC_REG:
        case OP_SWAPCALL_REG_FUNC_REG_REG_REG:
        case OP_SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG:
        case OP_SETTPCALL_REG_FUNC_REG_REG_INT:
            call.kind = RXOP_CALL_DIRECT; call.target_operand = 2;
            call.result_operand = 1; call.count_operand = 3;
            call.window_operand = 3;
            if (opcode == OP_SWAPCALL_REG_FUNC_REG_REG_REG)
                call.prefix = "swap:4:5";
            if (opcode == OP_SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG)
                call.prefix = "flags:4:5;swap:4:6";
            if (opcode == OP_SETTPCALL_REG_FUNC_REG_REG_INT)
                call.prefix = "flags:4:5";
            break;
        case OP_DCALL_REG_REG_REG:
            call.kind = RXOP_CALL_DYNAMIC; call.target_operand = 2;
            call.result_operand = 1; call.count_operand = 3;
            call.window_operand = 3; break;
        case OP_SRCFPROCSEL_REG_STRING_REG:
            call.kind = RXOP_CALL_FACTORY; call.target_operand = 2;
            call.result_operand = 1; call.count_operand = 3;
            call.window_operand = 3; break;
        case OP_CALL1_REG_FUNC_REG:
        case OP_CALL2_REG_FUNC_REG_REG:
        case OP_CALL3_REG_FUNC_REG_REG_REG:
        case OP_CALL4_REG_FUNC_REG_REG_REG_REG:
            call.kind = RXOP_CALL_DIRECT; call.target_operand = 2;
            call.result_operand = 1;
            if (opcode == OP_CALL1_REG_FUNC_REG) call.actual_operands = "3";
            if (opcode == OP_CALL2_REG_FUNC_REG_REG) call.actual_operands = "3,4";
            if (opcode == OP_CALL3_REG_FUNC_REG_REG_REG) call.actual_operands = "3,4,5";
            if (opcode == OP_CALL4_REG_FUNC_REG_REG_REG_REG) call.actual_operands = "3,4,5,6";
            break;
        default: break;
    }
    return call;
}

const char *rxop_binding_recipe(int opcode) {
    switch (opcode) {
        case OP_LOAD_REG_REG:
        case OP_LINK_REG_REG: return "link:1:2";
        case OP_LOAD_INT_INT: return "linkindex:1:2";
        case OP_LOAD_INT_REG: return "linkindexreg:1:2";
        case OP_UNLINK_REG: return "unlink:1";
        case OP_SWAP_REG_REG: return "swap:1:2";
        case OP_SWAPN_REG_REG_REG_REG: return "swap:1:2;swap:3:4";
        case OP_SWAPN_REG_REG_REG_REG_REG_REG:
            return "swap:1:2;swap:3:4;swap:5:6";
        case OP_SWAPN_REG_REG_REG_REG_REG_REG_REG_REG:
            return "swap:1:2;swap:3:4;swap:5:6;swap:7:8";
        case OP_LINKARG_REG_INT: return "argument:1:2";
        case OP_LINKARG_REG_REG_INT: return "argumentadd:1:2:3";
        case OP_LINKREF_REG_REG: return "reference:1:2";
        case OP_ENDLIFE_REG: return "endlife:1";
        case OP_LINKATTR_REG_REG_REG:
        case OP_LINKATTR_REG_REG_INT: return "attribute:1:2:3";
        case OP_LINKATTR1_REG_REG_REG:
        case OP_LINKATTR1_REG_REG_INT: return "attribute1:1:2:3";
        case OP_LINKTOATTR_REG_REG_REG:
        case OP_LINKTOATTR_INT_REG_REG: return "linktoattribute:1:2:3";
        case OP_LINKTOATTR1_REG_REG_REG:
        case OP_LINKTOATTR1_INT_REG_REG: return "linktoattribute1:1:2:3";
        case OP_UNLINKATTR_REG_REG:
        case OP_UNLINKATTR_INT_REG: return "unlinkattribute:1:2";
        case OP_UNLINKATTR1_REG_REG:
        case OP_UNLINKATTR1_INT_REG: return "unlinkattribute1:1:2";
        case OP_SETTPSWAP_REG_INT_REG: return "flags:1:2;swap:1:3";
        case OP_LOADSETTPSWAP_REG_INT_REG_INT_REG:
            return "setint:1:2;flags:3:4;swap:3:5";
        case OP_SWAPSETTP_REG_REG_REG_INT: return "swap:1:2;flags:3:4";
        case OP_SWAPSETTPSWAP_REG_REG_REG_INT_REG:
            return "swap:1:2;flags:3:4;swap:3:5";
        case OP_SETTPSWAPSETTPSWAP_REG_INT_REG_REG_REG:
            return "flags:1:2;swap:1:3;flags:4:2;swap:4:5";
        case OP_SWAPCALL_REG_FUNC_REG_REG_REG:
        case OP_SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG:
            return rxop_call_contract(opcode).prefix;
        case OP_UNLINKN_REG_REG: return "unlink:1;unlink:2";
        case OP_ISETUNLINK_REG_REG: return "icopy:1:2;unlink:1";
        /* IGETUNLINK captures the source before restoring its binding, then
         * resolves the destination. Aliased operand slots make order material. */
        case OP_IGETUNLINK_REG_REG: return "captureint:2;unlink:2;putint:1";
        case OP_ILOADSETUNLINK_REG_INT: return "setint:1:2;unlink:1";
        case OP_ISETUNLINKN_REG_REG_REG:
            return "icopy:1:2;unlink:1;unlink:3";
        case OP_UNLINKBR_REG_ID: return "unlink:1;branch:2";
        case OP_ILOADSETUNLINKN_REG_INT_REG:
            return "setint:1:2;unlink:1;unlink:3";
        case OP_ILOADSETUNLINKN_REG_REG_INT_REG:
            return "setint:1:3;icopy:2:1;unlink:2;unlink:4";
        case OP_SETLINKATTR1_REG_REG_INT_REG:
            return "setattrs:2:3;attribute1:1:2:4";
        case OP_SETLINKATTR1_REG_REG_INT_REG_INT:
            return "setattrs:2:3;attribute1add:1:2:4:5";
        case OP_MINLINKATTR1_REG_REG_INT:
            return "minattrs:2:3;attribute1:1:2:3";
        case OP_MINLINKATTR1_REG_REG_REG_INT:
            return "captureindex:3:4;minattrscaptured:2;attribute1captured:1:2";
        case OP_LINKSETATTRSLINKADD_REG_REG_INT_INT_REG_REG_INT:
            return "attribute1:1:2:3;setattrs:1:4;attribute1add:5:1:6:7";
        case OP_SETLINKILOAD_REG_REG_INT_REG_REG_INT:
            return "setattrs:2:3;attribute1:1:2:4;setint:5:6";
        case OP_METALINKPREG_REG_REG: return "parent:1:2";
        case OP_LINKARGV_REG_REG_INT: return "capturedargument:1:2:3";
        case OP_LINKARGV_REG_INT: return "capturedargument:1:2";
        case OP_QGLOBBIND_REG_STRING_INT: return "qualifiedglobal:1:2:3";
        default: break;
    }
    return rxop_binding_action(opcode) == RXOP_BINDING_NONE ? "" : NULL;
}

const char *rxop_implicit_recipe(int opcode) {
    switch (rxop_effects(opcode).implicit) {
        case RXOP_IMPLICIT_NONE: return "";
        case RXOP_IMPLICIT_LOCAL_COPY: return "localindex:1;localindex:2";
        case RXOP_IMPLICIT_LOCAL_TARGET: return "localindex:1";
        case RXOP_IMPLICIT_LOCAL_R0_READ_WRITE: return "local:0";
        case RXOP_IMPLICIT_LOCAL_R1_READ_WRITE: return "local:1";
        case RXOP_IMPLICIT_LOCAL_R2_READ_WRITE: return "local:2";
        case RXOP_IMPLICIT_ARGUMENT_INDEX: return "argument:2";
        case RXOP_IMPLICIT_LOCAL_RANGE_AFTER_OP3: return "windowafter:3";
        case RXOP_IMPLICIT_CAPTURED_ARGUMENT_INDEX:
            return opcode == OP_LINKARGV_REG_INT
                    ? "capturedargument:2" : "capturedargument:2:3";
        case RXOP_IMPLICIT_INLINE_RECIPE: return "recipe:3";
        case RXOP_IMPLICIT_QUALIFIED_GLOBAL: return "moduleglobal:2:3";
    }
    return NULL;
}

int rxop_return_kind(int opcode) {
    switch (opcode) {
        case OP_RET: return 1;
        case OP_RET_REG: return 2;
        case OP_RET_INT: return 3;
        case OP_RET_FLOAT: return 4;
        case OP_RET_STRING: return 5;
        default: return 0;
    }
}

unsigned int rxop_return_source_operand(int opcode) {
    return rxop_return_kind(opcode) > 1 ? 1u : 0u;
}

unsigned int rxop_return_result_component_invalidations(int opcode) {
    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS) return RXOP_COMPONENT_ALL;
    /* RET_INT/FLOAT write direct scalar fields. RET_STRING uses the owning
     * string setter, which can release reference/native payloads and reset
     * string-related state. Invalidate conservatively across all components. */
    return opcode == OP_RET_STRING ? RXOP_COMPONENT_ALL : RXOP_COMPONENT_NONE;
}

uint32_t rxop_return_result_flag_invalidations(int opcode) {
    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS) return RXFLAG_READABLE_MASK;
    /* String cache/private status changes must not retain earlier flag facts.
     * A precise subset can replace this safe mask at this canonical owner. */
    return opcode == OP_RET_STRING ? RXFLAG_READABLE_MASK : UINT32_C(0);
}

unsigned int rxop_scope_continuation_operand(int opcode) {
    return opcode == OP_INLENTER_REG_FUNC_STRING_ID ? 4u : 0u;
}

unsigned int rxop_recipe_operand(int opcode) {
    return opcode == OP_INLENTER_REG_FUNC_STRING_ID ? 3u : 0u;
}

int rxop_explicit_variant(int opcode) {
    switch (opcode) {
        case OP_LOAD_INT_INT:
        case OP_LOAD_INT_REG: return OP_LINK_REG_REG;
        case OP_INC0:
        case OP_INC1:
        case OP_INC2: return OP_INC_REG;
        case OP_DEC0:
        case OP_DEC1:
        case OP_DEC2: return OP_DEC_REG;
        default: return -1;
    }
}

RxOpRelocationKind rxop_operand_relocation(int opcode, size_t operand_index) {
    OperandType type;
    if (opcode == OP_INLENTER_REG_FUNC_STRING_ID && operand_index == 2)
        return RXOP_RELOC_INLINE_RECIPE;
    if (opcode == OP_QGLOBBIND_REG_STRING_INT && operand_index == 1)
        return RXOP_RELOC_MODULE;
    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS ||
        (rxop_effects(opcode).state != RXOP_EFFECT_CLASSIFIED &&
         rxop_effects(opcode).state != RXOP_EFFECT_CONSERVATIVE))
        return RXOP_RELOC_UNKNOWN;
    type = rxop_format_operand_type(op_table[opcode].format, operand_index);
    if ((opcode == OP_LOAD_INT_INT && operand_index < 2) ||
        (opcode == OP_LOAD_INT_REG && operand_index == 0))
        return RXOP_RELOC_LOCAL_INDEX;
    switch (type) {
        case OP_REG: return RXOP_RELOC_REGISTER;
        case OP_ID: return RXOP_RELOC_LABEL;
        case OP_FUNC: return RXOP_RELOC_PROCEDURE;
        case OP_STRING: return RXOP_RELOC_STRING;
        case OP_BINARY:
            return rxop_effects(opcode).semantics & RXOP_SEM_INDIRECT_BRANCH
                    ? RXOP_RELOC_JUMP_TABLE : RXOP_RELOC_BINARY;
        case OP_DECIMAL: return RXOP_RELOC_DECIMAL;
        default: return RXOP_RELOC_NONE;
    }
}

unsigned int rxop_context_read_mask(int opcode) {
    RxOpSignalContract signal = rxop_signal_contract(opcode);
    RxOpEffects effects = rxop_effects(opcode);
    unsigned int mask = 0;
    if (effects.state != RXOP_EFFECT_CLASSIFIED) return RXOP_CONTEXT_UNKNOWN;
    switch (opcode) {
        case OP_GETNUMDGTS_REG: return RXOP_CONTEXT_DIGITS;
        case OP_GETNUMFUZ_REG: return RXOP_CONTEXT_FUZZ;
        case OP_GETNUMFRM_REG: return RXOP_CONTEXT_FORM;
        case OP_GETNUMCAS_REG: return RXOP_CONTEXT_CASE;
        case OP_GETNUMSTD_REG: return RXOP_CONTEXT_STANDARD;
        default: break;
    }
    if ((opcode >= OP_DADD_REG_REG_REG && opcode <= OP_DMOD_REG_DECIMAL_REG) ||
        (opcode >= OP_DPOW_REG_REG_REG && opcode <= OP_DSEX_REG) ||
        (opcode >= OP_DEQ_REG_REG_REG && opcode <= OP_DLTE_REG_DECIMAL_REG))
        mask |= RXOP_CONTEXT_ALL_NUMERIC | RXOP_CONTEXT_PLUGIN;
    if (signal.dependencies & RXOP_SIGNAL_DEP_NUMERIC_CONTEXT)
        mask |= RXOP_CONTEXT_ALL_NUMERIC;
    if (signal.dependencies & RXOP_SIGNAL_DEP_PLUGIN) mask |= RXOP_CONTEXT_PLUGIN;
    if (signal.dependencies & RXOP_SIGNAL_DEP_LOCALE) mask |= RXOP_CONTEXT_LOCALE;
    if (signal.dependencies & RXOP_SIGNAL_DEP_EXTERNAL_STATE) mask |= RXOP_CONTEXT_EXTERNAL;
    if (signal.dependencies & RXOP_SIGNAL_DEP_HANDLER_POLICY) mask |= RXOP_CONTEXT_HANDLER;
    if (signal.dependencies & RXOP_SIGNAL_DEP_UNKNOWN) mask |= RXOP_CONTEXT_UNKNOWN;
    return mask;
}

unsigned int rxop_context_write_mask(int opcode) {
    if (rxop_effects(opcode).state != RXOP_EFFECT_CLASSIFIED)
        return RXOP_CONTEXT_UNKNOWN;
    switch (opcode) {
        case OP_SETNUMDGTS_REG:
        case OP_SETNUMDGTS_INT: return RXOP_CONTEXT_DIGITS;
        case OP_SETNUMFUZ_REG:
        case OP_SETNUMFUZ_INT: return RXOP_CONTEXT_FUZZ;
        case OP_SETNUMFRM_REG:
        case OP_SETNUMFRM_INT: return RXOP_CONTEXT_FORM;
        case OP_SETNUMCAS_REG:
        case OP_SETNUMCAS_INT: return RXOP_CONTEXT_CASE;
        case OP_SETNUMSTD_REG:
        case OP_SETNUMSTD_INT: return RXOP_CONTEXT_STANDARD;
        case OP_NUMSCI_INT_INT_INT:
        case OP_NUMENG_INT_INT_INT: return RXOP_CONTEXT_ALL_NUMERIC;
        default:
            return rxop_signal_contract(opcode).dependencies & RXOP_SIGNAL_DEP_UNKNOWN
                    ? RXOP_CONTEXT_UNKNOWN : RXOP_CONTEXT_READ_NONE;
    }
}

unsigned int rxop_observation_mask(int opcode) {
    if (opcode == OP_INLENTER_REG_FUNC_STRING_ID ||
        opcode == OP_QGLOBBIND_REG_STRING_INT)
        return RXOP_ACTIVATION_NONE;
    return rxop_activation_effects(opcode) &
            (RXOP_ACTIVATION_FRAME | RXOP_ACTIVATION_MODULE |
             RXOP_ACTIVATION_CODE | RXOP_ACTIVATION_DEBUG |
             RXOP_ACTIVATION_UNKNOWN);
}

unsigned int rxop_component_may_reads(int opcode, size_t operand_index) {
    RxOpEffects effects = rxop_effects(opcode);
    return rxop_effect_reads_operand(&effects, operand_index)
            ? rxop_component_reads(opcode, operand_index) : RXOP_COMPONENT_NONE;
}

unsigned int rxop_observed_components(int opcode, size_t operand_index) {
    RxOpEffects effects = rxop_effects(opcode);
    RxOpCallContract call = rxop_call_contract(opcode);
    if (!rxop_effect_reads_operand(&effects, operand_index)) return RXOP_COMPONENT_NONE;
    /* RET transfers the complete register to its caller. The caller's later
     * typed use observes a component; copying a dirty but unused component
     * here is not itself a read-before-write. */
    if (opcode == OP_RET_REG) return RXOP_COMPONENT_NONE;
    /* The payload is copied whole into pending signal storage. Validation
     * observes the separate origin structure, not unused payload components. */
    if (opcode == OP_SIGNALORIGIN_STRING_REG_REG && operand_index == 1)
        return RXOP_COMPONENT_NONE;
    if (call.kind != RXOP_CALL_NONE) {
        if (operand_index + 1 == call.count_operand) return RXOP_COMPONENT_INTEGER;
        if (call.kind == RXOP_CALL_DYNAMIC && operand_index + 1 == call.target_operand)
            return RXOP_COMPONENT_STRING;
        /* Passing a value pointer transports it. The selected callee owns
         * its typed observations; arguments are not unconditional whole-value
         * observations in the caller. */
        return RXOP_COMPONENT_NONE;
    }
    if (rxop_transport_source_operand(opcode) ||
        rxop_binding_action(opcode) != RXOP_BINDING_NONE)
        return RXOP_COMPONENT_NONE;
    /* SIGBRV records the register slot number for later event delivery. The
     * installation handler stores REG_IDX(2), without reading the selected
     * register's payload. Keep the conservative may-read/liveness inventory
     * separate from this typed read-before-definition observation. */
    if (opcode == OP_SIGBRV_ID_REG_STRING) return RXOP_COMPONENT_NONE;
    /* These operations only inspect status. Preserving unrelated flags is
     * transport, not a typed observation of a scalar payload. */
    if (opcode == OP_GETTP_REG_REG || opcode == OP_GETANDTP_REG_REG_INT ||
        opcode == OP_SETTP_REG_INT || opcode == OP_SETORTP_REG_INT ||
        opcode == OP_BRTPT_ID_REG || opcode == OP_BRTPANDT_ID_REG_INT)
        return RXOP_COMPONENT_NONE;
    if (opcode == OP_SETTPMASK_REG_REG_INT)
        return operand_index == 1 ? RXOP_COMPONENT_INTEGER : RXOP_COMPONENT_NONE;
    return rxop_component_reads(opcode, operand_index);
}

uint32_t rxop_observed_flags(int opcode, size_t operand_index) {
    if (operand_index == 1 &&
        (opcode == OP_GETTP_REG_REG || opcode == OP_GETANDTP_REG_REG_INT ||
         opcode == OP_BRTPANDT_ID_REG_INT)) return RXFLAG_READABLE_MASK;
    if (operand_index == 1 && opcode == OP_BRTPT_ID_REG)
        return RXFLAG_PUBLIC_TEST_MASK;
    if ((opcode == OP_ASSERTINITIALIZED_REG && operand_index == 0) ||
        (opcode == OP_ISINITIALIZED_REG_REG && operand_index == 1))
        return RXFLAG_VM_OBJECT_UNINITIALIZED;
    return UINT32_C(0);
}

int rxop_observation_is_known(int opcode, size_t operand_index) {
    RxOpEffects effects = rxop_effects(opcode);
    if (effects.state != RXOP_EFFECT_CLASSIFIED) return 0;
    if (!rxop_effect_reads_operand(&effects, operand_index)) return 1;
    if (opcode == OP_RET_REG) return 1;
    /* rxsignal_runtime_origin validates the wrapper and nested raw fields.
     * ALL is deliberate complete structural coverage here, not the inventory's
     * unknown fallback. A known observation does not prove a valid origin. */
    if (opcode == OP_SIGNALORIGIN_STRING_REG_REG && operand_index == 2) return 1;
    /* ALL is the conservative fallback of the component-read inventory, not
     * an independently proved typed observation of every value component. */
    return rxop_observed_components(opcode, operand_index) != RXOP_COMPONENT_ALL;
}

unsigned int rxop_observed_flags_mask_operand(int opcode) {
    return opcode == OP_GETANDTP_REG_REG_INT || opcode == OP_BRTPANDT_ID_REG_INT
            ? 3u : 0u;
}

unsigned int rxop_component_may_writes(int opcode, size_t operand_index) {
    RxOpEffects effects = rxop_effects(opcode);
    if (!rxop_effect_writes_operand(&effects, operand_index)) return RXOP_COMPONENT_NONE;
    /* An explicit write can release a reference/native payload, run a
     * finalizer, or change a value visible through another alias. Keeping
     * ALL here makes the inventory complete without pretending the optional
     * success-component helper describes all of those lifetime effects. */
    return RXOP_COMPONENT_ALL;
}

uint32_t rxop_flag_may_reads(int opcode, size_t operand_index) {
    RxOpEffects effects = rxop_effects(opcode);
    /* Flags can influence release/copy behavior, presence, normalization and
     * native payload dispatch. The whole published mask is a safe may-read;
     * it is deliberately not a value transfer or individual-flag proof. */
    return rxop_effect_reads_operand(&effects, operand_index)
            ? RXFLAG_READABLE_MASK : UINT32_C(0);
}

uint32_t rxop_flag_may_writes(int opcode, size_t operand_index) {
    RxOpEffects effects = rxop_effects(opcode);
    return rxop_effect_writes_operand(&effects, operand_index)
            ? RXFLAG_READABLE_MASK : UINT32_C(0);
}

unsigned int rxop_transport_source_operand(int opcode) {
    return rxop_same_storage_copy_is_noop(opcode) ? 2u : 0u;
}

unsigned int rxop_transport_components(int opcode) {
    return rxop_transport_source_operand(opcode)
            ? rxop_component_reads(opcode, 1) : RXOP_COMPONENT_NONE;
}

unsigned int rxop_literal_operand(int opcode) {
    switch (opcode) {
        case OP_LOAD_REG_INT:
        case OP_LOAD_REG_FLOAT:
        case OP_LOAD_REG_STRING:
        case OP_LOAD_REG_DECIMAL:
        case OP_LOAD_REG_BINARY:
        case OP_LOADSETTP_REG_INT_INT:
        case OP_LOADSETTP_REG_FLOAT_INT:
        case OP_LOADSETTP_REG_STRING_INT: return 2;
        default: return 0;
    }
}

unsigned int rxop_literal_components(int opcode) {
    switch (opcode) {
        case OP_LOAD_REG_INT:
        case OP_LOADSETTP_REG_INT_INT: return RXOP_COMPONENT_INTEGER;
        case OP_LOAD_REG_FLOAT:
        case OP_LOADSETTP_REG_FLOAT_INT: return RXOP_COMPONENT_FLOAT;
        case OP_LOAD_REG_STRING:
        case OP_LOADSETTP_REG_STRING_INT: return RXOP_COMPONENT_STRING;
        case OP_LOAD_REG_DECIMAL: return RXOP_COMPONENT_DECIMAL;
        case OP_LOAD_REG_BINARY: return RXOP_COMPONENT_BINARY;
        default: return RXOP_COMPONENT_NONE;
    }
}

unsigned int rxop_definition_components(int opcode, size_t operand_index) {
    RxOpEffects effects = rxop_effects(opcode);
    if (effects.state != RXOP_EFFECT_CLASSIFIED ||
        operand_index >= rxop_format_operand_count(op_table[opcode].format) ||
        rxop_transport_source_operand(opcode) ||
        rxop_binding_action(opcode) != RXOP_BINDING_NONE ||
        (effects.semantics & (RXOP_SEM_CALL | RXOP_SEM_RETURN)))
        return RXOP_COMPONENT_NONE;
    if (operand_index == 0 && rxop_literal_operand(opcode))
        return rxop_literal_components(opcode);
    if (rxop_effect_writes_operand(&effects, operand_index) &&
        rxop_component_write_is_known(opcode, operand_index))
        return rxop_component_writes(opcode, operand_index);
    return RXOP_COMPONENT_NONE;
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
