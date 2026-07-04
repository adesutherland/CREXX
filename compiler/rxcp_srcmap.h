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

/**
 * Raw source-map prepass for preprocessed CREXX input.
 */

#ifndef CREXX_RXCP_SRCMAP_H
#define CREXX_RXCP_SRCMAP_H

#include <stddef.h>
#include "rxcp_types.h"

typedef struct RxcpSrcMapLocation {
    const char *file_name;
    int line;
    int column;
    int length;
    const char *line_text;
    size_t line_text_length;
} RxcpSrcMapLocation;

typedef struct RxcpSrcMapRawMapping {
    size_t *cleaned_to_raw_start;
    size_t *cleaned_to_raw_end;
    size_t cleaned_len;
    size_t raw_len;
} RxcpSrcMapRawMapping;

int rxcp_srcmap_preprocess(Context *context, char **cleaned_out, size_t *cleaned_len_out);
int rxcp_srcmap_preprocess_with_raw_map(Context *context,
                                        char **cleaned_out,
                                        size_t *cleaned_len_out,
                                        RxcpSrcMapRawMapping *mapping_out);
int rxcp_srcmap_lookup(Context *context,
                       const char *generated_ptr,
                       int generated_line,
                       int generated_column,
                       RxcpSrcMapLocation *location_out);
void rxcp_srcmap_free(RxcpSrcMap *map);
void rxcp_srcmap_raw_mapping_free(RxcpSrcMapRawMapping *mapping);

#endif
