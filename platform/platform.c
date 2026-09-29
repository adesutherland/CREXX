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
// Std C - Utility Functions
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "platform.h"
#include "platform_native.h"
#include "text_codec.h"

#if defined(CREXX_MAINFRAME_ELF) && !defined(CREXX_NATIVE_RAW_IO) && \
    (defined(CREXX_CMS_TEXT_IO) || defined(CREXX_PLATFORM_TSO))
/* The SDK supplies raw record bytes when mainframe text conversion is off.
 * Keep character decoding in cREXX's existing codec at its file boundary. */
typedef struct {
    FILE *raw;
    const uint32_t *map;
    crexx_utf8_state utf8;
    unsigned char pending[4];
    unsigned pending_at, pending_count;
    int writing, failed;
} crexx_file_codec;
static const uint32_t *selected_file_map;
static int selected_file_map_set;

static const uint32_t *file_codec(void) {
    const uint32_t *map;
    if (selected_file_map_set) return selected_file_map;
    if (platform_text_codec_lookup("IBM1047", &map)) return NULL;
    return map;
}
static int native_file_codec(void) {
    const uint32_t *native;
    return !platform_text_codec_lookup("IBM1047", &native) && file_codec() == native;
}
static int codec_failure(crexx_file_codec *s, int error) {
    if (!s->failed) s->failed = error ? error : EIO;
    errno = s->failed;
    return -1;
}
static int codec_read(void *cookie, char *out, int count) {
    crexx_file_codec *s = cookie;
    int done = 0;
    if (s->failed) return codec_failure(s, s->failed);
    while (done < count) {
        int byte, n;
        uint32_t scalar;
        if (s->pending_at < s->pending_count) {
            out[done++] = (char)s->pending[s->pending_at++];
            continue;
        }
        s->pending_at = s->pending_count = 0;
        byte = fgetc(s->raw);
        if (byte == EOF) {
            if (ferror(s->raw) || (!s->map && crexx_utf8_finish(&s->utf8))) {
                codec_failure(s, ferror(s->raw) ? errno : EILSEQ);
                return done ? done : -1;
            }
            break;
        }
        /* The SDK preserves ASCII LF as the record/byte-stream delimiter. */
        if (byte == '\n') {
            if (!s->map && crexx_utf8_finish(&s->utf8)) {
                codec_failure(s, errno);
                return done ? done : -1;
            }
            out[done++] = '\n';
            continue;
        }
        if (!s->map) {
            n = crexx_utf8_feed(&s->utf8, (unsigned char)byte, &scalar);
            if (n < 0) {
                codec_failure(s, errno);
                return done ? done : -1;
            }
            out[done++] = (char)byte;
        } else {
            scalar = s->map[(unsigned char)byte];
            n = crexx_utf8_emit(scalar, s->pending);
            if (n < 0) {
                codec_failure(s, errno);
                return done ? done : -1;
            }
            s->pending_count = (unsigned)n;
        }
    }
    return done;
}
static int codec_write(void *cookie, const char *input, int count) {
    crexx_file_codec *s = cookie;
    int i;
    if (s->failed) return codec_failure(s, s->failed);
    for (i = 0; i < count; ++i) {
        uint32_t scalar;
        unsigned char bytes[4];
        int ready = crexx_utf8_feed(&s->utf8, (unsigned char)input[i], &scalar);
        int n;
        if (ready < 0) return codec_failure(s, errno);
        if (!ready) continue;
        if (scalar == '\n') { bytes[0] = '\n'; n = 1; }
        else n = crexx_text_encode(s->map, scalar, bytes);
        if (n < 0) return codec_failure(s, errno);
        if (fwrite(bytes, 1, (size_t)n, s->raw) != (size_t)n)
            return codec_failure(s, errno);
    }
    return count;
}
static int codec_close(void *cookie) {
    crexx_file_codec *s = cookie;
    int error = s->failed;
    if (s->writing && !error && crexx_utf8_finish(&s->utf8)) error = errno;
    if (fclose(s->raw) && !error) error = errno;
    free(s);
    if (error) { errno = error; return -1; }
    return 0;
}
static FILE *codec_wrap(FILE *raw, const char *mode) {
    crexx_file_codec *s;
    FILE *file;
    if (!raw) return NULL;
    s = calloc(1, sizeof(*s));
    if (!s) { int error = errno; fclose(raw); errno = error; return NULL; }
    s->raw = raw;
    s->map = file_codec();
    s->writing = *mode != 'r';
    file = funopen(s, s->writing ? NULL : codec_read,
                   s->writing ? codec_write : NULL, NULL, codec_close);
    if (!file) { int error = errno; fclose(raw); free(s); errno = error; }
    return file;
}
static const char *codec_storage_mode(const char *mode) {
    if (native_file_codec() || strchr(mode, 'b')) return mode;
    return *mode == 'r' ? "rb" : *mode == 'w' ? "wb" : mode;
}
#endif

