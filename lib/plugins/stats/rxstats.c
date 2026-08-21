/*
 * cREXX License (MIT)
 *
 * Transitional boxed-array statistics supplied by the Level-G rxstats
 * provider. RCC-5F replaces this call shape with packed typed storage.
 */

#include <math.h>
#include <float.h>

#include "crexxpa.h"

RXPA_PLUGIN_PROCESS_REENTRANT

typedef struct rxstats_moments {
    rxinteger count;
    double mean;
    double m2;
} rxstats_moments;

typedef struct rxstats_pair_moments {
    rxinteger count;
    double mean_x;
    double mean_y;
    double m2_x;
    double m2_y;
    double co_moment;
} rxstats_pair_moments;

typedef struct rxstats_compensated_sum {
    double sum;
    double correction;
} rxstats_compensated_sum;

static void rxstats_sum_reset(rxstats_compensated_sum *accumulator)
{
    accumulator->sum = 0.0;
    accumulator->correction = 0.0;
}

/* Neumaier compensation retains a low-order residual when magnitudes differ. */
static void rxstats_sum_add(rxstats_compensated_sum *accumulator, double value)
{
    double combined = accumulator->sum + value;

    if (fabs(accumulator->sum) >= fabs(value))
        accumulator->correction += (accumulator->sum - combined) + value;
    else
        accumulator->correction += (value - combined) + accumulator->sum;
    accumulator->sum = combined;
}

static double rxstats_sum_value(const rxstats_compensated_sum *accumulator)
{
    return accumulator->sum + accumulator->correction;
}

/*
 * A shifted single pass normally avoids cancellation.  If its final central
 * moment is tiny compared with the shifted square sum, recompute about the
 * already-known mean rather than returning a cancellation artifact.
 */
static int rxstats_needs_second_pass(double central_moment,
                                     double shifted_square_sum)
{
    if (central_moment < 0.0) return 1;
    if (shifted_square_sum == 0.0) return 0;
    return central_moment <= 64.0 * DBL_EPSILON * fabs(shifted_square_sum);
}

static double rxstats_second_moment(void *values, rxinteger count, double mean)
{
    rxinteger i;
    rxstats_compensated_sum squares;

    rxstats_sum_reset(&squares);
    for (i = 0; i < count; ++i) {
        double difference = GETFARRAY(values, i) - mean;
        rxstats_sum_add(&squares, difference * difference);
    }
    return rxstats_sum_value(&squares);
}

static void rxstats_accumulate(void *values, rxstats_moments *result)
{
    rxinteger i;
    double origin;
    double shifted_total;
    double shifted_square_total;
    rxstats_compensated_sum shifted;
    rxstats_compensated_sum shifted_squares;

    result->count = GETARRAYHI(values);
    result->mean = 0.0;
    result->m2 = 0.0;
    if (result->count == 0) return;

    origin = GETFARRAY(values, 0);
    rxstats_sum_reset(&shifted);
    rxstats_sum_reset(&shifted_squares);
    for (i = 1; i < result->count; ++i) {
        double difference = GETFARRAY(values, i) - origin;
        rxstats_sum_add(&shifted, difference);
        rxstats_sum_add(&shifted_squares, difference * difference);
    }
    shifted_total = rxstats_sum_value(&shifted);
    shifted_square_total = rxstats_sum_value(&shifted_squares);
    result->mean = origin + shifted_total / (double)result->count;
    result->m2 = shifted_square_total -
                 shifted_total * shifted_total / (double)result->count;
    if (rxstats_needs_second_pass(result->m2, shifted_square_total))
        result->m2 = rxstats_second_moment(values, result->count, result->mean);
}

