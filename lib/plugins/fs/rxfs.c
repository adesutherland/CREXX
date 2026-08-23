/* cREXX Level-G filesystem provider. */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#define RXFS_CHDIR _chdir
#define RXFS_GETCWD _getcwd
#define RXFS_MKDIR(path) _mkdir(path)
#define RXFS_RMDIR _rmdir
#else
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#define RXFS_CHDIR chdir
#define RXFS_GETCWD getcwd
#define RXFS_MKDIR(path) mkdir((path), 0755)
#define RXFS_RMDIR rmdir
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#include "crexxpa.h"

RXPA_PLUGIN_PROCESS_REENTRANT

#define RXFS_PATH_CAPACITY 4096

static int rxfs_copy_path(const char *source, char *target, size_t capacity)
{
    size_t i;
    size_t length;

    if (!source) return 0;
    length = strlen(source);
    if (length >= capacity) return 0;
    for (i = 0; i <= length; ++i) {
        char value = source[i];
        target[i] = value == '\\' ? '/' : value;
    }
    return 1;
}

static int rxfs_stat(const char *path, struct stat *info)
{
    char normalized[RXFS_PATH_CAPACITY];
    if (!rxfs_copy_path(path, normalized, sizeof(normalized))) return 0;
    return stat(normalized, info) == 0;
}

static void rxfs_strip_filename(char *path)
{
    char *slash = strrchr(path, '/');
#ifdef _WIN32
    char *backslash = strrchr(path, '\\');
    if (!slash || (backslash && backslash > slash)) slash = backslash;
#endif
    if (slash) *slash = '\0';
}

