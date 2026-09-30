/* Linux-only host fixture for the BSD/newlib stream interface. */
#ifndef CREXX_TEST_FUNOPEN_COMPAT_H
#define CREXX_TEST_FUNOPEN_COMPAT_H
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include <stdio.h>
#include <sys/types.h>
FILE *funopen(const void *cookie, int (*read_fn)(void *, char *, int),
              int (*write_fn)(void *, const char *, int),
              off_t (*seek_fn)(void *, off_t, int), int (*close_fn)(void *));
#endif
