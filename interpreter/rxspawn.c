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

//
// Created by Adrian Sutherland on 03/05/2023.
//

#ifdef __linux__
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <dirent.h>
#include <pthread.h>
#endif

#ifdef _WIN32
#include <windows.h>
#ifndef _MSC_VER // Windows Visual Studio
#include <stdint.h>
#endif
#endif

#ifdef __APPLE__
#define _GNU_SOURCE            /* See feature_test_macros(7) */
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <pthread.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#include "rxvmintp.h"
#include "rxvmbyteendpoint.h"
#include "rxvmvars.h"
#include "rxcrexxcmd.h"

// Private structure to allow all the threads to share data etc. and
// make the shellspawn() call re-enterent
typedef struct shelldata {
    REDIRECT* pInput;
    REDIRECT* pOutput;
    REDIRECT* pError;
    char *waitThreadErrorText;
    int waitThreadRC;
#ifdef _WIN32
    PROCESS_INFORMATION ChildProcessInfo;
    HANDLE ChildJob;
#else
    int ChildProcessPID;
#endif
    int ChildProcessRC;
    char* buffer;
    char* file_path;
    char* application_path;
    char** argv;
    char *working_directory;
    char **environment;
    value* variables;
    value* crexx_bindings;
    const char *const *crexx_binding_snapshot;
    size_t crexx_binding_snapshot_count;
    const atomic_uchar *cancelled;
    atomic_uchar *input_stopped;
    atomic_uchar *output_stopped;
    uint64_t deadline_microseconds;
    unsigned char terminated;
    unsigned char timed_out;
} SHELLDATA;

#if defined(_MSC_VER)
#define RXSPAWN_THREAD_LOCAL __declspec(thread)
#else
#define RXSPAWN_THREAD_LOCAL __thread
#endif

typedef struct rxspawn_snapshot_override {
    const char *working_directory;
    const char *const *environment;
    const char *const *crexx_bindings;
    size_t crexx_binding_count;
    const atomic_uchar *cancelled;
    atomic_uchar *input_stopped;
    atomic_uchar *output_stopped;
    uint64_t deadline_microseconds;
    int *termination_reason;
} RXSPAWN_SNAPSHOT_OVERRIDE;

static RXSPAWN_THREAD_LOCAL const RXSPAWN_SNAPSHOT_OVERRIDE
        *rxspawn_thread_snapshot;

#ifdef _WIN32
typedef HANDLE REDIRECT_IO_HANDLE;
#define REDIRECT_INVALID_IO_HANDLE INVALID_HANDLE_VALUE
#else
typedef int REDIRECT_IO_HANDLE;
#define REDIRECT_INVALID_IO_HANDLE (-1)
#endif

enum {
    REDIRECT_TRANSFER_NONE = 0,
    REDIRECT_TRANSFER_INPUT_STRING = 1,
    REDIRECT_TRANSFER_INPUT_ARRAY = 2,
    REDIRECT_TRANSFER_OUTPUT_STRING = 3,
    REDIRECT_TRANSFER_OUTPUT_ARRAY = 4,
    REDIRECT_TRANSFER_ENDPOINT_INPUT = 5,
    REDIRECT_TRANSFER_ENDPOINT_OUTPUT = 6
};

enum {
    REDIRECT_COMPLETION_CREATED = 0,
    REDIRECT_COMPLETION_RUNNING = 1,
    REDIRECT_COMPLETION_SUCCEEDED = 2,
    REDIRECT_COMPLETION_FAILED = 3
};

/*
 * Single-shot EF-0 transfer state. This object is deliberately libc-owned and
 * contains no VM worker or value pointer: the I/O thread owns it until join,
 * then the receiver thread consumes or discards its byte payload exactly once.
 */
typedef struct redirect_completion {
    REDIRECT_IO_HANDLE io_handle;
    char *bytes;
    size_t length;
    size_t capacity;
    rxvm_byte_endpoint *endpoint;
    const atomic_uchar *cancelled;
    int errorCode;
    int lastError;
    int errorSource;
    unsigned char transfer_mode;
    unsigned char terminal_state;
    unsigned char consumed;
} REDIRECT_COMPLETION;

struct redirect {
    rxvm_memory_worker *receiver_worker;
#ifdef _WIN32
    HANDLE hRead;
    HANDLE hWrite;
    HANDLE thread;
#else
    int hRead;
    int hWrite;
    pthread_t thread;
#endif
    char has_thread;
    value* receiver;
    int errorCode;
    int lastError;
    int errorSource;
    REDIRECT_COMPLETION *completion;
    char endpoint_kind;
};

// Defined in a header file: typedef struct redirect REDIRECT;

enum {
    REDIRECT_ENDPOINT_NULL = 0,
    REDIRECT_ENDPOINT_INPUT = 1,
    REDIRECT_ENDPOINT_OUTPUT = 2
};

typedef struct redirect_endpoint_cell {
    int refcount;
    REDIRECT redirect;
} REDIRECT_ENDPOINT_CELL;

typedef struct redirect_endpoint_payload {
    REDIRECT_ENDPOINT_CELL *cell;
} REDIRECT_ENDPOINT_PAYLOAD;

#ifdef _WIN32
#define THREAD_RETURN unsigned long
#else
#define THREAD_RETURN void*
#endif

// Private functions
static void Error(char *context, char **errorText);
static void CleanUp(SHELLDATA* data);
static char *copy_string(const char *text);
static int ParseCommand(const char *command_string, char **command, char **file, char ***argv);
static int merge_child_variables(SHELLDATA *data);
static int spawn_argv_capture(const char *const *argv,
                              int argc,
                              REDIRECT* pIn,
                              REDIRECT* pOut,
                              REDIRECT* pErr,
                              value* variables,
                              int *rc,
                              char **errorText);
static int launchChild(SHELLDATA* data);
static void WaitForProcess(SHELLDATA* data);
static void appendTextOutput(char **outputText, char *inputText);
static void WriteToStdin(REDIRECT* data, char *line, size_t nBytes);
static void redirectInput(value* redirect_reg, value* string_reg, unsigned char transfer_mode);
static void redirectOutput(value* redirect_reg, value* string_reg, unsigned char transfer_mode);
static REDIRECT *redirect_endpoint_create(value *redirect_reg, char endpoint_kind);
static void redirect_endpoint_init(REDIRECT *redirect, char endpoint_kind);
static void redirect_endpoint_payload_copy(void *dest_value, void *source_value);
static void redirect_endpoint_payload_finalize(void *payload_value);
static value* add_new_element(value* array); /* Appends record to an array and returns the new record */
static THREAD_RETURN OutputCaptureThread(void* lpvThreadParam);
static THREAD_RETURN InputSnapshotThread(void* lpvThreadParam);
static REDIRECT_COMPLETION *redirect_completion_create(unsigned char transfer_mode);
static void redirect_completion_destroy(REDIRECT_COMPLETION *completion);
static int redirect_completion_append(REDIRECT_COMPLETION *completion,
                                      const char *bytes, size_t length);
static void redirect_completion_publish(REDIRECT_COMPLETION *completion,
                                        unsigned char terminal_state);
static int redirect_pipe_start(REDIRECT *redirect,
                               REDIRECT_COMPLETION *completion,
                               int output_from_child);
static void collect_redirect_thread_context(REDIRECT *redirect);
static int join_redirect_thread(REDIRECT *redirect);
#ifndef _WIN32
static int ExeFound(char* exe);
static char *find_executable_in_path_list(const char *path_list,
                                          const char *exe,
                                          const char *working_directory);
static char *resolve_executable_path(const char *working_directory,
                                     const char *exe);
static char *find_standard_shell(void);
static int split_shell_args(char *text, char **argv);
static char **build_shell_argv(const char *shell_path, const char *args_text, char *buffer, char *command_text);
#endif
static int crexxcmd_write_output(void *userdata, const char *text, size_t length);
static int crexxcmd_write_error(void *userdata, const char *text, size_t length);
static int crexxcmd_read_input(void *userdata, char **out_text, size_t *out_length);
static int crexxcmd_run_path(void *userdata,
                             const char *command,
                             char **out_text,
                             char **err_text,
                             int *command_rc,
                             char **error_text);
static int crexxcmd_run_argv(void *userdata,
                             int argc,
                             const char *const *argv,
                             char **out_text,
                             char **err_text,
                             int *command_rc,
                             char **error_text);
static int crexxcmd_get_binding(void *userdata, const char *name, char **out_value);
static int crexxcmd_get_stem_count(void *userdata, const char *name, size_t *out_count);
static int crexxcmd_get_stem_value(void *userdata, const char *name, size_t index, char **out_value);
static int crexxcmd_finalize_redirects(SHELLDATA *data, char **errorText);
static int crexxcmd_write_redirect(REDIRECT *redirect, FILE *fallback, const char *text, size_t length);
static int crexxcmd_read_redirect(REDIRECT *redirect, char **out_text, size_t *out_length);
static int crexxcmd_close_output_redirect(REDIRECT *redirect);
static int crexxcmd_close_input_redirect(REDIRECT *redirect);
static char *copy_value_string(value *string_value);

static const RXSPAWN_SNAPSHOT_OVERRIDE *crexxcmd_enter_parent_snapshot(
        const SHELLDATA *parent_data,
        RXSPAWN_SNAPSHOT_OVERRIDE *snapshot,
        int *termination_reason) {
    const RXSPAWN_SNAPSHOT_OVERRIDE *previous = rxspawn_thread_snapshot;

    if (!parent_data || !snapshot) return previous;
    snapshot->working_directory = parent_data->working_directory;
    snapshot->environment = (const char *const *)parent_data->environment;
    snapshot->crexx_bindings = parent_data->crexx_binding_snapshot;
    snapshot->crexx_binding_count = parent_data->crexx_binding_snapshot_count;
    snapshot->cancelled = parent_data->cancelled;
    snapshot->input_stopped = parent_data->input_stopped;
    snapshot->output_stopped = parent_data->output_stopped;
    snapshot->deadline_microseconds = parent_data->deadline_microseconds;
    snapshot->termination_reason = termination_reason;
    if (termination_reason) *termination_reason = 0;
    rxspawn_thread_snapshot = snapshot;
    return previous;
}

static void crexxcmd_leave_parent_snapshot(
        SHELLDATA *parent_data,
        const RXSPAWN_SNAPSHOT_OVERRIDE *previous,
        int termination_reason) {
    rxspawn_thread_snapshot = previous;
    if (!parent_data || termination_reason == 0) return;
    parent_data->terminated = 1u;
    if (termination_reason == 2) parent_data->timed_out = 1u;
}

static const rxvm_native_payload_ops redirect_endpoint_payload_ops = {
    "rxsysb.redirect_endpoint",
    redirect_endpoint_payload_copy,
    redirect_endpoint_payload_finalize
};

static void *rxspawn_memory_alloc(size_t size) {
    return rxvm_memory_alloc_bytes(rxvm_memory_current_worker(), size);
}

static void rxspawn_memory_free(void *pointer) {
    rxvm_memory_worker *previous;
    if (!pointer) return;
    previous = rxvm_memory_enter(rxvm_memory_owner(pointer));
    (void)rxvm_memory_release(pointer);
    rxvm_memory_leave(previous);
}

static void *rxspawn_memory_resize(void *pointer,
                                   size_t copy_size,
                                   size_t new_size) {
    rxvm_memory_worker *worker = pointer
        ? rxvm_memory_owner(pointer)
        : rxvm_memory_current_worker();
    rxvm_memory_worker *previous = rxvm_memory_enter(worker);
    void *resized = rxvm_memory_resize_bytes(worker, pointer,
                                             copy_size, new_size);
    rxvm_memory_leave(previous);
    return resized;
}

/* Text returned through the historical spawn/RXCREXXCMD callbacks is libc-owned. */
static char *copy_string_external(const char *text) {
    size_t length = strlen(text);
    char *copy = malloc(length + 1u);
    if (copy) memcpy(copy, text, length + 1u);
    return copy;
}

static int rxspawn_copy_process_snapshot(
        const RXSPAWN_SNAPSHOT_OVERRIDE *snapshot,
        char **working_directory,
        char ***environment) {
    size_t count = 0u;
    char **copy = NULL;
    if (working_directory) *working_directory = NULL;
    if (environment) *environment = NULL;
    if (!snapshot) return -1;
    if (snapshot->working_directory && working_directory) {
        *working_directory = copy_string_external(
                snapshot->working_directory);
        if (!*working_directory) return -1;
    }
    if (snapshot->environment) {
        while (snapshot->environment[count]) count++;
        copy = (char **)calloc(count + 1u, sizeof(*copy));
        if (!copy) goto failed;
        for (count = 0u; snapshot->environment[count]; count++) {
            copy[count] = copy_string_external(snapshot->environment[count]);
            if (!copy[count]) goto failed;
        }
    }
    if (environment) *environment = copy;
    else rxcrexxcmd_process_snapshot_free(NULL, copy);
    return 0;

failed:
    rxcrexxcmd_process_snapshot_free(
            working_directory ? *working_directory : NULL, copy);
    if (working_directory) *working_directory = NULL;
    return -1;
}

static uint64_t rxspawn_monotonic_microseconds(void) {
#ifdef _WIN32
    return (uint64_t)GetTickCount64() * UINT64_C(1000);
#else
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) return 0u;
    return (uint64_t)now.tv_sec * UINT64_C(1000000) +
           (uint64_t)now.tv_nsec / UINT64_C(1000);
#endif
}

static void rxspawn_stop_redirect_io(SHELLDATA *data, int stop_outputs) {
    REDIRECT *redirects[3];
    size_t index;
    if (!data) return;
    redirects[0] = data->pInput;
    redirects[1] = data->pOutput;
    redirects[2] = data->pError;
    /* Snapshot stop flags belong to the byte-endpoint adapters that were
     * installed by the child provider. A nested CREXX run also inherits the
     * snapshot while using ordinary local capture redirects; it must not
     * cancel its parent's endpoint merely because those local pipes close. */
    if (data->input_stopped && redirects[0] && redirects[0]->completion &&
        redirects[0]->completion->endpoint) {
        atomic_store_explicit(
                data->input_stopped, 1u, memory_order_release);
    }
    if (stop_outputs && data->output_stopped &&
        ((redirects[1] && redirects[1]->completion &&
          redirects[1]->completion->endpoint) ||
         (redirects[2] && redirects[2]->completion &&
          redirects[2]->completion->endpoint))) {
        atomic_store_explicit(
                data->output_stopped, 1u, memory_order_release);
    }
    for (index = 0u; index < 3u; index++) {
        REDIRECT_COMPLETION *completion = redirects[index]
                ? redirects[index]->completion : NULL;
        if (completion && completion->endpoint &&
            (index == 0u || stop_outputs)) {
            rxvm_byte_endpoint_wake(completion->endpoint);
        }
    }
}

static int rxspawn_stop_requested(SHELLDATA *data) {
    if (!data) return 0;
    if (data->cancelled && atomic_load_explicit(
            data->cancelled, memory_order_acquire)) {
        rxspawn_stop_redirect_io(data, 1);
        return 1;
    }
    if (data->deadline_microseconds &&
        rxspawn_monotonic_microseconds() >= data->deadline_microseconds) {
        data->timed_out = 1u;
        rxspawn_stop_redirect_io(data, 1);
        return 1;
    }
    return 0;
}

static char *copy_string(const char *text) {
    size_t length = strlen(text);
    char *copy = rxspawn_memory_alloc(length + 1u);
    if (copy) {
        memcpy(copy, text, length + 1);
    }
    return copy;
}

#ifdef _WIN32
static char *copy_string_length(const char *text, size_t length) {
    char *copy = rxspawn_memory_alloc(length + 1u);
    if (copy) {
        memcpy(copy, text, length);
        copy[length] = '\0';
    }
    return copy;
}

static wchar_t *wide_from_utf8(const char *text) {
    int length;
    wchar_t *wide;

    if (!text) return NULL;
    length = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
    if (length <= 0) return NULL;
    wide = rxspawn_memory_alloc((size_t)length * sizeof(wchar_t));
    if (!wide) return NULL;
    if (!MultiByteToWideChar(CP_UTF8, 0, text, -1, wide, length)) {
        rxspawn_memory_free(wide);
        return NULL;
    }
    return wide;
}

static char *utf8_from_wide(const wchar_t *wide) {
    int length;
    char *text;

    if (!wide) return NULL;
    length = WideCharToMultiByte(CP_UTF8, 0, wide, -1, NULL, 0, NULL, NULL);
    if (length <= 0) return NULL;
    text = rxspawn_memory_alloc((size_t)length);
    if (!text) return NULL;
    if (!WideCharToMultiByte(CP_UTF8, 0, wide, -1, text, length, NULL, NULL)) {
        rxspawn_memory_free(text);
        return NULL;
    }
    return text;
}

static char *windows_command_token(const char *command) {
    const char *start;
    const char *end;
    char quote;
    char *token;

    if (!command) return NULL;
    start = command;
    while (*start && isspace((unsigned char)*start)) start++;
    if (!*start) return NULL;

    quote = 0;
    if (*start == '"' || *start == '\'') quote = *start++;
    end = start;
    if (quote) {
        while (*end && *end != quote) end++;
    } else {
        while (*end && !isspace((unsigned char)*end)) end++;
    }
    if (end == start) return NULL;

    token = copy_string_length(start, (size_t)(end - start));
    if (!token) return NULL;
    for (char *cursor = token; *cursor; cursor++) {
        if (*cursor == '/') *cursor = '\\';
    }
    return token;
}

static int windows_path_is_absolute(const char *path) {
    return path &&
           ((isalpha((unsigned char)path[0]) && path[1] == ':') ||
            (path[0] == '\\' && path[1] == '\\') || path[0] == '/');
}

static char *windows_join_path(const char *directory, const char *path) {
    size_t directory_length;
    size_t path_length;
    int needs_separator;
    char *joined;
    if (!path) return NULL;
    if (!directory || !*directory || windows_path_is_absolute(path)) {
        return copy_string(path);
    }
    directory_length = strlen(directory);
    path_length = strlen(path);
    needs_separator = directory[directory_length - 1u] != '\\' &&
                      directory[directory_length - 1u] != '/';
    joined = rxspawn_memory_alloc(directory_length + path_length +
                                  (size_t)needs_separator + 1u);
    if (!joined) return NULL;
    memcpy(joined, directory, directory_length);
    if (needs_separator) joined[directory_length++] = '\\';
    memcpy(joined + directory_length, path, path_length + 1u);
    return joined;
}

