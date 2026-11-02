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
    int callback_inventory_complete;
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
    const char *value_writers_signature;
} RxOpEffectSpec;

#define RXEW(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS, WRITERS) \
    STATE, READS, WRITES, KILLS, BRANCH_TARGETS, NULL, NULL, NULL, NULL, IMPLICIT, SEMANTICS, \
    RXOP_CONST_EVAL_NONE, WRITERS
#define RXE(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS) \
    RXEW(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS, NULL)
#define RXEVW(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS, WRITERS) \
    STATE, RXOP_OP_NONE, RXOP_OP_NONE, RXOP_OP_NONE, RXOP_OP_NONE, \
    READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS, \
    RXOP_CONST_EVAL_NONE, WRITERS
#define RXEV(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS) \
    RXEVW(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS, NULL)
#define RXEVCW(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS, \
              CONST_EVALUATOR, WRITERS) \
    STATE, RXOP_OP_NONE, RXOP_OP_NONE, RXOP_OP_NONE, RXOP_OP_NONE, \
    READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS, \
    CONST_EVALUATOR, WRITERS
#define RXEVC(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS, CONST_EVALUATOR) \
    RXEVCW(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS, CONST_EVALUATOR, NULL)
#define RXECW(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS, \
             CONST_EVALUATOR, WRITERS) \
    STATE, READS, WRITES, KILLS, BRANCH_TARGETS, NULL, NULL, NULL, NULL, IMPLICIT, SEMANTICS, \
             CONST_EVALUATOR, WRITERS
#define RXEC(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS, CONST_EVALUATOR) \
    RXECW(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS, CONST_EVALUATOR, NULL)
#define RXOP_EFFECT(NAME, EFFECTS) { OP_##NAME, 0, EFFECTS },
#define RXOP_EFFECT_KNOWN(NAME, EFFECTS) { OP_##NAME, 1, EFFECTS },
static const RxOpEffectSpec rxop_effect_specs[] = {
#include "rxopeffects.h"
};
#undef RXOP_EFFECT
#undef RXOP_EFFECT_KNOWN
#undef RXEC
#undef RXEVC
#undef RXEV
#undef RXE
#undef RXEW
#undef RXEVW
#undef RXECW
#undef RXEVCW

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
#define RXSC_NONE_PENDING(EFFECT, POLICY_SOURCE, POLICY_OPERAND, POLICY_NAME) \
    RXOP_SIGNAL_STATE_NONE, RXOP_SIGNAL_PHASE_NONE, RXOP_SIGNAL_SOURCE_NONE, \
    SIZE_MAX, NULL, RXOP_OP_NONE, NULL, RXOP_COMPONENT_NONE, \
    RXOP_CONTEXT_NONE, RXOP_SIGNAL_DEP_NONE, \
    RXOP_SIGNAL_CONT_NORMAL, RXOP_SIGNAL_PROP_PENDING_WRITE, \
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
    DEPENDENCIES, RXOP_SIGNAL_CONT_NORMAL | RXOP_SIGNAL_CONT_TERMINAL, \
    RXOP_SIGNAL_PROP_ADDRESS_OBSERVABLE | \
        RXOP_SIGNAL_PROP_PAYLOAD_OBSERVABLE | RXOP_SIGNAL_PROP_STATIC_NAMES_TERMINAL | PROPERTIES, \
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
#undef RXSC_NONE_PENDING
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
    effects.value_writers_signature = NULL;
    effects.callback_inventory_complete = 0;
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
    effects.value_writers_signature = spec->value_writers_signature;
    effects.callback_inventory_complete = spec->callback_inventory_complete;
    effects.flow = op_table[opcode].flow;
    effects.optimizer_barrier =
        spec->state != RXOP_EFFECT_CLASSIFIED ||
        (op_table[opcode].flags & FLG_OPT_BARRIER) != 0;

    return effects;
}

RxOpValueWriter rxop_value_writer(int opcode, size_t operand_index) {
    RxOpEffects effects = rxop_effects(opcode);
    const char *signature = effects.value_writers_signature;
    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS ||
        effects.state != RXOP_EFFECT_CLASSIFIED ||
        operand_index >= rxop_format_operand_count(op_table[opcode].format))
        return RXOP_VALUE_WRITER_UNKNOWN;
    if (!rxop_effect_writes_operand(&effects, operand_index)) return RXOP_VALUE_WRITER_NONE;
    if (!signature || operand_index >= strlen(signature)) return RXOP_VALUE_WRITER_UNKNOWN;
    if (signature[operand_index] == 's') return RXOP_VALUE_WRITER_SETTER;
    if (signature[operand_index] == 'f') return RXOP_VALUE_WRITER_FIELD;
    return RXOP_VALUE_WRITER_UNKNOWN;
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
    /* These writers change only public status bits. Their value-component
     * projection is a proved empty set, not a missing classification. */
    if (operand_index == 0 &&
        (opcode == OP_SETTP_REG_INT || opcode == OP_SETORTP_REG_INT)) return 1;
    /* The conservative write mask includes the source slot restored by
     * IGETUNLINK. That binding write never changes the source pointee. */
    if (opcode == OP_IGETUNLINK_REG_REG && operand_index == 1) return 1;
    /* The component helpers intentionally have an unknown-write default of
     * NONE. Do not confuse that absent projection with a proved no-op. */
    return rxop_component_writes(opcode, operand_index) != RXOP_COMPONENT_NONE;
}

unsigned int rxop_component_reads(int opcode, size_t operand_index) {
    RxOpSelectionContract selection = rxop_selection_contract(opcode);
    if (selection.kind != RXOP_SELECTION_NONE) {
        if (operand_index + 1 == selection.count_operand)
            return RXOP_COMPONENT_INTEGER;
        if (operand_index + 1 == selection.receiver_operand)
            return RXOP_COMPONENT_OBJECT_TYPE;
    }
    if (rxop_call_invokes(opcode) &&
        operand_index + 1 == rxop_call_contract(opcode).target_operand)
        return rxop_call_target_components(opcode);
    /* Public flag operations inspect status bits, never value components.
     * Preserve the distinction from the conservative ALL default below. */
    if (((opcode == OP_SETTP_REG_INT || opcode == OP_SETORTP_REG_INT) && operand_index == 0) ||
        ((opcode == OP_GETTP_REG_REG || opcode == OP_GETANDTP_REG_REG_INT ||
          opcode == OP_BRTPT_ID_REG || opcode == OP_BRTPANDT_ID_REG_INT) && operand_index == 1))
        return RXOP_COMPONENT_NONE;
    if ((opcode == OP_SAY_REG || opcode == OP_SAYX_REG) && operand_index == 0)
        return RXOP_COMPONENT_STRING;
    if (opcode == OP_SETOBJTYPE_REG_STRING && operand_index == 0)
        return RXOP_COMPONENT_ATTRIBUTES;
    if ((opcode == OP_ASSERTTYPE_REG_STRING ||
         opcode == OP_ASSERTINITIALIZED_REG) && operand_index == 0)
        return RXOP_COMPONENT_OBJECT_TYPE;
    if ((opcode == OP_ISTYPE_REG_REG_STRING || opcode == OP_TYPEOF_REG_REG) &&
        operand_index == 1) return RXOP_COMPONENT_OBJECT_TYPE;
    if (opcode == OP_ISINITIALIZED_REG_REG && operand_index == 1)
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
    if ((opcode == OP_COPY_REG_REG || opcode == OP_MOVE_REG_REG) &&
        operand_index == 1) return RXOP_COMPONENT_ALL;
    if (opcode == OP_ICOPY_REG_REG && operand_index == 1) return RXOP_COMPONENT_INTEGER;
    if (opcode == OP_FCOPY_REG_REG && operand_index == 1) return RXOP_COMPONENT_FLOAT;
    if (opcode == OP_SCOPY_REG_REG && operand_index == 1) return RXOP_COMPONENT_STRING;
    if (opcode == OP_DCOPY_REG_REG && operand_index == 1) return RXOP_COMPONENT_DECIMAL;
    if (opcode == OP_ACOPY_REG_REG && operand_index == 1) return RXOP_COMPONENT_ATTRIBUTES;
    if (opcode == OP_BCOPY_REG_REG && operand_index == 1)
        return RXOP_COMPONENT_BINARY | RXOP_COMPONENT_NATIVE_PAYLOAD;
    if ((opcode == OP_IGETUNLINK_REG_REG ||
         opcode == OP_ISETUNLINK_REG_REG) && operand_index == 1)
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
        case OP_MOVE_REG_REG:
            return 1;
        default:
            return 0;
    }
}

/* NONE alone cannot distinguish a known no-pointee-write (including a slot
 * restore) from a missing projection; consult component_write_is_known. */
