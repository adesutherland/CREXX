#include <rxpa/crexxpa.h>

RXPA_PLUGIN_PROCESS_REENTRANT

PROCEDURE(sdkversion)
{
    const char *version = rxpa_version;
    (void)_arg;
    if (NUM_ARGS != 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "sdkversion takes no arguments")
    SETSTRING(RETURN, version);
    RESETSIGNAL
}

PROCEDURE(addints)
{
    (void)_numargs;
    RETURNINT(GETINT(ARG0) + GETINT(ARG1));
    RESETSIGNAL
}

PROCEDURE(copycontract)
{
    char result[] = "copy-owned";
    (void)_numargs;
    (void)_arg;
    SETSTRING(RETURN, result);
    result[0] = 'X';
    RESETSIGNAL
}

/* Registration metadata remains on its separate legacy mutable-string API. */
static char sdkversion_name[] = "cri07_sdk_probe.sdkversion";
static char addints_name[] = "cri07_sdk_probe.addints";
static char copycontract_name[] = "cri07_sdk_probe.copycontract";
static char level_b[] = "b";
static char string_type[] = ".string";
static char int_type[] = ".int";
static char no_arguments[] = "";
static char addints_arguments[] = "left=.int,right=.int";

LOADFUNCS
ADDPROC(sdkversion, sdkversion_name, level_b, string_type, no_arguments);
ADDPROC(addints, addints_name, level_b, int_type, addints_arguments);
ADDPROC(copycontract, copycontract_name, level_b, string_type, no_arguments);
ENDLOADFUNCS
