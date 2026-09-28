/* cREXX logical names over generic TSO dataset/member and stdio services.
 * Native record formats, codecs, low buffers and OS calls remain below here. */
#include "platform.h"
#if defined(CREXX_PLATFORM_TSO)
#include "platform_native.h"
#if defined(CREXX_NATIVE_RAW_IO)
#include "native_raw.h"
#include "text_codec.h"
#else
#include "native_services.h"
#endif
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static int invalid_path(void) { errno = EINVAL; return -1; }
static FILE *open_native_file(const char *path, const char *mode, int storage) {
#if defined(CREXX_NATIVE_RAW_IO)
    return crexx_native_fopen_storage(path, mode, storage);
#else
    (void)storage;
    return fopen(path, mode);
#endif
}
/* Construct root.TYPE(NAME); never truncate native dataset/member names. */
static int member_path(char *path, const char *root, size_t root_length,
                       const char *name, size_t name_length,
                       const char *type, size_t type_length) {
    if (!root_length || !name_length || name_length > 8 || !type_length ||
        type_length > 8 || root_length + type_length + 1 > 44 ||
        memchr(root, ';', root_length) || memchr(root, '/', root_length) ||
        memchr(root, ':', root_length) || memchr(root, '(', root_length) ||
        memchr(name, '.', name_length) || memchr(name, '/', name_length))
        return invalid_path();
    memcpy(path, root, root_length); path[root_length++] = '.';
    memcpy(path + root_length, type, type_length); root_length += type_length;
    path[root_length++] = '(';
    memcpy(path + root_length, name, name_length); root_length += name_length;
    path[root_length++] = ')'; path[root_length] = 0;
    return 0;
}
FILE *crexx_tso_openfile_storage(const char *name, const char *type, const char *roots,
                                 const char *mode, int storage) {
    char path[56];
    const char *slash, *dot;
    size_t n, k;
    if (!name || !type || !mode) { invalid_path(); return NULL; }
    /* Explicit native names do not acquire a cREXX suffix or location. */
    if (strchr(name, ':') || strchr(name, '(') || name[0] == '\'')
        return open_native_file(name, mode, storage);
    slash = strchr(name, '/');
    if (slash) {
        dot = strrchr(slash + 1, '.');
        if (!dot || strchr(slash + 1, '/') ||
            member_path(path, name, (size_t)(slash - name), slash + 1,
                        (size_t)(dot - slash - 1), dot + 1, strlen(dot + 1)))
            return NULL;
        return open_native_file(path, mode, storage);
    }
    n = strlen(name); k = strlen(type);
    if (k && n > k + 1 && name[n-k-1] == '.' && !strcmp(name+n-k, type)) n -= k+1;
    if (!k && (dot = strrchr(name, '.')) != NULL) {
        n = (size_t)(dot - name); type = dot + 1; k = strlen(type);
    }
    if (!n || n > 8 || !k || k > 8) { invalid_path(); return NULL; }
    while (roots && *roots) {
        const char *end = strchr(roots, ';');
        size_t length = end ? (size_t)(end - roots) : strlen(roots);
        if (length && !(length == 1 && *roots == '.')) {
            FILE *file;
            if (member_path(path, roots, length, name, n, type, k)) return NULL;
            file = open_native_file(path, mode, storage);
            if (file) return file;
            if (errno != ENOENT && errno != ENOTDIR) return NULL;
        }
        roots = end ? end + 1 : NULL;
    }
    errno = ENOENT; /* No implicit native current directory or DD allocation. */
    return NULL;
}
FILE *crexx_tso_openfile(const char *name, const char *type, const char *roots,
                         const char *mode) {
    return crexx_tso_openfile_storage(name, type, roots, mode,
                                      CREXX_TEXT_STORAGE_DEFAULT);
}

