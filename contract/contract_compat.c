/* cREXX License (MIT) */

#include "contract_model.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum JsonTokenKind {
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_PRIMITIVE
} JsonTokenKind;

typedef struct JsonToken {
    JsonTokenKind kind;
    size_t start;
    size_t end;
    size_t child_count;
    int parent;
} JsonToken;

typedef struct JsonDocument {
    char *text;
    JsonToken *tokens;
    size_t token_count;
} JsonDocument;

typedef enum CompatEntryKind {
    COMPAT_INPUT,
    COMPAT_RESULT,
    COMPAT_ERROR,
    COMPAT_FIELD
} CompatEntryKind;

typedef struct CompatEntry {
    CompatEntryKind kind;
    char *key;
    char *schema;
    int required;
    int nullable;
} CompatEntry;

typedef struct CompatContract {
    char *version;
    char *operation;
    CompatEntry *entries;
    size_t entry_count;
} CompatContract;

typedef struct SemanticVersion {
    unsigned major;
    unsigned minor;
    unsigned patch;
} SemanticVersion;

static char *compat_strdup(const char *text) {
    size_t length = strlen(text);
    char *copy = (char *)malloc(length + 1u);
    if (!copy) return 0;
    memcpy(copy, text, length + 1u);
    return copy;
}

static void json_document_free(JsonDocument *document) {
    if (!document) return;
    free(document->text);
    free(document->tokens);
    memset(document, 0, sizeof(*document));
}

static int json_add_token(JsonDocument *document,
                          JsonTokenKind kind,
                          size_t start,
                          int parent,
                          int *index,
                          char **error_message) {
    JsonToken *grown = (JsonToken *)realloc(document->tokens,
                                            (document->token_count + 1u) * sizeof(*grown));
    if (!grown) {
        crexx_contract_set_error(error_message, "out of memory parsing previous contract");
        return 0;
    }
    document->tokens = grown;
    *index = (int)document->token_count++;
    document->tokens[*index].kind = kind;
    document->tokens[*index].start = start;
    document->tokens[*index].end = 0u;
    document->tokens[*index].child_count = 0u;
    document->tokens[*index].parent = parent;
    if (parent >= 0) document->tokens[parent].child_count++;
    return 1;
}

static int json_hex(unsigned char value) {
    return isdigit(value) || (value >= 'a' && value <= 'f') ||
           (value >= 'A' && value <= 'F');
}

static void json_skip_space(const char *text, size_t length, size_t *cursor) {
    while (*cursor < length &&
           (text[*cursor] == ' ' || text[*cursor] == '\t' ||
            text[*cursor] == '\r' || text[*cursor] == '\n')) (*cursor)++;
}

static int json_validate_string_syntax(const char *text,
                                       size_t length,
                                       size_t *cursor) {
    if (*cursor >= length || text[*cursor] != '"') return 0;
    (*cursor)++;
    while (*cursor < length) {
        unsigned char ch = (unsigned char)text[(*cursor)++];
        if (ch == '"') return 1;
        if (ch < 0x20u) return 0;
        if (ch == '\\') {
            size_t hex;
            if (*cursor >= length || !strchr("\"\\/bfnrtu", text[*cursor])) return 0;
            if (text[(*cursor)++] == 'u') {
                for (hex = 0; hex < 4u; hex++) {
                    if (*cursor >= length ||
                        !json_hex((unsigned char)text[(*cursor)++])) return 0;
                }
            }
        } else if (ch >= 0x80u) {
            unsigned remaining;
            unsigned codepoint;
            unsigned minimum;
            if ((ch & 0xe0u) == 0xc0u) {
                remaining = 1u;
                codepoint = ch & 0x1fu;
                minimum = 0x80u;
            } else if ((ch & 0xf0u) == 0xe0u) {
                remaining = 2u;
                codepoint = ch & 0x0fu;
                minimum = 0x800u;
            } else if ((ch & 0xf8u) == 0xf0u) {
                remaining = 3u;
                codepoint = ch & 0x07u;
                minimum = 0x10000u;
            } else {
                return 0;
            }
            while (remaining--) {
                unsigned char continuation;
                if (*cursor >= length) return 0;
                continuation = (unsigned char)text[(*cursor)++];
                if ((continuation & 0xc0u) != 0x80u) return 0;
                codepoint = (codepoint << 6u) | (continuation & 0x3fu);
            }
            if (codepoint < minimum || codepoint > 0x10ffffu ||
                (codepoint >= 0xd800u && codepoint <= 0xdfffu)) return 0;
        }
    }
    return 0;
}

