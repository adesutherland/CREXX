#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
# include <process.h>
# define CREXXSAA_PATH_SEP '\\'
# define CREXXSAA_MKDIR(path) _mkdir(path)
# define CREXXSAA_RMDIR(path) _rmdir(path)
# define CREXXSAA_UNLINK(path) _unlink(path)
# define CREXXSAA_GETPID() _getpid()
# define CREXXSAA_STAT_STRUCT struct _stat
# define CREXXSAA_STAT(path, st) _stat((path), (st))
#else
# include <dirent.h>
# include <limits.h>
# include <unistd.h>
# define CREXXSAA_PATH_SEP '/'
# define CREXXSAA_MKDIR(path) mkdir((path), 0777)
# define CREXXSAA_RMDIR(path) rmdir(path)
# define CREXXSAA_UNLINK(path) unlink(path)
# define CREXXSAA_GETPID() getpid()
# define CREXXSAA_STAT_STRUCT struct stat
# define CREXXSAA_STAT(path, st) stat((path), (st))
#endif

#ifndef PATH_MAX
# define PATH_MAX 4096
#endif

#include "crexxsaa.h"
#include "rxvml.h"

#define CREXXSAA_CACHE_SCHEMA "1"
#define CREXXSAA_FNV_OFFSET UINT64_C(1469598103934665603)
#define CREXXSAA_FNV_PRIME UINT64_C(1099511628211)

typedef struct crexxsaa_address_registration {
    crexxsaa_context* ctx;
    crexxsaa_address_callback callback;
    void* userdata;
} crexxsaa_address_registration;

typedef struct crexxsaa_file_digest {
    uint64_t hash;
    long long size;
    long long mtime;
} crexxsaa_file_digest;

typedef struct crexxsaa_manifest {
    int found;
    char source_hash[17];
    char config_hash[17];
    char rxbin_path[PATH_MAX];
} crexxsaa_manifest;

struct crexxsaa_context {
    rxvml_context* rxvml;
    crexxsaa_address_registration** registrations;
    size_t registration_count;
    const rxvml_address_request* active_address_request;
    rxvml_address_response* active_address_response;
    rxvml_address_binding* response_updates;
    size_t response_update_count;
    size_t response_update_capacity;
    char** response_update_strings;
    size_t response_update_string_count;
    size_t response_update_string_capacity;
    char* library_rxbin_path;
    char* rxc_path;
    char* rxas_path;
    char* import_dir;
    char* cache_dir;
    char last_error[512];
};

static void crexxsaa_set_error(crexxsaa_context* ctx, const char* message) {
    if (!ctx) return;
    if (!message) message = "unknown CREXXSAA error";
    strncpy(ctx->last_error, message, sizeof(ctx->last_error) - 1);
    ctx->last_error[sizeof(ctx->last_error) - 1] = '\0';
}

static void crexxsaa_set_errorf(crexxsaa_context* ctx, const char* format, ...) {
    va_list args;
    if (!ctx) return;
    va_start(args, format);
    vsnprintf(ctx->last_error, sizeof(ctx->last_error), format, args);
    va_end(args);
    ctx->last_error[sizeof(ctx->last_error) - 1] = '\0';
}

static void crexxsaa_copy_rxvml_error(crexxsaa_context* ctx, const char* fallback) {
    const char* error = NULL;
    if (!ctx || !ctx->rxvml) return;
    rxvml_last_error(ctx->rxvml, &error);
    crexxsaa_set_error(ctx, error ? error : fallback);
}

static char* crexxsaa_strdup(const char* value) {
    char* copy;
    size_t len;
    if (!value) return NULL;
    len = strlen(value) + 1;
    copy = (char*)malloc(len);
    if (!copy) return NULL;
    memcpy(copy, value, len);
    return copy;
}

static int crexxsaa_replace_string(crexxsaa_context* ctx, char** target, const char* value) {
    char* copy = NULL;
    if (value && *value) {
        copy = crexxsaa_strdup(value);
        if (!copy) {
            crexxsaa_set_error(ctx, "Failed to allocate CREXXSAA configuration value");
            return -1;
        }
    }
    free(*target);
    *target = copy;
    return 0;
}

static char* crexxsaa_strndup(const char* value, size_t len) {
    char* copy;
    if (!value && len > 0) return NULL;
    copy = (char*)malloc(len + 1);
    if (!copy) return NULL;
    if (len > 0) memcpy(copy, value, len);
    copy[len] = '\0';
    return copy;
}

static int crexxsaa_char_equal_ci(char left, char right) {
    return toupper((unsigned char)left) == toupper((unsigned char)right);
}

static int crexxsaa_name_equal(const char* left, const char* right) {
    if (!left || !right) return 0;
    while (*left && *right) {
        if (!crexxsaa_char_equal_ci(*left, *right)) return 0;
        left++;
        right++;
    }
    return *left == '\0' && *right == '\0';
}

static int crexxsaa_binding_kind_is(const rxvml_address_binding* binding, const char* kind) {
    return binding && crexxsaa_name_equal(binding->kind, kind);
}

static int crexxsaa_split_variable_name(
    const char* name,
    char* base,
    size_t base_len,
    const char** tail_out) {

    const char* dot;
    size_t len;

    if (tail_out) *tail_out = NULL;
    if (!name || !*name || !base || base_len == 0) return CREXXSAA_VARIABLE_BAD_NAME;

    dot = strchr(name, '.');
    len = dot ? (size_t)(dot - name) : strlen(name);
    if (len == 0 || len >= base_len) return CREXXSAA_VARIABLE_BAD_NAME;

    memcpy(base, name, len);
    base[len] = '\0';
    if (dot && tail_out) *tail_out = dot + 1;
    return CREXXSAA_VARIABLE_OK;
}

static void crexxsaa_clear_response_updates(crexxsaa_context* ctx) {
    size_t i;
    if (!ctx) return;
    for (i = 0; i < ctx->response_update_string_count; i++) {
        free(ctx->response_update_strings[i]);
    }
    ctx->response_update_string_count = 0;
    ctx->response_update_count = 0;
    if (ctx->active_address_response) {
        ctx->active_address_response->updated_binding_count = 0;
        ctx->active_address_response->updated_bindings = NULL;
    }
}

static char* crexxsaa_track_response_string(crexxsaa_context* ctx, const char* value) {
    char** strings;
    char* copy;
    size_t new_capacity;

    if (!ctx) return NULL;
    copy = crexxsaa_strdup(value ? value : "");
    if (!copy) return NULL;

    if (ctx->response_update_string_count == ctx->response_update_string_capacity) {
        new_capacity = ctx->response_update_string_capacity ? ctx->response_update_string_capacity * 2 : 8;
        strings = (char**)realloc(ctx->response_update_strings, new_capacity * sizeof(char*));
        if (!strings) {
            free(copy);
            return NULL;
        }
        ctx->response_update_strings = strings;
        ctx->response_update_string_capacity = new_capacity;
    }

    ctx->response_update_strings[ctx->response_update_string_count++] = copy;
    return copy;
}

static int crexxsaa_ensure_response_update_capacity(crexxsaa_context* ctx) {
    rxvml_address_binding* updates;
    size_t new_capacity;

    if (!ctx) return CREXXSAA_VARIABLE_UNSUPPORTED;
    if (ctx->response_update_count < ctx->response_update_capacity) return CREXXSAA_VARIABLE_OK;

    new_capacity = ctx->response_update_capacity ? ctx->response_update_capacity * 2 : 8;
    updates = (rxvml_address_binding*)realloc(
        ctx->response_updates,
        new_capacity * sizeof(rxvml_address_binding));
    if (!updates) return CREXXSAA_VARIABLE_NO_MEMORY;

    ctx->response_updates = updates;
    ctx->response_update_capacity = new_capacity;
    return CREXXSAA_VARIABLE_OK;
}

