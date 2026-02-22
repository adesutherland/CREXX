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
    size_t n;

    /* Get file size */
    fseek(file, 0, SEEK_END);
    *bytes = ftell(file);
    rewind(file);

    /* Allocate buffer and read */
    buff = (char*) malloc((*bytes + 2) * sizeof(char) );
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
        strncpy(new_name, name, name_len - ext_len - 1);
        new_name[name_len - ext_len - 1] = 0;
        return new_name;
    }
    return strdup(name);
}

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
    token = dir_copy;
    while (token) {
        next_token = strchr(token, ';');
        if (next_token) *next_token = 0;

        if (*token) {
            len = strlen(name) + strlen(type) + strlen(token) + 3;
            file_name = malloc(len);
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
        if (type[0] == 0 || has_extension(name, type)) snprintf(file_name, len, "%s", name);
        else snprintf(file_name, len, "%s.%s", name, type);
        stream = fopen(file_name, mode);
        free(file_name);
        return stream;
    }

    /* Multiple directories support */
    dir_copy = strdup(dir);
    token = dir_copy;
    while (token) {
        next_token = strchr(token, ';');
        if (next_token) *next_token = 0;

        if (*token) {
            len = strlen(name) + strlen(type) + strlen(token) + 3;
            file_name = malloc(len);
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
                result[0] = path[0];
                result[1] = 0;
                return result;
            }
            result = malloc(i);
            result[i-1] = 0;
            memcpy(result, path, i-1);
            return result;
        }
    }

    return 0;
}
