/*
 * PERF3-13 E4a independent-load control and E4b sealed-generation proof.
 *
 * This is deliberately an internal structural test, not a public RXVML API.
 * E4a records the fully independent fallback. E4b uses one internal runtime,
 * publishes bytecode-only immutable generations and materializes distinct
 * worker-owned overlays without extending the public RXVML API.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "rxvm.h"
#include "rxvmintp.h"

typedef struct e4a_structure_counts {
    size_t module_count;
    size_t instruction_bytes;
    size_t constant_bytes;
    size_t overlay_bytes_lower_bound;
    size_t semantic_graph_count;
} e4a_structure_counts;

static int failures;

#define CHECK(condition, message)                                             \
    do {                                                                      \
        if (!(condition)) {                                                   \
            fprintf(stderr, "FAIL: %s\n", (message));                        \
            failures++;                                                       \
        }                                                                     \
    } while (0)

static void add_size(size_t *total, size_t count, size_t item_size,
                     const char *description) {
    if (count && item_size > SIZE_MAX / count) {
        fprintf(stderr, "FAIL: size overflow for %s\n", description);
        failures++;
        return;
    }
    if (*total > SIZE_MAX - count * item_size) {
        fprintf(stderr, "FAIL: total size overflow for %s\n", description);
        failures++;
        return;
    }
    *total += count * item_size;
}

static e4a_structure_counts measure_context(const rxvm_context *context) {
    e4a_structure_counts counts = {0};
    size_t i;

    counts.module_count = context->num_modules;
    for (i = 0; i < context->num_modules; i++) {
        const module *mod = context->modules[i];

        add_size(&counts.instruction_bytes, mod->segment.inst_size,
                 sizeof(bin_code), "canonical instructions");
        add_size(&counts.constant_bytes, mod->segment.const_size, 1u,
                 "canonical constants");

        /*
         * This intentionally stays a conservative structural floor. Values,
         * trees, graph bindings, frame recycler contents, names and allocator
         * bookkeeping are not assigned guessed sizes.
         */
        add_size(&counts.overlay_bytes_lower_bound, 1u, sizeof(*mod),
                 "module runtime");
        add_size(&counts.overlay_bytes_lower_bound,
                 (size_t)mod->segment.globals, sizeof(*mod->globals),
                 "global pointers");
        add_size(&counts.overlay_bytes_lower_bound,
                 (size_t)mod->segment.globals,
                 sizeof(*mod->globals_dont_free), "global ownership flags");
        add_size(&counts.overlay_bytes_lower_bound, mod->procedure_count,
                 sizeof(*mod->procedures), "procedure runtimes");
        add_size(&counts.overlay_bytes_lower_bound,
                 mod->proc_runtime_lookup_size,
                 sizeof(*mod->proc_runtime_lookup), "procedure lookup");
        if (mod->execution_image) {
            add_size(&counts.overlay_bytes_lower_bound, mod->segment.inst_size,
                     sizeof(*mod->execution_image), "execution image");
        }
        if (mod->dynamic_site_cache_slots) {
            add_size(&counts.overlay_bytes_lower_bound, mod->segment.inst_size,
                     sizeof(*mod->dynamic_site_cache_slots),
                     "dynamic cache slots");
        }
        add_size(&counts.overlay_bytes_lower_bound,
                 mod->dynamic_site_cache_count,
                 sizeof(*mod->dynamic_site_caches), "dynamic caches");
        if (mod->file && mod->file->semantic_graph) {
            counts.semantic_graph_count++;
        }
    }
    return counts;
}

