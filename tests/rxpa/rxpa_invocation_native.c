/* cREXX License (MIT). Native controls for checked ordinary invocation. */
#include "crexxpa.h"
#ifndef DECL_ONLY
#if defined(_WIN32)
#include <windows.h>
static volatile LONG retained_payloads;
static void payload_retain(void) { InterlockedIncrement(&retained_payloads); }
static void payload_release(void) { InterlockedDecrement(&retained_payloads); }
static rxinteger payload_count(void) { return InterlockedCompareExchange(&retained_payloads,0,0); }
#else
static volatile int retained_payloads;
static void payload_retain(void) { (void)__atomic_add_fetch(&retained_payloads,1,__ATOMIC_SEQ_CST); }
static void payload_release(void) { (void)__atomic_sub_fetch(&retained_payloads,1,__ATOMIC_SEQ_CST); }
static rxinteger payload_count(void) { return __atomic_load_n(&retained_payloads,__ATOMIC_SEQ_CST); }
#endif
static void copy_payload(void *, void *);
static void finalize_payload(void *value) { (void)value; payload_release(); }
static const rxpa_native_payload_ops payload_ops = {"rxinvoketest.retained",copy_payload,finalize_payload};
static void copy_payload(void *destination, void *source) {
    size_t length;
    const void *bytes=GETNATIVEPAYLOAD(source,&length,NULL,NULL);
    if (!SETNATIVEPAYLOAD(destination,bytes,length,&payload_ops,0)) payload_retain();
}
static int tracked_result(void *destination) {
    static const unsigned char marker=1;
    if (SETNATIVEPAYLOAD(destination,&marker,sizeof(marker),&payload_ops,0)) return -1;
    payload_retain(); return 0;
}
RXPA_PLUGIN_PROCESS_REENTRANT
PROCEDURE(native_integer) {
    SETINT(RETURN, GETINT(ARG0) + 7); RESETSIGNAL
}
PROCEDURE(native_text) {
    SETINT(RETURN, 17); SETSTRING(RETURN, "native text"); RESETSIGNAL
}
PROCEDURE(native_failure) {
    (void)NUM_ARGS; (void)_arg;
    SETINT(RETURN, SIGNAL_DIVISION_BY_ZERO);
    if (tracked_result(RETURN)) { RETURNSIGNAL(SIGNAL_FAILURE,"Cannot track failed return") }
    RETURNSIGNAL(SIGNAL_DIVISION_BY_ZERO, "owned native invocation event")
}
PROCEDURE(native_named_failure) {
    (void)NUM_ARGS; (void)_arg;
    SETINT(RETURN, 99);
    if (tracked_result(RETURN)) { RETURNSIGNAL(SIGNAL_FAILURE,"Cannot track named failed return") }
    SETINT(SIGNAL, 0); SETSTRING(SIGNAL, "DIVISION_BY_ZERO");
}
PROCEDURE(native_boolean) {
    (void)NUM_ARGS; (void)_arg;
    SETINT(RETURN, 2);
    if (tracked_result(RETURN)) { RETURNSIGNAL(SIGNAL_FAILURE,"Cannot track invalid return") }
    RESETSIGNAL
}
PROCEDURE(native_void) {
    (void)NUM_ARGS; (void)_arg;
    if (tracked_result(RETURN)) { RETURNSIGNAL(SIGNAL_FAILURE,"Cannot track void return") }
    RESETSIGNAL
}
PROCEDURE(native_retained) {
    (void)NUM_ARGS; (void)_arg;
    SETINT(RETURN,payload_count()); RESETSIGNAL
}
#endif
LOADFUNCS
ADDPROC(native_integer,"rxinvoketest.integer","b",".int","value=.int");
ADDPROC(native_text,"rxinvoketest.text","b",".string","");
ADDPROC(native_failure,"rxinvoketest.failure","b",".int","");
ADDPROC(native_boolean,"rxinvoketest.boolean","b",".boolean","");
ADDPROC(native_named_failure,"rxinvoketest.namedfailure","b",".int","");
ADDPROC(native_void,"rxinvoketest.novalue","b",".void","");
ADDPROC(native_retained,"rxinvoketest.retained","b",".int","");
ENDLOADFUNCS
