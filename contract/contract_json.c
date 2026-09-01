/* cREXX License (MIT) */

#include "contract_model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int write_indent(FILE *output, unsigned depth) {
    unsigned i;
    for (i = 0; i < depth; i++) {
        if (fputs("  ", output) == EOF) return 0;
    }
    return 1;
}

static int write_json_string(FILE *output, const char *text) {
    const unsigned char *cursor = (const unsigned char *)text;
    if (fputc('"', output) == EOF) return 0;
    while (*cursor) {
        switch (*cursor) {
            case '"': if (fputs("\\\"", output) == EOF) return 0; break;
            case '\\': if (fputs("\\\\", output) == EOF) return 0; break;
            case '\b': if (fputs("\\b", output) == EOF) return 0; break;
            case '\f': if (fputs("\\f", output) == EOF) return 0; break;
            case '\n': if (fputs("\\n", output) == EOF) return 0; break;
            case '\r': if (fputs("\\r", output) == EOF) return 0; break;
            case '\t': if (fputs("\\t", output) == EOF) return 0; break;
            default:
                if (*cursor < 0x20u) {
                    if (fprintf(output, "\\u%04x", (unsigned)*cursor) < 0) return 0;
                } else if (fputc((int)*cursor, output) == EOF) {
                    return 0;
                }
        }
        cursor++;
    }
    return fputc('"', output) != EOF;
}

static int write_key(FILE *output, unsigned depth, const char *key) {
    return write_indent(output, depth) && write_json_string(output, key) &&
           fputs(": ", output) != EOF;
}

static int write_schema(FILE *output,
                        const CrexxContractSchema *schema,
                        unsigned depth) {
    (void)depth;
    if (fputc('{', output) == EOF) return 0;
    switch (schema->kind) {
        case CREXX_CONTRACT_VOID:
            if (fputs("\"kind\": \"void\"", output) == EOF) return 0;
            break;
        case CREXX_CONTRACT_STRING:
            if (fputs("\"kind\": \"string\"", output) == EOF) return 0;
            break;
        case CREXX_CONTRACT_BOOLEAN:
            if (fputs("\"kind\": \"boolean\"", output) == EOF) return 0;
            break;
        case CREXX_CONTRACT_INTEGER:
            if (fputs("\"kind\": \"integer\", \"minimum\": -9223372036854775808, "
                      "\"maximum\": 9223372036854775807", output) == EOF) return 0;
            break;
        case CREXX_CONTRACT_NUMBER:
            if (fputs("\"kind\": \"number\", \"finite\": true", output) == EOF) return 0;
            break;
        case CREXX_CONTRACT_DECIMAL:
            if (fputs("\"kind\": \"string\", \"encoding\": \"crexx-decimal\"", output) == EOF) return 0;
            break;
        case CREXX_CONTRACT_BINARY:
            if (fputs("\"kind\": \"string\", \"contentEncoding\": \"base64\"", output) == EOF) return 0;
            break;
        case CREXX_CONTRACT_ARRAY:
            if (fputs("\"kind\": \"array\", \"items\": ", output) == EOF ||
                !schema->items || !write_schema(output, schema->items, depth + 1u)) return 0;
            break;
        case CREXX_CONTRACT_RECORD:
            if (fputs("\"kind\": \"record\", \"type\": ", output) == EOF ||
                !write_json_string(output, schema->record_name)) return 0;
            break;
    }
    return fputc('}', output) != EOF;
}

static int write_field(FILE *output,
                       const CrexxContractField *field,
                       unsigned depth) {
    if (!write_indent(output, depth) || fputs("{\"name\": ", output) == EOF ||
        !write_json_string(output, field->name) ||
        fprintf(output,
                ", \"required\": %s, \"nullable\": %s, \"schema\": ",
                field->required ? "true" : "false",
                field->nullable ? "true" : "false") < 0 ||
        !write_schema(output, &field->schema, depth)) return 0;
    return fputc('}', output) != EOF;
}

int crexx_contract_write_json(const CrexxContractModel *model,
                              const char *output_path,
                              char **error_message) {
    FILE *output;
    size_t i;
    size_t j;
    int ok = 1;

    output = fopen(output_path, "wb");
    if (!output) {
        crexx_contract_set_error(error_message,
                                 "cannot open contract output '%s'",
                                 output_path);
        return 0;
    }
    ok = fputs("{\n", output) != EOF &&
         write_key(output, 1u, "format") &&
         write_json_string(output, "crexx.operation-contract") &&
         fputs(",\n", output) != EOF &&
         write_key(output, 1u, "formatVersion") &&
         fputs("1,\n", output) != EOF &&
         write_key(output, 1u, "contractVersion") &&
         write_json_string(output, model->contract_version) &&
         fputs(",\n", output) != EOF &&
         write_key(output, 1u, "operation") &&
         write_json_string(output, model->operation) &&
         fputs(",\n", output) != EOF &&
         write_key(output, 1u, "input") &&
         fputs("[", output) != EOF;
    if (ok && model->input_count) ok = fputc('\n', output) != EOF;
    for (i = 0; ok && i < model->input_count; i++) {
        ok = write_field(output, &model->inputs[i], 2u) &&
             fputs(i + 1u < model->input_count ? ",\n" : "\n", output) != EOF;
    }
    if (ok) ok = write_indent(output, 1u) && fputs("],\n", output) != EOF &&
                 write_key(output, 1u, "result") &&
                 write_schema(output, &model->result, 1u) &&
                 fputs(",\n", output) != EOF &&
                 write_key(output, 1u, "errors") &&
                 fputs("[", output) != EOF;
    if (ok && model->error_count) ok = fputc('\n', output) != EOF;
    for (i = 0; ok && i < model->error_count; i++) {
        ok = write_indent(output, 2u) &&
             write_schema(output, &model->errors[i], 2u) &&
             fputs(i + 1u < model->error_count ? ",\n" : "\n", output) != EOF;
    }
    if (ok) ok = write_indent(output, 1u) && fputs("],\n", output) != EOF &&
                 write_key(output, 1u, "types") && fputs("[", output) != EOF;
    if (ok && model->record_count) ok = fputc('\n', output) != EOF;
    for (i = 0; ok && i < model->record_count; i++) {
        const CrexxContractRecord *record = &model->records[i];
        ok = write_indent(output, 2u) && fputs("{\"name\": ", output) != EOF &&
             write_json_string(output, record->name) &&
             fputs(", \"kind\": \"record\", \"fields\": [", output) != EOF;
        if (ok && record->field_count) ok = fputc('\n', output) != EOF;
        for (j = 0; ok && j < record->field_count; j++) {
            ok = write_field(output, &record->fields[j], 3u) &&
                 fputs(j + 1u < record->field_count ? ",\n" : "\n", output) != EOF;
        }
        if (ok) ok = write_indent(output, 2u) && fputs("]}", output) != EOF;
        if (ok) ok = fputs(i + 1u < model->record_count ? ",\n" : "\n", output) != EOF;
    }
    if (ok) ok = write_indent(output, 1u) && fputs("]\n}\n", output) != EOF;
    if (fclose(output) != 0) ok = 0;
    if (!ok) {
        crexx_contract_set_error(error_message,
                                 "failed writing contract output '%s'",
                                 output_path);
    }
    return ok;
}
