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
    unsigned int mask = RXOP_OP_NONE;
    size_t i;
    size_t count = rxop_format_operand_count(format);
    for (i = 0; i < count && i < 3; i++) {
        if (rxop_format_operand_type(format, i) == OP_REG) mask |= 1u << i;
    }
    return mask;
}

static unsigned int format_label_mask(OpFormat format) {
    unsigned int mask = RXOP_OP_NONE;
    size_t i;
    size_t count = rxop_format_operand_count(format);
    for (i = 0; i < count && i < 3; i++) {
        if (rxop_format_operand_type(format, i) == OP_ID) mask |= 1u << i;
    }
    return mask;
}

static int is_internal_opcode(int opcode) {
    return opcode == OP_INULL || opcode == OP_INTERRUPT || opcode == OP_IUNKNOWN;
}

static int effect_has_any_branch_target(const RxOpEffects *effects, OpFormat format) {
    size_t i;
    for (i = 0; i < rxop_format_operand_count(format); i++) {
        if (rxop_effect_branch_target_operand(effects, i)) return 1;
    }
    return 0;
}

static int valid_effect_signature(const char *signature, size_t operand_count) {
    size_t i;
    if (!signature) return 1;
    if (strlen(signature) != operand_count) return 0;
    for (i = 0; i < operand_count; i++) {
        if (signature[i] != '0' && signature[i] != '1') return 0;
    }
    return 1;
}

static void check_unknown_effects(int opcode) {
    RxOpEffects effects;
    RxOpSignalContract signal;

    effects = rxop_effects(opcode);
    check(effects.opcode == opcode, "unknown effect preserves queried opcode", NULL);
    check(effects.state == RXOP_EFFECT_CONSERVATIVE,
          "unknown effect must be conservative", NULL);
    check(effects.reads == RXOP_OP_ALL && effects.writes == RXOP_OP_ALL,
          "unknown effect must expose worst-case explicit access", NULL);
    check(effects.cursor_reads == RXOP_OP_ALL &&
              effects.cursor_writes == RXOP_OP_ALL,
          "unknown effect must expose worst-case cursor access", NULL);
    check(effects.kills == RXOP_OP_NONE,
          "unknown effect must not claim a kill", NULL);
    check(effects.branch_targets == RXOP_OP_ALL,
          "unknown effect must expose worst-case branch operands", NULL);
    check(effects.flow == FLOW_TERM && effects.optimizer_barrier,
          "unknown effect must stop flow and optimization", NULL);
    check(effects.semantics == RXOP_SEM_OPAQUE,
          "unknown effect must expose opaque behavior", NULL);
    signal = rxop_signal_contract(opcode);
    check(signal.opcode == opcode &&
              signal.state == RXOP_SIGNAL_STATE_UNKNOWN &&
              signal.phase == RXOP_SIGNAL_PHASE_UNKNOWN &&
              signal.source == RXOP_SIGNAL_SOURCE_UNKNOWN &&
              signal.policy_effect == RXOP_POLICY_EFFECT_UNKNOWN &&
              signal.policy_source == RXOP_SIGNAL_SOURCE_UNKNOWN &&
              signal.continuations == RXOP_SIGNAL_CONT_ALL,
          "unknown signal contract must fail closed", NULL);
}

static void check_conversion_metadata(int opcode, unsigned int source,
                                      unsigned int target,
                                      RxOpValueDerivation derivation) {
    check(rxop_component_reads(opcode, 0) == source,
          "conversion source-component metadata regression",
          &op_table[opcode]);
    check(rxop_component_writes(opcode, 0) == target,
          "conversion target-component metadata regression",
          &op_table[opcode]);
    check(rxop_value_derivation(opcode) == derivation,
          "conversion derivation metadata regression", &op_table[opcode]);
    check(rxop_derivation_source_operand(opcode) == 0 &&
              rxop_derivation_source_component(opcode) == source,
          "conversion derivation-source metadata regression",
          &op_table[opcode]);
}

