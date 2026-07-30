/* cREXX License (MIT) */

#include "contract_model.h"

#include "rxbin.h"
#include "rxgraph.h"
#include "rxsignature.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ContractAdapter {
    const CrexxContractOptions *options;
    const RxGraph *graph;
    CrexxContractModel *model;
    unsigned char *nullable_used;
    unsigned char *optional_used;
} ContractAdapter;

static char *contract_strdup(const char *text) {
    size_t length;
    char *copy;
    if (!text) return 0;
    length = strlen(text);
    copy = (char *)malloc(length + 1u);
    if (!copy) return 0;
    memcpy(copy, text, length + 1u);
    return copy;
}

void crexx_contract_set_error(char **error_message, const char *format, ...) {
    va_list args;
    va_list copy;
    int length;
    char *message;

    if (!error_message || *error_message) return;
    va_start(args, format);
    va_copy(copy, args);
    length = vsnprintf(0, 0, format, copy);
    va_end(copy);
    if (length < 0) {
        va_end(args);
        return;
    }
    message = (char *)malloc((size_t)length + 1u);
    if (message) vsnprintf(message, (size_t)length + 1u, format, args);
    va_end(args);
    *error_message = message;
}

static void schema_free(CrexxContractSchema *schema) {
    if (!schema) return;
    free(schema->record_name);
    if (schema->items) {
        schema_free(schema->items);
        free(schema->items);
    }
    memset(schema, 0, sizeof(*schema));
}

static void field_free(CrexxContractField *field) {
    if (!field) return;
    free(field->name);
    schema_free(&field->schema);
    memset(field, 0, sizeof(*field));
}

void crexx_contract_model_init(CrexxContractModel *model) {
    if (model) memset(model, 0, sizeof(*model));
}

void crexx_contract_model_free(CrexxContractModel *model) {
    size_t i;
    size_t j;
    if (!model) return;
    free(model->contract_version);
    free(model->operation);
    for (i = 0; i < model->input_count; i++) field_free(&model->inputs[i]);
    free(model->inputs);
    schema_free(&model->result);
    for (i = 0; i < model->error_count; i++) schema_free(&model->errors[i]);
    free(model->errors);
    for (i = 0; i < model->record_count; i++) {
        free(model->records[i].name);
        for (j = 0; j < model->records[i].field_count; j++) {
            field_free(&model->records[i].fields[j]);
        }
        free(model->records[i].fields);
    }
    free(model->records);
    crexx_contract_model_init(model);
}

static int suffix_matches(const char *name, const char *short_name) {
    size_t name_length = strlen(name);
    size_t short_length = strlen(short_name);
    return name_length > short_length &&
           name[name_length - short_length - 1u] == '.' &&
           strcmp(name + name_length - short_length, short_name) == 0;
}

static char *type_namespace(const char *type_name) {
    const char *dot = strrchr(type_name, '.');
    size_t length;
    char *result;
    if (!dot) return contract_strdup("");
    length = (size_t)(dot - type_name);
    result = (char *)malloc(length + 1u);
    if (!result) return 0;
    memcpy(result, type_name, length);
    result[length] = 0;
    return result;
}

static int graph_declared_kind(RxGraphTypeKind kind) {
    return kind == RX_GRAPH_TYPE_INTERFACE || kind == RX_GRAPH_TYPE_CLASS;
}

