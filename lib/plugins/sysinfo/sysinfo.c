//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // For POSIX systems (Linux/macOS)
#ifdef _WIN32
#include <direct.h>   // For Windows
#define getcwd _getcwd  // Map to Windows-specific version
#endif
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file

// Function to get an environment variable
PROCEDURE(getEnv) {

    // Should never happen as the compiler checks arguments; best practice is to check this anyway
    if( NUM_ARGS != 1) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "environment variable name expected") // sets the signal and returns

    char *varName = GETSTRING(ARG(0)); // Get the environment variable name
    char *varValue = getenv(varName); // Get the environment variable value

    /* Set the return value */
    if (varValue == NULL) {
        // If the environment variable is not found
        SETSTRING(RETURN, "");
    }
    else {
        SETSTRING(RETURN, varValue);
    }

    // Make sure the signal is reset to ok - best practice
    RESETSIGNAL
}

// Function to get the current working directory
PROCEDURE(getCwd) {
    char cwd[1024];
    // Should never happen as the compiler checks arguments; best practice is to check this anyway
    if( NUM_ARGS != 0) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "no arguments expected") // sets the signal and returns

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        /* Set the return value */
        SETSTRING(RETURN, cwd);
    } else {
        RETURNSIGNAL(SIGNAL_FAILURE, "Unable to get current working directory")
    }

    // Make sure the signal is reset to ok - best practice
    RESETSIGNAL
}

// Functions to be provided to rexx
LOADFUNCS
//      C Function, REXX namespace & name, Option, Return Type, Arguments
ADDPROC(getEnv,     "sysinfo.getenv",      "b",    ".string",   "env_name=.string");
ADDPROC(getCwd,     "sysinfo.getcwd",      "b",    ".string",   "");
ENDLOADFUNCS
