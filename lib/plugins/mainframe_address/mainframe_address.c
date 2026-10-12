/* SPDX-License-Identifier: MIT
 * Static mainframe ADDRESS provider. The common Rexx controller owns redirect
 * receivers/finish and Classic conditions. Native resources are VM affine. */
#include "crexxpa.h"
#include "mainframe_command.h"
#include "rxaddress_layout.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Native startup does not run compiler constructors. Register this immutable
 * provider catalogue explicitly from the shared standalone/embedding entry. */
#if defined(CREXX_VM_SINGLE_THREADED)
#undef INITIALIZER
#define INITIALIZER(f) static void f(void) {
#endif
#ifndef DECL_ONLY
typedef struct mf_session { const rxpa_host_services_v1 *host; size_t refs; } mf_session;
typedef struct mf_environment {
    mf_session *owner; size_t refs; char name[9]; crexx_command_handle *driver;
} mf_environment;
#if defined(CREXX_VM_SINGLE_THREADED)
#define MF_LOCAL
#elif defined(_MSC_VER)
#define MF_LOCAL __declspec(thread)
#else
#define MF_LOCAL __thread
#endif
static MF_LOCAL mf_session *current;
static void *create(const rxpa_host_services_v1 *host) {
    mf_session *s;
    if(!rxpa_host_has_object_set_type(host) || !rxpa_host_has_string_view(host) || !rxpa_host_has_string_set(host)) return NULL;
    s=(mf_session *)calloc(1,sizeof(*s));
    if(s) { s->host=host; s->refs=1; } return s;
}
static void *old_create(void) { return NULL; }
static void destroy(void *opaque) { mf_session *s=(mf_session *)opaque; if(s && !--s->refs) free(s); }
static int enter(void *s,uint32_t caps,void **previous) { (void)caps; *previous=current; current=(mf_session *)s; return 0; }
static void leave(void *previous) { current=(mf_session *)previous; }
static uint32_t capabilities(const char *name) { (void)name; return RXPA_PROCEDURE_CAP_SESSION_AFFINE; }
RXPA_PLUGIN_SESSION_WITH_HOST(old_create,destroy,enter,leave,capabilities,create)
static void copy(void *,void *);
static void finalize(void *);
static const rxpa_native_payload_ops payload_ops={"rxmainframe.environment",copy,finalize};
static mf_environment *payload(rxpa_attribute_value value) {
    size_t n=0; const rxpa_native_payload_ops *ops=NULL; mf_environment *e=NULL;
    void *p=GETNATIVEPAYLOAD(value,&n,&ops,NULL);
    if(p && n==sizeof(e) && ops==&payload_ops) memcpy(&e,p,n);
    return e;
}
static void release(mf_environment *e) {
    if(e && !--e->refs) { crexx_command_close(e->driver); destroy(e->owner); free(e); }
}
static void copy(void *destination,void *source) {
    mf_environment *e=payload(source);
    if(e) { ++e->refs; if(SETNATIVEPAYLOAD(destination,&e,sizeof(e),&payload_ops,0)) release(e); }
}
static void finalize(void *value) { release(payload(value)); }
PROCEDURE(match_environment) {
    const char *text; size_t n;
    if(current->host->string_view(ARG0,&text,&n)) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Invalid ADDRESS environment name") }
    SETINT(RETURN,crexx_command_match(text,n)); RESETSIGNAL
}
PROCEDURE(make_environment) {
    const char *text; size_t n; mf_environment *e;
    e=(mf_environment *)calloc(1,sizeof(*e));
    if(!e) { RETURNSIGNAL(SIGNAL_FAILURE,"Native ADDRESS allocation failed") }
    if(current->host->string_view(ARG0,&text,&n) || crexx_command_name(text,n,e->name)) {
        free(e); RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Invalid native ADDRESS environment name")
    }
    e->owner=current; e->refs=1; e->driver=crexx_command_open();
    if(!e->driver) { free(e); RETURNSIGNAL(SIGNAL_FAILURE,"Native command allocation failed") }
    ++current->refs;
    if(SETNATIVEPAYLOAD(RETURN,&e,sizeof(e),&payload_ops,0)) {
        release(e); RETURNSIGNAL(SIGNAL_FAILURE,"Native ADDRESS payload allocation failed")
    }
    if(SETOBJECTTYPE(current->host,RETURN,"rxmainframe.environment")) {
        RETURNSIGNAL(SIGNAL_FAILURE,"Cannot publish native ADDRESS environment")
    }
    RESETSIGNAL
}
#define RECEIVER mf_environment *e=payload(ARG0); if(!e || e->owner!=current) { RETURNSIGNAL(SIGNAL_FAILURE,"Invalid native ADDRESS owner") }
METHODPROCEDURE(environment_name) { RECEIVER SETSTRING(RETURN,e->name); RESETSIGNAL }
METHODPROCEDURE(environment_id) { RECEIVER SETSTRING(RETURN,e->name); RESETSIGNAL }
typedef struct mf_emit {
    const rxpa_host_services_v1 *host;
    rxpa_attribute_value endpoint,argument,result,signal;
    int failed;
} mf_emit;
static int emit_output(void *opaque,const char *text,size_t length) {
    mf_emit *s=(mf_emit *)opaque;
    rxpa_attribute_value args[1]; args[0]=s->argument;
    if(SETSTRINGLENGTH(s->host,s->argument,text,length)) return -1;
    if(CALLMETHODX(s->endpoint,"rxsig1|native_emit|.int|text=.string",1,args,s->result,s->signal)) {
        s->failed=1; return -1;
    }
    return GETINT(s->result)==1 ? 0:-1;
}
METHODPROCEDURE(execute_environment) {
    RECEIVER
    const char *view; char *text; size_t n; unsigned requested=0;
    rxpa_attribute_value request=ARG1, scratch, endpoint;
    crexx_command_io io; crexx_command_result result; mf_emit output;
    if(GETNUMATTRS(request)<RXVML_ADDRESS_REQUEST_ATTR_COUNT ||
       current->host->string_view(GETATTR(request,RXVML_ADDRESS_REQUEST_COMMAND),&view,&n)) {
        char diagnostic[96];
        snprintf(diagnostic,sizeof(diagnostic),"Invalid ADDRESS request: argc=%ld attributes=%ld",(long)NUM_ARGS,(long)GETNUMATTRS(request));
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,diagnostic)
    }
    text=(char *)malloc(n ? n:1);
    if(!text) { RETURNSIGNAL(SIGNAL_FAILURE,"Native command text allocation failed") }
    memcpy(text,view,n);
    SETNUMATTRS(RETURN,RXVML_ADDRESS_RESPONSE_ATTR_COUNT);
    scratch=GETATTR(RETURN,RXVML_ADDRESS_RESPONSE_DIAGNOSTIC_COUNT);
    endpoint=GETATTR(request,RXVML_ADDRESS_REQUEST_STDIN_ENDPOINT);
    if(CALLMETHOD(endpoint,"rxsig1|has_input|.boolean|",0,NULL,scratch)) { free(text); return; }
    if(GETINT(scratch)) requested|=CREXX_COMMAND_INPUT;
    endpoint=GETATTR(request,RXVML_ADDRESS_REQUEST_STDOUT_ENDPOINT);
    if(CALLMETHOD(endpoint,"rxsig1|captures_output|.boolean|",0,NULL,scratch)) { free(text); return; }
    if(GETINT(scratch)) requested|=CREXX_COMMAND_CAPTURE_OUTPUT;
    memset(&output,0,sizeof(output)); output.host=current->host;
    output.endpoint=endpoint; output.result=scratch; output.signal=SIGNAL;
    SETNUMATTRS(GETATTR(RETURN,RXVML_ADDRESS_RESPONSE_UPDATED_BINDINGS),1);
    output.argument=GETATTR(GETATTR(RETURN,RXVML_ADDRESS_RESPONSE_UPDATED_BINDINGS),0);
    endpoint=GETATTR(request,RXVML_ADDRESS_REQUEST_STDERR_ENDPOINT);
    if(CALLMETHOD(endpoint,"rxsig1|captures_output|.boolean|",0,NULL,scratch)) { free(text); return; }
    if(GETINT(scratch)) requested|=CREXX_COMMAND_CAPTURE_ERROR;
    memset(&io,0,sizeof(io)); io.requested=requested;
    io.output=emit_output; io.owner=&output;
    crexx_command_execute(e->driver,e->name,text,n,&io,&result);
    free(text);
    if(output.failed) return;
    SETINT(GETATTR(RETURN,RXVML_ADDRESS_RESPONSE_RC),result.rc);
    SETSTRING(GETATTR(RETURN,RXVML_ADDRESS_RESPONSE_CONDITION_NAME),
        result.condition==CREXX_COMMAND_ERROR ? "ERROR":result.condition==CREXX_COMMAND_FAILURE ? "FAILURE":"");
    SETINT(scratch,result.diagnostic[0] ? 1:0);
    SETNUMATTRS(GETATTR(RETURN,RXVML_ADDRESS_RESPONSE_DIAGNOSTICS),result.diagnostic[0] ? 1:0);
    if(result.diagnostic[0]) SETSTRING(GETATTR(GETATTR(RETURN,RXVML_ADDRESS_RESPONSE_DIAGNOSTICS),0),result.diagnostic);
    SETINT(GETATTR(RETURN,RXVML_ADDRESS_RESPONSE_UPDATED_BINDING_COUNT),0);
    SETNUMATTRS(GETATTR(RETURN,RXVML_ADDRESS_RESPONSE_UPDATED_BINDINGS),0);
    if(SETOBJECTTYPE(current->host,RETURN,"_rxsysb.addressresponse")) {
        RETURNSIGNAL(SIGNAL_FAILURE,"Cannot publish ADDRESS response")
    }
    RESETSIGNAL
}
#endif
LOADFUNCS
ADDCLASS("rxmainframe.environment");
ADDIMPLEMENTS("rxmainframe.environment","_rxsysb.addressenvironment");
ADDFACTORYPROC(make_environment,"rxmainframe.environment",".rxmainframe..environment","env_name=.string");
ADDMATCHPROC(match_environment,"rxmainframe.environment","env_name=.string");
ADDMETHODPROC(environment_name,"rxmainframe.environment","environment_name",".string","");
ADDMETHODPROC(environment_id,"rxmainframe.environment","environment_id",".string","");
ADDMETHODPROC(execute_environment,"rxmainframe.environment","execute","._rxsysb..addressresponse","request=.addressrequest");
ENDLOADFUNCS
#if defined(CREXX_VM_SINGLE_THREADED)
void crexx_mainframe_address_init(void) {
    static int initialized;
    if(initialized) return;
    initialized=1;
#ifndef DECL_ONLY
    UNIQUE_CAPABILITY_FUNCTION_NAME(PLUGIN_ID)();
#endif
    UNIQUE_INIT_FUNCTION_NAME(PLUGIN_ID)();
}
#endif
