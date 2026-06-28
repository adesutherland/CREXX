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

#include "rxsignature.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static char *rxsig_strdup_range(const char *start, size_t length) {
    char *copy;

    copy = (char *)malloc(length + 1);
    if (!copy) return 0;
    if (length) memcpy(copy, start, length);
    copy[length] = 0;
    return copy;
}

static char *rxsig_strdup_trimmed(const char *start, size_t length) {
    const char *end;

    if (!start) return rxsig_strdup_range("", 0);

    end = start + length;
    while (start < end && isspace((unsigned char)*start)) start++;
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    return rxsig_strdup_range(start, (size_t)(end - start));
}

static int rxsig_starts_with_word(const char *text, const char *word) {
    size_t length;

    if (!text || !word) return 0;
    length = strlen(word);
    if (strncmp(text, word, length) != 0) return 0;
    return text[length] == 0 || isspace((unsigned char)text[length]);
}

static char *rxsig_normal_return_type(const char *return_type) {
    if (!return_type || !*return_type) return rxsig_strdup_range(".void", 5);
    return rxsig_strdup_trimmed(return_type, strlen(return_type));
}

void rx_sig_init_empty(rx_callable_signature *signature) {
    if (!signature) return;
    signature->name = 0;
    signature->return_type = 0;
    signature->args = 0;
    signature->arg_count = 0;
}

void rx_sig_free(rx_callable_signature *signature) {
    size_t i;

    if (!signature) return;
    if (signature->name) free(signature->name);
    if (signature->return_type) free(signature->return_type);
    if (signature->args) {
        for (i = 0; i < signature->arg_count; i++) {
            if (signature->args[i].type) free(signature->args[i].type);
        }
        free(signature->args);
    }
    rx_sig_init_empty(signature);
}

static int rx_sig_parse_one_arg(rx_callable_arg *arg, const char *start, size_t length) {
    char *text;
    char *cursor;
    char *equals;
    char *name_start;
    char *name_end;
    char *type_start;
    char *type_end;

    memset(arg, 0, sizeof(*arg));
    text = rxsig_strdup_trimmed(start, length);
    if (!text) return 0;
    if (!*text) {
        free(text);
        return 0;
    }

    cursor = text;
    if (rxsig_starts_with_word(cursor, "expose")) {
        arg->is_ref = 1;
        cursor += strlen("expose");
        while (*cursor && isspace((unsigned char)*cursor)) cursor++;
    }

    if (*cursor == '?') {
        arg->is_optional = 1;
        cursor++;
        while (*cursor && isspace((unsigned char)*cursor)) cursor++;
    }

    equals = strchr(cursor, '=');
    if (!equals) {
        free(text);
        return 0;
    }

    name_start = cursor;
    name_end = equals;
    while (name_end > name_start && isspace((unsigned char)*(name_end - 1))) name_end--;
    while (name_start < name_end && isspace((unsigned char)*name_start)) name_start++;
    if ((size_t)(name_end - name_start) == 3 && strncmp(name_start, "...", 3) == 0) {
        arg->is_vararg = 1;
    }

    type_start = equals + 1;
    type_end = text + strlen(text);
    while (type_start < type_end && isspace((unsigned char)*type_start)) type_start++;
    while (type_end > type_start && isspace((unsigned char)*(type_end - 1))) type_end--;
    if (type_start == type_end) {
        free(text);
        return 0;
    }

    arg->type = rxsig_strdup_range(type_start, (size_t)(type_end - type_start));
    free(text);
    return arg->type != 0;
}

static int rx_sig_parse_args(rx_callable_signature *signature, const char *args) {
    const char *cursor;
    const char *segment_start;
    size_t count;
    size_t index;

    if (!args || !*args) return 1;

    count = 1;
    for (cursor = args; *cursor; cursor++) {
        if (*cursor == ',') count++;
    }

    signature->args = (rx_callable_arg *)calloc(count, sizeof(rx_callable_arg));
    if (!signature->args) return 0;
    signature->arg_count = count;

    segment_start = args;
    index = 0;
    for (cursor = args; ; cursor++) {
        if (*cursor == ',' || *cursor == 0) {
            if (!rx_sig_parse_one_arg(&signature->args[index],
                                      segment_start,
                                      (size_t)(cursor - segment_start))) {
                return 0;
            }
            index++;
            if (*cursor == 0) break;
            segment_start = cursor + 1;
        }
    }

    return index == count;
}