static rxvml_address_binding* crexxsaa_find_pending_var_update(
    crexxsaa_context* ctx,
    const rxvml_address_binding* binding) {

    size_t i;

    if (!ctx || !binding) return NULL;
    for (i = ctx->response_update_count; i > 0; i--) {
        rxvml_address_binding* update = &ctx->response_updates[i - 1];
        if (!crexxsaa_binding_kind_is(update, "var")) continue;
        if (crexxsaa_name_equal(update->internal_name, binding->internal_name) ||
            crexxsaa_name_equal(update->external_alias, binding->external_alias)) {
            return update;
        }
    }
    return NULL;
}

static int crexxsaa_add_var_update(
    crexxsaa_context* ctx,
    const rxvml_address_binding* binding,
    const char* value) {

    rxvml_address_binding* update;
    char* value_copy;
    char* kind_copy;
    char* internal_copy;
    char* alias_copy;
    char* flags_copy;
    int rc;

    if (!ctx || !binding || !ctx->active_address_response) return CREXXSAA_VARIABLE_NO_ACTIVE_REQUEST;

    value_copy = crexxsaa_track_response_string(ctx, value);
    if (!value_copy) return CREXXSAA_VARIABLE_NO_MEMORY;

    update = crexxsaa_find_pending_var_update(ctx, binding);
    if (update) {
        update->value = value_copy;
        return CREXXSAA_VARIABLE_OK;
    }

    kind_copy = crexxsaa_track_response_string(ctx, "var");
    internal_copy = crexxsaa_track_response_string(ctx, binding->internal_name);
    alias_copy = crexxsaa_track_response_string(ctx, binding->external_alias);
    flags_copy = crexxsaa_track_response_string(ctx, binding->flags);
    if (!kind_copy || !internal_copy || !alias_copy || !flags_copy)
        return CREXXSAA_VARIABLE_NO_MEMORY;

    rc = crexxsaa_ensure_response_update_capacity(ctx);
    if (rc != CREXXSAA_VARIABLE_OK) return rc;

    update = &ctx->response_updates[ctx->response_update_count++];
    update->kind = kind_copy;
    update->internal_name = internal_copy;
    update->external_alias = alias_copy;
    update->value = value_copy;
    update->value_object = NULL;
    update->flags = flags_copy;

    ctx->active_address_response->updated_binding_count = ctx->response_update_count;
    ctx->active_address_response->updated_bindings = ctx->response_updates;
    return CREXXSAA_VARIABLE_OK;
}

static const rxvml_address_binding* crexxsaa_find_binding(
    const rxvml_address_request* request,
    const char* base,
    const char* kind) {

    size_t i;

    if (!request || !base || !kind) return NULL;
    for (i = 0; i < request->binding_count; i++) {
        const rxvml_address_binding* binding = &request->bindings[i];
        if (!crexxsaa_binding_kind_is(binding, kind)) continue;
        if (crexxsaa_name_equal(binding->internal_name, base) ||
            crexxsaa_name_equal(binding->external_alias, base)) {
            return binding;
        }
    }
    return NULL;
}

static int crexxsaa_tail_is_zero(const char* tail) {
    return tail && strcmp(tail, "0") == 0;
}

static int crexxsaa_tail_is_one(const char* tail) {
    return tail && strcmp(tail, "1") == 0;
}

static int crexxsaa_env_truthy(const char* name) {
    const char* value = getenv(name);
    if (!value || !*value) return 0;
    if (strcmp(value, "0") == 0) return 0;
    if (strcmp(value, "false") == 0 || strcmp(value, "FALSE") == 0) return 0;
    if (strcmp(value, "no") == 0 || strcmp(value, "NO") == 0) return 0;
    return 1;
}

static const char* crexxsaa_env_value(const char* name) {
    const char* value = getenv(name);
    return (value && *value) ? value : NULL;
}

static const char* crexxsaa_rxc_path(crexxsaa_context* ctx) {
    const char* value = crexxsaa_env_value("CREXXSAA_RXC");
    if (value) return value;
    if (ctx && ctx->rxc_path) return ctx->rxc_path;
    return "rxc";
}

static const char* crexxsaa_rxas_path(crexxsaa_context* ctx) {
    const char* value = crexxsaa_env_value("CREXXSAA_RXAS");
    if (value) return value;
    if (ctx && ctx->rxas_path) return ctx->rxas_path;
    return "rxas";
}

static const char* crexxsaa_import_dir(crexxsaa_context* ctx) {
    const char* value = crexxsaa_env_value("CREXXSAA_IMPORT_DIR");
    if (value) return value;
    if (ctx && ctx->import_dir) return ctx->import_dir;
    return ".";
}

static int crexxsaa_is_path_sep(char value) {
    if (value == '/' || value == '\\') return 1;
    return 0;
}

static char* crexxsaa_path_join(const char* left, const char* right) {
    char* path;
    size_t left_len;
    size_t right_len;
    int need_sep;

    if (!left || !*left) return crexxsaa_strdup(right ? right : "");
    if (!right || !*right) return crexxsaa_strdup(left);

    left_len = strlen(left);
    right_len = strlen(right);
    need_sep = !crexxsaa_is_path_sep(left[left_len - 1]);

    path = (char*)malloc(left_len + (need_sep ? 1 : 0) + right_len + 1);
    if (!path) return NULL;
    memcpy(path, left, left_len);
    if (need_sep) path[left_len++] = CREXXSAA_PATH_SEP;
    memcpy(path + left_len, right, right_len + 1);
    return path;
}

static char* crexxsaa_path_with_suffix(const char* base, const char* suffix) {
    char* path;
    size_t base_len = strlen(base);
    size_t suffix_len = strlen(suffix);
    path = (char*)malloc(base_len + suffix_len + 1);
    if (!path) return NULL;
    memcpy(path, base, base_len);
    memcpy(path + base_len, suffix, suffix_len + 1);
    return path;
}

static int crexxsaa_make_dir(const char* path) {
    CREXXSAA_STAT_STRUCT st;
    if (CREXXSAA_MKDIR(path) == 0) return 0;
    if (errno == EEXIST && CREXXSAA_STAT(path, &st) == 0) return 0;
    return -1;
}

static int crexxsaa_mkdir_p(const char* path) {
    char* copy;
    char* cursor;
    size_t len;

    if (!path || !*path) return -1;
    copy = crexxsaa_strdup(path);
    if (!copy) return -1;

    len = strlen(copy);
    while (len > 1 && crexxsaa_is_path_sep(copy[len - 1])) {
        copy[--len] = '\0';
    }

    cursor = copy;
#ifdef _WIN32
    if (len >= 2 && copy[1] == ':') cursor = copy + 2;
#endif
    if (crexxsaa_is_path_sep(*cursor)) cursor++;
    for (; *cursor; cursor++) {
        if (crexxsaa_is_path_sep(*cursor)) {
            *cursor = '\0';
            if (*copy && crexxsaa_make_dir(copy) != 0) {
                free(copy);
                return -1;
            }
            *cursor = CREXXSAA_PATH_SEP;
        }
    }

    if (crexxsaa_make_dir(copy) != 0) {
        free(copy);
        return -1;
    }
    free(copy);
    return 0;
}

