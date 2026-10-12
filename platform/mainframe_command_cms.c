/* SPDX-License-Identifier: MIT. CMS policy over the maintained SDK ABI. */
#include "platform_config.h"
#if defined(CREXX_MAINFRAME_ADDRESS) && defined(CREXX_PLATFORM_CMS)
#include "mainframe_command_internal.h"
#include <command-internal.h>
#include "text_codec.h"
#include <cms/adapter.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
const char *crexx_command_profile(void) { return "CMS"; }
int crexx_command_native_query(crexx_command_handle *h,const char *name,int *present,MfCommandResult *r) {
    int status=0;
    if(!h->discovery.active) status=mf_command_open(&h->discovery,r);
    return status ? status:mf_command_query(&h->discovery,name,present,r);
}
int crexx_command_native_execute(crexx_command_handle *h,const char *name,const char *text,unsigned length,MfCommandResult *r) {
    int status=0;
    if(!h->discovery.active) status=mf_command_open(&h->discovery,r);
    return status ? status:mf_command_execute(&h->discovery,name,text,length,r);
}
unsigned crexx_command_native_capabilities(crexx_command_handle *h,const char *name) {
    MfCommandResult r; int pdos=0;
    if(strcmp(name,"CMS") || crexx_command_native_query(h,"PDOS",&pdos,&r) || pdos) return 0;
    return CREXX_COMMAND_CAPTURE_OUTPUT;
}
/* This qualification covers QUERY DISK and an optional one-letter disk mode.
 * CMS commands which bypass the pipeline stage (including QUERY TIME) must
 * not report successful empty capture. The bounded grammar also excludes
 * pipeline separators and substitutions before any native side effect. */
static int query_disk(const char *text,unsigned n) {
    char words[3][9]; unsigned i=0,w=0,k;
    while(i<n) {
        while(i<n && text[i]==' ') ++i;
        if(i==n) break;
        if(w==3) return 0;
        k=0;
        while(i<n && text[i]!=' ') {
            unsigned char c=(unsigned char)text[i++];
            if(c>='a' && c<='z') c-= 'a'-'A';
            if(k==8) return 0;
            words[w][k++]=(char)c;
        }
        words[w++][k]=0;
    }
    return w>=2 && !strcmp(words[0],"QUERY") && !strcmp(words[1],"DISK") &&
           (w==2 || (strlen(words[2])==1 && words[2][0]>='A' && words[2][0]<='Z'));
}
int crexx_command_native_capture(crexx_command_handle *h,const char *name,const char *text,unsigned length,
                                 const crexx_command_io *io,MfCommandResult *r) {
    unsigned char id[18],fst[40], *raw=NULL,*utf8=NULL;
    char file[32],pipeline[128]; LabCmsFile f;
    const uint32_t *codec; unsigned attempt; size_t written;
    int status=MF_COMMAND_NATIVE_ERROR,rc=0,open=0,owned=0,n=0,cleanup=0;
    mf_command_result_init(r);
    if(strcmp(name,"CMS") || !io || io->requested!=CREXX_COMMAND_CAPTURE_OUTPUT ||
       !io->output || length>80 || !query_disk(text,length)) return MF_COMMAND_UNSUPPORTED;
    raw=(unsigned char *)malloc(65535); utf8=(unsigned char *)malloc(65535*4u+1u);
    if(!raw || !utf8) { status=MF_COMMAND_NO_MEMORY; goto done; }
    if(platform_text_codec_lookup("IBM1047",&codec)) goto done;
    /* CMS executes synchronously in this address space. STATE + reservation
     * own a private file through readback and cleanup; never erase a collision.
     * We make no shared-disk cross-user atomic-create claim. */
    for(attempt=0;attempt<64;++attempt) {
        h->capture_serial=h->capture_serial%999999u+1u;
        snprintf(file,sizeof(file),"RX%06u CMDOUT A",h->capture_serial);
        if(lab_cms_name(file,id)) goto done;
        rc=lab_cms_state(id,fst);
        if(!rc) continue;
        if(rc!=28) goto done;
        rc=lab_cms_open(&f,id,raw,65535);
        if(rc!=28) goto done;
        open=1; owned=1; raw[0]=0x40;
        rc=lab_cms_write(&f,1);
        if(rc) goto done;
        rc=lab_cms_close(&f);
        if(rc) goto done;
        open=0;
        break;
    }
    if(!owned) goto done;
    snprintf(pipeline,sizeof(pipeline),"PIPE (SEP !) CMS %.*s ! > %s",(int)length,text,file);
    status=crexx_command_native_execute(h,"CMS",pipeline,(unsigned)strlen(pipeline),r);
    if(status || !r->command_rc_valid || r->command_rc) goto done;
    rc=lab_cms_open_text(&f,id,raw,65535);
    if(rc) { status=MF_COMMAND_NATIVE_ERROR; goto done; }
    open=1;
    while(!(rc=lab_cms_read(&f,&n))) {
        if(n<0 || n>65535 || crexx_text_convert(codec,1,raw,(size_t)n,utf8,65535*4u,&written)) {
            status=MF_COMMAND_NATIVE_ERROR; goto done;
        }
        utf8[written++]='\n';
        if(io->output(io->owner,(const char *)utf8,written)) {
            status=MF_COMMAND_NATIVE_ERROR; goto done;
        }
    }
    if(rc!=12) status=MF_COMMAND_NATIVE_ERROR;
done:
    if(open) cleanup=lab_cms_close(&f);
    if(owned) { int erase=lab_cms_erase(id); if(erase==28) erase=0; if(!cleanup) cleanup=erase; }
    if(cleanup) status=MF_COMMAND_NATIVE_ERROR;
    if(status && r->service_rc==0) r->service_rc=rc ? rc:cleanup;
    free(utf8); free(raw); return status;
}
#endif
