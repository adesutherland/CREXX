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
    effects.semantics = RXOP_SEM_MAY_THROW | RXOP_SEM_OPAQUE;
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