static char *windows_resolve_search_path(const char *path_list,
                                         const char *working_directory) {
    const char *cursor;
    size_t size;
    char *resolved;
    char *writer;

    if (!working_directory || !*working_directory) {
        return path_list ? copy_string(path_list) : NULL;
    }
    size = strlen(working_directory) + 1u;
    for (cursor = path_list; cursor && *cursor;) {
        const char *next = strchr(cursor, ';');
        const char *start = cursor;
        size_t length = next ? (size_t)(next - cursor) : strlen(cursor);
        if (length >= 2u && start[0] == '"' && start[length - 1u] == '"') {
            start++;
            length -= 2u;
        }
        if (length) {
            size += length + 1u;
            if (!windows_path_is_absolute(start)) {
                size += strlen(working_directory) + 1u;
            }
        }
        cursor = next ? next + 1 : NULL;
    }
    resolved = rxspawn_memory_alloc(size + 1u);
    if (!resolved) return NULL;
    writer = resolved;
    memcpy(writer, working_directory, strlen(working_directory));
    writer += strlen(working_directory);
    for (cursor = path_list; cursor && *cursor;) {
        const char *next = strchr(cursor, ';');
        const char *start = cursor;
        size_t length = next ? (size_t)(next - cursor) : strlen(cursor);
        if (length >= 2u && start[0] == '"' && start[length - 1u] == '"') {
            start++;
            length -= 2u;
        }
        if (length) {
            *writer++ = ';';
            if (!windows_path_is_absolute(start)) {
                size_t cwd_length = strlen(working_directory);
                memcpy(writer, working_directory, cwd_length);
                writer += cwd_length;
                if (writer[-1] != '\\' && writer[-1] != '/') *writer++ = '\\';
            }
            memcpy(writer, start, length);
            writer += length;
        }
        cursor = next ? next + 1 : NULL;
    }
    *writer = '\0';
    return resolved;
}

static char *windows_search_executable(const char *name,
                                       const char *working_directory) {
    static const wchar_t *extensions[] = {NULL, L".exe", L".cmd", L".bat", NULL};
    char *resolved_name;
    wchar_t *wide_name;
    wchar_t *wide_search_path;
    const char *search_path;
    char *result;
    int i;

    if (!name || !*name) return NULL;
    if ((strchr(name, '\\') || strchr(name, '/')) &&
        !windows_path_is_absolute(name)) {
        resolved_name = windows_join_path(working_directory, name);
    } else {
        resolved_name = copy_string(name);
    }
    if (!resolved_name) return NULL;
    wide_name = wide_from_utf8(resolved_name);
    rxspawn_memory_free(resolved_name);
    if (!wide_name) return NULL;
    search_path = rxcrexxcmd_active_getenv("PATH");
    {
        char *resolved_search_path = windows_resolve_search_path(
            search_path, working_directory);
        wide_search_path = resolved_search_path
            ? wide_from_utf8(resolved_search_path) : NULL;
        rxspawn_memory_free(resolved_search_path);
    }
    if ((search_path || working_directory) && !wide_search_path) {
        rxspawn_memory_free(wide_name);
        return NULL;
    }

    result = NULL;
    for (i = 0; extensions[i] || i == 0; i++) {
        DWORD needed;
        wchar_t *wide_path;
        DWORD copied;

        needed = SearchPathW(wide_search_path, wide_name, extensions[i],
                             0, NULL, NULL);
        if (needed == 0) {
            if (!extensions[i]) continue;
            continue;
        }

        wide_path = rxspawn_memory_alloc(
            ((size_t)needed + 1u) * sizeof(wchar_t));
        if (!wide_path) break;
        copied = SearchPathW(wide_search_path, wide_name, extensions[i],
                             needed + 1, wide_path, NULL);
        if (copied > 0 && copied <= needed) {
            DWORD attributes = GetFileAttributesW(wide_path);
            if (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
                result = utf8_from_wide(wide_path);
                rxspawn_memory_free(wide_path);
                break;
            }
        }
        rxspawn_memory_free(wide_path);

        if (!extensions[i]) continue;
    }

    rxspawn_memory_free(wide_name);
    rxspawn_memory_free(wide_search_path);
    return result;
}

static void windows_release_startup_attributes(STARTUPINFOEXW *startup) {
    if (!startup || !startup->lpAttributeList) return;
    DeleteProcThreadAttributeList(startup->lpAttributeList);
    rxspawn_memory_free(startup->lpAttributeList);
    startup->lpAttributeList = NULL;
}

static char *windows_resolve_application_path(const char *command,
                                              const char *working_directory) {
    char *token;
    char *path;

    token = windows_command_token(command);
    if (!token) return NULL;
    path = windows_search_executable(token, working_directory);
    rxspawn_memory_free(token);
    return path;
}

static int append_to_buffer(char **buffer, size_t *length, size_t *capacity, const char *text, size_t text_length) {
    char *new_buffer;
    size_t needed;
    size_t new_capacity;

    needed = *length + text_length + 1;
    if (needed > *capacity) {
        new_capacity = *capacity ? *capacity : 64;
        while (new_capacity < needed) new_capacity *= 2;
        new_buffer = rxspawn_memory_resize(*buffer, *length,
                                           new_capacity);
        if (!new_buffer) return -1;
        *buffer = new_buffer;
        *capacity = new_capacity;
    }

    if (text_length) memcpy(*buffer + *length, text, text_length);
    *length += text_length;
    (*buffer)[*length] = '\0';
    return 0;
}

static int append_char_to_buffer(char **buffer, size_t *length, size_t *capacity, char ch) {
    return append_to_buffer(buffer, length, capacity, &ch, 1);
}

static int windows_append_quoted_arg(char **buffer, size_t *length, size_t *capacity, const char *arg) {
    const char *cursor;
    int needs_quotes;
    size_t backslashes;

    if (!arg) arg = "";
    needs_quotes = *arg == '\0';
    for (cursor = arg; *cursor && !needs_quotes; cursor++) {
        if (isspace((unsigned char)*cursor) || *cursor == '"') needs_quotes = 1;
    }
    if (!needs_quotes) return append_to_buffer(buffer, length, capacity, arg, strlen(arg));

    if (append_char_to_buffer(buffer, length, capacity, '"') != 0) return -1;
    cursor = arg;
    backslashes = 0;
    while (*cursor) {
        if (*cursor == '\\') {
            backslashes++;
            cursor++;
            continue;
        }
        if (*cursor == '"') {
            while (backslashes > 0) {
                if (append_to_buffer(buffer, length, capacity, "\\\\", 2) != 0) return -1;
                backslashes--;
            }
            if (append_to_buffer(buffer, length, capacity, "\\\"", 2) != 0) return -1;
            cursor++;
            continue;
        }
        while (backslashes > 0) {
            if (append_char_to_buffer(buffer, length, capacity, '\\') != 0) return -1;
            backslashes--;
        }
        if (append_char_to_buffer(buffer, length, capacity, *cursor++) != 0) return -1;
    }
    while (backslashes > 0) {
        if (append_to_buffer(buffer, length, capacity, "\\\\", 2) != 0) return -1;
        backslashes--;
    }
    return append_char_to_buffer(buffer, length, capacity, '"');
}

static char *windows_build_command_line(const char *const *argv, int argc) {
    char *buffer = NULL;
    size_t length = 0;
    size_t capacity = 0;
    int i;

    for (i = 0; i < argc; i++) {
        if (i > 0 && append_char_to_buffer(&buffer, &length, &capacity, ' ') != 0) {
            rxspawn_memory_free(buffer);
            return NULL;
        }
        if (windows_append_quoted_arg(&buffer, &length, &capacity, argv[i]) != 0) {
            rxspawn_memory_free(buffer);
            return NULL;
        }
    }

    if (!buffer) buffer = copy_string("");
    return buffer;
}

static char *windows_normalize_executable_arg(const char *arg) {
    char *copy;
    char *cursor;

    copy = copy_string(arg ? arg : "");
    if (!copy) return NULL;
    for (cursor = copy; *cursor; cursor++) {
        if (*cursor == '/') *cursor = '\\';
    }
    return copy;
}
#endif

#ifndef _WIN32
static char *resolve_executable_path(const char *working_directory,
                                     const char *exe) {
    size_t size;
    char *path;
    if (!exe || !*exe) return NULL;
    if (exe[0] == '/' || !working_directory || !*working_directory) {
        return copy_string(exe);
    }
    size = strlen(working_directory) + strlen(exe) + 2u;
    path = rxspawn_memory_alloc(size);
    if (!path) return NULL;
    snprintf(path, size, "%s/%s", working_directory, exe);
    return path;
}

static char *find_executable_in_path_list(const char *path_list,
                                          const char *exe,
                                          const char *working_directory) {
    const char *cursor = path_list;

    if (!cursor || !*cursor || !exe || !*exe) return NULL;

    while (cursor) {
        const char *next_colon = strchr(cursor, ':');
        size_t dir_length = next_colon ? (size_t)(next_colon - cursor) : strlen(cursor);
        size_t directory_length;
        char *directory;
        char *candidate;

        if (dir_length) {
            directory = rxspawn_memory_alloc(dir_length + 1u);
            if (!directory) return NULL;
            memcpy(directory, cursor, dir_length);
            directory[dir_length] = '\0';
        } else {
            directory = copy_string(working_directory && *working_directory
                                    ? working_directory : ".");
            if (!directory) return NULL;
        }
        if (directory[0] != '/' && working_directory && *working_directory) {
            char *resolved = resolve_executable_path(working_directory,
                                                     directory);
            rxspawn_memory_free(directory);
            directory = resolved;
            if (!directory) return NULL;
        }
        directory_length = strlen(directory);
        candidate = rxspawn_memory_alloc(directory_length + strlen(exe) + 2u);
        if (!candidate) {
            rxspawn_memory_free(directory);
            return NULL;
        }
        sprintf(candidate, "%s/%s", directory, exe);
        rxspawn_memory_free(directory);

        if (ExeFound(candidate)) return candidate;
        rxspawn_memory_free(candidate);

        cursor = next_colon ? next_colon + 1 : NULL;
    }

    return NULL;
}

static char *find_standard_shell(void) {
    char *shell = NULL;

#ifdef _CS_PATH
    size_t standard_path_length = confstr(_CS_PATH, NULL, 0);
    if (standard_path_length > 0) {
        char *standard_path = rxspawn_memory_alloc(standard_path_length);
        if (standard_path) {
            confstr(_CS_PATH, standard_path, standard_path_length);
            shell = find_executable_in_path_list(standard_path, "sh", NULL);
            rxspawn_memory_free(standard_path);
            if (shell) return shell;
        }
    }
#endif

    if (ExeFound("/bin/sh")) return copy_string("/bin/sh");

    shell = find_executable_in_path_list(
        rxcrexxcmd_active_getenv("PATH"), "sh",
        rxcrexxcmd_active_working_directory());
    if (shell) return shell;

    return NULL;
}

static int split_shell_args(char *text, char **argv) {
    int count = 0;
    char *cursor = text;

    if (!text) return 0;
    if (!argv) {
        const char *reader = text;
        while (*reader) {
            char quote = 0;
            while (*reader && isspace((unsigned char)*reader)) reader++;
            if (!*reader) break;
            count++;
            while (*reader) {
                if (quote) {
                    if (*reader == quote) quote = 0;
                    reader++;
                    continue;
                }
                if (*reader == '"' || *reader == '\'') {
                    quote = *reader++;
                    continue;
                }
                if (isspace((unsigned char)*reader)) break;
                reader++;
            }
        }
        return count;
    }

    while (*cursor) {
        char quote = 0;
        while (*cursor && isspace((unsigned char)*cursor)) cursor++;
        if (!*cursor) break;

        if (argv) argv[count] = cursor;
        count++;

        while (*cursor) {
            if (quote) {
                if (*cursor == quote) {
                    memmove(cursor, cursor + 1, strlen(cursor));
                    quote = 0;
                    continue;
                }
                cursor++;
                continue;
            }
            if (*cursor == '"' || *cursor == '\'') {
                quote = *cursor;
                memmove(cursor, cursor + 1, strlen(cursor));
                continue;
            }
            if (isspace((unsigned char)*cursor)) {
                *cursor++ = '\0';
                break;
            }
            cursor++;
        }
    }

    return count;
}

static char *shell_argv_name(const char *shell_path) {
    char *slash;

    slash = strrchr(shell_path, '/');
    return slash ? slash + 1 : (char *)shell_path;
}

static char **build_shell_argv(const char *shell_path, const char *args_text, char *buffer, char *command_text) {
    int arg_count;
    char **argv;

    arg_count = split_shell_args(buffer, NULL);
    argv = rxspawn_memory_alloc(sizeof(char *) * (size_t)(arg_count + 3));
    if (!argv) return NULL;

    argv[0] = shell_argv_name(shell_path);
    if (arg_count) split_shell_args(buffer, argv + 1);
    argv[arg_count + 1] = command_text;
    argv[arg_count + 2] = NULL;
    (void)args_text;
    return argv;
}
#endif

static char *copy_value_string(value *string_value) {
    char *copy;

    if (!string_value || !string_value->string_value) {
        return copy_string_external("");
    }
    copy = malloc(string_value->string_length + 1);
    if (!copy) return NULL;
    if (string_value->string_length) {
        memcpy(copy, string_value->string_value, string_value->string_length);
    }
    copy[string_value->string_length] = '\0';
    return copy;
}

static int value_string_iequals(value *string_value, const char *text) {
    size_t i;
    size_t text_length;

    if (!string_value || !text) return 0;
    text_length = strlen(text);
    if (string_value->string_length != text_length) return 0;
    for (i = 0; i < text_length; i++) {
        if (tolower((unsigned char)string_value->string_value[i]) !=
            tolower((unsigned char)text[i])) return 0;
    }
    return 1;
}

static int value_string_to_size(value *string_value, size_t *out) {
    char *text;
    char *end;
    unsigned long parsed;

    if (out) *out = 0;
    text = copy_value_string(string_value);
    if (!text) return -1;
    errno = 0;
    parsed = strtoul(text, &end, 10);
    if (errno || !end || *end) {
        free(text);
        return -1;
    }
    if (out) *out = (size_t)parsed;
    free(text);
    return 0;
}

static int snapshot_string_iequals(const char *left, const char *right) {
    if (!left || !right) return 0;
    while (*left || *right) {
        if (tolower((unsigned char)*left) !=
            tolower((unsigned char)*right)) return 0;
        if (*left) left++;
        if (*right) right++;
    }
    return 1;
}

static int snapshot_string_to_size(const char *text, size_t *out) {
    char *end = NULL;
    unsigned long long parsed;
    if (out) *out = 0u;
    if (!text || !*text) return -1;
    errno = 0;
    parsed = strtoull(text, &end, 10);
    if (errno || !end || *end || parsed > SIZE_MAX) return -1;
    if (out) *out = (size_t)parsed;
    return 0;
}

static int snapshot_binding_get(const SHELLDATA *data,
                                const char *name,
                                char **out_value) {
    size_t index = 0u;
    if (out_value) *out_value = NULL;
    if (!data || !name) return 0;
    while (index < data->crexx_binding_snapshot_count) {
        const char *kind;
        const char *binding_name;
        const char *value;
        size_t count;
        if (index + 2u >= data->crexx_binding_snapshot_count) return 0;
        kind = data->crexx_binding_snapshot[index];
        binding_name = data->crexx_binding_snapshot[index + 1u];
        value = data->crexx_binding_snapshot[index + 2u];
        if (snapshot_string_iequals(kind, "VAR")) {
            if (snapshot_string_iequals(binding_name, name)) {
                if (out_value) {
                    *out_value = copy_string_external(value ? value : "");
                    if (!*out_value) return -1;
                }
                return 1;
            }
            index += 3u;
        } else if (snapshot_string_iequals(kind, "STEM")) {
            if (snapshot_string_to_size(value, &count) != 0 ||
                count > data->crexx_binding_snapshot_count - index - 3u) {
                return 0;
            }
            index += 3u + count;
        } else return 0;
    }
    return 0;
}

static int snapshot_stem_count(const SHELLDATA *data,
                               const char *name,
                               size_t *out_count) {
    size_t index = 0u;
    if (out_count) *out_count = 0u;
    if (!data || !name) return 0;
    while (index < data->crexx_binding_snapshot_count) {
        const char *kind;
        const char *binding_name;
        const char *value;
        size_t count;
        if (index + 2u >= data->crexx_binding_snapshot_count) return 0;
        kind = data->crexx_binding_snapshot[index];
        binding_name = data->crexx_binding_snapshot[index + 1u];
        value = data->crexx_binding_snapshot[index + 2u];
        if (snapshot_string_iequals(kind, "VAR")) index += 3u;
        else if (snapshot_string_iequals(kind, "STEM")) {
            if (snapshot_string_to_size(value, &count) != 0 ||
                count > data->crexx_binding_snapshot_count - index - 3u) {
                return 0;
            }
            if (snapshot_string_iequals(binding_name, name)) {
                if (out_count) *out_count = count;
                return 1;
            }
            index += 3u + count;
        } else return 0;
    }
    return 0;
}

static int snapshot_stem_value(const SHELLDATA *data,
                               const char *name,
                               size_t wanted,
                               char **out_value) {
    size_t index = 0u;
    if (out_value) *out_value = NULL;
    if (!data || !name || wanted == 0u) return 0;
    while (index < data->crexx_binding_snapshot_count) {
        const char *kind;
        const char *binding_name;
        const char *value;
        size_t count;
        if (index + 2u >= data->crexx_binding_snapshot_count) return 0;
        kind = data->crexx_binding_snapshot[index];
        binding_name = data->crexx_binding_snapshot[index + 1u];
        value = data->crexx_binding_snapshot[index + 2u];
        if (snapshot_string_iequals(kind, "VAR")) index += 3u;
        else if (snapshot_string_iequals(kind, "STEM")) {
            if (snapshot_string_to_size(value, &count) != 0 ||
                count > data->crexx_binding_snapshot_count - index - 3u) {
                return 0;
            }
            if (snapshot_string_iequals(binding_name, name)) {
                if (wanted > count) return 0;
                if (out_value) {
                    value = data->crexx_binding_snapshot[index + 2u + wanted];
                    *out_value = copy_string_external(value ? value : "");
                    if (!*out_value) return -1;
                }
                return 1;
            }
            index += 3u + count;
        } else return 0;
    }
    return 0;
}

