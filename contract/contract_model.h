/* cREXX License (MIT) */

#ifndef CREXX_CONTRACT_MODEL_H
#define CREXX_CONTRACT_MODEL_H

#include <stddef.h>

typedef enum CrexxContractSchemaKind {
    CREXX_CONTRACT_VOID,
    CREXX_CONTRACT_STRING,
    CREXX_CONTRACT_BOOLEAN,
    CREXX_CONTRACT_INTEGER,
    CREXX_CONTRACT_NUMBER,
    CREXX_CONTRACT_DECIMAL,
    CREXX_CONTRACT_BINARY,
    CREXX_CONTRACT_ARRAY,
    CREXX_CONTRACT_RECORD
} CrexxContractSchemaKind;

typedef struct CrexxContractSchema {
    CrexxContractSchemaKind kind;
    char *record_name;
    struct CrexxContractSchema *items;
} CrexxContractSchema;

typedef struct CrexxContractField {
    char *name;
    int required;
    int nullable;
    CrexxContractSchema schema;
} CrexxContractField;

typedef struct CrexxContractRecord {
    char *name;
    CrexxContractField *fields;
    size_t field_count;
    int build_state;
} CrexxContractRecord;

typedef struct CrexxContractModel {
    char *contract_version;
    char *operation;
    CrexxContractField *inputs;
    size_t input_count;
    CrexxContractSchema result;
    CrexxContractSchema *errors;
    size_t error_count;
    CrexxContractRecord *records;
    size_t record_count;
} CrexxContractModel;

typedef struct CrexxContractOptions {
    const char *rxbin_path;
    const char *operation;
    const char *contract_version;
    const char *const *nullable_fields;
    size_t nullable_count;
    const char *const *optional_fields;
    size_t optional_count;
    const char *const *error_types;
    size_t error_count;
} CrexxContractOptions;

void crexx_contract_model_init(CrexxContractModel *model);
void crexx_contract_model_free(CrexxContractModel *model);

int crexx_contract_model_from_rxbin(const CrexxContractOptions *options,
                                    CrexxContractModel *model,
                                    char **error_message);

int crexx_contract_write_json(const CrexxContractModel *model,
                              const char *output_path,
                              char **error_message);

int crexx_contract_check_previous(const CrexxContractModel *model,
                                  const char *previous_path,
                                  char **error_message);

void crexx_contract_set_error(char **error_message, const char *format, ...);

#endif
