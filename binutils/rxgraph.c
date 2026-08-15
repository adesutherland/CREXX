/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxgraph.h"
#include "rxsha256.h"

#include "rxbin.h"
#include "rxsignature.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct RxGraphTypeRecord {
    uint32_t name_offset;
    uint32_t name_length;
    uint32_t kind;
    uint32_t flags;
    uint64_t hash;
} RxGraphTypeRecord;

typedef struct RxGraphMemberRecord {
    uint32_t name_offset;
    uint32_t descriptor_offset;
    uint32_t return_type;
    uint32_t parameter_first;
    uint32_t parameter_count;
    uint32_t flags;
    uint64_t hash;
} RxGraphMemberRecord;

typedef struct RxGraphParamRecord {
    uint32_t type;
    uint32_t flags;
} RxGraphParamRecord;

typedef struct RxGraphEdgeRecord {
    uint32_t from;
    uint32_t to;
    uint32_t relation;
    uint32_t origin_module;
    uint32_t ordinal;
} RxGraphEdgeRecord;

typedef struct RxGraphDeclarationRecord {
    uint32_t owner;
    uint32_t member;
    uint32_t flags;
    uint32_t origin_module;
    uint32_t ordinal;
} RxGraphDeclarationRecord;

typedef struct RxGraphCallableRecord {
    uint32_t symbol_offset;
    uint32_t descriptor_offset;
    uint32_t owner_type;
    uint32_t member;
    uint32_t module_index;
    uint32_t flags;
    uint64_t procedure_offset;
    uint64_t hash;
} RxGraphCallableRecord;

typedef struct RxGraphDispatchRecord {
    uint32_t owner;
    uint32_t member;
    uint32_t callable;
} RxGraphDispatchRecord;

typedef struct RxGraphProviderRecord {
    uint32_t interface_type;
    uint32_t factory_member;
    uint32_t class_type;
    uint32_t factory_callable;
    uint32_t match_callable;
    uint32_t origin_module;
    uint32_t ordinal;
} RxGraphProviderRecord;

typedef struct RxGraphFactoryRecord {
    uint32_t interface_type;
    uint32_t member;
    uint32_t flags;
    uint32_t reserved;
} RxGraphFactoryRecord;

typedef struct RxGraphHashIndex {
    uint64_t hash;
    uint32_t id;
} RxGraphHashIndex;

typedef struct RxGraphEdgeIndex {
    uint32_t type;
    uint32_t relation;
    uint32_t other;
    uint32_t edge;
} RxGraphEdgeIndex;

typedef struct RxGraphDispatchIndex {
    uint32_t owner;
    uint32_t member;
    uint32_t callable;
} RxGraphDispatchIndex;

typedef struct RxGraphDeclarationIndex {
    uint32_t owner;
    uint32_t member;
    uint32_t declaration;
} RxGraphDeclarationIndex;

typedef struct RxGraphProviderIndex {
    uint32_t interface_type;
    uint32_t factory_member;
    uint32_t provider;
} RxGraphProviderIndex;

typedef struct RxGraphFactoryIndex {
    uint32_t interface_type;
    uint32_t member;
    uint32_t factory;
} RxGraphFactoryIndex;

struct RxGraph {
    size_t refcount;
    char *strings;
    uint32_t string_size;
    uint32_t string_capacity;
    RxGraphTypeRecord *types;
    uint32_t type_count;
    uint32_t type_capacity;
    RxGraphMemberRecord *members;
    uint32_t member_count;
    uint32_t member_capacity;
    RxGraphParamRecord *parameters;
    uint32_t parameter_count;
    uint32_t parameter_capacity;
    RxGraphEdgeRecord *edges;
    uint32_t edge_count;
    uint32_t edge_capacity;
    RxGraphDeclarationRecord *declarations;
    uint32_t declaration_count;
    uint32_t declaration_capacity;
    RxGraphCallableRecord *callables;
    uint32_t callable_count;
    uint32_t callable_capacity;
    RxGraphDispatchRecord *dispatches;
    uint32_t dispatch_count;
    uint32_t dispatch_capacity;
    RxGraphFactoryRecord *factories;
    uint32_t factory_count;
    uint32_t factory_capacity;
    RxGraphProviderRecord *providers;
    uint32_t provider_count;
    uint32_t provider_capacity;
    RxGraphHashIndex *type_index;
    RxGraphHashIndex *member_index;
    RxGraphHashIndex *callable_index;
    RxGraphEdgeIndex *outgoing_index;
    RxGraphEdgeIndex *incoming_index;
    RxGraphDeclarationIndex *declaration_index;
    RxGraphDispatchIndex *dispatch_index;
    RxGraphFactoryIndex *factory_index;
    RxGraphProviderIndex *provider_index;
    uint64_t *assignability_view;
    uint32_t assignability_word_count;
    uint32_t *dispatch_view;
    uint32_t *factory_view;
    uint32_t *provider_first_by_factory;
    uint32_t *provider_count_by_factory;
    RxGraphTypeRef *runtime_types;
};

struct RxGraphBuilder {
    RxGraph *graph;
    int failed;
};

