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

/* REXX ASSEMBLER */
/* The Disassembler */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>
#include "platform.h"
#include "rxas.h"
#include "rxdadism.h"
#include "../binutils/include/rxdefs.h"
#include "../binutils/include/rxjtable.h"
#include "../binutils/include/rxnumparse.h"

/* Max buffer size - todo change to a dynamic solution */
#define MAX_LINE_SIZE 5000
#include "../binutils/include/opdata.c"

static void get_mnemonic(char *dest, const char *name) {
    int i = 0;
    while (name[i] && name[i] != '_') {
        dest[i] = (char)tolower((unsigned char)name[i]);
        i++;
    }
    dest[i] = 0;
}

/* Encodes a string to a buffer. Like snprintf() it returns the number of characters
 * that would have been written */
static size_t encode_print(char* buffer, size_t buffer_len, char* string, size_t length) {

#define ADD_CHAR_TO_BUFFER(ch) {out_len++; if (buffer_len) { *(buffer++) = (ch); buffer_len--; }}

    size_t out_len = 0;
    char hex_buffer[3];
    while (length) {
        switch (*string) {
            case '\\':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('\\')
                break;
            case '\n':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('n')
                break;
            case '\t':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('t')
                break;
            case '\a':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('a')
                break;
            case '\b':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('b')
                break;
            case '\f':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('f')
                break;
            case '\r':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('r')
                break;
            case '\v':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('v')
                break;
            case '\'':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('\'')
                break;
            case '\"':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('\"')
                break;
            case 0:
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('0')
                break;
            case '\?':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('?')
                break;
            default:
                /* Should we escape this character? */
                if ((unsigned char)(*string) < 0x80) /* Not utf-8 */ {
                    if (isprint(*string)) {
                        ADD_CHAR_TO_BUFFER(*string) /* Normal Character */
                    }
                    else {
                        /* Escape as a hex character */
                        snprintf(hex_buffer, 3, "%02x", *string);
                        ADD_CHAR_TO_BUFFER('\\')
                        ADD_CHAR_TO_BUFFER('x')
                        ADD_CHAR_TO_BUFFER(hex_buffer[0])
                        ADD_CHAR_TO_BUFFER(hex_buffer[1])
                    }
                }
                else ADD_CHAR_TO_BUFFER(*string) /* Pass through UTF-8 */
        }
        string++;
        length--;
    }
    if (buffer_len) *buffer = 0;
    return out_len;
#undef ADD_CHAR_TO_BUFFER
}

/* Encodes a binary array to a hex string buffer. Like snprintf() it returns the number of characters
 * that would have been written */
static size_t encode_binary_to_hex(char* buffer, size_t buffer_len, const char* binary, size_t length) {
    static const char hex_digits[] = "0123456789abcdef";
    size_t out_len = 0;

#define ADD_HEX_CHAR_TO_BUFFER(ch) \
    do { \
        out_len++; \
        if (buffer_len > 1u) { \
            *(buffer++) = (ch); \
            buffer_len--; \
        } \
    } while (0)

    ADD_HEX_CHAR_TO_BUFFER('0');
    ADD_HEX_CHAR_TO_BUFFER('x');

    while (length) {
        unsigned char byte = (unsigned char)*binary;
        ADD_HEX_CHAR_TO_BUFFER(hex_digits[byte >> 4u]);
        ADD_HEX_CHAR_TO_BUFFER(hex_digits[byte & 0x0fu]);
        binary++;
        length--;
    }
    if (buffer_len) *buffer = 0;
    return out_len;
#undef ADD_HEX_CHAR_TO_BUFFER
}

/* Get the constant string
 * Returns the number of characters that would have been written assuming the
 * buffer was big enough - like snprintf() */
static size_t get_const_string(bin_space *pgm, char* buffer, size_t buffer_len, size_t ix) {

    size_t i;
    char *c;
    size_t sz;
    size_t out_len = 0;

    c = ((string_constant *)(pgm->const_pool + ix))->string;
    sz = ((string_constant *)(pgm->const_pool + ix))->string_len;

    out_len++; if (buffer_len) { *(buffer++) = '\"'; buffer_len--; }
    i = encode_print(buffer, buffer_len, c, sz);
    out_len += i;
    i = i>buffer_len?buffer_len:i;
    buffer += i; buffer_len -= i;
    out_len++; if (buffer_len) { *(buffer++) = '\"'; buffer_len--; }
    if (buffer_len) *buffer = 0;

    return out_len;
}

static char *get_const_raw_string_alloc(bin_space *pgm, size_t ix) {
    string_constant *sentry;
    char *out;

    sentry = (string_constant *)(pgm->const_pool + ix);
    out = malloc(sentry->string_len + 1);
    if (!out) return NULL;
    memcpy(out, sentry->string, sentry->string_len);
    out[sentry->string_len] = 0;
    return out;
}

static char *get_const_string_alloc(bin_space *pgm, size_t ix) {
    string_constant *sentry;
    char *out;
    size_t encoded_len;

    sentry = (string_constant *)(pgm->const_pool + ix);
    encoded_len = encode_print(NULL, 0, sentry->string, sentry->string_len);
    out = malloc(encoded_len + 3);
    if (!out) return NULL;
    out[0] = '"';
    encode_print(out + 1, encoded_len + 1, sentry->string, sentry->string_len);
    out[encoded_len + 1] = '"';
    out[encoded_len + 2] = 0;
    return out;
}

static void output_meta_inline_line(FILE *stream, bin_space *pgm, meta_inline_constant *mentry, const char *indent) {
    char *symbol;
    char *payload;

    symbol = get_const_string_alloc(pgm, mentry->symbol);
    payload = get_const_string_alloc(pgm, mentry->payload);
    if (symbol && payload) {
        fprintf(stream, "%s.meta %s=\".inline\" %s\n", indent ? indent : "", symbol, payload);
    }
    if (symbol) free(symbol);
    if (payload) free(payload);
}

static void output_meta_task_target_line(FILE *stream,
                                         bin_space *pgm,
                                         meta_task_target_constant *mentry,
                                         const char *indent) {
    char symbol_buffer[MAX_LINE_SIZE];
    char binding_buffer[MAX_LINE_SIZE];
    string_constant *binding;

    get_const_string(pgm, symbol_buffer, sizeof(symbol_buffer), mentry->symbol);
    binding = (string_constant *)(pgm->const_pool + mentry->binding);
    encode_binary_to_hex(binding_buffer,
                         sizeof(binding_buffer),
                         binding->string,
                         binding->string_len);
    fprintf(stream,
            "%s.meta %s=\".task%u\" %s\n",
            indent,
            symbol_buffer,
            mentry->kind,
            binding_buffer);
}

static void output_meta_source_step_line(FILE *stream, bin_space *pgm, meta_source_step_constant *mentry, const char *indent) {
    char file_buffer[MAX_LINE_SIZE];
    char source_buffer[MAX_LINE_SIZE];

    get_const_string(pgm, file_buffer, MAX_LINE_SIZE, mentry->file);
    get_const_string(pgm, source_buffer, MAX_LINE_SIZE, mentry->source_line);
    fprintf(stream, "%s.srcstep %u %u %u %s %u %u %u %s\n",
            indent ? indent : "",
            mentry->step_id,
            mentry->clause_id,
            mentry->flags,
            file_buffer,
            mentry->line,
            mentry->active_start_column,
            mentry->active_end_column,
            source_buffer);
}

static void trace_code_string(uint8_t code, char *buffer, size_t buffer_len) {
    if (!buffer || buffer_len == 0) return;
    if (!code) {
        snprintf(buffer, buffer_len, "\"\"");
    } else {
        snprintf(buffer, buffer_len, "\"%c\"", (char) code);
    }
}

static void trace_ref_string(size_t ref, char *buffer, size_t buffer_len) {
    if (!buffer || buffer_len == 0) return;
    if (ref == RXBIN_TRACE_REF_NONE) {
        snprintf(buffer, buffer_len, "-1");
    } else {
        snprintf(buffer, buffer_len, "%lu", (unsigned long) ref);
    }
}

static void trace_optional_string(bin_space *pgm, size_t ref, char *buffer, size_t buffer_len) {
    chameleon_constant *entry;

    if (!buffer || buffer_len == 0) return;
    if (ref == RXBIN_TRACE_REF_NONE) {
        snprintf(buffer, buffer_len, "\"\"");
        return;
    }

    if (!pgm || ref >= pgm->const_size) {
        snprintf(buffer, buffer_len, "\"\"");
        return;
    }

    entry = (chameleon_constant *)(pgm->const_pool + ref);
    if (entry->type != STRING_CONST) {
        snprintf(buffer, buffer_len, "\"\"");
        return;
    }

    get_const_string(pgm, buffer, buffer_len, ref);
}

static void output_meta_trace_event_line(FILE *stream, bin_space *pgm, meta_trace_event_constant *mentry, const char *indent) {
    char kind_buffer[8];
    char value_source_buffer[8];
    char value_type_buffer[8];
    char register_type_buffer[8];
    char value_ref_buffer[32];
    char symbol_buffer[MAX_LINE_SIZE];
    char resolved_buffer[MAX_LINE_SIZE];

    trace_code_string(mentry->kind, kind_buffer, sizeof(kind_buffer));
    trace_code_string(mentry->value_source, value_source_buffer, sizeof(value_source_buffer));
    trace_code_string(mentry->value_type, value_type_buffer, sizeof(value_type_buffer));
    trace_code_string(mentry->register_type, register_type_buffer, sizeof(register_type_buffer));
    trace_ref_string(mentry->value_ref, value_ref_buffer, sizeof(value_ref_buffer));
    trace_optional_string(pgm, mentry->symbol, symbol_buffer, sizeof(symbol_buffer));
    trace_optional_string(pgm, mentry->resolved_name, resolved_buffer, sizeof(resolved_buffer));
    fprintf(stream, "%s.traceevent %s %u %s %s %s %s %u %u %u %s %s\n",
            indent ? indent : "",
            kind_buffer,
            mentry->mode_mask,
            value_source_buffer,
            value_type_buffer,
            register_type_buffer,
            value_ref_buffer,
            mentry->source_step_id,
            mentry->clause_id,
            mentry->flags,
            symbol_buffer,
            resolved_buffer);
}

