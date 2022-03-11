//
// Platform Specific functions
//

#ifndef CREXX_PLATFORM_H
#define CREXX_PLATFORM_H

#include <stdio.h>

#if defined(__clang__) || defined(__GNUC__)
# ifdef NDEBUG  // RELEASE
#  define RX_INLINE static inline __attribute__((always_inline))
#  define RX_FLATTEN __attribute__((flatten))
# else // DEBUG
#  define RX_INLINE static
#  define RX_FLATTEN
# endif
#elif defined(_MSC_VER)
# ifdef NDEBUG  // RELEASE
#  define RX_INLINE static inline
#  define RX_FLATTEN
# else
#  define RX_INLINE static
#  define RX_FLATTEN
# endif
#else
# define RX_INLINE static
# define RX_FLATTEN
# warning "Functions may not be inlined as intended"
#endif

/* Load Platform Specific Headers */
#ifdef __CMS__
#include "cms.h"
#endif

#ifdef __32BIT__
typedef long rxinteger;
#else
typedef long long rxinteger;
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__) || defined(__MSYS__)
# define FILE_SEPARATOR '\\'
#else
# define FILE_SEPARATOR '/'
#endif

typedef struct S_VFILE VFILE;
struct S_VFILE {
    char *path;
    char *basename;
    char *extension;
    char *fullname;
    int   wildcarded;
    int   exists;
    int   opened;
    FILE *fp;
    VFILE *next;
};

/*
 * Read a file into a returned buffer
 *
 * This function malloc()s the buffer to the right size therefore it needs
 * to be free()d by the caller
 */
char* file2buf(FILE *file);

/*
 * Function opens and returns a file handle
 * dir can be null - the default is platform specific (e.g. ./ )
 * mode - is the fopen() file mode
 */
FILE *openfile(char *name, char *type, char *dir, char *mode);

/*
 * Function returns the extension part of a file name, if present.
 * Otherwise, it returs NULL.
 */
const char *fnext(const char *file_name);

/*
 * Function to create the internal VFILE structure from a given input file name.
 * If given file name did not have a path, the optional default path will be used.
 * If the default path is NULL the local directory path './' will be used.
 * If given file name did not have an extension, the default extension will be used.
 *
 * If the given file name contains wildcards, a list of VFILEs will be created.
 */
VFILE* vfnew(const char *inputName, VFILE *vfile, const char *defaultPath, const char *defaultExtension);

/*
 * Function to open a VFILE in given mode.
 * mode - is the fopen() file mode
 */
VFILE* vfopen(VFILE *vfile, char *mode);

/*
 * Function to close a single open VFILE.
 */
void vfclose(VFILE *vfile);

/*
 * Function to close all eventually open file pointer and
 * free all allocated memory for given VFILE structure.
 */
void vffree(VFILE *vfile);

/*
 * Function to cnt all non wildcard files
 * for given VFILE structure.
 */
size_t vfcnt(VFILE *vfile);

#endif //CREXX_PLATFORM_H
