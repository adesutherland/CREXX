/*
 * cREXX License (MIT)
 *
 * Standalone semantic-graph size and query benchmark. This executable links
 * the production rxbin/rxgraph library but does not link or rebuild the VM.
 */

#include "rxbin.h"
#include "rxgraph.h"
#include "../../interpreter/rxvalue.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define RXGRAPH_BENCH_SCHEMA 1u
#define RXGRAPH_SECTION_COUNT 6u
#define RXGRAPH_HEADER_SIZE 64u
#define RXGRAPH_DIRECTORY_ROW_SIZE 40u

typedef struct ImageModules {
    module_file **items;
    size_t count;
    RxGraph *graph;
} ImageModules;

typedef struct ImageLayout {
    uint64_t file_size;
    uint32_t module_count;
    uint32_t section_flags[RXGRAPH_SECTION_COUNT];
    uint64_t section_stored_sizes[RXGRAPH_SECTION_COUNT];
    uint64_t section_expanded_sizes[RXGRAPH_SECTION_COUNT];
} ImageLayout;

typedef struct GraphOperandAudit {
    size_t type_sites;
    size_t member_sites;
    size_t factory_sites;
    size_t providerless_factory_sites;
    size_t invalid_sites;
} GraphOperandAudit;

typedef struct BenchContext {
    const RxGraph *graph;
    RxGraphId exact_type;
    RxGraphId positive_concrete;
    RxGraphId positive_target;
    RxGraphId negative_concrete;
    RxGraphId negative_target;
    RxGraphId dispatch_type;
    RxMemberId dispatch_member;
    RxMemberId dispatch_miss_member;
    RxCallableId dispatch_callable;
    RxFactoryId factory;
    RxGraphId factory_interface;
    RxMemberId factory_member;
    size_t provider_count;
    const char *type_name;
    const char *member_descriptor;
    const char *callable_symbol;
    const RxGraphTypeRef *positive_concrete_ref;
    const RxGraphTypeRef *positive_target_ref;
    const RxGraphTypeRef *negative_concrete_ref;
    const RxGraphTypeRef *negative_target_ref;
    const RxGraphTypeRef *dispatch_type_ref;
    uintptr_t *bound_targets;
    size_t bound_target_count;
    volatile uint64_t *assignability_words;
    size_t assignability_word_count;
    size_t assignability_entry_count;
    volatile uintptr_t *direct_dispatch_targets;
    size_t direct_dispatch_member_count;
    size_t direct_dispatch_entry_count;
} BenchContext;

typedef uint64_t (*BenchOperation)(BenchContext *, uint64_t);

static uint32_t read_u32_le(const unsigned char *data) {
    return (uint32_t)data[0] |
           ((uint32_t)data[1] << 8u) |
           ((uint32_t)data[2] << 16u) |
           ((uint32_t)data[3] << 24u);
}

static uint64_t read_u64_le(const unsigned char *data) {
    uint64_t value;
    unsigned int i;

    value = 0u;
    for (i = 0u; i < 8u; i++) value |= (uint64_t)data[i] << (i * 8u);
    return value;
}

static uint64_t now_ns(void) {
#ifdef _WIN32
    LARGE_INTEGER counter;
    LARGE_INTEGER frequency;
    QueryPerformanceCounter(&counter);
    QueryPerformanceFrequency(&frequency);
    return (uint64_t)(counter.QuadPart / frequency.QuadPart) * UINT64_C(1000000000) +
           (uint64_t)(counter.QuadPart % frequency.QuadPart) * UINT64_C(1000000000) /
               (uint64_t)frequency.QuadPart;
#else
    struct timespec value;
    if (clock_gettime(CLOCK_MONOTONIC, &value) != 0) return 0u;
    return (uint64_t)value.tv_sec * UINT64_C(1000000000) + (uint64_t)value.tv_nsec;
#endif
}

static void free_image_modules(ImageModules *image) {
    size_t i;

    if (!image) return;
    for (i = 0u; i < image->count; i++) free_module(image->items[i]);
    free(image->items);
    memset(image, 0, sizeof(*image));
}

