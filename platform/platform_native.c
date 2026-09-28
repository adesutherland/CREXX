/* cREXX-owned text/record adaptation over allocation-free raw native services.
 * The backend never sees UTF-8 text or cREXX naming policy. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include "platform.h"
#if defined(CREXX_NATIVE_RAW_IO)
#include "platform_native.h"
#include "native_raw.h"
#include "text_codec.h"
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#if defined(CREXX_PLATFORM_CMS)
#define RAW(name) lab_cms_raw_##name
#elif defined(CREXX_PLATFORM_TSO)
#define RAW(name) lab_tso_raw_##name
#else
#error CREXX_NATIVE_RAW_IO requires an explicit CMS or TSO platform
#endif

typedef struct native_stream {
    lab_raw_file *raw;
    const uint32_t *map;
    crexx_utf8_state utf8;
    unsigned char *record;
    size_t capacity, used;
    unsigned char pending[4];
    unsigned pending_at, pending_count;
    int records, writing, binary, failed, newline, any_input, ended_line;
} native_stream;
static const uint32_t *selected_file_map;
static int selected_file_map_set;
static int selected_default_storage = CREXX_TEXT_STORAGE_RECORDS;
static const uint32_t *host_map(void);

void crexx_native_select_file_codec(const uint32_t *map) {
    selected_file_map = map;
    selected_file_map_set = 1;
    selected_default_storage = map == host_map() ? CREXX_TEXT_STORAGE_RECORDS :
                               CREXX_TEXT_STORAGE_BYTES;
}
static const uint32_t *file_map(void) {
    const uint32_t *map;
    if (selected_file_map_set) return selected_file_map;
    if (platform_text_codec_lookup("IBM1047", &map)) return NULL;
    return map;
}
static const uint32_t *host_map(void) {
    const uint32_t *map;
    if (platform_text_codec_lookup("IBM1047", &map)) return NULL;
    return map;
}

int crexx_native_name(const char *utf8, unsigned char *native,
                      size_t capacity, size_t *length) {
    if (!utf8 || !native || !length) { errno = EINVAL; return -1; }
    return crexx_text_convert(host_map(), 0, (const unsigned char *)utf8,
                              strlen(utf8), native, capacity, length);
}

/* The raw copy calls report required length on ERANGE. Never accept their
 * partial output or infer absence from an unsupported environment service. */
static int raw_process_value(int environment, size_t index,
                             const unsigned char *name, size_t name_length,
                             char **value) {
    size_t capacity = 64, length = 0, written, output_capacity;
    unsigned char *raw = (unsigned char *)malloc(capacity);
    char *decoded;
    int rc;
    if (!raw) return -1;
    for (;;) {
        length = 0;
        rc = environment ? RAW(environment)(name, name_length, raw, capacity, &length) :
                           RAW(argument)(index, raw, capacity, &length);
        if (environment && rc == 0) { free(raw); return 0; }
        if ((!environment && rc == 0) || (environment && rc == 1)) break;
        if (rc != -1 || errno != ERANGE || length <= capacity) {
            int error = rc == -1 && errno != ERANGE ? errno : EIO;
            free(raw); errno = error ? error : EIO; return -1;
        }
        {
            unsigned char *grown = (unsigned char *)realloc(raw, length);
            if (!grown) { free(raw); return -1; }
            raw = grown; capacity = length;
        }
    }
    if (length > capacity || length > (SIZE_MAX - 1) / 4) {
        free(raw); errno = EOVERFLOW; return -1;
    }
    output_capacity = length * 4 + 1;
    decoded = (char *)malloc(output_capacity);
    if (!decoded) { free(raw); return -1; }
    if (crexx_text_convert(host_map(), 1, raw, length, (unsigned char *)decoded,
                           output_capacity - 1, &written)) {
        int error = errno;
        free(raw); free(decoded); errno = error; return -1;
    }
    if (memchr(decoded, 0, written)) {
        free(raw); free(decoded); errno = EINVAL; return -1;
    }
    decoded[written] = 0;
    free(raw);
    *value = decoded;
    return 1;
}

