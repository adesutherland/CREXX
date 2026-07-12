/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXINTEGER_H
#define CREXX_RXINTEGER_H

#include <inttypes.h>
#include <stdint.h>

/* Release 1 language and bytecode contract: .int is signed 64-bit. */
typedef int64_t rxinteger;

#define RXINTEGER_MIN INT64_MIN
#define RXINTEGER_MAX INT64_MAX
#define RXINTEGER_PRI PRId64

#if defined(__cplusplus) && __cplusplus >= 201103L
static_assert(sizeof(rxinteger) == 8, "cREXX .int must be 64-bit");
static_assert((rxinteger)-1 < 0, "cREXX .int must be signed");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(rxinteger) == 8, "cREXX .int must be 64-bit");
_Static_assert((rxinteger)-1 < 0, "cREXX .int must be signed");
#else
typedef char crexx_rxinteger_must_be_64_bits[(sizeof(rxinteger) == 8) ? 1 : -1];
typedef char crexx_rxinteger_must_be_signed[((rxinteger)-1 < 0) ? 1 : -1];
#endif

#if defined(_MSC_VER)
#define RXINTEGER_INLINE static __inline
#elif defined(__GNUC__) || defined(__clang__)
#define RXINTEGER_INLINE static __inline__
#else
#define RXINTEGER_INLINE static
#endif

#if defined(__has_builtin)
# if __has_builtin(__builtin_add_overflow) && \
     __has_builtin(__builtin_sub_overflow) && \
     __has_builtin(__builtin_mul_overflow)
#  define RXINTEGER_HAS_OVERFLOW_BUILTINS 1
# endif
#elif defined(__GNUC__)
# define RXINTEGER_HAS_OVERFLOW_BUILTINS 1
#endif
#ifndef RXINTEGER_HAS_OVERFLOW_BUILTINS
#define RXINTEGER_HAS_OVERFLOW_BUILTINS 0
#endif

RXINTEGER_INLINE int rxinteger_checked_add(rxinteger left,
                                           rxinteger right,
                                           rxinteger *result) {
#if RXINTEGER_HAS_OVERFLOW_BUILTINS
    return !__builtin_add_overflow(left, right, result);
#else
    if ((right > 0 && left > RXINTEGER_MAX - right) ||
        (right < 0 && left < RXINTEGER_MIN - right)) return 0;
    *result = left + right;
    return 1;
#endif
}

RXINTEGER_INLINE int rxinteger_checked_sub(rxinteger left,
                                           rxinteger right,
                                           rxinteger *result) {
#if RXINTEGER_HAS_OVERFLOW_BUILTINS
    return !__builtin_sub_overflow(left, right, result);
#else
    if ((right > 0 && left < RXINTEGER_MIN + right) ||
        (right < 0 && left > RXINTEGER_MAX + right)) return 0;
    *result = left - right;
    return 1;
#endif
}

RXINTEGER_INLINE uint64_t rxinteger_magnitude(rxinteger value) {
    if (value >= 0) return (uint64_t)value;
    return (uint64_t)(-(value + 1)) + UINT64_C(1);
}

RXINTEGER_INLINE int rxinteger_checked_mul(rxinteger left,
                                           rxinteger right,
                                           rxinteger *result) {
#if RXINTEGER_HAS_OVERFLOW_BUILTINS
    return !__builtin_mul_overflow(left, right, result);
#else
    uint64_t left_magnitude;
    uint64_t right_magnitude;
    uint64_t limit;
    uint64_t product;
    int negative;

    left_magnitude = rxinteger_magnitude(left);
    right_magnitude = rxinteger_magnitude(right);
    negative = (left < 0) != (right < 0);
    limit = negative ? (UINT64_C(1) << 63) : (uint64_t)RXINTEGER_MAX;

    if (left_magnitude == 0 || right_magnitude == 0) {
        *result = 0;
        return 1;
    }
    if (right_magnitude > limit / left_magnitude) return 0;
    product = left_magnitude * right_magnitude;
    if (negative) *result = -(rxinteger)(product - 1) - 1;
    else *result = (rxinteger)product;
    return 1;
#endif
}

RXINTEGER_INLINE int rxinteger_checked_neg(rxinteger value,
                                           rxinteger *result) {
    if (value == RXINTEGER_MIN) return 0;
    *result = -value;
    return 1;
}

RXINTEGER_INLINE int rxinteger_checked_pow(rxinteger base,
                                           rxinteger exponent,
                                           rxinteger *result) {
    rxinteger power;

    power = 1;
    if (exponent < 0) {
        if (base == 1) {
            *result = 1;
            return 1;
        }
        if (base == -1) {
            *result = (exponent & 1) ? -1 : 1;
            return 1;
        }
        *result = 0;
        return 0;
    }

    while (exponent > 0) {
        if ((exponent & 1) && !rxinteger_checked_mul(power, base, &power)) return 0;
        exponent >>= 1;
        if (exponent > 0 && !rxinteger_checked_mul(base, base, &base)) return 0;
    }

    *result = power;
    return 1;
}

/* Parse a decimal prefix. The caller decides whether trailing text is valid. */
int rxinteger_parse(const char *text, char **end, rxinteger *result);

#endif /* CREXX_RXINTEGER_H */