static int append_module(ImageModules *image, module_file *module) {
    module_file **items;
    size_t capacity;

    if (!image || !module) return 0;
    capacity = image->count + 1u;
    if (capacity > SIZE_MAX / sizeof(*items)) return 0;
    items = (module_file **)realloc(image->items, capacity * sizeof(*items));
    if (!items) return 0;
    image->items = items;
    image->items[image->count++] = module;
    return 1;
}

static int audit_graph_operands(const ImageModules *image,
                                GraphOperandAudit *audit) {
    size_t module_index;

    if (!image || !image->graph || !audit) return 0;
    memset(audit, 0, sizeof(*audit));
    for (module_index = 0u; module_index < image->count; module_index++) {
        const module_file *module;
        const bin_code *instructions;
        size_t instruction_index;

        module = image->items[module_index];
        if (!module || !module->instructions || !module->graph_operands) return 0;
        instructions = (const bin_code *)module->instructions;
        instruction_index = 0u;
        while (instruction_index < module->header.instruction_size) {
            OperandType types[3];
            int opcode;
            int operand_count;
            int operand_index;

            opcode = instructions[instruction_index].instruction.opcode;
            if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS) return 0;
            operand_count = rxbin_get_operand_types(rxbin_opcode_format(opcode), types);
            if (instructions[instruction_index].instruction.no_ops != operand_count ||
                instruction_index + (size_t)operand_count >=
                    module->header.instruction_size) return 0;
            for (operand_index = 0; operand_index < operand_count; operand_index++) {
                RxGraphOperandKind kind;
                size_t raw_id;

                kind = rx_graph_operand_kind(opcode, (unsigned int)operand_index);
                if (kind == RX_GRAPH_OPERAND_NONE) continue;
                raw_id = instructions[instruction_index +
                                      (size_t)operand_index + 1u].index;
                if (raw_id > UINT32_MAX) {
                    audit->invalid_sites++;
                    continue;
                }
                if (kind == RX_GRAPH_OPERAND_TYPE) {
                    audit->type_sites++;
                    if (raw_id >= rx_graph_type_count(image->graph)) {
                        audit->invalid_sites++;
                    }
                } else if (kind == RX_GRAPH_OPERAND_MEMBER) {
                    audit->member_sites++;
                    if (raw_id >= rx_graph_member_count(image->graph)) {
                        audit->invalid_sites++;
                    }
                } else {
                    RxGraphFactoryView factory;
                    audit->factory_sites++;
                    if (!rx_graph_factory(image->graph,
                                          (RxFactoryId)raw_id,
                                          &factory)) {
                        audit->invalid_sites++;
                    } else if (rx_graph_provider_bucket_size(image->graph,
                                                              factory.interface_type,
                                                              factory.member) == 0u) {
                        audit->providerless_factory_sites++;
                    }
                }
            }
            instruction_index += (size_t)operand_count + 1u;
        }
    }
    return audit->invalid_sites == 0u;
}

static int load_image(const char *path, ImageModules *image) {
    rxbin_reader reader;
    FILE *input;
    int rc;

    memset(image, 0, sizeof(*image));
    input = fopen(path, "rb");
    if (!input) return 0;
    rxbin_reader_init_file(&reader, input);
    for (;;) {
        module_file *module;
        rc = rxbin_reader_next_module(&reader, &module);
        if (rc == 1) break;
        if (rc != 0 || !module || !append_module(image, module)) {
            free_module(module);
            rxbin_reader_close(&reader);
            fclose(input);
            free_image_modules(image);
            return 0;
        }
        if (!image->graph) image->graph = module->semantic_graph;
        if (module->semantic_graph != image->graph) {
            rxbin_reader_close(&reader);
            fclose(input);
            free_image_modules(image);
            return 0;
        }
    }
    rxbin_reader_close(&reader);
    fclose(input);
    return image->count > 0u && image->graph != 0;
}