static int crexxcmd_get_binding(void *userdata, const char *name, char **out_value) {
    SHELLDATA *data = (SHELLDATA *)userdata;
    value *bindings;
    size_t i;

    if (out_value) *out_value = NULL;
    if (!data || !name) return 0;
    if (data->crexx_binding_snapshot) {
        return snapshot_binding_get(data, name, out_value);
    }
    if (!data->crexx_bindings) return 0;

    bindings = data->crexx_bindings;
    i = 0;
    while (i < bindings->num_attributes) {
        if (i + 2 >= bindings->num_attributes) return 0;
        if (value_string_iequals(bindings->attributes[i], "VAR")) {
            if (value_string_iequals(bindings->attributes[i + 1], name)) {
                if (out_value) {
                    *out_value = copy_value_string(bindings->attributes[i + 2]);
                    if (!*out_value) return -1;
                }
                return 1;
            }
            i += 3;
        } else if (value_string_iequals(bindings->attributes[i], "STEM")) {
            size_t count = 0;
            if (value_string_to_size(bindings->attributes[i + 2], &count) != 0) return 0;
            i += 3 + count;
        } else {
            return 0;
        }
    }

    return 0;
}

static int crexxcmd_get_stem_count(void *userdata, const char *name, size_t *out_count) {
    SHELLDATA *data = (SHELLDATA *)userdata;
    value *bindings;
    size_t i;

    if (out_count) *out_count = 0;
    if (!data || !name) return 0;
    if (data->crexx_binding_snapshot) {
        return snapshot_stem_count(data, name, out_count);
    }
    if (!data->crexx_bindings) return 0;

    bindings = data->crexx_bindings;
    i = 0;
    while (i < bindings->num_attributes) {
        if (i + 2 >= bindings->num_attributes) return 0;
        if (value_string_iequals(bindings->attributes[i], "VAR")) {
            i += 3;
        } else if (value_string_iequals(bindings->attributes[i], "STEM")) {
            size_t count = 0;
            if (value_string_to_size(bindings->attributes[i + 2], &count) != 0) return 0;
            if (value_string_iequals(bindings->attributes[i + 1], name)) {
                if (out_count) *out_count = count;
                return 1;
            }
            i += 3 + count;
        } else {
            return 0;
        }
    }

    return 0;
}

static int crexxcmd_get_stem_value(void *userdata, const char *name, size_t index, char **out_value) {
    SHELLDATA *data = (SHELLDATA *)userdata;
    value *bindings;
    size_t i;

    if (out_value) *out_value = NULL;
    if (!data || !name || index == 0) return 0;
    if (data->crexx_binding_snapshot) {
        return snapshot_stem_value(data, name, index, out_value);
    }
    if (!data->crexx_bindings) return 0;

    bindings = data->crexx_bindings;
    i = 0;
    while (i < bindings->num_attributes) {
        if (i + 2 >= bindings->num_attributes) return 0;
        if (value_string_iequals(bindings->attributes[i], "VAR")) {
            i += 3;
        } else if (value_string_iequals(bindings->attributes[i], "STEM")) {
            size_t count = 0;
            if (value_string_to_size(bindings->attributes[i + 2], &count) != 0) return 0;
            if (value_string_iequals(bindings->attributes[i + 1], name)) {
                if (index > count || i + 2 + index >= bindings->num_attributes) return 0;
                if (out_value) {
                    *out_value = copy_value_string(bindings->attributes[i + 2 + index]);
                    if (!*out_value) return -1;
                }
                return 1;
            }
            i += 3 + count;
        } else {
            return 0;
        }
    }

    return 0;
}

static REDIRECT_COMPLETION *redirect_completion_create(unsigned char transfer_mode) {
    REDIRECT_COMPLETION *completion;

    completion = (REDIRECT_COMPLETION *)calloc(1, sizeof(*completion));
    if (!completion) return NULL;
    completion->io_handle = REDIRECT_INVALID_IO_HANDLE;
    completion->transfer_mode = transfer_mode;
    completion->terminal_state = REDIRECT_COMPLETION_CREATED;
    return completion;
}

static void redirect_completion_close_handle(REDIRECT_COMPLETION *completion) {
    if (!completion || completion->io_handle == REDIRECT_INVALID_IO_HANDLE) return;
#ifdef _WIN32
    CloseHandle(completion->io_handle);
#else
    close(completion->io_handle);
#endif
    completion->io_handle = REDIRECT_INVALID_IO_HANDLE;
}

static int redirect_completion_restrict_child_inheritance(
        REDIRECT_COMPLETION *completion) {
#ifdef _WIN32
    (void)completion;
    return 0; /* DuplicateHandle already made the completion end private. */
#else
    int flags;
    if (!completion || completion->io_handle == -1) return -1;
    flags = fcntl(completion->io_handle, F_GETFD);
    if (flags == -1) return -1;
    return fcntl(completion->io_handle, F_SETFD, flags | FD_CLOEXEC) == -1
        ? -1 : 0;
#endif
}

/* Build one child pipe and transfer the parent end to its C-only I/O owner.
 * The caller supplies either a byte-endpoint transfer or an immutable legacy
 * snapshot/capture completion. No VM value is reachable from the I/O thread. */
static int redirect_pipe_start(REDIRECT *redirect,
                               REDIRECT_COMPLETION *completion,
                               int output_from_child) {
    if (!redirect || !completion) return -1;
#ifdef _WIN32
    {
        SECURITY_ATTRIBUTES attributes;
        HANDLE read_handle = INVALID_HANDLE_VALUE;
        HANDLE write_handle = INVALID_HANDLE_VALUE;
        HANDLE parent_temporary;
        HANDLE child_handle;
        HANDLE parent_duplicate = INVALID_HANDLE_VALUE;
        DWORD thread_id;
        attributes.nLength = sizeof(attributes);
        attributes.lpSecurityDescriptor = NULL;
        attributes.bInheritHandle = TRUE;
        if (!CreatePipe(&read_handle, &write_handle, &attributes, 0)) {
            redirect->lastError = (int)GetLastError();
            goto failed;
        }
        child_handle = output_from_child ? write_handle : read_handle;
        parent_temporary = output_from_child ? read_handle : write_handle;
        if (!DuplicateHandle(GetCurrentProcess(), parent_temporary,
                             GetCurrentProcess(), &parent_duplicate,
                             0, FALSE, DUPLICATE_SAME_ACCESS)) {
            redirect->lastError = (int)GetLastError();
            goto failed;
        }
        if (!CloseHandle(parent_temporary)) {
            redirect->lastError = (int)GetLastError();
            goto failed;
        }
        if (output_from_child) {
            read_handle = INVALID_HANDLE_VALUE;
            redirect->hWrite = child_handle;
        } else {
            write_handle = INVALID_HANDLE_VALUE;
            redirect->hRead = child_handle;
        }
        completion->io_handle = parent_duplicate;
        parent_duplicate = INVALID_HANDLE_VALUE;
        completion->terminal_state = REDIRECT_COMPLETION_RUNNING;
        redirect->completion = completion;
        redirect->thread = CreateThread(
                NULL, 0,
                output_from_child ? OutputCaptureThread : InputSnapshotThread,
                completion, 0, &thread_id);
        if (!redirect->thread) {
            redirect->lastError = (int)GetLastError();
            goto failed;
        }
        redirect->has_thread = 1;
        return 0;

failed:
        redirect->errorCode = 1;
        redirect->errorSource = 5;
        if (read_handle != INVALID_HANDLE_VALUE) CloseHandle(read_handle);
        if (write_handle != INVALID_HANDLE_VALUE) CloseHandle(write_handle);
        if (parent_duplicate != INVALID_HANDLE_VALUE) {
            CloseHandle(parent_duplicate);
        }
        redirect->hRead = INVALID_HANDLE_VALUE;
        redirect->hWrite = INVALID_HANDLE_VALUE;
        redirect->completion = NULL;
        return -1;
    }
#else
    {
        int handles[2];
        int create_rc;
        if (pipe(handles) != 0) {
            redirect->errorCode = 1;
            redirect->lastError = errno;
            redirect->errorSource = 5;
            return -1;
        }
        if (output_from_child) {
            completion->io_handle = handles[0];
            redirect->hWrite = handles[1];
        } else {
            redirect->hRead = handles[0];
            completion->io_handle = handles[1];
        }
        if (redirect_completion_restrict_child_inheritance(completion) != 0) {
            redirect->errorCode = 1;
            redirect->lastError = errno;
            redirect->errorSource = 8;
            close(handles[0]);
            close(handles[1]);
            redirect->hRead = -1;
            redirect->hWrite = -1;
            completion->io_handle = -1;
            return -1;
        }
        completion->terminal_state = REDIRECT_COMPLETION_RUNNING;
        redirect->completion = completion;
        create_rc = pthread_create(
                &redirect->thread, NULL,
                output_from_child ? OutputCaptureThread : InputSnapshotThread,
                completion);
        if (create_rc != 0) {
            redirect->errorCode = 1;
            redirect->lastError = create_rc;
            redirect->errorSource = 5;
            if (redirect->hRead != -1) close(redirect->hRead);
            if (redirect->hWrite != -1) close(redirect->hWrite);
            redirect->hRead = -1;
            redirect->hWrite = -1;
            redirect_completion_close_handle(completion);
            redirect->completion = NULL;
            return -1;
        }
        redirect->has_thread = 1;
        return 0;
    }
#endif
}

static void redirect_completion_destroy(REDIRECT_COMPLETION *completion) {
    if (!completion) return;
    redirect_completion_close_handle(completion);
    rxvm_byte_endpoint_release(completion->endpoint);
    completion->endpoint = NULL;
    free(completion->bytes);
    completion->bytes = NULL;
    free(completion);
}

static int redirect_completion_append(REDIRECT_COMPLETION *completion,
                                      const char *bytes, size_t length) {
    size_t needed;
    size_t capacity;
    char *resized;

    if (!completion || (!bytes && length != 0)) return -1;
    if (length > (size_t)-1 - completion->length - 1u) return -1;
    needed = completion->length + length + 1u;
    if (needed > completion->capacity) {
        capacity = completion->capacity ? completion->capacity : 256u;
        while (capacity < needed) {
            if (capacity > (size_t)-1 / 2u) {
                capacity = needed;
                break;
            }
            capacity *= 2u;
        }
        resized = (char *)realloc(completion->bytes, capacity);
        if (!resized) return -1;
        completion->bytes = resized;
        completion->capacity = capacity;
    }
    if (length) memcpy(completion->bytes + completion->length, bytes, length);
    completion->length += length;
    completion->bytes[completion->length] = '\0';
    return 0;
}

static void redirect_completion_publish(REDIRECT_COMPLETION *completion,
                                        unsigned char terminal_state) {
    if (!completion) return;
    if (completion->terminal_state != REDIRECT_COMPLETION_RUNNING ||
        (terminal_state != REDIRECT_COMPLETION_SUCCEEDED &&
         terminal_state != REDIRECT_COMPLETION_FAILED)) {
        completion->errorCode = 1;
        completion->errorSource = 9;
        completion->terminal_state = REDIRECT_COMPLETION_FAILED;
        return;
    }
    completion->terminal_state = terminal_state;
}

static int redirect_completion_copy_to_receiver(REDIRECT *redirect,
                                                REDIRECT_COMPLETION *completion) {
    size_t start;
    size_t i;
    value *element;

    if (!redirect || !completion || completion->consumed) return 0;
    completion->consumed = 1;
    if (completion->transfer_mode == REDIRECT_TRANSFER_INPUT_STRING ||
        completion->transfer_mode == REDIRECT_TRANSFER_INPUT_ARRAY ||
        completion->transfer_mode == REDIRECT_TRANSFER_ENDPOINT_INPUT ||
        completion->transfer_mode == REDIRECT_TRANSFER_ENDPOINT_OUTPUT) {
        return 0;
    }
    if (!redirect->receiver ||
        rxvm_memory_current_worker() != redirect->receiver_worker) {
        completion->errorCode = 1;
        completion->errorSource = 10;
        return -1;
    }

    if (completion->transfer_mode == REDIRECT_TRANSFER_OUTPUT_STRING) {
        if (completion->length) {
            string_append_chars(redirect->receiver, completion->bytes,
                                completion->length);
        }
        return 0;
    }
    if (completion->transfer_mode != REDIRECT_TRANSFER_OUTPUT_ARRAY) return -1;

    start = 0;
    for (i = 0; i < completion->length; i++) {
        size_t end;
        if (completion->bytes[i] != '\n') continue;
        end = i;
#ifdef _WIN32
        if (end > start && completion->bytes[end - 1] == '\r') end--;
#endif
        element = add_new_element(redirect->receiver);
        if (!element) return -1;
        if (end > start) {
            string_append_chars(element, completion->bytes + start, end - start);
        }
        start = i + 1u;
    }
    if (start < completion->length) {
        element = add_new_element(redirect->receiver);
        if (!element) return -1;
        string_append_chars(element, completion->bytes + start,
                            completion->length - start);
    }
    return 0;
}

static void collect_redirect_thread_context(REDIRECT *redirect) {
    REDIRECT_COMPLETION *completion;

    if (!redirect || !redirect->completion) return;
    completion = redirect->completion;
    if (completion->terminal_state != REDIRECT_COMPLETION_SUCCEEDED &&
        completion->terminal_state != REDIRECT_COMPLETION_FAILED) {
        completion->errorCode = 1;
        completion->errorSource = 11;
    }
    if (redirect_completion_copy_to_receiver(redirect, completion) != 0 &&
        completion->errorCode == 0) {
        completion->errorCode = 1;
        completion->errorSource = 12;
    }
    if (completion->errorCode != 0) {
        redirect->errorCode = completion->errorCode;
        redirect->lastError = completion->lastError;
        redirect->errorSource = completion->errorSource;
    }
    redirect_completion_destroy(completion);
    redirect->completion = NULL;
}

/* Join is the publication/acquire boundary. Never consume or destroy a
 * completion unless the owning I/O thread has definitely stopped. */
static int join_redirect_thread(REDIRECT *redirect) {
    if (!redirect || !redirect->has_thread) return 0;

#ifdef _WIN32
    if (!redirect->thread ||
        WaitForSingleObject(redirect->thread, INFINITE) != WAIT_OBJECT_0) {
        redirect->errorCode = 1;
        redirect->lastError = (int)GetLastError();
        redirect->errorSource = 13;
        return -1;
    }
    CloseHandle(redirect->thread);
    redirect->thread = NULL;
#else
    {
        int join_rc = pthread_join(redirect->thread, NULL);
        if (join_rc != 0) {
            redirect->errorCode = 1;
            redirect->lastError = join_rc;
            redirect->errorSource = 13;
            return -1;
        }
    }
#endif

    redirect->has_thread = 0;
    collect_redirect_thread_context(redirect);
    return 0;
}

static int crexxcmd_write_redirect(REDIRECT *redirect, FILE *fallback, const char *text, size_t length) {
    if (!text) length = 0;
    if (!redirect) {
        if (fwrite(text ? text : "", 1, length, fallback) != length) return -1;
        return fflush(fallback) == 0 ? 0 : -1;
    }

#ifdef _WIN32
    {
        DWORD written;
        DWORD total = 0;
        if (redirect->hWrite == INVALID_HANDLE_VALUE) {
            return fwrite(text ? text : "", 1, length, fallback) == length ? 0 : -1;
        }
        while (total < length) {
            if (!WriteFile(redirect->hWrite, text + total, (DWORD)(length - total), &written, NULL)) {
                if (GetLastError() == ERROR_NO_DATA) return 0;
                redirect->errorCode = 1;
                redirect->lastError = (int)GetLastError();
                redirect->errorSource = 1;
                return -1;
            }
            total += written;
        }
    }
#else
    {
        size_t total = 0;
        ssize_t written;
        if (redirect->hWrite == -1) {
            return fwrite(text ? text : "", 1, length, fallback) == length ? 0 : -1;
        }
        while (total < length) {
            written = write(redirect->hWrite, text + total, length - total);
            if (written == -1) {
                if (errno == EPIPE) return 0;
                redirect->errorCode = 1;
                return -1;
            }
            total += (size_t)written;
        }
    }
#endif

    return 0;
}

static int crexxcmd_write_output(void *userdata, const char *text, size_t length) {
    SHELLDATA *data = (SHELLDATA *)userdata;
    return crexxcmd_write_redirect(data ? data->pOutput : NULL, stdout, text, length);
}

static int crexxcmd_write_error(void *userdata, const char *text, size_t length) {
    SHELLDATA *data = (SHELLDATA *)userdata;
    return crexxcmd_write_redirect(data ? data->pError : NULL, stderr, text, length);
}

static int append_read_buffer(char **out_text, size_t *out_length, const char *buffer, size_t length) {
    char *new_text;

    if (length > ((size_t)-1) - *out_length - 1) return -1;
    new_text = realloc(*out_text, *out_length + length + 1);
    if (!new_text) return -1;
    if (length) memcpy(new_text + *out_length, buffer, length);
    *out_length += length;
    new_text[*out_length] = '\0';
    *out_text = new_text;
    return 0;
}

static int crexxcmd_read_redirect(REDIRECT *redirect, char **out_text, size_t *out_length) {
    char buffer[4096];

    *out_text = malloc(1);
    if (!*out_text) return -1;
    (*out_text)[0] = '\0';
    *out_length = 0;
    if (!redirect) return 0;

#ifdef _WIN32
    {
        DWORD bytes_read;
        while (redirect->hRead != INVALID_HANDLE_VALUE) {
            if (!ReadFile(redirect->hRead, buffer, sizeof(buffer), &bytes_read, NULL)) {
                DWORD read_error = GetLastError();
                if (read_error == ERROR_BROKEN_PIPE || read_error == ERROR_HANDLE_EOF) break;
                redirect->errorCode = 1;
                redirect->lastError = (int)read_error;
                redirect->errorSource = 2;
                return -1;
            }
            if (bytes_read == 0) break;
            if (append_read_buffer(out_text, out_length, buffer, bytes_read) != 0) return -1;
        }
        if (redirect->hRead != INVALID_HANDLE_VALUE) {
            CloseHandle(redirect->hRead);
            redirect->hRead = INVALID_HANDLE_VALUE;
        }
        if (join_redirect_thread(redirect) != 0) return -1;
        if (redirect->hWrite != INVALID_HANDLE_VALUE) {
            CloseHandle(redirect->hWrite);
            redirect->hWrite = INVALID_HANDLE_VALUE;
        }
    }
#else
    {
        ssize_t bytes_read;
        while (redirect->hRead != -1) {
            bytes_read = read(redirect->hRead, buffer, sizeof(buffer));
            if (bytes_read == 0) break;
            if (bytes_read == -1) {
                redirect->errorCode = 1;
                return -1;
            }
            if (append_read_buffer(out_text, out_length, buffer, (size_t)bytes_read) != 0) return -1;
        }
        if (redirect->hRead != -1) {
            close(redirect->hRead);
            redirect->hRead = -1;
        }
        if (join_redirect_thread(redirect) != 0) return -1;
        if (redirect->hWrite != -1) {
            close(redirect->hWrite);
            redirect->hWrite = -1;
        }
    }
#endif

    return redirect->errorCode == 0 ? 0 : -1;
}

