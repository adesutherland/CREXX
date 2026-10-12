/* SPDX-License-Identifier: MIT */
#include "platform_config.h"
#if defined(CREXX_MAINFRAME_ADDRESS)
#include "mainframe_command_internal.h"
#include "text_codec.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
crexx_command_handle *crexx_command_open(void) {
    return (crexx_command_handle *)calloc(1,sizeof(crexx_command_handle));
}
void crexx_command_close(crexx_command_handle *h) {
    if(h) { if(h->discovery.active) mf_command_close(&h->discovery); free(h); }
}
int crexx_command_name(const char *text,size_t length,char name[9]) {
    size_t start=0,i; unsigned char c;
    if(!text || !name) return -1;
    while(start<length && text[start]==' ') ++start;
    while(length>start && text[length-1]==' ') --length;
    length-=start;
    if(!length || length>8) return -1;
    for(i=0;i<length;++i) {
        c=(unsigned char)text[start+i];
        if(c>='a' && c<='z') c-= 'a'-'A';
        if(!((c>='A' && c<='Z') || (c>='0' && c<='9') || c=='@' || c=='$' || c=='#')) return -1;
        name[i]=(char)c;
    }
    name[length]=0; return 0;
}
static int alias(const char *name) {
    return !strcmp(name,"SYSTEM") || !strcmp(name,"COMMAND") || !strcmp(name,"CMD");
}
int crexx_command_match(const char *text,size_t length) {
    char name[9];
    if(crexx_command_name(text,length,name)) return 0;
    if(alias(name) || !strcmp(name,crexx_command_profile())) return 200;
    if(!strcmp(name,"CREXX") || !strcmp(name,"SHELL") || !strcmp(name,"PATH")) return 0;
    return 50;
}
static void initial(crexx_command_result *r) {
    memset(r,0,sizeof(*r)); r->service_rc=-1;
}
static int failure(crexx_command_result *r,int status,const char *message) {
    r->rc=-3; r->condition=CREXX_COMMAND_FAILURE; r->status=status;
    snprintf(r->diagnostic,sizeof(r->diagnostic),"%s",message); return status;
}
static void native_result(crexx_command_result *r,int status,const MfCommandResult *n) {
    char reason[24],abend[24];
    r->status=status; r->service_rc=n->service_rc; r->reason_rc=n->reason_rc;
    r->module_rc=n->module_rc; r->abend_code=n->abend_code;
    r->command_rc_valid=n->command_rc_valid;
    if(!status && n->command_rc_valid) {
        r->rc=n->command_rc;
        r->condition=n->command_rc<0 ? CREXX_COMMAND_FAILURE :
                     n->command_rc>0 ? CREXX_COMMAND_ERROR : CREXX_COMMAND_NORMAL;
    } else { r->rc=-3; r->condition=CREXX_COMMAND_FAILURE; }
    if(status || r->condition!=CREXX_COMMAND_NORMAL) {
        /* The TSO facility may leave these outputs at their initial sentinels
         * on a normal nonzero command. Do not present them as a real ABEND. */
        if(n->reason_rc!=-1) snprintf(reason,sizeof(reason),"%d",n->reason_rc);
        else snprintf(reason,sizeof(reason),"not reported");
        if(n->abend_code && n->abend_code!=~0u) snprintf(abend,sizeof(abend),"%X",n->abend_code);
        else snprintf(abend,sizeof(abend),"not reported");
        snprintf(r->diagnostic,sizeof(r->diagnostic),
            "Native %s command: status=%d service=%d command=%d valid=%u reason=%s module=%d abend=%s",
            crexx_command_profile(),status,n->service_rc,n->command_rc,n->command_rc_valid,
            reason,n->module_rc,abend);
    }
}
int crexx_command_query(crexx_command_handle *h,const char *name,int *present,crexx_command_result *r) {
    MfCommandResult n; int status; char normalized[9];
    if(!r) return MF_COMMAND_INVALID;
    initial(r);
    if(!h || !name || !present || crexx_command_name(name,strlen(name),normalized))
        return failure(r,MF_COMMAND_INVALID,"Invalid native command handle/name");
    *present=0;
    if(alias(normalized) || !strcmp(normalized,crexx_command_profile())) { *present=1; return 0; }
    status=crexx_command_native_query(h,normalized,present,&n);
    if(status) native_result(r,status,&n);
    else r->service_rc=n.service_rc;
    return status;
}
unsigned crexx_command_capabilities(crexx_command_handle *h,const char *name) {
    char normalized[9];
    if(!h || !name || crexx_command_name(name,strlen(name),normalized)) return 0;
    return crexx_command_native_capabilities(h,alias(normalized) ? crexx_command_profile():normalized);
}
static int valid_text(const char *text,size_t length) {
    const uint32_t *map; crexx_utf8_state state={0,0,0};
    uint32_t scalar; unsigned char bytes[4]; size_t i; int step;
    if(!text || !length || length>MF_COMMAND_MAX_BYTES || platform_text_codec_lookup("IBM1047",&map)) return 0;
    for(i=0;i<length;++i) {
        step=crexx_utf8_feed(&state,(unsigned char)text[i],&scalar);
        if(step<0 || (step && (!scalar || crexx_text_encode(map,scalar,bytes)<0))) return 0;
    }
    return !crexx_utf8_finish(&state);
}
int crexx_command_execute(crexx_command_handle *h,const char *name,const char *text,size_t length,
                          const crexx_command_io *io,crexx_command_result *r) {
    MfCommandResult n; int status,present; char normalized[9];
    if(!r) return MF_COMMAND_INVALID;
    initial(r);
    if(!h || !name || crexx_command_name(name,strlen(name),normalized) || !valid_text(text,length))
        return failure(r,MF_COMMAND_INVALID,"Invalid native command text/name (IBM1047, no NUL, maximum 32767 UTF-8 bytes)");
    if(io && (io->requested & ~crexx_command_capabilities(h,normalized)))
        return failure(r,MF_COMMAND_UNSUPPORTED,"Requested native command input/capture is unavailable; command was not executed");
    status=crexx_command_query(h,normalized,&present,r);
    if(status) return status;
    if(!present) {
        char message[96]; snprintf(message,sizeof(message),"Unknown address environment %s",normalized);
        return failure(r,MF_COMMAND_NO_ENVIRONMENT,message);
    }
    if(io && io->requested)
        status=crexx_command_native_capture(h,alias(normalized) ? crexx_command_profile():normalized,text,(unsigned)length,io,&n);
    else status=crexx_command_native_execute(h,alias(normalized) ? crexx_command_profile():normalized,text,(unsigned)length,&n);
    native_result(r,status,&n); return status;
}
#endif
