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

#include "rxcrexxcmd.h"

#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <direct.h>
#include <io.h>
#include <process.h>
#include <sys/stat.h>
#include <sys/utime.h>
#define RX_GETCWD _getcwd
#define RX_CHDIR _chdir
#define RX_MKDIR(path) _mkdir(path)
#define RX_RMDIR _rmdir
#define RX_UNLINK _unlink
#define RX_STAT _stat
#define RX_UTIME _utime
#define RX_PATH_SEP '\\'
#define RX_PATH_LIST_SEP ';'
typedef struct _stat rx_stat_t;
#else
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>
#define RX_GETCWD getcwd
#define RX_CHDIR chdir
#define RX_MKDIR(path) mkdir((path), 0777)
#define RX_RMDIR rmdir
#define RX_UNLINK unlink
#define RX_STAT stat
#define RX_UTIME utime
#define RX_PATH_SEP '/'
#define RX_PATH_LIST_SEP ':'
typedef struct stat rx_stat_t;
extern char **environ;
#endif

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define RXCREXXCMD_RC_USAGE 2
#define RXCREXXCMD_RC_NOT_FOUND 4
#define RXCREXXCMD_RC_ERROR 8
#define RXCREXXCMD_RC_UNKNOWN 127
#define RXCREXXCMD_MAX_STACK 64

typedef struct rxcrexxcmd_context {
    const rxcrexxcmd_io *io;
} rxcrexxcmd_context;

typedef struct rxcrexxcmd_args {
    int argc;
    char **argv;
} rxcrexxcmd_args;

typedef int (*rxcrexxcmd_handler)(rxcrexxcmd_context *ctx,
                                  const char *command,
                                  rxcrexxcmd_args *args);

typedef struct rxcrexxcmd_entry {
    const char *name;
    rxcrexxcmd_handler handler;
} rxcrexxcmd_entry;

static char *directory_stack[RXCREXXCMD_MAX_STACK];
static int directory_stack_count = 0;

