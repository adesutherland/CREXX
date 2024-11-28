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

#if defined(_WIN32)
    #define REMOVE_DIR(path) _rmdir(path)
#else
    #define REMOVE_DIR(path) rmdir(path)
#endif


void searchReplace(char *str, char search, char replace) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {  // Loop until the end of the string
        if (str[i] == search) {
            str[i] = replace;  // Replace the character
        }
    }
}

// Function to get environment directory
PROCEDURE(getEnv) {
    char *varName = GETSTRING(ARG0);    // Get the environment variable name
    char *varValue = getenv(varName);   // Get the environment variable value
    if (varValue == NULL) {             // If the environment variable is not found
        RETURNSIGNAL(SIGNAL_FAILURE, "Environment not found")
    } else {
        RETURNSTR(varValue);
    }
ENDPROC
}

// Function to get the current working directory
PROCEDURE(getdir) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        RETURNSTR(cwd);     /* Set the return value */
    } else {
        RETURNSIGNAL(SIGNAL_FAILURE, "Unable to get current working directory")
    }
ENDPROC
}
PROCEDURE(setdir) {
    searchReplace(GETSTRING(ARG0),'\\','/');
    if (chdir(GETSTRING(ARG0)) == 0) {
        RETURNINT(0);
    } else {
        RETURNINT(-8);
    }
ENDPROC
}
PROCEDURE(createdir) {
    searchReplace(GETSTRING(ARG0),'\\','/');
    if (chdir(GETSTRING(ARG0)) == 0) {
        RETURNINT(-4);     // already there
    } else {
        if (_mkdir(GETSTRING(ARG0)) == 0) {
            RETURNINT(0);
        } else RETURNINT(-8);   // not created
    }
    ENDPROC
}

PROCEDURE(removedir) {
    searchReplace(GETSTRING(ARG0),'\\','/');
    if (chdir(GETSTRING(ARG0)) == 0) {
        if (REMOVE_DIR(GETSTRING(ARG0)) == 0) RETURNINT(0);
        else RETURNINT(-8);   // not removed
    } else RETURNINT(-4);     // not available
    ENDPROC
}

// Functions to be provided to rexx
LOADFUNCS
//      C Function, REXX namespace & name, Option, Return Type, Arguments
ADDPROC(getEnv,     "system.getenv",      "b",    ".string",   "env_name=.string");
ADDPROC(getdir,     "system.getdir",      "b",    ".string",   "");
ADDPROC(setdir,     "system.setdir",      "b",    ".int",   "arg0=.string");
ADDPROC(createdir,  "system.createdir",   "b",    ".int",   "arg0=.string");
ADDPROC(removedir,  "system.removedir",   "b",    ".int",   "arg0=.string");
ENDLOADFUNCS
