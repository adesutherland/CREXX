/*
 * cREXX License (MIT)
 *
 * Exact vector arithmetic over host-native packed numeric owners.
 */

#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "crexxpa.h"
#include "rxvector_kernel.h"

#ifndef RXVECTOR_KERNEL_ONLY
RXPA_PLUGIN_PROCESS_REENTRANT
#endif

typedef struct rxvector_sum {
    double sum;
    double correction;
} rxvector_sum;

static double rxvector_float_at(const rxvector_float_span *span, size_t index)
{
    double value;

    memcpy(&value, span->data + index * sizeof(value), sizeof(value));
    return value;
}

static int64_t rxvector_int_at(const rxvector_int_span *span, size_t index)
{
    int64_t value;

    memcpy(&value, span->data + index * sizeof(value), sizeof(value));
    return value;
}

static void rxvector_sum_reset(rxvector_sum *accumulator)
{
    accumulator->sum = 0.0;
    accumulator->correction = 0.0;
}

static int rxvector_sum_add(rxvector_sum *accumulator, double value)
{
    double combined = accumulator->sum + value;
    double residual;
    double correction;

    if (!isfinite(combined)) return 0;
    if (fabs(accumulator->sum) >= fabs(value))
        residual = (accumulator->sum - combined) + value;
    else
        residual = (value - combined) + accumulator->sum;
    correction = accumulator->correction + residual;
    if (!isfinite(residual) || !isfinite(correction)) return 0;
    accumulator->sum = combined;
    accumulator->correction = correction;
    return 1;
}

static int rxvector_sum_value(const rxvector_sum *accumulator, double *value)
{
    *value = accumulator->sum + accumulator->correction;
    return isfinite(*value);
}

static rxvector_status rxvector_scale(const rxvector_float_span *values,
                                      size_t start, size_t count,
                                      double *scale)
{
    size_t index;

    *scale = 0.0;
    for (index = 0u; index < count; ++index) {
        double value = rxvector_float_at(values, start + index);
        double magnitude;

        if (!isfinite(value)) return RXVECTOR_NONFINITE_INPUT;
        magnitude = fabs(value);
        if (magnitude > *scale) *scale = magnitude;
    }
    return *scale == 0.0 ? RXVECTOR_ZERO_NORM : RXVECTOR_OK;
}

static rxvector_status rxvector_scaled_cosine(
        const rxvector_float_span *left, size_t left_start, double left_scale,
        const rxvector_float_span *right, size_t right_start,
        double right_scale, size_t count, double *score)
{
    size_t index;
    rxvector_sum dot;
    rxvector_sum left_squares;
    rxvector_sum right_squares;
    double dot_value;
    double left_square_value;
    double right_square_value;
    double denominator;

    rxvector_sum_reset(&dot);
    rxvector_sum_reset(&left_squares);
    rxvector_sum_reset(&right_squares);
    for (index = 0u; index < count; ++index) {
        double left_value =
                rxvector_float_at(left, left_start + index) / left_scale;
        double right_value =
                rxvector_float_at(right, right_start + index) / right_scale;

        if (!rxvector_sum_add(&dot, left_value * right_value) ||
            !rxvector_sum_add(&left_squares, left_value * left_value) ||
            !rxvector_sum_add(&right_squares, right_value * right_value))
            return RXVECTOR_NUMERIC_OVERFLOW;
    }
    if (!rxvector_sum_value(&dot, &dot_value) ||
        !rxvector_sum_value(&left_squares, &left_square_value) ||
        !rxvector_sum_value(&right_squares, &right_square_value))
        return RXVECTOR_NUMERIC_OVERFLOW;
    denominator = sqrt(left_square_value) * sqrt(right_square_value);
    if (!isfinite(denominator) || denominator == 0.0)
        return RXVECTOR_NUMERIC_OVERFLOW;
    *score = dot_value / denominator;
    if (!isfinite(*score)) return RXVECTOR_NUMERIC_OVERFLOW;
    if (*score > 1.0) *score = 1.0;
    else if (*score < -1.0) *score = -1.0;
    return RXVECTOR_OK;
}

