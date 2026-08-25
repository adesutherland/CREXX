/* cREXX License (MIT) */

#include "rxgraph.h"
#include "rxbin.h"
#include "rxsha256.h"
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
    RxGraphId work;
    RxMemberId describe;
    RxMemberId increment;
    RxMemberId factory_member;
    RxCallableId callable;
    RxCallableId factory_callable;
    RxCallableId task_method_callable;
    RxCallableId receiver_factory_callable;
    RxCallableId taskwork_factory_callable;
    RxCallableId taskwork_run_callable;
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

    {
        static const unsigned char abc_digest[32] = {
            0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
            0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
            0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
            0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
        };
        unsigned char digest[32];
        rx_sha256("abc", 3u, digest);
        ok &= require(memcmp(digest, abc_digest, sizeof(digest)) == 0,
                      "compute the SHA-256 standard vector");
    }

    ok &= require(rx_graph_builder_find_type(builder, ".boolean") != RX_GRAPH_NONE,
                  "seed boolean as a built-in graph type");
    {
        rx_callable_signature parsed;
        ok &= require(rx_sig_init_from_parts(&parsed,
                                             "execute",
                                             ".string",
                                             "request=.object,?correlation=.string") &&
                      parsed.arg_count == 2u &&
                      parsed.args[0].name &&
                      strcmp(parsed.args[0].name, "request") == 0 &&
                      parsed.args[1].name &&
                      strcmp(parsed.args[1].name, "correlation") == 0 &&
                      parsed.args[1].is_optional,
                      "retain callable parameter names in the private signature model");
        rx_sig_free(&parsed);
    }
    {
        rx_callable_signature parsed;
        ok &= require(rx_sig_init_from_parts(&parsed,
                                             "shape",
                                             ".void",
                                             "matrix=.string[2,3],label=.string") &&
                      parsed.arg_count == 2u &&
                      strcmp(parsed.args[0].type, ".string[2,3]") == 0 &&
                      strcmp(parsed.args[1].name, "label") == 0,
                      "split signature arguments outside array dimension brackets");
        rx_sig_free(&parsed);
    }

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
    work = rx_graph_builder_add_type(builder,
                                     "graph_test.work",
                                     RX_GRAPH_TYPE_CLASS,
                                     0u);
    describe = rx_graph_builder_add_member(builder,
                                           "describe",
                                           ".string",
                                           "",
                                           RX_GRAPH_MEMBER_METHOD);
    increment = rx_graph_builder_add_member(builder,
                                            "increment",
                                            ".int",
                                            "value=.int",
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
    procedure.procedure_offset = 1350u;
    receiver_factory_callable = rx_graph_builder_add_callable(
            builder,
            "graph_test.box.\xC2\xA7" "factory.from_channel",
            ".graph_test..box",
            "encoded=.channelvalue",
            procedure,
            0u);
    procedure.procedure_offset = 1400u;
    task_method_callable = rx_graph_builder_add_callable(
            builder,
            "graph_test.box.increment",
            ".int",
            "value=.int",
            procedure,
            0u);
    procedure.procedure_offset = 1450u;
    taskwork_factory_callable = rx_graph_builder_add_callable(
            builder,
            "graph_test.work.\xC2\xA7" "factory",
            "graph_test.work",
            "delta=.int",
            procedure,
            0u);
    procedure.procedure_offset = 1500u;
    taskwork_run_callable = rx_graph_builder_add_callable(
            builder,
            "graph_test.work.run",
            ".channelvalue",
            "request=.channelvalue,context=.taskcontext",
            procedure,
            0u);
    ok &= require(shape != RX_GRAPH_NONE && box != RX_GRAPH_NONE,
                  "add graph types");
    ok &= require(describe != RX_GRAPH_NONE && increment != RX_GRAPH_NONE &&
                  callable != RX_GRAPH_NONE &&
                  factory_member != RX_GRAPH_NONE &&
                  factory_callable != RX_GRAPH_NONE &&
                  receiver_factory_callable != RX_GRAPH_NONE &&
                  task_method_callable != RX_GRAPH_NONE &&
                  taskwork_factory_callable != RX_GRAPH_NONE &&
                  taskwork_run_callable != RX_GRAPH_NONE &&
                  work != RX_GRAPH_NONE,
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
    ok &= require(rx_graph_builder_add_dispatch(
                          builder, box, increment, task_method_callable),
                  "add task-method dispatch row");
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
    {
        unsigned char binding[RX_GRAPH_TASK_BINDING_SIZE];
        unsigned char changed[RX_GRAPH_TASK_BINDING_SIZE];
        unsigned char graph_digest[32];
        unsigned char wrong_digest[32];
        RxCallableId bound_callable = RX_GRAPH_NONE;
        unsigned int bound_kind = 0u;
        ok &= require(rx_graph_task_binding(graph,
                                            "graph_test.box.describe",
                                            1u,
                                            binding) &&
                      rx_graph_digest(graph, graph_digest) &&
                      rx_graph_task_binding_validate_digest(
                              graph, graph_digest, binding,
                              &bound_callable, &bound_kind) &&
                      rx_graph_task_binding_validate(graph,
                                                     binding,
                                                     &bound_callable,
                                                     &bound_kind) &&
                      bound_callable == callable && bound_kind == 1u,
                      "seal and validate a callable task binding");
        memcpy(wrong_digest, graph_digest, sizeof(wrong_digest));
        wrong_digest[0] ^= 1u;
        ok &= require(!rx_graph_task_binding_validate_digest(
                              graph, wrong_digest, binding, 0, 0),
                      "reject a task binding against a wrong cached graph digest");
        memcpy(changed, binding, sizeof(changed));
        changed[12] ^= 1u;
        ok &= require(!rx_graph_task_binding_validate(graph,
                                                      changed,
                                                      0,
                                                      0),
                      "reject a task binding for a different image graph");
        memcpy(changed, binding, sizeof(changed));
        changed[44] ^= 1u;
        ok &= require(!rx_graph_task_binding_validate(graph,
                                                      changed,
                                                      0,
                                                      0),
                      "reject a task binding with a stale callable signature");
        memcpy(changed, binding, sizeof(changed));
        changed[76] = 1u;
        ok &= require(!rx_graph_task_binding_validate(graph,
                                                      changed,
                                                      0,
                                                      0),
                      "reject noncanonical task binding reserved bytes");
        ok &= require(!rx_graph_task_binding(graph,
                                             "graph_test.missing",
                                             1u,
                                             changed),
                      "reject a missing callable task target");
        ok &= require(rx_graph_task_binding(graph,
                                            "graph_test.box.increment",
                                            2u,
                                            binding) &&
                      binding[76] ==
                              (unsigned char)(receiver_factory_callable + 1u) &&
                      binding[77] == 0u && binding[78] == 0u &&
                      binding[79] == 0u &&
                      rx_graph_task_binding_validate(graph,
                                                     binding,
                                                     &bound_callable,
                                                     &bound_kind) &&
                      bound_callable == task_method_callable &&
                      bound_kind == 2u,
                      "seal and validate a task method receiver factory");
        memcpy(changed, binding, sizeof(changed));
        changed[76] ^= 1u;
        ok &= require(!rx_graph_task_binding_validate(graph,
                                                      changed,
                                                      0,
                                                      0),
                      "reject a task method with a changed receiver factory");
        ok &= require(rx_graph_task_binding(
                              graph,
                              "graph_test.work.\xC2\xA7" "factory",
                              3u,
                              binding) &&
                      binding[76] ==
                              (unsigned char)(taskwork_run_callable + 1u) &&
                      binding[77] == 0u && binding[78] == 0u &&
                      binding[79] == 0u &&
                      rx_graph_task_binding_validate(graph,
                                                     binding,
                                                     &bound_callable,
                                                     &bound_kind) &&
                      bound_callable == taskwork_factory_callable &&
                      bound_kind == 3u,
                      "seal and validate a taskwork run adapter");
        memcpy(changed, binding, sizeof(changed));
        changed[76] ^= 1u;
        ok &= require(!rx_graph_task_binding_validate(graph,
                                                      changed,
                                                      0,
                                                      0),
                      "reject a taskwork binding with a changed run adapter");
    }
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
    ok &= require(builder != 0, "create unresolved imported-callable builder");
    if (builder) {
        RxCallableId imported;
        RxCallableId definition;
        procedure.module_index = 47u;
        procedure.procedure_offset = SIZE_MAX;
        imported = rx_graph_builder_add_callable(
            builder,
            "time.offsetdatetime.factory",
            ".time..offsetdatetime",
            "?datetime=.unknown,?offset_minutes=.int",
            procedure,
            RX_GRAPH_CALLABLE_IMPORTED);
        procedure.module_index = 48u;
        procedure.procedure_offset = 0u;
        definition = rx_graph_builder_add_callable(
            builder,
            "time.offsetdatetime.factory",
            ".time..offsetdatetime",
            "?datetime=.datetime,?offset_minutes=.int",
            procedure,
            0u);
        graph = rx_graph_builder_finish(builder);
        ok &= require(imported != RX_GRAPH_NONE && definition == imported && graph &&
                      rx_graph_callable(graph, imported, &callable_view) &&
                      callable_view.procedure.module_index == 48u &&
                      callable_view.procedure.procedure_offset == 0u &&
                      callable_view.flags == 0u &&
                      strstr(callable_view.descriptor, "?datetime=.datetime") != 0 &&
                      rx_graph_find_type(graph, ".datetime") != RX_GRAPH_NONE,
                      "refine unresolved imported callable from selected definition");
        rx_graph_release(&graph);
    }

    builder = rx_graph_builder_create();
    ok &= require(builder != 0, "create definition-first unresolved import builder");
    if (builder) {
        RxCallableId definition;
        RxCallableId imported;
        procedure.module_index = 2u;
        procedure.procedure_offset = 30u;
        definition = rx_graph_builder_add_callable(builder,
                                                   "graph_test.definition_first",
                                                   ".string",
                                                   "value=.int",
                                                   procedure,
                                                   0u);
        procedure.module_index = 3u;
        procedure.procedure_offset = SIZE_MAX;
        imported = rx_graph_builder_add_callable(builder,
                                                 "graph_test.definition_first",
                                                 ".unknown",
                                                 "value=.unknown",
                                                 procedure,
                                                 RX_GRAPH_CALLABLE_IMPORTED);
        graph = rx_graph_builder_finish(builder);
        ok &= require(definition != RX_GRAPH_NONE && imported == definition && graph &&
                      rx_graph_callable(graph, definition, &callable_view) &&
                      callable_view.procedure.module_index == 2u &&
                      callable_view.procedure.procedure_offset == 30u &&
                      strstr(callable_view.descriptor, "|.string|value=.int") != 0,
                      "accept unresolved import after its selected definition");
        rx_graph_release(&graph);
    }

    builder = rx_graph_builder_create();
    ok &= require(builder != 0, "create incompatible imported-callable builder");
    if (builder) {
        RxCallableId imported;
        RxCallableId definition;
        procedure.module_index = 0u;
        procedure.procedure_offset = SIZE_MAX;
        imported = rx_graph_builder_add_callable(builder,
                                                 "graph_test.incompatible_import",
                                                 ".string",
                                                 "value=.int",
                                                 procedure,
                                                 RX_GRAPH_CALLABLE_IMPORTED);
        procedure.module_index = 1u;
        procedure.procedure_offset = 20u;
        definition = rx_graph_builder_add_callable(builder,
                                                   "graph_test.incompatible_import",
                                                   ".string",
                                                   "value=.string",
                                                   procedure,
                                                   0u);
        ok &= require(imported != RX_GRAPH_NONE && definition == RX_GRAPH_NONE,
                      "reject known imported callable type conflict");
        graph = rx_graph_builder_finish(builder);
        ok &= require(graph == 0, "do not finalize an incompatible imported callable");
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
