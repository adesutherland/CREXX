/* Minimal native-call kernel used by the PERF3-13 E3b-P1 Release verdict. */

#include "crexxpa.h"

#ifdef RXPA_BENCH_PROCESS_REENTRANT
RXPA_PLUGIN_PROCESS_REENTRANT
#define RXPA_BENCH_PROCEDURE "rxpabenchreentrant.tick"
#else
#define RXPA_BENCH_PROCEDURE "rxpabenchlegacy.tick"
#endif

PROCEDURE(tick)
{
    (void)_numargs;
    (void)_arg;
    (void)_return;
    RESETSIGNAL
}

LOADFUNCS
ADDPROC(tick, RXPA_BENCH_PROCEDURE, "b", ".void", "");
ENDLOADFUNCS