unsigned int rxop_component_writes(int opcode, size_t operand_index) {
    RxOpSelectionContract selection = rxop_selection_contract(opcode);
    if (selection.kind != RXOP_SELECTION_NONE)
        return operand_index + 1 == selection.result_operand
                ? RXOP_COMPONENT_INTEGER : RXOP_COMPONENT_NONE;
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
    if (opcode == OP_MOVE_REG_REG && operand_index == 1)
        return RXOP_COMPONENT_ALL;
    if (operand_index != 0) return RXOP_COMPONENT_NONE;
    if (opcode == OP_COPY_REG_REG || opcode == OP_MOVE_REG_REG ||
        opcode == OP_NULL_REG)
        return RXOP_COMPONENT_ALL;
    if (opcode == OP_SETOBJUNINIT_REG_STRING) return RXOP_COMPONENT_ALL;
    if (opcode == OP_ICOPY_REG_REG || opcode == OP_LOAD_REG_INT ||
        opcode == OP_STRLEN_REG_REG || opcode == OP_STRPOS_REG_REG_REG ||
        opcode == OP_GETTP_REG_REG || opcode == OP_GETANDTP_REG_REG_INT ||
        opcode == OP_GETNUMDGTS_REG || opcode == OP_GETNUMFUZ_REG ||
        opcode == OP_GETNUMFRM_REG || opcode == OP_GETNUMCAS_REG ||
        opcode == OP_GETNUMSTD_REG || opcode == OP_ISTYPE_REG_REG_STRING ||
        opcode == OP_ISINITIALIZED_REG_REG)
        return RXOP_COMPONENT_INTEGER;
    if (opcode == OP_FCOPY_REG_REG || opcode == OP_LOAD_REG_FLOAT)
        return RXOP_COMPONENT_FLOAT;
    if (opcode == OP_SCOPY_REG_REG || opcode == OP_LOAD_REG_STRING ||
        opcode == OP_TYPEOF_REG_REG)
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
        return RXOP_COMPONENT_OBJECT_TYPE | RXOP_COMPONENT_ATTRIBUTES;
    if (opcode == OP_ISETATTR1_REG_INT_REG)
        return RXOP_COMPONENT_INTEGER;
    if (opcode == OP_IGETUNLINK_REG_REG || opcode == OP_ISETUNLINK_REG_REG)
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
    RxOpSelectionContract selection = rxop_selection_contract(opcode);
    if (selection.kind != RXOP_SELECTION_NONE &&
        operand_index + 1 == selection.result_operand)
        return RXOP_COMPONENT_ALL;
    if (opcode == OP_NULLN_REG_REG || opcode == OP_NULLN_REG_REG_REG ||
        opcode == OP_NULLN_REG_REG_REG_REG) return RXOP_COMPONENT_ALL;
    if (opcode == OP_MOVE_REG_REG && operand_index == 1)
        return RXOP_COMPONENT_ALL;
    if (operand_index == 0) {
        if (opcode == OP_NULL_REG || opcode == OP_TYPEOF_REG_REG ||
            opcode == OP_ISTYPE_REG_REG_STRING || opcode == OP_ISINITIALIZED_REG_REG)
            return RXOP_COMPONENT_ALL;
        if (opcode == OP_SETOBJUNINIT_REG_STRING)
            return RXOP_COMPONENT_ALL & ~RXOP_COMPONENT_OBJECT_TYPE;
    }
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
    /* The owning row names the implementation, not a result-type range.
     * This also covers setters at non-first explicit operands. */
    if (rxop_value_writer(opcode, operand_index) == RXOP_VALUE_WRITER_SETTER)
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

int rxop_scoped_diagnostics(void) {
    /* Matching current VM semantics interleave committed logical scopes in
     * the existing detached diagnostic walker. See INLINE_SCOPES contract. */
    return 1;
}

int rxop_residual_observable(int opcode) {
    RxOpEffects effects = rxop_effects(opcode);
    RxOpSignalContract signal = rxop_signal_contract(opcode);
    size_t i, count;
    if (effects.state != RXOP_EFFECT_CLASSIFIED) return 1;
    if (effects.semantics != RXOP_SEM_NONE ||
        effects.flow != FLOW_NEXT || signal.state != RXOP_SIGNAL_STATE_NONE ||
        signal.policy_effect != RXOP_POLICY_EFFECT_NONE ||
        rxop_binding_action(opcode) != RXOP_BINDING_NONE ||
        rxop_context_write_mask(opcode) != RXOP_CONTEXT_READ_NONE ||
        rxop_observation_mask(opcode) != RXOP_ACTIVATION_NONE)
        return 1;
    count = rxop_format_operand_count(op_table[opcode].format);
    for (i = 0; i < count; ++i)
        if (rxop_component_clears(opcode, i) &
            (RXOP_COMPONENT_REFERENCE | RXOP_COMPONENT_NATIVE_PAYLOAD)) return 1;
    return 0;
}

RxOpSelectionContract rxop_selection_contract(int opcode) {
    RxOpSelectionContract selection = { RXOP_SELECTION_NONE, 0, 0, 0, 0, 0 };
    switch (opcode) {
        case OP_SRCMETHODSEL_REG_REG_STRING:
            selection.kind = RXOP_SELECTION_METHOD;
            selection.result_operand = 1; selection.descriptor_operand = 3;
            selection.receiver_operand = 2; break;
        case OP_SRCFPROCSEL_REG_STRING_REG:
            selection.kind = RXOP_SELECTION_FACTORY;
            selection.result_operand = 1; selection.descriptor_operand = 2;
            selection.count_operand = 3; selection.window_operand = 3; break;
        default: break;
    }
    return selection;
}

RxOpCallContract rxop_call_contract(int opcode) {
    RxOpCallContract call = { RXOP_CALL_NONE, 0, 0, 0, 0, "", "", 1, 1 };
    switch (opcode) {
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
        case OP_SRCFPROCSEL_REG_STRING_REG: {
            RxOpSelectionContract selection = rxop_selection_contract(opcode);
            call.kind = RXOP_CALL_FACTORY;
            call.target_operand = selection.descriptor_operand;
            call.result_operand = selection.result_operand;
            call.count_operand = selection.count_operand;
            call.window_operand = selection.window_operand; break;
        }
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

int rxop_call_invokes(int opcode) {
    RxOpCallKind kind = rxop_call_contract(opcode).kind;
    return kind == RXOP_CALL_DIRECT || kind == RXOP_CALL_DYNAMIC;
}

#define CALL_CAUSE_CALLBACKS(KIND, SOURCE, NAMES, POINT, CONDITION, CALLBACKS) \
    { RXOP_CALL_CAUSE_##KIND, RXOP_SIGNAL_SOURCE_##SOURCE, NAMES, \
      RXOP_CALL_POINT_##POINT, RXOP_CALL_CONDITION_##CONDITION, CALLBACKS }
#define CALL_CAUSE(KIND, SOURCE, NAMES, POINT, CONDITION) \
    CALL_CAUSE_CALLBACKS(KIND, SOURCE, NAMES, POINT, CONDITION, RXOP_CALLBACK_NONE)
#define CALL_TARGET_CAUSE \
    CALL_CAUSE(TARGET, STATIC_NAMES, "FUNCTION_NOT_FOUND", TARGET, TARGET_UNRESOLVED)
#define CALL_FRAME_CAUSES \
    CALL_CAUSE(INITIALIZER, PROPAGATED_CALL, "FAILURE", INITIALIZER, BYTECODE_INITIALIZER_NEEDED), \
    CALL_CAUSE(FRAME_LAYOUT, STATIC_NAMES, "FAILURE", AFTER_INITIALIZER, BYTECODE_FRAME_LAYOUT_INVALID), \
    CALL_CAUSE(FRAME_ALLOCATION, STATIC_NAMES, "FAILURE", FRAME_ALLOCATION, BYTECODE_ALLOCATION_FAILED)
static const RxOpCallCause call_plain_causes[] = {
    CALL_TARGET_CAUSE,
    CALL_FRAME_CAUSES,
    CALL_CAUSE_CALLBACKS(PROVIDER, PROPAGATED_CALL, NULL, PROVIDER, PROVIDER_INVOKED, \
                         RXOP_CALLBACK_INVOCATION)
};
static const RxOpCallCause call_counted_causes[] = {
    CALL_TARGET_CAUSE,
    CALL_CAUSE(INITIALIZER, PROPAGATED_CALL, "FAILURE", INITIALIZER, BYTECODE_INITIALIZER_NEEDED),
    CALL_CAUSE(WINDOW, STATIC_NAMES, "FAILURE", AFTER_INITIALIZER, BYTECODE_WINDOW_INVALID),
    CALL_CAUSE(FRAME_LAYOUT, STATIC_NAMES, "FAILURE", AFTER_INITIALIZER, BYTECODE_FRAME_LAYOUT_INVALID),
    CALL_CAUSE(FRAME_ALLOCATION, STATIC_NAMES, "FAILURE", FRAME_ALLOCATION, BYTECODE_ALLOCATION_FAILED),
    CALL_CAUSE_CALLBACKS(PROVIDER, PROPAGATED_CALL, NULL, PROVIDER, PROVIDER_INVOKED, \
                         RXOP_CALLBACK_INVOCATION)
};
static const RxOpCallCause call_fixed_causes[] = {
    CALL_TARGET_CAUSE,
    CALL_CAUSE(TARGET_KIND, STATIC_NAMES, "NOT_IMPLEMENTED", TARGET, FIXED_TARGET_NATIVE),
    CALL_FRAME_CAUSES,
    CALL_CAUSE_CALLBACKS(PROVIDER, PROPAGATED_CALL, NULL, PROVIDER, BYTECODE_PROVIDER_INVOKED, \
                         RXOP_CALLBACK_INVOCATION)
};
#undef CALL_FRAME_CAUSES
#undef CALL_TARGET_CAUSE
#undef CALL_CAUSE
#undef CALL_CAUSE_CALLBACKS

RxOpCallCauseContract rxop_call_cause_contract(int opcode) {
    RxOpCallCauseContract result = { RXOP_CALL_CAUSES_UNKNOWN, NULL, 0u };
    RxOpCallContract call;
    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS) return result;
    call = rxop_call_contract(opcode);
    if (!rxop_call_invokes(opcode)) {
        if (rxop_effects(opcode).state == RXOP_EFFECT_CLASSIFIED)
            result.state = RXOP_CALL_CAUSES_NONE;
        return result;
    }
    result.state = RXOP_CALL_CAUSES_KNOWN;
    if (call.actual_operands[0]) {
        result.causes = call_fixed_causes;
        result.count = sizeof(call_fixed_causes) / sizeof(*call_fixed_causes);
    } else if (opcode == OP_CALL_REG_FUNC_REG || opcode == OP_DCALL_REG_REG_REG) {
        /* These handlers check rxvm_call_count_fits after initializer return.
         * Legacy fused mapped calls currently have no such separate check;
         * their invalid frame extent remains the FRAME_LAYOUT obligation. */
        result.causes = call_counted_causes;
        result.count = sizeof(call_counted_causes) / sizeof(*call_counted_causes);
    } else {
        result.causes = call_plain_causes;
        result.count = sizeof(call_plain_causes) / sizeof(*call_plain_causes);
    }
    return result;
}

const char *rxop_call_cause_state_name(RxOpCallCauseState value) {
    static const char *const names[] = { "none", "known", "unknown" };
    return value >= RXOP_CALL_CAUSES_NONE && value <= RXOP_CALL_CAUSES_UNKNOWN
            ? names[value] : "unknown";
}

const char *rxop_call_cause_kind_name(RxOpCallCauseKind value) {
    static const char *const names[] = { "preparation-callback", "target", "target-kind",
        "initializer", "window", "frame-layout", "frame-allocation", "frame-callback", "provider",
        "context-sync" };
    return value >= RXOP_CALL_CAUSE_PREPARATION_CALLBACK && value <= RXOP_CALL_CAUSE_CONTEXT_SYNC
            ? names[value] : "unknown";
}

const char *rxop_call_checkpoint_name(RxOpCallCheckpoint value) {
    static const char *const names[] = { "preparation", "target", "initializer",
        "after-initializer", "frame-allocation", "frame-preparation", "provider" };
    return value >= RXOP_CALL_POINT_PREPARATION && value <= RXOP_CALL_POINT_PROVIDER
            ? names[value] : "unknown";
}

const char *rxop_call_cause_condition_name(RxOpCallCauseCondition value) {
    static const char *const names[] = { "result-clear", "target-unresolved", "fixed-target-native",
        "bytecode-initializer-needed", "bytecode-window-invalid", "bytecode-frame-layout-invalid",
        "bytecode-allocation-failed", "bytecode-frame-hook", "provider-invoked", "bytecode-provider-invoked",
        "bytecode-decimal-sync" };
    return value >= RXOP_CALL_CONDITION_RESULT_CLEAR && value <= RXOP_CALL_CONDITION_BYTECODE_DECIMAL_SYNC
            ? names[value] : "unknown";
}

RxOpCallFrameContract rxop_call_frame_contract(int opcode) {
    RxOpCallFrameContract result = {
        RXOP_CALL_FRAME_UNKNOWN, RXOP_CALL_FRAME_FRESH_UNKNOWN,
        RXOP_ABI_ROLE_NONE, RXOP_ABI_EXTENT_NONE, RXOP_CALL_FRAME_CLEAR_UNKNOWN,
        RXOP_CALL_FRAME_LOCAL_UNKNOWN, RXOP_ABI_ROLE_NONE
    };
    RxOpCallCauseContract causes = rxop_call_cause_contract(opcode);
    RxOpEffects effects;
    size_t row;
    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS) return result;
    if (!rxop_call_invokes(opcode)) {
        if (causes.state == RXOP_CALL_CAUSES_NONE) {
            result.state = RXOP_CALL_FRAME_NONE;
            result.fresh_mode = RXOP_CALL_FRAME_FRESH_NONE;
            result.clear_when = RXOP_CALL_FRAME_CLEAR_NONE;
            result.local_mode = RXOP_CALL_FRAME_LOCAL_NONE;
        }
        return result;
    }
    effects = rxop_effects(opcode);
    if (causes.state != RXOP_CALL_CAUSES_KNOWN || !causes.causes || !causes.count ||
            effects.state != RXOP_EFFECT_CLASSIFIED || !effects.callback_inventory_complete ||
            RXOP_CALL_FRAME_REUSED_LOCALS_CLEAR != 0) return result;
    for (row = 0u; row < causes.count; ++row) {
        const RxOpCallCause *cause = &causes.causes[row];
        /* Prior owned finalizers now run at frame retirement. No ordinary
         * recycler resource callback remains at frame preparation. */
        if (cause->kind == RXOP_CALL_CAUSE_FRAME_CALLBACK) return result;
    }
    /* frame_f's reused path relinks owned locals without clearing them;
     * it resets only original-owned a0, then writes its integer count.
     * Fresh value_init and the reused reset are distinct native operations. */
    result.state = RXOP_CALL_FRAME_KNOWN;
    result.fresh_mode = RXOP_CALL_FRAME_VALUE_INIT;
    result.clear_role = RXOP_ABI_A0_ORIGINAL_BACKING;
    result.clear_extent = RXOP_ABI_ACTIVE_TREE;
    result.clear_when = RXOP_CALL_FRAME_REUSED_FRAME;
    result.local_mode = RXOP_CALL_FRAME_RELINK_ORIGINAL;
    result.count_role = RXOP_ABI_A0_ORIGINAL_BACKING;
    return result;
}

const char *rxop_call_frame_state_name(RxOpCallFrameState value) {
    static const char *const names[] = { "none", "known", "unknown" };
    return (unsigned int)value < sizeof(names) / sizeof(names[0]) ? names[value] : "unknown";
}
const char *rxop_call_frame_fresh_mode_name(RxOpCallFrameFreshMode value) {
    static const char *const names[] = { "none", "value-init", "unknown" };
    return (unsigned int)value < sizeof(names) / sizeof(names[0]) ? names[value] : "unknown";
}
const char *rxop_call_frame_clear_when_name(RxOpCallFrameClearWhen value) {
    static const char *const names[] = { "none", "reused-frame", "unknown" };
    return (unsigned int)value < sizeof(names) / sizeof(names[0]) ? names[value] : "unknown";
}
const char *rxop_call_frame_local_mode_name(RxOpCallFrameLocalMode value) {
    static const char *const names[] = { "none", "relink-original", "unknown" };
    return (unsigned int)value < sizeof(names) / sizeof(names[0]) ? names[value] : "unknown";
}

RxOpCallFrameState rxop_call_frame_control_state(int opcode) {
    return rxop_call_frame_contract(opcode).state;
}

RxOpFrameControlEntryValue rxop_call_frame_control_entry(int opcode,
                                                       RxOpFrameControlEntryField field) {
    RxOpCallFrameState state = rxop_call_frame_control_state(opcode);
    if ((unsigned int)field >= RXOP_FRAME_ENTRY_FIELD_COUNT ||
            state == RXOP_CALL_FRAME_UNKNOWN) return RXOP_FRAME_ENTRY_UNKNOWN;
    if (state == RXOP_CALL_FRAME_NONE) return RXOP_FRAME_ENTRY_NONE;
    switch (field) {
        case RXOP_FRAME_ENTRY_PLUGINS: return RXOP_FRAME_ENTRY_PARENT_BORROWED;
        case RXOP_FRAME_ENTRY_PLUGIN_OWNERS: return RXOP_FRAME_ENTRY_NONE;
        case RXOP_FRAME_ENTRY_INVOCATION:
        case RXOP_FRAME_ENTRY_HANDLERS: return RXOP_FRAME_ENTRY_EMPTY;
        case RXOP_FRAME_ENTRY_SCOPE: return RXOP_FRAME_ENTRY_NONE;
        case RXOP_FRAME_ENTRY_POLICY: return RXOP_FRAME_ENTRY_PARENT_BORROWED_COW;
        case RXOP_FRAME_ENTRY_REFERENCES: return RXOP_FRAME_ENTRY_UNMARKED;
        case RXOP_FRAME_ENTRY_INTERRUPT: return RXOP_FRAME_ENTRY_PARENT_INHERITED;
        case RXOP_FRAME_ENTRY_ACTION: return RXOP_FRAME_ENTRY_OFF;
        case RXOP_FRAME_ENTRY_CALLER_WINDOW:
            return rxop_call_contract(opcode).window_operand
                    ? RXOP_FRAME_ENTRY_MAPPED_WINDOW : RXOP_FRAME_ENTRY_NONE;
        default: return RXOP_FRAME_ENTRY_UNKNOWN;
    }
}

const char *rxop_frame_control_entry_field_name(RxOpFrameControlEntryField field) {
    static const char *const names[] = { "plugins", "pluginowners", "invocation", "scope",
        "handlers", "policy", "references", "interrupt", "action", "callerwindow" };
    return (unsigned int)field < sizeof(names) / sizeof(names[0]) ? names[field] : "unknown";
}

const char *rxop_frame_control_entry_value_name(RxOpFrameControlEntryValue value) {
    static const char *const names[] = { "none", "parent-borrowed", "empty",
        "parent-borrowed-cow", "unmarked", "parent-inherited", "off", "mapped-window", "unknown" };
    return (unsigned int)value < sizeof(names) / sizeof(names[0]) ? names[value] : "unknown";
}

RxOpFrameControlContract rxop_frame_control_contract(int opcode) {
    RxOpFrameControlContract result = { RXOP_FRAME_CONTROL_UNKNOWN, RXOP_FRAME_CONTROL_UNCLASSIFIED };
    RxOpEffects effects;
    RxOpSignalContract signal;
    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS) return result;
    effects = rxop_effects(opcode);
    signal = rxop_signal_contract(opcode);
    if (effects.state != RXOP_EFFECT_CLASSIFIED) return result;
    /* An incomplete intrinsic policy inventory is not a no-control-write
     * theorem. The separately reviewed scalar-output/getter footprint family
     * and MKREF have exact direct control despite their open signal routes. */
    if (signal.policy_effect == RXOP_POLICY_EFFECT_UNKNOWN &&
            !rxop_intrinsic_value_footprint_is_known(opcode) &&
            opcode != OP_MKREF_REG_REG) return result;
    result.state = RXOP_FRAME_CONTROL_KNOWN;
    result.mask = RXOP_FRAME_CONTROL_NONE;
    switch (signal.policy_effect) {
        case RXOP_POLICY_EFFECT_NONE:
        case RXOP_POLICY_EFFECT_UNKNOWN:
        case RXOP_POLICY_EFFECT_BREAKPOINT_ENABLE_EXISTING:
        case RXOP_POLICY_EFFECT_BREAKPOINT_DISABLE: break;
        case RXOP_POLICY_EFFECT_PUSH:
            result.mask |= RXOP_FRAME_CONTROL_HANDLERS; break;
        case RXOP_POLICY_EFFECT_POP:
            result.mask |= RXOP_FRAME_CONTROL_POLICY | RXOP_FRAME_CONTROL_HANDLERS; break;
        default: result.mask |= RXOP_FRAME_CONTROL_POLICY; break;
    }
    if (effects.semantics & (RXOP_SEM_REFERENCE_CREATE | RXOP_SEM_REFERENCE_RELEASE |
                             RXOP_SEM_LIFETIME_END))
        result.mask |= RXOP_FRAME_CONTROL_REFERENCES;
    if (opcode == OP_MKREF_REG_REG || opcode == OP_RET_REG)
        result.mask |= RXOP_FRAME_CONTROL_ANCESTOR_REFERENCES;
    if (rxop_call_invokes(opcode)) result.mask |= RXOP_FRAME_CONTROL_CHILD;
    if (effects.semantics & RXOP_SEM_RETURN)
        result.mask |= RXOP_FRAME_CONTROL_RETIRE;
    if (rxop_callback_obligations(opcode) ||
            ((effects.semantics & RXOP_SEM_OPAQUE) &&
             !rxop_intrinsic_value_footprint_is_known(opcode)) ||
            (signal.dependencies & RXOP_SIGNAL_DEP_PLUGIN))
        result.mask |= RXOP_FRAME_CONTROL_FOREIGN;
    if (signal.state != RXOP_SIGNAL_STATE_NONE ||
            (signal.properties & (RXOP_SIGNAL_PROP_ASYNC_ENTRY | RXOP_SIGNAL_PROP_PENDING_WRITE)))
        result.mask |= RXOP_FRAME_CONTROL_SIGNAL;
    /* No reviewed intrinsic above installs a plugin owner. FOREIGN/CHILD and
     * every ambient edge must still be closed before relying on that bit's
     * absence. In particular, loaded_here is not inferred from value purity. */
    return result;
}

const char *rxop_frame_control_state_name(RxOpFrameControlState state) {
    return state == RXOP_FRAME_CONTROL_KNOWN ? "known" : "unknown";
}

unsigned int rxop_frame_control_mask_named(const char *name) {
    static const struct { const char *name; unsigned int mask; } names[] = {
        { "plugin-owners", RXOP_FRAME_CONTROL_PLUGIN_OWNERS },
        { "policy", RXOP_FRAME_CONTROL_POLICY }, { "handlers", RXOP_FRAME_CONTROL_HANDLERS },
        { "scope", RXOP_FRAME_CONTROL_SCOPE }, { "references", RXOP_FRAME_CONTROL_REFERENCES },
        { "ancestor-references", RXOP_FRAME_CONTROL_ANCESTOR_REFERENCES },
        { "child", RXOP_FRAME_CONTROL_CHILD }, { "retire", RXOP_FRAME_CONTROL_RETIRE },
        { "foreign", RXOP_FRAME_CONTROL_FOREIGN }, { "signal", RXOP_FRAME_CONTROL_SIGNAL },
        { "unknown", RXOP_FRAME_CONTROL_UNCLASSIFIED }
    };
    size_t i;
    if (!name) return 0u;
    for (i = 0u; i < sizeof(names) / sizeof(names[0]); ++i)
        if (!strcmp(name, names[i].name)) return names[i].mask;
    return 0u;
}

RxOpFramePluginOwnership rxop_frame_plugin_ownership(void) {
    return RXOP_FRAME_PLUGIN_OWNERSHIP_NONE_PUBLISHED_INTERFACE_V1;
}

const char *rxop_frame_plugin_ownership_name(RxOpFramePluginOwnership state) {
    return state == RXOP_FRAME_PLUGIN_OWNERSHIP_NONE_PUBLISHED_INTERFACE_V1
            ? "none-published-interface.v1" : "unknown";
}

int rxop_call_variant(int opcode, size_t count) {
    static const int fixed[] = {0, OP_CALL1_REG_FUNC_REG,
        OP_CALL2_REG_FUNC_REG_REG, OP_CALL3_REG_FUNC_REG_REG_REG,
        OP_CALL4_REG_FUNC_REG_REG_REG_REG};
    RxOpCallContract call = rxop_call_contract(opcode);
    if (count > 4u || call.kind != RXOP_CALL_DIRECT ||
        call.result_operand != 1u || call.prefix[0]) return 0;
    if (call.actual_operands[0]) return fixed[count];
    /* Fixed CALL1--4 capture pointers before initialization. Counted forms
     * capture afterwards. Zero-actual CALL additionally clears its result
     * before ingress, unlike counted zero. Neither conversion is equivalent
     * without a separate site proof, so preserve the counted form. */
    return 0;
}

unsigned int rxop_selection_window_components(int opcode) {
    /* A context-wide factory match procedure receives the actual values and
     * can inspect more than their static type/status. No provider-universe
     * theorem is implied by this complete conservative input observation. */
    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS) return RXOP_COMPONENT_ALL;
    return rxop_selection_contract(opcode).kind == RXOP_SELECTION_FACTORY
            ? RXOP_COMPONENT_ALL : RXOP_COMPONENT_NONE;
}

