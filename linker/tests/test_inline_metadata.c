/* cREXX License (MIT). Independent selected-startup preservation controls. */
#include "rxlink_inline_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int prepare(module_file *module) {
    proc_constant *a, *b;
    string_constant *sa, *sb;
    meta_initializer_constant *ia, *ib;
    init_module(module);
    module->constant = (unsigned char *)calloc(1024u, 1u);
    if (!module->constant || sizeof(proc_constant) + 8u > 128u ||
        sizeof(string_constant) + 8u > 128u || sizeof(meta_initializer_constant) > 128u) return 0;
    module->header.constant_size = 768u; module->header.proc_head = 0; module->header.meta_head = 512;
    a = (proc_constant *)module->constant; b = (proc_constant *)(module->constant + 128u);
    a->base.type = b->base.type = PROC_CONST; a->base.size_in_pool = b->base.size_in_pool = 128u;
    a->next = 128; b->next = -1; a->start = 0u; b->start = 1u;
    strcpy(a->name, "first"); strcpy(b->name, "second");
    sa = (string_constant *)(module->constant + 256u); sb = (string_constant *)(module->constant + 384u);
    sa->base.type = sb->base.type = STRING_CONST; sa->base.size_in_pool = sb->base.size_in_pool = 128u;
    sa->string_len = 5u; sb->string_len = 6u; strcpy(sa->string, "first"); strcpy(sb->string, "second");
    ia = (meta_initializer_constant *)(module->constant + 512u); ib = (meta_initializer_constant *)(module->constant + 640u);
    ia->base.base.type = ib->base.base.type = META_INITIALIZER;
    ia->base.base.size_in_pool = ib->base.base.size_in_pool = 128u;
    ia->base.prev = -1; ia->base.next = 640; ib->base.prev = 512; ib->base.next = -1;
    ia->symbol = 256u; ia->function = 0u; ib->symbol = 384u; ib->function = 128u;
    return 1;
}

static void add_static(module_file *module) {
    meta_entry *tail = (meta_entry *)(module->constant + 640u);
    meta_inline_frame_constant *frame = (meta_inline_frame_constant *)(module->constant + 768u);
    meta_inline_map_constant *map = (meta_inline_map_constant *)(module->constant + 896u);
    module->header.constant_size = 1024u; tail->next = 768;
    frame->base.base.type = META_INLINE_FRAME; frame->base.base.size_in_pool = 128u;
    frame->base.prev = 640; frame->base.next = 896; frame->frame_id = 7u;
    frame->module_name = 256u; frame->procedure_name = 384u;
    map->base.base.type = META_INLINE_MAP; map->base.base.size_in_pool = 128u;
    map->base.prev = 768; map->base.next = -1; map->start = 0u; map->end = 1u; map->frame_id = 7u;
}

int main(void) {
    module_file original, candidate;
    meta_initializer_constant *first, *second;
    int failures = 0;
    if (!prepare(&original) || !prepare(&candidate)) return 1;
    if (!rxlink_inline_metadata_equal(&original, &candidate)) ++failures;
    candidate.header.meta_head = -1;
    if (rxlink_inline_metadata_equal(&original, &candidate)) ++failures;
    candidate.header.meta_head = 512;
    first = (meta_initializer_constant *)(candidate.constant + 512u);
    second = (meta_initializer_constant *)(candidate.constant + 640u);
    first->function = 128u;
    if (rxlink_inline_metadata_equal(&original, &candidate)) ++failures;
    first->function = 0u;
    candidate.header.meta_head = 640; second->base.next = 512; first->base.next = -1;
    if (rxlink_inline_metadata_equal(&original, &candidate)) ++failures;
    candidate.header.meta_head = 512; first->base.next = 640; second->base.next = -1;
    add_static(&candidate);
    if (!rxlink_inline_metadata_equal(&original,&candidate)) ++failures;
    add_static(&original);
    if (!rxlink_inline_metadata_equal(&original,&candidate)) ++failures;
    ((meta_inline_frame_constant *)(candidate.constant + 768u))->procedure_name = 256u;
    if (rxlink_inline_metadata_equal(&original,&candidate)) ++failures;
    ((meta_inline_frame_constant *)(candidate.constant + 768u))->procedure_name = 384u;
    ((meta_inline_map_constant *)(candidate.constant + 896u))->start = 1u;
    if (rxlink_inline_metadata_equal(&original,&candidate)) ++failures;
    free(original.constant); free(candidate.constant);
    printf("RXLINK_INLINE_METADATA|failures=%d\n", failures);
    return failures != 0;
}
