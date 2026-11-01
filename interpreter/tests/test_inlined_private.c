/* Same immutable image, independent worker overlays and frame-owned cleanup. */
#include "rxvm.h"
#include "rxvmintp.h"
#include "rxinline_recipe.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(x,m) do { if (!(x)) { fprintf(stderr,"FAIL: %s\n",m); failures++; } } while (0)

static module *named_module(rxvm_context *context, const char *name) {
    size_t i;
    for (i = 0u; i < context->num_modules; i++)
        if (context->modules[i]->file && !strcmp(context->modules[i]->file->name,name))
            return context->modules[i];
    return NULL;
}

/* Inject a validly decoded but unresolved sealed storage request after image
 * loading. This exercises the separate pointer-resolution boundary, including
 * a partially populated prefix and context-owned cleanup/retry. */
static void check_resolution_failure(const char *path) {
    rxvm_context *context = rxvm_create();
    module *root;
    meta_inlined_private_constant *last = NULL;
    rxvm_inlined_private_binding *retained = NULL;
    size_t i, populated = 0u;
    uint32_t original;
    int offset;
    CHECK(context != NULL,"create failure context");
    if (!context) return;
    CHECK(rxvm_load_file(context,(char *)path) != NULL,"load sealed image for resolution corruption");
    root = named_module(context,"private-root.rxas");
    CHECK(root != NULL,"locate physical root by exact name");
    if (!root) { rxvm_destroy(context); return; }
    offset = root->file->header.meta_head;
    while (offset != -1) {
        meta_entry *entry = (meta_entry *)(root->segment.const_pool + offset);
        if (entry->base.type == META_INLINED_PRIVATE) last = (meta_inlined_private_constant *)entry;
        offset = entry->next;
    }
    CHECK(last != NULL,"find final sealed alias");
    if (!last) { rxvm_destroy(context); return; }
    original = last->global_index;
    last->global_index = UINT32_MAX;
    CHECK(rxvm_link(context) != 0 && context->link_dirty,"bad sealed global rejected before execution");
    for (i = 0u; i < root->procedure_count; i++) {
        proc_runtime *procedure = &root->procedures[i];
        if (procedure->inlined_private_owner == procedure && procedure->inlined_private_count) {
            if (!retained) retained = procedure->inlined_private;
            populated++;
        }
    }
    CHECK(populated > 0u,"failure follows a context-owned partial binding prefix");
    last->global_index = original;
    CHECK(rxvm_link(context) == 0,"repaired request retries existing context tables");
    populated = 0u;
    for (i = 0u; i < root->procedure_count; i++)
        if (root->procedures[i].inlined_private == retained) populated++;
    CHECK(populated == 1u,"retry preserves rather than replaces the existing table allocation");
    CHECK(rxvm_run(context,0,NULL) == 0,"repaired resolution executes independent oracle");
    rxvm_destroy(context);

    context = rxvm_create();
    CHECK(context != NULL,"create partial-failure destruction context");
    if (!context) return;
    CHECK(rxvm_load_file(context,(char *)path) != NULL,"reload corrupt-resolution control");
    root = named_module(context,"private-root.rxas");
    if (root) {
        string_constant *selector;
        offset = root->file->header.meta_head;
        last = NULL;
        while (offset != -1) {
            meta_entry *entry = (meta_entry *)(root->segment.const_pool + offset);
            if (entry->base.type == META_INLINED_PRIVATE) last = (meta_inlined_private_constant *)entry;
            offset = entry->next;
        }
        if (last) {
            selector = (string_constant *)(root->segment.const_pool + last->provider);
            CHECK(rxinl_module_ref_seal(selector->string,selector->string_len,UINT32_MAX - 1u),"inject missing sealed provider ordinal");
            CHECK(rxvm_link(context) != 0 && context->link_dirty,"missing sealed provider rejected before preparation");
        }
    }
    rxvm_destroy(context);
}

static rxvm_context *load_image(const char *path) {
    rxvm_context *context = rxvm_create();
    CHECK(context != NULL,"create context");
    if (!context) return NULL;
    CHECK(rxvm_load_file(context,(char *)path) != NULL,"load private binding image");
    CHECK(rxvm_link(context) == 0,"resolve borrowed slots");
    CHECK(rxvm_prepare(context) == 0,"prepare image");
    return context;
}

int main(int argc, char **argv) {
    rxvm_context *left, *right;
    module *lp, *rp, *root;
    size_t i, declared = 0u;
    if (argc != 2) return 2;
    left = load_image(argv[1]); right = load_image(argv[1]);
    if (!left || !right) { rxvm_destroy(left); rxvm_destroy(right); return 1; }
    lp = named_module(left,"private-provider.rxas"); rp = named_module(right,"private-provider.rxas");
    root = named_module(left,"private-root.rxas");
    CHECK(lp && rp,"private provider retained without public exposure");
    if (lp && rp) {
        CHECK(lp->globals[0] != rp->globals[0],"workers own distinct mutable globals");
        CHECK(lp->initializer_state == RXVM_INIT_UNINITIALIZED && rp->initializer_state == RXVM_INIT_UNINITIALIZED,
              "alias linking/preparation does not run provider initializer");
        CHECK(lp->globals[1]->int_value == 0 && rp->globals[1]->int_value == 0,"initializer marker untouched before execution");
        CHECK(root != NULL,"locate root independently of module ordering");
        for (i = 0u; root && i < root->procedure_count; i++) {
            proc_runtime *procedure = &root->procedures[i];
            if (procedure->inlined_private_owner == procedure && procedure->inlined_private_count) {
                CHECK(procedure->inlined_private_count == (size_t)(procedure->locals - procedure->owned_locals),"complete borrowed suffix");
                declared++;
            }
        }
        CHECK(declared == 6u,"all six procedure binding tables materialized");
        CHECK(rxvm_run(left,0,NULL) == 0,"left independent return/reuse/binding/reference/unwind oracle");
        CHECK(lp->globals[0]->int_value == 8 && lp->globals[1]->int_value == 321,"left provider state survives returns");
        CHECK(rp->globals[0]->int_value == 0 && rp->initializer_state == RXVM_INIT_UNINITIALIZED,"left execution leaves right worker uninitialized");
        left->link_dirty = 1;
        CHECK(rxvm_link(left) == 0,"repeat linking refreshes existing tables");
        CHECK(rxvm_call(left,"private.peek",0,NULL) == 8,"borrowed storage survives relink");
        CHECK(rxvm_run(right,0,NULL) == 0,"right independent oracle");
        CHECK(rp->globals[0]->int_value == 8,"right storage updated independently");
    }
    rxvm_destroy(left);
    if (rp) CHECK(rxvm_call(right,"private.peek",0,NULL) == 8,"destroying one worker preserves another");
    rxvm_destroy(right);
    check_resolution_failure(argv[1]);
    printf("INLINED_PRIVATE_VM|failures=%d\n",failures);
    return failures ? 1 : 0;
}