int rxop_selection_callbacks(int opcode) {
    return rxop_selection_contract(opcode).kind == RXOP_SELECTION_FACTORY;
}

enum {
    RXOP_HOST_CALLBACK_PRESENT = 1u,
    RXOP_HOST_CALLBACK_PRESERVES_SIGNAL_POLICY = 2u,
    RXOP_HOST_CALLBACK_BYTES_SNAPSHOT = 4u,
    RXOP_HOST_CALLBACK_PRESERVES_FRAME = 8u
};

/* Keep the reviewed route and its separate structural-policy contract in one
 * inventory. A future arbitrary host route must not inherit the bytes-output
 * contract merely by being classified as a host callback. */
static unsigned int rxop_host_callback_contract(int opcode) {
    switch (opcode) {
        case OP_SAY_REG: case OP_SAYX_REG: case OP_SAYX_STRING:
        case OP_SAY_INT: case OP_SAY_FLOAT: case OP_SAY_STRING:
        case OP_SAY_CHAR:
            /* rxvm_say_write/rxvm_mprintf pass bytes only to the public exit.
             * Supported nested VM entry uses another root/frame; child handler
             * writes COW their own table. Requesting an interrupt changes the
             * pending word, not this suspended frame's structural policy. The
             * context output writer separately forbids reentry. The custom
             * exit/writer receives a temporary byte copy, no value handle.
             * Neither exit receives a live frame pointer or a value handle.
             * A normal return preserves the suspended pointer table and
             * activation, but promises no value, observation or I/O result. */
            return RXOP_HOST_CALLBACK_PRESENT |
                   RXOP_HOST_CALLBACK_PRESERVES_SIGNAL_POLICY |
                   RXOP_HOST_CALLBACK_BYTES_SNAPSHOT |
                   RXOP_HOST_CALLBACK_PRESERVES_FRAME;
        default: return 0;
    }
}