void crexx_native_free_arguments(int count, char **values) {
    int i;
    if (!values) return;
    for (i = 0; i < count; ++i) free(values[i]);
    free(values);
}
int crexx_native_arguments(int *count, char ***values) {
    int n, i;
    char **result;
    if (!count || !values) { errno = EINVAL; return -1; }
    *count = 0; *values = NULL;
    n = RAW(argument_count)();
    if (n < 0) return -1;
    if ((size_t)n > SIZE_MAX / sizeof(char *) - 1) { errno = EOVERFLOW; return -1; }
    result = (char **)calloc((size_t)n + 1, sizeof(char *));
    if (!result) return -1;
    for (i = 0; i < n; ++i) {
        if (raw_process_value(0, (size_t)i, NULL, 0, &result[i]) < 0) {
            int error = errno;
            crexx_native_free_arguments(n, result);
            errno = error; return -1;
        }
    }
    *count = n; *values = result;
    return 0;
}
int crexx_native_environment(const char *name, char **value) {
    unsigned char *native;
    size_t capacity, length;
    int rc;
    if (!name || !*name || !value) { errno = EINVAL; return -1; }
    *value = NULL;
    capacity = strlen(name);
    native = (unsigned char *)malloc(capacity);
    if (!native) return -1;
    if (crexx_text_convert(host_map(), 0, (const unsigned char *)name, capacity,
                           native, capacity, &length)) {
        int error = errno;
        free(native); errno = error; return -1;
    }
    rc = raw_process_value(1, 0, native, length, value);
    free(native);
    return rc;
}

/* CMS filename, type and mode are cREXX policy. The raw service receives the
 * final native three-word identifier, with no dotted path or suffix inference. */
#if defined(CREXX_PLATFORM_CMS)
static int cms_name(const char *path, char result[28]) {
    const char *name = path, *dot, *mode = "A1";
    char selected_mode[3];
    size_t n, type_length, mode_length, i;
    if (!path) { errno = EINVAL; return -1; }
    if (name[0] == '.' && name[1] == '/') name += 2;
    else if ((name[0] == 'A' || name[0] == 'a') &&
             (name[1] == '1' || name[1] == '2' || name[1] == '5') && name[2] == '/') {
        selected_mode[0] = 'A'; selected_mode[1] = name[1]; selected_mode[2] = 0;
        mode = selected_mode; name += 3;
    }
    dot = strchr(name, '.');
    if (!dot) { errno = EINVAL; return -1; }
    n = (size_t)(dot - name);
    path = dot + 1;
    dot = strchr(path, '.');
    type_length = dot ? (size_t)(dot - path) : strlen(path);
    if (dot) mode = dot + 1;
    mode_length = strlen(mode);
    if (!n || n > 8 || !type_length || type_length > 8 || mode_length != 2 ||
        n + type_length + mode_length + 3 > 28) { errno = EINVAL; return -1; }
    for (i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)name[i];
        if (c < 33 || c > 126 || c == '/' || c == ' ') { errno = EINVAL; return -1; }
        result[i] = c >= 'a' && c <= 'z' ? (char)(c - 'a' + 'A') : (char)c;
    }
    result[n++] = ' ';
    for (i = 0; i < type_length; ++i) {
        unsigned char c = (unsigned char)path[i];
        if (c < 33 || c > 126 || c == '/' || c == ' ') { errno = EINVAL; return -1; }
        result[n++] = c >= 'a' && c <= 'z' ? (char)(c - 'a' + 'A') : (char)c;
    }
    result[n++] = ' ';
    for (i = 0; i < mode_length; ++i) {
        unsigned char c = (unsigned char)mode[i];
        if (c < 33 || c > 126 || c == '/' || c == ' ') { errno = EINVAL; return -1; }
        result[n++] = c >= 'a' && c <= 'z' ? (char)(c - 'a' + 'A') : (char)c;
    }
    result[n] = 0;
    return 0;
}

/* CMS enumeration returns the actual 18-byte FID: padded 8-byte name,
 * padded 8-byte type and two-byte mode, all native IBM1047. cREXX filters
 * type/root and creates logical source names above that raw service. */