/*
 * Normal finite embedding data normally needs one pass. Accumulate bounded
 * blocks directly and compensate when combining the block totals. If raw
 * products, squares, or norms overflow/underflow, replay through the scaled
 * path above; finite extreme inputs therefore retain the full contract.
 */
static rxvector_status rxvector_block_cosine(
        const rxvector_float_span *left, size_t left_start,
        const rxvector_float_span *right, size_t right_start,
        size_t count, double *score)
{
    const size_t block_size = 64u;
    size_t block_start = 0u;
    double left_scale = 0.0;
    double right_scale = 0.0;
    int raw_valid = 1;
    rxvector_sum dot;
    rxvector_sum left_squares;
    rxvector_sum right_squares;
    double dot_value = 0.0;
    double left_square_value = 0.0;
    double right_square_value = 0.0;

    rxvector_sum_reset(&dot);
    rxvector_sum_reset(&left_squares);
    rxvector_sum_reset(&right_squares);
    while (block_start < count) {
        size_t block_end = block_start + block_size;
        size_t index;
        double block_dot = 0.0;
        double block_left_squares = 0.0;
        double block_right_squares = 0.0;

        if (block_end > count) block_end = count;
        for (index = block_start; index < block_end; ++index) {
            double left_value = rxvector_float_at(left, left_start + index);
            double right_value = rxvector_float_at(right, right_start + index);
            double left_magnitude;
            double right_magnitude;

            if (!isfinite(left_value) || !isfinite(right_value))
                return RXVECTOR_NONFINITE_INPUT;
            left_magnitude = fabs(left_value);
            right_magnitude = fabs(right_value);
            if (left_magnitude > left_scale) left_scale = left_magnitude;
            if (right_magnitude > right_scale) right_scale = right_magnitude;
            block_dot += left_value * right_value;
            block_left_squares += left_value * left_value;
            block_right_squares += right_value * right_value;
        }
        if (!isfinite(block_dot) || !isfinite(block_left_squares) ||
            !isfinite(block_right_squares) ||
            !rxvector_sum_add(&dot, block_dot) ||
            !rxvector_sum_add(&left_squares, block_left_squares) ||
            !rxvector_sum_add(&right_squares, block_right_squares)) {
            raw_valid = 0;
        }
        block_start = block_end;
    }
    if (left_scale == 0.0 || right_scale == 0.0)
        return RXVECTOR_ZERO_NORM;
    if (raw_valid &&
        rxvector_sum_value(&dot, &dot_value) &&
        rxvector_sum_value(&left_squares, &left_square_value) &&
        rxvector_sum_value(&right_squares, &right_square_value) &&
        left_square_value > 0.0 && right_square_value > 0.0) {
        double denominator =
                sqrt(left_square_value) * sqrt(right_square_value);
        if (isfinite(denominator) && denominator > 0.0) {
            *score = dot_value / denominator;
            if (isfinite(*score)) {
                if (*score > 1.0) *score = 1.0;
                else if (*score < -1.0) *score = -1.0;
                return RXVECTOR_OK;
            }
        }
    }
    return rxvector_scaled_cosine(left, left_start, left_scale,
                                  right, right_start, right_scale,
                                  count, score);
}

rxvector_status rxvector_cosine_kernel(const rxvector_float_span *left,
                                       const rxvector_float_span *right,
                                       double *score)
{
    double left_scale;
    double right_scale;
    rxvector_status status;

    if (left->count != right->count) return RXVECTOR_UNEQUAL_LENGTH;
    if (left->count == 0u) return RXVECTOR_EMPTY_VECTOR;
    status = rxvector_scale(left, 0u, left->count, &left_scale);
    if (status != RXVECTOR_OK) return status;
    status = rxvector_scale(right, 0u, right->count, &right_scale);
    if (status != RXVECTOR_OK) return status;
    return rxvector_block_cosine(left, 0u, right, 0u,
                                 left->count, score);
}

