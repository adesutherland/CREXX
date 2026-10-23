/*
 * Bounded same-process ownership regressions, using both existing VM cores.
 * Checkpoint counters describe only RXVM ownership and the native test fixture.
 * RSS, arbitrary provider-private allocators and physical Apple DSO unloading
 * are deliberately not inferred from these logical counters.
 */
#include "rxvm.h"
#include "rxvmintp.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sustained_vm_provider_sessions(rxinteger *created, rxinteger *destroyed);

enum { BATCHES = 3, DEPOT_MAX_SLABS = 32 };

typedef struct checkpoint {
    rxvm_memory_stats memory;
    uint64_t frame_count, frame_slots, frame_capacity, frame_geometry;
    size_t plugin_instances, catalogue_entries, logical_handles;
    rxinteger provider_created, provider_destroyed;
    int provider_live;
} checkpoint;

static int failed;

#define CHECK(condition, message) do {                                        \
    if (!(condition)) {                                                       \
        fprintf(stderr, "FAIL: %s\n", (message));                              \
        failed = 1;                                                          \
    }                                                                        \
} while (0)

static int call(rxvm_context *context, const char *name, int argc, char **argv) {
    int rc = rxvm_call(context, (char *)name, argc, argv);
    if (rc) {
        fprintf(stderr, "FAIL: %s returned %d\n", name, rc);
        failed = 1;
    }
    return rc == 0;
}

static uint64_t mix(uint64_t hash, uint64_t item) {
    return (hash ^ item) * UINT64_C(1099511628211);
}

/* The allocator rounds byte blocks to a power-of-two class; oversized extents
 * retain exact requested capacity at a 16-byte-aligned address. The frame contains two nominal pointer arrays and only
 * locals+a0 owned values; globals/arguments are borrowed mappings. */
static size_t frame_block_capacity(size_t bytes) {
    size_t capacity = 16u;
    if (bytes > RXVM_MEMORY_MAX_STANDARD_SIZE) return bytes;
    while (capacity < bytes) capacity *= 2u;
    return capacity;
}

static void capture(rxvm_context *context, const char *scenario, checkpoint *point) {
    size_t module_index, procedure_index;
    rxvm_memory_worker *previous;
    memset(point, 0, sizeof(*point));
    if (strcmp(scenario, "providers") == 0) {
        point->provider_live = rxvm_call(context, "sustainedproviders.live", 0, NULL);
        CHECK(point->provider_live >= 0, "native fixture live-resource query succeeds");
    }
    CHECK(rxvm_worker_get_state(&context->worker) == RXVM_WORKER_IDLE &&
          context->worker.execution_depth == 0u,
          "checkpoint is outside execution and native callbacks");
    CHECK(sustained_vm_provider_sessions(&point->provider_created,
                                        &point->provider_destroyed),
          "native fixture restored its previous session binding");
    rxvm_memory_get_stats(rxvm_runtime_memory_context(context->worker.runtime),
                          &point->memory);
    point->plugin_instances = rxvmplugin_live_instance_count();
    point->catalogue_entries = rxvmplugin_catalogue_count();
    point->logical_handles = rxpa_live_plugin_handle_count();
    point->frame_geometry = UINT64_C(1469598103934665603);
    previous = rxvm_memory_enter(context->worker.memory_worker);
    for (module_index = 0u; module_index < context->num_modules; ++module_index) {
        module *loaded = context->modules[module_index];
        for (procedure_index = 0u; procedure_index < loaded->procedure_count;
             ++procedure_index) {
            proc_runtime *procedure = &loaded->procedures[procedure_index];
            stack_frame *frame;
            uint64_t frames = 0u, slots = 0u, capacity = 0u;
            /* Imported aliases share the owner's list and must not be counted. */
            if (procedure->frame_free_list != &procedure->frame_free_list_head) continue;
            for (frame = procedure->frame_free_list_head; frame; frame = frame->prev_free) {
                size_t block_bytes, actual_capacity;
                if (++point->frame_count > point->memory.live_allocations) {
                    CHECK(0, "frame recycler is acyclic and every frame is owned");
                    rxvm_memory_leave(previous);
                    return;
                }
                CHECK(frame->procedure == procedure && procedure->locals >= 0 &&
                      frame->number_locals <= frame->nominal_number_locals,
                      "retained frame preserves its owning procedure geometry");
                block_bytes = sizeof(*frame) +
                        2u * frame->nominal_number_locals * sizeof(value *) +
                        ((size_t)procedure->locals + 1u) * sizeof(value);
                actual_capacity = rxvm_memory_capacity(frame);
                CHECK(actual_capacity == frame_block_capacity(block_bytes),
                      "retained frame capacity matches its register/block geometry");
                ++frames;
                slots += frame->nominal_number_locals;
                capacity += actual_capacity;
            }
            point->frame_slots += slots;
            point->frame_capacity += capacity;
            point->frame_geometry = mix(mix(mix(point->frame_geometry,
                                                frames), slots), capacity);
        }
    }
    rxvm_memory_leave(previous);
    CHECK(point->memory.allocation_failures == 0u &&
          point->memory.invalid_frees == 0u &&
          point->memory.wrong_owner_allocations == 0u &&
          point->memory.wrong_owner_resizes == 0u &&
          point->memory.wrong_owner_frees == 0u,
          "bounded workload has no allocator errors");
    CHECK(point->memory.standard_slabs_reserved <= DEPOT_MAX_SLABS,
          "central empty-slab depot stays within 32 x 64KiB (not a heap cap)");
    /* At most one local empty slab per worker/class; every other local slab
     * needs at least one live allocation. This is a geometry-derived bound,
     * separate from the tighter frozen warmed ownership comparison below. */
    CHECK(point->memory.standard_slabs_owned <=
          point->memory.live_allocations + RXVM_MEMORY_CLASS_COUNT,
          "one-worker slab ownership respects live slots plus class reserves");
}

