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
 * Compiler source extension policy.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "rxcp_source_ext.h"

static int extension_equals_ci(const char *left, const char *right) {
    size_t i;

    if (!left || !right) return 0;
    for (i = 0; left[i] && right[i]; i++) {
        if (tolower((unsigned char) left[i]) != tolower((unsigned char) right[i])) return 0;
    }
    return left[i] == 0 && right[i] == 0;
}

static void add_extension(RxcpSourceExtension extensions[RXCP_SOURCE_EXTENSION_MAX],
                          size_t *count, const char *extension, RexxLevel default_level) {
    if (!extensions || !count || !extension || !extension[0]) return;
    if (*count >= RXCP_SOURCE_EXTENSION_MAX) return;
    extensions[*count].extension = extension;
    extensions[*count].default_level = default_level;
    (*count)++;
}

char *rxcp_source_extension_copy(const char *path) {
    const char *extension;
    char *copy;
    size_t length;
    size_t i;

    extension = path ? filenext(path) : "";
    length = strlen(extension);
    copy = malloc(length + 1);
    if (!copy) return 0;
    for (i = 0; i < length; i++) {
        copy[i] = (char) tolower((unsigned char) extension[i]);
    }
    copy[length] = 0;
    return copy;
}

int rxcp_source_extension_equals(const char *left, const char *right) {
    return extension_equals_ci(left, right);
}

int rxcp_source_extension_is_known_source(const char *extension) {
    return extension_equals_ci(extension, "crexx") ||
           extension_equals_ci(extension, "crx") ||
           extension_equals_ci(extension, "rexx");
}

int rxcp_source_extension_is_reserved(const char *extension) {
    return extension_equals_ci(extension, "rxas") ||
           extension_equals_ci(extension, "rxbin") ||
           extension_equals_ci(extension, "rxplugin") ||
           extension_equals_ci(extension, "rxpp");
}

RexxLevel rxcp_source_default_level_for_extension(const char *extension) {
    if (extension_equals_ci(extension, "rexx")) return LEVELC;
    if (extension_equals_ci(extension, "crexx")) return LEVELG;
    if (extension_equals_ci(extension, "crx")) return LEVELG;
    if (extension && extension[0] && !rxcp_source_extension_is_reserved(extension)) return LEVELG;
    return LEVELC;
}

RexxLevel rxcp_source_default_level_for_file(const char *file_name) {
    if (!file_name) return LEVELC;
    return rxcp_source_default_level_for_extension(filenext(file_name));
}

size_t rxcp_source_extension_list(const char *initial_extension,
                                  RxcpSourceExtension extensions[RXCP_SOURCE_EXTENSION_MAX]) {
    size_t count;

    count = 0;
    if (extensions) {
        size_t i;
        for (i = 0; i < RXCP_SOURCE_EXTENSION_MAX; i++) {
            extensions[i].extension = 0;
            extensions[i].default_level = UNKNOWN;
        }
    }

    if (initial_extension &&
        initial_extension[0] &&
        !rxcp_source_extension_is_known_source(initial_extension) &&
        !rxcp_source_extension_is_reserved(initial_extension)) {
        add_extension(extensions, &count, initial_extension, LEVELG);
    }

    add_extension(extensions, &count, "crexx", LEVELG);
    add_extension(extensions, &count, "crx", LEVELG);
    add_extension(extensions, &count, "rexx", LEVELC);
    return count;
}
