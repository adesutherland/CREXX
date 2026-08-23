#include "crexxpa.h"

RXPA_PLUGIN_PROCESS_REENTRANT

#ifndef DECL_ONLY
PROCEDURE(rcc_value)
{
    SETSTRING(RETURN, "rcc provider value");
    RESETSIGNAL
}
#endif

LOADFUNCS
ADDPROC(rcc_value, "rcc_provider.value", "b", ".string", "");
ENDLOADFUNCS