static int rxvector_hit_better(const rxvector_hit *left,
                               const rxvector_hit *right)
{
    if (left->score != right->score) return left->score > right->score;
    if (left->identity != right->identity)
        return left->identity < right->identity;
    return left->row < right->row;
}

static int rxvector_hit_worse(const rxvector_hit *left,
                              const rxvector_hit *right)
{
    return rxvector_hit_better(right, left);
}

static void rxvector_hit_swap(rxvector_hit *left, rxvector_hit *right)
{
    rxvector_hit temporary = *left;
    *left = *right;
    *right = temporary;
}

static void rxvector_heap_push(rxvector_hit *hits, size_t index)
{
    while (index > 0u) {
        size_t parent = (index - 1u) / 2u;
        if (!rxvector_hit_worse(&hits[index], &hits[parent])) break;
        rxvector_hit_swap(&hits[index], &hits[parent]);
        index = parent;
    }
}

static void rxvector_heap_restore(rxvector_hit *hits, size_t count)
{
    size_t index = 0u;

    for (;;) {
        size_t left = index * 2u + 1u;
        size_t right = left + 1u;
        size_t worse = index;

        if (left < count && rxvector_hit_worse(&hits[left], &hits[worse]))
            worse = left;
        if (right < count && rxvector_hit_worse(&hits[right], &hits[worse]))
            worse = right;
        if (worse == index) break;
        rxvector_hit_swap(&hits[index], &hits[worse]);
        index = worse;
    }
}

static int rxvector_hit_compare(const void *left, const void *right)
{
    const rxvector_hit *left_hit = (const rxvector_hit *)left;
    const rxvector_hit *right_hit = (const rxvector_hit *)right;

    if (rxvector_hit_better(left_hit, right_hit)) return -1;
    if (rxvector_hit_better(right_hit, left_hit)) return 1;
    return 0;
}

rxvector_status rxvector_topk_kernel(const rxvector_float_span *vectors,
                                     const rxvector_int_span *identities,
                                     size_t dimensions,
                                     const rxvector_float_span *query,
                                     size_t requested,
                                     rxvector_hit *hits)
{
    size_t rows;
    size_t row;
    size_t retained = 0u;
    double query_scale;
    rxvector_status status;

    if (dimensions == 0u || vectors->count % dimensions != 0u)
        return RXVECTOR_INVALID_PAYLOAD;
    rows = vectors->count / dimensions;
    if (rows != identities->count || query->count != dimensions)
        return RXVECTOR_UNEQUAL_LENGTH;
    if (rows == 0u || query->count == 0u) return RXVECTOR_EMPTY_VECTOR;
    if (requested > rows) return RXVECTOR_INVALID_PAYLOAD;
    status = rxvector_scale(query, 0u, query->count, &query_scale);
    if (status != RXVECTOR_OK) return status;

    for (row = 0u; row < rows; ++row) {
        rxvector_hit candidate;

        status = rxvector_block_cosine(vectors, row * dimensions,
                                       query, 0u, dimensions,
                                       &candidate.score);
        if (status != RXVECTOR_OK) return status;
        candidate.identity = rxvector_int_at(identities, row);
        candidate.row = row;
        if (retained < requested) {
            hits[retained] = candidate;
            rxvector_heap_push(hits, retained);
            ++retained;
        } else if (requested > 0u &&
                   rxvector_hit_better(&candidate, &hits[0])) {
            hits[0] = candidate;
            rxvector_heap_restore(hits, retained);
        }
    }
    if (requested > 1u)
        qsort(hits, requested, sizeof(*hits), rxvector_hit_compare);
    return RXVECTOR_OK;
}

#ifndef RXVECTOR_KERNEL_ONLY
static int rxvector_open_float_span(rxpa_attribute_value value,
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

static int rxvector_open_int_span(rxpa_attribute_value value,
                                  rxvector_int_span *span)
{
    const void *payload;
    size_t length = 0u;

    payload = GETNATIVEPAYLOAD(value, &length, NULL, NULL);
    if (sizeof(rxinteger) != sizeof(int64_t)) return 0;
    if ((!payload && length != 0u) || length % sizeof(rxinteger) != 0u)
        return 0;
    span->data = (const unsigned char *)payload;
    span->count = length / sizeof(rxinteger);
    return 1;
}

static uint32_t rxvector_read_u32le(const unsigned char *data)
{
    return (uint32_t)data[0] |
           ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) |
           ((uint32_t)data[3] << 24);
}