static int json_validate_number_syntax(const char *text,
                                       size_t length,
                                       size_t *cursor) {
    if (*cursor < length && text[*cursor] == '-') (*cursor)++;
    if (*cursor >= length) return 0;
    if (text[*cursor] == '0') {
        (*cursor)++;
        if (*cursor < length && isdigit((unsigned char)text[*cursor])) return 0;
    } else {
        if (!isdigit((unsigned char)text[*cursor])) return 0;
        while (*cursor < length && isdigit((unsigned char)text[*cursor])) (*cursor)++;
    }
    if (*cursor < length && text[*cursor] == '.') {
        (*cursor)++;
        if (*cursor >= length || !isdigit((unsigned char)text[*cursor])) return 0;
        while (*cursor < length && isdigit((unsigned char)text[*cursor])) (*cursor)++;
    }
    if (*cursor < length && (text[*cursor] == 'e' || text[*cursor] == 'E')) {
        (*cursor)++;
        if (*cursor < length && (text[*cursor] == '+' || text[*cursor] == '-')) (*cursor)++;
        if (*cursor >= length || !isdigit((unsigned char)text[*cursor])) return 0;
        while (*cursor < length && isdigit((unsigned char)text[*cursor])) (*cursor)++;
    }
    return 1;
}

static int json_validate_value_syntax(const char *text,
                                      size_t length,
                                      size_t *cursor,
                                      unsigned depth) {
    if (depth > 128u) return 0;
    json_skip_space(text, length, cursor);
    if (*cursor >= length) return 0;
    if (text[*cursor] == '"') return json_validate_string_syntax(text, length, cursor);
    if (text[*cursor] == '{') {
        (*cursor)++;
        json_skip_space(text, length, cursor);
        if (*cursor < length && text[*cursor] == '}') {
            (*cursor)++;
            return 1;
        }
        for (;;) {
            if (!json_validate_string_syntax(text, length, cursor)) return 0;
            json_skip_space(text, length, cursor);
            if (*cursor >= length || text[(*cursor)++] != ':') return 0;
            if (!json_validate_value_syntax(text, length, cursor, depth + 1u)) return 0;
            json_skip_space(text, length, cursor);
            if (*cursor < length && text[*cursor] == '}') {
                (*cursor)++;
                return 1;
            }
            if (*cursor >= length || text[(*cursor)++] != ',') return 0;
            json_skip_space(text, length, cursor);
        }
    }
    if (text[*cursor] == '[') {
        (*cursor)++;
        json_skip_space(text, length, cursor);
        if (*cursor < length && text[*cursor] == ']') {
            (*cursor)++;
            return 1;
        }
        for (;;) {
            if (!json_validate_value_syntax(text, length, cursor, depth + 1u)) return 0;
            json_skip_space(text, length, cursor);
            if (*cursor < length && text[*cursor] == ']') {
                (*cursor)++;
                return 1;
            }
            if (*cursor >= length || text[(*cursor)++] != ',') return 0;
        }
    }
    if (length - *cursor >= 4u && strncmp(text + *cursor, "true", 4u) == 0) {
        *cursor += 4u;
        return 1;
    }
    if (length - *cursor >= 5u && strncmp(text + *cursor, "false", 5u) == 0) {
        *cursor += 5u;
        return 1;
    }
    if (length - *cursor >= 4u && strncmp(text + *cursor, "null", 4u) == 0) {
        *cursor += 4u;
        return 1;
    }
    return json_validate_number_syntax(text, length, cursor);
}

static int json_validate_document_syntax(const char *text, char **error_message) {
    size_t length = strlen(text);
    size_t cursor = 0u;
    if (!json_validate_value_syntax(text, length, &cursor, 0u)) {
        crexx_contract_set_error(error_message,
                                 "malformed previous contract JSON at byte %lu",
                                 (unsigned long)cursor);
        return 0;
    }
    json_skip_space(text, length, &cursor);
    if (cursor != length) {
        crexx_contract_set_error(error_message,
                                 "malformed previous contract JSON at byte %lu",
                                 (unsigned long)cursor);
        return 0;
    }
    return 1;
}

