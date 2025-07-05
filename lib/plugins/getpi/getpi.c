//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // For POSIX systems (Linux/macOS)
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file

// Function to get an environment variable
PROCEDURE(getpi) {
    // Should never happen as the compiler checks arguments; best practice is to check this anyway
    if( NUM_ARGS != 1) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "PI calculation type expected") // sets the signal and returns

    char *varName = GETSTRING(ARG(0)); // Get the pi type variable name
    double pi = 0.0;
    double sign = 1.0;
    int terms, i;

    if (strncmp(&varName[0], "L",1)     == 0) {     // Calculate PI according to Leibnitz method
        terms = 10000000; // Number of terms to sum
        for (i = 0; i < terms; i++) {
            if (i % 2 == 0) pi += 1.0 / (2.0 * i + 1.0);
            else pi -= 1.0 / (2.0 * i + 1.0);
        }
        pi=pi*4.0;
     }  else if (strncmp(&varName[0], "M",1)     == 0) { // Monte Carlo method
        terms = 1000000; // Number of points to be generated
        long points_in_circle = 0;
        double x, y;
         for (i = 0; i < terms; i++) {
         // Generate random point (x, y) in the range [-1, 1]
            x = (double) rand() / RAND_MAX * 2.0 - 1.0;
            y = (double) rand() / RAND_MAX * 2.0 - 1.0;
         // Check if the point is inside the unit circle
            if (x * x + y * y <= 1) points_in_circle++;
        }
        pi = 4.0 * (double) points_in_circle / (double) terms;         // Estimate the value of Pi
    } else {     //    if (varName == NULL) {  // If the PI type variable is not found
         pi=3.14;   /* Set a constant as return value */
    }
    SETFLOAT(RETURN, pi);
 // Make sure the signal is reset to ok - best practice
    RESETSIGNAL
}

// Functions to be provided to rexx
LOADFUNCS
//      C Function, REXX namespace & name, Option, Return Type, Arguments
    ADDPROC(getpi,     "getpi.getpi",      "b",    ".float",   "pi_arg=.string");
ENDLOADFUNCS