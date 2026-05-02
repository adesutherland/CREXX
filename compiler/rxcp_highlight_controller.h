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

#ifdef ENABLE_PARSER_MODE

#ifndef CREXX_RXCP_HIGHLIGHT_CONTROLLER_H
#define CREXX_RXCP_HIGHLIGHT_CONTROLLER_H

#ifndef restrict
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
/* restrict is a keyword */
#elif defined(__GNUC__) || defined(__clang__)
#define restrict __restrict
#elif defined(_MSC_VER)
#define restrict __restrict
#else
#define restrict
#endif
#endif

#include "dslsyntax_common.h"

typedef struct RXCPHighlightCacheStats {
    unsigned long generation;
    unsigned long invalidation_count;
    unsigned long exit_warm_count;
    unsigned long import_inventory_warm_count;
    size_t cached_import_file_count;
    size_t cached_import_function_count;
    size_t cached_exit_primary_count;
    size_t watched_directory_count;
    size_t watched_file_count;
    int exits_disabled;
} RXCPHighlightCacheStats;

void rxc_highlight_controller_parse(CodeBuffer *cb);
void rxcp_highlight_controller_reset_cache(void);
void rxcp_highlight_controller_get_cache_stats(RXCPHighlightCacheStats *stats);

#endif

#endif
