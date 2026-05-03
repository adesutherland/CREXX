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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
# include <io.h>
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# define CREXXSAA_TOOL_PATH_SEP '\\'
# define CREXXSAA_TOOL_STAT_STRUCT struct _stat
# define CREXXSAA_TOOL_STAT(path, st) _stat((path), (st))
# define CREXXSAA_TOOL_IS_DIR(st) (((st).st_mode & _S_IFDIR) != 0)
#else
# include <dirent.h>
# include <limits.h>
# include <unistd.h>
# define CREXXSAA_TOOL_PATH_SEP '/'
# define CREXXSAA_TOOL_STAT_STRUCT struct stat
# define CREXXSAA_TOOL_STAT(path, st) stat((path), (st))
# define CREXXSAA_TOOL_IS_DIR(st) S_ISDIR((st).st_mode)
#endif

#ifndef PATH_MAX
# define PATH_MAX 4096
#endif

#include "crexxsaa.h"

typedef struct crexxsaa_tool_manifest {
    char source_path[PATH_MAX];
    char source_hash[64];
    char config_hash[64];
    char rxbin[PATH_MAX];
} crexxsaa_tool_manifest;

static void crexxsaa_tool_usage(FILE* stream) {
    fprintf(stream,
            "usage: crexxsaa [--location] [--list] [--clear] [--cache-dir DIR] [--help]\n"
            "\n"
            "  --location       Print the resolved CREXXSAA cache directory\n"
            "  --list           List cached source entries\n"
            "  --clear          Clear the CREXXSAA cache\n"
            "  --cache-dir DIR  Use DIR instead of the default or CREXXSAA_CACHE_DIR\n");
}

static int crexxsaa_tool_is_path_sep(char value) {
    return value == '/' || value == '\\';
}

static char* crexxsaa_tool_strdup(const char* value) {
    char* copy;
    size_t len;
    if (!value) return NULL;
    len = strlen(value) + 1;
    copy = (char*)malloc(len);
    if (!copy) return NULL;
    memcpy(copy, value, len);
    return copy;
}

static char* crexxsaa_tool_path_join(const char* left, const char* right) {
    char* path;
    size_t left_len;
    size_t right_len;
    int need_sep;

    if (!left || !*left) return crexxsaa_tool_strdup(right ? right : "");
    if (!right || !*right) return crexxsaa_tool_strdup(left);

    left_len = strlen(left);
    right_len = strlen(right);
    need_sep = !crexxsaa_tool_is_path_sep(left[left_len - 1]);

    path = (char*)malloc(left_len + (need_sep ? 1 : 0) + right_len + 1);
    if (!path) return NULL;
    memcpy(path, left, left_len);
    if (need_sep) path[left_len++] = CREXXSAA_TOOL_PATH_SEP;
    memcpy(path + left_len, right, right_len + 1);
    return path;
}

static int crexxsaa_tool_path_exists(const char* path) {
    CREXXSAA_TOOL_STAT_STRUCT st;
    return path && CREXXSAA_TOOL_STAT(path, &st) == 0;
}

static void crexxsaa_tool_chomp(char* value) {
    size_t len;
    if (!value) return;
    len = strlen(value);
    while (len > 0 && (value[len - 1] == '\n' || value[len - 1] == '\r')) {
        value[--len] = '\0';
    }
}

static void crexxsaa_tool_copy_field(char* target, size_t target_len, const char* value) {
    if (!target || target_len == 0) return;
    if (!value) value = "";
    strncpy(target, value, target_len - 1);
    target[target_len - 1] = '\0';
}

static int crexxsaa_tool_read_manifest(const char* path, crexxsaa_tool_manifest* manifest) {
    FILE* file;
    char line[PATH_MAX + 80];

    memset(manifest, 0, sizeof(*manifest));
    file = fopen(path, "r");
    if (!file) return -1;

    while (fgets(line, sizeof(line), file) != NULL) {
        char* value;
        crexxsaa_tool_chomp(line);
        value = strchr(line, '=');
        if (!value) continue;
        *value++ = '\0';

        if (strcmp(line, "source_path") == 0)
            crexxsaa_tool_copy_field(manifest->source_path, sizeof(manifest->source_path), value);
        else if (strcmp(line, "source_hash") == 0)
            crexxsaa_tool_copy_field(manifest->source_hash, sizeof(manifest->source_hash), value);
        else if (strcmp(line, "config_hash") == 0)
            crexxsaa_tool_copy_field(manifest->config_hash, sizeof(manifest->config_hash), value);
        else if (strcmp(line, "rxbin") == 0)
            crexxsaa_tool_copy_field(manifest->rxbin, sizeof(manifest->rxbin), value);
    }

    fclose(file);
    return manifest->source_path[0] ? 0 : -1;
}

static void crexxsaa_tool_print_entry(const char* bucket, const crexxsaa_tool_manifest* manifest) {
    CREXXSAA_TOOL_STAT_STRUCT st;

    printf("source: %s\n", manifest->source_path);
    printf("  bucket: %s\n", bucket);
    if (manifest->rxbin[0]) {
        printf("  rxbin: %s\n", manifest->rxbin);
        if (CREXXSAA_TOOL_STAT(manifest->rxbin, &st) == 0)
            printf("  rxbin_size: %lld\n", (long long)st.st_size);
    }
    if (manifest->source_hash[0])
        printf("  source_hash: %s\n", manifest->source_hash);
    if (manifest->config_hash[0])
        printf("  config_hash: %s\n", manifest->config_hash);
}

