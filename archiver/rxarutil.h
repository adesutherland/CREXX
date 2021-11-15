#ifndef CREXX_RXARUTIL_H
#define CREXX_RXARUTIL_H

#include "types.h"
#include "errors.h"
#include "rxas.h"
#include "queue.h"

typedef struct {
    BOOL loaded;
    char *name;
    bin_space segment;
} abin;

#pragma pack(push,1)
typedef struct {
    BYTE sig[4];        /* Signature / ID / Eycatcher           */
    short fnlength;     /* length of the binary file name       */
    short pnlength;     /* length of all proc name elements     */
    BYTE fn_pn[];       /* contains the binaray file name       */
                        /* and a set of proc names elements.    */
} RXLIB_BIN_HEADER;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    short pnlen;
    char  pname[];
} RXLIB_BIN_PROC_NAME;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
} RXLIB_DIRECTORY;
#pragma pack(pop)

#define RXLIB_FILE_SIG          { 0xC0, 0xDE, 0xBA, 0x5E }
#define RXLIB_FILE_HEADER_SIG   { 0x42, 0x00, 0x02, 0x43 }

QUEUE* findExposedProcedures(VFILE *vfile, QUEUE **procnames);
QUEUE* findImportedProcedures(VFILE *vfile, QUEUE **procnames);

#endif //CREXX_RXARUTIL_H
