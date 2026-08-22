/*
 * cREXX License (MIT)
 *
 * Native statistics over host-native packed float owners.
 */

#include <float.h>
#include <math.h>
#include <stddef.h>
#include <string.h>

#include "crexxpa.h"
#include "rxstats_kernel.h"

#ifndef RXSTATS_KERNEL_ONLY
RXPA_PLUGIN_PROCESS_REENTRANT
#endif

typedef struct rxstats_compensated_sum {
    double sum;
    double correction;
} rxstats_compensated_sum;

#ifndef RXSTATS_KERNEL_ONLY
static rxstats_status rxstats_open_span(rxpa_attribute_value value,
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
#endif

/* memcpy is alias-safe and compilers lower this fixed-width load directly. */
static double rxstats_span_value(const rxstats_span *span, size_t index)
{
    double value;

    memcpy(&value, span->data + index * sizeof(value), sizeof(value));
    return value;
}

static void rxstats_sum_reset(rxstats_compensated_sum *accumulator)
{
    accumulator->sum = 0.0;
    accumulator->correction = 0.0;
}

/* Neumaier compensation retains a low-order residual when magnitudes differ. */
static rxstats_status rxstats_sum_add(rxstats_compensated_sum *accumulator,
                                      double value)
{
    double combined = accumulator->sum + value;
    double residual;
    double correction;

    if (!isfinite(combined)) return RXSTATS_NUMERIC_OVERFLOW;
    if (fabs(accumulator->sum) >= fabs(value))
        residual = (accumulator->sum - combined) + value;
    else
        residual = (value - combined) + accumulator->sum;
    correction = accumulator->correction + residual;
    if (!isfinite(residual) || !isfinite(correction))
        return RXSTATS_NUMERIC_OVERFLOW;
    accumulator->sum = combined;
    accumulator->correction = correction;
    return RXSTATS_OK;
}

static rxstats_status rxstats_sum_value(
        const rxstats_compensated_sum *accumulator, double *result)
{
    *result = accumulator->sum + accumulator->correction;
    return isfinite(*result) ? RXSTATS_OK : RXSTATS_NUMERIC_OVERFLOW;
}

/*
 * A shifted single pass normally avoids cancellation. If its final central
 * moment is tiny compared with the shifted square sum, recompute about the
 * already-known mean rather than returning a cancellation artifact.
 */
static int rxstats_needs_second_pass(double central_moment,
                                     double shifted_square_sum)
{
    if (!isfinite(central_moment)) return 1;
    if (central_moment < 0.0) return 1;
    if (shifted_square_sum == 0.0) return 0;
    return central_moment <= 64.0 * DBL_EPSILON * fabs(shifted_square_sum);
}

static rxstats_status rxstats_scaled_mean(const rxstats_span *values,
                                          double *mean)
{
    size_t index;
    double count = (double)values->count;
    rxstats_compensated_sum scaled;
    rxstats_status status;

    rxstats_sum_reset(&scaled);
    for (index = 0u; index < values->count; ++index) {
        double value = rxstats_span_value(values, index);

        if (!isfinite(value)) return RXSTATS_NONFINITE_INPUT;
        status = rxstats_sum_add(&scaled, value / count);
        if (status != RXSTATS_OK) return status;
    }
    return rxstats_sum_value(&scaled, mean);
}

static rxstats_status rxstats_second_moment(const rxstats_span *values,
                                            double mean, double *moment)
{
    size_t index;
    rxstats_compensated_sum squares;
    rxstats_status status;

    rxstats_sum_reset(&squares);
    for (index = 0u; index < values->count; ++index) {
        double difference = rxstats_span_value(values, index) - mean;
        double square = difference * difference;

        if (!isfinite(difference) || !isfinite(square))
            return RXSTATS_NUMERIC_OVERFLOW;
        status = rxstats_sum_add(&squares, square);
        if (status != RXSTATS_OK) return status;
    }
    return rxstats_sum_value(&squares, moment);
}

rxstats_status rxstats_accumulate(const rxstats_span *values,
                                  int need_second_moment,
                                  rxstats_moments *result)
{
    size_t index;
    double origin;
    double shifted_total;
    double shifted_square_total = 0.0;
    rxstats_compensated_sum shifted;
    rxstats_compensated_sum shifted_squares;
    rxstats_status status;

    result->count = values->count;
    result->mean = 0.0;
    result->m2 = 0.0;
    if (values->count == 0u) return RXSTATS_OK;

    origin = rxstats_span_value(values, 0u);
    if (!isfinite(origin)) return RXSTATS_NONFINITE_INPUT;
    rxstats_sum_reset(&shifted);
    rxstats_sum_reset(&shifted_squares);
    for (index = 1u; index < values->count; ++index) {
        double value = rxstats_span_value(values, index);
        double difference;

        if (!isfinite(value)) return RXSTATS_NONFINITE_INPUT;
        difference = value - origin;
        if (!isfinite(difference)) {
            if (!need_second_moment)
                return rxstats_scaled_mean(values, &result->mean);
            return RXSTATS_NUMERIC_OVERFLOW;
        }
        status = rxstats_sum_add(&shifted, difference);
        if (status != RXSTATS_OK) return status;
        if (need_second_moment) {
            double square = difference * difference;

            if (!isfinite(square)) return RXSTATS_NUMERIC_OVERFLOW;
            status = rxstats_sum_add(&shifted_squares, square);
            if (status != RXSTATS_OK) return status;
        }
    }
    status = rxstats_sum_value(&shifted, &shifted_total);
    if (status != RXSTATS_OK) return status;
    result->mean = origin + shifted_total / (double)values->count;
    if (!isfinite(result->mean)) {
        if (!need_second_moment)
            return rxstats_scaled_mean(values, &result->mean);
        return RXSTATS_NUMERIC_OVERFLOW;
    }
    if (!need_second_moment) return RXSTATS_OK;

    status = rxstats_sum_value(&shifted_squares, &shifted_square_total);
    if (status != RXSTATS_OK) return status;
    result->m2 = shifted_square_total -
                 shifted_total * shifted_total / (double)values->count;
    if (rxstats_needs_second_pass(result->m2, shifted_square_total))
        return rxstats_second_moment(values, result->mean, &result->m2);
    return RXSTATS_OK;
}

rxstats_status rxstats_accumulate_pairs(
        const rxstats_span *x_values, const rxstats_span *y_values,
        rxstats_pair_moments *result)
{
    size_t index;
    double origin_x;
    double origin_y;
    double shifted_x_total;
    double shifted_y_total;
    double shifted_x_square_total;
    double shifted_y_square_total;
    double shifted_cross_total;
    rxstats_compensated_sum shifted_x;
    rxstats_compensated_sum shifted_y;
    rxstats_compensated_sum shifted_x_squares;
    rxstats_compensated_sum shifted_y_squares;
    rxstats_compensated_sum shifted_cross;
    rxstats_status status;

    if (x_values->count != y_values->count)
        return RXSTATS_UNEQUAL_LENGTH;
    result->count = x_values->count;
    result->mean_x = 0.0;
    result->mean_y = 0.0;
    result->m2_x = 0.0;
    result->m2_y = 0.0;
    result->co_moment = 0.0;
    if (x_values->count == 0u) return RXSTATS_OK;

    origin_x = rxstats_span_value(x_values, 0u);
    origin_y = rxstats_span_value(y_values, 0u);
    if (!isfinite(origin_x) || !isfinite(origin_y))
        return RXSTATS_NONFINITE_INPUT;

    rxstats_sum_reset(&shifted_x);
    rxstats_sum_reset(&shifted_y);
    rxstats_sum_reset(&shifted_x_squares);
    rxstats_sum_reset(&shifted_y_squares);
    rxstats_sum_reset(&shifted_cross);
    for (index = 1u; index < x_values->count; ++index) {
        double x = rxstats_span_value(x_values, index);
        double y = rxstats_span_value(y_values, index);
        double difference_x;
        double difference_y;
        double square_x;
        double square_y;
        double cross;

        if (!isfinite(x) || !isfinite(y))
            return RXSTATS_NONFINITE_INPUT;
        difference_x = x - origin_x;
        difference_y = y - origin_y;
        square_x = difference_x * difference_x;
        square_y = difference_y * difference_y;
        cross = difference_x * difference_y;
        if (!isfinite(difference_x) || !isfinite(difference_y) ||
            !isfinite(square_x) || !isfinite(square_y) || !isfinite(cross))
            return RXSTATS_NUMERIC_OVERFLOW;
        status = rxstats_sum_add(&shifted_x, difference_x);
        if (status != RXSTATS_OK) return status;
        status = rxstats_sum_add(&shifted_y, difference_y);
        if (status != RXSTATS_OK) return status;
        status = rxstats_sum_add(&shifted_x_squares, square_x);
        if (status != RXSTATS_OK) return status;
        status = rxstats_sum_add(&shifted_y_squares, square_y);
        if (status != RXSTATS_OK) return status;
        status = rxstats_sum_add(&shifted_cross, cross);
        if (status != RXSTATS_OK) return status;
    }

    status = rxstats_sum_value(&shifted_x, &shifted_x_total);
    if (status != RXSTATS_OK) return status;
    status = rxstats_sum_value(&shifted_y, &shifted_y_total);
    if (status != RXSTATS_OK) return status;
    status = rxstats_sum_value(&shifted_x_squares,
                               &shifted_x_square_total);
    if (status != RXSTATS_OK) return status;
    status = rxstats_sum_value(&shifted_y_squares,
                               &shifted_y_square_total);
    if (status != RXSTATS_OK) return status;
    status = rxstats_sum_value(&shifted_cross, &shifted_cross_total);
    if (status != RXSTATS_OK) return status;

    result->mean_x = origin_x +
                     shifted_x_total / (double)x_values->count;
    result->mean_y = origin_y +
                     shifted_y_total / (double)y_values->count;
    if (!isfinite(result->mean_x) || !isfinite(result->mean_y))
        return RXSTATS_NUMERIC_OVERFLOW;
    result->m2_x = shifted_x_square_total -
                   shifted_x_total * shifted_x_total /
                   (double)x_values->count;
    result->m2_y = shifted_y_square_total -
                   shifted_y_total * shifted_y_total /
                   (double)y_values->count;
    result->co_moment = shifted_cross_total -
                        shifted_x_total * shifted_y_total /
                        (double)x_values->count;

    if (rxstats_needs_second_pass(result->m2_x,
                                  shifted_x_square_total) ||
        rxstats_needs_second_pass(result->m2_y,
                                  shifted_y_square_total) ||
        !isfinite(result->co_moment)) {
        rxstats_compensated_sum centered_x_squares;
        rxstats_compensated_sum centered_y_squares;
        rxstats_compensated_sum centered_cross;

        rxstats_sum_reset(&centered_x_squares);
        rxstats_sum_reset(&centered_y_squares);
        rxstats_sum_reset(&centered_cross);
        for (index = 0u; index < x_values->count; ++index) {
            double difference_x =
                    rxstats_span_value(x_values, index) - result->mean_x;
            double difference_y =
                    rxstats_span_value(y_values, index) - result->mean_y;
            double square_x = difference_x * difference_x;
            double square_y = difference_y * difference_y;
            double cross = difference_x * difference_y;

            if (!isfinite(difference_x) || !isfinite(difference_y) ||
                !isfinite(square_x) || !isfinite(square_y) ||
                !isfinite(cross))
                return RXSTATS_NUMERIC_OVERFLOW;
            status = rxstats_sum_add(&centered_x_squares, square_x);
            if (status != RXSTATS_OK) return status;
            status = rxstats_sum_add(&centered_y_squares, square_y);
            if (status != RXSTATS_OK) return status;
            status = rxstats_sum_add(&centered_cross, cross);
            if (status != RXSTATS_OK) return status;
        }
        status = rxstats_sum_value(&centered_x_squares, &result->m2_x);
        if (status != RXSTATS_OK) return status;
        status = rxstats_sum_value(&centered_y_squares, &result->m2_y);
        if (status != RXSTATS_OK) return status;
        status = rxstats_sum_value(&centered_cross, &result->co_moment);
        if (status != RXSTATS_OK) return status;
    }
    return RXSTATS_OK;
}

#ifndef RXSTATS_KERNEL_ONLY

PROCEDURE(mean)
{
    rxstats_span values;
    rxstats_moments moments;
    rxstats_status status;

    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.MEAN expects one packed float owner")
    if (!ISINITIALIZED(ARG0))
        RETURNSIGNAL(SIGNAL_OBJECT_NOT_INITIALIZED,
                     "RXSTATS.MEAN received an uninitialized packedfloat")
    status = rxstats_open_span(ARG0, &values);
    if (status == RXSTATS_INVALID_PAYLOAD)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.MEAN received an invalid packed float payload")
    status = rxstats_accumulate(&values, 0, &moments);
    if (status == RXSTATS_NONFINITE_INPUT)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.MEAN requires finite values")
    if (status == RXSTATS_NUMERIC_OVERFLOW)
        RETURNSIGNAL(SIGNAL_OVERFLOW_UNDERFLOW,
                     "RXSTATS.MEAN result is outside the native float range")
    if (moments.count < 1u)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.MEAN requires at least one value")
    RETURNFLOAT(moments.mean);
    RESETSIGNAL
}

PROCEDURE(stddev)
{
    rxstats_span values;
    rxstats_moments moments;
    rxstats_status status;
    double result;

    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.STDDEV expects one packed float owner")
    if (!ISINITIALIZED(ARG0))
        RETURNSIGNAL(SIGNAL_OBJECT_NOT_INITIALIZED,
                     "RXSTATS.STDDEV received an uninitialized packedfloat")
    status = rxstats_open_span(ARG0, &values);
    if (status == RXSTATS_INVALID_PAYLOAD)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.STDDEV received an invalid packed float payload")
    status = rxstats_accumulate(&values, 1, &moments);
    if (status == RXSTATS_NONFINITE_INPUT)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.STDDEV requires finite values")
    if (status == RXSTATS_NUMERIC_OVERFLOW)
        RETURNSIGNAL(SIGNAL_OVERFLOW_UNDERFLOW,
                     "RXSTATS.STDDEV result is outside the native float range")
    if (moments.count < 2u)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.STDDEV requires at least two values")
    result = sqrt(moments.m2 / (double)(moments.count - 1u));
    if (!isfinite(result))
        RETURNSIGNAL(SIGNAL_OVERFLOW_UNDERFLOW,
                     "RXSTATS.STDDEV result is outside the native float range")
    RETURNFLOAT(result);
    RESETSIGNAL
}

PROCEDURE(covariance)
{
    rxstats_span x_values;
    rxstats_span y_values;
    rxstats_pair_moments moments;
    rxstats_status status;
    double result;

    if (NUM_ARGS != 2)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.COVARIANCE expects two packed float owners")
    if (!ISINITIALIZED(ARG0) || !ISINITIALIZED(ARG1))
        RETURNSIGNAL(SIGNAL_OBJECT_NOT_INITIALIZED,
                     "RXSTATS.COVARIANCE received an uninitialized packedfloat")
    status = rxstats_open_span(ARG0, &x_values);
    if (status == RXSTATS_OK)
        status = rxstats_open_span(ARG1, &y_values);
    if (status == RXSTATS_INVALID_PAYLOAD)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.COVARIANCE received an invalid packed float payload")
    status = rxstats_accumulate_pairs(&x_values, &y_values, &moments);
    if (status == RXSTATS_UNEQUAL_LENGTH)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.COVARIANCE requires equal-length inputs")
    if (status == RXSTATS_NONFINITE_INPUT)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.COVARIANCE requires finite values")
    if (status == RXSTATS_NUMERIC_OVERFLOW)
        RETURNSIGNAL(SIGNAL_OVERFLOW_UNDERFLOW,
                     "RXSTATS.COVARIANCE result is outside the native float range")
    if (moments.count < 2u)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.COVARIANCE requires at least two pairs")
    result = moments.co_moment / (double)(moments.count - 1u);
    if (!isfinite(result))
        RETURNSIGNAL(SIGNAL_OVERFLOW_UNDERFLOW,
                     "RXSTATS.COVARIANCE result is outside the native float range")
    RETURNFLOAT(result);
    RESETSIGNAL
}

PROCEDURE(correlation)
{
    rxstats_span x_values;
    rxstats_span y_values;
    rxstats_pair_moments moments;
    rxstats_status status;
    double result;

    if (NUM_ARGS != 2)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.CORRELATION expects two packed float owners")
    if (!ISINITIALIZED(ARG0) || !ISINITIALIZED(ARG1))
        RETURNSIGNAL(SIGNAL_OBJECT_NOT_INITIALIZED,
                     "RXSTATS.CORRELATION received an uninitialized packedfloat")
    status = rxstats_open_span(ARG0, &x_values);
    if (status == RXSTATS_OK)
        status = rxstats_open_span(ARG1, &y_values);
    if (status == RXSTATS_INVALID_PAYLOAD)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.CORRELATION received an invalid packed float payload")
    status = rxstats_accumulate_pairs(&x_values, &y_values, &moments);
    if (status == RXSTATS_UNEQUAL_LENGTH)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.CORRELATION requires equal-length inputs")
    if (status == RXSTATS_NONFINITE_INPUT)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.CORRELATION requires finite values")
    if (status == RXSTATS_NUMERIC_OVERFLOW)
        RETURNSIGNAL(SIGNAL_OVERFLOW_UNDERFLOW,
                     "RXSTATS.CORRELATION result is outside the native float range")
    if (moments.count < 2u)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.CORRELATION requires at least two pairs")
    if (moments.m2_x == 0.0 || moments.m2_y == 0.0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.CORRELATION is undefined for zero variance")
    result = (moments.co_moment / sqrt(moments.m2_x)) /
             sqrt(moments.m2_y);
    if (!isfinite(result))
        RETURNSIGNAL(SIGNAL_OVERFLOW_UNDERFLOW,
                     "RXSTATS.CORRELATION result is outside the native float range")
    RETURNFLOAT(result);
    RESETSIGNAL
}

PROCEDURE(regression)
{
    rxstats_span x_values;
    rxstats_span y_values;
    rxstats_pair_moments moments;
    rxstats_status status;
    double coefficients[2];

    if (NUM_ARGS != 2)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.REGRESSION expects two packed float owners")
    if (!ISINITIALIZED(ARG0) || !ISINITIALIZED(ARG1))
        RETURNSIGNAL(SIGNAL_OBJECT_NOT_INITIALIZED,
                     "RXSTATS.REGRESSION received an uninitialized packedfloat")
    status = rxstats_open_span(ARG0, &x_values);
    if (status == RXSTATS_OK)
        status = rxstats_open_span(ARG1, &y_values);
    if (status == RXSTATS_INVALID_PAYLOAD)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.REGRESSION received an invalid packed float payload")
    status = rxstats_accumulate_pairs(&x_values, &y_values, &moments);
    if (status == RXSTATS_UNEQUAL_LENGTH)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.REGRESSION requires equal-length inputs")
    if (status == RXSTATS_NONFINITE_INPUT)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.REGRESSION requires finite values")
    if (status == RXSTATS_NUMERIC_OVERFLOW)
        RETURNSIGNAL(SIGNAL_OVERFLOW_UNDERFLOW,
                     "RXSTATS.REGRESSION result is outside the native float range")
    if (moments.count < 2u)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.REGRESSION requires at least two pairs")
    if (moments.m2_x == 0.0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXSTATS.REGRESSION is undefined for zero x variance")

    coefficients[0] = moments.co_moment / moments.m2_x;
    coefficients[1] = moments.mean_y - coefficients[0] * moments.mean_x;
    if (!isfinite(coefficients[0]) || !isfinite(coefficients[1]))
        RETURNSIGNAL(SIGNAL_OVERFLOW_UNDERFLOW,
                     "RXSTATS.REGRESSION result is outside the native float range")
    if (SETNATIVEPAYLOAD(RETURN, coefficients, sizeof(coefficients),
                         NULL, 0u) != 0)
        RETURNSIGNAL(SIGNAL_FAILURE,
                     "RXSTATS.REGRESSION could not allocate its result")
    RESETSIGNAL
}

LOADFUNCS
    ADDPROC(mean, "rxstats.mean", "b", ".float",
            "values = .packedfloat");
    ADDPROC(stddev, "rxstats.stddev", "b", ".float",
            "values = .packedfloat");
    ADDPROC(covariance, "rxstats.covariance", "b", ".float",
            "x = .packedfloat,y = .packedfloat");
    ADDPROC(correlation, "rxstats.correlation", "b", ".float",
            "x = .packedfloat,y = .packedfloat");
    /* RXBIN metadata stores namespaced class types in canonical form. */
    ADDPROC(regression, "rxstats.regression", "b", ".rxstats..linearfit",
            "x = .packedfloat,y = .packedfloat");
ENDLOADFUNCS

#endif