static void rxvector_write_u32le(unsigned char *data, uint32_t value)
{
    data[0] = (unsigned char)value;
    data[1] = (unsigned char)(value >> 8);
    data[2] = (unsigned char)(value >> 16);
    data[3] = (unsigned char)(value >> 24);
}

PROCEDURE(decodef32le)
{
    const unsigned char *data;
    const void *payload;
    size_t length = 0u;
    size_t count;
    size_t index;
    double *values;

    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXVECTOR.DECODEF32LE expects one binary value")
    payload = GETNATIVEPAYLOAD(ARG0, &length, NULL, NULL);
    if ((!payload && length != 0u) || length % 4u != 0u)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXVECTOR.DECODEF32LE requires complete f32le items")
    count = length / 4u;
    if (count > SIZE_MAX / sizeof(*values))
        RETURNSIGNAL(SIGNAL_OUT_OF_RANGE,
                     "RXVECTOR.DECODEF32LE result is too large")
    values = count ? (double *)malloc(count * sizeof(*values)) : NULL;
    if (count && !values)
        RETURNSIGNAL(SIGNAL_FAILURE,
                     "RXVECTOR.DECODEF32LE could not allocate its result")
    data = (const unsigned char *)payload;
    for (index = 0u; index < count; ++index) {
        uint32_t bits = rxvector_read_u32le(data + index * 4u);
        float narrow;

        memcpy(&narrow, &bits, sizeof(narrow));
        if (!isfinite(narrow)) {
            free(values);
            RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                         "RXVECTOR.DECODEF32LE requires finite values")
        }
        values[index] = (double)narrow;
    }
    if (SETNATIVEPAYLOAD(RETURN, values, count * sizeof(*values),
                         NULL, 0u) != 0) {
        free(values);
        RETURNSIGNAL(SIGNAL_FAILURE,
                     "RXVECTOR.DECODEF32LE could not allocate its result")
    }
    free(values);
    RESETSIGNAL
}

PROCEDURE(encodef32le)
{
    rxvector_float_span values;
    unsigned char *data;
    size_t index;

    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXVECTOR.ENCODEF32LE expects one packedfloat")
    if (!ISINITIALIZED(ARG0))
        RETURNSIGNAL(SIGNAL_OBJECT_NOT_INITIALIZED,
                     "RXVECTOR.ENCODEF32LE received an uninitialized packedfloat")
    if (!rxvector_open_float_span(ARG0, &values) ||
        values.count > SIZE_MAX / 4u)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXVECTOR.ENCODEF32LE received an invalid packedfloat")
    data = values.count ? (unsigned char *)malloc(values.count * 4u) : NULL;
    if (values.count && !data)
        RETURNSIGNAL(SIGNAL_FAILURE,
                     "RXVECTOR.ENCODEF32LE could not allocate its result")
    for (index = 0u; index < values.count; ++index) {
        double wide = rxvector_float_at(&values, index);
        float narrow;
        uint32_t bits;

        if (!isfinite(wide)) {
            free(data);
            RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                         "RXVECTOR.ENCODEF32LE requires finite values")
        }
        narrow = (float)wide;
        if (!isfinite(narrow) || (wide != 0.0 && narrow == 0.0f)) {
            free(data);
            RETURNSIGNAL(SIGNAL_OVERFLOW_UNDERFLOW,
                         "RXVECTOR.ENCODEF32LE value is outside the f32 range")
        }
        memcpy(&bits, &narrow, sizeof(bits));
        rxvector_write_u32le(data + index * 4u, bits);
    }
    if (SETNATIVEPAYLOAD(RETURN, data, values.count * 4u, NULL, 0u) != 0) {
        free(data);
        RETURNSIGNAL(SIGNAL_FAILURE,
                     "RXVECTOR.ENCODEF32LE could not allocate its result")
    }
    free(data);
    RESETSIGNAL
}

