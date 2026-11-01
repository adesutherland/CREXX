/* Executable borrowed-local ownership metadata, independent codec controls. */
#include "rxbin.h"
#include "rxdefs.h"
#include "rxinline_recipe.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"inlined private line %d: %s (%s)\n",__LINE__,#x,rxbin_last_error() ? rxbin_last_error() : ""); return 1; } } while (0)

typedef struct fixture {
    module_file module;
    unsigned char pool[2048];
    bin_code code[1];
    size_t used, procedure, provider;
    meta_inlined_private_constant *first, *second;
} fixture;

static size_t append(fixture *f, enum const_pool_type type, size_t bytes) {
    size_t offset = f->used;
    chameleon_constant *entry;
    bytes = (bytes + 7u) & ~(size_t)7u;
    if (bytes > sizeof(f->pool) - offset) abort();
    memset(f->pool + offset,0,bytes);
    f->used += bytes;
    entry = (chameleon_constant *)(f->pool + offset);
    entry->type = type;
    entry->size_in_pool = bytes;
    return offset;
}

static void prepare(fixture *f) {
    unsigned char *bytes = NULL;
    size_t length = 0u, first, second;
    proc_constant *procedure;
    string_constant *provider;
    memset(f,0,sizeof(*f));
    init_module(&f->module);
    f->module.name = "private-codec";
    f->module.description = "";
    f->module.header.globals = 1;
    f->module.header.expose_head = -1;
    f->procedure = append(f,PROC_CONST,offsetof(proc_constant,name) + 5u);
    procedure = (proc_constant *)(f->pool + f->procedure);
    procedure->next = -1;
    procedure->locals = 3;
    procedure->start = 0u;
    procedure->exposed = SIZE_MAX;
    memcpy(procedure->name,"main",5u);
    if (!rxinl_module_ref_from_text("self",&bytes,&length)) abort();
    f->provider = append(f,BINARY_CONST,offsetof(string_constant,string) + length);
    provider = (string_constant *)(f->pool + f->provider);
    provider->string_len = length;
    memcpy(provider->string,bytes,length);
    free(bytes);
    first = append(f,META_INLINED_PRIVATE,sizeof(*f->first));
    second = append(f,META_INLINED_PRIVATE,sizeof(*f->second));
    f->first = (meta_inlined_private_constant *)(f->pool + first);
    f->second = (meta_inlined_private_constant *)(f->pool + second);
    f->first->base.prev = -1; f->first->base.next = (int)second;
    f->second->base.prev = (int)first; f->second->base.next = -1;
    f->first->procedure = f->second->procedure = f->procedure;
    f->first->provider = f->second->provider = f->provider;
    f->first->owned_locals = f->second->owned_locals = 1u;
    f->first->local_index = 1u; f->second->local_index = 2u;
    f->code[0].instruction.opcode = OP_RET;
    f->code[0].instruction.no_ops = 0;
    f->module.header.proc_head = (int)f->procedure;
    f->module.header.meta_head = (int)first;
    f->module.header.instruction_size = 1u;
    f->module.header.constant_size = f->used;
    f->module.constant = f->pool;
    f->module.instructions = f->code;
}

static int rejected(fixture *f) {
    FILE *stream = tmpfile();
    int result;
    if (!stream) return 0;
    result = write_module(&f->module,stream);
    fclose(stream);
    return result != 0;
}

int main(void) {
    fixture f;
    FILE *stream;
    module_file *loaded = NULL;
    unsigned char header[16];
    const meta_inlined_private_constant *binding;
    prepare(&f);
    stream = tmpfile(); CHECK(stream);
    CHECK(write_module(&f.module,stream) == 0);
    rewind(stream); CHECK(fread(header,1,sizeof(header),stream) == sizeof(header));
    CHECK((header[13] & 4u) != 0u); /* bit 10 */
    rewind(stream); CHECK(read_module(&loaded,stream) == 0 && loaded);
    binding = (const meta_inlined_private_constant *)((const unsigned char *)loaded->constant + loaded->header.meta_head);
    CHECK(binding->base.base.type == META_INLINED_PRIVATE && binding->local_index == 1u && binding->owned_locals == 1u);
    CHECK(binding->global_index == 0u && binding->base.address == 0u);
    free_module(loaded); loaded = NULL;
    header[13] &= (unsigned char)~4u;
    rewind(stream); CHECK(fwrite(header,1,sizeof(header),stream) == sizeof(header)); fflush(stream);
    rewind(stream); CHECK(read_module(&loaded,stream) == -1 && !loaded);
    CHECK(strstr(rxbin_last_error(),"feature flag")); fclose(stream);
    prepare(&f); f.second->local_index = 1u; CHECK(rejected(&f));
    prepare(&f); f.first->base.next = -1; CHECK(rejected(&f));
    prepare(&f); f.second->owned_locals = 2u; CHECK(rejected(&f));
    prepare(&f); f.second->local_index = 3u; CHECK(rejected(&f));
    prepare(&f); f.first->owned_locals = 3u; CHECK(rejected(&f));
    prepare(&f); f.first->base.address = 1u; CHECK(rejected(&f));
    prepare(&f); f.first->provider = f.procedure; CHECK(rejected(&f));
    prepare(&f); f.first->procedure = f.provider; CHECK(rejected(&f));
    prepare(&f); ((proc_constant *)(f.pool + f.procedure))->start = SIZE_MAX; CHECK(rejected(&f));
    puts("INLINED_PRIVATE_RECORDS|failures=0");
    return 0;
}