int rx_sig_init_from_parts(rx_callable_signature *signature,
                           const char *name,
                           const char *return_type,
                           const char *args) {
    rx_sig_init_empty(signature);

    signature->name = rxsig_strdup_trimmed(name ? name : "", name ? strlen(name) : 0);
    signature->return_type = rxsig_normal_return_type(return_type);
    if (!signature->name || !signature->return_type) {
        rx_sig_free(signature);
        return 0;
    }

    if (!rx_sig_parse_args(signature, args)) {
        rx_sig_free(signature);
        return 0;
    }

    return 1;
}

int rx_sig_parse_descriptor(const char *descriptor,
                            rx_callable_signature *signature) {
    const char *name_start;
    const char *name_end;
    const char *return_start;
    const char *return_end;
    const char *args_start;
    size_t prefix_length;
    char *name;
    char *return_type;
    int ok;

    rx_sig_init_empty(signature);
    if (!descriptor) return 0;

    prefix_length = strlen(RXSIG_DESCRIPTOR_PREFIX);
    if (strncmp(descriptor, RXSIG_DESCRIPTOR_PREFIX, prefix_length) != 0) return 0;

    name_start = descriptor + prefix_length;
    name_end = strchr(name_start, '|');
    if (!name_end) return 0;
    return_start = name_end + 1;
    return_end = strchr(return_start, '|');
    if (!return_end) return 0;
    args_start = return_end + 1;

    name = rxsig_strdup_range(name_start, (size_t)(name_end - name_start));
    return_type = rxsig_strdup_range(return_start, (size_t)(return_end - return_start));
    if (!name || !return_type) {
        if (name) free(name);
        if (return_type) free(return_type);
        return 0;
    }

    ok = rx_sig_init_from_parts(signature, name, return_type, args_start);
    free(name);
    free(return_type);
    return ok;
}

char *rx_sig_build_descriptor(const char *name,
                              const char *return_type,
                              const char *args) {
    const char *actual_name;
    const char *actual_return;
    const char *actual_args;
    size_t prefix_length;
    size_t name_length;
    size_t return_length;
    size_t args_length;
    char *descriptor;
    char *cursor;

    actual_name = name ? name : "";
    actual_return = (return_type && *return_type) ? return_type : ".void";
    actual_args = args ? args : "";

    prefix_length = strlen(RXSIG_DESCRIPTOR_PREFIX);
    name_length = strlen(actual_name);
    return_length = strlen(actual_return);
    args_length = strlen(actual_args);

    descriptor = (char *)malloc(prefix_length + name_length + 1 + return_length + 1 + args_length + 1);
    if (!descriptor) return 0;

    cursor = descriptor;
    memcpy(cursor, RXSIG_DESCRIPTOR_PREFIX, prefix_length);
    cursor += prefix_length;
    memcpy(cursor, actual_name, name_length);
    cursor += name_length;
    *cursor++ = '|';
    memcpy(cursor, actual_return, return_length);
    cursor += return_length;
    *cursor++ = '|';
    memcpy(cursor, actual_args, args_length);
    cursor += args_length;
    *cursor = 0;

    return descriptor;
}

int rx_sig_args_match(const rx_callable_signature *expected,
                      const rx_callable_signature *actual) {
    size_t i;

    if (!expected || !actual) return 0;
    if (expected->arg_count != actual->arg_count) return 0;

    for (i = 0; i < expected->arg_count; i++) {
        const rx_callable_arg *left;
        const rx_callable_arg *right;

        left = &expected->args[i];
        right = &actual->args[i];
        if (left->is_ref != right->is_ref) return 0;
        if (left->is_optional != right->is_optional) return 0;
        if (left->is_vararg != right->is_vararg) return 0;
        if (!left->type || !right->type) return 0;
        if (strcmp(left->type, right->type) != 0) return 0;
    }

    return 1;
}

int rx_sig_matches_contract(const rx_callable_signature *expected,
                            const rx_callable_signature *actual,
                            const rx_callable_compare_options *options) {
    if (!expected || !actual) return 0;

    if (expected->name && *expected->name && actual->name && *actual->name &&
        strcmp(expected->name, actual->name) != 0) {
        return 0;
    }

    if (!rx_sig_args_match(expected, actual)) return 0;

    if (!expected->return_type || !actual->return_type) return 0;
    if (strcmp(expected->return_type, actual->return_type) == 0) return 1;

    if (options &&
        options->allow_return_covariance &&
        options->type_assignable &&
        options->type_assignable(options->userdata,
                                 actual->return_type,
                                 expected->return_type)) {
        return 1;
    }

    return 0;
}
