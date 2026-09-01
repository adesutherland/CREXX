#include <rxpa/crexxpa.h>

RXPA_PLUGIN_PROCESS_REENTRANT

PROCEDURE(cppconst)
{
    const char *result = "cpp-immutable";
    (void)_arg;
    SETSTRING(RETURN, result);
    if (NUM_ARGS != 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "cppconst takes no arguments")
    RESETSIGNAL
}

static char cppconst_name[] = "cri08_const_cpp.cppconst";
static char level_b[] = "b";
static char string_type[] = ".string";
static char no_arguments[] = "";

LOADFUNCS
ADDPROC(cppconst, cppconst_name, level_b, string_type, no_arguments);
ENDLOADFUNCS