static int read_image_layout(const char *path, ImageLayout *layout) {
    unsigned char bytes[RXGRAPH_HEADER_SIZE +
                        RXGRAPH_SECTION_COUNT * RXGRAPH_DIRECTORY_ROW_SIZE];
    FILE *input;
    size_t i;

    memset(layout, 0, sizeof(*layout));
    input = fopen(path, "rb");
    if (!input) return 0;
    if (fread(bytes, 1u, sizeof(bytes), input) != sizeof(bytes)) {
        fclose(input);
        return 0;
    }
    fclose(input);
    if (memcmp(bytes, "cReXx007", 8u) != 0 ||
        read_u32_le(bytes + 8u) != RXGRAPH_HEADER_SIZE ||
        read_u32_le(bytes + 24u) != RXGRAPH_SECTION_COUNT ||
        read_u64_le(bytes + 32u) != RXGRAPH_HEADER_SIZE) return 0;
    layout->file_size = read_u64_le(bytes + 16u);
    layout->module_count = read_u32_le(bytes + 28u);
    for (i = 0u; i < RXGRAPH_SECTION_COUNT; i++) {
        const unsigned char *row;
        row = bytes + RXGRAPH_HEADER_SIZE + i * RXGRAPH_DIRECTORY_ROW_SIZE;
        if (read_u32_le(row) != i + 1u) return 0;
        layout->section_flags[i] = read_u32_le(row + 4u);
        layout->section_stored_sizes[i] = read_u64_le(row + 24u);
        layout->section_expanded_sizes[i] = read_u64_le(row + 32u);
        if ((layout->section_flags[i] != 0u &&
             layout->section_flags[i] != RXBIN007_SECTION_LZSS) ||
            (layout->section_flags[i] == RXBIN007_SECTION_LZSS
                ? layout->section_stored_sizes[i] >= layout->section_expanded_sizes[i]
                : layout->section_stored_sizes[i] != layout->section_expanded_sizes[i])) {
            return 0;
        }
    }
    return 1;
}

