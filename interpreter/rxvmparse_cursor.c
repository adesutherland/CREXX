/* cREXX License (MIT). Mandatory core RXPA binding for the shared PARSE kernel.
 * Binutils builds this file DECL_ONLY for the compiler/linker catalogue. Keep
 * the seven binding definitions below as the single signature owner. */
#ifndef DECL_ONLY
/* Native compilers provide metadata-only rxpa_* shims in the same process as
 * their embedded VM. Core execution must always call the real VM services. */
#define rxpa_getnativepayload rxvm_getnativepayload
#define rxpa_setnativepayload rxvm_setnativepayload
#define rxpa_isinitialized rxvm_isinitialized
#define rxpa_setint rxvm_setint
#define rxpa_setstring rxvm_setstring
#define rxpa_addfunc_for_plugin rxvm_addfunc_for_plugin
#define rxpa_addclass rxvm_addclass
#define rxpa_addmember rxvm_addmember
#define rxpa_register_static_plugin_manifest_v2 rxvm_register_static_plugin_manifest_v2
#endif
#ifndef PLUGIN_ID
#define PLUGIN_ID rxparse
#endif
#include "../rxpa/crexxpa.h"
#include "rxvmparse_bindings.h"

#ifdef DECL_ONLY
const char rxvm_parse_cursor_provider_id[] = RXPA_STRINGIFY(PLUGIN_ID);
#define PARSE_CLASS(owner) { RXVM_PARSE_BIND_CLASS, owner, 0, 0, ".unknown", 0 },
#define PARSE_FACTORY(fn, owner, type, args) \
    { RXVM_PARSE_BIND_MEMBER, owner, "factory", "*", type, args }, \
    { RXVM_PARSE_BIND_FUNCTION, owner ".\xc2\xa7" "factory", 0, 0, type, args },
#define PARSE_METHOD(fn, owner, member, type, args) \
    { RXVM_PARSE_BIND_MEMBER, owner, "method", member, type, args }, \
    { RXVM_PARSE_BIND_FUNCTION, owner "." member, 0, 0, type, args },