PROCEDURE(cwd)
{
    char path[RXFS_PATH_CAPACITY];
    if (NUM_ARGS != 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.CWD expects no arguments")
    if (!RXFS_GETCWD(path, sizeof(path)))
        RETURNSIGNAL(SIGNAL_FAILURE, "RXFS.CWD could not read the working directory")
    RETURNSTR(path);
    RESETSIGNAL
}

PROCEDURE(loadpath)
{
    char path[RXFS_PATH_CAPACITY];

    if (NUM_ARGS != 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.LOADPATH expects no arguments")
#ifdef _WIN32
    {
        DWORD length = GetModuleFileNameA(NULL, path, (DWORD)sizeof(path));
        if (length == 0 || length >= sizeof(path))
            RETURNSIGNAL(SIGNAL_FAILURE, "RXFS.LOADPATH could not read the executable path")
    }
#elif defined(__APPLE__)
    {
        uint32_t size = (uint32_t)sizeof(path);
        char resolved[RXFS_PATH_CAPACITY];
        if (_NSGetExecutablePath(path, &size) != 0 || !realpath(path, resolved))
            RETURNSIGNAL(SIGNAL_FAILURE, "RXFS.LOADPATH could not read the executable path")
        memcpy(path, resolved, strlen(resolved) + 1u);
    }
#elif defined(__linux__)
    {
        ssize_t length = readlink("/proc/self/exe", path, sizeof(path) - 1u);
        if (length < 0 || (size_t)length >= sizeof(path) - 1u)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXFS.LOADPATH could not read the executable path")
        path[length] = '\0';
    }
#else
    RETURNSIGNAL(SIGNAL_FAILURE, "RXFS.LOADPATH is unavailable on this platform")
#endif
    rxfs_strip_filename(path);
    RETURNSTR(path);
    RESETSIGNAL
}

PROCEDURE(change_directory)
{
    char path[RXFS_PATH_CAPACITY];
    if (NUM_ARGS != 1 || !rxfs_copy_path(GETSTRING(ARG0), path, sizeof(path)))
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.CHDIR expects one valid path")
    RETURNINT(RXFS_CHDIR(path) == 0 ? 0 : -8);
    RESETSIGNAL
}

PROCEDURE(is_directory)
{
    struct stat info;
    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.ISDIR expects one path")
    RETURNINT(rxfs_stat(GETSTRING(ARG0), &info) && S_ISDIR(info.st_mode) ? 1 : 0);
    RESETSIGNAL
}

PROCEDURE(make_directory)
{
    char path[RXFS_PATH_CAPACITY];
    struct stat info;
    if (NUM_ARGS != 1 || !rxfs_copy_path(GETSTRING(ARG0), path, sizeof(path)))
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.MKDIR expects one valid path")
    if (stat(path, &info) == 0) {
        RETURNINT(-4);
    } else {
        RETURNINT(RXFS_MKDIR(path) == 0 ? 0 : -8);
    }
    RESETSIGNAL
}

PROCEDURE(remove_directory)
{
    char path[RXFS_PATH_CAPACITY];
    struct stat info;
    if (NUM_ARGS != 1 || !rxfs_copy_path(GETSTRING(ARG0), path, sizeof(path)))
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.RMDIR expects one valid path")
    if (stat(path, &info) != 0 || !S_ISDIR(info.st_mode)) {
        RETURNINT(-4);
    } else {
        RETURNINT(RXFS_RMDIR(path) == 0 ? 0 : -8);
    }
    RESETSIGNAL
}

PROCEDURE(is_file)
{
    struct stat info;
    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.ISFILE expects one path")
    RETURNINT(rxfs_stat(GETSTRING(ARG0), &info) && S_ISREG(info.st_mode) ? 1 : 0);
    RESETSIGNAL
}

PROCEDURE(delete_file)
{
    char path[RXFS_PATH_CAPACITY];
    struct stat info;
    if (NUM_ARGS != 1 || !rxfs_copy_path(GETSTRING(ARG0), path, sizeof(path)))
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.DELETE expects one valid path")
    if (stat(path, &info) != 0) {
        RETURNINT(-4);
    } else if (remove(path) == 0) {
        RETURNINT(0);
    } else {
        RETURNINT(errno == EACCES ? -3 : -8);
    }
    RESETSIGNAL
}

PROCEDURE(rename_file)
{
    char source[RXFS_PATH_CAPACITY];
    char target[RXFS_PATH_CAPACITY];
    struct stat info;
    int result;
    if (NUM_ARGS != 2 ||
        !rxfs_copy_path(GETSTRING(ARG0), source, sizeof(source)) ||
        !rxfs_copy_path(GETSTRING(ARG1), target, sizeof(target)))
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.RENAME expects two valid paths")
    if (stat(source, &info) != 0) {
        RETURNINT(-4);
        RESETSIGNAL
        return;
    }
#ifdef _WIN32
    result = MoveFileExA(source, target, MOVEFILE_REPLACE_EXISTING) ? 0 : -8;
#else
    result = rename(source, target) == 0 ? 0 : -8;
#endif
    RETURNINT(result);
    RESETSIGNAL
}

PROCEDURE(list_directory)
{
    char path[RXFS_PATH_CAPACITY];
    rxinteger count = 0;
    if (NUM_ARGS != 2 || !rxfs_copy_path(GETSTRING(ARG0), path, sizeof(path)))
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.LISTDIR expects a path and exposed array")
    SETARRAYHI(ARG1, 0);
#ifdef _WIN32
    {
        WIN32_FIND_DATAA entry;
        HANDLE handle;
        char pattern[RXFS_PATH_CAPACITY];
        int written = snprintf(pattern, sizeof(pattern), "%s%s*", path,
                               path[0] && path[strlen(path) - 1u] != '/' ? "/" : "");
        if (written < 0 || (size_t)written >= sizeof(pattern))
            RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.LISTDIR path is too long")
        handle = FindFirstFileA(pattern, &entry);
        if (handle == INVALID_HANDLE_VALUE) {
            RETURNINT(-8);
            RESETSIGNAL
            return;
        }
        do {
            if (strcmp(entry.cFileName, ".") != 0 && strcmp(entry.cFileName, "..") != 0) {
                SETARRAYHI(ARG1, count + 1);
                SETSARRAY(ARG1, count, entry.cFileName);
                ++count;
            }
        } while (FindNextFileA(handle, &entry));
        FindClose(handle);
    }
#else
    {
        DIR *directory = opendir(path);
        struct dirent *entry;
        if (!directory) {
            RETURNINT(errno == ENOENT ? -4 : (errno == EACCES ? -3 : -8));
            RESETSIGNAL
            return;
        }
        while ((entry = readdir(directory)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                SETARRAYHI(ARG1, count + 1);
                SETSARRAY(ARG1, count, entry->d_name);
                ++count;
            }
        }
        closedir(directory);
    }
#endif
    RETURNINT(count);
    RESETSIGNAL
}

PROCEDURE(append_file)
{
    char source_path[RXFS_PATH_CAPACITY];
    char target_path[RXFS_PATH_CAPACITY];
    unsigned char buffer[8192];
    FILE *source;
    FILE *target;
    size_t total = 0u;
    size_t bytes;

    if (NUM_ARGS != 2 ||
        !rxfs_copy_path(GETSTRING(ARG0), source_path, sizeof(source_path)) ||
        !rxfs_copy_path(GETSTRING(ARG1), target_path, sizeof(target_path)))
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.APPEND expects two valid paths")
    source = fopen(source_path, "rb");
    if (!source) {
        RETURNINT(-8);
        RESETSIGNAL
        return;
    }
    target = fopen(target_path, "ab");
    if (!target) {
        fclose(source);
        RETURNINT(-12);
        RESETSIGNAL
        return;
    }
    while ((bytes = fread(buffer, 1u, sizeof(buffer), source)) != 0u) {
        if (fwrite(buffer, 1u, bytes, target) != bytes) {
            fclose(source);
            fclose(target);
            RETURNINT(-16);
            RESETSIGNAL
            return;
        }
        total += bytes;
    }
    if (ferror(source) || fclose(source) != 0 || fclose(target) != 0) {
        RETURNINT(-16);
    } else {
        RETURNINT((rxinteger)total);
    }
    RESETSIGNAL
}

LOADFUNCS
    ADDPROC(cwd, "rxfs.cwd", "b", ".string", "");
    ADDPROC(loadpath, "rxfs.loadpath", "b", ".string", "");
    ADDPROC(change_directory, "rxfs.chdir", "b", ".int", "path = .string");
    ADDPROC(is_directory, "rxfs.isdir", "b", ".int", "path = .string");
    ADDPROC(make_directory, "rxfs.mkdir", "b", ".int", "path = .string");
    ADDPROC(remove_directory, "rxfs.rmdir", "b", ".int", "path = .string");
    ADDPROC(delete_file, "rxfs.delete", "b", ".int", "path = .string");
    ADDPROC(rename_file, "rxfs.rename", "b", ".int", "source = .string,target = .string");
    ADDPROC(is_file, "rxfs.isfile", "b", ".int", "path = .string");
    ADDPROC(list_directory, "rxfs.listdir", "b", ".int", "path = .string,expose entries = .string[]");
    ADDPROC(append_file, "rxfs.append", "b", ".int", "source = .string,target = .string");
ENDLOADFUNCS