static int json_tokenize(JsonDocument *document, char **error_message) {
    size_t length = strlen(document->text);
    size_t cursor = 0u;
    int parent = -1;

    if (!json_validate_document_syntax(document->text, error_message)) return 0;
    while (cursor < length) {
        unsigned char ch = (unsigned char)document->text[cursor];
        int index;
        if (isspace(ch) || ch == ':' || ch == ',') {
            cursor++;
            continue;
        }
        if (ch == '{' || ch == '[') {
            if (!json_add_token(document,
                                ch == '{' ? JSON_OBJECT : JSON_ARRAY,
                                cursor,
                                parent,
                                &index,
                                error_message)) return 0;
            parent = index;
            cursor++;
            continue;
        }
        if (ch == '}' || ch == ']') {
            JsonTokenKind expected = ch == '}' ? JSON_OBJECT : JSON_ARRAY;
            if (parent < 0 || document->tokens[parent].kind != expected) {
                crexx_contract_set_error(error_message,
                                         "malformed previous contract JSON at byte %lu",
                                         (unsigned long)cursor);
                return 0;
            }
            document->tokens[parent].end = cursor + 1u;
            parent = document->tokens[parent].parent;
            cursor++;
            continue;
        }
        if (ch == '"') {
            size_t start = ++cursor;
            int closed = 0;
            while (cursor < length) {
                ch = (unsigned char)document->text[cursor];
                if (ch == '"') {
                    closed = 1;
                    break;
                }
                if (ch < 0x20u) break;
                if (ch == '\\') {
                    cursor++;
                    if (cursor >= length ||
                        !strchr("\"\\/bfnrtu", document->text[cursor])) break;
                    if (document->text[cursor] == 'u') {
                        size_t hex;
                        for (hex = 0; hex < 4u; hex++) {
                            cursor++;
                            if (cursor >= length ||
                                !json_hex((unsigned char)document->text[cursor])) break;
                        }
                        if (hex != 4u) break;
                    }
                }
                cursor++;
            }
            if (!closed ||
                !json_add_token(document, JSON_STRING, start, parent, &index, error_message)) {
                if (!*error_message) {
                    crexx_contract_set_error(error_message,
                                             "malformed JSON string in previous contract");
                }
                return 0;
            }
            document->tokens[index].end = cursor;
            cursor++;
            continue;
        }
        {
            size_t start = cursor;
            while (cursor < length &&
                   !isspace((unsigned char)document->text[cursor]) &&
                   !strchr(",:]}", document->text[cursor])) cursor++;
            if (cursor == start ||
                !json_add_token(document, JSON_PRIMITIVE, start, parent, &index, error_message)) {
                if (!*error_message) {
                    crexx_contract_set_error(error_message,
                                             "malformed primitive in previous contract");
                }
                return 0;
            }
            document->tokens[index].end = cursor;
        }
    }
    if (parent >= 0 || document->token_count == 0u ||
        document->tokens[0].kind != JSON_OBJECT ||
        document->tokens[0].end == 0u) {
        crexx_contract_set_error(error_message, "previous contract is not a complete JSON object");
        return 0;
    }
    return 1;
}

static int json_read_file(const char *path,
                          JsonDocument *document,
                          char **error_message) {
    FILE *input;
    long length;
    size_t read_count;
    memset(document, 0, sizeof(*document));
    input = fopen(path, "rb");
    if (!input) {
        crexx_contract_set_error(error_message, "cannot open previous contract '%s'", path);
        return 0;
    }
    if (fseek(input, 0, SEEK_END) != 0 || (length = ftell(input)) < 0 ||
        length > 16 * 1024 * 1024 || fseek(input, 0, SEEK_SET) != 0) {
        fclose(input);
        crexx_contract_set_error(error_message,
                                 "previous contract '%s' is unreadable or exceeds 16 MiB",
                                 path);
        return 0;
    }
    document->text = (char *)malloc((size_t)length + 1u);
    if (!document->text) {
        fclose(input);
        crexx_contract_set_error(error_message, "out of memory reading previous contract");
        return 0;
    }
    read_count = fread(document->text, 1u, (size_t)length, input);
    fclose(input);
    if (read_count != (size_t)length) {
        json_document_free(document);
        crexx_contract_set_error(error_message, "failed reading previous contract '%s'", path);
        return 0;
    }
    document->text[length] = 0;
    if (!json_tokenize(document, error_message)) {
        json_document_free(document);
        return 0;
    }
    return 1;
}

static size_t json_token_next(const JsonDocument *document, size_t index) {
    size_t child;
    size_t next = index + 1u;
    for (child = 0; child < document->tokens[index].child_count; child++) {
        next = json_token_next(document, next);
    }
    return next;
}

static int json_token_raw_equal(const JsonDocument *document,
                                size_t index,
                                const char *text) {
    size_t length = document->tokens[index].end - document->tokens[index].start;
    return strlen(text) == length &&
           memcmp(document->text + document->tokens[index].start, text, length) == 0;
}