int rxop_host_callbacks(int opcode) {
    return (rxop_host_callback_contract(opcode) & RXOP_HOST_CALLBACK_PRESENT) != 0u;
}

int rxop_context_sync_callbacks(int opcode) {
    /* ABI-3 synchronization is total, nonallocating and non-reentrant. The
     * numeric write and live provider attachment remain separate ABI effects. */
    (void)opcode;
    return 0;
}

int rxop_intrinsic_value_footprint_is_known(int opcode) {
    /* SAY/SAYX read only their typed scalar operand (or constant pool) before
     * entering rxvm_say_write/rxvm_mprintf. They perform no intrinsic value
     * write. Legacy byte exits may reenter: retain host callbacks, signals,
     * opaque optimizer barriers and the observable external output. */
    switch (opcode) {
        case OP_SAY_REG: case OP_SAYX_REG: case OP_SAYX_STRING:
        case OP_SAY_INT: case OP_SAY_FLOAT: case OP_SAY_STRING:
        case OP_SAY_CHAR: return 1;
        /* Direct context getters observe no Rexx value operand. Their
         * numeric context read and integer field write remain separate. */
        case OP_GETNUMDGTS_REG: case OP_GETNUMFUZ_REG:
        case OP_GETNUMFRM_REG: case OP_GETNUMCAS_REG:
        case OP_GETNUMSTD_REG: return 1;
        default: return 0;
    }
}