typedef struct cms_directory {
    lab_raw_directory *raw;
    char type[9], prefix[9], mode[3], logical[18];
    int finished;
} cms_directory;
static int cms_token(const unsigned char *native, size_t length, char *output,
                     size_t capacity, int padded) {
    const uint32_t *map = host_map();
    size_t i, used = 0;
    int blanks = 0;
    if (capacity <= length) { errno = EIO; return -1; }
    for (i = 0; i < length; ++i) {
        uint32_t scalar = map[native[i]];
        if (scalar == ' ' && padded) { blanks = 1; continue; }
        if (blanks || scalar < 33 || scalar > 126 || scalar == '.' || scalar == '/') {
            errno = EIO; return -1;
        }
        output[used++] = scalar >= 'A' && scalar <= 'Z' ?
                         (char)(scalar - 'A' + 'a') : (char)scalar;
    }
    if (!used) { errno = EIO; return -1; }
    output[used] = 0;
    return 0;
}
char *crexx_cms_dirnext(void **context) {
    cms_directory *dir = context ? (cms_directory *)*context : NULL;
    unsigned char fid[32];
    size_t length;
    int rc;
    char name[9], type[9], mode[3];
    errno = 0;
    if (!dir || dir->finished) return NULL;
    for (;;) {
        rc = lab_cms_raw_directory_next(dir->raw, fid, sizeof(fid), &length);
        if (rc <= 0) {
            dir->finished = 1;
            if (!rc) errno = 0;
            else if (!errno) errno = EIO;
            return NULL;
        }
        if (length != 18 || cms_token(fid, 8, name, sizeof(name), 1) ||
            cms_token(fid + 8, 8, type, sizeof(type), 1) ||
            cms_token(fid + 16, 2, mode, sizeof(mode), 0)) {
            dir->finished = 1; if (!errno) errno = EIO; return NULL;
        }
        if (strcmp(type, dir->type) || strcmp(mode, dir->mode) ||
            strncmp(name, dir->prefix, strlen(dir->prefix))) continue;
        strcpy(dir->logical, name); strcat(dir->logical, "."); strcat(dir->logical, type);
        return dir->logical;
    }
}
char *crexx_cms_dirfirst(const char *root, const char *prefix, const char *type,
                         void **context) {
    cms_directory *dir;
    const char *mode = !root || !*root || !strcmp(root, ".") ? "A1" : root;
    char canonical_mode[3];
    unsigned char native_mode[4];
    size_t native_length;
    size_t i;
    if (context) *context = NULL;
    if (!context || !type || !*type || strlen(type) > 8 ||
        (prefix && strlen(prefix) > 8) || strlen(mode) != 2 ||
        (mode[0] != 'A' && mode[0] != 'a') ||
        (mode[1] != '1' && mode[1] != '2' && mode[1] != '5')) {
        errno = EINVAL; return NULL;
    }
    canonical_mode[0] = 'A'; canonical_mode[1] = mode[1]; canonical_mode[2] = 0;
    dir = (cms_directory *)calloc(1, sizeof(*dir));
    if (!dir) return NULL;
    for (i = 0; type[i]; ++i)
        dir->type[i] = type[i] >= 'A' && type[i] <= 'Z' ? (char)(type[i] - 'A' + 'a') : type[i];
    if (prefix) for (i = 0; prefix[i]; ++i)
        dir->prefix[i] = prefix[i] >= 'A' && prefix[i] <= 'Z' ?
                         (char)(prefix[i] - 'A' + 'a') : prefix[i];
    dir->mode[0] = 'a'; dir->mode[1] = mode[1]; dir->mode[2] = 0;
    if (crexx_native_name(canonical_mode, native_mode, sizeof(native_mode), &native_length)) {
        free(dir); return NULL;
    }
    dir->raw = lab_cms_raw_directory_open(native_mode, native_length);
    if (!dir->raw) { free(dir); return NULL; }
    *context = dir;
    return crexx_cms_dirnext(context);
}
void crexx_cms_dirclose(void **context) {
    cms_directory *dir = context ? (cms_directory *)*context : NULL;
    int saved = errno, rc;
    if (!dir) return;
    errno = 0;
    rc = lab_cms_raw_directory_close(dir->raw);
    if (saved) errno = saved;
    else if (rc && !errno) errno = EIO;
    free(dir); *context = NULL;
}
#endif

