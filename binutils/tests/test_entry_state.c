/* Typed program-default entry contract: wire format, owner and feature gates. */
#include "rxbin.h"
#include "rxdefs.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"entry state line %d: %s (%s)\n",__LINE__,#x,rxbin_last_error() ? rxbin_last_error() : ""); return 1; } } while (0)

typedef struct fixture {
    module_file module;
    unsigned char pool[1024];
    bin_code code[2];
    size_t used, procedure;
    meta_entry_state_constant *state;
} fixture;

static size_t append(fixture *f, enum const_pool_type type, size_t bytes) {
    size_t offset = f->used;
    chameleon_constant *entry;
    bytes = (bytes + 7u) & ~(size_t)7u;
    if (bytes > sizeof(f->pool) - offset) abort();
    memset(f->pool + offset,0,bytes); f->used += bytes;
    entry = (chameleon_constant *)(f->pool + offset);
    entry->type = type; entry->size_in_pool = bytes;
    return offset;
}

static void prepare(fixture *f) {
    proc_constant *procedure;
    size_t state;
    memset(f,0,sizeof(*f)); init_module(&f->module);
    f->module.name = "entry-codec"; f->module.description = "";
    f->module.header.expose_head = -1;
    f->procedure = append(f,PROC_CONST,offsetof(proc_constant,name) + 5u);
    procedure = (proc_constant *)(f->pool + f->procedure);
    procedure->next = -1; procedure->exposed = SIZE_MAX; procedure->start = 0u; procedure->locals = 0;
    memcpy(procedure->name,"main",5u);
    state = append(f,META_ENTRY_STATE,sizeof(*f->state));
    f->state = (meta_entry_state_constant *)(f->pool + state);
    f->state->base.prev = f->state->base.next = -1;
    f->state->procedure = f->procedure;
    f->state->version = RXBIN_ENTRY_STATE_VERSION;
    f->state->kind = RXBIN_ENTRY_PROGRAM_DEFAULT;
    f->module.header.meta_head = (int)state;
    f->module.header.proc_head = (int)f->procedure;
    f->module.header.instruction_size = 2u;
    f->module.header.constant_size = f->used;
    f->module.constant = f->pool; f->module.instructions = f->code;
    f->code[0].instruction.opcode = OP_RET_INT; f->code[0].instruction.no_ops = 1u;
    f->code[1].iconst = 0;
}

static int rejected(fixture *f) {
    FILE *stream = tmpfile();
    int result;
    if (!stream) return 0;
    f->module.header.constant_size = f->used;
    result = write_module(&f->module,stream); fclose(stream);
    return result != 0;
}

static void add_initializer(fixture *f, int before_entry) {
    static const char name[] = "entry.boot";
    size_t symbol = append(f,STRING_CONST,offsetof(string_constant,string) + sizeof(name));
    string_constant *text = (string_constant *)(f->pool + symbol);
    size_t offset;
    meta_initializer_constant *initializer;
    text->string_len = sizeof(name) - 1u;
#ifndef NUTF8
    text->string_chars = text->string_len;
#endif
    memcpy(text->string,name,sizeof(name));
    offset = append(f,META_INITIALIZER,sizeof(*initializer));
    initializer = (meta_initializer_constant *)(f->pool + offset);
    initializer->function = f->procedure; initializer->symbol = symbol;
    if (before_entry) {
        initializer->base.prev = -1; initializer->base.next = f->module.header.meta_head;
        f->state->base.prev = (int)offset; f->module.header.meta_head = (int)offset;
    } else {
        initializer->base.prev = f->module.header.meta_head; initializer->base.next = -1;
        f->state->base.next = (int)offset;
    }
}

int main(int argc, char **argv) {
    fixture f;
    FILE *stream;
    module_file *loaded = NULL;
    unsigned char header[16];
    meta_entry_state_constant *second;
    size_t offset;
    prepare(&f);
    if (argc == 3 && !strcmp(argv[1],"--static-child")) {
        /* A constructed image bypasses assembler admission so the independent
         * selected-image linker/loader boundary can reject its child target. */
        f.code[0].instruction.opcode = OP_CALL_FUNC;
        f.code[1].index = f.procedure;
        stream = fopen(argv[2],"wb"); CHECK(stream);
        CHECK(write_module(&f.module,stream) == 0); CHECK(fclose(stream) == 0);
        return 0;
    }
    if (argc != 1) return 2;
    stream = tmpfile(); CHECK(stream);
    CHECK(write_module(&f.module,stream) == 0);
    rewind(stream); CHECK(fread(header,1,sizeof(header),stream) == sizeof(header));
    CHECK((header[13] & 8u) != 0u); /* bit 11 */
    rewind(stream); CHECK(read_module(&loaded,stream) == 0 && loaded);
    second = (meta_entry_state_constant *)((unsigned char *)loaded->constant + loaded->header.meta_head);
    CHECK(second->base.base.type == META_ENTRY_STATE && second->version == 1u && second->kind == 1u);
    CHECK(((const proc_constant *)((const unsigned char *)loaded->constant + second->procedure))->start == 0u);
    free_module(loaded); loaded = NULL;
    header[13] &= (unsigned char)~8u;
    rewind(stream); CHECK(fwrite(header,1,sizeof(header),stream) == sizeof(header)); fflush(stream);
    rewind(stream); CHECK(read_module(&loaded,stream) == -1 && !loaded);
    CHECK(strstr(rxbin_last_error(),"feature flag")); fclose(stream);
    prepare(&f); f.state->version = 0u; CHECK(rejected(&f));
    prepare(&f); f.state->version = 2u; CHECK(rejected(&f));
    prepare(&f); f.state->kind = 0u; CHECK(rejected(&f));
    prepare(&f); f.state->kind = 2u; CHECK(rejected(&f));
    prepare(&f); f.state->base.address = 1u; CHECK(rejected(&f));
    prepare(&f); ((proc_constant *)(f.pool + f.procedure))->locals = -1; CHECK(rejected(&f));
    prepare(&f); ((proc_constant *)(f.pool + f.procedure))->start = SIZE_MAX; CHECK(rejected(&f));
    prepare(&f); ((proc_constant *)(f.pool + f.procedure))->start = f.state->base.address = 1u; CHECK(rejected(&f));
    prepare(&f); memcpy(((proc_constant *)(f.pool + f.procedure))->name,"help",5u); CHECK(rejected(&f));
    prepare(&f); f.state->procedure = (size_t)f.module.header.meta_head; CHECK(rejected(&f));
    prepare(&f); add_initializer(&f,0); CHECK(rejected(&f));
    CHECK(strstr(rxbin_last_error(),"main cannot also be a module initializer"));
    prepare(&f); add_initializer(&f,1); CHECK(rejected(&f));
    CHECK(strstr(rxbin_last_error(),"main cannot also be a module initializer"));
    prepare(&f);
    offset = append(&f,META_ENTRY_STATE,sizeof(*second));
    second = (meta_entry_state_constant *)(f.pool + offset); *second = *f.state;
    second->base.prev = f.module.header.meta_head; second->base.next = -1; f.state->base.next = (int)offset;
    CHECK(rejected(&f));
    puts("PROGRAM_ENTRY_RECORDS|failures=0"); return 0;
}
