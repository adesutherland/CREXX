//
// Std C - Utility Functions
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Read a file into a returned buffer
 *
 * This function malloc()s the buffer to the right size therefore it needs
 * to be free()d by the caller
 */
char* file2buf(FILE *file) {
    size_t bytes;
    char *buff;

    /* Get file size */
    fseek(file, 0, SEEK_END);
    bytes = ftell(file);
    rewind(file);

    /* Allocate buffer and read */
    buff = (char*) malloc((bytes + 2) * sizeof(char) );
    bytes = fread(buff, 1, bytes, file);
    if (!bytes) {
        fprintf(stderr, "Error reading input file\n");
        exit(-1);
    }
    buff[bytes] = 0;
    buff[bytes+1] = 0; /* Add an extra byte for the token peak */
    return buff;
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
    if (!dir) dir = ".";

    len = strlen(name) + strlen(type) + strlen(dir) + 3;

    file_name = malloc(len);
    if (type == 0) {
      snprintf(file_name, len, "%s/%s", dir, name);
    }
    else {
      snprintf(file_name, len, "%s/%s.%s", dir, name, type);
    }

    stream = fopen(file_name, mode);

    free(file_name);

    return stream;
}