static int stream_error(native_stream *stream, int error) {
    if (!stream->failed) stream->failed = error ? error : EIO;
    errno = stream->failed;
    return -1;
}
static int raw_write_all(native_stream *stream, const unsigned char *bytes,
                         size_t count, int record_end) {
    size_t offset = 0;
    if (!count && record_end) {
        ptrdiff_t n;
        do { n = RAW(write)(stream->raw, bytes, 0, 1); } while (n < 0 && errno == EINTR);
        return n == 0 ? 0 : stream_error(stream, n < 0 ? errno : EIO);
    }
    while (offset < count) {
        ptrdiff_t n = RAW(write)(stream->raw, bytes + offset, count - offset, record_end);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0 || (size_t)n > count - offset) return stream_error(stream, n < 0 ? errno : EIO);
        offset += (size_t)n;
    }
    return 0;
}
static int commit_record(native_stream *stream) {
    if (raw_write_all(stream, stream->record, stream->used, 1)) return -1;
    stream->used = 0;
    return 0;
}
static int put_scalar(native_stream *stream, uint32_t scalar) {
    unsigned char bytes[4];
    int length;
    if (stream->records && scalar == '\n') {
        if (commit_record(stream)) return -1;
        stream->ended_line = 1;
        return 0;
    }
    length = crexx_text_encode(stream->map, scalar, bytes);
    if (length < 0) return stream_error(stream, errno);
    if (stream->records) {
        if ((size_t)length > stream->capacity - stream->used)
            return stream_error(stream, EOVERFLOW);
        memcpy(stream->record + stream->used, bytes, (size_t)length);
        stream->used += (size_t)length;
    } else if (raw_write_all(stream, bytes, (size_t)length, 0)) return -1;
    stream->ended_line = 0;
    return 0;
}
static ptrdiff_t stream_write(native_stream *stream, const char *bytes, size_t count) {
    size_t i;
    if (stream->failed) { errno = stream->failed; return -1; }
    if (stream->binary) return raw_write_all(stream, (const unsigned char *)bytes, count, 0) ? -1 : (ptrdiff_t)count;
    for (i = 0; i < count; ++i) {
        uint32_t scalar;
        int status = crexx_utf8_feed(&stream->utf8, (unsigned char)bytes[i], &scalar);
        if (status < 0) return stream_error(stream, errno);
        stream->any_input = 1;
        if (status && put_scalar(stream, scalar)) return -1;
    }
    return (ptrdiff_t)count;
}
static ptrdiff_t stream_read(native_stream *stream, char *output, size_t count) {
    size_t done = 0;
    if (stream->failed) { errno = stream->failed; return -1; }
    if (stream->binary) {
        size_t got = 0;
        int end = 0, eof = 0;
        if (!count) return 0;
        if (RAW(read)(stream->raw, (unsigned char *)output, count, &got, &end, &eof))
            return stream_error(stream, errno);
        if (end || (eof && got) || (!got && !eof)) return stream_error(stream, EIO);
        return (ptrdiff_t)got;
    }
    while (done < count) {
        unsigned char byte;
        size_t got = 0;
        int end = 0, eof = 0, status;
        uint32_t scalar;
        if (stream->pending_at < stream->pending_count) {
            output[done++] = (char)stream->pending[stream->pending_at++];
            continue;
        }
        stream->pending_at = stream->pending_count = 0;
        if (stream->newline) { output[done++] = '\n'; stream->newline = 0; continue; }
        if (RAW(read)(stream->raw, &byte, 1, &got, &end, &eof)) {
            stream_error(stream, errno); return done ? (ptrdiff_t)done : -1;
        }
        if (eof) {
            if (crexx_utf8_finish(&stream->utf8)) {
                stream_error(stream, errno); return done ? (ptrdiff_t)done : -1;
            }
            break;
        }
        if (!got && !end) {
            stream_error(stream, EIO); return done ? (ptrdiff_t)done : -1;
        }
        if (got) {
            if (stream->map) { scalar = stream->map[byte]; status = 1; }
            else status = crexx_utf8_feed(&stream->utf8, byte, &scalar);
            if (status < 0) {
                stream_error(stream, errno); return done ? (ptrdiff_t)done : -1;
            }
            if (status) {
                stream->pending_count = (unsigned)crexx_utf8_emit(scalar, stream->pending);
                if (stream->pending_count > 4) {
                    stream_error(stream, EILSEQ); return done ? (ptrdiff_t)done : -1;
                }
            }
        }
        if (end) {
            if (crexx_utf8_finish(&stream->utf8)) {
                stream_error(stream, errno); return done ? (ptrdiff_t)done : -1;
            }
            stream->newline = 1;
        }
    }
    return (ptrdiff_t)done;
}
static int stream_close(void *cookie) {
    native_stream *stream = (native_stream *)cookie;
    int error = stream->failed;
    if (stream->writing && !stream->binary) {
        if (!error && crexx_utf8_finish(&stream->utf8)) error = errno;
        if (!error && stream->records && stream->any_input && !stream->ended_line &&
            commit_record(stream)) error = stream->failed;
    }
    if (RAW(flush)(stream->raw) && !error) error = errno;
    if (RAW(close)(stream->raw) && !error) error = errno;
    free(stream->record);
    free(stream);
    if (error) { errno = error; return -1; }
    return 0;
}

