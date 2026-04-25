// rxpa (plugin architecture) functional test procedures

#include <stdio.h>
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file

// Do not need to define functions for static linking to the compiler - only REXX declarations are needed
#ifndef DECL_ONLY

typedef struct native_payload_test {
    int id;
} native_payload_test;

static int native_payload_copy_count = 0;
static int native_payload_finalize_count = 0;

static void native_payload_copy(void *dest_value, void *source_value);
static void native_payload_finalize(void *value);

static const rxpa_native_payload_ops native_payload_test_ops = {
    "rxpatests.native_payload_test",
    native_payload_copy,
    native_payload_finalize
};

static void native_payload_copy(void *dest_value, void *source_value) {
    native_payload_test *source_payload;
    native_payload_test copied_payload;

    source_payload = (native_payload_test*)GETNATIVEPAYLOAD(source_value, NULL, NULL, NULL);
    if (!source_payload) return;

    copied_payload = *source_payload;
    native_payload_copy_count++;
    SETNATIVEPAYLOAD(dest_value, &copied_payload, sizeof(copied_payload), &native_payload_test_ops, 0);
}

static void native_payload_finalize(void *value) {
    native_payload_test *payload;

    payload = (native_payload_test*)GETNATIVEPAYLOAD(value, NULL, NULL, NULL);
    if (payload) native_payload_finalize_count++;
}

PROCEDURE(native_payload_reset)
{
    native_payload_copy_count = 0;
    native_payload_finalize_count = 0;
    RESETSIGNAL
}

PROCEDURE(native_payload_make)
{
    native_payload_test payload;

    if( NUM_ARGS != 1) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")

    payload.id = (int)GETINT(ARG(0));
    SETNATIVEPAYLOAD(RETURN, &payload, sizeof(payload), &native_payload_test_ops, 0);
    RESETSIGNAL
}

PROCEDURE(native_payload_id)
{
    native_payload_test *payload;

    if( NUM_ARGS != 1) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")

    payload = (native_payload_test*)GETNATIVEPAYLOAD(ARG(0), NULL, NULL, NULL);
    SETINT(RETURN, payload ? payload->id : -1);
    RESETSIGNAL
}

PROCEDURE(native_payload_clear)
{
    if( NUM_ARGS != 1) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")

    SETNATIVEPAYLOAD(ARG(0), NULL, 0, NULL, 0);
    RESETSIGNAL
}

PROCEDURE(native_payload_copies)
{
    SETINT(RETURN, native_payload_copy_count);
    RESETSIGNAL
}

PROCEDURE(native_payload_finalizers)
{
    SETINT(RETURN, native_payload_finalize_count);
    RESETSIGNAL
}

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

// Bubble Sorts an array of integers
PROCEDURE(bubble_sort)
{
    int i, j, pivot, temp;
    int size;

    // Check the number of arguments
    if (NUM_ARGS != 1) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")

    // Get the ayyay size
    size = GETNUMATTRS(ARG(0));

    // Bubble-sort the array
    REGISTER array = ARG(0);
    for (i = 0; i < size - 1; i++) {
        for (j = 0; j < size - i - 1; j++) {
            if (GETINT(GETATTR(array, j)) > GETINT(GETATTR(array, j + 1))) {
                SWAPATTRS(array, j, j + 1);
            }
        }
    }
    // Make sure the signal is reset/ok
    RESETSIGNAL
}

// Populate caller-provided arrays using the RXPA push helpers.
PROCEDURE(fill_push_arrays)
{
    if (NUM_ARGS != 2) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")

    PUSHIARRAY(ARG(0), 0, 7);
    PUSHIARRAY(ARG(0), 1, 8);
    PUSHSARRAY(ARG(1), 0, "alpha");
    PUSHSARRAY(ARG(1), 1, "beta");

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
ADDPROC(bubble_sort,       "rxpatests.bubble_sort",         "b",     ".void",      "expose a = .int[]");
ADDPROC(fill_push_arrays,  "rxpatests.fill_push_arrays",    "b",     ".void",      "expose ints = .int[], expose strings = .string[]");
ADDPROC(native_payload_reset,      "rxpatests.native_payload_reset",      "b",     ".void",   "");
ADDPROC(native_payload_make,       "rxpatests.native_payload_make",       "b",     ".binary", "id = .int");
ADDPROC(native_payload_id,         "rxpatests.native_payload_id",         "b",     ".int",    "payload = .binary");
ADDPROC(native_payload_clear,      "rxpatests.native_payload_clear",      "b",     ".void",   "expose payload = .binary");
ADDPROC(native_payload_copies,     "rxpatests.native_payload_copies",     "b",     ".int",    "");
ADDPROC(native_payload_finalizers, "rxpatests.native_payload_finalizers", "b",     ".int",    "");
ENDLOADFUNCS
