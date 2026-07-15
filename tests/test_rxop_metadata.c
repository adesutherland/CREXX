/*
 * Generated-database consistency checks for the canonical opcode effects
 * inventory and its RXAS optimizer consumers.
 */

#include <stdio.h>
#include <string.h>

#include "rxdefs.h"

static int failures;

static void check(int condition, const char *message, const OpInfo *op) {
    if (condition) return;
    if (op) fprintf(stderr, "%s: %s (%d)\n", message, op->mnemonic, op->opcode);
    else fprintf(stderr, "%s\n", message);
    failures++;
}

static unsigned int format_register_mask(OpFormat format) {
    switch (format) {
        case FMT_I_I_R:
        case FMT_S_S_R:
            return RXOP_OP_3;
        case FMT_I_R:
        case FMT_L_R:
        case FMT_L_R_I:
        case FMT_L_R_S:
        case FMT_S_R:
            return RXOP_OP_2;
        case FMT_I_R_R:
        case FMT_L_R_R:
            return RXOP_OP_23;
        case FMT_L_L_R:
            return RXOP_OP_3;
        case FMT_R:
        case FMT_R_B:
        case FMT_R_B_B:
        case FMT_R_B_S:
        case FMT_R_C:
        case FMT_R_D:
        case FMT_R_F:
        case FMT_R_F_I:
        case FMT_R_I:
        case FMT_R_I_I:
        case FMT_R_P:
        case FMT_R_S:
        case FMT_R_S_I:
        case FMT_R_S_S:
            return RXOP_OP_1;
        case FMT_R_B_R:
        case FMT_R_D_R:
        case FMT_R_F_R:
        case FMT_R_I_R:
        case FMT_R_P_R:
        case FMT_R_S_R:
            return RXOP_OP_13;
        case FMT_R_R:
        case FMT_R_R_B:
        case FMT_R_R_D:
        case FMT_R_R_F:
        case FMT_R_R_I:
        case FMT_R_R_S:
            return RXOP_OP_12;
        case FMT_R_R_R:
            return RXOP_OP_ALL;
        case FMT_EMPTY:
        case FMT_B:
        case FMT_C:
        case FMT_F:
        case FMT_I:
        case FMT_I_I:
        case FMT_I_I_I:
        case FMT_L:
        case FMT_L_P_S:
        case FMT_L_S:
        case FMT_P:
        case FMT_P_S:
        case FMT_S:
        case FMT_S_S:
            return RXOP_OP_NONE;
        default:
            return RXOP_OP_ALL;
    }
}

static unsigned int format_label_mask(OpFormat format) {
    switch (format) {
        case FMT_L_L_R:
            return RXOP_OP_12;
        case FMT_L:
        case FMT_L_P_S:
        case FMT_L_R:
        case FMT_L_R_I:
        case FMT_L_R_R:
        case FMT_L_R_S:
        case FMT_L_S:
            return RXOP_OP_1;
        default:
            return RXOP_OP_NONE;
    }
}

static int is_internal_opcode(int opcode) {
    return opcode == OP_INULL || opcode == OP_INTERRUPT || opcode == OP_IUNKNOWN;
}

static void check_unknown_effects(int opcode) {
    RxOpEffects effects;

    effects = rxop_effects(opcode);
    check(effects.opcode == opcode, "unknown effect preserves queried opcode", NULL);
    check(effects.state == RXOP_EFFECT_CONSERVATIVE,
          "unknown effect must be conservative", NULL);
    check(effects.reads == RXOP_OP_ALL && effects.writes == RXOP_OP_ALL,
          "unknown effect must expose worst-case explicit access", NULL);
    check(effects.kills == RXOP_OP_NONE,
          "unknown effect must not claim a kill", NULL);
    check(effects.branch_targets == RXOP_OP_ALL,
          "unknown effect must expose worst-case branch operands", NULL);
    check(effects.flow == FLOW_TERM && effects.optimizer_barrier,
          "unknown effect must stop flow and optimization", NULL);
    check((effects.semantics & (RXOP_SEM_MAY_THROW | RXOP_SEM_OPAQUE)) ==
              (RXOP_SEM_MAY_THROW | RXOP_SEM_OPAQUE),
          "unknown effect must expose exceptional opaque behavior", NULL);
}

