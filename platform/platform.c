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

#ifdef __linux__
#include <unistd.h>
#include <dirent.h>
#include <sys/sysinfo.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

#ifndef _MSC_VER // Windows Visual Studio
#include <stdint.h>
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <dirent.h>
#include <unistd.h>
#endif

#include "platform.h"

static void rx_print_bytes(FILE *out, const char *label, unsigned long long bytes) {
    fprintf(out, "  %s: %llu bytes (%.2f MiB)\n",
            label, bytes, (double)bytes / 1048576.0);
}

static unsigned long rx_process_id(void) {
#ifdef _WIN32
    return (unsigned long)GetCurrentProcessId();
#elif defined(__linux__) || defined(__APPLE__)
    return (unsigned long)getpid();
#else
    return 0;
#endif
}

#ifdef _WIN32
static void rx_print_windows_commit_status(FILE *out, const MEMORYSTATUSEX *status) {
    PERFORMANCE_INFORMATION performance;
    unsigned long long commit_total;
    unsigned long long commit_limit;

    rx_print_bytes(out, "commit available to this process", (unsigned long long)status->ullAvailPageFile);

    memset(&performance, 0, sizeof(performance));
    performance.cb = sizeof(performance);
    if (GetPerformanceInfo(&performance, (DWORD)sizeof(performance))) {
        commit_total = (unsigned long long)performance.CommitTotal * (unsigned long long)performance.PageSize;
        commit_limit = (unsigned long long)performance.CommitLimit * (unsigned long long)performance.PageSize;
        rx_print_bytes(out, "system commit total", commit_total);
        rx_print_bytes(out, "system commit limit", commit_limit);
        if (commit_limit >= commit_total) {
            rx_print_bytes(out, "system commit available", commit_limit - commit_total);
        } else {
            fprintf(out, "  system commit available: unavailable (total exceeds limit)\n");
        }
    } else {
        fprintf(out, "  system commit total: unavailable (GetPerformanceInfo failed)\n");
        fprintf(out, "  system commit limit: unavailable (GetPerformanceInfo failed)\n");
        fprintf(out, "  system commit available: unavailable (GetPerformanceInfo failed)\n");
    }
}
#endif

static void rx_print_memory_status(FILE *out) {
#ifdef _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        fprintf(out, "  system memory load: %lu%%\n", (unsigned long)status.dwMemoryLoad);
        rx_print_windows_commit_status(out, &status);
        rx_print_bytes(out, "physical memory available", (unsigned long long)status.ullAvailPhys);
        rx_print_bytes(out, "physical memory total", (unsigned long long)status.ullTotalPhys);
        rx_print_bytes(out, "page file total", (unsigned long long)status.ullTotalPageFile);
        rx_print_bytes(out, "virtual memory available", (unsigned long long)status.ullAvailVirtual);
        rx_print_bytes(out, "virtual memory total", (unsigned long long)status.ullTotalVirtual);
    } else {
        fprintf(out, "  system memory: unavailable (GlobalMemoryStatusEx failed)\n");
    }
#elif defined(__linux__)
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        unsigned long long unit = info.mem_unit ? (unsigned long long)info.mem_unit : 1ULL;
        rx_print_bytes(out, "physical memory free", (unsigned long long)info.freeram * unit);
        rx_print_bytes(out, "physical memory total", (unsigned long long)info.totalram * unit);
        rx_print_bytes(out, "shared memory", (unsigned long long)info.sharedram * unit);
        rx_print_bytes(out, "buffer memory", (unsigned long long)info.bufferram * unit);
        rx_print_bytes(out, "swap free", (unsigned long long)info.freeswap * unit);
        rx_print_bytes(out, "swap total", (unsigned long long)info.totalswap * unit);
    } else {
        fprintf(out, "  system memory: unavailable (sysinfo failed)\n");
    }
#elif defined(__APPLE__)
    long page_size = -1;
    long total_pages = -1;
    long available_pages = -1;
#ifdef _SC_PAGESIZE
    page_size = sysconf(_SC_PAGESIZE);
#endif
#ifdef _SC_PHYS_PAGES
    total_pages = sysconf(_SC_PHYS_PAGES);
#endif
#ifdef _SC_AVPHYS_PAGES
    available_pages = sysconf(_SC_AVPHYS_PAGES);
#endif
    if (page_size > 0 && total_pages > 0) {
        if (available_pages > 0) {
            rx_print_bytes(out, "physical memory available",
                           (unsigned long long)available_pages * (unsigned long long)page_size);
        }
        rx_print_bytes(out, "physical memory total",
                       (unsigned long long)total_pages * (unsigned long long)page_size);
    } else {
        fprintf(out, "  system memory: unavailable\n");
    }
#else
    fprintf(out, "  system memory: unavailable\n");
#endif
}

void rx_report_out_of_memory(const char *operation, size_t requested_bytes,
                             const char *detail, const char *source_file,
                             int source_line, const char *function_name) {
    int saved_errno = errno;
    unsigned long pid = rx_process_id();

    fprintf(stderr, "PANIC: Out of memory\n");
    if (pid) fprintf(stderr, "  process id: %lu\n", pid);
    if (operation && operation[0]) fprintf(stderr, "  allocation: %s\n", operation);
    if (requested_bytes == RX_OOM_UNKNOWN_SIZE) {
        fprintf(stderr, "  requested bytes: unknown\n");
    } else {
        rx_print_bytes(stderr, "requested bytes", (unsigned long long)requested_bytes);
    }
    if (detail && detail[0]) fprintf(stderr, "  detail: %s\n", detail);
    if (source_file && source_file[0]) {
        fprintf(stderr, "  source: %s:%d", source_file, source_line);
        if (function_name && function_name[0]) fprintf(stderr, " (%s)", function_name);
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "  errno: %d", saved_errno);
    if (saved_errno) fprintf(stderr, " (%s)", strerror(saved_errno));
    else fprintf(stderr, " (not set)");
    fprintf(stderr, "\n");
    rx_print_memory_status(stderr);
}