static void compare_independent_modules(const module *left,
                                        const module *right) {
    size_t i;

    CHECK(left != right, "module runtime objects are independent");
    CHECK(left->memory_worker != right->memory_worker,
          "module allocator owners are independent");
    CHECK(left->file != right->file, "materialized file sections are independent");
    CHECK(left->segment.module == left && right->segment.module == right,
          "binary spaces retain their local module owner");
    CHECK(left->segment.globals == right->segment.globals,
          "global counts are equivalent");
    CHECK(left->segment.inst_size == right->segment.inst_size,
          "instruction sizes are equivalent");
    CHECK(left->segment.const_size == right->segment.const_size,
          "constant sizes are equivalent");
    CHECK(left->procedure_count == right->procedure_count,
          "procedure counts are equivalent");
    CHECK(left->proc_runtime_lookup_size == right->proc_runtime_lookup_size,
          "procedure lookup sizes are equivalent");
    CHECK(left->dynamic_site_cache_count == right->dynamic_site_cache_count,
          "dynamic cache counts are equivalent");
    CHECK(left->state == right->state, "module lifecycle states are equivalent");
    CHECK(left->name && right->name && strcmp(left->name, right->name) == 0,
          "module names are equivalent");

    if (left->segment.inst_size && right->segment.inst_size) {
        size_t bytes = left->segment.inst_size * sizeof(bin_code);
        CHECK(left->segment.binary != right->segment.binary,
              "canonical instruction storage is currently duplicated");
        CHECK(memcmp(left->segment.binary, right->segment.binary, bytes) == 0,
              "canonical instruction content is equivalent");
        CHECK(left->execution_image && right->execution_image,
              "both contexts own prepared execution images");
        CHECK(left->execution_image != right->execution_image,
              "prepared execution images are independent");
    }
    if (left->segment.const_size && right->segment.const_size) {
        CHECK(left->segment.const_pool != right->segment.const_pool,
              "canonical constant storage is currently duplicated");
        CHECK(memcmp(left->segment.const_pool, right->segment.const_pool,
                     left->segment.const_size) == 0,
              "canonical constant content is equivalent");
    }
    if (left->segment.globals && right->segment.globals) {
        CHECK(left->globals != right->globals,
              "global pointer tables are independent");
        CHECK(left->globals_dont_free != right->globals_dont_free,
              "global ownership maps are independent");
        for (i = 0; i < (size_t)left->segment.globals; i++) {
            CHECK(left->globals[i] != right->globals[i],
                  "mutable global values are independent");
        }
    }
    if (left->procedure_count && right->procedure_count) {
        CHECK(left->procedures != right->procedures,
              "procedure runtimes are independent");
        for (i = 0; i < left->procedure_count; i++) {
            CHECK(left->procedures[i].definition !=
                      right->procedures[i].definition,
                  "procedure definitions point into independent pools");
            CHECK(left->procedures[i].frame_free_list !=
                      right->procedures[i].frame_free_list,
                  "procedure frame recyclers are independent");
        }
    }
    if (left->proc_runtime_lookup_size && right->proc_runtime_lookup_size) {
        CHECK(left->proc_runtime_lookup != right->proc_runtime_lookup,
              "procedure lookup tables are independent");
    }
    if (left->dynamic_site_cache_slots && right->dynamic_site_cache_slots) {
        CHECK(left->dynamic_site_cache_slots != right->dynamic_site_cache_slots,
              "dynamic cache slot maps are independent");
    }
    if (left->dynamic_site_cache_count && right->dynamic_site_cache_count) {
        CHECK(left->dynamic_site_caches != right->dynamic_site_caches,
              "dynamic caches are independent");
    }
    if (left->file && right->file && left->file->semantic_graph &&
        right->file->semantic_graph) {
        CHECK(left->file->semantic_graph != right->file->semantic_graph,
              "semantic graphs are currently independently materialized");
    }
    if (left->file && right->file && left->file->shared_constant_pool &&
        right->file->shared_constant_pool) {
        CHECK(left->file->shared_constant_pool !=
                  right->file->shared_constant_pool,
              "linked-image shared pools do not cross contexts today");
    }
    if (left->graph_binding && right->graph_binding) {
        CHECK(left->graph_binding != right->graph_binding,
              "semantic callable bindings are independent");
    }
}

static void compare_independent_contexts(const rxvm_context *left,
                                         const rxvm_context *right) {
    size_t i;

    CHECK(left != right, "VM contexts are independent");
    CHECK(left->worker.runtime != right->worker.runtime,
          "VM runtimes are independent");
    CHECK(left->worker.memory_worker != right->worker.memory_worker,
          "VM allocator workers are independent");
    CHECK(left->modules != right->modules, "module tables are independent");
    CHECK(left->num_modules == right->num_modules,
          "independent loads contain the same module count");
    for (i = 0; i < left->num_modules && i < right->num_modules; i++) {
        compare_independent_modules(left->modules[i], right->modules[i]);
    }
}

