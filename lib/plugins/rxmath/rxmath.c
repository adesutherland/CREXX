//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // For POSIX systems (Linux/macOS)
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file
#include <math.h>

// some internal macros for the RXMATH library, can remain in coding
#define STRINGIFY(x) #x      //   stringify
#define ADDMATH(func) ADDPROC(x##func,STRINGIFY(rxmath.func),"b",".float","argin=.float");
#define MATHPROC(func) PROCEDURE(x##func) { \
            RETURNFLOAT(func(GETFLOAT(ARG0))) ; \
            PROCRETURN \
        ENDPROC}
/* --------------------------------------------------------------------------------------------
 * some Mathematical functions
 * * --------------------------------------------------------------------------------------------
 */
// RXMATH function code
    MATHPROC(acos)
    MATHPROC(asin)
    MATHPROC(atan)
    MATHPROC(cos )
    MATHPROC(cosh)
    MATHPROC(exp )
    MATHPROC(exp2)
    MATHPROC(log )
    MATHPROC(log2)
    MATHPROC(log10)
    MATHPROC(ceil)
    MATHPROC(floor)
    MATHPROC(fabs)
    MATHPROC(round)
    MATHPROC(trunc)

    PROCEDURE(xfmod) {  //  ADDMATH uses functionX to distinguish plugin function name from math function name
    RETURNFLOAT( fmod(GETFLOAT(ARG0), GETFLOAT(ARG1)));
    PROCRETURN
    ENDPROC}

    MATHPROC(sin )
    MATHPROC(sinh)
    MATHPROC(sqrt)
    MATHPROC(tan )
    MATHPROC(tanh)
    MATHPROC(erf)
    MATHPROC(erfc)
    MATHPROC(tgamma)
    MATHPROC(lgamma)
    MATHPROC(asinh)
    MATHPROC(acosh)
    MATHPROC(atanh)

PROCEDURE(xpow) {  //  ADDMATH uses functionX to distinguish plugin function name from math function name
    RETURNFLOAT( pow(GETFLOAT(ARG0), GETFLOAT(ARG1)));
    PROCRETURN
ENDPROC}
PROCEDURE(xpow10) {  //  ADDMATH uses functionX to distinguish plugin function name from math function name
    RETURNFLOAT( pow(10.0, (GETFLOAT(ARG0))));
    PROCRETURN
    ENDPROC}

PROCEDURE(xhypot) {  //  ADDMATH uses functionX to distinguish plugin function name from math function name
    RETURNFLOAT(hypot(GETFLOAT(ARG0),GETFLOAT(ARG0)));
    PROCRETURN
    ENDPROC}

PROCEDURE(pi) {
    RETURNFLOAT(M_PI);
    PROCRETURN
ENDPROC
}
PROCEDURE(euler) {
    RETURNFLOAT(M_E);
    PROCRETURN
    ENDPROC
}
// RXMATH function definitions
LOADFUNCS
    ADDMATH(acos)
    ADDMATH(asin)
    ADDMATH(atan)
    ADDMATH(cos)
    ADDMATH(cosh)
    ADDMATH(exp)
    ADDMATH(exp2)
    ADDMATH(log)
    ADDMATH(log2)
    ADDMATH(log10)
    ADDPROC(xpow,"rxmath.pow",   "b",  ".float", "arg1=.float,arg2=.float");
    ADDMATH(pow10)
    ADDMATH(ceil)
    ADDMATH(floor)
    ADDMATH(fabs)
    ADDMATH(round)
    ADDMATH(trunc)
    ADDPROC(xfmod,"rxmath.fmod",   "b",  ".float", "arg1=.float,arg2=.float");
    ADDMATH(sin)
    ADDMATH(sinh)
    ADDMATH(sqrt)
    ADDMATH(tan)
    ADDMATH(tanh)
    ADDMATH(erf)
    ADDMATH(erfc)
    ADDMATH(tgamma)
    ADDMATH(lgamma)
    ADDMATH(asinh)
    ADDMATH(acosh)
    ADDMATH(atanh)
    ADDPROC(xhypot,"rxmath.hypot",   "b",  ".float", "arg1=.float,arg2=.float");
    ADDPROC(pi,   "rxmath.pi",   "b",  ".float", "");
    ADDPROC(euler,"rxmath.euler","b",  ".float", "");
 ENDLOADFUNCS