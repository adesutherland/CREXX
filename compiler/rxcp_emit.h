/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
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

/**
 * Shared internal header for the Emitter subsystem
 */

#ifndef CREXX_RXCP_EMIT_H
#define CREXX_RXCP_EMIT_H

#include "rxcp_types.h"
#include "rxflags.h"
#include "rxbin.h"

#define UNSET_REGISTER (-1)
#define DONT_ASSIGN_REGISTER (-2)

typedef struct walker_payload {
    Context *context;
    int globals;
    FILE *file;
} walker_payload;

/* Output Marshalling */
struct OutputFragment {
    OutputFragment *before;
    OutputFragment *after;
    char *output;
};

extern const char* emit_promotion[10][10];

OutputFragment *output_f();
OutputFragment *output_fs(char* text);
void f_output(OutputFragment *output);
void output_insert_before(OutputFragment* existing, OutputFragment* before);
void output_insert_after(OutputFragment* existing, OutputFragment* after);
void output_concat(OutputFragment* before, OutputFragment* after);
void output_append_text(OutputFragment* before, char* after);
void output_prepend_text(char* before, OutputFragment* after);
void print_output(FILE* file, OutputFragment* existing);

char* get_metaline(ASTNode *node);
char* get_reporting_metalines(ASTNode *node);
void reset_metaline_source_file(const char *file_name);
char* get_metaline_range(ASTNode *from, ASTNode *to);
char* get_metaline_between(ASTNode *from, ASTNode *to);
char* get_metaline_token_after(ASTNode *node);
char* get_metaline_clause(ASTNode *node);
char* get_metaline_token_at(ASTNode *node);
char* get_comment(ASTNode *node, char* prefix);
char* get_comment_line_number_only(ASTNode *node, char* comment_text);
unsigned int trace_source_step_id_from_metaline(const char *metaline);
unsigned int trace_clause_id_from_metaline(const char *metaline);
char *trace_symbol_name_malloc(ASTNode *node);
char *trace_event_metaline(char kind,
                           unsigned int mode_mask,
                           char value_source,
                           ValueType value_type,
                           char register_type,
                           int value_ref,
                           unsigned int source_step_id,
                           unsigned int clause_id,
                           unsigned int flags,
                           const char *symbol,
                           const char *resolved_name);
char *trace_event_register_metaline(char kind,
                                    unsigned int mode_mask,
                                    ASTNode *value_node,
                                    unsigned int source_step_id,
                                    unsigned int clause_id,
                                    const char *symbol,
                                    const char *resolved_name);
void output_append_trace_event_register(OutputFragment *output,
                                        char kind,
                                        unsigned int mode_mask,
                                        ASTNode *value_node,
                                        unsigned int source_step_id,
                                        unsigned int clause_id,
                                        const char *symbol,
                                        const char *resolved_name);
void output_apply_trace_source_ids(OutputFragment *output,
                                   unsigned int source_step_id,
                                   unsigned int clause_id);

void type_promotion(ASTNode *node);
void add_variable_metadata(ASTNode* node);
void clear_variable_metadata(ASTNode* node);
void emit_scope_reference_lifetimes(ASTNode *node);
void clear_scope_variable_metadata(ASTNode *node);
void clear_global_variable_metadata(ASTNode *node);

/* Metadata Helpers */
void meta_set_symbol(Symbol *symbol, void *payload);
void add_initiator(Symbol *symbol, void *payload);
void add_scope_initiators(ASTNode* node);
void add_global_symbol(Symbol *symbol, void *payload);
void add_exposed_global_variable(ASTNode* node);
void meta_set_global_symbol(Symbol *symbol, void *payload);
void add_global_variable_metadata(ASTNode* node);
void add_all_class_metadata(ASTNode* scope_node, ASTNode* output_node);
void meta_clear_symbol(Symbol *symbol, void *payload);
void meta_clear_global_symbol(Symbol *symbol, void *payload);
char *meta_narg(ASTNode *node);
char *callable_effective_return_type(ASTNode *node);

/* Procedure Emitter */
void emit_proc(ASTNode *node, void *payload);

void emit_flow(ASTNode *node, void *payload);
void emit_expression(ASTNode *node, void *payload);
int is_constant(ASTNode* node);
char* format_constant(ValueType type, ASTNode* node);
char* type_to_prefix(ValueType value_type);

/* Register Allocation Pass */
walker_result register_walker(walker_direction direction,
                              ASTNode* node,
                              void *pl);

#endif //CREXX_RXCP_EMIT_H