static int json_object_get(const JsonDocument *document,
                           size_t object,
                           const char *key,
                           size_t *value) {
    size_t child;
    size_t index;
    if (document->tokens[object].kind != JSON_OBJECT ||
        document->tokens[object].child_count % 2u) return 0;
    index = object + 1u;
    for (child = 0; child < document->tokens[object].child_count; child += 2u) {
        size_t key_index = index;
        size_t value_index = key_index + 1u;
        if (document->tokens[key_index].kind != JSON_STRING) return 0;
        if (json_token_raw_equal(document, key_index, key)) {
            *value = value_index;
            return 1;
        }
        index = json_token_next(document, value_index);
    }
    return 0;
}

static int append_utf8(char **buffer,
                       size_t *length,
                       size_t *capacity,
                       unsigned codepoint) {
    unsigned char encoded[4];
    size_t count;
    char *grown;
    if (codepoint <= 0x7fu) {
        encoded[0] = (unsigned char)codepoint;
        count = 1u;
    } else if (codepoint <= 0x7ffu) {
        encoded[0] = (unsigned char)(0xc0u | (codepoint >> 6u));
        encoded[1] = (unsigned char)(0x80u | (codepoint & 0x3fu));
        count = 2u;
    } else if (codepoint <= 0xffffu) {
        encoded[0] = (unsigned char)(0xe0u | (codepoint >> 12u));
        encoded[1] = (unsigned char)(0x80u | ((codepoint >> 6u) & 0x3fu));
        encoded[2] = (unsigned char)(0x80u | (codepoint & 0x3fu));
        count = 3u;
    } else {
        encoded[0] = (unsigned char)(0xf0u | (codepoint >> 18u));
        encoded[1] = (unsigned char)(0x80u | ((codepoint >> 12u) & 0x3fu));
        encoded[2] = (unsigned char)(0x80u | ((codepoint >> 6u) & 0x3fu));
        encoded[3] = (unsigned char)(0x80u | (codepoint & 0x3fu));
        count = 4u;
    }
    if (*length + count + 1u > *capacity) {
        size_t next_capacity = *capacity ? *capacity * 2u : 32u;
        while (next_capacity < *length + count + 1u) next_capacity *= 2u;
        grown = (char *)realloc(*buffer, next_capacity);
        if (!grown) return 0;
        *buffer = grown;
        *capacity = next_capacity;
    }
    memcpy(*buffer + *length, encoded, count);
    *length += count;
    (*buffer)[*length] = 0;
    return 1;
}

static int append_byte(char **buffer,
                       size_t *length,
                       size_t *capacity,
                       unsigned char value) {
    char *grown;
    if (*length + 2u > *capacity) {
        size_t next_capacity = *capacity ? *capacity * 2u : 32u;
        grown = (char *)realloc(*buffer, next_capacity);
        if (!grown) return 0;
        *buffer = grown;
        *capacity = next_capacity;
    }
    (*buffer)[(*length)++] = (char)value;
    (*buffer)[*length] = 0;
    return 1;
}

static unsigned json_hex_value(unsigned char value) {
    if (isdigit(value)) return value - '0';
    if (value >= 'a' && value <= 'f') return value - 'a' + 10u;
    return value - 'A' + 10u;
}

static char *json_string_value(const JsonDocument *document, size_t index) {
    const JsonToken *token = &document->tokens[index];
    size_t cursor;
    size_t length = 0u;
    size_t capacity = 0u;
    char *result = 0;
    if (token->kind != JSON_STRING) return 0;
    for (cursor = token->start; cursor < token->end; cursor++) {
        unsigned char ch = (unsigned char)document->text[cursor];
        if (ch != '\\') {
            if (!append_byte(&result, &length, &capacity, ch)) goto fail;
            continue;
        }
        cursor++;
        ch = (unsigned char)document->text[cursor];
        if (ch == 'u') {
            unsigned codepoint = 0u;
            size_t hex;
            for (hex = 0; hex < 4u; hex++) {
                codepoint = (codepoint << 4u) |
                            json_hex_value((unsigned char)document->text[++cursor]);
            }
            if (codepoint >= 0xd800u && codepoint <= 0xdbffu &&
                cursor + 6u < token->end && document->text[cursor + 1u] == '\\' &&
                document->text[cursor + 2u] == 'u') {
                unsigned low = 0u;
                cursor += 2u;
                for (hex = 0; hex < 4u; hex++) {
                    low = (low << 4u) |
                          json_hex_value((unsigned char)document->text[++cursor]);
                }
                if (low < 0xdc00u || low > 0xdfffu) goto fail;
                codepoint = 0x10000u + ((codepoint - 0xd800u) << 10u) +
                            (low - 0xdc00u);
            } else if (codepoint >= 0xd800u && codepoint <= 0xdfffu) {
                goto fail;
            }
            if (!append_utf8(&result, &length, &capacity, codepoint)) goto fail;
        } else {
            switch (ch) {
                case '"': case '\\': case '/': break;
                case 'b': ch = '\b'; break;
                case 'f': ch = '\f'; break;
                case 'n': ch = '\n'; break;
                case 'r': ch = '\r'; break;
                case 't': ch = '\t'; break;
                default: goto fail;
            }
            if (!append_utf8(&result, &length, &capacity, ch)) goto fail;
        }
    }
    if (!result) result = compat_strdup("");
    return result;
fail:
    free(result);
    return 0;
}

