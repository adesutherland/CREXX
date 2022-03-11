#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "platform.h"
#include "rxarlist.h"
#include "queue.h"
#include "rxarutil.h"

const BYTE libHeaderSigRef[4] = RXLIB_FILE_SIG;

static const char *humanSize(uint64_t bytes)
{
    char *suffix[] = {"B", "KB", "MB", "GB", "TB"};
    char length = sizeof(suffix) / sizeof(suffix[0]);

    int i = 0;
    double dblBytes = bytes;

    if (bytes > 1024) {
        for (i = 0; (bytes / 1024) > 0 && i<length-1; i++, bytes /= 1024)
            dblBytes = bytes / 1024.0;
    }

    static char output[200];
    sprintf(output, "%.02lf %s", dblBytes, suffix[i]);
    return output;
}

int listBinaries(char *libraryName) {

    VFILE *library;
    RXLIB_LIB_HEADER libHeader;
    RXLIB_DIRECTORY_ENTRY directoryEntry;



    long directoryPos;

    if (libraryName) {
        library = calloc(1, sizeof(VFILE));
        library = vfnew(libraryName, library, NULL, RXLIB_EXT);
        library = vfopen(library, "rb");

        fread(&libHeader, sizeof(RXLIB_LIB_HEADER), 1, library->fp);

        if (memcmp(libHeader.sig, libHeaderSigRef, sizeof(libHeaderSigRef)) != 0) {
            fprintf(stderr, "Error reading library %s. This is not a legal cREXX library file.\n", libraryName);
            return -1;
        }

        fseek(library->fp, libHeader.dirpos, SEEK_SET);

        short length = 0;
        int ii = 1;
        while (fread(&length, sizeof(short), 1, library->fp) == 1 && length > 0) {

            // seek back 2 bytes (the length field
            fseek(library->fp, -2, SEEK_CUR);
            fread(&directoryEntry, length, 1, library->fp);


            fprintf(stdout, "(%.5d) \t %s \t %.*s \n", ii, humanSize(directoryEntry.bytecnt_u), directoryEntry.fnlength, directoryEntry.fn);
            ii++;
        }

        vfclose(library);
    }

    return 0;
}