/* Transitional converted TSO directory service declarations. Native raw
 * profiles use native_raw.h; remove these when their iterator is migrated. */
#ifndef CREXX_NATIVE_SERVICES_H
#define CREXX_NATIVE_SERVICES_H
#include <stddef.h>
#include <stdint.h>
/* Generic dataset/member enumeration: 1 entry, 0 EOF, -1 error; close always
 * releases resources, including after an error. No cREXX suffix or root policy. */
void *lab_tso_directory_open(const char *);
int lab_tso_directory_next(void *, char *, size_t);
int lab_tso_directory_close(void *);
#endif