static void compare_shared_modules(const module *left,
                                   const module *right) {
    size_t i;

    CHECK(left != right, "shared-image module overlays are independent");
    CHECK(left->memory_worker != right->memory_worker,
          "shared-image overlays retain distinct allocator owners");
    CHECK(left->file == right->file,
          "materialized immutable file descriptor is shared");
    CHECK(left->segment.module == left && right->segment.module == right,
          "shared binary spaces retain local module back-pointers");
    CHECK(left->segment.binary == right->segment.binary,
          "canonical instruction storage is shared");
    CHECK(left->segment.const_pool == right->segment.const_pool,
          "canonical constant storage is shared");
    CHECK(left->name == right->name && left->description == right->description,
          "immutable module text descriptors are shared");
    CHECK(left->file->semantic_graph == right->file->semantic_graph,
          "semantic graph identity is shared");
    CHECK(left->state == right->state,
          "worker-local module lifecycle states are equivalent");
    CHECK(left->globals != right->globals,
          "global pointer tables remain worker-owned");
    CHECK(left->globals_dont_free != right->globals_dont_free,
          "global ownership maps remain worker-owned");
    for (i = 0u; i < (size_t)left->segment.globals; i++) {
        CHECK(left->globals[i] != right->globals[i],
              "mutable global values remain worker-owned");
    }
    CHECK(left->procedures != right->procedures,
          "procedure runtimes remain worker-owned");
    CHECK(left->procedure_count == right->procedure_count,
          "procedure runtime counts are equivalent");
    for (i = 0u; i < left->procedure_count; i++) {
        CHECK(left->procedures[i].definition ==
                  right->procedures[i].definition,
              "procedure definitions share canonical metadata");
        CHECK(left->procedures[i].frame_free_list !=
                  right->procedures[i].frame_free_list,
              "procedure frame recyclers remain worker-owned");
    }
    CHECK(left->proc_runtime_lookup != right->proc_runtime_lookup,
          "procedure runtime lookups remain worker-owned");
    CHECK(left->execution_image && right->execution_image &&
              left->execution_image != right->execution_image,
          "prepared execution images remain worker-owned");
    if (left->dynamic_site_cache_slots && right->dynamic_site_cache_slots) {
        CHECK(left->dynamic_site_cache_slots !=
                  right->dynamic_site_cache_slots,
              "dynamic cache slot maps remain worker-owned");
    }
    if (left->dynamic_site_cache_count && right->dynamic_site_cache_count) {
        CHECK(left->dynamic_site_caches != right->dynamic_site_caches,
              "dynamic caches remain worker-owned");
    }
    if (left->graph_binding && right->graph_binding) {
        CHECK(left->graph_binding != right->graph_binding,
              "semantic callable bindings remain worker-owned");
    }
}

static void compare_shared_contexts(const rxvm_context *left,
                                    const rxvm_context *right) {
    size_t i;

    CHECK(left != right, "shared-runtime VM contexts are independent");
    CHECK(left->worker.runtime == right->worker.runtime,
          "worker VMs share one runtime domain");
    CHECK(left->worker.memory_worker != right->worker.memory_worker,
          "worker VMs retain distinct allocator workers");
    CHECK(left->modules != right->modules,
          "worker VMs retain distinct module tables");
    CHECK(left->program_generation &&
              left->program_generation == right->program_generation,
          "worker VMs pin the same sealed generation");
    CHECK(left->num_modules == right->num_modules,
          "shared-generation workers contain the same module count");
    for (i = 0u; i < left->num_modules && i < right->num_modules; i++) {
        compare_shared_modules(left->modules[i], right->modules[i]);
    }
}

static int call_one(rxvm_context *context, const char *procedure,
                    const char *argument) {
    char *arguments[1];

    arguments[0] = (char *)argument;
    return rxvm_call(context, (char *)procedure, 1, arguments);
}

