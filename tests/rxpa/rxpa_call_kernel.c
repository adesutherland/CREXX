/* Minimal native-call kernel used by the PERF3-13 E3b-P1 Release verdict. */

#include "crexxpa.h"

#ifdef RXPA_BENCH_SESSION_AWARE
#include <stdlib.h>
#if defined(_MSC_VER)
#define RXPA_BENCH_THREAD_LOCAL __declspec(thread)
#else
#define RXPA_BENCH_THREAD_LOCAL __thread
#endif
static RXPA_BENCH_THREAD_LOCAL void *rxpa_bench_session;
static uint32_t rxpa_bench_capabilities(const char *procedure_name) {
    (void)procedure_name;
    return RXPA_PROCEDURE_CAP_SESSION_AFFINE;
}
static void *rxpa_bench_session_create(void) {
    return malloc(1u);
}
static void rxpa_bench_session_destroy(void *session) {
    free(session);
}
static int rxpa_bench_session_enter(void *session, uint32_t capabilities,
                                    void **previous) {
    if (!session || !previous ||
        capabilities != RXPA_PROCEDURE_CAP_SESSION_AFFINE) return -1;
    *previous = rxpa_bench_session;
    rxpa_bench_session = session;
    return 0;
}
static void rxpa_bench_session_leave(void *previous) {
    rxpa_bench_session = previous;
}
RXPA_PLUGIN_SESSION_AWARE(rxpa_bench_session_create,
                          rxpa_bench_session_destroy,
                          rxpa_bench_session_enter,
                          rxpa_bench_session_leave,
                          rxpa_bench_capabilities)
#define RXPA_BENCH_PROCEDURE "rxpabenchsession.tick"
#elif defined(RXPA_BENCH_PROCESS_REENTRANT)
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
