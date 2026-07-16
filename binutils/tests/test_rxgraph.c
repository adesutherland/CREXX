/* cREXX License (MIT) */

#include "rxgraph.h"
#include "rxbin.h"
#include "rxsignature.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int require(int condition, const char *message) {
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void) {
    RxGraphBuilder *builder;
    RxGraph *graph;
    RxGraph *copy;
    RxGraphId shape;
    RxGraphId box;
    RxGraphId asset;
    RxGraphId base_box;
    RxGraphId derived_box;
    RxMemberId describe;
    RxMemberId factory_member;
    RxCallableId callable;
    RxCallableId factory_callable;
    RxFactoryId factory;
    RxGraphProcRef procedure;
    RxGraphCallableView callable_view;
    RxGraphDeclarationView declaration_view;
    const RxGraphTypeRef *box_type_ref;
    const RxGraphTypeRef *shape_type_ref;
    const RxGraphTypeRef *asset_type_ref;
    unsigned char *bytes;
    unsigned char *corrupt;
    unsigned char *facts;
    unsigned char *indexes;
    size_t byte_count;
    size_t facts_size;
    size_t indexes_size;
    char *descriptor;
    char *factory_text;
    char *error;
    int ok;

    ok = 1;
    bytes = 0;
    corrupt = 0;
    facts = 0;
    indexes = 0;
    error = 0;
    builder = rx_graph_builder_create();
    ok &= require(builder != 0, "create graph builder");
    if (!builder) return 1;

    shape = rx_graph_builder_add_type(builder,
                                      "graph_test.shape",
                                      RX_GRAPH_TYPE_INTERFACE,
                                      0u);
    box = rx_graph_builder_add_type(builder,
                                    "graph_test.box",
                                    RX_GRAPH_TYPE_CLASS,
                                    0u);
    asset = rx_graph_builder_add_type(builder,
                                      "graph_test.asset",
                                      RX_GRAPH_TYPE_INTERFACE,
                                      0u);
    base_box = rx_graph_builder_add_type(builder,
                                         "graph_test.base_box",
                                         RX_GRAPH_TYPE_CLASS,
                                         0u);
    derived_box = rx_graph_builder_add_type(builder,
                                            "graph_test.derived_box",
                                            RX_GRAPH_TYPE_CLASS,
                                            0u);
    describe = rx_graph_builder_add_member(builder,
                                           "describe",
                                           ".string",
                                           "",
                                           RX_GRAPH_MEMBER_METHOD);
    factory_member = rx_graph_builder_add_member(builder,
                                                 "*",
                                                 ".graph_test..shape",
                                                 "",
                                                 RX_GRAPH_MEMBER_FACTORY);
    procedure.module_index = 3u;
    procedure.procedure_offset = 1234u;
    callable = rx_graph_builder_add_callable(builder,
                                             "graph_test.box.describe",
                                             ".string",
                                             "",
                                             procedure,
                                             0u);
    procedure.procedure_offset = 1300u;
    factory_callable = rx_graph_builder_add_callable(builder,
                                                     "graph_test.box.\xC2\xA7" "factory",
                                                     ".shape",
                                                     "",
                                                     procedure,
                                                     0u);
    ok &= require(shape != RX_GRAPH_NONE && box != RX_GRAPH_NONE,
                  "add graph types");
    ok &= require(describe != RX_GRAPH_NONE && callable != RX_GRAPH_NONE &&
                  factory_member != RX_GRAPH_NONE && factory_callable != RX_GRAPH_NONE,
                  "add graph member and callable");
    ok &= require(rx_graph_builder_add_edge(builder,
                                            box,
                                            shape,
                                            RX_GRAPH_REL_IMPLEMENTS,
                                            3u,
                                            7u),
                  "add implements edge");
    ok &= require(rx_graph_builder_add_edge(builder,
                                            shape,
                                            asset,
                                            RX_GRAPH_REL_EXTENDS_INTERFACE,
                                            3u,
                                            8u) &&
                  rx_graph_builder_add_edge(builder,
                                            base_box,
                                            shape,
                                            RX_GRAPH_REL_IMPLEMENTS,
                                            3u,
                                            9u) &&
                  rx_graph_builder_add_edge(builder,
                                            derived_box,
                                            base_box,
                                            RX_GRAPH_REL_INHERITS_CLASS,
                                            3u,
                                            10u),
                  "add inheritance hierarchy");
    ok &= require(rx_graph_builder_add_declaration(builder,
                                                   shape,
                                                   describe,
                                                   RX_GRAPH_MEMBER_METHOD,
                                                   3u,
                                                   8u),
                  "add interface declaration");
    ok &= require(rx_graph_builder_add_declaration(builder,
                                                   shape,
                                                   factory_member,
                                                   RX_GRAPH_MEMBER_FACTORY,
                                                   3u,
                                                   9u),
                  "add factory declaration");
    ok &= require(rx_graph_builder_add_dispatch(builder, box, describe, callable),
                  "add dispatch row");
    ok &= require(rx_graph_builder_add_provider(builder,
                                                shape,
                                                factory_member,
                                                box,
                                                factory_callable,
                                                RX_GRAPH_NONE,
                                                3u,
                                                10u) != RX_GRAPH_NONE,
                  "add factory provider");
    graph = rx_graph_builder_finish(builder);
    ok &= require(graph != 0, "finish graph");
    if (!graph) return 1;

    descriptor = rx_sig_build_descriptor("describe", ".string", "");
    ok &= require(descriptor != 0, "build member descriptor");
    ok &= require(rx_graph_type_supports(graph, box, shape),
                  "walk implements relationship");
    ok &= require(rx_graph_find_type(graph, ".graph_test..shape") == shape,
                  "link source-qualified type spelling to declared node");
    ok &= require(rx_graph_type_supports(graph, derived_box, asset),
                  "walk class and interface inheritance transitively");
    box_type_ref = rx_graph_type_ref(graph, box);
    shape_type_ref = rx_graph_type_ref(graph, shape);
    asset_type_ref = rx_graph_type_ref(graph, asset);
    ok &= require(box_type_ref && shape_type_ref && asset_type_ref &&
                  box_type_ref->graph == graph && box_type_ref->id == box &&
                  strcmp(box_type_ref->name, "graph_test.box") == 0 &&
                  rx_graph_type_ref_supports(box_type_ref, shape_type_ref) &&
                  rx_graph_type_ref_supports(box_type_ref, asset_type_ref),
                  "materialize stable type descriptors and direct support view");
    ok &= require(descriptor && rx_graph_find_member(graph, descriptor) == describe,
                  "find member by descriptor");
    ok &= require(rx_graph_declaration_count(graph, shape) == 2u &&
                  rx_graph_declaration(graph, shape, 0u, &declaration_view) &&
                  declaration_view.owner == shape,
                  "enumerate declarations by owner index");
    ok &= require(rx_graph_dispatch(graph, box, describe) == callable,
                  "resolve dispatch");
    ok &= require(rx_graph_type_ref_dispatch(box_type_ref, describe) == callable,
                  "resolve dispatch through stable type descriptor");
    ok &= require(rx_graph_callable(graph, callable, &callable_view) &&
                  callable_view.procedure.module_index == 3u &&
                  callable_view.procedure.procedure_offset == 1234u,
                  "read callable procedure reference");
    factory = rx_graph_find_factory(graph, shape, factory_member);
    ok &= require(factory != RX_GRAPH_NONE && rx_graph_factory_count(graph) == 1u,
                  "find factory bucket");
    factory_text = rx_graph_operand_text(graph,
                                         OP_SRCFPROCSEL_REG_STRING_REG,
                                         1u,
                                         factory);
    ok &= require(factory_text &&
                  strcmp(factory_text,
                         "rxsig1|graph_test.shape|.graph_test..shape|") == 0,
                  "render factory operand text");
    if (factory_text) {
        uint32_t resolved_factory = RX_GRAPH_NONE;
        ok &= require(rx_graph_resolve_operand(graph,
                                               OP_SRCFPROCSEL_REG_STRING_REG,
                                               1u,
                                               factory_text,
                                               &resolved_factory,
                                               &error) &&
                      resolved_factory == factory,
                      "resolve factory operand ID");
        free(error);
        error = 0;
    }
    {
        uint32_t resolved_factory = RX_GRAPH_NONE;
        ok &= require(rx_graph_resolve_operand(
                          graph,
                          OP_SRCFPROCSEL_REG_STRING_REG,
                          1u,
                          "rxsig1|graph_test.shape|.shape|",
                          &resolved_factory,
                          &error) &&
                      resolved_factory == factory,
                      "resolve source-short factory return type to declared bucket");
        free(error);
        error = 0;
    }
    free(factory_text);
    free(descriptor);

    ok &= require(rx_graph_serialize(graph, &bytes, &byte_count),
                  "serialize graph");
    copy = bytes ? rx_graph_deserialize(bytes, byte_count, &error) : 0;
    ok &= require(copy != 0 && error == 0, "deserialize valid graph");
    if (copy) {
        RxGraphId copied_box = rx_graph_find_type(copy, "graph_test.box");
        RxGraphId copied_shape = rx_graph_find_type(copy, "graph_test.shape");
        const RxGraphTypeRef *copied_box_ref = rx_graph_type_ref(copy, copied_box);
        const RxGraphTypeRef *copied_shape_ref = rx_graph_type_ref(copy, copied_shape);
        ok &= require(copied_box != RX_GRAPH_NONE && copied_shape != RX_GRAPH_NONE &&
                      rx_graph_type_supports(copy, copied_box, copied_shape) &&
                      copied_box_ref && copied_shape_ref &&
                      rx_graph_type_ref_supports(copied_box_ref,
                                                 copied_shape_ref) &&
                      rx_graph_type_ref_dispatch(copied_box_ref,
                                                 describe) == callable,
                      "rebuild runtime views for deserialized graph");
        rx_graph_release(&copy);
    }
    free(error);
    error = 0;

    ok &= require(rx_graph_serialize_sections(graph,
                                              &facts,
                                              &facts_size,
                                              &indexes,
                                              &indexes_size),
                  "serialize split graph sections");
    copy = facts && indexes
        ? rx_graph_deserialize_sections(facts,
                                        facts_size,
                                        indexes,
                                        indexes_size,
                                        &error)
        : 0;
    ok &= require(copy != 0 && error == 0,
                  "deserialize split graph sections");
    rx_graph_release(&copy);
    free(error);
    error = 0;
    if (indexes && indexes_size > 12u) {
        indexes[12] ^= 1u;
        copy = rx_graph_deserialize_sections(facts,
                                             facts_size,
                                             indexes,
                                             indexes_size,
                                             &error);
        ok &= require(copy == 0 && error && strstr(error, "counts"),
                      "report graph facts/index count mismatch");
        rx_graph_release(&copy);
        free(error);
        error = 0;
        indexes[12] ^= 1u;
    }

    if (bytes && byte_count) {
        corrupt = (unsigned char *)malloc(byte_count);
        ok &= require(corrupt != 0, "allocate corruption fixture");
        if (corrupt) {
            memcpy(corrupt, bytes, byte_count);
            corrupt[0] = 'X';
            copy = rx_graph_deserialize(corrupt, byte_count, &error);
            ok &= require(copy == 0 && error && strstr(error, "magic"),
                          "report corrupt graph magic");
            rx_graph_release(&copy);
            free(error);
            error = 0;
        }
    }
    copy = bytes && byte_count
        ? rx_graph_deserialize(bytes, byte_count - 1u, &error)
        : 0;
    ok &= require(copy == 0 && error && strstr(error, "truncated"),
                  "report truncated graph records");
    rx_graph_release(&copy);

    free(error);
    free(corrupt);
    free(bytes);
    free(facts);
    free(indexes);
    rx_graph_release(&graph);

    builder = rx_graph_builder_create();
    ok &= require(builder != 0, "create imported-callable builder");
    if (builder) {
        RxCallableId imported;
        RxCallableId definition;
        procedure.module_index = 0u;
        procedure.procedure_offset = 10u;
        imported = rx_graph_builder_add_callable(builder,
                                                 "graph_test.imported",
                                                 ".string",
                                                 "source=.string",
                                                 procedure,
                                                 RX_GRAPH_CALLABLE_IMPORTED);
        procedure.module_index = 1u;
        procedure.procedure_offset = 20u;
        definition = rx_graph_builder_add_callable(builder,
                                                   "graph_test.imported",
                                                   ".string",
                                                   "value=.string",
                                                   procedure,
                                                   0u);
        graph = rx_graph_builder_finish(builder);
        ok &= require(imported != RX_GRAPH_NONE && definition == imported && graph &&
                      rx_graph_callable(graph, imported, &callable_view) &&
                      callable_view.procedure.module_index == 1u &&
                      callable_view.procedure.procedure_offset == 20u &&
                      callable_view.flags == 0u &&
                      strstr(callable_view.descriptor, "value=.string") != 0,
                      "upgrade compatible imported callable to selected definition");
        rx_graph_release(&graph);
    }

    builder = rx_graph_builder_create();
    ok &= require(builder != 0, "create member-contract builder");
    if (builder) {
        RxMemberId declaration;
        RxMemberId implementation;
        char *lookup;
        declaration = rx_graph_builder_add_member(builder,
                                                  "transform",
                                                  ".string",
                                                  "source=.string",
                                                  RX_GRAPH_MEMBER_METHOD);
        implementation = rx_graph_builder_add_member(builder,
                                                     "transform",
                                                     ".string",
                                                     "value=.string",
                                                     RX_GRAPH_MEMBER_METHOD);
        graph = rx_graph_builder_finish(builder);
        lookup = rx_sig_build_descriptor("transform", ".string", "input=.string");
        ok &= require(declaration != RX_GRAPH_NONE && implementation == declaration &&
                      graph && lookup && rx_graph_find_member(graph, lookup) == declaration,
                      "identify member contracts by parameter types rather than names");
        free(lookup);
        rx_graph_release(&graph);
    }

    builder = rx_graph_builder_create();
    ok &= require(builder != 0, "create duplicate-callable builder");
    if (builder) {
        RxCallableId first;
        RxCallableId conflicting;
        procedure.module_index = 0u;
        procedure.procedure_offset = 10u;
        first = rx_graph_builder_add_callable(builder,
                                              "graph_test.duplicate",
                                              ".string",
                                              "",
                                              procedure,
                                              0u);
        procedure.procedure_offset = 11u;
        conflicting = rx_graph_builder_add_callable(builder,
                                                    "graph_test.duplicate",
                                                    ".int",
                                                    "",
                                                    procedure,
                                                    0u);
        ok &= require(first != RX_GRAPH_NONE && conflicting == RX_GRAPH_NONE,
                      "reject conflicting callable identity");
        graph = rx_graph_builder_finish(builder);
        ok &= require(graph == 0, "do not finalize a conflicted graph");
        rx_graph_release(&graph);
    }

    builder = rx_graph_builder_create();
    ok &= require(builder != 0, "create cyclic-inheritance builder");
    if (builder) {
        RxGraphId left;
        RxGraphId right;
        left = rx_graph_builder_add_type(builder,
                                         "graph_test.left",
                                         RX_GRAPH_TYPE_CLASS,
                                         0u);
        right = rx_graph_builder_add_type(builder,
                                          "graph_test.right",
                                          RX_GRAPH_TYPE_CLASS,
                                          0u);
        ok &= require(rx_graph_builder_add_edge(builder,
                                                left,
                                                right,
                                                RX_GRAPH_REL_INHERITS_CLASS,
                                                0u,
                                                0u) &&
                      rx_graph_builder_add_edge(builder,
                                                right,
                                                left,
                                                RX_GRAPH_REL_INHERITS_CLASS,
                                                0u,
                                                1u),
                      "build cyclic inheritance fixture");
        graph = rx_graph_builder_finish(builder);
        error = 0;
        ok &= require(graph && !rx_graph_validate(graph, &error) && error &&
                      strstr(error, "cycle"),
                      "report inheritance cycle");
        free(error);
        rx_graph_release(&graph);
    }
    return ok ? 0 : 1;
}
