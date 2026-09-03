#include "crexxpa.h"

#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
typedef volatile LONG cri17_counter;
static SRWLOCK cri17_session_call_lock = SRWLOCK_INIT;
static CONDITION_VARIABLE cri17_session_call_condition =
        CONDITION_VARIABLE_INIT;
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
static void cri17_session_call_enter(void) {
    AcquireSRWLockExclusive(&cri17_session_call_lock);
}
static void cri17_session_call_leave(void) {
    ReleaseSRWLockExclusive(&cri17_session_call_lock);
}
static void cri17_session_call_wait(void) {
    (void)SleepConditionVariableSRW(
            &cri17_session_call_condition,
            &cri17_session_call_lock, INFINITE, 0);
}
static void cri17_session_call_broadcast(void) {
    WakeAllConditionVariable(&cri17_session_call_condition);
}
#else
#include <pthread.h>
typedef volatile int cri17_counter;
static pthread_mutex_t cri17_session_call_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cri17_session_call_condition = PTHREAD_COND_INITIALIZER;
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
static void cri17_session_call_enter(void) {
    (void)pthread_mutex_lock(&cri17_session_call_lock);
}
static void cri17_session_call_leave(void) {
    (void)pthread_mutex_unlock(&cri17_session_call_lock);
}
static void cri17_session_call_wait(void) {
    (void)pthread_cond_wait(
            &cri17_session_call_condition, &cri17_session_call_lock);
}
static void cri17_session_call_broadcast(void) {
    (void)pthread_cond_broadcast(&cri17_session_call_condition);
}
#endif

typedef struct cri17_session {
    rxinteger id;
} cri17_session;

static cri17_counter cri17_created_count;
static cri17_counter cri17_destroyed_count;
static cri17_counter cri17_live_count;
static CRI17_THREAD_LOCAL cri17_session *cri17_current_session;
static int cri17_session_call_arrivals;

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
    rxinteger session_id;
    (void)_numargs;
    (void)_arg;
    if (!cri17_current_session) {
        SETINT(SIGNAL, 1);
        return;
    }
    session_id = cri17_current_session->id;
    /* Pool capacity permits concurrency but does not promise worker affinity.
     * Make both calls overlap before comparing their session identities. */
    cri17_session_call_enter();
    cri17_session_call_arrivals++;
    cri17_session_call_broadcast();
    while (cri17_session_call_arrivals < 2) cri17_session_call_wait();
    cri17_session_call_leave();
    SETINT(RETURN, session_id);
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