static void plateau(const checkpoint *warm, const checkpoint *point) {
    size_t class_index;
    CHECK(point->memory.live_allocations <= warm->memory.live_allocations,
          "live allocation count does not grow after warmup");
    CHECK(point->memory.live_capacity_bytes <= warm->memory.live_capacity_bytes,
          "owned live capacity does not grow after warmup");
    CHECK(point->memory.oversized_live_allocations <= warm->memory.oversized_live_allocations &&
          point->memory.oversized_live_bytes <= warm->memory.oversized_live_bytes,
          "oversized ownership does not grow after warmup");
    CHECK(point->memory.standard_slabs_owned <= warm->memory.standard_slabs_owned,
          "retained worker slab capacity does not grow after warmup");
    CHECK(point->memory.standard_slabs_reserved <= warm->memory.standard_slabs_reserved,
          "central cached reserve does not grow after warmup");
    for (class_index = 0u; class_index < RXVM_MEMORY_CLASS_COUNT; ++class_index) {
        CHECK(point->memory.class_live_allocations[class_index] <=
              warm->memory.class_live_allocations[class_index],
              "each allocator class plateaus after warmup");
    }
    CHECK(point->frame_count == warm->frame_count &&
          point->frame_slots == warm->frame_slots &&
          point->frame_capacity == warm->frame_capacity &&
          point->frame_geometry == warm->frame_geometry,
          "per-procedure warmed frame/register capacity stays fixed");
    CHECK(point->plugin_instances == warm->plugin_instances &&
          point->catalogue_entries == warm->catalogue_entries &&
          point->logical_handles == warm->logical_handles,
          "VM instances, catalogue and logical RXPA handles plateau");
    CHECK(point->provider_created == warm->provider_created &&
          point->provider_destroyed == warm->provider_destroyed &&
          point->provider_live == warm->provider_live,
          "existing native fixture session/resource ownership plateaus");
}

static int operations(rxvm_context *context, const char *scenario, size_t count) {
    static char *modes[] = {"0", "1", "2", "3", "0", "0", "1", "2", "4", "0"};
    size_t index;
    for (index = 0u; index < count && !failed; ++index) {
        if (strcmp(scenario, "values") == 0) {
            (void)call(context, "sustainedvalues.cycle", 0, NULL);
        } else if (strcmp(scenario, "providers") == 0) {
            (void)call(context, "sustainedproviders.cycle", 0, NULL);
        } else {
            /* Exactly one request per operation. Each invalid-source/format
             * request is immediately followed by a successful RXAS request. */
            (void)call(context, "sustainedlevell.cycle", 1, &modes[index % 10u]);
        }
    }
    return !failed;
}

