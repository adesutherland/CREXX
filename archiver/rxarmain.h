#ifndef CREXX_RXARMAIN_H
#define CREXX_RXARMAIN_H

#include "version.h"

typedef enum ARCHIVE_ACTION {
    ADD, CREATE, DELETE, LIST, LICENSE, HELP, UNKNOWN
} ARCHIVE_ACTION;

static struct option archiverOptions[] =
        {
                {"license", no_argument,       0, 't'},
                {"verbose", no_argument,       0, 'v'},
                {"help",    no_argument,       0, 'h'},
                {"create",  required_argument, 0, 'c'},
                {"add",     required_argument, 0, 'a'},
                {"list",    required_argument, 0, 'l'},
                {"delete",  required_argument, 0, 'd'},
                {"test",    required_argument, 0, 'x'},
                {0, 0,                         0, 0}
        };

static void help() {
    char *helpMessage =
            "\n"
            "cREXX " CREXX_VERSION " Archiver\n"
            "Usage   : rxar [options] library-name [rxbin-file...]\n"
            "Options :\n"

            "      --license   Display Copyright & License information.\n"
            "  -h  --help      Display this usage information.\n"
            "  -c  --create    Create new library and add given binaries.\n"
            "  -c  --add     * Add binaries to given library.\n"
            "  -l  --list      List content of library.\n"
            "  -d  --delete  * Delete given binaries from library.\n"
            "  -v  --verbose   Enable verbose output.\n"
            "\n"
            "   *  this functionality is currently not implemented.";

    printf("%s", helpMessage);
}

static void error_and_exit(int rc, char *message) {
    fprintf(stderr, "ERROR: %s - try \"rxdas -h\"\n", message);
    exit(rc);
}

#endif //CREXX_RXARMAIN_H