#if defined(__APPLE__) || (defined(CREXX_MAINFRAME_ELF) && !defined(CREXX_NATIVE_HOST_MOCK))
static int native_read_cookie(void *cookie, char *buffer, int count) {
    ptrdiff_t n = stream_read((native_stream *)cookie, buffer, (size_t)count);
    return n > INT_MAX ? INT_MAX : (int)n;
}
static int native_write_cookie(void *cookie, const char *buffer, int count) {
    ptrdiff_t n = stream_write((native_stream *)cookie, buffer, (size_t)count);
    return n > INT_MAX ? INT_MAX : (int)n;
}
#else
static ssize_t native_read_cookie(void *cookie, char *buffer, size_t count) {
    return (ssize_t)stream_read((native_stream *)cookie, buffer, count);
}
static ssize_t native_write_cookie(void *cookie, const char *buffer, size_t count) {
    return (ssize_t)stream_write((native_stream *)cookie, buffer, count);
}
#endif

static FILE *wrap_raw_stream(lab_raw_file *raw, size_t record_capacity,
                             unsigned flags, const uint32_t *map, int binary,
                             const char *mode) {
    native_stream *stream;
    FILE *file;
    if (!raw) return NULL;
    stream = (native_stream *)calloc(1, sizeof(*stream));
    if (!stream) goto fail_raw;
    stream->raw = raw;
    stream->binary = binary;
    stream->records = !!(flags & LAB_RAW_RECORDS);
    stream->writing = !!(flags & LAB_RAW_WRITE);
    stream->map = map;
    stream->capacity = record_capacity;
    if (stream->records && !record_capacity) { errno = EIO; goto fail_stream; }
    if (stream->writing && stream->records) {
        stream->record = (unsigned char *)malloc(record_capacity);
        if (!stream->record) goto fail_stream;
    }
#if defined(__APPLE__) || (defined(CREXX_MAINFRAME_ELF) && !defined(CREXX_NATIVE_HOST_MOCK))
    file = funopen(stream, stream->writing ? NULL : native_read_cookie,
                   stream->writing ? native_write_cookie : NULL, NULL, stream_close);
#else
    {
        cookie_io_functions_t callbacks = {0};
        callbacks.read = stream->writing ? NULL : native_read_cookie;
        callbacks.write = stream->writing ? native_write_cookie : NULL;
        callbacks.close = stream_close;
        file = fopencookie(stream, mode, callbacks);
    }
#endif
    if (file) return file;
fail_stream:
    {
        int error = errno ? errno : ENOMEM;
        free(stream->record);
        free(stream);
        errno = error;
    }
fail_raw:
    {
        int error = errno ? errno : ENOMEM;
        RAW(close)(raw);
        errno = error;
        return NULL;
    }
}

FILE *crexx_native_standard(int descriptor) {
    size_t record_capacity = 0;
    unsigned flags;
    lab_raw_file *raw;
    if (descriptor < 0 || descriptor > 2) { errno = EINVAL; return NULL; }
    flags = (descriptor ? LAB_RAW_WRITE : LAB_RAW_READ) | LAB_RAW_RECORDS;
    raw = RAW(standard)(descriptor, flags, &record_capacity);
    return wrap_raw_stream(raw, record_capacity, flags, host_map(), 0,
                           descriptor ? "w" : "r");
}

