/* RXVECTOR-01 one-call direct exact-kernel control. */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "crexxpa.h"
#include "rxvector_kernel.h"

RXPA_PLUGIN_PROCESS_REENTRANT

static int open_float_span(rxpa_attribute_value value,
                           rxvector_float_span *span)
{
    const void *payload;
    size_t length = 0u;

    payload = GETNATIVEPAYLOAD(value, &length, NULL, NULL);
    if ((!payload && length != 0u) || length % sizeof(double) != 0u)
        return 0;
    span->data = (const unsigned char *)payload;
    span->count = length / sizeof(double);
    return 1;
}

static int open_int_span(rxpa_attribute_value value, rxvector_int_span *span)
{
    const void *payload;
    size_t length = 0u;

    payload = GETNATIVEPAYLOAD(value, &length, NULL, NULL);
    if (sizeof(rxinteger) != sizeof(int64_t) ||
        (!payload && length != 0u) || length % sizeof(rxinteger) != 0u)
        return 0;
    span->data = (const unsigned char *)payload;
    span->count = length / sizeof(rxinteger);
    return 1;
}

PROCEDURE(topk)
{
    rxvector_float_span vectors;
    rxvector_int_span identities;
    rxvector_float_span query_vector;
    rxvector_hit *hits;
    rxinteger dimensions_value;
    rxinteger requested_value;
    rxinteger iterations;
    rxinteger iteration;
    size_t requested;
    size_t index;
    double checksum = 0.0;

    if (NUM_ARGS != 6)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXVECTOR01DIRECT.TOPK expects six arguments")
    if (!open_float_span(ARG0, &vectors) ||
        !open_int_span(ARG1, &identities) ||
        !open_float_span(ARG3, &query_vector))
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXVECTOR01DIRECT.TOPK received invalid packed data")
    dimensions_value = GETINT(ARG2);
    requested_value = GETINT(ARG4);
    iterations = GETINT(ARG5);
    if (dimensions_value <= 0 || requested_value <= 0 || iterations <= 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXVECTOR01DIRECT.TOPK requires positive counts")
    requested = (size_t)requested_value;
    if ((rxinteger)requested != requested_value ||
        requested > SIZE_MAX / sizeof(*hits))
        RETURNSIGNAL(SIGNAL_OUT_OF_RANGE,
                     "RXVECTOR01DIRECT.TOPK result is too large")
    hits = (rxvector_hit *)malloc(requested * sizeof(*hits));
    if (!hits)
        RETURNSIGNAL(SIGNAL_FAILURE,
                     "RXVECTOR01DIRECT.TOPK could not allocate results")
    for (iteration = 0; iteration < iterations; ++iteration) {
        rxvector_status status = rxvector_topk_kernel(
                &vectors, &identities, (size_t)dimensions_value,
                &query_vector, requested, hits);
        if (status != RXVECTOR_OK) {
            free(hits);
            RETURNSIGNAL(SIGNAL_FAILURE,
                         "RXVECTOR01DIRECT.TOPK kernel failed")
        }
        for (index = 0u; index < requested; ++index) {
            checksum += (double)hits[index].identity;
            checksum += hits[index].score;
        }
    }
    free(hits);
    RETURNFLOAT(checksum);
    RESETSIGNAL
}

LOADFUNCS
ADDPROC(topk, "rxvector01direct.topk", "b", ".float",
        "vectors = .packedfloat,identities = .packedint,dimensions = .int,query_vector = .packedfloat,requested = .int,iterations = .int");
ENDLOADFUNCS
