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

#ifndef CREXX_RXSIGNATURE_H
#define CREXX_RXSIGNATURE_H

#include <stddef.h>

#define RXSIG_DESCRIPTOR_PREFIX "rxsig1|"

typedef struct rx_callable_arg {
    char *type;
    unsigned char is_ref;
    unsigned char is_optional;
    unsigned char is_vararg;
} rx_callable_arg;

typedef struct rx_callable_signature {
    char *name;
    char *return_type;
    rx_callable_arg *args;
    size_t arg_count;
} rx_callable_signature;

typedef int (*rxsig_type_assignable_fn)(void *userdata,
                                        const char *actual_type,
                                        const char *expected_type);

typedef struct rx_callable_compare_options {
    rxsig_type_assignable_fn type_assignable;
    void *userdata;
    int allow_return_covariance;
} rx_callable_compare_options;

void rx_sig_init_empty(rx_callable_signature *signature);
void rx_sig_free(rx_callable_signature *signature);

int rx_sig_init_from_parts(rx_callable_signature *signature,
                           const char *name,
                           const char *return_type,
                           const char *args);

int rx_sig_parse_descriptor(const char *descriptor,
                            rx_callable_signature *signature);

char *rx_sig_build_descriptor(const char *name,
                              const char *return_type,
                              const char *args);

int rx_sig_args_match(const rx_callable_signature *expected,
                      const rx_callable_signature *actual);

int rx_sig_matches_contract(const rx_callable_signature *expected,
                            const rx_callable_signature *actual,
                            const rx_callable_compare_options *options);

#endif
