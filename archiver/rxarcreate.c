#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "platform.h"
#include "rxaradd.h"
#include "queue.h"
#include "rxarutil.h"

extern BOOL verboseFlag;

int createLibrary(VFILE *library, VFILE *binaries) {

    RXLIB_DIRECTORY_ENTRY *directoryEntry;
    QUEUE                 *qDirectoryEntries;
    long                   directoryOffset;
    long                   directoryOffsetPos;
    size_t                 directoryEntryCount;
    size_t                 binaryCount;

    if (binaries) {
        BYTE libFileSig[4] = RXLIB_FILE_SIG;

        binaryCount = vfcnt(binaries);

        if (verboseFlag) {
            fprintf(stdout, "Library '%s' will be created from following %lu binary modules:\n", library->fullname, binaryCount);
        }

        qDirectoryEntries = newqueue(binaryCount);
        directoryEntryCount = 0;

        library = vfopen(library, "wb");

        fwrite(libFileSig, sizeof(libFileSig), 1, library->fp);

        // skipping directory offset field, will be written later
        directoryOffsetPos = ftell(library->fp);
        fseek(library->fp, 8, SEEK_CUR);

        do {
            if (!binaries->wildcarded) {
                if (binaries->exists) {

                    VFILE *currentBinary;
                    RXLIB_BIN_HEADER   binHeader;
                    QUEUE *qprocs;

                    BYTE binHeaderSig[4] = RXLIB_FILE_HEADER_SIG;

                    long  binHeaderOffset;
                    long  binStartOffset, binEndOffset;

                    currentBinary = binaries;

                    memcpy(&binHeader.sig, binHeaderSig, sizeof(binHeaderSig));
                    binHeader.fnlength = (short) strlen(currentBinary->basename);
                    // will be written later
                    binHeader.pnlength = 0;

                    // remember position von binHeader
                    binHeaderOffset = ftell(library->fp);

                    directoryEntryCount++;
                    directoryEntry = calloc(1, sizeof(RXLIB_DIRECTORY_ENTRY) + binHeader.fnlength);
                    directoryEntry->size = (short) (sizeof(RXLIB_DIRECTORY_ENTRY) + binHeader.fnlength);
                    directoryEntry->offset   = binHeaderOffset;
                    directoryEntry->fnlength = binHeader.fnlength;
                    memcpy(directoryEntry->fn, currentBinary->basename, binHeader.fnlength);

                    if (verboseFlag) {
                        fprintf(stdout, "\t\t- (%.5lu) '%s' \n", directoryEntryCount, currentBinary->fullname);
                    } else {
                        fprintf(stdout, "a %s\n", currentBinary->fullname);
                    }

                    fwrite(&binHeader, sizeof(RXLIB_BIN_HEADER), 1, library->fp);
                    fwrite(currentBinary->basename, binHeader.fnlength, 1, library->fp);

                    qprocs      = findExposedProcedures(currentBinary, &qprocs);

                    if (verboseFlag) {
                        fprintf(stdout, "\t\t  Exposed procedures: \n");
                    }

                    while (!isEmpty(qprocs)) {
                        RXLIB_BIN_PROC_NAME *binProcName;

                        binProcName = dequeue(qprocs);

                        if (verboseFlag) {
                            fprintf(stdout, "\t\t\t- '%s' \n", binProcName->pname);
                        }

                        // the +2 is the size of the pnlen field, that will be written, too.
                        fwrite(binProcName, binProcName->pnlen + 2, 1, library->fp);
                        binHeader.pnlength += binProcName->pnlen + 2;
                    }

                    // save current position
                    binStartOffset = ftell(library->fp);

                    // update header for this binary (+6 to seek directly to the pnlength field)
                    fseek(library->fp, binHeaderOffset + 6, SEEK_SET);
                    fwrite(&binHeader.pnlength, 2, 1, library->fp);

                    // seek back to old position
                    fseek(library->fp, binStartOffset, SEEK_SET);

                    // writing binary file to library
                    currentBinary = vfopen(binaries, "rb");
                    if (currentBinary->opened) {
                        int ch;

                        while((ch = fgetc(currentBinary->fp)) != EOF)
                            fputc(ch, library->fp);

                        vfclose(currentBinary);
                    }
                    binEndOffset = ftell(library->fp);

                    directoryEntry->bytecnt_u = binEndOffset - binStartOffset;

                    // enqueue current directory entry
                    enqueue(qDirectoryEntries, directoryEntry);

                    // cleanup
                    freequeue(qprocs);

                } else {
                    fprintf(stderr, "WARN: File '%s' did not exists. \n", binaries->fullname);
                }
            }

            binaries = binaries->next;

        } while (binaries);


        // write directory
        if (directoryEntryCount > 0) {
            directoryOffset = ftell(library->fp);

            // seek back to field holding the directory position
            // and write postion of the directory to this field
            fseek(library->fp, directoryOffsetPos, SEEK_SET);
            fwrite(&directoryOffset, sizeof(long), 1, library->fp);

            // seek back to last position;
            fseek(library->fp, directoryOffset, SEEK_SET);

            while (!isEmpty(qDirectoryEntries)) {
                directoryEntry = dequeue(qDirectoryEntries);

                fwrite(directoryEntry, sizeof(RXLIB_DIRECTORY_ENTRY) + directoryEntry->fnlength, 1, library->fp);
                free(directoryEntry);
            }
        }

        // cleanup
        freequeue(qDirectoryEntries);
        vfclose(library);
    } else {
        // TODO: throw error
    }

    return(0);
}