/* cREXX License (MIT). Build-time SDK view of the owning native metadata. */
#include "rxdefs.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#ifndef LL_RXAS_CATALOG_FINGERPRINT
#error LL_RXAS_CATALOG_FINGERPRINT must identify the owning metadata inputs
#endif

static void quoted(FILE *out, const char *text) {
    fputc('\'', out);
    for (; *text; ++text) {
        fputc(*text, out);
        if (*text == '\'') fputc('\'', out);
    }
    fputc('\'', out);
}

static void signature(FILE *out, const RxOpEffects *effect, size_t count,
                      int which) {
    size_t i;
    if (!count) { fputc('-', out); return; }
    for (i = 0; i < count; ++i) {
        int bit = which == 0 ? rxop_effect_reads_operand(effect, i) :
                  which == 1 ? rxop_effect_writes_operand(effect, i) :
                  which == 2 ? rxop_effect_kills_operand(effect, i) :
                               rxop_effect_branch_target_operand(effect, i);
        fputc(bit ? '1' : '0', out);
    }
}

static void components(FILE *out, int opcode, size_t count, int which) {
    size_t i;
    if (!count) { fputc('-', out); return; }
    for (i = 0; i < count; ++i) {
        if (i) fputc(',', out);
        fprintf(out, "%u", which == 0 ? rxop_component_reads(opcode, i) :
                           which == 1 ? rxop_component_writes(opcode, i) :
                                        rxop_component_clears(opcode, i));
    }
}

static void target_operands(FILE *out, int opcode, size_t count, int which) {
    size_t i;
    if (!count) { fputc('-', out); return; }
    for (i = 0; i < count; ++i) {
        unsigned long value;
        if (i) fputc(',', out);
        switch (which) {
            case 0: value = (unsigned long)rxop_operand_relocation(opcode, i); break;
            case 1: value = rxop_component_may_reads(opcode, i); break;
            case 2: value = rxop_component_may_writes(opcode, i); break;
            case 3: value = rxop_flag_may_reads(opcode, i); break;
            case 4: value = rxop_flag_may_writes(opcode, i); break;
            case 5: value = rxop_definition_components(opcode, i); break;
            case 6: value = rxop_observed_components(opcode, i); break;
            case 7: value = rxop_observed_flags(opcode, i); break;
            case 8: value = (unsigned long)rxop_observation_is_known(opcode, i); break;
            case 9: value = rxop_callback_inputs(opcode, i); break;
            case 10: value = rxop_callback_outputs(opcode, i); break;
            case 12: value = (unsigned long)rxop_value_writer(opcode, i); break;
            default: value = (unsigned long)rxop_callback_capacity(opcode, i); break;
        }
        fprintf(out, "%lu", value);
    }
}

static const char *nonempty(const char *value) {
    return value && *value ? value : "-";
}

typedef struct { char key[256]; int opcode; } Lookup;
typedef struct { const char *name; int field; int numeric; } Query;
typedef struct { RxOpAbiContract contract; uint32_t start; uint32_t size; } AbiTable;
#define ABI_TABLE_LIMIT 64u

static size_t abi_table(AbiTable *tables, size_t *count, RxOpAbiContract contract) {
    size_t i;
    for (i = 0u; i < *count; ++i)
        if (tables[i].contract.state == contract.state &&
            tables[i].contract.steps == contract.steps &&
            tables[i].contract.count == contract.count) return i;
    if (*count >= ABI_TABLE_LIMIT) return ABI_TABLE_LIMIT;
    i = (*count)++; tables[i].contract = contract;
    return i;
}