static int crexxsaa_path_exists(const char* path) {
    CREXXSAA_STAT_STRUCT st;
    return path && CREXXSAA_STAT(path, &st) == 0;
}

static int crexxsaa_remove_tree(const char* path) {
#ifdef _WIN32
    struct _finddata_t entry;
    intptr_t handle;
    char* pattern;
    char* child;

    if (!path || !*path) return -1;
    pattern = crexxsaa_path_join(path, "*");
    if (!pattern) return -1;
    handle = _findfirst(pattern, &entry);
    free(pattern);
    if (handle == -1) {
        return (errno == ENOENT) ? 0 : -1;
    }
    do {
        if (strcmp(entry.name, ".") == 0 || strcmp(entry.name, "..") == 0)
            continue;
        child = crexxsaa_path_join(path, entry.name);
        if (!child) {
            _findclose(handle);
            return -1;
        }
        if (entry.attrib & _A_SUBDIR)
            crexxsaa_remove_tree(child);
        else
            CREXXSAA_UNLINK(child);
        free(child);
    } while (_findnext(handle, &entry) == 0);
    _findclose(handle);
    return CREXXSAA_RMDIR(path);
#else
    DIR* dir;
    struct dirent* entry;
    int rc = 0;

    if (!path || !*path) return -1;
    dir = opendir(path);
    if (!dir) return (errno == ENOENT) ? 0 : -1;

    while ((entry = readdir(dir)) != NULL) {
        char* child;
        struct stat st;
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        child = crexxsaa_path_join(path, entry->d_name);
        if (!child) {
            rc = -1;
            break;
        }
        if (lstat(child, &st) == 0 && S_ISDIR(st.st_mode))
            rc = crexxsaa_remove_tree(child);
        else if (CREXXSAA_UNLINK(child) != 0 && errno != ENOENT)
            rc = -1;
        free(child);
        if (rc != 0) break;
    }
    closedir(dir);
    if (rc == 0 && CREXXSAA_RMDIR(path) != 0 && errno != ENOENT)
        rc = -1;
    return rc;
#endif
}

static uint64_t crexxsaa_hash_update(uint64_t hash, const void* data, size_t len) {
    const unsigned char* bytes = (const unsigned char*)data;
    while (len-- > 0) {
        hash ^= (uint64_t)(*bytes++);
        hash *= CREXXSAA_FNV_PRIME;
    }
    return hash;
}

static uint64_t crexxsaa_hash_update_string(uint64_t hash, const char* value) {
    const unsigned char nul = 0;
    if (value) hash = crexxsaa_hash_update(hash, value, strlen(value));
    return crexxsaa_hash_update(hash, &nul, 1);
}

static void crexxsaa_hash_hex(uint64_t hash, char out[17]) {
    snprintf(out, 17, "%016llx", (unsigned long long)hash);
}

