/* C-only class fixture: declarations and bodies, with no Rexx class shim. */
#include "crexxpa.h"
#include <stdlib.h>
#include <string.h>

#ifndef DECL_ONLY
#if defined(_WIN32)
#include <windows.h>
typedef volatile LONG fixture_counter;
static rxinteger increment(fixture_counter *counter) { return InterlockedIncrement(counter); }
static rxinteger read_counter(fixture_counter *counter) { return InterlockedCompareExchange(counter, 0, 0); }
#else
typedef volatile int fixture_counter;
static rxinteger increment(fixture_counter *counter) { return __atomic_add_fetch(counter, 1, __ATOMIC_SEQ_CST); }
static rxinteger read_counter(fixture_counter *counter) { return __atomic_load_n(counter, __ATOMIC_SEQ_CST); }
#endif
static fixture_counter sessions_created, sessions_destroyed;
#if defined(_MSC_VER)
#define LOCAL __declspec(thread)
#else
#define LOCAL __thread
#endif
typedef struct fixture_session {
    const rxpa_host_services_v1 *host;
    size_t refs;
    rxinteger live, copies, finalized, calls, id;
} fixture_session;
typedef struct box_resource {
    fixture_session *session;
    size_t refs;
    rxinteger tag;
} box_resource;
static LOCAL fixture_session *current;
static void *old_host(void) { return NULL; }
static void *create(const rxpa_host_services_v1 *host) {
    fixture_session *session;
    if (!rxpa_host_has_object_set_type(host)) return NULL;
    session = calloc(1, sizeof(*session));
    if (session) {
        session->host = host; session->refs = 1;
        session->id = increment(&sessions_created);
    }
    return session;
}
static void destroy(void *opaque) {
    fixture_session *session = opaque;
    if (session && !--session->refs) {
        increment(&sessions_destroyed);
        free(session);
    }
}
static int enter(void *session, uint32_t caps, void **previous) {
    (void)caps;
    *previous = current;
    current = session;
    return 0;
}
static void leave(void *previous) { current = previous; }
static uint32_t caps(const char *name) {
    return name ? RXPA_PROCEDURE_CAP_SESSION_AFFINE : 0;
}
RXPA_PLUGIN_SESSION_WITH_HOST(old_host, destroy, enter, leave, caps, create)
static void copy_box(void *destination, void *source);
static void finalize_box(void *value);
static const rxpa_native_payload_ops box_ops = {"rxpa_objects.box", copy_box, finalize_box};
static box_resource *resource(void *value) {
    size_t length;
    const rxpa_native_payload_ops *ops;
    box_resource *result = NULL;
    void *data = GETNATIVEPAYLOAD(value, &length, &ops, NULL);
    if (data && length == sizeof(result) && ops == &box_ops) memcpy(&result, data, length);
    return result;
}
static void release_box(box_resource *box) {
    if (box && !--box->refs) {
        --box->session->live;
        destroy(box->session);
        free(box);
    }
}
static void copy_box(void *destination, void *source) {
    box_resource *box = resource(source);
    if (box) {
        ++box->refs; ++box->session->copies;
        if (SETNATIVEPAYLOAD(destination, &box, sizeof(box), &box_ops, 0)) release_box(box);
    }
}
static void finalize_box(void *value) {
    box_resource *box = resource(value);
    if (box) ++box->session->finalized;
    release_box(box);
}
PROCEDURE(make_box)
{
    box_resource *box;
    if (GETINT(ARG0) < 0) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "negative initial value") }
    box = calloc(1, sizeof(*box));
    if (!box) { RETURNSIGNAL(SIGNAL_FAILURE, "allocation") }
    box->session = current; box->refs = 1; box->tag = GETINT(ARG0);
    ++current->refs; ++current->live; ++current->calls;
    if (SETNATIVEPAYLOAD(RETURN, &box, sizeof(box), &box_ops, 0)) {
        release_box(box); RETURNSIGNAL(SIGNAL_FAILURE, "payload")
    }
    SETNUMATTRS(RETURN, 1);
    SETINT(GETATTR(RETURN, 0), GETINT(ARG0));
    if (SETOBJECTTYPE(current->host, RETURN, "rxpa_objects.box")) {
        RETURNSIGNAL(SIGNAL_FAILURE, "native object publication failed")
    }
    RESETSIGNAL
}
METHODPROCEDURE(read_box)
{
    if (!resource(ARG0)) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "missing box resource") }
    SETINT(RETURN, GETINT(GETATTR(ARG0, 0)));
    RESETSIGNAL
}
METHODPROCEDURE(add_box)
{
    rxinteger total = GETINT(GETATTR(ARG0, 0)) + GETINT(ARG1);
    SETINT(GETATTR(ARG0, 0), total);
    SETINT(RETURN, total);
    RESETSIGNAL
}
METHODPROCEDURE(exchange_box)
{
    rxinteger old = GETINT(GETATTR(ARG0, 0));
    SETINT(GETATTR(ARG0, 0), GETINT(ARG1));
    SETINT(ARG1, old);
    RESETSIGNAL
}
PROCEDURE(match_box) { SETINT(RETURN, GETINT(ARG0) >= 0); RESETSIGNAL }
/* RXPA values have independent physical fields. A string return may retain
 * an integer field that is not its result contract or an execution failure. */