static int json_boolean(const JsonDocument *document, size_t index, int *value) {
    if (document->tokens[index].kind != JSON_PRIMITIVE) return 0;
    if (json_token_raw_equal(document, index, "true")) *value = 1;
    else if (json_token_raw_equal(document, index, "false")) *value = 0;
    else return 0;
    return 1;
}

static char *format_text(const char *format, const char *left, const char *right) {
    int length = snprintf(0, 0, format, left, right);
    char *result;
    if (length < 0) return 0;
    result = (char *)malloc((size_t)length + 1u);
    if (result) snprintf(result, (size_t)length + 1u, format, left, right);
    return result;
}

static char *schema_text_from_model(const CrexxContractSchema *schema) {
    char *item;
    char *result;
    switch (schema->kind) {
        case CREXX_CONTRACT_VOID: return compat_strdup("void");
        case CREXX_CONTRACT_STRING: return compat_strdup("string");
        case CREXX_CONTRACT_BOOLEAN: return compat_strdup("boolean");
        case CREXX_CONTRACT_INTEGER: return compat_strdup("integer:i64");
        case CREXX_CONTRACT_NUMBER: return compat_strdup("number:finite");
        case CREXX_CONTRACT_DECIMAL: return compat_strdup("string:crexx-decimal");
        case CREXX_CONTRACT_BINARY: return compat_strdup("string:base64");
        case CREXX_CONTRACT_RECORD: return format_text("record:%s%s", schema->record_name, "");
        case CREXX_CONTRACT_ARRAY:
            item = schema_text_from_model(schema->items);
            if (!item) return 0;
            result = format_text("array:(%s)%s", item, "");
            free(item);
            return result;
    }
    return 0;
}

static char *schema_text_from_json(const JsonDocument *document,
                                   size_t schema,
                                   char **error_message) {
    size_t kind_index;
    size_t value_index;
    char *kind;
    char *value = 0;
    char *result = 0;
    if (document->tokens[schema].kind != JSON_OBJECT ||
        !json_object_get(document, schema, "kind", &kind_index) ||
        !(kind = json_string_value(document, kind_index))) {
        crexx_contract_set_error(error_message, "previous contract contains an invalid schema object");
        return 0;
    }
    if (strcmp(kind, "void") == 0) result = compat_strdup("void");
    else if (strcmp(kind, "boolean") == 0) result = compat_strdup("boolean");
    else if (strcmp(kind, "integer") == 0) {
        size_t minimum;
        size_t maximum;
        if (json_object_get(document, schema, "minimum", &minimum) &&
            json_object_get(document, schema, "maximum", &maximum) &&
            json_token_raw_equal(document, minimum, "-9223372036854775808") &&
            json_token_raw_equal(document, maximum, "9223372036854775807")) {
            result = compat_strdup("integer:i64");
        }
    } else if (strcmp(kind, "number") == 0) {
        int finite;
        if (json_object_get(document, schema, "finite", &value_index) &&
            json_boolean(document, value_index, &finite) && finite) {
            result = compat_strdup("number:finite");
        }
    } else if (strcmp(kind, "record") == 0) {
        if (json_object_get(document, schema, "type", &value_index) &&
            (value = json_string_value(document, value_index))) {
            result = format_text("record:%s%s", value, "");
        }
    } else if (strcmp(kind, "array") == 0) {
        if (json_object_get(document, schema, "items", &value_index) &&
            (value = schema_text_from_json(document, value_index, error_message))) {
            result = format_text("array:(%s)%s", value, "");
        }
    } else if (strcmp(kind, "string") == 0) {
        if (json_object_get(document, schema, "encoding", &value_index)) {
            value = json_string_value(document, value_index);
            if (value && strcmp(value, "crexx-decimal") == 0) {
                result = compat_strdup("string:crexx-decimal");
            }
        } else if (json_object_get(document, schema, "contentEncoding", &value_index)) {
            value = json_string_value(document, value_index);
            if (value && strcmp(value, "base64") == 0) {
                result = compat_strdup("string:base64");
            }
        } else {
            result = compat_strdup("string");
        }
    }
    free(value);
    free(kind);
    if (!result && !*error_message) {
        crexx_contract_set_error(error_message,
                                 "previous contract contains an unsupported or incomplete format-1 schema");
    }
    return result;
}