static const Query queries[] = {
    {"mnemonic", 1, 0}, {"signature", 2, 0}, {"flow", 3, 1},
    {"classification", 4, 1}, {"implicit", 5, 1}, {"semantics", 6, 1},
    {"barrier", 7, 1}, {"sourcevisible", 8, 1},
    {"signalstate", 15, 1}, {"signalphase", 16, 1},
    {"signalsource", 17, 1}, {"signalsourceoperand", 18, 1},
    {"signalnames", 19, 0}, {"failurecomponents", 21, 1},
    {"failurecontext", 22, 1}, {"signaldependencies", 23, 1},
    {"continuations", 24, 1}, {"signalproperties", 25, 1},
    {"policyeffect", 26, 1}, {"policysource", 27, 1},
    {"policysourceoperand", 28, 1}, {"policyname", 29, 0},
    {"samecopyisnoop", 32, 1}, {"derivation", 33, 1},
    {"derivationsource", 34, 1}, {"derivationcomponent", 35, 1},
    {"contextreads", 36, 1}, {"contextwrites", 37, 1},
    {"normalsuccessors", 38, 1}, {"normalfallthrough", 39, 1},
    {"clonecategory", 40, 1}, {"binding", 41, 1},
    {"activation", 43, 1},
    {"callkind", 44, 1}, {"calltarget", 45, 1}, {"callresult", 46, 1},
    {"callcount", 47, 1}, {"callwindow", 48, 1}, {"callactuals", 49, 0},
    {"callprefix", 50, 0}, {"callcapture", 51, 1}, {"callatomic", 52, 1},
    {"bindingrecipe", 53, 0}, {"returnkind", 54, 1}, {"returnsource", 55, 1},
    {"contextreadmask", 56, 1}, {"contextwritemask", 57, 1},
    {"observations", 58, 1}, {"implicitrecipe", 64, 0},
    {"bindingcomplete", 65, 1}, {"callbytecode", 66, 1},
    {"transportsource", 68, 1}, {"transportcomponents", 69, 1},
    {"literaloperand", 70, 1}, {"literalcomponents", 71, 1},
    {"scopecontinuation", 72, 1}, {"observedflagmaskoperand", 75, 1},
    {"recipeoperand", 77, 1}, {"explicitvariant", 78, 1},
    {"returninvalidcomponents", 79, 1}, {"returninvalidflags", 80, 1},
    {"callinvokes", 81, 1}, {"calltargetcomponents", 82, 1},
    {"selectionkind", 83, 1}, {"selectionresult", 84, 1},
    {"selectiondescriptor", 85, 1}, {"selectionreceiver", 86, 1},
    {"selectioncount", 87, 1}, {"selectionwindow", 88, 1},
    {"opaqueeffects", 89, 1}, {"residualobservable", 90, 1},
    {"selectionwindowcomponents", 92, 1}, {"selectioncallbacks", 93, 1},
    {"hostcallbacks", 94, 1}, {"abicallbacks", 98, 1},
    {"departurecallbacks", 99, 1}, {"intrinsicfootprint", 101, 1}, {"callbackobligations", 103, 1}
};

static int compare_lookup(const void *a, const void *b) {
    return strcmp(((const Lookup *)a)->key, ((const Lookup *)b)->key);
}

static void hex_file(FILE *out, FILE *input) {
    int byte;
    rewind(input);
    while ((byte = fgetc(input)) != EOF) fprintf(out, "%02X", (unsigned int)byte);
}

static void hex_u32(FILE *out, uint32_t value) {
    unsigned int i;
    for (i = 0; i < 4; ++i) fprintf(out, "%02X", (value >> (i * 8)) & 255u);
}

