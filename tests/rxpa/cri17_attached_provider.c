#include "crexxpa.h"

#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
typedef volatile LONG cri17_counter;
static LONG cri17_increment(cri17_counter *counter) {
    return InterlockedIncrement(counter);
}
static LONG cri17_decrement(cri17_counter *counter) {
    return InterlockedDecrement(counter);
}
static LONG cri17_read(cri17_counter *counter) {
    return InterlockedCompareExchange(counter, 0, 0);
}
#define CRI17_THREAD_LOCAL __declspec(thread)
#else
typedef volatile int cri17_counter;
static int cri17_increment(cri17_counter *counter) {
    return __atomic_add_fetch(counter, 1, __ATOMIC_SEQ_CST);
}
static int cri17_decrement(cri17_counter *counter) {
    return __atomic_sub_fetch(counter, 1, __ATOMIC_SEQ_CST);
}
static int cri17_read(cri17_counter *counter) {
    return __atomic_load_n(counter, __ATOMIC_SEQ_CST);
}
#define CRI17_THREAD_LOCAL __thread
#endif

typedef struct cri17_session {
    rxinteger id;
} cri17_session;

static cri17_counter cri17_created_count;
static cri17_counter cri17_destroyed_count;
static cri17_counter cri17_live_count;
static CRI17_THREAD_LOCAL cri17_session *cri17_current_session;

static uint32_t cri17_capabilities(const char *procedure_name) {
    if (procedure_name &&
        strcmp(procedure_name, "cri17provider.session_id") == 0) {
        return RXPA_PROCEDURE_CAP_SESSION_AFFINE;
    }
    return RXPA_PROCEDURE_CAP_PROCESS_REENTRANT;
}

static void *cri17_session_create(void) {
    cri17_session *session = (cri17_session *)calloc(1u, sizeof(*session));
    if (!session) return 0;
    session->id = (rxinteger)cri17_increment(&cri17_created_count);
    (void)cri17_increment(&cri17_live_count);
    return session;
}

static void cri17_session_destroy(void *opaque_session) {
    cri17_session *session = (cri17_session *)opaque_session;
    if (!session) return;
    (void)cri17_increment(&cri17_destroyed_count);
    (void)cri17_decrement(&cri17_live_count);
    free(session);
}

static int cri17_session_enter(void *opaque_session, uint32_t capabilities,
                               void **previous) {
    if (!opaque_session || !previous ||
        capabilities != RXPA_PROCEDURE_CAP_SESSION_AFFINE) return -1;
    *previous = cri17_current_session;
    cri17_current_session = (cri17_session *)opaque_session;
    return 0;
}

static void cri17_session_leave(void *previous) {
    cri17_current_session = (cri17_session *)previous;
}

RXPA_PLUGIN_SESSION_AWARE(cri17_session_create, cri17_session_destroy,
                          cri17_session_enter, cri17_session_leave,
                          cri17_capabilities)

PROCEDURE(cri17_session_id)
{
    (void)_numargs;
    (void)_arg;
    if (!cri17_current_session) {
        SETINT(SIGNAL, 1);
        return;
    }
    SETINT(RETURN, cri17_current_session->id);
    RESETSIGNAL
}

PROCEDURE(cri17_created)
{
    (void)_numargs;
    (void)_arg;
    SETINT(RETURN, cri17_read(&cri17_created_count));
    RESETSIGNAL
}

PROCEDURE(cri17_destroyed)
{
    (void)_numargs;
    (void)_arg;
    SETINT(RETURN, cri17_read(&cri17_destroyed_count));
    RESETSIGNAL
}

PROCEDURE(cri17_live)
{
    (void)_numargs;
    (void)_arg;
    SETINT(RETURN, cri17_read(&cri17_live_count));
    RESETSIGNAL
}

LOADFUNCS
ADDPROC(cri17_session_id, "cri17provider.session_id", "b", ".int", "");
ADDPROC(cri17_created, "cri17provider.created", "b", ".int", "");
ADDPROC(cri17_destroyed, "cri17provider.destroyed", "b", ".int", "");
ADDPROC(cri17_live, "cri17provider.live", "b", ".int", "");
ENDLOADFUNCS