static void compat_contract_free(CompatContract *contract) {
    size_t i;
    if (!contract) return;
    free(contract->version);
    free(contract->operation);
    for (i = 0; i < contract->entry_count; i++) {
        free(contract->entries[i].key);
        free(contract->entries[i].schema);
    }
    free(contract->entries);
    memset(contract, 0, sizeof(*contract));
}

static int compat_add(CompatContract *contract,
                      CompatEntryKind kind,
                      const char *key,
                      char *schema,
                      int required,
                      int nullable,
                      char **error_message) {
    CompatEntry *grown;
    char *key_copy;
    size_t i;
    for (i = 0; i < contract->entry_count; i++) {
        if (strcmp(contract->entries[i].key, key) == 0) {
            crexx_contract_set_error(error_message,
                                     "previous or current contract has duplicate identity '%s'",
                                     key);
            return 0;
        }
    }
    key_copy = compat_strdup(key);
    if (!key_copy) {
        crexx_contract_set_error(error_message, "out of memory comparing contracts");
        return 0;
    }
    grown = (CompatEntry *)realloc(contract->entries,
                                   (contract->entry_count + 1u) * sizeof(*grown));
    if (!grown) {
        free(key_copy);
        crexx_contract_set_error(error_message, "out of memory comparing contracts");
        return 0;
    }
    contract->entries = grown;
    grown = &contract->entries[contract->entry_count++];
    memset(grown, 0, sizeof(*grown));
    grown->kind = kind;
    grown->key = key_copy;
    grown->schema = schema;
    grown->required = required;
    grown->nullable = nullable;
    return 1;
}

static int compat_add_field_json(const JsonDocument *document,
                                 CompatContract *contract,
                                 size_t field,
                                 CompatEntryKind kind,
                                 const char *prefix,
                                 char **error_message) {
    size_t name_index;
    size_t required_index;
    size_t nullable_index;
    size_t schema_index;
    char *name = 0;
    char *key = 0;
    char *schema = 0;
    int required;
    int nullable;
    int ok = 0;
    if (document->tokens[field].kind != JSON_OBJECT ||
        !json_object_get(document, field, "name", &name_index) ||
        !json_object_get(document, field, "required", &required_index) ||
        !json_object_get(document, field, "nullable", &nullable_index) ||
        !json_object_get(document, field, "schema", &schema_index) ||
        !(name = json_string_value(document, name_index)) ||
        !json_boolean(document, required_index, &required) ||
        !json_boolean(document, nullable_index, &nullable) ||
        !(schema = schema_text_from_json(document, schema_index, error_message))) {
        if (!*error_message) crexx_contract_set_error(error_message, "invalid field in previous contract");
        goto cleanup;
    }
    key = format_text("%s%s", prefix, name);
    if (!key) {
        crexx_contract_set_error(error_message, "out of memory comparing field identity");
        goto cleanup;
    }
    ok = compat_add(contract, kind, key, schema, required, nullable, error_message);
    schema = 0;
cleanup:
    free(name);
    free(key);
    free(schema);
    return ok;
}

