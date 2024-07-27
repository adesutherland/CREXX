//
// Platform Specific functions
//

#ifndef CREXX_PLATFORM_H
#define CREXX_PLATFORM_H

#include <stdio.h>

#if defined(__clang__) || defined(__GNUC__)
# ifdef NDEBUG  // RELEASE
#  define RX_INLINE static inline __attribute__((always_inline))
#  define RX_MOSTLYINLINE static inline
#  define RX_FLATTEN __attribute__((flatten))
# else // DEBUG
#  define RX_INLINE static
#  define RX_MOSTLYINLINE static
#  define RX_FLATTEN
# endif
#elif defined(_MSC_VER)
# ifdef NDEBUG  // RELEASE
#  define RX_INLINE static inline
#  define RX_MOSTLYINLINE static inline
#  define RX_FLATTEN
# else
#  define RX_INLINE static
#  define RX_MOSTLYINLINE static
#  define RX_FLATTEN
# endif
#else
# define RX_INLINE static
#  define RX_MOSTLYINLINE static
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

/*
 * Read a file into a returned buffer
 *
 * This function malloc()s the buffer to the right size therefore it needs
 * to be free()d by the caller
 */
char* file2buf(FILE *file, size_t *bytes);

/*
 * Function checks if a file exists
 * dir can be null
 * returns 1 if the file exists
 */
int fileexists(char *name, char *type, char *dir);

/*
 * Function opens and returns a file handle
 * dir can be null - the default is platform specific (e.g. ./ )
 * mode - is the fopen() file mode
 */
FILE *openfile(char *name, char *type, char *dir, char *mode);

/*
 * Get the first file from a directory (or null if there isn't one)
 * (pass the & of void *dir_ptr to hold an opaque directory context)
 * if dir is null then the "current" (platform specific) dir is searched
 */
char *dirfstfl(const char *dir, char* prefix, char *type, void **dir_ptr);

/*
 * Get the next file from a directory (or null if there isn't one)
 * (pass the & of void *dir_ptr to hold an opaque directory context)
 */
char *dirnxtfl(void **dir_ptr);

/*
 * Close the opaque directory context
 */
void dirclose(void **dir_ptr);

/* Returns the executable directory path in a malloced buffer */
char* exepath();

/* Returns the executable fully qualified name in a malloced buffer */
char* exefqname();

/* Gets the file extention of a path */
const char *filenext(const char *filename);

/* Gets the filename of a path */
const char *filename(const char *path);

/* Gets the directory of a filename in a malloced buffer */
/* returns null if there is no directory part */
char *file_dir(const char *path);

#endif //CREXX_PLATFORM_H
