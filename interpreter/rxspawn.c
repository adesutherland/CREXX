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

#include "rxvmintp.h"
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
#else
    int ChildProcessPID;
#endif
    int ChildProcessRC;
    char* buffer;
    char* file_path;
    char* application_path;
    char** argv;
    value* variables;
    value* crexx_bindings;
} SHELLDATA;

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
    REDIRECT_TRANSFER_OUTPUT_ARRAY = 4
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
static void collect_redirect_thread_context(REDIRECT *redirect);
static int join_redirect_thread(REDIRECT *redirect);
#ifndef _WIN32
static int ExeFound(char* exe);
static char *find_executable_in_path_list(const char *path_list, const char *exe);
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

static char *windows_search_executable(const char *name) {
    static const wchar_t *extensions[] = {NULL, L".exe", L".cmd", L".bat", NULL};
    wchar_t *wide_name;
    char *result;
    int i;

    if (!name || !*name) return NULL;
    wide_name = wide_from_utf8(name);
    if (!wide_name) return NULL;

    result = NULL;
    for (i = 0; extensions[i] || i == 0; i++) {
        DWORD needed;
        wchar_t *wide_path;
        DWORD copied;

        needed = SearchPathW(NULL, wide_name, extensions[i], 0, NULL, NULL);
        if (needed == 0) {
            if (!extensions[i]) continue;
            continue;
        }

        wide_path = rxspawn_memory_alloc(
            ((size_t)needed + 1u) * sizeof(wchar_t));
        if (!wide_path) break;
        copied = SearchPathW(NULL, wide_name, extensions[i], needed + 1, wide_path, NULL);
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
    return result;
}

static char *windows_resolve_application_path(const char *command) {
    char *token;
    char *path;

    token = windows_command_token(command);
    if (!token) return NULL;
    path = windows_search_executable(token);
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
static char *find_executable_in_path_list(const char *path_list, const char *exe) {
    const char *cursor = path_list;

    if (!cursor || !*cursor || !exe || !*exe) return NULL;

    while (cursor) {
        const char *next_colon = strchr(cursor, ':');
        size_t dir_length = next_colon ? (size_t)(next_colon - cursor) : strlen(cursor);
        size_t candidate_length = (dir_length ? dir_length : 1) + strlen(exe) + 2;
        char *candidate = rxspawn_memory_alloc(candidate_length);
        if (!candidate) return NULL;

        if (dir_length) {
            memcpy(candidate, cursor, dir_length);
            candidate[dir_length] = '\0';
        } else {
            strcpy(candidate, ".");
        }

        strcat(candidate, "/");
        strcat(candidate, exe);

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
            shell = find_executable_in_path_list(standard_path, "sh");
            rxspawn_memory_free(standard_path);
            if (shell) return shell;
        }
    }
#endif

    if (ExeFound("/bin/sh")) return copy_string("/bin/sh");

    shell = find_executable_in_path_list(getenv("PATH"), "sh");
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

static int crexxcmd_get_binding(void *userdata, const char *name, char **out_value) {
    SHELLDATA *data = (SHELLDATA *)userdata;
    value *bindings;
    size_t i;

    if (out_value) *out_value = NULL;
    if (!data || !data->crexx_bindings || !name) return 0;

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
    if (!data || !data->crexx_bindings || !name) return 0;

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
    if (!data || !data->crexx_bindings || !name || index == 0) return 0;

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

static void redirect_completion_destroy(REDIRECT_COMPLETION *completion) {
    if (!completion) return;
    redirect_completion_close_handle(completion);
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
        completion->transfer_mode == REDIRECT_TRANSFER_INPUT_ARRAY) return 0;
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
    value input_redirect;
    value output_redirect;
    value error_redirect;
    value output_value;
    value error_value;
    REDIRECT *pIn;
    REDIRECT *pOut;
    REDIRECT *pErr;
    int spawn_rc;
    char *spawn_error;

    if (out_text) *out_text = NULL;
    if (err_text) *err_text = NULL;
    if (error_text) *error_text = NULL;
    if (command_rc) *command_rc = 0;

    value_init(&input_redirect);
    value_init(&output_redirect);
    value_init(&error_redirect);
    value_init(&output_value);
    value_init(&error_value);

    if (parent_data && parent_data->pInput) {
        pIn = parent_data->pInput;
    } else {
        nullredr(&input_redirect);
        pIn = rxspawn_redirect_from_value(&input_redirect);
    }
    redr2str(&output_redirect, &output_value);
    redr2str(&error_redirect, &error_value);
    pOut = rxspawn_redirect_from_value(&output_redirect);
    pErr = rxspawn_redirect_from_value(&error_redirect);
    spawn_error = NULL;
    spawn_rc = shellspawn(command, pIn, pOut, pErr,
                          parent_data ? parent_data->variables : NULL,
                          NULL,
                          SHELLSPAWN_MODE_PATH,
                          command_rc,
                          &spawn_error);

    if (out_text) *out_text = copy_value_string(&output_value);
    if (err_text) *err_text = copy_value_string(&error_value);
    if (error_text && spawn_error) *error_text = copy_string_external(spawn_error);

    clear_value(&input_redirect);
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
    value input_redirect;
    value output_redirect;
    value error_redirect;
    value output_value;
    value error_value;
    REDIRECT *pIn;
    REDIRECT *pOut;
    REDIRECT *pErr;
    int spawn_rc;
    char *spawn_error;

    if (out_text) *out_text = NULL;
    if (err_text) *err_text = NULL;
    if (error_text) *error_text = NULL;
    if (command_rc) *command_rc = 0;

    value_init(&input_redirect);
    value_init(&output_redirect);
    value_init(&error_redirect);
    value_init(&output_value);
    value_init(&error_value);

    if (parent_data && parent_data->pInput) {
        pIn = parent_data->pInput;
    } else {
        nullredr(&input_redirect);
        pIn = rxspawn_redirect_from_value(&input_redirect);
    }
    redr2str(&output_redirect, &output_value);
    redr2str(&error_redirect, &error_value);
    pOut = rxspawn_redirect_from_value(&output_redirect);
    pErr = rxspawn_redirect_from_value(&error_redirect);
    spawn_error = NULL;
    spawn_rc = spawn_argv_capture(argv,
                                  argc,
                                  pIn,
                                  pOut,
                                  pErr,
                                  parent_data ? parent_data->variables : NULL,
                                  command_rc,
                                  &spawn_error);

    if (out_text) *out_text = copy_value_string(&output_value);
    if (err_text) *err_text = copy_value_string(&error_value);
    if (error_text && spawn_error) *error_text = copy_string_external(spawn_error);

    clear_value(&input_redirect);
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
    data.waitThreadRC = 0;
    data.variables = variables;
    data.crexx_bindings = crexx_bindings;

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
        const char *shell = mode == SHELLSPAWN_MODE_CONFIGURED_SHELL ? getenv("CREXX_ADDRESS_SHELL") : NULL;
        const char *shell_args = mode == SHELLSPAWN_MODE_CONFIGURED_SHELL ? getenv("CREXX_ADDRESS_SHELL_ARGS") : NULL;
        if (!shell || !*shell) shell = getenv("COMSPEC");
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
        data.application_path = windows_resolve_application_path(command);
        if (!data.application_path) {
            Error("Command not found", errorText);
            CleanUp(&data);
            return SHELLSPAWN_NOFOUND;
        }
    }
#else
    if (mode == SHELLSPAWN_MODE_SHELL || mode == SHELLSPAWN_MODE_CONFIGURED_SHELL) {
        const char *configured_shell = mode == SHELLSPAWN_MODE_CONFIGURED_SHELL ? getenv("CREXX_ADDRESS_SHELL") : NULL;
        const char *shell_args = mode == SHELLSPAWN_MODE_CONFIGURED_SHELL ? getenv("CREXX_ADDRESS_SHELL_ARGS") : NULL;
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
        if (ExeFound(base_name)) {
            data.file_path = rxspawn_memory_alloc(strlen(base_name) + 1u);
            strcpy(data.file_path, base_name);
            commandFound = 1;
        } else if (base_name[0] != '/') {
            data.file_path = find_executable_in_path_list(getenv("PATH"), base_name);
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
    data.waitThreadRC = 0;
    data.variables = variables;
    data.crexx_bindings = NULL;

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
        data.application_path = windows_search_executable(normalized_exe);
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
            if (ExeFound((char *)argv[0])) data.file_path = copy_string(argv[0]);
        } else {
            data.file_path = find_executable_in_path_list(getenv("PATH"), argv[0]);
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

    if (data.waitThreadRC) {
        appendTextOutput(errorText, data.waitThreadErrorText);
        CleanUp(&data);
        return SHELLSPAWN_FAILURE;
    }

    if (rc) *rc = (int)data.ChildProcessRC;
    CleanUp(&data);
    return SHELLSPAWN_OK;
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

/* Capture raw bytes only. Join publishes them to the receiver VM thread. */
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
        if (!capture_failed &&
            redirect_completion_append(completion, buffer, (size_t)bytes_read) != 0) {
            /* Continue draining so allocation failure cannot deadlock the child. */
            capture_failed = 1;
            completion->errorCode = 1;
            completion->errorSource = 4;
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
        if (!capture_failed &&
            redirect_completion_append(completion, buffer, (size_t)bytes_read) != 0) {
            /* Continue draining so allocation failure cannot deadlock the child. */
            capture_failed = 1;
            completion->errorCode = 1;
            completion->errorSource = 4;
        }
    }
#endif

    redirect_completion_close_handle(completion);
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

/* Write an immutable snapshot only; this thread has no VM state. */
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

    WriteToStdin(redirect, (char*)data, nBytes);
    if (redirect->errorCode != 0) return -1;

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

void CleanUp(SHELLDATA* data)
{
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

#ifdef _WIN32

    PROCESS_INFORMATION* pProcInfo = &(data->ChildProcessInfo);
    if (pProcInfo->hProcess) {
        TerminateProcess(pProcInfo->hProcess, 0);
        CloseHandle(pProcInfo->hProcess);
        pProcInfo->hProcess = NULL;
    }
    if (pProcInfo->hThread) {
        CloseHandle(pProcInfo->hThread);
        pProcInfo->hThread = NULL;
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
        if (kill(child_pid, SIGKILL) == -1 && errno != ESRCH) {
            data->waitThreadRC = 1;
        }
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

    // Wait for child process to exit
    dwWaitResult = WaitForSingleObject(data->ChildProcessInfo.hProcess, INFINITE);
    if (dwWaitResult == WAIT_OBJECT_0) {
        // The child process has terminated.
        DWORD process_rc;
        if (!GetExitCodeProcess(data->ChildProcessInfo.hProcess, &process_rc)) {
            // Error in GetExitCodeProcess.
            data->waitThreadRC = 1;
            Error("Failure spawn U43", &(data->waitThreadErrorText));
        }
        data->ChildProcessRC = (int)process_rc;
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

    do {
        w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
        if (w == -1)
        {
            if (errno == EINTR) continue;
            data->waitThreadRC = 1;
            Error("Failure spawn U43", &data->waitThreadErrorText);
            if (errno == ECHILD) data->ChildProcessPID = 0;
            break;
        }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    if (w != -1) {
        data->ChildProcessPID = 0;
        if (WIFEXITED(status)) {
            data->ChildProcessRC = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            data->ChildProcessRC = 128 + WTERMSIG(status);
        }
    }

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
int launchChild(SHELLDATA* data) {

#ifdef _WIN32

    // Launch the redirected command
    STARTUPINFOEXW si;
    SIZE_T attributeListSize;
    HANDLE inheritedHandles[3];
    int inheritedHandleCount;
    int useHandleList;
    int i;

    // Set up the start up info struct.
    ZeroMemory(&si, sizeof(STARTUPINFOEXW));
    si.StartupInfo.cb = sizeof(STARTUPINFOEXW);
    si.StartupInfo.dwFlags = STARTF_USESTDHANDLES;

    si.StartupInfo.hStdOutput = (data->pOutput && data->pOutput->hWrite != INVALID_HANDLE_VALUE) ? data->pOutput->hWrite : GetStdHandle(STD_OUTPUT_HANDLE);
    si.StartupInfo.hStdError = (data->pError && data->pError->hWrite != INVALID_HANDLE_VALUE) ? data->pError->hWrite : GetStdHandle(STD_ERROR_HANDLE);
    si.StartupInfo.hStdInput = (data->pInput && data->pInput->hRead != INVALID_HANDLE_VALUE) ? data->pInput->hRead : GetStdHandle(STD_INPUT_HANDLE);

    int flags = CREATE_UNICODE_ENVIRONMENT; // UTF16 Environment Variables
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
            DeleteProcThreadAttributeList(si.lpAttributeList);
            rxspawn_memory_free(si.lpAttributeList);
            si.lpAttributeList = NULL;
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }
        flags |= EXTENDED_STARTUPINFO_PRESENT;
    }

    /* Environment variables */
    LPWSTR pszCurrentEnvironment = GetEnvironmentStringsW();  // Get parent process's environment block.
    if (pszCurrentEnvironment == NULL) {
        // Handle error.
        if (si.lpAttributeList) {
            DeleteProcThreadAttributeList(si.lpAttributeList);
            rxspawn_memory_free(si.lpAttributeList);
        }
        CleanUp(data);
        return SHELLSPAWN_FAILURE;
    }

    // Calculate the size of the parent's environment block.
    LPWSTR pszTemp = pszCurrentEnvironment;
    while (*pszTemp) {
        pszTemp += wcslen(pszTemp) + 1;
    }
    size_t parentEnvironmentSize = pszTemp - pszCurrentEnvironment;

    // Calculate total length of the new environment block.
    size_t newEnvironmentSize = parentEnvironmentSize + 1; // +1 For the final extra '\0'
    if (data->variables) {
        for (i = 0; i + 1 < data->variables->num_attributes; i += 2) {
            newEnvironmentSize += MultiByteToWideChar(CP_UTF8, 0,
                                                      data->variables->attributes[i]->string_value,
                                                      (int) data->variables->attributes[i]->string_length, NULL,
                                                      0);
            newEnvironmentSize += MultiByteToWideChar(CP_UTF8, 0,
                                                      data->variables->attributes[i + 1]->string_value,
                                                      (int) data->variables->attributes[i + 1]->string_length, NULL,
                                                      0);
            newEnvironmentSize += 2;  // For the '=' and '\0'.
        }
    }
    newEnvironmentSize++;  // For the final extra '\0'.

    // Allocate the new environment block.
    LPWSTR pszNewEnvironment = (LPWSTR)rxvm_memory_calloc_bytes(
        rxvm_memory_current_worker(), newEnvironmentSize, sizeof(wchar_t));
    if (pszNewEnvironment == NULL) {
        // Handle memory allocation failure.
        if (si.lpAttributeList) {
            DeleteProcThreadAttributeList(si.lpAttributeList);
            rxspawn_memory_free(si.lpAttributeList);
        }
        FreeEnvironmentStringsW(pszCurrentEnvironment);
        CleanUp(data);
        return SHELLSPAWN_FAILURE;
    }

    // Copy the parent's environment into the new environment block.
    memcpy(pszNewEnvironment, pszCurrentEnvironment,
           parentEnvironmentSize * sizeof(wchar_t));

    FreeEnvironmentStringsW(pszCurrentEnvironment);

    // Add the custom variables at the end of the new environment block.
    LPWSTR pszCurrentVariable = pszNewEnvironment + parentEnvironmentSize;

    for (i = 0; data->variables && i + 1 < data->variables->num_attributes; i += 2) {

        // Assuming string_value is UTF-8 encoded
        int numChars = MultiByteToWideChar(CP_UTF8, 0,
                                           data->variables->attributes[i]->string_value,
                                           (int)data->variables->attributes[i]->string_length,
                                           pszCurrentVariable, (int)(newEnvironmentSize - (pszCurrentVariable - pszNewEnvironment)));

        pszCurrentVariable += numChars;
        *pszCurrentVariable++ = L'=';

        numChars = MultiByteToWideChar(CP_UTF8, 0,
                                       data->variables->attributes[i + 1]->string_value,
                                       (int)data->variables->attributes[i + 1]->string_length,
                                       pszCurrentVariable, (int)(newEnvironmentSize - (pszCurrentVariable - pszNewEnvironment)));

        pszCurrentVariable += numChars;
        *pszCurrentVariable++ = L'\0';
    }

    *pszCurrentVariable++ = L'\0';  // Add the final '\0'.

    /* Make filepath wide too */
    int filePathLength = MultiByteToWideChar(CP_UTF8, 0, data->file_path, -1, NULL, 0);
    if (filePathLength == 0) {
        // Handle the error here. Call GetLastError() to get the error code.
        if (si.lpAttributeList) {
            DeleteProcThreadAttributeList(si.lpAttributeList);
            rxspawn_memory_free(si.lpAttributeList);
        }
        rxspawn_memory_free(pszNewEnvironment);
        CleanUp(data);
        return SHELLSPAWN_FAILURE;
    }

    // Allocate memory for the wide character string.
    wchar_t* wideFilePath = rxspawn_memory_alloc(
        filePathLength * sizeof(wchar_t));
    if (wideFilePath == NULL) {
        if (si.lpAttributeList) {
            DeleteProcThreadAttributeList(si.lpAttributeList);
            rxspawn_memory_free(si.lpAttributeList);
        }
        rxspawn_memory_free(pszNewEnvironment);
        CleanUp(data);
        return SHELLSPAWN_FAILURE;
    }

    // Do the conversion.
    MultiByteToWideChar(CP_UTF8, 0, data->file_path, -1, wideFilePath, filePathLength);

    wchar_t* wideApplicationPath = NULL;
    if (data->application_path) {
        int applicationPathLength = MultiByteToWideChar(CP_UTF8, 0, data->application_path, -1, NULL, 0);
        if (applicationPathLength == 0) {
            if (si.lpAttributeList) {
                DeleteProcThreadAttributeList(si.lpAttributeList);
                rxspawn_memory_free(si.lpAttributeList);
            }
            rxspawn_memory_free(wideFilePath);
            rxspawn_memory_free(pszNewEnvironment);
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }

        wideApplicationPath = rxspawn_memory_alloc(
            applicationPathLength * sizeof(wchar_t));
        if (wideApplicationPath == NULL) {
            if (si.lpAttributeList) {
                DeleteProcThreadAttributeList(si.lpAttributeList);
                rxspawn_memory_free(si.lpAttributeList);
            }
            rxspawn_memory_free(wideFilePath);
            rxspawn_memory_free(pszNewEnvironment);
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }
        MultiByteToWideChar(CP_UTF8, 0, data->application_path, -1, wideApplicationPath, applicationPathLength);
    }

    /* Start the child process */
    if (!CreateProcessW(wideApplicationPath,wideFilePath,NULL,NULL,TRUE,
                       flags,pszNewEnvironment,NULL,&si.StartupInfo,&data->ChildProcessInfo))
    {
        if (GetLastError() == 2) // File not found
        {
            if (si.lpAttributeList) {
                DeleteProcThreadAttributeList(si.lpAttributeList);
                rxspawn_memory_free(si.lpAttributeList);
            }
            rxspawn_memory_free(wideApplicationPath);
            rxspawn_memory_free(wideFilePath);
            rxspawn_memory_free(pszNewEnvironment);
            CleanUp(data);
            return SHELLSPAWN_NOFOUND;
        }
        else
        {
            if (si.lpAttributeList) {
                DeleteProcThreadAttributeList(si.lpAttributeList);
                rxspawn_memory_free(si.lpAttributeList);
            }
            rxspawn_memory_free(wideApplicationPath);
            rxspawn_memory_free(wideFilePath);
            rxspawn_memory_free(pszNewEnvironment);
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }
    }

    /* Cleanup */
    if (si.lpAttributeList) {
        DeleteProcThreadAttributeList(si.lpAttributeList);
        rxspawn_memory_free(si.lpAttributeList);
    }
    rxspawn_memory_free(wideApplicationPath);
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

    if (data->ChildProcessPID != 0) // Parent Process
        return 0;

    /* Set Environmental Variables */
    int i;
    char *name;
    char *value;
    for (i = 0; data->variables && i + 1 < data->variables->num_attributes; i += 2) {
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
    execv(data->file_path, data->argv);
    perror("Failure spawn launchChild");
    exit(-1);
#endif
}

#ifndef _WIN32
int ExeFound(char* exe)
{
    if(access(exe, X_OK) == 0) return 1;
    else return 0;
}
#endif
