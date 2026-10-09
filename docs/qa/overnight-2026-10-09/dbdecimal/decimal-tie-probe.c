#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
int main(void) { long double x=strtold("1.23445",0), s=x*10000.L; printf("LDBL_MANT_DIG=%d LDBL_DIG=%d sizeof=%zu\n", LDBL_MANT_DIG,LDBL_DIG,sizeof(x));printf("parsed=%.40Lg\nscaled=%.40Lg\ntie_error=%.40Lg\nscaled_ulp=%.40Lg\n",x,s,s-12344.5L,nextafterl(s,INFINITY)-s);return 0; }