PROCEDURE(native_text) { SETINT(RETURN, 17); SETSTRING(RETURN, "native text"); RESETSIGNAL }
PROCEDURE(callback_text) {
    if (CALLMETHOD(ARG0, "rxsig1|text|.string|", 0, NULL, RETURN)) return;
    RESETSIGNAL
}
METHODPROCEDURE(double_box)
{
    if (CALLMETHOD(ARG0, "rxsig1|read|.int|", 0, NULL, RETURN)) return;
    SETINT(RETURN, 2 * GETINT(RETURN));
    RESETSIGNAL
}
METHODPROCEDURE(callback_box)
{
    rxpa_attribute_value args[1];
    args[0] = ARG0;
    if (CALLMETHOD(ARG1, "rxsig1|apply|.int|value=.rxpa_objects..counter", 1, args, RETURN)) return;
    RESETSIGNAL
}
METHODPROCEDURE(fail_box)
{
    /* The integer result intentionally equals the signal code. */
    SETINT(RETURN, SIGNAL_DIVISION_BY_ZERO);
    RETURNSIGNAL(SIGNAL_DIVISION_BY_ZERO, "native method failure")
}
METHODPROCEDURE(nested_fail_box)
{
    if (CALLMETHOD(ARG0, "rxsig1|fail|.int|", 0, NULL, RETURN)) return;
    RESETSIGNAL
}
PROCEDURE(restamp)
{
    SETINT(RETURN, SETOBJECTTYPE(current->host, ARG0, GETSTRING(ARG1)));
    RESETSIGNAL
}
PROCEDURE(invalid_inputs)
{
    if (!SETOBJECTTYPE(current->host, NULL, "rxpa_objects.box") ||
        !SETOBJECTTYPE(current->host, ARG0, NULL)) {
        RETURNSIGNAL(SIGNAL_FAILURE, "invalid type publication accepted")
    }
    SETINT(RETURN, 0); RESETSIGNAL
}
PROCEDURE(counts)
{
    const char *key = GETSTRING(ARG0);
    SETINT(RETURN, !strcmp(key, "live") ? current->live :
                  !strcmp(key, "copies") ? current->copies :
                  !strcmp(key, "finalized") ? current->finalized :
                  !strcmp(key, "session") ? current->id :
                  !strcmp(key, "sessions_created") ? read_counter(&sessions_created) :
                  !strcmp(key, "sessions_destroyed") ? read_counter(&sessions_destroyed) : current->calls);
    RESETSIGNAL
}
#endif

LOADFUNCS
ADDINTERFACE("rxpa_objects.counter");
ADDINTERFACE("rxpa_objects.text_source");
ADDMETHOD("rxpa_objects.text_source", "text", ".string", "");
ADDFACTORY("rxpa_objects.counter", "*", ".rxpa_objects..counter", "initial=.int");
ADDFACTORY("rxpa_objects.counter", "from", ".rxpa_objects..counter", "initial=.int");
ADDMETHOD("rxpa_objects.counter", "read", ".int", "");
ADDMETHOD("rxpa_objects.counter", "add", ".int", "amount=.int");
ADDDEFAULTMETHODPROC(double_box, "rxpa_objects.counter", "doubled", ".int", "");
ADDINTERFACE("rxpa_objects.callback");
ADDMETHOD("rxpa_objects.callback", "apply", ".int", "value=.rxpa_objects..counter");
ADDCLASS("rxpa_objects.box");
ADDIMPLEMENTS("rxpa_objects.box", "rxpa_objects.counter");
ADDFACTORYPROC(make_box, "rxpa_objects.box", ".rxpa_objects..box", "initial=.int");
ADDNAMEDFACTORYPROC(make_box, "rxpa_objects.box", "from", ".rxpa_objects..box", "initial=.int");
ADDMATCHPROC(match_box, "rxpa_objects.box", "initial=.int");
ADDNAMEDMATCHPROC(match_box, "rxpa_objects.box", "from", "initial=.int");
ADDMETHODPROC(read_box, "rxpa_objects.box", "read", ".int", "");
ADDMETHODPROC(add_box, "rxpa_objects.box", "add", ".int", "amount=.int");
ADDMETHODPROC(exchange_box, "rxpa_objects.box", "exchange", ".void", "expose amount=.int");
ADDMETHODPROC(callback_box, "rxpa_objects.box", "callback", ".int", "target=.rxpa_objects..callback");
ADDMETHODPROC(fail_box, "rxpa_objects.box", "fail", ".int", "");
ADDMETHODPROC(nested_fail_box, "rxpa_objects.box", "nested_fail", ".int", "");
ADDPROC(make_box, "rxpa_objects.make", "b", ".rxpa_objects..box", "initial=.int");
ADDPROC(restamp, "rxpa_objects.restamp", "b", ".int", "expose value=.object,type=.string");
ADDPROC(invalid_inputs, "rxpa_objects.invalid_inputs", "b", ".int", "expose value=.object");
ADDPROC(counts, "rxpa_objects.counts", "b", ".int", "key=.string");
ADDPROC(native_text, "rxpa_objects.native_text", "b", ".string", "");
ADDPROC(callback_text, "rxpa_objects.callback_text", "b", ".string", "source=.rxpa_objects..text_source");
ENDLOADFUNCS
