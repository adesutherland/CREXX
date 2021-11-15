#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "platform.h"
#include "rxaradd.h"
#include "queue.h"
#include "rxarutil.h"

extern BOOL verboseFlag;

int addBinaries(VFILE *library, VFILE *binaries) {

    RXLIB_DIRECTORY     directory;

    if (verboseFlag) {
        fprintf(stdout, "Library '%s' will be created with following binaries:\n", library->fullname);
    }

    if (binaries) {
        BYTE libFileSig[4] = RXLIB_FILE_SIG;

        library = vfopen(library, "wb");


        fwrite(libFileSig, sizeof(libFileSig), 1, library->fp);

        // skipping directory offset field, will be written later
        fseek(library->fp, 4, SEEK_CUR);

        do {
            if (!binaries->wildcarded) {
                if (binaries->exists) {

                    VFILE *currentBinary;
                    RXLIB_BIN_HEADER   binHeader;
                    QUEUE *qprocs;
                    BYTE binHeaderSig[4] = RXLIB_FILE_HEADER_SIG;

                    currentBinary = binaries;

                    fprintf(stdout, "\t\t- '%s' \n", currentBinary->fullname);

                    memcpy(&binHeader.sig, binHeaderSig, sizeof(binHeaderSig));

                    binHeader.fnlength = (short) strlen(currentBinary->basename);
                    // will be written later
                    binHeader.pnlength = 0;

                    fwrite(&binHeader, sizeof(RXLIB_BIN_HEADER), 1, library->fp);
                    fwrite(currentBinary->basename, binHeader.fnlength, 1, library->fp);

                    qprocs = findExposedProcedures(currentBinary, &qprocs);

                    fprintf(stdout, "\t\t  Exposed procedures: \n");

                    while (!isEmpty(qprocs)) {
                        RXLIB_BIN_PROC_NAME *binProcName;

                        binProcName = dequeue(qprocs);

                        fprintf(stdout, "\t\t\t- '%s' \n", binProcName->pname);

                        fwrite(binProcName, binProcName->pnlen + 2, 1, library->fp);
                    }

                    currentBinary = vfopen(binaries, "rb");
                    if (currentBinary->opened) {
                        int ch;

                        while((ch = fgetc(currentBinary->fp)) != EOF)
                            fputc(ch, library->fp);

                        vfclose(currentBinary);
                    }

                    // cleanup
                    freequeue(qprocs);

                } else {
                    fprintf(stderr, "WARN: File '%s' did not exists. \n", binaries->fullname);
                }
            }

            binaries = binaries->next;

        } while (binaries);

        vfclose(library);
    } else {
        // TODO: throw error
    }

    return(0);
}