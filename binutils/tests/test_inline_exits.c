/* Executable cold-exit records: independent model, roundtrip and invalids. */
#include "rxbin.h"
#include "rxdefs.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"inline exits line %d: %s (%s)\n",__LINE__,#x,rxbin_last_error()); return 1; } } while (0)

typedef struct fixture {
    module_file module;
    unsigned char pool[8192];
    bin_code code[14];
    size_t used;
    int tail;
    meta_inline_region_constant *outer, *inner;
    meta_inline_range_constant *outer_range, *inner_range, *fragment;
    meta_inline_exit_constant *saved, *literal, *expiry, *binding;
} fixture;

static size_t append(fixture *f, enum const_pool_type type, size_t bytes) {
    size_t at = f->used;
    chameleon_constant *base;
    bytes = (bytes + 7u) & ~(size_t)7u;
    if (bytes > sizeof(f->pool) - at) abort();
    memset(f->pool + at,0,bytes); f->used += bytes;
    base = (chameleon_constant *)(f->pool + at); base->type = type; base->size_in_pool = bytes;
    return at;
}
static meta_entry *metadata(fixture *f, enum const_pool_type type, size_t bytes, size_t address) {
    size_t at = append(f,type,bytes);
    meta_entry *meta = (meta_entry *)(f->pool + at);
    meta->prev = f->tail; meta->next = -1; meta->address = address;
    if (f->tail == -1) f->module.header.meta_head = (int)at;
    else ((meta_entry *)(f->pool + f->tail))->next = (int)at;
    f->tail = (int)at; return meta;
}
static void prepare(fixture *f) {
    size_t p, i;
    proc_constant *proc;
    memset(f,0,sizeof(*f)); init_module(&f->module); f->tail = -1;
    f->module.name = "inline-exits"; f->module.description = "";
    f->module.header.globals = 1;
    f->module.header.proc_head = f->module.header.meta_head = f->module.header.expose_head = -1;
    p = append(f,PROC_CONST,offsetof(proc_constant,name) + 5u);
    proc = (proc_constant *)(f->pool + p); proc->next = -1; proc->locals = 4;
    proc->start = 0u; proc->exposed = SIZE_MAX; memcpy(proc->name,"main",5u);
    f->module.header.proc_head = (int)p;
    f->outer = (meta_inline_region_constant *)metadata(f,META_INLINE_REGION,sizeof(*f->outer),0u);
    f->outer->procedure = p; f->outer->version = 1u; f->outer->region_id = 1u;
    f->inner = (meta_inline_region_constant *)metadata(f,META_INLINE_REGION,sizeof(*f->inner),2u);
    f->inner->procedure = p; f->inner->version = 1u; f->inner->region_id = 2u; f->inner->parent_id = 1u;
    f->outer_range = (meta_inline_range_constant *)metadata(f,META_INLINE_RANGE,sizeof(*f->outer_range),0u);
    f->outer_range->region_id = 1u; f->outer_range->start = 0u; f->outer_range->end = 12u;
    f->inner_range = (meta_inline_range_constant *)metadata(f,META_INLINE_RANGE,sizeof(*f->inner_range),0u);
    f->inner_range->region_id = 2u; f->inner_range->start = 2u; f->inner_range->end = 5u;
    f->fragment = (meta_inline_range_constant *)metadata(f,META_INLINE_RANGE,sizeof(*f->fragment),0u);
    f->fragment->region_id = 2u; f->fragment->start = 7u; f->fragment->end = 9u;
    f->saved = (meta_inline_exit_constant *)metadata(f,META_INLINE_EXIT,sizeof(*f->saved),0u);
    f->saved->region_id = 2u; f->saved->start = 3u; f->saved->end = 5u;
    f->saved->kind = RXBIN_INLINE_ACTION_NUMERIC; f->saved->field = RXOP_CONTEXT_DIGITS;
    f->saved->source = RXBIN_INLINE_SOURCE_OWNED_BASE; f->saved->index1 = 3u;
    f->literal = (meta_inline_exit_constant *)metadata(f,META_INLINE_EXIT,sizeof(*f->literal),0u);
    f->literal->region_id = 2u; f->literal->start = 7u; f->literal->end = 9u;
    f->literal->kind = RXBIN_INLINE_ACTION_NUMERIC; f->literal->field = RXOP_CONTEXT_DIGITS;
    f->literal->source = RXBIN_INLINE_SOURCE_LITERAL; f->literal->literal = 18;
    f->expiry = (meta_inline_exit_constant *)metadata(f,META_INLINE_EXIT,sizeof(*f->expiry),0u);
    f->expiry->region_id = 2u; f->expiry->start = 3u; f->expiry->end = 5u;
    f->expiry->order = 1u; f->expiry->kind = RXBIN_INLINE_ACTION_ENDLIFE; f->expiry->index1 = 1u;
    f->binding = (meta_inline_exit_constant *)metadata(f,META_INLINE_EXIT,sizeof(*f->binding),0u);
    f->binding->region_id = 2u; f->binding->start = 3u; f->binding->end = 5u;
    f->binding->order = 2u; f->binding->kind = RXBIN_INLINE_ACTION_LINK;
    f->binding->index1 = 2u; f->binding->bank2 = RXBIN_INLINE_BANK_A; f->binding->index2 = 1u;
    for (i = 0u; i < 12u; i++) f->code[i].instruction.opcode = OP_CNOP;
    f->code[12].instruction.opcode = OP_RET_INT; f->code[12].instruction.no_ops = 1; f->code[13].iconst = 0;
    f->module.instructions = f->code; f->module.header.instruction_size = 14u;
    f->module.constant = f->pool; f->module.header.constant_size = f->used;
}
static int rejected(fixture *f) {
    FILE *out = tmpfile(); int result;
    if (!out) return 0;
    f->module.header.constant_size = f->used;
    result = write_module(&f->module,out); fclose(out);
    return result != 0 && strstr(rxbin_last_error(),"inline-exit") != NULL;
}

