/* CMS Platform - Utility Functions */

#include <stdlib.h>
#include <stdio.h>
#include "platform.h"

/* Read a file into a buffer - this function mallocs the buffer to the right size */
char *file2buf(FILE *file, size_t *l) {
    int i;
    size_t buf_size = 1024 * 4;
    char *buffer;

    /* GCCLIB function to try to get the file size */
    *l = fgetlen(file);
    if (!*l) return 0; /* Empty file */
    /* Note fgetlen() returns -1 for a variable length file */

    /* Disable cache and rewind file to the beginning */
    setbuf(file, 0);
    rewind(file);

    if (*l > 0) {
        /* Read the file in one go */
        buffer = malloc(*l + 2);
        *l = fread(buffer, 1, *l, file);

        /* Null terminate */
        buffer[*l] = 0;
        buffer[*l + 1]  = 0;
    } else {
        /* Don't know file size - have to read the file line by line */
        *l = 0;
        buffer = malloc(buf_size);
        while ((i = nextrecLen(file)) > 0) {
            while (*l + i + 1 > buf_size) {
                buf_size *= 2;
                buffer = realloc(buffer, buf_size);
            }
            fgets(buffer + *l, i + 1, file);
            *l += i;
        }

        /* Give back unwanted memory */
        buffer = realloc(buffer, *l + 2);

        /* Null terminate */
        buffer[*l] = 0;
        buffer[*l + 1] = 0;
    }

    return buffer;
}

/*
 * Function opens and returns a file handle
 * dir can be null - the default is platform specific (e.g. ./ )
 * mode - is the fopen() file mode
 */
FILE *openfile(char *name, char *type, char *dir, char *mode) {
    size_t len;
    char *file_name;
    FILE *stream;
    if (!dir)
        dir = "A";

    len = strlen(name) + strlen(type) + strlen(dir) + 3;

    file_name = malloc(len);
    snprintf(file_name, len,
             "%s %s %s", name, type, dir);

    stream = fopen(file_name, mode);

    free(file_name);

    return stream;
}