int platform_console_text_write(FILE *stream, const char *text, size_t length) {
#if defined(CREXX_MAINFRAME_ELF)
    const uint32_t *native_map;
    crexx_utf8_state state = {0, 0, 0};
    unsigned char chunk[128];
    size_t i, used = 0;
    if (!stream || !text || platform_text_codec_lookup("IBM1047", &native_map)) return -1;
    for (i = 0; i < length; ++i) {
        uint32_t scalar;
        unsigned char encoded[4];
        int ready = crexx_utf8_feed(&state, (unsigned char)text[i], &scalar);
        int count;
        if (ready < 0) return -1;
        if (!ready) continue;
        /* The runtime keeps ASCII LF as its record delimiter in raw mode. */
        if (scalar == '\n') { encoded[0] = '\n'; count = 1; }
        else {
            count = crexx_text_encode(native_map, scalar, encoded);
            if (count < 0) return -1;
        }
        if (used + (size_t)count > sizeof(chunk)) {
            if (fwrite(chunk, 1, used, stream) != used) return -1;
            used = 0;
        }
        memcpy(chunk + used, encoded, (size_t)count);
        used += (size_t)count;
    }
    if (crexx_utf8_finish(&state)) return -1;
    return !used || fwrite(chunk, 1, used, stream) == used ? 0 : -1;
#else
    return fwrite(text, 1, length, stream) == length ? 0 : -1;
#endif
}

#if defined(CREXX_CMS_TEXT_IO)
#if !defined(CREXX_MAINFRAME_ELF)
#error CREXX_CMS_TEXT_IO requires an explicit CMS ELF platform
#endif
extern FILE *crexx_cms_text_open(const char *, const char *);
extern int crexx_cms_text_encoding(const char *);
#endif

int platform_text_encoding(const char *encoding) {
#if defined(CREXX_NATIVE_RAW_IO)
    const uint32_t *map;
    if (platform_text_codec_lookup(encoding, &map)) return -1;
    crexx_native_select_file_codec(map);
    return 0;
#elif defined(CREXX_MAINFRAME_ELF) && \
      (defined(CREXX_CMS_TEXT_IO) || defined(CREXX_PLATFORM_TSO))
    const uint32_t *map;
    if (platform_text_codec_lookup(encoding, &map)) return -1;
    selected_file_map = map;
    selected_file_map_set = 1;
    return 0;
#else
    if (encoding && (!strcmp(encoding,"UTF8") || !strcmp(encoding,"utf8") ||
                     !strcmp(encoding,"UTF-8") || !strcmp(encoding,"utf-8"))) return 0;
    errno = EINVAL;
    return -1;
#endif
}

static FILE *platform_open_stream_storage(const char *path, const char *mode, int storage) {
#if defined(CREXX_NATIVE_RAW_IO)
    return crexx_native_fopen_storage(path, mode, storage);
#elif defined(CREXX_CMS_TEXT_IO)
    (void)storage;
    if (!strchr(mode,'b')) {
        FILE *raw;
        if (native_file_codec()) {
            if (crexx_cms_text_encoding("IBM1047")) return NULL;
            raw = crexx_cms_text_open(path, mode);
        } else raw = fopen(path, codec_storage_mode(mode));
        return codec_wrap(raw, mode);
    }
#else
    (void)storage;
#endif
    return fopen(path,mode);
}
static FILE *platform_open_stream(const char *path, const char *mode) {
    return platform_open_stream_storage(path, mode, CREXX_TEXT_STORAGE_DEFAULT);
}

#if defined(__linux__) && !defined(CREXX_MAINFRAME_ELF)
#include <unistd.h>
#include <dirent.h>
#include <sys/sysinfo.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#endif

