/*
 * cREXX License (MIT)
 *
 * Shared packed-statistics kernel contract.  The production RXPA surface and
 * the RCC-5F direct-scan performance control compile the same implementation.
 */

#ifndef CREXX_RXSTATS_KERNEL_H
#define CREXX_RXSTATS_KERNEL_H

#include <stddef.h>

typedef enum rxstats_status {
    RXSTATS_OK = 0,
    RXSTATS_INVALID_PAYLOAD,
    RXSTATS_UNEQUAL_LENGTH,
    RXSTATS_NONFINITE_INPUT,
    RXSTATS_NUMERIC_OVERFLOW
} rxstats_status;

typedef struct rxstats_span {
    const unsigned char *data;
    size_t count;
} rxstats_span;

typedef struct rxstats_moments {
    size_t count;
    double mean;
    double m2;
} rxstats_moments;

typedef struct rxstats_pair_moments {
    size_t count;
    double mean_x;
    double mean_y;
    double m2_x;
    double m2_y;
    double co_moment;
} rxstats_pair_moments;

rxstats_status rxstats_accumulate(const rxstats_span *values,
                                  int need_second_moment,
                                  rxstats_moments *result);
rxstats_status rxstats_accumulate_pairs(const rxstats_span *x_values,
                                        const rxstats_span *y_values,
                                        rxstats_pair_moments *result);

#endif
