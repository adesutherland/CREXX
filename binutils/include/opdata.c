#include "rxdefs.h"
#include <stddef.h> /* for NULL */

#define X(NAME, OPCODE, FMT, FLOW, FLAGS, DESC) \
    { #NAME, OPCODE, FMT, FLOW, FLAGS, DESC },

const OpInfo op_table[] = {
    #include "rxops.h"
    { NULL, -1, FMT_EMPTY, FLOW_NEXT, 0, NULL }
};
#undef X