#ifndef _MSC_VER // Windows Visual Studio
#include <stdint.h>
#endif

#if defined(__APPLE__) && !defined(CREXX_MAINFRAME_ELF)
#include <mach-o/dyld.h>
#include <dirent.h>
#include <unistd.h>
#endif

#if defined(CREXX_CMS_ELF) && defined(CREXX_CMS_DIRENT)
/* Supplied by the CMS host runtime, never inherited Linux directory services. */
#include <dirent.h>
#endif

/* Keep basic filename handling within the ISO C library surface. */
static char *platform_copy_string(const char *text) {
    size_t size = strlen(text) + 1;
    char *copy = malloc(size);
    if (copy) memcpy(copy, text, size);
    return copy;
}

/* Sequential streams have no usable length. Retain two scanner sentinels,
 * check growth before arithmetic, and never return partial data after error. */
static char *stream2buf(FILE *file, size_t *bytes) {
    size_t used = 0, capacity = 1024;
    const size_t limit = (size_t)-1 - 2;
    char *buff = (char *)malloc(capacity + 2);
    if (!buff) {
        RX_REPORT_OOM("malloc file read buffer", capacity + 2, "file2buf");
        return 0;
    }
    for (;;) {
        size_t n = fread(buff + used, 1, capacity - used, file);
        used += n;
        if (ferror(file)) { free(buff); return 0; }
        if (feof(file)) break;
        if (!n) { free(buff); return 0; }
        if (used == capacity) {
            size_t next = capacity <= limit / 2 ? capacity * 2 : limit;
            char *grown;
            if (next <= capacity) { free(buff); return 0; }
            grown = (char *)realloc(buff, next + 2);
            if (!grown) {
                RX_REPORT_OOM("grow file read buffer", next + 2, "file2buf");
                free(buff);
                return 0;
            }
            buff = grown;
            capacity = next;
        }
    }
    buff[used] = buff[used + 1] = 0;
    *bytes = used;
    return buff;
}

/* Read a seekable file from the beginning, or a sequential stream from its
 * current position. The caller frees the buffer, including two NUL sentinels. */
char* file2buf(FILE *file, size_t *bytes) {
    char *buff;
    size_t n;
    long pos;
    *bytes = 0;
    if (fseek(file, 0, SEEK_END) != 0) {
        clearerr(file);
        return stream2buf(file, bytes);
    }
    pos = ftell(file);
    if (pos < 0 || (unsigned long)pos > (size_t)-1 - 2) return 0;
    if (fseek(file, 0, SEEK_SET) != 0) return 0;
    buff = (char*)malloc((size_t)pos + 2);
    if (!buff) {
        RX_REPORT_OOM("malloc file read buffer", (size_t)pos + 2, "file2buf");
        return 0;
    }
    n = fread(buff, 1, (size_t)pos, file);
    if (ferror(file) || (n == 0 && pos > 0)) {
        free(buff);
        return 0;
    }
    *bytes = n;
    buff[n] = buff[n + 1] = 0;
    return buff;
}

#include <ctype.h>

/*
 * Function checks if a file name has a specific extension
 */
static int has_extension(const char *name, const char *type) {
    size_t name_len, type_len;
    if (!type || !type[0]) return 1;
    name_len = strlen(name);
    type_len = strlen(type);
    if (name_len >= type_len + 1 && name[name_len - type_len - 1] == '.' &&
        strcmp(name + name_len - type_len, type) == 0) {
        return 1;
    }
    return 0;
}

/* Checks if a file has any extension */
int has_any_extension(const char *name) {
    const char *last_slash = strrchr(name, '/');
#ifdef _WIN32
    const char *last_bsl = strrchr(name, '\\');
    if (!last_slash || (last_bsl && last_bsl > last_slash)) last_slash = last_bsl;
#endif
    const char *fname = last_slash ? last_slash + 1 : name;
    return strchr(fname, '.') != NULL;
}