void rx_panic_out_of_memory(const char *operation, size_t requested_bytes,
                            const char *detail, const char *source_file,
                            int source_line, const char *function_name) {
    rx_report_out_of_memory(operation, requested_bytes, detail,
                            source_file, source_line, function_name);
    exit(-1);
}

/*
 * Read a file into a returned buffer
 *
 * This function malloc()s the buffer to the right size therefore it needs
 * to be free()d by the caller
 */
char* file2buf(FILE *file, size_t *bytes) {
    char *buff;
    size_t n;
    long pos;

    /* Get file size */
    if (fseek(file, 0, SEEK_END) != 0) {
        return 0;
    }
    pos = ftell(file);
    if (pos < 0) {
        return 0;
    }
    *bytes = (size_t)pos;
    rewind(file);

    /* Allocate buffer and read */
    buff = (char*) malloc((*bytes + 2) * sizeof(char) );
    if (!buff) {
        RX_REPORT_OOM("malloc file read buffer", *bytes + 2, "file2buf");
        return 0;
    }
    n = fread(buff, 1, *bytes, file);
    if (n == 0 && *bytes > 0) {
        free(buff);
        return 0;
    }
    *bytes = n;
    buff[*bytes] = 0;
    buff[*bytes+1] = 0; /* Add an extra byte for the token peak */
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
        char *copy = strdup(name);
        if (!copy) RX_PANIC_OOM("strdup file name", strlen(name) + 1, name);
        return copy;
    }
}

#if !defined(_WIN32) && !defined(__CMS__)
#include <termios.h>
#include <unistd.h>
#include <signal.h>

static struct termios orig_termios;
static int termios_saved = 0;

void platform_term_save() {
    if (!termios_saved) {
        if (isatty(STDIN_FILENO)) {
            if (tcgetattr(STDIN_FILENO, &orig_termios) == 0) {
                termios_saved = 1;
            }
        }
    }
}

void platform_term_restore() {
    if (termios_saved) {
        tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
    }
}

static void signal_handler(int sig) {
    platform_term_restore();
    /* Re-raise the signal or exit */
    signal(sig, SIG_DFL);
    raise(sig);
}

void platform_install_signal_handlers() {
    platform_term_save();
    if (termios_saved) {
        atexit(platform_term_restore);
        signal(SIGSEGV, signal_handler);
        signal(SIGILL, signal_handler);
        signal(SIGFPE, signal_handler);
        signal(SIGBUS, signal_handler);
        signal(SIGABRT, signal_handler);
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
    }
}
#else
/* Stub for Windows or other platforms if not needed */
void platform_term_save() {}
void platform_term_restore() {}
void platform_install_signal_handlers() {}
#endif

/*
 * Function checks if a file exists
 * dir can be null, and can contain multiple directories separated by ;
 * returns 1 if the file exists
 */
int fileexists(char *name, char *type, char *dir) {
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
#if defined(__linux__) || defined(__APPLE__)
        result = access(file_name, F_OK) != -1;
#elif defined(_WIN32)
        DWORD dwAttrib = GetFileAttributes(file_name);
        result = (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#endif
        free(file_name);
        return result;
    }

    /* Multiple directories support */
    dir_copy = strdup(dir);
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
#if defined(__linux__) || defined(__APPLE__)
            result = access(file_name, F_OK) != -1;
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
}

/*
 * Function opens and returns a file handle
 * dir can be null, and can contain multiple directories separated by ;
 * mode - is the fopen() file mode
 */
FILE *openfile(char *name, char *type, char *dir, char *mode) {
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
        stream = fopen(file_name, mode);
        free(file_name);
        return stream;
    }

    /* Multiple directories support */
    dir_copy = strdup(dir);
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
            stream = fopen(file_name, mode);
            free(file_name);
            if (stream) break;
        }

        token = next_token ? next_token + 1 : 0;
    }
    free(dir_copy);

    return stream;
}

#if defined(__APPLE__) || defined(__linux__)
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

#if defined(__APPLE__) || defined(__linux__)

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
        *dir_ptr = 0;
        free(win_data);
        return 0;
    }

    *dir_ptr = win_data;

    if( win_data->fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
        return dirnxtfl(dir_ptr); /* Return the next valid file */
    }

    return win_data->fdFile.cFileName;

#else

    return 0;

#endif
}

/*
 * Get the next file from a directory (or null if there isn't one)
 * (pass the & of void *dir_ptr to hold an opaque directory context)
 */
char *dirnxtfl(void **dir_ptr) {

#if defined(__APPLE__) || defined(__linux__)

    struct dirent *dirent;
    struct fl_dir *ptr = *dir_ptr;
    const char *ext;
    do {
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
    if (!win_data) return 0;

    while ( FindNextFile(win_data->hFind, &(win_data->fdFile)) ) {

        if (win_data->fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

        return win_data->fdFile.cFileName;
    }
    return 0;

#else

    return 0;

#endif
}

/*
 * Close the opaque directory context
 */
void dirclose(void **dir_ptr) {

#if defined(__APPLE__) || defined(__linux__)

    struct fl_dir *ptr = *dir_ptr;

    if (!ptr || !ptr->d) return;
    closedir(ptr->d);
    free(ptr);
    *dir_ptr = 0;

#elif defined(_WIN32)

    struct WIN_FILE_DATA *win_data = *dir_ptr;
    if (!win_data) return;

    FindClose(win_data->hFind);
    free(win_data);

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

#elif defined(__linux)

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

#elif defined(__APPLE__)

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
