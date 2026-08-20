#!/usr/bin/env python3
"""Offline arbitrary-precision oracle for RCC-5C integer boundary vectors."""

import math


INT64_MIN = -(2**63)
INT64_MAX = 2**63 - 1
LARGE_MODULUS = 9223372036854775783


def emit(label: str, value: int) -> None:
    print(f"{label}: {value}")


emit("gcd consecutive Fibonacci", math.gcd(7540113804746346429,
                                             4660046610375530309))
emit("lcm adjacent representable boundary", math.lcm(3037000499, 3037000500))
emit("lcm adjacent overflow boundary", math.lcm(3037000500, 3037000501))
emit("isqrt maximum root squared", math.isqrt(9223372030926249001))
emit("isqrt integer maximum", math.isqrt(INT64_MAX))
emit("powmod minimum base", pow(INT64_MIN, 2, LARGE_MODULUS))
emit("powmod maximum exponent", pow(INT64_MAX, INT64_MAX, LARGE_MODULUS))
emit("powmod negative normalization", pow(INT64_MIN, 123456789, 2147483647))
emit("powmod repeated doubling boundary",
     pow(3037000500, 3037000500, LARGE_MODULUS))

for value in range(21):
    emit(f"factorial {value}", math.factorial(value))