/* Strips the rightmost extension from a filename if it matches the provided extension */
char *strip_rightmost_extension_if(const char *name, const char *ext) {
    if (has_extension(name, ext)) {
        size_t name_len = strlen(name);
        size_t ext_len = strlen(ext);
        char *new_name = malloc(name_len - ext_len); /* -ext_len - 1 (for dot) + 1 (for null) = -ext_len */
        if (!new_name) {
            RX_PANIC_OOM("malloc stripped file name", name_len - ext_len, name);
        }
        strncpy(new_name, name, name_len - ext_len - 1);
        new_name[name_len - ext_len - 1] = 0;
        return new_name;
    }
    {
        char *copy = platform_copy_string(name);
        if (!copy) RX_PANIC_OOM("strdup file name", strlen(name) + 1, name);
        return copy;
    }
}

#if !defined(_WIN32) && !defined(__CMS__) && !defined(CREXX_MAINFRAME_ELF)
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

static struct termios orig_termios;
static int termios_saved = 0;
static int termios_fd = -1;
static pid_t termios_owner = (pid_t)-1;
static int termios_handlers_installed = 0;

static int platform_term_disconnected_error(int error_number) {
    return error_number == EBADF || error_number == ENOTTY ||
           error_number == EIO || error_number == ENXIO;
}

static void platform_term_report(const char *operation, int error_number) {
    fprintf(stderr,
            "CREXX terminal restore: %s failed for the saved terminal "
            "(errno=%d: %s)\n",
            operation, error_number, strerror(error_number));
}

static void platform_term_forget(void) {
    int saved_fd = termios_fd;
    termios_saved = 0;
    termios_fd = -1;
    termios_owner = (pid_t)-1;
    if (saved_fd >= 0) close(saved_fd);
}

static int platform_term_duplicate_stdin(void) {
    int duplicate_fd;
#ifdef F_DUPFD_CLOEXEC
    duplicate_fd = fcntl(STDIN_FILENO, F_DUPFD_CLOEXEC, 3);
#else
    duplicate_fd = fcntl(STDIN_FILENO, F_DUPFD, 3);
    if (duplicate_fd >= 0 &&
            fcntl(duplicate_fd, F_SETFD, FD_CLOEXEC) != 0) {
        int saved_errno = errno;
        close(duplicate_fd);
        errno = saved_errno;
        return -1;
    }
#endif
    return duplicate_fd;
}

void platform_term_save(void) {
    pid_t foreground_group;
    int duplicate_fd;

    if (termios_saved) return;
    if (!isatty(STDIN_FILENO)) return;

    duplicate_fd = platform_term_duplicate_stdin();
    if (duplicate_fd < 0) {
        platform_term_report("duplicate", errno);
        return;
    }

    errno = 0;
    foreground_group = tcgetpgrp(duplicate_fd);
    if (foreground_group == (pid_t)-1) {
        int saved_errno = errno;
        close(duplicate_fd);
        if (!platform_term_disconnected_error(saved_errno)) {
            platform_term_report("query foreground process group",
                                 saved_errno);
        }
        return;
    }
    if (foreground_group != getpgrp()) {
        /* A background process must not claim or mutate the terminal. */
        close(duplicate_fd);
        return;
    }
    if (tcgetattr(duplicate_fd, &orig_termios) != 0) {
        int saved_errno = errno;
        close(duplicate_fd);
        if (!platform_term_disconnected_error(saved_errno)) {
            platform_term_report("snapshot attributes", saved_errno);
        }
        return;
    }

    termios_fd = duplicate_fd;
    termios_owner = getpid();
    termios_saved = 1;
}

static void platform_term_restore_internal(int report_errors) {
    pid_t foreground_group;

    if (!termios_saved || termios_fd < 0) return;
    if (termios_owner != getpid()) return;

    errno = 0;
    foreground_group = tcgetpgrp(termios_fd);
    if (foreground_group == (pid_t)-1) {
        int saved_errno = errno;
        if (platform_term_disconnected_error(saved_errno)) {
            /* The saved terminal endpoint no longer exists: do not apply the
             * snapshot to a replacement stdin, and do not report a false
             * restoration failure. */
            platform_term_forget();
        } else if (report_errors) {
            platform_term_report("query foreground process group",
                                 saved_errno);
        }
        return;
    }
    if (foreground_group != getpgrp()) {
        /* Restoration is only safe while this process group owns the same
         * terminal that was snapshotted.  Retain the snapshot for a possible
         * later foreground restoration. */
        return;
    }

    while (tcsetattr(termios_fd, TCSANOW, &orig_termios) != 0) {
        int saved_errno = errno;
        if (saved_errno == EINTR) continue;
        if (platform_term_disconnected_error(saved_errno)) {
            platform_term_forget();
        } else if (report_errors) {
            platform_term_report("restore attributes", saved_errno);
        }
        return;
    }
    platform_term_forget();
}

