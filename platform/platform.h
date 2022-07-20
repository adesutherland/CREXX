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
 * Function opens and returns a file handle
 * dir can be null - the default is platform specific (e.g. ./ )
 * mode - is the fopen() file mode
 */
FILE *openfile(char *name, char *type, char *dir, char *mode);

#endif //CREXX_PLATFORM_H
