//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // For POSIX systems (Linux/macOS)
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file
#include <math.h>
#include <stdint.h>
// #include "windows.h"

/* --------------------------------------------------------------------------------------------
 * some internal macros for the RXMATH library, can remain in coding
 * these macros take into account that most math functions have the same structure
 * float in -> math function -> float out, the only difference is thea actual math function
 * to distinguish the rexx math function call and the base c math function the function in the plugin call has x prefix
 * --------------------------------------------------------------------------------------------
 */
#define STRINGIFY(x) #x      //   stringify
#define ADDMATH(func) ADDPROC(x##func,STRINGIFY(rxmath.func),"b",".float","argin=.float");
#define MATHPROC(func) PROCEDURE(x##func) { \
            RETURNFLOAT(func(GETFLOAT(ARG0))) ; \
            PROCRETURN \
        ENDPROC}
/* --------------------------------------------------------------------------------------------
 * some standard statistic modules
 * --------------------------------------------------------------------------------------------
 */
// Function to calculate the mean of an array
double meanx(void *array) {
    int i,n;
    double sum;
    n=GETARRAYHI(array);
    sum = 0.0;
    for (i = 0; i < n; i++) {
        sum += GETFARRAY(array,i);
    }
    return sum / n;
}
// Function to calculate the standard deviation of an array
double stddevx(void *array) {
    int i,n;
    double sum = 0.0;
    double avg = meanx(array);

    n=GETARRAYHI(array);
    for (i = 0; i < n; i++) {
        sum += pow(GETFARRAY(array,i) - avg, 2);
    }
    return sqrt(sum /(n-1));
  }
double covarx(void *array1, void *array2) {
    int    i;
    int    n=GETARRAYHI(array1);
    double mean_x = meanx(array1);
    double mean_y = meanx(array2);
    double covariance = 0.0;

    for (i = 0; i < n; i++) {
        covariance += (GETFARRAY(array1,i) - mean_x) * (GETFARRAY(array2,i) - mean_y);
    }
    covariance=covariance/(n-1);
    return covariance;
}
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
    MATHPROC(cbrt)

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
ENDPROC}

PROCEDURE(euler) {
    RETURNFLOAT(M_E);
    PROCRETURN
ENDPROC
}
PROCEDURE(mean) {
   RETURNFLOAT(meanx(ARG0));
   PROCRETURN
ENDPROC
}
PROCEDURE(stddev) {
    RETURNFLOAT(stddevx(ARG0));
    PROCRETURN
ENDPROC
}

PROCEDURE(correl) {
    double stddev_x = stddevx(ARG0);
    double stddev_y = stddevx(ARG1);

    RETURNFLOAT(covarx(ARG0,ARG1)/(stddev_x * stddev_y));
    PROCRETURN 
    ENDPROC
}

PROCEDURE(covar) {
    RETURNFLOAT(covarx(ARG0,ARG1));
    PROCRETURN
    ENDPROC
}
/* ***** not working at the moment as calling fields can't be updated */
PROCEDURE(regression) {
//void linear_regression(double *x, double *y, int n, double *m, double *b) {
    double sum_x = 0.0, sum_y = 0.0, sum_xx = 0.0, sum_xy = 0.0;
    double m,b;
    int i;
    int n=GETARRAYHI(ARG0);
    for (i = 0; i < n; i++) {
        sum_x += GETFARRAY(ARG0,i);
        sum_y += GETFARRAY(ARG1,i);
        sum_xx += GETFARRAY(ARG0,i) * GETFARRAY(ARG0,i);
        sum_xy += GETFARRAY(ARG0,i) * GETFARRAY(ARG1,i);
    }
    // Berechnung der Steigung (m) und des y-Achsenabschnitts (b)
    m = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
    b = (sum_y - m * sum_x) / n;
    SETFLOAT(ARG2,m);
    SETFLOAT(ARG3,b);
    RETURNFLOAT(m);
    PROCRETURN
    ENDPROC
}
PROCEDURE(djb2) {
 // DJB2-funktion by Daniel J. Bernstein
    unsigned int hash = 5381;
    int c;
    char * str=GETSTRING(ARG0);
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    RETURNINT(hash);
    PROCRETURN
ENDPROC
}
PROCEDURE(murmur) {                 //   algorithm MurmurHash3
    char * key= GETSTRING(ARG0);
    int len=strlen(key);
    const uint8_t *data = (const uint8_t *) key;
    const int nblocks = len / 4;
    const uint32_t const1 = 0xcc9e2d51;
    const uint32_t const2 = 0x1b873593;
    uint32_t hash = GETINT(ARG1);
    uint32_t block;
    int i;

    // Process 4-byte blocks
    for (i = 0; i < nblocks; i++) {
        block = *((uint32_t *) data);
        data += 4;

        // Mix
        block *= const1;
        block = (block << 15) | (block >> (32 - 15));
        block *= const2;

        hash ^= block;
        hash = (hash << 13) | (hash >> (32 - 13));
        hash = hash * 5 + 0xe6546b64;
    }

    // Process remaining bytes
    block = 0;
    switch (len & 3) {
        case 3:
            block ^= data[2] << 16;
        case 2:
            block ^= data[1] << 8;
        case 1:
            block ^= data[0];
            block *= const1;
            block = (block << 15) | (block >> (32 - 15));
            block *= const2;
            hash ^= block;
    }

    // Finalize hash
    hash ^= len;
    hash = (hash ^ (hash >> 16)) * 0x85ebca6b;
    hash = (hash ^ (hash >> 13)) * 0xc2b2ae35;
    hash ^= hash >> 16;
    RETURNINT(hash);
    PROCRETURN
ENDPROC
}

PROCEDURE(fnv1a) {
    uint32_t hash = 2166136261u;  // FNV offset basis
    char * str=GETSTRING(ARG0);
    while (*str) {
        hash ^= (uint8_t)(*str++);
        hash *= 16777619;  // FNV prime
    }
    RETURNINT(hash);
    PROCRETURN
ENDPROC
}

// CRC32 Hash function
PROCEDURE(crc32) {
    uint32_t hash = 0xFFFFFFFF;
    int i;
    char * str= GETSTRING(ARG0);
    while (*str) {
        hash ^= (uint8_t)(*str++);
        for (i = 8; i; i--) {
            if (hash & 1) hash = (hash >> 1) ^ 0xEDB88320;
            else hash >>= 1;
        }
    }
    RETURNINT(~hash);
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
    ADDMATH(cbrt)
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
    ADDPROC(mean, "rxmath.mean", "b",  ".float", "expose a = .float[]");
    ADDPROC(stddev,"rxmath.stddev","b",  ".float", "expose a = .float[]");
    ADDPROC(covar,"rxmath.covar",  "b",  ".float", "expose arg1=.float[],arg2=.float[]");
    ADDPROC(correl,"rxmath.correl","b",  ".float", "expose arg1=.float[],expose arg2=.float[]");
    ADDPROC(regression, "rxmath.regression","b",".float", "expose arg0=.float[],expose arg1=.float[],expose arg2=.float,expose arg3=.float");
    ADDPROC(djb2,   "rxmath.djb2","b",".int", "arg0=.string");
    ADDPROC(murmur, "rxmath.murmur","b",".int", "arg0=.string, seed=.int");
    ADDPROC(fnv1a,  "rxmath.fnv1a", "b",".int", "arg0=.string");
    ADDPROC(crc32,  "rxmath.crc32", "b",".int", "arg0=.string");

ENDLOADFUNCS