FILE *crexx_native_fopen_storage(const char *name, const char *mode, int storage) {
    char mapped[28];
    unsigned char native_name[256];
    size_t native_length = 0, record_capacity = 0;
    lab_raw_file *raw;
    unsigned flags;
    const char *p;
    if (!name || !mode || !*mode) { errno = EINVAL; return NULL; }
    flags = *mode == 'r' ? LAB_RAW_READ : *mode == 'w' ? LAB_RAW_WRITE :
            *mode == 'a' ? LAB_RAW_WRITE | LAB_RAW_APPEND : 0;
    if (!flags) { errno = EINVAL; return NULL; }
    p = mode + 1;
    if (*p == 'b') ++p;
    if (*p) { errno = ENOTSUP; return NULL; }
    if (storage != CREXX_TEXT_STORAGE_DEFAULT && storage != CREXX_TEXT_STORAGE_BYTES &&
        storage != CREXX_TEXT_STORAGE_RECORDS) { errno = EINVAL; return NULL; }
    if (mode[1] != 'b' && (storage == CREXX_TEXT_STORAGE_RECORDS ||
        (storage == CREXX_TEXT_STORAGE_DEFAULT &&
         selected_default_storage == CREXX_TEXT_STORAGE_RECORDS))) flags |= LAB_RAW_RECORDS;
#if defined(CREXX_PLATFORM_CMS)
    if (cms_name(name, mapped)) return NULL;
    name = mapped;
#endif
    if (crexx_native_name(name, native_name, sizeof(native_name), &native_length)) return NULL;
    raw = RAW(open)(native_name, native_length, flags, &record_capacity);
    return wrap_raw_stream(raw, record_capacity, flags, file_map(), mode[1] == 'b', mode);
}
FILE *crexx_native_fopen(const char *name, const char *mode) {
    return crexx_native_fopen_storage(name, mode, CREXX_TEXT_STORAGE_DEFAULT);
}

static int panic_write(const unsigned char *bytes, size_t count, int end) {
    size_t used = 0;
    if (!count && end) {
        ptrdiff_t n;
        do { n = RAW(stderr)(bytes, 0, 1); } while (n < 0 && errno == EINTR);
        return n == 0 ? 0 : -1;
    }
    while (used < count) {
        ptrdiff_t n = RAW(stderr)(bytes + used, count - used, end);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0 || (size_t)n > count - used) return -1;
        used += (size_t)n;
    }
    return 0;
}
void crexx_native_panic(const char *bytes, size_t length) {
    const uint32_t *map = host_map();
    crexx_utf8_state state = {0, 0, 0};
    unsigned char chunk[256];
    size_t i, used = 0;
    for (i = 0; i < length; ++i) {
        uint32_t scalar;
        unsigned char encoded[4];
        int status = crexx_utf8_feed(&state, (unsigned char)bytes[i], &scalar);
        int count;
        if (status < 0) { state.remaining = 0; scalar = '?'; status = 1; }
        if (!status) continue;
        if (scalar == '\n') {
            if (panic_write(chunk, used, 1)) return;
            used = 0;
            continue;
        }
        count = crexx_text_encode(map, scalar, encoded);
        if (count < 0) count = crexx_text_encode(map, '?', encoded);
        if (count < 0) return;
        if (used + (size_t)count > sizeof(chunk)) {
            if (panic_write(chunk, used, 0)) return;
            used = 0;
        }
        memcpy(chunk + used, encoded, (size_t)count);
        used += (size_t)count;
    }
    if (state.remaining) {
        unsigned char fallback[4];
        int count = crexx_text_encode(map, '?', fallback);
        if (count > 0) {
            if (used + (size_t)count > sizeof(chunk)) {
                if (panic_write(chunk, used, 0)) return;
                used = 0;
            }
            memcpy(chunk + used, fallback, (size_t)count);
            used += (size_t)count;
        }
    }
    if (used) (void)panic_write(chunk, used, 0);
}
#endif