int main(void) {
    int i;
    int source_count;
    int classified_count;
    int conservative_count;
    int reserved_count;
    int internal_count;
    unsigned int legal_registers;
    unsigned int legal_labels;
    unsigned int legal_semantics;
    RxOpEffects effects;
    const OpInfo *op;

    failures = 0;
    source_count = 0;
    classified_count = 0;
    conservative_count = 0;
    reserved_count = 0;
    internal_count = 0;
    legal_semantics = RXOP_SEM_MAY_THROW | RXOP_SEM_CALL |
        RXOP_SEM_DYNAMIC_CALL | RXOP_SEM_RETURN | RXOP_SEM_ALIAS_CREATE |
        RXOP_SEM_ALIAS_RELEASE | RXOP_SEM_REFERENCE_CREATE |
        RXOP_SEM_REFERENCE_READ | RXOP_SEM_REFERENCE_WRITE |
        RXOP_SEM_REFERENCE_RELEASE | RXOP_SEM_LIFETIME_END |
        RXOP_SEM_INDIRECT_WRITE | RXOP_SEM_INDIRECT_BRANCH |
        RXOP_SEM_OPAQUE;

    for (i = 0; op_table[i].mnemonic != NULL; i++) {
        op = &op_table[i];
        effects = rxop_effects(op->opcode);
        legal_registers = format_register_mask(op->format);
        legal_labels = format_label_mask(op->format);

        check(op->opcode == i, "opcode table is not dense and index-aligned", op);
        check(effects.opcode == op->opcode,
              "effects table is not dense and index-aligned", op);
        check(effects.flow == op->flow,
              "effect flow disagrees with canonical opcode flow", op);
        check((effects.reads & ~legal_registers) == 0,
              "read mask names a non-register operand", op);
        check((effects.writes & ~legal_registers) == 0,
              "write mask names a non-register operand", op);
        check((effects.kills & ~effects.writes) == 0,
              "kill mask is not a subset of writes", op);
        check((effects.kills & effects.reads) == 0,
              "definite kill also claims to read the same operand", op);
        check((effects.branch_targets & ~legal_labels) == 0,
              "branch-target mask names a non-label operand", op);
        check((effects.semantics & ~legal_semantics) == 0,
              "effect has unknown semantic flags", op);
        check(!(effects.semantics & RXOP_SEM_DYNAMIC_CALL) ||
                  (effects.semantics & RXOP_SEM_CALL),
              "dynamic call is not classified as a call", op);
        check(!(effects.semantics & RXOP_SEM_RETURN) || op->flow == FLOW_TERM,
              "return classification is not terminal", op);

        check(((op->flags & FLG_IMPLICIT_REG_USE) != 0) ==
                  (effects.implicit != RXOP_IMPLICIT_NONE),
              "implicit-register flag and effect metadata disagree", op);
        if (effects.implicit == RXOP_IMPLICIT_LOCAL_RANGE_AFTER_OP3) {
            check(op->format == FMT_R_P_R || op->format == FMT_R_R_R ||
                      op->format == FMT_R_S_R,
                  "runtime register range requires register operand 3", op);
            check(effects.optimizer_barrier,
                  "runtime register range must remain an optimizer barrier", op);
        }

        if (op->flow == FLOW_JUMP) {
            check(effects.branch_targets != RXOP_OP_NONE,
                  "unconditional jump lacks a branch target", op);
        }
        if (op->flow == FLOW_COND) {
            check(effects.branch_targets != RXOP_OP_NONE ||
                      (effects.semantics & RXOP_SEM_INDIRECT_BRANCH),
                  "conditional flow lacks explicit or indirect targets", op);
        }

        if (rxop_is_source_mnemonic(op->mnemonic)) {
            source_count++;
            check(effects.state == RXOP_EFFECT_CLASSIFIED ||
                      effects.state == RXOP_EFFECT_CONSERVATIVE,
                  "source opcode lacks classified/conservative effects", op);
        } else if (strncmp(op->mnemonic, "RESERVED_", 9) == 0) {
            check(effects.state == RXOP_EFFECT_RESERVED,
                  "reserved opcode lacks reserved effects state", op);
        } else if (is_internal_opcode(op->opcode)) {
            check(effects.state == RXOP_EFFECT_INTERNAL,
                  "internal opcode lacks internal effects state", op);
        } else {
            check(0, "non-source opcode has unknown treatment", op);
        }

        switch (effects.state) {
            case RXOP_EFFECT_CLASSIFIED:
                classified_count++;
                check(effects.optimizer_barrier ==
                          ((op->flags & FLG_OPT_BARRIER) != 0),
                      "classified optimizer barrier disagrees with opcode flags", op);
                break;
            case RXOP_EFFECT_CONSERVATIVE:
                conservative_count++;
                check(effects.optimizer_barrier && effects.kills == RXOP_OP_NONE,
                      "conservative effect must fail closed", op);
                break;
            case RXOP_EFFECT_RESERVED:
                reserved_count++;
                check(effects.optimizer_barrier && effects.kills == RXOP_OP_NONE,
                      "reserved effect must fail closed", op);
                break;
            case RXOP_EFFECT_INTERNAL:
                internal_count++;
                check(effects.optimizer_barrier && effects.kills == RXOP_OP_NONE,
                      "internal effect must fail closed", op);
                break;
            default:
                check(0, "effect has unknown state", op);
                break;
        }
    }

    check(i == OP_MAX_INSTRUCTIONS,
          "opcode table size does not match opcode enum", NULL);
    check(rxop_effect_count() == (size_t)OP_MAX_INSTRUCTIONS,
          "effects inventory size does not match opcode enum", NULL);
    check(source_count == classified_count + conservative_count,
          "source effects coverage count does not close", NULL);
    check(i == classified_count + conservative_count + reserved_count + internal_count,
          "effect-state totals do not close", NULL);

    /* Representative handler-level semantic regressions. */
    effects = rxop_effects(OP_LOAD_REG_REG);
    check(effects.reads == RXOP_OP_2 && effects.writes == RXOP_OP_1 &&
              effects.kills == RXOP_OP_1,
          "load/copy effects regression", &op_table[OP_LOAD_REG_REG]);
    effects = rxop_effects(OP_INC_REG);
    check(effects.reads == RXOP_OP_1 && effects.writes == RXOP_OP_1 &&
              effects.kills == RXOP_OP_NONE,
          "read-modify-write effects regression", &op_table[OP_INC_REG]);
    effects = rxop_effects(OP_IEQ_REG_REG_REG);
    check(effects.reads == RXOP_OP_23 && effects.writes == RXOP_OP_1 &&
              effects.kills == RXOP_OP_1,
          "comparison effects regression", &op_table[OP_IEQ_REG_REG_REG]);
    effects = rxop_effects(OP_BRT_ID_REG);
    check(effects.reads == RXOP_OP_2 && effects.branch_targets == RXOP_OP_1 &&
              effects.flow == FLOW_COND,
          "conditional branch effects regression", &op_table[OP_BRT_ID_REG]);
    effects = rxop_effects(OP_CALL_REG_FUNC_REG);
    check(effects.reads == RXOP_OP_3 && effects.writes == RXOP_OP_1 &&
              effects.kills == RXOP_OP_1 &&
              effects.implicit == RXOP_IMPLICIT_LOCAL_RANGE_AFTER_OP3 &&
              (effects.semantics & RXOP_SEM_CALL) && effects.optimizer_barrier,
          "call/range effects regression", &op_table[OP_CALL_REG_FUNC_REG]);
    effects = rxop_effects(OP_DCALL_REG_REG_REG);
    check((effects.semantics & (RXOP_SEM_CALL | RXOP_SEM_DYNAMIC_CALL)) ==
              (RXOP_SEM_CALL | RXOP_SEM_DYNAMIC_CALL),
          "dynamic-call effects regression", &op_table[OP_DCALL_REG_REG_REG]);
    effects = rxop_effects(OP_RET_REG);
    check(effects.reads == RXOP_OP_1 && effects.flow == FLOW_TERM &&
              (effects.semantics & RXOP_SEM_RETURN),
          "return effects regression", &op_table[OP_RET_REG]);
    effects = rxop_effects(OP_FEXTR_REG_REG_REG);
    check(effects.reads == RXOP_OP_3 && effects.writes == RXOP_OP_12 &&
              effects.kills == RXOP_OP_12,
          "multi-destination effects regression", &op_table[OP_FEXTR_REG_REG_REG]);
    effects = rxop_effects(OP_LINK_REG_REG);
    check(effects.reads == RXOP_OP_2 && effects.writes == RXOP_OP_1 &&
              (effects.semantics & RXOP_SEM_ALIAS_CREATE),
          "alias-create effects regression", &op_table[OP_LINK_REG_REG]);
    effects = rxop_effects(OP_DEREF_REG_REG);
    check(effects.reads == RXOP_OP_2 && effects.kills == RXOP_OP_1 &&
              (effects.semantics & RXOP_SEM_REFERENCE_READ),
          "reference-read effects regression", &op_table[OP_DEREF_REG_REG]);
    effects = rxop_effects(OP_NULL_REG);
    check(effects.reads == RXOP_OP_NONE && effects.kills == RXOP_OP_1,
          "NULL kill effects regression", &op_table[OP_NULL_REG]);
    effects = rxop_effects(OP_ENDLIFE_REG);
    check(effects.reads == RXOP_OP_1 && effects.writes == RXOP_OP_1 &&
              effects.kills == RXOP_OP_NONE &&
              (effects.semantics & (RXOP_SEM_REFERENCE_RELEASE |
                                     RXOP_SEM_LIFETIME_END)) ==
                  (RXOP_SEM_REFERENCE_RELEASE | RXOP_SEM_LIFETIME_END),
          "ENDLIFE lifetime effects regression", &op_table[OP_ENDLIFE_REG]);
    effects = rxop_effects(OP_TRANSCHAR_REG_REG_REG);
    check(effects.reads == RXOP_OP_ALL && effects.writes == RXOP_OP_1 &&
              effects.kills == RXOP_OP_NONE,
          "TRANSCHAR must read before writing operand 1",
          &op_table[OP_TRANSCHAR_REG_REG_REG]);
    effects = rxop_effects(OP_CONCCHAR_REG_REG_REG);
    check(effects.reads == RXOP_OP_ALL && effects.writes == RXOP_OP_1 &&
              effects.kills == RXOP_OP_NONE,
          "CONCCHAR must append through operand 1",
          &op_table[OP_CONCCHAR_REG_REG_REG]);
    effects = rxop_effects(OP_DROPCHAR_REG_REG_REG);
    check(effects.reads == RXOP_OP_ALL && effects.writes == RXOP_OP_1 &&
              effects.kills == RXOP_OP_NONE,
          "DROPCHAR must append through operand 1",
          &op_table[OP_DROPCHAR_REG_REG_REG]);
    effects = rxop_effects(OP_PADSTR_REG_REG_REG);
    check(effects.reads == RXOP_OP_ALL && effects.writes == RXOP_OP_1 &&
              effects.kills == RXOP_OP_NONE,
          "PADSTR must append through operand 1",
          &op_table[OP_PADSTR_REG_REG_REG]);
    effects = rxop_effects(OP_STRPOS_REG_REG_REG);
    check(effects.reads == RXOP_OP_ALL && effects.writes == RXOP_OP_1 &&
              effects.kills == RXOP_OP_NONE,
          "STRPOS must read its operand-1 start position",
          &op_table[OP_STRPOS_REG_REG_REG]);
    effects = rxop_effects(OP_TRIML_REG_REG);
    check(effects.reads == RXOP_OP_12 && effects.writes == RXOP_OP_1 &&
              effects.kills == RXOP_OP_NONE,
          "deprecated TRIML must read before mutating operand 1",
          &op_table[OP_TRIML_REG_REG]);
    effects = rxop_effects(OP_TRIMR_REG_REG);
    check(effects.reads == RXOP_OP_12 && effects.writes == RXOP_OP_1 &&
              effects.kills == RXOP_OP_NONE,
          "deprecated TRIMR must read before mutating operand 1",
          &op_table[OP_TRIMR_REG_REG]);
    effects = rxop_effects(OP_SIGNAL_STRING);
    check((effects.semantics & (RXOP_SEM_MAY_THROW | RXOP_SEM_OPAQUE)) ==
              (RXOP_SEM_MAY_THROW | RXOP_SEM_OPAQUE) &&
              effects.optimizer_barrier,
          "signal/barrier effects regression", &op_table[OP_SIGNAL_STRING]);
    effects = rxop_effects(OP_JUMPS_REG_BINARY);
    check(effects.flow == FLOW_COND && effects.branch_targets == RXOP_OP_NONE &&
              (effects.semantics & RXOP_SEM_INDIRECT_BRANCH),
          "jump-table indirect-branch effects regression",
          &op_table[OP_JUMPS_REG_BINARY]);
    effects = rxop_effects(OP_SPAWN_REG_REG_REG);
    check(effects.state == RXOP_EFFECT_CONSERVATIVE &&
              effects.reads == RXOP_OP_ALL && effects.writes == RXOP_OP_ALL &&
              effects.optimizer_barrier,
          "opaque process effect must remain conservative",
          &op_table[OP_SPAWN_REG_REG_REG]);
    effects = rxop_effects(OP_RESERVED_087);
    check(effects.state == RXOP_EFFECT_RESERVED && effects.optimizer_barrier,
          "reserved opcode must fail closed", &op_table[OP_RESERVED_087]);

    check_unknown_effects(-1);
    check_unknown_effects(OP_MAX_INSTRUCTIONS);
    check_unknown_effects(OP_MAX_INSTRUCTIONS + 100);

    if (failures) {
        fprintf(stderr, "%d opcode metadata consistency failure(s)\n", failures);
        return 1;
    }

    printf("opcode effects: total=%d source=%d classified=%d conservative=%d "
           "reserved=%d internal=%d\n",
           i, source_count, classified_count, conservative_count,
           reserved_count, internal_count);
    return 0;
}
