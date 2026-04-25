// RXPA class/interface declaration test plugin.

#include "crexxpa.h"

#ifndef DECL_ONLY

PROCEDURE(make_env)
{
    SETSTRING(RETURN, "native environment placeholder");
    RESETSIGNAL
}

PROCEDURE(make_native)
{
    SETSTRING(RETURN, "native class placeholder");
    RESETSIGNAL
}

#endif

LOADFUNCS
ADDINTERFACE("rxpa_classdecl.environment");
ADDFACTORY("rxpa_classdecl.environment", "*", ".environment", "name=.string");
ADDMETHOD("rxpa_classdecl.environment", "describe", ".string", "");

ADDCLASS("rxpa_classdecl.nativeenvironment");
ADDIMPLEMENTS("rxpa_classdecl.nativeenvironment", "rxpa_classdecl.environment");
ADDFACTORY("rxpa_classdecl.nativeenvironment", "*", ".nativeenvironment", "name=.string");
ADDMETHOD("rxpa_classdecl.nativeenvironment", "describe", ".string", "");

ADDPROC(make_env, "rxpa_classdecl.make", "b", ".environment", "");
ADDPROC(make_native, "rxpa_classdecl.make_native", "b", ".nativeenvironment", "");
ENDLOADFUNCS