static int cmd_help(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_echo(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_pwd(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_cd(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_pushd(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_popd(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_ls(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_exists(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_stat(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_mkdir(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_rmdir(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_rm(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_copy(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_move(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_touch(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_cat(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_head(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_tail(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_lines(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_write(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_append(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_which(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_now(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_sleep(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_platform(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_env(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_setenv(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_unsetenv(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_pid(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_ps(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_kill(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_resolve(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_tcp(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_batch(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);
static int cmd_run(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args);

static const rxcrexxcmd_entry command_table[] = {
    {"help", cmd_help},
    {"echo", cmd_echo},
    {"pwd", cmd_pwd},
    {"cd", cmd_cd},
    {"pushd", cmd_pushd},
    {"popd", cmd_popd},
    {"ls", cmd_ls},
    {"dir", cmd_ls},
    {"exists", cmd_exists},
    {"stat", cmd_stat},
    {"mkdir", cmd_mkdir},
    {"rmdir", cmd_rmdir},
    {"rm", cmd_rm},
    {"del", cmd_rm},
    {"copy", cmd_copy},
    {"cp", cmd_copy},
    {"move", cmd_move},
    {"mv", cmd_move},
    {"rename", cmd_move},
    {"touch", cmd_touch},
    {"cat", cmd_cat},
    {"type", cmd_cat},
    {"head", cmd_head},
    {"tail", cmd_tail},
    {"lines", cmd_lines},
    {"write", cmd_write},
    {"append", cmd_append},
    {"which", cmd_which},
    {"now", cmd_now},
    {"date", cmd_now},
    {"sleep", cmd_sleep},
    {"platform", cmd_platform},
    {"os", cmd_platform},
    {"env", cmd_env},
    {"setenv", cmd_setenv},
    {"unsetenv", cmd_unsetenv},
    {"pid", cmd_pid},
    {"ps", cmd_ps},
    {"kill", cmd_kill},
    {"resolve", cmd_resolve},
    {"tcp", cmd_tcp},
    {"batch", cmd_batch},
    {"run", cmd_run},
    {NULL, NULL}
};

static int rx_stricmp(const char *left, const char *right) {
    unsigned char lc;
    unsigned char rc;

    if (!left) left = "";
    if (!right) right = "";

    while (*left || *right) {
        lc = (unsigned char)tolower((unsigned char)*left);
        rc = (unsigned char)tolower((unsigned char)*right);
        if (lc != rc) return (int)lc - (int)rc;
        if (*left) left++;
        if (*right) right++;
    }

    return 0;
}

static char *rx_strdup(const char *text) {
    size_t length;
    char *copy;

    if (!text) text = "";
    length = strlen(text);
    copy = (char *)malloc(length + 1);
    if (!copy) return NULL;
    memcpy(copy, text, length + 1);
    return copy;
}

static char *rx_strndup(const char *text, size_t length) {
    char *copy;

    copy = (char *)malloc(length + 1);
    if (!copy) return NULL;
    if (length) memcpy(copy, text, length);
    copy[length] = '\0';
    return copy;
}

static char *rx_join_path(const char *left, const char *right) {
    size_t left_len;
    size_t right_len;
    int needs_sep;
    char *path;

    if (!left || !*left) return rx_strdup(right ? right : "");
    if (!right || !*right) return rx_strdup(left);

    left_len = strlen(left);
    right_len = strlen(right);
    needs_sep = left[left_len - 1] != '/' && left[left_len - 1] != '\\';
    path = (char *)malloc(left_len + (needs_sep ? 1 : 0) + right_len + 1);
    if (!path) return NULL;

    memcpy(path, left, left_len);
    if (needs_sep) path[left_len++] = RX_PATH_SEP;
    memcpy(path + left_len, right, right_len + 1);
    return path;
}

static int append_bytes(char **text, size_t *length, const char *bytes, size_t byte_count) {
    char *new_text;

    if (!bytes) byte_count = 0;
    if (byte_count > ((size_t)-1) - *length - 1) return -1;

    new_text = (char *)realloc(*text, *length + byte_count + 1);
    if (!new_text) return -1;

    if (byte_count) memcpy(new_text + *length, bytes, byte_count);
    *length += byte_count;
    new_text[*length] = '\0';
    *text = new_text;
    return 0;
}

static int ctx_write(rxcrexxcmd_context *ctx, int is_error, const char *text, size_t length) {
    rxcrexxcmd_write_fn writer;
    FILE *fallback;

    if (!text) text = "";
    writer = NULL;
    if (ctx && ctx->io) writer = is_error ? ctx->io->write_error : ctx->io->write_output;
    if (writer) return writer(ctx->io->userdata, text, length);

    fallback = is_error ? stderr : stdout;
    return fwrite(text, 1, length, fallback) == length ? 0 : -1;
}

static int ctx_puts(rxcrexxcmd_context *ctx, const char *text) {
    return ctx_write(ctx, 0, text, text ? strlen(text) : 0);
}

static int ctx_putln(rxcrexxcmd_context *ctx, const char *text) {
    if (ctx_write(ctx, 0, text, text ? strlen(text) : 0) != 0) return -1;
    return ctx_write(ctx, 0, "\n", 1);
}

static int ctx_errln(rxcrexxcmd_context *ctx, const char *text) {
    if (ctx_write(ctx, 1, text, text ? strlen(text) : 0) != 0) return -1;
    return ctx_write(ctx, 1, "\n", 1);
}

static int ctx_printf(rxcrexxcmd_context *ctx, const char *format, ...) {
    va_list ap;
    va_list ap2;
    int needed;
    char stack_buffer[512];
    char *heap_buffer;
    int result;

    va_start(ap, format);
    va_copy(ap2, ap);
    needed = vsnprintf(stack_buffer, sizeof(stack_buffer), format, ap);
    va_end(ap);

    if (needed < 0) {
        va_end(ap2);
        return -1;
    }

    if ((size_t)needed < sizeof(stack_buffer)) {
        va_end(ap2);
        return ctx_write(ctx, 0, stack_buffer, (size_t)needed);
    }

    heap_buffer = (char *)malloc((size_t)needed + 1);
    if (!heap_buffer) {
        va_end(ap2);
        return -1;
    }

    vsnprintf(heap_buffer, (size_t)needed + 1, format, ap2);
    va_end(ap2);
    result = ctx_write(ctx, 0, heap_buffer, (size_t)needed);
    free(heap_buffer);
    return result;
}

static int ctx_errorf(rxcrexxcmd_context *ctx, const char *format, ...) {
    va_list ap;
    va_list ap2;
    int needed;
    char stack_buffer[512];
    char *heap_buffer;
    int result;

    va_start(ap, format);
    va_copy(ap2, ap);
    needed = vsnprintf(stack_buffer, sizeof(stack_buffer), format, ap);
    va_end(ap);

    if (needed < 0) {
        va_end(ap2);
        return -1;
    }

    if ((size_t)needed < sizeof(stack_buffer)) {
        va_end(ap2);
        if (ctx_write(ctx, 1, stack_buffer, (size_t)needed) != 0) return -1;
        return ctx_write(ctx, 1, "\n", 1);
    }

    heap_buffer = (char *)malloc((size_t)needed + 1);
    if (!heap_buffer) {
        va_end(ap2);
        return -1;
    }

    vsnprintf(heap_buffer, (size_t)needed + 1, format, ap2);
    va_end(ap2);
    result = ctx_write(ctx, 1, heap_buffer, (size_t)needed);
    if (result == 0) result = ctx_write(ctx, 1, "\n", 1);
    free(heap_buffer);
    return result;
}

static void free_args(rxcrexxcmd_args *args) {
    int i;

    if (!args || !args->argv) return;
    for (i = 0; i < args->argc; i++) free(args->argv[i]);
    free(args->argv);
    args->argv = NULL;
    args->argc = 0;
}

static int push_arg(rxcrexxcmd_args *args, const char *text, size_t length) {
    char **new_argv;
    char *copy;

    copy = rx_strndup(text, length);
    if (!copy) return -1;

    new_argv = (char **)realloc(args->argv, sizeof(char *) * (size_t)(args->argc + 1));
    if (!new_argv) {
        free(copy);
        return -1;
    }

    args->argv = new_argv;
    args->argv[args->argc++] = copy;
    return 0;
}

static int token_append(char **token, size_t *length, size_t *capacity, char ch) {
    char *new_token;
    size_t new_capacity;

    if (*length + 1 >= *capacity) {
        new_capacity = *capacity ? *capacity * 2 : 32;
        new_token = (char *)realloc(*token, new_capacity);
        if (!new_token) return -1;
        *token = new_token;
        *capacity = new_capacity;
    }

    (*token)[(*length)++] = ch;
    return 0;
}

static int parse_args(const char *command, rxcrexxcmd_args *args, char **diagnostic) {
    const char *cursor;
    char quote;
    char *token;
    size_t token_len;
    size_t token_cap;

    args->argc = 0;
    args->argv = NULL;
    if (diagnostic) *diagnostic = NULL;
    if (!command) return 0;

    cursor = command;
    while (*cursor) {
        while (*cursor && isspace((unsigned char)*cursor)) cursor++;
        if (!*cursor) break;

        token = NULL;
        token_len = 0;
        token_cap = 0;
        quote = 0;

        while (*cursor) {
            if (quote) {
                if (*cursor == quote) {
                    quote = 0;
                    cursor++;
                    continue;
                }
                if (*cursor == '\\' && cursor[1]) cursor++;
                if (token_append(&token, &token_len, &token_cap, *cursor++) != 0) {
                    free(token);
                    return -1;
                }
                continue;
            }

            if (isspace((unsigned char)*cursor)) break;
            if (*cursor == '"' || *cursor == '\'') {
                quote = *cursor++;
                continue;
            }
            if (*cursor == '\\' && cursor[1]) cursor++;
            if (token_append(&token, &token_len, &token_cap, *cursor++) != 0) {
                free(token);
                return -1;
            }
        }

        if (quote) {
            free(token);
            if (diagnostic) *diagnostic = rx_strdup("unterminated quoted string");
            free_args(args);
            return RXCREXXCMD_RC_USAGE;
        }

        if (push_arg(args, token ? token : "", token_len) != 0) {
            free(token);
            free_args(args);
            return -1;
        }
        free(token);
    }

    return 0;
}

static char *join_args(rxcrexxcmd_args *args, int start) {
    size_t length;
    int i;
    char *text;

    if (!args || start >= args->argc) return rx_strdup("");

    length = 0;
    for (i = start; i < args->argc; i++) {
        if (i > start) length++;
        length += strlen(args->argv[i]);
    }

    text = (char *)malloc(length + 1);
    if (!text) return NULL;
    text[0] = '\0';

    for (i = start; i < args->argc; i++) {
        if (i > start) strcat(text, " ");
        strcat(text, args->argv[i]);
    }

    return text;
}

static const char *command_tail_after_first_word(const char *command) {
    const char *cursor;
    char quote;

    if (!command) return "";
    cursor = command;
    while (*cursor && isspace((unsigned char)*cursor)) cursor++;

    quote = 0;
    while (*cursor) {
        if (quote) {
            if (*cursor == quote) quote = 0;
            cursor++;
            continue;
        }
        if (*cursor == '"' || *cursor == '\'') {
            quote = *cursor++;
            continue;
        }
        if (isspace((unsigned char)*cursor)) break;
        cursor++;
    }

    while (*cursor && isspace((unsigned char)*cursor)) cursor++;
    return cursor;
}

static int is_shell_operator_token(const char *text) {
    return text &&
           (strcmp(text, ";") == 0 ||
            strcmp(text, "&&") == 0 ||
            strcmp(text, "||") == 0 ||
            strcmp(text, "|") == 0);
}

static const char *default_home(void) {
#ifdef _WIN32
    const char *home;
    static char combined[MAX_PATH * 2];

    home = getenv("USERPROFILE");
    if (home && *home) return home;

    if (getenv("HOMEDRIVE") && getenv("HOMEPATH")) {
        snprintf(combined, sizeof(combined), "%s%s", getenv("HOMEDRIVE"), getenv("HOMEPATH"));
        return combined;
    }

    return ".";
#else
    const char *home = getenv("HOME");
    return (home && *home) ? home : ".";
#endif
}

static char *current_directory(void) {
    size_t size;
    char *buffer;

    size = 256;
    for (;;) {
        buffer = (char *)malloc(size);
        if (!buffer) return NULL;
        if (RX_GETCWD(buffer, (int)size)) return buffer;
        free(buffer);
        if (errno != ERANGE) return NULL;
        if (size > ((size_t)-1) / 2) return NULL;
        size *= 2;
    }
}

static int path_exists(const char *path, rx_stat_t *st) {
    rx_stat_t local_st;

    if (!st) st = &local_st;
    return path && RX_STAT(path, st) == 0;
}

static int is_directory_stat(const rx_stat_t *st) {
#ifdef _WIN32
    return st && (st->st_mode & _S_IFDIR) != 0;
#else
    return st && S_ISDIR(st->st_mode);
#endif
}

static int is_regular_stat(const rx_stat_t *st) {
#ifdef _WIN32
    return st && (st->st_mode & _S_IFREG) != 0;
#else
    return st && S_ISREG(st->st_mode);
#endif
}

static int make_directory_recursive(const char *path) {
    char *copy;
    char *cursor;
    int rc;

    if (!path || !*path) return -1;
    copy = rx_strdup(path);
    if (!copy) return -1;

    cursor = copy;
#ifdef _WIN32
    if (isalpha((unsigned char)cursor[0]) && cursor[1] == ':') cursor += 2;
#endif
    while (*cursor == '/' || *cursor == '\\') cursor++;

    for (; *cursor; cursor++) {
        if (*cursor != '/' && *cursor != '\\') continue;
        *cursor = '\0';
        if (*copy && !path_exists(copy, NULL) && RX_MKDIR(copy) != 0) {
            free(copy);
            return -1;
        }
        *cursor = RX_PATH_SEP;
    }

    rc = 0;
    if (!path_exists(copy, NULL) && RX_MKDIR(copy) != 0) rc = -1;
    free(copy);
    return rc;
}

static int copy_file(const char *source, const char *target) {
    FILE *in;
    FILE *out;
    char buffer[8192];
    size_t nread;
    int rc;

    in = fopen(source, "rb");
    if (!in) return -1;
    out = fopen(target, "wb");
    if (!out) {
        fclose(in);
        return -1;
    }

    rc = 0;
    while ((nread = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        if (fwrite(buffer, 1, nread, out) != nread) {
            rc = -1;
            break;
        }
    }
    if (ferror(in)) rc = -1;
    if (fclose(out) != 0) rc = -1;
    fclose(in);
    return rc;
}

static int remove_path_recursive(const char *path) {
    rx_stat_t st;
    char *child;
    int rc;

    if (!path_exists(path, &st)) return -1;
    if (!is_directory_stat(&st)) return RX_UNLINK(path);

#ifdef _WIN32
    {
        WIN32_FIND_DATAA entry;
        HANDLE handle;
        char *pattern;

        pattern = rx_join_path(path, "*");
        if (!pattern) return -1;
        handle = FindFirstFileA(pattern, &entry);
        free(pattern);
        if (handle == INVALID_HANDLE_VALUE) return RX_RMDIR(path);

        rc = 0;
        do {
            if (strcmp(entry.cFileName, ".") == 0 || strcmp(entry.cFileName, "..") == 0) continue;
            child = rx_join_path(path, entry.cFileName);
            if (!child) {
                rc = -1;
                break;
            }
            if (remove_path_recursive(child) != 0) rc = -1;
            free(child);
            if (rc != 0) break;
        } while (FindNextFileA(handle, &entry));
        FindClose(handle);
        if (rc != 0) return rc;
    }
#else
    {
        DIR *dir;
        struct dirent *entry;

        dir = opendir(path);
        if (!dir) return -1;

        rc = 0;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            child = rx_join_path(path, entry->d_name);
            if (!child) {
                rc = -1;
                break;
            }
            if (remove_path_recursive(child) != 0) rc = -1;
            free(child);
            if (rc != 0) break;
        }
        closedir(dir);
        if (rc != 0) return rc;
    }
#endif

    return RX_RMDIR(path);
}

static int list_directory(rxcrexxcmd_context *ctx, const char *path) {
#ifdef _WIN32
    WIN32_FIND_DATAA entry;
    HANDLE handle;
    char *pattern;

    pattern = rx_join_path(path, "*");
    if (!pattern) return -1;
    handle = FindFirstFileA(pattern, &entry);
    free(pattern);
    if (handle == INVALID_HANDLE_VALUE) return -1;

    do {
        if (strcmp(entry.cFileName, ".") == 0 || strcmp(entry.cFileName, "..") == 0) continue;
        if (ctx_putln(ctx, entry.cFileName) != 0) {
            FindClose(handle);
            return -1;
        }
    } while (FindNextFileA(handle, &entry));

    FindClose(handle);
    return 0;
#else
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (!dir) return -1;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        if (ctx_putln(ctx, entry->d_name) != 0) {
            closedir(dir);
            return -1;
        }
    }

    closedir(dir);
    return 0;
#endif
}

static const char *os_name(void) {
#ifdef _WIN32
    return "windows";
#elif defined(__APPLE__)
    return "macos";
#elif defined(__linux__)
    return "linux";
#elif defined(__unix__)
    return "unix";
#else
    return "unknown";
#endif
}

static const char *architecture_name(void) {
#if defined(__x86_64__) || defined(_M_X64)
    return "x86_64";
#elif defined(__aarch64__) || defined(_M_ARM64)
    return "arm64";
#elif defined(__arm__) || defined(_M_ARM)
    return "arm";
#elif defined(__i386__) || defined(_M_IX86)
    return "x86";
#else
    return "unknown";
#endif
}

static int command_found_at(const char *path) {
    rx_stat_t st;

    if (!path_exists(path, &st) || is_directory_stat(&st)) return 0;
#ifdef _WIN32
    return 1;
#else
    return access(path, X_OK) == 0;
#endif
}

static char *find_executable(const char *name) {
    const char *path_list;
    const char *cursor;
    const char *next;
    size_t dir_len;
    char *dir;
    char *candidate;
#ifdef _WIN32
    static const char *extensions[] = {"", ".exe", ".cmd", ".bat", NULL};
    int ext_index;
#endif

    if (!name || !*name) return NULL;
    if (strchr(name, '/') || strchr(name, '\\')) {
        if (command_found_at(name)) return rx_strdup(name);
#ifdef _WIN32
        for (ext_index = 1; extensions[ext_index]; ext_index++) {
            candidate = (char *)malloc(strlen(name) + strlen(extensions[ext_index]) + 1);
            if (!candidate) return NULL;
            strcpy(candidate, name);
            strcat(candidate, extensions[ext_index]);
            if (command_found_at(candidate)) return candidate;
            free(candidate);
        }
#endif
        return NULL;
    }

    path_list = getenv("PATH");
    if (!path_list || !*path_list) return NULL;

    cursor = path_list;
    while (cursor) {
        next = strchr(cursor, RX_PATH_LIST_SEP);
        dir_len = next ? (size_t)(next - cursor) : strlen(cursor);
        dir = dir_len ? rx_strndup(cursor, dir_len) : rx_strdup(".");
        if (!dir) return NULL;

#ifdef _WIN32
        for (ext_index = 0; extensions[ext_index]; ext_index++) {
            char *with_ext;
            with_ext = (char *)malloc(strlen(name) + strlen(extensions[ext_index]) + 1);
            if (!with_ext) {
                free(dir);
                return NULL;
            }
            strcpy(with_ext, name);
            strcat(with_ext, extensions[ext_index]);
            candidate = rx_join_path(dir, with_ext);
            free(with_ext);
            if (!candidate) {
                free(dir);
                return NULL;
            }
            if (command_found_at(candidate)) {
                free(dir);
                return candidate;
            }
            free(candidate);
        }
#else
        candidate = rx_join_path(dir, name);
        if (!candidate) {
            free(dir);
            return NULL;
        }
        if (command_found_at(candidate)) {
            free(dir);
            return candidate;
        }
        free(candidate);
#endif

        free(dir);
        cursor = next ? next + 1 : NULL;
    }

    return NULL;
}

static int read_stream_to_output(rxcrexxcmd_context *ctx, FILE *file) {
    char buffer[8192];
    size_t nread;

    while ((nread = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (ctx_write(ctx, 0, buffer, nread) != 0) return -1;
    }
    return ferror(file) ? -1 : 0;
}

static int read_input_text(rxcrexxcmd_context *ctx, char **text, size_t *length) {
    if (text) *text = NULL;
    if (length) *length = 0;
    if (!ctx || !ctx->io || !ctx->io->read_input) return 0;
    return ctx->io->read_input(ctx->io->userdata, text, length);
}

static int command_usage(rxcrexxcmd_context *ctx, const char *usage) {
    ctx_errorf(ctx, "usage: %s", usage);
    return RXCREXXCMD_RC_USAGE;
}

static int cmd_help(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    (void)command;
    (void)args;

    ctx_putln(ctx, "CREXX command environment");
    ctx_putln(ctx, "commands:");
    ctx_putln(ctx, "  help");
    ctx_putln(ctx, "  echo [text...]");
    ctx_putln(ctx, "  pwd | cd [path] | pushd path | popd");
    ctx_putln(ctx, "  ls [path] | dir [path] | exists path... | stat path...");
    ctx_putln(ctx, "  mkdir [-p] path... | rmdir path... | rm [-r] path... | del [-r] path...");
    ctx_putln(ctx, "  copy source target | move source target | touch path...");
    ctx_putln(ctx, "  cat path... | head [-n count] path | tail [-n count] path | lines [path]");
    ctx_putln(ctx, "  write path text... | append path text...");
    ctx_putln(ctx, "  which command | now [local|utc] | date [local|utc] | sleep seconds");
    ctx_putln(ctx, "  platform | os | env [name] | setenv name value | unsetenv name");
    ctx_putln(ctx, "  pid | ps [pid] | kill pid [signal]");
    ctx_putln(ctx, "  resolve host | tcp host port | batch | run command...");
    return 0;
}

static int cmd_echo(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    char *text;
    int rc;

    (void)command;
    text = join_args(args, 1);
    if (!text) return RXCREXXCMD_RC_ERROR;
    rc = ctx_putln(ctx, text) == 0 ? 0 : RXCREXXCMD_RC_ERROR;
    free(text);
    return rc;
}

static int cmd_pwd(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    char *cwd;
    int rc;

    (void)command;
    if (args->argc != 1) return command_usage(ctx, "pwd");
    cwd = current_directory();
    if (!cwd) {
        ctx_errorf(ctx, "pwd: %s", strerror(errno));
        return RXCREXXCMD_RC_ERROR;
    }
    rc = ctx_putln(ctx, cwd) == 0 ? 0 : RXCREXXCMD_RC_ERROR;
    free(cwd);
    return rc;
}

static int cmd_cd(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    const char *path;

    (void)command;
    if (args->argc > 2) return command_usage(ctx, "cd [path]");
    path = args->argc == 1 ? default_home() : args->argv[1];
    if (RX_CHDIR(path) != 0) {
        ctx_errorf(ctx, "cd: %s: %s", path, strerror(errno));
        return RXCREXXCMD_RC_NOT_FOUND;
    }
    return 0;
}

static int cmd_pushd(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    char *cwd;
    char *new_cwd;
    int rc;

    (void)command;
    if (args->argc != 2) return command_usage(ctx, "pushd path");
    if (directory_stack_count >= RXCREXXCMD_MAX_STACK) {
        ctx_errln(ctx, "pushd: directory stack full");
        return RXCREXXCMD_RC_ERROR;
    }

    cwd = current_directory();
    if (!cwd) {
        ctx_errorf(ctx, "pushd: %s", strerror(errno));
        return RXCREXXCMD_RC_ERROR;
    }

    if (RX_CHDIR(args->argv[1]) != 0) {
        ctx_errorf(ctx, "pushd: %s: %s", args->argv[1], strerror(errno));
        free(cwd);
        return RXCREXXCMD_RC_NOT_FOUND;
    }

    directory_stack[directory_stack_count++] = cwd;
    new_cwd = current_directory();
    if (!new_cwd) return RXCREXXCMD_RC_ERROR;
    rc = ctx_putln(ctx, new_cwd) == 0 ? 0 : RXCREXXCMD_RC_ERROR;
    free(new_cwd);
    return rc;
}

static int cmd_popd(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    char *target;
    int rc;

    (void)command;
    if (args->argc != 1) return command_usage(ctx, "popd");
    if (directory_stack_count == 0) {
        ctx_errln(ctx, "popd: directory stack empty");
        return RXCREXXCMD_RC_ERROR;
    }

    target = directory_stack[--directory_stack_count];
    directory_stack[directory_stack_count] = NULL;
    rc = RX_CHDIR(target);
    if (rc != 0) {
        ctx_errorf(ctx, "popd: %s: %s", target, strerror(errno));
        free(target);
        return RXCREXXCMD_RC_NOT_FOUND;
    }
    free(target);
    return cmd_pwd(ctx, command, args);
}

static int cmd_ls(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    const char *path;

    (void)command;
    if (args->argc > 2) return command_usage(ctx, "ls [path]");
    path = args->argc == 1 ? "." : args->argv[1];
    if (list_directory(ctx, path) != 0) {
        ctx_errorf(ctx, "ls: %s: %s", path, strerror(errno));
        return RXCREXXCMD_RC_NOT_FOUND;
    }
    return 0;
}

static int cmd_exists(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    int i;
    int all_exist;

    (void)command;
    if (args->argc < 2) return command_usage(ctx, "exists path...");
    all_exist = 1;
    for (i = 1; i < args->argc; i++) {
        int exists = path_exists(args->argv[i], NULL);
        if (!exists) all_exist = 0;
        if (ctx_printf(ctx, "%d %s\n", exists ? 1 : 0, args->argv[i]) != 0) return RXCREXXCMD_RC_ERROR;
    }
    return all_exist ? 0 : RXCREXXCMD_RC_NOT_FOUND;
}

static const char *type_name_for_stat(const rx_stat_t *st) {
    if (is_directory_stat(st)) return "directory";
    if (is_regular_stat(st)) return "file";
    return "other";
}

static int cmd_stat(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    int i;
    int rc;

    (void)command;
    if (args->argc < 2) return command_usage(ctx, "stat path...");
    rc = 0;
    for (i = 1; i < args->argc; i++) {
        rx_stat_t st;
        if (!path_exists(args->argv[i], &st)) {
            ctx_errorf(ctx, "stat: %s: %s", args->argv[i], strerror(errno));
            rc = RXCREXXCMD_RC_NOT_FOUND;
            continue;
        }
        if (ctx_printf(ctx, "%s type=%s size=%lld mtime=%lld\n",
                       args->argv[i],
                       type_name_for_stat(&st),
                       (long long)st.st_size,
                       (long long)st.st_mtime) != 0) return RXCREXXCMD_RC_ERROR;
    }
    return rc;
}

static int cmd_mkdir(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    int recursive;
    int start;
    int i;
    int rc;

    (void)command;
    if (args->argc < 2) return command_usage(ctx, "mkdir [-p] path...");
    recursive = args->argc > 1 && strcmp(args->argv[1], "-p") == 0;
    start = recursive ? 2 : 1;
    if (start >= args->argc) return command_usage(ctx, "mkdir [-p] path...");

    rc = 0;
    for (i = start; i < args->argc; i++) {
        if ((recursive ? make_directory_recursive(args->argv[i]) : RX_MKDIR(args->argv[i])) != 0) {
            ctx_errorf(ctx, "mkdir: %s: %s", args->argv[i], strerror(errno));
            rc = RXCREXXCMD_RC_ERROR;
        }
    }
    return rc;
}

static int cmd_rmdir(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    int i;
    int rc;

    (void)command;
    if (args->argc < 2) return command_usage(ctx, "rmdir path...");
    rc = 0;
    for (i = 1; i < args->argc; i++) {
        if (RX_RMDIR(args->argv[i]) != 0) {
            ctx_errorf(ctx, "rmdir: %s: %s", args->argv[i], strerror(errno));
            rc = RXCREXXCMD_RC_ERROR;
        }
    }
    return rc;
}

static int cmd_rm(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    int recursive;
    int start;
    int i;
    int rc;

    (void)command;
    if (args->argc < 2) return command_usage(ctx, "rm [-r] path...");
    recursive = args->argc > 1 && strcmp(args->argv[1], "-r") == 0;
    start = recursive ? 2 : 1;
    if (start >= args->argc) return command_usage(ctx, "rm [-r] path...");

    rc = 0;
    for (i = start; i < args->argc; i++) {
        if ((recursive ? remove_path_recursive(args->argv[i]) : RX_UNLINK(args->argv[i])) != 0) {
            ctx_errorf(ctx, "%s: %s: %s", args->argv[0], args->argv[i], strerror(errno));
            rc = RXCREXXCMD_RC_ERROR;
        }
    }
    return rc;
}

static int cmd_copy(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    (void)command;
    if (args->argc != 3) return command_usage(ctx, "copy source target");
    if (copy_file(args->argv[1], args->argv[2]) != 0) {
        ctx_errorf(ctx, "copy: %s -> %s: %s", args->argv[1], args->argv[2], strerror(errno));
        return RXCREXXCMD_RC_ERROR;
    }
    return 0;
}

static int cmd_move(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    (void)command;
    if (args->argc != 3) return command_usage(ctx, "move source target");
    if (rename(args->argv[1], args->argv[2]) != 0) {
        ctx_errorf(ctx, "move: %s -> %s: %s", args->argv[1], args->argv[2], strerror(errno));
        return RXCREXXCMD_RC_ERROR;
    }
    return 0;
}

static int cmd_touch(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    int i;
    int rc;

    (void)command;
    if (args->argc < 2) return command_usage(ctx, "touch path...");
    rc = 0;
    for (i = 1; i < args->argc; i++) {
        FILE *file = fopen(args->argv[i], "ab");
        if (!file) {
            ctx_errorf(ctx, "touch: %s: %s", args->argv[i], strerror(errno));
            rc = RXCREXXCMD_RC_ERROR;
            continue;
        }
        fclose(file);
        if (RX_UTIME(args->argv[i], NULL) != 0) {
            ctx_errorf(ctx, "touch: %s: %s", args->argv[i], strerror(errno));
            rc = RXCREXXCMD_RC_ERROR;
        }
    }
    return rc;
}

static int cmd_cat(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    int i;
    int rc;

    (void)command;
    if (args->argc < 2) return command_usage(ctx, "cat path...");
    rc = 0;
    for (i = 1; i < args->argc; i++) {
        FILE *file = fopen(args->argv[i], "rb");
        if (!file) {
            ctx_errorf(ctx, "cat: %s: %s", args->argv[i], strerror(errno));
            rc = RXCREXXCMD_RC_NOT_FOUND;
            continue;
        }
        if (read_stream_to_output(ctx, file) != 0) rc = RXCREXXCMD_RC_ERROR;
        fclose(file);
    }
    return rc;
}

static int parse_line_count(rxcrexxcmd_args *args, int *start, long *count, const char *usage, rxcrexxcmd_context *ctx) {
    char *end;

    *start = 1;
    *count = 10;
    if (args->argc > 2 && strcmp(args->argv[1], "-n") == 0) {
        errno = 0;
        *count = strtol(args->argv[2], &end, 10);
        if (errno || !end || *end || *count < 0) return command_usage(ctx, usage);
        *start = 3;
    }
    return 0;
}

static int cmd_head(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    int start;
    long count;
    long emitted;
    FILE *file;
    char line[4096];
    int rc;

    (void)command;
    rc = parse_line_count(args, &start, &count, "head [-n count] path", ctx);
    if (rc != 0) return rc;
    if (args->argc != start + 1) return command_usage(ctx, "head [-n count] path");

    file = fopen(args->argv[start], "rb");
    if (!file) {
        ctx_errorf(ctx, "head: %s: %s", args->argv[start], strerror(errno));
        return RXCREXXCMD_RC_NOT_FOUND;
    }

    emitted = 0;
    while (emitted < count && fgets(line, sizeof(line), file)) {
        if (ctx_puts(ctx, line) != 0) {
            fclose(file);
            return RXCREXXCMD_RC_ERROR;
        }
        if (strchr(line, '\n')) emitted++;
    }

    fclose(file);
    return 0;
}

static int cmd_tail(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    int start;
    long count;
    FILE *file;
    char line[4096];
    char **ring;
    long index;
    long stored;
    long i;
    int rc;

    (void)command;
    rc = parse_line_count(args, &start, &count, "tail [-n count] path", ctx);
    if (rc != 0) return rc;
    if (args->argc != start + 1) return command_usage(ctx, "tail [-n count] path");
    if (count == 0) return 0;

    file = fopen(args->argv[start], "rb");
    if (!file) {
        ctx_errorf(ctx, "tail: %s: %s", args->argv[start], strerror(errno));
        return RXCREXXCMD_RC_NOT_FOUND;
    }

    ring = (char **)calloc((size_t)count, sizeof(char *));
    if (!ring) {
        fclose(file);
        return RXCREXXCMD_RC_ERROR;
    }

    index = 0;
    stored = 0;
    while (fgets(line, sizeof(line), file)) {
        free(ring[index % count]);
        ring[index % count] = rx_strdup(line);
        if (!ring[index % count]) {
            fclose(file);
            for (i = 0; i < count; i++) free(ring[i]);
            free(ring);
            return RXCREXXCMD_RC_ERROR;
        }
        index++;
        if (stored < count) stored++;
    }

    fclose(file);
    for (i = 0; i < stored; i++) {
        char *item = ring[(index - stored + i) % count];
        if (item && ctx_puts(ctx, item) != 0) rc = RXCREXXCMD_RC_ERROR;
    }

    for (i = 0; i < count; i++) free(ring[i]);
    free(ring);
    return rc;
}

static int count_lines_in_text(const char *text, size_t length, long *line_count) {
    size_t i;
    long count;

    count = 0;
    for (i = 0; i < length; i++) {
        if (text[i] == '\n') count++;
    }
    if (length > 0 && text[length - 1] != '\n') count++;
    *line_count = count;
    return 0;
}

static int cmd_lines(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    long count;

    (void)command;
    if (args->argc > 2) return command_usage(ctx, "lines [path]");

    if (args->argc == 1) {
        char *input = NULL;
        size_t input_length = 0;
        if (read_input_text(ctx, &input, &input_length) != 0) return RXCREXXCMD_RC_ERROR;
        count_lines_in_text(input, input_length, &count);
        free(input);
    } else {
        FILE *file;
        int ch;
        int saw_any;
        int last;

        file = fopen(args->argv[1], "rb");
        if (!file) {
            ctx_errorf(ctx, "lines: %s: %s", args->argv[1], strerror(errno));
            return RXCREXXCMD_RC_NOT_FOUND;
        }
        count = 0;
        saw_any = 0;
        last = '\n';
        while ((ch = fgetc(file)) != EOF) {
            saw_any = 1;
            last = ch;
            if (ch == '\n') count++;
        }
        if (saw_any && last != '\n') count++;
        fclose(file);
    }

    return ctx_printf(ctx, "%ld\n", count) == 0 ? 0 : RXCREXXCMD_RC_ERROR;
}

static int write_text_file(rxcrexxcmd_context *ctx, rxcrexxcmd_args *args, const char *mode, const char *usage) {
    FILE *file;
    char *text;
    int rc;

    if (args->argc < 3) return command_usage(ctx, usage);
    text = join_args(args, 2);
    if (!text) return RXCREXXCMD_RC_ERROR;
    file = fopen(args->argv[1], mode);
    if (!file) {
        ctx_errorf(ctx, "%s: %s: %s", args->argv[0], args->argv[1], strerror(errno));
        free(text);
        return RXCREXXCMD_RC_ERROR;
    }
    rc = fputs(text, file) == EOF ? RXCREXXCMD_RC_ERROR : 0;
    if (fclose(file) != 0) rc = RXCREXXCMD_RC_ERROR;
    free(text);
    return rc;
}

static int cmd_write(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    (void)command;
    return write_text_file(ctx, args, "wb", "write path text...");
}

static int cmd_append(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    (void)command;
    return write_text_file(ctx, args, "ab", "append path text...");
}

static int cmd_which(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    char *path;

    (void)command;
    if (args->argc != 2) return command_usage(ctx, "which command");
    path = find_executable(args->argv[1]);
    if (!path) {
        ctx_errorf(ctx, "which: %s: not found", args->argv[1]);
        return RXCREXXCMD_RC_NOT_FOUND;
    }
    ctx_putln(ctx, path);
    free(path);
    return 0;
}

static int cmd_now(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    time_t now;
    struct tm tm_value;
    char buffer[64];
    int use_utc;

    (void)command;
    if (args->argc > 2) return command_usage(ctx, "now [local|utc]");
    use_utc = args->argc == 2 && rx_stricmp(args->argv[1], "utc") == 0;
    if (args->argc == 2 && !use_utc && rx_stricmp(args->argv[1], "local") != 0) {
        return command_usage(ctx, "now [local|utc]");
    }

    now = time(NULL);
#ifdef _WIN32
    if (use_utc) gmtime_s(&tm_value, &now);
    else localtime_s(&tm_value, &now);
#else
    if (use_utc) gmtime_r(&now, &tm_value);
    else localtime_r(&now, &tm_value);
#endif
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S%z", &tm_value);
    return ctx_putln(ctx, buffer) == 0 ? 0 : RXCREXXCMD_RC_ERROR;
}

static int cmd_sleep(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    double seconds;
    char *end;

    (void)command;
    if (args->argc != 2) return command_usage(ctx, "sleep seconds");
    errno = 0;
    seconds = strtod(args->argv[1], &end);
    if (errno || !end || *end || seconds < 0) return command_usage(ctx, "sleep seconds");

#ifdef _WIN32
    Sleep((DWORD)(seconds * 1000.0));
#else
    {
        struct timespec req;
        req.tv_sec = (time_t)seconds;
        req.tv_nsec = (long)((seconds - (double)req.tv_sec) * 1000000000.0);
        while (nanosleep(&req, &req) != 0 && errno == EINTR) {
        }
    }
#endif
    return 0;
}

static int cmd_platform(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    (void)command;
    if (args->argc != 1) return command_usage(ctx, "platform");

    ctx_printf(ctx, "os=%s\n", os_name());
    ctx_printf(ctx, "arch=%s\n", architecture_name());
#ifdef _WIN32
    ctx_putln(ctx, "line_end=crlf");
#else
    ctx_putln(ctx, "line_end=lf");
#endif
    ctx_printf(ctx, "path_separator=%c\n", RX_PATH_SEP);
    ctx_printf(ctx, "path_list_separator=%c\n", RX_PATH_LIST_SEP);
#if !defined(_WIN32)
    {
        struct utsname uts;
        if (uname(&uts) == 0) {
            ctx_printf(ctx, "sysname=%s\n", uts.sysname);
            ctx_printf(ctx, "release=%s\n", uts.release);
            ctx_printf(ctx, "machine=%s\n", uts.machine);
        }
    }
#endif
    return 0;
}

static int cmd_env(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    (void)command;
    if (args->argc > 2) return command_usage(ctx, "env [name]");

    if (args->argc == 2) {
        const char *value = getenv(args->argv[1]);
        if (!value) return RXCREXXCMD_RC_NOT_FOUND;
        return ctx_putln(ctx, value) == 0 ? 0 : RXCREXXCMD_RC_ERROR;
    }

#ifdef _WIN32
    {
        LPCH block = GetEnvironmentStringsA();
        LPCH cursor = block;
        if (!block) return RXCREXXCMD_RC_ERROR;
        while (*cursor) {
            ctx_putln(ctx, cursor);
            cursor += strlen(cursor) + 1;
        }
        FreeEnvironmentStringsA(block);
    }
#else
    {
        char **cursor = environ;
        while (cursor && *cursor) {
            if (ctx_putln(ctx, *cursor) != 0) return RXCREXXCMD_RC_ERROR;
            cursor++;
        }
    }
#endif
    return 0;
}

static int cmd_setenv(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    char *value;
    int rc;

    (void)command;
    if (args->argc < 3) return command_usage(ctx, "setenv name value");
    value = join_args(args, 2);
    if (!value) return RXCREXXCMD_RC_ERROR;
#ifdef _WIN32
    rc = _putenv_s(args->argv[1], value);
#else
    rc = setenv(args->argv[1], value, 1);
#endif
    if (rc != 0) ctx_errorf(ctx, "setenv: %s: %s", args->argv[1], strerror(errno));
    free(value);
    return rc == 0 ? 0 : RXCREXXCMD_RC_ERROR;
}

static int cmd_unsetenv(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    int rc;

    (void)command;
    if (args->argc != 2) return command_usage(ctx, "unsetenv name");
#ifdef _WIN32
    rc = _putenv_s(args->argv[1], "");
#else
    rc = unsetenv(args->argv[1]);
#endif
    if (rc != 0) ctx_errorf(ctx, "unsetenv: %s: %s", args->argv[1], strerror(errno));
    return rc == 0 ? 0 : RXCREXXCMD_RC_ERROR;
}

static int cmd_pid(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    (void)command;
    if (args->argc != 1) return command_usage(ctx, "pid");
#ifdef _WIN32
    return ctx_printf(ctx, "%lu\n", (unsigned long)GetCurrentProcessId()) == 0 ? 0 : RXCREXXCMD_RC_ERROR;
#else
    return ctx_printf(ctx, "%ld\n", (long)getpid()) == 0 ? 0 : RXCREXXCMD_RC_ERROR;
#endif
}

static int cmd_ps(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    (void)command;
    if (args->argc > 2) return command_usage(ctx, "ps [pid]");

    if (args->argc == 1) {
#ifdef _WIN32
        ctx_printf(ctx, "pid=%lu\n", (unsigned long)GetCurrentProcessId());
#else
        ctx_printf(ctx, "pid=%ld\n", (long)getpid());
        ctx_printf(ctx, "ppid=%ld\n", (long)getppid());
#endif
        return 0;
    }

    {
        char *end;
        long pid;
        errno = 0;
        pid = strtol(args->argv[1], &end, 10);
        if (errno || !end || *end || pid <= 0) return command_usage(ctx, "ps [pid]");
#ifdef _WIN32
        {
            HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, (DWORD)pid);
            if (!process) return RXCREXXCMD_RC_NOT_FOUND;
            CloseHandle(process);
        }
#else
        if (kill((pid_t)pid, 0) != 0) return RXCREXXCMD_RC_NOT_FOUND;
#endif
        return ctx_printf(ctx, "pid=%ld alive=1\n", pid) == 0 ? 0 : RXCREXXCMD_RC_ERROR;
    }
}

static int cmd_kill(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    char *end;
    long pid;
    long signal_number;

    (void)command;
    if (args->argc < 2 || args->argc > 3) return command_usage(ctx, "kill pid [signal]");
    errno = 0;
    pid = strtol(args->argv[1], &end, 10);
    if (errno || !end || *end || pid <= 0) return command_usage(ctx, "kill pid [signal]");

    signal_number = 15;
    if (args->argc == 3) {
        errno = 0;
        signal_number = strtol(args->argv[2], &end, 10);
        if (errno || !end || *end) return command_usage(ctx, "kill pid [signal]");
    }

#ifdef _WIN32
    {
        HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)pid);
        (void)signal_number;
        if (!process) {
            ctx_errorf(ctx, "kill: %ld: not found", pid);
            return RXCREXXCMD_RC_NOT_FOUND;
        }
        if (!TerminateProcess(process, 1)) {
            CloseHandle(process);
            ctx_errorf(ctx, "kill: %ld: failed", pid);
            return RXCREXXCMD_RC_ERROR;
        }
        CloseHandle(process);
    }
#else
    if (kill((pid_t)pid, (int)signal_number) != 0) {
        ctx_errorf(ctx, "kill: %ld: %s", pid, strerror(errno));
        return RXCREXXCMD_RC_ERROR;
    }
#endif
    return 0;
}

static void socket_startup(void) {
#ifdef _WIN32
    static int started = 0;
    WSADATA data;
    if (!started) {
        if (WSAStartup(MAKEWORD(2, 2), &data) == 0) started = 1;
    }
#endif
}

static int cmd_resolve(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    struct addrinfo hints;
    struct addrinfo *result;
    struct addrinfo *cursor;
    int rc;
    int emitted;
    char host[NI_MAXHOST];

    (void)command;
    if (args->argc != 2) return command_usage(ctx, "resolve host");
    socket_startup();
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;

    rc = getaddrinfo(args->argv[1], NULL, &hints, &result);
    if (rc != 0) {
        ctx_errorf(ctx, "resolve: %s: %s", args->argv[1], gai_strerror(rc));
        return RXCREXXCMD_RC_NOT_FOUND;
    }

    emitted = 0;
    for (cursor = result; cursor; cursor = cursor->ai_next) {
        if (getnameinfo(cursor->ai_addr, (socklen_t)cursor->ai_addrlen,
                        host, sizeof(host), NULL, 0, NI_NUMERICHOST) == 0) {
            ctx_putln(ctx, host);
            emitted++;
        }
    }

    freeaddrinfo(result);
    return emitted ? 0 : RXCREXXCMD_RC_NOT_FOUND;
}

static int cmd_tcp(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    struct addrinfo hints;
    struct addrinfo *result;
    struct addrinfo *cursor;
    int rc;
    int connected;

    (void)command;
    if (args->argc != 3) return command_usage(ctx, "tcp host port");
    socket_startup();

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;

    rc = getaddrinfo(args->argv[1], args->argv[2], &hints, &result);
    if (rc != 0) {
        ctx_errorf(ctx, "tcp: %s:%s: %s", args->argv[1], args->argv[2], gai_strerror(rc));
        return RXCREXXCMD_RC_NOT_FOUND;
    }

    connected = 0;
    for (cursor = result; cursor; cursor = cursor->ai_next) {
#ifdef _WIN32
        SOCKET sock = socket(cursor->ai_family, cursor->ai_socktype, cursor->ai_protocol);
        if (sock == INVALID_SOCKET) continue;
        if (connect(sock, cursor->ai_addr, (int)cursor->ai_addrlen) == 0) connected = 1;
        closesocket(sock);
#else
        int sock = socket(cursor->ai_family, cursor->ai_socktype, cursor->ai_protocol);
        if (sock < 0) continue;
        if (connect(sock, cursor->ai_addr, cursor->ai_addrlen) == 0) connected = 1;
        close(sock);
#endif
        if (connected) break;
    }

    freeaddrinfo(result);
    if (!connected) {
        ctx_errorf(ctx, "tcp: %s:%s: connection failed", args->argv[1], args->argv[2]);
        return RXCREXXCMD_RC_ERROR;
    }

    return ctx_printf(ctx, "ok %s %s\n", args->argv[1], args->argv[2]) == 0 ? 0 : RXCREXXCMD_RC_ERROR;
}

static int execute_line(rxcrexxcmd_context *ctx, const char *line);

static int cmd_batch(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    char *input;
    size_t input_length;
    size_t start;
    size_t i;
    int rc;

    (void)command;
    if (args->argc != 1) return command_usage(ctx, "batch");
    input = NULL;
    input_length = 0;
    if (read_input_text(ctx, &input, &input_length) != 0) return RXCREXXCMD_RC_ERROR;

    rc = 0;
    start = 0;
    for (i = 0; i <= input_length; i++) {
        if (i == input_length || input[i] == '\n') {
            char *line;
            size_t line_length = i - start;
            char *trim;

            if (line_length > 0 && input[start + line_length - 1] == '\r') line_length--;
            line = rx_strndup(input + start, line_length);
            if (!line) {
                free(input);
                return RXCREXXCMD_RC_ERROR;
            }
            trim = line;
            while (*trim && isspace((unsigned char)*trim)) trim++;
            if (*trim && !(trim[0] == '-' && trim[1] == '-')) {
                rc = execute_line(ctx, trim);
                if (rc != 0) {
                    free(line);
                    break;
                }
            }
            free(line);
            start = i + 1;
        }
    }

    free(input);
    return rc;
}

static int cmd_run(rxcrexxcmd_context *ctx, const char *command, rxcrexxcmd_args *args) {
    const char *tail;
    char *out_text;
    char *err_text;
    char *run_error;
    int command_rc;
    int run_rc;

    if (args->argc < 2) return command_usage(ctx, "run command...");
    if (!ctx || !ctx->io || !ctx->io->run_path) {
        ctx_errln(ctx, "run: PATH execution callback is unavailable");
        return RXCREXXCMD_RC_ERROR;
    }

    tail = command_tail_after_first_word(command);
    out_text = NULL;
    err_text = NULL;
    run_error = NULL;
    command_rc = 0;
    run_rc = ctx->io->run_path(ctx->io->userdata, tail, &out_text, &err_text, &command_rc, &run_error);
    if (run_rc != 0) {
        if (run_error) ctx_errorf(ctx, "run: %s", run_error);
        else ctx_errorf(ctx, "run: %s", tail);
        free(out_text);
        free(err_text);
        free(run_error);
        return command_rc ? command_rc : RXCREXXCMD_RC_ERROR;
    }

    if (out_text) ctx_write(ctx, 0, out_text, strlen(out_text));
    if (err_text) ctx_write(ctx, 1, err_text, strlen(err_text));
    free(out_text);
    free(err_text);
    free(run_error);
    return command_rc;
}

static int execute_line(rxcrexxcmd_context *ctx, const char *line) {
    rxcrexxcmd_args args;
    char *diagnostic;
    int parse_rc;
    int i;
    int rc;

    diagnostic = NULL;
    parse_rc = parse_args(line, &args, &diagnostic);
    if (parse_rc == -1) return RXCREXXCMD_RC_ERROR;
    if (parse_rc != 0) {
        if (diagnostic) ctx_errorf(ctx, "CREXX: %s", diagnostic);
        free(diagnostic);
        return parse_rc;
    }

    if (args.argc == 0) {
        free_args(&args);
        return 0;
    }

    for (i = 0; i < args.argc; i++) {
        if (is_shell_operator_token(args.argv[i])) {
            ctx_errorf(ctx, "CREXX: shell operator '%s' is not supported; use multiple ADDRESS statements or batch input", args.argv[i]);
            free_args(&args);
            return RXCREXXCMD_RC_USAGE;
        }
    }

    for (i = 0; command_table[i].name; i++) {
        if (rx_stricmp(args.argv[0], command_table[i].name) == 0) {
            rc = command_table[i].handler(ctx, line, &args);
            free_args(&args);
            return rc;
        }
    }

    ctx_errorf(ctx, "CREXX: unknown command: %s", args.argv[0]);
    free_args(&args);
    return RXCREXXCMD_RC_UNKNOWN;
}

int rxcrexxcmd_execute(const char *command,
                       const rxcrexxcmd_io *io,
                       int *rc,
                       char **error_text) {
    rxcrexxcmd_context ctx;

    if (error_text) *error_text = NULL;
    if (rc) *rc = 0;
    ctx.io = io;

    if (!command) command = "";
    if (rc) *rc = execute_line(&ctx, command);
    else execute_line(&ctx, command);
    return 0;
}

void rxcrexxcmd_free(char *text) {
    free(text);
}