static int find_fixtures(BenchContext *context) {
    size_t type_count;
    size_t member_count;
    size_t callable_count;
    size_t type;
    size_t member;

    type_count = rx_graph_type_count(context->graph);
    member_count = rx_graph_member_count(context->graph);
    callable_count = rx_graph_callable_count(context->graph);
    context->exact_type = RX_GRAPH_NONE;
    context->positive_concrete = RX_GRAPH_NONE;
    context->positive_target = RX_GRAPH_NONE;
    context->negative_concrete = RX_GRAPH_NONE;
    context->negative_target = RX_GRAPH_NONE;
    context->dispatch_type = RX_GRAPH_NONE;
    context->dispatch_member = RX_GRAPH_NONE;
    context->dispatch_miss_member = RX_GRAPH_NONE;
    context->dispatch_callable = RX_GRAPH_NONE;
    context->factory = RX_GRAPH_NONE;
    context->factory_interface = RX_GRAPH_NONE;
    context->factory_member = RX_GRAPH_NONE;

    for (type = 0u; type < type_count; type++) {
        if (rx_graph_type_kind(context->graph, (RxGraphId)type) == RX_GRAPH_TYPE_CLASS) {
            context->exact_type = (RxGraphId)type;
            break;
        }
    }
    if (context->exact_type == RX_GRAPH_NONE && type_count) context->exact_type = 0u;

    for (type = 0u; type < type_count &&
                        context->positive_concrete == RX_GRAPH_NONE; type++) {
        RxGraphEdgeView edge;
        if (rx_graph_edge_count(context->graph,
                                (RxGraphId)type,
                                RX_GRAPH_REL_IMPLEMENTS,
                                0) &&
            rx_graph_edge(context->graph,
                          (RxGraphId)type,
                          RX_GRAPH_REL_IMPLEMENTS,
                          0,
                          0u,
                          &edge)) {
            context->positive_concrete = edge.from;
            context->positive_target = edge.to;
        }
    }
    if (context->positive_concrete == RX_GRAPH_NONE) return 0;

    context->negative_concrete = context->positive_concrete;
    for (type = 0u; type < type_count; type++) {
        if ((RxGraphId)type != context->negative_concrete &&
            rx_graph_type_kind(context->graph, (RxGraphId)type) ==
                RX_GRAPH_TYPE_INTERFACE &&
            !rx_graph_type_supports(context->graph,
                                    context->negative_concrete,
                                    (RxGraphId)type)) {
            context->negative_target = (RxGraphId)type;
            break;
        }
    }
    if (context->negative_target == RX_GRAPH_NONE) return 0;

    for (type = 0u; type < type_count && context->dispatch_type == RX_GRAPH_NONE; type++) {
        for (member = 0u; member < member_count; member++) {
            RxCallableId callable;
            callable = rx_graph_dispatch(context->graph,
                                         (RxGraphId)type,
                                         (RxMemberId)member);
            if (callable != RX_GRAPH_NONE) {
                context->dispatch_type = (RxGraphId)type;
                context->dispatch_member = (RxMemberId)member;
                context->dispatch_callable = callable;
                break;
            }
        }
    }
    if (context->dispatch_type == RX_GRAPH_NONE) return 0;
    for (member = 0u; member < member_count; member++) {
        if (rx_graph_dispatch(context->graph,
                              context->dispatch_type,
                              (RxMemberId)member) == RX_GRAPH_NONE) {
            context->dispatch_miss_member = (RxMemberId)member;
            break;
        }
    }
    if (context->dispatch_miss_member == RX_GRAPH_NONE) return 0;

    for (type = 0u; type < type_count && context->factory == RX_GRAPH_NONE; type++) {
        for (member = 0u; member < member_count; member++) {
            RxFactoryId factory;
            factory = rx_graph_find_factory(context->graph,
                                            (RxGraphId)type,
                                            (RxMemberId)member);
            if (factory != RX_GRAPH_NONE) {
                context->factory = factory;
                context->factory_interface = (RxGraphId)type;
                context->factory_member = (RxMemberId)member;
                context->provider_count = rx_graph_provider_bucket_size(
                    context->graph, (RxGraphId)type, (RxMemberId)member);
                break;
            }
        }
    }
    if (context->factory == RX_GRAPH_NONE || !context->provider_count) return 0;

    context->positive_concrete_ref = rx_graph_type_ref(
        context->graph, context->positive_concrete);
    context->positive_target_ref = rx_graph_type_ref(
        context->graph, context->positive_target);
    context->negative_concrete_ref = rx_graph_type_ref(
        context->graph, context->negative_concrete);
    context->negative_target_ref = rx_graph_type_ref(
        context->graph, context->negative_target);
    context->dispatch_type_ref = rx_graph_type_ref(
        context->graph, context->dispatch_type);
    if (!context->positive_concrete_ref || !context->positive_target_ref ||
        !context->negative_concrete_ref || !context->negative_target_ref ||
        !context->dispatch_type_ref) return 0;

    context->type_name = rx_graph_type_name(context->graph, context->exact_type);
    if (member_count) {
        RxGraphMemberView view;
        if (rx_graph_member(context->graph, 0u, &view)) {
            context->member_descriptor = view.descriptor;
        }
    }
    if (callable_count) {
        RxGraphCallableView view;
        if (rx_graph_callable(context->graph, 0u, &view)) {
            context->callable_symbol = view.symbol;
        }
    }
    if (!context->type_name || !context->member_descriptor ||
        !context->callable_symbol) return 0;

    context->bound_target_count = callable_count;
    context->bound_targets = (uintptr_t *)malloc(callable_count *
                                                  sizeof(*context->bound_targets));
    if (!context->bound_targets) return 0;
    for (type = 0u; type < callable_count; type++) {
        context->bound_targets[type] = (uintptr_t)(type + 1u);
    }

    context->assignability_word_count = (type_count + 63u) / 64u;
    if (context->assignability_word_count &&
        type_count > SIZE_MAX / context->assignability_word_count) return 0;
    context->assignability_entry_count =
        type_count * context->assignability_word_count;
    context->assignability_words = (volatile uint64_t *)calloc(
        context->assignability_entry_count, sizeof(*context->assignability_words));
    if (!context->assignability_words) return 0;
    for (type = 0u; type < type_count; type++) {
        size_t target;
        for (target = 0u; target < type_count; target++) {
            if (rx_graph_type_supports(context->graph,
                                       (RxGraphId)type,
                                       (RxGraphId)target)) {
                size_t index;
                index = type * context->assignability_word_count + target / 64u;
                context->assignability_words[index] |=
                    UINT64_C(1) << (target & 63u);
            }
        }
    }

    if (member_count && type_count > SIZE_MAX / member_count) return 0;
    context->direct_dispatch_member_count = member_count;
    context->direct_dispatch_entry_count = type_count * member_count;
    if (context->direct_dispatch_entry_count >
        SIZE_MAX / sizeof(*context->direct_dispatch_targets)) return 0;
    context->direct_dispatch_targets = (volatile uintptr_t *)calloc(
        context->direct_dispatch_entry_count,
        sizeof(*context->direct_dispatch_targets));
    if (!context->direct_dispatch_targets) return 0;
    for (type = 0u; type < type_count; type++) {
        for (member = 0u; member < member_count; member++) {
            RxCallableId callable;
            callable = rx_graph_dispatch(context->graph,
                                         (RxGraphId)type,
                                         (RxMemberId)member);
            if (callable < context->bound_target_count) {
                context->direct_dispatch_targets[type * member_count + member] =
                    context->bound_targets[callable];
            }
        }
    }
    return 1;
}