static void output_meta_inline_for_symbol(FILE *stream, module_file *module, bin_space *pgm, const char *symbol, const char *indent) {
    int m;

    if (!symbol) return;

    m = module->header.meta_head;
    while (m != -1) {
        chameleon_constant *entry;
        entry = (chameleon_constant *)(module->constant + m);
        if (entry->type == META_INLINE) {
            meta_inline_constant *inline_meta;
            char *inline_symbol;
            inline_meta = (meta_inline_constant *)(module->constant + m);
            inline_symbol = get_const_raw_string_alloc(pgm, inline_meta->symbol);
            if (inline_symbol) {
                if (strcmp(inline_symbol, symbol) == 0) {
                    output_meta_inline_line(stream, pgm, inline_meta, indent);
                    free(inline_symbol);
                    return;
                }
                free(inline_symbol);
            }
        }
        m = ((meta_entry *)(module->constant + m))->next;
    }
}

static void output_meta_provider_line(FILE *stream,
                                      bin_space *pgm,
                                      const meta_provider_constant *provider,
                                      const char *indent) {
    char symbol[MAX_LINE_SIZE];
    char provider_id[MAX_LINE_SIZE];
    const char *option;

    get_const_string(pgm, symbol, sizeof(symbol), provider->symbol);
    get_const_string(pgm, provider_id, sizeof(provider_id), provider->provider);
    option = (provider->flags & RXBIN_PROVIDER_REQUIRED)
            ? ".provider" : ".provider.optional";
    fprintf(stream, "%s.meta %s=\"%s\" %s\n",
            indent ? indent : "", symbol, option, provider_id);
}

static void output_meta_provider_for_symbol(FILE *stream,
                                            module_file *module,
                                            bin_space *pgm,
                                            const char *symbol,
                                            const char *indent) {
    int m;

    if (!symbol) return;
    m = module->header.meta_head;
    while (m != -1) {
        chameleon_constant *entry =
            (chameleon_constant *)(module->constant + m);
        if (entry->type == META_PROVIDER) {
            meta_provider_constant *provider =
                (meta_provider_constant *)(module->constant + m);
            char *provider_symbol =
                get_const_raw_string_alloc(pgm, provider->symbol);
            if (provider_symbol && strcmp(provider_symbol, symbol) == 0) {
                output_meta_provider_line(stream, pgm, provider, indent);
            }
            free(provider_symbol);
        }
        m = ((meta_entry *)(module->constant + m))->next;
    }
}

static void output_meta_autoload_for_symbol(FILE *stream,
                                            module_file *module,
                                            bin_space *pgm,
                                            const char *symbol,
                                            const char *indent) {
    int m;

    if (!symbol) return;
    m = module->header.meta_head;
    while (m != -1) {
        chameleon_constant *entry =
            (chameleon_constant *)(module->constant + m);
        if (entry->type == META_AUTOLOAD) {
            meta_autoload_constant *autoload =
                (meta_autoload_constant *)(module->constant + m);
            char *autoload_symbol =
                get_const_raw_string_alloc(pgm, autoload->symbol);
            if (autoload_symbol && strcmp(autoload_symbol, symbol) == 0) {
                char artifact[MAX_LINE_SIZE];
                get_const_string(pgm, artifact, sizeof(artifact),
                                 autoload->artifact);
                fprintf(stream, "%s.meta \"%s\"=\".autoload\" %s\n",
                        indent ? indent : "", symbol, artifact);
            }
            free(autoload_symbol);
        }
        m = ((meta_entry *)(module->constant + m))->next;
    }
}

static size_t get_func_string(bin_space *pgm, char *buffer,
                              size_t buffer_len, size_t index);

static void output_meta_initializer_for_symbol(FILE *stream,
                                               module_file *module,
                                               bin_space *pgm,
                                               const char *symbol,
                                               const char *indent) {
    int m;

    if (!symbol) return;
    m = module->header.meta_head;
    while (m != -1) {
        chameleon_constant *entry =
            (chameleon_constant *)(module->constant + m);
        if (entry->type == META_INITIALIZER) {
            meta_initializer_constant *initializer =
                (meta_initializer_constant *)(module->constant + m);
            char *initializer_symbol =
                get_const_raw_string_alloc(pgm, initializer->symbol);
            if (initializer_symbol && strcmp(initializer_symbol, symbol) == 0) {
                char symbol_buffer[MAX_LINE_SIZE];
                char function_buffer[MAX_LINE_SIZE];
                get_const_string(pgm, symbol_buffer, sizeof(symbol_buffer),
                                 initializer->symbol);
                get_func_string(pgm, function_buffer, sizeof(function_buffer),
                                initializer->function);
                fprintf(stream,
                        "%s.meta %s=\".initializer\" \".void\" %s \"\"\n",
                        indent ? indent : "", symbol_buffer, function_buffer);
            }
            free(initializer_symbol);
        }
        m = ((meta_entry *)(module->constant + m))->next;
    }
}

/* Get the function name string
 * Returns the number of characters that would have been written assuming the
 * buffer was big enough - like snprintf() */
static size_t get_func_string(bin_space *pgm, char* buffer, size_t buffer_len, size_t index) {
    return snprintf(buffer, buffer_len, "%s()", ((proc_constant *) (pgm->const_pool + index))->name);
}

/* Get the register name string
 * Returns the number of characters that would have been written assuming the
 * buffer was big enough - like snprintf() */
static size_t get_reg_string(bin_space *pgm, char* buffer, size_t buffer_len, int r, int globals, int locals) {
    size_t out_len;

    if (r < locals)
        out_len = snprintf(buffer, buffer_len, "r%d", r);
    else if (r < locals + globals)
        out_len = snprintf(buffer, buffer_len, "g%d /* aka r%d */", r - locals, r);
    else
        out_len = snprintf(buffer, buffer_len, "a%d /* aka r%d */", r - locals - globals, r);

    return out_len;
}

static size_t format_float_literal(char *buffer, size_t buffer_len, double value) {
    char temp[64];
    const char *scan;
    int needs_float_marker;

    snprintf(temp, sizeof(temp), "%.17g", value);

    needs_float_marker = 1;
    for (scan = temp; *scan; scan++) {
        if (*scan == '.' || *scan == 'e' || *scan == 'E') {
            needs_float_marker = 0;
            break;
        }
    }

    scan = temp;
    if (*scan == '-' || *scan == '+') scan++;
    if (!isdigit((unsigned char)*scan)) needs_float_marker = 0;

    if (needs_float_marker) return snprintf(buffer, buffer_len, "%s.0", temp);
    return snprintf(buffer, buffer_len, "%s", temp);
}

typedef struct rxda_binary_alias {
    size_t pool_index;
    struct rxda_binary_alias *next;
} rxda_binary_alias;

static const rxda_binary_alias *rxda_find_binary_alias(
        const rxda_binary_alias *aliases, size_t pool_index) {
    while (aliases) {
        if (aliases->pool_index == pool_index) return aliases;
        aliases = aliases->next;
    }
    return NULL;
}

/* Disassemble an operand
 *
 * Returns the number of characters that would have been written assuming the
 * buffer was big enough - like snprintf() */
static size_t disassemble_operand(bin_space *pgm,
                                  module_file *module,
                                  int opcode,
                                  unsigned int operand_index,
                                  char* buffer,
                                  size_t buffer_len,
                                  OperandType type,
                                  size_t index,
                                  const rxda_binary_alias *binary_aliases,
                                  int globals,
                                  int locals) {

    size_t out_len;
    size_t ix;
    char *c;
    size_t sz;
    int r;

    if (module && module->graph_operands &&
        rx_graph_operand_kind(opcode, operand_index) != RX_GRAPH_OPERAND_NONE) {
        char *text;
        size_t encoded;

        text = rx_graph_operand_text(module->semantic_graph,
                                     opcode,
                                     operand_index,
                                     (uint32_t)pgm->binary[index].index);
        if (!text) return (size_t)snprintf(buffer, buffer_len, "*INVALID_GRAPH_ID*");
        out_len = 1u;
        if (buffer_len) {
            *buffer++ = '"';
            buffer_len--;
        }
        encoded = encode_print(buffer, buffer_len, text, strlen(text));
        out_len += encoded;
        if (encoded < buffer_len) {
            buffer += encoded;
            buffer_len -= encoded;
        } else {
            buffer += buffer_len;
            buffer_len = 0u;
        }
        out_len++;
        if (buffer_len) {
            *buffer++ = '"';
            buffer_len--;
            *buffer = 0;
        }
        free(text);
        return out_len;
    }

    switch(type) {
        case OP_ID:
            out_len = snprintf(buffer, buffer_len, "lb_%x", (int)pgm->binary[index].index);
            break;
        case OP_REG:
            r = (int)pgm->binary[index].index;
            out_len = get_reg_string(pgm, buffer, buffer_len, r, globals, locals);
            break;
        case OP_FUNC:
            ix = pgm->binary[index].index;
            out_len = get_func_string(pgm, buffer, buffer_len, ix);
            break;
        case OP_INT:
            out_len = snprintf(buffer, buffer_len, "%" RXINTEGER_PRI,
                               pgm->binary[index].iconst);
            break;
        case OP_FLOAT:
            out_len = format_float_literal(buffer, buffer_len,
                                           FLOAT_CONST_VALUE(pgm->const_pool, pgm->binary[index].index));
            break;
        case OP_CHAR:
            out_len = snprintf(buffer, buffer_len, "\'%c\'", pgm->binary[index].cconst);
            break;
        case OP_STRING:
            ix = pgm->binary[index].index;
            out_len = get_const_string(pgm, buffer, buffer_len, ix);
            break;
        case OP_DECIMAL:
            ix = pgm->binary[index].index;
            c = ((string_constant *)(pgm->const_pool + ix))->string;
            sz = ((string_constant *)(pgm->const_pool + ix))->string_len;
            out_len = snprintf(buffer, buffer_len, "%.*sd", (int)sz, c);
            break;
        case OP_BINARY:
            ix = pgm->binary[index].index;
            if (rxda_find_binary_alias(binary_aliases, ix)) {
                out_len = snprintf(buffer, buffer_len,
                                   "\xC2\xA7" "rxdas.const.%lx",
                                   (unsigned long)ix);
            }
            else {
                c = ((string_constant *)(pgm->const_pool + ix))->string;
                sz = ((string_constant *)(pgm->const_pool + ix))->string_len;
                out_len = encode_binary_to_hex(buffer, buffer_len, c, sz);
            }
            break;
        default:
            out_len = snprintf(buffer, buffer_len, "*INTERNAL_ERROR*");
            break;
    }
    return out_len;
}