const rxvm_parse_binding rxvm_parse_cursor_bindings[] = {
#else
#include "rxvmparse.h"
#include <stdlib.h>
#include <string.h>
#if defined(CREXX_VM_SINGLE_THREADED)
#define PARSE_LOCAL
#elif defined(_WIN32)
#include <windows.h>
#if defined(_MSC_VER)
#define PARSE_LOCAL __declspec(thread)
#else
#define PARSE_LOCAL __thread
#endif
#else
#include <pthread.h>
#define PARSE_LOCAL __thread
#endif

typedef struct parse_session {
    const rxpa_host_services_v1 *host;
    size_t refs;
} parse_session;
typedef struct parse_resource {
    parse_session *session;
    rxvm_parse_cursor *cursor;
    size_t refs;
} parse_resource;
static PARSE_LOCAL parse_session *current;

static void *old_session(void) { return NULL; }
static void *create_session(const rxpa_host_services_v1 *host) {
    parse_session *session;
    if (!rxpa_host_has_object_set_type(host) ||
        !rxpa_host_has_string_view(host) || !rxpa_host_has_string_set(host)) return NULL;
    session = calloc(1, sizeof(*session));
    if (session) { session->host = host; session->refs = 1; }
    return session;
}
static void release_session(void *opaque) {
    parse_session *session = opaque;
    if (session && !--session->refs) free(session);
}
static int enter_session(void *session, uint32_t capabilities, void **previous) {
    (void)capabilities;
    *previous = current; current = session; return 0;
}
static void leave_session(void *previous) { current = previous; }
static uint32_t capabilities(const char *name) {
    return name ? RXPA_PROCEDURE_CAP_SESSION_AFFINE : 0;
}
/* Catalogue registration is explicit, including on platforms without C
 * constructors. The outer once primitive covers simultaneous VM creation. */
#undef INITIALIZER
#define INITIALIZER(fn) static void fn(void) {
RXPA_PLUGIN_SESSION_WITH_HOST(old_session, release_session, enter_session,
                             leave_session, capabilities, create_session)

static void copy_cursor(void *destination, void *source);
static void finalize_cursor(void *value);
static const rxpa_native_payload_ops cursor_ops = {
    "rxparse.cursor", copy_cursor, finalize_cursor
};
static parse_resource *resource(void *value) {
    size_t length;
    const rxpa_native_payload_ops *ops;
    parse_resource *result = NULL;
    void *data = GETNATIVEPAYLOAD(value, &length, &ops, NULL);
    if (data && length == sizeof(result) && ops == &cursor_ops)
        memcpy(&result, data, length);
    return result;
}
static void release_resource(parse_resource *owner) {
    if (owner && !--owner->refs) {
        rxvm_parse_destroy(owner->cursor);
        release_session(owner->session);
        free(owner);
    }
}
static void copy_cursor(void *destination, void *source) {
    parse_resource *owner = resource(source);
    if (!owner) return;
    ++owner->refs;
    if (SETNATIVEPAYLOAD(destination, &owner, sizeof(owner), &cursor_ops, 0))
        release_resource(owner);
}
static void finalize_cursor(void *value) { release_resource(resource(value)); }
static int valid_resource(parse_resource *owner) {
    return owner && current && owner->session == current;
}
static void signal_parse(void *signal, int result, parse_resource *owner) {
    rxvm_parse_diagnostic problem = rxvm_parse_diagnose(
        result, owner ? rxvm_parse_classic_numeric(owner->cursor) : 0);
    SETINT(signal, problem.signal); SETSTRING(signal, problem.message);
}
PROCEDURE(make_cursor) {
    const char *source;
    const unsigned char *plan;
    size_t source_length, plan_length;
    parse_resource *owner;
    int status;
    if (NUM_ARGS != 2 || !_arg || !ARG0 || !ARG1 || !current) {
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "PARSE cursor requires source and plan")
    }
    if (current->host->string_view(ARG0, &source, &source_length)) {
        RETURNSIGNAL(SIGNAL_UNICODE_ERROR, "Invalid PARSE source text")
    }
    plan = GETNATIVEPAYLOAD(ARG1, &plan_length, NULL, NULL);
    owner = calloc(1, sizeof(*owner));
    if (!owner) { RETURNSIGNAL(SIGNAL_FAILURE, "Cannot allocate PARSE cursor") }
    owner->cursor = rxvm_parse_create(source, source_length, plan, plan_length, &status);
    if (!owner->cursor) {
        free(owner); signal_parse(SIGNAL, status, NULL); return;
    }
    owner->session = current; owner->refs = 1; ++current->refs;
    if (SETNATIVEPAYLOAD(RETURN, &owner, sizeof(owner), &cursor_ops, 0)) {
        release_resource(owner);
        RETURNSIGNAL(SIGNAL_FAILURE, "Cannot publish PARSE cursor payload")
    }
    if (SETOBJECTTYPE(current->host, RETURN, "rxparse.cursor")) {
        (void)SETNATIVEPAYLOAD(RETURN, NULL, 0, NULL, 0);
        RETURNSIGNAL(SIGNAL_FAILURE, "Cannot publish PARSE cursor type")
    }
    RESETSIGNAL
}