static int crexxcmd_read_input(void *userdata, char **out_text, size_t *out_length) {
    SHELLDATA *data = (SHELLDATA *)userdata;
    return crexxcmd_read_redirect(data ? data->pInput : NULL, out_text, out_length);
}

static int crexxcmd_close_output_redirect(REDIRECT *redirect) {
    if (!redirect) return 0;

#ifdef _WIN32
    if (redirect->hWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(redirect->hWrite);
        redirect->hWrite = INVALID_HANDLE_VALUE;
    }
    if (join_redirect_thread(redirect) != 0) return -1;
    if (redirect->hRead != INVALID_HANDLE_VALUE) {
        CloseHandle(redirect->hRead);
        redirect->hRead = INVALID_HANDLE_VALUE;
    }
#else
    if (redirect->hWrite != -1) {
        close(redirect->hWrite);
        redirect->hWrite = -1;
    }
    if (join_redirect_thread(redirect) != 0) return -1;
    if (redirect->hRead != -1) {
        close(redirect->hRead);
        redirect->hRead = -1;
    }
#endif

    return redirect->errorCode == 0 ? 0 : -1;
}

static int crexxcmd_close_input_redirect(REDIRECT *redirect) {
    if (!redirect) return 0;

#ifdef _WIN32
    if (redirect->hRead != INVALID_HANDLE_VALUE) {
        CloseHandle(redirect->hRead);
        redirect->hRead = INVALID_HANDLE_VALUE;
    }
    if (join_redirect_thread(redirect) != 0) return -1;
    if (redirect->hWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(redirect->hWrite);
        redirect->hWrite = INVALID_HANDLE_VALUE;
    }
#else
    if (redirect->hRead != -1) {
        close(redirect->hRead);
        redirect->hRead = -1;
    }
    if (join_redirect_thread(redirect) != 0) return -1;
    if (redirect->hWrite != -1) {
        close(redirect->hWrite);
        redirect->hWrite = -1;
    }
#endif

    return redirect->errorCode == 0 ? 0 : -1;
}

static void redirect_endpoint_init(REDIRECT *redirect, char endpoint_kind) {
    memset(redirect, 0, sizeof(*redirect));
    redirect->receiver_worker = rxvm_memory_current_worker();
#ifdef _WIN32
    redirect->hRead = INVALID_HANDLE_VALUE;
    redirect->hWrite = INVALID_HANDLE_VALUE;
    redirect->thread = NULL;
#else
    redirect->hRead = -1;
    redirect->hWrite = -1;
#endif
    redirect->completion = NULL;
    redirect->endpoint_kind = endpoint_kind;
}

static void redirect_endpoint_cell_release(REDIRECT_ENDPOINT_CELL *cell) {
    REDIRECT *redirect;
    int close_rc;

    if (!cell) return;
    cell->refcount--;
    if (cell->refcount > 0) return;

    redirect = &cell->redirect;
    if (redirect->endpoint_kind == REDIRECT_ENDPOINT_INPUT) {
        close_rc = crexxcmd_close_input_redirect(redirect);
    } else {
        close_rc = crexxcmd_close_output_redirect(redirect);
    }
    /* A failed join cannot justify freeing storage a live thread may own. The
     * worker teardown assertion will make this exceptional leak visible. */
    if (close_rc != 0 && redirect->has_thread) return;
    rxspawn_memory_free(cell);
}

static REDIRECT_ENDPOINT_PAYLOAD *redirect_endpoint_payload_from_value(value *payload_value) {
    size_t payload_length = 0;
    const rxvm_native_payload_ops *ops = NULL;
    REDIRECT_ENDPOINT_PAYLOAD *payload;

    payload = (REDIRECT_ENDPOINT_PAYLOAD *)get_native_payload(payload_value, &payload_length, &ops, NULL);
    if (ops != &redirect_endpoint_payload_ops ||
        payload_length != sizeof(REDIRECT_ENDPOINT_PAYLOAD)) return NULL;
    return payload;
}

REDIRECT *rxspawn_redirect_from_value(value *redirect_reg) {
    REDIRECT_ENDPOINT_PAYLOAD *payload;

    if (!redirect_reg ||
        rxvm_value_native_ops(redirect_reg) != &redirect_endpoint_payload_ops)
        return NULL;
    payload = redirect_endpoint_payload_from_value(redirect_reg);
    return payload && payload->cell ? &payload->cell->redirect : NULL;
}

static REDIRECT *redirect_endpoint_create(value *redirect_reg, char endpoint_kind) {
    REDIRECT_ENDPOINT_CELL *cell;
    REDIRECT_ENDPOINT_PAYLOAD payload;

    if (!redirect_reg) return NULL;

    value_zero(redirect_reg);
    cell = rxspawn_memory_alloc(sizeof(*cell));
    if (!cell) return NULL;
    cell->refcount = 1;
    redirect_endpoint_init(&cell->redirect, endpoint_kind);

    payload.cell = cell;
    if (set_native_payload(redirect_reg, &payload, sizeof(payload),
                           &redirect_endpoint_payload_ops, 0) != 0) {
        rxspawn_memory_free(cell);
        return NULL;
    }
    return &cell->redirect;
}

static void redirect_endpoint_payload_copy(void *dest_value, void *source_value) {
    REDIRECT_ENDPOINT_PAYLOAD *source_payload;
    REDIRECT_ENDPOINT_PAYLOAD dest_payload;

    source_payload = redirect_endpoint_payload_from_value((value *)source_value);
    dest_payload.cell = source_payload ? source_payload->cell : NULL;
    if (dest_payload.cell) dest_payload.cell->refcount++;
    if (set_native_payload((value *)dest_value, &dest_payload, sizeof(dest_payload),
                           &redirect_endpoint_payload_ops, 0) != 0) {
        if (dest_payload.cell) dest_payload.cell->refcount--;
        abort();
    }
}

static void redirect_endpoint_payload_finalize(void *payload_value) {
    REDIRECT_ENDPOINT_PAYLOAD *payload;

    payload = redirect_endpoint_payload_from_value((value *)payload_value);
    if (!payload) return;
    redirect_endpoint_cell_release(payload->cell);
    payload->cell = NULL;
}

static int crexxcmd_finalize_redirects(SHELLDATA *data, char **errorText) {
    int rc = 0;
    char details[192];

    if (!data) return 0;
    if (crexxcmd_close_input_redirect(data->pInput) != 0) rc = -1;
    if (crexxcmd_close_output_redirect(data->pOutput) != 0) rc = -1;
    if (crexxcmd_close_output_redirect(data->pError) != 0) rc = -1;
    if (rc != 0) {
        snprintf(details, sizeof(details),
                 "CREXX command redirect failure input=%d/%d/%d output=%d/%d/%d error=%d/%d/%d",
                 data->pInput ? data->pInput->errorCode : 0,
                 data->pInput ? data->pInput->lastError : 0,
                 data->pInput ? data->pInput->errorSource : 0,
                 data->pOutput ? data->pOutput->errorCode : 0,
                 data->pOutput ? data->pOutput->lastError : 0,
                 data->pOutput ? data->pOutput->errorSource : 0,
                 data->pError ? data->pError->errorCode : 0,
                 data->pError ? data->pError->lastError : 0,
                 data->pError ? data->pError->errorSource : 0);
        appendTextOutput(errorText, details);
    }
    return rc;
}

static int crexxcmd_run_path(void *userdata,
                             const char *command,
                             char **out_text,
                             char **err_text,
                             int *command_rc,
                             char **error_text) {
    SHELLDATA *parent_data = (SHELLDATA *)userdata;
    value output_redirect;
    value error_redirect;
    value output_value;
    value error_value;
    REDIRECT *pIn;
    REDIRECT *pOut;
    REDIRECT *pErr;
    int spawn_rc;
    char *spawn_error;
    RXSPAWN_SNAPSHOT_OVERRIDE nested_snapshot;
    const RXSPAWN_SNAPSHOT_OVERRIDE *previous_snapshot;
    int capture_output;
    int capture_error;
    int termination_reason = 0;

    if (out_text) *out_text = NULL;
    if (err_text) *err_text = NULL;
    if (error_text) *error_text = NULL;
    if (command_rc) *command_rc = 0;

    value_init(&output_redirect);
    value_init(&error_redirect);
    value_init(&output_value);
    value_init(&error_value);

    /* A missing redirect means the normal process stream. Preserve NULL for
     * each absent stream so nested PATH execution inherits stdin and emits
     * stdout/stderr immediately instead of adding a hidden capture layer. */
    pIn = parent_data ? parent_data->pInput : NULL;
    capture_output = parent_data && parent_data->pOutput;
    capture_error = parent_data && parent_data->pError;
    if (capture_output) redr2str(&output_redirect, &output_value);
    if (capture_error) redr2str(&error_redirect, &error_value);
    pOut = capture_output
            ? rxspawn_redirect_from_value(&output_redirect) : NULL;
    pErr = capture_error
            ? rxspawn_redirect_from_value(&error_redirect) : NULL;
    spawn_error = NULL;
    previous_snapshot = crexxcmd_enter_parent_snapshot(
            parent_data, &nested_snapshot, &termination_reason);
    spawn_rc = shellspawn(command, pIn, pOut, pErr,
                          parent_data ? parent_data->variables : NULL,
                          NULL,
                          SHELLSPAWN_MODE_PATH,
                          command_rc,
                          &spawn_error);
    crexxcmd_leave_parent_snapshot(
            parent_data, previous_snapshot, termination_reason);

    if (out_text && capture_output) *out_text = copy_value_string(&output_value);
    if (err_text && capture_error) *err_text = copy_value_string(&error_value);
    if (error_text && spawn_error) *error_text = copy_string_external(spawn_error);

    clear_value(&output_redirect);
    clear_value(&error_redirect);
    clear_value(&output_value);
    clear_value(&error_value);

    if (spawn_error) free(spawn_error);
    if (spawn_rc == SHELLSPAWN_NOFOUND) {
        if (command_rc) *command_rc = 127;
        return 0;
    }
    return spawn_rc == SHELLSPAWN_OK ? 0 : -1;
}

static int crexxcmd_run_argv(void *userdata,
                             int argc,
                             const char *const *argv,
                             char **out_text,
                             char **err_text,
                             int *command_rc,
                             char **error_text) {
    SHELLDATA *parent_data = (SHELLDATA *)userdata;
    value output_redirect;
    value error_redirect;
    value output_value;
    value error_value;
    REDIRECT *pIn;
    REDIRECT *pOut;
    REDIRECT *pErr;
    int spawn_rc;
    char *spawn_error;
    RXSPAWN_SNAPSHOT_OVERRIDE nested_snapshot;
    const RXSPAWN_SNAPSHOT_OVERRIDE *previous_snapshot;
    int capture_output;
    int capture_error;
    int termination_reason = 0;

    if (out_text) *out_text = NULL;
    if (err_text) *err_text = NULL;
    if (error_text) *error_text = NULL;
    if (command_rc) *command_rc = 0;

    value_init(&output_redirect);
    value_init(&error_redirect);
    value_init(&output_value);
    value_init(&error_value);

    /* A missing redirect means the normal process stream. Preserve NULL for
     * each absent stream so argv-preserving CREXX run inherits stdin and
     * emits stdout/stderr immediately instead of adding a hidden capture. */
    pIn = parent_data ? parent_data->pInput : NULL;
    capture_output = parent_data && parent_data->pOutput;
    capture_error = parent_data && parent_data->pError;
    if (capture_output) redr2str(&output_redirect, &output_value);
    if (capture_error) redr2str(&error_redirect, &error_value);
    pOut = capture_output
            ? rxspawn_redirect_from_value(&output_redirect) : NULL;
    pErr = capture_error
            ? rxspawn_redirect_from_value(&error_redirect) : NULL;
    spawn_error = NULL;
    previous_snapshot = crexxcmd_enter_parent_snapshot(
            parent_data, &nested_snapshot, &termination_reason);
    spawn_rc = spawn_argv_capture(argv,
                                  argc,
                                  pIn,
                                  pOut,
                                  pErr,
                                  parent_data ? parent_data->variables : NULL,
                                  command_rc,
                                  &spawn_error);
    crexxcmd_leave_parent_snapshot(
            parent_data, previous_snapshot, termination_reason);

    if (out_text && capture_output) *out_text = copy_value_string(&output_value);
    if (err_text && capture_error) *err_text = copy_value_string(&error_value);
    if (error_text && spawn_error) *error_text = copy_string_external(spawn_error);

    clear_value(&output_redirect);
    clear_value(&error_redirect);
    clear_value(&output_value);
    clear_value(&error_value);

    if (spawn_error) free(spawn_error);
    if (spawn_rc == SHELLSPAWN_NOFOUND) {
        if (command_rc) *command_rc = 127;
        return 0;
    }
    return spawn_rc == SHELLSPAWN_OK ? 0 : -1;
}

/* Get Environment Value
 * Sets value (null terminated) (and a handle) from env variable name length name_length (not null terminated)
 * Value can be set to point to a zero length string (if the variable is not set)
 *
 * Returns 1 if value should bee free()d
 * Otherwise returns 0
 */
int getEnvVal(char **value, char *name, size_t name_length) {

    char* nulled_name;
    if (!name_length) {
        *value = "";
        return 0;
    }
    nulled_name = malloc(name_length + 1);
    memcpy(nulled_name, name, name_length);
    nulled_name[name_length] = 0;

#ifdef _WIN32

    wchar_t *wname;
    int wname_length = MultiByteToWideChar(CP_UTF8, 0, nulled_name, -1, NULL, 0);
    wname = (wchar_t *)malloc(wname_length * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, nulled_name, -1, wname, wname_length);

    DWORD len = GetEnvironmentVariableW(wname, NULL, 0);
    if (len > 0) {
        wchar_t *wvalue = (wchar_t *)malloc(len * sizeof(wchar_t));
        GetEnvironmentVariableW(wname, wvalue, len);

        int utf8_length = WideCharToMultiByte(CP_UTF8, 0, wvalue, len, NULL, 0, NULL, NULL);
        *value = malloc(utf8_length + 1);
        WideCharToMultiByte(CP_UTF8, 0, wvalue, len, *value, utf8_length, NULL, NULL);
        (*value)[utf8_length] = '\0';

        free(wvalue);
    }
    else {
        *value = "";
    }
    free(wname);
    free(nulled_name);
    return len > 0 ? 1 : 0;

#else

    *value = getenv(nulled_name);
    free(nulled_name);
    if (!(*value)) {
        *value = "";
    }
    return 0;

#endif
}