/* Stores the output of pass 1 and 1b */
typedef struct code_line {
    enum {
        operand=0, normal, show_label, show_proc
    } flags;
    size_t proc_index;
    const OpInfo *op;
    const char *comment;
} code_line;

enum rxda_jtable_mode {
    RXDA_JTABLE_STRING,
    RXDA_JTABLE_PADDED,
    RXDA_JTABLE_NUMERIC,
    RXDA_JTABLE_BINARY,
    RXDA_JTABLE_INTEGER
};

typedef struct rxda_jtable_case {
    const unsigned char *key;
    uint32_t key_length;
    uint32_t target;
    struct rxda_jtable_case *next;
} rxda_jtable_case;

typedef struct rxda_jtable {
    size_t pool_index;
    size_t procedure_start;
    unsigned char algorithm;
    enum rxda_jtable_mode mode;
    int valid;
    rxda_jtable_case *cases;
    rxda_jtable_case *cases_tail;
    struct rxda_jtable *next;
} rxda_jtable;

static uint16_t rxda_read_u16le(const unsigned char *bytes) {
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8u));
}

static uint32_t rxda_read_u32le(const unsigned char *bytes) {
    return (uint32_t)bytes[0] |
           ((uint32_t)bytes[1] << 8u) |
           ((uint32_t)bytes[2] << 16u) |
           ((uint32_t)bytes[3] << 24u);
}

static int rxda_range_valid(size_t total, size_t offset, size_t length) {
    return offset <= total && length <= total - offset;
}

static int rxda_add_jtable_case(rxda_jtable *table, const unsigned char *payload,
                                size_t payload_length, uint32_t key_offset,
                                uint32_t key_length, uint32_t target,
                                size_t instruction_count) {
    rxda_jtable_case *entry;

    if (!rxda_range_valid(payload_length, key_offset, key_length) ||
        target >= instruction_count) return 0;
    entry = calloc(1, sizeof(*entry));
    if (!entry) return 0;
    entry->key = payload + key_offset;
    entry->key_length = key_length;
    entry->target = target;
    if (table->cases_tail) table->cases_tail->next = entry;
    else table->cases = entry;
    table->cases_tail = entry;
    return 1;
}

static int rxda_decode_linear_jtable(rxda_jtable *table, const unsigned char *payload,
                                     size_t payload_length, uint32_t case_count,
                                     size_t instruction_count) {
    size_t entries_length;
    size_t i;

    if ((size_t)case_count > SIZE_MAX / RX_JTABLE_LINEAR_ENTRY_SIZE) return 0;
    entries_length = (size_t)case_count * RX_JTABLE_LINEAR_ENTRY_SIZE;
    if (!rxda_range_valid(payload_length, RX_JTABLE_HEADER_SIZE, entries_length)) return 0;
    for (i = 0; i < case_count; i++) {
        size_t offset = RX_JTABLE_HEADER_SIZE + i * RX_JTABLE_LINEAR_ENTRY_SIZE;
        if (!rxda_add_jtable_case(table, payload, payload_length,
                                  rxda_read_u32le(payload + offset),
                                  rxda_read_u32le(payload + offset + 4u),
                                  rxda_read_u32le(payload + offset + 8u),
                                  instruction_count)) return 0;
    }
    return 1;
}

static int rxda_decode_open_jtable(rxda_jtable *table, const unsigned char *payload,
                                   size_t payload_length, uint32_t case_count,
                                   size_t instruction_count) {
    uint32_t slot_count;
    size_t slots_length;
    size_t i;
    size_t found = 0;

    if (!rxda_range_valid(payload_length, 0, RX_JTABLE_OPEN_HEADER_SIZE)) return 0;
    slot_count = rxda_read_u32le(payload + 12u);
    if (slot_count == 0 || (slot_count & (slot_count - 1u)) != 0 ||
        (size_t)slot_count > SIZE_MAX / RX_JTABLE_OPEN_SLOT_SIZE) return 0;
    slots_length = (size_t)slot_count * RX_JTABLE_OPEN_SLOT_SIZE;
    if (!rxda_range_valid(payload_length, RX_JTABLE_OPEN_HEADER_SIZE, slots_length)) return 0;
    for (i = 0; i < slot_count; i++) {
        size_t offset = RX_JTABLE_OPEN_HEADER_SIZE + i * RX_JTABLE_OPEN_SLOT_SIZE;
        uint32_t key_offset = rxda_read_u32le(payload + offset + 4u);
        if (key_offset == RX_JTABLE_OPEN_EMPTY) continue;
        if (!rxda_add_jtable_case(table, payload, payload_length, key_offset,
                                  rxda_read_u32le(payload + offset + 8u),
                                  rxda_read_u32le(payload + offset + 12u),
                                  instruction_count)) return 0;
        found++;
    }
    return found == case_count;
}

static int rxda_decode_acph_jtable(rxda_jtable *table, const unsigned char *payload,
                                   size_t payload_length, uint32_t case_count,
                                   size_t instruction_count) {
    uint32_t *pending;
    uint32_t *visited;
    size_t pending_count = 0;
    size_t visited_count = 0;
    size_t found = 0;
    size_t capacity = (size_t)case_count + 1u;
    int ok = 1;

    if (!rxda_range_valid(payload_length, 0, RX_JTABLE_ACPH_HEADER_SIZE) ||
        capacity > SIZE_MAX / sizeof(*pending)) return 0;
    pending = malloc(capacity * sizeof(*pending));
    visited = malloc(capacity * sizeof(*visited));
    if (!pending || !visited) {
        free(pending);
        free(visited);
        return 0;
    }
    pending[pending_count++] = rxda_read_u32le(payload + 12u);
    while (pending_count && ok) {
        uint32_t node_offset = pending[--pending_count];
        uint16_t slot_count;
        size_t node_length;
        size_t i;

        for (i = 0; i < visited_count; i++) {
            if (visited[i] == node_offset) {
                ok = 0;
                break;
            }
        }
        if (!ok || visited_count >= capacity ||
            !rxda_range_valid(payload_length, node_offset, RX_JTABLE_ACPH_NODE_SIZE)) {
            ok = 0;
            break;
        }
        visited[visited_count++] = node_offset;
        slot_count = rxda_read_u16le(payload + node_offset + 4u);
        if (slot_count == 0 || slot_count > RX_JTABLE_ACPH_SYMBOL_COUNT ||
            (size_t)slot_count > (SIZE_MAX - RX_JTABLE_ACPH_NODE_SIZE) / RX_JTABLE_ACPH_SLOT_SIZE) {
            ok = 0;
            break;
        }
        node_length = RX_JTABLE_ACPH_NODE_SIZE + (size_t)slot_count * RX_JTABLE_ACPH_SLOT_SIZE;
        if (!rxda_range_valid(payload_length, node_offset, node_length)) {
            ok = 0;
            break;
        }
        for (i = 0; i < slot_count; i++) {
            size_t slot_offset = node_offset + RX_JTABLE_ACPH_NODE_SIZE +
                                 i * RX_JTABLE_ACPH_SLOT_SIZE;
            unsigned char kind = payload[slot_offset + 2u];
            uint32_t value_offset = rxda_read_u32le(payload + slot_offset + 4u);
            if (kind == RX_JTABLE_ACPH_SLOT_EMPTY) continue;
            if (kind == RX_JTABLE_ACPH_SLOT_CHILD) {
                if (pending_count >= capacity) {
                    ok = 0;
                    break;
                }
                pending[pending_count++] = value_offset;
            }
            else if (kind == RX_JTABLE_ACPH_SLOT_LEAF) {
                if (!rxda_range_valid(payload_length, value_offset, RX_JTABLE_ACPH_LEAF_SIZE) ||
                    !rxda_add_jtable_case(table, payload, payload_length,
                                          rxda_read_u32le(payload + value_offset),
                                          rxda_read_u32le(payload + value_offset + 4u),
                                          rxda_read_u32le(payload + value_offset + 8u),
                                          instruction_count)) {
                    ok = 0;
                    break;
                }
                found++;
            }
            else {
                ok = 0;
                break;
            }
        }
    }
    free(pending);
    free(visited);
    return ok && found == case_count;
}

