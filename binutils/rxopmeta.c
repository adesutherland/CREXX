/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxdefs.h"

RxOpEffects rxop_effects(int opcode) {
    RxOpEffects effects;

    effects.flags = RXOP_EFFECT_NONE;
    effects.implicit = RXOP_IMPLICIT_NONE;

    switch (opcode) {
#define RXOP_EFFECT(NAME, FLAGS, IMPLICIT) \
        case OP_##NAME: \
            effects.flags = (FLAGS); \
            effects.implicit = (IMPLICIT); \
            break;
#include "rxopeffects.h"
#undef RXOP_EFFECT
        default:
            break;
    }

    return effects;
}
