#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <strings.h>

#include "types.h"
#include "license.h"
#include "rxarmain.h"
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

    archiveAction = parseOptions(argc, argv, archiverOptions, &library);

    switch (archiveAction) {
        case ADD:
            if (verboseFlag) {
                if (library->exists) {
                    fprintf(stdout, "Adding to existing library '%s'. \n", library->fullname);
                } else {
                    fprintf(stdout, "Creating library '%s'. \n", library->fullname);
                }
            }

            VFILE *binaries, *current, *last;

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

            rc = addBinaries(library, binaries);
            break;

        case DELETE:
            if (verboseFlag) {
                fprintf(stdout, "Deleting binaries from %s \n", library->fullname);
            }

            rc = deleteBinaries(library->basename);
            break;

        case LIST:
            if (verboseFlag) {
                fprintf(stdout, "List binaries in %s \n", library->fullname);
            }

            rc = listBinaries(library->basename);
            break;

        case LICENSE:
            license();
            break;

        case HELP:
            help();
            break;

        default:
            error_and_exit(-1, "Unknown option selected.");
    }

   return rc;
}

static ARCHIVE_ACTION
parseOptions(int argc, char *argv[], const struct option *options, VFILE **library) {

    ARCHIVE_ACTION action = UNKNOWN;

    while (TRUE) {
        int optionIndex = 0;

        int option = getopt_long(argc, argv, "v?ha:l:d:",
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

            case 't':
                if (verboseFlag) {
                    fprintf(stdout, "Calling cREXX Linker test function. \n");
                }

                test(optarg);
                exit (0);

            case 'v':
                verboseFlag = 1;
                break;

            case 'c':
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
