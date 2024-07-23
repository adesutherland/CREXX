// rxpa (plugin architecture) test program

#include <stdio.h>
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file

// Do not need to define functions for static linking to the compiler -
// only REXX declarations are needed
#ifndef DECL_ONLY

// Proc 1
PROCEDURE(proc1)
{
    // Set return and make sure the signal is reset/ok
    SETSTRING(RETURN, "static proc1 output");
    RESETSIGNAL
}

// Proc 1
PROCEDURE(proc2)
{
    // Set return and make sure the signal is reset/ok
    SETSTRING(RETURN, "static proc2 output");
    RESETSIGNAL
}

#endif

// Functions to be provided to rexx - these are loaded either when the plugin is loaded (dynamic) or
// before main() is called (static)
LOADFUNCS
//      C Function__, REXX namespace & name, Option_, Return Type_, Arguments
ADDPROC(proc1,   "rxpa_staticlink.proc1",          "b",        ".string",           "");
ADDPROC(proc2,  "rxpa_staticlink.proc2",          "b",        ".string",            "");
ENDLOADFUNCS