static uint64_t rx_graph_hash(const char *text) {
    const unsigned char *cursor;
    uint64_t hash;

    hash = UINT64_C(1469598103934665603);
    cursor = (const unsigned char *)(text ? text : "");
    while (*cursor) {
        hash ^= (uint64_t)*cursor++;
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static char *rx_graph_strdup(const char *text) {
    size_t length;
    char *copy;

    if (!text) text = "";
    length = strlen(text);
    copy = (char *)malloc(length + 1);
    if (!copy) return 0;
    memcpy(copy, text, length + 1);
    return copy;
}

static char *rx_graph_strdup_range(const char *text, size_t length) {
    char *copy;

    if (!text) return 0;
    copy = (char *)malloc(length + 1u);
    if (!copy) return 0;
    if (length) memcpy(copy, text, length);
    copy[length] = 0;
    return copy;
}

static int rx_graph_grow(void **data,
                         uint32_t *capacity,
                         uint32_t needed,
                         size_t item_size) {
    uint32_t new_capacity;
    void *new_data;

    if (needed <= *capacity) return 1;
    new_capacity = *capacity ? *capacity : 8u;
    while (new_capacity < needed) {
        if (new_capacity > UINT32_MAX / 2u) {
            new_capacity = needed;
            break;
        }
        new_capacity *= 2u;
    }
    if ((size_t)new_capacity > SIZE_MAX / item_size) return 0;
    new_data = realloc(*data, (size_t)new_capacity * item_size);
    if (!new_data) return 0;
    *data = new_data;
    *capacity = new_capacity;
    return 1;
}

static const char *rx_graph_string(const RxGraph *graph, uint32_t offset) {
    if (!graph || offset >= graph->string_size) return 0;
    return graph->strings + offset;
}

static uint32_t rx_graph_add_string(RxGraph *graph, const char *text) {
    uint32_t offset;
    size_t length;
    uint32_t needed;

    if (!graph || !text) return RX_GRAPH_NONE;
    length = strlen(text) + 1u;
    if (length > UINT32_MAX || graph->string_size > UINT32_MAX - (uint32_t)length) {
        return RX_GRAPH_NONE;
    }
    needed = graph->string_size + (uint32_t)length;
    if (!rx_graph_grow((void **)&graph->strings,
                       &graph->string_capacity,
                       needed,
                       sizeof(char))) {
        return RX_GRAPH_NONE;
    }
    offset = graph->string_size;
    memcpy(graph->strings + offset, text, length);
    graph->string_size = needed;
    return offset;
}

char *rx_graph_normalize_type_name(const char *name) {
    size_t length;
    size_t input;
    size_t output;
    size_t start;
    char *normalized;

    if (!name) return 0;
    length = strlen(name);
    if (length == 0) return rx_graph_strdup("");

    if (name[0] == '.' && strchr(name + 1, '.') == 0 && strchr(name + 1, ':') == 0) {
        return rx_graph_strdup(name);
    }

    start = name[0] == '.' ? 1u : 0u;
    normalized = (char *)malloc(length + 1u);
    if (!normalized) return 0;
    output = 0;
    for (input = start; input < length; input++) {
        if (input + 1u < length &&
            ((name[input] == '.' && name[input + 1u] == '.') ||
             (name[input] == ':' && name[input + 1u] == ':'))) {
            normalized[output++] = '.';
            input++;
        } else {
            normalized[output++] = name[input];
        }
    }
    normalized[output] = 0;
    return normalized;
}

char *rx_graph_type_source_name(const char *canonical_name) {
    size_t length;
    size_t dots;
    size_t input;
    size_t output;
    char *source_name;

    if (!canonical_name) return 0;
    if (canonical_name[0] == '.') return rx_graph_strdup(canonical_name);
    length = strlen(canonical_name);
    dots = 0;
    for (input = 0; input < length; input++) {
        if (canonical_name[input] == '.') dots++;
    }
    if (length > SIZE_MAX - dots - 2u) return 0;
    source_name = (char *)malloc(length + dots + 2u);
    if (!source_name) return 0;
    output = 0;
    source_name[output++] = '.';
    for (input = 0; input < length; input++) {
        if (canonical_name[input] == '.') source_name[output++] = '.';
        source_name[output++] = canonical_name[input];
    }
    source_name[output] = 0;
    return source_name;
}

RxGraphOperandKind rx_graph_operand_kind(int opcode, unsigned int operand_index) {
    switch (opcode) {
        case OP_SETOBJTYPE_REG_STRING:
        case OP_SETOBJUNINIT_REG_STRING:
            return operand_index == 1u ? RX_GRAPH_OPERAND_TYPE : RX_GRAPH_OPERAND_NONE;
        case OP_ISTYPE_REG_REG_STRING:
            return operand_index == 2u ? RX_GRAPH_OPERAND_TYPE : RX_GRAPH_OPERAND_NONE;
        case OP_ASSERTTYPE_REG_STRING:
            return operand_index == 1u ? RX_GRAPH_OPERAND_TYPE : RX_GRAPH_OPERAND_NONE;
        case OP_SRCMETHODSEL_REG_REG_STRING:
            return operand_index == 2u ? RX_GRAPH_OPERAND_MEMBER : RX_GRAPH_OPERAND_NONE;
        case OP_SRCFPROCSEL_REG_STRING_REG:
            return operand_index == 1u ? RX_GRAPH_OPERAND_FACTORY : RX_GRAPH_OPERAND_NONE;
        default:
            return RX_GRAPH_OPERAND_NONE;
    }
}

static void rx_graph_destroy(RxGraph *graph) {
    if (!graph) return;
    free(graph->strings);
    free(graph->types);
    free(graph->members);
    free(graph->parameters);
    free(graph->edges);
    free(graph->declarations);
    free(graph->callables);
    free(graph->dispatches);
    free(graph->factories);
    free(graph->providers);
    free(graph->type_index);
    free(graph->member_index);
    free(graph->callable_index);
    free(graph->outgoing_index);
    free(graph->incoming_index);
    free(graph->declaration_index);
    free(graph->dispatch_index);
    free(graph->factory_index);
    free(graph->provider_index);
    free(graph->assignability_view);
    free(graph->dispatch_view);
    free(graph->factory_view);
    free(graph->provider_first_by_factory);
    free(graph->provider_count_by_factory);
    free(graph->runtime_types);
    free(graph);
}

RxGraphBuilder *rx_graph_builder_create(void) {
    static const char *builtins[] = {
        ".void", ".object", ".boolean", ".int", ".float", ".decimal", ".string",
        ".binary", ".unknown"
    };
    RxGraphBuilder *builder;
    size_t i;

    builder = (RxGraphBuilder *)calloc(1, sizeof(*builder));
    if (!builder) return 0;
    builder->graph = (RxGraph *)calloc(1, sizeof(*builder->graph));
    if (!builder->graph) {
        free(builder);
        return 0;
    }
    builder->graph->refcount = 1u;
    for (i = 0; i < sizeof(builtins) / sizeof(builtins[0]); i++) {
        if (rx_graph_builder_add_type(builder,
                                      builtins[i],
                                      RX_GRAPH_TYPE_BUILTIN,
                                      0u) == RX_GRAPH_NONE) {
            rx_graph_builder_free(builder);
            return 0;
        }
    }
    return builder;
}

void rx_graph_builder_free(RxGraphBuilder *builder) {
    if (!builder) return;
    rx_graph_destroy(builder->graph);
    free(builder);
}

RxGraphId rx_graph_builder_find_type(const RxGraphBuilder *builder,
                                     const char *name) {
    char *normalized;
    uint32_t i;

    if (!builder || !builder->graph || !name) return RX_GRAPH_NONE;
    normalized = rx_graph_normalize_type_name(name);
    if (!normalized) return RX_GRAPH_NONE;
    for (i = 0; i < builder->graph->type_count; i++) {
        const char *candidate;
        candidate = rx_graph_string(builder->graph,
                                    builder->graph->types[i].name_offset);
        if (candidate && strcmp(candidate, normalized) == 0) {
            free(normalized);
            return i;
        }
    }
    free(normalized);
    return RX_GRAPH_NONE;
}

RxGraphId rx_graph_builder_add_type(RxGraphBuilder *builder,
                                    const char *name,
                                    RxGraphTypeKind kind,
                                    uint32_t flags) {
    RxGraph *graph;
    RxGraphId existing;
    RxGraphTypeRecord *record;
    char *normalized;
    uint32_t offset;

    if (!builder || builder->failed || !name) return RX_GRAPH_NONE;
    graph = builder->graph;
    existing = rx_graph_builder_find_type(builder, name);
    if (existing != RX_GRAPH_NONE) {
        record = &graph->types[existing];
        if (record->kind == RX_GRAPH_TYPE_OPAQUE && kind != RX_GRAPH_TYPE_OPAQUE) {
            record->kind = (uint32_t)kind;
        } else if (kind != RX_GRAPH_TYPE_OPAQUE &&
                   record->kind != RX_GRAPH_TYPE_OPAQUE &&
                   record->kind != (uint32_t)kind) {
            builder->failed = 1;
            return RX_GRAPH_NONE;
        }
        record->flags |= flags;
        return existing;
    }

    normalized = rx_graph_normalize_type_name(name);
    if (!normalized) {
        builder->failed = 1;
        return RX_GRAPH_NONE;
    }
    if (!rx_graph_grow((void **)&graph->types,
                       &graph->type_capacity,
                       graph->type_count + 1u,
                       sizeof(*graph->types))) {
        free(normalized);
        builder->failed = 1;
        return RX_GRAPH_NONE;
    }
    offset = rx_graph_add_string(graph, normalized);
    if (offset == RX_GRAPH_NONE) {
        free(normalized);
        builder->failed = 1;
        return RX_GRAPH_NONE;
    }
    record = &graph->types[graph->type_count];
    record->name_offset = offset;
    record->name_length = (uint32_t)strlen(normalized);
    record->kind = (uint32_t)kind;
    record->flags = flags;
    record->hash = rx_graph_hash(normalized);
    free(normalized);
    return graph->type_count++;
}

static uint32_t rx_graph_param_flags(const rx_callable_arg *arg) {
    uint32_t flags;

    flags = 0u;
    if (arg->is_ref) flags |= RX_GRAPH_PARAM_REF;
    if (arg->is_optional) flags |= RX_GRAPH_PARAM_OPTIONAL;
    if (arg->is_vararg) flags |= RX_GRAPH_PARAM_VARARG;
    return flags;
}

static int rx_graph_descriptors_match(const char *left_descriptor,
                                      const char *right_descriptor) {
    rx_callable_signature left;
    rx_callable_signature right;
    int matches;

    if (!left_descriptor || !right_descriptor) return 0;
    if (strcmp(left_descriptor, right_descriptor) == 0) return 1;
    if (!rx_sig_parse_descriptor(left_descriptor, &left)) return 0;
    if (!rx_sig_parse_descriptor(right_descriptor, &right)) {
        rx_sig_free(&left);
        return 0;
    }
    matches = rx_sig_matches_contract(&left, &right, 0);
    rx_sig_free(&left);
    rx_sig_free(&right);
    return matches;
}

RxMemberId rx_graph_builder_find_member(const RxGraphBuilder *builder,
                                        const char *descriptor) {
    uint32_t i;

    if (!builder || !builder->graph || !descriptor) return RX_GRAPH_NONE;
    for (i = 0; i < builder->graph->member_count; i++) {
        const char *candidate;
        candidate = rx_graph_string(builder->graph,
                                    builder->graph->members[i].descriptor_offset);
        if (candidate && rx_graph_descriptors_match(candidate, descriptor)) return i;
    }
    return RX_GRAPH_NONE;
}

RxMemberId rx_graph_builder_add_member(RxGraphBuilder *builder,
                                       const char *name,
                                       const char *return_type,
                                       const char *args,
                                       uint32_t flags) {
    RxGraph *graph;
    rx_callable_signature signature;
    RxMemberId existing;
    RxGraphMemberRecord *record;
    char *descriptor;
    uint32_t name_offset;
    uint32_t descriptor_offset;
    uint32_t first_parameter;
    RxGraphId return_id;
    size_t i;

    if (!builder || builder->failed || !name) return RX_GRAPH_NONE;
    graph = builder->graph;
    descriptor = rx_sig_build_descriptor(name, return_type, args);
    if (!descriptor) {
        builder->failed = 1;
        return RX_GRAPH_NONE;
    }
    existing = rx_graph_builder_find_member(builder, descriptor);
    if (existing != RX_GRAPH_NONE) {
        graph->members[existing].flags |= flags;
        free(descriptor);
        return existing;
    }
    if (!rx_sig_init_from_parts(&signature, name, return_type, args)) {
        free(descriptor);
        builder->failed = 1;
        return RX_GRAPH_NONE;
    }
    return_id = rx_graph_builder_add_type(builder,
                                          signature.return_type,
                                          RX_GRAPH_TYPE_OPAQUE,
                                          0u);
    if (return_id == RX_GRAPH_NONE ||
        signature.arg_count > UINT32_MAX - graph->parameter_count ||
        !rx_graph_grow((void **)&graph->parameters,
                       &graph->parameter_capacity,
                       graph->parameter_count + (uint32_t)signature.arg_count,
                       sizeof(*graph->parameters)) ||
        !rx_graph_grow((void **)&graph->members,
                       &graph->member_capacity,
                       graph->member_count + 1u,
                       sizeof(*graph->members))) {
        rx_sig_free(&signature);
        free(descriptor);
        builder->failed = 1;
        return RX_GRAPH_NONE;
    }
    first_parameter = graph->parameter_count;
    for (i = 0; i < signature.arg_count; i++) {
        RxGraphId type_id;
        type_id = rx_graph_builder_add_type(builder,
                                            signature.args[i].type,
                                            RX_GRAPH_TYPE_OPAQUE,
                                            0u);
        if (type_id == RX_GRAPH_NONE) {
            rx_sig_free(&signature);
            free(descriptor);
            builder->failed = 1;
            return RX_GRAPH_NONE;
        }
        graph->parameters[graph->parameter_count].type = type_id;
        graph->parameters[graph->parameter_count].flags =
            rx_graph_param_flags(&signature.args[i]);
        graph->parameter_count++;
    }
    name_offset = rx_graph_add_string(graph, signature.name);
    descriptor_offset = rx_graph_add_string(graph, descriptor);
    if (name_offset == RX_GRAPH_NONE || descriptor_offset == RX_GRAPH_NONE) {
        rx_sig_free(&signature);
        free(descriptor);
        builder->failed = 1;
        return RX_GRAPH_NONE;
    }
    record = &graph->members[graph->member_count];
    record->name_offset = name_offset;
    record->descriptor_offset = descriptor_offset;
    record->return_type = return_id;
    record->parameter_first = first_parameter;
    record->parameter_count = (uint32_t)signature.arg_count;
    record->flags = flags;
    record->hash = rx_graph_hash(descriptor);
    rx_sig_free(&signature);
    free(descriptor);
    return graph->member_count++;
}

RxCallableId rx_graph_builder_find_callable(const RxGraphBuilder *builder,
                                            const char *symbol) {
    uint32_t i;

    if (!builder || !builder->graph || !symbol) return RX_GRAPH_NONE;
    for (i = 0; i < builder->graph->callable_count; i++) {
        const char *candidate;
        candidate = rx_graph_string(builder->graph,
                                    builder->graph->callables[i].symbol_offset);
        if (candidate && strcmp(candidate, symbol) == 0) return i;
    }
    return RX_GRAPH_NONE;
}

RxCallableId rx_graph_builder_add_callable(RxGraphBuilder *builder,
                                           const char *symbol,
                                           const char *return_type,
                                           const char *args,
                                           RxGraphProcRef procedure,
                                           uint32_t flags) {
    RxGraph *graph;
    rx_callable_signature signature;
    RxCallableId existing;
    RxGraphCallableRecord *record;
    char *descriptor;
    uint32_t symbol_offset;
    uint32_t descriptor_offset;
    size_t i;

    if (!builder || builder->failed || !symbol) return RX_GRAPH_NONE;
    graph = builder->graph;
    descriptor = rx_sig_build_descriptor(symbol, return_type, args);
    if (!descriptor) {
        builder->failed = 1;
        return RX_GRAPH_NONE;
    }
    existing = rx_graph_builder_find_callable(builder, symbol);
    if (existing != RX_GRAPH_NONE) {
        RxGraphCallableRecord *current;
        const char *current_descriptor;
        uint32_t current_semantic_flags;
        uint32_t incoming_semantic_flags;
        int current_imported;
        int incoming_imported;

        current = &graph->callables[existing];
        current_descriptor = rx_graph_string(graph, current->descriptor_offset);
        current_semantic_flags = current->flags & ~RX_GRAPH_CALLABLE_IMPORTED;
        incoming_semantic_flags = flags & ~RX_GRAPH_CALLABLE_IMPORTED;
        current_imported = (current->flags & RX_GRAPH_CALLABLE_IMPORTED) != 0u;
        incoming_imported = (flags & RX_GRAPH_CALLABLE_IMPORTED) != 0u;
        if (!current_descriptor ||
            !rx_graph_descriptors_match(current_descriptor, descriptor) ||
            current_semantic_flags != incoming_semantic_flags ||
            (!current_imported && !incoming_imported &&
             (current->module_index != procedure.module_index ||
              current->procedure_offset != procedure.procedure_offset))) {
            builder->failed = 1;
            free(descriptor);
            return RX_GRAPH_NONE;
        }
        if (current_imported && !incoming_imported) {
            descriptor_offset = rx_graph_add_string(graph, descriptor);
            if (descriptor_offset == RX_GRAPH_NONE) {
                builder->failed = 1;
                free(descriptor);
                return RX_GRAPH_NONE;
            }
            current->descriptor_offset = descriptor_offset;
            current->module_index = procedure.module_index;
            current->procedure_offset = procedure.procedure_offset;
            current->flags = flags;
        }
        free(descriptor);
        return existing;
    }
    if (!rx_sig_init_from_parts(&signature, symbol, return_type, args)) {
        free(descriptor);
        builder->failed = 1;
        return RX_GRAPH_NONE;
    }
    if (rx_graph_builder_add_type(builder,
                                  signature.return_type,
                                  RX_GRAPH_TYPE_OPAQUE,
                                  0u) == RX_GRAPH_NONE) {
        rx_sig_free(&signature);
        free(descriptor);
        builder->failed = 1;
        return RX_GRAPH_NONE;
    }
    for (i = 0u; i < signature.arg_count; i++) {
        if (rx_graph_builder_add_type(builder,
                                      signature.args[i].type,
                                      RX_GRAPH_TYPE_OPAQUE,
                                      0u) == RX_GRAPH_NONE) {
            rx_sig_free(&signature);
            free(descriptor);
            builder->failed = 1;
            return RX_GRAPH_NONE;
        }
    }
    if (!rx_graph_grow((void **)&graph->callables,
                       &graph->callable_capacity,
                       graph->callable_count + 1u,
                       sizeof(*graph->callables))) {
        rx_sig_free(&signature);
        free(descriptor);
        builder->failed = 1;
        return RX_GRAPH_NONE;
    }
    symbol_offset = rx_graph_add_string(graph, symbol);
    descriptor_offset = rx_graph_add_string(graph, descriptor);
    if (symbol_offset == RX_GRAPH_NONE || descriptor_offset == RX_GRAPH_NONE) {
        rx_sig_free(&signature);
        free(descriptor);
        builder->failed = 1;
        return RX_GRAPH_NONE;
    }
    record = &graph->callables[graph->callable_count];
    record->symbol_offset = symbol_offset;
    record->descriptor_offset = descriptor_offset;
    record->owner_type = RX_GRAPH_NONE;
    record->member = RX_GRAPH_NONE;
    record->module_index = procedure.module_index;
    record->flags = flags;
    record->procedure_offset = procedure.procedure_offset;
    record->hash = rx_graph_hash(symbol);
    rx_sig_free(&signature);
    free(descriptor);
    return graph->callable_count++;
}

int rx_graph_builder_add_edge(RxGraphBuilder *builder,
                              RxGraphId from,
                              RxGraphId to,
                              RxGraphRelation relation,
                              uint32_t origin_module,
                              uint32_t ordinal) {
    RxGraph *graph;
    uint32_t i;

    if (!builder || builder->failed) return 0;
    graph = builder->graph;
    if (from >= graph->type_count || to >= graph->type_count) return 0;
    for (i = 0; i < graph->edge_count; i++) {
        if (graph->edges[i].from == from &&
            graph->edges[i].to == to &&
            graph->edges[i].relation == (uint32_t)relation) {
            return 1;
        }
    }
    if (!rx_graph_grow((void **)&graph->edges,
                       &graph->edge_capacity,
                       graph->edge_count + 1u,
                       sizeof(*graph->edges))) {
        builder->failed = 1;
        return 0;
    }
    graph->edges[graph->edge_count].from = from;
    graph->edges[graph->edge_count].to = to;
    graph->edges[graph->edge_count].relation = (uint32_t)relation;
    graph->edges[graph->edge_count].origin_module = origin_module;
    graph->edges[graph->edge_count].ordinal = ordinal;
    graph->edge_count++;
    return 1;
}

static int rx_graph_builder_ensure_factory(RxGraphBuilder *builder,
                                           RxGraphId owner,
                                           RxMemberId member,
                                           uint32_t flags) {
    RxGraph *graph;
    uint32_t i;

    if (!builder || builder->failed) return 0;
    graph = builder->graph;
    if (owner >= graph->type_count || member >= graph->member_count) return 0;
    for (i = 0u; i < graph->factory_count; i++) {
        if (graph->factories[i].interface_type == owner &&
            graph->factories[i].member == member) return 1;
    }
    if (!rx_graph_grow((void **)&graph->factories,
                       &graph->factory_capacity,
                       graph->factory_count + 1u,
                       sizeof(*graph->factories))) {
        builder->failed = 1;
        return 0;
    }
    graph->factories[graph->factory_count].interface_type = owner;
    graph->factories[graph->factory_count].member = member;
    graph->factories[graph->factory_count].flags = flags;
    graph->factories[graph->factory_count].reserved = 0u;
    graph->factory_count++;
    return 1;
}

int rx_graph_builder_add_declaration(RxGraphBuilder *builder,
                                     RxGraphId owner,
                                     RxMemberId member,
                                     uint32_t flags,
                                     uint32_t origin_module,
                                     uint32_t ordinal) {
    RxGraph *graph;
    RxGraphDeclarationRecord *record;
    uint32_t i;

    if (!builder || builder->failed) return 0;
    graph = builder->graph;
    if (owner >= graph->type_count || member >= graph->member_count) return 0;
    for (i = 0; i < graph->declaration_count; i++) {
        if (graph->declarations[i].owner == owner &&
            graph->declarations[i].member == member &&
            graph->declarations[i].flags == flags) {
            return 1;
        }
    }
    if (!rx_graph_grow((void **)&graph->declarations,
                       &graph->declaration_capacity,
                       graph->declaration_count + 1u,
                       sizeof(*graph->declarations))) {
        builder->failed = 1;
        return 0;
    }
    record = &graph->declarations[graph->declaration_count++];
    record->owner = owner;
    record->member = member;
    record->flags = flags;
    record->origin_module = origin_module;
    record->ordinal = ordinal;
    graph->members[member].flags |= flags;
    if ((flags & RX_GRAPH_MEMBER_FACTORY) &&
        !rx_graph_builder_ensure_factory(builder, owner, member, flags)) return 0;
    return 1;
}

int rx_graph_builder_add_dispatch(RxGraphBuilder *builder,
                                  RxGraphId owner,
                                  RxMemberId member,
                                  RxCallableId callable) {
    RxGraph *graph;
    RxGraphDispatchRecord *record;
    uint32_t i;

    if (!builder || builder->failed) return 0;
    graph = builder->graph;
    if (owner >= graph->type_count || member >= graph->member_count ||
        callable >= graph->callable_count) return 0;
    for (i = 0; i < graph->dispatch_count; i++) {
        if (graph->dispatches[i].owner == owner &&
            graph->dispatches[i].member == member) {
            return graph->dispatches[i].callable == callable;
        }
    }
    if (!rx_graph_grow((void **)&graph->dispatches,
                       &graph->dispatch_capacity,
                       graph->dispatch_count + 1u,
                       sizeof(*graph->dispatches))) {
        builder->failed = 1;
        return 0;
    }
    record = &graph->dispatches[graph->dispatch_count++];
    record->owner = owner;
    record->member = member;
    record->callable = callable;
    graph->callables[callable].owner_type = owner;
    graph->callables[callable].member = member;
    return 1;
}

RxProviderId rx_graph_builder_add_provider(RxGraphBuilder *builder,
                                           RxGraphId interface_type,
                                           RxMemberId factory_member,
                                           RxGraphId class_type,
                                           RxCallableId factory_callable,
                                           RxCallableId match_callable,
                                           uint32_t origin_module,
                                           uint32_t ordinal) {
    RxGraph *graph;
    RxGraphProviderRecord *record;
    uint32_t i;

    if (!builder || builder->failed) return RX_GRAPH_NONE;
    graph = builder->graph;
    if (interface_type >= graph->type_count || class_type >= graph->type_count ||
        factory_member >= graph->member_count ||
        factory_callable >= graph->callable_count ||
        (match_callable != RX_GRAPH_NONE && match_callable >= graph->callable_count)) {
        return RX_GRAPH_NONE;
    }
    for (i = 0; i < graph->provider_count; i++) {
        if (graph->providers[i].interface_type == interface_type &&
            graph->providers[i].factory_member == factory_member &&
            graph->providers[i].class_type == class_type) {
            return i;
        }
    }
    if (!rx_graph_grow((void **)&graph->providers,
                       &graph->provider_capacity,
                       graph->provider_count + 1u,
                       sizeof(*graph->providers))) {
        builder->failed = 1;
        return RX_GRAPH_NONE;
    }
    record = &graph->providers[graph->provider_count];
    record->interface_type = interface_type;
    record->factory_member = factory_member;
    record->class_type = class_type;
    record->factory_callable = factory_callable;
    record->match_callable = match_callable;
    record->origin_module = origin_module;
    record->ordinal = ordinal;
    return graph->provider_count++;
}

static int rx_graph_hash_index_compare(const void *left, const void *right) {
    const RxGraphHashIndex *a;
    const RxGraphHashIndex *b;

    a = (const RxGraphHashIndex *)left;
    b = (const RxGraphHashIndex *)right;
    if (a->hash < b->hash) return -1;
    if (a->hash > b->hash) return 1;
    if (a->id < b->id) return -1;
    if (a->id > b->id) return 1;
    return 0;
}

static int rx_graph_edge_index_compare(const void *left, const void *right) {
    const RxGraphEdgeIndex *a;
    const RxGraphEdgeIndex *b;

    a = (const RxGraphEdgeIndex *)left;
    b = (const RxGraphEdgeIndex *)right;
    if (a->type < b->type) return -1;
    if (a->type > b->type) return 1;
    if (a->relation < b->relation) return -1;
    if (a->relation > b->relation) return 1;
    if (a->other < b->other) return -1;
    if (a->other > b->other) return 1;
    if (a->edge < b->edge) return -1;
    if (a->edge > b->edge) return 1;
    return 0;
}

static int rx_graph_dispatch_index_compare(const void *left, const void *right) {
    const RxGraphDispatchIndex *a;
    const RxGraphDispatchIndex *b;

    a = (const RxGraphDispatchIndex *)left;
    b = (const RxGraphDispatchIndex *)right;
    if (a->owner < b->owner) return -1;
    if (a->owner > b->owner) return 1;
    if (a->member < b->member) return -1;
    if (a->member > b->member) return 1;
    if (a->callable < b->callable) return -1;
    if (a->callable > b->callable) return 1;
    return 0;
}

static int rx_graph_declaration_index_compare(const void *left,
                                              const void *right) {
    const RxGraphDeclarationIndex *a;
    const RxGraphDeclarationIndex *b;

    a = (const RxGraphDeclarationIndex *)left;
    b = (const RxGraphDeclarationIndex *)right;
    if (a->owner < b->owner) return -1;
    if (a->owner > b->owner) return 1;
    if (a->member < b->member) return -1;
    if (a->member > b->member) return 1;
    if (a->declaration < b->declaration) return -1;
    if (a->declaration > b->declaration) return 1;
    return 0;
}

static int rx_graph_factory_index_compare(const void *left, const void *right) {
    const RxGraphFactoryIndex *a;
    const RxGraphFactoryIndex *b;

    a = (const RxGraphFactoryIndex *)left;
    b = (const RxGraphFactoryIndex *)right;
    if (a->interface_type < b->interface_type) return -1;
    if (a->interface_type > b->interface_type) return 1;
    if (a->member < b->member) return -1;
    if (a->member > b->member) return 1;
    if (a->factory < b->factory) return -1;
    if (a->factory > b->factory) return 1;
    return 0;
}

static int rx_graph_provider_index_less(const RxGraph *graph,
                                        const RxGraphProviderIndex *left,
                                        const RxGraphProviderIndex *right) {
    const RxGraphProviderRecord *a;
    const RxGraphProviderRecord *b;
    const char *a_name;
    const char *b_name;
    int comparison;

    if (left->interface_type != right->interface_type) {
        return left->interface_type < right->interface_type;
    }
    if (left->factory_member != right->factory_member) {
        return left->factory_member < right->factory_member;
    }
    a = &graph->providers[left->provider];
    b = &graph->providers[right->provider];
    a_name = rx_graph_string(graph, graph->types[a->class_type].name_offset);
    b_name = rx_graph_string(graph, graph->types[b->class_type].name_offset);
    comparison = strcmp(a_name ? a_name : "", b_name ? b_name : "");
    if (comparison != 0) return comparison < 0;
    return left->provider < right->provider;
}

static void rx_graph_sort_provider_index(RxGraph *graph) {
    uint32_t i;

    for (i = 1; i < graph->provider_count; i++) {
        RxGraphProviderIndex value;
        uint32_t position;

        value = graph->provider_index[i];
        position = i;
        while (position > 0u &&
               rx_graph_provider_index_less(graph,
                                            &value,
                                            &graph->provider_index[position - 1u])) {
            graph->provider_index[position] = graph->provider_index[position - 1u];
            position--;
        }
        graph->provider_index[position] = value;
    }
}

static void rx_graph_free_runtime_views(RxGraph *graph) {
    if (!graph) return;
    free(graph->assignability_view);
    free(graph->dispatch_view);
    free(graph->factory_view);
    free(graph->provider_first_by_factory);
    free(graph->provider_count_by_factory);
    free(graph->runtime_types);
    graph->assignability_view = 0;
    graph->assignability_word_count = 0u;
    graph->dispatch_view = 0;
    graph->factory_view = 0;
    graph->provider_first_by_factory = 0;
    graph->provider_count_by_factory = 0;
    graph->runtime_types = 0;
}

static int rx_graph_size_multiply(size_t left,
                                  size_t right,
                                  size_t *result) {
    if (!result || (left && right > SIZE_MAX / left)) return 0;
    *result = left * right;
    return 1;
}

/* C3 process-local view: the serialized graph remains portable while rxbin
   owns the precomputed representation reached through the existing API. */
static int rx_graph_build_runtime_views(RxGraph *graph) {
    size_t type_words;
    size_t dense_entries;
    size_t allocation_bytes;
    uint32_t i;

    if (!graph) return 0;
    rx_graph_free_runtime_views(graph);

    if (graph->type_count) {
        graph->assignability_word_count =
            graph->type_count / 64u + (graph->type_count % 64u != 0u);
        if (!rx_graph_size_multiply((size_t)graph->type_count,
                                    graph->assignability_word_count,
                                    &type_words) ||
            !rx_graph_size_multiply(type_words,
                                    sizeof(*graph->assignability_view),
                                    &allocation_bytes)) goto error;
        graph->assignability_view = (uint64_t *)calloc(1u, allocation_bytes);
        graph->runtime_types = (RxGraphTypeRef *)calloc(
            graph->type_count, sizeof(*graph->runtime_types));
        if (!graph->assignability_view || !graph->runtime_types) goto error;
        for (i = 0u; i < graph->type_count; i++) {
            size_t row;
            row = (size_t)i * graph->assignability_word_count;
            graph->runtime_types[i].name =
                rx_graph_string(graph, graph->types[i].name_offset);
            graph->runtime_types[i].name_length = graph->types[i].name_length;
            graph->runtime_types[i].graph = graph;
            graph->runtime_types[i].id = i;
            graph->runtime_types[i].assignability_words =
                graph->assignability_view + row;
            graph->runtime_types[i].assignability_word_count =
                graph->assignability_word_count;
            if (!graph->runtime_types[i].name) goto error;
            graph->assignability_view[row + i / 64u] |=
                UINT64_C(1) << (i & 63u);
        }
        for (i = 0u; i < graph->edge_count; i++) {
            const RxGraphEdgeRecord *edge;
            size_t row;
            edge = &graph->edges[i];
            if (edge->from >= graph->type_count || edge->to >= graph->type_count ||
                edge->relation < RX_GRAPH_REL_IMPLEMENTS ||
                edge->relation > RX_GRAPH_REL_TYPE_ALIAS) goto error;
            row = (size_t)edge->from * graph->assignability_word_count;
            graph->assignability_view[row + edge->to / 64u] |=
                UINT64_C(1) << (edge->to & 63u);
        }
        for (i = 0u; i < graph->type_count; i++) {
            size_t source;
            size_t closure_row;
            closure_row = (size_t)i * graph->assignability_word_count;
            for (source = 0u; source < graph->type_count; source++) {
                uint64_t *row;
                uint32_t word;
                row = graph->assignability_view +
                      source * graph->assignability_word_count;
                if (!(row[i / 64u] & (UINT64_C(1) << (i & 63u)))) continue;
                for (word = 0u; word < graph->assignability_word_count; word++) {
                    row[word] |= graph->assignability_view[closure_row + word];
                }
            }
        }
    }

    if (!rx_graph_size_multiply((size_t)graph->type_count,
                                graph->member_count,
                                &dense_entries) ||
        !rx_graph_size_multiply(dense_entries,
                                sizeof(*graph->dispatch_view),
                                &allocation_bytes)) goto error;
    if (dense_entries) {
        graph->dispatch_view = (uint32_t *)malloc(allocation_bytes);
        graph->factory_view = (uint32_t *)malloc(allocation_bytes);
        if (!graph->dispatch_view || !graph->factory_view) goto error;
        memset(graph->dispatch_view, 0xff, allocation_bytes);
        memset(graph->factory_view, 0xff, allocation_bytes);
    }
    for (i = 0u; i < graph->type_count; i++) {
        graph->runtime_types[i].dispatch_row = graph->dispatch_view
            ? graph->dispatch_view + (size_t)i * graph->member_count
            : 0;
        graph->runtime_types[i].dispatch_member_count = graph->member_count;
    }
    for (i = 0u; i < graph->dispatch_count; i++) {
        const RxGraphDispatchIndex *entry;
        size_t slot;
        entry = &graph->dispatch_index[i];
        if (entry->owner >= graph->type_count ||
            entry->member >= graph->member_count ||
            entry->callable >= graph->callable_count) goto error;
        slot = (size_t)entry->owner * graph->member_count + entry->member;
        if (graph->dispatch_view[slot] == RX_GRAPH_NONE) {
            graph->dispatch_view[slot] = entry->callable;
        }
    }
    for (i = 0u; i < graph->factory_count; i++) {
        const RxGraphFactoryIndex *entry;
        size_t slot;
        entry = &graph->factory_index[i];
        if (entry->interface_type >= graph->type_count ||
            entry->member >= graph->member_count ||
            entry->factory >= graph->factory_count) goto error;
        slot = (size_t)entry->interface_type * graph->member_count + entry->member;
        if (graph->factory_view[slot] == RX_GRAPH_NONE) {
            graph->factory_view[slot] = entry->factory;
        }
    }

    if (graph->factory_count) {
        graph->provider_first_by_factory = (uint32_t *)malloc(
            (size_t)graph->factory_count *
            sizeof(*graph->provider_first_by_factory));
        graph->provider_count_by_factory = (uint32_t *)calloc(
            graph->factory_count,
            sizeof(*graph->provider_count_by_factory));
        if (!graph->provider_first_by_factory ||
            !graph->provider_count_by_factory) goto error;
        memset(graph->provider_first_by_factory,
               0xff,
               (size_t)graph->factory_count *
                   sizeof(*graph->provider_first_by_factory));
    }
    for (i = 0u; i < graph->provider_count; i++) {
        const RxGraphProviderIndex *entry;
        uint32_t factory;
        size_t slot;
        entry = &graph->provider_index[i];
        if (entry->interface_type >= graph->type_count ||
            entry->factory_member >= graph->member_count ||
            entry->provider >= graph->provider_count) goto error;
        slot = (size_t)entry->interface_type * graph->member_count +
               entry->factory_member;
        factory = graph->factory_view[slot];
        if (factory == RX_GRAPH_NONE || factory >= graph->factory_count) goto error;
        if (!graph->provider_count_by_factory[factory]) {
            graph->provider_first_by_factory[factory] = i;
        }
        graph->provider_count_by_factory[factory]++;
    }
    return 1;

error:
    rx_graph_free_runtime_views(graph);
    return 0;
}

static int rx_graph_build_indices(RxGraph *graph) {
    uint32_t i;

    free(graph->type_index);
    free(graph->member_index);
    free(graph->callable_index);
    free(graph->outgoing_index);
    free(graph->incoming_index);
    free(graph->declaration_index);
    free(graph->dispatch_index);
    free(graph->factory_index);
    free(graph->provider_index);
    graph->type_index = 0;
    graph->member_index = 0;
    graph->callable_index = 0;
    graph->outgoing_index = 0;
    graph->incoming_index = 0;
    graph->declaration_index = 0;
    graph->dispatch_index = 0;
    graph->factory_index = 0;
    graph->provider_index = 0;

    if (graph->type_count) {
        graph->type_index = (RxGraphHashIndex *)malloc(
            (size_t)graph->type_count * sizeof(*graph->type_index));
        if (!graph->type_index) return 0;
        for (i = 0; i < graph->type_count; i++) {
            graph->type_index[i].hash = graph->types[i].hash;
            graph->type_index[i].id = i;
        }
        qsort(graph->type_index,
              graph->type_count,
              sizeof(*graph->type_index),
              rx_graph_hash_index_compare);
    }

    if (graph->member_count) {
        graph->member_index = (RxGraphHashIndex *)malloc(
            (size_t)graph->member_count * sizeof(*graph->member_index));
        if (!graph->member_index) return 0;
        for (i = 0; i < graph->member_count; i++) {
            graph->member_index[i].hash = graph->members[i].hash;
            graph->member_index[i].id = i;
        }
        qsort(graph->member_index,
              graph->member_count,
              sizeof(*graph->member_index),
              rx_graph_hash_index_compare);
    }

    if (graph->callable_count) {
        graph->callable_index = (RxGraphHashIndex *)malloc(
            (size_t)graph->callable_count * sizeof(*graph->callable_index));
        if (!graph->callable_index) return 0;
        for (i = 0; i < graph->callable_count; i++) {
            graph->callable_index[i].hash = graph->callables[i].hash;
            graph->callable_index[i].id = i;
        }
        qsort(graph->callable_index,
              graph->callable_count,
              sizeof(*graph->callable_index),
              rx_graph_hash_index_compare);
    }

    if (graph->edge_count) {
        graph->outgoing_index = (RxGraphEdgeIndex *)malloc(
            (size_t)graph->edge_count * sizeof(*graph->outgoing_index));
        graph->incoming_index = (RxGraphEdgeIndex *)malloc(
            (size_t)graph->edge_count * sizeof(*graph->incoming_index));
        if (!graph->outgoing_index || !graph->incoming_index) return 0;
        for (i = 0; i < graph->edge_count; i++) {
            graph->outgoing_index[i].type = graph->edges[i].from;
            graph->outgoing_index[i].relation = graph->edges[i].relation;
            graph->outgoing_index[i].other = graph->edges[i].to;
            graph->outgoing_index[i].edge = i;
            graph->incoming_index[i].type = graph->edges[i].to;
            graph->incoming_index[i].relation = graph->edges[i].relation;
            graph->incoming_index[i].other = graph->edges[i].from;
            graph->incoming_index[i].edge = i;
        }
        qsort(graph->outgoing_index,
              graph->edge_count,
              sizeof(*graph->outgoing_index),
              rx_graph_edge_index_compare);
        qsort(graph->incoming_index,
              graph->edge_count,
              sizeof(*graph->incoming_index),
              rx_graph_edge_index_compare);
    }

    if (graph->declaration_count) {
        graph->declaration_index = (RxGraphDeclarationIndex *)malloc(
            (size_t)graph->declaration_count * sizeof(*graph->declaration_index));
        if (!graph->declaration_index) return 0;
        for (i = 0u; i < graph->declaration_count; i++) {
            graph->declaration_index[i].owner = graph->declarations[i].owner;
            graph->declaration_index[i].member = graph->declarations[i].member;
            graph->declaration_index[i].declaration = i;
        }
        qsort(graph->declaration_index,
              graph->declaration_count,
              sizeof(*graph->declaration_index),
              rx_graph_declaration_index_compare);
    }

    if (graph->dispatch_count) {
        graph->dispatch_index = (RxGraphDispatchIndex *)malloc(
            (size_t)graph->dispatch_count * sizeof(*graph->dispatch_index));
        if (!graph->dispatch_index) return 0;
        for (i = 0; i < graph->dispatch_count; i++) {
            graph->dispatch_index[i].owner = graph->dispatches[i].owner;
            graph->dispatch_index[i].member = graph->dispatches[i].member;
            graph->dispatch_index[i].callable = graph->dispatches[i].callable;
        }
        qsort(graph->dispatch_index,
              graph->dispatch_count,
              sizeof(*graph->dispatch_index),
              rx_graph_dispatch_index_compare);
    }

    if (graph->factory_count) {
        graph->factory_index = (RxGraphFactoryIndex *)malloc(
            (size_t)graph->factory_count * sizeof(*graph->factory_index));
        if (!graph->factory_index) return 0;
        for (i = 0u; i < graph->factory_count; i++) {
            graph->factory_index[i].interface_type = graph->factories[i].interface_type;
            graph->factory_index[i].member = graph->factories[i].member;
            graph->factory_index[i].factory = i;
        }
        qsort(graph->factory_index,
              graph->factory_count,
              sizeof(*graph->factory_index),
              rx_graph_factory_index_compare);
    }

    if (graph->provider_count) {
        graph->provider_index = (RxGraphProviderIndex *)malloc(
            (size_t)graph->provider_count * sizeof(*graph->provider_index));
        if (!graph->provider_index) return 0;
        for (i = 0; i < graph->provider_count; i++) {
            graph->provider_index[i].interface_type =
                graph->providers[i].interface_type;
            graph->provider_index[i].factory_member =
                graph->providers[i].factory_member;
            graph->provider_index[i].provider = i;
        }
        rx_graph_sort_provider_index(graph);
    }

    return 1;
}

RxGraph *rx_graph_builder_finish(RxGraphBuilder *builder) {
    RxGraph *graph;

    if (!builder) return 0;
    graph = builder->graph;
    if (builder->failed || !rx_graph_build_indices(graph) ||
        !rx_graph_build_runtime_views(graph)) {
        rx_graph_builder_free(builder);
        return 0;
    }
    builder->graph = 0;
    free(builder);
    return graph;
}

void rx_graph_retain(RxGraph *graph) {
    if (graph) graph->refcount++;
}

void rx_graph_release(RxGraph **graph) {
    RxGraph *current;

    if (!graph || !*graph) return;
    current = *graph;
    *graph = 0;
    if (current->refcount > 1u) {
        current->refcount--;
        return;
    }
    rx_graph_destroy(current);
}

size_t rx_graph_type_count(const RxGraph *graph) {
    return graph ? graph->type_count : 0u;
}

size_t rx_graph_member_count(const RxGraph *graph) {
    return graph ? graph->member_count : 0u;
}

size_t rx_graph_callable_count(const RxGraph *graph) {
    return graph ? graph->callable_count : 0u;
}

size_t rx_graph_factory_count(const RxGraph *graph) {
    return graph ? graph->factory_count : 0u;
}

size_t rx_graph_provider_count(const RxGraph *graph) {
    return graph ? graph->provider_count : 0u;
}

size_t rx_graph_relationship_count(const RxGraph *graph) {
    return graph ? graph->edge_count : 0u;
}

size_t rx_graph_declaration_total(const RxGraph *graph) {
    return graph ? graph->declaration_count : 0u;
}

const RxGraphTypeRef *rx_graph_type_ref(const RxGraph *graph, RxGraphId type) {
    if (!graph || type >= graph->type_count || !graph->runtime_types) return 0;
    return &graph->runtime_types[type];
}

static RxGraphId rx_graph_find_hash(const RxGraph *graph,
                                    const RxGraphHashIndex *index,
                                    uint32_t count,
                                    const char *key,
                                    int kind) {
    uint64_t hash;
    uint32_t left;
    uint32_t right;
    uint32_t position;

    if (!graph || !index || !key) return RX_GRAPH_NONE;
    hash = rx_graph_hash(key);
    left = 0u;
    right = count;
    while (left < right) {
        uint32_t mid;
        mid = left + ((right - left) >> 1u);
        if (index[mid].hash < hash) left = mid + 1u;
        else right = mid;
    }
    position = left;
    while (position < count && index[position].hash == hash) {
        uint32_t id;
        const char *candidate;

        id = index[position].id;
        if (kind == 0) {
            candidate = rx_graph_string(graph, graph->types[id].name_offset);
        } else if (kind == 1) {
            candidate = rx_graph_string(graph,
                                        graph->members[id].descriptor_offset);
        } else {
            candidate = rx_graph_string(graph,
                                        graph->callables[id].symbol_offset);
        }
        if (candidate && strcmp(candidate, key) == 0) return id;
        position++;
    }
    return RX_GRAPH_NONE;
}

RxGraphId rx_graph_find_type(const RxGraph *graph, const char *name) {
    char *normalized;
    RxGraphId result;

    if (!graph || !name) return RX_GRAPH_NONE;
    normalized = rx_graph_normalize_type_name(name);
    if (!normalized) return RX_GRAPH_NONE;
    result = rx_graph_find_hash(graph,
                                graph->type_index,
                                graph->type_count,
                                normalized,
                                0);
    free(normalized);
    return result;
}

const char *rx_graph_type_name(const RxGraph *graph, RxGraphId type) {
    if (!graph || type >= graph->type_count) return 0;
    return rx_graph_string(graph, graph->types[type].name_offset);
}

RxGraphTypeKind rx_graph_type_kind(const RxGraph *graph, RxGraphId type) {
    if (!graph || type >= graph->type_count) return RX_GRAPH_TYPE_OPAQUE;
    return (RxGraphTypeKind)graph->types[type].kind;
}

uint32_t rx_graph_type_flags(const RxGraph *graph, RxGraphId type) {
    if (!graph || type >= graph->type_count) return 0u;
    return graph->types[type].flags;
}

RxMemberId rx_graph_find_member(const RxGraph *graph, const char *descriptor) {
    RxMemberId member;
    uint32_t i;

    member = rx_graph_find_hash(graph,
                                graph ? graph->member_index : 0,
                                graph ? graph->member_count : 0u,
                                descriptor,
                                1);
    if (member != RX_GRAPH_NONE || !graph || !descriptor) return member;

    for (i = 0u; i < graph->member_count; i++) {
        const char *candidate;
        candidate = rx_graph_string(graph, graph->members[i].descriptor_offset);
        if (candidate && rx_graph_descriptors_match(candidate, descriptor)) return i;
    }
    return RX_GRAPH_NONE;
}

int rx_graph_member(const RxGraph *graph,
                    RxMemberId member,
                    RxGraphMemberView *view) {
    const RxGraphMemberRecord *record;

    if (!graph || !view || member >= graph->member_count) return 0;
    record = &graph->members[member];
    view->name = rx_graph_string(graph, record->name_offset);
    view->descriptor = rx_graph_string(graph, record->descriptor_offset);
    view->return_type = record->return_type;
    view->parameter_count = record->parameter_count;
    view->flags = record->flags;
    return view->name != 0 && view->descriptor != 0;
}

int rx_graph_member_parameter(const RxGraph *graph,
                              RxMemberId member,
                              uint32_t parameter_index,
                              RxGraphParamView *view) {
    const RxGraphMemberRecord *record;
    const RxGraphParamRecord *parameter;

    if (!graph || !view || member >= graph->member_count) return 0;
    record = &graph->members[member];
    if (record->parameter_count > graph->parameter_count ||
        parameter_index >= record->parameter_count ||
        record->parameter_first > graph->parameter_count - record->parameter_count) {
        return 0;
    }
    parameter = &graph->parameters[record->parameter_first + parameter_index];
    view->type = parameter->type;
    view->flags = parameter->flags;
    return 1;
}

static void rx_graph_declaration_range(const RxGraph *graph,
                                       RxGraphId owner,
                                       uint32_t *first,
                                       uint32_t *count) {
    uint32_t left;
    uint32_t right;
    uint32_t start;

    *first = 0u;
    *count = 0u;
    if (!graph || owner >= graph->type_count || !graph->declaration_count) return;
    left = 0u;
    right = graph->declaration_count;
    while (left < right) {
        uint32_t mid;
        mid = left + ((right - left) >> 1u);
        if (graph->declaration_index[mid].owner < owner) left = mid + 1u;
        else right = mid;
    }
    start = left;
    while (left < graph->declaration_count &&
           graph->declaration_index[left].owner == owner) left++;
    *first = start;
    *count = left - start;
}

size_t rx_graph_declaration_count(const RxGraph *graph, RxGraphId owner) {
    uint32_t first;
    uint32_t count;

    rx_graph_declaration_range(graph, owner, &first, &count);
    return count;
}

int rx_graph_declaration(const RxGraph *graph,
                         RxGraphId owner,
                         size_t position,
                         RxGraphDeclarationView *view) {
    const RxGraphDeclarationRecord *record;
    uint32_t declaration;
    uint32_t first;
    uint32_t count;

    if (!view) return 0;
    rx_graph_declaration_range(graph, owner, &first, &count);
    if (position >= count) return 0;
    declaration = graph->declaration_index[first + (uint32_t)position].declaration;
    record = &graph->declarations[declaration];
    view->owner = record->owner;
    view->member = record->member;
    view->flags = record->flags;
    view->origin_module = record->origin_module;
    view->ordinal = record->ordinal;
    return 1;
}

RxCallableId rx_graph_find_callable(const RxGraph *graph, const char *symbol) {
    return rx_graph_find_hash(graph,
                              graph ? graph->callable_index : 0,
                              graph ? graph->callable_count : 0u,
                              symbol,
                              2);
}

int rx_graph_callable(const RxGraph *graph,
                      RxCallableId callable,
                      RxGraphCallableView *view) {
    const RxGraphCallableRecord *record;

    if (!graph || !view || callable >= graph->callable_count) return 0;
    record = &graph->callables[callable];
    view->symbol = rx_graph_string(graph, record->symbol_offset);
    view->descriptor = rx_graph_string(graph, record->descriptor_offset);
    view->owner_type = record->owner_type;
    view->member = record->member;
    view->procedure.module_index = record->module_index;
    view->procedure.procedure_offset = record->procedure_offset;
    view->flags = record->flags;
    return view->symbol != 0 && view->descriptor != 0;
}

static uint32_t rx_graph_task_u32(const unsigned char *source) {
    return (uint32_t)source[0] |
           ((uint32_t)source[1] << 8) |
           ((uint32_t)source[2] << 16) |
           ((uint32_t)source[3] << 24);
}

static void rx_graph_task_put_u32(unsigned char *target, uint32_t value) {
    target[0] = (unsigned char)value;
    target[1] = (unsigned char)(value >> 8);
    target[2] = (unsigned char)(value >> 16);
    target[3] = (unsigned char)(value >> 24);
}

int rx_graph_digest(const RxGraph *graph, unsigned char digest[32]) {
    static const unsigned char domain[8] = {'R','X','G','R','D','1',0,0};
    unsigned char *facts = 0;
    unsigned char *indexes = 0;
    unsigned char *document = 0;
    size_t facts_size = 0u;
    size_t indexes_size = 0u;
    size_t document_size;
    int result = 0;

    if (!graph || !digest ||
        !rx_graph_serialize_sections(
                graph, &facts, &facts_size, &indexes, &indexes_size) ||
        facts_size > SIZE_MAX - sizeof(domain) ||
        indexes_size > SIZE_MAX - sizeof(domain) - facts_size) {
        free(facts);
        free(indexes);
        return 0;
    }
    document_size = sizeof(domain) + facts_size + indexes_size;
    document = (unsigned char *)malloc(document_size ? document_size : 1u);
    if (document) {
        memcpy(document, domain, sizeof(domain));
        if (facts_size) memcpy(document + sizeof(domain), facts, facts_size);
        if (indexes_size) {
            memcpy(document + sizeof(domain) + facts_size,
                   indexes, indexes_size);
        }
        rx_sha256(document, document_size, digest);
        result = 1;
    }
    free(document);
    free(facts);
    free(indexes);
    return result;
}

static int rx_graph_task_method_receiver_factory(
        const RxGraph *graph,
        const RxGraphCallableView *method,
        RxCallableId *factory_out) {
    const char *owner_name;
    const char *argument_type;
    char *normalized_argument_type;
    char *factory_symbol;
    size_t symbol_length;
    RxCallableId factory;
    RxGraphCallableView view;
    rx_callable_signature signature;
    RxGraphId return_type;
    int valid;

    if (factory_out) *factory_out = RX_GRAPH_NONE;
    if (!graph || !method || method->owner_type == RX_GRAPH_NONE ||
        method->owner_type >= rx_graph_type_count(graph)) return 0;
    owner_name = rx_graph_type_name(graph, method->owner_type);
    if (!owner_name ||
        strlen(owner_name) > SIZE_MAX - sizeof(".§factory.from_channel")) {
        return 0;
    }
    symbol_length = strlen(owner_name) +
                    sizeof(".§factory.from_channel");
    factory_symbol = (char *)malloc(symbol_length);
    if (!factory_symbol) return 0;
    snprintf(factory_symbol, symbol_length, "%s.§factory.from_channel",
             owner_name);
    factory = rx_graph_find_callable(graph, factory_symbol);
    free(factory_symbol);
    if (factory == RX_GRAPH_NONE ||
        !rx_graph_callable(graph, factory, &view) ||
        !rx_sig_parse_descriptor(view.descriptor, &signature)) {
        return 0;
    }

    return_type = rx_graph_find_type(graph, signature.return_type);
    argument_type = signature.arg_count == 1u ? signature.args[0].type : 0;
    normalized_argument_type = argument_type
            ? rx_graph_normalize_type_name(argument_type) : 0;
    valid = signature.arg_count == 1u &&
            !signature.args[0].is_ref &&
            !signature.args[0].is_vararg &&
            return_type == method->owner_type &&
            normalized_argument_type &&
            (strcmp(normalized_argument_type, ".channelvalue") == 0 ||
             strcmp(normalized_argument_type,
                    "concurrency.channelvalue") == 0);
    free(normalized_argument_type);
    rx_sig_free(&signature);
    if (!valid) return 0;
    if (factory_out) *factory_out = factory;
    return 1;
}

static int rx_graph_task_type_spelling_is(const char *type_name,
                                          const char *short_name,
                                          const char *canonical_name) {
    char *normalized;
    int matches;

    if (!type_name || !short_name || !canonical_name) return 0;
    normalized = rx_graph_normalize_type_name(type_name);
    if (!normalized) return 0;
    matches = strcmp(normalized, short_name) == 0 ||
              strcmp(normalized, canonical_name) == 0;
    free(normalized);
    return matches;
}

static int rx_graph_taskwork_run_adapter(
        const RxGraph *graph,
        const RxGraphCallableView *factory,
        RxCallableId *adapter_out) {
    rx_callable_signature factory_signature;
    rx_callable_signature run_signature;
    char *class_name;
    char *run_symbol;
    size_t symbol_length;
    RxCallableId adapter;
    RxGraphCallableView view;
    int valid;

    if (adapter_out) *adapter_out = RX_GRAPH_NONE;
    if (!graph || !factory ||
        !rx_sig_parse_descriptor(factory->descriptor, &factory_signature)) {
        return 0;
    }
    class_name = rx_graph_normalize_type_name(factory_signature.return_type);
    rx_sig_free(&factory_signature);
    if (!class_name || strlen(class_name) > SIZE_MAX - sizeof(".run")) {
        free(class_name);
        return 0;
    }
    symbol_length = strlen(class_name) + sizeof(".run");
    run_symbol = (char *)malloc(symbol_length);
    if (!run_symbol) {
        free(class_name);
        return 0;
    }
    snprintf(run_symbol, symbol_length, "%s.run", class_name);
    adapter = rx_graph_find_callable(graph, run_symbol);
    free(run_symbol);
    if (adapter == RX_GRAPH_NONE ||
        !rx_graph_callable(graph, adapter, &view) ||
        !rx_sig_parse_descriptor(view.descriptor, &run_signature)) {
        free(class_name);
        return 0;
    }
    if (view.owner_type != RX_GRAPH_NONE) {
        const char *owner_name = rx_graph_type_name(graph, view.owner_type);
        if (!owner_name || strcmp(owner_name, class_name) != 0) {
            rx_sig_free(&run_signature);
            free(class_name);
            return 0;
        }
    }
    free(class_name);
    valid = run_signature.arg_count == 2u &&
            !run_signature.args[0].is_ref &&
            !run_signature.args[0].is_vararg &&
            !run_signature.args[1].is_ref &&
            !run_signature.args[1].is_vararg &&
            rx_graph_task_type_spelling_is(
                    run_signature.return_type, ".channelvalue",
                    "concurrency.channelvalue") &&
            rx_graph_task_type_spelling_is(
                    run_signature.args[0].type, ".channelvalue",
                    "concurrency.channelvalue") &&
            rx_graph_task_type_spelling_is(
                    run_signature.args[1].type, ".taskcontext",
                    "concurrency.taskcontext");
    rx_sig_free(&run_signature);
    if (!valid) return 0;
    if (adapter_out) *adapter_out = adapter;
    return 1;
}

int rx_graph_task_binding(const RxGraph *graph,
                          const char *symbol,
                          unsigned int kind,
                          unsigned char binding[RX_GRAPH_TASK_BINDING_SIZE]) {
    RxCallableId callable;
    RxCallableId auxiliary = RX_GRAPH_NONE;
    RxGraphCallableView view;

    if (!graph || !symbol || !binding || kind < 1u || kind > 3u) return 0;
    callable = rx_graph_find_callable(graph, symbol);
    if (callable == RX_GRAPH_NONE || !rx_graph_callable(graph, callable, &view)) {
        return 0;
    }
    memset(binding, 0, RX_GRAPH_TASK_BINDING_SIZE);
    memcpy(binding, "RXTB", 4u);
    binding[4] = 1u;
    binding[5] = (unsigned char)kind;
    rx_graph_task_put_u32(binding + 8u, callable);
    if (kind == 2u && view.owner_type != RX_GRAPH_NONE &&
        !rx_graph_task_method_receiver_factory(graph, &view, &auxiliary)) {
        return 0;
    } else if (kind == 3u &&
               !rx_graph_taskwork_run_adapter(graph, &view, &auxiliary)) {
        return 0;
    }
    if (!rx_graph_digest(graph, binding + 12u)) return 0;
    rx_sha256(view.descriptor, strlen(view.descriptor), binding + 44u);
    if (auxiliary != RX_GRAPH_NONE) {
        if (auxiliary == UINT32_MAX) return 0;
        rx_graph_task_put_u32(binding + 76u, auxiliary + 1u);
    }
    return 1;
}

int rx_graph_task_binding_validate_digest(
        const RxGraph *graph,
        const unsigned char graph_digest[32],
        const unsigned char binding[RX_GRAPH_TASK_BINDING_SIZE],
        RxCallableId *callable_out,
        unsigned int *kind_out) {
    RxCallableId callable;
    RxCallableId auxiliary;
    RxCallableId expected_auxiliary = RX_GRAPH_NONE;
    RxGraphCallableView view;
    unsigned char signature[32];
    unsigned int kind;

    if (!graph || !graph_digest || !binding ||
        memcmp(binding, "RXTB", 4u) != 0 || binding[4] != 1u ||
        binding[6] != 0u || binding[7] != 0u) return 0;
    kind = binding[5];
    callable = rx_graph_task_u32(binding + 8u);
    auxiliary = rx_graph_task_u32(binding + 76u);
    if (kind < 1u || kind > 3u ||
        !rx_graph_callable(graph, callable, &view) ||
        memcmp(graph_digest, binding + 12u, 32u) != 0) return 0;
    rx_sha256(view.descriptor, strlen(view.descriptor), signature);
    if (memcmp(signature, binding + 44u, sizeof(signature)) != 0) return 0;
    if (kind == 2u) {
        if (view.owner_type != RX_GRAPH_NONE) {
            if (!rx_graph_task_method_receiver_factory(
                        graph, &view, &expected_auxiliary) ||
                expected_auxiliary == UINT32_MAX ||
                auxiliary != expected_auxiliary + 1u) return 0;
        } else if (auxiliary != 0u) {
            return 0;
        }
    } else if (kind == 3u) {
        if (!rx_graph_taskwork_run_adapter(
                    graph, &view, &expected_auxiliary) ||
            expected_auxiliary == UINT32_MAX ||
            auxiliary != expected_auxiliary + 1u) return 0;
    } else if (auxiliary != 0u) {
        return 0;
    }
    if (callable_out) *callable_out = callable;
    if (kind_out) *kind_out = kind;
    return 1;
}

int rx_graph_task_binding_validate(
        const RxGraph *graph,
        const unsigned char binding[RX_GRAPH_TASK_BINDING_SIZE],
        RxCallableId *callable_out,
        unsigned int *kind_out) {
    unsigned char graph_digest[32];

    return graph && rx_graph_digest(graph, graph_digest) &&
           rx_graph_task_binding_validate_digest(
                   graph, graph_digest, binding, callable_out, kind_out);
}

RxFactoryId rx_graph_find_factory(const RxGraph *graph,
                                  RxGraphId interface_type,
                                  RxMemberId member) {
    if (!graph || interface_type >= graph->type_count ||
        member >= graph->member_count || !graph->factory_view) {
        return RX_GRAPH_NONE;
    }
    return graph->factory_view[
        (size_t)interface_type * graph->member_count + member];
}

int rx_graph_factory(const RxGraph *graph,
                     RxFactoryId factory,
                     RxGraphFactoryView *view) {
    const RxGraphFactoryRecord *record;

    if (!graph || !view || factory >= graph->factory_count) return 0;
    record = &graph->factories[factory];
    view->interface_type = record->interface_type;
    view->member = record->member;
    view->flags = record->flags;
    return 1;
}

static void rx_graph_edge_range(const RxGraph *graph,
                                RxGraphId type,
                                RxGraphRelation relation,
                                int incoming,
                                uint32_t *first,
                                uint32_t *count) {
    const RxGraphEdgeIndex *index;
    uint32_t left;
    uint32_t right;
    uint32_t start;

    *first = 0u;
    *count = 0u;
    if (!graph || type >= graph->type_count || !graph->edge_count) return;
    index = incoming ? graph->incoming_index : graph->outgoing_index;
    left = 0u;
    right = graph->edge_count;
    while (left < right) {
        uint32_t mid;
        mid = left + ((right - left) >> 1u);
        if (index[mid].type < type ||
            (index[mid].type == type && index[mid].relation < (uint32_t)relation)) {
            left = mid + 1u;
        } else {
            right = mid;
        }
    }
    start = left;
    while (left < graph->edge_count &&
           index[left].type == type &&
           index[left].relation == (uint32_t)relation) {
        left++;
    }
    *first = start;
    *count = left - start;
}

size_t rx_graph_edge_count(const RxGraph *graph,
                           RxGraphId type,
                           RxGraphRelation relation,
                           int incoming) {
    uint32_t first;
    uint32_t count;

    rx_graph_edge_range(graph, type, relation, incoming, &first, &count);
    return count;
}

int rx_graph_edge(const RxGraph *graph,
                  RxGraphId type,
                  RxGraphRelation relation,
                  int incoming,
                  size_t position,
                  RxGraphEdgeView *view) {
    const RxGraphEdgeIndex *index;
    const RxGraphEdgeRecord *record;
    uint32_t first;
    uint32_t count;

    if (!view) return 0;
    rx_graph_edge_range(graph, type, relation, incoming, &first, &count);
    if (position >= count) return 0;
    index = incoming ? graph->incoming_index : graph->outgoing_index;
    record = &graph->edges[index[first + (uint32_t)position].edge];
    view->from = record->from;
    view->to = record->to;
    view->relation = record->relation;
    view->origin_module = record->origin_module;
    view->ordinal = record->ordinal;
    return 1;
}

int rx_graph_type_supports(const RxGraph *graph,
                           RxGraphId concrete_type,
                           RxGraphId target_type) {
    const uint64_t *row;

    if (!graph || concrete_type >= graph->type_count ||
        target_type >= graph->type_count || !graph->assignability_view) return 0;
    row = graph->assignability_view +
          (size_t)concrete_type * graph->assignability_word_count;
    return (row[target_type / 64u] &
            (UINT64_C(1) << (target_type & 63u))) != 0u;
}

RxCallableId rx_graph_dispatch(const RxGraph *graph,
                               RxGraphId concrete_type,
                               RxMemberId member) {
    if (!graph || concrete_type >= graph->type_count ||
        member >= graph->member_count || !graph->dispatch_view) return RX_GRAPH_NONE;
    return graph->dispatch_view[
        (size_t)concrete_type * graph->member_count + member];
}

static void rx_graph_provider_range(const RxGraph *graph,
                                    RxGraphId interface_type,
                                    RxMemberId factory_member,
                                    uint32_t *first,
                                    uint32_t *count) {
    RxFactoryId factory;

    *first = 0u;
    *count = 0u;
    factory = rx_graph_find_factory(graph, interface_type, factory_member);
    if (factory == RX_GRAPH_NONE || factory >= graph->factory_count ||
        !graph->provider_first_by_factory ||
        !graph->provider_count_by_factory) return;
    *count = graph->provider_count_by_factory[factory];
    if (*count) *first = graph->provider_first_by_factory[factory];
}

size_t rx_graph_provider_bucket_size(const RxGraph *graph,
                                     RxGraphId interface_type,
                                     RxMemberId factory_member) {
    uint32_t first;
    uint32_t count;

    rx_graph_provider_range(graph,
                            interface_type,
                            factory_member,
                            &first,
                            &count);
    return count;
}

int rx_graph_provider(const RxGraph *graph,
                      RxGraphId interface_type,
                      RxMemberId factory_member,
                      size_t position,
                      RxGraphProviderView *view) {
    const RxGraphProviderRecord *record;
    uint32_t first;
    uint32_t count;
    uint32_t provider;

    if (!view) return 0;
    rx_graph_provider_range(graph,
                            interface_type,
                            factory_member,
                            &first,
                            &count);
    if (position >= count) return 0;
    provider = graph->provider_index[first + (uint32_t)position].provider;
    record = &graph->providers[provider];
    view->interface_type = record->interface_type;
    view->factory_member = record->factory_member;
    view->class_type = record->class_type;
    view->factory_callable = record->factory_callable;
    view->match_callable = record->match_callable;
    view->origin_module = record->origin_module;
    view->ordinal = record->ordinal;
    return 1;
}

typedef struct RxGraphByteBuffer {
    unsigned char *data;
    size_t size;
    size_t capacity;
} RxGraphByteBuffer;

typedef struct RxGraphByteReader {
    const unsigned char *cursor;
    const unsigned char *end;
} RxGraphByteReader;

static int rx_graph_buffer_reserve(RxGraphByteBuffer *buffer, size_t extra) {
    size_t required;
    size_t capacity;
    unsigned char *data;

    if (extra > SIZE_MAX - buffer->size) return 0;
    required = buffer->size + extra;
    if (required <= buffer->capacity) return 1;
    capacity = buffer->capacity ? buffer->capacity : 256u;
    while (capacity < required) {
        if (capacity > SIZE_MAX / 2u) {
            capacity = required;
            break;
        }
        capacity *= 2u;
    }
    data = (unsigned char *)realloc(buffer->data, capacity);
    if (!data) return 0;
    buffer->data = data;
    buffer->capacity = capacity;
    return 1;
}

static int rx_graph_buffer_bytes(RxGraphByteBuffer *buffer,
                                 const void *data,
                                 size_t size) {
    if (!rx_graph_buffer_reserve(buffer, size)) return 0;
    if (size) memcpy(buffer->data + buffer->size, data, size);
    buffer->size += size;
    return 1;
}

static int rx_graph_buffer_u32(RxGraphByteBuffer *buffer, uint32_t value) {
    unsigned char bytes[4];

    bytes[0] = (unsigned char)(value & 0xffu);
    bytes[1] = (unsigned char)((value >> 8u) & 0xffu);
    bytes[2] = (unsigned char)((value >> 16u) & 0xffu);
    bytes[3] = (unsigned char)((value >> 24u) & 0xffu);
    return rx_graph_buffer_bytes(buffer, bytes, sizeof(bytes));
}

static int rx_graph_buffer_u64(RxGraphByteBuffer *buffer, uint64_t value) {
    unsigned char bytes[8];
    unsigned int i;

    for (i = 0u; i < 8u; i++) {
        bytes[i] = (unsigned char)((value >> (i * 8u)) & UINT64_C(0xff));
    }
    return rx_graph_buffer_bytes(buffer, bytes, sizeof(bytes));
}

static int rx_graph_reader_bytes(RxGraphByteReader *reader,
                                 void *data,
                                 size_t size) {
    if (!reader || size > (size_t)(reader->end - reader->cursor)) return 0;
    if (size && data) memcpy(data, reader->cursor, size);
    reader->cursor += size;
    return 1;
}

static int rx_graph_reader_u32(RxGraphByteReader *reader, uint32_t *value) {
    unsigned char bytes[4];

    if (!rx_graph_reader_bytes(reader, bytes, sizeof(bytes))) return 0;
    *value = (uint32_t)bytes[0] |
             ((uint32_t)bytes[1] << 8u) |
             ((uint32_t)bytes[2] << 16u) |
             ((uint32_t)bytes[3] << 24u);
    return 1;
}

static int rx_graph_reader_u64(RxGraphByteReader *reader, uint64_t *value) {
    unsigned char bytes[8];
    uint64_t result;
    unsigned int i;

    if (!rx_graph_reader_bytes(reader, bytes, sizeof(bytes))) return 0;
    result = 0u;
    for (i = 0u; i < 8u; i++) {
        result |= ((uint64_t)bytes[i]) << (i * 8u);
    }
    *value = result;
    return 1;
}

static void rx_graph_set_error(char **error_message, const char *message) {
    if (!error_message || *error_message) return;
    *error_message = rx_graph_strdup(message ? message : "invalid semantic graph");
}

static int rx_graph_write_hash_indices(RxGraphByteBuffer *buffer,
                                       const RxGraphHashIndex *indices,
                                       uint32_t count) {
    uint32_t i;
    for (i = 0u; i < count; i++) {
        if (!rx_graph_buffer_u64(buffer, indices[i].hash) ||
            !rx_graph_buffer_u32(buffer, indices[i].id)) return 0;
    }
    return 1;
}

int rx_graph_serialize(const RxGraph *graph,
                       unsigned char **data,
                       size_t *size) {
    RxGraphByteBuffer buffer;
    uint32_t i;

    if (!graph || !data || !size) return 0;
    *data = 0;
    *size = 0u;
    memset(&buffer, 0, sizeof(buffer));
    if (!rx_graph_buffer_bytes(&buffer, "RXG7", 4u) ||
        !rx_graph_buffer_u32(&buffer, RX_GRAPH_SERIAL_VERSION) ||
        !rx_graph_buffer_u32(&buffer, graph->string_size) ||
        !rx_graph_buffer_u32(&buffer, graph->type_count) ||
        !rx_graph_buffer_u32(&buffer, graph->member_count) ||
        !rx_graph_buffer_u32(&buffer, graph->parameter_count) ||
        !rx_graph_buffer_u32(&buffer, graph->edge_count) ||
        !rx_graph_buffer_u32(&buffer, graph->declaration_count) ||
        !rx_graph_buffer_u32(&buffer, graph->callable_count) ||
        !rx_graph_buffer_u32(&buffer, graph->dispatch_count) ||
        !rx_graph_buffer_u32(&buffer, graph->factory_count) ||
        !rx_graph_buffer_u32(&buffer, graph->provider_count) ||
        !rx_graph_buffer_bytes(&buffer, graph->strings, graph->string_size)) {
        free(buffer.data);
        return 0;
    }

    for (i = 0u; i < graph->type_count; i++) {
        const RxGraphTypeRecord *record;
        record = &graph->types[i];
        if (!rx_graph_buffer_u32(&buffer, record->name_offset) ||
            !rx_graph_buffer_u32(&buffer, record->name_length) ||
            !rx_graph_buffer_u32(&buffer, record->kind) ||
            !rx_graph_buffer_u32(&buffer, record->flags) ||
            !rx_graph_buffer_u64(&buffer, record->hash)) goto error;
    }
    for (i = 0u; i < graph->member_count; i++) {
        const RxGraphMemberRecord *record;
        record = &graph->members[i];
        if (!rx_graph_buffer_u32(&buffer, record->name_offset) ||
            !rx_graph_buffer_u32(&buffer, record->descriptor_offset) ||
            !rx_graph_buffer_u32(&buffer, record->return_type) ||
            !rx_graph_buffer_u32(&buffer, record->parameter_first) ||
            !rx_graph_buffer_u32(&buffer, record->parameter_count) ||
            !rx_graph_buffer_u32(&buffer, record->flags) ||
            !rx_graph_buffer_u64(&buffer, record->hash)) goto error;
    }
    for (i = 0u; i < graph->parameter_count; i++) {
        if (!rx_graph_buffer_u32(&buffer, graph->parameters[i].type) ||
            !rx_graph_buffer_u32(&buffer, graph->parameters[i].flags)) goto error;
    }
    for (i = 0u; i < graph->edge_count; i++) {
        const RxGraphEdgeRecord *record;
        record = &graph->edges[i];
        if (!rx_graph_buffer_u32(&buffer, record->from) ||
            !rx_graph_buffer_u32(&buffer, record->to) ||
            !rx_graph_buffer_u32(&buffer, record->relation) ||
            !rx_graph_buffer_u32(&buffer, record->origin_module) ||
            !rx_graph_buffer_u32(&buffer, record->ordinal)) goto error;
    }
    for (i = 0u; i < graph->declaration_count; i++) {
        const RxGraphDeclarationRecord *record;
        record = &graph->declarations[i];
        if (!rx_graph_buffer_u32(&buffer, record->owner) ||
            !rx_graph_buffer_u32(&buffer, record->member) ||
            !rx_graph_buffer_u32(&buffer, record->flags) ||
            !rx_graph_buffer_u32(&buffer, record->origin_module) ||
            !rx_graph_buffer_u32(&buffer, record->ordinal)) goto error;
    }
    for (i = 0u; i < graph->callable_count; i++) {
        const RxGraphCallableRecord *record;
        record = &graph->callables[i];
        if (!rx_graph_buffer_u32(&buffer, record->symbol_offset) ||
            !rx_graph_buffer_u32(&buffer, record->descriptor_offset) ||
            !rx_graph_buffer_u32(&buffer, record->owner_type) ||
            !rx_graph_buffer_u32(&buffer, record->member) ||
            !rx_graph_buffer_u32(&buffer, record->module_index) ||
            !rx_graph_buffer_u32(&buffer, record->flags) ||
            !rx_graph_buffer_u64(&buffer, record->procedure_offset) ||
            !rx_graph_buffer_u64(&buffer, record->hash)) goto error;
    }
    for (i = 0u; i < graph->dispatch_count; i++) {
        if (!rx_graph_buffer_u32(&buffer, graph->dispatches[i].owner) ||
            !rx_graph_buffer_u32(&buffer, graph->dispatches[i].member) ||
            !rx_graph_buffer_u32(&buffer, graph->dispatches[i].callable)) goto error;
    }
    for (i = 0u; i < graph->factory_count; i++) {
        if (!rx_graph_buffer_u32(&buffer, graph->factories[i].interface_type) ||
            !rx_graph_buffer_u32(&buffer, graph->factories[i].member) ||
            !rx_graph_buffer_u32(&buffer, graph->factories[i].flags) ||
            !rx_graph_buffer_u32(&buffer, graph->factories[i].reserved)) goto error;
    }
    for (i = 0u; i < graph->provider_count; i++) {
        const RxGraphProviderRecord *record;
        record = &graph->providers[i];
        if (!rx_graph_buffer_u32(&buffer, record->interface_type) ||
            !rx_graph_buffer_u32(&buffer, record->factory_member) ||
            !rx_graph_buffer_u32(&buffer, record->class_type) ||
            !rx_graph_buffer_u32(&buffer, record->factory_callable) ||
            !rx_graph_buffer_u32(&buffer, record->match_callable) ||
            !rx_graph_buffer_u32(&buffer, record->origin_module) ||
            !rx_graph_buffer_u32(&buffer, record->ordinal)) goto error;
    }

    if (!rx_graph_write_hash_indices(&buffer, graph->type_index, graph->type_count) ||
        !rx_graph_write_hash_indices(&buffer, graph->member_index, graph->member_count) ||
        !rx_graph_write_hash_indices(&buffer, graph->callable_index, graph->callable_count)) goto error;
    for (i = 0u; i < graph->edge_count; i++) {
        if (!rx_graph_buffer_u32(&buffer, graph->outgoing_index[i].type) ||
            !rx_graph_buffer_u32(&buffer, graph->outgoing_index[i].relation) ||
            !rx_graph_buffer_u32(&buffer, graph->outgoing_index[i].other) ||
            !rx_graph_buffer_u32(&buffer, graph->outgoing_index[i].edge)) goto error;
    }
    for (i = 0u; i < graph->edge_count; i++) {
        if (!rx_graph_buffer_u32(&buffer, graph->incoming_index[i].type) ||
            !rx_graph_buffer_u32(&buffer, graph->incoming_index[i].relation) ||
            !rx_graph_buffer_u32(&buffer, graph->incoming_index[i].other) ||
            !rx_graph_buffer_u32(&buffer, graph->incoming_index[i].edge)) goto error;
    }
    for (i = 0u; i < graph->declaration_count; i++) {
        if (!rx_graph_buffer_u32(&buffer, graph->declaration_index[i].owner) ||
            !rx_graph_buffer_u32(&buffer, graph->declaration_index[i].member) ||
            !rx_graph_buffer_u32(&buffer, graph->declaration_index[i].declaration)) goto error;
    }
    for (i = 0u; i < graph->dispatch_count; i++) {
        if (!rx_graph_buffer_u32(&buffer, graph->dispatch_index[i].owner) ||
            !rx_graph_buffer_u32(&buffer, graph->dispatch_index[i].member) ||
            !rx_graph_buffer_u32(&buffer, graph->dispatch_index[i].callable)) goto error;
    }
    for (i = 0u; i < graph->factory_count; i++) {
        if (!rx_graph_buffer_u32(&buffer, graph->factory_index[i].interface_type) ||
            !rx_graph_buffer_u32(&buffer, graph->factory_index[i].member) ||
            !rx_graph_buffer_u32(&buffer, graph->factory_index[i].factory)) goto error;
    }
    for (i = 0u; i < graph->provider_count; i++) {
        if (!rx_graph_buffer_u32(&buffer, graph->provider_index[i].interface_type) ||
            !rx_graph_buffer_u32(&buffer, graph->provider_index[i].factory_member) ||
            !rx_graph_buffer_u32(&buffer, graph->provider_index[i].provider)) goto error;
    }

    *data = buffer.data;
    *size = buffer.size;
    return 1;

error:
    free(buffer.data);
    return 0;
}

static int rx_graph_alloc_records(void **records, uint32_t count, size_t size) {
    if (!count) {
        *records = 0;
        return 1;
    }
    if ((size_t)count > SIZE_MAX / size) return 0;
    *records = calloc(count, size);
    return *records != 0;
}

static int rx_graph_read_hash_indices(RxGraphByteReader *reader,
                                      RxGraphHashIndex *indices,
                                      uint32_t count) {
    uint32_t i;
    for (i = 0u; i < count; i++) {
        if (!rx_graph_reader_u64(reader, &indices[i].hash) ||
            !rx_graph_reader_u32(reader, &indices[i].id)) return 0;
    }
    return 1;
}

RxGraph *rx_graph_deserialize(const unsigned char *data,
                              size_t size,
                              char **error_message) {
    RxGraphByteReader reader;
    RxGraph *graph;
    unsigned char magic[4];
    uint32_t version;
    uint32_t i;

    if (error_message) *error_message = 0;
    if (!data || size < 4u) {
        rx_graph_set_error(error_message, "semantic graph section is truncated");
        return 0;
    }
    reader.cursor = data;
    reader.end = data + size;
    if (!rx_graph_reader_bytes(&reader, magic, sizeof(magic)) ||
        memcmp(magic, "RXG7", sizeof(magic)) != 0) {
        rx_graph_set_error(error_message, "semantic graph has an invalid magic value");
        return 0;
    }
    graph = (RxGraph *)calloc(1u, sizeof(*graph));
    if (!graph) {
        rx_graph_set_error(error_message, "out of memory reading semantic graph");
        return 0;
    }
    graph->refcount = 1u;
    if (!rx_graph_reader_u32(&reader, &version) ||
        !rx_graph_reader_u32(&reader, &graph->string_size) ||
        !rx_graph_reader_u32(&reader, &graph->type_count) ||
        !rx_graph_reader_u32(&reader, &graph->member_count) ||
        !rx_graph_reader_u32(&reader, &graph->parameter_count) ||
        !rx_graph_reader_u32(&reader, &graph->edge_count) ||
        !rx_graph_reader_u32(&reader, &graph->declaration_count) ||
        !rx_graph_reader_u32(&reader, &graph->callable_count) ||
        !rx_graph_reader_u32(&reader, &graph->dispatch_count) ||
        !rx_graph_reader_u32(&reader, &graph->factory_count) ||
        !rx_graph_reader_u32(&reader, &graph->provider_count)) {
        rx_graph_set_error(error_message, "semantic graph header is truncated");
        goto error;
    }
    if (version != RX_GRAPH_SERIAL_VERSION) {
        rx_graph_set_error(error_message, "semantic graph version is not supported");
        goto error;
    }
    if (!graph->string_size) {
        rx_graph_set_error(error_message, "semantic graph has an empty string table");
        goto error;
    }
    graph->string_capacity = graph->string_size;
    graph->type_capacity = graph->type_count;
    graph->member_capacity = graph->member_count;
    graph->parameter_capacity = graph->parameter_count;
    graph->edge_capacity = graph->edge_count;
    graph->declaration_capacity = graph->declaration_count;
    graph->callable_capacity = graph->callable_count;
    graph->dispatch_capacity = graph->dispatch_count;
    graph->factory_capacity = graph->factory_count;
    graph->provider_capacity = graph->provider_count;
    if (!rx_graph_alloc_records((void **)&graph->strings,
                                graph->string_size,
                                sizeof(*graph->strings)) ||
        !rx_graph_alloc_records((void **)&graph->types,
                                graph->type_count,
                                sizeof(*graph->types)) ||
        !rx_graph_alloc_records((void **)&graph->members,
                                graph->member_count,
                                sizeof(*graph->members)) ||
        !rx_graph_alloc_records((void **)&graph->parameters,
                                graph->parameter_count,
                                sizeof(*graph->parameters)) ||
        !rx_graph_alloc_records((void **)&graph->edges,
                                graph->edge_count,
                                sizeof(*graph->edges)) ||
        !rx_graph_alloc_records((void **)&graph->declarations,
                                graph->declaration_count,
                                sizeof(*graph->declarations)) ||
        !rx_graph_alloc_records((void **)&graph->callables,
                                graph->callable_count,
                                sizeof(*graph->callables)) ||
        !rx_graph_alloc_records((void **)&graph->dispatches,
                                graph->dispatch_count,
                                sizeof(*graph->dispatches)) ||
        !rx_graph_alloc_records((void **)&graph->factories,
                                graph->factory_count,
                                sizeof(*graph->factories)) ||
        !rx_graph_alloc_records((void **)&graph->providers,
                                graph->provider_count,
                                sizeof(*graph->providers)) ||
        !rx_graph_alloc_records((void **)&graph->type_index,
                                graph->type_count,
                                sizeof(*graph->type_index)) ||
        !rx_graph_alloc_records((void **)&graph->member_index,
                                graph->member_count,
                                sizeof(*graph->member_index)) ||
        !rx_graph_alloc_records((void **)&graph->callable_index,
                                graph->callable_count,
                                sizeof(*graph->callable_index)) ||
        !rx_graph_alloc_records((void **)&graph->outgoing_index,
                                graph->edge_count,
                                sizeof(*graph->outgoing_index)) ||
        !rx_graph_alloc_records((void **)&graph->incoming_index,
                                graph->edge_count,
                                sizeof(*graph->incoming_index)) ||
        !rx_graph_alloc_records((void **)&graph->declaration_index,
                                graph->declaration_count,
                                sizeof(*graph->declaration_index)) ||
        !rx_graph_alloc_records((void **)&graph->dispatch_index,
                                graph->dispatch_count,
                                sizeof(*graph->dispatch_index)) ||
        !rx_graph_alloc_records((void **)&graph->factory_index,
                                graph->factory_count,
                                sizeof(*graph->factory_index)) ||
        !rx_graph_alloc_records((void **)&graph->provider_index,
                                graph->provider_count,
                                sizeof(*graph->provider_index))) {
        rx_graph_set_error(error_message, "out of memory reading semantic graph records");
        goto error;
    }
    if (!rx_graph_reader_bytes(&reader, graph->strings, graph->string_size)) {
        rx_graph_set_error(error_message, "semantic graph string table is truncated");
        goto error;
    }
    for (i = 0u; i < graph->type_count; i++) {
        RxGraphTypeRecord *record = &graph->types[i];
        if (!rx_graph_reader_u32(&reader, &record->name_offset) ||
            !rx_graph_reader_u32(&reader, &record->name_length) ||
            !rx_graph_reader_u32(&reader, &record->kind) ||
            !rx_graph_reader_u32(&reader, &record->flags) ||
            !rx_graph_reader_u64(&reader, &record->hash)) goto truncated;
    }
    for (i = 0u; i < graph->member_count; i++) {
        RxGraphMemberRecord *record = &graph->members[i];
        if (!rx_graph_reader_u32(&reader, &record->name_offset) ||
            !rx_graph_reader_u32(&reader, &record->descriptor_offset) ||
            !rx_graph_reader_u32(&reader, &record->return_type) ||
            !rx_graph_reader_u32(&reader, &record->parameter_first) ||
            !rx_graph_reader_u32(&reader, &record->parameter_count) ||
            !rx_graph_reader_u32(&reader, &record->flags) ||
            !rx_graph_reader_u64(&reader, &record->hash)) goto truncated;
    }
    for (i = 0u; i < graph->parameter_count; i++) {
        if (!rx_graph_reader_u32(&reader, &graph->parameters[i].type) ||
            !rx_graph_reader_u32(&reader, &graph->parameters[i].flags)) goto truncated;
    }
    for (i = 0u; i < graph->edge_count; i++) {
        RxGraphEdgeRecord *record = &graph->edges[i];
        if (!rx_graph_reader_u32(&reader, &record->from) ||
            !rx_graph_reader_u32(&reader, &record->to) ||
            !rx_graph_reader_u32(&reader, &record->relation) ||
            !rx_graph_reader_u32(&reader, &record->origin_module) ||
            !rx_graph_reader_u32(&reader, &record->ordinal)) goto truncated;
    }
    for (i = 0u; i < graph->declaration_count; i++) {
        RxGraphDeclarationRecord *record = &graph->declarations[i];
        if (!rx_graph_reader_u32(&reader, &record->owner) ||
            !rx_graph_reader_u32(&reader, &record->member) ||
            !rx_graph_reader_u32(&reader, &record->flags) ||
            !rx_graph_reader_u32(&reader, &record->origin_module) ||
            !rx_graph_reader_u32(&reader, &record->ordinal)) goto truncated;
    }
    for (i = 0u; i < graph->callable_count; i++) {
        RxGraphCallableRecord *record = &graph->callables[i];
        if (!rx_graph_reader_u32(&reader, &record->symbol_offset) ||
            !rx_graph_reader_u32(&reader, &record->descriptor_offset) ||
            !rx_graph_reader_u32(&reader, &record->owner_type) ||
            !rx_graph_reader_u32(&reader, &record->member) ||
            !rx_graph_reader_u32(&reader, &record->module_index) ||
            !rx_graph_reader_u32(&reader, &record->flags) ||
            !rx_graph_reader_u64(&reader, &record->procedure_offset) ||
            !rx_graph_reader_u64(&reader, &record->hash)) goto truncated;
    }
    for (i = 0u; i < graph->dispatch_count; i++) {
        if (!rx_graph_reader_u32(&reader, &graph->dispatches[i].owner) ||
            !rx_graph_reader_u32(&reader, &graph->dispatches[i].member) ||
            !rx_graph_reader_u32(&reader, &graph->dispatches[i].callable)) goto truncated;
    }
    for (i = 0u; i < graph->factory_count; i++) {
        if (!rx_graph_reader_u32(&reader, &graph->factories[i].interface_type) ||
            !rx_graph_reader_u32(&reader, &graph->factories[i].member) ||
            !rx_graph_reader_u32(&reader, &graph->factories[i].flags) ||
            !rx_graph_reader_u32(&reader, &graph->factories[i].reserved)) goto truncated;
    }
    for (i = 0u; i < graph->provider_count; i++) {
        RxGraphProviderRecord *record = &graph->providers[i];
        if (!rx_graph_reader_u32(&reader, &record->interface_type) ||
            !rx_graph_reader_u32(&reader, &record->factory_member) ||
            !rx_graph_reader_u32(&reader, &record->class_type) ||
            !rx_graph_reader_u32(&reader, &record->factory_callable) ||
            !rx_graph_reader_u32(&reader, &record->match_callable) ||
            !rx_graph_reader_u32(&reader, &record->origin_module) ||
            !rx_graph_reader_u32(&reader, &record->ordinal)) goto truncated;
    }
    if (!rx_graph_read_hash_indices(&reader, graph->type_index, graph->type_count) ||
        !rx_graph_read_hash_indices(&reader, graph->member_index, graph->member_count) ||
        !rx_graph_read_hash_indices(&reader, graph->callable_index, graph->callable_count)) {
        goto truncated;
    }
    for (i = 0u; i < graph->edge_count; i++) {
        if (!rx_graph_reader_u32(&reader, &graph->outgoing_index[i].type) ||
            !rx_graph_reader_u32(&reader, &graph->outgoing_index[i].relation) ||
            !rx_graph_reader_u32(&reader, &graph->outgoing_index[i].other) ||
            !rx_graph_reader_u32(&reader, &graph->outgoing_index[i].edge)) goto truncated;
    }
    for (i = 0u; i < graph->edge_count; i++) {
        if (!rx_graph_reader_u32(&reader, &graph->incoming_index[i].type) ||
            !rx_graph_reader_u32(&reader, &graph->incoming_index[i].relation) ||
            !rx_graph_reader_u32(&reader, &graph->incoming_index[i].other) ||
            !rx_graph_reader_u32(&reader, &graph->incoming_index[i].edge)) goto truncated;
    }
    for (i = 0u; i < graph->declaration_count; i++) {
        if (!rx_graph_reader_u32(&reader, &graph->declaration_index[i].owner) ||
            !rx_graph_reader_u32(&reader, &graph->declaration_index[i].member) ||
            !rx_graph_reader_u32(&reader, &graph->declaration_index[i].declaration)) goto truncated;
    }
    for (i = 0u; i < graph->dispatch_count; i++) {
        if (!rx_graph_reader_u32(&reader, &graph->dispatch_index[i].owner) ||
            !rx_graph_reader_u32(&reader, &graph->dispatch_index[i].member) ||
            !rx_graph_reader_u32(&reader, &graph->dispatch_index[i].callable)) goto truncated;
    }
    for (i = 0u; i < graph->factory_count; i++) {
        if (!rx_graph_reader_u32(&reader, &graph->factory_index[i].interface_type) ||
            !rx_graph_reader_u32(&reader, &graph->factory_index[i].member) ||
            !rx_graph_reader_u32(&reader, &graph->factory_index[i].factory)) goto truncated;
    }
    for (i = 0u; i < graph->provider_count; i++) {
        if (!rx_graph_reader_u32(&reader, &graph->provider_index[i].interface_type) ||
            !rx_graph_reader_u32(&reader, &graph->provider_index[i].factory_member) ||
            !rx_graph_reader_u32(&reader, &graph->provider_index[i].provider)) goto truncated;
    }
    if (reader.cursor != reader.end) {
        rx_graph_set_error(error_message, "semantic graph section has trailing data");
        goto error;
    }
    if (!rx_graph_validate(graph, error_message)) goto error;
    if (!rx_graph_build_runtime_views(graph)) {
        rx_graph_set_error(error_message,
                           "out of memory building semantic graph runtime views");
        goto error;
    }
    return graph;

truncated:
    rx_graph_set_error(error_message, "semantic graph records are truncated");
error:
    rx_graph_destroy(graph);
    return 0;
}

static int rx_graph_serial_fact_size(const RxGraph *graph, size_t *size) {
    size_t result;
    size_t extra;

    if (!graph || !size) return 0;
    result = 48u;
#define RX_GRAPH_ADD_SERIAL_BYTES(count_, width_) \
    do { \
        if ((size_t)(count_) > SIZE_MAX / (size_t)(width_)) return 0; \
        extra = (size_t)(count_) * (size_t)(width_); \
        if (result > SIZE_MAX - extra) return 0; \
        result += extra; \
    } while (0)
    RX_GRAPH_ADD_SERIAL_BYTES(graph->string_size, 1u);
    RX_GRAPH_ADD_SERIAL_BYTES(graph->type_count, 24u);
    RX_GRAPH_ADD_SERIAL_BYTES(graph->member_count, 32u);
    RX_GRAPH_ADD_SERIAL_BYTES(graph->parameter_count, 8u);
    RX_GRAPH_ADD_SERIAL_BYTES(graph->edge_count, 20u);
    RX_GRAPH_ADD_SERIAL_BYTES(graph->declaration_count, 20u);
    RX_GRAPH_ADD_SERIAL_BYTES(graph->callable_count, 40u);
    RX_GRAPH_ADD_SERIAL_BYTES(graph->dispatch_count, 12u);
    RX_GRAPH_ADD_SERIAL_BYTES(graph->factory_count, 16u);
    RX_GRAPH_ADD_SERIAL_BYTES(graph->provider_count, 28u);
#undef RX_GRAPH_ADD_SERIAL_BYTES
    *size = result;
    return 1;
}

static int rx_graph_serial_index_size(const RxGraph *graph, size_t *size) {
    size_t result;
    size_t extra;

    if (!graph || !size) return 0;
    result = 40u;
#define RX_GRAPH_ADD_INDEX_BYTES(count_, width_) \
    do { \
        if ((size_t)(count_) > SIZE_MAX / (size_t)(width_)) return 0; \
        extra = (size_t)(count_) * (size_t)(width_); \
        if (result > SIZE_MAX - extra) return 0; \
        result += extra; \
    } while (0)
    RX_GRAPH_ADD_INDEX_BYTES(graph->type_count, 12u);
    RX_GRAPH_ADD_INDEX_BYTES(graph->member_count, 12u);
    RX_GRAPH_ADD_INDEX_BYTES(graph->callable_count, 12u);
    RX_GRAPH_ADD_INDEX_BYTES(graph->edge_count, 32u);
    RX_GRAPH_ADD_INDEX_BYTES(graph->declaration_count, 12u);
    RX_GRAPH_ADD_INDEX_BYTES(graph->dispatch_count, 12u);
    RX_GRAPH_ADD_INDEX_BYTES(graph->factory_count, 12u);
    RX_GRAPH_ADD_INDEX_BYTES(graph->provider_count, 12u);
#undef RX_GRAPH_ADD_INDEX_BYTES
    *size = result;
    return 1;
}

static int rx_graph_storage_add(size_t *total, size_t count, size_t width) {
    size_t bytes;

    if (!total || (count && width > SIZE_MAX / count)) return 0;
    bytes = count * width;
    if (*total > SIZE_MAX - bytes) return 0;
    *total += bytes;
    return 1;
}

int rx_graph_storage_stats(const RxGraph *graph, RxGraphStorageStats *stats) {
    size_t retained;
    size_t allocations;
    size_t string_offset;
    size_t type_words;
    size_t dense_entries;

    if (!graph || !stats) return 0;
    if (!rx_graph_size_multiply((size_t)graph->type_count,
                                graph->assignability_word_count,
                                &type_words) ||
        !rx_graph_size_multiply((size_t)graph->type_count,
                                graph->member_count,
                                &dense_entries)) return 0;
    memset(stats, 0, sizeof(*stats));
    retained = sizeof(*graph);
    allocations = 1u;
#define RX_GRAPH_ADD_RETAINED(pointer_, capacity_) \
    do { \
        if (!rx_graph_storage_add(&retained, \
                                  (size_t)(capacity_), \
                                  sizeof(*(pointer_)))) return 0; \
        if ((pointer_)) allocations++; \
    } while (0)
    RX_GRAPH_ADD_RETAINED(graph->strings, graph->string_capacity);
    RX_GRAPH_ADD_RETAINED(graph->types, graph->type_capacity);
    RX_GRAPH_ADD_RETAINED(graph->members, graph->member_capacity);
    RX_GRAPH_ADD_RETAINED(graph->parameters, graph->parameter_capacity);
    RX_GRAPH_ADD_RETAINED(graph->edges, graph->edge_capacity);
    RX_GRAPH_ADD_RETAINED(graph->declarations, graph->declaration_capacity);
    RX_GRAPH_ADD_RETAINED(graph->callables, graph->callable_capacity);
    RX_GRAPH_ADD_RETAINED(graph->dispatches, graph->dispatch_capacity);
    RX_GRAPH_ADD_RETAINED(graph->factories, graph->factory_capacity);
    RX_GRAPH_ADD_RETAINED(graph->providers, graph->provider_capacity);
    RX_GRAPH_ADD_RETAINED(graph->type_index, graph->type_count);
    RX_GRAPH_ADD_RETAINED(graph->member_index, graph->member_count);
    RX_GRAPH_ADD_RETAINED(graph->callable_index, graph->callable_count);
    RX_GRAPH_ADD_RETAINED(graph->outgoing_index, graph->edge_count);
    RX_GRAPH_ADD_RETAINED(graph->incoming_index, graph->edge_count);
    RX_GRAPH_ADD_RETAINED(graph->declaration_index, graph->declaration_count);
    RX_GRAPH_ADD_RETAINED(graph->dispatch_index, graph->dispatch_count);
    RX_GRAPH_ADD_RETAINED(graph->factory_index, graph->factory_count);
    RX_GRAPH_ADD_RETAINED(graph->provider_index, graph->provider_count);
    RX_GRAPH_ADD_RETAINED(graph->assignability_view, type_words);
    RX_GRAPH_ADD_RETAINED(graph->dispatch_view, dense_entries);
    RX_GRAPH_ADD_RETAINED(graph->factory_view, dense_entries);
    RX_GRAPH_ADD_RETAINED(graph->provider_first_by_factory,
                          graph->factory_count);
    RX_GRAPH_ADD_RETAINED(graph->provider_count_by_factory,
                          graph->factory_count);
    RX_GRAPH_ADD_RETAINED(graph->runtime_types, graph->type_count);
#undef RX_GRAPH_ADD_RETAINED
    if (!rx_graph_serial_fact_size(graph, &stats->serialized_facts_bytes) ||
        !rx_graph_serial_index_size(graph, &stats->serialized_indexes_bytes)) {
        return 0;
    }
    stats->retained_bytes = retained;
    stats->retained_allocations = allocations;
    stats->string_bytes = graph->string_size;
    string_offset = 0u;
    while (string_offset < graph->string_size) {
        const char *text;
        const char *terminator;
        size_t length;
        size_t previous_offset;
        int duplicate;

        text = graph->strings + string_offset;
        terminator = (const char *)memchr(text,
                                         0,
                                         graph->string_size - string_offset);
        if (!terminator) return 0;
        length = (size_t)(terminator - text) + 1u;
        stats->string_count++;
        duplicate = 0;
        previous_offset = 0u;
        while (previous_offset < string_offset) {
            const char *previous;
            const char *previous_terminator;
            size_t previous_length;
            previous = graph->strings + previous_offset;
            previous_terminator = (const char *)memchr(
                previous, 0, string_offset - previous_offset);
            if (!previous_terminator) return 0;
            previous_length = (size_t)(previous_terminator - previous) + 1u;
            if (previous_length == length &&
                memcmp(previous, text, length) == 0) {
                duplicate = 1;
                break;
            }
            previous_offset += previous_length;
        }
        if (!duplicate) {
            stats->unique_string_count++;
            if (!rx_graph_storage_add(&stats->unique_string_bytes, length, 1u)) {
                return 0;
            }
        }
        string_offset += length;
    }
    stats->type_count = graph->type_count;
    stats->member_count = graph->member_count;
    stats->parameter_count = graph->parameter_count;
    stats->relationship_count = graph->edge_count;
    stats->declaration_count = graph->declaration_count;
    stats->callable_count = graph->callable_count;
    stats->dispatch_count = graph->dispatch_count;
    stats->factory_count = graph->factory_count;
    stats->provider_count = graph->provider_count;
    return 1;
}

int rx_graph_serialize_sections(const RxGraph *graph,
                                unsigned char **facts,
                                size_t *facts_size,
                                unsigned char **indexes,
                                size_t *indexes_size) {
    unsigned char *combined;
    size_t combined_size;
    size_t fact_bytes;
    RxGraphByteBuffer index_buffer;
    size_t index_payload_size;

    if (!facts || !facts_size || !indexes || !indexes_size) return 0;
    *facts = 0;
    *facts_size = 0u;
    *indexes = 0;
    *indexes_size = 0u;
    combined = 0;
    combined_size = 0u;
    if (!rx_graph_serial_fact_size(graph, &fact_bytes) ||
        !rx_graph_serialize(graph, &combined, &combined_size) ||
        combined_size < fact_bytes) {
        free(combined);
        return 0;
    }
    *facts = (unsigned char *)malloc(fact_bytes);
    if (!*facts) {
        free(combined);
        return 0;
    }
    memcpy(*facts, combined, fact_bytes);
    memset(&index_buffer, 0, sizeof(index_buffer));
    index_payload_size = combined_size - fact_bytes;
    if (!rx_graph_buffer_bytes(&index_buffer, "RXI7", 4u) ||
        !rx_graph_buffer_u32(&index_buffer, RX_GRAPH_SERIAL_VERSION) ||
        !rx_graph_buffer_u32(&index_buffer, graph->type_count) ||
        !rx_graph_buffer_u32(&index_buffer, graph->member_count) ||
        !rx_graph_buffer_u32(&index_buffer, graph->callable_count) ||
        !rx_graph_buffer_u32(&index_buffer, graph->edge_count) ||
        !rx_graph_buffer_u32(&index_buffer, graph->declaration_count) ||
        !rx_graph_buffer_u32(&index_buffer, graph->dispatch_count) ||
        !rx_graph_buffer_u32(&index_buffer, graph->factory_count) ||
        !rx_graph_buffer_u32(&index_buffer, graph->provider_count) ||
        !rx_graph_buffer_bytes(&index_buffer,
                               combined + fact_bytes,
                               index_payload_size)) {
        free(combined);
        free(*facts);
        *facts = 0;
        free(index_buffer.data);
        return 0;
    }
    free(combined);
    *facts_size = fact_bytes;
    *indexes = index_buffer.data;
    *indexes_size = index_buffer.size;
    return 1;
}

RxGraph *rx_graph_deserialize_sections(const unsigned char *facts,
                                       size_t facts_size,
                                       const unsigned char *indexes,
                                       size_t indexes_size,
                                       char **error_message) {
    RxGraphByteReader index_reader;
    unsigned char magic[4];
    uint32_t version;
    uint32_t counts[8];
    uint32_t fact_counts[8];
    unsigned char *combined;
    size_t payload_size;
    size_t combined_size;
    RxGraph *graph;
    size_t i;

    if (error_message) *error_message = 0;
    if (!facts || facts_size < 48u || !indexes || indexes_size < 40u) {
        rx_graph_set_error(error_message,
                           "semantic graph facts or index section is truncated");
        return 0;
    }
    if (memcmp(facts, "RXG7", 4u) != 0) {
        rx_graph_set_error(error_message, "semantic graph facts have an invalid magic value");
        return 0;
    }
    index_reader.cursor = indexes;
    index_reader.end = indexes + indexes_size;
    if (!rx_graph_reader_bytes(&index_reader, magic, sizeof(magic)) ||
        memcmp(magic, "RXI7", sizeof(magic)) != 0 ||
        !rx_graph_reader_u32(&index_reader, &version)) {
        rx_graph_set_error(error_message, "semantic graph index has an invalid header");
        return 0;
    }
    if (version != RX_GRAPH_SERIAL_VERSION) {
        rx_graph_set_error(error_message, "semantic graph index version is not supported");
        return 0;
    }
    for (i = 0u; i < 8u; i++) {
        if (!rx_graph_reader_u32(&index_reader, &counts[i])) {
            rx_graph_set_error(error_message, "semantic graph index header is truncated");
            return 0;
        }
    }
    /* Index count order: types, members, callables, edges, declarations,
       dispatches, factories, providers. */
    fact_counts[0] = (uint32_t)facts[12] |
                     ((uint32_t)facts[13] << 8u) |
                     ((uint32_t)facts[14] << 16u) |
                     ((uint32_t)facts[15] << 24u);
    fact_counts[1] = (uint32_t)facts[16] |
                     ((uint32_t)facts[17] << 8u) |
                     ((uint32_t)facts[18] << 16u) |
                     ((uint32_t)facts[19] << 24u);
    fact_counts[2] = (uint32_t)facts[32] |
                     ((uint32_t)facts[33] << 8u) |
                     ((uint32_t)facts[34] << 16u) |
                     ((uint32_t)facts[35] << 24u);
    fact_counts[3] = (uint32_t)facts[24] |
                     ((uint32_t)facts[25] << 8u) |
                     ((uint32_t)facts[26] << 16u) |
                     ((uint32_t)facts[27] << 24u);
    fact_counts[4] = (uint32_t)facts[28] |
                     ((uint32_t)facts[29] << 8u) |
                     ((uint32_t)facts[30] << 16u) |
                     ((uint32_t)facts[31] << 24u);
    fact_counts[5] = (uint32_t)facts[36] |
                     ((uint32_t)facts[37] << 8u) |
                     ((uint32_t)facts[38] << 16u) |
                     ((uint32_t)facts[39] << 24u);
    fact_counts[6] = (uint32_t)facts[40] |
                     ((uint32_t)facts[41] << 8u) |
                     ((uint32_t)facts[42] << 16u) |
                     ((uint32_t)facts[43] << 24u);
    fact_counts[7] = (uint32_t)facts[44] |
                     ((uint32_t)facts[45] << 8u) |
                     ((uint32_t)facts[46] << 16u) |
                     ((uint32_t)facts[47] << 24u);
    if (counts[0] != fact_counts[0] || counts[1] != fact_counts[1] ||
        counts[2] != fact_counts[2] || counts[3] != fact_counts[3] ||
        counts[4] != fact_counts[4] || counts[5] != fact_counts[5] ||
        counts[6] != fact_counts[6] || counts[7] != fact_counts[7]) {
        rx_graph_set_error(error_message,
                           "semantic graph facts and index counts do not match");
        return 0;
    }
    payload_size = (size_t)(index_reader.end - index_reader.cursor);
    if (facts_size > SIZE_MAX - payload_size) {
        rx_graph_set_error(error_message, "semantic graph section sizes overflow");
        return 0;
    }
    combined_size = facts_size + payload_size;
    combined = (unsigned char *)malloc(combined_size);
    if (!combined) {
        rx_graph_set_error(error_message, "out of memory joining semantic graph sections");
        return 0;
    }
    memcpy(combined, facts, facts_size);
    memcpy(combined + facts_size, index_reader.cursor, payload_size);
    graph = rx_graph_deserialize(combined, combined_size, error_message);
    free(combined);
    return graph;
}

static int rx_graph_string_valid(const RxGraph *graph,
                                 uint32_t offset,
                                 uint32_t expected_length) {
    const char *text;
    const void *nul;
    size_t available;

    if (!graph || offset >= graph->string_size) return 0;
    text = graph->strings + offset;
    available = graph->string_size - offset;
    nul = memchr(text, 0, available);
    if (!nul) return 0;
    if (expected_length != RX_GRAPH_NONE &&
        (size_t)((const char *)nul - text) != expected_length) return 0;
    return 1;
}

static int rx_graph_relation_acyclic(const RxGraph *graph,
                                     RxGraphRelation relation) {
    uint32_t *indegree;
    RxGraphId *queue;
    uint32_t head;
    uint32_t tail;
    uint32_t visited;
    uint32_t i;
    int result;

    if (!graph || !graph->type_count) return 1;
    indegree = (uint32_t *)calloc(graph->type_count, sizeof(*indegree));
    queue = (RxGraphId *)malloc((size_t)graph->type_count * sizeof(*queue));
    if (!indegree || !queue) {
        free(indegree);
        free(queue);
        return 0;
    }
    for (i = 0u; i < graph->edge_count; i++) {
        if (graph->edges[i].relation == (uint32_t)relation) {
            if (indegree[graph->edges[i].to] == UINT32_MAX) {
                free(indegree);
                free(queue);
                return 0;
            }
            indegree[graph->edges[i].to]++;
        }
    }
    head = 0u;
    tail = 0u;
    for (i = 0u; i < graph->type_count; i++) {
        if (!indegree[i]) queue[tail++] = i;
    }
    visited = 0u;
    while (head < tail) {
        uint32_t first;
        uint32_t count;
        uint32_t edge_index;
        RxGraphId current;

        current = queue[head++];
        visited++;
        rx_graph_edge_range(graph, current, relation, 0, &first, &count);
        for (edge_index = 0u; edge_index < count; edge_index++) {
            RxGraphId next;
            next = graph->outgoing_index[first + edge_index].other;
            if (--indegree[next] == 0u) queue[tail++] = next;
        }
    }
    result = visited == graph->type_count;
    free(indegree);
    free(queue);
    return result;
}

int rx_graph_validate(const RxGraph *graph, char **error_message) {
    uint32_t i;

    if (error_message) *error_message = 0;
    if (!graph || !graph->strings || graph->string_size == 0u) {
        rx_graph_set_error(error_message, "semantic graph has no string table");
        return 0;
    }
    for (i = 0u; i < graph->type_count; i++) {
        const RxGraphTypeRecord *record;
        record = &graph->types[i];
        if (!rx_graph_string_valid(graph, record->name_offset, record->name_length) ||
            record->kind > RX_GRAPH_TYPE_EXPRESSION ||
            record->hash != rx_graph_hash(rx_graph_string(graph, record->name_offset))) {
            rx_graph_set_error(error_message, "semantic graph has an invalid type node");
            return 0;
        }
    }
    for (i = 0u; i < graph->member_count; i++) {
        const RxGraphMemberRecord *record;
        record = &graph->members[i];
        if (!rx_graph_string_valid(graph, record->name_offset, RX_GRAPH_NONE) ||
            !rx_graph_string_valid(graph, record->descriptor_offset, RX_GRAPH_NONE) ||
            record->return_type >= graph->type_count ||
            record->parameter_count > graph->parameter_count ||
            record->parameter_first > graph->parameter_count - record->parameter_count ||
            record->hash != rx_graph_hash(rx_graph_string(graph, record->descriptor_offset))) {
            rx_graph_set_error(error_message, "semantic graph has an invalid member node");
            return 0;
        }
    }
    for (i = 0u; i < graph->parameter_count; i++) {
        if (graph->parameters[i].type >= graph->type_count ||
            (graph->parameters[i].flags &
             ~(RX_GRAPH_PARAM_REF | RX_GRAPH_PARAM_OPTIONAL | RX_GRAPH_PARAM_VARARG))) {
            rx_graph_set_error(error_message, "semantic graph has an invalid parameter type reference");
            return 0;
        }
    }
    for (i = 0u; i < graph->edge_count; i++) {
        const RxGraphEdgeRecord *record;
        RxGraphTypeKind from_kind;
        RxGraphTypeKind to_kind;
        record = &graph->edges[i];
        if (record->from >= graph->type_count || record->to >= graph->type_count ||
            record->relation < RX_GRAPH_REL_IMPLEMENTS ||
            record->relation > RX_GRAPH_REL_TYPE_ALIAS) {
            rx_graph_set_error(error_message, "semantic graph has an invalid relationship edge");
            return 0;
        }
        from_kind = (RxGraphTypeKind)graph->types[record->from].kind;
        to_kind = (RxGraphTypeKind)graph->types[record->to].kind;
        if (record->from == record->to ||
            (record->relation == RX_GRAPH_REL_IMPLEMENTS &&
             (from_kind != RX_GRAPH_TYPE_CLASS || to_kind != RX_GRAPH_TYPE_INTERFACE)) ||
            (record->relation == RX_GRAPH_REL_INHERITS_CLASS &&
             (from_kind != RX_GRAPH_TYPE_CLASS || to_kind != RX_GRAPH_TYPE_CLASS)) ||
            (record->relation == RX_GRAPH_REL_EXTENDS_INTERFACE &&
             (from_kind != RX_GRAPH_TYPE_INTERFACE || to_kind != RX_GRAPH_TYPE_INTERFACE))) {
            rx_graph_set_error(error_message, "semantic graph relationship kinds are inconsistent");
            return 0;
        }
    }
    for (i = 0u; i < graph->declaration_count; i++) {
        if (graph->declarations[i].owner >= graph->type_count ||
            graph->declarations[i].member >= graph->member_count ||
            (graph->types[graph->declarations[i].owner].kind != RX_GRAPH_TYPE_CLASS &&
             graph->types[graph->declarations[i].owner].kind != RX_GRAPH_TYPE_INTERFACE) ||
            (graph->declarations[i].flags &
             ~(RX_GRAPH_MEMBER_METHOD | RX_GRAPH_MEMBER_FINAL | RX_GRAPH_MEMBER_FACTORY)) ||
            !(graph->declarations[i].flags &
              (RX_GRAPH_MEMBER_METHOD | RX_GRAPH_MEMBER_FACTORY))) {
            rx_graph_set_error(error_message, "semantic graph has an invalid member declaration");
            return 0;
        }
    }
    for (i = 0u; i < graph->callable_count; i++) {
        const RxGraphCallableRecord *record;
        record = &graph->callables[i];
        if (!rx_graph_string_valid(graph, record->symbol_offset, RX_GRAPH_NONE) ||
            !rx_graph_string_valid(graph, record->descriptor_offset, RX_GRAPH_NONE) ||
            (record->owner_type != RX_GRAPH_NONE && record->owner_type >= graph->type_count) ||
            (record->member != RX_GRAPH_NONE && record->member >= graph->member_count) ||
            (record->flags & ~RX_GRAPH_CALLABLE_IMPORTED) ||
            record->hash != rx_graph_hash(rx_graph_string(graph, record->symbol_offset))) {
            rx_graph_set_error(error_message, "semantic graph has an invalid callable node");
            return 0;
        }
    }
    for (i = 0u; i < graph->dispatch_count; i++) {
        if (graph->dispatches[i].owner >= graph->type_count ||
            graph->dispatches[i].member >= graph->member_count ||
            graph->dispatches[i].callable >= graph->callable_count) {
            rx_graph_set_error(error_message, "semantic graph has an invalid dispatch row");
            return 0;
        }
    }
    for (i = 0u; i < graph->factory_count; i++) {
        const RxGraphFactoryRecord *record;
        record = &graph->factories[i];
        if (record->interface_type >= graph->type_count ||
            record->member >= graph->member_count ||
            (graph->types[record->interface_type].kind != RX_GRAPH_TYPE_INTERFACE &&
             graph->types[record->interface_type].kind != RX_GRAPH_TYPE_OPAQUE) ||
            !(record->flags & RX_GRAPH_MEMBER_FACTORY) ||
            !(graph->members[record->member].flags & RX_GRAPH_MEMBER_FACTORY) ||
            record->reserved != 0u) {
            rx_graph_set_error(error_message, "semantic graph has an invalid factory bucket");
            return 0;
        }
    }
    for (i = 0u; i < graph->provider_count; i++) {
        const RxGraphProviderRecord *record;
        uint32_t factory_index;
        int factory_found;
        record = &graph->providers[i];
        if (record->interface_type >= graph->type_count ||
            record->factory_member >= graph->member_count ||
            record->class_type >= graph->type_count ||
            record->factory_callable >= graph->callable_count ||
            (record->match_callable != RX_GRAPH_NONE &&
             record->match_callable >= graph->callable_count) ||
            graph->types[record->interface_type].kind != RX_GRAPH_TYPE_INTERFACE ||
            graph->types[record->class_type].kind != RX_GRAPH_TYPE_CLASS ||
            !(graph->members[record->factory_member].flags & RX_GRAPH_MEMBER_FACTORY)) {
            rx_graph_set_error(error_message, "semantic graph has an invalid factory provider");
            return 0;
        }
        factory_found = 0;
        for (factory_index = 0u; factory_index < graph->factory_count; factory_index++) {
            if (graph->factories[factory_index].interface_type == record->interface_type &&
                graph->factories[factory_index].member == record->factory_member) {
                factory_found = 1;
                break;
            }
        }
        if (!factory_found) {
            rx_graph_set_error(error_message, "semantic graph provider has no factory bucket");
            return 0;
        }
    }

    for (i = 0u; i < graph->type_count; i++) {
        if (graph->type_index[i].id >= graph->type_count ||
            graph->type_index[i].hash != graph->types[graph->type_index[i].id].hash ||
            (i && rx_graph_hash_index_compare(&graph->type_index[i - 1u],
                                              &graph->type_index[i]) > 0)) {
            rx_graph_set_error(error_message, "semantic graph has an invalid type index");
            return 0;
        }
        if (i && graph->type_index[i - 1u].hash == graph->type_index[i].hash &&
            strcmp(rx_graph_string(graph,
                                   graph->types[graph->type_index[i - 1u].id].name_offset),
                   rx_graph_string(graph,
                                   graph->types[graph->type_index[i].id].name_offset)) == 0) {
            rx_graph_set_error(error_message, "semantic graph has a duplicate type identity");
            return 0;
        }
    }
    for (i = 0u; i < graph->member_count; i++) {
        if (graph->member_index[i].id >= graph->member_count ||
            graph->member_index[i].hash != graph->members[graph->member_index[i].id].hash ||
            (i && rx_graph_hash_index_compare(&graph->member_index[i - 1u],
                                              &graph->member_index[i]) > 0)) {
            rx_graph_set_error(error_message, "semantic graph has an invalid member index");
            return 0;
        }
        if (i && graph->member_index[i - 1u].hash == graph->member_index[i].hash &&
            strcmp(rx_graph_string(graph,
                                   graph->members[graph->member_index[i - 1u].id].descriptor_offset),
                   rx_graph_string(graph,
                                   graph->members[graph->member_index[i].id].descriptor_offset)) == 0) {
            rx_graph_set_error(error_message, "semantic graph has a duplicate member identity");
            return 0;
        }
    }
    for (i = 0u; i < graph->callable_count; i++) {
        if (graph->callable_index[i].id >= graph->callable_count ||
            graph->callable_index[i].hash != graph->callables[graph->callable_index[i].id].hash ||
            (i && rx_graph_hash_index_compare(&graph->callable_index[i - 1u],
                                              &graph->callable_index[i]) > 0)) {
            rx_graph_set_error(error_message, "semantic graph has an invalid callable index");
            return 0;
        }
        if (i && graph->callable_index[i - 1u].hash == graph->callable_index[i].hash &&
            strcmp(rx_graph_string(graph,
                                   graph->callables[graph->callable_index[i - 1u].id].symbol_offset),
                   rx_graph_string(graph,
                                   graph->callables[graph->callable_index[i].id].symbol_offset)) == 0) {
            rx_graph_set_error(error_message, "semantic graph has a duplicate callable identity");
            return 0;
        }
    }
    for (i = 0u; i < graph->edge_count; i++) {
        const RxGraphEdgeIndex *outgoing;
        const RxGraphEdgeIndex *incoming;
        const RxGraphEdgeRecord *outgoing_edge;
        const RxGraphEdgeRecord *incoming_edge;
        outgoing = &graph->outgoing_index[i];
        incoming = &graph->incoming_index[i];
        if (outgoing->edge >= graph->edge_count ||
            incoming->edge >= graph->edge_count ||
            (i && rx_graph_edge_index_compare(&graph->outgoing_index[i - 1u],
                                              &graph->outgoing_index[i]) > 0) ||
            (i && rx_graph_edge_index_compare(&graph->incoming_index[i - 1u],
                                              &graph->incoming_index[i]) > 0)) {
            rx_graph_set_error(error_message, "semantic graph has an invalid adjacency index");
            return 0;
        }
        outgoing_edge = &graph->edges[outgoing->edge];
        incoming_edge = &graph->edges[incoming->edge];
        if (outgoing->type != outgoing_edge->from ||
            outgoing->relation != outgoing_edge->relation ||
            outgoing->other != outgoing_edge->to ||
            incoming->type != incoming_edge->to ||
            incoming->relation != incoming_edge->relation ||
            incoming->other != incoming_edge->from) {
            rx_graph_set_error(error_message, "semantic graph adjacency index disagrees with its edge");
            return 0;
        }
        if (i && outgoing->type == graph->outgoing_index[i - 1u].type &&
            outgoing->relation == graph->outgoing_index[i - 1u].relation &&
            outgoing->other == graph->outgoing_index[i - 1u].other) {
            rx_graph_set_error(error_message, "semantic graph has a duplicate relationship edge");
            return 0;
        }
    }
    if (!rx_graph_relation_acyclic(graph, RX_GRAPH_REL_INHERITS_CLASS) ||
        !rx_graph_relation_acyclic(graph, RX_GRAPH_REL_EXTENDS_INTERFACE)) {
        rx_graph_set_error(error_message, "semantic graph inheritance contains a cycle");
        return 0;
    }
    for (i = 0u; i < graph->declaration_count; i++) {
        const RxGraphDeclarationIndex *entry;
        const RxGraphDeclarationRecord *record;
        entry = &graph->declaration_index[i];
        if (entry->declaration >= graph->declaration_count) {
            rx_graph_set_error(error_message, "semantic graph has an invalid declaration index ID");
            return 0;
        }
        record = &graph->declarations[entry->declaration];
        if (entry->owner != record->owner || entry->member != record->member ||
            (i && rx_graph_declaration_index_compare(
                      &graph->declaration_index[i - 1u], entry) > 0)) {
            rx_graph_set_error(error_message, "semantic graph has an invalid declaration index");
            return 0;
        }
    }
    for (i = 0u; i < graph->dispatch_count; i++) {
        if (graph->dispatch_index[i].owner >= graph->type_count ||
            graph->dispatch_index[i].member >= graph->member_count ||
            graph->dispatch_index[i].callable >= graph->callable_count ||
            (i && rx_graph_dispatch_index_compare(&graph->dispatch_index[i - 1u],
                                                  &graph->dispatch_index[i]) > 0)) {
            rx_graph_set_error(error_message, "semantic graph has an invalid dispatch index");
            return 0;
        }
    }
    for (i = 0u; i < graph->factory_count; i++) {
        const RxGraphFactoryIndex *entry;
        const RxGraphFactoryRecord *record;
        entry = &graph->factory_index[i];
        if (entry->factory >= graph->factory_count) {
            rx_graph_set_error(error_message, "semantic graph has an invalid factory index ID");
            return 0;
        }
        record = &graph->factories[entry->factory];
        if (entry->interface_type != record->interface_type ||
            entry->member != record->member ||
            (i && rx_graph_factory_index_compare(&graph->factory_index[i - 1u],
                                                 entry) > 0)) {
            rx_graph_set_error(error_message, "semantic graph has an invalid factory index");
            return 0;
        }
    }
    for (i = 0u; i < graph->provider_count; i++) {
        if (graph->provider_index[i].provider >= graph->provider_count ||
            graph->provider_index[i].interface_type !=
                graph->providers[graph->provider_index[i].provider].interface_type ||
            graph->provider_index[i].factory_member !=
                graph->providers[graph->provider_index[i].provider].factory_member ||
            (i && rx_graph_provider_index_less(graph,
                                               &graph->provider_index[i],
                                               &graph->provider_index[i - 1u]))) {
            rx_graph_set_error(error_message, "semantic graph has an invalid provider index");
            return 0;
        }
    }
    return 1;
}

typedef struct RxGraphCrexxMetaRef {
    module_file *module;
    uint32_t module_index;
    uint32_t ordinal;
    size_t offset;
    meta_entry *entry;
} RxGraphCrexxMetaRef;

typedef struct RxGraphCrexxMetaList {
    RxGraphCrexxMetaRef *items;
    size_t count;
    size_t capacity;
} RxGraphCrexxMetaList;

static void rx_graph_crexx_error(char **error_message,
                                 const module_file *module,
                                 size_t offset,
                                 const char *detail) {
    char message[512];
    const char *name;

    name = module && module->name ? module->name : "<unnamed>";
    snprintf(message,
             sizeof(message),
             "semantic graph metadata error in module %s at offset %lu: %s",
             name,
             (unsigned long)offset,
             detail ? detail : "invalid metadata");
    rx_graph_set_error(error_message, message);
}

static size_t rx_graph_crexx_meta_size(enum const_pool_type type) {
    switch (type) {
        case META_FUNC: return sizeof(meta_func_constant);
        case META_REG: return sizeof(meta_reg_constant);
        case META_CONST: return sizeof(meta_const_constant);
        case META_CLEAR: return sizeof(meta_clear_constant);
        case META_CLASS: return sizeof(meta_class_constant);
        case META_ATTR: return sizeof(meta_attr_constant);
        case META_INTERFACE: return sizeof(meta_interface_constant);
        case META_IMPLEMENTS: return sizeof(meta_implements_constant);
        case META_MEMBER: return sizeof(meta_member_constant);
        case META_INLINE: return sizeof(meta_inline_constant);
        case META_SOURCE_STEP: return sizeof(meta_source_step_constant);
        case META_TRACE_EVENT: return sizeof(meta_trace_event_constant);
        case META_TASK_TARGET: return sizeof(meta_task_target_constant);
        default: return 0u;
    }
}

static int rx_graph_crexx_meta_append(RxGraphCrexxMetaList *list,
                                      const RxGraphCrexxMetaRef *reference) {
    RxGraphCrexxMetaRef *items;
    size_t capacity;

    if (list->count == list->capacity) {
        capacity = list->capacity ? list->capacity * 2u : 32u;
        if (capacity < list->count || capacity > SIZE_MAX / sizeof(*items)) return 0;
        items = (RxGraphCrexxMetaRef *)realloc(list->items,
                                               capacity * sizeof(*items));
        if (!items) return 0;
        list->items = items;
        list->capacity = capacity;
    }
    list->items[list->count++] = *reference;
    return 1;
}

static int rx_graph_crexx_collect_module(RxGraphCrexxMetaList *list,
                                         module_file *module,
                                         uint32_t module_index,
                                         char **error_message) {
    unsigned char *visited;
    int offset;
    int previous;
    uint32_t ordinal;

    if (!module) {
        rx_graph_set_error(error_message, "semantic graph builder received a null module");
        return 0;
    }
    if (module->header.meta_head == -1) return 1;
    if (!module->constant || !module->header.constant_size) {
        rx_graph_crexx_error(error_message,
                             module,
                             (size_t)module->header.meta_head,
                             "metadata head has no constant pool");
        return 0;
    }
    visited = (unsigned char *)calloc(module->header.constant_size, 1u);
    if (!visited) {
        rx_graph_set_error(error_message, "out of memory indexing semantic metadata");
        return 0;
    }
    offset = module->header.meta_head;
    previous = -1;
    ordinal = 0u;
    while (offset != -1) {
        size_t unsigned_offset;
        size_t expected_size;
        meta_entry *entry;
        RxGraphCrexxMetaRef reference;

        if (offset < 0) {
            rx_graph_crexx_error(error_message, module, 0u, "negative metadata offset");
            free(visited);
            return 0;
        }
        unsigned_offset = (size_t)offset;
        if (unsigned_offset > module->header.constant_size ||
            module->header.constant_size - unsigned_offset < sizeof(meta_entry)) {
            rx_graph_crexx_error(error_message,
                                 module,
                                 unsigned_offset,
                                 "metadata entry is outside the constant pool");
            free(visited);
            return 0;
        }
        if (visited[unsigned_offset]) {
            rx_graph_crexx_error(error_message,
                                 module,
                                 unsigned_offset,
                                 "metadata chain contains a cycle or duplicate offset");
            free(visited);
            return 0;
        }
        visited[unsigned_offset] = 1u;
        entry = (meta_entry *)((unsigned char *)module->constant + unsigned_offset);
        expected_size = rx_graph_crexx_meta_size(entry->base.type);
        if (!expected_size || entry->base.size_in_pool < expected_size ||
            entry->base.size_in_pool > module->header.constant_size - unsigned_offset) {
            rx_graph_crexx_error(error_message,
                                 module,
                                 unsigned_offset,
                                 "metadata entry has an invalid kind or size");
            free(visited);
            return 0;
        }
        if (entry->prev != previous) {
            rx_graph_crexx_error(error_message,
                                 module,
                                 unsigned_offset,
                                 "metadata previous-link does not match traversal order");
            free(visited);
            return 0;
        }
        reference.module = module;
        reference.module_index = module_index;
        reference.ordinal = ordinal++;
        reference.offset = unsigned_offset;
        reference.entry = entry;
        if (!rx_graph_crexx_meta_append(list, &reference)) {
            rx_graph_set_error(error_message, "out of memory collecting semantic metadata");
            free(visited);
            return 0;
        }
        previous = offset;
        offset = entry->next;
    }
    free(visited);
    return 1;
}

static const char *rx_graph_crexx_string(const module_file *module, size_t offset) {
    const string_constant *constant;
    size_t header_size;
    size_t available;

    header_size = offsetof(string_constant, string);
    if (!module || !module->constant || offset > module->header.constant_size ||
        module->header.constant_size - offset < header_size + 1u) return 0;
    constant = (const string_constant *)((const unsigned char *)module->constant + offset);
    if (constant->base.type != STRING_CONST ||
        constant->base.size_in_pool < header_size + 1u ||
        constant->base.size_in_pool > module->header.constant_size - offset) return 0;
    available = constant->base.size_in_pool - header_size;
    if (!memchr(constant->string, 0, available)) return 0;
    return constant->string;
}

static const proc_constant *rx_graph_crexx_procedure(const module_file *module,
                                                     size_t offset) {
    const proc_constant *procedure;
    size_t header_size;
    size_t available;

    header_size = offsetof(proc_constant, name);
    if (!module || !module->constant || offset > module->header.constant_size ||
        module->header.constant_size - offset < header_size + 1u) return 0;
    procedure = (const proc_constant *)((const unsigned char *)module->constant + offset);
    if (procedure->base.type != PROC_CONST ||
        procedure->base.size_in_pool < header_size + 1u ||
        procedure->base.size_in_pool > module->header.constant_size - offset) return 0;
    available = procedure->base.size_in_pool - header_size;
    return memchr(procedure->name, 0, available) ? procedure : 0;
}

static const expose_proc_constant *rx_graph_crexx_exposed_procedure(
        const module_file *module,
        size_t offset) {
    const expose_proc_constant *exposed;
    size_t header_size;
    size_t available;

    if (offset == SIZE_MAX) return 0;
    header_size = offsetof(expose_proc_constant, index);
    if (!module || !module->constant || offset > module->header.constant_size ||
        module->header.constant_size - offset < header_size + 1u) return 0;
    exposed = (const expose_proc_constant *)((const unsigned char *)module->constant + offset);
    if (exposed->base.type != EXPOSE_PROC_CONST ||
        exposed->base.size_in_pool < header_size + 1u ||
        exposed->base.size_in_pool > module->header.constant_size - offset) return 0;
    available = exposed->base.size_in_pool - header_size;
    return memchr(exposed->index, 0, available) ? exposed : 0;
}

static int rx_graph_crexx_strings5(const RxGraphCrexxMetaRef *reference,
                                   const size_t *offsets,
                                   size_t count,
                                   const char **values,
                                   char **error_message) {
    size_t i;

    for (i = 0u; i < count; i++) {
        values[i] = rx_graph_crexx_string(reference->module, offsets[i]);
        if (!values[i]) {
            rx_graph_crexx_error(error_message,
                                 reference->module,
                                 reference->offset,
                                 "metadata contains an invalid string reference");
            return 0;
        }
    }
    return 1;
}

static uint32_t rx_graph_crexx_member_flags(const char *kind) {
    uint32_t flags;

    flags = 0u;
    if (kind && strncmp(kind, "method", 6u) == 0) {
        flags |= RX_GRAPH_MEMBER_METHOD;
        if (strstr(kind, "final")) flags |= RX_GRAPH_MEMBER_FINAL;
    } else if (kind && strcmp(kind, "factory") == 0) {
        flags |= RX_GRAPH_MEMBER_FACTORY;
    }
    return flags;
}

static RxGraphId rx_graph_crexx_symbol_owner(const RxGraphBuilder *builder,
                                             const char *symbol,
                                             const char **suffix) {
    RxGraphId result;
    size_t result_length;
    uint32_t i;

    result = RX_GRAPH_NONE;
    result_length = 0u;
    if (suffix) *suffix = 0;
    if (!builder || !builder->graph || !symbol) return result;
    for (i = 0u; i < builder->graph->type_count; i++) {
        const RxGraphTypeRecord *record;
        const char *name;
        size_t length;

        record = &builder->graph->types[i];
        if (record->kind != RX_GRAPH_TYPE_CLASS &&
            record->kind != RX_GRAPH_TYPE_INTERFACE) continue;
        name = rx_graph_string(builder->graph, record->name_offset);
        if (!name) continue;
        length = strlen(name);
        if (length > result_length && strncmp(symbol, name, length) == 0 &&
            symbol[length] == '.' && symbol[length + 1u] != 0) {
            result = i;
            result_length = length;
        }
    }
    if (result != RX_GRAPH_NONE && suffix) *suffix = symbol + result_length + 1u;
    return result;
}

static int rx_graph_crexx_add_types_and_edges(RxGraphBuilder *builder,
                                               const RxGraphCrexxMetaList *list,
                                               char **error_message) {
    size_t i;

    for (i = 0u; i < list->count; i++) {
        const RxGraphCrexxMetaRef *reference = &list->items[i];
        if (reference->entry->base.type == META_CLASS ||
            reference->entry->base.type == META_INTERFACE) {
            const meta_class_constant *declaration;
            const char *symbol;
            RxGraphTypeKind kind;

            declaration = (const meta_class_constant *)reference->entry;
            symbol = rx_graph_crexx_string(reference->module, declaration->symbol);
            if (!symbol) {
                rx_graph_crexx_error(error_message,
                                     reference->module,
                                     reference->offset,
                                     "type declaration has an invalid symbol");
                return 0;
            }
            kind = reference->entry->base.type == META_CLASS
                ? RX_GRAPH_TYPE_CLASS : RX_GRAPH_TYPE_INTERFACE;
            if (rx_graph_builder_add_type(builder, symbol, kind, 0u) == RX_GRAPH_NONE) {
                rx_graph_crexx_error(error_message,
                                     reference->module,
                                     reference->offset,
                                     "conflicting or unallocatable type declaration");
                return 0;
            }
        }
    }
    for (i = 0u; i < list->count; i++) {
        const RxGraphCrexxMetaRef *reference = &list->items[i];
        const meta_implements_constant *implementation;
        const char *values[2];
        size_t offsets[2];
        RxGraphId class_type;
        RxGraphId interface_type;

        if (reference->entry->base.type != META_IMPLEMENTS) continue;
        implementation = (const meta_implements_constant *)reference->entry;
        offsets[0] = implementation->symbol;
        offsets[1] = implementation->interface_symbol;
        if (!rx_graph_crexx_strings5(reference, offsets, 2u, values, error_message)) return 0;
        class_type = rx_graph_builder_add_type(builder,
                                               values[0],
                                               RX_GRAPH_TYPE_CLASS,
                                               0u);
        interface_type = rx_graph_builder_add_type(builder,
                                                   values[1],
                                                   RX_GRAPH_TYPE_INTERFACE,
                                                   0u);
        if (class_type == RX_GRAPH_NONE || interface_type == RX_GRAPH_NONE ||
            !rx_graph_builder_add_edge(builder,
                                       class_type,
                                       interface_type,
                                       RX_GRAPH_REL_IMPLEMENTS,
                                       reference->module_index,
                                       reference->ordinal)) {
            rx_graph_crexx_error(error_message,
                                 reference->module,
                                 reference->offset,
                                 "cannot add implements relationship");
            return 0;
        }
    }
    return 1;
}

static int rx_graph_crexx_add_members_and_callables(RxGraphBuilder *builder,
                                                     const RxGraphCrexxMetaList *list,
                                                     char **error_message) {
    size_t i;

    for (i = 0u; i < list->count; i++) {
        const RxGraphCrexxMetaRef *reference = &list->items[i];
        if (reference->entry->base.type == META_MEMBER) {
            const meta_member_constant *metadata;
            const char *values[5];
            size_t offsets[5];
            RxGraphId owner;
            RxMemberId member;
            uint32_t flags;

            metadata = (const meta_member_constant *)reference->entry;
            offsets[0] = metadata->owner;
            offsets[1] = metadata->kind;
            offsets[2] = metadata->member;
            offsets[3] = metadata->type;
            offsets[4] = metadata->args;
            if (!rx_graph_crexx_strings5(reference, offsets, 5u, values, error_message)) return 0;
            flags = rx_graph_crexx_member_flags(values[1]);
            if (!flags) {
                rx_graph_crexx_error(error_message,
                                     reference->module,
                                     reference->offset,
                                     "member declaration has an unknown kind");
                return 0;
            }
            owner = rx_graph_builder_add_type(builder,
                                              values[0],
                                              RX_GRAPH_TYPE_OPAQUE,
                                              0u);
            member = rx_graph_builder_add_member(builder,
                                                 values[2],
                                                 values[3],
                                                 values[4],
                                                 flags);
            if (owner == RX_GRAPH_NONE || member == RX_GRAPH_NONE ||
                !rx_graph_builder_add_declaration(builder,
                                                  owner,
                                                  member,
                                                  flags,
                                                  reference->module_index,
                                                  reference->ordinal)) {
                rx_graph_crexx_error(error_message,
                                     reference->module,
                                     reference->offset,
                                     "cannot add member declaration");
                return 0;
            }
        } else if (reference->entry->base.type == META_FUNC) {
            const meta_func_constant *metadata;
            const proc_constant *procedure_constant;
            const char *values[3];
            size_t offsets[3];
            RxGraphProcRef procedure;
            RxCallableId callable;
            RxCallableId existing_callable;
            RxGraphId owner;
            uint32_t callable_flags;
            const char *suffix;

            metadata = (const meta_func_constant *)reference->entry;
            offsets[0] = metadata->symbol;
            offsets[1] = metadata->type;
            offsets[2] = metadata->args;
            if (!rx_graph_crexx_strings5(reference, offsets, 3u, values, error_message)) return 0;
            procedure_constant = rx_graph_crexx_procedure(reference->module, metadata->func);
            if (!procedure_constant) {
                rx_graph_crexx_error(error_message,
                                     reference->module,
                                     reference->offset,
                                     "callable has an invalid procedure reference");
                return 0;
            }
            procedure.module_index = reference->module_index;
            procedure.procedure_offset = procedure_constant->start;
            callable_flags = 0u;
            if (procedure_constant->exposed != SIZE_MAX) {
                const expose_proc_constant *exposed;
                exposed = rx_graph_crexx_exposed_procedure(
                    reference->module, procedure_constant->exposed);
                if (!exposed) {
                    rx_graph_crexx_error(error_message,
                                         reference->module,
                                         reference->offset,
                                         "callable has an invalid exposure reference");
                    return 0;
                }
                if (exposed->imported) callable_flags |= RX_GRAPH_CALLABLE_IMPORTED;
            }
            existing_callable = rx_graph_builder_find_callable(builder, values[0]);
            callable = rx_graph_builder_add_callable(builder,
                                                     values[0],
                                                     values[1],
                                                     values[2],
                                                     procedure,
                                                     callable_flags);
            if (callable == RX_GRAPH_NONE) {
                char detail[512];
                if (existing_callable != RX_GRAPH_NONE) {
                    const RxGraphCallableRecord *existing_record;
                    existing_record = &builder->graph->callables[existing_callable];
                    snprintf(detail,
                             sizeof(detail),
                             "conflicting callable %s (existing module=%u offset=%llu flags=%u, incoming module=%u offset=%llu flags=%u)",
                             values[0],
                             existing_record->module_index,
                             (unsigned long long)existing_record->procedure_offset,
                             existing_record->flags,
                             procedure.module_index,
                             (unsigned long long)procedure.procedure_offset,
                             callable_flags);
                } else {
                    snprintf(detail,
                             sizeof(detail),
                             "cannot add callable declaration for %s",
                             values[0]);
                }
                rx_graph_crexx_error(error_message,
                                     reference->module,
                                     reference->offset,
                                     detail);
                return 0;
            }
            owner = rx_graph_crexx_symbol_owner(builder, values[0], &suffix);
            if (owner != RX_GRAPH_NONE && suffix &&
                strncmp(suffix, "\xC2\xA7" "factory", 9u) != 0 &&
                strncmp(suffix, "\xC2\xA7" "match", 7u) != 0) {
                RxMemberId member;
                member = rx_graph_builder_add_member(builder,
                                                     suffix,
                                                     values[1],
                                                     values[2],
                                                     RX_GRAPH_MEMBER_METHOD);
                if (member == RX_GRAPH_NONE ||
                    !rx_graph_builder_add_dispatch(builder, owner, member, callable)) {
                    rx_graph_crexx_error(error_message,
                                         reference->module,
                                         reference->offset,
                                         "cannot add callable dispatch row");
                    return 0;
                }
            }
        }
    }
    return 1;
}

static char *rx_graph_crexx_provider_symbol(const char *class_name,
                                            const char *prefix,
                                            const char *member_name) {
    size_t class_length;
    size_t prefix_length;
    size_t member_length;
    size_t size;
    char *symbol;

    if (!class_name || !prefix || !member_name) return 0;
    class_length = strlen(class_name);
    prefix_length = strlen(prefix);
    member_length = strcmp(member_name, "*") == 0 ? 0u : strlen(member_name);
    size = class_length + 1u + prefix_length +
           (member_length ? 1u + member_length : 0u) + 1u;
    symbol = (char *)malloc(size);
    if (!symbol) return 0;
    if (member_length) {
        snprintf(symbol, size, "%s.%s.%s", class_name, prefix, member_name);
    } else {
        snprintf(symbol, size, "%s.%s", class_name, prefix);
    }
    return symbol;
}

static const char *rx_graph_descriptor_args(const char *descriptor);
static RxFactoryId rx_graph_crexx_find_factory_signature(
        const RxGraph *graph,
        RxGraphId interface_type,
        const char *member_name,
        const rx_callable_signature *signature,
        int require_provider);

static const char *rx_graph_crexx_instruction_text(const module_file *module,
                                                   size_t offset) {
    const string_constant *constant;
    size_t header_size;
    size_t available;

    if (!module || !module->constant || offset >= module->header.constant_size ||
        module->header.constant_size - offset < sizeof(chameleon_constant)) return 0;
    constant = (const string_constant *)((const unsigned char *)module->constant + offset);
    header_size = offsetof(string_constant, string);
    if (constant->base.type != STRING_CONST ||
        constant->base.size_in_pool < header_size + 1u ||
        constant->base.size_in_pool > module->header.constant_size - offset) return 0;
    available = constant->base.size_in_pool - header_size;
    if (constant->string_len >= available || constant->string[constant->string_len] != 0) return 0;
    return constant->string;
}

static int rx_graph_crexx_add_instruction_operand(RxGraphBuilder *builder,
                                                  RxGraphOperandKind kind,
                                                  const char *text) {
    rx_callable_signature signature;
    const char *args;

    if (kind == RX_GRAPH_OPERAND_TYPE) {
        return rx_graph_builder_add_type(builder,
                                         text,
                                         RX_GRAPH_TYPE_OPAQUE,
                                         0u) != RX_GRAPH_NONE;
    }
    if (!rx_sig_parse_descriptor(text, &signature)) return 0;
    args = rx_graph_descriptor_args(text);
    if (!args) {
        rx_sig_free(&signature);
        return 0;
    }
    if (kind == RX_GRAPH_OPERAND_MEMBER) {
        int ok;
        ok = rx_graph_builder_add_member(builder,
                                         signature.name,
                                         signature.return_type,
                                         args,
                                         RX_GRAPH_MEMBER_METHOD) != RX_GRAPH_NONE;
        rx_sig_free(&signature);
        return ok;
    }
    {
        const char *separator;
        const char *member_name;
        char *interface_name;
        RxGraphId interface_type;
        RxFactoryId existing_factory;
        RxMemberId member;
        int ok;

        separator = strstr(signature.name, "..");
        member_name = separator ? separator + 2 : "*";
        interface_name = separator
            ? rx_graph_strdup_range(signature.name,
                                    (size_t)(separator - signature.name))
            : rx_graph_strdup(signature.name);
        interface_type = interface_name
            ? rx_graph_builder_add_type(builder,
                                        interface_name,
                                        RX_GRAPH_TYPE_OPAQUE,
                                        0u)
            : RX_GRAPH_NONE;
        existing_factory = interface_type != RX_GRAPH_NONE
            ? rx_graph_crexx_find_factory_signature(builder->graph,
                                                    interface_type,
                                                    member_name,
                                                    &signature,
                                                    0)
            : RX_GRAPH_NONE;
        if (existing_factory != RX_GRAPH_NONE) {
            free(interface_name);
            rx_sig_free(&signature);
            return 1;
        }
        member = rx_graph_builder_add_member(builder,
                                             member_name,
                                             signature.return_type,
                                             args,
                                             RX_GRAPH_MEMBER_FACTORY);
        free(interface_name);
        ok = interface_type != RX_GRAPH_NONE && member != RX_GRAPH_NONE &&
             rx_graph_builder_ensure_factory(builder,
                                              interface_type,
                                              member,
                                              RX_GRAPH_MEMBER_FACTORY);
        rx_sig_free(&signature);
        return ok;
    }
}

static int rx_graph_crexx_add_instruction_operands(RxGraphBuilder *builder,
                                                   module_file *const *modules,
                                                   size_t module_count,
                                                   char **error_message) {
    size_t module_index;

    for (module_index = 0u; module_index < module_count; module_index++) {
        module_file *module;
        const bin_code *instructions;
        size_t code_index;

        module = modules[module_index];
        if (!module) continue;
        instructions = (const bin_code *)module->instructions;
        code_index = 0u;
        while (code_index < module->header.instruction_size) {
            int opcode;
            size_t operand_count;
            size_t operand_index;

            opcode = instructions[code_index].instruction.opcode;
            if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS) goto invalid_instruction;
            operand_count = rxop_format_operand_count(rxbin_opcode_format(opcode));
            if (operand_count > INT_MAX ||
                instructions[code_index].instruction.no_ops != operand_count ||
                code_index + operand_count >= module->header.instruction_size) {
                goto invalid_instruction;
            }
            for (operand_index = 0; operand_index < operand_count; operand_index++) {
                RxGraphOperandKind kind;
                const char *text;
                char *owned_text;

                kind = rx_graph_operand_kind(opcode, (unsigned int)operand_index);
                if (kind == RX_GRAPH_OPERAND_NONE) continue;
                owned_text = 0;
                if (module->graph_operands) {
                    owned_text = rx_graph_operand_text(
                        module->semantic_graph,
                        opcode,
                        (unsigned int)operand_index,
                        (uint32_t)instructions[code_index +
                                              (size_t)operand_index + 1u].index);
                    text = owned_text;
                } else {
                    text = rx_graph_crexx_instruction_text(
                        module,
                        instructions[code_index + (size_t)operand_index + 1u].index);
                }
                if (!text || !rx_graph_crexx_add_instruction_operand(builder, kind, text)) {
                    free(owned_text);
                    goto invalid_instruction;
                }
                free(owned_text);
            }
            code_index += operand_count + 1u;
        }
    }
    return 1;

invalid_instruction:
    rx_graph_crexx_error(error_message,
                         modules[module_index],
                         0u,
                         "invalid graph-bearing instruction operand");
    return 0;
}

static int rx_graph_crexx_type_spelling_matches(const RxGraph *graph,
                                                const char *type_name,
                                                RxGraphId type) {
    const char *canonical;
    const char *canonical_leaf;
    const char *spelling_leaf;
    char *normalized;
    int matches;

    if (!graph || !type_name || type >= graph->type_count) return 0;
    canonical = rx_graph_string(graph, graph->types[type].name_offset);
    normalized = rx_graph_normalize_type_name(type_name);
    if (!canonical || !normalized) {
        free(normalized);
        return 0;
    }
    matches = strcmp(canonical, normalized) == 0;
    if (!matches) {
        canonical_leaf = strrchr(canonical, '.');
        canonical_leaf = canonical_leaf ? canonical_leaf + 1u : canonical;
        spelling_leaf = strrchr(normalized, '.');
        spelling_leaf = spelling_leaf ? spelling_leaf + 1u : normalized;
        if (*spelling_leaf == '.') spelling_leaf++;
        matches = *canonical_leaf && strcmp(canonical_leaf, spelling_leaf) == 0;
    }
    free(normalized);
    return matches;
}

static RxFactoryId rx_graph_crexx_find_factory_signature(
        const RxGraph *graph,
        RxGraphId interface_type,
        const char *member_name,
        const rx_callable_signature *signature,
        int require_provider) {
    RxFactoryId factory;

    if (!graph || interface_type >= graph->type_count || !member_name ||
        !signature) return RX_GRAPH_NONE;
    for (factory = 0u; factory < graph->factory_count; factory++) {
        const RxGraphFactoryRecord *factory_record;
        const RxGraphMemberRecord *member_record;
        const char *candidate_name;
        const char *candidate_descriptor;
        rx_callable_signature candidate;
        int matches;

        factory_record = &graph->factories[factory];
        if (factory_record->interface_type != interface_type ||
            factory_record->member >= graph->member_count) continue;
        member_record = &graph->members[factory_record->member];
        candidate_name = rx_graph_string(graph, member_record->name_offset);
        candidate_descriptor = rx_graph_string(graph,
                                               member_record->descriptor_offset);
        if (!candidate_name || strcmp(candidate_name, member_name) != 0 ||
            !candidate_descriptor ||
            !rx_sig_parse_descriptor(candidate_descriptor, &candidate)) continue;
        matches = rx_sig_args_match(&candidate, signature) &&
                  (strcmp(candidate.return_type, signature->return_type) == 0 ||
                   rx_graph_crexx_type_spelling_matches(graph,
                                                        signature->return_type,
                                                        interface_type) ||
                   rx_graph_crexx_type_spelling_matches(graph,
                                                        signature->return_type,
                                                        member_record->return_type));
        rx_sig_free(&candidate);
        if (!matches) continue;
        if (require_provider &&
            rx_graph_provider_bucket_size(graph,
                                          interface_type,
                                          factory_record->member) == 0u) continue;
        return factory;
    }
    return RX_GRAPH_NONE;
}

static int rx_graph_crexx_factory_signature_matches(const RxGraph *graph,
                                                     RxGraphId interface_type,
                                                     RxGraphId class_type,
                                                     RxMemberId member,
                                                     RxCallableId callable) {
    rx_callable_signature expected;
    rx_callable_signature actual;
    const char *expected_descriptor;
    const char *actual_descriptor;
    int matches;

    if (!graph || member >= graph->member_count || callable >= graph->callable_count) return 0;
    expected_descriptor = rx_graph_string(graph, graph->members[member].descriptor_offset);
    actual_descriptor = rx_graph_string(graph, graph->callables[callable].descriptor_offset);
    if (!rx_sig_parse_descriptor(expected_descriptor, &expected)) return 0;
    if (!rx_sig_parse_descriptor(actual_descriptor, &actual)) {
        rx_sig_free(&expected);
        return 0;
    }
    matches = rx_sig_args_match(&expected, &actual) &&
              (strcmp(expected.return_type, actual.return_type) == 0 ||
               rx_graph_crexx_type_spelling_matches(graph,
                                                     actual.return_type,
                                                     interface_type) ||
               rx_graph_crexx_type_spelling_matches(graph,
                                                     actual.return_type,
                                                     class_type));
    rx_sig_free(&expected);
    rx_sig_free(&actual);
    return matches;
}

static int rx_graph_crexx_match_signature_matches(const RxGraph *graph,
                                                   RxMemberId member,
                                                   RxCallableId callable) {
    rx_callable_signature expected;
    rx_callable_signature actual;
    const char *expected_descriptor;
    const char *actual_descriptor;
    int matches;

    if (!graph || member >= graph->member_count || callable >= graph->callable_count) return 0;
    expected_descriptor = rx_graph_string(graph, graph->members[member].descriptor_offset);
    actual_descriptor = rx_graph_string(graph, graph->callables[callable].descriptor_offset);
    if (!rx_sig_parse_descriptor(expected_descriptor, &expected)) return 0;
    if (!rx_sig_parse_descriptor(actual_descriptor, &actual)) {
        rx_sig_free(&expected);
        return 0;
    }
    matches = rx_sig_args_match(&expected, &actual) &&
              strcmp(actual.return_type, ".int") == 0;
    rx_sig_free(&expected);
    rx_sig_free(&actual);
    return matches;
}

static int rx_graph_crexx_complete_views(RxGraphBuilder *builder,
                                         char **error_message) {
    RxGraph *graph;
    uint32_t edge_index;

    graph = builder->graph;
    for (edge_index = 0u; edge_index < graph->edge_count; edge_index++) {
        const RxGraphEdgeRecord *edge;
        uint32_t declaration_index;

        edge = &graph->edges[edge_index];
        if (edge->relation != RX_GRAPH_REL_IMPLEMENTS) continue;
        for (declaration_index = 0u;
             declaration_index < graph->declaration_count;
             declaration_index++) {
            const RxGraphDeclarationRecord *declaration;
            RxCallableId interface_callable;

            declaration = &graph->declarations[declaration_index];
            if (declaration->owner != edge->to) continue;
            if (declaration->flags & RX_GRAPH_MEMBER_METHOD) {
                uint32_t dispatch_index;
                int class_has_dispatch;

                interface_callable = RX_GRAPH_NONE;
                class_has_dispatch = 0;
                for (dispatch_index = 0u;
                     dispatch_index < graph->dispatch_count;
                     dispatch_index++) {
                    const RxGraphDispatchRecord *dispatch;
                    dispatch = &graph->dispatches[dispatch_index];
                    if (dispatch->owner == edge->from &&
                        dispatch->member == declaration->member) {
                        class_has_dispatch = 1;
                    }
                    if (dispatch->owner == edge->to &&
                        dispatch->member == declaration->member) {
                        interface_callable = dispatch->callable;
                    }
                }
                if (!class_has_dispatch &&
                    interface_callable != RX_GRAPH_NONE &&
                    (declaration->flags & RX_GRAPH_MEMBER_FINAL) &&
                    !rx_graph_builder_add_dispatch(builder,
                                                   edge->from,
                                                   declaration->member,
                                                   interface_callable)) {
                    rx_graph_set_error(error_message,
                                       "cannot materialize interface default dispatch");
                    return 0;
                }
            } else if (declaration->flags & RX_GRAPH_MEMBER_FACTORY) {
                const char *class_name;
                const char *member_name;
                char *factory_symbol;
                char *match_symbol;
                RxCallableId factory_callable;
                RxCallableId match_callable;

                class_name = rx_graph_string(graph, graph->types[edge->from].name_offset);
                member_name = rx_graph_string(graph,
                                              graph->members[declaration->member].name_offset);
                factory_symbol = rx_graph_crexx_provider_symbol(class_name,
                                                                "\xC2\xA7" "factory",
                                                                member_name);
                match_symbol = rx_graph_crexx_provider_symbol(class_name,
                                                              "\xC2\xA7" "match",
                                                              member_name);
                if (!factory_symbol || !match_symbol) {
                    free(factory_symbol);
                    free(match_symbol);
                    rx_graph_set_error(error_message,
                                       "out of memory materializing factory provider");
                    return 0;
                }
                factory_callable = rx_graph_builder_find_callable(builder, factory_symbol);
                match_callable = rx_graph_builder_find_callable(builder, match_symbol);
                free(factory_symbol);
                free(match_symbol);
                if (factory_callable != RX_GRAPH_NONE &&
                    rx_graph_crexx_factory_signature_matches(graph,
                                                             edge->to,
                                                             edge->from,
                                                             declaration->member,
                                                             factory_callable) &&
                    (match_callable == RX_GRAPH_NONE ||
                     rx_graph_crexx_match_signature_matches(graph,
                                                            declaration->member,
                                                            match_callable)) &&
                    rx_graph_builder_add_provider(builder,
                                                  edge->to,
                                                  declaration->member,
                                                  edge->from,
                                                  factory_callable,
                                                  match_callable,
                                                  edge->origin_module,
                                                  edge->ordinal) == RX_GRAPH_NONE) {
                    rx_graph_set_error(error_message,
                                       "cannot materialize factory provider index");
                    return 0;
                }
            }
        }
    }
    return 1;
}

RxGraph *rx_graph_build_crexx(struct module_file *const *modules,
                              size_t module_count,
                              char **error_message) {
    RxGraphCrexxMetaList metadata;
    RxGraphBuilder *builder;
    RxGraph *graph;
    size_t i;

    if (error_message) *error_message = 0;
    if (!modules && module_count) {
        rx_graph_set_error(error_message, "semantic graph builder received no module array");
        return 0;
    }
    if (module_count > UINT32_MAX) {
        rx_graph_set_error(error_message, "semantic graph module count exceeds 32-bit identity space");
        return 0;
    }
    memset(&metadata, 0, sizeof(metadata));
    for (i = 0u; i < module_count; i++) {
        if (!rx_graph_crexx_collect_module(&metadata,
                                           modules[i],
                                           (uint32_t)i,
                                           error_message)) {
            free(metadata.items);
            return 0;
        }
    }
    builder = rx_graph_builder_create();
    if (!builder) {
        free(metadata.items);
        rx_graph_set_error(error_message, "out of memory creating semantic graph builder");
        return 0;
    }
    if (!rx_graph_crexx_add_types_and_edges(builder, &metadata, error_message) ||
        !rx_graph_crexx_add_members_and_callables(builder, &metadata, error_message) ||
        !rx_graph_crexx_add_instruction_operands(builder,
                                                 modules,
                                                 module_count,
                                                 error_message) ||
        !rx_graph_crexx_complete_views(builder, error_message)) {
        free(metadata.items);
        rx_graph_builder_free(builder);
        return 0;
    }
    free(metadata.items);
    graph = rx_graph_builder_finish(builder);
    if (!graph) {
        rx_graph_set_error(error_message, "cannot finalize semantic graph indexes");
        return 0;
    }
    if (!rx_graph_validate(graph, error_message)) {
        rx_graph_release(&graph);
        return 0;
    }
    return graph;
}

static const char *rx_graph_descriptor_args(const char *descriptor) {
    const char *first;
    const char *second;

    if (!descriptor || strncmp(descriptor,
                               RXSIG_DESCRIPTOR_PREFIX,
                               strlen(RXSIG_DESCRIPTOR_PREFIX)) != 0) return 0;
    first = strchr(descriptor + strlen(RXSIG_DESCRIPTOR_PREFIX), '|');
    if (!first) return 0;
    second = strchr(first + 1, '|');
    return second ? second + 1 : 0;
}

int rx_graph_resolve_operand(const RxGraph *graph,
                             int opcode,
                             unsigned int operand_index,
                             const char *text,
                             uint32_t *id,
                             char **error_message) {
    RxGraphOperandKind kind;

    if (error_message) *error_message = 0;
    if (id) *id = RX_GRAPH_NONE;
    kind = rx_graph_operand_kind(opcode, operand_index);
    if (!graph || !text || !id || kind == RX_GRAPH_OPERAND_NONE) {
        rx_graph_set_error(error_message, "instruction operand is not graph-bearing");
        return 0;
    }
    if (kind == RX_GRAPH_OPERAND_TYPE) {
        *id = rx_graph_find_type(graph, text);
    } else if (kind == RX_GRAPH_OPERAND_MEMBER) {
        *id = rx_graph_find_member(graph, text);
    } else {
        rx_callable_signature signature;
        const char *separator;
        char *interface_name;
        const char *member_name;
        RxGraphId interface_type;

        if (!rx_sig_parse_descriptor(text, &signature)) {
            rx_graph_set_error(error_message, "factory operand has an invalid descriptor");
            return 0;
        }
        separator = strstr(signature.name, "..");
        member_name = separator ? separator + 2 : "*";
        if (!*signature.name || (separator && !*member_name)) {
            rx_sig_free(&signature);
            rx_graph_set_error(error_message, "factory operand has an invalid selector name");
            return 0;
        }
        interface_name = separator
            ? rx_graph_strdup_range(signature.name,
                                    (size_t)(separator - signature.name))
            : rx_graph_strdup(signature.name);
        interface_type = interface_name
            ? rx_graph_find_type(graph, interface_name) : RX_GRAPH_NONE;
        if (interface_type != RX_GRAPH_NONE) {
            *id = rx_graph_crexx_find_factory_signature(graph,
                                                        interface_type,
                                                        member_name,
                                                        &signature,
                                                        1);
            if (*id == RX_GRAPH_NONE) {
                *id = rx_graph_crexx_find_factory_signature(graph,
                                                            interface_type,
                                                            member_name,
                                                            &signature,
                                                            0);
            }
        }
        free(interface_name);
        rx_sig_free(&signature);
    }
    if (*id == RX_GRAPH_NONE) {
        rx_graph_set_error(error_message,
                           kind == RX_GRAPH_OPERAND_TYPE
                               ? "type operand does not resolve to a graph node"
                               : kind == RX_GRAPH_OPERAND_MEMBER
                                   ? "member operand does not resolve to a graph node"
                                   : "factory operand does not resolve to a graph bucket");
        return 0;
    }
    return 1;
}

char *rx_graph_operand_text(const RxGraph *graph,
                            int opcode,
                            unsigned int operand_index,
                            uint32_t id) {
    RxGraphOperandKind kind;

    kind = rx_graph_operand_kind(opcode, operand_index);
    if (!graph || kind == RX_GRAPH_OPERAND_NONE) return 0;
    if (kind == RX_GRAPH_OPERAND_TYPE) {
        return id < graph->type_count
            ? rx_graph_strdup(rx_graph_string(graph, graph->types[id].name_offset)) : 0;
    }
    if (kind == RX_GRAPH_OPERAND_MEMBER) {
        return id < graph->member_count
            ? rx_graph_strdup(rx_graph_string(graph,
                                              graph->members[id].descriptor_offset)) : 0;
    }
    if (id < graph->factory_count) {
        const RxGraphFactoryRecord *factory;
        const RxGraphMemberRecord *member;
        const char *interface_name;
        const char *member_name;
        const char *member_descriptor;
        const char *args;
        rx_callable_signature signature;
        size_t name_size;
        char *name;
        char *descriptor;

        factory = &graph->factories[id];
        member = &graph->members[factory->member];
        interface_name = rx_graph_string(graph,
                                         graph->types[factory->interface_type].name_offset);
        member_name = rx_graph_string(graph, member->name_offset);
        member_descriptor = rx_graph_string(graph, member->descriptor_offset);
        if (!interface_name || !member_name || !member_descriptor ||
            !rx_sig_parse_descriptor(member_descriptor, &signature)) return 0;
        args = rx_graph_descriptor_args(member_descriptor);
        name_size = strlen(interface_name) +
                    (strcmp(member_name, "*") == 0 ? 0u : strlen(member_name) + 2u) + 1u;
        name = (char *)malloc(name_size);
        if (!name) {
            rx_sig_free(&signature);
            return 0;
        }
        if (strcmp(member_name, "*") == 0) {
            snprintf(name, name_size, "%s", interface_name);
        } else {
            snprintf(name, name_size, "%s..%s", interface_name, member_name);
        }
        descriptor = rx_sig_build_descriptor(name,
                                             signature.return_type,
                                             args ? args : "");
        free(name);
        rx_sig_free(&signature);
        return descriptor;
    }
    return 0;
}
