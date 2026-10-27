/* cREXX License (MIT). Exercise the real generator without CRT temp access. */
#include <stdio.h>
#include <errno.h>

static FILE *forbidden_tmpfile(void) {
    errno = EACCES;
    return NULL;
}

#define tmpfile forbidden_tmpfile
#define main catalog_generator_main
#include "../generate-rxas-catalog.c"
#undef main
#undef tmpfile

int main(int argc, char **argv) {
    /* Keep the denial control live even after the generator stops using it. */
    if (forbidden_tmpfile() != NULL || errno != EACCES) return 2;
    return catalog_generator_main(argc, argv);
}