static void run_independent_control(const char *control_rxbin,
                                    const char *late_rxbin) {
    rxvm_context *left = 0;
    rxvm_context *right = 0;
    e4a_structure_counts left_counts;
    e4a_structure_counts right_counts;
    uint64_t left_generation;
    uint64_t right_generation;
    size_t initial_modules;
    size_t immutable_candidate_bytes;

    left = rxvm_create();
    right = rxvm_create();
    CHECK(left && right, "create two VM contexts");
    if (!left || !right) goto cleanup;

    CHECK(rxvm_load_file(left, (char *)control_rxbin) != 0,
          "load control image into left context");
    CHECK(rxvm_load_file(right, (char *)control_rxbin) != 0,
          "load control image into right context");
    CHECK(rxvm_link(left) == 0 && rxvm_link(right) == 0,
          "link independent control images");
    CHECK(rxvm_prepare(left) == 0 && rxvm_prepare(right) == 0,
          "prepare independent control images");
    if (failures) goto cleanup;

    compare_independent_contexts(left, right);
    left_counts = measure_context(left);
    right_counts = measure_context(right);
    CHECK(left_counts.module_count == right_counts.module_count &&
              left_counts.instruction_bytes == right_counts.instruction_bytes &&
              left_counts.constant_bytes == right_counts.constant_bytes &&
              left_counts.overlay_bytes_lower_bound ==
                  right_counts.overlay_bytes_lower_bound &&
              left_counts.semantic_graph_count ==
                  right_counts.semantic_graph_count,
          "independent contexts have equivalent structural counts");

    immutable_candidate_bytes = left_counts.instruction_bytes;
    add_size(&immutable_candidate_bytes, left_counts.constant_bytes, 1u,
             "immutable candidate total");
    printf("E4A_CONTROL phase=initial modules=%zu "
           "canonical_instruction_bytes=%zu canonical_constant_bytes=%zu "
           "immutable_candidate_bytes_per_context_lower_bound=%zu "
           "duplicated_immutable_bytes_two_contexts_lower_bound=%zu "
           "runtime_overlay_bytes_per_context_lower_bound=%zu "
           "semantic_graphs_unmeasured=%zu\n",
           left_counts.module_count, left_counts.instruction_bytes,
           left_counts.constant_bytes, immutable_candidate_bytes,
           immutable_candidate_bytes, left_counts.overlay_bytes_lower_bound,
           left_counts.semantic_graph_count);

    CHECK(rxvm_call(left, "e4control.get", 0, 0) == 0,
          "left mutable global starts at zero");
    CHECK(rxvm_call(right, "e4control.get", 0, 0) == 0,
          "right mutable global starts at zero");
    CHECK(call_one(left, "e4control.set", "41") == 41,
          "left context mutates its own global");
    CHECK(rxvm_call(right, "e4control.get", 0, 0) == 0,
          "left mutation is invisible to right context");
    CHECK(call_one(right, "e4control.set", "17") == 17,
          "right context mutates its own global");
    CHECK(rxvm_call(left, "e4control.get", 0, 0) == 41,
          "right mutation is invisible to left context");

    initial_modules = left->num_modules;
    left_generation = left->semantic_generation;
    right_generation = right->semantic_generation;
    CHECK(initial_modules == right->num_modules,
          "module counts match before late load");
    CHECK(left_generation == right_generation,
          "semantic generations match before late load");

    CHECK(rxvm_load_file(left, (char *)late_rxbin) != 0,
          "late-load image into left context");
    CHECK(rxvm_link(left) == 0 && rxvm_prepare(left) == 0,
          "link and prepare left late-load image");
    CHECK(left->num_modules > initial_modules,
          "left context publishes a new module");
    CHECK(right->num_modules == initial_modules,
          "right module table is unchanged by left late load");
    CHECK(left->semantic_generation != left_generation,
          "left semantic generation advances after late load");
    CHECK(right->semantic_generation == right_generation,
          "right semantic generation is unchanged by left late load");
    CHECK(call_one(left, "e4late.identity", "9") == 10,
          "left late-loaded procedure executes");
    CHECK(rxvm_call(left, "e4control.get", 0, 0) == 41 &&
              rxvm_call(right, "e4control.get", 0, 0) == 17,
          "late load preserves independent existing globals");

    CHECK(rxvm_load_file(right, (char *)late_rxbin) != 0,
          "late-load image independently into right context");
    CHECK(rxvm_link(right) == 0 && rxvm_prepare(right) == 0,
          "link and prepare right late-load image");
    CHECK(right->semantic_generation != right_generation,
          "right semantic generation advances on its own late load");
    CHECK(call_one(right, "e4late.identity", "19") == 20,
          "right late-loaded procedure executes");
    compare_independent_contexts(left, right);

cleanup:
    if (left) rxvm_destroy(left);
    if (right) rxvm_destroy(right);
    if (!failures) {
        printf("E4A_CONTROL isolation=PASS execution=PASS late_load=PASS "
               "teardown=PASS result=PASS\n");
    }
}

