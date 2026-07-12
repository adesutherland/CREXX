/*
 * Generated-database consistency checks for RXAS optimizer metadata.
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

static int format_starts_with_register(OpFormat format) {
    return format >= FMT_R && format <= FMT_R_S_S;
}

static int format_starts_with_label(OpFormat format) {
    return format == FMT_L ||
           format == FMT_L_L_R ||
           format == FMT_L_P_S ||
           format == FMT_L_R ||
           format == FMT_L_R_I ||
           format == FMT_L_R_R ||
           format == FMT_L_R_S ||
           format == FMT_L_S;
}

static int is_jump_table_opcode(int opcode) {
    return opcode >= OP_JUMPS_REG_BINARY && opcode <= OP_JUMPN_REG_BINARY;
}

int main(void) {
    int i;
    RxOpEffects effects;
    const OpInfo *op;

    failures = 0;
    for (i = 0; op_table[i].mnemonic != NULL; i++) {
        op = &op_table[i];
        effects = rxop_effects(op->opcode);

        check(op->opcode == i, "opcode table is not dense and index-aligned", op);
        check((effects.flags & ~RXOP_EFFECT_OP1_KILL) == 0,
              "optimizer effect has unknown flags", op);
        check(((op->flags & FLG_IMPLICIT_REG_USE) != 0) ==
                  (effects.implicit != RXOP_IMPLICIT_NONE),
              "implicit-register flag and effect metadata disagree", op);

        if (effects.flags & RXOP_EFFECT_OP1_KILL) {
            check(format_starts_with_register(op->format),
                  "operand-1 kill requires a register destination", op);
        }

        if (effects.implicit == RXOP_IMPLICIT_LOCAL_RANGE_AFTER_OP3) {
            check((op->flags & FLG_OPT_BARRIER) != 0,
                  "runtime register range must also be an optimizer barrier", op);
            check(op->format == FMT_R_P_R ||
                      op->format == FMT_R_R_R ||
                      op->format == FMT_R_S_R,
                  "runtime register range requires register operand 3", op);
        }

        if (op->flow == FLOW_JUMP) {
            check(format_starts_with_label(op->format),
                  "unconditional jump requires a label first operand", op);
        }
        if (op->flow == FLOW_COND && !is_jump_table_opcode(op->opcode)) {
            check(format_starts_with_label(op->format),
                  "conditional flow requires a label or jump-table metadata", op);
        }
    }

    check(i == OP_MAX_INSTRUCTIONS, "opcode table size does not match opcode enum", NULL);
    check(op_table[OP_BTOI_REG].flow == FLOW_NEXT, "btoi must fall through", &op_table[OP_BTOI_REG]);
    check(op_table[OP_BTOD_REG].flow == FLOW_NEXT, "btod must fall through", &op_table[OP_BTOD_REG]);
    check(op_table[OP_BTOF_REG].flow == FLOW_NEXT, "btof must fall through", &op_table[OP_BTOF_REG]);
    check(op_table[OP_BTOS_REG].flow == FLOW_NEXT, "btos must fall through", &op_table[OP_BTOS_REG]);
    check(op_table[OP_BCTP_ID_REG].flow == FLOW_JUMP,
          "bctp must be an unconditional jump", &op_table[OP_BCTP_ID_REG]);
    check((op_table[OP_METALOADMODULE_REG_REG].flags & FLG_OPT_BARRIER) != 0,
          "metaloadmodule must be an optimizer barrier", &op_table[OP_METALOADMODULE_REG_REG]);
    check((rxop_effects(OP_TRIML_REG_REG).flags & RXOP_EFFECT_OP1_KILL) == 0,
          "deprecated triml reads operand 1 before mutation", &op_table[OP_TRIML_REG_REG]);
    check((rxop_effects(OP_TRIMR_REG_REG).flags & RXOP_EFFECT_OP1_KILL) == 0,
          "deprecated trimr reads operand 1 before mutation", &op_table[OP_TRIMR_REG_REG]);

    if (failures) {
        fprintf(stderr, "%d RXAS metadata consistency failure(s)\n", failures);
        return 1;
    }
    return 0;
}