int rxop_abi_callbacks(int opcode) {
    /* Register assignment invokes real source native COPY hooks, including
     * active attributes. String setters only run conforming native cleanup. */
    return rxop_return_kind(opcode) == 2;
}

int rxop_departure_callbacks(int opcode) {
    /* A physical root can copy external arguments through real COPY hooks.
     * Ordinary owned backing retirement only runs cleanup-only finalizers.
     * An internal-parent fact can discharge the separate root obligation. */
    return !!(rxop_effects(opcode).semantics & RXOP_SEM_RETURN);
}

/* Hidden ABI ordering mirrors the ordinary RET handlers. These rows describe
 * effects, never authority that an activation, storage or callback is safe. */
#define ABI_STEP(OP, POINT, SRC, DEST, SE, DE, RULE, REPEAT) \
    { RXOP_ABI_##OP, RXOP_ABI_##POINT, RXOP_ABI_##SRC, RXOP_ABI_##DEST, \
      RXOP_ABI_##SE, RXOP_ABI_##DE, RXOP_ABI_##RULE, RXOP_ABI_##REPEAT }
#define ABI_PHYSICAL_ROWS \
    ABI_STEP(RESTORE_NUMERIC, PHYSICAL_NUMERIC, PARENT_NUMERIC, ACTIVE_NUMERIC, EXTENT_NONE, EXTENT_NONE, INTERNAL_PARENT, ONCE), \
    ABI_STEP(RETIRE_FRAME, PHYSICAL_RETIRE, PHYSICAL_FRAME, ROLE_NONE, EXTENT_NONE, EXTENT_NONE, INTERNAL_PARENT, ONCE)
#define ABI_RESULT_REGISTER \
    ABI_STEP(RETURN_REGISTER, RETURN_RESULT, RETURN_ACTIVE, CAPTURED_RESULT, ACTIVE_TREE, FULL_CAPACITY, RESULT_PRESENT, ONCE)
#define ABI_RESULT_INTEGER \
    ABI_STEP(RETURN_INTEGER, RETURN_RESULT, RETURN_LITERAL, CAPTURED_RESULT, EXTENT_NONE, EXTENT_NONE, RESULT_PRESENT, ONCE)
#define ABI_RESULT_FLOAT \
    ABI_STEP(RETURN_FLOAT, RETURN_RESULT, RETURN_LITERAL, CAPTURED_RESULT, EXTENT_NONE, EXTENT_NONE, RESULT_PRESENT, ONCE)
#define ABI_RESULT_STRING \
    ABI_STEP(RETURN_STRING, RETURN_RESULT, RETURN_LITERAL, CAPTURED_RESULT, EXTENT_NONE, TOP, RESULT_PRESENT, ONCE)
/* Every return to a parent restores its borrowed decimal attachment/cache.
 * Frame retirement retains owned capacity. */
static const RxOpAbiStep abi_physical_void[] = { ABI_PHYSICAL_ROWS };
static const RxOpAbiStep abi_physical_register[] = { ABI_RESULT_REGISTER, ABI_PHYSICAL_ROWS };
static const RxOpAbiStep abi_physical_integer[] = { ABI_RESULT_INTEGER, ABI_PHYSICAL_ROWS };
static const RxOpAbiStep abi_physical_float[] = { ABI_RESULT_FLOAT, ABI_PHYSICAL_ROWS };
static const RxOpAbiStep abi_physical_string[] = { ABI_RESULT_STRING, ABI_PHYSICAL_ROWS };
/* A nested signal return/unwind restores the live parent before free_frame(),
 * without the ordinary value-return setter. INTERNAL_PARENT
 * is an executable premise: raw-root copyback/external-argument destruction
 * remains unrepresented, and must not borrow this nested retirement row. */
static const RxOpAbiStep abi_nested_unwind[] = { ABI_PHYSICAL_ROWS };
/* Subprogram of an already selected RETIRE_FRAME/free_frame. Private-child
 * presence is captured before handler/policy cleanup. FRAME_OWNED_BACKING is
 * the owned local prefix plus private a0, following unlinked attribute storage
 * to full capacity. clear_frame's full value/plugin destruction is separate. */
static const RxOpAbiStep abi_frame_retire[] = {
    ABI_STEP(CLEAR_FRAME_HANDLERS, RETIRE_HANDLERS, FRAME_HANDLERS, ROLE_NONE, EXTENT_NONE, EXTENT_NONE, RETIRING_FRAME, ONCE),
    ABI_STEP(RELEASE_FRAME_POLICY, RETIRE_POLICY, FRAME_PRIVATE_POLICY, ROLE_NONE, EXTENT_NONE, EXTENT_NONE, RETIRING_FRAME, ONCE),
    ABI_STEP(RESTORE_NATIVE_POLICY, RETIRE_NATIVE_POLICY, PARENT_POLICY, ACTIVE_POLICY, EXTENT_NONE, EXTENT_NONE, RETIRING_PRIVATE_CHILD, ONCE),
    ABI_STEP(RELEASE_FRAME_REFERENCES, RETIRE_REFERENCES, FRAME_REFERENCES, ROLE_NONE, EXTENT_NONE, EXTENT_NONE, FRAME_REFERENCES_MARKED, ONCE),
    ABI_STEP(ABANDON_FRAME_INVOCATION, RETIRE_INVOCATION_ABANDON, FRAME_INVOCATION, FRAME_INVOCATION, FULL_CAPACITY, FULL_CAPACITY, FRAME_INVOCATION_PRESENT, ONCE),
    ABI_STEP(UNPIN_FRAME_INVOCATION, RETIRE_INVOCATION_UNPIN, FRAME_INVOCATION, ROLE_NONE, FULL_CAPACITY, EXTENT_NONE, FRAME_INVOCATION_PRESENT, ONCE),
    ABI_STEP(FINALIZE_FRAME_PAYLOADS, RETIRE_PAYLOADS, FRAME_OWNED_BACKING, ROLE_NONE, FULL_CAPACITY, EXTENT_NONE, RETIRING_FRAME, ONCE),
    ABI_STEP(RECYCLE_FRAME, RETIRE_RECYCLE, PHYSICAL_FRAME, PROCEDURE_RECYCLER, EXTENT_NONE, EXTENT_NONE, RETIRING_FRAME, ONCE)
};
/* Root RET runs result hooks while active, then deactivates before copyback.
 * Copyback may run hooks, so the later free count is a NEW base-a0 snapshot.
 * These conditional programs cover CLI argv and external-copied entry without
 * claiming either endpoint shape, a clean root or a harmless observer. */
#define ABI_ROOT_DEACTIVATE \
    ABI_STEP(DEACTIVATE_ROOT, ROOT_DEACTIVATE, PHYSICAL_ROOT, ROLE_NONE, EXTENT_NONE, EXTENT_NONE, ROOT_PHYSICAL_RETURN, ONCE)
#define ABI_ROOT_COPYBACK \
    ABI_STEP(CAPTURE_ROOT_COUNT, ROOT_COPYBACK_COUNT, ROOT_ORIGINAL_A0, ROOT_COPYBACK_COUNT_VALUE, TOP, TOP, ROOT_EXTERNAL_COPYBACK, ONCE), \
    ABI_STEP(COPY_ROOT_ARGUMENT, ROOT_COPYBACK, ROOT_ACTIVE_ARGUMENT, EXTERNAL_ARGUMENT, ACTIVE_TREE, FULL_CAPACITY, ROOT_EXTERNAL_COPYBACK, COPYBACK_ARGUMENTS)
#define ABI_ROOT_FREE \
    ABI_STEP(CAPTURE_ROOT_COUNT, ROOT_FREE_COUNT, ROOT_ORIGINAL_A0, ROOT_FREE_COUNT_VALUE, TOP, TOP, ROOT_PHYSICAL_RETURN, ONCE), \
    ABI_STEP(FREE_ROOT_ARGUMENT, ROOT_ARGUMENT_FREE, ROOT_ORIGINAL_ARGUMENT, ROLE_NONE, FULL_CAPACITY, EXTENT_NONE, ROOT_PHYSICAL_RETURN, ROOT_FREE_ARGUMENTS)
#define ABI_ROOT_RETIRE \
    ABI_STEP(RETIRE_FRAME, PHYSICAL_RETIRE, PHYSICAL_ROOT, ROLE_NONE, EXTENT_NONE, EXTENT_NONE, ROOT_PHYSICAL_RETURN, ONCE), \
    ABI_STEP(FORGET_ROOT_ARGV_OWNER, ROOT_ARGV_FORGET, CLI_ARGV_OWNER, ROLE_NONE, EXTENT_NONE, EXTENT_NONE, ROOT_PHYSICAL_RETURN, ONCE)
#define ABI_ROOT_ZERO_STATUS \
    ABI_STEP(PUBLISH_ROOT_STATUS, ROOT_STATUS_PUBLISH, ROLE_NONE, EXECUTION_STATUS, EXTENT_NONE, TOP, ROOT_STATUS_ZERO, ONCE)
static const RxOpAbiStep abi_root_void[] = {
    ABI_ROOT_DEACTIVATE, ABI_ROOT_COPYBACK, ABI_ROOT_FREE,
    ABI_ROOT_ZERO_STATUS, ABI_ROOT_RETIRE
};
static const RxOpAbiStep abi_root_register[] = {
    ABI_STEP(CAPTURE_ROOT_STATUS, ROOT_STATUS_CAPTURE, RETURN_ACTIVE, ROOT_SAVED_RETURN_STATUS, TOP, TOP, ROOT_PHYSICAL_RETURN, ONCE),
    ABI_STEP(RETURN_REGISTER, RETURN_RESULT, RETURN_ACTIVE, ROOT_RETURN_RECEIVER, ACTIVE_TREE, FULL_CAPACITY, ROOT_RESULT_PRESENT, ONCE),
    ABI_ROOT_DEACTIVATE,
    ABI_STEP(PUBLISH_ROOT_STATUS, ROOT_STATUS_PUBLISH, ROOT_SELECTED_RETURN_STATUS, EXECUTION_STATUS, TOP, TOP, ROOT_STATUS_RECEIVER_OR_SAVED, ONCE),
    ABI_ROOT_COPYBACK, ABI_ROOT_FREE, ABI_ROOT_RETIRE
};
static const RxOpAbiStep abi_root_integer[] = {
    ABI_STEP(RETURN_INTEGER, RETURN_RESULT, RETURN_LITERAL, ROOT_RETURN_RECEIVER, EXTENT_NONE, EXTENT_NONE, ROOT_RESULT_PRESENT, ONCE),
    ABI_ROOT_DEACTIVATE, ABI_ROOT_COPYBACK, ABI_ROOT_FREE,
    ABI_STEP(PUBLISH_ROOT_STATUS, ROOT_STATUS_PUBLISH, RETURN_LITERAL, EXECUTION_STATUS, EXTENT_NONE, TOP, ROOT_PHYSICAL_RETURN, ONCE),
    ABI_ROOT_RETIRE
};
static const RxOpAbiStep abi_root_float[] = {
    ABI_STEP(RETURN_FLOAT, RETURN_RESULT, RETURN_LITERAL, ROOT_RETURN_RECEIVER, EXTENT_NONE, EXTENT_NONE, ROOT_RESULT_PRESENT, ONCE),
    ABI_ROOT_DEACTIVATE, ABI_ROOT_COPYBACK, ABI_ROOT_FREE,
    ABI_ROOT_ZERO_STATUS, ABI_ROOT_RETIRE
};
static const RxOpAbiStep abi_root_string[] = {
    ABI_STEP(RETURN_STRING, RETURN_RESULT, RETURN_LITERAL, ROOT_RETURN_RECEIVER, EXTENT_NONE, TOP, ROOT_RESULT_PRESENT, ONCE),
    ABI_ROOT_DEACTIVATE, ABI_ROOT_COPYBACK, ABI_ROOT_FREE, ABI_ROOT_RETIRE
};
/* EXIT/HALT preserves its existing status/event. Only external-copied roots
 * free their argument copies here; CLI argv ownership survives to finish.
 * Each abnormal free clears its two slot pointers before the next argument.
 * This runs with root still active, unlike normal root RET. */
static const RxOpAbiStep abi_root_unwind[] = {
    ABI_STEP(FREE_ROOT_ARGUMENT_UNBIND, ROOT_ARGUMENT_FREE, ROOT_ORIGINAL_ARGUMENT, ROLE_NONE, FULL_CAPACITY, EXTENT_NONE, ROOT_EXTERNAL_COPIED, SAVED_ROOT_ARGUMENTS),
    ABI_STEP(RETIRE_FRAME, PHYSICAL_RETIRE, PHYSICAL_ROOT, ROLE_NONE, EXTENT_NONE, EXTENT_NONE, ROOT_FRAME, ONCE),
    ABI_STEP(DEACTIVATE_ROOT, ROOT_DEACTIVATE, PHYSICAL_ROOT, ROLE_NONE, EXTENT_NONE, EXTENT_NONE, ROOT_FRAME, ONCE)
};
/* One helper boundary per native phase. CLEAR_FRAME_CONTROL includes pending
 * invocation/handler/private-policy cleanup; RELEASE_FRAME_PLUGINS
 * follows owned storage destruction, decimal then Unicode when loaded here.
 * The repeated block is module/procedure/LIFO, not clear-all then release-all.
 * Runtime storage/control tails remain observable, not empty/no-hook rows. */
static const RxOpAbiStep abi_execution_finish[] = {
    ABI_STEP(CLEANUP_EXECUTION_SIGNALS, FINISH_SIGNALS, EXECUTION_CONTROL, ROLE_NONE, EXTENT_NONE, EXTENT_NONE, EXECUTION_FINISHED, ONCE),
    ABI_STEP(BEGIN_RECYCLED_FRAME, FINISH_RECYCLER, CANONICAL_FRAME_RECYCLERS, RECYCLED_FRAME, EXTENT_NONE, EXTENT_NONE, CANONICAL_RECYCLER_OWNER, RECYCLED_BEGIN),
    ABI_STEP(CLEAR_FRAME_CONTROL, FINISH_FRAME_CONTROL, RECYCLED_FRAME, ROLE_NONE, EXTENT_NONE, EXTENT_NONE, RECYCLED_FRAME_PRESENT, ONCE),
    ABI_STEP(DESTROY_STORAGE, FINISH_OWNED_CLEAR, ROLE_NONE, RECYCLED_OWNED_BACKING, EXTENT_NONE, FULL_CAPACITY, RECYCLED_FRAME_PRESENT, EACH_RECYCLED_OWNED),
    ABI_STEP(DESTROY_STORAGE, FINISH_A0_CLEAR, ROLE_NONE, RECYCLED_A0_BACKING, EXTENT_NONE, FULL_CAPACITY, RECYCLED_FRAME_PRESENT, ONCE),
    ABI_STEP(RELEASE_FRAME_PLUGINS, FINISH_PLUGIN_RELEASE, RECYCLED_FRAME, ROLE_NONE, EXTENT_NONE, EXTENT_NONE, RECYCLED_FRAME_PRESENT, ONCE),
    ABI_STEP(RELEASE_RECYCLED_FRAME, FINISH_FRAME_RELEASE, RECYCLED_FRAME, ROLE_NONE, EXTENT_NONE, EXTENT_NONE, RECYCLED_FRAME_PRESENT, RECYCLED_END),
    ABI_STEP(FINISH_RUNTIME_STORAGE, FINISH_STORAGE, EXECUTION_OWNED_STORAGE, ROLE_NONE, FULL_CAPACITY, EXTENT_NONE, EXECUTION_FINISHED, ONCE),
    ABI_STEP(DESTROY_STORAGE, FINISH_STORAGE, ROLE_NONE, CLI_ARGV_OWNER, EXTENT_NONE, FULL_CAPACITY, ROOT_CLI_ARGV, ONCE),
    ABI_STEP(FINISH_RUNTIME_CONTROL, FINISH_CONTROL, EXECUTION_CONTROL, ROLE_NONE, EXTENT_NONE, EXTENT_NONE, EXECUTION_FINISHED, ONCE)
};
#undef ABI_ROOT_ZERO_STATUS
#undef ABI_ROOT_RETIRE
#undef ABI_ROOT_FREE
#undef ABI_ROOT_COPYBACK
#undef ABI_ROOT_DEACTIVATE
#undef ABI_RESULT_STRING
#undef ABI_RESULT_FLOAT
#undef ABI_RESULT_INTEGER
#undef ABI_RESULT_REGISTER
#undef ABI_PHYSICAL_ROWS
#undef ABI_STEP

RxOpAbiContract rxop_abi_contract(int opcode, RxOpAbiMode mode) {
    RxOpAbiContract result = { RXOP_ABI_UNKNOWN, NULL, 0u };
    int kind;
    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS ||
        (unsigned int)mode >= RXOP_ABI_MODE_COUNT) return result;
    if (rxop_effects(opcode).state != RXOP_EFFECT_CLASSIFIED) return result;
    result.state = RXOP_ABI_NONE;
#define ABI_CONTRACT(TABLE) do { result.state = RXOP_ABI_KNOWN; result.steps = TABLE; result.count = sizeof(TABLE) / sizeof((TABLE)[0]); return result; } while (0)
    if (mode == RXOP_ABI_ROOT_UNWIND) ABI_CONTRACT(abi_root_unwind);
    if (mode == RXOP_ABI_EXECUTION_FINISH) ABI_CONTRACT(abi_execution_finish);
    if (mode == RXOP_ABI_FRAME_RETIRE) ABI_CONTRACT(abi_frame_retire);
    if (mode == RXOP_ABI_FRAME_UNWIND) ABI_CONTRACT(abi_nested_unwind);
    /* Retained schema modes have no executable scope program after retirement. */
    if (mode == RXOP_ABI_SCOPE_LEAVE || mode == RXOP_ABI_SCOPED_RETURN ||
            mode == RXOP_ABI_ENTRY) return result;
    kind = rxop_return_kind(opcode);
    if (!kind) return result;
    if (mode == RXOP_ABI_ROOT_RETURN) {
        switch (kind) {
            case 1: ABI_CONTRACT(abi_root_void);
            case 2: ABI_CONTRACT(abi_root_register);
            case 3: ABI_CONTRACT(abi_root_integer);
            case 4: ABI_CONTRACT(abi_root_float);
            case 5: ABI_CONTRACT(abi_root_string);
            default: result.state = RXOP_ABI_UNKNOWN; return result;
        }
    }
    switch (kind) {
        case 1: ABI_CONTRACT(abi_physical_void);
        case 2: ABI_CONTRACT(abi_physical_register);
        case 3: ABI_CONTRACT(abi_physical_integer);
        case 4: ABI_CONTRACT(abi_physical_float);
        case 5: ABI_CONTRACT(abi_physical_string);
        default: result.state = RXOP_ABI_UNKNOWN; return result;
    }
#undef ABI_CONTRACT
}

