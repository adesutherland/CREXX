#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxdadism.h"
#include "rxbin.h"

#define ALIGN8(value) (((value) + (size_t)7) & ~((size_t)7))

static size_t reserve_entry(unsigned char *pool, size_t *used, size_t raw_size, enum const_pool_type type) {
    size_t offset = *used;
    size_t size = ALIGN8(raw_size);
    chameleon_constant *entry = (chameleon_constant *)(pool + offset);
    memset(entry, 0, size);
    entry->size_in_pool = size;
    entry->type = type;
    *used += size;
    return offset;
}

static size_t add_string_entry(unsigned char *pool, size_t *used, enum const_pool_type type, const char *data, size_t len) {
    size_t offset = reserve_entry(pool, used, sizeof(string_constant) + len, type);
    string_constant *entry = (string_constant *)(pool + offset);
    entry->string_len = len;
#ifndef NUTF8
    entry->string_chars = len;
#endif
    memcpy(entry->string, data, len);
    entry->string[len] = 0;
    return offset;
}

static size_t add_float_entry(unsigned char *pool, size_t *used, double value) {
    size_t offset = reserve_entry(pool, used, sizeof(float_constant), FLOAT_CONST);
    float_constant *entry = (float_constant *)(pool + offset);
    entry->double_value = value;
    return offset;
}

static size_t add_proc_entry(unsigned char *pool, size_t *used, const char *name, int locals, size_t start) {
    size_t offset = reserve_entry(pool, used, sizeof(proc_constant) + strlen(name), PROC_CONST);
    proc_constant *entry = (proc_constant *)(pool + offset);
    entry->next = -1;
    entry->locals = locals;
    entry->start = start;
    entry->exposed = SIZE_MAX;
    memcpy(entry->name, name, strlen(name) + 1);
    return offset;
}

static size_t add_expose_reg_entry(unsigned char *pool, size_t *used, const char *index, int global_reg) {
    size_t offset = reserve_entry(pool, used, sizeof(expose_reg_constant) + strlen(index), EXPOSE_REG_CONST);
    expose_reg_constant *entry = (expose_reg_constant *)(pool + offset);
    entry->next = -1;
    entry->global_reg = global_reg;
    memcpy(entry->index, index, strlen(index) + 1);
    return offset;
}

static size_t add_expose_proc_entry(unsigned char *pool, size_t *used, const char *index, size_t procedure, int imported) {
    size_t offset = reserve_entry(pool, used, sizeof(expose_proc_constant) + strlen(index), EXPOSE_PROC_CONST);
    expose_proc_constant *entry = (expose_proc_constant *)(pool + offset);
    entry->next = -1;
    entry->procedure = procedure;
    entry->imported = imported ? 1u : 0u;
    memcpy(entry->index, index, strlen(index) + 1);
    return offset;
}

static size_t add_meta_file_entry(unsigned char *pool, size_t *used, size_t address, size_t file) {
    size_t offset = reserve_entry(pool, used, sizeof(meta_file_constant), META_FILE);
    meta_file_constant *entry = (meta_file_constant *)(pool + offset);
    entry->base.next = -1;
    entry->base.address = address;
    entry->file = file;
    return offset;
}

static size_t add_meta_src_entry(unsigned char *pool, size_t *used, size_t address, int line, int column, size_t source) {
    size_t offset = reserve_entry(pool, used, sizeof(meta_src_constant), META_SRC);
    meta_src_constant *entry = (meta_src_constant *)(pool + offset);
    entry->base.next = -1;
    entry->base.address = address;
    entry->line = line;
    entry->column = column;
    entry->source = source;
    return offset;
}

static size_t add_meta_func_entry(unsigned char *pool, size_t *used, size_t address, size_t symbol, size_t option, size_t type, size_t func, size_t args) {
    size_t offset = reserve_entry(pool, used, sizeof(meta_func_constant), META_FUNC);
    meta_func_constant *entry = (meta_func_constant *)(pool + offset);
    entry->base.next = -1;
    entry->base.address = address;
    entry->symbol = symbol;
    entry->option = option;
    entry->type = type;
    entry->func = func;
    entry->args = args;
    return offset;
}

