/* Public Level B RXPA surface. Each VM owns its console lease and cleanup. */
#include "crexxpa.h"
#include "rxconsole_core.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
typedef struct console_session { rxcon_terminal *terminal; rxinteger generation; } console_session;
#ifdef _MSC_VER
#define CONSOLE_TLS __declspec(thread)
#else
#define CONSOLE_TLS __thread
#endif
static CONSOLE_TLS console_session *current;
static console_session legacy;
static console_session *session(void) {return current?current:&legacy;}
static void *create_session(void) {return calloc(1,sizeof(console_session));}
static void destroy_session(void *value) {
    console_session *s=(console_session *)value;
    if(s){if(s->terminal)rxcon_close(s->terminal);free(s);}
}
static int enter_session(void *value,uint32_t flags,void **previous) {
    (void)flags;*previous=current;current=(console_session *)value;return 0;
}
static void leave_session(void *previous) {current=(console_session *)previous;}
static uint32_t capabilities(const char *name) {(void)name;return RXPA_PROCEDURE_CAP_SESSION_AFFINE;}
RXPA_PLUGIN_SESSION_AWARE(create_session,destroy_session,enter_session,leave_session,capabilities)
static rxcon_terminal *lookup(rxinteger handle) {
    console_session *s=session();return handle>0 && handle==s->generation?s->terminal:NULL;
}
PROCEDURE(console_open) {
    console_session *s=session();rxinteger modes=GETINT(ARG0);int rc;
    if(modes<0 || modes>15){RETURNINT(RXCON_INVALID);RESETSIGNAL return;}
    if(s->terminal){RETURNINT(RXCON_BUSY);RESETSIGNAL return;}
    if(s->generation==INT_MAX){RETURNINT(RXCON_LIMIT);RESETSIGNAL return;}
    rc=rxcon_open(&s->terminal,(int)modes);
    if(!rc)++s->generation;
    RETURNINT(rc<0?rc:s->generation);RESETSIGNAL
}
PROCEDURE(console_close) {
    console_session *s=session();rxcon_terminal *t=lookup(GETINT(ARG0));int rc=RXCON_INVALID;
    if(t){rc=rxcon_close(t);s->terminal=NULL;}
    RETURNINT(rc);RESETSIGNAL
}
PROCEDURE(console_modes) {
    rxinteger modes=GETINT(ARG1);
    RETURNINT(modes<0 || modes>15?RXCON_INVALID:rxcon_modes(lookup(GETINT(ARG0)),(int)modes));RESETSIGNAL
}
PROCEDURE(console_size) {
    int columns=0,rows=0,rc=rxcon_size(lookup(GETINT(ARG0)),&columns,&rows);
    SETINT(ARG1,columns);SETINT(ARG2,rows);RETURNINT(rc);RESETSIGNAL
}
PROCEDURE(console_caps) {
    rxinteger handle=GETINT(ARG0);
    RETURNINT(handle==0?rxcon_profile_capabilities():rxcon_capabilities(lookup(handle)));RESETSIGNAL
}
PROCEDURE(console_write) {
    const char *text=GETSTRING(ARG1);
    RETURNINT(rxcon_write(lookup(GETINT(ARG0)),text,strlen(text)));RESETSIGNAL
}
PROCEDURE(console_poll) {
    rxcon_event event;rxinteger timeout=GETINT(ARG1);int i,rc;
    memset(&event,0,sizeof(event));
    rc=timeout < -1 || timeout>INT_MAX ? RXCON_INVALID : rxcon_poll(lookup(GETINT(ARG0)),(int)timeout,&event);
    if(rc<=0)memset(&event,0,sizeof(event));
    SETARRAYHI(ARG2,10);for(i=0;i<10;++i)SETIARRAY(ARG2,i,event.fields[i]);
    SETSTRING(ARG3,event.text);RETURNINT(rc);RESETSIGNAL
}
LOADFUNCS
    ADDPROC(console_open,"rxconsole.open","b",".int","modes=.int");
    ADDPROC(console_close,"rxconsole.close","b",".int","handle=.int");
    ADDPROC(console_modes,"rxconsole.modes","b",".int","handle=.int,modes=.int");
    ADDPROC(console_size,"rxconsole.size","b",".int","handle=.int,expose columns=.int,expose rows=.int");
    ADDPROC(console_caps,"rxconsole.capabilities","b",".int","handle=.int");
    ADDPROC(console_write,"rxconsole.write","b",".int","handle=.int,text=.string");
    ADDPROC(console_poll,"rxconsole.poll","b",".int","handle=.int,timeout=.int,expose fields=.int[],expose text=.string");
ENDLOADFUNCS