const char *rxop_abi_state_name(RxOpAbiState value) {
    static const char *const names[] = { "none", "known", "unknown" };
    return (unsigned int)value < sizeof(names) / sizeof(names[0]) ? names[value] : "unknown";
}
const char *rxop_abi_mode_name(RxOpAbiMode value) {
    static const char *const names[] = {
        "entry", "physical-return", "scoped-return", "scope-leave", "frame-unwind",
        "root-return", "reserved-root-sigreturn", "root-unwind", "execution-finish", "frame-retire"
    };
    return (unsigned int)value < sizeof(names) / sizeof(names[0]) ? names[value] : "unknown";
}
const char *rxop_abi_operation_name(RxOpAbiOperation value) {
    static const char *const names[] = {
        "capture-pointers", "clear-storage", "publish-scope", "set-integer", "bind-proxy",
        "return-register", "return-integer", "return-float", "return-string",
        "restore-policy", "restore-slot", "restore-numeric", "pop-scope", "retire-frame",
        "deactivate-root", "capture-root-count", "copy-root-argument", "free-root-argument",
        "free-root-argument-unbind", "forget-root-argv-owner", "capture-root-status",
        "publish-root-status", "cleanup-execution-signals", "begin-recycled-frame",
        "clear-frame-control", "destroy-storage", "release-frame-plugins",
        "release-recycled-frame", "finish-runtime-storage", "finish-runtime-control", "reset-storage",
        "leave-frame-scopes", "clear-frame-handlers", "release-frame-policy",
        "release-frame-references", "abandon-frame-invocation", "unpin-frame-invocation", "recycle-frame",
        "finalize-frame-payloads", "restore-native-policy"
    };
    return (unsigned int)value < sizeof(names) / sizeof(names[0]) ? names[value] : "unknown";
}
const char *rxop_abi_checkpoint_name(RxOpAbiCheckpoint value) {
    static const char *const names[] = {
        "entry-after-prep-before-init", "entry-before-commit", "entry-publish", "entry-a0",
        "entry-proxies", "return-before-result", "leave-policy", "leave-owned-clear",
        "leave-owned-restore", "leave-numeric", "leave-pop", "physical-retire", "physical-numeric",
        "root-deactivate", "root-copyback-count", "root-copyback", "root-free-count",
        "root-argument-free", "root-argv-forget", "root-status-capture", "root-status-publish",
        "finish-signals", "finish-recycler", "finish-frame-control", "finish-owned-clear",
        "finish-a0-clear", "finish-plugin-release", "finish-frame-release", "finish-storage",
        "finish-control", "root-signal-clear", "retire-scopes", "retire-handlers", "retire-policy",
        "retire-references", "retire-invocation-abandon", "retire-invocation-unpin", "retire-recycle",
        "retire-payloads", "retire-native-policy"
    };
    return (unsigned int)value < sizeof(names) / sizeof(names[0]) ? names[value] : "unknown";
}
const char *rxop_abi_role_name(RxOpAbiRole value) {
    static const char *const names[] = {
        "none", "recipe-captures", "scope-captures", "recipe-original-backing", "scope-context",
        "previous-scope", "captured-count", "a0-original-backing", "captured-argument",
        "recipe-proxy-slot", "return-active", "return-literal", "captured-result", "saved-policy",
        "active-policy", "scope-original-backing", "saved-slot", "owned-slot", "saved-numeric",
        "active-numeric", "physical-frame", "parent-numeric", "physical-root", "root-return-receiver",
        "root-active-argument", "root-original-argument", "root-original-a0", "root-copyback-count",
        "root-free-count", "root-saved-number-args", "external-argument", "external-argument-count",
        "cli-argv-owner", "root-saved-return-status", "root-selected-return-status",
        "execution-status", "canonical-frame-recyclers", "recycled-frame", "recycled-owned-backing",
        "recycled-a0-backing", "execution-owned-storage", "execution-control", "root-signal-diagnostic",
        "frame-scopes", "frame-handlers", "frame-private-policy", "frame-references", "frame-invocation",
        "procedure-recycler", "frame-owned-backing", "parent-policy"
    };
    return (unsigned int)value < sizeof(names) / sizeof(names[0]) ? names[value] : "unknown";
}
const char *rxop_abi_extent_name(RxOpAbiExtent value) {
    static const char *const names[] = { "none", "top", "active-tree", "full-capacity" };
    return (unsigned int)value < sizeof(names) / sizeof(names[0]) ? names[value] : "unknown";
}
const char *rxop_abi_rule_name(RxOpAbiRule value) {
    static const char *const names[] = {
        "successful-entry", "committed-scope", "result-present", "owned-base-move-else-copy",
        "internal-parent", "argument-present", "validated-stage", "root-frame",
        "root-physical-return", "root-external-copyback", "root-external-copied", "root-cli-argv",
        "root-result-present", "root-owned-base-move-else-copy", "root-status-receiver-or-saved",
        "root-status-zero", "execution-finished", "canonical-recycler-owner", "recycled-frame-present",
        "retiring-frame", "frame-references-marked", "frame-invocation-present",
        "retiring-private-child"
    };
    return (unsigned int)value < sizeof(names) / sizeof(names[0]) ? names[value] : "unknown";
}
const char *rxop_abi_repeat_name(RxOpAbiRepeat value) {
    static const char *const names[] = {
        "once", "each-owned", "owned-begin", "owned-end", "each-proxy", "copyback-arguments",
        "root-free-arguments", "saved-root-arguments", "recycled-begin", "each-recycled-owned",
        "recycled-end"
    };
    return (unsigned int)value < sizeof(names) / sizeof(names[0]) ? names[value] : "unknown";
}