static int replace_output(const char *temporary, const char *output) {
#ifdef _WIN32
    if (MoveFileExA(temporary, output, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) return 0;
    fprintf(stderr, "catalog replacement failed (Windows error %lu)\n", (unsigned long)GetLastError());
    return -1;
#else
    return rename(temporary, output);
#endif
}

static int close_catalog_data(FILE *stream, const char *path) {
    int failed;
    if (!stream) return 0;
    failed = fclose(stream) != 0;
    if (remove(path)) { perror(path); failed = 1; }
    return failed;
}

int main(int argc, char **argv) {
    FILE *out = NULL, *rows = NULL, *keys = NULL;
    char *temporary, *rows_path, *keys_path;
    size_t path_size;
    int opcode, write_failed, temporary_owned = 0, result = 1;
    Lookup lookup[OP_MAX_INSTRUCTIONS];
    uint32_t row_start[OP_MAX_INSTRUCTIONS], row_size[OP_MAX_INSTRUCTIONS];
    uint32_t key_start[OP_MAX_INSTRUCTIONS], key_size[OP_MAX_INSTRUCTIONS];
    size_t lookup_count = 0, entry, abi_count = 1u;
    AbiTable abi_tables[ABI_TABLE_LIMIT] = { { { RXOP_ABI_UNKNOWN, NULL, 0u }, 0u, 0u } };
    if (argc != 2) {
        fprintf(stderr, "usage: %s OUTPUT\n", argv[0]);
        return 2;
    }
    path_size = strlen(argv[1]) + sizeof(".rows.tmp");
    temporary = malloc(path_size);
    rows_path = malloc(path_size);
    keys_path = malloc(path_size);
    if (!temporary || !rows_path || !keys_path) {
        fputs("catalog output path allocation failed\n", stderr);
        goto cleanup;
    }
    snprintf(temporary, path_size, "%s.tmp", argv[1]);
    snprintf(rows_path, path_size, "%s.rows.tmp", argv[1]);
    snprintf(keys_path, path_size, "%s.keys.tmp", argv[1]);
    out = fopen(temporary, "wb");
    if (!out) { perror(temporary); goto cleanup; }
    temporary_owned = 1;
    /* Windows CRT tmpfile() can require root-directory write access. Keep
     * binary update streams beside the output already owned by this build. */
    rows = fopen(rows_path, "w+b");
    if (!rows) { perror(rows_path); goto cleanup; }
    keys = fopen(keys_path, "w+b");
    if (!keys) { perror(keys_path); goto cleanup; }
    fputs("/* cREXX License (MIT). Generated; native rxop metadata is the owner. */\n"
          "options levelb\nnamespace llrxascatalog expose catalog\nimport rxfnsb\n\n"
          "catalog: class\n  _rows = .binary\n  _rowindex = .binary\n"
          "  _keys = .binary\n  _keyindex = .binary\n  _abiindex = .binary\n  *: factory\n", out);
    for (opcode = 0; opcode < OP_MAX_INSTRUCTIONS; ++opcode) {
        const OpInfo *op = &op_table[opcode];
        RxOpEffects effect = rxop_effects(opcode);
        RxOpSignalContract signal = rxop_signal_contract(opcode);
        RxOpCallContract call = rxop_call_contract(opcode);
        RxOpSelectionContract selection = rxop_selection_contract(opcode);
        const char *binding_recipe = rxop_binding_recipe(opcode);
        size_t derivation_source = rxop_derivation_source_operand(opcode);
        size_t count = rxop_format_operand_count(op->format), i;
        char name[128];
        for (i = 0; op->mnemonic[i] && op->mnemonic[i] != '_' &&
                    i + 1 < sizeof(name); ++i)
            name[i] = (char)tolower((unsigned char)op->mnemonic[i]);
        name[i] = 0;
        if (rxop_is_source_mnemonic(op->mnemonic)) {
            snprintf(lookup[lookup_count].key, sizeof(lookup[lookup_count].key),
                     "%s|%s", name, op->format);
            lookup[lookup_count++].opcode = opcode;
        }
        row_start[opcode] = (uint32_t)ftell(rows);
        fprintf(rows, "%s %s %d %d %d %u %d %d ",
                name, nonempty(op->format), (int)effect.flow,
                (int)effect.state, (int)effect.implicit, effect.semantics,
                effect.optimizer_barrier, rxop_is_source_mnemonic(op->mnemonic));
        for (i = 0; i < 4; ++i) { signature(rows, &effect, count, (int)i); fputc(' ', rows); }
        components(rows, opcode, count, 0); fputc(' ', rows);
        components(rows, opcode, count, 1);
        fprintf(rows, " %d %d %d %lld %s ", (int)signal.state,
                (int)signal.phase, (int)signal.source,
                signal.source_operand == SIZE_MAX ? -1LL : (long long)signal.source_operand,
                nonempty(signal.static_names));
        if (!count) fputc('-', rows);
        for (i = 0; i < count; ++i)
            fputc(rxop_signal_failure_writes_operand(&signal, i) ? '1' : '0', rows);
        fprintf(rows, " %u %u %u %u %u %d %d %lld %s %d ",
                signal.failure_component_writes, signal.failure_context_writes,
                signal.dependencies, signal.continuations, signal.properties,
                (int)signal.policy_effect, (int)signal.policy_source,
                signal.policy_source_operand == SIZE_MAX ? -1LL : (long long)signal.policy_source_operand,
                nonempty(signal.policy_static_name),
                (effect.semantics & (RXOP_SEM_INDIRECT_WRITE | RXOP_SEM_INDIRECT_BRANCH)) != 0 ||
                (signal.properties & RXOP_SIGNAL_PROP_ASYNC_ENTRY) != 0);
        components(rows, opcode, count, 2);
        fprintf(rows, " %d %d %lld %u %u %u %d %d %d %d ",
                rxop_same_storage_copy_is_noop(opcode),
                (int)rxop_value_derivation(opcode),
                derivation_source == SIZE_MAX ? -1LL : (long long)derivation_source,
                rxop_derivation_source_component(opcode),
                rxop_derivation_context_reads(opcode), rxop_context_writes(opcode),
                rxop_normal_successors(opcode), rxop_normal_fallthrough(opcode),
                (int)rxop_clone_category(opcode), (int)rxop_binding_action(opcode));
        if (!count) fputc('-', rows);
        for (i = 0; i < count; ++i)
            fputc(rxop_component_write_is_known(opcode, i) ? '1' : '0', rows);
        fprintf(rows, " %u %d %u %u %u %u %s %s %d %d %s %d %u %u %u %u ",
                rxop_activation_effects(opcode), (int)call.kind,
                call.target_operand, call.result_operand, call.count_operand,
                call.window_operand, nonempty(call.actual_operands),
                nonempty(call.prefix), call.capture_after_prefix, call.atomic_prefix,
                nonempty(binding_recipe), rxop_return_kind(opcode),
                rxop_return_source_operand(opcode), rxop_context_read_mask(opcode),
                rxop_context_write_mask(opcode), rxop_observation_mask(opcode));
        for (i = 0; i < 5; ++i) {
            target_operands(rows, opcode, count, (int)i); fputc(' ', rows);
        }
        fprintf(rows, "%s %d %d ", nonempty(rxop_implicit_recipe(opcode)),
                binding_recipe != NULL, *call.actual_operands != 0);
        target_operands(rows, opcode, count, 5);
        fprintf(rows, " %u %u %u %u %u ", rxop_transport_source_operand(opcode),
                rxop_transport_components(opcode), rxop_literal_operand(opcode),
                rxop_literal_components(opcode), rxop_scope_continuation_operand(opcode));
        target_operands(rows, opcode, count, 6); fputc(' ', rows);
        target_operands(rows, opcode, count, 7);
        fprintf(rows, " %u ", rxop_observed_flags_mask_operand(opcode));
        target_operands(rows, opcode, count, 8);
        fprintf(rows, " %u %d %u %u %d %u %d %u %u %u %u %u %d %d ",
                rxop_recipe_operand(opcode), rxop_explicit_variant(opcode),
                rxop_return_result_component_invalidations(opcode),
                rxop_return_result_flag_invalidations(opcode),
                rxop_call_invokes(opcode), rxop_call_target_components(opcode),
                (int)selection.kind, selection.result_operand,
                selection.descriptor_operand, selection.receiver_operand,
                selection.count_operand, selection.window_operand,
                !!(effect.semantics & RXOP_SEM_OPAQUE),
                rxop_residual_observable(opcode));
        for (i = 0u; i <= 4u; ++i) {
            if (i) fputc(',', rows);
            fprintf(rows, "%d", rxop_call_variant(opcode, i));
        }
        fprintf(rows, " %u %d %d ", rxop_selection_window_components(opcode),
                rxop_selection_callbacks(opcode), rxop_host_callbacks(opcode));
        target_operands(rows, opcode, count, 9); fputc(' ', rows);
        target_operands(rows, opcode, count, 10); fputc(' ', rows);
        target_operands(rows, opcode, count, 11);
        fprintf(rows, " %d %d ", rxop_abi_callbacks(opcode),
                rxop_departure_callbacks(opcode));
        for (i = 0u; i < RXOP_ABI_MODE_COUNT; ++i) {
            size_t table = abi_table(abi_tables, &abi_count,
                    rxop_abi_contract(opcode, (RxOpAbiMode)i));
            if (table >= ABI_TABLE_LIMIT) {
                fputs("catalog ABI table limit exceeded\n", stderr); goto cleanup;
            }
            if (i) fputc(',', rows);
            fprintf(rows, "%lu", (unsigned long)table);
        }
        fprintf(rows, " %d ", rxop_intrinsic_value_footprint_is_known(opcode));
        target_operands(rows, opcode, count, 12);
        fprintf(rows, " %u", rxop_callback_obligations(opcode));
        fputc('\n', rows);
        row_size[opcode] = (uint32_t)ftell(rows) - row_start[opcode];
    }
    /* Store each native immutable contract once. The opcode row holds only
     * mode-specific table IDs; common leave templates are not duplicated. */
    for (entry = 0u; entry < abi_count; ++entry) {
        const RxOpAbiContract *contract = &abi_tables[entry].contract;
        size_t i;
        abi_tables[entry].start = (uint32_t)ftell(rows);
        fprintf(rows, "%s %lu", rxop_abi_state_name(contract->state),
                (unsigned long)contract->count);
        for (i = 0u; i < contract->count; ++i) {
            const RxOpAbiStep *step = &contract->steps[i];
            fprintf(rows, " %s %s %s %s %s %s %s %s",
                    rxop_abi_operation_name(step->operation),
                    rxop_abi_checkpoint_name(step->checkpoint),
                    rxop_abi_role_name(step->source),
                    rxop_abi_role_name(step->destination),
                    rxop_abi_extent_name(step->source_extent),
                    rxop_abi_extent_name(step->destination_extent),
                    rxop_abi_rule_name(step->rule),
                    rxop_abi_repeat_name(step->repeat));
        }
        fputc('\n', rows);
        abi_tables[entry].size = (uint32_t)ftell(rows) - abi_tables[entry].start;
    }
    qsort(lookup, lookup_count, sizeof(*lookup), compare_lookup);
    for (entry = 0; entry < lookup_count; ++entry) {
        key_start[entry] = (uint32_t)ftell(keys);
        fputs(lookup[entry].key, keys);
        key_size[entry] = (uint32_t)ftell(keys) - key_start[entry];
    }
    write_failed = ferror(rows) || ferror(keys);
    fputs("    _rows = '", out); hex_file(out, rows);
    fputs("'x\n    _rowindex = '", out);
    for (opcode = 0; opcode < OP_MAX_INSTRUCTIONS; ++opcode) {
        hex_u32(out, row_start[opcode]); hex_u32(out, row_size[opcode]);
    }
    fputs("'x\n    _abiindex = '", out);
    for (entry = 0u; entry < abi_count; ++entry) {
        hex_u32(out, abi_tables[entry].start); hex_u32(out, abi_tables[entry].size);
    }
    fputs("'x\n    _keys = '", out); hex_file(out, keys);
    fputs("'x\n    _keyindex = '", out);
    for (entry = 0; entry < lookup_count; ++entry) {
        hex_u32(out, key_start[entry]); hex_u32(out, key_size[entry]);
        hex_u32(out, (uint32_t)lookup[entry].opcode);
    }
    fputs("'x\n", out);
    fprintf(out, "    return\n  count: method = .int\n    return %d\n", OP_MAX_INSTRUCTIONS);
    fputs("  fingerprint: method = .string\n    return ", out); quoted(out, LL_RXAS_CATALOG_FINGERPRINT);
    fputs("\n  version: method = .string\n    return 'llrxas.catalog.v1'\n"
          "  field: method = .string\n    arg opcode = .int, column = .int\n"
          "    if opcode < 0 | opcode >= count() then return ''\n"
          "    start = <at..u32>(opcode * 8) _rowindex\n"
          "    size = <at..u32>(opcode * 8 + 4) _rowindex\n"
          "    row = binsubstr(_rows,start + 1,size) as .string\n    return word(row,column)\n"
          "  find: method = .int\n    arg mnemonic = .string, signature = .string\n"
          "    key = lower(mnemonic) || '|' || signature\n"
          "    low = 0; high = <blen>(_keyindex) <idiv> 12 - 1\n    do while low <= high\n"
          "      middle = low + (high - low) <idiv> 2\n"
          "      start = <at..u32>(middle * 12) _keyindex\n"
          "      size = <at..u32>(middle * 12 + 4) _keyindex\n"
          "      candidate = binsubstr(_keys,start + 1,size) as .string\n"
          "      if candidate = key then return <at..u32>(middle * 12 + 8) _keyindex\n"
          "      if candidate < key then low = middle + 1\n"
          "      else high = middle - 1\n    end\n    return -1\n"
          "  bit: method = .int\n    arg opcode = .int, column = .int, operand = .int\n"
          "    return substr(field(opcode,column),operand,1) = '1'\n"
          "  reads: method = .int\n    arg opcode = .int, operand = .int\n    return bit(opcode,9,operand)\n"
          "  writes: method = .int\n    arg opcode = .int, operand = .int\n    return bit(opcode,10,operand)\n"
          "  kills: method = .int\n    arg opcode = .int, operand = .int\n    return bit(opcode,11,operand)\n"
          "  branch: method = .int\n    arg opcode = .int, operand = .int\n    return bit(opcode,12,operand)\n"
          "  failurewrites: method = .int\n    arg opcode = .int, operand = .int\n    return bit(opcode,20,operand)\n"
          "  /* Component-write coverage is optional cleanup information; an\n"
          "   * unknown projection does not prohibit unchanged cloning. */\n"
          "  writecomplete: method = .int\n    arg opcode = .int, operand = .int\n    return bit(opcode,42,operand)\n"
          "  unbounded: method = .int\n    arg opcode = .int\n    return field(opcode,30) = '1'\n"
          "  readcomponents: method = .int\n    arg opcode = .int, operand = .int\n"
          "    return word(translate(field(opcode,13),' ',','),operand) as .int\n"
          "  writecomponents: method = .int\n    arg opcode = .int, operand = .int\n"
          "    return word(translate(field(opcode,14),' ',','),operand) as .int\n"
          "  /* Operand queries are one-based; derivationsource retains the native\n"
          "   * zero-based operand index, with -1 for no derivation source. */\n"
          "  clearcomponents: method = .int\n    arg opcode = .int, operand = .int\n"
          "    return word(translate(field(opcode,31),' ',','),operand) as .int\n", out);
    fputs("  /* Complete conservative may-effects are separate from optional\n"
          "   * precise success/value projections. No may-write proves a kill. */\n"
          "  relocation: method = .int\n    arg opcode = .int, operand = .int\n"
          "    return word(translate(field(opcode,59),' ',','),operand) as .int\n"
          "  mayreadcomponents: method = .int\n    arg opcode = .int, operand = .int\n"
          "    return word(translate(field(opcode,60),' ',','),operand) as .int\n"
          "  maywritecomponents: method = .int\n    arg opcode = .int, operand = .int\n"
          "    return word(translate(field(opcode,61),' ',','),operand) as .int\n"
          "  mayreadflags: method = .int\n    arg opcode = .int, operand = .int\n"
          "    return word(translate(field(opcode,62),' ',','),operand) as .int\n"
          "  maywriteflags: method = .int\n    arg opcode = .int, operand = .int\n"
          "    return word(translate(field(opcode,63),' ',','),operand) as .int\n"
          "  definitioncomponents: method = .int\n    arg opcode = .int, operand = .int\n"
          "    return word(translate(field(opcode,67),' ',','),operand) as .int\n"
          "  observedcomponents: method = .int\n    arg opcode = .int, operand = .int\n"
          "    return word(translate(field(opcode,73),' ',','),operand) as .int\n"
          "  observedflags: method = .int\n    arg opcode = .int, operand = .int\n"
          "    return word(translate(field(opcode,74),' ',','),operand) as .int\n"
          "  observationcomplete: method = .int\n    arg opcode = .int, operand = .int\n"
          "    return word(translate(field(opcode,76),' ',','),operand) as .int\n", out);
    fputs("  /* Hidden ABI rows use native enums. Unknown is never an empty proof.\n"
          "   * owned-begin..owned-end repeats one body per slot: clear/restore\n"
          "   * slot0, then clear/restore slot1; never two whole-array phases.\n"
          "   * internal-parent requires an ordinary parent with no invocation or\n"
          "   * interrupt-action completion. Unproved is unknown, not a skipped row. */\n"
          "  abifield: method = .string\n    arg opcode = .int, mode = .string, row = .int, column = .int\n"
          "    if opcode < 0 | opcode >= count() | row < 0 | column < 1 | column > 8 then return 'unknown'\n"
          "    selected = 0\n", out);
    for (entry = 0u; entry < RXOP_ABI_MODE_COUNT; ++entry) {
        fputs("    if mode = ", out); quoted(out, rxop_abi_mode_name((RxOpAbiMode)entry));
        fprintf(out, " then selected = %lu\n", (unsigned long)entry + 1u);
    }
    fputs("    if selected = 0 then return 'unknown'\n"
          "    table = word(translate(field(opcode,100),' ',','),selected) as .int\n"
          "    if table < 0 then return 'unknown'\n"
          "    indexbytes = .int\n    indexbytes = <blen>(_abiindex)\n"
          "    entries = indexbytes <idiv> 8\n"
          "    if table >= entries then return 'unknown'\n"
          "    indexoffset = table * 8\n"
          "    start = <at..u32>(indexoffset) _abiindex\n"
          "    sizeoffset = indexoffset + 4\n"
          "    size = <at..u32>(sizeoffset) _abiindex\n"
          "    record = binsubstr(_rows,start + 1,size) as .string\n"
          "    if row = 0 then do\n      if column > 2 then return 'unknown'\n      return word(record,column)\n    end\n"
          "    if word(record,1) <> 'known' then return 'unknown'\n"
          "    if row > word(record,2) as .int then return 'unknown'\n"
          "    return word(record,2 + (row - 1) * 8 + column)\n"
          "  abistate: method = .string\n    arg opcode = .int, mode = .string\n    return abifield(opcode,mode,0,1)\n"
          "  abisteps: method = .int\n    arg opcode = .int, mode = .string\n"
          "    if abistate(opcode,mode) <> 'known' then return 0\n    return abifield(opcode,mode,0,2) as .int\n", out);
    {
        static const char *const fields[] = { "abioperation", "abicheckpoint", "abisource", "abidestination",
                "abisourceextent", "abidestinationextent", "abirule", "abirepeat" };
        for (entry = 0u; entry < sizeof(fields) / sizeof(fields[0]); ++entry)
            fprintf(out, "  %s: method = .string\n    arg opcode = .int, mode = .string, row = .int\n"
                    "    if row < 1 then return 'unknown'\n    return abifield(opcode,mode,row,%lu)\n",
                    fields[entry], (unsigned long)entry + 1u);
    }
    fputs("  callbackinputs: method = .int\n    arg opcode = .int, operand = .int\n"
          "    return word(translate(field(opcode,95),' ',','),operand) as .int\n"
          "  callbackoutputs: method = .int\n    arg opcode = .int, operand = .int\n"
          "    return word(translate(field(opcode,96),' ',','),operand) as .int\n"
          "  callbackcapacity: method = .int\n    arg opcode = .int, operand = .int\n"
          "    return word(translate(field(opcode,97),' ',','),operand) as .int\n"
          "  valuewriter: method = .string\n    arg opcode = .int, operand = .int\n"
          "    if opcode < 0 | opcode >= count() | operand < 1 then return 'unknown'\n"
          "    encoded = field(opcode,102)\n"
          "    if encoded = '-' | operand > words(translate(encoded,' ',',')) then return 'unknown'\n"
          "    kind = word(translate(encoded,' ',','),operand) as .int\n"
          "    if kind < 0 | kind > 3 then return 'unknown'\n"
          "    return word('none field setter unknown',kind + 1)\n", out);
    fputs("  callvariant: method = .int\n    arg opcode = .int, count = .int\n"
          "    if count < 0 | count > 4 then return 0\n"
          "    return word(translate(field(opcode,91),' ',','),count + 1) as .int\n", out);
    fprintf(out, "  callbackmask: method = .int\n    arg kind = .string\n"
                 "    if kind = 'input' then return %u\n"
                 "    if kind = 'output' then return %u\n"
                 "    if kind = 'capacity' then return %u\n"
                 "    if kind = 'selection' then return %u\n"
                 "    if kind = 'host' then return %u\n"
                 "    if kind = 'abi' then return %u\n"
                 "    if kind = 'departure' then return %u\n"
                 "    if kind = 'invocation' then return %u\n"
                 "    return %u\n", RXOP_CALLBACK_INPUT, RXOP_CALLBACK_OUTPUT,
                 RXOP_CALLBACK_CAPACITY, RXOP_CALLBACK_SELECTION,
                 RXOP_CALLBACK_HOST, RXOP_CALLBACK_ABI, RXOP_CALLBACK_DEPARTURE,
                 RXOP_CALLBACK_INVOCATION, RXOP_CALLBACK_UNKNOWN);
    fprintf(out, "  componentmask: method = .int\n    arg type = .string\n"
                 "    if type = 'I' then return %u\n"
                 "    if type = 'F' then return %u\n"
                 "    if type = 'S' then return %u\n"
                 "    if type = 'D' then return %u\n"
                 "    if type = 'B' then return %u\n"
                 "    if type = 'attributes' then return %u\n"
                 "    if type = 'reference' then return %u\n"
                 "    if type = 'native-payload' then return %u\n"
                 "    if type = 'attribute-count' then return %u\n"
                 "    if type = 'object-type' then return %u\n"
                 "    return %u\n", RXOP_COMPONENT_INTEGER, RXOP_COMPONENT_FLOAT,
                 RXOP_COMPONENT_STRING, RXOP_COMPONENT_DECIMAL, RXOP_COMPONENT_BINARY,
                 RXOP_COMPONENT_ATTRIBUTES, RXOP_COMPONENT_REFERENCE,
                 RXOP_COMPONENT_NATIVE_PAYLOAD, RXOP_COMPONENT_ATTRIBUTE_COUNT,
                 RXOP_COMPONENT_OBJECT_TYPE,
                 RXOP_COMPONENT_ALL);
    fprintf(out, "  scopeddiagnostics: method = .int\n    return %d\n",
            rxop_scoped_diagnostics());
    /* Recipe verbs describe existing VM operations; the Level L decoder must
     * obtain setter payload effects from this owner rather than copy masks. */
    fprintf(out, "  recipeintegerclears: method = .int\n    return %u\n",
            rxop_component_clears(OP_LOAD_REG_INT, 0));
    for (entry = 0; entry < sizeof(queries) / sizeof(*queries); ++entry) {
        const Query *query = &queries[entry];
        fprintf(out, "  %s: method = .%s\n    arg opcode = .int\n", query->name,
                query->numeric ? "int" : "string");
        if (!strcmp(query->name, "callbackobligations"))
            fprintf(out, "    if opcode < 0 | opcode >= count() then return %u\n", RXOP_CALLBACK_UNKNOWN);
        if (query->numeric) fprintf(out, "    return field(opcode,%d) as .int\n", query->field);
        else fprintf(out, "    value = field(opcode,%d)\n    if value = '-' then return ''\n"
                          "    return value\n", query->field);
    }
    write_failed = write_failed || ferror(out) || ferror(rows) || ferror(keys);
    if (close_catalog_data(rows, rows_path)) write_failed = 1;
    rows = NULL;
    if (close_catalog_data(keys, keys_path)) write_failed = 1;
    keys = NULL;
    if (fclose(out)) write_failed = 1;
    out = NULL;
    if (write_failed) {
        perror(temporary); goto cleanup;
    }
    if (replace_output(temporary, argv[1])) {
        perror(argv[1]); goto cleanup;
    }
    temporary_owned = 0;
    result = 0;

cleanup:
    if (close_catalog_data(rows, rows_path)) result = 1;
    if (close_catalog_data(keys, keys_path)) result = 1;
    if (out && fclose(out)) result = 1;
    if (temporary_owned && remove(temporary)) perror(temporary);
    free(temporary);
    free(rows_path);
    free(keys_path);
    return result;
}