static size_t add_meta_inline_entry(unsigned char *pool, size_t *used, size_t address, size_t symbol, size_t payload) {
    size_t offset = reserve_entry(pool, used, sizeof(meta_inline_constant), META_INLINE);
    meta_inline_constant *entry = (meta_inline_constant *)(pool + offset);
    entry->base.next = -1;
    entry->base.address = address;
    entry->symbol = symbol;
    entry->payload = payload;
    return offset;
}

static size_t add_meta_reg_entry(unsigned char *pool, size_t *used, size_t address, size_t symbol, size_t option, size_t type, size_t reg) {
    size_t offset = reserve_entry(pool, used, sizeof(meta_reg_constant), META_REG);
    meta_reg_constant *entry = (meta_reg_constant *)(pool + offset);
    entry->base.next = -1;
    entry->base.address = address;
    entry->symbol = symbol;
    entry->option = option;
    entry->type = type;
    entry->reg = reg;
    return offset;
}

static size_t add_meta_const_entry(unsigned char *pool, size_t *used, size_t address, size_t symbol, size_t option, size_t type, size_t constant) {
    size_t offset = reserve_entry(pool, used, sizeof(meta_const_constant), META_CONST);
    meta_const_constant *entry = (meta_const_constant *)(pool + offset);
    entry->base.next = -1;
    entry->base.address = address;
    entry->symbol = symbol;
    entry->option = option;
    entry->type = type;
    entry->constant = constant;
    return offset;
}

static size_t add_meta_clear_entry(unsigned char *pool, size_t *used, size_t address, size_t symbol) {
    size_t offset = reserve_entry(pool, used, sizeof(meta_clear_constant), META_CLEAR);
    meta_clear_constant *entry = (meta_clear_constant *)(pool + offset);
    entry->base.next = -1;
    entry->base.address = address;
    entry->symbol = symbol;
    return offset;
}

static size_t add_meta_class_entry(unsigned char *pool, size_t *used, size_t address, size_t symbol, size_t option, size_t type) {
    size_t offset = reserve_entry(pool, used, sizeof(meta_class_constant), META_CLASS);
    meta_class_constant *entry = (meta_class_constant *)(pool + offset);
    entry->base.next = -1;
    entry->base.address = address;
    entry->symbol = symbol;
    entry->option = option;
    entry->type = type;
    return offset;
}

static size_t add_meta_attr_entry(unsigned char *pool, size_t *used, size_t address, size_t symbol, size_t option, size_t type, size_t reg) {
    size_t offset = reserve_entry(pool, used, sizeof(meta_attr_constant), META_ATTR);
    meta_attr_constant *entry = (meta_attr_constant *)(pool + offset);
    entry->base.next = -1;
    entry->base.address = address;
    entry->symbol = symbol;
    entry->option = option;
    entry->type = type;
    entry->reg = reg;
    return offset;
}

static size_t add_meta_interface_entry(unsigned char *pool, size_t *used, size_t address, size_t symbol, size_t option, size_t type) {
    size_t offset = reserve_entry(pool, used, sizeof(meta_interface_constant), META_INTERFACE);
    meta_interface_constant *entry = (meta_interface_constant *)(pool + offset);
    entry->base.next = -1;
    entry->base.address = address;
    entry->symbol = symbol;
    entry->option = option;
    entry->type = type;
    return offset;
}

static size_t add_meta_implements_entry(unsigned char *pool, size_t *used, size_t address, size_t symbol, size_t interface_symbol) {
    size_t offset = reserve_entry(pool, used, sizeof(meta_implements_constant), META_IMPLEMENTS);
    meta_implements_constant *entry = (meta_implements_constant *)(pool + offset);
    entry->base.next = -1;
    entry->base.address = address;
    entry->symbol = symbol;
    entry->interface_symbol = interface_symbol;
    return offset;
}

static size_t add_meta_member_entry(unsigned char *pool, size_t *used, size_t address, size_t owner, size_t kind, size_t member, size_t type, size_t args) {
    size_t offset = reserve_entry(pool, used, sizeof(meta_member_constant), META_MEMBER);
    meta_member_constant *entry = (meta_member_constant *)(pool + offset);
    entry->base.next = -1;
    entry->base.address = address;
    entry->owner = owner;
    entry->kind = kind;
    entry->member = member;
    entry->type = type;
    entry->args = args;
    return offset;
}

