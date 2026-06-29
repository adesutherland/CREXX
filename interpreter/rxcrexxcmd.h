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

#ifndef CREXX_RXCREXXCMD_H
#define CREXX_RXCREXXCMD_H

#include <stddef.h>

typedef int (*rxcrexxcmd_write_fn)(void *userdata, const char *text, size_t length);
typedef int (*rxcrexxcmd_read_all_fn)(void *userdata, char **out_text, size_t *out_length);
typedef int (*rxcrexxcmd_run_path_fn)(
    void *userdata,
    const char *command,
    char **out_text,
    char **err_text,
    int *command_rc,
    char **error_text);

typedef struct rxcrexxcmd_io {
    rxcrexxcmd_write_fn write_output;
    rxcrexxcmd_write_fn write_error;
    rxcrexxcmd_read_all_fn read_input;
    rxcrexxcmd_run_path_fn run_path;
    void *userdata;
} rxcrexxcmd_io;

int rxcrexxcmd_execute(
    const char *command,
    const rxcrexxcmd_io *io,
    int *rc,
    char **error_text);

void rxcrexxcmd_free(char *text);

#endif