static int rxstats_accumulate_pairs(void *x_values, void *y_values,
                                    rxstats_pair_moments *result)
{
    rxinteger i;
    rxinteger x_count = GETARRAYHI(x_values);
    rxinteger y_count = GETARRAYHI(y_values);
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

    if (x_count != y_count) return 0;
    result->count = x_count;
    result->mean_x = 0.0;
    result->mean_y = 0.0;
    result->m2_x = 0.0;
    result->m2_y = 0.0;
    result->co_moment = 0.0;
    if (x_count == 0) return 1;

    origin_x = GETFARRAY(x_values, 0);
    origin_y = GETFARRAY(y_values, 0);
    rxstats_sum_reset(&shifted_x);
    rxstats_sum_reset(&shifted_y);
    rxstats_sum_reset(&shifted_x_squares);
    rxstats_sum_reset(&shifted_y_squares);
    rxstats_sum_reset(&shifted_cross);
    for (i = 1; i < x_count; ++i) {
        double difference_x = GETFARRAY(x_values, i) - origin_x;
        double difference_y = GETFARRAY(y_values, i) - origin_y;
        rxstats_sum_add(&shifted_x, difference_x);
        rxstats_sum_add(&shifted_y, difference_y);
        rxstats_sum_add(&shifted_x_squares, difference_x * difference_x);
        rxstats_sum_add(&shifted_y_squares, difference_y * difference_y);
        rxstats_sum_add(&shifted_cross, difference_x * difference_y);
    }
    shifted_x_total = rxstats_sum_value(&shifted_x);
    shifted_y_total = rxstats_sum_value(&shifted_y);
    shifted_x_square_total = rxstats_sum_value(&shifted_x_squares);
    shifted_y_square_total = rxstats_sum_value(&shifted_y_squares);
    shifted_cross_total = rxstats_sum_value(&shifted_cross);
    result->mean_x = origin_x + shifted_x_total / (double)x_count;
    result->mean_y = origin_y + shifted_y_total / (double)y_count;
    result->m2_x = shifted_x_square_total -
                   shifted_x_total * shifted_x_total / (double)x_count;
    result->m2_y = shifted_y_square_total -
                   shifted_y_total * shifted_y_total / (double)y_count;
    result->co_moment = shifted_cross_total -
                        shifted_x_total * shifted_y_total / (double)x_count;

    if (rxstats_needs_second_pass(result->m2_x, shifted_x_square_total) ||
        rxstats_needs_second_pass(result->m2_y, shifted_y_square_total)) {
        rxstats_compensated_sum centered_x_squares;
        rxstats_compensated_sum centered_y_squares;
        rxstats_compensated_sum centered_cross;

        rxstats_sum_reset(&centered_x_squares);
        rxstats_sum_reset(&centered_y_squares);
        rxstats_sum_reset(&centered_cross);
        for (i = 0; i < x_count; ++i) {
            double difference_x = GETFARRAY(x_values, i) - result->mean_x;
            double difference_y = GETFARRAY(y_values, i) - result->mean_y;
            rxstats_sum_add(&centered_x_squares, difference_x * difference_x);
            rxstats_sum_add(&centered_y_squares, difference_y * difference_y);
            rxstats_sum_add(&centered_cross, difference_x * difference_y);
        }
        result->m2_x = rxstats_sum_value(&centered_x_squares);
        result->m2_y = rxstats_sum_value(&centered_y_squares);
        result->co_moment = rxstats_sum_value(&centered_cross);
    }
    return 1;
}

PROCEDURE(mean)
{
    rxstats_moments moments;
    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXSTATS.MEAN expects one array")
    rxstats_accumulate(ARG0, &moments);
    if (moments.count < 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXSTATS.MEAN requires at least one value")
    RETURNFLOAT(moments.mean);
    RESETSIGNAL
}

PROCEDURE(stddev)
{
    rxstats_moments moments;
    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXSTATS.STDDEV expects one array")
    rxstats_accumulate(ARG0, &moments);
    if (moments.count < 2)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXSTATS.STDDEV requires at least two values")
    RETURNFLOAT(sqrt(moments.m2 / (double)(moments.count - 1)));
    RESETSIGNAL
}

PROCEDURE(covariance)
{
    rxstats_pair_moments moments;
    if (NUM_ARGS != 2)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXSTATS.COVARIANCE expects two arrays")
    if (!rxstats_accumulate_pairs(ARG0, ARG1, &moments))
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXSTATS.COVARIANCE requires equal-length arrays")
    if (moments.count < 2)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXSTATS.COVARIANCE requires at least two pairs")
    RETURNFLOAT(moments.co_moment / (double)(moments.count - 1));
    RESETSIGNAL
}

PROCEDURE(correlation)
{
    rxstats_pair_moments moments;
    if (NUM_ARGS != 2)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXSTATS.CORRELATION expects two arrays")
    if (!rxstats_accumulate_pairs(ARG0, ARG1, &moments))
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXSTATS.CORRELATION requires equal-length arrays")
    if (moments.count < 2)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXSTATS.CORRELATION requires at least two pairs")
    if (moments.m2_x == 0.0 || moments.m2_y == 0.0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXSTATS.CORRELATION is undefined for zero variance")
    RETURNFLOAT(moments.co_moment / sqrt(moments.m2_x * moments.m2_y));
    RESETSIGNAL
}

PROCEDURE(regression)
{
    rxstats_pair_moments moments;
    double slope;
    double intercept;

    if (NUM_ARGS != 4)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXSTATS.REGRESSION expects two arrays, slope, and intercept")
    if (!rxstats_accumulate_pairs(ARG0, ARG1, &moments))
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXSTATS.REGRESSION requires equal-length arrays")
    if (moments.count < 2)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXSTATS.REGRESSION requires at least two pairs")
    if (moments.m2_x == 0.0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXSTATS.REGRESSION is undefined for zero x variance")

    slope = moments.co_moment / moments.m2_x;
    intercept = moments.mean_y - slope * moments.mean_x;
    SETFLOAT(ARG2, slope);
    SETFLOAT(ARG3, intercept);
    RETURNFLOAT(slope);
    RESETSIGNAL
}

LOADFUNCS
    ADDPROC(mean, "rxstats.mean", "b", ".float", "expose values = .float[]");
    ADDPROC(stddev, "rxstats.stddev", "b", ".float", "expose values = .float[]");
    ADDPROC(covariance, "rxstats.covariance", "b", ".float", "expose x = .float[],expose y = .float[]");
    ADDPROC(correlation, "rxstats.correlation", "b", ".float", "expose x = .float[],expose y = .float[]");
    ADDPROC(regression, "rxstats.regression", "b", ".float", "expose x = .float[],expose y = .float[],expose slope = .float,expose intercept = .float");
ENDLOADFUNCS