/*
 * - A pin, pout or perr does not need to be specified ... in this case the std streams are used.
 * - Command contains the commands string to execute
 * - rc will contain the return code from the command
 * - errorText contains a descriptive text of any error in the spawn
 *   (i.e. NOT from the executed child process). This is set if this returns
 *   a non-zero return code.
 *
 * Return codes
 *  0 - SHELLSPAWN_OK         - All OK
 *  4 - SHELLSPAWN_NOFOUND    - The command was not found
 *  5 - SHELLSPAWN_FAILURE    - Spawn failed unexpectedly (see error text for details)
*/
int shellspawn (const char *command,
                REDIRECT* pIn,
                REDIRECT* pOut,
                REDIRECT* pErr,
                value* variables,
                value* crexx_bindings,
                int mode,
                int *rc,
                char **errorText) {

// Create data structure - and make sure we make all the members empty
    SHELLDATA data;
    data.waitThreadErrorText = 0;
#ifdef _WIN32
    ZeroMemory(&data.ChildProcessInfo, sizeof(PROCESS_INFORMATION));
    data.ChildJob = NULL;
#else
    data.ChildProcessPID = 0;
#endif
    data.ChildProcessRC = 0;
    data.pInput = pIn;
    data.pOutput = pOut;
    data.pError = pErr;
    data.buffer = 0;
    data.file_path = 0;
    data.application_path = 0;
    data.argv = 0;
    data.working_directory = 0;
    data.environment = 0;
    data.waitThreadRC = 0;
    data.variables = variables;
    data.crexx_bindings = crexx_bindings;
    data.crexx_binding_snapshot = rxspawn_thread_snapshot
            ? rxspawn_thread_snapshot->crexx_bindings : NULL;
    data.crexx_binding_snapshot_count = rxspawn_thread_snapshot
            ? rxspawn_thread_snapshot->crexx_binding_count : 0u;
    data.cancelled = rxspawn_thread_snapshot
            ? rxspawn_thread_snapshot->cancelled : NULL;
    data.input_stopped = rxspawn_thread_snapshot
            ? rxspawn_thread_snapshot->input_stopped : NULL;
    data.output_stopped = rxspawn_thread_snapshot
            ? rxspawn_thread_snapshot->output_stopped : NULL;
    data.deadline_microseconds = rxspawn_thread_snapshot
            ? rxspawn_thread_snapshot->deadline_microseconds : 0u;
    data.terminated = 0u;
    data.timed_out = 0u;

    if ((rxspawn_thread_snapshot
             ? rxspawn_copy_process_snapshot(
                   rxspawn_thread_snapshot, &data.working_directory,
                   &data.environment)
             : rxcrexxcmd_active_process_snapshot(
                   &data.working_directory, &data.environment)) != 0) {
        Error("Failure spawn environment snapshot", errorText);
        CleanUp(&data);
        return SHELLSPAWN_FAILURE;
    }

    if (mode == SHELLSPAWN_MODE_CREXX) {
        rxcrexxcmd_io io;
        int execute_rc;

        io.write_output = crexxcmd_write_output;
        io.write_error = crexxcmd_write_error;
        io.read_input = crexxcmd_read_input;
        io.run_path = crexxcmd_run_path;
        io.run_argv = crexxcmd_run_argv;
        io.get_binding = crexxcmd_get_binding;
        io.get_stem_count = crexxcmd_get_stem_count;
        io.get_stem_value = crexxcmd_get_stem_value;
        io.userdata = &data;

        execute_rc = rxcrexxcmd_execute(command, &io, rc, errorText);
        if (crexxcmd_finalize_redirects(&data, errorText) != 0) {
            CleanUp(&data);
            return SHELLSPAWN_FAILURE;
        }
        CleanUp(&data);
        return execute_rc == 0 ? SHELLSPAWN_OK : SHELLSPAWN_FAILURE;
    }

#ifdef _WIN32
/* Windows does the actual parsing and validating as part of CreateProcess() */
    if (mode == SHELLSPAWN_MODE_SHELL || mode == SHELLSPAWN_MODE_CONFIGURED_SHELL) {
        const char *shell = mode == SHELLSPAWN_MODE_CONFIGURED_SHELL ? rxcrexxcmd_active_getenv("CREXX_ADDRESS_SHELL") : NULL;
        const char *shell_args = mode == SHELLSPAWN_MODE_CONFIGURED_SHELL ? rxcrexxcmd_active_getenv("CREXX_ADDRESS_SHELL_ARGS") : NULL;
        if (!shell || !*shell) shell = rxcrexxcmd_active_getenv("COMSPEC");
        if (!shell || !*shell) shell = "cmd.exe";
        if (!shell_args || !*shell_args) shell_args = "/D /S /C";

        data.file_path = rxspawn_memory_alloc(
            strlen(shell) + strlen(shell_args) + strlen(command) + 8u);
        if (!data.file_path) {
            Error("Failure spawn W01", errorText);
            CleanUp(&data);
            return SHELLSPAWN_FAILURE;
        }
        sprintf(data.file_path, "\"%s\" %s %s", shell, shell_args, command);
    } else {
        data.file_path = copy_string(command);
        if (!data.file_path) {
            Error("Failure spawn W02", errorText);
            CleanUp(&data);
            return SHELLSPAWN_FAILURE;
        }
        data.application_path = windows_resolve_application_path(
            command, data.working_directory);
        if (!data.application_path) {
            Error("Command not found", errorText);
            CleanUp(&data);
            return SHELLSPAWN_NOFOUND;
        }
    }
#else
    if (mode == SHELLSPAWN_MODE_SHELL || mode == SHELLSPAWN_MODE_CONFIGURED_SHELL) {
        const char *configured_shell = mode == SHELLSPAWN_MODE_CONFIGURED_SHELL ? rxcrexxcmd_active_getenv("CREXX_ADDRESS_SHELL") : NULL;
        const char *shell_args = mode == SHELLSPAWN_MODE_CONFIGURED_SHELL ? rxcrexxcmd_active_getenv("CREXX_ADDRESS_SHELL_ARGS") : NULL;
        size_t args_length;
        size_t command_length;
        char *command_buffer;

        if (configured_shell && *configured_shell) data.file_path = copy_string(configured_shell);
        else data.file_path = find_standard_shell();
        if (!data.file_path) {
            Error("Command shell not found", errorText);
            CleanUp(&data);
            return SHELLSPAWN_NOFOUND;
        }
        if (!shell_args || !*shell_args) shell_args = "-c";

        args_length = strlen(shell_args);
        command_length = strlen(command);
        data.buffer = rxspawn_memory_alloc(
            args_length + 1u + command_length + 1u);
        if (!data.buffer) {
            Error("Failure spawn U19", errorText);
            CleanUp(&data);
            return SHELLSPAWN_FAILURE;
        }
        memcpy(data.buffer, shell_args, args_length + 1);
        command_buffer = data.buffer + args_length + 1;
        memcpy(command_buffer, command, command_length + 1);
        data.argv = build_shell_argv(data.file_path, shell_args, data.buffer, command_buffer);
        if (!data.argv) {
            Error("Failure spawn U20", errorText);
            CleanUp(&data);
            return SHELLSPAWN_FAILURE;
        }
    } else {
        // Parse the command
        char *base_name;
        if (ParseCommand(command, &data.buffer, &base_name, &data.argv)) {
            Error("Failure spawn U18", errorText);
            CleanUp(&data);
            return SHELLSPAWN_NOFOUND;
        }

        int commandFound = 0;
        {
            char *resolved_base = resolve_executable_path(
                data.working_directory, base_name);
            if (resolved_base && ExeFound(resolved_base)) {
                data.file_path = resolved_base;
                resolved_base = NULL;
                commandFound = 1;
            }
            rxspawn_memory_free(resolved_base);
        }
        if (!commandFound && base_name[0] != '/') {
            data.file_path = find_executable_in_path_list(
                rxcrexxcmd_active_getenv("PATH"), base_name,
                data.working_directory);
            if (data.file_path) commandFound = 1;
        }

        if (!commandFound) {
            Error("Command not found", errorText);
            CleanUp(&data);
            return SHELLSPAWN_NOFOUND;
        }
    }
#endif

    /* Launch the command */
    int lrc;
    lrc = launchChild(&data);
    if (lrc) {
        CleanUp(&data);
        return lrc;
    }

    /* Wait fot it to complete */
    WaitForProcess(&data);
    if (rxspawn_thread_snapshot &&
        rxspawn_thread_snapshot->termination_reason) {
        *rxspawn_thread_snapshot->termination_reason = data.terminated
                ? (data.timed_out ? 2 : 1) : 0;
    }

    // Handle any waitThread errors
    if (data.waitThreadRC) {
        appendTextOutput(errorText,data.waitThreadErrorText);
        CleanUp(&data);
        return SHELLSPAWN_FAILURE;
    }

    *rc = (int) data.ChildProcessRC;

    CleanUp(&data);

    return SHELLSPAWN_OK;
}

static int spawn_argv_capture(const char *const *argv,
                              int argc,
                              REDIRECT* pIn,
                              REDIRECT* pOut,
                              REDIRECT* pErr,
                              value* variables,
                              int *rc,
                              char **errorText) {
    SHELLDATA data;
    int lrc;

    if (rc) *rc = 0;
    if (!argv || argc < 1 || !argv[0] || !*argv[0]) {
        Error("Command not found", errorText);
        return SHELLSPAWN_NOFOUND;
    }

    data.waitThreadErrorText = 0;
#ifdef _WIN32
    ZeroMemory(&data.ChildProcessInfo, sizeof(PROCESS_INFORMATION));
    data.ChildJob = NULL;
#else
    data.ChildProcessPID = 0;
#endif
    data.ChildProcessRC = 0;
    data.pInput = pIn;
    data.pOutput = pOut;
    data.pError = pErr;
    data.buffer = 0;
    data.file_path = 0;
    data.application_path = 0;
    data.argv = 0;
    data.working_directory = 0;
    data.environment = 0;
    data.waitThreadRC = 0;
    data.variables = variables;
    data.crexx_bindings = NULL;
    data.crexx_binding_snapshot = NULL;
    data.crexx_binding_snapshot_count = 0u;
    data.cancelled = rxspawn_thread_snapshot
            ? rxspawn_thread_snapshot->cancelled : NULL;
    data.input_stopped = rxspawn_thread_snapshot
            ? rxspawn_thread_snapshot->input_stopped : NULL;
    data.output_stopped = rxspawn_thread_snapshot
            ? rxspawn_thread_snapshot->output_stopped : NULL;
    data.deadline_microseconds = rxspawn_thread_snapshot
            ? rxspawn_thread_snapshot->deadline_microseconds : 0u;
    data.terminated = 0u;
    data.timed_out = 0u;

    if ((rxspawn_thread_snapshot
             ? rxspawn_copy_process_snapshot(
                   rxspawn_thread_snapshot, &data.working_directory,
                   &data.environment)
             : rxcrexxcmd_active_process_snapshot(
                   &data.working_directory, &data.environment)) != 0) {
        Error("Failure spawn environment snapshot", errorText);
        CleanUp(&data);
        return SHELLSPAWN_FAILURE;
    }

#ifdef _WIN32
    {
        char *normalized_exe;

        data.file_path = windows_build_command_line(argv, argc);
        if (!data.file_path) {
            Error("Failure spawn W03", errorText);
            CleanUp(&data);
            return SHELLSPAWN_FAILURE;
        }

        normalized_exe = windows_normalize_executable_arg(argv[0]);
        if (!normalized_exe) {
            Error("Failure spawn W04", errorText);
            CleanUp(&data);
            return SHELLSPAWN_FAILURE;
        }
        data.application_path = windows_search_executable(
            normalized_exe, data.working_directory);
        rxspawn_memory_free(normalized_exe);
        if (!data.application_path) {
            Error("Command not found", errorText);
            CleanUp(&data);
            return SHELLSPAWN_NOFOUND;
        }
    }
#else
    {
        int i;

        data.argv = rxspawn_memory_alloc(
            sizeof(char *) * (size_t)(argc + 1));
        if (!data.argv) {
            Error("Failure spawn U21", errorText);
            CleanUp(&data);
            return SHELLSPAWN_FAILURE;
        }
        for (i = 0; i < argc; i++) data.argv[i] = (char *)argv[i];
        data.argv[argc] = NULL;

        if (strchr(argv[0], '/')) {
            char *resolved = resolve_executable_path(data.working_directory,
                                                     argv[0]);
            if (resolved && ExeFound(resolved)) data.file_path = resolved;
            else rxspawn_memory_free(resolved);
        } else {
            data.file_path = find_executable_in_path_list(
                rxcrexxcmd_active_getenv("PATH"), argv[0],
                data.working_directory);
        }

        if (!data.file_path) {
            Error("Command not found", errorText);
            CleanUp(&data);
            return SHELLSPAWN_NOFOUND;
        }
    }
#endif

    lrc = launchChild(&data);
    if (lrc) {
        CleanUp(&data);
        return lrc;
    }

    WaitForProcess(&data);
    if (rxspawn_thread_snapshot &&
        rxspawn_thread_snapshot->termination_reason) {
        *rxspawn_thread_snapshot->termination_reason = data.terminated
                ? (data.timed_out ? 2 : 1) : 0;
    }

    if (data.waitThreadRC) {
        appendTextOutput(errorText, data.waitThreadErrorText);
        CleanUp(&data);
        return SHELLSPAWN_FAILURE;
    }

    if (rc) *rc = (int)data.ChildProcessRC;
    CleanUp(&data);
    return SHELLSPAWN_OK;
}

static uint64_t rxspawn_deadline_from_wait(int64_t wait_microseconds) {
    uint64_t now;
    uint64_t duration;
    if (wait_microseconds <= 0) return 0u;
    now = rxspawn_monotonic_microseconds();
    duration = (uint64_t)wait_microseconds;
    return duration > UINT64_MAX - now ? UINT64_MAX : now + duration;
}

int shellspawn_snapshot(const char *command,
                        REDIRECT *pIn,
                        REDIRECT *pOut,
                        REDIRECT *pErr,
                        const char *working_directory,
                        const char *const *environment,
                        int mode,
                        int64_t wait_microseconds,
                        const atomic_uchar *cancelled,
                        atomic_uchar *input_stopped,
                        atomic_uchar *output_stopped,
                        int *termination_reason,
                        int *rc,
                        char **errorText) {
    return shellspawn_snapshot_bindings(
            command, pIn, pOut, pErr, working_directory, environment,
            NULL, 0u, mode, wait_microseconds, cancelled,
            input_stopped, output_stopped,
            termination_reason, rc, errorText);
}

int shellspawn_snapshot_bindings(const char *command,
                                 REDIRECT *pIn,
                                 REDIRECT *pOut,
                                 REDIRECT *pErr,
                                 const char *working_directory,
                                 const char *const *environment,
                                 const char *const *crexx_bindings,
                                 size_t crexx_binding_count,
                                 int mode,
                                 int64_t wait_microseconds,
                                 const atomic_uchar *cancelled,
                                 atomic_uchar *input_stopped,
                                 atomic_uchar *output_stopped,
                                 int *termination_reason,
                                 int *rc,
                                 char **errorText) {
    RXSPAWN_SNAPSHOT_OVERRIDE snapshot;
    const RXSPAWN_SNAPSHOT_OVERRIDE *previous = rxspawn_thread_snapshot;
    int result;
    snapshot.working_directory = working_directory;
    snapshot.environment = environment;
    snapshot.crexx_bindings = crexx_bindings;
    snapshot.crexx_binding_count = crexx_binding_count;
    snapshot.cancelled = cancelled;
    snapshot.input_stopped = input_stopped;
    snapshot.output_stopped = output_stopped;
    snapshot.deadline_microseconds =
            rxspawn_deadline_from_wait(wait_microseconds);
    snapshot.termination_reason = termination_reason;
    if (termination_reason) *termination_reason = 0;
    rxspawn_thread_snapshot = &snapshot;
    result = shellspawn(command, pIn, pOut, pErr, NULL, NULL,
                        mode, rc, errorText);
    rxspawn_thread_snapshot = previous;
    return result;
}

int shellspawn_argv_snapshot(const char *const *argv,
                             int argc,
                             REDIRECT *pIn,
                             REDIRECT *pOut,
                             REDIRECT *pErr,
                             const char *working_directory,
                             const char *const *environment,
                             int64_t wait_microseconds,
                             const atomic_uchar *cancelled,
                             atomic_uchar *input_stopped,
                             atomic_uchar *output_stopped,
                             int *termination_reason,
                             int *rc,
                             char **errorText) {
    RXSPAWN_SNAPSHOT_OVERRIDE snapshot;
    const RXSPAWN_SNAPSHOT_OVERRIDE *previous = rxspawn_thread_snapshot;
    int result;
    snapshot.working_directory = working_directory;
    snapshot.environment = environment;
    snapshot.crexx_bindings = NULL;
    snapshot.crexx_binding_count = 0u;
    snapshot.cancelled = cancelled;
    snapshot.input_stopped = input_stopped;
    snapshot.output_stopped = output_stopped;
    snapshot.deadline_microseconds =
            rxspawn_deadline_from_wait(wait_microseconds);
    snapshot.termination_reason = termination_reason;
    if (termination_reason) *termination_reason = 0;
    rxspawn_thread_snapshot = &snapshot;
    result = spawn_argv_capture(
            argv, argc, pIn, pOut, pErr, NULL, rc, errorText);
    rxspawn_thread_snapshot = previous;
    return result;
}

/* Create a null redirect pipe */
/* In general,the redirect_reg MUST then be used in shellspawn() to clean up/free memory */
void nullredr(value* redirect_reg) {
    REDIRECT *redirect;

    redirect = redirect_endpoint_create(redirect_reg, REDIRECT_ENDPOINT_NULL);
    if (!redirect) return;

#ifdef _WIN32

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    // Open the NUL device for reading
    redirect->hRead = CreateFile("NUL",
                           GENERIC_READ,
                           0,                  // no sharing
                           &sa,             // set the bInheritHandle flag
                           OPEN_EXISTING, // open existing file only
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);              // no attr. template

    // Open the NUL device for writing
    redirect->hWrite = CreateFile("NUL",
                            GENERIC_WRITE,
                            0,                 // no sharing
                            &sa,            // set the bInheritHandle flag
                            OPEN_EXISTING,// open existing file only
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);              // no attr. template

#else

    redirect->hRead = open("/dev/null", O_RDONLY);
    redirect->hWrite = open("/dev/null", O_WRONLY);

#endif
}

/* Create a redirect pipe to string */
/* the redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void redr2str(value* redirect_reg, value* string_reg) {
    redirectOutput(redirect_reg, string_reg, REDIRECT_TRANSFER_OUTPUT_STRING);
}

/* Create a redirect pipe to string */
/* the redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void redr2arr(value* redirect_reg, value* string_reg) {
    redirectOutput(redirect_reg, string_reg, REDIRECT_TRANSFER_OUTPUT_ARRAY);
}

/* Create a redirect output pipe */
void redirectOutput(value* redirect_reg, value* string_reg, unsigned char transfer_mode) {

    REDIRECT *redirect;
    REDIRECT_COMPLETION *completion;

    redirect = redirect_endpoint_create(redirect_reg, REDIRECT_ENDPOINT_OUTPUT);
    if (!redirect) return;
    redirect->receiver = string_reg;

#ifdef _WIN32

    redirect->hRead = INVALID_HANDLE_VALUE;
    redirect->hWrite = INVALID_HANDLE_VALUE;
    HANDLE hReadTmp;

    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE}; // Set the bInheritHandle flag: the pipe is inheritable

    // We Create a pipe
    if (!CreatePipe(&hReadTmp, &redirect->hWrite, &sa, 0))
    {
        // Error - try and clean-up
        redirect->errorCode = 1;
        redirect->lastError = (int)GetLastError();
        return;
    }

    // Make a non-inheritable duplicate of the reading side of the pipe
    if (!DuplicateHandle(GetCurrentProcess(), hReadTmp,
                         GetCurrentProcess(),
                         &redirect->hRead, // Address of new handle.
                         0, FALSE, // Make it uninheritable.
                         DUPLICATE_SAME_ACCESS))
    {
        DWORD last_error = GetLastError();
        // Error - try and clean-up
        CloseHandle(hReadTmp);
        CloseHandle(redirect->hWrite);
        redirect->hWrite = INVALID_HANDLE_VALUE;
        redirect->errorCode = 2;
        redirect->lastError = (int)last_error;
        return;
    }

    /* We don't want this inheritable handle */
    if (!CloseHandle(hReadTmp))
    {
        DWORD last_error = GetLastError();
        // Error - try and clean-up
        CloseHandle(redirect->hRead);
        CloseHandle(redirect->hWrite);
        redirect->hRead = INVALID_HANDLE_VALUE;
        redirect->hWrite = INVALID_HANDLE_VALUE;
        redirect->errorCode = 3;
        redirect->lastError = (int)last_error;
        return;
    }
    hReadTmp = NULL;

#else

    int temppipe[2];    // This holds the fd for the input & output of the pipe ([0] for reading, [1] for writing)
    redirect->hRead = -1;
    redirect->hWrite = -1;

    if (pipe(temppipe)) {
        redirect->errorCode = 1;
        return;
    }
    redirect->hRead = temppipe[0];
    redirect->hWrite = temppipe[1];