PROCEDURE(cosine)
{
    rxvector_float_span left;
    rxvector_float_span right;
    rxvector_status status;
    double score;

    if (NUM_ARGS != 2)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXVECTOR.COSINE expects two packedfloat owners")
    if (!ISINITIALIZED(ARG0) || !ISINITIALIZED(ARG1))
        RETURNSIGNAL(SIGNAL_OBJECT_NOT_INITIALIZED,
                     "RXVECTOR.COSINE received an uninitialized packedfloat")
    if (!rxvector_open_float_span(ARG0, &left) ||
        !rxvector_open_float_span(ARG1, &right))
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXVECTOR.COSINE received an invalid packedfloat")
    status = rxvector_cosine_kernel(&left, &right, &score);
    if (status == RXVECTOR_UNEQUAL_LENGTH || status == RXVECTOR_EMPTY_VECTOR)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXVECTOR.COSINE requires equal nonempty inputs")
    if (status == RXVECTOR_NONFINITE_INPUT)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXVECTOR.COSINE requires finite values")
    if (status == RXVECTOR_ZERO_NORM)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXVECTOR.COSINE is undefined for a zero-norm input")
    if (status != RXVECTOR_OK)
        RETURNSIGNAL(SIGNAL_OVERFLOW_UNDERFLOW,
                     "RXVECTOR.COSINE result is outside the native float range")
    RETURNFLOAT(score);
    RESETSIGNAL
}