static int resolve_object_type(ContractAdapter *adapter,
                               const char *spelling,
                               const char *context_type,
                               RxGraphId *resolved,
                               char **error_message) {
    char *normalized;
    const char *short_name;
    RxGraphId exact;
    RxGraphId match = RX_GRAPH_NONE;
    size_t match_count = 0u;
    size_t i;

    normalized = rx_graph_normalize_type_name(spelling);
    if (!normalized) {
        crexx_contract_set_error(error_message, "out of memory resolving type '%s'", spelling);
        return 0;
    }
    exact = rx_graph_find_type(adapter->graph, normalized);
    if (exact != RX_GRAPH_NONE && graph_declared_kind(rx_graph_type_kind(adapter->graph, exact))) {
        *resolved = exact;
        free(normalized);
        return 1;
    }

    short_name = normalized[0] == '.' ? normalized + 1 : normalized;
    if (!strchr(short_name, '.') && context_type) {
        char *name_space = type_namespace(context_type);
        if (name_space) {
            size_t length = strlen(name_space) + strlen(short_name) + 2u;
            char *candidate = (char *)malloc(length);
            if (candidate) {
                if (*name_space) snprintf(candidate, length, "%s.%s", name_space, short_name);
                else snprintf(candidate, length, "%s", short_name);
                exact = rx_graph_find_type(adapter->graph, candidate);
                if (exact != RX_GRAPH_NONE &&
                    graph_declared_kind(rx_graph_type_kind(adapter->graph, exact))) {
                    *resolved = exact;
                    free(candidate);
                    free(name_space);
                    free(normalized);
                    return 1;
                }
                free(candidate);
            }
            free(name_space);
        }
    }

    for (i = 0; i < rx_graph_type_count(adapter->graph); i++) {
        const char *candidate = rx_graph_type_name(adapter->graph, (RxGraphId)i);
        if (candidate && graph_declared_kind(rx_graph_type_kind(adapter->graph, (RxGraphId)i)) &&
            (strcmp(candidate, short_name) == 0 || suffix_matches(candidate, short_name))) {
            match = (RxGraphId)i;
            match_count++;
        }
    }
    if (match_count == 1u) {
        *resolved = match;
        free(normalized);
        return 1;
    }
    if (match_count > 1u) {
        crexx_contract_set_error(error_message,
                                 "type '%s' is ambiguous in compiled metadata; use a fully qualified Level B type",
                                 spelling);
    } else {
        crexx_contract_set_error(error_message,
                                 "type '%s' does not resolve to a compiled class or interface",
                                 spelling);
    }
    free(normalized);
    return 0;
}

static size_t find_record(const CrexxContractModel *model, const char *name) {
    size_t i;
    for (i = 0; i < model->record_count; i++) {
        if (strcmp(model->records[i].name, name) == 0) return i;
    }
    return (size_t)-1;
}

static int field_option(const char *record_name,
                        const char *field_name,
                        const char *const *options,
                        size_t option_count,
                        unsigned char *used) {
    size_t length = strlen(record_name) + strlen(field_name) + 2u;
    char *path = (char *)malloc(length);
    size_t i;
    int found = 0;
    if (!path) return 0;
    snprintf(path, length, "%s.%s", record_name, field_name);
    for (i = 0; i < option_count; i++) {
        if (strcmp(path, options[i]) == 0) {
            used[i] = 1u;
            found = 1;
        }
    }
    free(path);
    return found;
}

static int schema_equal(const CrexxContractSchema *left,
                        const CrexxContractSchema *right) {
    if (left->kind != right->kind) return 0;
    if (left->kind == CREXX_CONTRACT_RECORD) {
        return left->record_name && right->record_name &&
               strcmp(left->record_name, right->record_name) == 0;
    }
    if (left->kind == CREXX_CONTRACT_ARRAY) {
        return left->items && right->items && schema_equal(left->items, right->items);
    }
    return 1;
}

static int append_record_field(ContractAdapter *adapter,
                               size_t record_index,
                               CrexxContractField *field,
                               char **error_message) {
    CrexxContractRecord *record;
    CrexxContractField *grown;
    size_t i;
    record = &adapter->model->records[record_index];
    for (i = 0; i < record->field_count; i++) {
        if (strcmp(record->fields[i].name, field->name) == 0) {
            if (record->fields[i].required == field->required &&
                record->fields[i].nullable == field->nullable &&
                schema_equal(&record->fields[i].schema, &field->schema)) {
                field_free(field);
                return 1;
            }
            crexx_contract_set_error(error_message,
                                     "payload interface '%s' has conflicting inherited field '%s'",
                                     record->name,
                                     field->name);
            return 0;
        }
    }
    grown = (CrexxContractField *)realloc(record->fields,
                                          (record->field_count + 1u) * sizeof(*grown));
    if (!grown) {
        crexx_contract_set_error(error_message, "out of memory adding payload field");
        return 0;
    }
    record->fields = grown;
    record->fields[record->field_count++] = *field;
    memset(field, 0, sizeof(*field));
    return 1;
}

static int add_record(ContractAdapter *adapter,
                      RxGraphId type,
                      char **error_message);

