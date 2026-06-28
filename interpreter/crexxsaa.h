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

#ifndef CREXX_CREXXSAA_H
#define CREXX_CREXXSAA_H

#include <stddef.h>

#define CREXXSAA_ABI_VERSION 3

#define CREXXSAA_CACHE_DISABLE 0x01u
#define CREXXSAA_CACHE_REFRESH 0x02u

#define CREXXSAA_VARIABLE_OK 0
#define CREXXSAA_VARIABLE_NOT_FOUND 1
#define CREXXSAA_VARIABLE_NO_ACTIVE_REQUEST 2
#define CREXXSAA_VARIABLE_UNSUPPORTED 3
#define CREXXSAA_VARIABLE_BAD_NAME 4
#define CREXXSAA_VARIABLE_NO_MEMORY 5

typedef struct crexxsaa_context crexxsaa_context;

typedef struct crexxsaa_address_request {
    const char* environment_name;
    const char* command;
    crexxsaa_context* context;
} crexxsaa_address_request;

typedef struct crexxsaa_address_response {
    int rc;
    const char* condition_name;
    const char* diagnostic;
} crexxsaa_address_response;

typedef int (*crexxsaa_address_callback)(
    const crexxsaa_address_request* request,
    crexxsaa_address_response* response,
    void* userdata);

int crexxsaa_create(
    const char* location,
    const char* library_rxbin_path,
    crexxsaa_context** ctx_out);

void crexxsaa_destroy(crexxsaa_context* ctx);

int crexxsaa_register_address_environment(
    crexxsaa_context* ctx,
    const char* env_name,
    crexxsaa_address_callback callback,
    void* userdata);

int crexxsaa_set_address_environment(
    crexxsaa_context* ctx,
    const char* env_name);

int crexxsaa_set_compiler(
    crexxsaa_context* ctx,
    const char* rxc_path,
    const char* rxas_path,
    const char* import_dir);

int crexxsaa_set_cache_dir(
    crexxsaa_context* ctx,
    const char* cache_dir);

int crexxsaa_address_variable_set(
    crexxsaa_context* ctx,
    const char* name,
    const char* value,
    size_t value_len);

int crexxsaa_address_variable_get_alloc(
    crexxsaa_context* ctx,
    const char* name,
    char** value_out,
    size_t* value_len_out);

void crexxsaa_free(void* ptr);

int crexxsaa_run_rxbin(
    crexxsaa_context* ctx,
    const char* rxbin_path,
    int argc,
    const char** argv,
    int* program_rc);

int crexxsaa_run_source(
    crexxsaa_context* ctx,
    const char* source_path,
    const char* cache_namespace,
    unsigned flags,
    int argc,
    const char** argv,
    int* program_rc);

int crexxsaa_invalidate_source(
    crexxsaa_context* ctx,
    const char* source_path,
    const char* cache_namespace);

int crexxsaa_invalidate_all(crexxsaa_context* ctx);

int crexxsaa_get_cache_dir(
    const char* cache_dir_override,
    char* buffer,
    size_t buffer_len);

int crexxsaa_clear_cache(const char* cache_dir_override);

const char* crexxsaa_last_error(crexxsaa_context* ctx);

#endif /* CREXX_CREXXSAA_H */