#endif

    /* Transfer the parent read end into a non-VM single-owner completion. */
    completion = redirect_completion_create(transfer_mode);
    if (!completion) {
        redirect->errorCode = 1;
#ifdef _WIN32
        CloseHandle(redirect->hRead);
        CloseHandle(redirect->hWrite);
        redirect->hRead = INVALID_HANDLE_VALUE;
        redirect->hWrite = INVALID_HANDLE_VALUE;
#else
        close(redirect->hRead);
        close(redirect->hWrite);
        redirect->hRead = -1;
        redirect->hWrite = -1;
#endif
        return;
    }
    completion->io_handle = redirect->hRead;
    redirect->hRead = REDIRECT_INVALID_IO_HANDLE;
    if (redirect_completion_restrict_child_inheritance(completion) != 0) {
        redirect->errorCode = 1;
        redirect->lastError = errno;
        redirect->errorSource = 8;
#ifdef _WIN32
        CloseHandle(redirect->hWrite);
        redirect->hWrite = INVALID_HANDLE_VALUE;
#else
        close(redirect->hWrite);
        redirect->hWrite = -1;
#endif
        redirect_completion_destroy(completion);
        redirect->completion = NULL;
        return;
    }
    completion->terminal_state = REDIRECT_COMPLETION_RUNNING;
    redirect->completion = completion;

    /* Launch the thread that owns and drains the output read end. */
#ifdef _WIN32
    redirect->thread = CreateThread(NULL, 0, OutputCaptureThread,
                                    (LPVOID)completion, 0, NULL);
    if (redirect->thread == NULL) {
        DWORD last_error = GetLastError();
        CloseHandle(redirect->hWrite);
        redirect->hWrite = INVALID_HANDLE_VALUE;
        redirect->errorCode = 5;
        redirect->lastError = (int)last_error;
        completion->errorCode = 1;
        completion->lastError = (int)last_error;
        completion->errorSource = 5;
        redirect_completion_publish(completion, REDIRECT_COMPLETION_FAILED);
        redirect_completion_destroy(completion);
        redirect->completion = NULL;
        return;
    }
#else
    {
        int create_rc = pthread_create(&(redirect->thread), NULL,
                                       OutputCaptureThread, (void *)completion);
        if (create_rc) {
            close(redirect->hWrite);
            redirect->hWrite = -1;
            redirect->errorCode = 1;
            redirect->lastError = create_rc;
            completion->errorCode = 1;
            completion->lastError = create_rc;
            completion->errorSource = 5;
            redirect_completion_publish(completion, REDIRECT_COMPLETION_FAILED);
            redirect_completion_destroy(completion);
            redirect->completion = NULL;
            return;
        }
    }
#endif

    redirect->has_thread = 1;
}

static int redirect_capture_chunk(REDIRECT_COMPLETION *completion,
                                  const char *bytes,
                                  size_t length) {
    if (completion->transfer_mode == REDIRECT_TRANSFER_ENDPOINT_OUTPUT) {
        size_t accepted = 0u;
        rxvm_channel_status status = rxvm_byte_endpoint_write(
                completion->endpoint, bytes, length, -1,
                completion->cancelled, &accepted);
        if (status == RXVM_CHANNEL_OK && accepted == length) return 0;
        if (status == RXVM_CHANNEL_ALREADY_TERMINAL ||
            status == RXVM_CHANNEL_CLOSED) return 1;
        completion->errorCode = 1;
        completion->lastError = status;
        completion->errorSource = 14;
        return -1;
    }
    if (redirect_completion_append(completion, bytes, length) == 0) return 0;
    completion->errorCode = 1;
    completion->errorSource = 4;
    return -1;
}

/* Capture raw bytes only. Legacy joins publish them to the receiver VM
 * thread; byte-endpoint mode streams directly into bounded C-owned storage. */
THREAD_RETURN OutputCaptureThread(void* lpvThreadParam)
{
    REDIRECT_COMPLETION *completion = (REDIRECT_COMPLETION *)lpvThreadParam;
    char buffer[4096];
    int capture_failed = 0;

#ifdef _WIN32
    DWORD bytes_read;
    for (;;) {
        if (!ReadFile(completion->io_handle, buffer, (DWORD)sizeof(buffer),
                      &bytes_read, NULL)) {
            DWORD read_error = GetLastError();
            if (read_error == ERROR_BROKEN_PIPE || read_error == ERROR_HANDLE_EOF) break;
            completion->errorCode = 1;
            completion->lastError = (int)read_error;
            completion->errorSource = 3;
            break;
        }
        if (bytes_read == 0) break;
        if (!capture_failed) {
            int capture_rc = redirect_capture_chunk(
                    completion, buffer, (size_t)bytes_read);
            if (capture_rc > 0) break;
            if (capture_rc < 0) {
            /* Continue draining so allocation failure cannot deadlock the child. */
                if (completion->transfer_mode ==
                        REDIRECT_TRANSFER_ENDPOINT_OUTPUT) break;
                capture_failed = 1;
            }
        }
    }
#else
    for (;;) {
        ssize_t bytes_read = read(completion->io_handle, buffer, sizeof(buffer));
        if (bytes_read == 0) break;
        if (bytes_read == -1) {
            if (errno == EINTR) continue;
            completion->errorCode = 1;
            completion->lastError = errno;
            completion->errorSource = 3;
            break;
        }
        if (!capture_failed) {
            int capture_rc = redirect_capture_chunk(
                    completion, buffer, (size_t)bytes_read);
            if (capture_rc > 0) break;
            if (capture_rc < 0) {
            /* Continue draining so allocation failure cannot deadlock the child. */
                if (completion->transfer_mode ==
                        REDIRECT_TRANSFER_ENDPOINT_OUTPUT) break;
                capture_failed = 1;
            }
        }
    }
#endif

    redirect_completion_close_handle(completion);
    if (completion->transfer_mode == REDIRECT_TRANSFER_ENDPOINT_OUTPUT) {
        (void)rxvm_byte_endpoint_half_close(
                completion->endpoint, RXVM_BYTE_ENDPOINT_WRITE);
    }
    redirect_completion_publish(completion,
        completion->errorCode ? REDIRECT_COMPLETION_FAILED
                              : REDIRECT_COMPLETION_SUCCEEDED);
    return 0;
}

/* Appends record to an array and returns the new record */
value* add_new_element(value* array) {
    size_t index;
    size_t num;

    if (!array || array->num_attributes == (size_t)-1) return 0;

    index = array->num_attributes;
    num = index + 1;

    if (num > rxvm_value_max_attributes(array)) {
        if (num > ((size_t)-1) / 2) return 0;
        /* We need to increase the size of the buffer */
        /* Make the buffer double sized by setting the number of attributes */
        set_num_attributes(array, num * 2);
    }
    /* Set the number of attributes to the requested number */
    set_num_attributes(array, num);

    if (!array->attributes || array->num_attributes < num) return 0;
    return array->attributes[index];
}

/* Create a redirect pipe from a string */
/* the redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void str2redr(value* redirect_reg, value* string_reg) {
    redirectInput(redirect_reg, string_reg, REDIRECT_TRANSFER_INPUT_STRING);
}

/* Create a redirect pipe from a array */
/* the redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void arr2redr(value* redirect_reg, value* string_reg) {
    redirectInput(redirect_reg, string_reg, REDIRECT_TRANSFER_INPUT_ARRAY);
}

static int redirect_snapshot_input(REDIRECT_COMPLETION *completion, value *source) {
    size_t i;

    if (!completion || !source) return -1;
    if (completion->transfer_mode == REDIRECT_TRANSFER_INPUT_STRING) {
        if (redirect_completion_append(completion, source->string_value,
                                       source->string_length) != 0) return -1;
        return redirect_completion_append(completion, "\n", 1u);
    }
    if (completion->transfer_mode != REDIRECT_TRANSFER_INPUT_ARRAY) return -1;
    for (i = 0; i < source->num_attributes; i++) {
        value *element = source->attributes[i];
        if (redirect_completion_append(completion, element->string_value,
                                       element->string_length) != 0 ||
            redirect_completion_append(completion, "\n", 1u) != 0) return -1;
    }
    return 0;
}

/* Snapshot input before launching the thread that owns the pipe write end. */
void redirectInput(value* redirect_reg, value* string_reg, unsigned char transfer_mode) {
    REDIRECT *redirect;
    REDIRECT_COMPLETION *completion;

    redirect = redirect_endpoint_create(redirect_reg, REDIRECT_ENDPOINT_INPUT);
    if (!redirect) return;
    completion = redirect_completion_create(transfer_mode);
    if (!completion || redirect_snapshot_input(completion, string_reg) != 0) {
        redirect->errorCode = 1;
        redirect_completion_destroy(completion);
        return;
    }
    redirect->completion = completion;

#ifdef _WIN32

    redirect->hRead = INVALID_HANDLE_VALUE;
    redirect->hWrite = INVALID_HANDLE_VALUE;

    HANDLE hWriteTmp;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    // We Create a pipe
    if (!CreatePipe(&(redirect->hRead),&hWriteTmp,  &sa, 0))
    {
        // Error - try and clean-up
        redirect->errorCode = 1;
        redirect->lastError = (int)GetLastError();
        redirect_completion_destroy(completion);
        redirect->completion = NULL;
        return;
    }

    // Make a non-inheritable write handle to the pipe (i.e. the parent end)
    if (!DuplicateHandle(GetCurrentProcess(), hWriteTmp,
                         GetCurrentProcess(),
                         &(redirect->hWrite), // Address of new handle.
                         0, FALSE, // Make it uninheritable.
                         DUPLICATE_SAME_ACCESS))
    {
        DWORD last_error = GetLastError();
        // Error - try and clean-up
        CloseHandle(hWriteTmp);
        CloseHandle(redirect->hRead);
        redirect->hRead = INVALID_HANDLE_VALUE;
        redirect->errorCode = 2;
        redirect->lastError = (int)last_error;
        redirect_completion_destroy(completion);
        redirect->completion = NULL;
        return;
    }

    /* We don't want this closeable handle */
    if (!CloseHandle(hWriteTmp))
    {
        DWORD last_error = GetLastError();
        // Error - try and clean-up
        CloseHandle(redirect->hRead);
        CloseHandle(redirect->hWrite);
        redirect->hRead = INVALID_HANDLE_VALUE;
        redirect->hWrite = INVALID_HANDLE_VALUE;
        redirect->errorCode = 3;
        redirect->lastError = (int)last_error;
        redirect_completion_destroy(completion);
        redirect->completion = NULL;
        return;
    }

    completion->io_handle = redirect->hWrite;
    redirect->hWrite = INVALID_HANDLE_VALUE;
    if (redirect_completion_restrict_child_inheritance(completion) != 0) {
        redirect->errorCode = 1;
        redirect->lastError = errno;
        redirect->errorSource = 8;
        CloseHandle(redirect->hRead);
        redirect->hRead = INVALID_HANDLE_VALUE;
        redirect_completion_destroy(completion);
        redirect->completion = NULL;
        return;
    }
    completion->terminal_state = REDIRECT_COMPLETION_RUNNING;

    {
    DWORD threadID;
    redirect->thread = CreateThread(NULL, 0, InputSnapshotThread,
                                    completion, 0, &threadID);
    if (redirect->thread == NULL)
    {
        DWORD last_error = GetLastError();
        CloseHandle(redirect->hRead);
        redirect->hRead = INVALID_HANDLE_VALUE;
        redirect->errorCode = 4;
        redirect->lastError = (int)last_error;
        completion->errorCode = 1;
        completion->lastError = (int)last_error;
        completion->errorSource = 5;
        redirect_completion_publish(completion, REDIRECT_COMPLETION_FAILED);
        redirect_completion_destroy(completion);
        redirect->completion = NULL;
        return;
    }
    }

#else

    int temppipe[2];    // This holds the fd for the input & output of the pipe

    redirect->hRead = -1;
    redirect->hWrite = -1;

    // Create a pipe
    if (pipe(temppipe)) {
        redirect->errorCode = 1;
        redirect_completion_destroy(completion);
        redirect->completion = NULL;
        return;
    }
    redirect->hRead = temppipe[0];
    redirect->hWrite = temppipe[1];

    completion->io_handle = redirect->hWrite;
    redirect->hWrite = -1;
    if (redirect_completion_restrict_child_inheritance(completion) != 0) {
        redirect->errorCode = 1;
        redirect->lastError = errno;
        redirect->errorSource = 8;
        close(redirect->hRead);
        redirect->hRead = -1;
        redirect_completion_destroy(completion);
        redirect->completion = NULL;
        return;
    }
    completion->terminal_state = REDIRECT_COMPLETION_RUNNING;
    {
        int create_rc = pthread_create(&(redirect->thread), NULL,
                                       InputSnapshotThread, (void *)completion);
        if (create_rc) {
            close(redirect->hRead);
            redirect->hRead = -1;
            redirect->errorCode = 2;
            redirect->lastError = create_rc;
            completion->errorCode = 1;
            completion->lastError = create_rc;
            completion->errorSource = 5;
            redirect_completion_publish(completion, REDIRECT_COMPLETION_FAILED);
            redirect_completion_destroy(completion);
            redirect->completion = NULL;
            return;
        }
    }
#endif
    redirect->has_thread = 1;
}

static int redirect_write_chunk_to_child(REDIRECT_COMPLETION *completion,
                                         const unsigned char *bytes,
                                         size_t length) {
    size_t total = 0u;
    while (total < length) {
#ifdef _WIN32
        size_t remaining = length - total;
        DWORD request = remaining > (size_t)0x7fffffffu
                ? (DWORD)0x7fffffffu : (DWORD)remaining;
        DWORD written = 0u;
        if (!WriteFile(completion->io_handle, bytes + total,
                       request, &written, NULL)) {
            DWORD error = GetLastError();
            if (error == ERROR_NO_DATA || error == ERROR_BROKEN_PIPE) return 1;
            completion->errorCode = 1;
            completion->lastError = (int)error;
            completion->errorSource = 7;
            return -1;
        }
        if (!written) return 1;
        total += (size_t)written;
#else
        ssize_t written = write(
                completion->io_handle, bytes + total, length - total);
        if (written == -1) {
            if (errno == EINTR) continue;
            if (errno == EPIPE) return 1;
            completion->errorCode = 1;
            completion->lastError = errno;
            completion->errorSource = 7;
            return -1;
        }
        if (!written) return 1;
        total += (size_t)written;
#endif
    }
    return 0;
}

/* Write an immutable snapshot or stream a C-owned byte endpoint. This thread
 * has no VM value, object, register or worker-state pointer. */
THREAD_RETURN InputSnapshotThread(void* lpvThreadParam)
{
    REDIRECT_COMPLETION *completion = (REDIRECT_COMPLETION *)lpvThreadParam;
    size_t total = 0;

#ifndef _WIN32
    sigset_t signal_mask;
    int mask_rc;
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGPIPE);
    mask_rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
    if (mask_rc) {
        completion->errorCode = 1;
        completion->lastError = mask_rc;
        completion->errorSource = 6;
    }
#endif

    if (completion->transfer_mode == REDIRECT_TRANSFER_ENDPOINT_INPUT) {
        while (!completion->errorCode) {
            unsigned char bytes[4096];
            size_t length = 0u;
            int eof = 0;
            rxvm_channel_status status = rxvm_byte_endpoint_read(
                    completion->endpoint, bytes, sizeof(bytes), -1,
                    completion->cancelled, &length, &eof);
            if (status == RXVM_CHANNEL_ALREADY_TERMINAL ||
                status == RXVM_CHANNEL_CLOSED) break;
            if (status != RXVM_CHANNEL_OK) {
                completion->errorCode = 1;
                completion->lastError = status;
                completion->errorSource = 15;
                break;
            }
            if (length && redirect_write_chunk_to_child(
                    completion, bytes, length) != 0) break;
            if (eof) break;
        }
        redirect_completion_close_handle(completion);
        redirect_completion_publish(completion,
            completion->errorCode ? REDIRECT_COMPLETION_FAILED
                                  : REDIRECT_COMPLETION_SUCCEEDED);
        return 0;
    }

#ifdef _WIN32
    while (!completion->errorCode && total < completion->length) {
        size_t remaining = completion->length - total;
        DWORD request = remaining > (size_t)0x7fffffffu
            ? (DWORD)0x7fffffffu : (DWORD)remaining;
        DWORD written = 0;
        if (!WriteFile(completion->io_handle, completion->bytes + total,
                       request, &written, NULL)) {
            DWORD write_error = GetLastError();
            if (write_error == ERROR_NO_DATA || write_error == ERROR_BROKEN_PIPE) break;
            completion->errorCode = 1;
            completion->lastError = (int)write_error;
            completion->errorSource = 7;
            break;
        }
        if (written == 0) break;
        total += (size_t)written;
    }
#else
    while (!completion->errorCode && total < completion->length) {
        ssize_t written = write(completion->io_handle,
                                completion->bytes + total,
                                completion->length - total);
        if (written == -1) {
            if (errno == EINTR) continue;
            if (errno == EPIPE) break;
            completion->errorCode = 1;
            completion->lastError = errno;
            completion->errorSource = 7;
            break;
        }
        if (written == 0) break;
        total += (size_t)written;
    }
#endif

    redirect_completion_close_handle(completion);
    redirect_completion_publish(completion,
        completion->errorCode ? REDIRECT_COMPLETION_FAILED
                              : REDIRECT_COMPLETION_SUCCEEDED);
    return 0;
}

static REDIRECT *redirect_byte_endpoint_create(
        rxvm_byte_endpoint *endpoint,
        const atomic_uchar *cancelled,
        int output_from_child) {
    REDIRECT *redirect;
    REDIRECT_COMPLETION *completion;
    int required_direction = output_from_child
            ? RXVM_BYTE_ENDPOINT_WRITE : RXVM_BYTE_ENDPOINT_READ;
    if (!endpoint ||
        !(rxvm_byte_endpoint_direction(endpoint) & required_direction)) {
        return NULL;
    }
    redirect = (REDIRECT *)calloc(1u, sizeof(*redirect));
    if (!redirect) return NULL;
    redirect_endpoint_init(
            redirect, output_from_child
                    ? REDIRECT_ENDPOINT_OUTPUT : REDIRECT_ENDPOINT_INPUT);
    completion = redirect_completion_create(
            output_from_child
                    ? REDIRECT_TRANSFER_ENDPOINT_OUTPUT
                    : REDIRECT_TRANSFER_ENDPOINT_INPUT);
    if (!completion) {
        free(redirect);
        return NULL;
    }
    completion->endpoint = endpoint;
    completion->cancelled = cancelled;
    rxvm_byte_endpoint_retain(endpoint);
    if (redirect_pipe_start(redirect, completion, output_from_child) != 0) {
        redirect_completion_destroy(completion);
        free(redirect);
        return NULL;
    }
    return redirect;
}