static int build_schema(ContractAdapter *adapter,
                        const char *type_text,
                        const char *context_type,
                        int allow_void,
                        CrexxContractSchema *schema,
                        char **error_message) {
    const char *bracket;
    size_t base_length;
    char *base;
    RxGraphId resolved;
    const char *resolved_name;

    memset(schema, 0, sizeof(*schema));
    bracket = strchr(type_text, '[');
    if (bracket) {
        if (strcmp(bracket, "[*]") != 0 || strchr(bracket + 1, '[')) {
            crexx_contract_set_error(error_message,
                                     "type '%s' is not a one-dimensional dynamic array supported by format 1",
                                     type_text);
            return 0;
        }
        base_length = (size_t)(bracket - type_text);
        base = (char *)malloc(base_length + 1u);
        if (!base) {
            crexx_contract_set_error(error_message, "out of memory parsing array type");
            return 0;
        }
        memcpy(base, type_text, base_length);
        base[base_length] = 0;
        schema->kind = CREXX_CONTRACT_ARRAY;
        schema->items = (CrexxContractSchema *)calloc(1, sizeof(*schema->items));
        if (!schema->items ||
            !build_schema(adapter, base, context_type, 0, schema->items, error_message)) {
            free(base);
            schema_free(schema);
            return 0;
        }
        free(base);
        return 1;
    }

    if (strcmp(type_text, ".void") == 0) {
        if (!allow_void) {
            crexx_contract_set_error(error_message, "payload field type .void is unsupported");
            return 0;
        }
        schema->kind = CREXX_CONTRACT_VOID;
        return 1;
    }
    if (strcmp(type_text, ".string") == 0) schema->kind = CREXX_CONTRACT_STRING;
    else if (strcmp(type_text, ".boolean") == 0) schema->kind = CREXX_CONTRACT_BOOLEAN;
    else if (strcmp(type_text, ".int") == 0) schema->kind = CREXX_CONTRACT_INTEGER;
    else if (strcmp(type_text, ".float") == 0) schema->kind = CREXX_CONTRACT_NUMBER;
    else if (strcmp(type_text, ".decimal") == 0) schema->kind = CREXX_CONTRACT_DECIMAL;
    else if (strcmp(type_text, ".binary") == 0) schema->kind = CREXX_CONTRACT_BINARY;
    else if (strcmp(type_text, ".object") == 0 || strcmp(type_text, ".unknown") == 0 ||
             strncmp(type_text, "reference ", 10u) == 0) {
        crexx_contract_set_error(error_message,
                                 "type '%s' is outside the fail-closed format-1 contract slice",
                                 type_text);
        return 0;
    } else {
        if (!resolve_object_type(adapter, type_text, context_type, &resolved, error_message)) return 0;
        if (rx_graph_type_kind(adapter->graph, resolved) != RX_GRAPH_TYPE_INTERFACE) {
            crexx_contract_set_error(error_message,
                                     "payload type '%s' is a concrete class; format 1 exports Level B interfaces only",
                                     rx_graph_type_name(adapter->graph, resolved));
            return 0;
        }
        resolved_name = rx_graph_type_name(adapter->graph, resolved);
        schema->kind = CREXX_CONTRACT_RECORD;
        schema->record_name = contract_strdup(resolved_name);
        if (!schema->record_name || !add_record(adapter, resolved, error_message)) {
            schema_free(schema);
            return 0;
        }
    }
    return 1;
}

