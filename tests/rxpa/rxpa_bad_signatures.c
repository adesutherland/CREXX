#include "crexxpa.h"

PROCEDURE(return_42)
{
    RETURNINT(42);
    RESETSIGNAL
}

LOADFUNCS
ADDPROC(return_42, "signature_probe.valid_zero", "b", ".int", "");
ADDPROC(return_42, "signature_probe.valid_one", "b", ".int", "value=.int");
ADDPROC(return_42, "signature_probe.valid_multi", "b", ".int", "left=.int,right=.int");

ADDPROC(return_42, "signature_probe.bad_unnamed", "b", ".int", ".int,.int");
ADDPROC(return_42, "signature_probe.bad_separator", "b", ".int", "left=.int;right=.int");
ADDPROC(return_42, "signature_probe.bad_arg_type", "b", ".int", "value=.");
ADDPROC(return_42, "signature_probe.bad_return_type", "b", ".", "value=.int");
ADDPROC(return_42, "signature_probe.bad_empty_component", "b", ".int", "left=.int,,right=.int");
ADDPROC(return_42, "signature_probe.bad_trailing_component", "b", ".int", "left=.int,");
ENDLOADFUNCS
