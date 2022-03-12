#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <strings.h>

#include "types.h"
#include "license.h"
#include "rxarmain.h"
#include "rxarcreate.h"
#include "rxaradd.h"
#include "rxarlist.h"
#include "rxardel.h"

#include "platform.h"

#include "rxlntest.h"

// internal functions
static ARCHIVE_ACTION parseOptions(int argc, char *argv[], const struct option *options, VFILE **library);

// global variables
BOOL verboseFlag = FALSE;

// main function
int main(int argc, char *argv[]) {

    int rc = 0;

    ARCHIVE_ACTION  archiveAction;
    VFILE           *library;

    // TODO: would like to use argp library, but it's GPL
    archiveAction = parseOptions(argc, argv, archiverOptions, &library);

    switch (archiveAction) {

        VFILE *binaries, *current, *last;

        case ADD:
            rc = addBinaries(library, NULL);
            break;

        case CREATE:
            binaries = calloc(1, sizeof(VFILE));

            current = binaries;
            while (optind < argc) {

                if (!current) {
                    current = calloc(1, sizeof(VFILE));

                    // insert new VFILE element in last element
                    last->next = current;
                }

                current = vfnew(argv[optind], current, NULL, RXBIN_EXT);

                // find last VFILE
                while (current->next != NULL) {
                    current = current->next;
                }

                // save last VFILE element to insert new one
                // point to the next free slot
                last    = current;
                current = last->next;

                optind++;
            }

            rc = createLibrary(library, binaries);
            break;

        case DELETE:
            rc = deleteBinaries(library->basename);
            break;

        case LIST:
            rc = listBinaries(library->basename);
            break;

        case LICENSE:
            license();
            break;

        case HELP:
        default:
            help();
            break;

    }

    return rc;
}

static ARCHIVE_ACTION
parseOptions(int argc, char *argv[], const struct option *options, VFILE **library) {

    ARCHIVE_ACTION action = UNKNOWN;

    while (TRUE) {
        int optionIndex = 0;

        int option = getopt_long(argc, argv, "vh?c:a:l:d:x:",
                                 options, &optionIndex);

        if (option == -1)
            break;

        switch (option) {
            case 'a':
                if (action == UNKNOWN) {
                    *library = calloc(1, sizeof(VFILE));
                    *library = vfnew(optarg, *library, NULL, RXLIB_EXT);

                    action = ADD;
                }

                break;

            case 'c':
                if (action == UNKNOWN) {
                    *library = calloc(1, sizeof(VFILE));
                    *library = vfnew(optarg, *library, NULL, RXLIB_EXT);

                    action = CREATE;
                }

                break;

            case 'd':
                if (action == UNKNOWN) {
                    *library = calloc(1, sizeof(VFILE));
                    *library = vfnew(optarg, *library, NULL, RXLIB_EXT);

                    action = DELETE;
                }

                break;

            case 'l':
                if (action == UNKNOWN) {
                    *library = calloc(1, sizeof(VFILE));
                    *library = vfnew(optarg, *library, NULL, RXLIB_EXT);

                    action = LIST;
                }

                break;

            case 'x':
                if (verboseFlag) {
                    fprintf(stdout, "Calling cREXX Linker test function. \n");
                }

                test(optarg);
                exit (0);

            case 'v':
                verboseFlag = 1;
                break;

            case 't':
                if (action == UNKNOWN)
                    action = LICENSE;

                break;

            case 'h':
            case '?':
                if (action == UNKNOWN)
                    action = HELP;

                break;

            default:
                abort();
        }
    }

    return action;
}
