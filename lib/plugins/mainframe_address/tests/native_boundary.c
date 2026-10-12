/* SPDX-License-Identifier: MIT
 * Deterministic native boundary only. This is not a CMS/TSO emulator or proof
 * of native linkage/capture; the same driver/provider must run on guests. */
#include "platform_config.h"
#include "command-internal.h"
#include <string.h>
#include <limits.h>
#include <stdlib.h>
int native_test_pdos;
#if defined(CREXX_PLATFORM_TSO)
static void *parameter(unsigned long *p,unsigned i) { return (void *)(p[i] & ~(1UL<<(sizeof(unsigned long)*CHAR_BIT-1))); }
int mf_tso_command_call(const unsigned char name[8],unsigned long *p,unsigned long env,int *load_rc) {
    unsigned char expected[8]; (void)env; *load_rc=0;
    mf_command_name("IRXINIT",expected);
    if(!memcmp(name,expected,8)) {
        if(!native_test_pdos) return 28;
        *(unsigned long *)parameter(p,5)=0x1238; return 0;
    }
    mf_command_name("IRXSUBCM",expected);
    if(!memcmp(name,expected,8)) {
        mf_command_name("PDOS",expected);
        return native_test_pdos && !memcmp(parameter(p,3),expected,8) ? 0:8;
    }
    mf_command_name("IKJEFTSR",expected);
    if(memcmp(name,expected,8)) {
        mf_command_name("PDOSCMD",expected);
        if(memcmp(name,expected,8)) { *load_rc=8; return 20; }
    }
    mf_command_name("BAD@CMD",expected);
    *(int *)parameter(p,3)= *(unsigned long *)parameter(p,2)==7 &&
                           !memcmp(parameter(p,1),expected,7) ? 20:0;
    mf_command_name("LISTDS",expected);
    if(*(unsigned long *)parameter(p,2)>=6 && !memcmp(parameter(p,1),expected,6))
        *(int *)parameter(p,3)=8;
    *(int *)parameter(p,4)=0; *(unsigned *)parameter(p,5)=0;
    return *(int *)parameter(p,3) ? 4:0;
}
#else
#include <cms/adapter.h>
#include <cms/text-codec.h>
static unsigned char owned_id[18],fst[40];
static int owned,read_record;
int native_test_capture_fault;
int native_test_calls;
static void check_cleanup(void) { if(owned) abort(); }
int lab_cms_svc202(void *opaque,unsigned long regs[2]) {
    LabCmsFile *f=(LabCmsFile *)opaque; unsigned char verb[8],collision[18];
    static int registered;
    (void)regs;
    if(!registered) { atexit(check_cleanup); registered=1; }
    lab_cms_name("RX000001 CMDOUT A",collision);
    mf_command_name("STATE",verb);
    if(!memcmp(f->command,verb,8)) {
        if(memcmp(f->filename,collision,18) && (!owned || memcmp(f->filename,owned_id,18))) return 28;
        memset(fst,0,sizeof(fst)); memcpy(fst+24,f->filemode,2); fst[30]=0xe5;
        { int n=256; memcpy(fst+32,&n,4); }
        f->buffer=fst; return 0;
    }
    /* Never write/erase the deliberately occupied first candidate. */
    if(!memcmp(f->filename,collision,18)) abort();
    mf_command_name("WRBUF",verb);
    if(!memcmp(f->command,verb,8)) {
        memcpy(owned_id,f->filename,18); owned=1; read_record=0; return 0;
    }
    mf_command_name("FINIS",verb);
    if(!memcmp(f->command,verb,8)) return native_test_capture_fault==2 ? 24:0;
    mf_command_name("ERASE",verb);
    if(!memcmp(f->command,verb,8)) { owned=0; return 0; }
    mf_command_name("RDBUF",verb);
    if(!memcmp(f->command,verb,8)) {
        const char *record=read_record ? " ":"MiXeD|Case 'quoted'  ";
        unsigned i;
        if(native_test_capture_fault==1) return 24;
        if(read_record++==2) return 12;
        f->bytes=(int)strlen(record);
        for(i=0;i<(unsigned)f->bytes;++i) f->buffer[i]=(unsigned char)lab_ascii_to_ebcdic((unsigned char)record[i]);
        return 0;
    }
    return 20;
}
int mf_cms_command_call(void *plist,void *extended,unsigned long regs[2],unsigned type) {
    unsigned char expected[8]; (void)regs;
    if(!type) {
        mf_command_name("CMS",expected);
        if(!memcmp((unsigned char *)plist+8,expected,8)) return 0;
        mf_command_name("PDOS",expected);
        return native_test_pdos && !memcmp((unsigned char *)plist+8,expected,8) ? 0:1;
    }
    ++native_test_calls;
    { size_t length=(unsigned char *)((void **)extended)[2] -
                    (unsigned char *)((void **)extended)[1];
      mf_command_name("BAD@CMD",expected);
      if(length==7 && !memcmp(((void **)extended)[1],expected,7)) return -3;
      mf_command_name("STATE",expected);
      if(length>=5 && !memcmp(((void **)extended)[1],expected,5)) return 28;
      return 0; }
}
#endif
