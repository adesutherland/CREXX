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

rxvm_reference_cell *rxvm_reference_cell_create(rxvm_ref_owner_kind owner_kind,
                                                value *target,
                                                void *owner,
                                                uint64_t owner_generation,
                                                const char *debug_name);
void rxvm_reference_cell_retain(rxvm_reference_cell *cell);
void rxvm_reference_cell_release(rxvm_reference_cell *cell);
void rxvm_reference_cell_invalidate(rxvm_reference_cell *cell);
void rxvm_reference_cell_retarget(rxvm_reference_cell *cell,
                                  rxvm_ref_owner_kind owner_kind,
                                  value *target,
                                  void *owner,
                                  uint64_t owner_generation,
                                  const char *debug_name);
int rxvm_reference_cell_is_valid(const rxvm_reference_cell *cell);
value *rxvm_reference_cell_target(const rxvm_reference_cell *cell);

rxvm_reference_cell *rxvm_reference_identity_for(value *target,
                                                 rxvm_ref_owner_kind owner_kind,
                                                 void *owner,
                                                 uint64_t owner_generation,
                                                 const char *debug_name);
void rxvm_reference_identity_invalidate(value *target);
void rxvm_reference_identity_release(value *target);

#endif