static int callback_binding_only(int opcode) {
    const char *recipe = rxop_binding_recipe(opcode);
    if (!recipe || !*recipe) return 0;
    /* These reviewed recipe primitives change pointers or raw scalar/status
     * fields without value copy/release hooks. Resize, setters, references and
     * lifetime verbs deliberately remain unproved. Whole-op routes are unioned
     * separately: a pointer-only call prefix never certifies its invocation. */
    while (*recipe) {
        const char *end = strchr(recipe, ';');
        const char *colon = strchr(recipe, ':');
        size_t n;
        if (!end) end = recipe + strlen(recipe);
        if (!colon || colon >= end) return 0;
        n = (size_t)(colon - recipe);
        if (!((n == 4u && !memcmp(recipe, "link", n)) ||
              (n == 6u && !memcmp(recipe, "unlink", n)) ||
              (n == 4u && !memcmp(recipe, "swap", n)) ||
              (n == 9u && !memcmp(recipe, "linkindex", n)) ||
              (n == 12u && !memcmp(recipe, "linkindexreg", n)) ||
              (n == 8u && !memcmp(recipe, "argument", n)) ||
              (n == 11u && !memcmp(recipe, "argumentadd", n)) ||
              (n == 16u && !memcmp(recipe, "capturedargument", n)) ||
              (n == 15u && !memcmp(recipe, "qualifiedglobal", n)) ||
              (n == 6u && !memcmp(recipe, "branch", n)) ||
              (n == 5u && !memcmp(recipe, "flags", n)) ||
              (n == 5u && !memcmp(recipe, "icopy", n)) ||
              (n == 10u && !memcmp(recipe, "captureint", n)) ||
              (n == 6u && !memcmp(recipe, "putint", n)) ||
              (n == 9u && !memcmp(recipe, "attribute", n)) ||
              (n == 10u && !memcmp(recipe, "attribute1", n)) ||
              (n == 15u && !memcmp(recipe, "linktoattribute", n)) ||
              (n == 16u && !memcmp(recipe, "linktoattribute1", n)) ||
              (n == 15u && !memcmp(recipe, "unlinkattribute", n)) ||
              (n == 16u && !memcmp(recipe, "unlinkattribute1", n)))) return 0;
        recipe = *end ? end + 1 : end;
    }
    return 1;
}

unsigned int rxop_callback_inputs(int opcode, size_t operand_index) {
    unsigned int source = rxop_transport_source_operand(opcode);
    if (callback_binding_only(opcode)) return RXOP_COMPONENT_NONE;
    /* MOVE transfers ownership and never invokes the source copy hook. */
    if (opcode == OP_MOVE_REG_REG) return RXOP_COMPONENT_NONE;
    if (source && operand_index + 1u == source) {
        unsigned int components = rxop_transport_components(opcode);
        /* BCOPY carries native binary ownership through copy_binary_value,
         * even though its ordinary component transport is named BINARY. */
        if (components & RXOP_COMPONENT_BINARY) components |= RXOP_COMPONENT_NATIVE_PAYLOAD;
        return components & (RXOP_COMPONENT_NATIVE_PAYLOAD | RXOP_COMPONENT_ATTRIBUTE_COUNT);
    }
    return RXOP_COMPONENT_NONE;
}

