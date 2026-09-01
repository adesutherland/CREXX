/*
 * cREXX License (MIT)
 *
 * Scalar native binary-float mathematics supplied by rxfloat.
 */

#include <math.h>

#include "crexxpa.h"

RXPA_PLUGIN_PROCESS_REENTRANT

#define RXFLOAT_UNARY_PROCEDURE(name, operation) \
    PROCEDURE(name) \
    { \
        if (NUM_ARGS != 1) \
            RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected") \
        RETURNFLOAT(operation(GETFLOAT(ARG(0)))); \
        RESETSIGNAL \
    }

#define RXFLOAT_BINARY_PROCEDURE(name, operation) \
    PROCEDURE(name) \
    { \
        if (NUM_ARGS != 2) \
            RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected") \
        RETURNFLOAT(operation(GETFLOAT(ARG(0)), GETFLOAT(ARG(1)))); \
        RESETSIGNAL \
    }

RXFLOAT_UNARY_PROCEDURE(xacos, acos)
RXFLOAT_UNARY_PROCEDURE(xasin, asin)
RXFLOAT_UNARY_PROCEDURE(xatan, atan)
RXFLOAT_UNARY_PROCEDURE(xcos, cos)
RXFLOAT_UNARY_PROCEDURE(xcosh, cosh)
RXFLOAT_UNARY_PROCEDURE(xexp, exp)
RXFLOAT_UNARY_PROCEDURE(xexp2, exp2)
RXFLOAT_UNARY_PROCEDURE(xexpm1, expm1)
RXFLOAT_UNARY_PROCEDURE(xlog, log)
RXFLOAT_UNARY_PROCEDURE(xlog2, log2)
RXFLOAT_UNARY_PROCEDURE(xlog10, log10)
RXFLOAT_UNARY_PROCEDURE(xlog1p, log1p)
RXFLOAT_UNARY_PROCEDURE(xceil, ceil)
RXFLOAT_UNARY_PROCEDURE(xfloor, floor)
RXFLOAT_UNARY_PROCEDURE(xfabs, fabs)
RXFLOAT_UNARY_PROCEDURE(xround, round)
RXFLOAT_UNARY_PROCEDURE(xtrunc, trunc)
RXFLOAT_UNARY_PROCEDURE(xsin, sin)
RXFLOAT_UNARY_PROCEDURE(xsinh, sinh)
RXFLOAT_UNARY_PROCEDURE(xsqrt, sqrt)
RXFLOAT_UNARY_PROCEDURE(xcbrt, cbrt)
RXFLOAT_UNARY_PROCEDURE(xtan, tan)
RXFLOAT_UNARY_PROCEDURE(xtanh, tanh)
RXFLOAT_UNARY_PROCEDURE(xerf, erf)
RXFLOAT_UNARY_PROCEDURE(xerfc, erfc)
RXFLOAT_UNARY_PROCEDURE(xtgamma, tgamma)
RXFLOAT_UNARY_PROCEDURE(xlgamma, lgamma)
RXFLOAT_UNARY_PROCEDURE(xasinh, asinh)
RXFLOAT_UNARY_PROCEDURE(xacosh, acosh)
RXFLOAT_UNARY_PROCEDURE(xatanh, atanh)

RXFLOAT_BINARY_PROCEDURE(xpow, pow)
RXFLOAT_BINARY_PROCEDURE(xfmod, fmod)
RXFLOAT_BINARY_PROCEDURE(xhypot, hypot)
RXFLOAT_BINARY_PROCEDURE(xatan2, atan2)

PROCEDURE(xpow10)
{
    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")
    RETURNFLOAT(pow(10.0, GETFLOAT(ARG(0))));
    RESETSIGNAL
}

PROCEDURE(xpi)
{
    if (NUM_ARGS != 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "no arguments expected")
    RETURNFLOAT(acos(-1.0));
    RESETSIGNAL
}

PROCEDURE(xeuler)
{
    if (NUM_ARGS != 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "no arguments expected")
    RETURNFLOAT(exp(1.0));
    RESETSIGNAL
}

#define RXFLOAT_ADD_UNARY(name, proc) \
    ADDPROC(proc, "rxfloat." #name, "b", ".float", "value = .float"); \
    ADDPROC(proc, "rxmath." #name, "b", ".float", "value = .float");

#define RXFLOAT_ADD_BINARY(name, proc) \
    ADDPROC(proc, "rxfloat." #name, "b", ".float", \
            "left = .float, right = .float"); \
    ADDPROC(proc, "rxmath." #name, "b", ".float", \
            "left = .float, right = .float");

#define RXFLOAT_ADD_NULLARY(name, proc) \
    ADDPROC(proc, "rxfloat." #name, "b", ".float", ""); \
    ADDPROC(proc, "rxmath." #name, "b", ".float", "");

LOADFUNCS
RXFLOAT_ADD_UNARY(acos, xacos)
RXFLOAT_ADD_UNARY(asin, xasin)
RXFLOAT_ADD_UNARY(atan, xatan)
RXFLOAT_ADD_UNARY(cos, xcos)
RXFLOAT_ADD_UNARY(cosh, xcosh)
RXFLOAT_ADD_UNARY(exp, xexp)
RXFLOAT_ADD_UNARY(exp2, xexp2)
RXFLOAT_ADD_UNARY(expm1, xexpm1)
RXFLOAT_ADD_UNARY(log, xlog)
RXFLOAT_ADD_UNARY(log2, xlog2)
RXFLOAT_ADD_UNARY(log10, xlog10)
RXFLOAT_ADD_UNARY(log1p, xlog1p)
RXFLOAT_ADD_UNARY(ceil, xceil)
RXFLOAT_ADD_UNARY(floor, xfloor)
RXFLOAT_ADD_UNARY(fabs, xfabs)
RXFLOAT_ADD_UNARY(round, xround)
RXFLOAT_ADD_UNARY(trunc, xtrunc)
RXFLOAT_ADD_UNARY(sin, xsin)
RXFLOAT_ADD_UNARY(sinh, xsinh)
RXFLOAT_ADD_UNARY(sqrt, xsqrt)
RXFLOAT_ADD_UNARY(cbrt, xcbrt)
RXFLOAT_ADD_UNARY(tan, xtan)
RXFLOAT_ADD_UNARY(tanh, xtanh)
RXFLOAT_ADD_UNARY(erf, xerf)
RXFLOAT_ADD_UNARY(erfc, xerfc)
RXFLOAT_ADD_UNARY(tgamma, xtgamma)
RXFLOAT_ADD_UNARY(lgamma, xlgamma)
RXFLOAT_ADD_UNARY(asinh, xasinh)
RXFLOAT_ADD_UNARY(acosh, xacosh)
RXFLOAT_ADD_UNARY(atanh, xatanh)
RXFLOAT_ADD_BINARY(pow, xpow)
RXFLOAT_ADD_BINARY(fmod, xfmod)
RXFLOAT_ADD_BINARY(hypot, xhypot)
RXFLOAT_ADD_BINARY(atan2, xatan2)
RXFLOAT_ADD_UNARY(pow10, xpow10)
RXFLOAT_ADD_NULLARY(pi, xpi)
RXFLOAT_ADD_NULLARY(euler, xeuler)
ENDLOADFUNCS
