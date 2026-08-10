#include "crexxpa.h"

static uint32_t failure_capabilities(const char *procedure_name) {
    (void)procedure_name;
    return RXPA_PROCEDURE_CAP_SESSION_AFFINE;
}

static void *failure_create(void) { return 0; }
static void failure_destroy(void *session) { (void)session; }
static int failure_enter(void *session, uint32_t capabilities,
                         void **previous) {
    (void)session; (void)capabilities; (void)previous;
    return -1;
}
static void failure_leave(void *previous) { (void)previous; }

RXPA_PLUGIN_SESSION_AWARE(failure_create, failure_destroy,
                          failure_enter, failure_leave,
                          failure_capabilities)

PROCEDURE(failure_probe) {
    (void)_numargs; (void)_arg; (void)_return; (void)_signal;
}

LOADFUNCS
    ADDPROC(failure_probe, "e3.dynamic_session_failure", "b", ".void", "");
ENDLOADFUNCS
