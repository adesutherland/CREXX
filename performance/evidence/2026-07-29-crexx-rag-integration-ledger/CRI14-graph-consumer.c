/* Minimal diagnostic consumer for the current internal RXBIN semantic graph. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "rxbin.h"
#include "rxgraph.h"

static const char *type_kind_name(RxGraphTypeKind kind) {
    switch (kind) {
        case RX_GRAPH_TYPE_OPAQUE: return "opaque";
        case RX_GRAPH_TYPE_BUILTIN: return "builtin";
        case RX_GRAPH_TYPE_CLASS: return "class";
        case RX_GRAPH_TYPE_INTERFACE: return "interface";
        case RX_GRAPH_TYPE_EXPRESSION: return "expression";
    }
    return "unknown";
}

static void print_member(const RxGraph *graph, RxMemberId member) {
    RxGraphMemberView view;
    uint32_t parameter;

    if (!rx_graph_member(graph, member, &view)) return;
    printf(" member=%u name=%s return=%s descriptor=%s flags=%u params=%u\n",
           member,
           view.name,
           rx_graph_type_name(graph, view.return_type),
           view.descriptor,
           view.flags,
           view.parameter_count);
    for (parameter = 0; parameter < view.parameter_count; parameter++) {
        RxGraphParamView parameter_view;
        if (!rx_graph_member_parameter(graph, member, parameter, &parameter_view)) continue;
        printf("  param=%u type=%s flags=%u\n",
               parameter,
               rx_graph_type_name(graph, parameter_view.type),
               parameter_view.flags);
    }
}

static void print_graph(const RxGraph *graph) {
    size_t type_count = rx_graph_type_count(graph);
    size_t member_count = rx_graph_member_count(graph);
    size_t callable_count = rx_graph_callable_count(graph);
    RxGraphStorageStats storage;
    RxGraphId type;
    RxMemberId member;
    RxCallableId callable;

    printf("counts types=%zu members=%zu callables=%zu relationships=%zu declarations=%zu\n",
           type_count,
           member_count,
           callable_count,
           rx_graph_relationship_count(graph),
           rx_graph_declaration_total(graph));
    if (rx_graph_storage_stats(graph, &storage)) {
        printf("storage retained_bytes=%zu retained_allocations=%zu facts_bytes=%zu indexes_bytes=%zu strings=%zu unique_strings=%zu\n",
               storage.retained_bytes,
               storage.retained_allocations,
               storage.serialized_facts_bytes,
               storage.serialized_indexes_bytes,
               storage.string_count,
               storage.unique_string_count);
    }

    for (type = 0; type < type_count; type++) {
        size_t declaration_count = rx_graph_declaration_count(graph, type);
        size_t position;
        printf("type=%u name=%s kind=%s declarations=%zu\n",
               type,
               rx_graph_type_name(graph, type),
               type_kind_name(rx_graph_type_kind(graph, type)),
               declaration_count);
        for (position = 0; position < declaration_count; position++) {
            RxGraphDeclarationView declaration;
            if (!rx_graph_declaration(graph, type, position, &declaration)) continue;
            printf(" declaration owner=%u flags=%u origin=%u ordinal=%u\n",
                   declaration.owner,
                   declaration.flags,
                   declaration.origin_module,
                   declaration.ordinal);
            print_member(graph, declaration.member);
        }
        for (member = 0; member < member_count; member++) {
            RxCallableId target = rx_graph_dispatch(graph, type, member);
            if (target != RX_GRAPH_NONE) {
                RxGraphCallableView callable_view;
                if (rx_graph_callable(graph, target, &callable_view)) {
                    printf(" dispatch type=%u member=%u callable=%u symbol=%s\n",
                           type, member, target, callable_view.symbol);
                }
            }
        }
    }

    for (callable = 0; callable < callable_count; callable++) {
        RxGraphCallableView view;
        if (!rx_graph_callable(graph, callable, &view)) continue;
        printf("callable=%u symbol=%s descriptor=%s owner=%u member=%u flags=%u\n",
               callable,
               view.symbol,
               view.descriptor,
               view.owner_type,
               view.member,
               view.flags);
    }
}

int main(int argc, char **argv) {
    FILE *input;
    module_file *module = NULL;
    int read_result;
    char *graph_error = NULL;

    if (argc != 2) {
        fprintf(stderr, "usage: %s CONTRACT.rxbin\n", argv[0]);
        return 2;
    }
    input = fopen(argv[1], "rb");
    if (!input) {
        perror(argv[1]);
        return 2;
    }
    read_result = read_module(&module, input);
    fclose(input);
    if (read_result != 0 || !module) {
        fprintf(stderr, "read failed: %s\n", rxbin_last_error());
        return 1;
    }
    if (!module->semantic_graph ||
        !rx_graph_validate(module->semantic_graph, &graph_error)) {
        fprintf(stderr, "graph validation failed: %s\n",
                graph_error ? graph_error : "missing graph");
        free(graph_error);
        free_module(module);
        return 1;
    }
    print_graph(module->semantic_graph);
    free_module(module);
    return 0;
}