static int rxda_decode_jtable(bin_space *pgm, rxda_jtable *table) {
    string_constant *constant;
    const unsigned char *payload;
    size_t payload_length;
    uint16_t header_size;
    uint32_t case_count;

    if (table->pool_index >= pgm->const_size) return 0;
    constant = (string_constant *)(pgm->const_pool + table->pool_index);
    if (constant->base.type != BINARY_CONST) return 0;
    payload = (const unsigned char *)constant->string;
    payload_length = constant->string_len;
    if (!rxda_range_valid(payload_length, 0, RX_JTABLE_HEADER_SIZE)) return 0;
    table->algorithm = payload[0];
    header_size = rxda_read_u16le(payload + 2u);
    case_count = rxda_read_u32le(payload + 8u);
    if (case_count == 0) return 0;
    switch (table->algorithm) {
        case RX_JTABLE_ALG_LINEAR:
            if (header_size != RX_JTABLE_HEADER_SIZE) return 0;
            return rxda_decode_linear_jtable(table, payload, payload_length,
                                             case_count, pgm->inst_size);
        case RX_JTABLE_ALG_OPENHASH:
            if (header_size != RX_JTABLE_OPEN_HEADER_SIZE) return 0;
            return rxda_decode_open_jtable(table, payload, payload_length,
                                           case_count, pgm->inst_size);
        case RX_JTABLE_ALG_ACPH:
            if (header_size != RX_JTABLE_ACPH_HEADER_SIZE) return 0;
            return rxda_decode_acph_jtable(table, payload, payload_length,
                                           case_count, pgm->inst_size);
        default:
            return 0;
    }
}

static int rxda_jump_info(const OpInfo *op, enum rxda_jtable_mode *mode,
                          int *table_operand) {
    if (strcmp(op->mnemonic, "JUMPS_REG_BINARY") == 0) {
        *mode = RXDA_JTABLE_STRING; *table_operand = 1; return 1;
    }
    if (strcmp(op->mnemonic, "JUMPR_REG_BINARY") == 0) {
        *mode = RXDA_JTABLE_PADDED; *table_operand = 1; return 1;
    }
    if (strcmp(op->mnemonic, "JUMPN_REG_BINARY") == 0) {
        *mode = RXDA_JTABLE_NUMERIC; *table_operand = 1; return 1;
    }
    if (strcmp(op->mnemonic, "JUMPB_REG_BINARY") == 0) {
        *mode = RXDA_JTABLE_BINARY; *table_operand = 1; return 1;
    }
    if (strcmp(op->mnemonic, "JUMPBS_REG_REG_BINARY") == 0) {
        *mode = RXDA_JTABLE_BINARY; *table_operand = 2; return 1;
    }
    if (strcmp(op->mnemonic, "JUMPI_REG_BINARY") == 0) {
        *mode = RXDA_JTABLE_INTEGER; *table_operand = 1; return 1;
    }
    return 0;
}

static rxda_jtable *rxda_find_jtable(rxda_jtable *tables, size_t pool_index) {
    while (tables) {
        if (tables->pool_index == pool_index) return tables;
        tables = tables->next;
    }
    return NULL;
}

static rxda_jtable *rxda_collect_jtables(bin_space *pgm, code_line *source) {
    rxda_jtable *tables = NULL;
    rxda_jtable *tail = NULL;
    size_t procedure_start = SIZE_MAX;
    size_t i = 0;

    while (i < pgm->inst_size) {
        size_t instruction = i;
        const OpInfo *op = &op_table[pgm->binary[i++].instruction.opcode];
        size_t operand_count = rxop_format_operand_count(op->format);
        enum rxda_jtable_mode mode;
        int table_operand;

        if (source[instruction].flags == show_proc) procedure_start = instruction;
        if (rxda_jump_info(op, &mode, &table_operand) && table_operand < operand_count) {
            size_t pool_index = pgm->binary[instruction + 1u + (size_t)table_operand].index;
            rxda_jtable *table = rxda_find_jtable(tables, pool_index);
            if (!table) {
                table = calloc(1, sizeof(*table));
                if (!table) break;
                table->pool_index = pool_index;
                table->procedure_start = procedure_start;
                table->mode = mode;
                table->valid = procedure_start != SIZE_MAX && rxda_decode_jtable(pgm, table);
                if (tail) tail->next = table;
                else tables = table;
                tail = table;
            }
            else if (table->procedure_start != procedure_start || table->mode != mode) {
                table->valid = 0;
            }
        }
        i = instruction + 1u + (size_t)operand_count;
    }
    return tables;
}

static int rxda_binary_needs_alias(bin_space *pgm, size_t pool_index) {
    string_constant *constant;
    size_t header_size = offsetof(string_constant, string);
    size_t encoded_length;

    if (pool_index > pgm->const_size ||
        header_size > pgm->const_size - pool_index) return 0;
    constant = (string_constant *)(pgm->const_pool + pool_index);
    if (constant->base.type != BINARY_CONST ||
        constant->base.size_in_pool < header_size ||
        constant->base.size_in_pool > pgm->const_size - pool_index ||
        constant->string_len > constant->base.size_in_pool - header_size) return 0;
    if (constant->string_len > (SIZE_MAX - 2u) / 2u) return 1;
    encoded_length = 2u + constant->string_len * 2u;
    return encoded_length >= MAX_LINE_SIZE;
}

/* Oversized binary operands are emitted once as private .const aliases. This
 * keeps ordinary rxdas output proportional to the constant-pool size while
 * preserving exact assembler round-tripping. Valid jump tables already have
 * their own symbolic .jtable representation and do not need another alias. */
static rxda_binary_alias *rxda_collect_binary_aliases(bin_space *pgm,
                                                      code_line *source,
                                                      rxda_jtable *jump_tables) {
    rxda_binary_alias *aliases = NULL;
    rxda_binary_alias *tail = NULL;
    size_t i = 0;

    while (i < pgm->inst_size) {
        size_t instruction = i;
        const OpInfo *op = source[instruction].op;
        size_t operand_count = rxop_format_operand_count(op->format);
        enum rxda_jtable_mode jump_mode;
        int table_operand = -1;
        size_t k;

        (void)rxda_jump_info(op, &jump_mode, &table_operand);
        i++;
        for (k = 0; k < operand_count; k++, i++) {
            size_t pool_index;
            rxda_jtable *table;

            if (rxop_format_operand_type(op->format, k) != OP_BINARY) continue;
            pool_index = pgm->binary[i].index;
            table = k == (size_t)table_operand
                    ? rxda_find_jtable(jump_tables, pool_index) : NULL;
            if ((table && table->valid) ||
                !rxda_binary_needs_alias(pgm, pool_index) ||
                rxda_find_binary_alias(aliases, pool_index)) continue;
            {
                rxda_binary_alias *alias = calloc(1u, sizeof(*alias));
                if (!alias) return aliases;
                alias->pool_index = pool_index;
                if (tail) tail->next = alias;
                else aliases = alias;
                tail = alias;
            }
        }
    }
    return aliases;
}

static void rxda_output_binary_hex(FILE *stream, const char *binary, size_t length) {
    static const char hex_digits[] = "0123456789abcdef";

    fputs("0x", stream);
    while (length) {
        unsigned char byte = (unsigned char)*binary++;
        fputc(hex_digits[byte >> 4u], stream);
        fputc(hex_digits[byte & 0x0fu], stream);
        length--;
    }
}

static void rxda_output_binary_aliases(FILE *stream, bin_space *pgm,
                                       const rxda_binary_alias *aliases) {
    while (aliases) {
        size_t pool_index = aliases->pool_index;
        string_constant *constant =
                (string_constant *)(pgm->const_pool + pool_index);
        fprintf(stream, ".const \xC2\xA7" "rxdas.const.%lx binary ",
                (unsigned long)pool_index);
        rxda_output_binary_hex(stream, constant->string, constant->string_len);
        fputc('\n', stream);
        aliases = aliases->next;
    }
}

static void rxda_free_binary_aliases(rxda_binary_alias *aliases) {
    while (aliases) {
        rxda_binary_alias *next = aliases->next;
        free(aliases);
        aliases = next;
    }
}

static const char *rxda_jtable_algorithm(unsigned char algorithm) {
    switch (algorithm) {
        case RX_JTABLE_ALG_LINEAR: return "linear";
        case RX_JTABLE_ALG_OPENHASH: return "openhash";
        case RX_JTABLE_ALG_ACPH: return "acph";
        default: return "auto";
    }
}

static void rxda_output_jtable_declarations(FILE *stream, rxda_jtable *tables,
                                            size_t procedure_start) {
    while (tables) {
        if (tables->valid && tables->procedure_start == procedure_start) {
            fprintf(stream, "                .jtable jtable_%lx %s\n",
                    (unsigned long)tables->pool_index,
                    rxda_jtable_algorithm(tables->algorithm));
        }
        tables = tables->next;
    }
}

static int rxda_numeric_case_value(const rxda_jtable_case *entry, double *value) {
    uint64_t bits = 0;
    size_t i;

    if (entry->key_length != RX_NUMERIC_KEY_SIZE) return 0;
    for (i = 0; i < RX_NUMERIC_KEY_SIZE; i++) {
        bits |= (uint64_t)entry->key[i] << (i * 8u);
    }
    memcpy(value, &bits, sizeof(bits));
    return !isnan(*value);
}