void platform_term_restore(void) {
    platform_term_restore_internal(1);
}

static void signal_handler(int sig) {
    /* Avoid stdio diagnostics from a signal context. */
    platform_term_restore_internal(0);
    /* Re-raise the signal or exit */
    signal(sig, SIG_DFL);
    raise(sig);
}

void platform_install_signal_handlers(void) {
    platform_term_save();
    if (termios_saved && !termios_handlers_installed) {
        atexit(platform_term_restore);
        signal(SIGSEGV, signal_handler);
        signal(SIGILL, signal_handler);
        signal(SIGFPE, signal_handler);
        signal(SIGBUS, signal_handler);
        signal(SIGABRT, signal_handler);
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        termios_handlers_installed = 1;
    }
}
#else
/* Stub for Windows or other platforms if not needed */
void platform_term_save(void) {}
void platform_term_restore(void) {}
void platform_install_signal_handlers(void) {}
#endif

/*
 * Function checks if a file exists
 * dir can be null, and can contain multiple directories separated by ;
 * returns 1 if the file exists
 */
int fileexists(char *name, char *type, char *dir) {
#if defined(CREXX_PLATFORM_TSO)
    FILE *probe = crexx_tso_openfile(name, type, dir, "rb");
    if (!probe) return 0;
    return fclose(probe) == 0;
#else
    size_t len;
    char *file_name;
    int result = 0;
    char *dir_copy;
    char *token;
    char *next_token;

    /* If name already contains a directory separator, ignore dir */
    if (name && (strchr(name, '/') || strchr(name, '\\'))) {
        dir = 0;
    }

    if (!dir || !*dir) {
        /* Single attempt with current directory */
        len = strlen(name) + strlen(type) + 2;
        file_name = malloc(len);
        if (!file_name) RX_PANIC_OOM("malloc file existence path", len, name);
        if (type[0] == 0 || has_extension(name, type)) snprintf(file_name, len, "%s", name);
        else snprintf(file_name, len, "%s.%s", name, type);
#if (defined(__linux__) || defined(__APPLE__)) && !defined(CREXX_MAINFRAME_ELF)
        result = access(file_name, F_OK) != -1;
#elif defined(CREXX_CMS_ELF)
        FILE *probe = platform_open_stream(file_name, "rb");
        if (probe) { result = 1; fclose(probe); }
#elif defined(_WIN32)
        DWORD dwAttrib = GetFileAttributes(file_name);
        result = (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#endif
        free(file_name);
        return result;
    }

    /* Multiple directories support */
    dir_copy = platform_copy_string(dir);
    if (!dir_copy) RX_PANIC_OOM("strdup file existence directory list", strlen(dir) + 1, dir);
    token = dir_copy;
    while (token) {
        next_token = strchr(token, ';');
        if (next_token) *next_token = 0;

        if (*token) {
            len = strlen(name) + strlen(type) + strlen(token) + 3;
            file_name = malloc(len);
            if (!file_name) RX_PANIC_OOM("malloc file existence path", len, name);
            if (type[0] == 0 || has_extension(name, type)) snprintf(file_name, len, "%s/%s", token, name);
            else snprintf(file_name, len, "%s/%s.%s", token, name, type);
#if (defined(__linux__) || defined(__APPLE__)) && !defined(CREXX_MAINFRAME_ELF)
            result = access(file_name, F_OK) != -1;
#elif defined(CREXX_CMS_ELF)
            FILE *probe = platform_open_stream(file_name, "rb");
            if (probe) { result = 1; fclose(probe); }
#elif defined(_WIN32)
            DWORD dwAttrib = GetFileAttributes(file_name);
            result = (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#endif
            free(file_name);
            if (result) break;
        }

        token = next_token ? next_token + 1 : 0;
    }
    free(dir_copy);

    return result;
#endif
}

/*
 * Function opens and returns a file handle
 * dir can be null, and can contain multiple directories separated by ;
 * mode - is the fopen() file mode
 */
FILE *openfile(char *name, char *type, char *dir, char *mode) {
#if defined(CREXX_PLATFORM_TSO)
#if defined(CREXX_PLATFORM_TSO) && !defined(CREXX_NATIVE_RAW_IO)
    if (!strchr(mode, 'b'))
        return codec_wrap(crexx_tso_openfile(name, type, dir,
                                            codec_storage_mode(mode)), mode);
#endif
    return crexx_tso_openfile(name, type, dir, mode);
#else
    size_t len;
    char *file_name;
    FILE *stream = NULL;
    char *dir_copy;
    char *token;
    char *next_token;

    /* If name already contains a directory separator, ignore dir */
    if (name && (strchr(name, '/') || strchr(name, '\\'))) {
        dir = 0;
    }

    if (!dir || !*dir) {
        /* Single attempt with current directory */
        len = strlen(name) + strlen(type) + 2;
        file_name = malloc(len);
        if (!file_name) RX_PANIC_OOM("malloc openfile path", len, name);
        if (type[0] == 0 || has_extension(name, type)) snprintf(file_name, len, "%s", name);
        else snprintf(file_name, len, "%s.%s", name, type);
        stream = platform_open_stream(file_name, mode);
        free(file_name);
        return stream;
    }

    /* Multiple directories support */
    dir_copy = platform_copy_string(dir);
    if (!dir_copy) RX_PANIC_OOM("strdup openfile directory list", strlen(dir) + 1, dir);
    token = dir_copy;
    while (token) {
        next_token = strchr(token, ';');
        if (next_token) *next_token = 0;

        if (*token) {
            len = strlen(name) + strlen(type) + strlen(token) + 3;
            file_name = malloc(len);
            if (!file_name) RX_PANIC_OOM("malloc openfile path", len, name);
            if (type[0] == 0 || has_extension(name, type)) snprintf(file_name, len, "%s/%s", token, name);
            else snprintf(file_name, len, "%s/%s.%s", token, name, type);
            stream = platform_open_stream(file_name, mode);
            free(file_name);
            if (stream) break;
        }

        token = next_token ? next_token + 1 : 0;
    }
    free(dir_copy);

    return stream;
#endif
}

FILE *platform_fopen_storage(const char *path, const char *mode, int storage) {
#if defined(CREXX_PLATFORM_TSO)
#if defined(CREXX_PLATFORM_TSO) && !defined(CREXX_NATIVE_RAW_IO)
    if (!strchr(mode, 'b'))
        return codec_wrap(crexx_tso_openfile_storage(path, "", NULL,
                                                     codec_storage_mode(mode), storage), mode);
#endif
    return crexx_tso_openfile_storage(path, "", NULL, mode, storage);
#else
    return platform_open_stream_storage(path, mode, storage);
#endif
}
FILE *platform_fopen(const char *path, const char *mode) {
    return platform_fopen_storage(path, mode, CREXX_TEXT_STORAGE_DEFAULT);
}

#if ((defined(__APPLE__) || defined(__linux__)) && !defined(CREXX_MAINFRAME_ELF)) || \
    (defined(CREXX_CMS_ELF) && defined(CREXX_CMS_DIRENT))
struct fl_dir {
    DIR *d;
    char *type;
    char *prefix;
};
#endif

#ifdef _WIN32
struct WIN_FILE_DATA {
    HANDLE hFind;
    WIN32_FIND_DATA fdFile;
    char sPath[MAXFILEPATH];
    char dir[MAXFILEPATH];
};
#endif

/*
 * Get the first file from a directory (or null if there isn't one)
 * (pass the & of void *dir_ptr to hold an opaque directory context)
 * if dir is null then the "current" (platform specific) dir is searched
 */
char *dirfstfl(const char *dir, char* prefix, char *type, void **dir_ptr) {

#if defined(CREXX_PLATFORM_CMS) && defined(CREXX_NATIVE_RAW_IO)
    return crexx_cms_dirfirst(dir, prefix, type, dir_ptr);
#elif defined(CREXX_PLATFORM_TSO)
    return crexx_tso_dirfirst(dir, prefix, type, dir_ptr);
#elif ((defined(__APPLE__) || defined(__linux__)) && !defined(CREXX_MAINFRAME_ELF)) || \
    (defined(CREXX_CMS_ELF) && defined(CREXX_CMS_DIRENT))

    struct fl_dir *ptr = malloc(sizeof(struct fl_dir));
    if (!ptr) RX_PANIC_OOM("malloc directory iterator", sizeof(struct fl_dir), dir);
    *dir_ptr = ptr;

    ptr->type = type;
    ptr->prefix = prefix;

    if (dir && strlen(dir)) ptr->d = opendir(dir);
    else ptr->d = opendir(".");

    if (!ptr->d) {
        free(ptr);
        *dir_ptr = 0;
        return 0;
    }

    return dirnxtfl(dir_ptr);

#elif defined(_WIN32)

    struct WIN_FILE_DATA *win_data = malloc(sizeof(struct WIN_FILE_DATA));
    if (!win_data) RX_PANIC_OOM("malloc Windows directory iterator", sizeof(struct WIN_FILE_DATA), dir);
    if (dir && strlen(dir)) strncpy(win_data->dir, dir, MAXFILEPATH);
    else strncpy(win_data->dir, ".", MAXFILEPATH);
    if (prefix) {
        snprintf(win_data->sPath, MAXFILEPATH, "%s\\%s*.%s", win_data->dir, prefix, type);
    }
    else {
        snprintf(win_data->sPath, MAXFILEPATH, "%s\\*.%s", win_data->dir, type);
    }

    if ( (win_data->hFind = FindFirstFile(win_data->sPath, &(win_data->fdFile) ) ) == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        *dir_ptr = 0;
        free(win_data);
        errno = error == ERROR_FILE_NOT_FOUND ? 0 :
                error == ERROR_PATH_NOT_FOUND ? ENOENT : EIO;
        return 0;
    }

    *dir_ptr = win_data;

    if( win_data->fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
        return dirnxtfl(dir_ptr); /* Return the next valid file */
    }

    return win_data->fdFile.cFileName;

#elif defined(CREXX_CMS_ELF)

    if (dir_ptr) *dir_ptr = 0;
    errno = ENOSYS;
    return 0;

#else

    return 0;

#endif
}

/*
 * Get the next file from a directory (or null if there isn't one)
 * (pass the & of void *dir_ptr to hold an opaque directory context)
 */
char *dirnxtfl(void **dir_ptr) {

#if defined(CREXX_PLATFORM_CMS) && defined(CREXX_NATIVE_RAW_IO)
    return crexx_cms_dirnext(dir_ptr);
#elif defined(CREXX_PLATFORM_TSO)
    return crexx_tso_dirnext(dir_ptr);
#elif ((defined(__APPLE__) || defined(__linux__)) && !defined(CREXX_MAINFRAME_ELF)) || \
    (defined(CREXX_CMS_ELF) && defined(CREXX_CMS_DIRENT))

    struct dirent *dirent;
    struct fl_dir *ptr = dir_ptr ? *dir_ptr : NULL;
    const char *ext;
    if (!ptr) { errno = 0; return NULL; }
    do {
        errno = 0;
        dirent = readdir(ptr->d);
        if (!dirent) return 0;
        ext = filenext(dirent->d_name);
        if ( strcmp(ext,ptr->type) == 0 ) {
           if (ptr->prefix == 0 ) return dirent->d_name;
           else if ( strncmp(dirent->d_name, ptr->prefix, strlen(ptr->prefix)) == 0 ) return dirent->d_name;
        }
    } while (1);

#elif defined(_WIN32)

    struct WIN_FILE_DATA *win_data = *dir_ptr;
    if (!win_data) { errno = 0; return 0; }

    while ( FindNextFile(win_data->hFind, &(win_data->fdFile)) ) {

        if (win_data->fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

        return win_data->fdFile.cFileName;
    }
    errno = GetLastError() == ERROR_NO_MORE_FILES ? 0 : EIO;
    return 0;

#elif defined(CREXX_CMS_ELF)

    if (dir_ptr) *dir_ptr = 0;
    errno = ENOSYS;
    return 0;

#else

    return 0;

#endif
}

/*
 * Close the opaque directory context
 */
void dirclose(void **dir_ptr) {

#if defined(CREXX_PLATFORM_CMS) && defined(CREXX_NATIVE_RAW_IO)
    crexx_cms_dirclose(dir_ptr);
#elif defined(CREXX_PLATFORM_TSO)
    crexx_tso_dirclose(dir_ptr);
#elif ((defined(__APPLE__) || defined(__linux__)) && !defined(CREXX_MAINFRAME_ELF)) || \
    (defined(CREXX_CMS_ELF) && defined(CREXX_CMS_DIRENT))

    struct fl_dir *ptr = dir_ptr ? *dir_ptr : NULL;
    int saved = errno, rc;
    if (!ptr) return;
    errno = 0;
    rc = closedir(ptr->d);
    if (saved) errno = saved;
    else if (rc && !errno) errno = EIO;
    free(ptr);
    *dir_ptr = 0;

#elif defined(_WIN32)

    struct WIN_FILE_DATA *win_data = *dir_ptr;
    if (!win_data) return;

    {
        int saved = errno;
        BOOL success = FindClose(win_data->hFind);
        errno = saved ? saved : success ? 0 : EIO;
    }
    free(win_data);
    *dir_ptr = 0;

#elif defined(CREXX_CMS_ELF)
    if (dir_ptr) *dir_ptr = 0;
#endif

}

/* Returns the executable directory in a malloced buffer */
char* exepath()
{
    char *name = exefqname();
    size_t len = strlen(name);
    size_t i;

    if (!len) return name;

    for (i = len; i > 0; i--)
    {
        if ( name[i-1] == '\\' || name[i-1] == '/' )
        {
            if (i == 1) {
                name[1] = 0; /* Keep the root separator */
            } else {
                name[i-1] = 0;
            }
            break;
        }
    }
    return name;
}

/* Returns the executable path name in a malloced buffer */
char* exefqname()
{
    char *exePath = malloc(MAXFILEPATH);
    if (!exePath) RX_PANIC_OOM("malloc executable path", MAXFILEPATH, 0);

#ifdef _WIN32

    DWORD len = GetModuleFileNameA(NULL, exePath, MAXFILEPATH);
	if(len <= 0 || len == MAXFILEPATH)
	{
		// an error occured, clear exe path
		exePath[0] = '\0';
	}

#elif defined(__linux) && !defined(CREXX_MAINFRAME_ELF)

    char buf[MAXFILEPATH] = {0};
    snprintf(buf, sizeof(buf), "/proc/%d/exe", getpid());
    // readlink() doesn't null-terminate!
    ssize_t len = readlink(buf, exePath, MAXFILEPATH-1);
    if (len <= 0)
    {
        // an error occured, clear exe path
        exePath[0] = '\0';
    }
    else
    {
        exePath[len] = '\0';
    }

#elif defined(__APPLE__) && !defined(CREXX_MAINFRAME_ELF)

	uint32_t bufSize = MAXFILEPATH;
	if(_NSGetExecutablePath(exePath, &bufSize) != 0)
	{
		// an error occured, clear exe path
		exePath[0] = '\0';
	}

#else

    exePath[0] = '\0';

#endif

    return exePath;
}

/* Gets the file extention of a path */
const char *filenext(const char *filename_in) {
    const char *fname = filename(filename_in);
    const char *dot = strrchr(fname, '.');
    if(!dot || dot == fname) return "";
    return dot + 1;
}

/* Gets the filename of a path */
const char *filename(const char *path)
{
    size_t len = strlen(path);
    size_t i;
    if (!len) return "";

    for (i = len; i > 0; i--)
    {
        if ( path[i-1] == '\\' || path[i-1] == '/' )
        {
            return path + i;
        }
    }
    return path;
}

/* Gets the directory of a filename in a malloced buffer */
/* returns null if there is no directory part */
char *file_dir(const char *path)
{
    size_t len = strlen(path);
    size_t i;
    if (!len) return 0;
    char* result;

    for (i = len; i > 0; i--)
    {
        if ( path[i-1] == '\\' || path[i-1] == '/' )
        {
            if (i == 1) {
                result = malloc(2);
                if (!result) RX_PANIC_OOM("malloc file directory", 2, path);
                result[0] = path[0];
                result[1] = 0;
                return result;
            }
            result = malloc(i);
            if (!result) RX_PANIC_OOM("malloc file directory", i, path);
            result[i-1] = 0;
            memcpy(result, path, i-1);
            return result;
        }
    }

    return 0;
}