#define DEFINE_BENCH(name_, expression_) \
    static uint64_t bench_##name_(BenchContext *context, uint64_t iterations) { \
        volatile uint64_t checksum = 0u; \
        uint64_t i; \
        for (i = 0u; i < iterations; i++) { \
            checksum += (uint64_t)(expression_); \
        } \
        return checksum; \
    }

DEFINE_BENCH(control, (context->exact_type + (RxGraphId)(i & 1u)))
DEFINE_BENCH(type_exact,
             rx_graph_type_supports(context->graph,
                                    context->exact_type,
                                    context->exact_type))
DEFINE_BENCH(type_positive,
             rx_graph_type_supports(context->graph,
                                    context->positive_concrete,
                                    context->positive_target))
DEFINE_BENCH(type_negative,
             rx_graph_type_supports(context->graph,
                                    context->negative_concrete,
                                    context->negative_target))
DEFINE_BENCH(type_ref_positive,
             rx_graph_type_ref_supports(context->positive_concrete_ref,
                                        context->positive_target_ref))
DEFINE_BENCH(type_ref_negative,
             rx_graph_type_ref_supports(context->negative_concrete_ref,
                                        context->negative_target_ref))
DEFINE_BENCH(type_find_positive,
             rx_graph_find_type(context->graph, context->type_name))
DEFINE_BENCH(type_find_negative,
             rx_graph_find_type(context->graph, "rxgraph.benchmark.missing.type"))
DEFINE_BENCH(member_find_positive,
             rx_graph_find_member(context->graph, context->member_descriptor))
DEFINE_BENCH(callable_find_positive,
             rx_graph_find_callable(context->graph, context->callable_symbol))
DEFINE_BENCH(dispatch_positive,
             rx_graph_dispatch(context->graph,
                               context->dispatch_type,
                               context->dispatch_member))
DEFINE_BENCH(dispatch_negative,
             rx_graph_dispatch(context->graph,
                               context->dispatch_type,
                               context->dispatch_miss_member))
DEFINE_BENCH(dispatch_type_ref,
             rx_graph_type_ref_dispatch(context->dispatch_type_ref,
                                        context->dispatch_member))
DEFINE_BENCH(factory_bucket,
             rx_graph_provider_bucket_size(context->graph,
                                           context->factory_interface,
                                           context->factory_member))

static uint64_t bench_provider_first(BenchContext *context, uint64_t iterations) {
    volatile uint64_t checksum;
    uint64_t i;

    checksum = 0u;
    for (i = 0u; i < iterations; i++) {
        RxGraphProviderView view;
        if (rx_graph_provider(context->graph,
                              context->factory_interface,
                              context->factory_member,
                              0u,
                              &view)) checksum += view.class_type;
    }
    return checksum;
}

static uint64_t bench_dispatch_portable_proc(BenchContext *context,
                                             uint64_t iterations) {
    volatile uint64_t checksum;
    uint64_t i;

    checksum = 0u;
    for (i = 0u; i < iterations; i++) {
        RxCallableId callable;
        RxGraphCallableView view;
        callable = rx_graph_dispatch(context->graph,
                                     context->dispatch_type,
                                     context->dispatch_member);
        if (rx_graph_callable(context->graph, callable, &view)) {
            checksum += view.procedure.procedure_offset;
        }
    }
    return checksum;
}

static uint64_t bench_poc_bound_dispatch(BenchContext *context,
                                         uint64_t iterations) {
    volatile uint64_t checksum;
    uint64_t i;

    checksum = 0u;
    for (i = 0u; i < iterations; i++) {
        RxCallableId callable;
        callable = rx_graph_dispatch(context->graph,
                                     context->dispatch_type,
                                     context->dispatch_member);
        if (callable < context->bound_target_count) {
            checksum += context->bound_targets[callable];
        }
    }
    return checksum;
}