static int rxda_output_jcase_literal(FILE *stream, enum rxda_jtable_mode mode,
                                     const rxda_jtable_case *entry) {
    size_t i;

    if (mode == RXDA_JTABLE_BINARY) {
        fprintf(stream, "0x");
        for (i = 0; i < entry->key_length; i++) fprintf(stream, "%02x", entry->key[i]);
        return 1;
    }
    if (mode == RXDA_JTABLE_INTEGER) {
        uint64_t bits = 0;
        int64_t value;
        if (entry->key_length != sizeof(value)) return 0;
        for (i = 0; i < sizeof(value); i++) bits |= (uint64_t)entry->key[i] << (i * 8u);
        memcpy(&value, &bits, sizeof(value));
        fprintf(stream, "%" PRId64, value);
        return 1;
    }
    if (mode == RXDA_JTABLE_NUMERIC) {
        char value_buffer[64];
        double value;
        if (!rxda_numeric_case_value(entry, &value)) return 0;
        format_float_literal(value_buffer, sizeof(value_buffer), value);
        fprintf(stream, "\"%s\"", value_buffer);
        return 1;
    }
    fprintf(stream, "\"");
    for (i = 0; i < entry->key_length; i++) {
        char encoded[8];
        size_t encoded_length = encode_print(encoded, sizeof(encoded),
                                             (char *)entry->key + i, 1u);
        fwrite(encoded, 1, encoded_length < sizeof(encoded) ? encoded_length : sizeof(encoded), stream);
    }
    fprintf(stream, "\"");
    return 1;
}

static int rxda_is_visible_jcase(enum rxda_jtable_mode mode,
                                 const rxda_jtable_case *entry) {
    double value;
    return mode != RXDA_JTABLE_NUMERIC || rxda_numeric_case_value(entry, &value);
}

static void rxda_output_jcases(FILE *stream, rxda_jtable *tables, size_t target) {
    while (tables) {
        if (tables->valid) {
            rxda_jtable_case *entry = tables->cases;
            size_t case_index = 0;
            while (entry) {
                if (entry->target == target && rxda_is_visible_jcase(tables->mode, entry)) {
                    fprintf(stream, "jt_%lx_case_%lu: .jcase jtable_%lx ",
                            (unsigned long)tables->pool_index,
                            (unsigned long)case_index,
                            (unsigned long)tables->pool_index);
                    if (rxda_output_jcase_literal(stream, tables->mode, entry)) fprintf(stream, "\n");
                }
                case_index++;
                entry = entry->next;
            }
        }
        tables = tables->next;
    }
}

static void rxda_free_jtables(rxda_jtable *tables) {
    while (tables) {
        rxda_jtable *next = tables->next;
        rxda_jtable_case *entry = tables->cases;
        while (entry) {
            rxda_jtable_case *case_next = entry->next;
            free(entry);
            entry = case_next;
        }
        free(tables);
        tables = next;
    }
}


/* Disassembly is quite straightforward - just needs to look up the instructions
 * and print it out. However, the complexity is to work out where and which labels
 * to include and where to put the procedure details.
 * So we have to run through the code and flag where to add label and procedure
 * details - in sum, 2 passes */

/* Get the address of the first meta constant entry at address (or -1 if none) */
static int get_first_meta_at(module_file *module, size_t address) {
    meta_entry *entry;
    int addr =  module->header.meta_head;
    while (addr != -1) {
        entry =  (meta_entry *)(module->constant +  addr);
        if (entry->address == address) return addr;
        else if (entry->address > address) return -1;
        addr = entry->next;
    }
    return -1;
}

static int get_next_meta_at(module_file *module, int addr, size_t address) {
    meta_entry *entry;

    if (addr == -1) return -1;

    entry = (meta_entry *)(module->constant + addr);
    addr = entry->next;
    if (addr == -1) return -1;

    entry = (meta_entry *)(module->constant + addr);
    if (entry->address != address) return -1;

    return addr;
}

static void mark_module_procedure_sources(module_file *module, bin_space *pgm, code_line *source) {
    int i = module->header.proc_head;

    while (i != -1) {
        proc_constant *entry = (proc_constant *)(module->constant + i);

        if (entry->base.type != PROC_CONST) break;
        if (entry->start < pgm->inst_size) {
            source[entry->start].flags = show_proc;
            source[entry->start].proc_index = (size_t)i;
        }
        i = entry->next;
    }
}

static void output_module_exposed_summary(FILE *stream, module_file *module, bin_space *pgm) {
    int i = module->header.expose_head;

    while (i != -1) {
        chameleon_constant *entry = (chameleon_constant *)(module->constant + i);

        if (entry->type == EXPOSE_PROC_CONST) {
            expose_proc_constant *eentry = (expose_proc_constant *)entry;
            proc_constant *pentry = (proc_constant *)(pgm->const_pool + eentry->procedure);

            if (eentry->imported) {
                fprintf(stream,
                        "* 0x%.6x EXPOSED-PROC %s() <-- as %s\n",
                        i,
                        pentry->name,
                        eentry->index);
            } else {
                fprintf(stream,
                        "* 0x%.6x EXPOSED-PROC %s() --> as %s\n",
                        i,
                        pentry->name,
                        eentry->index);
            }
            i = eentry->next;
        } else if (entry->type == EXPOSE_REG_CONST) {
            expose_reg_constant *rentry = (expose_reg_constant *)entry;
            fprintf(stream,
                    "* 0x%.6x EXPOSED-REG g%d <-> as %s\n",
                    i,
                    rentry->global_reg,
                    rentry->index);
            i = rentry->next;
        } else {
            break;
        }
    }
}

/* Print Meta Data for a single procedure - an imported one */
static void output_imported_proc_meta(FILE *stream, module_file *module, bin_space *pgm, size_t func) {
    char line_buffer[MAX_LINE_SIZE];
    int  m = module->header.meta_head;
    while (m != -1) {
        switch ( ((chameleon_constant*)(module->constant + m))->type ) {

            case META_FUNC: {
                /* META function symbol - .meta "MAIN"="B" ".int" main() "" */
                meta_func_constant *mentry = ((meta_func_constant *) (module->constant + m));
                if (mentry->func == func) {
                    char *raw_symbol;
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, ".meta %s=",line_buffer);
                    raw_symbol = get_const_raw_string_alloc(pgm, mentry->symbol);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                    fprintf(stream, "%s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                    fprintf(stream, " %s",line_buffer);
                    get_func_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->func);
                    fprintf(stream, " %s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->args);
                    fprintf(stream, " %s\n",line_buffer);
                    output_meta_inline_for_symbol(stream, module, pgm, raw_symbol, "");
                    output_meta_provider_for_symbol(stream, module, pgm, raw_symbol, "");
                    output_meta_autoload_for_symbol(stream, module, pgm,
                                                    raw_symbol, "");
                    output_meta_initializer_for_symbol(stream, module, pgm,
                                                       raw_symbol, "");
                    if (raw_symbol) free(raw_symbol);
                }
                m = mentry->base.next;
            }
            break;

            default: {
                meta_entry *mentry = ((meta_entry *) (module->constant + m));
                m = mentry->next;
            }
        }
    }
}

