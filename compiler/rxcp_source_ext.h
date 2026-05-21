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

#ifndef CREXX_RXCP_SOURCE_EXT_H
#define CREXX_RXCP_SOURCE_EXT_H

#include <stddef.h>
#include "rxcp_types.h"

#define RXCP_SOURCE_EXTENSION_MAX 4

typedef struct RxcpSourceExtension {
    const char *extension;
    RexxLevel default_level;
} RxcpSourceExtension;

char *rxcp_source_extension_copy(const char *path);
int rxcp_source_extension_is_known_source(const char *extension);
int rxcp_source_extension_is_reserved(const char *extension);
int rxcp_source_extension_equals(const char *left, const char *right);
RexxLevel rxcp_source_default_level_for_extension(const char *extension);
RexxLevel rxcp_source_default_level_for_file(const char *file_name);
size_t rxcp_source_extension_list(const char *initial_extension,
                                  RxcpSourceExtension extensions[RXCP_SOURCE_EXTENSION_MAX]);

#endif