static int crexxsaa_tool_list_cache(const char* cache_dir) {
    char* version_dir;
    int count = 0;
    int rc = 0;

    version_dir = crexxsaa_tool_path_join(cache_dir, "v1");
    if (!version_dir) {
        fprintf(stderr, "crexxsaa: out of memory\n");
        return 1;
    }

    if (!crexxsaa_tool_path_exists(version_dir)) {
        printf("(empty)\n");
        free(version_dir);
        return 0;
    }

#ifdef _WIN32
    {
        WIN32_FIND_DATAA find_data;
        HANDLE handle;
        char* pattern = crexxsaa_tool_path_join(version_dir, "*");
        if (!pattern) {
            fprintf(stderr, "crexxsaa: out of memory\n");
            free(version_dir);
            return 1;
        }
        handle = FindFirstFileA(pattern, &find_data);
        free(pattern);
        if (handle == INVALID_HANDLE_VALUE) {
            printf("(empty)\n");
            free(version_dir);
            return 0;
        }
        do {
            char* source_dir;
            char* manifest_path;
            crexxsaa_tool_manifest manifest;
            CREXXSAA_TOOL_STAT_STRUCT st;

            if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0)
                continue;
            source_dir = crexxsaa_tool_path_join(version_dir, find_data.cFileName);
            manifest_path = source_dir ? crexxsaa_tool_path_join(source_dir, "manifest") : NULL;
            if (!source_dir || !manifest_path) {
                fprintf(stderr, "crexxsaa: out of memory\n");
                free(source_dir);
                free(manifest_path);
                rc = 1;
                break;
            }
            if (CREXXSAA_TOOL_STAT(source_dir, &st) == 0
                && CREXXSAA_TOOL_IS_DIR(st)
                && crexxsaa_tool_read_manifest(manifest_path, &manifest) == 0) {
                crexxsaa_tool_print_entry(find_data.cFileName, &manifest);
                count++;
            }
            free(source_dir);
            free(manifest_path);
        } while (FindNextFileA(handle, &find_data));
        FindClose(handle);
    }
#else
    {
        DIR* dir = opendir(version_dir);
        struct dirent* entry;
        if (!dir) {
            printf("(empty)\n");
            free(version_dir);
            return 0;
        }
        while ((entry = readdir(dir)) != NULL) {
            char* source_dir;
            char* manifest_path;
            crexxsaa_tool_manifest manifest;
            CREXXSAA_TOOL_STAT_STRUCT st;

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            source_dir = crexxsaa_tool_path_join(version_dir, entry->d_name);
            manifest_path = source_dir ? crexxsaa_tool_path_join(source_dir, "manifest") : NULL;
            if (!source_dir || !manifest_path) {
                fprintf(stderr, "crexxsaa: out of memory\n");
                free(source_dir);
                free(manifest_path);
                rc = 1;
                break;
            }
            if (CREXXSAA_TOOL_STAT(source_dir, &st) == 0
                && CREXXSAA_TOOL_IS_DIR(st)
                && crexxsaa_tool_read_manifest(manifest_path, &manifest) == 0) {
                crexxsaa_tool_print_entry(entry->d_name, &manifest);
                count++;
            }
            free(source_dir);
            free(manifest_path);
        }
        closedir(dir);
    }
#endif

    if (rc == 0 && count == 0)
        printf("(empty)\n");

    free(version_dir);
    return rc;
}

int main(int argc, char** argv) {
    const char* cache_dir_override = NULL;
    char cache_dir[PATH_MAX];
    int show_location = 0;
    int list_cache = 0;
    int clear_cache = 0;
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            crexxsaa_tool_usage(stdout);
            return 0;
        }
        else if (strcmp(argv[i], "--location") == 0) {
            show_location = 1;
        }
        else if (strcmp(argv[i], "--list") == 0) {
            list_cache = 1;
        }
        else if (strcmp(argv[i], "--clear") == 0) {
            clear_cache = 1;
        }
        else if (strcmp(argv[i], "--cache-dir") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "crexxsaa: --cache-dir requires a directory\n");
                crexxsaa_tool_usage(stderr);
                return 2;
            }
            cache_dir_override = argv[++i];
        }
        else {
            fprintf(stderr, "crexxsaa: unknown option '%s'\n", argv[i]);
            crexxsaa_tool_usage(stderr);
            return 2;
        }
    }

    if (!show_location && !list_cache && !clear_cache) {
        show_location = 1;
        list_cache = 1;
    }

    if (crexxsaa_get_cache_dir(cache_dir_override, cache_dir, sizeof(cache_dir)) != 0) {
        fprintf(stderr, "crexxsaa: unable to resolve cache directory\n");
        return 1;
    }

    if (show_location && !list_cache && !clear_cache) {
        printf("%s\n", cache_dir);
        return 0;
    }

    printf("cache: %s\n", cache_dir);

    if (clear_cache) {
        if (crexxsaa_clear_cache(cache_dir_override) != 0) {
            fprintf(stderr, "crexxsaa: unable to clear cache: %s\n", strerror(errno));
            return 1;
        }
        printf("cleared\n");
    }

    if (list_cache)
        return crexxsaa_tool_list_cache(cache_dir);

    return 0;
}
