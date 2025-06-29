//
// System Information Plugin for crexx/pa - Plugin Architecture
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
    #include <windows.h>
    #include <direct.h>   // For Windows
    #define getcwd _getcwd  // Map to Windows-specific version
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
    #include <libgen.h>
    #include <limits.h>
    #include <unistd.h>
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    #include <unistd.h>
    #include <libgen.h>
    #include <limits.h>
#else
    #error "Unsupported platform"
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

PROCEDURE(getLoadPath) {
    char path[4096];
    char *dir = NULL;

#if defined(_WIN32)
    if (GetModuleFileNameA(NULL, path, sizeof(path)) == 0) {
      RETURNSIGNAL(SIGNAL_FAILURE, "Unable to get current load path")
    }
    char *last_backslash = strrchr(path, '\\');
    if (last_backslash)
        *last_backslash = '\0';
    dir = _strdup(path);

#elif defined(__APPLE__)
    uint32_t size = sizeof(path);
     if (_NSGetExecutablePath(path, &size) != 0)
       RETURNSIGNAL(SIGNAL_FAILURE, "Unable to get current load path")

    char resolved[PATH_MAX];
    if (realpath(path, resolved) == NULL)
      RETURNSIGNAL(SIGNAL_FAILURE, "Unable to get current load path")

    // dirname may modify its argument, so copy it
    char *dirbuf = strdup(resolved);
    /* if (!dirbuf) return NULL; */

    dir = strdup(dirname(dirbuf));
    free(dirbuf);

#elif defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1)
      RETURNSIGNAL(SIGNAL_FAILURE, "Unable to get current load path")

    path[len] = '\0';
    char *dirbuf = strdup(path);
    if (!dirbuf) return NULL;
    RETURNSIGNAL(SIGNAL_FAILURE, "Unable to get current load path")

    dir = strdup(dirname(dirbuf));
    free(dirbuf);

#endif

    SETSTRING(RETURN, dir);
}



// Functions to be provided to rexx
LOADFUNCS
//      C Function, REXX namespace & name, Option, Return Type, Arguments
ADDPROC(getEnv,      "sysinfo.getenv",      "b",    ".string",   "env_name=.string");
ADDPROC(getCwd,      "sysinfo.getcwd",      "b",    ".string",   "");
ADDPROC(getLoadPath, "sysinfo.getloadpath", "b",    ".string",   "");
ENDLOADFUNCS