static void check_same_storage_copy_metadata(int opcode) {
    check(rxop_same_storage_copy_is_noop(opcode),
          "same-storage copy no-op metadata regression", &op_table[opcode]);
    check(rxop_component_reads(opcode, 1) != RXOP_COMPONENT_NONE &&
              rxop_component_writes(opcode, 0) != RXOP_COMPONENT_NONE,
          "same-storage copy component metadata regression", &op_table[opcode]);
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
    RxOpSignalContract signal;
    const OpInfo *op;

    failures = 0;
    check_same_storage_copy_metadata(OP_COPY_REG_REG);
    check_same_storage_copy_metadata(OP_ICOPY_REG_REG);
    check_same_storage_copy_metadata(OP_FCOPY_REG_REG);
    check_same_storage_copy_metadata(OP_SCOPY_REG_REG);
    check_same_storage_copy_metadata(OP_DCOPY_REG_REG);
    check_same_storage_copy_metadata(OP_ACOPY_REG_REG);
    check_same_storage_copy_metadata(OP_BCOPY_REG_REG);
    check(!rxop_same_storage_copy_is_noop(OP_LINK_REG_REG),
          "non-copy opcode gained same-storage no-op contract",
          &op_table[OP_LINK_REG_REG]);
    source_count = 0;
    classified_count = 0;
    conservative_count = 0;
    reserved_count = 0;
    internal_count = 0;
    legal_semantics = RXOP_SEM_CALL | RXOP_SEM_DYNAMIC_CALL |
        RXOP_SEM_RETURN | RXOP_SEM_ALIAS_CREATE |
        RXOP_SEM_ALIAS_RELEASE | RXOP_SEM_REFERENCE_CREATE |
        RXOP_SEM_REFERENCE_READ | RXOP_SEM_REFERENCE_WRITE |
        RXOP_SEM_REFERENCE_RELEASE | RXOP_SEM_LIFETIME_END |
        RXOP_SEM_INDIRECT_WRITE | RXOP_SEM_INDIRECT_BRANCH |
        RXOP_SEM_OPAQUE;
    check((legal_semantics & 1u) == 0 && RXOP_SEM_CALL == 2 &&
              RXOP_SEM_OPAQUE == 8192,
          "retired MAY_THROW bit or surviving semantic flag values drifted",
          NULL);

    check_conversion_metadata(OP_BTOI_REG, RXOP_COMPONENT_INTEGER,
                              RXOP_COMPONENT_INTEGER,
                              RXOP_DERIVATION_BOOLEAN_TO_INTEGER);
    check_conversion_metadata(OP_BTOD_REG, RXOP_COMPONENT_INTEGER,
                              RXOP_COMPONENT_DECIMAL,
                              RXOP_DERIVATION_BOOLEAN_TO_DECIMAL);
    check_conversion_metadata(OP_BTOF_REG, RXOP_COMPONENT_INTEGER,
                              RXOP_COMPONENT_FLOAT,
                              RXOP_DERIVATION_BOOLEAN_TO_FLOAT);
    check_conversion_metadata(OP_BTOS_REG, RXOP_COMPONENT_INTEGER,
                              RXOP_COMPONENT_STRING,
                              RXOP_DERIVATION_BOOLEAN_TO_STRING);
    check_conversion_metadata(OP_ITOS_REG, RXOP_COMPONENT_INTEGER,
                              RXOP_COMPONENT_STRING,
                              RXOP_DERIVATION_INTEGER_TO_STRING);
    check_conversion_metadata(OP_FTOS_REG, RXOP_COMPONENT_FLOAT,
                              RXOP_COMPONENT_STRING,
                              RXOP_DERIVATION_FLOAT_TO_STRING);
    check_conversion_metadata(OP_ITOF_REG, RXOP_COMPONENT_INTEGER,
                              RXOP_COMPONENT_FLOAT,
                              RXOP_DERIVATION_INTEGER_TO_FLOAT);
    check_conversion_metadata(OP_FTOI_REG, RXOP_COMPONENT_FLOAT,
                              RXOP_COMPONENT_INTEGER,
                              RXOP_DERIVATION_FLOAT_TO_INTEGER);
    check_conversion_metadata(OP_FTOB_REG, RXOP_COMPONENT_FLOAT,
                              RXOP_COMPONENT_INTEGER,
                              RXOP_DERIVATION_FLOAT_TO_BOOLEAN);
    check_conversion_metadata(OP_ITOB_REG, RXOP_COMPONENT_INTEGER,
                              RXOP_COMPONENT_INTEGER,
                              RXOP_DERIVATION_INTEGER_TO_BOOLEAN);
    check_conversion_metadata(OP_STOB_REG, RXOP_COMPONENT_STRING,
                              RXOP_COMPONENT_INTEGER,
                              RXOP_DERIVATION_STRING_TO_BOOLEAN);
    check_conversion_metadata(OP_STOF_REG, RXOP_COMPONENT_STRING,
                              RXOP_COMPONENT_FLOAT,
                              RXOP_DERIVATION_STRING_TO_FLOAT);
    check_conversion_metadata(OP_STOI_REG, RXOP_COMPONENT_STRING,
                              RXOP_COMPONENT_INTEGER,
                              RXOP_DERIVATION_STRING_TO_INTEGER);
    check_conversion_metadata(OP_STOD_REG, RXOP_COMPONENT_STRING,
                              RXOP_COMPONENT_DECIMAL,
                              RXOP_DERIVATION_STRING_TO_DECIMAL);
    check_conversion_metadata(OP_DTOS_REG, RXOP_COMPONENT_DECIMAL,
                              RXOP_COMPONENT_STRING,
                              RXOP_DERIVATION_DECIMAL_TO_STRING);
    check_conversion_metadata(OP_DTOI_REG, RXOP_COMPONENT_DECIMAL,
                              RXOP_COMPONENT_INTEGER,
                              RXOP_DERIVATION_DECIMAL_TO_INTEGER);
    check_conversion_metadata(OP_DTOB_REG, RXOP_COMPONENT_DECIMAL,
                              RXOP_COMPONENT_INTEGER,
                              RXOP_DERIVATION_DECIMAL_TO_BOOLEAN);
    check_conversion_metadata(OP_ITOD_REG, RXOP_COMPONENT_INTEGER,
                              RXOP_COMPONENT_DECIMAL,
                              RXOP_DERIVATION_INTEGER_TO_DECIMAL);
    check_conversion_metadata(OP_FTOD_REG, RXOP_COMPONENT_FLOAT,
                              RXOP_COMPONENT_DECIMAL,
                              RXOP_DERIVATION_FLOAT_TO_DECIMAL);
    check_conversion_metadata(OP_DTOF_REG, RXOP_COMPONENT_DECIMAL,
                              RXOP_COMPONENT_FLOAT,
                              RXOP_DERIVATION_DECIMAL_TO_FLOAT);
    check((RXOP_COMPONENT_ALL & RXOP_COMPONENT_NATIVE_PAYLOAD) != 0 &&
              rxop_component_clears(OP_LOAD_REG_INT, 0) ==
                  (RXOP_COMPONENT_REFERENCE |
                   RXOP_COMPONENT_NATIVE_PAYLOAD) &&
              rxop_component_clears(OP_LOAD_REG_FLOAT, 0) ==
                  (RXOP_COMPONENT_REFERENCE |
                   RXOP_COMPONENT_NATIVE_PAYLOAD) &&
              rxop_component_clears(OP_LOAD_REG_INT, 1) ==
                  RXOP_COMPONENT_NONE,
          "scalar load cleanup-component metadata regression", NULL);
    check(rxop_component_reads(OP_BCOPY_REG_REG, 1) ==
                  (RXOP_COMPONENT_BINARY |
                   RXOP_COMPONENT_NATIVE_PAYLOAD) &&
              rxop_component_writes(OP_BCOPY_REG_REG, 0) ==
                  (RXOP_COMPONENT_BINARY |
                   RXOP_COMPONENT_NATIVE_PAYLOAD),
          "binary copy native-payload metadata regression",
          &op_table[OP_BCOPY_REG_REG]);

    for (i = 0; op_table[i].mnemonic != NULL; i++) {
        size_t operand_index;
        op = &op_table[i];
        effects = rxop_effects(op->opcode);
        signal = rxop_signal_contract(op->opcode);
        legal_registers = format_register_mask(op->format);
        legal_labels = format_label_mask(op->format);

        check(op->opcode == i, "opcode table is not dense and index-aligned", op);
        check(effects.opcode == op->opcode,
              "effects table is not dense and index-aligned", op);
        check(signal.opcode == op->opcode,
              "signal table is not dense and index-aligned", op);
        check(effects.flow == op->flow,
              "effect flow disagrees with canonical opcode flow", op);
        check((effects.reads & ~legal_registers) == 0,
              "read mask names a non-register operand", op);
        check((effects.writes & ~legal_registers) == 0,
              "write mask names a non-register operand", op);
        check((effects.cursor_reads & ~legal_registers) == 0,
              "cursor-read mask names a non-register operand", op);
        check((effects.cursor_writes & ~legal_registers) == 0,
              "cursor-write mask names a non-register operand", op);
        check((effects.kills & ~effects.writes) == 0,
              "kill mask is not a subset of writes", op);
        check((effects.kills & effects.reads) == 0,
              "definite kill also claims to read the same operand", op);
        check((effects.branch_targets & ~legal_labels) == 0,
              "branch-target mask names a non-label operand", op);
        check((effects.semantics & ~legal_semantics) == 0,
              "effect has unknown semantic flags", op);
        if (signal.state != RXOP_SIGNAL_STATE_UNKNOWN) {
            check((signal.failure_writes & ~legal_registers) == 0,
                  "signal failure-write mask names a non-register operand", op);
            check(valid_effect_signature(signal.failure_writes_signature,
                                         rxop_format_operand_count(op->format)),
                  "signal failure-write signature length or bit is invalid", op);
            check(!signal.failure_writes_signature ||
                      signal.failure_writes == RXOP_OP_NONE,
                  "wide signal failure-write signature also carries a legacy mask",
                  op);
        }
        check(valid_effect_signature(effects.reads_signature,
                                     rxop_format_operand_count(op->format)) &&
              valid_effect_signature(effects.writes_signature,
                                     rxop_format_operand_count(op->format)) &&
              valid_effect_signature(effects.kills_signature,
                                     rxop_format_operand_count(op->format)) &&
              valid_effect_signature(effects.branch_targets_signature,
                                     rxop_format_operand_count(op->format)),
              "wide effect signature length or bit is invalid", op);
        check((!effects.reads_signature || effects.reads == RXOP_OP_NONE) &&
              (!effects.writes_signature || effects.writes == RXOP_OP_NONE) &&
              (!effects.kills_signature || effects.kills == RXOP_OP_NONE) &&
              (!effects.branch_targets_signature ||
                   effects.branch_targets == RXOP_OP_NONE),
              "wide effect signature also carries a legacy mask", op);
        for (operand_index = 0;
             operand_index < rxop_format_operand_count(op->format);
             operand_index++) {
            OperandType type = rxop_format_operand_type(op->format, operand_index);
            int reads = rxop_effect_reads_operand(&effects, operand_index);
            int writes = rxop_effect_writes_operand(&effects, operand_index);
            int kills = rxop_effect_kills_operand(&effects, operand_index);
            int branch = rxop_effect_branch_target_operand(&effects, operand_index);
            int cursor_reads = rxop_effect_reads_cursor(&effects, operand_index);
            int cursor_writes = rxop_effect_writes_cursor(&effects, operand_index);
            int failure_writes =
                rxop_signal_failure_writes_operand(&signal, operand_index);
            check(!reads || type == OP_REG,
                  "read effect names a non-register operand", op);
            check(!writes || type == OP_REG,
                  "write effect names a non-register operand", op);
            check(!kills || writes,
                  "kill effect is not a subset of writes", op);
            check(!kills || !reads,
                  "definite kill also claims to read the same operand", op);
            check(!branch || type == OP_ID,
                  "branch effect names a non-label operand", op);
            check(!cursor_reads || type == OP_REG,
                  "cursor-read effect names a non-register operand", op);
            check(!cursor_writes || type == OP_REG,
                  "cursor-write effect names a non-register operand", op);
            check(signal.state == RXOP_SIGNAL_STATE_UNKNOWN ||
                      !failure_writes || type == OP_REG,
                  "signal failure write names a non-register operand", op);
        }
        check(signal.state >= RXOP_SIGNAL_STATE_NONE &&
                  signal.state <= RXOP_SIGNAL_STATE_UNKNOWN,
              "invalid signal contract state", op);
        if (signal.state == RXOP_SIGNAL_STATE_NONE) {
            check(signal.phase == RXOP_SIGNAL_PHASE_NONE &&
                      signal.source == RXOP_SIGNAL_SOURCE_NONE &&
                      signal.failure_writes == RXOP_OP_NONE &&
                      signal.failure_writes_signature == NULL &&
                      signal.failure_component_writes == RXOP_COMPONENT_NONE &&
                      signal.failure_context_writes == RXOP_CONTEXT_NONE &&
                      signal.continuations == RXOP_SIGNAL_CONT_NORMAL,
                  "non-signalling contract carries signal state", op);
        } else if (signal.state == RXOP_SIGNAL_STATE_KNOWN) {
            check(signal.phase != RXOP_SIGNAL_PHASE_NONE &&
                      signal.phase != RXOP_SIGNAL_PHASE_UNKNOWN,
                  "known signal contract lacks an exact phase", op);
            check(signal.source != RXOP_SIGNAL_SOURCE_NONE &&
                      signal.source != RXOP_SIGNAL_SOURCE_UNKNOWN,
                  "known signal contract lacks a signal source", op);
        } else {
            check(signal.phase == RXOP_SIGNAL_PHASE_UNKNOWN &&
                      signal.source == RXOP_SIGNAL_SOURCE_UNKNOWN,
                  "unknown signal contract is not explicitly fail closed", op);
        }
        if (signal.source == RXOP_SIGNAL_SOURCE_STATIC_NAMES)
            check(signal.static_names && signal.static_names[0],
                  "static signal contract lacks names", op);
        else
            check(signal.static_names == NULL,
                  "non-static signal contract carries static names", op);
        if (signal.source == RXOP_SIGNAL_SOURCE_LITERAL_OPERAND ||
            signal.source == RXOP_SIGNAL_SOURCE_REGISTER_OPERAND) {
            check(signal.source_operand < rxop_format_operand_count(op->format),
                  "dynamic signal source operand is out of range", op);
            if (signal.source_operand < rxop_format_operand_count(op->format))
                check(rxop_format_operand_type(op->format,
                                               signal.source_operand) ==
                          (signal.source == RXOP_SIGNAL_SOURCE_LITERAL_OPERAND
                               ? OP_STRING : OP_REG),
                      "dynamic signal source operand has the wrong type", op);
        } else {
            check(signal.source_operand == SIZE_MAX,
                  "non-operand signal source carries an operand index", op);
        }
        if (signal.properties & RXOP_SIGNAL_PROP_POLICY_WRITE) {
            check(signal.policy_effect > RXOP_POLICY_EFFECT_NONE &&
                      signal.policy_effect < RXOP_POLICY_EFFECT_UNKNOWN,
                  "policy write lacks an exact normal-path effect", op);
            check(signal.policy_source == RXOP_SIGNAL_SOURCE_STATIC_NAMES ||
                      signal.policy_source ==
                          RXOP_SIGNAL_SOURCE_LITERAL_OPERAND ||
                      signal.policy_source ==
                          RXOP_SIGNAL_SOURCE_REGISTER_OPERAND ||
                      signal.policy_source == RXOP_SIGNAL_SOURCE_UNKNOWN,
                  "policy write has an invalid name source", op);
            if (signal.policy_source == RXOP_SIGNAL_SOURCE_STATIC_NAMES) {
                check(signal.policy_static_name &&
                          signal.policy_static_name[0] &&
                          signal.policy_source_operand == SIZE_MAX,
                      "static policy write lacks its exact name", op);
            } else if (signal.policy_source ==
                       RXOP_SIGNAL_SOURCE_LITERAL_OPERAND ||
                       signal.policy_source ==
                           RXOP_SIGNAL_SOURCE_REGISTER_OPERAND) {
                check(signal.policy_static_name == NULL &&
                          signal.policy_source_operand <
                              rxop_format_operand_count(op->format),
                      "operand policy source is out of range", op);
                if (signal.policy_source_operand <
                    rxop_format_operand_count(op->format))
                    check(rxop_format_operand_type(
                                  op->format,
                                  signal.policy_source_operand) ==
                              (signal.policy_source ==
                                       RXOP_SIGNAL_SOURCE_LITERAL_OPERAND
                                   ? OP_STRING : OP_REG),
                          "policy source operand has the wrong type", op);
            }
        } else {
            check(signal.policy_effect == RXOP_POLICY_EFFECT_NONE &&
                      signal.policy_source == RXOP_SIGNAL_SOURCE_NONE &&
                      signal.policy_source_operand == SIZE_MAX &&
                      signal.policy_static_name == NULL,
                  "non-policy contract carries a policy transfer", op);
        }
        check(effects.const_evaluator >= RXOP_CONST_EVAL_NONE &&
                  effects.const_evaluator <= RXOP_CONST_EVAL_STRUPPER,
              "constant evaluator id is invalid", op);
        check(!(effects.semantics & RXOP_SEM_DYNAMIC_CALL) ||
                  (effects.semantics & RXOP_SEM_CALL),
              "dynamic call is not classified as a call", op);
        check(!(effects.semantics & RXOP_SEM_RETURN) || op->flow == FLOW_TERM,
              "return classification is not terminal", op);

        check(((op->flags & FLG_IMPLICIT_REG_USE) != 0) ==
                  (effects.implicit != RXOP_IMPLICIT_NONE),
              "implicit-register flag and effect metadata disagree", op);
        if (effects.implicit == RXOP_IMPLICIT_LOCAL_RANGE_AFTER_OP3) {
            check(rxop_format_operand_count(op->format) >= 3 &&
                      rxop_format_operand_type(op->format, 2) == OP_REG,
                  "runtime register range requires register operand 3", op);
            check(effects.optimizer_barrier,
                  "runtime register range must remain an optimizer barrier", op);
        }

        if (op->flow == FLOW_JUMP) {
            check(effect_has_any_branch_target(&effects, op->format),
                  "unconditional jump lacks a branch target", op);
        }
        if (op->flow == FLOW_COND) {
            check(effect_has_any_branch_target(&effects, op->format) ||
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
    check(rxop_signal_contract_count() == (size_t)OP_MAX_INSTRUCTIONS,
          "signal inventory size does not match opcode enum", NULL);
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
    signal = rxop_signal_contract(OP_LOAD_REG_DECIMAL);
    check(signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.source == RXOP_SIGNAL_SOURCE_PLUGIN &&
              signal.phase == RXOP_SIGNAL_PHASE_PARTIAL_WRITES,
          "decimal literal load must expose plugin-partial failure",
          &op_table[OP_LOAD_REG_DECIMAL]);
    signal = rxop_signal_contract(OP_DCOPY_REG_REG);
    check(signal.state == RXOP_SIGNAL_STATE_NONE,
          "decimal copy must remain total over absent payloads",
          &op_table[OP_DCOPY_REG_REG]);
    signal = rxop_signal_contract(OP_SIGBR_ID_STRING);
    check((signal.properties & RXOP_SIGNAL_PROP_POLICY_WRITE) &&
              signal.policy_effect == RXOP_POLICY_EFFECT_BRANCH &&
              signal.policy_source == RXOP_SIGNAL_SOURCE_LITERAL_OPERAND &&
              signal.policy_source_operand == 1,
          "branch-handler policy metadata regression",
          &op_table[OP_SIGBR_ID_STRING]);
    signal = rxop_signal_contract(OP_SIGPUSH_STRING);
    check(signal.policy_effect == RXOP_POLICY_EFFECT_PUSH &&
              signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES,
          "signal-stack policy metadata regression",
          &op_table[OP_SIGPUSH_STRING]);
    signal = rxop_signal_contract(OP_BPOFF);
    check(signal.state == RXOP_SIGNAL_STATE_NONE &&
              signal.policy_effect == RXOP_POLICY_EFFECT_BREAKPOINT_DISABLE &&
              signal.policy_static_name &&
              strcmp(signal.policy_static_name, "BREAKPOINT") == 0,
          "breakpoint policy metadata regression", &op_table[OP_BPOFF]);
    signal = rxop_signal_contract(OP_CALL_FUNC);
    check(signal.state == RXOP_SIGNAL_STATE_UNKNOWN &&
              signal.policy_effect == RXOP_POLICY_EFFECT_NONE &&
              signal.policy_source == RXOP_SIGNAL_SOURCE_NONE,
          "unknown call signals must not imply a caller-policy write",
          &op_table[OP_CALL_FUNC]);
    signal = rxop_signal_contract(OP_ITOF_REG);
    check(signal.state == RXOP_SIGNAL_STATE_NONE &&
              (signal.properties & RXOP_SIGNAL_PROP_SUCCESS_STABLE),
          "integer-to-float conversion must remain stable and non-signalling",
          &op_table[OP_ITOF_REG]);
    signal = rxop_signal_contract(OP_BTOD_REG);
    check(signal.state == RXOP_SIGNAL_STATE_NONE &&
              signal.dependencies ==
                  (RXOP_SIGNAL_DEP_NUMERIC_CONTEXT | RXOP_SIGNAL_DEP_PLUGIN) &&
              (signal.properties & RXOP_SIGNAL_PROP_SUCCESS_STABLE),
          "boolean-to-decimal conversion must remain total",
          &op_table[OP_BTOD_REG]);
    signal = rxop_signal_contract(OP_SCONCAT_REG_REG_STRING);
    check(signal.state == RXOP_SIGNAL_STATE_NONE &&
              rxop_signal_contract(OP_CONCAT_REG_REG_REG).state ==
                    RXOP_SIGNAL_STATE_NONE &&
              rxop_signal_contract(OP_CONCAT_REG_STRING_REG).state ==
                    RXOP_SIGNAL_STATE_NONE &&
              rxop_signal_contract(OP_SCONCAT_REG_REG_REG).state ==
                    RXOP_SIGNAL_STATE_NONE,
          "concat family must match its non-signalling VM implementation",
          &op_table[OP_SCONCAT_REG_REG_STRING]);
    signal = rxop_signal_contract(OP_STEMSET_REG_REG_REG);
    check(signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES &&
              signal.source == RXOP_SIGNAL_SOURCE_STATIC_NAMES &&
              signal.static_names &&
              strcmp(signal.static_names,
                     "UNICODE_ERROR|INVALID_ARGUMENTS|FAILURE") == 0 &&
              signal.failure_writes == RXOP_OP_NONE &&
              signal.failure_component_writes == RXOP_COMPONENT_NONE &&
              signal.failure_context_writes == RXOP_CONTEXT_NONE &&
              signal.dependencies == RXOP_SIGNAL_DEP_EXTERNAL_STATE &&
              rxop_signal_contract(OP_STEMSET2_REG_REG_REG_REG).phase ==
                    RXOP_SIGNAL_PHASE_BEFORE_WRITES,
          "stem writes must expose their failure-atomic VM signal contract",
          &op_table[OP_STEMSET_REG_REG_REG]);
    signal = rxop_signal_contract(OP_DGT_REG_REG_REG);
    check(signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.source == RXOP_SIGNAL_SOURCE_PLUGIN &&
              signal.phase == RXOP_SIGNAL_PHASE_AFTER_WRITES &&
              signal.failure_writes == RXOP_OP_1 &&
              signal.failure_component_writes == RXOP_COMPONENT_INTEGER &&
              signal.dependencies == (RXOP_SIGNAL_DEP_NUMERIC_CONTEXT |
                                      RXOP_SIGNAL_DEP_PLUGIN) &&
              (signal.properties & RXOP_SIGNAL_PROP_SUCCESS_STABLE) &&
              rxop_signal_contract(OP_DLT_REG_REG_DECIMAL).phase ==
                    RXOP_SIGNAL_PHASE_AFTER_WRITES,
          "decimal comparisons must expose their post-result plugin signal",
          &op_table[OP_DGT_REG_REG_REG]);
    effects = rxop_effects(OP_ITOS_REG);
    signal = rxop_signal_contract(OP_ITOS_REG);
    check(rxop_component_reads(OP_ITOS_REG, 0) == RXOP_COMPONENT_INTEGER &&
              rxop_component_writes(OP_ITOS_REG, 0) == RXOP_COMPONENT_STRING &&
              rxop_value_derivation(OP_ITOS_REG) ==
                  RXOP_DERIVATION_INTEGER_TO_STRING &&
              rxop_derivation_context_reads(OP_ITOS_REG) ==
                  RXOP_CONTEXT_NUMERIC &&
              signal.state == RXOP_SIGNAL_STATE_NONE &&
              signal.dependencies == RXOP_SIGNAL_DEP_NUMERIC_CONTEXT &&
              (signal.properties & RXOP_SIGNAL_PROP_SUCCESS_STABLE),
          "integer-to-string component/signal metadata regression",
          &op_table[OP_ITOS_REG]);
    check(rxop_context_writes(OP_SETNUMCAS_INT) == RXOP_CONTEXT_NUMERIC &&
              rxop_context_writes(OP_GETNUMCAS_REG) == RXOP_CONTEXT_NONE,
          "numeric-context component metadata regression", NULL);
    signal = rxop_signal_contract(OP_ITOF_REG_REG);
    check(signal.state == RXOP_SIGNAL_STATE_NONE &&
              rxop_value_derivation(OP_ITOF_REG_REG) ==
                    RXOP_DERIVATION_INTEGER_TO_FLOAT &&
              rxop_derivation_source_operand(OP_ITOF_REG_REG) == 1 &&
              rxop_derivation_source_operand(OP_ITOF_REG) == 0 &&
              rxop_derivation_source_operand(OP_LOAD_REG_INT) == SIZE_MAX,
          "two-register conversion source metadata regression",
          &op_table[OP_ITOF_REG_REG]);
    signal = rxop_signal_contract(OP_FEQ_REG_REG_FLOAT);
    check(signal.state == RXOP_SIGNAL_STATE_NONE,
          "float comparison must remain non-signalling",
          &op_table[OP_FEQ_REG_REG_FLOAT]);
    signal = rxop_signal_contract(OP_FTOS_REG);
    check(rxop_component_reads(OP_FTOS_REG, 0) == RXOP_COMPONENT_FLOAT &&
              rxop_component_writes(OP_FTOS_REG, 0) ==
                  RXOP_COMPONENT_STRING &&
              rxop_value_derivation(OP_FTOS_REG) ==
                  RXOP_DERIVATION_FLOAT_TO_STRING &&
              rxop_derivation_context_reads(OP_FTOS_REG) ==
                  RXOP_CONTEXT_NUMERIC &&
              signal.state == RXOP_SIGNAL_STATE_NONE &&
              signal.dependencies == RXOP_SIGNAL_DEP_NUMERIC_CONTEXT &&
              (signal.properties & RXOP_SIGNAL_PROP_SUCCESS_STABLE),
          "float-to-string component/signal metadata regression",
          &op_table[OP_FTOS_REG]);
    signal = rxop_signal_contract(OP_DTOS_REG);
    check(rxop_component_reads(OP_DTOS_REG, 0) == RXOP_COMPONENT_DECIMAL &&
              rxop_component_writes(OP_DTOS_REG, 0) ==
                  RXOP_COMPONENT_STRING &&
              rxop_value_derivation(OP_DTOS_REG) ==
                  RXOP_DERIVATION_DECIMAL_TO_STRING &&
              rxop_derivation_context_reads(OP_DTOS_REG) ==
                  RXOP_CONTEXT_NUMERIC &&
              signal.state == RXOP_SIGNAL_STATE_NONE &&
              signal.dependencies ==
                  (RXOP_SIGNAL_DEP_NUMERIC_CONTEXT | RXOP_SIGNAL_DEP_PLUGIN) &&
              (signal.properties & RXOP_SIGNAL_PROP_SUCCESS_STABLE),
          "decimal-to-string component/signal metadata regression",
          &op_table[OP_DTOS_REG]);
    signal = rxop_signal_contract(OP_ITOD_REG);
    check(rxop_component_reads(OP_ITOD_REG, 0) == RXOP_COMPONENT_INTEGER &&
              rxop_component_writes(OP_ITOD_REG, 0) ==
                  RXOP_COMPONENT_DECIMAL &&
              rxop_value_derivation(OP_ITOD_REG) ==
                  RXOP_DERIVATION_INTEGER_TO_DECIMAL &&
              rxop_derivation_source_operand(OP_ITOD_REG) == 0 &&
              rxop_derivation_source_component(OP_ITOD_REG) ==
                  RXOP_COMPONENT_INTEGER &&
              rxop_derivation_context_reads(OP_ITOD_REG) ==
                  RXOP_CONTEXT_NUMERIC &&
              signal.state == RXOP_SIGNAL_STATE_NONE &&
              signal.phase == RXOP_SIGNAL_PHASE_NONE &&
              signal.failure_component_writes == RXOP_COMPONENT_NONE &&
              signal.dependencies ==
                  (RXOP_SIGNAL_DEP_NUMERIC_CONTEXT | RXOP_SIGNAL_DEP_PLUGIN) &&
              (signal.properties & RXOP_SIGNAL_PROP_SUCCESS_STABLE),
          "integer-to-decimal derivation/signal metadata regression",
          &op_table[OP_ITOD_REG]);
    signal = rxop_signal_contract(OP_INC_REG);
    check(signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES &&
              strcmp(signal.static_names, "OVERFLOW_UNDERFLOW") == 0,
          "checked increment signal metadata regression",
          &op_table[OP_INC_REG]);
    signal = rxop_signal_contract(OP_SETNUMFUZ_INT);
    check(signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES &&
              signal.failure_context_writes == RXOP_CONTEXT_NONE,
          "numeric fuzz signal phase regression",
          &op_table[OP_SETNUMFUZ_INT]);
    signal = rxop_signal_contract(OP_FREADCDPT_REG_REG);
    check(signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.phase == RXOP_SIGNAL_PHASE_PARTIAL_WRITES &&
              signal.failure_writes == RXOP_OP_1 &&
              signal.failure_component_writes == RXOP_COMPONENT_STRING,
          "partial UTF-8 read signal metadata regression",
          &op_table[OP_FREADCDPT_REG_REG]);
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
    check(effects.cursor_writes == RXOP_OP_1 &&
              effects.const_evaluator == RXOP_CONST_EVAL_PADSTR,
          "PADSTR cursor/evaluator regression",
          &op_table[OP_PADSTR_REG_REG_REG]);
    effects = rxop_effects(OP_STRLEN_REG_REG);
    check(effects.cursor_reads == RXOP_OP_NONE &&
              effects.cursor_writes == RXOP_OP_NONE &&
              effects.const_evaluator == RXOP_CONST_EVAL_STRLEN,
          "STRLEN evaluator must not invent cursor effects",
          &op_table[OP_STRLEN_REG_REG]);
    effects = rxop_effects(OP_SETSTRPOS_REG_REG);
    check(effects.cursor_writes == RXOP_OP_1 &&
              effects.const_evaluator == RXOP_CONST_EVAL_SETSTRPOS,
          "SETSTRPOS cursor/evaluator regression",
          &op_table[OP_SETSTRPOS_REG_REG]);
    effects = rxop_effects(OP_GETSTRPOS_REG_REG);
    check(effects.cursor_reads == RXOP_OP_2 &&
              effects.const_evaluator == RXOP_CONST_EVAL_GETSTRPOS,
          "GETSTRPOS cursor/evaluator regression",
          &op_table[OP_GETSTRPOS_REG_REG]);
    effects = rxop_effects(OP_STRCHAR_REG_REG_REG);
    check(effects.cursor_writes == RXOP_OP_2 &&
              effects.const_evaluator == RXOP_CONST_EVAL_STRCHAR_AT,
          "indexed STRCHAR source cursor regression",
          &op_table[OP_STRCHAR_REG_REG_REG]);
    effects = rxop_effects(OP_SUBSTRING_REG_REG_REG);
    check(effects.cursor_reads == RXOP_OP_2 &&
              effects.cursor_writes == RXOP_OP_1 &&
              effects.const_evaluator == RXOP_CONST_EVAL_SUBSTRING,
          "SUBSTRING cursor/evaluator regression",
          &op_table[OP_SUBSTRING_REG_REG_REG]);
    effects = rxop_effects(OP_FNDBLNK_REG_REG_REG);
    check(effects.cursor_writes == RXOP_OP_2 &&
              effects.const_evaluator == RXOP_CONST_EVAL_FNDBLNK,
          "FNDBLNK source cursor regression",
          &op_table[OP_FNDBLNK_REG_REG_REG]);
    effects = rxop_effects(OP_FNDNBLNK_REG_REG_REG);
    check(effects.cursor_writes == RXOP_OP_2 &&
              effects.const_evaluator == RXOP_CONST_EVAL_FNDNBLNK,
          "FNDNBLNK source cursor regression",
          &op_table[OP_FNDNBLNK_REG_REG_REG]);
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
    signal = rxop_signal_contract(OP_SIGNAL_STRING);
    check(effects.semantics == RXOP_SEM_OPAQUE && effects.optimizer_barrier &&
              signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.source == RXOP_SIGNAL_SOURCE_LITERAL_OPERAND &&
              signal.source_operand == 0,
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
    effects = rxop_effects(OP_CNOP_REG_REG_REG_REG_REG_REG_REG_REG_REG);
    check(rxop_format_operand_count(
              op_table[OP_CNOP_REG_REG_REG_REG_REG_REG_REG_REG_REG].format) == 9 &&
              rxop_effect_reads_operand(&effects, 0) &&
              rxop_effect_reads_operand(&effects, 8) &&
              !rxop_effect_writes_operand(&effects, 8) &&
              !rxop_effect_reads_operand(&effects, 9),
          "wide operand signature/effects regression",
          &op_table[OP_CNOP_REG_REG_REG_REG_REG_REG_REG_REG_REG]);
    effects = rxop_effects(OP_RESERVED_088);
    check(effects.state == RXOP_EFFECT_RESERVED && effects.optimizer_barrier,
          "reserved opcode must fail closed", &op_table[OP_RESERVED_088]);

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