static int collect_interface_fields(ContractAdapter *adapter,
                                    size_t record_index,
                                    RxGraphId type,
                                    unsigned depth,
                                    char **error_message) {
    size_t edge_count;
    size_t i;
    const char *record_name;

    if (depth > rx_graph_type_count(adapter->graph)) {
        crexx_contract_set_error(error_message, "cyclic payload-interface inheritance");
        return 0;
    }
    edge_count = rx_graph_edge_count(adapter->graph, type, RX_GRAPH_REL_EXTENDS_INTERFACE, 0);
    for (i = 0; i < edge_count; i++) {
        RxGraphEdgeView edge;
        if (!rx_graph_edge(adapter->graph, type, RX_GRAPH_REL_EXTENDS_INTERFACE, 0, i, &edge) ||
            rx_graph_type_kind(adapter->graph, edge.to) != RX_GRAPH_TYPE_INTERFACE ||
            !collect_interface_fields(adapter, record_index, edge.to, depth + 1u, error_message)) {
            if (!*error_message) crexx_contract_set_error(error_message, "invalid interface inheritance metadata");
            return 0;
        }
    }

    record_name = adapter->model->records[record_index].name;
    for (i = 0; i < rx_graph_declaration_count(adapter->graph, type); i++) {
        RxGraphDeclarationView declaration;
        RxGraphMemberView member;
        rx_callable_signature signature;
        CrexxContractField field;

        memset(&field, 0, sizeof(field));
        if (!rx_graph_declaration(adapter->graph, type, i, &declaration) ||
            !rx_graph_member(adapter->graph, declaration.member, &member)) {
            crexx_contract_set_error(error_message, "invalid payload member metadata");
            return 0;
        }
        if ((declaration.flags & RX_GRAPH_MEMBER_FACTORY) ||
            (declaration.flags & RX_GRAPH_MEMBER_FINAL) ||
            !(declaration.flags & RX_GRAPH_MEMBER_METHOD)) {
            crexx_contract_set_error(error_message,
                                     "payload interface '%s' member '%s' is not an abstract field method",
                                     record_name,
                                     member.name);
            return 0;
        }
        if (!rx_sig_parse_descriptor(member.descriptor, &signature)) {
            crexx_contract_set_error(error_message,
                                     "malformed signature metadata for '%s.%s'",
                                     record_name,
                                     member.name);
            return 0;
        }
        if (signature.arg_count != 0u) {
            crexx_contract_set_error(error_message,
                                     "payload interface '%s' member '%s' has arguments; fields must be zero-argument methods",
                                     record_name,
                                     member.name);
            rx_sig_free(&signature);
            return 0;
        }
        field.name = contract_strdup(member.name);
        field.required = !field_option(record_name,
                                       member.name,
                                       adapter->options->optional_fields,
                                       adapter->options->optional_count,
                                       adapter->optional_used);
        field.nullable = field_option(record_name,
                                      member.name,
                                      adapter->options->nullable_fields,
                                      adapter->options->nullable_count,
                                      adapter->nullable_used);
        if (!field.name ||
            !build_schema(adapter,
                          signature.return_type,
                          rx_graph_type_name(adapter->graph, type),
                          0,
                          &field.schema,
                          error_message)) {
            rx_sig_free(&signature);
            field_free(&field);
            return 0;
        }
        rx_sig_free(&signature);
        if (!append_record_field(adapter, record_index, &field, error_message)) {
            field_free(&field);
            return 0;
        }
        record_name = adapter->model->records[record_index].name;
    }
    return 1;
}

static int add_record(ContractAdapter *adapter,
                      RxGraphId type,
                      char **error_message) {
    const char *name = rx_graph_type_name(adapter->graph, type);
    size_t index = find_record(adapter->model, name);
    CrexxContractRecord *grown;
    if (index != (size_t)-1) {
        return adapter->model->records[index].build_state != 3;
    }
    grown = (CrexxContractRecord *)realloc(adapter->model->records,
                                           (adapter->model->record_count + 1u) * sizeof(*grown));
    if (!grown) {
        crexx_contract_set_error(error_message, "out of memory adding payload interface");
        return 0;
    }
    adapter->model->records = grown;
    index = adapter->model->record_count++;
    memset(&adapter->model->records[index], 0, sizeof(adapter->model->records[index]));
    adapter->model->records[index].name = contract_strdup(name);
    adapter->model->records[index].build_state = 1;
    if (!adapter->model->records[index].name) {
        crexx_contract_set_error(error_message, "out of memory naming payload interface");
        return 0;
    }
    if (!collect_interface_fields(adapter, index, type, 0u, error_message)) {
        adapter->model->records[index].build_state = 3;
        return 0;
    }
    adapter->model->records[index].build_state = 2;
    return 1;
}

static int field_compare(const void *left, const void *right) {
    return strcmp(((const CrexxContractField *)left)->name,
                  ((const CrexxContractField *)right)->name);
}

static int record_compare(const void *left, const void *right) {
    return strcmp(((const CrexxContractRecord *)left)->name,
                  ((const CrexxContractRecord *)right)->name);
}

static int schema_compare(const void *left, const void *right) {
    const CrexxContractSchema *a = (const CrexxContractSchema *)left;
    const CrexxContractSchema *b = (const CrexxContractSchema *)right;
    if (a->record_name && b->record_name) return strcmp(a->record_name, b->record_name);
    return (int)a->kind - (int)b->kind;
}

