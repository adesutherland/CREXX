/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
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

#include <stdlib.h>
#include <string.h>
#include "rxvmref.h"

static uint64_t fallback_next_reference_id = 1;

static uint64_t rxvm_next_fallback_reference_id(void) {
    uint64_t id = fallback_next_reference_id++;
    if (fallback_next_reference_id == 0) fallback_next_reference_id = 1;
    return id;
}

static uint64_t rxvm_next_context_reference_id(rxvm_reference_context *context) {
    uint64_t id;

    if (!context) return rxvm_next_fallback_reference_id();

    if (context->next_reference_id == 0) context->next_reference_id = 1;
    id = context->next_reference_id++;
    if (context->next_reference_id == 0) context->next_reference_id = 1;
    return id;
}

static size_t rxvm_reference_bucket_index(uint64_t id) {
    return (size_t)(id % RXVM_REFERENCE_ROOT_BUCKETS);
}

static void rxvm_reference_context_add_active(rxvm_reference_context *context,
                                              rxvm_reference_cell *cell) {
    size_t bucket;

    if (!context || !cell) return;

    bucket = rxvm_reference_bucket_index(cell->id);
    cell->next_active = context->active_buckets[bucket];
    context->active_buckets[bucket] = cell;
    context->active_count++;
}

static void rxvm_reference_context_remove_active(rxvm_reference_cell *cell) {
    rxvm_reference_context *context;
    rxvm_reference_cell **cursor;
    size_t bucket;

    if (!cell || !cell->context) return;

    context = cell->context;
    bucket = rxvm_reference_bucket_index(cell->id);
    cursor = &context->active_buckets[bucket];
    while (*cursor) {
        if (*cursor == cell) {
            *cursor = cell->next_active;
            if (context->active_count) context->active_count--;
            break;
        }
        cursor = &(*cursor)->next_active;
    }
    cell->next_active = 0;
}

static void rxvm_reference_context_push_free(rxvm_reference_context *context,
                                             rxvm_reference_cell *cell) {
    if (!context || !cell ||
        context->free_count >= RXVM_REFERENCE_FREE_LIST_SOFT_LIMIT) {
        free(cell);
        return;
    }

    cell->state = RXVM_REF_INVALID;
    cell->target = 0;
    cell->owner_kind = RXVM_REF_OWNER_NONE;
    cell->owner = 0;
    cell->owner_generation = 0;
    cell->debug_name = 0;
    cell->next_free = context->free_list;
    context->free_list = cell;
    context->free_count++;
}

static rxvm_reference_cell *rxvm_reference_context_pop_free(rxvm_reference_context *context) {
    rxvm_reference_cell *cell;

    if (!context || !context->free_list) return 0;

    cell = context->free_list;
    context->free_list = cell->next_free;
    cell->next_free = 0;
    if (context->free_count) context->free_count--;
    return cell;
}

static void rxvm_reference_cell_setup(rxvm_reference_cell *cell,
                                      rxvm_reference_context *context,
                                      rxvm_ref_owner_kind owner_kind,
                                      value *target,
                                      void *owner,
                                      uint64_t owner_generation,
                                      const char *debug_name) {
    cell->id = rxvm_next_context_reference_id(context);
    cell->retain_count = 1;
    cell->state = target ? RXVM_REF_VALID : RXVM_REF_INVALID;
    cell->owner_kind = owner_kind;
    cell->target = target;
    cell->owner = owner;
    cell->owner_generation = owner_generation;
    cell->debug_name = debug_name;
    cell->context = context;
    cell->next_active = 0;
    cell->next_free = 0;
}

void rxvm_reference_context_init(rxvm_reference_context *context) {
    if (!context) return;
    memset(context, 0, sizeof(*context));
    context->next_reference_id = 1;
}

void rxvm_reference_context_free(rxvm_reference_context *context) {
    size_t i;
    rxvm_reference_cell *cell;

    if (!context) return;

    for (i = 0; i < RXVM_REFERENCE_ROOT_BUCKETS; i++) {
        cell = context->active_buckets[i];
        while (cell) {
            rxvm_reference_cell *next = cell->next_active;
            rxvm_reference_cell_invalidate(cell);
            free(cell);
            cell = next;
        }
        context->active_buckets[i] = 0;
    }

    cell = context->free_list;
    while (cell) {
        rxvm_reference_cell *next = cell->next_free;
        free(cell);
        cell = next;
    }

    context->free_list = 0;
    context->active_count = 0;
    context->free_count = 0;
    context->next_reference_id = 1;
}

rxvm_reference_cell *rxvm_reference_context_find(rxvm_reference_context *context,
                                                 uint64_t id) {
    rxvm_reference_cell *cell;

    if (!context || id == 0) return 0;

    cell = context->active_buckets[rxvm_reference_bucket_index(id)];
    while (cell) {
        if (cell->id == id) return cell;
        cell = cell->next_active;
    }
    return 0;
}

rxvm_reference_cell *rxvm_reference_cell_create(rxvm_ref_owner_kind owner_kind,
                                                value *target,
                                                void *owner,
                                                uint64_t owner_generation,
                                                const char *debug_name) {
    return rxvm_reference_cell_create_in_context(0, owner_kind, target, owner,
                                                 owner_generation, debug_name);
}

rxvm_reference_cell *rxvm_reference_cell_create_in_context(rxvm_reference_context *context,
                                                           rxvm_ref_owner_kind owner_kind,
                                                           value *target,
                                                           void *owner,
                                                           uint64_t owner_generation,
                                                           const char *debug_name) {
    rxvm_reference_cell *cell;

    cell = rxvm_reference_context_pop_free(context);
    if (!cell) {
        cell = malloc(sizeof(rxvm_reference_cell));
        if (!cell) return 0;
    }

    rxvm_reference_cell_setup(cell, context, owner_kind, target, owner,
                              owner_generation, debug_name);
    rxvm_reference_context_add_active(context, cell);
    return cell;
}

void rxvm_reference_cell_release_slow(rxvm_reference_cell *cell) {
    rxvm_reference_context *context;

    if (!cell || cell->retain_count == 0) return;

    cell->retain_count--;
    if (cell->retain_count != 0) return;

    context = cell->context;
    rxvm_reference_context_remove_active(cell);
    rxvm_reference_context_push_free(context, cell);
}

rxvm_reference_cell *rxvm_reference_identity_for(value *target,
                                                 rxvm_ref_owner_kind owner_kind,
                                                 void *owner,
                                                 uint64_t owner_generation,
                                                 const char *debug_name) {
    return rxvm_reference_identity_for_context(0, target, owner_kind, owner,
                                               owner_generation, debug_name);
}

rxvm_reference_cell *rxvm_reference_identity_for_context(rxvm_reference_context *context,
                                                         value *target,
                                                         rxvm_ref_owner_kind owner_kind,
                                                         void *owner,
                                                         uint64_t owner_generation,
                                                         const char *debug_name) {
    if (!target) return 0;

    if (!target->reference_identity) {
        target->reference_identity = rxvm_reference_cell_create_in_context(context,
                                                                           owner_kind,
                                                                           target,
                                                                           owner,
                                                                           owner_generation,
                                                                           debug_name);
    } else {
        rxvm_reference_cell_retarget(target->reference_identity, owner_kind,
                                     target, owner, owner_generation,
                                     debug_name);
    }

    return target->reference_identity;
}
