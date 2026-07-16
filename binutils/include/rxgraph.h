/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef CREXX_RXGRAPH_H
#define CREXX_RXGRAPH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RX_GRAPH_NONE UINT32_MAX
#define RX_GRAPH_SERIAL_VERSION 1u

typedef uint32_t RxGraphId;
typedef uint32_t RxMemberId;
typedef uint32_t RxCallableId;
typedef uint32_t RxFactoryId;
typedef uint32_t RxProviderId;

typedef enum RxGraphTypeKind {
    RX_GRAPH_TYPE_OPAQUE = 0,
    RX_GRAPH_TYPE_BUILTIN = 1,
    RX_GRAPH_TYPE_CLASS = 2,
    RX_GRAPH_TYPE_INTERFACE = 3,
    RX_GRAPH_TYPE_EXPRESSION = 4
} RxGraphTypeKind;

typedef enum RxGraphRelation {
    RX_GRAPH_REL_IMPLEMENTS = 1,
    RX_GRAPH_REL_INHERITS_CLASS = 2,
    RX_GRAPH_REL_EXTENDS_INTERFACE = 3,
    RX_GRAPH_REL_TYPE_ALIAS = 4
} RxGraphRelation;

typedef enum RxGraphOperandKind {
    RX_GRAPH_OPERAND_NONE = 0,
    RX_GRAPH_OPERAND_TYPE = 1,
    RX_GRAPH_OPERAND_MEMBER = 2,
    RX_GRAPH_OPERAND_FACTORY = 3
} RxGraphOperandKind;

enum RxGraphMemberFlags {
    RX_GRAPH_MEMBER_METHOD = 1u << 0,
    RX_GRAPH_MEMBER_FINAL = 1u << 1,
    RX_GRAPH_MEMBER_FACTORY = 1u << 2
};

enum RxGraphParamFlags {
    RX_GRAPH_PARAM_REF = 1u << 0,
    RX_GRAPH_PARAM_OPTIONAL = 1u << 1,
    RX_GRAPH_PARAM_VARARG = 1u << 2
};

enum RxGraphCallableFlags {
    RX_GRAPH_CALLABLE_IMPORTED = 1u << 0
};

typedef struct RxGraphProcRef {
    uint32_t module_index;
    uint64_t procedure_offset;
} RxGraphProcRef;

typedef struct RxGraphEdgeView {
    RxGraphId from;
    RxGraphId to;
    uint32_t relation;
    uint32_t origin_module;
    uint32_t ordinal;
} RxGraphEdgeView;

typedef struct RxGraphMemberView {
    const char *name;
    const char *descriptor;
    RxGraphId return_type;
    uint32_t parameter_count;
    uint32_t flags;
} RxGraphMemberView;

typedef struct RxGraphParamView {
    RxGraphId type;
    uint32_t flags;
} RxGraphParamView;

typedef struct RxGraphCallableView {
    const char *symbol;
    const char *descriptor;
    RxGraphId owner_type;
    RxMemberId member;
    RxGraphProcRef procedure;
    uint32_t flags;
} RxGraphCallableView;

typedef struct RxGraphDeclarationView {
    RxGraphId owner;
    RxMemberId member;
    uint32_t flags;
    uint32_t origin_module;
    uint32_t ordinal;
} RxGraphDeclarationView;

typedef struct RxGraphProviderView {
    RxGraphId interface_type;
    RxMemberId factory_member;
    RxGraphId class_type;
    RxCallableId factory_callable;
    RxCallableId match_callable;
    uint32_t origin_module;
    uint32_t ordinal;
} RxGraphProviderView;

typedef struct RxGraphFactoryView {
    RxGraphId interface_type;
    RxMemberId member;
    uint32_t flags;
} RxGraphFactoryView;

typedef struct RxGraph RxGraph;
typedef struct RxGraphBuilder RxGraphBuilder;
struct module_file;

RxGraphBuilder *rx_graph_builder_create(void);
void rx_graph_builder_free(RxGraphBuilder *builder);

RxGraphId rx_graph_builder_add_type(RxGraphBuilder *builder,
                                    const char *name,
                                    RxGraphTypeKind kind,
                                    uint32_t flags);
RxMemberId rx_graph_builder_add_member(RxGraphBuilder *builder,
                                       const char *name,
                                       const char *return_type,
                                       const char *args,
                                       uint32_t flags);
RxCallableId rx_graph_builder_add_callable(RxGraphBuilder *builder,
                                           const char *symbol,
                                           const char *return_type,
                                           const char *args,
                                           RxGraphProcRef procedure,
                                           uint32_t flags);
int rx_graph_builder_add_edge(RxGraphBuilder *builder,
                              RxGraphId from,
                              RxGraphId to,
                              RxGraphRelation relation,
                              uint32_t origin_module,
                              uint32_t ordinal);
int rx_graph_builder_add_declaration(RxGraphBuilder *builder,
                                     RxGraphId owner,
                                     RxMemberId member,
                                     uint32_t flags,
                                     uint32_t origin_module,
                                     uint32_t ordinal);
int rx_graph_builder_add_dispatch(RxGraphBuilder *builder,
                                  RxGraphId owner,
                                  RxMemberId member,
                                  RxCallableId callable);
