/* C-only class fixture: declarations and bodies, with no Rexx class shim. */
#include "crexxpa.h"
RXPA_PLUGIN_PROCESS_REENTRANT

#ifndef DECL_ONLY
PROCEDURE(make_box)
{
    SETNUMATTRS(RETURN, 1);
    SETINT(GETATTR(RETURN, 0), GETINT(ARG0));
    /* Pre-repair control: existing RXPA cannot publish the concrete type. */
    RESETSIGNAL
}
PROCEDURE(read_box)
{
    SETINT(RETURN, GETINT(GETATTR(ARG0, 0)));
    RESETSIGNAL
}
PROCEDURE(add_box)
{
    rxinteger total = GETINT(GETATTR(ARG0, 0)) + GETINT(ARG1);
    SETINT(GETATTR(ARG0, 0), total);
    SETINT(RETURN, total);
    RESETSIGNAL
}
#endif

LOADFUNCS
ADDINTERFACE("rxpa_objects.counter");
ADDFACTORY("rxpa_objects.counter", "*", ".rxpa_objects..counter", "initial=.int");
ADDMETHOD("rxpa_objects.counter", "read", ".int", "");
ADDMETHOD("rxpa_objects.counter", "add", ".int", "amount=.int");
ADDCLASS("rxpa_objects.box");
ADDIMPLEMENTS("rxpa_objects.box", "rxpa_objects.counter");
ADDFACTORY("rxpa_objects.box", "*", ".rxpa_objects..box", "initial=.int");
ADDFACTORY("rxpa_objects.box", "from", ".rxpa_objects..box", "initial=.int");
ADDMETHOD("rxpa_objects.box", "read", ".int", "");
ADDMETHOD("rxpa_objects.box", "add", ".int", "amount=.int");
ADDPROC(make_box, "rxpa_objects.make", "b", ".rxpa_objects..box", "initial=.int");
ADDPROC(make_box, "rxpa_objects.box.\xc2\xa7" "factory", "b", ".rxpa_objects..box", "initial=.int");
ADDPROC(make_box, "rxpa_objects.box.\xc2\xa7" "factory.from", "b", ".rxpa_objects..box", "initial=.int");
ADDPROC(read_box, "rxpa_objects.box.read", "b", ".int", "");
ADDPROC(add_box, "rxpa_objects.box.add", "b", ".int", "amount=.int");
ENDLOADFUNCS