REDIRECT *rxspawn_redirect_from_byte_endpoint(
        rxvm_byte_endpoint *endpoint,
        const atomic_uchar *cancelled) {
    return redirect_byte_endpoint_create(endpoint, cancelled, 0);
}

REDIRECT *rxspawn_redirect_to_byte_endpoint(
        rxvm_byte_endpoint *endpoint,
        const atomic_uchar *cancelled) {
    return redirect_byte_endpoint_create(endpoint, cancelled, 1);
}

int rxspawn_redirect_byte_endpoint_destroy(REDIRECT *redirect) {
    int result;
    if (!redirect) return 0;
    if (redirect->endpoint_kind == REDIRECT_ENDPOINT_INPUT) {
        result = crexxcmd_close_input_redirect(redirect);
    } else {
        result = crexxcmd_close_output_redirect(redirect);
    }
    if (result == 0 || !redirect->has_thread) free(redirect);
    return result;
}

int rxspawn_redirect_write_close(REDIRECT *redirect,
                                 const char *data,
                                 size_t length) {
    if (!redirect || redirect->endpoint_kind != REDIRECT_ENDPOINT_OUTPUT ||
        (!data && length)) return -1;
    WriteToStdin(redirect, (char *)(data ? data : ""), length);
    if (redirect->errorCode) return -1;
    return crexxcmd_close_output_redirect(redirect);
}

void WriteToStdin(REDIRECT* data, char *line, size_t nBytes)
{
#ifdef _WIN32
    DWORD nTotalWrote = 0;
    DWORD nBytesWrote = 0;
#else
    size_t nTotalWrote = 0;
    size_t nBytesWrote;
#endif
    while (nTotalWrote < nBytes)
    {

#ifdef _WIN32

        if (!WriteFile(data->hWrite,(line+nTotalWrote),(nBytes-nTotalWrote),&nBytesWrote, NULL))
        {
            if (GetLastError() == ERROR_NO_DATA) {
                // Pipe was closed, a normal exit path - the child exited before processing all input
                return;
            }
            else {
                data->errorCode = 1;
                data->lastError = (int)GetLastError();
                data->errorSource = 5;
                return;
            }
        }

#else

        nBytesWrote = write(data->hWrite, (void*)(line+nTotalWrote), (nBytes-nTotalWrote));

        if (nBytesWrote == -1)
        {
            if (errno == EPIPE) {
                // Pipe was closed, a normal exit path - the child exited before processing all input
                return;
            }
            else {
                data->errorCode = 1;
                return;
            }
        }

#endif

        nTotalWrote += nBytesWrote;
    }
}

int redrwriteclose(value* redirect_reg, const char* data, size_t nBytes)
{
    REDIRECT* redirect;

    redirect = rxspawn_redirect_from_value(redirect_reg);
    if (!redirect) return 1;
    if (!data) data = "";

    return rxspawn_redirect_write_close(redirect, data, nBytes);
}

void CleanUp(SHELLDATA* data)
{
    /* Every abandonment path must release endpoint transfers before joining
     * them, including failures that occur before a child is launched. */
    rxspawn_stop_redirect_io(data, 1);
    if (data->buffer) {
        rxspawn_memory_free(data->buffer);
        data->buffer = 0;
    }
    if (data->argv) {
        rxspawn_memory_free(data->argv);
        data->argv = 0;
    }
    if (data->file_path) {
        rxspawn_memory_free(data->file_path);
        data->file_path = 0;
    }
    if (data->application_path) {
        rxspawn_memory_free(data->application_path);
        data->application_path = 0;
    }
    rxcrexxcmd_process_snapshot_free(data->working_directory,
                                     data->environment);
    data->working_directory = 0;
    data->environment = 0;

#ifdef _WIN32

    PROCESS_INFORMATION* pProcInfo = &(data->ChildProcessInfo);
    if (data->ChildJob) {
        (void)TerminateJobObject(data->ChildJob, 130u);
    }
    if (pProcInfo->hProcess) {
        TerminateProcess(pProcInfo->hProcess, 0);
        CloseHandle(pProcInfo->hProcess);
        pProcInfo->hProcess = NULL;
    }
    if (pProcInfo->hThread) {
        CloseHandle(pProcInfo->hThread);
        pProcInfo->hThread = NULL;
    }
    if (data->ChildJob) {
        CloseHandle(data->ChildJob);
        data->ChildJob = NULL;
    }

    // Close any pipes
    if (data->pInput && data->pInput->hRead != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pInput->hRead);
        data->pInput->hRead = INVALID_HANDLE_VALUE;
    }
    if (data->pOutput && data->pOutput->hWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pOutput->hWrite);
        data->pOutput->hWrite = INVALID_HANDLE_VALUE;
    }
    if (data->pError && data->pError->hWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pError->hWrite);
        data->pError->hWrite = INVALID_HANDLE_VALUE;
    }
    if (data->pInput && data->pInput->hWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pInput->hWrite);
        data->pInput->hWrite = INVALID_HANDLE_VALUE;
    }
    if (data->pOutput && data->pOutput->hRead != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pOutput->hRead);
        data->pOutput->hRead = INVALID_HANDLE_VALUE;
    }
    if (data->pError && data->pError->hRead != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pError->hRead);
        data->pError->hRead = INVALID_HANDLE_VALUE;
    }

    // Close the thread handles in the redirect structures
    (void)join_redirect_thread(data->pInput);
    (void)join_redirect_thread(data->pOutput);
    (void)join_redirect_thread(data->pError);

#else

    if (data->ChildProcessPID > 0) {
        pid_t child_pid = data->ChildProcessPID;
        int child_status;

        /* CleanUp is an abandonment path: stop and reap the direct child
         * before joining pipe owners, so no inherited end can keep them live. */
        if (kill(-child_pid, SIGKILL) == -1 &&
            errno == ESRCH && kill(child_pid, SIGKILL) == -1 &&
            errno != ESRCH) data->waitThreadRC = 1;
        while (waitpid(child_pid, &child_status, 0) == -1 && errno == EINTR) {
        }
        data->ChildProcessPID = 0;
    }

    // Close any pipes
    if (data->pInput && data->pInput->hRead != -1) {
        close(data->pInput->hRead);
        data->pInput->hRead = -1;
    }
    if (data->pOutput && data->pOutput->hWrite != -1) {
        close(data->pOutput->hWrite);
        data->pOutput->hWrite = -1;
    }
    if (data->pError && data->pError->hWrite != -1) {
        close(data->pError->hWrite);
        data->pError->hWrite = -1;
    }
    if (data->pInput && data->pInput->hWrite != -1) {
        close(data->pInput->hWrite);
        data->pInput->hWrite = -1;
    }
    if (data->pOutput && data->pOutput->hRead != -1) {
        close(data->pOutput->hRead);
        data->pOutput->hRead = -1;
    }
    if (data->pError && data->pError->hRead != -1) {
        close(data->pError->hRead);
        data->pError->hRead = -1;
    }

    /* The completion owns the opposite pipe end; join before its payload dies. */
    (void)join_redirect_thread(data->pInput);
    (void)join_redirect_thread(data->pOutput);
    (void)join_redirect_thread(data->pError);
#endif
}

void appendTextOutput(char **outputText, char *inputText) {
    if (*outputText) {
        *outputText = realloc(*outputText, strlen(*outputText) + strlen(inputText) + 1);
        strcat(*outputText, inputText);
    }
    else {
        *outputText = malloc(strlen(inputText) + 1);
        strcpy(*outputText, inputText);
    }
}

// Waits for the child process and all the input/output thread handlers to exit.
void WaitForProcess(SHELLDATA* data)
{

#ifdef _WIN32

    DWORD dwWaitResult;

    // Close the child ends of any pipes
    if (data->pInput && data->pInput->hRead != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pInput->hRead);
        data->pInput->hRead = INVALID_HANDLE_VALUE;
    }
    if (data->pOutput && data->pOutput->hWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pOutput->hWrite);
        data->pOutput->hWrite = INVALID_HANDLE_VALUE;
    }
    if (data->pError && data->pError->hWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pError->hWrite);
        data->pError->hWrite = INVALID_HANDLE_VALUE;
    }

    // Wait in bounded slices so provider cancellation/deadline can terminate
    // and join the direct child deterministically.
    for (;;) {
        dwWaitResult = WaitForSingleObject(
                data->ChildProcessInfo.hProcess, 10u);
        if (dwWaitResult != WAIT_TIMEOUT) break;
        if (rxspawn_stop_requested(data)) {
            BOOL terminated = data->ChildJob
                    ? TerminateJobObject(
                          data->ChildJob, data->timed_out ? 124u : 130u)
                    : TerminateProcess(
                          data->ChildProcessInfo.hProcess,
                          data->timed_out ? 124u : 130u);
            if (!terminated) {
                data->waitThreadRC = 1;
                Error("Failure spawn terminate", &data->waitThreadErrorText);
            } else {
                data->terminated = 1u;
            }
            dwWaitResult = WaitForSingleObject(
                    data->ChildProcessInfo.hProcess, INFINITE);
            break;
        }
    }
    if (dwWaitResult == WAIT_OBJECT_0) {
        // The child process has terminated.
        DWORD process_rc;
        if (!GetExitCodeProcess(data->ChildProcessInfo.hProcess, &process_rc)) {
            // Error in GetExitCodeProcess.
            data->waitThreadRC = 1;
            Error("Failure spawn U43", &(data->waitThreadErrorText));
        }
        data->ChildProcessRC = data->terminated
                ? (data->timed_out ? 124 : 130) : (int)process_rc;
    }
    else {
        // The child process is not signaled.
        data->waitThreadRC = 1;
        Error("Failure spawn U43", &(data->waitThreadErrorText));
    }

    CloseHandle(data->ChildProcessInfo.hProcess);
    data->ChildProcessInfo.hProcess = NULL;
    if (data->ChildProcessInfo.hThread) {
        CloseHandle(data->ChildProcessInfo.hThread);
    }
    data->ChildProcessInfo.hThread = NULL;
    if (data->ChildJob) {
        /* The direct child may have completed while descendants retained the
         * stdio handles. Closing the job ends the entire owned tree first. */
        CloseHandle(data->ChildJob);
        data->ChildJob = NULL;
    }

    /* A normally exited child can leave its stdin producer blocked on an
     * open endpoint. Stop only input; stdout/stderr must still drain to EOF. */
    rxspawn_stop_redirect_io(data, 0);

    // Wait for the Input, Output and Error threads to die
    if (data->pInput && data->pInput->has_thread)
    {
        if (join_redirect_thread(data->pInput) != 0) {
            data->waitThreadRC = 1;
            Error("Failure spawn U44", &data->waitThreadErrorText);
        }
    }
    if (data->pOutput && data->pOutput->has_thread)
    {
        if (join_redirect_thread(data->pOutput) != 0) {
            data->waitThreadRC = 1;
            Error("Failure spawn U45", &data->waitThreadErrorText);
        }
    }
    if (data->pError && data->pError->has_thread)
    {
        if (join_redirect_thread(data->pError) != 0) {
            data->waitThreadRC = 1;
            Error("Failure spawn U46", &data->waitThreadErrorText);
        }
    }

    // Close 'my' end of any pipes
    if (data->pInput && data->pInput->hWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pInput->hWrite);
        data->pInput->hWrite = INVALID_HANDLE_VALUE;
    }
    if (data->pOutput && data->pOutput->hRead != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pOutput->hRead);
        data->pOutput->hRead = INVALID_HANDLE_VALUE;
    }
    if (data->pError && data->pError->hRead != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pError->hRead);
        data->pError->hRead = INVALID_HANDLE_VALUE;
    }

#else

    pid_t w;
    int status;

    // Close the child ends of any pipes
    if (data->pInput && data->pInput->hRead != -1) {
        close(data->pInput->hRead);
        data->pInput->hRead = -1;
    }
    if (data->pOutput && data->pOutput->hWrite != -1) {
        close(data->pOutput->hWrite);
        data->pOutput->hWrite = -1;
    }
    if (data->pError && data->pError->hWrite != -1) {
        close(data->pError->hWrite);
        data->pError->hWrite = -1;
    }

    // Wait for child process to exit
    int pid;
    pid = data->ChildProcessPID;

    for (;;) {
        w = waitpid(pid, &status, WUNTRACED | WCONTINUED |
                                 ((data->cancelled ||
                                   data->deadline_microseconds)
                                  ? WNOHANG : 0));
        if (w == 0) {
            if (rxspawn_stop_requested(data)) {
                int kill_result = kill(-pid, SIGKILL);
                if (kill_result == -1 && errno == ESRCH) {
                    kill_result = kill(pid, SIGKILL);
                }
                if (kill_result == -1 && errno != ESRCH) {
                    data->waitThreadRC = 1;
                    Error("Failure spawn terminate",
                          &data->waitThreadErrorText);
                } else {
                    data->terminated = 1u;
                }
                do {
                    w = waitpid(pid, &status, 0);
                } while (w == -1 && errno == EINTR);
                break;
            }
            {
                struct timespec pause_time;
                pause_time.tv_sec = 0;
                pause_time.tv_nsec = 10000000L;
                nanosleep(&pause_time, NULL);
            }
            continue;
        }
        if (w == -1)
        {
            if (errno == EINTR) continue;
            data->waitThreadRC = 1;
            Error("Failure spawn U43", &data->waitThreadErrorText);
            if (errno == ECHILD) data->ChildProcessPID = 0;
            break;
        }
        if (WIFEXITED(status) || WIFSIGNALED(status)) break;
    }
    if (w != -1) {
        data->ChildProcessPID = 0;
        if (data->terminated) {
            data->ChildProcessRC = data->timed_out ? 124 : 130;
        } else if (WIFEXITED(status)) {
            data->ChildProcessRC = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            data->ChildProcessRC = 128 + WTERMSIG(status);
        }
    }

    /* See the Windows path above: normal exit closes stdin ownership only. */
    rxspawn_stop_redirect_io(data, 0);

    /* Wait for the Input, Output and Error threads to die */
    if (data->pInput && data->pInput->has_thread)
    {
        if (join_redirect_thread(data->pInput) != 0) {
            data->waitThreadRC = 1;
            Error("Failure spawn U44", &data->waitThreadErrorText);
        }
    }

    if (data->pOutput && data->pOutput->has_thread)
    {
        if (join_redirect_thread(data->pOutput) != 0) {
            data->waitThreadRC = 1;
            Error("Failure spawn U45", &data->waitThreadErrorText);
        }
    }

    if (data->pError && data->pError->has_thread)
    {
        if (join_redirect_thread(data->pError) != 0) {
            data->waitThreadRC = 1;
            Error("Failure spawn U46", &data->waitThreadErrorText);
        }
    }

    // Close 'my' end of any pipes
    if (data->pInput && data->pInput->hWrite != -1) {
        close(data->pInput->hWrite);
        data->pInput->hWrite = -1;
    }
    if (data->pOutput && data->pOutput->hRead != -1) {
        close(data->pOutput->hRead);
        data->pOutput->hRead = -1;
    }
    if (data->pError && data->pError->hRead != -1) {
        close(data->pError->hRead);
        data->pError->hRead = -1;
    }

#endif

    /* Check for redirect errors */
    if (data->pInput && data->pInput->errorCode) {
        data->waitThreadRC = 1;
        Error("Failure spawn U47", &data->waitThreadErrorText);
    }
    if (data->pOutput && data->pOutput->errorCode) {
        data->waitThreadRC = 1;
        Error("Failure spawn U48", &data->waitThreadErrorText);
    }
    if (data->pError && data->pError->errorCode) {
        data->waitThreadRC = 1;
        Error("Failure spawn U49", &data->waitThreadErrorText);
    }
}

void Error(char *context, char **errorText)
{
    size_t message_len;
    char *message = "%s. Details: RC=%s Text=%s";
    char sRC[10];
    sprintf(sRC, "%d", errno);

    message_len = strlen(message) + strlen((char*)strerror(errno)) + strlen(context) + 11;
    *errorText = malloc(message_len);
    snprintf(*errorText, message_len, context, sRC, (char*)strerror(errno));
}

/* Parse the command to get the arguments */
int ParseCommand(const char *command_string, char **command, char **file, char ***argv) {
    int l = 0;
    int args = 1;
    int a;
    int arg_start;

    *command = rxspawn_memory_alloc(strlen(command_string) + 1u);
    if (*command == NULL) {
        *command = 0;
        *file = 0;
        *argv = 0;
        return -1;
    }
    strcpy(*command, command_string);

    // Skip Leading Spaces
    for (; (*command)[l]; l++) if ((*command)[l] != ' ') break;

    // Program bin/exe name
    *file = *command + l;
    for (; (*command)[l]; l++) if ((*command)[l] == ' ') break;
    if ((*command)[l] != 0) {
        (*command)[l] = 0;
        l++;
    }

    // Is there any command at all
    if (!(*file)[0]) {
        rxspawn_memory_free(*command);
        *command = 0;
        *file = 0;
        *argv = 0;
        return -1;
    }

    // Skip Trailing Spaces
    for (; (*command)[l]; l++) if ((*command)[l] != ' ') break;

    if ((*command)[l] != 0) { // There are some arguments
        arg_start = l;

        // Count Arguments
        while ((*command)[l]) {
            switch ((*command)[l]) {
                case '"':
                    // Read to the end of the string
                    for (l++; (*command)[l]; l++)
                        if ((*command)[l] == '"') {
                            l++;
                            break;
                        }
                    args++;
                    break;
                case '\'':
                    // Read to the end of the string
                    for (l++; (*command)[l]; l++)
                        if ((*command)[l] == '\'') {
                            l++;
                            break;
                        }
                    args++;
                    break;
                default:
                    for (l++; (*command)[l]; l++)
                        if ((*command)[l] == ' ') {
                            l++;
                            break;
                        }
                    args++;
                    break;
            }
            // Skip Trailing Spaces
            for (; (*command)[l]; l++) if ((*command)[l] != ' ') break;
        }
    }

    *argv = rxspawn_memory_alloc(sizeof(char*) * (size_t)(args + 1));
    if (*argv == NULL) {
        rxspawn_memory_free(*command);
        *command = 0;
        *file = 0;
        *argv = 0;
        return -1;
    }
    if (((*argv)[0] = strrchr(*file, '/')) != NULL)
        (*argv)[0]++;
    else
        (*argv)[0] = *file;

    // Null Terminator
    (*argv)[args] = 0;

    // Process Arguments
    if (args > 1) {
        a = 1;
        l = arg_start;
        while ((*command)[l]) {
            switch ((*command)[l]) {
                case '"':
                    (*argv)[a] = *command + l + 1;
                    for (l++; (*command)[l]; l++) {
                        if ((*command)[l] == '"') {
                            (*command)[l] = 0;
                            l++;
                            break;
                        }
                    }
                    a++;
                    break;
                case '\'':
                    (*argv)[a] = *command + l + 1;
                    for (l++; (*command)[l]; l++) {
                        if ((*command)[l] == '\'') {
                            (*command)[l] = 0;
                            l++;
                            break;
                        }
                    }
                    a++;
                    break;
                default:
                    (*argv)[a] = *command + l;
                    for (l++; (*command)[l]; l++) {
                        if ((*command)[l] == ' ') {
                            (*command)[l] = 0;
                            l++;
                            break;
                        }
                    }
                    a++;
                    break;
            }
            // Skip Trailing Spaces
            for (; (*command)[l]; l++) if ((*command)[l] != ' ') break;
        }
    }

    return 0;
}

