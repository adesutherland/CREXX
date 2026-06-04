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
// Platform Specific functions
//

#ifndef CREXX_PLATFORM_H
#define CREXX_PLATFORM_H
#define MAXFILEPATH 4096

#include <stdio.h>
#include <stddef.h>
#include "crexx_license.h"

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

#ifndef RXINTEGER_T
#define RXINTEGER_T
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L /* C99 */
#include <stdint.h>
typedef intmax_t rxinteger;
#else
#ifdef __32BIT__
typedef long rxinteger;
#else
typedef long long rxinteger;
#endif
#endif
#endif //RXINTEGER_T

#ifndef IS_RXINTEGER_32BIT
#define IS_RXINTEGER_32BIT (sizeof(rxinteger) == 4)
#endif

#if defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)
#define RX_FUNCTION_NAME __FUNCTION__
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define RX_FUNCTION_NAME __func__
#else
#define RX_FUNCTION_NAME "unknown"
#endif

#define RX_OOM_UNKNOWN_SIZE ((size_t)-1)

/*
 * Standard allocation failure diagnostics. Use RX_REPORT_OOM when the caller
 * can return an error, and RX_PANIC_OOM when the current code path already
 * treats allocation failure as unrecoverable.
 */
void rx_report_out_of_memory(const char *operation, size_t requested_bytes,
                             const char *detail, const char *source_file,
                             int source_line, const char *function_name);
void rx_panic_out_of_memory(const char *operation, size_t requested_bytes,
                            const char *detail, const char *source_file,
                            int source_line, const char *function_name);

#define RX_REPORT_OOM(operation, requested_bytes, detail) \
    rx_report_out_of_memory((operation), (requested_bytes), (detail), __FILE__, __LINE__, RX_FUNCTION_NAME)

#define RX_PANIC_OOM(operation, requested_bytes, detail) \
    rx_panic_out_of_memory((operation), (requested_bytes), (detail), __FILE__, __LINE__, RX_FUNCTION_NAME)

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

/* Checks if a file has any extension */
int has_any_extension(const char *name);

/* Strips the rightmost extension from a filename if it matches the provided extension */
char *strip_rightmost_extension_if(const char *name, const char *ext);

/*
 * Terminal management functions for ensuring sane terminal state on exit/crash
 */
void platform_term_save(void);
void platform_term_restore(void);
void platform_install_signal_handlers(void);

#endif //CREXX_PLATFORM_H
