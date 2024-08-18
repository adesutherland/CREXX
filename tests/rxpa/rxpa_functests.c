// rxpa (plugin architecture) functional test procedures

#include <stdio.h>
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file

// Do not need to define functions for static linking to the compiler - only REXX declarations are needed
#ifndef DECL_ONLY


// Throw a signal
PROCEDURE(throw_signal)
{
    // Set return and make sure the signal is reset/ok
    RETURNSIGNAL(SIGNAL_ERROR, "This is a test signal")
}

// string_concat
PROCEDURE(string_concat)
{
    // buffer to hold the concatenated strings
    char buffer[1024];

    /* This should never happen as the compiler will have spotted this error */
    /* However best practice is to check this as versions may have changed   */
    if( NUM_ARGS != 2) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")

    // Concatenate the strings
    sprintf(buffer, "%s%s", GETSTRING(ARG(0)), GETSTRING(ARG(1)));

    // Set return
    SETSTRING(RETURN, buffer);

    // Set return and make sure the signal is reset/ok
    RESETSIGNAL
}

// Concatenate two strings to a third argument
PROCEDURE(string_concat_ref)
{
    // buffer to hold the concatenated strings
    char buffer[1024];

    /* This should never happen as the compiler will have spotted this error */
    /* However best practice is to check this as versions may have changed   */
    if( NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")

    // Concatenate the strings
    sprintf(buffer, "%s%s", GETSTRING(ARG(0)), GETSTRING(ARG(1)));

    // Set return
    SETSTRING(ARG(2), buffer);

    // Make sure the signal is reset/ok
    RESETSIGNAL
}

// Add two integers
PROCEDURE(add_integers)
{
    int result;

    /* This should never happen as the compiler will have spotted this error */
    /* However, the best practice is to check this as versions may have changed   */
    if( NUM_ARGS != 2) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")

    // Add the integers
    result = GETINT(ARG(0)) + GETINT(ARG(1));

    // Set return
    SETINT(RETURN, result);

    // Make sure the signal is reset/ok
    RESETSIGNAL
}

// Add two integers to a third argument
PROCEDURE(add_integers_ref)
{
    int result;

    /* This should never happen as the compiler will have spotted this error */
    /* However best practice is to check this as versions may have changed   */
    if( NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")

    // Add the integers
    result = GETINT(ARG(0)) + GETINT(ARG(1));

    // Set return
    SETINT(ARG(2), result);

    // Make sure the signal is reset/ok
    RESETSIGNAL
}

// Add two floats
PROCEDURE(add_floats)
{
    double result;

    /* This should never happen as the compiler will have spotted this error */
    /* However best practice is to check this as versions may have changed   */
    if( NUM_ARGS != 2) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")

    // Add the floats
    result = GETFLOAT(ARG(0)) + GETFLOAT(ARG(1));

    // Set return
    SETFLOAT(RETURN, result);

    // Make sure the signal is reset/ok
    RESETSIGNAL
}

// Add two floats to a third argument
PROCEDURE(add_floats_ref)
{
    double result;

    /* This should never happen as the compiler will have spotted this error */
    /* However, the best practice is to check this as versions may have changed   */
    if( NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")

    // Add the floats
    result = GETFLOAT(ARG(0)) + GETFLOAT(ARG(1));

    // Set return
    SETFLOAT(ARG(2), result);

    // Make sure the signal is reset/ok
    RESETSIGNAL
}

#endif

// Functions to be provided to rexx
LOADFUNCS
//      C Function__,      REXX namespace & name,           Option_, Return Type_, Arguments
ADDPROC(throw_signal,      "rxpatests.throw_signal",        "b",     ".void",      "");
ADDPROC(string_concat,     "rxpatests.string_concat",       "b",     ".string",    "s1 = .string, s2 = .string");
ADDPROC(string_concat_ref, "rxpatests.string_concat_ref",   "b",     ".void",      "s1 = .string, s2 = .string, expose s3 = .string");
ADDPROC(add_integers,      "rxpatests.add_integers",        "b",     ".int",       "i1 = .int, i2 = .int");
ADDPROC(add_integers_ref,  "rxpatests.add_integers_ref",    "b",     ".void",      "i1 = .int, i2 = .int, expose i3 = .int");
ADDPROC(add_floats,        "rxpatests.add_floats",          "b",     ".float",     "f1 = .float, f2 = .float");
ADDPROC(add_floats_ref,    "rxpatests.add_floats_ref",      "b",     ".void",      "f1 = .float, f2 = .float, expose f3 = .float");
ENDLOADFUNCS