#define CURSOR_RESOURCE(expected) \
    parse_resource *owner = resource(ARG0); \
    if (!valid_resource(owner)) { \
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Foreign or forged PARSE cursor") \
    } \
    if (NUM_ARGS != (expected)) { \
        signal_parse(SIGNAL, rxvm_parse_fail(owner->cursor, RXVM_PARSE_INVALID), owner); return; \
    }

METHODPROCEDURE(cursor_next) {
    int result;
    CURSOR_RESOURCE(1)
    result = rxvm_parse_next(owner->cursor);
    if (result < 0) { signal_parse(SIGNAL, result, owner); return; }
    SETINT(RETURN, result); RESETSIGNAL
}
METHODPROCEDURE(cursor_item) {
    CURSOR_RESOURCE(1)
    SETINT(RETURN, (rxinteger)rxvm_parse_item(owner->cursor)); RESETSIGNAL
}
METHODPROCEDURE(cursor_slot) {
    CURSOR_RESOURCE(1)
    SETINT(RETURN, (rxinteger)rxvm_parse_slot(owner->cursor)); RESETSIGNAL
}
METHODPROCEDURE(cursor_text) {
    const char *text;
    size_t length;
    int result;
    CURSOR_RESOURCE(1)
    result = rxvm_parse_text(owner->cursor, &text, &length);
    if (!result && SETSTRINGLENGTH(current->host, RETURN, text, length))
        result = rxvm_parse_fail(owner->cursor, RXVM_PARSE_NOMEM);
    if (result < 0) { signal_parse(SIGNAL, result, owner); return; }
    RESETSIGNAL
}
METHODPROCEDURE(cursor_supply) {
    const char *text;
    size_t length;
    int result;
    CURSOR_RESOURCE(2)
    (void)RETURN;
    if (!ARG1) {
        signal_parse(SIGNAL, rxvm_parse_fail(owner->cursor, RXVM_PARSE_INVALID), owner); return;
    }
    if (current->host->string_view(ARG1, &text, &length))
        result = rxvm_parse_fail(owner->cursor, RXVM_PARSE_UNICODE);
    else result = rxvm_parse_supply(owner->cursor, text, length);
    if (result < 0) { signal_parse(SIGNAL, result, owner); return; }
    RESETSIGNAL
}
METHODPROCEDURE(cursor_close) {
    CURSOR_RESOURCE(1)
    (void)RETURN;
    rxvm_parse_close(owner->cursor); RESETSIGNAL
}

#define PARSE_CLASS(owner) ADDCLASS(owner);
#define PARSE_FACTORY(fn, owner, type, args) ADDFACTORYPROC(fn, owner, type, args);
#define PARSE_METHOD(fn, owner, member, type, args) ADDMETHODPROC(fn, owner, member, type, args);
static void register_cursor(void) {
    UNIQUE_CAPABILITY_FUNCTION_NAME(PLUGIN_ID)();
#endif
    PARSE_CLASS("rxparse.cursor")
    PARSE_FACTORY(make_cursor, "rxparse.cursor", ".rxparse..cursor", "source=.string,plan=.binary")
    PARSE_METHOD(cursor_next, "rxparse.cursor", "next", ".int", "")
    PARSE_METHOD(cursor_item, "rxparse.cursor", "item", ".int", "")
    PARSE_METHOD(cursor_slot, "rxparse.cursor", "slot", ".int", "")
    PARSE_METHOD(cursor_text, "rxparse.cursor", "text", ".string", "")
    PARSE_METHOD(cursor_supply, "rxparse.cursor", "supply", ".void", "text=.string")
    PARSE_METHOD(cursor_close, "rxparse.cursor", "close", ".void", "")
#ifdef DECL_ONLY
};
const size_t rxvm_parse_cursor_binding_count =
    sizeof(rxvm_parse_cursor_bindings) / sizeof(rxvm_parse_cursor_bindings[0]);
#else
}
#if defined(CREXX_VM_SINGLE_THREADED)
void rxvm_parse_cursor_init(void) {
    static int initialized;
    if (!initialized) { register_cursor(); initialized = 1; }
}
#elif defined(_WIN32)
static INIT_ONCE cursor_once = INIT_ONCE_STATIC_INIT;
static BOOL CALLBACK initialize_cursor(PINIT_ONCE once, PVOID parameter, PVOID *context) {
    (void)once; (void)parameter; (void)context;
    register_cursor(); return TRUE;
}
void rxvm_parse_cursor_init(void) {
    (void)InitOnceExecuteOnce(&cursor_once, initialize_cursor, NULL, NULL);
}
#else
static pthread_once_t cursor_once = PTHREAD_ONCE_INIT;
void rxvm_parse_cursor_init(void) { (void)pthread_once(&cursor_once, register_cursor); }
#endif
#endif