PROCEDURE(topkcosine)
{
    rxvector_float_span vectors;
    rxvector_int_span identities;
    rxvector_float_span query;
    rxvector_hit *hits = NULL;
    int64_t *result_identities = NULL;
    double *result_scores = NULL;
    rxinteger dimensions_value;
    rxinteger requested_value;
    size_t dimensions;
    size_t requested;
    size_t index;
    rxvector_status status;

    if (NUM_ARGS != 7)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXVECTOR.TOPKCOSINE expects seven arguments")
    if (!ISINITIALIZED(ARG5) || !ISINITIALIZED(ARG6))
        RETURNSIGNAL(SIGNAL_OBJECT_NOT_INITIALIZED,
                     "RXVECTOR.TOPKCOSINE requires initialized result owners")
    if (SETNATIVEPAYLOAD(ARG5, NULL, 0u, NULL, 0u) != 0 ||
        SETNATIVEPAYLOAD(ARG6, NULL, 0u, NULL, 0u) != 0)
        RETURNSIGNAL(SIGNAL_FAILURE,
                     "RXVECTOR.TOPKCOSINE could not reset its results")
    if (!ISINITIALIZED(ARG0) || !ISINITIALIZED(ARG1) ||
        !ISINITIALIZED(ARG3))
        RETURNSIGNAL(SIGNAL_OBJECT_NOT_INITIALIZED,
                     "RXVECTOR.TOPKCOSINE received an uninitialized input owner")
    if (!rxvector_open_float_span(ARG0, &vectors) ||
        !rxvector_open_int_span(ARG1, &identities) ||
        !rxvector_open_float_span(ARG3, &query))
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXVECTOR.TOPKCOSINE received an invalid packed owner")
    dimensions_value = GETINT(ARG2);
    requested_value = GETINT(ARG4);
    if (dimensions_value <= 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXVECTOR.TOPKCOSINE requires a positive dimension")
    if (requested_value < 0)
        RETURNSIGNAL(SIGNAL_OUT_OF_RANGE,
                     "RXVECTOR.TOPKCOSINE requested count is out of range")
    dimensions = (size_t)dimensions_value;
    requested = (size_t)requested_value;
    if ((rxinteger)dimensions != dimensions_value ||
        (rxinteger)requested != requested_value)
        RETURNSIGNAL(SIGNAL_OUT_OF_RANGE,
                     "RXVECTOR.TOPKCOSINE count is out of range")
    if (dimensions == 0u || vectors.count % dimensions != 0u ||
        vectors.count / dimensions != identities.count ||
        query.count != dimensions)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXVECTOR.TOPKCOSINE matrix, identities and query shapes disagree")
    if (requested > identities.count)
        RETURNSIGNAL(SIGNAL_OUT_OF_RANGE,
                     "RXVECTOR.TOPKCOSINE requested count exceeds the row count")
    if (identities.count == 0u)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXVECTOR.TOPKCOSINE requires at least one row")
    if (requested == 0u) {
        RESETSIGNAL
        return;
    }
    if (requested > SIZE_MAX / sizeof(*hits))
        RETURNSIGNAL(SIGNAL_OUT_OF_RANGE,
                     "RXVECTOR.TOPKCOSINE result is too large")
    hits = (rxvector_hit *)malloc(requested * sizeof(*hits));
    result_identities = (int64_t *)malloc(requested * sizeof(*result_identities));
    result_scores = (double *)malloc(requested * sizeof(*result_scores));
    if (!hits || !result_identities || !result_scores) {
        free(result_scores);
        free(result_identities);
        free(hits);
        RETURNSIGNAL(SIGNAL_FAILURE,
                     "RXVECTOR.TOPKCOSINE could not allocate its results")
    }
    status = rxvector_topk_kernel(&vectors, &identities, dimensions, &query,
                                  requested, hits);
    if (status != RXVECTOR_OK) {
        free(result_scores);
        free(result_identities);
        free(hits);
        if (status == RXVECTOR_NONFINITE_INPUT)
            RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                         "RXVECTOR.TOPKCOSINE requires finite values")
        if (status == RXVECTOR_ZERO_NORM)
            RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                         "RXVECTOR.TOPKCOSINE is undefined for a zero-norm vector")
        if (status == RXVECTOR_ALLOCATION_FAILED)
            RETURNSIGNAL(SIGNAL_FAILURE,
                         "RXVECTOR.TOPKCOSINE could not allocate working memory")
        if (status == RXVECTOR_NUMERIC_OVERFLOW)
            RETURNSIGNAL(SIGNAL_OVERFLOW_UNDERFLOW,
                         "RXVECTOR.TOPKCOSINE result is outside the native float range")
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXVECTOR.TOPKCOSINE received incompatible inputs")
    }
    for (index = 0u; index < requested; ++index) {
        result_identities[index] = hits[index].identity;
        result_scores[index] = hits[index].score;
    }
    free(hits);
    if (SETNATIVEPAYLOAD(ARG5, result_identities,
                         requested * sizeof(*result_identities),
                         NULL, 0u) != 0 ||
        SETNATIVEPAYLOAD(ARG6, result_scores,
                         requested * sizeof(*result_scores),
                         NULL, 0u) != 0) {
        SETNATIVEPAYLOAD(ARG5, NULL, 0u, NULL, 0u);
        SETNATIVEPAYLOAD(ARG6, NULL, 0u, NULL, 0u);
        free(result_scores);
        free(result_identities);
        RETURNSIGNAL(SIGNAL_FAILURE,
                     "RXVECTOR.TOPKCOSINE could not publish its results")
    }
    free(result_scores);
    free(result_identities);
    RESETSIGNAL
}

LOADFUNCS
    ADDPROC(decodef32le, "rxvector.decodef32le", "b", ".rxfnsg..packedfloat",
            "data = .binary");
    ADDPROC(encodef32le, "rxvector.encodef32le", "b", ".binary",
            "values = .packedfloat");
    ADDPROC(cosine, "rxvector.cosine", "b", ".float",
            "left = .packedfloat,right = .packedfloat");
    ADDPROC(topkcosine, "rxvector.topkcosine", "b", ".void",
            "vectors = .packedfloat,identities = .packedint,dimensions = .int,query_vector = .packedfloat,requested = .int,expose result_identities = .packedint,expose result_scores = .packedfloat");
ENDLOADFUNCS

#endif