int crexx_contract_model_from_rxbin(const CrexxContractOptions *options,
                                    CrexxContractModel *model,
                                    char **error_message) {
    FILE *input = 0;
    module_file *module = 0;
    char *graph_error = 0;
    char *owner_name = 0;
    const char *member_name;
    const char *dot;
    RxGraphId owner;
    RxMemberId operation_member = RX_GRAPH_NONE;
    RxGraphMemberView member;
    rx_callable_signature signature;
    ContractAdapter adapter;
    size_t i;
    int ok = 0;

    if (error_message) *error_message = 0;
    crexx_contract_model_init(model);
    memset(&adapter, 0, sizeof(adapter));
    if (!options || !options->rxbin_path || !options->operation || !options->contract_version) {
        crexx_contract_set_error(error_message, "missing required contract options");
        return 0;
    }
    dot = strrchr(options->operation, '.');
    if (!dot || dot == options->operation || !dot[1]) {
        crexx_contract_set_error(error_message,
                                 "operation must be a fully qualified interface member");
        return 0;
    }
    owner_name = (char *)malloc((size_t)(dot - options->operation) + 1u);
    if (!owner_name) {
        crexx_contract_set_error(error_message, "out of memory parsing operation");
        return 0;
    }
    memcpy(owner_name, options->operation, (size_t)(dot - options->operation));
    owner_name[dot - options->operation] = 0;
    member_name = dot + 1;

    input = fopen(options->rxbin_path, "rb");
    if (!input) {
        crexx_contract_set_error(error_message, "cannot open RXBIN '%s'", options->rxbin_path);
        goto cleanup;
    }
    if (read_module(&module, input) != 0 || !module) {
        crexx_contract_set_error(error_message,
                                 "cannot read RXBIN '%s': %s",
                                 options->rxbin_path,
                                 rxbin_last_error());
        goto cleanup;
    }
    fclose(input);
    input = 0;
    if (!module->semantic_graph ||
        !rx_graph_validate(module->semantic_graph, &graph_error)) {
        crexx_contract_set_error(error_message,
                                 "RXBIN semantic graph is unavailable or invalid: %s",
                                 graph_error ? graph_error : "missing graph");
        goto cleanup;
    }
    adapter.options = options;
    adapter.graph = module->semantic_graph;
    adapter.model = model;
    adapter.nullable_used = (unsigned char *)calloc(options->nullable_count, 1u);
    adapter.optional_used = (unsigned char *)calloc(options->optional_count, 1u);
    if ((options->nullable_count && !adapter.nullable_used) ||
        (options->optional_count && !adapter.optional_used)) {
        crexx_contract_set_error(error_message, "out of memory tracking field options");
        goto cleanup;
    }

    owner = rx_graph_find_type(adapter.graph, owner_name);
    if (owner == RX_GRAPH_NONE || rx_graph_type_kind(adapter.graph, owner) != RX_GRAPH_TYPE_INTERFACE) {
        crexx_contract_set_error(error_message,
                                 "operation owner '%s' is not a compiled Level B interface",
                                 owner_name);
        goto cleanup;
    }
    for (i = 0; i < rx_graph_declaration_count(adapter.graph, owner); i++) {
        RxGraphDeclarationView declaration;
        RxGraphMemberView candidate;
        if (!rx_graph_declaration(adapter.graph, owner, i, &declaration) ||
            !rx_graph_member(adapter.graph, declaration.member, &candidate)) continue;
        if (strcmp(candidate.name, member_name) == 0) {
            if (operation_member != RX_GRAPH_NONE) {
                crexx_contract_set_error(error_message,
                                         "operation '%s' is overloaded or ambiguous",
                                         options->operation);
                goto cleanup;
            }
            operation_member = declaration.member;
            member = candidate;
        }
    }
    if (operation_member == RX_GRAPH_NONE) {
        crexx_contract_set_error(error_message,
                                 "operation '%s' is not declared by its interface",
                                 options->operation);
        goto cleanup;
    }
    if ((member.flags & RX_GRAPH_MEMBER_FACTORY) ||
        (member.flags & RX_GRAPH_MEMBER_FINAL) ||
        !(member.flags & RX_GRAPH_MEMBER_METHOD)) {
        crexx_contract_set_error(error_message,
                                 "operation '%s' must be an abstract interface method",
                                 options->operation);
        goto cleanup;
    }
    if (!rx_sig_parse_descriptor(member.descriptor, &signature)) {
        crexx_contract_set_error(error_message, "malformed operation signature metadata");
        goto cleanup;
    }

    model->contract_version = contract_strdup(options->contract_version);
    model->operation = contract_strdup(options->operation);
    model->input_count = signature.arg_count;
    model->inputs = (CrexxContractField *)calloc(model->input_count, sizeof(*model->inputs));
    if (!model->contract_version || !model->operation ||
        (model->input_count && !model->inputs)) {
        crexx_contract_set_error(error_message, "out of memory building operation contract");
        rx_sig_free(&signature);
        goto cleanup;
    }
    for (i = 0; i < signature.arg_count; i++) {
        if (signature.args[i].is_ref || signature.args[i].is_vararg) {
            crexx_contract_set_error(error_message,
                                     "operation input '%s' uses a reference or vararg outside format 1",
                                     signature.args[i].name ? signature.args[i].name : "");
            rx_sig_free(&signature);
            goto cleanup;
        }
        if (!signature.args[i].name || !*signature.args[i].name) {
            crexx_contract_set_error(error_message, "operation has an unnamed input");
            rx_sig_free(&signature);
            goto cleanup;
        }
        model->inputs[i].name = contract_strdup(signature.args[i].name);
        model->inputs[i].required = !signature.args[i].is_optional;
        model->inputs[i].nullable = 0;
        if (!model->inputs[i].name ||
            !build_schema(&adapter,
                          signature.args[i].type,
                          owner_name,
                          0,
                          &model->inputs[i].schema,
                          error_message)) {
            rx_sig_free(&signature);
            goto cleanup;
        }
    }
    if (!build_schema(&adapter,
                      signature.return_type,
                      owner_name,
                      1,
                      &model->result,
                      error_message)) {
        rx_sig_free(&signature);
        goto cleanup;
    }
    rx_sig_free(&signature);

    model->error_count = options->error_count;
    model->errors = (CrexxContractSchema *)calloc(model->error_count, sizeof(*model->errors));
    if (model->error_count && !model->errors) {
        crexx_contract_set_error(error_message, "out of memory adding structured errors");
        goto cleanup;
    }
    for (i = 0; i < model->error_count; i++) {
        size_t prior;
        if (!build_schema(&adapter,
                          options->error_types[i],
                          owner_name,
                          0,
                          &model->errors[i],
                          error_message) ||
            model->errors[i].kind != CREXX_CONTRACT_RECORD) {
            if (!*error_message) {
                crexx_contract_set_error(error_message,
                                         "error type '%s' must be a payload interface",
                                         options->error_types[i]);
            }
            goto cleanup;
        }
        for (prior = 0; prior < i; prior++) {
            if (schema_equal(&model->errors[prior], &model->errors[i])) {
                crexx_contract_set_error(error_message,
                                         "error type '%s' is declared more than once",
                                         model->errors[i].record_name);
                goto cleanup;
            }
        }
    }

    for (i = 0; i < options->nullable_count; i++) {
        if (!adapter.nullable_used[i]) {
            crexx_contract_set_error(error_message,
                                     "nullable field '%s' is not present in the reachable payload contract",
                                     options->nullable_fields[i]);
            goto cleanup;
        }
    }
    for (i = 0; i < options->optional_count; i++) {
        if (!adapter.optional_used[i]) {
            crexx_contract_set_error(error_message,
                                     "optional field '%s' is not present in the reachable payload contract",
                                     options->optional_fields[i]);
            goto cleanup;
        }
    }
    for (i = 0; i < model->record_count; i++) {
        qsort(model->records[i].fields,
              model->records[i].field_count,
              sizeof(*model->records[i].fields),
              field_compare);
        model->records[i].build_state = 0;
    }
    qsort(model->records, model->record_count, sizeof(*model->records), record_compare);
    qsort(model->errors, model->error_count, sizeof(*model->errors), schema_compare);
    ok = 1;

cleanup:
    free(owner_name);
    free(graph_error);
    free(adapter.nullable_used);
    free(adapter.optional_used);
    if (input) fclose(input);
    if (module) free_module(module);
    if (!ok) crexx_contract_model_free(model);
    return ok;
}