static void run_shared_generation(const char *control_rxbin,
                                  const char *late_rxbin) {
    rxvm_runtime *runtime = 0;
    rxvm_context *source = 0;
    rxvm_context *peer = 0;
    const rxvm_program_generation *initial_generation = 0;
    const rxvm_program_generation *derived_generation = 0;
    e4a_structure_counts source_counts;
    module *peer_initial_module = 0;
    proc_runtime *peer_initial_procedures = 0;
    value *peer_initial_global = 0;
    size_t initial_modules = 0u;
    size_t instruction_bytes;
    size_t constant_bytes;
    size_t canonical_bytes;
    size_t runtime_leaks = 0u;
    int failures_before = failures;

    runtime = rxvm_runtime_create();
    CHECK(runtime != 0, "create shared E4b runtime");
    if (!runtime) goto cleanup;
    source = rxvm_context_create_in_runtime(runtime);
    peer = rxvm_context_create_in_runtime(runtime);
    CHECK(source && peer, "create two worker VMs in one runtime");
    CHECK(rxvm_runtime_worker_count(runtime) == 2u,
          "shared runtime registers both worker VMs");
    if (!source || !peer) goto cleanup;

    CHECK(rxvm_load_file(source, (char *)control_rxbin) != 0,
          "load initial bytecode into generation source");
    CHECK(rxvm_link(source) == 0 && rxvm_prepare(source) == 0,
          "link and prepare initial generation source");
    CHECK(rxvm_program_generation_seal(source, &initial_generation) ==
              RXVM_PROGRAM_OK && initial_generation,
          "seal initial bytecode generation");
    CHECK(rxvm_program_generation_current(source) == initial_generation,
          "source pins its published generation");
    CHECK(rxvm_program_generation_seal(source, &derived_generation) ==
              RXVM_PROGRAM_OK && derived_generation == initial_generation,
          "sealing an unchanged context is idempotent");
    derived_generation = 0;

    CHECK(rxvm_program_generation_attach(peer, initial_generation) ==
              RXVM_PROGRAM_OK,
          "attach peer worker to initial generation");
    CHECK(rxvm_link(peer) == 0 && rxvm_prepare(peer) == 0,
          "link and prepare peer worker overlay");
    if (failures != failures_before) goto cleanup;

    compare_shared_contexts(source, peer);
    source_counts = measure_context(source);
    instruction_bytes =
            rxvm_program_generation_instruction_bytes(initial_generation);
    constant_bytes =
            rxvm_program_generation_constant_bytes(initial_generation);
    canonical_bytes = instruction_bytes;
    add_size(&canonical_bytes, constant_bytes, 1u,
             "shared immutable candidate total");
    CHECK(rxvm_program_generation_id(initial_generation) != 0u,
          "sealed generation has a non-zero runtime identity");
    CHECK(rxvm_program_generation_module_count(initial_generation) ==
              source->num_modules,
          "sealed generation records every bytecode module");
    CHECK(instruction_bytes == source_counts.instruction_bytes,
          "generation counts each canonical instruction backing once");
    CHECK(constant_bytes <= source_counts.constant_bytes,
          "generation counts shared constant backing once");
    printf("E4B_CONTROL phase=initial modules=%zu generation=%llu "
           "canonical_instruction_bytes_unique=%zu "
           "canonical_constant_bytes_unique=%zu "
           "canonical_bytes_unique_two_contexts_lower_bound=%zu "
           "independent_duplicate_bytes_removed_lower_bound=%zu "
           "runtime_overlay_bytes_per_context_lower_bound=%zu\n",
           source->num_modules,
           (unsigned long long)rxvm_program_generation_id(initial_generation),
           instruction_bytes, constant_bytes, canonical_bytes, canonical_bytes,
           source_counts.overlay_bytes_lower_bound);

    CHECK(rxvm_call(source, "e4control.get", 0, 0) == 0 &&
              rxvm_call(peer, "e4control.get", 0, 0) == 0,
          "shared generation starts with independent zero globals");
    CHECK(call_one(source, "e4control.set", "41") == 41,
          "source worker mutates its own global");
    CHECK(rxvm_call(peer, "e4control.get", 0, 0) == 0,
          "source global mutation is invisible to peer");
    CHECK(call_one(peer, "e4control.set", "17") == 17,
          "peer worker mutates its own global");
    CHECK(rxvm_call(source, "e4control.get", 0, 0) == 41,
          "peer global mutation is invisible to source");

    initial_modules = source->num_modules;
    peer_initial_module = peer->modules[0];
    peer_initial_procedures = peer_initial_module->procedures;
    if (peer_initial_module->segment.globals) {
        peer_initial_global = peer_initial_module->globals[0];
    }
    CHECK(rxvm_load_file(source, (char *)late_rxbin) != 0,
          "late-load bytecode into generation source");
    CHECK(rxvm_link(source) == 0 && rxvm_prepare(source) == 0,
          "link and prepare source late-load overlay");
    CHECK(rxvm_program_generation_seal(source, &derived_generation) ==
              RXVM_PROGRAM_OK && derived_generation,
          "publish append-only derived generation");
    CHECK(derived_generation != initial_generation &&
              rxvm_program_generation_id(derived_generation) >
                  rxvm_program_generation_id(initial_generation),
          "derived generation has a later immutable identity");
    CHECK(rxvm_program_generation_module_count(derived_generation) >
              rxvm_program_generation_module_count(initial_generation),
          "derived generation appends the late bytecode image");
    CHECK(peer->num_modules == initial_modules &&
              rxvm_program_generation_current(peer) == initial_generation,
          "peer remains pinned to the old generation");
    CHECK(rxvm_call(peer, "e4control.get", 0, 0) == 17,
          "old-generation peer remains executable and isolated");
    CHECK(call_one(source, "e4late.identity", "9") == 10,
          "source executes its derived-generation procedure");

    CHECK(rxvm_program_generation_attach(peer, derived_generation) ==
              RXVM_PROGRAM_OK,
          "advance peer to compatible derived generation");
    CHECK(peer->modules[0] == peer_initial_module &&
              peer->modules[0]->procedures == peer_initial_procedures,
          "generation advance preserves existing overlay addresses");
    if (peer_initial_global) {
        CHECK(peer->modules[0]->globals[0] == peer_initial_global,
              "generation advance preserves existing global identity");
    }
    CHECK(rxvm_link(peer) == 0 && rxvm_prepare(peer) == 0,
          "link and prepare appended peer overlay");
    CHECK(rxvm_call(peer, "e4control.get", 0, 0) == 17,
          "generation advance preserves existing peer global value");
    CHECK(call_one(peer, "e4late.identity", "19") == 20,
          "peer executes the shared late bytecode image");
    CHECK(rxvm_call(source, "e4control.get", 0, 0) == 41,
          "peer generation advance does not disturb source state");
    compare_shared_contexts(source, peer);

    /* One context may disappear while the other keeps generation storage. */
    rxvm_destroy(source);
    source = 0;
    CHECK(rxvm_runtime_worker_count(runtime) == 1u,
          "runtime retains one worker after source teardown");
    CHECK(call_one(peer, "e4late.identity", "29") == 30,
          "peer remains executable after source generation pin releases");

cleanup:
    if (source) rxvm_destroy(source);
    if (peer) rxvm_destroy(peer);
    if (runtime) {
        CHECK(rxvm_runtime_worker_count(runtime) == 0u,
              "all shared-runtime workers unregister before teardown");
        runtime_leaks = rxvm_runtime_destroy(runtime);
        CHECK(runtime_leaks == 0u,
              "shared runtime generation catalogue tears down without leaks");
    }
    if (failures == failures_before) {
        printf("E4B_CONTROL immutable_sharing=PASS overlay_isolation=PASS "
               "late_generation=PASS teardown=PASS result=PASS\n");
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s CONTROL_RXBIN LATE_RXBIN\n", argv[0]);
        return 2;
    }

    run_independent_control(argv[1], argv[2]);
    if (!failures) run_shared_generation(argv[1], argv[2]);
    if (failures) {
        fprintf(stderr, "E4_PROGRAM_CONTROL result=FAIL failures=%d\n",
                failures);
        return 1;
    }
    printf("E4_PROGRAM_CONTROL result=PASS\n");
    return 0;
}