static int crexxsaa_file_digest_read(
    crexxsaa_context* ctx,
    const char* path,
    crexxsaa_file_digest* digest) {

    FILE* file;
    unsigned char buffer[8192];
    CREXXSAA_STAT_STRUCT st;
    size_t bytes;

    if (!path || !digest) {
        crexxsaa_set_error(ctx, "Invalid source digest arguments");
        return -1;
    }
    if (CREXXSAA_STAT(path, &st) != 0) {
        crexxsaa_set_errorf(ctx, "Unable to stat CREXX source '%s': %s", path, strerror(errno));
        return -1;
    }

    file = fopen(path, "rb");
    if (!file) {
        crexxsaa_set_errorf(ctx, "Unable to read CREXX source '%s': %s", path, strerror(errno));
        return -1;
    }

    digest->hash = CREXXSAA_FNV_OFFSET;
    digest->size = (long long)st.st_size;
    digest->mtime = (long long)st.st_mtime;
    while ((bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        digest->hash = crexxsaa_hash_update(digest->hash, buffer, bytes);
    }
    if (ferror(file)) {
        fclose(file);
        crexxsaa_set_errorf(ctx, "Unable to hash CREXX source '%s'", path);
        return -1;
    }
    fclose(file);
    return 0;
}

static uint64_t crexxsaa_config_hash(crexxsaa_context* ctx) {
    const char* paths[4];
    uint64_t hash = CREXXSAA_FNV_OFFSET;
    size_t i;

    paths[0] = crexxsaa_rxc_path(ctx);
    paths[1] = crexxsaa_rxas_path(ctx);
    paths[2] = crexxsaa_import_dir(ctx);
    paths[3] = ctx ? ctx->library_rxbin_path : NULL;

    hash = crexxsaa_hash_update_string(hash, "crexxsaa-cache");
    hash = crexxsaa_hash_update_string(hash, CREXXSAA_CACHE_SCHEMA);
    hash = crexxsaa_hash_update_string(hash, "abi");
    {
        char abi[32];
        snprintf(abi, sizeof(abi), "%d", CREXXSAA_ABI_VERSION);
        hash = crexxsaa_hash_update_string(hash, abi);
    }

    for (i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
        CREXXSAA_STAT_STRUCT st;
        char statbuf[96];
        hash = crexxsaa_hash_update_string(hash, paths[i] ? paths[i] : "");
        if (paths[i] && CREXXSAA_STAT(paths[i], &st) == 0) {
            snprintf(statbuf, sizeof(statbuf), "%lld:%lld",
                     (long long)st.st_size,
                     (long long)st.st_mtime);
            hash = crexxsaa_hash_update_string(hash, statbuf);
        }
        else {
            hash = crexxsaa_hash_update_string(hash, "missing");
        }
    }

    return hash;
}

static int crexxsaa_same_char(int left, int right) {
    if (left >= 'A' && left <= 'Z') left = left - 'A' + 'a';
    if (right >= 'A' && right <= 'Z') right = right - 'A' + 'a';
    return left == right;
}

static int crexxsaa_has_suffix(const char* value, const char* suffix) {
    size_t value_len;
    size_t suffix_len;
    size_t i;
    if (!value || !suffix) return 0;
    value_len = strlen(value);
    suffix_len = strlen(suffix);
    if (value_len < suffix_len) return 0;
    value += value_len - suffix_len;
    for (i = 0; i < suffix_len; i++) {
        if (!crexxsaa_same_char(value[i], suffix[i])) return 0;
    }
    return 1;
}

static char* crexxsaa_canonical_path(const char* path) {
    char resolved[PATH_MAX];
    if (!path) return NULL;
#ifdef _WIN32
    if (_fullpath(resolved, path, sizeof(resolved)) != NULL)
        return crexxsaa_strdup(resolved);
#else
    if (realpath(path, resolved) != NULL)
        return crexxsaa_strdup(resolved);
#endif
    return crexxsaa_strdup(path);
}

static char* crexxsaa_temp_dir(void) {
    const char* value;
    value = crexxsaa_env_value("TMPDIR");
    if (value) return crexxsaa_strdup(value);
    value = crexxsaa_env_value("TEMP");
    if (value) return crexxsaa_strdup(value);
    value = crexxsaa_env_value("TMP");
    if (value) return crexxsaa_strdup(value);
#ifdef _WIN32
    return crexxsaa_strdup(".");
#else
    return crexxsaa_strdup("/tmp");
#endif
}

static char* crexxsaa_default_cache_dir(void) {
    char* path;
    char* path2;
    const char* base;

#ifdef _WIN32
    base = crexxsaa_env_value("LOCALAPPDATA");
    if (base) {
        path = crexxsaa_path_join(base, "crexx");
        if (!path) return NULL;
        path2 = crexxsaa_path_join(path, "crexxsaa");
        free(path);
        return path2;
    }
    base = crexxsaa_env_value("USERPROFILE");
    if (!base) return NULL;
    path = crexxsaa_path_join(base, "AppData");
    if (!path) return NULL;
    path2 = crexxsaa_path_join(path, "Local");
    free(path);
    if (!path2) return NULL;
    path = crexxsaa_path_join(path2, "crexx");
    free(path2);
    if (!path) return NULL;
    path2 = crexxsaa_path_join(path, "crexxsaa");
    free(path);
    return path2;
#elif defined(__APPLE__)
    base = crexxsaa_env_value("HOME");
    if (!base) return NULL;
    path = crexxsaa_path_join(base, "Library");
    if (!path) return NULL;
    path2 = crexxsaa_path_join(path, "Caches");
    free(path);
    if (!path2) return NULL;
    path = crexxsaa_path_join(path2, "crexx");
    free(path2);
    if (!path) return NULL;
    path2 = crexxsaa_path_join(path, "crexxsaa");
    free(path);
    return path2;
#else
    base = crexxsaa_env_value("XDG_CACHE_HOME");
    if (base) {
        path = crexxsaa_path_join(base, "crexx");
        if (!path) return NULL;
        path2 = crexxsaa_path_join(path, "crexxsaa");
        free(path);
        return path2;
    }
    base = crexxsaa_env_value("HOME");
    if (!base) return NULL;
    path = crexxsaa_path_join(base, ".cache");
    if (!path) return NULL;
    path2 = crexxsaa_path_join(path, "crexx");
    free(path);
    if (!path2) return NULL;
    path = crexxsaa_path_join(path2, "crexxsaa");
    free(path2);
    return path;
#endif
}

static char* crexxsaa_resolve_cache_dir_override(crexxsaa_context* ctx, const char* cache_dir_override) {
    const char* value;
    if (cache_dir_override && *cache_dir_override)
        return crexxsaa_strdup(cache_dir_override);
    value = crexxsaa_env_value("CREXXSAA_CACHE_DIR");
    if (value) return crexxsaa_strdup(value);
    if (ctx && ctx->cache_dir) return crexxsaa_strdup(ctx->cache_dir);
    return crexxsaa_default_cache_dir();
}

static char* crexxsaa_resolve_cache_dir(crexxsaa_context* ctx) {
    return crexxsaa_resolve_cache_dir_override(ctx, NULL);
}

static void crexxsaa_cache_trace(const char* event, const char* source_path, const char* rxbin_path) {
    if (!crexxsaa_env_truthy("CREXXSAA_CACHE_TRACE")) return;
    if (rxbin_path && *rxbin_path)
        fprintf(stderr, "CREXXSAA cache %s: %s -> %s\n", event, source_path, rxbin_path);
    else
        fprintf(stderr, "CREXXSAA cache %s: %s\n", event, source_path);
}

static int crexxsaa_read_manifest(const char* path, crexxsaa_manifest* manifest) {
    FILE* file;
    char line[PATH_MAX + 64];

    memset(manifest, 0, sizeof(*manifest));
    file = fopen(path, "r");
    if (!file) return 0;
    manifest->found = 1;

    while (fgets(line, sizeof(line), file) != NULL) {
        char* value;
        char* newline;
        newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        newline = strchr(line, '\r');
        if (newline) *newline = '\0';
        value = strchr(line, '=');
        if (!value) continue;
        *value++ = '\0';
        if (strcmp(line, "source_hash") == 0) {
            strncpy(manifest->source_hash, value, sizeof(manifest->source_hash) - 1);
        }
        else if (strcmp(line, "config_hash") == 0) {
            strncpy(manifest->config_hash, value, sizeof(manifest->config_hash) - 1);
        }
        else if (strcmp(line, "rxbin") == 0) {
            strncpy(manifest->rxbin_path, value, sizeof(manifest->rxbin_path) - 1);
        }
    }
    fclose(file);
    return 0;
}

static int crexxsaa_replace_file(crexxsaa_context* ctx, const char* source, const char* target) {
#ifdef _WIN32
    CREXXSAA_UNLINK(target);
#endif
    if (rename(source, target) != 0) {
        crexxsaa_set_errorf(ctx, "Unable to move '%s' to '%s': %s", source, target, strerror(errno));
        return -1;
    }
    return 0;
}

static int crexxsaa_write_manifest(
    crexxsaa_context* ctx,
    const char* manifest_path,
    const char* canonical_source_path,
    const char* source_hash,
    const crexxsaa_file_digest* digest,
    const char* config_hash,
    const char* rxbin_path) {

    char suffix[64];
    char* temp_path;
    FILE* file;
    int rc = 0;

    snprintf(suffix, sizeof(suffix), ".tmp.%ld", (long)CREXXSAA_GETPID());
    temp_path = crexxsaa_path_with_suffix(manifest_path, suffix);
    if (!temp_path) {
        crexxsaa_set_error(ctx, "Failed to allocate CREXXSAA manifest path");
        return -1;
    }

    file = fopen(temp_path, "w");
    if (!file) {
        crexxsaa_set_errorf(ctx, "Unable to write CREXXSAA manifest '%s': %s", temp_path, strerror(errno));
        free(temp_path);
        return -1;
    }

    if (fprintf(file,
                "version=%s\nsource_path=%s\nsource_hash=%s\nsource_size=%lld\nsource_mtime=%lld\nconfig_hash=%s\nrxbin=%s\n",
                CREXXSAA_CACHE_SCHEMA,
                canonical_source_path,
                source_hash,
                digest->size,
                digest->mtime,
                config_hash,
                rxbin_path) < 0) {
        rc = -1;
    }
    if (fclose(file) != 0) rc = -1;
    if (rc != 0) {
        crexxsaa_set_errorf(ctx, "Unable to write CREXXSAA manifest '%s'", temp_path);
        CREXXSAA_UNLINK(temp_path);
        free(temp_path);
        return -1;
    }

    rc = crexxsaa_replace_file(ctx, temp_path, manifest_path);
    if (rc != 0) CREXXSAA_UNLINK(temp_path);
    free(temp_path);
    return rc;
}

static size_t crexxsaa_shell_quote_length(const char* value) {
    size_t len = 2;
    while (value && *value) {
#ifdef _WIN32
        len += (*value == '"') ? 2 : 1;
#else
        len += (*value == '\'') ? 4 : 1;
#endif
        value++;
    }
    return len;
}

static char* crexxsaa_append_shell_quoted(char* target, const char* value) {
#ifdef _WIN32
    *target++ = '"';
    while (value && *value) {
        if (*value == '"') *target++ = '\\';
        *target++ = *value++;
    }
    *target++ = '"';
#else
    *target++ = '\'';
    while (value && *value) {
        if (*value == '\'') {
            *target++ = '\'';
            *target++ = '\\';
            *target++ = '\'';
            *target++ = '\'';
        }
        else {
            *target++ = *value;
        }
        value++;
    }
    *target++ = '\'';
#endif
    *target = '\0';
    return target;
}

static char* crexxsaa_build_rxc_command(crexxsaa_context* ctx, const char* source, const char* output_base) {
    const char* rxc = crexxsaa_rxc_path(ctx);
    const char* import_dir = crexxsaa_import_dir(ctx);
    size_t len;
    char* command;
    char* cursor;

    len = crexxsaa_shell_quote_length(rxc)
        + strlen(" -i ")
        + crexxsaa_shell_quote_length(import_dir)
        + strlen(" -o ")
        + crexxsaa_shell_quote_length(output_base)
        + 1
        + crexxsaa_shell_quote_length(source)
        + 1;
    command = (char*)malloc(len);
    if (!command) return NULL;

    cursor = crexxsaa_append_shell_quoted(command, rxc);
    strcpy(cursor, " -i ");
    cursor += strlen(cursor);
    cursor = crexxsaa_append_shell_quoted(cursor, import_dir);
    strcpy(cursor, " -o ");
    cursor += strlen(cursor);
    cursor = crexxsaa_append_shell_quoted(cursor, output_base);
    *cursor++ = ' ';
    crexxsaa_append_shell_quoted(cursor, source);
    return command;
}

static char* crexxsaa_build_rxas_command(crexxsaa_context* ctx, const char* rxas_source, const char* output_base) {
    const char* rxas = crexxsaa_rxas_path(ctx);
    size_t len;
    char* command;
    char* cursor;

    len = crexxsaa_shell_quote_length(rxas)
        + strlen(" -o ")
        + crexxsaa_shell_quote_length(output_base)
        + 1
        + crexxsaa_shell_quote_length(rxas_source)
        + 1;
    command = (char*)malloc(len);
    if (!command) return NULL;

    cursor = crexxsaa_append_shell_quoted(command, rxas);
    strcpy(cursor, " -o ");
    cursor += strlen(cursor);
    cursor = crexxsaa_append_shell_quoted(cursor, output_base);
    *cursor++ = ' ';
    crexxsaa_append_shell_quoted(cursor, rxas_source);
    return command;
}

static int crexxsaa_run_tool(crexxsaa_context* ctx, const char* label, char* command) {
    int rc;
    if (!command) {
        crexxsaa_set_error(ctx, "Failed to allocate CREXX compiler command");
        return -1;
    }
    rc = system(command);
    if (rc != 0) {
        crexxsaa_set_errorf(ctx, "%s failed with status %d", label, rc);
        free(command);
        return -1;
    }
    free(command);
    return 0;
}

static int crexxsaa_compile_source(
    crexxsaa_context* ctx,
    const char* source_path,
    const char* output_base,
    char** rxas_path_out,
    char** rxbin_path_out) {

    char* rxas_path = NULL;
    char* rxbin_path = NULL;

    if (rxas_path_out) *rxas_path_out = NULL;
    if (rxbin_path_out) *rxbin_path_out = NULL;

    rxas_path = crexxsaa_path_with_suffix(output_base, ".rxas");
    rxbin_path = crexxsaa_path_with_suffix(output_base, ".rxbin");
    if (!rxas_path || !rxbin_path) {
        free(rxas_path);
        free(rxbin_path);
        crexxsaa_set_error(ctx, "Failed to allocate CREXX compiler output paths");
        return -1;
    }

    if (crexxsaa_run_tool(
            ctx,
            "rxc",
            crexxsaa_build_rxc_command(ctx, source_path, output_base)) != 0) {
        free(rxas_path);
        free(rxbin_path);
        return -1;
    }

    if (crexxsaa_run_tool(
            ctx,
            "rxas",
            crexxsaa_build_rxas_command(ctx, rxas_path, output_base)) != 0) {
        CREXXSAA_UNLINK(rxas_path);
        free(rxas_path);
        free(rxbin_path);
        return -1;
    }

    if (rxas_path_out) *rxas_path_out = rxas_path;
    else free(rxas_path);
    if (rxbin_path_out) *rxbin_path_out = rxbin_path;
    else free(rxbin_path);
    return 0;
}

static int crexxsaa_run_uncached_source(
    crexxsaa_context* ctx,
    const char* source_path,
    int argc,
    const char** argv,
    int* program_rc) {

    char suffix[96];
    char* temp_dir = NULL;
    char* output_base = NULL;
    char* rxas_path = NULL;
    char* rxbin_path = NULL;
    int rc = -1;

    temp_dir = crexxsaa_temp_dir();
    if (!temp_dir) {
        crexxsaa_set_error(ctx, "Unable to resolve temporary directory for CREXX compilation");
        goto cleanup;
    }

    snprintf(suffix, sizeof(suffix), "crexxsaa-%ld-%lld",
             (long)CREXXSAA_GETPID(),
             (long long)time(NULL));
    output_base = crexxsaa_path_join(temp_dir, suffix);
    if (!output_base) {
        crexxsaa_set_error(ctx, "Failed to allocate temporary CREXX output path");
        goto cleanup;
    }

    if (crexxsaa_compile_source(ctx, source_path, output_base, &rxas_path, &rxbin_path) != 0)
        goto cleanup;

    rc = crexxsaa_run_rxbin(ctx, rxbin_path, argc, argv, program_rc);

cleanup:
    if (rxas_path) CREXXSAA_UNLINK(rxas_path);
    if (rxbin_path) CREXXSAA_UNLINK(rxbin_path);
    free(rxas_path);
    free(rxbin_path);
    free(output_base);
    free(temp_dir);
    return rc;
}

static int crexxsaa_address_trampoline(
    rxvml_context* rxvml,
    const rxvml_address_request* rxvml_request,
    rxvml_address_response* rxvml_response,
    void* userdata) {

    crexxsaa_address_registration* registration;
    crexxsaa_context* ctx;
    crexxsaa_address_request request;
    crexxsaa_address_response response;
    const rxvml_address_request* previous_request;
    rxvml_address_response* previous_response;
    int rc;

    (void)rxvml;

    registration = (crexxsaa_address_registration*)userdata;
    if (!registration || !registration->callback || !rxvml_request || !rxvml_response) {
        if (rxvml_response) rxvml_response->rc = -1;
        return -1;
    }

    ctx = registration->ctx;
    request.environment_name = rxvml_request->environment_name;
    request.command = rxvml_request->command;
    request.context = ctx;

    memset(&response, 0, sizeof(response));
    previous_request = ctx ? ctx->active_address_request : NULL;
    previous_response = ctx ? ctx->active_address_response : NULL;
    if (ctx) {
        ctx->active_address_request = rxvml_request;
        ctx->active_address_response = rxvml_response;
        crexxsaa_clear_response_updates(ctx);
    }

    rc = registration->callback(&request, &response, registration->userdata);

    rxvml_response->rc = response.rc;
    rxvml_response->condition_name = response.condition_name;
    rxvml_response->diagnostic = response.diagnostic;
    if (ctx) {
        ctx->active_address_request = previous_request;
        ctx->active_address_response = previous_response;
    }

    return rc;
}

int crexxsaa_create(
    const char* location,
    const char* library_rxbin_path,
    crexxsaa_context** ctx_out) {

    crexxsaa_context* ctx;

    if (!ctx_out) return -1;
    *ctx_out = NULL;

    ctx = (crexxsaa_context*)calloc(1, sizeof(crexxsaa_context));
    if (!ctx) return -1;

    if (library_rxbin_path && *library_rxbin_path) {
        ctx->library_rxbin_path = crexxsaa_strdup(library_rxbin_path);
        if (!ctx->library_rxbin_path) {
            crexxsaa_set_error(ctx, "Failed to allocate CREXX library path");
            crexxsaa_destroy(ctx);
            return -1;
        }
    }

    ctx->rxvml = rxvml_create(location, 0);
    if (!ctx->rxvml) {
        crexxsaa_set_error(ctx, "Failed to create rxvml context");
        crexxsaa_destroy(ctx);
        return -1;
    }

    if (library_rxbin_path && *library_rxbin_path) {
        if (rxvml_load_module_file(ctx->rxvml, library_rxbin_path) <= 0) {
            crexxsaa_copy_rxvml_error(ctx, "Failed to load CREXX library module");
            crexxsaa_destroy(ctx);
            return -1;
        }
    }

    *ctx_out = ctx;
    return 0;
}

void crexxsaa_destroy(crexxsaa_context* ctx) {
    size_t i;
    if (!ctx) return;
    if (ctx->rxvml) rxvml_destroy(ctx->rxvml);
    crexxsaa_clear_response_updates(ctx);
    free(ctx->response_updates);
    free(ctx->response_update_strings);
    for (i = 0; i < ctx->registration_count; i++) {
        free(ctx->registrations[i]);
    }
    free(ctx->registrations);
    free(ctx->library_rxbin_path);
    free(ctx->rxc_path);
    free(ctx->rxas_path);
    free(ctx->import_dir);
    free(ctx->cache_dir);
    free(ctx);
}

int crexxsaa_register_address_environment(
    crexxsaa_context* ctx,
    const char* env_name,
    crexxsaa_address_callback callback,
    void* userdata) {

    crexxsaa_address_registration** registrations;
    crexxsaa_address_registration* registration;

    if (!ctx || !ctx->rxvml || !env_name || !callback) {
        crexxsaa_set_error(ctx, "Invalid ADDRESS registration arguments");
        return -1;
    }

    registrations = (crexxsaa_address_registration**)realloc(
        ctx->registrations,
        (ctx->registration_count + 1) * sizeof(crexxsaa_address_registration*));
    if (!registrations) {
        crexxsaa_set_error(ctx, "Failed to allocate ADDRESS registration");
        return -1;
    }

    ctx->registrations = registrations;
    registration = (crexxsaa_address_registration*)calloc(1, sizeof(crexxsaa_address_registration));
    if (!registration) {
        crexxsaa_set_error(ctx, "Failed to allocate ADDRESS registration");
        return -1;
    }
    registration->ctx = ctx;
    registration->callback = callback;
    registration->userdata = userdata;
    ctx->registrations[ctx->registration_count++] = registration;

    if (rxvml_address_register_callback_environment(
            ctx->rxvml,
            env_name,
            NULL,
            crexxsaa_address_trampoline,
            NULL,
            registration) != 0) {
        ctx->registration_count--;
        free(registration);
        crexxsaa_copy_rxvml_error(ctx, "Failed to register ADDRESS callback environment");
        return -1;
    }

    return 0;
}

int crexxsaa_set_address_environment(
    crexxsaa_context* ctx,
    const char* env_name) {

    if (!ctx || !ctx->rxvml || !env_name) {
        crexxsaa_set_error(ctx, "Invalid ADDRESS environment arguments");
        return -1;
    }

    if (rxvml_address_set_environment(ctx->rxvml, env_name) != 0) {
        crexxsaa_copy_rxvml_error(ctx, "Failed to set ADDRESS environment");
        return -1;
    }

    return 0;
}

int crexxsaa_set_compiler(
    crexxsaa_context* ctx,
    const char* rxc_path,
    const char* rxas_path,
    const char* import_dir) {

    if (!ctx) return -1;
    if (crexxsaa_replace_string(ctx, &ctx->rxc_path, rxc_path) != 0) return -1;
    if (crexxsaa_replace_string(ctx, &ctx->rxas_path, rxas_path) != 0) return -1;
    if (crexxsaa_replace_string(ctx, &ctx->import_dir, import_dir) != 0) return -1;
    return 0;
}

int crexxsaa_set_cache_dir(
    crexxsaa_context* ctx,
    const char* cache_dir) {

    if (!ctx) return -1;
    return crexxsaa_replace_string(ctx, &ctx->cache_dir, cache_dir);
}

int crexxsaa_address_variable_set(
    crexxsaa_context* ctx,
    const char* name,
    const char* value,
    size_t value_len) {

    const rxvml_address_request* request;
    const rxvml_address_binding* binding;
    char base[256];
    const char* tail = NULL;
    char* value_text;
    int rc;

    if (!ctx || !ctx->rxvml) return CREXXSAA_VARIABLE_UNSUPPORTED;
    request = ctx->active_address_request;
    if (!request) {
        crexxsaa_set_error(ctx, "No active CREXXSAA ADDRESS request");
        return CREXXSAA_VARIABLE_NO_ACTIVE_REQUEST;
    }

    rc = crexxsaa_split_variable_name(name, base, sizeof(base), &tail);
    if (rc != CREXXSAA_VARIABLE_OK) {
        crexxsaa_set_error(ctx, "Invalid CREXXSAA variable name");
        return rc;
    }

    value_text = crexxsaa_strndup(value ? value : "", value ? value_len : 0);
    if (!value_text) {
        crexxsaa_set_error(ctx, "Failed to allocate CREXXSAA variable value");
        return CREXXSAA_VARIABLE_NO_MEMORY;
    }

    if (tail && *tail) {
        binding = crexxsaa_find_binding(request, base, "stem");
        if (binding) {
            rc = rxvml_address_binding_stem_set(ctx->rxvml, binding, tail, value_text);
            free(value_text);
            if (rc == 0) return CREXXSAA_VARIABLE_OK;
            crexxsaa_copy_rxvml_error(ctx, "Failed to update CREXX ADDRESS stem binding");
            return CREXXSAA_VARIABLE_UNSUPPORTED;
        }

        binding = crexxsaa_find_binding(request, base, "var");
        if (binding) {
            if (crexxsaa_tail_is_zero(tail)) {
                free(value_text);
                return CREXXSAA_VARIABLE_OK;
            }
            rc = crexxsaa_add_var_update(ctx, binding, value_text);
            free(value_text);
            if (rc != CREXXSAA_VARIABLE_OK)
                crexxsaa_set_error(ctx, "Failed to record CREXX ADDRESS variable update");
            return rc;
        }
    } else if (!tail) {
        binding = crexxsaa_find_binding(request, base, "var");
        if (binding) {
            rc = crexxsaa_add_var_update(ctx, binding, value_text);
            free(value_text);
            if (rc != CREXXSAA_VARIABLE_OK)
                crexxsaa_set_error(ctx, "Failed to record CREXX ADDRESS variable update");
            return rc;
        }
    }

    rc = rxvml_address_sandbox_set(ctx->rxvml, request, name, value_text);
    free(value_text);
    if (rc == 0) return CREXXSAA_VARIABLE_OK;

    crexxsaa_copy_rxvml_error(ctx, "Failed to update CREXX ADDRESS sandbox variable");
    return CREXXSAA_VARIABLE_UNSUPPORTED;
}

int crexxsaa_address_variable_get_alloc(
    crexxsaa_context* ctx,
    const char* name,
    char** value_out,
    size_t* value_len_out) {

    const rxvml_address_request* request;
    const rxvml_address_binding* binding;
    rxvml_address_binding* pending;
    char base[256];
    const char* tail = NULL;
    const char* direct_value = NULL;
    char* value = NULL;
    int rc;

    if (value_out) *value_out = NULL;
    if (value_len_out) *value_len_out = 0;
    if (!ctx || !ctx->rxvml || !value_out) return CREXXSAA_VARIABLE_UNSUPPORTED;

    request = ctx->active_address_request;
    if (!request) {
        crexxsaa_set_error(ctx, "No active CREXXSAA ADDRESS request");
        return CREXXSAA_VARIABLE_NO_ACTIVE_REQUEST;
    }

    rc = crexxsaa_split_variable_name(name, base, sizeof(base), &tail);
    if (rc != CREXXSAA_VARIABLE_OK) {
        crexxsaa_set_error(ctx, "Invalid CREXXSAA variable name");
        return rc;
    }

    if (tail && *tail) {
        binding = crexxsaa_find_binding(request, base, "stem");
        if (binding) {
            value = (char*)malloc(65536);
            if (!value) {
                crexxsaa_set_error(ctx, "Failed to allocate CREXXSAA stem value buffer");
                return CREXXSAA_VARIABLE_NO_MEMORY;
            }
            rc = rxvml_address_binding_stem_get(binding, tail, value, 65536);
            if (rc == 0) {
                *value_out = value;
                if (value_len_out) *value_len_out = strlen(value);
                return CREXXSAA_VARIABLE_OK;
            }
            free(value);
        }

        binding = crexxsaa_find_binding(request, base, "var");
        if (binding && (crexxsaa_tail_is_zero(tail) || crexxsaa_tail_is_one(tail))) {
            pending = crexxsaa_find_pending_var_update(ctx, binding);
            direct_value = crexxsaa_tail_is_zero(tail) ? "1" : (pending ? pending->value : binding->value);
            if (!direct_value) direct_value = "";
            value = crexxsaa_strdup(direct_value);
            if (!value) {
                crexxsaa_set_error(ctx, "Failed to allocate CREXXSAA variable value");
                return CREXXSAA_VARIABLE_NO_MEMORY;
            }
            *value_out = value;
            if (value_len_out) *value_len_out = strlen(value);
            return CREXXSAA_VARIABLE_OK;
        }
    } else if (!tail) {
        binding = crexxsaa_find_binding(request, base, "var");
        if (binding) {
            pending = crexxsaa_find_pending_var_update(ctx, binding);
            direct_value = pending ? pending->value : binding->value;
            if (!direct_value) direct_value = "";
            value = crexxsaa_strdup(direct_value);
            if (!value) {
                crexxsaa_set_error(ctx, "Failed to allocate CREXXSAA variable value");
                return CREXXSAA_VARIABLE_NO_MEMORY;
            }
            *value_out = value;
            if (value_len_out) *value_len_out = strlen(value);
            return CREXXSAA_VARIABLE_OK;
        }
    }

    value = (char*)malloc(65536);
    if (!value) {
        crexxsaa_set_error(ctx, "Failed to allocate CREXXSAA sandbox value buffer");
        return CREXXSAA_VARIABLE_NO_MEMORY;
    }
    rc = rxvml_address_sandbox_get(request, name, value, 65536);
    if (rc == 0) {
        *value_out = value;
        if (value_len_out) *value_len_out = strlen(value);
        return CREXXSAA_VARIABLE_OK;
    }
    free(value);

    crexxsaa_set_error(ctx, "CREXXSAA variable was not exposed and was not found in the ADDRESS sandbox");
    return CREXXSAA_VARIABLE_NOT_FOUND;
}

void crexxsaa_free(void* ptr) {
    free(ptr);
}

int crexxsaa_run_rxbin(
    crexxsaa_context* ctx,
    const char* rxbin_path,
    int argc,
    const char** argv,
    int* program_rc) {

    if (program_rc) *program_rc = 0;
    if (!ctx || !ctx->rxvml || !rxbin_path) {
        crexxsaa_set_error(ctx, "Invalid CREXX run arguments");
        return -1;
    }

    if (rxvml_load_module_file(ctx->rxvml, rxbin_path) <= 0) {
        crexxsaa_copy_rxvml_error(ctx, "Failed to load CREXX program module");
        return -1;
    }

    if (rxvml_run(ctx->rxvml, argc, argv, program_rc) != 0) {
        crexxsaa_copy_rxvml_error(ctx, "Failed to run CREXX program");
        return -1;
    }

    return 0;
}

int crexxsaa_run_source(
    crexxsaa_context* ctx,
    const char* source_path,
    const char* cache_namespace,
    unsigned flags,
    int argc,
    const char** argv,
    int* program_rc) {

    crexxsaa_file_digest digest;
    crexxsaa_manifest manifest;
    char source_hash[17];
    char config_hash[17];
    char object_hash[17];
    char source_key[17];
    uint64_t hash;
    uint64_t cfg_hash;
    char* canonical_path = NULL;
    char* cache_root = NULL;
    char* version_dir = NULL;
    char* source_dir = NULL;
    char* manifest_path = NULL;
    char* final_rxbin_path = NULL;
    char* object_name = NULL;
    char* temp_base = NULL;
    char* temp_rxas_path = NULL;
    char* temp_rxbin_path = NULL;
    const char* namespace_value = cache_namespace ? cache_namespace : "";
    int refresh;
    int rc = -1;

    if (program_rc) *program_rc = 0;
    if (!ctx || !source_path || !*source_path) {
        crexxsaa_set_error(ctx, "Invalid CREXX source run arguments");
        return -1;
    }

    if (crexxsaa_has_suffix(source_path, ".rxbin"))
        return crexxsaa_run_rxbin(ctx, source_path, argc, argv, program_rc);

    if (crexxsaa_env_truthy("CREXXSAA_CACHE_DISABLE"))
        flags |= CREXXSAA_CACHE_DISABLE;
    if (crexxsaa_env_truthy("CREXXSAA_CACHE_REFRESH"))
        flags |= CREXXSAA_CACHE_REFRESH;

    if (flags & CREXXSAA_CACHE_DISABLE) {
        crexxsaa_cache_trace("disabled", source_path, NULL);
        return crexxsaa_run_uncached_source(ctx, source_path, argc, argv, program_rc);
    }

    if (crexxsaa_file_digest_read(ctx, source_path, &digest) != 0)
        goto cleanup;
    crexxsaa_hash_hex(digest.hash, source_hash);

    cfg_hash = crexxsaa_config_hash(ctx);
    crexxsaa_hash_hex(cfg_hash, config_hash);

    canonical_path = crexxsaa_canonical_path(source_path);
    cache_root = crexxsaa_resolve_cache_dir(ctx);
    if (!canonical_path || !cache_root) {
        crexxsaa_set_error(ctx, "Unable to resolve CREXXSAA cache paths");
        goto cleanup;
    }

    hash = CREXXSAA_FNV_OFFSET;
    hash = crexxsaa_hash_update_string(hash, namespace_value);
    hash = crexxsaa_hash_update_string(hash, canonical_path);
    crexxsaa_hash_hex(hash, source_key);

    version_dir = crexxsaa_path_join(cache_root, "v" CREXXSAA_CACHE_SCHEMA);
    source_dir = crexxsaa_path_join(version_dir, source_key);
    manifest_path = crexxsaa_path_join(source_dir, "manifest");
    if (!version_dir || !source_dir || !manifest_path) {
        crexxsaa_set_error(ctx, "Failed to allocate CREXXSAA cache paths");
        goto cleanup;
    }
    if (crexxsaa_mkdir_p(source_dir) != 0) {
        crexxsaa_set_errorf(ctx, "Unable to create CREXXSAA cache directory '%s': %s",
                            source_dir, strerror(errno));
        goto cleanup;
    }

    hash = CREXXSAA_FNV_OFFSET;
    hash = crexxsaa_hash_update_string(hash, source_hash);
    hash = crexxsaa_hash_update_string(hash, config_hash);
    crexxsaa_hash_hex(hash, object_hash);
    object_name = crexxsaa_path_with_suffix(object_hash, ".rxbin");
    final_rxbin_path = crexxsaa_path_join(source_dir, object_name);
    if (!object_name || !final_rxbin_path) {
        crexxsaa_set_error(ctx, "Failed to allocate CREXXSAA cached RXBIN path");
        goto cleanup;
    }

    crexxsaa_read_manifest(manifest_path, &manifest);
    refresh = (flags & CREXXSAA_CACHE_REFRESH) != 0;
    if (!refresh
        && manifest.found
        && strcmp(manifest.source_hash, source_hash) == 0
        && strcmp(manifest.config_hash, config_hash) == 0
        && crexxsaa_path_exists(manifest.rxbin_path)) {
        crexxsaa_cache_trace("hit", source_path, manifest.rxbin_path);
        rc = crexxsaa_run_rxbin(ctx, manifest.rxbin_path, argc, argv, program_rc);
        goto cleanup;
    }

    if (refresh)
        crexxsaa_cache_trace("refresh", source_path, final_rxbin_path);
    else if (manifest.found)
        crexxsaa_cache_trace("stale", source_path, final_rxbin_path);
    else
        crexxsaa_cache_trace("miss", source_path, final_rxbin_path);

    {
        char suffix[96];
        snprintf(suffix, sizeof(suffix), "%s.%ld.tmp", object_hash, (long)CREXXSAA_GETPID());
        temp_base = crexxsaa_path_join(source_dir, suffix);
    }
    if (!temp_base) {
        crexxsaa_set_error(ctx, "Failed to allocate CREXXSAA temporary output path");
        goto cleanup;
    }

    if (crexxsaa_compile_source(ctx, source_path, temp_base, &temp_rxas_path, &temp_rxbin_path) != 0)
        goto cleanup;

    if (crexxsaa_replace_file(ctx, temp_rxbin_path, final_rxbin_path) != 0)
        goto cleanup;
    free(temp_rxbin_path);
    temp_rxbin_path = NULL;

    if (crexxsaa_write_manifest(ctx,
                                manifest_path,
                                canonical_path,
                                source_hash,
                                &digest,
                                config_hash,
                                final_rxbin_path) != 0)
        goto cleanup;

    if (manifest.found
        && manifest.rxbin_path[0] != '\0'
        && strcmp(manifest.rxbin_path, final_rxbin_path) != 0) {
        CREXXSAA_UNLINK(manifest.rxbin_path);
    }

    rc = crexxsaa_run_rxbin(ctx, final_rxbin_path, argc, argv, program_rc);

cleanup:
    if (temp_rxas_path) CREXXSAA_UNLINK(temp_rxas_path);
    if (temp_rxbin_path) CREXXSAA_UNLINK(temp_rxbin_path);
    free(temp_rxas_path);
    free(temp_rxbin_path);
    free(temp_base);
    free(object_name);
    free(final_rxbin_path);
    free(manifest_path);
    free(source_dir);
    free(version_dir);
    free(cache_root);
    free(canonical_path);
    return rc;
}

int crexxsaa_invalidate_source(
    crexxsaa_context* ctx,
    const char* source_path,
    const char* cache_namespace) {

    char source_key[17];
    uint64_t hash;
    char* canonical_path = NULL;
    char* cache_root = NULL;
    char* version_dir = NULL;
    char* source_dir = NULL;
    const char* namespace_value = cache_namespace ? cache_namespace : "";
    int rc = -1;

    if (!ctx || !source_path || !*source_path) {
        crexxsaa_set_error(ctx, "Invalid CREXXSAA invalidate-source arguments");
        return -1;
    }

    canonical_path = crexxsaa_canonical_path(source_path);
    cache_root = crexxsaa_resolve_cache_dir(ctx);
    if (!canonical_path || !cache_root) {
        crexxsaa_set_error(ctx, "Unable to resolve CREXXSAA cache paths");
        goto cleanup;
    }

    hash = CREXXSAA_FNV_OFFSET;
    hash = crexxsaa_hash_update_string(hash, namespace_value);
    hash = crexxsaa_hash_update_string(hash, canonical_path);
    crexxsaa_hash_hex(hash, source_key);

    version_dir = crexxsaa_path_join(cache_root, "v" CREXXSAA_CACHE_SCHEMA);
    source_dir = crexxsaa_path_join(version_dir, source_key);
    if (!version_dir || !source_dir) {
        crexxsaa_set_error(ctx, "Failed to allocate CREXXSAA cache paths");
        goto cleanup;
    }

    if (crexxsaa_remove_tree(source_dir) != 0 && errno != ENOENT) {
        crexxsaa_set_errorf(ctx, "Unable to invalidate CREXXSAA cache '%s': %s",
                            source_dir, strerror(errno));
        goto cleanup;
    }
    rc = 0;

cleanup:
    free(source_dir);
    free(version_dir);
    free(cache_root);
    free(canonical_path);
    return rc;
}

int crexxsaa_invalidate_all(crexxsaa_context* ctx) {
    char* cache_root;
    char* version_dir;
    int rc = 0;

    if (!ctx) return -1;
    cache_root = crexxsaa_resolve_cache_dir(ctx);
    if (!cache_root) {
        crexxsaa_set_error(ctx, "Unable to resolve CREXXSAA cache path");
        return -1;
    }
    version_dir = crexxsaa_path_join(cache_root, "v" CREXXSAA_CACHE_SCHEMA);
    if (!version_dir) {
        free(cache_root);
        crexxsaa_set_error(ctx, "Failed to allocate CREXXSAA cache path");
        return -1;
    }
    if (crexxsaa_remove_tree(version_dir) != 0 && errno != ENOENT) {
        crexxsaa_set_errorf(ctx, "Unable to invalidate CREXXSAA cache '%s': %s",
                            version_dir, strerror(errno));
        rc = -1;
    }
    free(version_dir);
    free(cache_root);
    return rc;
}

int crexxsaa_get_cache_dir(
    const char* cache_dir_override,
    char* buffer,
    size_t buffer_len) {

    char* cache_root;
    size_t len;

    if (!buffer || buffer_len == 0) return -1;
    buffer[0] = '\0';

    cache_root = crexxsaa_resolve_cache_dir_override(NULL, cache_dir_override);
    if (!cache_root) return -1;

    len = strlen(cache_root);
    if (len + 1 > buffer_len) {
        free(cache_root);
        return -1;
    }

    memcpy(buffer, cache_root, len + 1);
    free(cache_root);
    return 0;
}

int crexxsaa_clear_cache(const char* cache_dir_override) {
    char* cache_root;
    char* version_dir;
    int rc = 0;

    cache_root = crexxsaa_resolve_cache_dir_override(NULL, cache_dir_override);
    if (!cache_root) return -1;

    version_dir = crexxsaa_path_join(cache_root, "v" CREXXSAA_CACHE_SCHEMA);
    if (!version_dir) {
        free(cache_root);
        return -1;
    }

    if (crexxsaa_remove_tree(version_dir) != 0 && errno != ENOENT)
        rc = -1;

    free(version_dir);
    free(cache_root);
    return rc;
}

const char* crexxsaa_last_error(crexxsaa_context* ctx) {
    if (!ctx || !ctx->last_error[0]) return "unknown CREXXSAA error";
    return ctx->last_error;
}
