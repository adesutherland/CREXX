/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
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

#ifndef RXFLAGS_H
#define RXFLAGS_H

#include <stdint.h>

/*
 * Runtime value/register status flag partition.
 *
 * The VM-private band is readable by normal RXAS instructions but writable only
 * by VM internals. SETTP/SETORTP/LOADSETTP mask it out when applying external
 * flag values.
 */
#define RXFLAG_VM_PRIVATE_MASK       UINT32_C(0x000000FF)
#define RXFLAG_COMPILER_MASK         UINT32_C(0x0000FF00)
#define RXFLAG_LIBRARY_MASK          UINT32_C(0x00FF0000)
#define RXFLAG_USER_MASK             UINT32_C(0x7F000000)
#define RXFLAG_RESERVED_SIGN_MASK    UINT32_C(0x80000000)

#define RXFLAG_READABLE_MASK         UINT32_C(0x7FFFFFFF)
#define RXFLAG_PUBLIC_WRITABLE_MASK  (RXFLAG_COMPILER_MASK | RXFLAG_LIBRARY_MASK | RXFLAG_USER_MASK)
#define RXFLAG_PUBLIC_TEST_MASK      RXFLAG_PUBLIC_WRITABLE_MASK
#define RXFLAG_SOURCE_WRITABLE_MASK  (RXFLAG_LIBRARY_MASK | RXFLAG_USER_MASK)

/* VM-private string content/cache flags. */
#define RXFLAG_VM_UTF8_VALID         UINT32_C(0x00000001)
#define RXFLAG_VM_UTF8_COUNT_VALID   UINT32_C(0x00000002)
#define RXFLAG_VM_NORMAL_KNOWN       UINT32_C(0x00000004)
#define RXFLAG_VM_NORMAL_NFC         UINT32_C(0x00000008)
#define RXFLAG_VM_NORMAL_NFD         UINT32_C(0x00000010)
#define RXFLAG_VM_NORMAL_NFKC        UINT32_C(0x00000020)
#define RXFLAG_VM_NORMAL_NFKD        UINT32_C(0x00000040)
#define RXFLAG_VM_OBJECT_UNINITIALIZED UINT32_C(0x00000080)
#define RXFLAG_VM_NORMAL_FORM_MASK   (RXFLAG_VM_NORMAL_NFC | RXFLAG_VM_NORMAL_NFD | RXFLAG_VM_NORMAL_NFKC | RXFLAG_VM_NORMAL_NFKD)

/* Compiler call ABI flags. */
#define REGTP_VAL                    UINT32_C(0x00000100)
#define REGTP_NOTSYM                 UINT32_C(0x00000200)

static inline uint32_t rxflags_replace_requested_band(uint32_t current_flags,
                                                      uint32_t requested_flags,
                                                      uint32_t band_mask) {
    uint32_t requested_band = requested_flags & band_mask;
    return requested_band ? requested_band : (current_flags & band_mask);
}

static inline uint32_t rxflags_public_write(uint32_t current_flags,
                                            uint32_t requested_flags) {
    uint32_t requested_public = requested_flags & RXFLAG_PUBLIC_WRITABLE_MASK;

    if (!requested_public) {
        return current_flags & RXFLAG_VM_PRIVATE_MASK;
    }

    return (current_flags & RXFLAG_VM_PRIVATE_MASK) |
           rxflags_replace_requested_band(current_flags, requested_flags, RXFLAG_COMPILER_MASK) |
           rxflags_replace_requested_band(current_flags, requested_flags, RXFLAG_LIBRARY_MASK) |
           rxflags_replace_requested_band(current_flags, requested_flags, RXFLAG_USER_MASK);
}

#define RXFLAGS_PUBLIC_WRITE(current_flags, requested_flags) \
    rxflags_public_write((uint32_t)(current_flags), (uint32_t)(requested_flags))

#define RXFLAGS_PUBLIC_OR(current_flags, requested_flags) \
    ((current_flags) | ((requested_flags) & RXFLAG_PUBLIC_WRITABLE_MASK))

#define RXFLAGS_PUBLIC_REPLACE(requested_flags) \
    ((requested_flags) & RXFLAG_PUBLIC_WRITABLE_MASK)

#define RXFLAGS_SOURCE_MASKED_REPLACE(current_flags, requested_flags, mask) \
    (((current_flags) & ~((uint32_t)(mask) & RXFLAG_SOURCE_WRITABLE_MASK)) | \
     ((requested_flags) & ((uint32_t)(mask) & RXFLAG_SOURCE_WRITABLE_MASK)))

#endif
