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
    MATHPROC(log )
    MATHPROC(log10)
// MATHPROC(pow10)
    MATHPROC(sin )
    MATHPROC(sinh)
    MATHPROC(sqrt)
    MATHPROC(tan )
    MATHPROC(tanh)
// RXMATH function definitions
LOADFUNCS
    ADDMATH(acos)
    ADDMATH(asin)
    ADDMATH(atan)
    ADDMATH(cos )
    ADDMATH(cosh)
    ADDMATH(exp )
    ADDMATH(log )
    ADDMATH(log10)
  //  ADDMATH(pow10)
    ADDMATH(sin )
    ADDMATH(sinh)
    ADDMATH(sqrt)
    ADDMATH(tan )
    ADDMATH(tanh)
ENDLOADFUNCS