/* Print Meta Data for address starting a procedure - before the procedure header  */
static void output_meta_pre_proc(FILE *stream, module_file *module, bin_space *pgm, size_t address, int globals, int locals) {
    char line_buffer[MAX_LINE_SIZE];
    int  m;
    m  = get_first_meta_at(module, address);
    while (m != -1) {
        switch ( ((chameleon_constant*)(module->constant + m))->type ) {
            case META_SOURCE_STEP: {
                meta_source_step_constant *mentry = ((meta_source_step_constant *) (module->constant + m));
                (void)mentry;
            }
            break;

            case META_TRACE_EVENT: {
                meta_trace_event_constant *mentry = ((meta_trace_event_constant *) (module->constant + m));
                (void)mentry;
            }
            break;

            case META_FUNC: {
                /* META function symbol */
                meta_func_constant *mentry = ((meta_func_constant *) (module->constant + m));
                (void)mentry;
            }
            break;

            case META_CLASS: {
                /* META class */
                meta_class_constant *mentry = ((meta_class_constant *) (module->constant + m));
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                fprintf(stream, "                .meta %s=", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                fprintf(stream, "%s", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                fprintf(stream, " %s .class\n", line_buffer);
            }
            break;

            case META_ATTR: {
                /* META attribute */
                meta_attr_constant *mentry = ((meta_attr_constant *) (module->constant + m));
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                fprintf(stream, "                .meta %s=", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                fprintf(stream, "%s", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                fprintf(stream, " %s .attr %d\n", line_buffer, (int)mentry->reg);
            }
            break;

            case META_INTERFACE: {
                meta_interface_constant *mentry = ((meta_interface_constant *) (module->constant + m));
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                fprintf(stream, "                .meta %s=", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                fprintf(stream, "%s", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                fprintf(stream, " %s .interface\n", line_buffer);
            }
            break;

            case META_IMPLEMENTS: {
                meta_implements_constant *mentry = ((meta_implements_constant *) (module->constant + m));
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                fprintf(stream, "                .meta %s=", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->interface_symbol);
                fprintf(stream, "%s .implements\n", line_buffer);
            }
            break;

            case META_MEMBER: {
                meta_member_constant *mentry = ((meta_member_constant *) (module->constant + m));
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->owner);
                fprintf(stream, "                .meta %s=", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->kind);
                fprintf(stream, "%s", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->member);
                fprintf(stream, " %s", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                fprintf(stream, " %s", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->args);
                fprintf(stream, " %s .member\n", line_buffer);
            }
            break;

            case META_REG: {
                /* META clear symbol */
                meta_reg_constant *mentry = ((meta_reg_constant *) (module->constant + m));
                (void)mentry;
            }
            break;

            case META_CONST: {
                /* META const symbol */
                meta_const_constant *mentry = ((meta_const_constant *) (module->constant + m));
                (void)mentry;
            }
            break;

            case META_CLEAR: {
                /* META clear symbol - .meta "MAIN:I" */
                meta_clear_constant *mentry = ((meta_clear_constant *) (module->constant + m));
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                fprintf(stream, "                .meta %s\n", line_buffer);
            }
            break;

            default:
                break;
        }

        m = get_next_meta_at(module, m, address);
    }
}

/* Print Meta Data for address starting a procedure - after the procedure header  */
static void output_meta_post_proc(FILE *stream, module_file *module, bin_space *pgm, size_t address, int globals, int locals) {
    char line_buffer[MAX_LINE_SIZE];
    int  m;
    m  = get_first_meta_at(module, address);
    while (m != -1) {
        switch ( ((chameleon_constant*)(module->constant + m))->type ) {
            case META_SOURCE_STEP: {
                meta_source_step_constant *mentry = ((meta_source_step_constant *) (module->constant + m));
                output_meta_source_step_line(stream, pgm, mentry, "                ");
            }
            break;

            case META_TRACE_EVENT: {
                meta_trace_event_constant *mentry = ((meta_trace_event_constant *) (module->constant + m));
                output_meta_trace_event_line(stream, pgm, mentry, "                ");
            }
            break;

            case META_FUNC: {
                /* META function symbol - .meta "MAIN"="B" ".int" main() "" */

                meta_func_constant *mentry = ((meta_func_constant *) (module->constant + m));
                /* Only print procedure that are not imported  - because these are handled in the header */
                if (((proc_constant *)(module->constant + mentry->func))->start != SIZE_MAX) {

                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, "                .meta %s=", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                    fprintf(stream, "%s", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                    fprintf(stream, " %s", line_buffer);
                    get_func_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->func);
                    fprintf(stream, " %s", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->args);
                    fprintf(stream, " %s\n",line_buffer);

                }
            }
            break;

            case META_INLINE: {
                meta_inline_constant *mentry = ((meta_inline_constant *) (module->constant + m));
                output_meta_inline_line(stream, pgm, mentry, "                ");
            }
            break;

            case META_TASK_TARGET: {
                meta_task_target_constant *mentry =
                    ((meta_task_target_constant *) (module->constant + m));
                output_meta_task_target_line(stream, pgm, mentry, "                ");
            }
            break;

            case META_CLASS: {
                /* Emitted before the procedure header */
            }
            break;

            case META_ATTR: {
                /* Emitted before the procedure header */
            }
            break;

            case META_INTERFACE: {
                /* Emitted before the procedure header */
            }
            break;

            case META_IMPLEMENTS: {
                /* Emitted before the procedure header */
            }
            break;

            case META_MEMBER: {
                /* Emitted before the procedure header */
            }
            break;

            case META_REG: {
                /* META clear symbol - .meta "PROC:I"="B" ".int" a1 */
                meta_reg_constant *mentry = ((meta_reg_constant *) (module->constant + m));
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                fprintf(stream, "                .meta %s=",line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                fprintf(stream, "%s",line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                fprintf(stream, " %s",line_buffer);
                get_reg_string(pgm, line_buffer, MAX_LINE_SIZE, (int)mentry->reg, globals, locals);
                fprintf(stream, " %s\n",line_buffer);
            }
            break;

            case META_CONST: {
                /* META const symbol - .meta "MAIN:A"="B" ".STRING" "Hello" */
                meta_const_constant *mentry = ((meta_const_constant *) (module->constant + m));
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                fprintf(stream, "                .meta %s=",line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                fprintf(stream, "%s",line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                fprintf(stream, " %s",line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->constant);
                fprintf(stream, " %s\n",line_buffer);
            }
            break;

            case META_CLEAR: {
                /* META clear symbol - .meta "MAIN:I" */
                meta_clear_constant *mentry = ((meta_clear_constant *) (module->constant + m));
                (void)mentry;
            }
            break;

            default:
                break;
        }

        m = get_next_meta_at(module, m, address);
    }
}

/* Print Meta Data for general address */
static void output_meta(FILE *stream, module_file *module, bin_space *pgm, size_t address, int globals, int locals) {
    char line_buffer[MAX_LINE_SIZE];
    int  m;
    m  = get_first_meta_at(module, address);
    while (m != -1) {
        switch ( ((chameleon_constant*)(module->constant + m))->type ) {
            case META_SOURCE_STEP: {
                meta_source_step_constant *mentry = ((meta_source_step_constant *) (module->constant + m));
                output_meta_source_step_line(stream, pgm, mentry, "                ");
            }
            break;

            case META_TRACE_EVENT: {
                meta_trace_event_constant *mentry = ((meta_trace_event_constant *) (module->constant + m));
                output_meta_trace_event_line(stream, pgm, mentry, "                ");
            }
            break;

            case META_FUNC: {
                /* META function symbol - .meta "MAIN"="B" ".int" main() "" */

                meta_func_constant *mentry = ((meta_func_constant *) (module->constant + m));
                if (((proc_constant *)(module->constant + mentry->func))->start != SIZE_MAX) {

                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, "                .meta %s=", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                    fprintf(stream, "%s", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                    fprintf(stream, " %s", line_buffer);
                    get_func_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->func);
                    fprintf(stream, " %s", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->args);
                    fprintf(stream, " %s\n",line_buffer);

                }
            }
            break;

            case META_INLINE: {
                meta_inline_constant *mentry = ((meta_inline_constant *) (module->constant + m));
                output_meta_inline_line(stream, pgm, mentry, "                ");
            }
            break;

            case META_TASK_TARGET: {
                meta_task_target_constant *mentry =
                    ((meta_task_target_constant *) (module->constant + m));
                output_meta_task_target_line(stream, pgm, mentry, "                ");
            }
            break;

            case META_CLASS: {
                /* META class */
                meta_class_constant *mentry = ((meta_class_constant *) (module->constant + m));
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                fprintf(stream, "                .meta %s=", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                fprintf(stream, "%s", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                fprintf(stream, " %s .class\n", line_buffer);
            }
            break;

            case META_ATTR: {
                /* META attribute */
                meta_attr_constant *mentry = ((meta_attr_constant *) (module->constant + m));
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                fprintf(stream, "                .meta %s=", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                fprintf(stream, "%s", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                fprintf(stream, " %s .attr %d\n", line_buffer, (int)mentry->reg);
            }
            break;

            case META_INTERFACE: {
                meta_interface_constant *mentry = ((meta_interface_constant *) (module->constant + m));
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                fprintf(stream, "                .meta %s=", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                fprintf(stream, "%s", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                fprintf(stream, " %s .interface\n", line_buffer);
            }
            break;

            case META_IMPLEMENTS: {
                meta_implements_constant *mentry = ((meta_implements_constant *) (module->constant + m));
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                fprintf(stream, "                .meta %s=", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->interface_symbol);
                fprintf(stream, "%s .implements\n", line_buffer);
            }
            break;

            case META_MEMBER: {
                meta_member_constant *mentry = ((meta_member_constant *) (module->constant + m));
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->owner);
                fprintf(stream, "                .meta %s=", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->kind);
                fprintf(stream, "%s", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->member);
                fprintf(stream, " %s", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                fprintf(stream, " %s", line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->args);
                fprintf(stream, " %s .member\n", line_buffer);
            }
            break;

            case META_REG: {
                /* META clear symbol - .meta "PROC:I"="B" ".int" a1 */
                meta_reg_constant *mentry = ((meta_reg_constant *) (module->constant + m));
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                fprintf(stream, "                .meta %s=",line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                fprintf(stream, "%s",line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                fprintf(stream, " %s",line_buffer);
                get_reg_string(pgm, line_buffer, MAX_LINE_SIZE, (int)mentry->reg, globals, locals);
                fprintf(stream, " %s\n",line_buffer);
            }
            break;

            case META_CONST: {
                /* META const symbol - .meta "MAIN:A"="B" ".STRING" "Hello" */
                meta_const_constant *mentry = ((meta_const_constant *) (module->constant + m));
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                fprintf(stream, "                .meta %s=",line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                fprintf(stream, "%s",line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                fprintf(stream, " %s",line_buffer);
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->constant);
                fprintf(stream, " %s\n",line_buffer);
            }
            break;

            case META_CLEAR: {
                /* META clear symbol - .meta "MAIN:I" */
                meta_clear_constant *mentry = ((meta_clear_constant *) (module->constant + m));
                get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                fprintf(stream, "                .meta %s\n", line_buffer);
            }
            break;

            default:
                break;
        }

        m = get_next_meta_at(module, m, address);
    }
}

/* Actual Disassembler */
void disassemble(bin_space *pgm, module_file *module, FILE *stream, int print_all_constant_pool) {
    size_t i, j;
    chameleon_constant *entry;
    proc_constant* pentry;
    expose_proc_constant* eentry;
    char line_buffer[MAX_LINE_SIZE];
    size_t displayed_mnemonic_length;
    rxda_jtable *jump_tables;
    rxda_binary_alias *binary_aliases;

    /* Prepare to Print Code */
    i = 0;

    /* calloc() so values will be zeroed */
    code_line *source = calloc(pgm->inst_size, sizeof(code_line));

    /* Module Header */
    fprintf(stream, "*******************************************************************************\n"
                    "* MODULE - %s\n"
                    "* DESCRIPTION - %s\n\n", module->name, module->description );

    /* Pass 1 - Go through the code, decode and flag destination labels */
    while (i < pgm->inst_size) {
        j = i;
        int opcode = pgm->binary[i++].instruction.opcode;
        const OpInfo *op = &op_table[opcode];
        size_t num_ops = rxop_format_operand_count(op->format);
        size_t k;

        if (num_ops != pgm->binary[j].instruction.no_ops) {
            printf("BINARY ERROR - Instruction operand count mismatch @ 0x%.6x\n",(int)j);
        }

        for (k = 0; k < num_ops; k++) {
            if (rxop_format_operand_type(op->format, k) == OP_ID) {
                source[pgm->binary[i].index].flags = show_label;
            }
            i++;
        }
        source[j].op = op;
        source[j].comment = op->description;
        if (source[j].flags == operand) source[j].flags = normal;
    }

    mark_module_procedure_sources(module, pgm, source);
    jump_tables = rxda_collect_jtables(pgm, source);
    binary_aliases = rxda_collect_binary_aliases(pgm, source, jump_tables);

    /* Pass 1b - Print Constant Pool and add Proc entry flags to the code listing */
    fprintf(stream, "* CONSTANT POOL - Size 0x%lx.  ", pgm->const_size);
    if (print_all_constant_pool) fprintf(stream, "Dump of all entries (option -p used):\n\n");
    else fprintf(stream, "Dump of EXPOSED entries only (option -p not used):\n\n");

    if (print_all_constant_pool) {
        i = 0;
        while (i < pgm->const_size) {
            entry = (chameleon_constant *)(pgm->const_pool + i);
            switch(entry->type) {
                case STRING_CONST:
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, i);
                    fprintf(stream, "* 0x%.6lx STRING %s\n", i, line_buffer);
                    break;
                case DECIMAL_CONST:
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, i);
                    fprintf(stream, "* 0x%.6lx DECIMAL %s\n", i, line_buffer);
                    break;
                case FLOAT_CONST:
                    format_float_literal(line_buffer, MAX_LINE_SIZE, ((float_constant *)entry)->double_value);
                    fprintf(stream, "* 0x%.6lx FLOAT %s\n", i, line_buffer);
                    break;
                case BINARY_CONST:
                {
                    char* c = ((string_constant *)(pgm->const_pool + i))->string;
                    size_t sz = ((string_constant *)(pgm->const_pool + i))->string_len;
                    size_t encoded_length = encode_binary_to_hex(
                            line_buffer, MAX_LINE_SIZE, c, sz);
                    char *encoded = line_buffer;
                    if (encoded_length >= MAX_LINE_SIZE && encoded_length < SIZE_MAX) {
                        encoded = malloc(encoded_length + 1u);
                        if (encoded) {
                            encode_binary_to_hex(encoded, encoded_length + 1u, c, sz);
                        }
                        else {
                            encoded = line_buffer;
                        }
                    }
                    fprintf(stream, "* 0x%.6lx BINARY %s\n", i, encoded);
                    if (encoded != line_buffer) free(encoded);
                }
                    break;
                case PROC_CONST:
                    if (((proc_constant *) entry)->start == SIZE_MAX) {
                        fprintf(stream,
                                "* 0x%.6lx PROC %s() exposed from external <-- %s\n",
                                i,
                                ((proc_constant *) entry)->name,
                                ((expose_proc_constant*)(pgm->const_pool + ((proc_constant *) entry)->exposed))->index
                        );
                    } else {
                        fprintf(stream,
                                "* 0x%.6lx PROC @ 0x%.6lx %s() (locals=%d)\n",
                                i,
                                ((proc_constant *) entry)->start,
                                ((proc_constant *) entry)->name,
                                ((proc_constant *) entry)->locals
                        );
                    }
                    break;

                case EXPOSE_REG_CONST:
                    fprintf(stream,
                            "* 0x%.6lx EXPOSED-REG g%d <-> as %s\n",
                            i,
                            ((expose_reg_constant *) entry)->global_reg,
                            ((expose_reg_constant *) entry)->index);
                    break;

                case EXPOSE_PROC_CONST:
                    pentry = (proc_constant *) (pgm->const_pool + ((expose_proc_constant *) entry)->procedure);

                    if (((expose_proc_constant *) entry)->imported) {
                        fprintf(stream,
                                "* 0x%.6lx EXPOSED-PROC %s() <-- as %s\n",
                                i,
                                pentry->name,
                                ((expose_proc_constant *) entry)->index
                        );
                    } else {
                        fprintf(stream,
                                "* 0x%.6lx EXPOSED-PROC %s() --> as %s\n",
                                i,
                                pentry->name,
                                ((expose_proc_constant *) entry)->index
                        );
                    }
                    break;

                case META_SOURCE_STEP:
                {
                    meta_source_step_constant *mentry = (meta_source_step_constant *) entry;
                    fprintf(stream,
                            "* 0x%.6lx META-SOURCE-STEP @0x%.6lx step=%u clause=%u flags=%u %u:%u-%u ",
                            i,
                            mentry->base.address,
                            mentry->step_id,
                            mentry->clause_id,
                            mentry->flags,
                            mentry->line,
                            mentry->active_start_column,
                            mentry->active_end_column);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->file);
                    fprintf(stream, "%s ", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->source_line);
                    fprintf(stream, "%s\n", line_buffer);
                }
                    break;

                case META_TRACE_EVENT:
                {
                    meta_trace_event_constant *mentry = (meta_trace_event_constant *) entry;
                    char symbol_buffer[MAX_LINE_SIZE];
                    char resolved_buffer[MAX_LINE_SIZE];
                    trace_optional_string(pgm, mentry->symbol, symbol_buffer, sizeof(symbol_buffer));
                    trace_optional_string(pgm, mentry->resolved_name, resolved_buffer, sizeof(resolved_buffer));
                    fprintf(stream,
                            "* 0x%.6lx META-TRACE-EVENT @0x%.6lx kind=%c modes=%u source=%c type=%c reg=%c value=",
                            i,
                            mentry->base.address,
                            mentry->kind ? (char) mentry->kind : '-',
                            mentry->mode_mask,
                            mentry->value_source ? (char) mentry->value_source : '-',
                            mentry->value_type ? (char) mentry->value_type : '-',
                            mentry->register_type ? (char) mentry->register_type : '-');
                    if (mentry->value_ref == RXBIN_TRACE_REF_NONE) fprintf(stream, "-1");
                    else fprintf(stream, "%lu", (unsigned long) mentry->value_ref);
                    fprintf(stream,
                            " step=%u clause=%u flags=%u symbol=%s resolved=%s\n",
                            mentry->source_step_id,
                            mentry->clause_id,
                            mentry->flags,
                            symbol_buffer,
                            resolved_buffer);
                }
                    break;

                case META_FUNC:
                {
                    meta_func_constant *mentry = (meta_func_constant *)entry;
                    fprintf(stream, "* 0x%.6lx META-FUNC @0x%.6lx", i, mentry->base.address);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, " %s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                    fprintf(stream, " %s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                    fprintf(stream, " %s",line_buffer);
                    get_func_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->func);
                    fprintf(stream, " %s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->args);
                    fprintf(stream, " %s\n",line_buffer);
                }
                    break;

                case META_CLASS:
                {
                    meta_class_constant *mentry = (meta_class_constant *)entry;
                    fprintf(stream, "* 0x%.6lx META-CLASS @0x%.6lx", i, mentry->base.address);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, " %s", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                    fprintf(stream, " %s", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                    fprintf(stream, " %s\n", line_buffer);
                }
                    break;

                case META_ATTR:
                {
                    meta_attr_constant *mentry = (meta_attr_constant *)entry;
                    fprintf(stream, "* 0x%.6lx META-ATTR @0x%.6lx", i, mentry->base.address);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, " %s", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                    fprintf(stream, " %s", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                    fprintf(stream, " %s", line_buffer);
                    fprintf(stream, " %d\n", (int)mentry->reg);
                }
                    break;

                case META_INTERFACE:
                {
                    meta_interface_constant *mentry = (meta_interface_constant *)entry;
                    fprintf(stream, "* 0x%.6lx META-INTERFACE @0x%.6lx", i, mentry->base.address);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, " %s", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                    fprintf(stream, " %s", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                    fprintf(stream, " %s\n", line_buffer);
                }
                    break;

                case META_IMPLEMENTS:
                {
                    meta_implements_constant *mentry = (meta_implements_constant *)entry;
                    fprintf(stream, "* 0x%.6lx META-IMPLEMENTS @0x%.6lx", i, mentry->base.address);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, " %s", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->interface_symbol);
                    fprintf(stream, " %s\n", line_buffer);
                }
                    break;

                case META_MEMBER:
                {
                    meta_member_constant *mentry = (meta_member_constant *)entry;
                    fprintf(stream, "* 0x%.6lx META-MEMBER @0x%.6lx", i, mentry->base.address);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->owner);
                    fprintf(stream, " %s", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->kind);
                    fprintf(stream, " %s", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->member);
                    fprintf(stream, " %s", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                    fprintf(stream, " %s", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->args);
                    fprintf(stream, " %s\n", line_buffer);
                }
                    break;

                case META_INLINE:
                {
                    meta_inline_constant *mentry = (meta_inline_constant *)entry;
                    char *symbol = get_const_string_alloc(pgm, mentry->symbol);
                    char *payload = get_const_string_alloc(pgm, mentry->payload);
                    fprintf(stream, "* 0x%.6lx META-INLINE @0x%.6lx", i, mentry->base.address);
                    if (symbol && payload) fprintf(stream, " %s %s\n", symbol, payload);
                    else fprintf(stream, "\n");
                    if (symbol) free(symbol);
                    if (payload) free(payload);
                }
                    break;

                case META_TASK_TARGET:
                {
                    meta_task_target_constant *mentry =
                        (meta_task_target_constant *)entry;
                    char symbol_buffer[MAX_LINE_SIZE];
                    char binding_buffer[MAX_LINE_SIZE];
                    string_constant *binding =
                        (string_constant *)(pgm->const_pool + mentry->binding);
                    get_const_string(pgm,
                                     symbol_buffer,
                                     sizeof(symbol_buffer),
                                     mentry->symbol);
                    encode_binary_to_hex(binding_buffer,
                                         sizeof(binding_buffer),
                                         binding->string,
                                         binding->string_len);
                    fprintf(stream,
                            "* 0x%.6lx META-TASK-TARGET @0x%.6lx %s kind=%u %s\n",
                            i,
                            mentry->base.address,
                            symbol_buffer,
                            mentry->kind,
                            binding_buffer);
                }
                    break;

                case META_PROVIDER:
                {
                    meta_provider_constant *mentry =
                        (meta_provider_constant *)entry;
                    fprintf(stream,
                            "* 0x%.6lx META-PROVIDER @0x%.6lx flags=%u",
                            i, mentry->base.address, mentry->flags);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE,
                                     mentry->symbol);
                    fprintf(stream, " %s", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE,
                                     mentry->provider);
                    fprintf(stream, " %s\n", line_buffer);
                }
                    break;

                case META_INITIALIZER:
                {
                    meta_initializer_constant *mentry =
                        (meta_initializer_constant *)entry;
                    fprintf(stream,
                            "* 0x%.6lx META-INITIALIZER @0x%.6lx",
                            i, mentry->base.address);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE,
                                     mentry->symbol);
                    fprintf(stream, " %s", line_buffer);
                    get_func_string(pgm, line_buffer, MAX_LINE_SIZE,
                                    mentry->function);
                    fprintf(stream, " %s\n", line_buffer);
                }
                    break;

                case META_AUTOLOAD:
                {
                    meta_autoload_constant *mentry =
                        (meta_autoload_constant *)entry;
                    fprintf(stream,
                            "* 0x%.6lx META-AUTOLOAD @0x%.6lx",
                            i, mentry->base.address);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE,
                                     mentry->symbol);
                    fprintf(stream, " %s", line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE,
                                     mentry->artifact);
                    fprintf(stream, " %s\n", line_buffer);
                }
                    break;

                case META_REG:
                {
                    meta_reg_constant *mentry = (meta_reg_constant *)entry;
                    fprintf(stream, "* 0x%.6lx META-REG @0x%.6lx", i, mentry->base.address);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, " %s=",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                    fprintf(stream, " %s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                    fprintf(stream, " %s",line_buffer);
                    fprintf(stream, " r%d\n",(int)mentry->reg);
                }
                    break;

                case META_CONST:
                {
                    meta_const_constant *mentry = (meta_const_constant *)entry;
                    fprintf(stream, "* 0x%.6lx META-CONST @0x%.6lx", i, mentry->base.address);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, " %s=",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                    fprintf(stream, " %s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                    fprintf(stream, " %s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->constant);
                    fprintf(stream, " %s\n",line_buffer);
                }
                    break;

                case META_CLEAR:
                {
                    meta_clear_constant *mentry = (meta_clear_constant *) entry;
                    fprintf(stream, "* 0x%.6lx META-CLEAR @0x%.6lx", i, mentry->base.address);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, " %s\n", line_buffer);
                }
                    break;

                default:
                    fprintf(stream, "* 0x%.6lx UNKNOWN\n", i);
            }

            i += entry->size_in_pool;
        }
    } else {
        output_module_exposed_summary(stream, module, pgm);
    }

    /* Pass 2a - Generate listing output - number of globals & header information */
    int globals = pgm->globals;
    int locals = 0;
    fprintf(stream, "\n* CODE SEGMENT - Size 0x%lx\n", pgm->inst_size);
    fprintf(stream, "\n.globals=%d\n", globals);
    rxda_output_binary_aliases(stream, pgm, binary_aliases);

    /* Pass 2b - Exposed Registers and procedures exposed from external modules
     * Note that the compiler does not put all these at the top - but hey this is the easiest way for us */
    i = module->header.expose_head;
    while (i != -1) {
        entry = (chameleon_constant *)(pgm->const_pool + i);
        if (entry->type == EXPOSE_PROC_CONST) {
            eentry = (expose_proc_constant *)entry;
            if (eentry->imported) {
                pentry = (proc_constant *)(pgm->const_pool + eentry->procedure);
                fprintf(stream,
                        "%s() .expose=%s\n",
                        pentry->name,
                        eentry->index
                );
                output_imported_proc_meta(stream, module, pgm, eentry->procedure);
            }
            i = eentry->next;
        } else if (entry->type == EXPOSE_REG_CONST) {
            fprintf(stream,
                    "g%d .expose=%s\n",
                    ((expose_reg_constant *) entry)->global_reg,
                    ((expose_reg_constant *) entry)->index
            );
            i = ((expose_reg_constant *)entry)->next;
        } else {
            break;
        }
    }

    /* Pass 2c - The assembler code itself */
    i = 0;
    while (i < pgm->inst_size) {
        if (source[i].flags == show_proc) {
            output_meta_pre_proc(stream, module, pgm, i, globals, locals);
            pentry = (proc_constant *)(pgm->const_pool +
                                       source[i].proc_index);
            if (pentry->exposed == SIZE_MAX) {
                locals = pentry->locals;
                snprintf(line_buffer, MAX_LINE_SIZE, "%s()", pentry->name);
                fprintf(stream, "\n%-15s .locals=%d\n", line_buffer, locals);
                line_buffer[0] = 0;
            }
            else {
                eentry = (expose_proc_constant *)(pgm->const_pool +
                                             pentry->exposed);
                locals = pentry->locals;
                snprintf(line_buffer, MAX_LINE_SIZE, "%s()", pentry->name);
                fprintf(stream, "\n%-15s .locals=%d .expose=%s\n",
                        line_buffer,
                        locals,
                        eentry->index);
                line_buffer[0] = 0;
            }
            output_meta_post_proc(stream, module, pgm, i, globals, locals);
            rxda_output_jtable_declarations(stream, jump_tables, i);
        }
        else if (source[i].flags == show_label) {
            snprintf(line_buffer, MAX_LINE_SIZE, "lb_%lx:", i);
            output_meta(stream, module, pgm, i, globals, locals);
        }
        else {
            line_buffer[0] = 0;
            output_meta(stream, module, pgm, i, globals, locals);
        }

        rxda_output_jcases(stream, jump_tables, i);

        j = i++;
        fprintf(stream, "%-15s",line_buffer);

        {
            char mnemonic[MAX_LINE_SIZE];
            get_mnemonic(mnemonic, source[j].op->mnemonic);
            displayed_mnemonic_length = strlen(mnemonic);
            fprintf(stream, " %s ", mnemonic);
            line_buffer[0] = 0;
        }

        {
            size_t num_ops = rxop_format_operand_count(source[j].op->format);
            size_t instruction_width = 2 + displayed_mnemonic_length;
            enum rxda_jtable_mode jump_mode;
            int table_operand = -1;
            size_t k;
            (void)rxda_jump_info(source[j].op, &jump_mode, &table_operand);
            for (k = 0; k < num_ops; k++) {
                size_t operand_cell;
                size_t operand_length;
                char *operand_text;
                if (k > 0) {
                    fputc(',', stream);
                    instruction_width++;
                }
                if (k == table_operand) {
                    size_t pool_index = pgm->binary[i].index;
                    rxda_jtable *table = rxda_find_jtable(jump_tables, pool_index);
                    if (table && table->valid) {
                        size_t length = (size_t)snprintf(line_buffer, MAX_LINE_SIZE,
                                                        "jtable_%lx", (unsigned long)pool_index);
                        fputs(line_buffer, stream);
                        instruction_width += length;
                        i++;
                        continue;
                    }
                }
                operand_cell = i++;
                operand_length = disassemble_operand(
                        pgm, module, source[j].op->opcode, (unsigned int)k,
                        line_buffer, MAX_LINE_SIZE,
                        rxop_format_operand_type(source[j].op->format, k),
                        operand_cell, binary_aliases, globals, locals);
                instruction_width += operand_length;
                operand_text = line_buffer;
                if (operand_length >= MAX_LINE_SIZE && operand_length < SIZE_MAX) {
                    operand_text = malloc(operand_length + 1u);
                    if (operand_text) {
                        disassemble_operand(
                                pgm, module, source[j].op->opcode, (unsigned int)k,
                                operand_text, operand_length + 1u,
                                rxop_format_operand_type(source[j].op->format, k),
                                operand_cell, binary_aliases, globals, locals);
                    }
                    else {
                        operand_text = line_buffer;
                    }
                }
                fputs(operand_text, stream);
                if (operand_text != line_buffer) free(operand_text);
            }
            while (instruction_width++ < 45) fputc(' ', stream);
        }
        fprintf(stream, " * 0x%.6lx:%.4x %s\n", j, source[j].op->opcode, source[j].comment);
    }

    /* Final Meta entries at the code address + 1 */
    output_meta(stream, module, pgm, i, globals, locals);

    fprintf(stream, "*******************************************************************************\n\n");

    /* Free memory */
    rxda_free_binary_aliases(binary_aliases);
    rxda_free_jtables(jump_tables);
    free(source);
}
