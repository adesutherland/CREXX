/* Adapt only test executables; the mainframe product uses its SDK funopen. */
#include "funopen_compat.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
typedef struct fixture_cookie {
    void *cookie;
    int (*read_fn)(void *, char *, int);
    int (*write_fn)(void *, const char *, int);
    int (*close_fn)(void *);
} fixture_cookie;
static ssize_t fixture_read(void *opaque, char *bytes, size_t count) {
    fixture_cookie *state = opaque;
    return state->read_fn(state->cookie, bytes, count > INT_MAX ? INT_MAX : (int)count);
}
static ssize_t fixture_write(void *opaque, const char *bytes, size_t count) {
    fixture_cookie *state = opaque;
    return state->write_fn(state->cookie, bytes, count > INT_MAX ? INT_MAX : (int)count);
}
static int fixture_close(void *opaque) {
    fixture_cookie *state = opaque;
    int result = state->close_fn ? state->close_fn(state->cookie) : 0;
    free(state);
    return result;
}
FILE *funopen(const void *cookie, int (*read_fn)(void *, char *, int),
              int (*write_fn)(void *, const char *, int),
              off_t (*seek_fn)(void *, off_t, int), int (*close_fn)(void *)) {
    fixture_cookie *state;
    FILE *file;
    cookie_io_functions_t callbacks = {0};
    if (seek_fn) { errno = ENOTSUP; return NULL; }
    state = malloc(sizeof(*state));
    if (!state) return NULL;
    state->cookie = (void *)cookie; state->read_fn = read_fn;
    state->write_fn = write_fn; state->close_fn = close_fn;
    callbacks.read = read_fn ? fixture_read : NULL;
    callbacks.write = write_fn ? fixture_write : NULL;
    callbacks.close = fixture_close;
    file = fopencookie(state, write_fn ? "w" : "r", callbacks);
    if (!file) free(state);
    return file;
}