static char *read_stream(FILE *stream) {
    char *buffer;
    long size;
    size_t read_size;

    if (fseek(stream, 0L, SEEK_END) != 0) return 0;
    size = ftell(stream);
    if (size < 0) return 0;
    rewind(stream);

    buffer = malloc((size_t)size + 1u);
    if (!buffer) return 0;

    read_size = fread(buffer, 1u, (size_t)size, stream);
    buffer[read_size] = 0;
    return buffer;
}

static int require_contains(const char *haystack, const char *needle) {
    if (!strstr(haystack, needle)) {
        fprintf(stderr, "Missing expected output: %s\n", needle);
        return 1;
    }
    return 0;
}

static int require_not_contains(const char *haystack, const char *needle) {
    if (strstr(haystack, needle)) {
        fprintf(stderr, "Unexpected output fragment: %s\n", needle);
        return 1;
    }
    return 0;
}

int main(void) {
    static const char binary_value[] = { 0x41, 0x42, 0x43 };
    unsigned char *pool;
    bin_code *code;
    bin_space pgm;
    module_file module;
    FILE *stream;
    char *output;
    size_t used;
    size_t s_hello;
    size_t s_decimal;
    size_t s_file;
    size_t s_source;
    size_t s_b;
    size_t s_void;
    size_t s_empty;
    size_t s_string_type;
    size_t s_unknown;
    size_t s_shape_type;
    size_t s_word_symbol;
    size_t s_word_args;
    size_t s_main_symbol;
    size_t s_helper_symbol;
    size_t s_local_symbol;
    size_t s_answer_symbol;
    size_t s_answer_value;
    size_t s_box_symbol;
    size_t s_box_attr_symbol;
    size_t s_shape_symbol;
    size_t s_method_kind;
    size_t s_member_name;
    size_t s_inline_payload;
    size_t binary_const;
    size_t float_const;
    size_t proc_main;
    size_t proc_word;
    size_t proc_helper;
    size_t expose_reg;
    size_t expose_word;
    size_t expose_helper;
    size_t meta_file;
    size_t meta_class;
    size_t meta_attr;
    size_t meta_interface;
    size_t meta_implements;
    size_t meta_member;
    size_t meta_word_func;
    size_t meta_word_inline;
    size_t meta_main_func;
    size_t meta_main_src;
    size_t meta_main_reg;
    size_t meta_main_const;
    size_t meta_helper_func;
    size_t meta_clear;
    int rc;

    pool = calloc(1u, 8192u);
    code = calloc(2u, sizeof(bin_code));
    output = 0;
    stream = 0;
    rc = 0;

    if (!pool || !code) {
        fprintf(stderr, "allocation failed\n");
        free(pool);
        free(code);
        return 1;
    }

    used = 0;
    s_hello = add_string_entry(pool, &used, STRING_CONST, "hello", 5u);
    s_decimal = add_string_entry(pool, &used, DECIMAL_CONST, "12.5", 4u);
    s_file = add_string_entry(pool, &used, STRING_CONST, "synthetic.rexx", 14u);
    s_source = add_string_entry(pool, &used, STRING_CONST, "say synthetic", 13u);
    s_b = add_string_entry(pool, &used, STRING_CONST, "b", 1u);
    s_void = add_string_entry(pool, &used, STRING_CONST, ".void", 5u);
    s_empty = add_string_entry(pool, &used, STRING_CONST, "", 0u);
    s_string_type = add_string_entry(pool, &used, STRING_CONST, ".string", 7u);
    s_unknown = add_string_entry(pool, &used, STRING_CONST, ".unknown", 8u);
    s_shape_type = add_string_entry(pool, &used, STRING_CONST, ".shape", 6u);
    s_word_symbol = add_string_entry(pool, &used, STRING_CONST, "rxfnsb.word", 11u);
    s_word_args = add_string_entry(pool, &used, STRING_CONST, "string=.string,wordnum=.int", 28u);
    s_main_symbol = add_string_entry(pool, &used, STRING_CONST, "synthetic.main", 14u);
    s_helper_symbol = add_string_entry(pool, &used, STRING_CONST, "synthetic.helper", 16u);
    s_local_symbol = add_string_entry(pool, &used, STRING_CONST, "synthetic.main.local", 20u);
    s_answer_symbol = add_string_entry(pool, &used, STRING_CONST, "synthetic.main.answer", 21u);
    s_answer_value = add_string_entry(pool, &used, STRING_CONST, "answer", 6u);
    s_box_symbol = add_string_entry(pool, &used, STRING_CONST, "synthetic.box", 13u);
    s_box_attr_symbol = add_string_entry(pool, &used, STRING_CONST, "synthetic.box.value", 19u);
    s_shape_symbol = add_string_entry(pool, &used, STRING_CONST, "synthetic.shape", 15u);
    s_method_kind = add_string_entry(pool, &used, STRING_CONST, "method", 6u);
    s_member_name = add_string_entry(pool, &used, STRING_CONST, "describe", 8u);
    s_inline_payload = add_string_entry(pool, &used, STRING_CONST, "I4;a;b", 6u);
    binary_const = add_string_entry(pool, &used, BINARY_CONST, binary_value, 3u);
    float_const = add_float_entry(pool, &used, 1.5);

    proc_main = add_proc_entry(pool, &used, "main", 1, 0u);
    proc_word = add_proc_entry(pool, &used, "word", -1, SIZE_MAX);
    proc_helper = add_proc_entry(pool, &used, "helper", 0, 1u);

    expose_reg = add_expose_reg_entry(pool, &used, "synthetic.shared", 0);
    expose_word = add_expose_proc_entry(pool, &used, "rxfnsb.word", proc_word, 1);
    expose_helper = add_expose_proc_entry(pool, &used, "synthetic.helper", proc_helper, 0);

    ((proc_constant *)(pool + proc_main))->next = (int)proc_word;
    ((proc_constant *)(pool + proc_word))->next = (int)proc_helper;
    ((proc_constant *)(pool + proc_word))->exposed = expose_word;
    ((proc_constant *)(pool + proc_helper))->exposed = expose_helper;
    ((expose_reg_constant *)(pool + expose_reg))->next = (int)expose_word;
    ((expose_proc_constant *)(pool + expose_word))->next = (int)expose_helper;

    meta_file = add_meta_file_entry(pool, &used, 0u, s_file);
    meta_class = add_meta_class_entry(pool, &used, 0u, s_box_symbol, s_b, s_unknown);
    meta_attr = add_meta_attr_entry(pool, &used, 0u, s_box_attr_symbol, s_b, s_string_type, 1u);
    meta_interface = add_meta_interface_entry(pool, &used, 0u, s_shape_symbol, s_b, s_unknown);
    meta_implements = add_meta_implements_entry(pool, &used, 0u, s_box_symbol, s_shape_symbol);
    meta_member = add_meta_member_entry(pool, &used, 0u, s_shape_symbol, s_method_kind, s_member_name, s_string_type, s_empty);
    meta_word_func = add_meta_func_entry(pool, &used, 0u, s_word_symbol, s_b, s_string_type, proc_word, s_word_args);
    meta_word_inline = add_meta_inline_entry(pool, &used, 0u, s_word_symbol, s_inline_payload);
    meta_main_func = add_meta_func_entry(pool, &used, 0u, s_main_symbol, s_b, s_void, proc_main, s_empty);
    meta_main_src = add_meta_src_entry(pool, &used, 0u, 1, 1, s_source);
    meta_main_reg = add_meta_reg_entry(pool, &used, 0u, s_local_symbol, s_b, s_string_type, 0u);
    meta_main_const = add_meta_const_entry(pool, &used, 0u, s_answer_symbol, s_b, s_string_type, s_answer_value);
    meta_helper_func = add_meta_func_entry(pool, &used, 1u, s_helper_symbol, s_b, s_void, proc_helper, s_empty);
    meta_clear = add_meta_clear_entry(pool, &used, 2u, s_local_symbol);

    ((meta_entry *)(pool + meta_file))->next = (int)meta_class;
    ((meta_entry *)(pool + meta_class))->next = (int)meta_attr;
    ((meta_entry *)(pool + meta_attr))->next = (int)meta_interface;
    ((meta_entry *)(pool + meta_interface))->next = (int)meta_implements;
    ((meta_entry *)(pool + meta_implements))->next = (int)meta_member;
    ((meta_entry *)(pool + meta_member))->next = (int)meta_word_func;
    ((meta_entry *)(pool + meta_word_func))->next = (int)meta_word_inline;
    ((meta_entry *)(pool + meta_word_inline))->next = (int)meta_main_func;
    ((meta_entry *)(pool + meta_main_func))->next = (int)meta_main_src;
    ((meta_entry *)(pool + meta_main_src))->next = (int)meta_main_reg;
    ((meta_entry *)(pool + meta_main_reg))->next = (int)meta_main_const;
    ((meta_entry *)(pool + meta_main_const))->next = (int)meta_helper_func;
    ((meta_entry *)(pool + meta_helper_func))->next = (int)meta_clear;

    memset(&pgm, 0, sizeof(pgm));
    init_module(&module);
    module.name = "synthetic";
    module.description = "synthetic";
    module.instructions = code;
    module.constant = pool;
    module.header.globals = 1;
    module.header.proc_head = (int)proc_main;
    module.header.expose_head = (int)expose_reg;
    module.header.meta_head = (int)meta_file;

    pgm.globals = 1;
    pgm.inst_size = 2u;
    pgm.const_size = used;
    pgm.binary = code;
    pgm.const_pool = pool;

    code[0].instruction.opcode = 48;
    code[0].instruction.no_ops = 0;
    code[1].instruction.opcode = 48;
    code[1].instruction.no_ops = 0;

    stream = tmpfile();
    if (!stream) {
        fprintf(stderr, "tmpfile failed\n");
        free(pool);
        free(code);
        return 1;
    }

    disassemble(&pgm, &module, stream, 1);

    output = read_stream(stream);
    fclose(stream);
    if (!output) {
        fprintf(stderr, "failed to read disassembly\n");
        free(pool);
        free(code);
        return 1;
    }

    rc |= require_contains(output, "STRING \"hello\"");
    rc |= require_contains(output, "DECIMAL \"12.5\"");
    rc |= require_contains(output, "FLOAT 1.500000");
    rc |= require_contains(output, "BINARY 0x414243");
    rc |= require_contains(output, "EXPOSED-REG g0 <-> as synthetic.shared");
    rc |= require_contains(output, "EXPOSED-PROC word() <-- as rxfnsb.word");
    rc |= require_contains(output, "EXPOSED-PROC helper() --> as synthetic.helper");
    rc |= require_contains(output, "word() .expose=rxfnsb.word");
    rc |= require_contains(output, ".srcfile=\"synthetic.rexx\"");
    rc |= require_contains(output, ".meta \"synthetic.box\"=\"b\" \".unknown\" .class");
    rc |= require_contains(output, ".meta \"synthetic.box.value\"=\"b\" \".string\" .attr 1");
    rc |= require_contains(output, ".meta \"synthetic.shape\"=\"b\" \".unknown\" .interface");
    rc |= require_contains(output, ".meta \"synthetic.box\"=\"synthetic.shape\" .implements");
    rc |= require_contains(output, ".meta \"synthetic.shape\"=\"method\" \"describe\" \".string\" \"\" .member");
    rc |= require_contains(output, ".meta \"rxfnsb.word\"=\".inline\" \"I4;a;b\"");
    rc |= require_contains(output, "META-INLINE");
    rc |= require_contains(output, ".meta \"synthetic.main\"=\"b\" \".void\" main() \"\"");
    rc |= require_contains(output, ".meta \"synthetic.main.local\"=\"b\" \".string\" r0");
    rc |= require_contains(output, ".meta \"synthetic.main.answer\"=\"b\" \".string\" \"answer\"");
    rc |= require_contains(output, ".meta \"synthetic.helper\"=\"b\" \".void\" helper() \"\"");
    rc |= require_contains(output, ".meta \"synthetic.main.local\"\n");
    rc |= require_not_contains(output, "UNKNOWN");

    free(output);
    free(pool);
    free(code);

    return rc ? 1 : 0;
}
