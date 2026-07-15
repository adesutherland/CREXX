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
    RxOpImplicitEffect implicit;
    unsigned int semantics;
} RxOpEffectSpec;

#define RXE(STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS) \
    STATE, READS, WRITES, KILLS, BRANCH_TARGETS, IMPLICIT, SEMANTICS
#define RXOP_EFFECT(NAME, EFFECTS) { OP_##NAME, EFFECTS },
static const RxOpEffectSpec rxop_effect_specs[] = {
#include "rxopeffects.h"
};
#undef RXOP_EFFECT
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
    effects.implicit = RXOP_IMPLICIT_NONE;
    effects.semantics = RXOP_SEM_MAY_THROW | RXOP_SEM_OPAQUE;
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
    effects.implicit = spec->implicit;
    effects.semantics = spec->semantics;
    effects.flow = op_table[opcode].flow;
    effects.optimizer_barrier =
        spec->state != RXOP_EFFECT_CLASSIFIED ||
        (op_table[opcode].flags & FLG_OPT_BARRIER) != 0;

    return effects;
}

size_t rxop_effect_count(void) {
    return sizeof(rxop_effect_specs) / sizeof(rxop_effect_specs[0]);
}