static int parse_previous_contract(const JsonDocument *document,
                                   CompatContract *contract,
                                   char **error_message) {
    size_t value;
    size_t index;
    size_t element;
    char *format = 0;
    memset(contract, 0, sizeof(*contract));
    if (!json_object_get(document, 0u, "format", &value) ||
        !(format = json_string_value(document, value)) ||
        strcmp(format, "crexx.operation-contract") != 0 ||
        !json_object_get(document, 0u, "formatVersion", &value) ||
        !json_token_raw_equal(document, value, "1")) {
        crexx_contract_set_error(error_message,
                                 "previous contract is not crexx.operation-contract format version 1");
        free(format);
        return 0;
    }
    free(format);
    if (!json_object_get(document, 0u, "contractVersion", &value) ||
        !(contract->version = json_string_value(document, value)) ||
        !json_object_get(document, 0u, "operation", &value) ||
        !(contract->operation = json_string_value(document, value))) {
        crexx_contract_set_error(error_message, "previous contract lacks version or operation identity");
        return 0;
    }
    if (!json_object_get(document, 0u, "input", &value) ||
        document->tokens[value].kind != JSON_ARRAY) {
        crexx_contract_set_error(error_message, "previous contract input must be an array");
        return 0;
    }
    index = value + 1u;
    for (element = 0; element < document->tokens[value].child_count; element++) {
        if (!compat_add_field_json(document,
                                   contract,
                                   index,
                                   COMPAT_INPUT,
                                   "input:",
                                   error_message)) return 0;
        index = json_token_next(document, index);
    }
    if (!json_object_get(document, 0u, "result", &value)) {
        crexx_contract_set_error(error_message, "previous contract lacks a result schema");
        return 0;
    }
    {
        char *schema = schema_text_from_json(document, value, error_message);
        if (!schema || !compat_add(contract, COMPAT_RESULT, "result", schema, 1, 0, error_message)) {
            free(schema);
            return 0;
        }
    }
    if (!json_object_get(document, 0u, "errors", &value) ||
        document->tokens[value].kind != JSON_ARRAY) {
        crexx_contract_set_error(error_message, "previous contract errors must be an array");
        return 0;
    }
    index = value + 1u;
    for (element = 0; element < document->tokens[value].child_count; element++) {
        char *schema = schema_text_from_json(document, index, error_message);
        char *key;
        if (!schema) return 0;
        key = format_text("error:%s%s", schema, "");
        if (!key || !compat_add(contract, COMPAT_ERROR, key, schema, 1, 0, error_message)) {
            free(key);
            free(schema);
            return 0;
        }
        free(key);
        index = json_token_next(document, index);
    }
    if (!json_object_get(document, 0u, "types", &value) ||
        document->tokens[value].kind != JSON_ARRAY) {
        crexx_contract_set_error(error_message, "previous contract types must be an array");
        return 0;
    }
    index = value + 1u;
    for (element = 0; element < document->tokens[value].child_count; element++) {
        size_t name_index;
        size_t fields_index;
        size_t field_index;
        size_t field_number;
        char *name = 0;
        char *prefix;
        if (document->tokens[index].kind != JSON_OBJECT ||
            !json_object_get(document, index, "name", &name_index) ||
            !(name = json_string_value(document, name_index)) ||
            !json_object_get(document, index, "fields", &fields_index) ||
            document->tokens[fields_index].kind != JSON_ARRAY) {
            free(name);
            crexx_contract_set_error(error_message, "invalid record definition in previous contract");
            return 0;
        }
        prefix = format_text("field:%s.%s", name, "");
        free(name);
        if (!prefix) {
            crexx_contract_set_error(error_message, "out of memory comparing record fields");
            return 0;
        }
        field_index = fields_index + 1u;
        for (field_number = 0;
             field_number < document->tokens[fields_index].child_count;
             field_number++) {
            if (!compat_add_field_json(document,
                                       contract,
                                       field_index,
                                       COMPAT_FIELD,
                                       prefix,
                                       error_message)) {
                free(prefix);
                return 0;
            }
            field_index = json_token_next(document, field_index);
        }
        free(prefix);
        index = json_token_next(document, index);
    }
    return 1;
}

static int current_contract(const CrexxContractModel *model,
                            CompatContract *contract,
                            char **error_message) {
    size_t i;
    size_t j;
    memset(contract, 0, sizeof(*contract));
    contract->version = compat_strdup(model->contract_version);
    contract->operation = compat_strdup(model->operation);
    if (!contract->version || !contract->operation) goto allocation_error;
    for (i = 0; i < model->input_count; i++) {
        char *key = format_text("input:%s%s", model->inputs[i].name, "");
        char *schema = schema_text_from_model(&model->inputs[i].schema);
        if (!key || !schema ||
            !compat_add(contract,
                        COMPAT_INPUT,
                        key,
                        schema,
                        model->inputs[i].required,
                        model->inputs[i].nullable,
                        error_message)) {
            free(key);
            free(schema);
            return 0;
        }
        free(key);
    }
    {
        char *schema = schema_text_from_model(&model->result);
        if (!schema || !compat_add(contract, COMPAT_RESULT, "result", schema, 1, 0, error_message)) {
            free(schema);
            return 0;
        }
    }
    for (i = 0; i < model->error_count; i++) {
        char *schema = schema_text_from_model(&model->errors[i]);
        char *key;
        if (!schema) goto allocation_error;
        key = format_text("error:%s%s", schema, "");
        if (!key || !compat_add(contract, COMPAT_ERROR, key, schema, 1, 0, error_message)) {
            free(key);
            free(schema);
            return 0;
        }
        free(key);
    }
    for (i = 0; i < model->record_count; i++) {
        for (j = 0; j < model->records[i].field_count; j++) {
            char *prefix = format_text("field:%s.%s", model->records[i].name, "");
            char *key;
            char *schema;
            if (!prefix) goto allocation_error;
            key = format_text("%s%s", prefix, model->records[i].fields[j].name);
            free(prefix);
            schema = schema_text_from_model(&model->records[i].fields[j].schema);
            if (!key || !schema ||
                !compat_add(contract,
                            COMPAT_FIELD,
                            key,
                            schema,
                            model->records[i].fields[j].required,
                            model->records[i].fields[j].nullable,
                            error_message)) {
                free(key);
                free(schema);
                return 0;
            }
            free(key);
        }
    }
    return 1;
allocation_error:
    crexx_contract_set_error(error_message, "out of memory comparing contracts");
    return 0;
}