unsigned int rxop_callback_outputs(int opcode, size_t operand_index) {
    /* Destination release runs cleanup-only finalizers. This historical
     * callback route is empty; component/ownership effects still describe
     * replacement, lifetime expiry and resource release. Missing intrinsic
     * inventories remain UNKNOWN in rxop_callback_obligations. */
    (void)opcode;
    (void)operand_index;
    return RXOP_COMPONENT_NONE;
}

int rxop_callback_capacity(int opcode, size_t operand_index) {
    /* Inactive capacity can retain resources, never a Rexx callback solely
     * through conforming finalization. Source COPY hooks remain inputs. */
    (void)opcode;
    (void)operand_index;
    return 0;
}

unsigned int rxop_callback_obligations(int opcode) {
    RxOpEffects effects = rxop_effects(opcode);
    unsigned int result = RXOP_CALLBACK_NONE;
    size_t operand, count;
    int pure_binding;
    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS)
        return RXOP_CALLBACK_UNKNOWN;
    /* A call prefix may itself contain only pointer verbs. That does not
     * describe the invocation or its hidden result/cleanup work. Exempt only
     * a complete pointer-only operation, never the whole-op callback routes. */
    pure_binding = callback_binding_only(opcode) && !rxop_call_invokes(opcode) &&
            !rxop_selection_callbacks(opcode) && !rxop_host_callbacks(opcode) &&
            !rxop_abi_callbacks(opcode) && !rxop_departure_callbacks(opcode);
    if ((!effects.callback_inventory_complete && !pure_binding) ||
        effects.state != RXOP_EFFECT_CLASSIFIED)
        result |= RXOP_CALLBACK_UNKNOWN;
    count = rxop_format_operand_count(op_table[opcode].format);
    for (operand = 0; operand < count; operand++) {
        if (rxop_callback_inputs(opcode, operand)) result |= RXOP_CALLBACK_INPUT;
        if (rxop_callback_outputs(opcode, operand)) result |= RXOP_CALLBACK_OUTPUT;
        if (rxop_callback_capacity(opcode, operand)) result |= RXOP_CALLBACK_CAPACITY;
    }
    if (rxop_selection_callbacks(opcode)) result |= RXOP_CALLBACK_SELECTION;
    if (rxop_host_callbacks(opcode)) result |= RXOP_CALLBACK_HOST;
    if (rxop_context_sync_callbacks(opcode)) result |= RXOP_CALLBACK_PLUGIN_SYNC;
    if (rxop_abi_callbacks(opcode)) result |= RXOP_CALLBACK_ABI;
    if (rxop_departure_callbacks(opcode)) result |= RXOP_CALLBACK_DEPARTURE;
    if (rxop_call_invokes(opcode)) result |= RXOP_CALLBACK_INVOCATION;
    return result;
}

int rxop_callback_normal_preserves_frame(int opcode) {
    unsigned int obligations = rxop_callback_obligations(opcode);
    unsigned int original = obligations;
    unsigned int resources = RXOP_CALLBACK_INPUT | RXOP_CALLBACK_OUTPUT | RXOP_CALLBACK_CAPACITY;
    if ((obligations & RXOP_CALLBACK_HOST) &&
        (rxop_host_callback_contract(opcode) & RXOP_HOST_CALLBACK_PRESERVES_FRAME))
        obligations &= ~RXOP_CALLBACK_HOST;
    return original != RXOP_CALLBACK_NONE && !(obligations & ~resources);
}

RxOpCallbackValueSurface rxop_callback_value_surface(int opcode) {
    unsigned int obligations = rxop_callback_obligations(opcode);
    if (obligations == RXOP_CALLBACK_NONE) return RXOP_CALLBACK_VALUES_NONE;
    /* Inspect the entire route, not just one HOST bit of a combined event.
     * Resource/selection/invocation/unknown routes do not inherit this fact. */
    if (obligations == RXOP_CALLBACK_HOST &&
            (rxop_host_callback_contract(opcode) & RXOP_HOST_CALLBACK_BYTES_SNAPSHOT))
        return RXOP_CALLBACK_VALUES_BYTES_SNAPSHOT;
    return RXOP_CALLBACK_VALUES_UNKNOWN;
}

const char *rxop_callback_value_surface_name(RxOpCallbackValueSurface value) {
    static const char *const names[] = { "none", "bytes-snapshot", "unknown" };
    return (unsigned int)value < sizeof(names) / sizeof(names[0]) ? names[value] : "unknown";
}

int rxop_callback_preserves_signal_policy(int opcode) {
    unsigned int obligations = rxop_callback_obligations(opcode);
    unsigned int resources = RXOP_CALLBACK_INPUT | RXOP_CALLBACK_OUTPUT | RXOP_CALLBACK_CAPACITY;
    if (!obligations || (obligations & RXOP_CALLBACK_UNKNOWN)) return 0;
    if (rxop_host_callback_contract(opcode) & RXOP_HOST_CALLBACK_PRESERVES_SIGNAL_POLICY)
        obligations &= ~RXOP_CALLBACK_HOST;
    return !(obligations & ~resources);
}

unsigned int rxop_callback_postwrite_operand(int opcode) {
    /* There is no cleanup-only Rexx callback to split a shared setter. */
    (void)opcode;
    return 0;
}

RxOpSignalContract rxop_execution_signal_contract(int opcode) {
    RxOpSignalContract result = rxop_signal_contract(opcode);
    if (!rxop_callback_obligations(opcode)) return result;
    /* Hooks may queue an interrupt through the public active-context API.
     * Intrinsic BEFORE_WRITES does not locate that hook. Keep names and the
     * checkpoint unknown; consumers must not resurrect pre-hook state. */
    result.state = RXOP_SIGNAL_STATE_UNKNOWN;
    result.phase = RXOP_SIGNAL_PHASE_UNKNOWN;
    result.source = RXOP_SIGNAL_SOURCE_UNKNOWN;
    result.source_operand = SIZE_MAX;
    result.static_names = NULL;
    result.failure_writes = RXOP_OP_ALL;
    result.failure_writes_signature = NULL;
    result.failure_component_writes = RXOP_COMPONENT_ALL;
    result.failure_context_writes = RXOP_CONTEXT_NUMERIC;
    result.dependencies |= RXOP_SIGNAL_DEP_UNKNOWN;
    result.continuations = RXOP_SIGNAL_CONT_ALL;
    result.properties &= ~RXOP_SIGNAL_PROP_SUCCESS_STABLE;
    result.properties |= RXOP_SIGNAL_PROP_ADDRESS_OBSERVABLE |
                         RXOP_SIGNAL_PROP_PAYLOAD_OBSERVABLE |
                         RXOP_SIGNAL_PROP_POLICY_WRITE;
    result.policy_effect = RXOP_POLICY_EFFECT_UNKNOWN;
    result.policy_source = RXOP_SIGNAL_SOURCE_UNKNOWN;
    result.policy_source_operand = SIZE_MAX;
    result.policy_static_name = NULL;
    return result;
}

unsigned int rxop_call_target_components(int opcode) {
    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS) return RXOP_COMPONENT_ALL;
    return rxop_call_contract(opcode).kind == RXOP_CALL_DYNAMIC
            ? RXOP_COMPONENT_INTEGER : RXOP_COMPONENT_NONE;
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
        case RXOP_IMPLICIT_INLINE_RECIPE:
        case RXOP_IMPLICIT_QUALIFIED_GLOBAL: return NULL;
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
    (void)opcode;
    return 0u;
}

unsigned int rxop_recipe_operand(int opcode) {
    (void)opcode;
    return 0u;
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
    if (opcode == OP_SETOBJTYPE_REG_STRING || opcode == OP_SETOBJUNINIT_REG_STRING)
        return RXOP_COMPONENT_NONE;
    if (opcode == OP_ISINITIALIZED_REG_REG && operand_index == 1)
        return RXOP_COMPONENT_NONE;
    /* The payload is copied whole into pending signal storage. Validation
     * observes the separate origin structure, not unused payload components. */
    if (opcode == OP_SIGNALORIGIN_STRING_REG_REG && operand_index == 1)
        return RXOP_COMPONENT_NONE;
    if (rxop_selection_contract(opcode).kind != RXOP_SELECTION_NONE)
        return rxop_component_reads(opcode, operand_index);
    if (rxop_call_invokes(opcode)) {
        if (operand_index + 1 == call.count_operand) return RXOP_COMPONENT_INTEGER;
        if (operand_index + 1 == call.target_operand)
            return rxop_call_target_components(opcode);
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
    RxOpSelectionContract selection = rxop_selection_contract(opcode);
    if (selection.receiver_operand && operand_index + 1 == selection.receiver_operand)
        return RXFLAG_VM_OBJECT_UNINITIALIZED;
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
    if (rxop_component_write_is_known(opcode, operand_index) &&
        rxop_component_writes(opcode, operand_index) == RXOP_COMPONENT_NONE &&
        rxop_component_clears(opcode, operand_index) == RXOP_COMPONENT_NONE)
        return RXOP_COMPONENT_NONE;
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
    if (operand_index == 0 && (opcode == OP_SETOBJTYPE_REG_STRING ||
        opcode == OP_SETOBJUNINIT_REG_STRING)) return RXOP_COMPONENT_OBJECT_TYPE;
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
