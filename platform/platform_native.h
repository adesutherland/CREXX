#ifndef CREXX_PLATFORM_NATIVE_H
#define CREXX_PLATFORM_NATIVE_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#if defined(CREXX_NATIVE_RAW_IO)
void crexx_native_select_file_codec(const uint32_t *map);
FILE *crexx_native_fopen(const char *name, const char *mode);
FILE *crexx_native_fopen_storage(const char *name, const char *mode, int storage);
/* Owned wrapper around a raw native standard stream. Console text is IBM1047
 * records, independent of the selected file codec. fclose releases it. */
FILE *crexx_native_standard(int descriptor);
/* Decode the raw process inputs once into owned, NUL-terminated UTF-8.
 * An absent environment value returns 0; an empty present value returns 1. */
int crexx_native_arguments(int *count, char ***values);
void crexx_native_free_arguments(int count, char **values);
int crexx_native_environment(const char *name, char **value);
int crexx_native_name(const char *utf8, unsigned char *native,
                      size_t capacity, size_t *length);
void crexx_native_panic(const char *bytes, size_t length);
#if defined(CREXX_PLATFORM_CMS)
char *crexx_cms_dirfirst(const char *, const char *, const char *, void **);
char *crexx_cms_dirnext(void **);
void crexx_cms_dirclose(void **);
#endif
#endif
#endif