static uint64_t bench_poc_assignability_bit(BenchContext *context,
                                            uint64_t iterations) {
    volatile uint64_t checksum;
    size_t index;
    uint64_t mask;
    uint64_t i;

    index = (size_t)context->positive_concrete *
                context->assignability_word_count +
            (size_t)context->positive_target / 64u;
    mask = UINT64_C(1) << (context->positive_target & 63u);
    checksum = 0u;
    for (i = 0u; i < iterations; i++) {
        checksum += (context->assignability_words[index] & mask) != 0u;
    }
    return checksum;
}

static uint64_t bench_poc_direct_bound_dispatch(BenchContext *context,
                                                uint64_t iterations) {
    volatile uint64_t checksum;
    size_t index;
    uint64_t i;

    index = (size_t)context->dispatch_type *
                context->direct_dispatch_member_count +
            context->dispatch_member;
    checksum = 0u;
    for (i = 0u; i < iterations; i++) {
        checksum += context->direct_dispatch_targets[index];
    }
    return checksum;
}

static int compare_double(const void *left, const void *right) {
    double a;
    double b;

    a = *(const double *)left;
    b = *(const double *)right;
    return a < b ? -1 : a > b ? 1 : 0;
}

static void run_metric(const char *name,
                       const char *classification,
                       BenchOperation operation,
                       BenchContext *context,
                       uint64_t iterations,
                       unsigned int samples) {
    double *values;
    unsigned int sample;
    uint64_t checksum;

    values = (double *)malloc((size_t)samples * sizeof(*values));
    if (!values) return;
    checksum = operation(context, iterations / 10u + 1u);
    for (sample = 0u; sample < samples; sample++) {
        uint64_t start;
        uint64_t end;
        start = now_ns();
        checksum ^= operation(context, iterations);
        end = now_ns();
        values[sample] = (double)(end - start) / (double)iterations;
    }
    qsort(values, samples, sizeof(*values), compare_double);
    printf("metric,name=%s,classification=%s,iterations=%" PRIu64
           ",samples=%u,median_ns_per_op=%.3f,min_ns_per_op=%.3f,checksum=%" PRIu64 "\n",
           name,
           classification,
           iterations,
           samples,
           values[samples / 2u],
           values[0],
           checksum);
    free(values);
}

static void run_load_metric(const char *path, unsigned int samples) {
    double *values;
    unsigned int sample;
    size_t modules;

    values = (double *)malloc((size_t)samples * sizeof(*values));
    if (!values) return;
    modules = 0u;
    for (sample = 0u; sample <= samples; sample++) {
        ImageModules image;
        uint64_t start;
        uint64_t end;
        start = now_ns();
        if (!load_image(path, &image)) {
            free(values);
            return;
        }
        end = now_ns();
        modules = image.count;
        free_image_modules(&image);
        if (sample) values[sample - 1u] = (double)(end - start);
    }
    qsort(values, samples, sizeof(*values), compare_double);
    printf("metric,name=image_load,classification=production,iterations=1,samples=%u,median_ns_per_op=%.3f,min_ns_per_op=%.3f,modules=%lu\n",
           samples,
           values[samples / 2u],
           values[0],
           (unsigned long)modules);
    free(values);
}

static int parse_u64(const char *text, uint64_t *value) {
    char *end;
    unsigned long long parsed;

    errno = 0;
    parsed = strtoull(text, &end, 10);
    if (errno || !text[0] || *end) return 0;
    *value = (uint64_t)parsed;
    return 1;
}

static void usage(const char *program) {
    fprintf(stderr,
            "Usage: %s [--iterations N] [--samples N] IMAGE.rxbin\n",
            program);
}

