/*
 * cREXX License (MIT)
 *
 * Binary digest functions supplied by the standard rx_hash provider.
 */

#include <stddef.h>

#include "crexxpa.h"
#include "rxsha256.h"

RXPA_PLUGIN_PROCESS_REENTRANT

PROCEDURE(sha256)
{
    const void *data;
    size_t length;
    unsigned char digest[32];

    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")

    length = 0u;
    data = GETNATIVEPAYLOAD(ARG(0), &length, NULL, NULL);
    if (!data && length != 0u)
        RETURNSIGNAL(SIGNAL_FAILURE, "binary payload is unavailable")

    rx_sha256(data, length, digest);
    if (SETNATIVEPAYLOAD(RETURN, digest, sizeof(digest), NULL, 0u) != 0)
        RETURNSIGNAL(SIGNAL_FAILURE, "SHA-256 digest allocation failed")
    RESETSIGNAL
}

LOADFUNCS
ADDPROC(sha256, "rxhash.sha256", "b", ".binary", "data = .binary");
ENDLOADFUNCS