typedef struct tso_directory {
    void *native;
    char type[9], prefix[9], name[18];
    int finished;
} tso_directory;
static void fold_ascii(char *name) {
    for (; *name; ++name) if (*name >= 'A' && *name <= 'Z') *name += 'a' - 'A';
}
char *crexx_tso_dirnext(void **context) {
    tso_directory *dir = context ? *context : NULL;
#if defined(CREXX_NATIVE_RAW_IO)
    char member[32];
    unsigned char native_member[32];
    size_t native_length, decoded_length;
    const uint32_t *native_map;
#else
    char member[9];
#endif
    int rc;
    errno = 0;
    if (!dir || dir->finished) return NULL;
    for (;;) {
#if defined(CREXX_NATIVE_RAW_IO)
        rc = lab_tso_raw_directory_next((lab_raw_directory *)dir->native,
                                        native_member, sizeof(native_member), &native_length);
        if (rc == 1) {
            if (platform_text_codec_lookup("IBM1047", &native_map) ||
                crexx_text_convert(native_map, 1, native_member, native_length,
                                   (unsigned char *)member, sizeof(member)-1, &decoded_length)) {
                dir->finished = 1;
                if (!errno) errno = EILSEQ;
                return NULL;
            }
            member[decoded_length] = 0;
        }
#else
        rc = lab_tso_directory_next(dir->native, member, sizeof(member));
#endif
        if (rc <= 0) {
            dir->finished = 1;
            if (!rc) errno = 0;
            else if (!errno) errno = EIO;
            return NULL;
        }
        if (rc != 1 || !memchr(member, 0, sizeof(member)) || !member[0] ||
            strlen(member) > 8 || strchr(member, '.')) {
            dir->finished = 1; errno = EIO; return NULL;
        }
        fold_ascii(member);
        if (strncmp(member, dir->prefix, strlen(dir->prefix))) continue;
        strcpy(dir->name, member); strcat(dir->name, "."); strcat(dir->name, dir->type);
        errno = 0;
        return dir->name;
    }
}
char *crexx_tso_dirfirst(const char *root, const char *prefix, const char *type, void **context) {
    char dataset[45];
#if defined(CREXX_NATIVE_RAW_IO)
    unsigned char native_dataset[64];
    size_t native_length;
#endif
    size_t r, k, p;
    tso_directory *dir;
    if (!context) { invalid_path(); return NULL; }
    *context = NULL;
    if (!root || !*root || !strcmp(root, ".")) { errno = ENOENT; return NULL; }
    r = strlen(root); k = type ? strlen(type) : 0; p = prefix ? strlen(prefix) : 0;
    if (!k || k > 8 || p > 8 || r + k + 1 > 44 || strchr(root, ';') ||
        strchr(root, '/') || strchr(root, ':') || strchr(root, '(')) { invalid_path(); return NULL; }
    dir = calloc(1, sizeof(*dir));
    if (!dir) return NULL;
    memcpy(dataset, root, r); dataset[r] = '.'; memcpy(dataset+r+1, type, k+1);
#if defined(CREXX_NATIVE_RAW_IO)
    if (crexx_native_name(dataset, native_dataset, sizeof(native_dataset), &native_length)) {
        free(dir);
        return NULL;
    }
    dir->native = lab_tso_raw_directory_open(native_dataset, native_length);
#else
    dir->native = lab_tso_directory_open(dataset);
#endif
    if (!dir->native) { free(dir); return NULL; }
    strcpy(dir->type, type); fold_ascii(dir->type);
    if (prefix) { strcpy(dir->prefix, prefix); fold_ascii(dir->prefix); }
    *context = dir;
    return crexx_tso_dirnext(context);
}
void crexx_tso_dirclose(void **context) {
    tso_directory *dir = context ? *context : NULL;
    int saved = errno, rc;
    if (!dir) return;
    errno = 0;
#if defined(CREXX_NATIVE_RAW_IO)
    rc = lab_tso_raw_directory_close((lab_raw_directory *)dir->native);
#else
    rc = lab_tso_directory_close(dir->native);
#endif
    if (saved) errno = saved;
    else if (rc && !errno) errno = EIO;
    free(dir); *context = NULL;
}
#endif
