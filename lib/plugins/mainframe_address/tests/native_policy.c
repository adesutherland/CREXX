/* Native boundary checks complement the actual guest Rexx scripts. */
#include "mainframe_command.h"
#undef NDEBUG
#include <assert.h>
#include <string.h>
#include <stdio.h>
extern int native_test_pdos;
#if defined(CREXX_CMS_ELF)
extern int native_test_capture_fault,native_test_calls;
static char records[128]; static size_t used;
static int sink(void *owner,const char *text,size_t n) {
    (void)owner; assert(n<sizeof(records)-used);
    memcpy(records+used,text,n); used+=n; return 0;
}
static int failed_sink(void *owner,const char *text,size_t n) {
    (void)owner; (void)text; (void)n; return -1;
}
#endif
int main(void) {
    crexx_command_handle *h=crexx_command_open(); crexx_command_result r;
    crexx_command_io io={0}; int present;
    assert(h);
    assert(crexx_command_query(h,"CMS",&present,NULL));
    assert(!crexx_command_query(h," system ",&present,&r) && present);
    assert(!crexx_command_execute(h,crexx_command_profile(),"TIME",4,NULL,&r) && !r.rc);
    assert(crexx_command_execute(h,crexx_command_profile(),"x\0y",3,NULL,&r) && r.rc==-3);
    assert(crexx_command_execute(h,crexx_command_profile(),"\xc0\xaf",2,NULL,&r) && r.rc==-3);
    assert(crexx_command_execute(h,crexx_command_profile(),"\xe2\x82\xac",3,NULL,&r) && r.rc==-3);
    assert(crexx_command_execute(h,crexx_command_profile(),"x",32768,NULL,&r) && r.rc==-3);
    assert(!crexx_command_query(h,"PDOS",&present,&r) && !present);
    native_test_pdos=1;
    assert(!crexx_command_query(h,"PDOS",&present,&r) && present);
    assert(!crexx_command_execute(h,"PDOS","VER",3,NULL,&r) && !r.rc);
    native_test_pdos=0;
    assert(!crexx_command_query(h,"PDOS",&present,&r) && !present);
#if defined(CREXX_CMS_ELF)
    io.requested=CREXX_COMMAND_CAPTURE_OUTPUT; io.output=sink;
    { int status=crexx_command_execute(h,"CMS","QUERY DISK A",12,&io,&r);
      if(status || r.rc) fprintf(stderr,"capture status=%d %s\n",status,r.diagnostic);
      assert(!status && !r.rc); }
    assert(used==24 && !memcmp(records,"MiXeD|Case 'quoted'  \n \n",24));
    { int before=native_test_calls;
      assert(crexx_command_execute(h,"CMS","QUERY TIME",10,&io,&r));
      assert(native_test_calls==before);
      io.requested|=CREXX_COMMAND_CAPTURE_ERROR;
      assert(crexx_command_execute(h,"CMS","QUERY DISK",10,&io,&r) && native_test_calls==before);
      io.requested=CREXX_COMMAND_CAPTURE_OUTPUT; }
    native_test_capture_fault=1;
    assert(crexx_command_execute(h,"CMS","QUERY DISK",10,&io,&r) && r.command_rc_valid && r.rc==-3);
    native_test_capture_fault=2;
    { int before=native_test_calls;
      assert(crexx_command_execute(h,"CMS","QUERY DISK",10,&io,&r) && native_test_calls==before); }
    native_test_capture_fault=0; io.output=failed_sink;
    assert(crexx_command_execute(h,"CMS","QUERY DISK",10,&io,&r));
    assert(!crexx_command_execute(h,"CMS","QUERY DISK",10,NULL,&r) && !r.rc);
#else
    io.requested=CREXX_COMMAND_CAPTURE_OUTPUT;
    assert(crexx_command_execute(h,"TSO","TIME",4,&io,&r) && r.rc==-3);
#endif
    crexx_command_close(h); puts("NATIVE POLICY PASS"); return 0;
}