RxProviderId rx_graph_builder_add_provider(RxGraphBuilder *builder,
                                           RxGraphId interface_type,
                                           RxMemberId factory_member,
                                           RxGraphId class_type,
                                           RxCallableId factory_callable,
                                           RxCallableId match_callable,
                                           uint32_t origin_module,
                                           uint32_t ordinal);

RxGraphId rx_graph_builder_find_type(const RxGraphBuilder *builder,
                                     const char *name);
RxMemberId rx_graph_builder_find_member(const RxGraphBuilder *builder,
                                        const char *descriptor);
RxCallableId rx_graph_builder_find_callable(const RxGraphBuilder *builder,
                                            const char *symbol);

RxGraph *rx_graph_builder_finish(RxGraphBuilder *builder);

/* cREXX metadata policy adapter used by RXAS, RXLINK, and native-module load. */
RxGraph *rx_graph_build_crexx(struct module_file *const *modules,
                              size_t module_count,
                              char **error_message);

void rx_graph_retain(RxGraph *graph);
void rx_graph_release(RxGraph **graph);

size_t rx_graph_type_count(const RxGraph *graph);
size_t rx_graph_member_count(const RxGraph *graph);
size_t rx_graph_callable_count(const RxGraph *graph);
size_t rx_graph_factory_count(const RxGraph *graph);
size_t rx_graph_provider_count(const RxGraph *graph);
size_t rx_graph_relationship_count(const RxGraph *graph);
size_t rx_graph_declaration_total(const RxGraph *graph);

RxGraphId rx_graph_find_type(const RxGraph *graph, const char *name);
const char *rx_graph_type_name(const RxGraph *graph, RxGraphId type);
RxGraphTypeKind rx_graph_type_kind(const RxGraph *graph, RxGraphId type);
uint32_t rx_graph_type_flags(const RxGraph *graph, RxGraphId type);

RxMemberId rx_graph_find_member(const RxGraph *graph, const char *descriptor);
int rx_graph_member(const RxGraph *graph,
                    RxMemberId member,
                    RxGraphMemberView *view);
int rx_graph_member_parameter(const RxGraph *graph,
                              RxMemberId member,
                              uint32_t parameter_index,
                              RxGraphParamView *view);
size_t rx_graph_declaration_count(const RxGraph *graph, RxGraphId owner);
int rx_graph_declaration(const RxGraph *graph,
                         RxGraphId owner,
                         size_t position,
                         RxGraphDeclarationView *view);

RxCallableId rx_graph_find_callable(const RxGraph *graph, const char *symbol);
int rx_graph_callable(const RxGraph *graph,
                      RxCallableId callable,
                      RxGraphCallableView *view);
RxFactoryId rx_graph_find_factory(const RxGraph *graph,
                                  RxGraphId interface_type,
                                  RxMemberId member);
int rx_graph_factory(const RxGraph *graph,
                     RxFactoryId factory,
                     RxGraphFactoryView *view);

size_t rx_graph_edge_count(const RxGraph *graph,
                           RxGraphId type,
                           RxGraphRelation relation,
                           int incoming);
int rx_graph_edge(const RxGraph *graph,
                  RxGraphId type,
                  RxGraphRelation relation,
                  int incoming,
                  size_t position,
                  RxGraphEdgeView *view);
int rx_graph_type_supports(const RxGraph *graph,
                           RxGraphId concrete_type,
                           RxGraphId target_type);

RxCallableId rx_graph_dispatch(const RxGraph *graph,
                               RxGraphId concrete_type,
                               RxMemberId member);
size_t rx_graph_provider_bucket_size(const RxGraph *graph,
                                     RxGraphId interface_type,
                                     RxMemberId factory_member);
int rx_graph_provider(const RxGraph *graph,
                      RxGraphId interface_type,
                      RxMemberId factory_member,
                      size_t position,
                      RxGraphProviderView *view);

char *rx_graph_normalize_type_name(const char *name);
char *rx_graph_type_source_name(const char *canonical_name);
RxGraphOperandKind rx_graph_operand_kind(int opcode, unsigned int operand_index);
int rx_graph_resolve_operand(const RxGraph *graph,
                             int opcode,
                             unsigned int operand_index,
                             const char *text,
                             uint32_t *id,
                             char **error_message);
char *rx_graph_operand_text(const RxGraph *graph,
                            int opcode,
                            unsigned int operand_index,
                            uint32_t id);

int rx_graph_serialize(const RxGraph *graph,
                       unsigned char **data,
                       size_t *size);
int rx_graph_serialize_sections(const RxGraph *graph,
                                unsigned char **facts,
                                size_t *facts_size,
                                unsigned char **indexes,
                                size_t *indexes_size);
RxGraph *rx_graph_deserialize(const unsigned char *data,
                              size_t size,
                              char **error_message);
RxGraph *rx_graph_deserialize_sections(const unsigned char *facts,
                                       size_t facts_size,
                                       const unsigned char *indexes,
                                       size_t indexes_size,
                                       char **error_message);
int rx_graph_validate(const RxGraph *graph, char **error_message);

#ifdef __cplusplus
}
#endif

#endif