int main(void) {
    fixture f;
    FILE *out;
    module_file *loaded = NULL;
    unsigned char header[16];
    int offset, expected_offset, regions = 0, ranges = 0, actions = 0;
    prepare(&f); CHECK(rxbin_validate_inline_exits(&f.module));
    out = tmpfile(); CHECK(out); CHECK(write_module(&f.module,out) == 0);
    rewind(out); CHECK(fread(header,1,sizeof(header),out) == sizeof(header));
    CHECK((header[13] & 16u) != 0u); /* feature bit 12 */
    rewind(out); CHECK(read_module(&loaded,out) == 0 && loaded);
    offset = loaded->header.meta_head; expected_offset = f.module.header.meta_head;
    while (offset != -1) {
        const meta_entry *meta = (const meta_entry *)(loaded->constant + offset);
        const meta_entry *expected = (const meta_entry *)(f.pool + expected_offset);
        CHECK(meta->base.type == expected->base.type && meta->address == expected->address);
        if (meta->base.type == META_INLINE_REGION) {
            const meta_inline_region_constant *x = (const meta_inline_region_constant *)meta;
            const meta_inline_region_constant *y = (const meta_inline_region_constant *)expected;
            const proc_constant *owner = (const proc_constant *)(loaded->constant + x->procedure);
            CHECK(x->version == y->version && x->region_id == y->region_id && x->parent_id == y->parent_id);
            CHECK(owner->base.type == PROC_CONST && !strcmp(owner->name,"main")); regions++;
        }
        if (meta->base.type == META_INLINE_RANGE) {
            const meta_inline_range_constant *x = (const meta_inline_range_constant *)meta;
            const meta_inline_range_constant *y = (const meta_inline_range_constant *)expected;
            CHECK(x->start == y->start && x->end == y->end && x->region_id == y->region_id); ranges++;
        }
        if (meta->base.type == META_INLINE_EXIT) {
            const meta_inline_exit_constant *x = (const meta_inline_exit_constant *)meta;
            const meta_inline_exit_constant *y = (const meta_inline_exit_constant *)expected;
            CHECK(x->start == y->start && x->end == y->end && x->region_id == y->region_id && x->order == y->order);
            CHECK(x->kind == y->kind && x->field == y->field && x->source == y->source && x->literal == y->literal);
            CHECK(x->bank1 == y->bank1 && x->index1 == y->index1 && x->bank2 == y->bank2 && x->index2 == y->index2); actions++;
        }
        offset = meta->next; expected_offset = expected->next;
    }
    CHECK(expected_offset == -1);
    CHECK(regions == 2 && ranges == 3 && actions == 4);
    CHECK(rxbin_validate_inline_exits(loaded)); free_module(loaded); loaded = NULL;
    header[13] &= (unsigned char)~16u; rewind(out);
    CHECK(fwrite(header,1,sizeof(header),out) == sizeof(header)); fflush(out); rewind(out);
    CHECK(read_module(&loaded,out) == -1 && !loaded); CHECK(strstr(rxbin_last_error(),"feature flag")); fclose(out);
    prepare(&f); f.inner->parent_id = 99u; CHECK(rejected(&f));
    prepare(&f); f.outer->parent_id = 2u; CHECK(rejected(&f));
    prepare(&f); f.inner->region_id = 1u; CHECK(rejected(&f));
    prepare(&f); f.inner->version = 2u; CHECK(rejected(&f));
    prepare(&f); f.inner->procedure = SIZE_MAX; CHECK(rejected(&f));
    prepare(&f); f.inner->base.address = 6u; CHECK(rejected(&f));
    prepare(&f); f.inner_range->region_id = 99u; CHECK(rejected(&f));
    prepare(&f); f.inner_range->end = 14u; CHECK(rejected(&f));
    prepare(&f); f.fragment->start = 4u; CHECK(rejected(&f));
    prepare(&f); f.inner_range->end = 13u; CHECK(rejected(&f)); /* operand cell */
    prepare(&f); f.inner->parent_id = 0u; CHECK(rejected(&f)); /* overlapping siblings */
    prepare(&f); f.saved->start = 5u; f.saved->end = 8u; CHECK(rejected(&f)); /* crosses a gap */
    prepare(&f); f.saved->region_id = 99u; CHECK(rejected(&f));
    prepare(&f); f.saved->index1 = 4u; CHECK(rejected(&f));
    prepare(&f); f.saved->bank1 = RXBIN_INLINE_BANK_G; CHECK(rejected(&f));
    prepare(&f); f.saved->literal = 1; CHECK(rejected(&f)); /* unused field */
    prepare(&f); f.literal->literal = 0; CHECK(rejected(&f));
    prepare(&f); f.literal->field = RXOP_CONTEXT_DIGITS | RXOP_CONTEXT_FUZZ; CHECK(rejected(&f));
    prepare(&f); f.expiry->order = 0u; CHECK(rejected(&f)); /* ambiguous action order */
    prepare(&f); f.expiry->index1 = 4u; CHECK(rejected(&f));
    prepare(&f); f.binding->bank2 = RXBIN_INLINE_BANK_G; f.binding->index2 = 1u; CHECK(rejected(&f));
    prepare(&f); f.binding->kind = 999u; CHECK(rejected(&f));
    puts("INLINE_EXIT_RECORDS|failures=0"); return 0;
}