static const CompatEntry *find_entry(const CompatContract *contract, const char *key) {
    size_t i;
    for (i = 0; i < contract->entry_count; i++) {
        if (strcmp(contract->entries[i].key, key) == 0) return &contract->entries[i];
    }
    return 0;
}

static int parse_version(const char *text, SemanticVersion *version) {
    const char *cursor = text;
    unsigned *parts[3] = {&version->major, &version->minor, &version->patch};
    size_t i;
    for (i = 0; i < 3u; i++) {
        unsigned long value = 0u;
        if (!isdigit((unsigned char)*cursor)) return 0;
        while (isdigit((unsigned char)*cursor)) {
            unsigned digit = (unsigned)(*cursor++ - '0');
            if (value > (ULONG_MAX - digit) / 10u) return 0;
            value = value * 10u + digit;
            if (value > UINT_MAX) return 0;
        }
        *parts[i] = (unsigned)value;
        if (i < 2u) {
            if (*cursor++ != '.') return 0;
        } else if (*cursor) return 0;
    }
    return 1;
}

static int version_compare(const SemanticVersion *left, const SemanticVersion *right) {
    if (left->major != right->major) return left->major > right->major ? 1 : -1;
    if (left->minor != right->minor) return left->minor > right->minor ? 1 : -1;
    if (left->patch != right->patch) return left->patch > right->patch ? 1 : -1;
    return 0;
}

int crexx_contract_check_previous(const CrexxContractModel *model,
                                  const char *previous_path,
                                  char **error_message) {
    JsonDocument document;
    CompatContract previous;
    CompatContract current;
    SemanticVersion old_version;
    SemanticVersion new_version;
    size_t i;
    int change_level = 0; /* 0=same/patch, 1=minor, 2=major */
    int ok = 0;
    memset(&document, 0, sizeof(document));
    memset(&previous, 0, sizeof(previous));
    memset(&current, 0, sizeof(current));
    if (!json_read_file(previous_path, &document, error_message) ||
        !parse_previous_contract(&document, &previous, error_message) ||
        !current_contract(model, &current, error_message)) goto cleanup;
    if (!parse_version(previous.version, &old_version) ||
        !parse_version(current.version, &new_version)) {
        crexx_contract_set_error(error_message,
                                 "contract versions must use MAJOR.MINOR.PATCH");
        goto cleanup;
    }
    if (strcmp(previous.operation, current.operation) != 0) change_level = 2;
    for (i = 0; i < previous.entry_count; i++) {
        const CompatEntry *old_entry = &previous.entries[i];
        const CompatEntry *new_entry = find_entry(&current, old_entry->key);
        if (!new_entry) {
            change_level = 2;
            continue;
        }
        if (strcmp(old_entry->schema, new_entry->schema) != 0 ||
            (old_entry->nullable && !new_entry->nullable) ||
            (!old_entry->required && new_entry->required)) {
            change_level = 2;
        } else if ((!old_entry->nullable && new_entry->nullable) ||
                   (old_entry->required && !new_entry->required)) {
            if (change_level < 1) change_level = 1;
        }
    }
    for (i = 0; i < current.entry_count; i++) {
        const CompatEntry *entry = &current.entries[i];
        if (find_entry(&previous, entry->key)) continue;
        if (entry->kind == COMPAT_ERROR ||
            ((entry->kind == COMPAT_INPUT || entry->kind == COMPAT_FIELD) &&
             !entry->required)) {
            if (change_level < 1) change_level = 1;
        } else {
            change_level = 2;
        }
    }
    if (version_compare(&new_version, &old_version) < 0) {
        crexx_contract_set_error(error_message,
                                 "contract version %s is older than previous %s",
                                 current.version,
                                 previous.version);
        goto cleanup;
    }
    if (change_level == 2 && new_version.major <= old_version.major) {
        crexx_contract_set_error(error_message,
                                 "breaking contract change requires a major version increase from %s",
                                 previous.version);
        goto cleanup;
    }
    if (change_level == 1 && new_version.major == old_version.major &&
        new_version.minor <= old_version.minor) {
        crexx_contract_set_error(error_message,
                                 "additive contract change requires a minor version increase from %s",
                                 previous.version);
        goto cleanup;
    }
    ok = 1;
cleanup:
    json_document_free(&document);
    compat_contract_free(&previous);
    compat_contract_free(&current);
    return ok;
}
