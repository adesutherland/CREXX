/* SPDX-License-Identifier: MIT. Execution does not require an IBM Rexx ENVBLOCK. */
#include "platform_config.h"
#if defined(CREXX_MAINFRAME_ADDRESS) && defined(CREXX_PLATFORM_TSO)
#include "mainframe_command_internal.h"
#include <command-internal.h>
unsigned crexx_command_native_capabilities(crexx_command_handle *h,const char *name) {
    (void)h; (void)name; return 0;
}
int crexx_command_native_capture(crexx_command_handle *h,const char *name,const char *text,unsigned length,
                                 const crexx_command_io *io,MfCommandResult *r) {
    (void)h; (void)name; (void)text; (void)length; (void)io;
    mf_command_result_init(r); return MF_COMMAND_UNSUPPORTED;
}
#include <string.h>
const char *crexx_command_profile(void) { return "TSO"; }
int crexx_command_native_query(crexx_command_handle *h,const char *name,int *present,MfCommandResult *r) {
    MfCommandSession session={0,0}; int status;
    (void)h;
    status=mf_command_open(&session,r);
    if(status==MF_COMMAND_NO_ENVIRONMENT) { *present=0; return 0; }
    if(!status) status=mf_command_query(&session,name,present,r);
    if(session.active) mf_command_close(&session);
    return status;
}
int crexx_command_native_execute(crexx_command_handle *h,const char *name,const char *text,unsigned length,MfCommandResult *r) {
    (void)h;
    if(!strcmp(name,"TSO")) return mf_command_host_execute(text,length,r);
    if(!strcmp(name,"PDOS")) {
        MfCommandSession session={0,0}; int status=mf_command_open(&session,r);
        if(!status) status=mf_pdos_command_execute(&session,text,length,r);
        if(session.active) mf_command_close(&session);
        return status;
    }
    memset(r,0,sizeof(*r)); r->service_rc=-1;
    return MF_COMMAND_UNSUPPORTED;
}
#endif
