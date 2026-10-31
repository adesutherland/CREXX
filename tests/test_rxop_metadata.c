/*
 * Generated-database consistency checks for the canonical opcode effects
 * inventory and its RXAS optimizer consumers.
 */

#include <stdio.h>
#include <string.h>

#include "rxdefs.h"
#include "rxflags.h"

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

/* One database-wide structural audit. Component projections and signal
 * precision are reported separately: neither is a value evaluator or a
 * reason to equate an ordinary external effect with a physical-frame read. */
static void check_clone_contract(const OpInfo *op, int *categories,
                                int *unknown_signals, int *unknown_writes) {
    RxOpEffects effects = rxop_effects(op->opcode);
    RxOpSignalContract signal = rxop_signal_contract(op->opcode);
    RxOpCloneCategory category = rxop_clone_category(op->opcode);
    RxOpBindingAction binding = rxop_binding_action(op->opcode);
    unsigned int activation = rxop_activation_effects(op->opcode);
    size_t i, count = rxop_format_operand_count(op->format);
    int targets = 0, successors = rxop_normal_successors(op->opcode);
    int fallthrough = rxop_normal_fallthrough(op->opcode);
    check(category >= RXOP_CLONE_MISSING && category <= RXOP_CLONE_RETAINED,
          "source opcode has no structural clone treatment", op);
    if (category >= RXOP_CLONE_MISSING && category <= RXOP_CLONE_RETAINED)
        ++categories[category];
    check(binding >= RXOP_BINDING_NONE && binding <= RXOP_BINDING_LIFETIME,
          "source opcode has an unknown binding action", op);
    check(effects.implicit >= RXOP_IMPLICIT_NONE &&
              effects.implicit <= RXOP_IMPLICIT_QUALIFIED_GLOBAL,
          "source opcode has an unknown implicit selector", op);
    if (signal.state == RXOP_SIGNAL_STATE_UNKNOWN) ++*unknown_signals;
    for (i = 0; i < count; ++i) {
        if (rxop_effect_branch_target_operand(&effects, i)) {
            ++targets;
            check(rxop_format_operand_type(op->format, i) == OP_ID,
                  "normal/exceptional relocation target is not a label", op);
        }
        if (rxop_effect_writes_operand(&effects, i) &&
            !rxop_component_write_is_known(op->opcode, i)) ++*unknown_writes;
        if (!rxop_effect_writes_operand(&effects, i))
            check(rxop_component_write_is_known(op->opcode, i) ==
                      (effects.state == RXOP_EFFECT_CLASSIFIED),
                  "known no-write is confused with missing write projection", op);
    }
    if (effects.state != RXOP_EFFECT_CLASSIFIED) {
        check(category == RXOP_CLONE_MISSING &&
                  (activation & RXOP_ACTIVATION_UNKNOWN),
              "unclassified opcode silently claims ordinary cloning", op);
        return;
    }
    if (effects.semantics & RXOP_SEM_INDIRECT_BRANCH) {
        check(successors == -2 && !fallthrough &&
                  category == RXOP_CLONE_RETAINED,
              "indirect jump invents a complete direct-label CFG", op);
    } else if (op->flow == FLOW_NEXT) {
        check(successors == -1 && !fallthrough,
              "normal fallthrough invents exceptional label successors", op);
    } else if (op->flow == FLOW_TERM) {
        check(successors == 0 && !fallthrough,
              "terminal opcode has normal successors", op);
    } else if (op->flow == FLOW_JUMP) {
        check(targets == 1 && successors == 1 && !fallthrough,
              "direct jump lacks its declared target edge", op);
    } else if (op->flow == FLOW_COND) {
        check((targets == 1 || targets == 2) && successors == 2 &&
                  fallthrough == (targets == 1),
              "direct conditional lacks ordered targets/fallthrough", op);
    }
    if (category == RXOP_CLONE_ORDINARY)
        check(activation == RXOP_ACTIVATION_NONE &&
                  effects.implicit == RXOP_IMPLICIT_NONE &&
                  binding == RXOP_BINDING_NONE && successors != -2,
              "ordinary clone hides an activation adapter/observation", op);
    if (binding != RXOP_BINDING_NONE)
        check((activation & RXOP_ACTIVATION_BINDING) &&
                  category != RXOP_CLONE_ORDINARY,
              "binding action silently claims ordinary cloning", op);
    if (effects.semantics & RXOP_SEM_CALL)
        check(category == RXOP_CLONE_CALL &&
                  (activation & RXOP_ACTIVATION_CALL),
              "call lacks its activation adapter", op);
    if (effects.semantics & RXOP_SEM_RETURN)
        check(category == RXOP_CLONE_RETURN &&
                  (activation & RXOP_ACTIVATION_RETURN),
              "return lacks its activation adapter", op);
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

static void check_target_contract(const OpInfo *op) {
    RxOpEffects effects = rxop_effects(op->opcode);
    RxOpCallContract call = rxop_call_contract(op->opcode);
    RxOpSelectionContract selection = rxop_selection_contract(op->opcode);
    size_t i, count = rxop_format_operand_count(op->format);
    if (effects.state != RXOP_EFFECT_CLASSIFIED) return;
    check(rxop_binding_recipe(op->opcode) != NULL,
          "classified binding lacks an ordered canonical recipe", op);
    check(rxop_implicit_recipe(op->opcode) != NULL,
          "classified implicit selector lacks a canonical recipe", op);
    check(rxop_call_invokes(op->opcode) == !!(effects.semantics & RXOP_SEM_CALL),
          "canonical call recipe disagrees with call semantics", op);
    if (call.kind != RXOP_CALL_NONE) {
        check(call.target_operand > 0 && call.target_operand <= count &&
                  call.result_operand <= count && call.count_operand <= count &&
                  call.window_operand == call.count_operand &&
                  call.capture_after_prefix && call.atomic_prefix &&
                  call.actual_operands && call.prefix,
              "call recipe has invalid selectors or capture boundary", op);
        if (call.kind == RXOP_CALL_DIRECT)
            check(rxop_format_operand_type(op->format, call.target_operand - 1) == OP_FUNC,
                  "direct call target is not a procedure reference", op);
    }
    if (selection.kind != RXOP_SELECTION_NONE) {
        RxOpSignalContract signal = rxop_signal_contract(op->opcode);
        check(!rxop_call_invokes(op->opcode) &&
                  !(effects.semantics & (RXOP_SEM_CALL | RXOP_SEM_DYNAMIC_CALL)),
              "procedure selection must not invoke an activation", op);
        check(selection.result_operand > 0 && selection.result_operand <= count &&
                  selection.descriptor_operand > 0 && selection.descriptor_operand <= count &&
                  selection.receiver_operand <= count && selection.count_operand <= count &&
                  selection.window_operand == selection.count_operand &&
                  rxop_format_operand_type(op->format, selection.result_operand - 1) == OP_REG &&
                  rxop_format_operand_type(op->format, selection.descriptor_operand - 1) == OP_STRING,
              "procedure selection roles must be typed valid operands", op);
        check(rxop_component_clears(op->opcode, selection.result_operand - 1) == RXOP_COMPONENT_ALL &&
                  rxop_definition_components(op->opcode, selection.result_operand - 1) == RXOP_COMPONENT_INTEGER,
              "selection success resets then defines pointer integer", op);
        if (selection.kind == RXOP_SELECTION_METHOD)
            check(!rxop_selection_callbacks(op->opcode) &&
                      signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES &&
                      !rxop_signal_failure_writes_operand(&signal, selection.result_operand - 1),
                  "method resolver failure preserves result before result finalization", op);
        else
            check(rxop_selection_callbacks(op->opcode) &&
                      signal.state == RXOP_SIGNAL_STATE_UNKNOWN,
                  "factory match callback has unknown mutation/failure phase", op);
    }
    for (i = 0; i < count; ++i) {
        RxOpRelocationKind relocation = rxop_operand_relocation(op->opcode, i);
        unsigned int definition = rxop_definition_components(op->opcode, i);
        check(relocation != RXOP_RELOC_UNKNOWN,
              "classified operand lacks a typed relocation contract", op);
        check(!(definition & ~rxop_component_may_writes(op->opcode, i)),
              "normal definition is outside conservative may-write coverage", op);
        if (rxop_effect_writes_operand(&effects, i))
            check(rxop_component_may_writes(op->opcode, i) == RXOP_COMPONENT_ALL &&
                      rxop_flag_may_writes(op->opcode, i) == RXFLAG_READABLE_MASK,
                  "explicit write lacks conservative component/flag coverage", op);
        if (rxop_format_operand_type(op->format, i) == OP_REG)
            check(relocation == RXOP_RELOC_REGISTER,
                  "explicit register relocation loses its typed selector", op);
    }
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

static void check_propagated_call_metadata(int opcode, int writes_result) {
    RxOpSignalContract signal;
    signal = rxop_signal_contract(opcode);
    check(signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.phase == RXOP_SIGNAL_PHASE_PARTIAL_WRITES &&
              signal.source == RXOP_SIGNAL_SOURCE_PROPAGATED_CALL &&
              signal.source_operand == SIZE_MAX &&
              signal.dependencies == RXOP_SIGNAL_DEP_UNKNOWN &&
              signal.continuations == RXOP_SIGNAL_CONT_ALL &&
              signal.policy_effect == RXOP_POLICY_EFFECT_NONE &&
              signal.policy_source == RXOP_SIGNAL_SOURCE_NONE,
          "propagated call signal contract regression", &op_table[opcode]);
    check(rxop_signal_failure_writes_operand(&signal, 0) == writes_result,
          "propagated call result failure phase regression",
          &op_table[opcode]);
}

static void check_native_stem_signal_contract(int opcode) {
    RxOpSignalContract signal = rxop_signal_contract(opcode);

    check(signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES &&
              signal.source == RXOP_SIGNAL_SOURCE_STATIC_NAMES &&
              signal.static_names &&
              strcmp(signal.static_names, "UNICODE_ERROR|FAILURE") == 0 &&
              signal.failure_writes == RXOP_OP_NONE &&
              signal.failure_component_writes == RXOP_COMPONENT_NONE &&
              signal.failure_context_writes == RXOP_CONTEXT_NONE &&
              signal.dependencies == RXOP_SIGNAL_DEP_EXTERNAL_STATE &&
              !(signal.properties & RXOP_SIGNAL_PROP_SUCCESS_STABLE),
          "native stem access must expose its exact failure-atomic contract",
          &op_table[opcode]);
}

static void check_checked_integer_opcode(int opcode) {
    const OpInfo *op = &op_table[opcode];
    RxOpSignalContract signal = rxop_signal_contract(opcode);
    RxOpEffects effects = rxop_effects(opcode);
    int partial = opcode >= OP_IPOW_REG_REG_REG && opcode <= OP_IPOW_REG_INT_REG;
    int setter = opcode >= OP_IADD_REG_REG_REG && opcode <= OP_IMOD_REG_INT_REG;
    const char *names = opcode >= OP_IDIV_REG_REG_REG && opcode <= OP_IMOD_REG_INT_REG
            ? "DIVISION_BY_ZERO|OVERFLOW_UNDERFLOW" : "OVERFLOW_UNDERFLOW";
    size_t operand;
    check(signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.phase == (partial ? RXOP_SIGNAL_PHASE_PARTIAL_WRITES
                                       : RXOP_SIGNAL_PHASE_BEFORE_WRITES) &&
              signal.source == RXOP_SIGNAL_SOURCE_STATIC_NAMES &&
              signal.static_names && !strcmp(signal.static_names, names) &&
              signal.failure_writes == (partial ? RXOP_OP_1 : RXOP_OP_NONE) &&
              signal.failure_component_writes == (partial ? RXOP_COMPONENT_INTEGER
                                                          : RXOP_COMPONENT_NONE) &&
              signal.failure_context_writes == RXOP_CONTEXT_NONE &&
              signal.dependencies == RXOP_SIGNAL_DEP_NONE &&
              signal.continuations == RXOP_SIGNAL_CONT_ALL,
          "checked integer arithmetic has exact failure phase, names and writes", op);
    check(rxop_component_clears(opcode, 0) ==
              (setter ? RXOP_COMPONENT_REFERENCE | RXOP_COMPONENT_NATIVE_PAYLOAD
                      : RXOP_COMPONENT_NONE),
          "integer setter versus direct-field payload lifetime contract", op);
    for (operand = 0; operand < rxop_format_operand_count(op->format); ++operand) {
        if (rxop_effect_reads_operand(&effects, operand))
            check(rxop_component_may_reads(opcode, operand) == RXOP_COMPONENT_INTEGER &&
                      rxop_observed_components(opcode, operand) == RXOP_COMPONENT_INTEGER &&
                      rxop_observation_is_known(opcode, operand),
                  "checked integer source is a known integer observation", op);
        if (rxop_effect_writes_operand(&effects, operand))
            check(rxop_component_writes(opcode, operand) == RXOP_COMPONENT_INTEGER,
                  "checked integer destination defines only its integer component", op);
    }
}

static void check_checked_integer_family(void) {
    int opcode;
    for (opcode = OP_IADD_REG_REG_REG; opcode <= OP_DEC2; ++opcode)
        check_checked_integer_opcode(opcode);
    for (opcode = OP_IPOW_REG_REG_REG; opcode <= OP_ISEX_REG; ++opcode)
        check_checked_integer_opcode(opcode);
}

static void check_abi_contracts(void) {
    RxOpAbiContract entry = rxop_abi_contract(OP_INLENTER_REG_FUNC_STRING_ID, RXOP_ABI_ENTRY);
    RxOpAbiContract leave = rxop_abi_contract(OP_IADD_REG_REG_REG, RXOP_ABI_SCOPE_LEAVE);
    RxOpAbiContract reg = rxop_abi_contract(OP_RET_REG, RXOP_ABI_SCOPED_RETURN);
    RxOpAbiContract scalar = rxop_abi_contract(OP_RET_INT, RXOP_ABI_SCOPED_RETURN);
    RxOpAbiContract floating = rxop_abi_contract(OP_RET_FLOAT, RXOP_ABI_SCOPED_RETURN);
    RxOpAbiContract string = rxop_abi_contract(OP_RET_STRING, RXOP_ABI_PHYSICAL_RETURN);
    RxOpAbiContract physical = rxop_abi_contract(OP_RET, RXOP_ABI_PHYSICAL_RETURN);
    RxOpAbiContract absent = rxop_abi_contract(OP_ICOPY_REG_REG, RXOP_ABI_ENTRY);
    RxOpAbiContract unwind = rxop_abi_contract(OP_IADD_REG_REG_REG, RXOP_ABI_FRAME_UNWIND);
    RxOpAbiContract return_unwind = rxop_abi_contract(OP_RET_REG, RXOP_ABI_FRAME_UNWIND);
    size_t i;
    check(entry.state == RXOP_ABI_KNOWN && entry.count == 5u &&
              entry.steps[0].operation == RXOP_ABI_CAPTURE_POINTERS &&
              entry.steps[0].checkpoint == RXOP_ABI_ENTRY_CAPTURE &&
              entry.steps[0].rule == RXOP_ABI_VALIDATED_STAGE &&
              entry.steps[1].operation == RXOP_ABI_CLEAR_STORAGE &&
              entry.steps[1].destination == RXOP_ABI_RECIPE_ORIGINAL_BACKING &&
              entry.steps[1].destination_extent == RXOP_ABI_FULL_CAPACITY &&
              entry.steps[1].repeat == RXOP_ABI_EACH_OWNED &&
              entry.steps[2].operation == RXOP_ABI_PUBLISH_SCOPE &&
              entry.steps[3].destination == RXOP_ABI_A0_ORIGINAL_BACKING &&
              entry.steps[4].operation == RXOP_ABI_BIND_PROXY,
          "hidden entry captures before initialization and clears exact backing before publication", NULL);
    check(leave.state == RXOP_ABI_KNOWN && leave.count == 5u &&
              leave.steps[0].operation == RXOP_ABI_RESTORE_POLICY &&
              leave.steps[1].operation == RXOP_ABI_CLEAR_STORAGE &&
              leave.steps[1].destination == RXOP_ABI_SCOPE_ORIGINAL_BACKING &&
              leave.steps[1].destination_extent == RXOP_ABI_FULL_CAPACITY &&
              leave.steps[1].repeat == RXOP_ABI_OWNED_BEGIN &&
              leave.steps[2].operation == RXOP_ABI_RESTORE_SLOT &&
              leave.steps[2].repeat == RXOP_ABI_OWNED_END &&
              leave.steps[3].operation == RXOP_ABI_RESTORE_NUMERIC &&
              leave.steps[4].operation == RXOP_ABI_POP_SCOPE,
          "leave restores policy, interleaves each full-capacity cleanup/slot restore, then numeric and pop", NULL);
    if (leave.state == RXOP_ABI_KNOWN && leave.count == 5u) {
        unsigned int trace[4] = { 0u, 0u, 0u, 0u };
        size_t slot, row, used = 0u;
        /* Independent expansion expectation for the descriptor's paired
         * repetition: two slots must yield clear0,restore0,clear1,restore1. */
        for (slot = 0u; slot < 2u; ++slot)
            for (row = 1u; row <= 2u; ++row)
                trace[used++] = (unsigned int)(slot * 10u) +
                    (leave.steps[row].operation == RXOP_ABI_CLEAR_STORAGE ? 1u : 2u);
        check(trace[0] == 1u && trace[1] == 2u && trace[2] == 11u && trace[3] == 12u,
              "ABI owned repetition is a paired body, never clear-all then restore-all", NULL);
    }
    check(reg.state == RXOP_ABI_KNOWN && reg.count == leave.count + 1u &&
              reg.steps[0].operation == RXOP_ABI_RETURN_REGISTER &&
              reg.steps[0].source == RXOP_ABI_RETURN_ACTIVE &&
              reg.steps[0].destination == RXOP_ABI_CAPTURED_RESULT &&
              reg.steps[0].rule == RXOP_ABI_OWNED_BASE_MOVE_ELSE_COPY &&
              reg.steps[0].source_extent == RXOP_ABI_ACTIVE_TREE &&
              reg.steps[0].destination_extent == RXOP_ABI_FULL_CAPACITY,
          "register return uses current source and exact owned-original-base MOVE/COPY rule", NULL);
    if (reg.state == RXOP_ABI_KNOWN && reg.count == leave.count + 1u)
        for (i = 0u; i < leave.count; ++i)
            check(reg.steps[i + 1u].operation == leave.steps[i].operation &&
                      reg.steps[i + 1u].checkpoint == leave.steps[i].checkpoint,
                  "same-storage or ignored result omits only transfer, never later scope cleanup", NULL);
    check(scalar.state == RXOP_ABI_KNOWN && floating.state == RXOP_ABI_KNOWN &&
              scalar.steps[0].operation == RXOP_ABI_RETURN_INTEGER &&
              floating.steps[0].operation == RXOP_ABI_RETURN_FLOAT &&
              scalar.steps[0].destination_extent == RXOP_ABI_EXTENT_NONE &&
              floating.steps[0].destination_extent == RXOP_ABI_EXTENT_NONE &&
              string.state == RXOP_ABI_KNOWN && string.steps[0].operation == RXOP_ABI_RETURN_STRING &&
              string.steps[0].destination_extent == RXOP_ABI_TOP,
          "direct numeric return setters differ from native-payload-releasing string setter", NULL);
    check(physical.state == RXOP_ABI_KNOWN && physical.count == 1u &&
              physical.steps[0].operation == RXOP_ABI_RETIRE_FRAME &&
              physical.steps[0].rule == RXOP_ABI_INTERNAL_PARENT &&
              physical.steps[0].destination_extent == RXOP_ABI_EXTENT_NONE,
          "ordinary physical retirement retains reusable backing, unlike scoped clear", NULL);
    check(unwind.state == RXOP_ABI_KNOWN && unwind.count == 1u &&
              unwind.steps[0].operation == RXOP_ABI_RETIRE_FRAME &&
              unwind.steps[0].checkpoint == RXOP_ABI_PHYSICAL_RETIRE &&
              unwind.steps[0].source == RXOP_ABI_PHYSICAL_FRAME &&
              unwind.steps[0].destination == RXOP_ABI_ROLE_NONE &&
              unwind.steps[0].source_extent == RXOP_ABI_EXTENT_NONE &&
              unwind.steps[0].destination_extent == RXOP_ABI_EXTENT_NONE &&
              unwind.steps[0].rule == RXOP_ABI_INTERNAL_PARENT &&
              unwind.steps[0].repeat == RXOP_ABI_ONCE &&
              return_unwind.state == RXOP_ABI_KNOWN && return_unwind.count == 1u &&
              return_unwind.steps[0].operation == RXOP_ABI_RETIRE_FRAME,
          "nested unwind retires conditionally without result transfer, numeric restore or backing clear", NULL);
    check(absent.state == RXOP_ABI_NONE && absent.count == 0u &&
              rxop_abi_contract(-1, RXOP_ABI_FRAME_UNWIND).state == RXOP_ABI_UNKNOWN &&
              rxop_abi_contract(OP_RESERVED_088, RXOP_ABI_FRAME_UNWIND).state == RXOP_ABI_UNKNOWN &&
              rxop_abi_contract(-1, RXOP_ABI_ENTRY).state == RXOP_ABI_UNKNOWN &&
              rxop_abi_contract(OP_RET, RXOP_ABI_MODE_COUNT).state == RXOP_ABI_UNKNOWN &&
              rxop_abi_contract(OP_RESERVED_088, RXOP_ABI_ENTRY).state == RXOP_ABI_UNKNOWN &&
              !strcmp(rxop_abi_mode_name(RXOP_ABI_SCOPE_LEAVE), "scope-leave") &&
              !strcmp(rxop_abi_checkpoint_name(RXOP_ABI_LEAVE_POLICY), "leave-policy") &&
              !strcmp(rxop_abi_repeat_name(RXOP_ABI_OWNED_BEGIN), "owned-begin") &&
              !strcmp(rxop_abi_role_name((RxOpAbiRole)-1), "unknown"),
          "ABI absence is distinct from invalid instructions; nested rows retain their parent premise", NULL);
}

static void check_value_writer_inventory(void) {
    static const int stem_outputs[] = {
        OP_STEMGET_REG_REG_REG, OP_STEMGET2_REG_REG_REG_REG,
        OP_STEMKEYAT_REG_REG_REG, OP_STEMVALUEAT_REG_REG_REG
    };
    int opcode;
    size_t setter_count = 0, field_count = 0, stem;
    for (opcode = 0; opcode < OP_MAX_INSTRUCTIONS; ++opcode) {
        RxOpEffects effects = rxop_effects(opcode);
        size_t operand, count = rxop_format_operand_count(op_table[opcode].format);
        const char *signature = effects.value_writers_signature;
        if (signature) {
            check(strlen(signature) == count, "writer signature covers the complete explicit operand vector", &op_table[opcode]);
            for (operand = 0; operand < strlen(signature); ++operand)
                check(strchr("0fs?", signature[operand]) != NULL,
                      "writer signature contains only canonical kinds", &op_table[opcode]);
        }
        for (operand = 0; operand < count; ++operand) {
            RxOpValueWriter writer = rxop_value_writer(opcode, operand);
            if (!rxop_effect_writes_operand(&effects, operand)) {
                if (effects.state == RXOP_EFFECT_CLASSIFIED)
                    check(writer == RXOP_VALUE_WRITER_NONE,
                          "unwritten explicit operand has no value writer", &op_table[opcode]);
                if (signature && operand < strlen(signature))
                    check(signature[operand] == '0', "writer annotation cannot create an explicit output", &op_table[opcode]);
            }
            if (writer == RXOP_VALUE_WRITER_SETTER) {
                ++setter_count;
                check(rxop_effect_writes_operand(&effects, operand) &&
                          (rxop_component_clears(opcode, operand) &
                           (RXOP_COMPONENT_REFERENCE | RXOP_COMPONENT_NATIVE_PAYLOAD)) ==
                              (RXOP_COMPONENT_REFERENCE | RXOP_COMPONENT_NATIVE_PAYLOAD) &&
                          (rxop_callback_outputs(opcode, operand) & RXOP_COMPONENT_NATIVE_PAYLOAD),
                      "shared setter derives reference/native prewrite release and possible finalizer", &op_table[opcode]);
            } else if (writer == RXOP_VALUE_WRITER_FIELD) {
                ++field_count;
                check(!(rxop_component_clears(opcode, operand) &
                        (RXOP_COMPONENT_REFERENCE | RXOP_COMPONENT_NATIVE_PAYLOAD)),
                      "field/copy writer does not invent an unrelated payload release", &op_table[opcode]);
            }
        }
    }
    check(setter_count >= 200u && field_count >= 25u,
          "VM setter inventory covers scalar/string setters and independent field controls", NULL);
    for (stem = 0; stem < sizeof(stem_outputs) / sizeof(stem_outputs[0]); ++stem) {
        opcode = stem_outputs[stem];
        check(rxop_value_writer(opcode, 0) == RXOP_VALUE_WRITER_UNKNOWN &&
                  rxop_component_clears(opcode, 0) == RXOP_COMPONENT_NONE &&
                  (rxop_callback_outputs(opcode, 0) & RXOP_COMPONENT_NATIVE_PAYLOAD),
              "selected stem child can alias its output: possible release is not a must clear",
              &op_table[opcode]);
    }
    check(rxop_value_writer(OP_IAND_REG_REG_INT, 0) == RXOP_VALUE_WRITER_SETTER &&
              rxop_value_writer(OP_IAND_REG_REG_REG, 0) == RXOP_VALUE_WRITER_SETTER &&
              rxop_value_writer(OP_LOAD_REG_INT, 0) == RXOP_VALUE_WRITER_SETTER &&
              rxop_value_writer(OP_LOAD_REG_FLOAT, 0) == RXOP_VALUE_WRITER_SETTER &&
              rxop_value_writer(OP_LOAD_REG_STRING, 0) == RXOP_VALUE_WRITER_SETTER &&
              rxop_value_writer(OP_SETLINKILOAD_REG_REG_INT_REG_REG_INT, 4) == RXOP_VALUE_WRITER_SETTER,
          "representative scalar/string and non-first shared setters have authoritative writer kinds", NULL);
    check(rxop_value_writer(OP_STRLEN_REG_REG, 0) == RXOP_VALUE_WRITER_FIELD &&
              rxop_value_writer(OP_GETNUMDGTS_REG, 0) == RXOP_VALUE_WRITER_FIELD &&
              rxop_value_writer(OP_ICOPY_REG_REG, 0) == RXOP_VALUE_WRITER_FIELD &&
              !rxop_callback_outputs(OP_STRLEN_REG_REG, 0) &&
              !rxop_callback_outputs(OP_GETNUMDGTS_REG, 0) &&
              !rxop_callback_outputs(OP_ICOPY_REG_REG, 0),
          "reviewed raw field writes remain distinct from shared setters", NULL);
    check(rxop_value_writer(OP_METALOADDATA_REG_REG_REG, 0) == RXOP_VALUE_WRITER_UNKNOWN &&
              rxop_value_writer(OP_STRUPPER_REG_REG, 0) == RXOP_VALUE_WRITER_UNKNOWN &&
              rxop_callback_outputs(OP_STRUPPER_REG_REG, 0) &&
              rxop_value_writer(-1, 0) == RXOP_VALUE_WRITER_UNKNOWN &&
              rxop_value_writer(OP_LOAD_REG_INT, 9) == RXOP_VALUE_WRITER_UNKNOWN &&
              rxop_signal_contract(OP_IAND_REG_REG_INT).state == RXOP_SIGNAL_STATE_UNKNOWN,
          "hidden, alias-conditional and unreviewed writers retain uncertainty without changing signal contracts", NULL);
}

static void check_output_intrinsic_footprints(void) {
    static const int outputs[] = {
        OP_SAY_REG, OP_SAYX_REG, OP_SAYX_STRING, OP_SAY_INT,
        OP_SAY_FLOAT, OP_SAY_STRING, OP_SAY_CHAR
    };
    size_t i;
    for (i = 0; i < sizeof(outputs) / sizeof(outputs[0]); ++i) {
        int opcode = outputs[i];
        int register_input = opcode == OP_SAY_REG || opcode == OP_SAYX_REG;
        RxOpEffects effects = rxop_effects(opcode);
        check(rxop_intrinsic_value_footprint_is_known(opcode) &&
                  effects.state == RXOP_EFFECT_CLASSIFIED &&
                  effects.implicit == RXOP_IMPLICIT_NONE &&
                  rxop_effect_reads_operand(&effects, 0) == register_input &&
                  !rxop_effect_writes_operand(&effects, 0) &&
                  !rxop_effect_kills_operand(&effects, 0) &&
                  rxop_component_may_reads(opcode, 0) ==
                      (register_input ? RXOP_COMPONENT_STRING : RXOP_COMPONENT_NONE) &&
                  rxop_observed_components(opcode, 0) ==
                      (register_input ? RXOP_COMPONENT_STRING : RXOP_COMPONENT_NONE) &&
                  rxop_observation_is_known(opcode, 0) &&
                  rxop_observed_flags(opcode, 0) == 0 &&
                  rxop_component_may_writes(opcode, 0) == RXOP_COMPONENT_NONE,
              "output intrinsic value footprint is complete and observes only its typed input",
              &op_table[opcode]);
        check((effects.semantics & RXOP_SEM_OPAQUE) &&
                  rxop_host_callbacks(opcode) && rxop_residual_observable(opcode) &&
                  rxop_signal_contract(opcode).state == RXOP_SIGNAL_STATE_UNKNOWN,
              "known intrinsic output footprint retains opaque host, signal and observable residuals",
              &op_table[opcode]);
    }
    check(!rxop_intrinsic_value_footprint_is_known(-1) &&
              !rxop_intrinsic_value_footprint_is_known(OP_MAX_INSTRUCTIONS) &&
              !rxop_intrinsic_value_footprint_is_known(OP_RESERVED_088) &&
              !rxop_intrinsic_value_footprint_is_known(OP_READLINE_REG) &&
              !rxop_intrinsic_value_footprint_is_known(OP_CALL_FUNC),
          "unclassified external footprints remain unknown rather than empty", NULL);
}

static void check_callback_execution_contracts(void) {
    int opcode;
    RxOpSignalContract intrinsic, execution;
    unsigned int obligations;
    for (opcode = 0; opcode < OP_MAX_INSTRUCTIONS; opcode++) {
        size_t operand;
        size_t count = rxop_format_operand_count(op_table[opcode].format);
        RxOpEffects effects = rxop_effects(opcode);
        const char *recipe = rxop_binding_recipe(opcode);
        obligations = rxop_callback_obligations(opcode);
        intrinsic = rxop_signal_contract(opcode);
        execution = rxop_execution_signal_contract(opcode);
        if (!effects.callback_inventory_complete && (!recipe || !recipe[0]))
            check(obligations & RXOP_CALLBACK_UNKNOWN,
                  "missing callback inventory is not empty", &op_table[opcode]);
        for (operand = 0; operand < count; operand++) {
            if (rxop_callback_inputs(opcode, operand))
                check(obligations & RXOP_CALLBACK_INPUT,
                      "copy-source resource hooks remain obligations", &op_table[opcode]);
            if (rxop_callback_outputs(opcode, operand))
                check(obligations & RXOP_CALLBACK_OUTPUT,
                      "destination resource hooks remain obligations", &op_table[opcode]);
            if (rxop_callback_capacity(opcode, operand))
                check(obligations & RXOP_CALLBACK_CAPACITY,
                      "retained capacity is not an active-count-only condition", &op_table[opcode]);
        }
        if (obligations)
            check(execution.state == RXOP_SIGNAL_STATE_UNKNOWN &&
                      execution.phase == RXOP_SIGNAL_PHASE_UNKNOWN &&
                      execution.source == RXOP_SIGNAL_SOURCE_UNKNOWN &&
                      execution.continuations == RXOP_SIGNAL_CONT_ALL &&
                      execution.policy_effect == RXOP_POLICY_EFFECT_UNKNOWN,
                  "possible hook carries independent unknown execution control", &op_table[opcode]);
        else
            check(execution.state == intrinsic.state && execution.phase == intrinsic.phase &&
                      execution.source == intrinsic.source &&
                      execution.continuations == intrinsic.continuations,
                  "complete no-hook inventory preserves intrinsic execution control", &op_table[opcode]);
    }
    {
        static const int pure[] = {
            OP_BPON_FUNC, OP_BPON, OP_BPOFF, OP_SIGIGNORE_STRING,
            OP_SIGHALT_STRING, OP_SIGSHALT_STRING, OP_SIGBR_ID_STRING,
            OP_SIGBRV_ID_REG_STRING, OP_SIGCALL_FUNC_STRING,
            OP_SIGCALLA_FUNC_STRING, OP_SIGCALLBR_ID_FUNC_STRING,
            OP_SIGRET_STRING, OP_SIGPUSH_STRING, OP_SIGPOP_STRING,
            OP_INC0, OP_DEC0, OP_INC1, OP_DEC1, OP_INC2, OP_DEC2,
            OP_ITOF_REG_REG, OP_SUBSTRING_REG_REG_REG_REG,
            OP_SETTPSWAP_REG_INT_REG, OP_IGETUNLINK_REG_REG,
            OP_ISETUNLINK_REG_REG, OP_ISETUNLINKN_REG_REG_REG,
            OP_LINKATTR_REG_REG_INT, OP_LINKATTR1_REG_REG_INT,
            OP_LINKTOATTR_REG_REG_REG, OP_UNLINKATTR_REG_REG
        };
        size_t index;
        for (index = 0; index < sizeof(pure) / sizeof(pure[0]); index++) {
            int code = pure[index];
            intrinsic = rxop_signal_contract(code);
            execution = rxop_execution_signal_contract(code);
            check(!rxop_callback_obligations(code) &&
                      execution.state == intrinsic.state &&
                      execution.phase == intrinsic.phase &&
                      execution.policy_effect == intrinsic.policy_effect,
                  "reviewed COW policy/raw-field/pointer recipe has no hidden value hook", &op_table[code]);
        }
        check(rxop_callback_obligations(OP_CONCAT_REG_STRING_REG) &&
                  rxop_callback_obligations(OP_ILOADSETUNLINK_REG_INT) &&
                  rxop_callback_obligations(OP_SETATTRS_REG_INT) &&
                  rxop_callback_obligations(OP_NULL_REG),
              "alias-sensitive buffer replacement, setter and resize cleanup remain conditional", NULL);
    }
    intrinsic = rxop_signal_contract(OP_LOAD_REG_INT);
    execution = rxop_execution_signal_contract(OP_LOAD_REG_INT);
    check(intrinsic.state == RXOP_SIGNAL_STATE_NONE &&
              rxop_callback_obligations(OP_LOAD_REG_INT) == RXOP_CALLBACK_OUTPUT &&
              execution.state == RXOP_SIGNAL_STATE_UNKNOWN,
          "intrinsic-NONE scalar setter still permits operation-caused callback control", NULL);
    check(!rxop_callback_obligations(OP_ITOS_REG) &&
              !rxop_callback_obligations(OP_FTOS_REG) &&
              !rxop_callback_obligations(OP_GETNUMDGTS_REG) &&
              !rxop_callback_obligations(OP_ICOPY_REG_REG) &&
              !rxop_callback_obligations(OP_SWAP_REG_REG),
          "reviewed field/formatting/context/pointer operations preserve no-hook positives", NULL);
    check((rxop_callback_obligations(OP_COPY_REG_REG) & RXOP_CALLBACK_INPUT) &&
              (rxop_callback_obligations(OP_COPY_REG_REG) & RXOP_CALLBACK_CAPACITY) &&
              (rxop_callback_obligations(OP_SAYX_STRING) & RXOP_CALLBACK_HOST) &&
              (rxop_callback_obligations(OP_CALL_FUNC) & RXOP_CALLBACK_INVOCATION) &&
              (rxop_callback_obligations(OP_INLENTER_REG_FUNC_STRING_ID) & RXOP_CALLBACK_ABI),
          "input, capacity and subjectless routes cannot be silently discharged", NULL);
    check((rxop_callback_obligations(OP_SWAPCALL_REG_FUNC_REG_REG_REG) & RXOP_CALLBACK_INVOCATION) &&
              rxop_execution_signal_contract(OP_SWAPCALL_REG_FUNC_REG_REG_REG).state == RXOP_SIGNAL_STATE_UNKNOWN,
          "pointer-only call prefix cannot suppress the fused invocation hook", NULL);
    check(rxop_callback_postwrite_operand(OP_LOAD_REG_INT) == 1u &&
              rxop_callback_postwrite_operand(OP_LOAD_REG_FLOAT) == 1u &&
              rxop_callback_postwrite_operand(OP_LOAD_REG_STRING) == 1u &&
              rxop_callback_postwrite_operand(OP_IAND_REG_REG_INT) == 1u &&
              !rxop_callback_postwrite_operand(OP_COPY_REG_REG) &&
              !rxop_callback_postwrite_operand(OP_NULL_REG) &&
              !rxop_callback_postwrite_operand(OP_CONCAT_REG_STRING_REG) &&
              !rxop_callback_postwrite_operand(OP_SWAPCALL_REG_FUNC_REG_REG_REG) &&
              !rxop_callback_postwrite_operand(OP_INLENTER_REG_FUNC_STRING_ID),
          "ordered captured postwrite profile requires one complete explicit setter", NULL);
    check(rxop_callback_normal_preserves_frame(OP_LOAD_REG_INT) &&
              rxop_callback_normal_preserves_frame(OP_COPY_REG_REG) &&
              !rxop_callback_normal_preserves_frame(OP_SAYX_STRING) &&
              !rxop_callback_normal_preserves_frame(OP_SWAPCALL_REG_FUNC_REG_REG_REG) &&
              !rxop_callback_normal_preserves_frame(OP_INLENTER_REG_FUNC_STRING_ID) &&
              !rxop_callback_normal_preserves_frame(-1),
          "normal callback frame-table premise is limited to complete public resource routes", NULL);
    check(rxop_callback_obligations(-1) == RXOP_CALLBACK_UNKNOWN &&
              rxop_callback_obligations(OP_MAX_INSTRUCTIONS) == RXOP_CALLBACK_UNKNOWN &&
              (rxop_callback_obligations(OP_RESERVED_088) & RXOP_CALLBACK_UNKNOWN),
          "invalid and reserved callback inventories fail closed", NULL);
}

int main(void) {
    int i;
    int source_count;
    int classified_count;
    int conservative_count;
    int reserved_count;
    int internal_count;
    int clone_categories[RXOP_CLONE_RETAINED + 1] = {0};
    int unknown_clone_signals = 0, unknown_clone_writes = 0;
    unsigned int legal_registers;
    unsigned int legal_labels;
    unsigned int legal_semantics;
    RxOpEffects effects;
    RxOpSignalContract signal;
    RxOpCompareBranchFusion fusion;
    const OpInfo *op;

    failures = 0;
    check_value_writer_inventory();
    check_output_intrinsic_footprints();
    check_callback_execution_contracts();
    check_abi_contracts();
    check_checked_integer_family();
    check(rxop_compare_branch_fusion(
                  OP_IEQ_REG_REG_REG, OP_BRT_ID_REG, &fusion) &&
              fusion.fused_opcode == OP_BEQ_ID_REG_REG &&
              fusion.left_source_operand == 1 &&
              fusion.right_source_operand == 2,
          "integer equality branch-fusion metadata regression", NULL);
    check(rxop_compare_branch_fusion(
                  OP_IGT_REG_INT_REG, OP_BRF_ID_REG, &fusion) &&
              fusion.fused_opcode == OP_BGE_ID_REG_INT &&
              fusion.left_source_operand == 2 &&
              fusion.right_source_operand == 1,
          "reversed integer branch-fusion metadata regression", NULL);
    check(!rxop_compare_branch_fusion(
                  OP_FEQ_REG_REG_REG, OP_BRT_ID_REG, &fusion),
          "non-integer comparison must not acquire integer fusion metadata",
          NULL);
    signal = rxop_signal_contract(OP_BGT_ID_REG_REG);
    check(signal.state == RXOP_SIGNAL_STATE_NONE &&
              rxop_signal_contract(OP_BGT_ID_REG_INT).state ==
                    RXOP_SIGNAL_STATE_NONE &&
              rxop_signal_contract(OP_BGE_ID_REG_REG).state ==
                    RXOP_SIGNAL_STATE_NONE &&
              rxop_signal_contract(OP_BGE_ID_REG_INT).state ==
                    RXOP_SIGNAL_STATE_NONE &&
              rxop_signal_contract(OP_BLT_ID_REG_REG).state ==
                    RXOP_SIGNAL_STATE_NONE &&
              rxop_signal_contract(OP_BLT_ID_REG_INT).state ==
                    RXOP_SIGNAL_STATE_NONE &&
              rxop_signal_contract(OP_BLE_ID_REG_REG).state ==
                    RXOP_SIGNAL_STATE_NONE &&
              rxop_signal_contract(OP_BLE_ID_REG_INT).state ==
                    RXOP_SIGNAL_STATE_NONE &&
              rxop_component_reads(OP_BGT_ID_REG_REG, 1) ==
                    RXOP_COMPONENT_INTEGER &&
              rxop_component_reads(OP_BGT_ID_REG_REG, 2) ==
                    RXOP_COMPONENT_INTEGER,
          "fused integer branches must be exact and non-signalling",
          &op_table[OP_BGT_ID_REG_REG]);
    check_same_storage_copy_metadata(OP_COPY_REG_REG);
    check_same_storage_copy_metadata(OP_ICOPY_REG_REG);
    check_same_storage_copy_metadata(OP_FCOPY_REG_REG);
    check_same_storage_copy_metadata(OP_SCOPY_REG_REG);
    check_same_storage_copy_metadata(OP_DCOPY_REG_REG);
    check_same_storage_copy_metadata(OP_ACOPY_REG_REG);
    check_same_storage_copy_metadata(OP_BCOPY_REG_REG);
    check(rxop_component_reads(OP_COPY_REG_REG, 1) ==
                  RXOP_COMPONENT_ALL &&
              rxop_component_writes(OP_COPY_REG_REG, 0) ==
                  RXOP_COMPONENT_ALL &&
              rxop_component_reads(OP_ACOPY_REG_REG, 1) ==
                  RXOP_COMPONENT_ATTRIBUTES &&
              rxop_component_writes(OP_ACOPY_REG_REG, 0) ==
                  RXOP_COMPONENT_ATTRIBUTES &&
              (rxop_component_writes(OP_COPY_REG_REG, 0) &
               rxop_component_writes(OP_ACOPY_REG_REG, 0)) ==
                  rxop_component_writes(OP_ACOPY_REG_REG, 0) &&
              rxop_signal_contract(OP_COPY_REG_REG).state ==
                  RXOP_SIGNAL_STATE_NONE &&
              rxop_signal_contract(OP_ACOPY_REG_REG).state ==
                  RXOP_SIGNAL_STATE_NONE,
          "full copy must subsume adjacent same-pair status copy",
          &op_table[OP_COPY_REG_REG]);
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
              (RXOP_COMPONENT_ALL & RXOP_COMPONENT_ATTRIBUTE_COUNT) != 0 &&
              (RXOP_COMPONENT_ALL & RXOP_COMPONENT_OBJECT_TYPE) != 0 &&
              rxop_component_clears(OP_LOAD_REG_INT, 0) ==
                  (RXOP_COMPONENT_REFERENCE |
                   RXOP_COMPONENT_NATIVE_PAYLOAD) &&
              rxop_component_clears(OP_LOAD_REG_FLOAT, 0) ==
                  (RXOP_COMPONENT_REFERENCE |
                   RXOP_COMPONENT_NATIVE_PAYLOAD) &&
              rxop_component_clears(OP_IEQ_REG_REG_INT, 0) ==
                  (RXOP_COMPONENT_REFERENCE |
                   RXOP_COMPONENT_NATIVE_PAYLOAD) &&
              rxop_component_clears(OP_FEQ_REG_REG_FLOAT, 0) ==
                  (RXOP_COMPONENT_REFERENCE |
                   RXOP_COMPONENT_NATIVE_PAYLOAD) &&
              rxop_component_clears(OP_ICOPY_REG_REG, 0) ==
                  RXOP_COMPONENT_NONE &&
              rxop_component_clears(OP_FCOPY_REG_REG, 0) ==
                  RXOP_COMPONENT_NONE &&
              rxop_component_clears(OP_LOAD_REG_INT, 1) ==
                  RXOP_COMPONENT_NONE,
          "scalar producer cleanup-component metadata regression", NULL);
    check(rxop_component_reads(OP_BCOPY_REG_REG, 1) ==
                  (RXOP_COMPONENT_BINARY |
                   RXOP_COMPONENT_NATIVE_PAYLOAD) &&
              rxop_component_writes(OP_BCOPY_REG_REG, 0) ==
                  (RXOP_COMPONENT_BINARY |
                   RXOP_COMPONENT_NATIVE_PAYLOAD),
          "binary copy native-payload metadata regression",
          &op_table[OP_BCOPY_REG_REG]);
    signal = rxop_signal_contract(OP_STRLEN_REG_REG);
    check(rxop_component_reads(OP_STRLEN_REG_REG, 1) ==
                  RXOP_COMPONENT_STRING &&
              rxop_component_writes(OP_STRLEN_REG_REG, 0) ==
                  RXOP_COMPONENT_INTEGER &&
              rxop_component_clears(OP_STRLEN_REG_REG, 0) ==
                  RXOP_COMPONENT_NONE &&
              signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES &&
              signal.source == RXOP_SIGNAL_SOURCE_STATIC_NAMES &&
              signal.static_names &&
              strcmp(signal.static_names, "UNICODE_ERROR") == 0 &&
              signal.failure_writes == RXOP_OP_NONE &&
              signal.failure_component_writes == RXOP_COMPONENT_NONE &&
              signal.failure_context_writes == RXOP_CONTEXT_NONE &&
              signal.dependencies == RXOP_SIGNAL_DEP_NONE &&
              (signal.properties & RXOP_SIGNAL_PROP_SUCCESS_STABLE),
          "strlen component/failure-atomic signal metadata regression",
          &op_table[OP_STRLEN_REG_REG]);
    signal = rxop_signal_contract(OP_ISUB_REG_REG_INT);
    check(signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES &&
              signal.source == RXOP_SIGNAL_SOURCE_STATIC_NAMES &&
              signal.static_names &&
              strcmp(signal.static_names, "OVERFLOW_UNDERFLOW") == 0 &&
              signal.failure_writes == RXOP_OP_NONE &&
              signal.failure_component_writes == RXOP_COMPONENT_NONE &&
              signal.failure_context_writes == RXOP_CONTEXT_NONE &&
              signal.dependencies == RXOP_SIGNAL_DEP_NONE &&
              (signal.properties & RXOP_SIGNAL_PROP_SUCCESS_STABLE) &&
              rxop_signal_contract(OP_ISUB_REG_REG_REG).phase ==
                    RXOP_SIGNAL_PHASE_BEFORE_WRITES &&
              rxop_signal_contract(OP_ISUB_REG_INT_REG).phase ==
                    RXOP_SIGNAL_PHASE_BEFORE_WRITES &&
              rxop_component_clears(OP_ISUB_REG_REG_REG, 0) ==
                    (RXOP_COMPONENT_REFERENCE |
                     RXOP_COMPONENT_NATIVE_PAYLOAD) &&
              rxop_component_clears(OP_ISUB_REG_REG_INT, 0) ==
                    (RXOP_COMPONENT_REFERENCE |
                     RXOP_COMPONENT_NATIVE_PAYLOAD) &&
              rxop_component_clears(OP_ISUB_REG_INT_REG, 0) ==
                    (RXOP_COMPONENT_REFERENCE |
                     RXOP_COMPONENT_NATIVE_PAYLOAD),
          "integer subtract component/failure-atomic signal metadata regression",
          &op_table[OP_ISUB_REG_REG_INT]);
    check(rxop_component_reads(OP_REQ_REG_REG_REG, 1) ==
                  RXOP_COMPONENT_STRING &&
              rxop_component_reads(OP_REQ_REG_REG_REG, 2) ==
                  RXOP_COMPONENT_STRING &&
              rxop_component_reads(OP_RLTE_REG_STRING_REG, 2) ==
                  RXOP_COMPONENT_STRING &&
              rxop_component_writes(OP_REQ_REG_REG_STRING, 0) ==
                  RXOP_COMPONENT_INTEGER &&
              rxop_component_writes(OP_RLTE_REG_STRING_REG, 0) ==
                  RXOP_COMPONENT_INTEGER &&
              rxop_component_clears(OP_REQ_REG_REG_STRING, 0) ==
                    (RXOP_COMPONENT_REFERENCE |
                     RXOP_COMPONENT_NATIVE_PAYLOAD) &&
              rxop_component_clears(OP_RLTE_REG_STRING_REG, 0) ==
                    (RXOP_COMPONENT_REFERENCE |
                     RXOP_COMPONENT_NATIVE_PAYLOAD) &&
              rxop_signal_contract(OP_REQ_REG_REG_STRING).state ==
                    RXOP_SIGNAL_STATE_NONE &&
              rxop_signal_contract(OP_RLTE_REG_STRING_REG).state ==
                    RXOP_SIGNAL_STATE_NONE,
          "loose string comparison component/signal metadata regression",
          &op_table[OP_REQ_REG_REG_STRING]);
    check(rxop_component_reads(OP_DADD_REG_REG_REG, 1) ==
                  RXOP_COMPONENT_DECIMAL &&
              rxop_component_reads(OP_DMULT_REG_REG_DECIMAL, 1) ==
                  RXOP_COMPONENT_DECIMAL &&
              rxop_component_writes(OP_DADD_REG_REG_REG, 0) ==
                  RXOP_COMPONENT_DECIMAL &&
              rxop_component_writes(OP_DMULT_REG_REG_DECIMAL, 0) ==
                  RXOP_COMPONENT_DECIMAL,
          "decimal arithmetic component metadata regression",
          &op_table[OP_DADD_REG_REG_REG]);
    check(rxop_component_reads(OP_STEMGET_REG_REG_REG, 2) ==
                  RXOP_COMPONENT_STRING &&
              rxop_component_reads(OP_STEMSET_REG_REG_REG, 1) ==
                  RXOP_COMPONENT_STRING &&
              rxop_component_reads(OP_STEMSET_REG_REG_REG, 2) ==
                  RXOP_COMPONENT_STRING &&
              rxop_component_reads(OP_STEMSET2_REG_REG_REG_REG, 3) ==
                  RXOP_COMPONENT_STRING &&
              rxop_component_writes(OP_STEMGET_REG_REG_REG, 0) ==
                  RXOP_COMPONENT_STRING &&
              rxop_value_writer(OP_STEMGET_REG_REG_REG, 0) == RXOP_VALUE_WRITER_UNKNOWN &&
              rxop_component_clears(OP_STEMGET_REG_REG_REG, 0) == RXOP_COMPONENT_NONE &&
              (rxop_callback_outputs(OP_STEMGET_REG_REG_REG, 0) & RXOP_COMPONENT_NATIVE_PAYLOAD),
          "native stem string write retains alias-conditional payload release",
          &op_table[OP_STEMGET_REG_REG_REG]);
    check(rxop_component_writes(OP_SETATTRS_REG_INT, 0) ==
                  RXOP_COMPONENT_ATTRIBUTE_COUNT &&
              rxop_component_reads(OP_LINKATTR1_REG_REG_INT, 1) ==
                  RXOP_COMPONENT_ATTRIBUTE_COUNT &&
              rxop_component_reads(OP_GETATTRS_REG_REG, 1) ==
                  RXOP_COMPONENT_ATTRIBUTE_COUNT,
          "attribute-count component metadata regression",
          &op_table[OP_SETATTRS_REG_INT]);
    check(rxop_signal_contract(OP_SETATTRS_REG_INT).state ==
                  RXOP_SIGNAL_STATE_NONE &&
              rxop_signal_contract(OP_BCOPY_REG_REG).state ==
                  RXOP_SIGNAL_STATE_NONE,
          "total SETATTRS/BCOPY signal metadata regression",
          &op_table[OP_SETATTRS_REG_INT]);

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
        else if (signal.source == RXOP_SIGNAL_SOURCE_LITERAL_OPERAND ||
                 signal.source == RXOP_SIGNAL_SOURCE_REGISTER_OPERAND)
            check(signal.static_names == NULL || signal.static_names[0],
                  "operand-selected signal validation alternatives are empty", op);
        else
            check(signal.static_names == NULL,
                  "non-operand signal contract carries unexpected static alternatives", op);
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
        } else if (signal.properties & RXOP_SIGNAL_PROP_PENDING_WRITE) {
            check(signal.policy_effect == RXOP_POLICY_EFFECT_BREAKPOINT_DISABLE &&
                      signal.policy_source == RXOP_SIGNAL_SOURCE_STATIC_NAMES &&
                      signal.policy_source_operand == SIZE_MAX && signal.policy_static_name &&
                      strcmp(signal.policy_static_name, "BREAKPOINT") == 0 &&
                      signal.dependencies == RXOP_SIGNAL_DEP_NONE,
                  "pending-only control carries an invalid effect or structural dependency", op);
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
            check_clone_contract(op, clone_categories, &unknown_clone_signals,
                                 &unknown_clone_writes);
            check_target_contract(op);
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
    effects = rxop_effects(OP_ASSERTINITIALIZED_REG);
    signal = rxop_signal_contract(OP_ASSERTINITIALIZED_REG);
    check(effects.reads == RXOP_OP_1 &&
              effects.writes == RXOP_OP_NONE &&
              effects.semantics == RXOP_SEM_NONE &&
              effects.optimizer_barrier &&
              rxop_component_reads(OP_ASSERTINITIALIZED_REG, 0) ==
                    RXOP_COMPONENT_OBJECT_TYPE &&
              signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES &&
              signal.static_names &&
              strcmp(signal.static_names, "OBJECT_NOT_INITIALIZED") == 0 &&
              (signal.properties & RXOP_SIGNAL_PROP_SUCCESS_STABLE),
          "initialized guard contract regression",
          &op_table[OP_ASSERTINITIALIZED_REG]);
    signal = rxop_signal_contract(OP_ASSERTTYPE_REG_STRING);
    check(rxop_component_reads(OP_ASSERTTYPE_REG_STRING, 0) ==
                    RXOP_COMPONENT_OBJECT_TYPE &&
              signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.static_names &&
              strcmp(signal.static_names, "CONVERSION_ERROR") == 0 &&
              (signal.properties & RXOP_SIGNAL_PROP_SUCCESS_STABLE),
          "type guard contract regression",
          &op_table[OP_ASSERTTYPE_REG_STRING]);
    effects = rxop_effects(OP_ICHKRNG_REG_REG_REG);
    signal = rxop_signal_contract(OP_ICHKRNG_REG_REG_REG);
    check(effects.semantics == RXOP_SEM_NONE &&
              effects.optimizer_barrier &&
              rxop_component_reads(OP_ICHKRNG_REG_REG_REG, 0) ==
                    RXOP_COMPONENT_INTEGER &&
              rxop_component_reads(OP_ICHKRNG_REG_REG_REG, 2) ==
                    RXOP_COMPONENT_INTEGER &&
              signal.static_names &&
              strcmp(signal.static_names, "OUT_OF_RANGE") == 0 &&
              (signal.properties & RXOP_SIGNAL_PROP_SUCCESS_STABLE),
          "integer range guard contract regression",
          &op_table[OP_ICHKRNG_REG_REG_REG]);
    effects = rxop_effects(OP_BCHECKRANGE_REG_REG_REG);
    signal = rxop_signal_contract(OP_BCHECKRANGE_REG_REG_REG);
    check(effects.semantics == RXOP_SEM_NONE &&
              effects.optimizer_barrier &&
              rxop_component_reads(OP_BCHECKRANGE_REG_REG_REG, 0) ==
                    RXOP_COMPONENT_BINARY &&
              rxop_component_reads(OP_BCHECKRANGE_REG_REG_REG, 1) ==
                    RXOP_COMPONENT_INTEGER &&
              signal.static_names &&
              strcmp(signal.static_names, "OUT_OF_RANGE") == 0 &&
              (signal.properties & RXOP_SIGNAL_PROP_SUCCESS_STABLE),
          "binary range guard contract regression",
          &op_table[OP_BCHECKRANGE_REG_REG_REG]);
    signal = rxop_signal_contract(OP_SETOBJTYPE_REG_STRING);
    check(signal.state == RXOP_SIGNAL_STATE_NONE &&
              rxop_component_reads(OP_SETOBJTYPE_REG_STRING, 0) ==
                    RXOP_COMPONENT_ATTRIBUTES &&
              rxop_component_writes(OP_SETOBJTYPE_REG_STRING, 0) ==
                    (RXOP_COMPONENT_OBJECT_TYPE | RXOP_COMPONENT_ATTRIBUTES) &&
              rxop_definition_components(OP_SETOBJTYPE_REG_STRING, 0) ==
                    RXOP_COMPONENT_OBJECT_TYPE &&
              rxop_observed_components(OP_SETOBJTYPE_REG_STRING, 0) == RXOP_COMPONENT_NONE,
          "object type producer contract regression",
          &op_table[OP_SETOBJTYPE_REG_STRING]);
    check(rxop_component_reads(OP_TYPEOF_REG_REG, 1) == RXOP_COMPONENT_OBJECT_TYPE &&
              rxop_component_reads(OP_ISTYPE_REG_REG_STRING, 1) == RXOP_COMPONENT_OBJECT_TYPE &&
              rxop_component_reads(OP_SRCMETHODSEL_REG_REG_STRING, 1) == RXOP_COMPONENT_OBJECT_TYPE &&
              !(rxop_component_writes(OP_ACOPY_REG_REG, 0) & RXOP_COMPONENT_OBJECT_TYPE) &&
              !(rxop_transport_components(OP_ACOPY_REG_REG) & RXOP_COMPONENT_OBJECT_TYPE) &&
              (rxop_transport_components(OP_COPY_REG_REG) & RXOP_COMPONENT_OBJECT_TYPE),
          "type stamp remains live through status-only ACOPY and is transported by whole COPY", NULL);
    check(rxop_component_clears(OP_NULL_REG, 0) == RXOP_COMPONENT_ALL &&
              rxop_component_clears(OP_SETOBJUNINIT_REG_STRING, 0) ==
                  (RXOP_COMPONENT_ALL & ~RXOP_COMPONENT_OBJECT_TYPE) &&
              rxop_definition_components(OP_SETOBJUNINIT_REG_STRING, 0) == RXOP_COMPONENT_OBJECT_TYPE &&
              rxop_component_clears(OP_TYPEOF_REG_REG, 0) == RXOP_COMPONENT_ALL &&
              rxop_component_clears(OP_ISTYPE_REG_REG_STRING, 0) == RXOP_COMPONENT_ALL &&
              rxop_component_clears(OP_ISINITIALIZED_REG_REG, 0) == RXOP_COMPONENT_ALL &&
              !(rxop_component_writes(OP_LOAD_REG_INT, 0) & RXOP_COMPONENT_OBJECT_TYPE) &&
              !(rxop_component_clears(OP_LOAD_REG_INT, 0) & RXOP_COMPONENT_OBJECT_TYPE),
          "whole resets clear type; uninitialized stamp restores it; scalar setters preserve it", NULL);
    check(rxop_selection_window_components(OP_SRCFPROCSEL_REG_STRING_REG) == RXOP_COMPONENT_ALL &&
              rxop_selection_window_components(OP_SRCMETHODSEL_REG_REG_STRING) == RXOP_COMPONENT_NONE &&
              rxop_selection_window_components(-1) == RXOP_COMPONENT_ALL,
          "factory match callbacks observe actual type and payload through the canonical window descriptor", NULL);
    check(rxop_selection_callbacks(OP_SRCFPROCSEL_REG_STRING_REG) &&
              !rxop_selection_callbacks(OP_SRCMETHODSEL_REG_REG_STRING) &&
              rxop_host_callbacks(OP_SAYX_STRING) && rxop_host_callbacks(OP_SAY_REG) &&
              !rxop_host_callbacks(OP_MKREF_REG_REG) &&
              rxop_signal_contract(OP_SAYX_STRING).state == RXOP_SIGNAL_STATE_UNKNOWN,
          "resolver and legacy host callback routes remain separate from inert opaque residuals", NULL);
    check((rxop_callback_inputs(OP_COPY_REG_REG, 1) & RXOP_COMPONENT_NATIVE_PAYLOAD) &&
              (rxop_callback_inputs(OP_BCOPY_REG_REG, 1) & RXOP_COMPONENT_NATIVE_PAYLOAD) &&
              !rxop_callback_inputs(OP_MOVE_REG_REG, 1) &&
              (rxop_callback_outputs(OP_MOVE_REG_REG, 0) & RXOP_COMPONENT_NATIVE_PAYLOAD) &&
              (rxop_callback_outputs(OP_LOAD_REG_INT, 0) & RXOP_COMPONENT_NATIVE_PAYLOAD) &&
              !rxop_callback_outputs(OP_ICOPY_REG_REG, 0),
          "copy-source and overwritten-destination payload hooks are independent conditional resources", NULL);
    check(!rxop_callback_inputs(OP_LOAD_REG_REG, 1) &&
              !rxop_callback_outputs(OP_LOAD_REG_REG, 0) &&
              !rxop_callback_inputs(OP_LINK_REG_REG, 1) &&
              !rxop_callback_outputs(OP_LINK_REG_REG, 0) &&
              !rxop_callback_outputs(OP_SWAPN_REG_REG_REG_REG, 0) &&
              !rxop_callback_outputs(OP_LOAD_INT_INT, 0),
          "pure ordered pointer bindings do not invoke value copy/finalizer hooks", NULL);
    check(rxop_callback_capacity(OP_COPY_REG_REG, 0) &&
              rxop_callback_capacity(OP_MOVE_REG_REG, 0) &&
              !rxop_callback_capacity(OP_NULL_REG, 0) &&
              !rxop_callback_capacity(OP_SRCMETHODSEL_REG_REG_STRING, 0) &&
              (rxop_callback_outputs(OP_NULL_REG, 0) & RXOP_COMPONENT_ATTRIBUTE_COUNT) &&
              (rxop_callback_outputs(OP_SRCMETHODSEL_REG_REG_STRING, 0) & RXOP_COMPONENT_NATIVE_PAYLOAD),
          "whole overwrites touch retained child capacity; exact value_zero routes only active children", NULL);
    check(!rxop_abi_callbacks(OP_RET) && !rxop_abi_callbacks(OP_RET_INT) &&
              !rxop_abi_callbacks(OP_RET_FLOAT) && rxop_abi_callbacks(OP_RET_REG) &&
              rxop_abi_callbacks(OP_RET_STRING) && rxop_abi_callbacks(OP_INLENTER_REG_FUNC_STRING_ID) &&
              rxop_departure_callbacks(OP_RET) && rxop_departure_callbacks(OP_RET_INT) &&
              !rxop_departure_callbacks(OP_ICOPY_REG_REG),
          "result copyback hooks remain distinct from frame/external-argument departure obligations", NULL);
    signal = rxop_signal_contract(OP_ISETATTR1_REG_INT_REG);
    check(signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES &&
              signal.static_names &&
              strcmp(signal.static_names, "OUT_OF_RANGE") == 0 &&
              signal.failure_writes == RXOP_OP_NONE &&
              signal.failure_component_writes == RXOP_COMPONENT_NONE &&
              (signal.properties & RXOP_SIGNAL_PROP_SUCCESS_STABLE) &&
              rxop_component_writes(OP_ISETATTR1_REG_INT_REG, 0) ==
                    RXOP_COMPONENT_INTEGER,
          "direct integer attribute store contract regression",
          &op_table[OP_ISETATTR1_REG_INT_REG]);
    effects = rxop_effects(OP_IGETUNLINK_REG_REG);
    signal = rxop_signal_contract(OP_IGETUNLINK_REG_REG);
    check(signal.state == RXOP_SIGNAL_STATE_NONE &&
              effects.reads == RXOP_OP_2 &&
              effects.writes == RXOP_OP_12 &&
              effects.kills == RXOP_OP_1 &&
              (effects.semantics & RXOP_SEM_ALIAS_RELEASE) &&
              !(effects.semantics & RXOP_SEM_OPAQUE) &&
              rxop_component_reads(OP_IGETUNLINK_REG_REG, 1) ==
                    RXOP_COMPONENT_INTEGER &&
              rxop_component_writes(OP_IGETUNLINK_REG_REG, 0) ==
                    RXOP_COMPONENT_INTEGER &&
              rxop_component_writes(OP_IGETUNLINK_REG_REG, 1) ==
                    RXOP_COMPONENT_NONE &&
              rxop_component_write_is_known(OP_IGETUNLINK_REG_REG, 0) &&
              rxop_component_write_is_known(OP_IGETUNLINK_REG_REG, 1),
          "integer read-unlink transfer contract regression",
          &op_table[OP_IGETUNLINK_REG_REG]);
    check(!rxop_callback_inputs(OP_IGETUNLINK_REG_REG, 1) &&
              !rxop_callback_outputs(OP_IGETUNLINK_REG_REG, 0) &&
              !rxop_callback_outputs(OP_IGETUNLINK_REG_REG, 1) &&
              !rxop_callback_capacity(OP_IGETUNLINK_REG_REG, 0) &&
              !rxop_callback_capacity(OP_IGETUNLINK_REG_REG, 1),
          "integer capture and pointer-only unlink have no value hooks",
          &op_table[OP_IGETUNLINK_REG_REG]);
    effects = rxop_effects(OP_ISETUNLINK_REG_REG);
    check(effects.reads == RXOP_OP_12 && effects.writes == RXOP_OP_1 &&
              (effects.semantics & RXOP_SEM_ALIAS_RELEASE) &&
              !(effects.semantics & RXOP_SEM_OPAQUE) &&
              rxop_component_reads(OP_ISETUNLINK_REG_REG, 1) == RXOP_COMPONENT_INTEGER &&
              rxop_component_writes(OP_ISETUNLINK_REG_REG, 0) == RXOP_COMPONENT_INTEGER &&
              rxop_component_write_is_known(OP_ISETUNLINK_REG_REG, 0) &&
              !rxop_component_clears(OP_ISETUNLINK_REG_REG, 0) &&
              !rxop_callback_inputs(OP_ISETUNLINK_REG_REG, 1) &&
              !rxop_callback_outputs(OP_ISETUNLINK_REG_REG, 0) &&
              !rxop_callback_capacity(OP_ISETUNLINK_REG_REG, 0),
          "integer field copy and slot restoration have no opaque or payload-hook effects",
          &op_table[OP_ISETUNLINK_REG_REG]);
    check(rxop_callback_outputs(OP_ILOADSETUNLINK_REG_INT, 0) &
              RXOP_COMPONENT_NATIVE_PAYLOAD,
          "compound unlink with an integer setter retains its payload hook",
          &op_table[OP_ILOADSETUNLINK_REG_INT]);
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
              (signal.properties & RXOP_SIGNAL_PROP_PENDING_WRITE) &&
              !(signal.properties & RXOP_SIGNAL_PROP_POLICY_WRITE) &&
              signal.policy_effect == RXOP_POLICY_EFFECT_BREAKPOINT_DISABLE &&
              signal.dependencies == RXOP_SIGNAL_DEP_NONE &&
              signal.policy_static_name &&
              strcmp(signal.policy_static_name, "BREAKPOINT") == 0,
          "pending breakpoint control falsely replaces structural policy", &op_table[OP_BPOFF]);
    check_propagated_call_metadata(OP_CALL_FUNC, 0);
    check_propagated_call_metadata(OP_CALL_REG_FUNC, 1);
    check_propagated_call_metadata(OP_CALL_REG_FUNC_REG, 1);
    check_propagated_call_metadata(OP_DCALL_REG_REG_REG, 1);
    check_propagated_call_metadata(OP_SWAPCALL_REG_FUNC_REG_REG_REG, 1);
    check_propagated_call_metadata(
            OP_SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG, 1);
    check_propagated_call_metadata(
            OP_SETTPCALL_REG_FUNC_REG_REG_INT, 1);
    check_propagated_call_metadata(OP_CALL1_REG_FUNC_REG, 1);
    check_propagated_call_metadata(OP_CALL2_REG_FUNC_REG_REG, 1);
    check_propagated_call_metadata(OP_CALL3_REG_FUNC_REG_REG_REG, 1);
    check_propagated_call_metadata(OP_CALL4_REG_FUNC_REG_REG_REG_REG, 1);
    signal = rxop_signal_contract(OP_CALL_REG_FUNC_REG);
    check(!rxop_signal_failure_writes_operand(&signal, 2),
          "counted CALL may not claim to write its count on failure",
          &op_table[OP_CALL_REG_FUNC_REG]);
    signal = rxop_signal_contract(OP_DCALL_REG_REG_REG);
    check(!rxop_signal_failure_writes_operand(&signal, 1) &&
              !rxop_signal_failure_writes_operand(&signal, 2),
          "dynamic CALL may not claim to write its target/count on failure",
          &op_table[OP_DCALL_REG_REG_REG]);
    signal = rxop_signal_contract(OP_SETTPCALL_REG_FUNC_REG_REG_INT);
    check(!rxop_signal_failure_writes_operand(&signal, 2) &&
              rxop_signal_failure_writes_operand(&signal, 3),
          "SETTPCALL count/set-type failure phase regression",
          &op_table[OP_SETTPCALL_REG_FUNC_REG_REG_INT]);
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
    check(rxop_component_reads(OP_CONCAT_REG_STRING_REG, 2) ==
                    RXOP_COMPONENT_STRING &&
              rxop_component_reads(OP_CONCAT_REG_REG_STRING, 1) ==
                    RXOP_COMPONENT_STRING &&
              rxop_component_writes(OP_CONCAT_REG_STRING_REG, 0) ==
                    RXOP_COMPONENT_STRING &&
              rxop_component_writes(OP_SCONCAT_REG_REG_REG, 0) ==
                    RXOP_COMPONENT_STRING,
          "concat family must expose exact string component flow",
          &op_table[OP_CONCAT_REG_STRING_REG]);
    check_native_stem_signal_contract(OP_STEMGET_REG_REG_REG);
    check_native_stem_signal_contract(OP_STEMSET_REG_REG_REG);
    check_native_stem_signal_contract(OP_STEMGET2_REG_REG_REG_REG);
    check_native_stem_signal_contract(OP_STEMSET2_REG_REG_REG_REG);
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
    check(effects.const_evaluator == RXOP_CONST_EVAL_PADSTR,
          "PADSTR evaluator regression",
          &op_table[OP_PADSTR_REG_REG_REG]);
    effects = rxop_effects(OP_STRLEN_REG_REG);
    check(effects.const_evaluator == RXOP_CONST_EVAL_STRLEN,
          "STRLEN evaluator regression",
          &op_table[OP_STRLEN_REG_REG]);
    effects = rxop_effects(OP_STRCHAR_REG_REG_REG);
    check(effects.reads == RXOP_OP_23 && effects.writes == RXOP_OP_1 &&
              effects.const_evaluator == RXOP_CONST_EVAL_STRCHAR_AT,
          "indexed STRCHAR effects/evaluator regression",
          &op_table[OP_STRCHAR_REG_REG_REG]);
    effects = rxop_effects(OP_SUBSTRING_REG_REG_REG_REG);
    signal = rxop_signal_contract(OP_SUBSTRING_REG_REG_REG_REG);
    check(effects.reads_signature &&
              strcmp(effects.reads_signature, "0111") == 0 &&
              effects.writes_signature &&
              strcmp(effects.writes_signature, "1000") == 0 &&
              effects.const_evaluator == RXOP_CONST_EVAL_SUBSTRING &&
              rxop_component_reads(OP_SUBSTRING_REG_REG_REG_REG, 1) ==
                  RXOP_COMPONENT_STRING &&
              rxop_component_reads(OP_SUBSTRING_REG_REG_REG_REG, 2) ==
                  RXOP_COMPONENT_INTEGER &&
              rxop_component_reads(OP_SUBSTRING_REG_REG_REG_REG, 3) ==
                  RXOP_COMPONENT_INTEGER &&
              rxop_component_writes(OP_SUBSTRING_REG_REG_REG_REG, 0) ==
                  RXOP_COMPONENT_STRING &&
              signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES &&
              signal.static_names &&
              strcmp(signal.static_names, "UNICODE_ERROR|FAILURE") == 0 &&
              signal.failure_writes == RXOP_OP_NONE,
          "explicit SUBSTRING effects/component regression",
          &op_table[OP_SUBSTRING_REG_REG_REG_REG]);
    effects = rxop_effects(OP_BSLICE_REG_REG_REG_REG);
    signal = rxop_signal_contract(OP_BSLICE_REG_REG_REG_REG);
    check(effects.reads_signature &&
              strcmp(effects.reads_signature, "0111") == 0 &&
              effects.writes_signature &&
              strcmp(effects.writes_signature, "1000") == 0 &&
              rxop_component_reads(OP_BSLICE_REG_REG_REG_REG, 1) ==
                  RXOP_COMPONENT_BINARY &&
              rxop_component_reads(OP_BSLICE_REG_REG_REG_REG, 2) ==
                  RXOP_COMPONENT_INTEGER &&
              rxop_component_reads(OP_BSLICE_REG_REG_REG_REG, 3) ==
                  RXOP_COMPONENT_INTEGER &&
              rxop_component_writes(OP_BSLICE_REG_REG_REG_REG, 0) ==
                  RXOP_COMPONENT_BINARY &&
              signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES &&
              signal.static_names &&
              strcmp(signal.static_names, "OUT_OF_RANGE|FAILURE") == 0 &&
              signal.failure_writes == RXOP_OP_NONE,
          "explicit BSLICE effects/component regression",
          &op_table[OP_BSLICE_REG_REG_REG_REG]);
    effects = rxop_effects(OP_FNDBLNK_REG_REG_REG);
    check(effects.const_evaluator == RXOP_CONST_EVAL_FNDBLNK,
          "FNDBLNK evaluator regression",
          &op_table[OP_FNDBLNK_REG_REG_REG]);
    effects = rxop_effects(OP_FNDNBLNK_REG_REG_REG);
    check(effects.const_evaluator == RXOP_CONST_EVAL_FNDNBLNK,
          "FNDNBLNK evaluator regression",
          &op_table[OP_FNDNBLNK_REG_REG_REG]);
    effects = rxop_effects(OP_STRPOS_REG_REG_REG);
    check(effects.reads == RXOP_OP_ALL && effects.writes == RXOP_OP_1 &&
              effects.kills == RXOP_OP_NONE &&
              rxop_component_reads(OP_STRPOS_REG_REG_REG, 0) ==
                  RXOP_COMPONENT_INTEGER &&
              rxop_component_reads(OP_STRPOS_REG_REG_REG, 1) ==
                  RXOP_COMPONENT_STRING &&
              rxop_component_reads(OP_STRPOS_REG_REG_REG, 2) ==
                  RXOP_COMPONENT_STRING,
          "STRPOS must read its operand-1 start position",
          &op_table[OP_STRPOS_REG_REG_REG]);
    check(rxop_component_writes(OP_NULLN_REG_REG, 1) ==
                  RXOP_COMPONENT_ALL &&
              rxop_component_writes(OP_NULLN_REG_REG_REG, 2) ==
                  RXOP_COMPONENT_ALL &&
              rxop_component_writes(OP_NULLN_REG_REG_REG_REG, 3) ==
                  RXOP_COMPONENT_ALL,
          "packed NULL writes every component of every target",
          &op_table[OP_NULLN_REG_REG_REG_REG]);
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
    effects = rxop_effects(OP_RESERVED_466);
    check(effects.state == RXOP_EFFECT_RESERVED && effects.optimizer_barrier,
          "retired process opcode must fail closed",
          &op_table[OP_RESERVED_466]);
    effects = rxop_effects(OP_CHANOPEN_REG_REG_REG_REG_REG);
    signal = rxop_signal_contract(OP_CHANOPEN_REG_REG_REG_REG_REG);
    check(OP_CHANOPEN_REG_REG_REG_REG_REG == 650 &&
              effects.state == RXOP_EFFECT_CLASSIFIED &&
              rxop_effect_reads_operand(&effects, 2) &&
              rxop_effect_reads_operand(&effects, 4) &&
              !rxop_effect_reads_operand(&effects, 0) &&
              rxop_effect_writes_operand(&effects, 0) &&
              rxop_effect_writes_operand(&effects, 1) &&
              !rxop_effect_writes_operand(&effects, 2) &&
              rxop_effect_kills_operand(&effects, 0) &&
              rxop_effect_kills_operand(&effects, 1) &&
              effects.semantics == RXOP_SEM_OPAQUE &&
              effects.optimizer_barrier &&
              signal.state == RXOP_SIGNAL_STATE_NONE &&
              rxop_component_reads(OP_CHANOPEN_REG_REG_REG_REG_REG, 2) ==
                  RXOP_COMPONENT_INTEGER &&
              rxop_component_reads(OP_CHANOPEN_REG_REG_REG_REG_REG, 4) ==
                  RXOP_COMPONENT_BINARY &&
              rxop_component_writes(OP_CHANWAIT_REG_REG_REG_REG, 0) ==
                  RXOP_COMPONENT_INTEGER &&
              rxop_component_writes(OP_CHANWAIT_REG_REG_REG_REG, 1) ==
                  RXOP_COMPONENT_BINARY &&
              rxop_component_clears(OP_CHANWAIT_REG_REG_REG_REG, 1) ==
                  (RXOP_COMPONENT_REFERENCE |
                   RXOP_COMPONENT_NATIVE_PAYLOAD),
          "Gate F channel metadata contract regression",
          &op_table[OP_CHANOPEN_REG_REG_REG_REG_REG]);
    effects = rxop_effects(OP_PGETI_REG_REG_REG);
    signal = rxop_signal_contract(OP_PGETI_REG_REG_REG);
    check(effects.state == RXOP_EFFECT_CLASSIFIED &&
              effects.reads == RXOP_OP_23 &&
              effects.writes == RXOP_OP_1 &&
              effects.kills == RXOP_OP_1 &&
              rxop_component_reads(OP_PGETI_REG_REG_REG, 1) ==
                  RXOP_COMPONENT_BINARY &&
              rxop_component_reads(OP_PGETI_REG_REG_REG, 2) ==
                  RXOP_COMPONENT_INTEGER &&
              rxop_component_writes(OP_PGETI_REG_REG_REG, 0) ==
                  RXOP_COMPONENT_INTEGER &&
              rxop_component_clears(OP_PGETI_REG_REG_REG, 0) ==
                  (RXOP_COMPONENT_REFERENCE |
                   RXOP_COMPONENT_NATIVE_PAYLOAD) &&
              signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES &&
              signal.static_names &&
              strcmp(signal.static_names, "OUT_OF_RANGE") == 0 &&
              (signal.properties & RXOP_SIGNAL_PROP_SUCCESS_STABLE),
          "packed integer read metadata regression",
          &op_table[OP_PGETI_REG_REG_REG]);
    effects = rxop_effects(OP_PSETF_REG_REG_REG);
    signal = rxop_signal_contract(OP_PSETF_REG_REG_REG);
    check(effects.state == RXOP_EFFECT_CLASSIFIED &&
              effects.reads == RXOP_OP_ALL &&
              effects.writes == RXOP_OP_1 &&
              effects.kills == RXOP_OP_NONE &&
              rxop_component_reads(OP_PSETF_REG_REG_REG, 0) ==
                  RXOP_COMPONENT_BINARY &&
              rxop_component_reads(OP_PSETF_REG_REG_REG, 1) ==
                  RXOP_COMPONENT_INTEGER &&
              rxop_component_reads(OP_PSETF_REG_REG_REG, 2) ==
                  RXOP_COMPONENT_FLOAT &&
              rxop_component_writes(OP_PSETF_REG_REG_REG, 0) ==
                  RXOP_COMPONENT_BINARY &&
              signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES &&
              signal.static_names &&
              strcmp(signal.static_names, "OUT_OF_RANGE") == 0,
          "packed float write metadata regression",
          &op_table[OP_PSETF_REG_REG_REG]);
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
    check(rxop_normal_successors(OP_UNLINKBR_REG_ID) == 1 &&
              rxop_binding_action(OP_UNLINKBR_REG_ID) == RXOP_BINDING_COMPOUND &&
              rxop_normal_successors(OP_BCT_ID_REG) == 2 &&
              rxop_normal_fallthrough(OP_BCT_ID_REG),
          "non-first and fused direct targets use generic normal flow", NULL);
    check(rxop_binding_action(OP_LOAD_REG_REG) == RXOP_BINDING_LINK &&
              rxop_clone_category(OP_LOAD_REG_REG) == RXOP_CLONE_BINDING,
          "register LOAD is rebinding rather than an ordinary value write", NULL);
    check(rxop_clone_category(OP_SAY_REG) == RXOP_CLONE_ORDINARY &&
              rxop_clone_category(OP_TIME_REG) == RXOP_CLONE_ORDINARY &&
              rxop_clone_category(OP_METALOADCALLERADDR_REG) == RXOP_CLONE_RETAINED,
          "external opacity is distinct from physical-frame observation", NULL);
    check(rxop_component_write_is_known(OP_LOAD_REG_INT, 0) &&
              !rxop_component_write_is_known(OP_FADD_REG_REG_REG, 0) &&
              rxop_clone_category(OP_FADD_REG_REG_REG) == RXOP_CLONE_ORDINARY,
          "missing optional component projection does not gate ordinary cloning", NULL);
    check(!strcmp(rxop_call_contract(OP_CALL_REG_FUNC).prefix, "clear:1") &&
              !strcmp(rxop_call_contract(OP_SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG).prefix,
                      "flags:4:5;swap:4:6") &&
              rxop_call_contract(OP_SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG).capture_after_prefix &&
              !strcmp(rxop_call_contract(OP_CALL4_REG_FUNC_REG_REG_REG_REG).actual_operands,
                      "3,4,5,6"),
          "call recipes preserve preclear, fused order and explicit pointer order", NULL);
    check(rxop_scoped_diagnostics(),
          "current scoped activation contract includes detached logical diagnostic chain", NULL);
    check(rxop_residual_observable(OP_SAYX_STRING) &&
              rxop_residual_observable(OP_CALL_FUNC) &&
              rxop_residual_observable(OP_EXIT_INT) &&
              rxop_residual_observable(OP_LOAD_REG_INT) &&
              rxop_residual_observable(-1) &&
              !rxop_residual_observable(OP_ICOPY_REG_REG),
          "discard residuals retain literal output, control, calls and released payloads without blocking ordinary cloning", NULL);
    check(rxop_call_variant(OP_CALL1_REG_FUNC_REG, 4u) == OP_CALL4_REG_FUNC_REG_REG_REG_REG &&
              rxop_call_variant(OP_CALL_REG_FUNC_REG, 0u) == 0 &&
              rxop_call_variant(OP_CALL_REG_FUNC, 0u) == 0 &&
              rxop_call_variant(OP_CALL_REG_FUNC_REG, 1u) == 0 &&
              rxop_call_variant(OP_DCALL_REG_REG_REG, 1u) == 0 &&
              rxop_call_variant(OP_CALL1_REG_FUNC_REG, 5u) == 0,
          "fixed call variants preserve invocation family and initializer capture checkpoint", NULL);
    check(rxop_call_invokes(OP_DCALL_REG_REG_REG) &&
              rxop_call_target_components(OP_DCALL_REG_REG_REG) == RXOP_COMPONENT_INTEGER &&
              rxop_observed_components(OP_DCALL_REG_REG_REG, 1) == RXOP_COMPONENT_INTEGER &&
              rxop_call_target_components(OP_CALL1_REG_FUNC_REG) == RXOP_COMPONENT_NONE,
          "dynamic target is an integer procedure pointer; typed direct target is not a scalar", NULL);
    check(rxop_call_contract(OP_SRCFPROCSEL_REG_STRING_REG).kind == RXOP_CALL_FACTORY &&
              !rxop_call_invokes(OP_SRCFPROCSEL_REG_STRING_REG) &&
              !rxop_call_invokes(OP_SRCMETHODSEL_REG_REG_STRING) &&
              rxop_selection_contract(OP_SRCFPROCSEL_REG_STRING_REG).kind == RXOP_SELECTION_FACTORY &&
              rxop_selection_contract(OP_SRCMETHODSEL_REG_REG_STRING).kind == RXOP_SELECTION_METHOD &&
              rxop_selection_contract(OP_SRCMETHODSEL_REG_REG_STRING).descriptor_operand == 3 &&
              rxop_selection_contract(OP_SRCFPROCSEL_REG_STRING_REG).count_operand == 3,
          "legacy factory layout and canonical method/factory selection remain distinct from invocation", NULL);
    check(rxop_call_contract(OP_INLENTER_REG_FUNC_STRING_ID).kind == RXOP_CALL_NONE &&
              rxop_call_contract(OP_INLENTER_REG_FUNC_STRING_ID).result_operand == 1 &&
              rxop_call_contract(OP_INLENTER_REG_FUNC_STRING_ID).target_operand == 2 &&
              rxop_recipe_operand(OP_INLENTER_REG_FUNC_STRING_ID) == 3 &&
              rxop_scope_continuation_operand(OP_INLENTER_REG_FUNC_STRING_ID) == 4,
          "logical scope capture selectors preserve distinct CALL admission", NULL);
    effects = rxop_effects(OP_QGLOBBIND_REG_STRING_INT);
    signal = rxop_signal_contract(OP_QGLOBBIND_REG_STRING_INT);
    check(effects.state == RXOP_EFFECT_CLASSIFIED &&
              effects.reads == RXOP_OP_NONE && effects.writes == RXOP_OP_1 &&
              effects.kills == RXOP_OP_NONE && effects.optimizer_barrier &&
              effects.implicit == RXOP_IMPLICIT_QUALIFIED_GLOBAL &&
              effects.semantics == RXOP_SEM_ALIAS_CREATE &&
              !strcmp(rxop_binding_recipe(OP_QGLOBBIND_REG_STRING_INT),
                      "qualifiedglobal:1:2:3") &&
              rxop_operand_relocation(OP_QGLOBBIND_REG_STRING_INT, 1) == RXOP_RELOC_MODULE &&
              !rxop_abi_callbacks(OP_QGLOBBIND_REG_STRING_INT) &&
              !rxop_host_callbacks(OP_QGLOBBIND_REG_STRING_INT) &&
              rxop_callback_outputs(OP_QGLOBBIND_REG_STRING_INT, 0) == RXOP_COMPONENT_NONE &&
              !rxop_callback_capacity(OP_QGLOBBIND_REG_STRING_INT, 0),
          "qualified global binding changes only its active/base pointer slot and keeps its optimizer barrier", NULL);
    check(signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES &&
              !strcmp(signal.static_names, "RXBIN_CORRUPTION|OUT_OF_RANGE") &&
              signal.failure_writes == RXOP_OP_NONE &&
              signal.failure_component_writes == RXOP_COMPONENT_NONE &&
              signal.failure_context_writes == RXOP_CONTEXT_NONE,
          "qualified global binding validates before changing either pointer", NULL);
    check(rxop_return_result_component_invalidations(OP_RET_STRING) == RXOP_COMPONENT_ALL &&
              rxop_return_result_flag_invalidations(OP_RET_STRING) == RXFLAG_READABLE_MASK &&
              rxop_return_result_component_invalidations(OP_RET_INT) == RXOP_COMPONENT_NONE &&
              rxop_return_result_flag_invalidations(OP_RET_INT) == 0 &&
              rxop_return_result_component_invalidations(OP_RET_FLOAT) == RXOP_COMPONENT_NONE &&
              rxop_return_result_flag_invalidations(OP_RET_FLOAT) == 0 &&
              rxop_return_result_component_invalidations(OP_RET_REG) == RXOP_COMPONENT_NONE &&
              rxop_return_result_flag_invalidations(OP_RET_REG) == 0,
          "return result setters invalidate string-owned state while direct scalar and transported returns retain their contracts", NULL);
    check(rxop_return_result_component_invalidations(-1) == RXOP_COMPONENT_ALL &&
              rxop_return_result_flag_invalidations(OP_MAX_INSTRUCTIONS) == RXFLAG_READABLE_MASK,
          "unknown return result effects cannot preserve earlier facts", NULL);
    check(!strcmp(rxop_binding_recipe(OP_IGETUNLINK_REG_REG),
                      "captureint:2;unlink:2;putint:1") &&
              !strcmp(rxop_binding_recipe(OP_ILOADSETUNLINKN_REG_REG_INT_REG),
                      "setint:1:3;icopy:2:1;unlink:2;unlink:4"),
          "compound recipes preserve value capture across binding restoration", NULL);
    check(rxop_operand_relocation(OP_LOAD_INT_INT, 1) == RXOP_RELOC_LOCAL_INDEX &&
              rxop_operand_relocation(OP_SIGCALLBR_ID_FUNC_STRING, 0) == RXOP_RELOC_LABEL &&
              rxop_operand_relocation(OP_SIGCALLBR_ID_FUNC_STRING, 1) == RXOP_RELOC_PROCEDURE &&
              rxop_operand_relocation(OP_JUMPI_REG_BINARY, 1) == RXOP_RELOC_JUMP_TABLE,
          "implicit, exceptional and table relocations retain their owner", NULL);
    check(rxop_context_read_mask(OP_GETNUMCAS_REG) == RXOP_CONTEXT_CASE &&
              rxop_context_write_mask(OP_GETNUMCAS_REG) == 0 &&
              rxop_context_write_mask(OP_SETNUMCAS_INT) == RXOP_CONTEXT_CASE &&
              rxop_context_write_mask(OP_NUMSCI_INT_INT_INT) == RXOP_CONTEXT_ALL_NUMERIC &&
              (rxop_context_read_mask(OP_DADD_REG_REG_REG) & RXOP_CONTEXT_ALL_NUMERIC),
          "context dependencies distinguish getters, setters and decimal reads", NULL);
    check(rxop_definition_components(OP_LOAD_REG_INT, 0) == RXOP_COMPONENT_INTEGER &&
              rxop_literal_operand(OP_LOAD_REG_INT) == 2 &&
              rxop_definition_components(OP_COPY_REG_REG, 0) == 0 &&
              rxop_transport_source_operand(OP_COPY_REG_REG) == 2 &&
              rxop_transport_components(OP_COPY_REG_REG) == RXOP_COMPONENT_ALL &&
              rxop_definition_components(OP_NULLN_REG_REG_REG, 2) == RXOP_COMPONENT_ALL,
          "definitions and transports have independent availability obligations", NULL);
    check(rxop_component_reads(OP_STRPOS_REG_REG_REG, 0) == RXOP_COMPONENT_INTEGER &&
              rxop_component_writes(OP_STRPOS_REG_REG_REG, 0) == RXOP_COMPONENT_INTEGER,
          "STRPOS exposes its integer start/result, not a VM-private string cache", NULL);
    check(rxop_component_reads(OP_RET_REG, 0) == RXOP_COMPONENT_ALL &&
              rxop_observed_components(OP_RET_REG, 0) == RXOP_COMPONENT_NONE &&
              rxop_observation_is_known(OP_RET_REG, 0),
          "RET transports a complete register without reading unused components", NULL);
    check(rxop_component_may_reads(OP_SIGBRV_ID_REG_STRING, 1) == RXOP_COMPONENT_ALL &&
              rxop_observed_components(OP_SIGBRV_ID_REG_STRING, 1) == RXOP_COMPONENT_NONE &&
              rxop_observed_flags(OP_SIGBRV_ID_REG_STRING, 1) == 0 &&
              rxop_observation_is_known(OP_SIGBRV_ID_REG_STRING, 1),
          "SIGBRV retains a destination slot without observing its payload", NULL);
    check(rxop_observed_components(OP_SAY_REG, 0) == RXOP_COMPONENT_STRING &&
              rxop_observation_is_known(OP_SAY_REG, 0) &&
              rxop_observed_components(OP_SIGNALORIGIN_STRING_REG_REG, 2) == RXOP_COMPONENT_ALL &&
              rxop_observation_is_known(OP_SIGNALORIGIN_STRING_REG_REG, 2),
          "slot-only policy installation preserves scalar and complete origin-structure observations", NULL);
    check(rxop_component_may_reads(OP_SIGNALORIGIN_STRING_REG_REG, 1) == RXOP_COMPONENT_ALL &&
              rxop_observed_components(OP_SIGNALORIGIN_STRING_REG_REG, 1) == RXOP_COMPONENT_NONE &&
              rxop_observation_is_known(OP_SIGNALORIGIN_STRING_REG_REG, 1),
          "SIGNALORIGIN transports a complete payload without inspecting unused scalar components", NULL);
    signal = rxop_signal_contract(OP_SIGNALORIGIN_STRING_REG_REG);
    effects = rxop_effects(OP_SIGNALORIGIN_STRING_REG_REG);
    check(signal.state == RXOP_SIGNAL_STATE_KNOWN &&
              signal.source == RXOP_SIGNAL_SOURCE_LITERAL_OPERAND &&
              signal.source_operand == 0 && signal.static_names &&
              strcmp(signal.static_names, "INVALID_SIGNAL_CODE|INVALID_ARGUMENTS") == 0 &&
              signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES &&
              signal.failure_writes == RXOP_OP_NONE &&
              signal.failure_component_writes == RXOP_COMPONENT_NONE &&
              signal.failure_context_writes == RXOP_CONTEXT_NONE &&
              effects.writes == RXOP_OP_NONE && effects.kills == RXOP_OP_NONE &&
              (signal.properties & RXOP_SIGNAL_PROP_ADDRESS_OBSERVABLE) &&
              (signal.properties & RXOP_SIGNAL_PROP_PAYLOAD_OBSERVABLE) &&
              signal.continuations == RXOP_SIGNAL_CONT_ALL,
          "SIGNALORIGIN preserves requested tag plus validation alternatives and pending context without input writes", NULL);
    /* Bitwise and logical operations are a single typed integer family even
     * when a source is immediate. Every register read must be observable as
     * integer-only; the result must define the integer component. */
    for (i = OP_IAND_REG_REG_REG; i <= OP_NOT_REG_REG; ++i) {
        size_t operand;
        const OpInfo *bitwise = &op_table[i];
        RxOpEffects bit_effects = rxop_effects(i);
        for (operand = 0; operand < rxop_format_operand_count(bitwise->format);
             ++operand) {
            if (rxop_effect_reads_operand(&bit_effects, operand) &&
                rxop_format_operand_type(bitwise->format, operand) == OP_REG)
                check(rxop_component_reads(i, operand) == RXOP_COMPONENT_INTEGER &&
                          rxop_observed_components(i, operand) == RXOP_COMPONENT_INTEGER &&
                          rxop_observation_is_known(i, operand),
                      "integer bitwise input has an unknown typed observation", bitwise);
            if (rxop_effect_writes_operand(&bit_effects, operand))
                check(rxop_component_writes(i, operand) == RXOP_COMPONENT_INTEGER,
                      "integer bitwise result has an unknown typed write", bitwise);
        }
    }

    if (failures) {
        fprintf(stderr, "%d opcode metadata consistency failure(s)\n", failures);
        return 1;
    }

    printf("opcode effects: total=%d source=%d classified=%d conservative=%d "
           "reserved=%d internal=%d\n",
           i, source_count, classified_count, conservative_count,
           reserved_count, internal_count);
    printf("clone contract: missing=%d ordinary=%d binding=%d call=%d return=%d "
           "context=%d retained=%d unknown-signals=%d unknown-component-writes=%d\n",
           clone_categories[RXOP_CLONE_MISSING], clone_categories[RXOP_CLONE_ORDINARY],
           clone_categories[RXOP_CLONE_BINDING], clone_categories[RXOP_CLONE_CALL],
           clone_categories[RXOP_CLONE_RETURN], clone_categories[RXOP_CLONE_CONTEXT],
           clone_categories[RXOP_CLONE_RETAINED], unknown_clone_signals,
           unknown_clone_writes);
    return 0;
}
