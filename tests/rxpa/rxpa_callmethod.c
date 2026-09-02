// RXPA native-to-cREXX method callback regression plugin.

#include "crexxpa.h"

PROCEDURE(invoke)
{
    rxpa_attribute_value method_args[1];

    method_args[0] = ARG1;
    if (CALLMETHOD(ARG0,
                   "rxsig1|on_value|.int|value=.int",
                   1,
                   method_args,
                   RETURN) != 0) {
        return;
    }
    RESETSIGNAL
}

PROCEDURE(invoke_failure)
{
    rxpa_attribute_value method_args[1];

    method_args[0] = ARG1;
    if (CALLMETHOD(ARG0,
                   "rxsig1|fail|.int|value=.int",
                   1,
                   method_args,
                   RETURN) != 0) {
        return;
    }
    RESETSIGNAL
}

LOADFUNCS
ADDINTERFACE("rxpa_callmethod.callback");
ADDMETHOD("rxpa_callmethod.callback", "on_value", ".int", "value=.int");
ADDMETHOD("rxpa_callmethod.callback", "fail", ".int", "value=.int");
ADDPROC(invoke, "rxpa_callmethod.invoke", "b", ".int",
        "receiver=.callback,value=.int");
ADDPROC(invoke_failure, "rxpa_callmethod.invoke_failure", "b", ".int",
        "receiver=.callback,value=.int");
ENDLOADFUNCS
