/*
 * cREXX License (MIT)
 *
 * Shared exact packed-vector kernel. The production RXPA provider and the
 * RXVECTOR-01 direct-C performance control compile this implementation.
 */

#ifndef CREXX_RXVECTOR_KERNEL_H
#define CREXX_RXVECTOR_KERNEL_H

#include <stddef.h>
#include <stdint.h>

typedef enum rxvector_status {
    RXVECTOR_OK = 0,
    RXVECTOR_INVALID_PAYLOAD,
    RXVECTOR_UNEQUAL_LENGTH,
    RXVECTOR_EMPTY_VECTOR,
    RXVECTOR_NONFINITE_INPUT,
    RXVECTOR_ZERO_NORM,
    RXVECTOR_NUMERIC_OVERFLOW,
    RXVECTOR_ALLOCATION_FAILED
} rxvector_status;

typedef struct rxvector_float_span {
    const unsigned char *data;
    size_t count;
} rxvector_float_span;

typedef struct rxvector_int_span {
    const unsigned char *data;
    size_t count;
} rxvector_int_span;

typedef struct rxvector_hit {
    double score;
    int64_t identity;
    size_t row;
} rxvector_hit;

rxvector_status rxvector_cosine_kernel(const rxvector_float_span *left,
                                       const rxvector_float_span *right,
                                       double *score);

rxvector_status rxvector_topk_kernel(const rxvector_float_span *vectors,
                                     const rxvector_int_span *identities,
                                     size_t dimensions,
                                     const rxvector_float_span *query,
                                     size_t requested,
                                     rxvector_hit *hits);

#endif