int main(int argc, char **argv) {
    static const char *section_names[RXGRAPH_SECTION_COUNT] = {
        "modules", "instructions", "constants", "metadata",
        "graph_facts", "graph_indexes"
    };
    const char *path;
    uint64_t iterations;
    unsigned int samples;
    int argument;
    ImageLayout layout;
    ImageModules image;
    RxGraphStorageStats stats;
    RxGraphProviderView fixture_provider;
    RxGraphCallableView fixture_factory_callable;
    GraphOperandAudit operand_audit;
    BenchContext context;
    size_t section;

    path = 0;
    iterations = UINT64_C(1000000);
    samples = 7u;
    for (argument = 1; argument < argc; argument++) {
        if (strcmp(argv[argument], "--iterations") == 0 && argument + 1 < argc) {
            if (!parse_u64(argv[++argument], &iterations) || !iterations) {
                usage(argv[0]);
                return 2;
            }
        } else if (strcmp(argv[argument], "--samples") == 0 && argument + 1 < argc) {
            uint64_t parsed;
            if (!parse_u64(argv[++argument], &parsed) || !parsed || parsed > 99u) {
                usage(argv[0]);
                return 2;
            }
            samples = (unsigned int)parsed;
        } else if (!path) {
            path = argv[argument];
        } else {
            usage(argv[0]);
            return 2;
        }
    }
    if (!path || !read_image_layout(path, &layout) || !load_image(path, &image) ||
        !rx_graph_storage_stats(image.graph, &stats)) {
        fprintf(stderr, "Unable to inspect RXBIN 007 image: %s\n", path ? path : "<none>");
        return 1;
    }
    memset(&context, 0, sizeof(context));
    context.graph = image.graph;
    if (!find_fixtures(&context)) {
        fprintf(stderr, "Image has insufficient semantic fixtures for the benchmark\n");
        free(context.bound_targets);
        free((void *)context.assignability_words);
        free((void *)context.direct_dispatch_targets);
        free_image_modules(&image);
        return 1;
    }
    if (!audit_graph_operands(&image, &operand_audit)) {
        fprintf(stderr, "Image contains invalid graph-bearing instruction operands\n");
        free(context.bound_targets);
        free((void *)context.assignability_words);
        free((void *)context.direct_dispatch_targets);
        free_image_modules(&image);
        return 1;
    }
    memset(&fixture_provider, 0xff, sizeof(fixture_provider));
    if (!rx_graph_provider(context.graph,
                           context.factory_interface,
                           context.factory_member,
                           0u,
                           &fixture_provider)) {
        fprintf(stderr, "Unable to inspect selected factory provider\n");
        free(context.bound_targets);
        free((void *)context.assignability_words);
        free((void *)context.direct_dispatch_targets);
        free_image_modules(&image);
        return 1;
    }
    memset(&fixture_factory_callable, 0, sizeof(fixture_factory_callable));
    if (!rx_graph_callable(context.graph,
                           fixture_provider.factory_callable,
                           &fixture_factory_callable)) {
        fprintf(stderr, "Unable to inspect selected factory callable\n");
        free(context.bound_targets);
        free((void *)context.assignability_words);
        free((void *)context.direct_dispatch_targets);
        free_image_modules(&image);
        return 1;
    }

    printf("rxgraph_bench,schema=%u,image=%s\n", RXGRAPH_BENCH_SCHEMA, path);
    printf("image,file_bytes=%" PRIu64 ",modules=%u\n",
           layout.file_size,
           layout.module_count);
    for (section = 0u; section < RXGRAPH_SECTION_COUNT; section++) {
        printf("section,name=%s,flags=%u,stored_bytes=%" PRIu64
               ",expanded_bytes=%" PRIu64 "\n",
               section_names[section],
               layout.section_flags[section],
               layout.section_stored_sizes[section],
               layout.section_expanded_sizes[section]);
    }
    printf("graph,types=%lu,members=%lu,parameters=%lu,relationships=%lu,declarations=%lu,callables=%lu,dispatches=%lu,factories=%lu,providers=%lu,string_bytes=%lu,string_count=%lu,unique_string_bytes=%lu,unique_string_count=%lu\n",
           (unsigned long)stats.type_count,
           (unsigned long)stats.member_count,
           (unsigned long)stats.parameter_count,
           (unsigned long)stats.relationship_count,
           (unsigned long)stats.declaration_count,
           (unsigned long)stats.callable_count,
           (unsigned long)stats.dispatch_count,
           (unsigned long)stats.factory_count,
           (unsigned long)stats.provider_count,
           (unsigned long)stats.string_bytes,
           (unsigned long)stats.string_count,
           (unsigned long)stats.unique_string_bytes,
           (unsigned long)stats.unique_string_count);
    printf("storage,retained_bytes=%lu,retained_allocations=%lu,serialized_facts_bytes=%lu,serialized_indexes_bytes=%lu\n",
           (unsigned long)stats.retained_bytes,
           (unsigned long)stats.retained_allocations,
           (unsigned long)stats.serialized_facts_bytes,
           (unsigned long)stats.serialized_indexes_bytes);
    printf("runtime_layout,value_bytes=%lu,type_ref_bytes=%lu\n",
           (unsigned long)sizeof(value),
           (unsigned long)sizeof(RxGraphTypeRef));
    printf("poc_layout,assignability_bytes=%lu,direct_dispatch_bytes=%lu\n",
           (unsigned long)(context.assignability_entry_count *
                           sizeof(*context.assignability_words)),
           (unsigned long)(context.direct_dispatch_entry_count *
                           sizeof(*context.direct_dispatch_targets)));
    printf("fixture,exact_type=%u,positive=%u:%u,negative=%u:%u,dispatch=%u:%u:%u,factory=%u:%u,providers=%lu,factory_callable=%u,match_callable=%u,factory_symbol=%s,factory_proc=%u:%llu\n",
           context.exact_type,
           context.positive_concrete,
           context.positive_target,
           context.negative_concrete,
           context.negative_target,
           context.dispatch_type,
           context.dispatch_member,
           context.dispatch_callable,
           context.factory_interface,
           context.factory_member,
           (unsigned long)context.provider_count,
           fixture_provider.factory_callable,
           fixture_provider.match_callable,
           fixture_factory_callable.symbol,
           fixture_factory_callable.procedure.module_index,
           (unsigned long long)fixture_factory_callable.procedure.procedure_offset);
    printf("instruction_graph_operands,type_sites=%lu,member_sites=%lu,factory_sites=%lu,providerless_factory_sites=%lu,invalid_sites=%lu\n",
           (unsigned long)operand_audit.type_sites,
           (unsigned long)operand_audit.member_sites,
           (unsigned long)operand_audit.factory_sites,
           (unsigned long)operand_audit.providerless_factory_sites,
           (unsigned long)operand_audit.invalid_sites);
    run_load_metric(path, samples);
    run_metric("loop_control", "control", bench_control,
               &context, iterations, samples);
    run_metric("type_support_exact", "production", bench_type_exact,
               &context, iterations, samples);
    run_metric("type_support_positive", "production", bench_type_positive,
               &context, iterations, samples);
    run_metric("type_support_negative", "production", bench_type_negative,
               &context, iterations, samples);
    run_metric("type_ref_support_positive", "production", bench_type_ref_positive,
               &context, iterations, samples);
    run_metric("type_ref_support_negative", "production", bench_type_ref_negative,
               &context, iterations, samples);
    run_metric("type_support_bitset", "poc-not-production",
               bench_poc_assignability_bit, &context, iterations, samples);
    run_metric("type_find_positive", "production", bench_type_find_positive,
               &context, iterations, samples);
    run_metric("type_find_negative", "production", bench_type_find_negative,
               &context, iterations, samples);
    run_metric("member_find_positive", "production", bench_member_find_positive,
               &context, iterations, samples);
    run_metric("callable_find_positive", "production", bench_callable_find_positive,
               &context, iterations, samples);
    run_metric("dispatch_id_positive", "production", bench_dispatch_positive,
               &context, iterations, samples);
    run_metric("dispatch_id_negative", "production", bench_dispatch_negative,
               &context, iterations, samples);
    run_metric("dispatch_type_ref", "production", bench_dispatch_type_ref,
               &context, iterations, samples);
    run_metric("dispatch_portable_proc", "production", bench_dispatch_portable_proc,
               &context, iterations, samples);
    run_metric("dispatch_bound_target", "poc-not-production", bench_poc_bound_dispatch,
               &context, iterations, samples);
    run_metric("dispatch_direct_bound_target", "poc-not-production",
               bench_poc_direct_bound_dispatch, &context, iterations, samples);
    run_metric("factory_bucket", "production", bench_factory_bucket,
               &context, iterations, samples);
    run_metric("provider_first", "production", bench_provider_first,
               &context, iterations, samples);

    free(context.bound_targets);
    free((void *)context.assignability_words);
    free((void *)context.direct_dispatch_targets);
    free_image_modules(&image);
    return 0;
}
