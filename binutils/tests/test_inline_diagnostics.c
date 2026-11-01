/* Typed static inline diagnostics: independent valid model and corrupt cases. */
#include "rxbin.h"
#include "rxdefs.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"inline diagnostics line %d: %s\n",__LINE__,#x); return 1; } } while (0)

typedef struct fixture {
    module_file module;
    unsigned char pool[4096];
    bin_code code[2];
    size_t used;
    int tail;
    meta_inline_frame_constant *frame;
    meta_inline_map_constant *map;
    meta_source_step_constant *step;
} fixture;

static size_t entry(fixture *f, enum const_pool_type type, size_t bytes) {
    size_t at = f->used;
    chameleon_constant *base;
    bytes = (bytes + 7u) & ~(size_t)7u;
    if (bytes > sizeof(f->pool) - at) abort();
    memset(f->pool + at,0,bytes); f->used += bytes;
    base = (chameleon_constant *)(f->pool + at); base->type = type; base->size_in_pool = bytes;
    return at;
}
static size_t text(fixture *f, const char *value) {
    size_t length = strlen(value), at = entry(f,STRING_CONST,offsetof(string_constant,string) + length + 1u);
    string_constant *s = (string_constant *)(f->pool + at);
    s->string_len = length;
#ifndef NUTF8
    s->string_chars = length;
#endif
    memcpy(s->string,value,length + 1u); return at;
}
static meta_entry *metadata(fixture *f, enum const_pool_type type, size_t bytes) {
    size_t at = entry(f,type,bytes); meta_entry *meta = (meta_entry *)(f->pool + at);
    meta->prev = f->tail; meta->next = -1; meta->address = 0u;
    if (f->tail == -1) f->module.header.meta_head = (int)at;
    else ((meta_entry *)(f->pool + (size_t)f->tail))->next = (int)at;
    f->tail = (int)at; return meta;
}
static void prepare(fixture *f) {
    size_t p, mod, name, file, source;
    proc_constant *proc;
    memset(f,0,sizeof(*f)); init_module(&f->module); f->tail = -1;
    f->module.name = "static-inline"; f->module.description = "";
    f->module.header.proc_head = f->module.header.meta_head = f->module.header.expose_head = -1;
    mod = text(f,"provider.rxas"); name = text(f,"helper"); file = text(f,"caller.rexx"); source = text(f,"call helper");
    p = entry(f,PROC_CONST,offsetof(proc_constant,name) + 5u); proc = (proc_constant *)(f->pool + p);
    proc->next = -1; proc->start = 0u; proc->exposed = SIZE_MAX; memcpy(proc->name,"main",5u);
    f->module.header.proc_head = (int)p;
    f->step = (meta_source_step_constant *)metadata(f,META_SOURCE_STEP,sizeof(*f->step));
    f->step->file = file; f->step->source_line = source; f->step->step_id = 42u; f->step->clause_id = 42u; f->step->line = 7u;
    f->frame = (meta_inline_frame_constant *)metadata(f,META_INLINE_FRAME,sizeof(*f->frame));
    f->frame->frame_id = 7u; f->frame->module_name = mod; f->frame->procedure_name = name; f->frame->callsite_step_id = 42u;
    f->map = (meta_inline_map_constant *)metadata(f,META_INLINE_MAP,sizeof(*f->map));
    f->map->start = 0u; f->map->end = 2u; f->map->frame_id = 7u;
    f->code[0].instruction.opcode = OP_RET_INT; f->code[0].instruction.no_ops = 1; f->code[1].iconst = 0;
    f->module.instructions = f->code; f->module.header.instruction_size = 2u;
    f->module.constant = f->pool; f->module.header.constant_size = f->used;
}
static int rejected(fixture *f) {
    FILE *out = tmpfile(); int result;
    if (!out) return 0;
    f->module.header.constant_size = f->used;
    result = write_module(&f->module,out); fclose(out);
    return result != 0 && rxbin_last_error() && strstr(rxbin_last_error(),"inline diagnostic");
}

int main(void) {
    fixture f; FILE *out; module_file *loaded = NULL; unsigned char header[16];
    meta_inline_frame_constant *frame2; meta_inline_map_constant *map2; meta_source_step_constant *step2;
    prepare(&f); out = tmpfile(); CHECK(out); CHECK(write_module(&f.module,out) == 0);
    rewind(out); CHECK(fread(header,1,sizeof(header),out) == sizeof(header));
    CHECK((header[13] & 2u) != 0u); /* bit 9 */
    rewind(out); CHECK(read_module(&loaded,out) == 0 && loaded); free_module(loaded); loaded = NULL;
    CHECK(read_module(&loaded,out) == 1 && !loaded);
    header[13] &= (unsigned char)~2u; rewind(out); CHECK(fwrite(header,1,sizeof(header),out) == sizeof(header)); fflush(out); rewind(out);
    CHECK(read_module(&loaded,out) == -1 && !loaded); CHECK(strstr(rxbin_last_error(),"feature flag")); fclose(out);
    prepare(&f); f.frame->parent_id = 7u; CHECK(rejected(&f));
    prepare(&f); f.frame->parent_id = 99u; CHECK(rejected(&f));
    prepare(&f); f.map->frame_id = 99u; CHECK(rejected(&f));
    prepare(&f); f.map->end = 1u; CHECK(rejected(&f)); /* operand cell */
    prepare(&f); map2 = (meta_inline_map_constant *)metadata(&f,META_INLINE_MAP,sizeof(*map2)); *map2 = *f.map; map2->base.prev = (int)((unsigned char *)f.map - f.pool); map2->base.next = -1; CHECK(rejected(&f));
    prepare(&f); frame2 = (meta_inline_frame_constant *)metadata(&f,META_INLINE_FRAME,sizeof(*frame2)); *frame2 = *f.frame; frame2->base.prev = (int)((unsigned char *)f.map - f.pool); frame2->base.next = -1; CHECK(rejected(&f));
    prepare(&f); step2 = (meta_source_step_constant *)metadata(&f,META_SOURCE_STEP,sizeof(*step2)); *step2 = *f.step; step2->base.prev = (int)((unsigned char *)f.map - f.pool); step2->base.next = -1; step2->line++; CHECK(rejected(&f));
    prepare(&f); step2 = (meta_source_step_constant *)metadata(&f,META_SOURCE_STEP,sizeof(*step2)); *step2 = *f.step; step2->base.prev = (int)((unsigned char *)f.map - f.pool); step2->base.next = -1; f.module.header.constant_size = f.used;
    out = tmpfile(); CHECK(out); CHECK(write_module(&f.module,out) == 0); fclose(out);
    puts("INLINE_DIAGNOSTIC_RECORDS|failures=0"); return 0;
}
