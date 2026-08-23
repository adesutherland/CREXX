/* RCC-5F test-only direct native payload-scan ceiling. */

#include <math.h>

#include "crexxpa.h"
#include "rxstats_kernel.h"

RXPA_PLUGIN_PROCESS_REENTRANT

static rxstats_status open_span(rxpa_attribute_value value,
                                rxstats_span *span)
{
    const void *payload;
    size_t length = 0u;

    payload = GETNATIVEPAYLOAD(value, &length, NULL, NULL);
    if ((!payload && length != 0u) || length % sizeof(double) != 0u)
        return RXSTATS_INVALID_PAYLOAD;
    span->data = (const unsigned char *)payload;
    span->count = length / sizeof(double);
    return RXSTATS_OK;
}

PROCEDURE(run)
{
    rxstats_span x_values;
    rxstats_span y_values;
    rxstats_moments x_mean;
    rxstats_moments x_deviation;
    rxstats_pair_moments covariance_moments;
    rxstats_pair_moments correlation_moments;
    rxstats_status status;
    rxinteger iterations;
    rxinteger iteration;
    double checksum = 0.0;

    if (NUM_ARGS != 3)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RCC5FDIRECT.RUN expects two packed floats and an iteration count")
    if (!ISINITIALIZED(ARG0) || !ISINITIALIZED(ARG1))
        RETURNSIGNAL(SIGNAL_OBJECT_NOT_INITIALIZED,
                     "RCC5FDIRECT.RUN received an uninitialized packedfloat")
    iterations = GETINT(ARG2);
    if (iterations < 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RCC5FDIRECT.RUN requires a positive iteration count")
    status = open_span(ARG0, &x_values);
    if (status == RXSTATS_OK)
        status = open_span(ARG1, &y_values);
    if (status != RXSTATS_OK || x_values.count < 2u ||
        x_values.count != y_values.count)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RCC5FDIRECT.RUN requires equal non-singleton packed floats")

    for (iteration = 0; iteration < iterations; ++iteration) {
        double deviation;
        double covariance;
        double correlation;

        status = rxstats_accumulate(&x_values, 0, &x_mean);
        if (status == RXSTATS_OK)
            status = rxstats_accumulate(&x_values, 1, &x_deviation);
        if (status == RXSTATS_OK)
            status = rxstats_accumulate_pairs(&x_values, &y_values,
                                               &covariance_moments);
        if (status == RXSTATS_OK)
            status = rxstats_accumulate_pairs(&x_values, &y_values,
                                               &correlation_moments);
        if (status != RXSTATS_OK)
            RETURNSIGNAL(SIGNAL_FAILURE,
                         "RCC5FDIRECT.RUN statistics kernel failed")

        deviation = sqrt(x_deviation.m2 /
                         (double)(x_deviation.count - 1u));
        covariance = covariance_moments.co_moment /
                     (double)(covariance_moments.count - 1u);
        correlation =
                (correlation_moments.co_moment /
                 sqrt(correlation_moments.m2_x)) /
                sqrt(correlation_moments.m2_y);
        /* Match the public Rexx workload's left-to-right checksum rounding. */
        checksum += x_mean.mean;
        checksum += deviation;
        checksum += covariance;
        checksum += correlation;
    }
    if (!isfinite(checksum))
        RETURNSIGNAL(SIGNAL_OVERFLOW_UNDERFLOW,
                     "RCC5FDIRECT.RUN checksum is outside the native float range")
    RETURNFLOAT(checksum);
    RESETSIGNAL
}

LOADFUNCS
ADDPROC(run, "rcc5fdirect.run", "b", ".float",
        "x = .packedfloat,y = .packedfloat,iterations = .int");
ENDLOADFUNCS