int main(int argc, char **argv) {
    rxvm_runtime *runtime;
    rxvm_context *context;
    rxvm_memory_context *memory;
    checkpoint warm, point;
    rxvm_memory_stats final;
    rxinteger created, destroyed;
    size_t count, batch, class_index;
    char *end;
    int levell;
    if (argc < 4 || (strcmp(argv[1], "values") != 0 &&
                     strcmp(argv[1], "providers") != 0 &&
                     strcmp(argv[1], "levell") != 0)) {
        fprintf(stderr, "usage: harness values|providers|levell fixture.rxbin batch-count [spec output]\n");
        return 2;
    }
    count = (size_t)strtoul(argv[3], &end, 10);
    levell = strcmp(argv[1], "levell") == 0;
    if (*end || !count || count > 100000u ||
        (levell && (argc != 6 || count % 10u != 0u)) || (!levell && argc != 4)) return 2;
    runtime = rxvm_runtime_create();
    context = runtime ? rxvm_context_create_in_runtime(runtime) : NULL;
    if (!context) {
        if (runtime) (void)rxvm_runtime_destroy(runtime);
        return 1;
    }
    memory = rxvm_runtime_memory_context(runtime);
    CHECK(rxvm_load_file(context, argv[2]) != NULL, "load the owned fixture image");
    if (!failed) CHECK(rxldmodp(context) >= 0, "load the owned static provider catalogue");
    if (!failed) CHECK(rxvm_initialize(context) == 0, "link, prepare and initialize fixture");
    if (!failed && levell) (void)call(context, "sustainedlevell.prepare", 2, &argv[4]);
    /* Warm every fixed operation/failure/recovery shape, including the live
     * query and generated-output reference buffers, before freezing geometry. */
    if (!failed) {
        (void)operations(context, argv[1], levell ? count : 10u);
        capture(context, argv[1], &warm);
        if (strcmp(argv[1], "providers") == 0) {
            CHECK(warm.provider_created > 0 && warm.provider_destroyed == 0,
                  "the owned static provider session executes and remains alive until context teardown");
        }
        for (batch = 0u; batch < BATCHES && !failed; ++batch) {
            (void)operations(context, argv[1], count);
            capture(context, argv[1], &point);
            plateau(&warm, &point);
        }
        if (!failed) {
            printf("SUSTAINED_VM|scenario=%s|batches=%d|operations=%zu|live=%" PRIu64
                   "|capacity=%" PRIu64 "|frames=%" PRIu64 "|slots=%" PRIu64
                   "|frame_capacity=%" PRIu64 "|worker_slabs=%" PRIu64 "|depot_slabs=%" PRIu64 "\n",
                   argv[1], BATCHES, count, warm.memory.live_allocations,
                   warm.memory.live_capacity_bytes, warm.frame_count,
                   warm.frame_slots, warm.frame_capacity,
                   warm.memory.standard_slabs_owned, warm.memory.standard_slabs_reserved);
        }
        if (levell) (void)call(context, "sustainedlevell.cleanup", 0, NULL);
    }
    /* Trim is legal here only because the owning worker is idle. It releases
     * the central reserve, not intentionally retained live/cached frames. */
    rxvm_memory_trim(memory);
    rxvm_memory_get_stats(memory, &final);
    CHECK(final.standard_slabs_reserved == 0u, "idle safe-point trim releases the depot reserve");
    /* Debug rxfremod itself aborts on live ownership before forced arena
     * release. The explicit runtime survives the context to permit checks at
     * the correct post-worker lifecycle boundary. */
    rxvm_destroy(context);
    rxvm_memory_get_stats(memory, &final);
    CHECK(rxvm_runtime_worker_count(runtime) == 0u && final.live_allocations == 0u &&
          final.live_capacity_bytes == 0u && final.oversized_live_allocations == 0u &&
          final.oversized_live_bytes == 0u && final.standard_slabs_owned == 0u,
          "context/worker teardown leaves no live runtime ownership");
    for (class_index = 0u; class_index < RXVM_MEMORY_CLASS_COUNT; ++class_index)
        CHECK(final.class_live_allocations[class_index] == 0u,
              "every allocator class has zero live ownership after worker teardown");
    CHECK(final.standard_slabs_reserved <= DEPOT_MAX_SLABS,
          "worker teardown preserves the central depot bound");
    CHECK(rxvmplugin_live_instance_count() == 0u && rxpa_live_plugin_handle_count() == 0u,
          "context teardown releases VM plugin instances and logical DSO handles");
    CHECK(sustained_vm_provider_sessions(&created, &destroyed) && created == destroyed,
          "all existing native fixture sessions and their payload owners are destroyed");
    rxvm_memory_trim(memory);
    rxvm_memory_get_stats(memory, &final);
    CHECK(final.standard_slabs_reserved == 0u, "post-worker trim leaves no central reserve");
    CHECK(rxvm_runtime_destroy(runtime) == 0u, "runtime teardown reports zero live allocations");
    clear_rxvmplugin_factories();
    CHECK(rxvmplugin_catalogue_count() == 0u && rxvmplugin_live_instance_count() == 0u,
          "final catalogue teardown releases all logical plugin ownership");
    if (!failed) printf("SUSTAINED_VM|failures=0\n");
    return failed;
}
