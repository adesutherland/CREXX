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
#include <ctype.h>
#include "platform.h"
#include "rxas.h"
#include "rxdadism.h"
#include "../binutils/include/rxdefs.h"

/* Max buffer size - todo change to a dynamic solution */
#define MAX_LINE_SIZE 5000
#include "../binutils/include/opdata.c"

static int get_operand_types(OpFormat format, OperandType *types) {
    switch (format) {
        case FMT_EMPTY: return 0;
        case FMT_B: types[0] = OP_BINARY; return 1;
        case FMT_C: types[0] = OP_CHAR; return 1;
        case FMT_F: types[0] = OP_FLOAT; return 1;
        case FMT_I: types[0] = OP_INT; return 1;
        case FMT_I_I: types[0] = OP_INT; types[1] = OP_INT; return 2;
        case FMT_I_I_I: types[0] = OP_INT; types[1] = OP_INT; types[2] = OP_INT; return 3;
        case FMT_I_I_R: types[0] = OP_INT; types[1] = OP_INT; types[2] = OP_REG; return 3;
        case FMT_I_R: types[0] = OP_INT; types[1] = OP_REG; return 2;
        case FMT_I_R_R: types[0] = OP_INT; types[1] = OP_REG; types[2] = OP_REG; return 3;
        case FMT_L: types[0] = OP_ID; return 1;
        case FMT_L_L_R: types[0] = OP_ID; types[1] = OP_ID; types[2] = OP_REG; return 3;
        case FMT_L_P_S: types[0] = OP_ID; types[1] = OP_FUNC; types[2] = OP_STRING; return 3;
        case FMT_L_R: types[0] = OP_ID; types[1] = OP_REG; return 2;
        case FMT_L_R_I: types[0] = OP_ID; types[1] = OP_REG; types[2] = OP_INT; return 3;
        case FMT_L_R_R: types[0] = OP_ID; types[1] = OP_REG; types[2] = OP_REG; return 3;
        case FMT_L_S: types[0] = OP_ID; types[1] = OP_STRING; return 2;
        case FMT_P: types[0] = OP_FUNC; return 1;
        case FMT_P_S: types[0] = OP_FUNC; types[1] = OP_STRING; return 2;
        case FMT_R: types[0] = OP_REG; return 1;
        case FMT_R_B: types[0] = OP_REG; types[1] = OP_BINARY; return 2;
        case FMT_R_C: types[0] = OP_REG; types[1] = OP_CHAR; return 2;
        case FMT_R_D: types[0] = OP_REG; types[1] = OP_DECIMAL; return 2;
        case FMT_R_D_R: types[0] = OP_REG; types[1] = OP_DECIMAL; types[2] = OP_REG; return 3;
        case FMT_R_F: types[0] = OP_REG; types[1] = OP_FLOAT; return 2;
        case FMT_R_F_I: types[0] = OP_REG; types[1] = OP_FLOAT; types[2] = OP_INT; return 3;
        case FMT_R_F_R: types[0] = OP_REG; types[1] = OP_FLOAT; types[2] = OP_REG; return 3;
        case FMT_R_I: types[0] = OP_REG; types[1] = OP_INT; return 2;
        case FMT_R_I_I: types[0] = OP_REG; types[1] = OP_INT; types[2] = OP_INT; return 3;
        case FMT_R_I_R: types[0] = OP_REG; types[1] = OP_INT; types[2] = OP_REG; return 3;
        case FMT_R_P: types[0] = OP_REG; types[1] = OP_FUNC; return 2;
        case FMT_R_P_R: types[0] = OP_REG; types[1] = OP_FUNC; types[2] = OP_REG; return 3;
        case FMT_R_R: types[0] = OP_REG; types[1] = OP_REG; return 2;
        case FMT_R_R_D: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_DECIMAL; return 3;
        case FMT_R_R_F: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_FLOAT; return 3;
        case FMT_R_R_I: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_INT; return 3;
        case FMT_R_R_R: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_REG; return 3;
        case FMT_R_R_S: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_STRING; return 3;
        case FMT_R_S: types[0] = OP_REG; types[1] = OP_STRING; return 2;
        case FMT_R_S_I: types[0] = OP_REG; types[1] = OP_STRING; types[2] = OP_INT; return 3;
        case FMT_R_S_R: types[0] = OP_REG; types[1] = OP_STRING; types[2] = OP_REG; return 3;
        case FMT_R_S_S: types[0] = OP_REG; types[1] = OP_STRING; types[2] = OP_STRING; return 3;
        case FMT_S: types[0] = OP_STRING; return 1;
        case FMT_S_R: types[0] = OP_STRING; types[1] = OP_REG; return 2;
        case FMT_S_S: types[0] = OP_STRING; types[1] = OP_STRING; return 2;
        case FMT_S_S_R: types[0] = OP_STRING; types[1] = OP_STRING; types[2] = OP_REG; return 3;
        default: return 0;
    }
}

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
static size_t encode_binary_to_hex(char* buffer, size_t buffer_len, char* binary, size_t length) {
    size_t out_len = 0;

    out_len++; if (buffer_len) { *(buffer++) = '0'; buffer_len--; }
    out_len++; if (buffer_len) { *(buffer++) = 'x'; buffer_len--; }

    while (length) {
        snprintf(buffer, buffer_len, "%02x", (unsigned int)(unsigned char)*binary);
        buffer += 2;
        binary++;
        length--;
        out_len += 2;
        buffer_len -= 2;
    }
    if (buffer_len) *buffer = 0;
    return out_len;
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

/* Disassemble an operand
 *
 * Returns the number of characters that would have been written assuming the
 * buffer was big enough - like snprintf() */
static size_t disassemble_operand(bin_space *pgm, char* buffer, size_t buffer_len,
                                  OperandType type, size_t index, int globals, int locals) {

    size_t out_len;
    size_t ix;
    char *c;
    size_t sz;
    int r;

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
#ifdef __32BIT__
            out_len = snprintf(buffer, buffer_len, "%d", pgm->binary[index].iconst);
#else
            out_len = snprintf(buffer, buffer_len, "%lld", pgm->binary[index].iconst);
#endif
            break;
        case OP_FLOAT:
            out_len = snprintf(buffer, buffer_len, "%f",
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
            c = ((string_constant *)(pgm->const_pool + ix))->string;
            sz = ((string_constant *)(pgm->const_pool + ix))->string_len;
            out_len = encode_binary_to_hex(buffer, buffer_len, c, sz);
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
    size_t line_len;

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
        OperandType types[3];
        int num_ops = get_operand_types(op->format, types);
        int k;

        if (num_ops != pgm->binary[j].instruction.no_ops) {
            printf("BINARY ERROR - Instruction operand count mismatch @ 0x%.6x\n",(int)j);
        }

        for (k = 0; k < num_ops; k++) {
            if (types[k] == OP_ID) source[pgm->binary[i].index].flags = show_label;
            i++;
        }
        source[j].op = op;
        source[j].comment = op->description;
        if (source[j].flags == operand) source[j].flags = normal;
    }

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
                    fprintf(stream, "* 0x%.6lx FLOAT %f\n", i, ((float_constant *)entry)->double_value);
                    break;
                case BINARY_CONST:
                {
                    char* c = ((string_constant *)(pgm->const_pool + i))->string;
                    size_t sz = ((string_constant *)(pgm->const_pool + i))->string_len;
                    encode_binary_to_hex(line_buffer, MAX_LINE_SIZE, c, sz);
                    fprintf(stream, "* 0x%.6lx BINARY %s\n", i, line_buffer);
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

    mark_module_procedure_sources(module, pgm, source);

    /* Pass 2a - Generate listing output - number of globals & header information */
    int globals = pgm->globals;
    int locals = 0;
    fprintf(stream, "\n* CODE SEGMENT - Size 0x%lx\n", pgm->inst_size);
    fprintf(stream, "\n.globals=%d\n", globals);

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
        }
        else if (source[i].flags == show_label) {
            snprintf(line_buffer, MAX_LINE_SIZE, "lb_%lx:", i);
            output_meta(stream, module, pgm, i, globals, locals);
        }
        else {
            line_buffer[0] = 0;
            output_meta(stream, module, pgm, i, globals, locals);
        }

        j = i++;
        fprintf(stream, "%-15s",line_buffer);

        {
            char mnemonic[MAX_LINE_SIZE];
            get_mnemonic(mnemonic, source[j].op->mnemonic);
            line_len = snprintf(line_buffer, MAX_LINE_SIZE, " %s ", mnemonic);
        }

        {
            OperandType types[3];
            int num_ops = get_operand_types(source[j].op->format, types);
            int k;
            for (k = 0; k < num_ops; k++) {
                if (k > 0) {
                    line_len += snprintf(line_buffer + line_len, MAX_LINE_SIZE - line_len, ",");
                }
                line_len += disassemble_operand(pgm, line_buffer + line_len, MAX_LINE_SIZE - line_len, types[k], i++, globals, locals);
            }
        }
        fprintf(stream, "%-45s * 0x%.6lx:%.4x %s\n", line_buffer, j, source[j].op->opcode, source[j].comment);
    }

    /* Final Meta entries at the code address + 1 */
    output_meta(stream, module, pgm, i, globals, locals);

    fprintf(stream, "*******************************************************************************\n\n");

    /* Free memory */
    free(source);
}
