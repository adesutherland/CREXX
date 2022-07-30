//
// Std C - Utility Functions
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifdef __linux__
#include <unistd.h>
#include <dirent.h>
#endif

#ifdef _WIN32
#include <windows.h>
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

/*
 * Read a file into a returned buffer
 *
 * This function malloc()s the buffer to the right size therefore it needs
 * to be free()d by the caller
 */
char* file2buf(FILE *file, size_t *bytes) {
    char *buff;

    /* Get file size */
    fseek(file, 0, SEEK_END);
    *bytes = ftell(file);
    rewind(file);

    /* Allocate buffer and read */
    buff = (char*) malloc((*bytes + 2) * sizeof(char) );
    *bytes = fread(buff, 1, *bytes, file);
    if (!*bytes) {
        fprintf(stderr, "Error reading input file\n");
        exit(-1);
    }
    buff[*bytes] = 0;
    buff[*bytes+1] = 0; /* Add an extra byte for the token peak */
    return buff;
}

/*
 * Function opens and returns a file handle
 * dir can be null
 * mode - is the fopen() file mode
 */
FILE *openfile(char *name, char *type, char *dir, char *mode) {
    size_t len;
    char *file_name;
    FILE *stream;

    if (dir) len = strlen(name) + strlen(type) + strlen(dir) + 3;
    else len = strlen(name) + strlen(type) + 2;

    file_name = malloc(len);
    if (type[0] == 0) {
        if (dir) snprintf(file_name, len, "%s/%s", dir, name);
        else snprintf(file_name, len, "%s", name);
    }
    else {
        if (dir) snprintf(file_name, len, "%s/%s.%s", dir, name, type);
        else snprintf(file_name, len, "%s.%s", name, type);
    }

    stream = fopen(file_name, mode);

    free(file_name);

    return stream;
}

#if defined(__APPLE__) || defined(__linux__)
struct fl_dir {
    DIR *d;
    char *type;
};
#endif

#if defined(__APPLE__) || defined(__linux__)
static const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}
#endif

/*
 * Get the first file from a directory (or null if there isn't one)
 * (pass the & of void *dir_ptr to hold an opaque directory context)
 * if dir is null then the "current" (platform specific) dir is searched
 */
char *dirfstfl(const char *dir, char *type, void **dir_ptr) {
#if defined(__APPLE__) || defined(__linux__)
    struct fl_dir *ptr = malloc(sizeof(struct fl_dir));
    *dir_ptr = ptr;

    ptr->type = type;
    if (dir) ptr->d = opendir(dir);
    else ptr->d = opendir(".");

    if (!ptr->d) return 0;

    return dirnxtfl(dir_ptr);
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
        ext = get_filename_ext(dirent->d_name);
        if ( strcmp(ext,ptr->type) == 0 ) return dirent->d_name;
    } while (1);
#endif
}

/*
 * Close the opaque directory context
 */
void dirclose(void **dir_ptr) {
#if defined(__APPLE__) || defined(__linux__)
    struct fl_dir *ptr = *dir_ptr;
    closedir(ptr->d);
    free(ptr);
    *dir_ptr = 0;
#endif
}

/* Returns the executable directory in a malloced buffer */
char* exepath()
{
    char *name = exefqname();
    size_t len = strlen(name);
    size_t i;

    if (!len) return name;

    for (i = len - 1; i; i--)
    {
        if ( name[i] == '\\' || name[i] == '/' )
        {
            name[i] = 0;
            break;
        }
    }
    return name;
}

#define PATH_MAX 4096

/* Returns the executable path name in a malloced buffer */
char* exefqname()
{
    char *exePath = malloc(PATH_MAX);

#ifdef _WIN32

    DWORD len = GetModuleFileNameA(NULL, exePath, PATH_MAX);
	if(len <= 0 || len == PATH_MAX)
	{
		// an error occured, clear exe path
		exePath[0] = '\0';
	}

#elif defined(__linux)

    char buf[PATH_MAX] = {0};
    snprintf(buf, sizeof(buf), "/proc/%d/exe", getpid());
    // readlink() doesn't null-terminate!
    ssize_t len = readlink(buf, exePath, PATH_MAX-1);
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

	uint32_t bufSize = PATH_MAX;
	if(_NSGetExecutablePath(exePath, &bufSize) != 0)
	{
		// an error occured, clear exe path
		exePath[0] = '\0';
	}

#else

    exePath[0] = '\0'

#endif

    return exePath;
}