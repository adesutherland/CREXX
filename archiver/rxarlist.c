#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "platform.h"
#include "rxarlist.h"
#include "rxarutil.h"

extern BOOL verboseFlag;
const BYTE libHeaderSigRef[4] = RXLIB_FILE_SIG;

int listBinaries(char *libraryName) {

    VFILE *library;
    RXLIB_LIB_HEADER libHeader;
    RXLIB_DIRECTORY_ENTRY directoryEntry;

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
        while (fread(&length, sizeof(short), 1, library->fp) == 1 && length > 0) {

            // seek back 2 bytes (the length field
            fseek(library->fp, -2, SEEK_CUR);
            fread(&directoryEntry, length, 1, library->fp);

            if (verboseFlag) {
                fprintf(stdout, "%9ld \t %.*s \n", directoryEntry.bytecnt_u, directoryEntry.fnlength, directoryEntry.fn);
            } else {
                fprintf(stdout, "%.*s \n", directoryEntry.fnlength, directoryEntry.fn);
            }
        }

        vfclose(library);
    }

    return 0;
}