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

#ifndef CREXX_RXVMREF_H
#define CREXX_RXVMREF_H

#include "rxvalue.h"

/*
 * Keep cheap reference operations local to hot VM value paths. If platform.h
 * has already supplied RX_INLINE we use it; otherwise fall back to static for
 * C90-ish helper targets.
 */
#ifdef RX_INLINE
#define RXVM_REF_INLINE RX_INLINE
#else
#define RXVM_REF_INLINE static
#endif

void rxvm_reference_context_init(rxvm_reference_context *context);
void rxvm_reference_context_free(rxvm_reference_context *context);
rxvm_reference_cell *rxvm_reference_context_find(rxvm_reference_context *context,
                                                 uint64_t id);

rxvm_reference_cell *rxvm_reference_cell_create(rxvm_ref_owner_kind owner_kind,
                                                value *target,
                                                void *owner,
                                                uint64_t owner_generation,
                                                const char *debug_name);
rxvm_reference_cell *rxvm_reference_cell_create_in_context(rxvm_reference_context *context,
                                                           rxvm_ref_owner_kind owner_kind,
                                                           value *target,
                                                           void *owner,
                                                           uint64_t owner_generation,
                                                           const char *debug_name);
void rxvm_reference_cell_release_slow(rxvm_reference_cell *cell);

RXVM_REF_INLINE void rxvm_reference_cell_retain(rxvm_reference_cell *cell) {
    if (cell) cell->retain_count++;
}

RXVM_REF_INLINE void rxvm_reference_cell_release(rxvm_reference_cell *cell) {
    if (cell) rxvm_reference_cell_release_slow(cell);
}

RXVM_REF_INLINE void rxvm_reference_cell_invalidate(rxvm_reference_cell *cell) {
    if (!cell) return;
    cell->state = RXVM_REF_INVALID;
    cell->target = 0;
}

RXVM_REF_INLINE void rxvm_reference_cell_retarget(rxvm_reference_cell *cell,
                                                  rxvm_ref_owner_kind owner_kind,
                                                  value *target,
                                                  void *owner,
                                                  uint64_t owner_generation,
                                                  const char *debug_name) {
    if (!cell) return;
    cell->state = target ? RXVM_REF_VALID : RXVM_REF_INVALID;
    cell->owner_kind = owner_kind;
    cell->target = target;
    cell->owner = owner;
    cell->owner_generation = owner_generation;
    cell->debug_name = debug_name;
}

RXVM_REF_INLINE int rxvm_reference_cell_is_valid(const rxvm_reference_cell *cell) {
    return cell && cell->state == RXVM_REF_VALID && cell->target;
}

RXVM_REF_INLINE value *rxvm_reference_cell_target(const rxvm_reference_cell *cell) {
    return rxvm_reference_cell_is_valid(cell) ? cell->target : 0;
}

rxvm_reference_cell *rxvm_reference_identity_for(value *target,
                                                 rxvm_ref_owner_kind owner_kind,
                                                 void *owner,
                                                 uint64_t owner_generation,
                                                 const char *debug_name);
rxvm_reference_cell *rxvm_reference_identity_for_context(rxvm_reference_context *context,
                                                         value *target,
                                                         rxvm_ref_owner_kind owner_kind,
                                                         void *owner,
                                                         uint64_t owner_generation,
                                                         const char *debug_name);

RXVM_REF_INLINE void rxvm_reference_identity_invalidate(value *target) {
    if (target && target->reference_identity) {
        rxvm_reference_cell_invalidate(target->reference_identity);
    }
}

RXVM_REF_INLINE void rxvm_reference_identity_release(value *target) {
    rxvm_reference_cell *cell;
    if (!target || !target->reference_identity) return;

    cell = target->reference_identity;
    target->reference_identity = 0;
    rxvm_reference_cell_invalidate(cell);
    rxvm_reference_cell_release(cell);
}

RXVM_REF_INLINE void rxvm_reference_identity_move(value *dest, value *source) {
    rxvm_reference_cell *cell;

    if (!dest || !source || dest == source) return;

    if (dest->reference_identity) rxvm_reference_identity_release(dest);
    cell = source->reference_identity;
    source->reference_identity = 0;
    dest->reference_identity = cell;

    if (cell) {
        rxvm_reference_cell_retarget(cell, cell->owner_kind, dest, cell->owner,
                                     cell->owner_generation, cell->debug_name);
    }
}

RXVM_REF_INLINE void rxvm_reference_value_release_payload(value *target) {
    rxvm_reference_cell *cell;

    if (!target || !target->reference_payload) return;

    cell = target->reference_payload;
    target->reference_payload = 0;
    rxvm_reference_cell_release(cell);
}

RXVM_REF_INLINE void rxvm_reference_value_set_payload(value *target, rxvm_reference_cell *cell) {
    if (!target) return;

    if (cell) rxvm_reference_cell_retain(cell);
    if (target->reference_payload) rxvm_reference_value_release_payload(target);
    target->reference_payload = cell;
}

RXVM_REF_INLINE void rxvm_reference_value_copy_payload(value *dest, const value *source) {
    if (!dest) return;

    if (source && source->reference_payload) {
        rxvm_reference_cell_retain(source->reference_payload);
    }

    if (dest->reference_payload) rxvm_reference_value_release_payload(dest);
    dest->reference_payload = source ? source->reference_payload : 0;
}

RXVM_REF_INLINE void rxvm_reference_value_move_payload(value *dest, value *source) {
    if (!dest || !source || dest == source) return;

    if (dest->reference_payload) rxvm_reference_value_release_payload(dest);
    dest->reference_payload = source->reference_payload;
    source->reference_payload = 0;
}

#endif