// Launches the child job - never returns
static int merge_child_variables(SHELLDATA *data) {
    size_t count = 0;
    int i;

    if (!data || !data->environment || !data->variables) return 0;
    while (data->environment[count]) count++;

    for (i = 0; i + 1 < data->variables->num_attributes; i += 2) {
        value *name_value = data->variables->attributes[i];
        value *text_value = data->variables->attributes[i + 1];
        size_t name_length = name_value->string_length;
        size_t text_length = text_value->string_length;
        char *entry = (char *)malloc(name_length + text_length + 2u);
        size_t j;
        size_t replace = count;
        char **resized;
        if (!entry) return -1;
        for (j = 0; j < name_length; j++) {
            entry[j] = (char)toupper((unsigned char)name_value->string_value[j]);
        }
        entry[name_length] = '=';
        memcpy(entry + name_length + 1u, text_value->string_value, text_length);
        entry[name_length + text_length + 1u] = '\0';

        for (j = 0; j < count; j++) {
            const char *equals = strchr(data->environment[j], '=');
            size_t existing_length = equals
                ? (size_t)(equals - data->environment[j])
                : strlen(data->environment[j]);
            if (existing_length == name_length &&
#ifdef _WIN32
                _strnicmp(data->environment[j], entry, name_length) == 0
#else
                strncmp(data->environment[j], entry, name_length) == 0
#endif
            ) {
                replace = j;
                break;
            }
        }
        if (replace < count) {
            free(data->environment[replace]);
            data->environment[replace] = entry;
            continue;
        }
        resized = (char **)realloc(data->environment,
                                   (count + 2u) * sizeof(char *));
        if (!resized) {
            free(entry);
            return -1;
        }
        data->environment = resized;
        data->environment[count++] = entry;
        data->environment[count] = NULL;
    }
    return 0;
}

int launchChild(SHELLDATA* data) {

    if (merge_child_variables(data) != 0) {
        CleanUp(data);
        return SHELLSPAWN_FAILURE;
    }

#ifdef _WIN32

    // Launch the redirected command
    STARTUPINFOEXW si;
    SIZE_T attributeListSize;
    HANDLE inheritedHandles[3];
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION job_information;
    int inheritedHandleCount;
    int useHandleList;
    int controlled_child;
    int i;

    // Set up the start up info struct.
    ZeroMemory(&si, sizeof(STARTUPINFOEXW));
    si.StartupInfo.cb = sizeof(STARTUPINFOEXW);
    si.StartupInfo.dwFlags = STARTF_USESTDHANDLES;

    si.StartupInfo.hStdOutput = (data->pOutput && data->pOutput->hWrite != INVALID_HANDLE_VALUE) ? data->pOutput->hWrite : GetStdHandle(STD_OUTPUT_HANDLE);
    si.StartupInfo.hStdError = (data->pError && data->pError->hWrite != INVALID_HANDLE_VALUE) ? data->pError->hWrite : GetStdHandle(STD_ERROR_HANDLE);
    si.StartupInfo.hStdInput = (data->pInput && data->pInput->hRead != INVALID_HANDLE_VALUE) ? data->pInput->hRead : GetStdHandle(STD_INPUT_HANDLE);

    int flags = CREATE_UNICODE_ENVIRONMENT; // UTF16 Environment Variables
    controlled_child = data->cancelled || data->deadline_microseconds;
    attributeListSize = 0;
    inheritedHandleCount = 0;
    useHandleList = data->pInput && data->pOutput && data->pError
        && data->pInput->hRead != INVALID_HANDLE_VALUE
        && data->pOutput->hWrite != INVALID_HANDLE_VALUE
        && data->pError->hWrite != INVALID_HANDLE_VALUE;
    if (useHandleList) {
        /* Restrict Windows child inheritance to the stdio handles for this
         * spawn, otherwise nested ADDRESS runs can inherit unrelated pipes. */
        inheritedHandles[inheritedHandleCount++] = si.StartupInfo.hStdInput;
        inheritedHandles[inheritedHandleCount++] = si.StartupInfo.hStdOutput;
        if (si.StartupInfo.hStdError != si.StartupInfo.hStdOutput) {
            inheritedHandles[inheritedHandleCount++] = si.StartupInfo.hStdError;
        }

        InitializeProcThreadAttributeList(NULL, 1, 0, &attributeListSize);
        si.lpAttributeList = rxspawn_memory_alloc(attributeListSize);
        if (!si.lpAttributeList) {
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }
        if (!InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &attributeListSize)) {
            rxspawn_memory_free(si.lpAttributeList);
            si.lpAttributeList = NULL;
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }
        if (!UpdateProcThreadAttribute(si.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
                                       inheritedHandles, (SIZE_T)inheritedHandleCount * sizeof(HANDLE),
                                       NULL, NULL)) {
            windows_release_startup_attributes(&si);
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }
        flags |= EXTENDED_STARTUPINFO_PRESENT;
    }

    /* Build the child environment. A CREXX logical-state snapshot is already
     * merged with host bindings; ordinary spawns retain the legacy parent
     * environment plus explicit host variables. */
    LPWSTR pszNewEnvironment = NULL;
    if (data->environment) {
        size_t wide_count = 1u;
        wchar_t *cursor;
        for (i = 0; data->environment[i]; i++) {
            int needed = MultiByteToWideChar(CP_UTF8, 0,
                                             data->environment[i], -1,
                                             NULL, 0);
            if (needed <= 0) {
                windows_release_startup_attributes(&si);
                CleanUp(data);
                return SHELLSPAWN_FAILURE;
            }
            wide_count += (size_t)needed;
        }
        pszNewEnvironment = (LPWSTR)rxvm_memory_calloc_bytes(
            rxvm_memory_current_worker(), wide_count, sizeof(wchar_t));
        if (!pszNewEnvironment) {
            windows_release_startup_attributes(&si);
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }
        cursor = pszNewEnvironment;
        for (i = 0; data->environment[i]; i++) {
            int remaining = (int)(wide_count -
                                  (size_t)(cursor - pszNewEnvironment));
            int copied = MultiByteToWideChar(CP_UTF8, 0,
                                              data->environment[i], -1,
                                              cursor, remaining);
            if (copied <= 0) {
                rxspawn_memory_free(pszNewEnvironment);
                windows_release_startup_attributes(&si);
                CleanUp(data);
                return SHELLSPAWN_FAILURE;
            }
            cursor += copied;
        }
        *cursor = L'\0';
    } else {
        LPWSTR current_environment = GetEnvironmentStringsW();
        LPWSTR end;
        LPWSTR cursor;
        size_t parent_size;
        size_t new_size;
        if (!current_environment) {
            windows_release_startup_attributes(&si);
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }
        end = current_environment;
        while (*end) end += wcslen(end) + 1u;
        parent_size = (size_t)(end - current_environment);
        new_size = parent_size + 2u;
        for (i = 0; data->variables &&
                    i + 1 < data->variables->num_attributes; i += 2) {
            new_size += (size_t)MultiByteToWideChar(
                CP_UTF8, 0, data->variables->attributes[i]->string_value,
                (int)data->variables->attributes[i]->string_length, NULL, 0);
            new_size += (size_t)MultiByteToWideChar(
                CP_UTF8, 0, data->variables->attributes[i + 1]->string_value,
                (int)data->variables->attributes[i + 1]->string_length, NULL, 0);
            new_size += 2u;
        }
        pszNewEnvironment = (LPWSTR)rxvm_memory_calloc_bytes(
            rxvm_memory_current_worker(), new_size, sizeof(wchar_t));
        if (!pszNewEnvironment) {
            FreeEnvironmentStringsW(current_environment);
            windows_release_startup_attributes(&si);
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }
        memcpy(pszNewEnvironment, current_environment,
               parent_size * sizeof(wchar_t));
        FreeEnvironmentStringsW(current_environment);
        cursor = pszNewEnvironment + parent_size;
        for (i = 0; data->variables &&
                    i + 1 < data->variables->num_attributes; i += 2) {
            cursor += MultiByteToWideChar(
                CP_UTF8, 0, data->variables->attributes[i]->string_value,
                (int)data->variables->attributes[i]->string_length,
                cursor, (int)(new_size - (size_t)(cursor - pszNewEnvironment)));
            *cursor++ = L'=';
            cursor += MultiByteToWideChar(
                CP_UTF8, 0, data->variables->attributes[i + 1]->string_value,
                (int)data->variables->attributes[i + 1]->string_length,
                cursor, (int)(new_size - (size_t)(cursor - pszNewEnvironment)));
            *cursor++ = L'\0';
        }
        *cursor = L'\0';
    }

    /* Make filepath wide too */
    int filePathLength = MultiByteToWideChar(CP_UTF8, 0, data->file_path, -1, NULL, 0);
    if (filePathLength == 0) {
        // Handle the error here. Call GetLastError() to get the error code.
        windows_release_startup_attributes(&si);
        rxspawn_memory_free(pszNewEnvironment);
        CleanUp(data);
        return SHELLSPAWN_FAILURE;
    }

    // Allocate memory for the wide character string.
    wchar_t* wideFilePath = rxspawn_memory_alloc(
        filePathLength * sizeof(wchar_t));
    if (wideFilePath == NULL) {
        windows_release_startup_attributes(&si);
        rxspawn_memory_free(pszNewEnvironment);
        CleanUp(data);
        return SHELLSPAWN_FAILURE;
    }

    // Do the conversion.
    MultiByteToWideChar(CP_UTF8, 0, data->file_path, -1, wideFilePath, filePathLength);

    wchar_t* wideApplicationPath = NULL;
    wchar_t* wideWorkingDirectory = NULL;
    if (data->application_path) {
        int applicationPathLength = MultiByteToWideChar(CP_UTF8, 0, data->application_path, -1, NULL, 0);
        if (applicationPathLength == 0) {
            windows_release_startup_attributes(&si);
            rxspawn_memory_free(wideFilePath);
            rxspawn_memory_free(pszNewEnvironment);
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }

        wideApplicationPath = rxspawn_memory_alloc(
            applicationPathLength * sizeof(wchar_t));
        if (wideApplicationPath == NULL) {
            windows_release_startup_attributes(&si);
            rxspawn_memory_free(wideFilePath);
            rxspawn_memory_free(pszNewEnvironment);
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }
        MultiByteToWideChar(CP_UTF8, 0, data->application_path, -1, wideApplicationPath, applicationPathLength);
    }

    if (data->working_directory) {
        int workingDirectoryLength = MultiByteToWideChar(
            CP_UTF8, 0, data->working_directory, -1, NULL, 0);
        if (workingDirectoryLength == 0) {
            windows_release_startup_attributes(&si);
            rxspawn_memory_free(wideApplicationPath);
            rxspawn_memory_free(wideFilePath);
            rxspawn_memory_free(pszNewEnvironment);
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }
        wideWorkingDirectory = rxspawn_memory_alloc(
            (size_t)workingDirectoryLength * sizeof(wchar_t));
        if (!wideWorkingDirectory) {
            windows_release_startup_attributes(&si);
            rxspawn_memory_free(wideApplicationPath);
            rxspawn_memory_free(wideFilePath);
            rxspawn_memory_free(pszNewEnvironment);
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }
        MultiByteToWideChar(CP_UTF8, 0, data->working_directory, -1,
                            wideWorkingDirectory, workingDirectoryLength);
    }

    /* Controlled providers use a kill-on-close job and create suspended so a
     * descendant cannot escape before the direct child is assigned. */
    if (controlled_child) {
        data->ChildJob = CreateJobObjectW(NULL, NULL);
        ZeroMemory(&job_information, sizeof(job_information));
        job_information.BasicLimitInformation.LimitFlags =
                JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        if (!data->ChildJob || !SetInformationJobObject(
                data->ChildJob, JobObjectExtendedLimitInformation,
                &job_information, sizeof(job_information))) {
            windows_release_startup_attributes(&si);
            rxspawn_memory_free(wideApplicationPath);
            rxspawn_memory_free(wideWorkingDirectory);
            rxspawn_memory_free(wideFilePath);
            rxspawn_memory_free(pszNewEnvironment);
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }
        flags |= CREATE_SUSPENDED;
    }

    /* Start the child process */
    if (!CreateProcessW(wideApplicationPath,wideFilePath,NULL,NULL,TRUE,
                       flags,pszNewEnvironment,wideWorkingDirectory,
                       &si.StartupInfo,&data->ChildProcessInfo))
    {
        if (GetLastError() == 2) // File not found
        {
            windows_release_startup_attributes(&si);
            rxspawn_memory_free(wideApplicationPath);
            rxspawn_memory_free(wideWorkingDirectory);
            rxspawn_memory_free(wideFilePath);
            rxspawn_memory_free(pszNewEnvironment);
            CleanUp(data);
            return SHELLSPAWN_NOFOUND;
        }
        else
        {
            windows_release_startup_attributes(&si);
            rxspawn_memory_free(wideApplicationPath);
            rxspawn_memory_free(wideWorkingDirectory);
            rxspawn_memory_free(wideFilePath);
            rxspawn_memory_free(pszNewEnvironment);
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }
    }
    if (data->ChildJob) {
        if (!AssignProcessToJobObject(
                    data->ChildJob, data->ChildProcessInfo.hProcess) ||
            ResumeThread(data->ChildProcessInfo.hThread) == (DWORD)-1) {
            (void)TerminateJobObject(data->ChildJob, 130u);
            windows_release_startup_attributes(&si);
            rxspawn_memory_free(wideApplicationPath);
            rxspawn_memory_free(wideWorkingDirectory);
            rxspawn_memory_free(wideFilePath);
            rxspawn_memory_free(pszNewEnvironment);
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }
    }

    /* Cleanup */
    windows_release_startup_attributes(&si);
    rxspawn_memory_free(wideApplicationPath);
    rxspawn_memory_free(wideWorkingDirectory);
    rxspawn_memory_free(wideFilePath);
    rxspawn_memory_free(pszNewEnvironment);

    return 0;

#else

    if ((data->ChildProcessPID = fork()) == -1) {
        // Error("Failure spawn U33", errorText);
        data->ChildProcessPID = 0;
        CleanUp(data);
        return SHELLSPAWN_FAILURE;
    }

    if (data->ChildProcessPID != 0) { // Parent process owns the child group.
        if (setpgid(data->ChildProcessPID, data->ChildProcessPID) != 0 &&
            errno != EACCES && errno != ESRCH) {
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }
        return 0;
    }

    if (setpgid(0, 0) != 0) _exit(127);

    if (data->working_directory && chdir(data->working_directory) != 0) {
        static const char message[] = "Failure spawn working directory\n";
        (void)write(2, message, sizeof(message) - 1u);
        _exit(127);
    }

    /* Set Environmental Variables */
    int i;
    char *name;
    char *value;
    for (i = 0; !data->environment && data->variables &&
                i + 1 < data->variables->num_attributes; i += 2) {
        /* Variable Name */
        name = rxspawn_memory_alloc(
            data->variables->attributes[i]->string_length + 1u);
        memcpy(name, data->variables->attributes[i]->string_value, data->variables->attributes[i]->string_length);
        name[data->variables->attributes[i]->string_length] = 0;

        /* Uppercase it - following exported variables convention on posix */
        char *s = name;
        while (*s) {
            *s = (char)toupper(*s);
            s++;
        }

        /* Variable Value */
        value = rxspawn_memory_alloc(
            data->variables->attributes[i + 1]->string_length + 1u);
        memcpy(value, data->variables->attributes[i + 1]->string_value, data->variables->attributes[i + 1]->string_length);
        value[data->variables->attributes[i + 1]->string_length] = 0;

        /* Set/export variable */
        setenv(name, value,1);

        rxspawn_memory_free(value);
        rxspawn_memory_free(name);
    }

    // Close parent end of the pipes
    if (data->pInput && data->pInput->hWrite != -1) {
        close(data->pInput->hWrite);
        data->pInput->hWrite = -1;
    }
    if (data->pOutput && data->pOutput->hRead != -1) {
        close(data->pOutput->hRead);
        data->pOutput->hRead = -1;
    }
    if (data->pError && data->pError->hRead != -1) {
        close(data->pError->hRead);
        data->pError->hRead = -1;
    }

    /* Duplicate to replace standard streams */
    if (data->pInput && data->pInput->hRead != -1) {
        dup2(data->pInput->hRead, 0);
    }
    if (data->pOutput && data->pOutput->hWrite != -1) {
        dup2(data->pOutput->hWrite, 1);
    }
    if (data->pError && data->pError->hWrite != -1) {
        dup2(data->pError->hWrite, 2);
    }

    /* Set the handling for job control signals back to the default. */
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);

    // Execute the command
    if (data->environment) execve(data->file_path, data->argv, data->environment);
    else execv(data->file_path, data->argv);
    {
        static const char message[] = "Failure spawn launchChild\n";
        (void)write(2, message, sizeof(message) - 1u);
    }
    _exit(127);
#endif
}

#ifndef _WIN32
int ExeFound(char* exe)
{
    if(access(exe, X_OK) == 0) return 1;
    else return 0;
}
#endif
