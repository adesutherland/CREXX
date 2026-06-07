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
#include "rxvmref.h"

static uint64_t next_reference_id = 1;

static uint64_t rxvm_next_reference_id(void) {
    uint64_t id = next_reference_id++;
    if (next_reference_id == 0) next_reference_id = 1;
    return id;
}

rxvm_reference_cell *rxvm_reference_cell_create(rxvm_ref_owner_kind owner_kind,
                                                value *target,
                                                void *owner,
                                                uint64_t owner_generation,
                                                const char *debug_name) {
    rxvm_reference_cell *cell = malloc(sizeof(rxvm_reference_cell));
    if (!cell) return 0;

    cell->id = rxvm_next_reference_id();
    cell->retain_count = 1;
    cell->state = target ? RXVM_REF_VALID : RXVM_REF_INVALID;
    cell->owner_kind = owner_kind;
    cell->target = target;
    cell->owner = owner;
    cell->owner_generation = owner_generation;
    cell->debug_name = debug_name;
    return cell;
}

void rxvm_reference_cell_retain(rxvm_reference_cell *cell) {
    if (!cell) return;
    cell->retain_count++;
}

void rxvm_reference_cell_release(rxvm_reference_cell *cell) {
    if (!cell) return;
    if (cell->retain_count == 0) return;
    cell->retain_count--;
    if (cell->retain_count == 0) free(cell);
}

void rxvm_reference_cell_invalidate(rxvm_reference_cell *cell) {
    if (!cell) return;
    cell->state = RXVM_REF_INVALID;
    cell->target = 0;
}

void rxvm_reference_cell_retarget(rxvm_reference_cell *cell,
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

int rxvm_reference_cell_is_valid(const rxvm_reference_cell *cell) {
    return cell && cell->state == RXVM_REF_VALID && cell->target;
}

value *rxvm_reference_cell_target(const rxvm_reference_cell *cell) {
    if (!rxvm_reference_cell_is_valid(cell)) return 0;
    return cell->target;
}

rxvm_reference_cell *rxvm_reference_identity_for(value *target,
                                                 rxvm_ref_owner_kind owner_kind,
                                                 void *owner,
                                                 uint64_t owner_generation,
                                                 const char *debug_name) {
    if (!target) return 0;

    if (!target->reference_identity) {
        target->reference_identity = rxvm_reference_cell_create(owner_kind, target,
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

void rxvm_reference_identity_invalidate(value *target) {
    if (!target || !target->reference_identity) return;
    rxvm_reference_cell_invalidate(target->reference_identity);
}

void rxvm_reference_identity_release(value *target) {
    rxvm_reference_cell *cell;
    if (!target || !target->reference_identity) return;

    cell = target->reference_identity;
    target->reference_identity = 0;
    rxvm_reference_cell_invalidate(cell);
    rxvm_reference_cell_release(cell);
}
