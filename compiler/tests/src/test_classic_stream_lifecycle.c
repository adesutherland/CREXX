/* Native Classic stream ownership across two live VM sessions. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxvml.h"
#ifdef _WIN32
#include <windows.h>
static long handles(void) { DWORD n=0; return GetProcessHandleCount(GetCurrentProcess(),&n) ? (long)n:-1; }
#else
#include <dirent.h>
static long handles(void) {
    DIR *d=opendir("/dev/fd"); struct dirent *e; long n=0;
    if(!d) return -1;
    while((e=readdir(d))) if(strcmp(e->d_name,".") && strcmp(e->d_name,"..")) ++n;
    closedir(d); return n;
}
#endif
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"FAIL: line %d: %s\n",__LINE__,#x); exit(1); } } while(0)
static rxvml_context *context(const char *bin) {
    const char *modules[]={"library","classlib","rxfnsc","rxfnsg"}; size_t i;
    char path[4096]; rxvml_context *ctx=rxvml_create(bin,0);
    CHECK(ctx);
    for(i=0;i<4;++i) { CHECK(snprintf(path,sizeof(path),"%s/%s",bin,modules[i])<(int)sizeof(path)); CHECK(rxvml_load_module_file(ctx,path)>0); }
    return ctx;
}
static rxvml_value *open_stream(rxvml_context *ctx,const char *path) {
    rxvml_value *args[4],*result=NULL;
    args[0]=rxvml_value_new(ctx); args[1]=rxvml_value_new(ctx); CHECK(args[0] && args[1]);
    CHECK(!rxvml_set_str(args[0],path,strlen(path))); CHECK(!rxvml_set_str(args[1],"READ",4));
    /* The public external entry is bytecode-only; native descriptors fail safely. */
    CHECK(rxvml_call_factory_descriptor(ctx,"_rxcstream.transport","rxsig1|\xc2\xa7""factory|._rxcstream..transport|name=.string,mode=.string",2,args,&result)!=0);
    CHECK(!result);
    args[2]=rxvml_value_new(ctx); args[3]=rxvml_value_new(ctx); CHECK(args[2] && args[3]);
    CHECK(!rxvml_set_str(args[2],"",0)); rxvml_set_int(args[3],0);
    CHECK(!rxvml_call_factory_descriptor(ctx,"rexxclassicstream.rexxclassicstream","rxsig1|\xc2\xa7""factory|.rexxclassicstream..rexxclassicstream|name=.string,mode=.string,?encoding=.string,?replacing=.int",4,args,&result));
    rxvml_value_free(args[2]); rxvml_value_free(args[3]);
    rxvml_value_free(args[0]); rxvml_value_free(args[1]); CHECK(result); return result;
}
static rxvml_value *method(rxvml_context *ctx,rxvml_value *object,const char *descriptor,size_t argc,rxvml_value **args) {
    rxvml_value *result=NULL;
    CHECK(!rxvml_call_method_descriptor(ctx,object,"rexxclassicstream.rexxclassicstream",descriptor,argc,args,&result)); CHECK(result); return result;
}
static void read_one(rxvml_context *ctx,rxvml_value *object,const char *expected,size_t length) {
    rxvml_value *args[2],*out; const char *text=NULL; size_t n=0;
    args[0]=rxvml_value_new(ctx); args[1]=rxvml_value_new(ctx); CHECK(args[0] && args[1]);
    rxvml_set_int(args[0],-1); rxvml_set_int(args[1],1);
    out=method(ctx,object,"rxsig1|readcharacters|.string|start=.int,count=.int",2,args);
    CHECK(!rxvml_to_str(ctx,out,&text,&n)); CHECK(n==length && !memcmp(text,expected,n));
    rxvml_value_free(out); rxvml_value_free(args[0]); rxvml_value_free(args[1]);
}
int main(int argc,char **argv) {
    static const unsigned char bytes[]={0,255,65};
    rxvml_context *first,*second; rxvml_value *a,*b,*out=NULL; FILE *file; long baseline,initial; int i;
    CHECK(argc==3); file=fopen(argv[2],"wb"); CHECK(file); CHECK(fwrite(bytes,1,sizeof(bytes),file)==sizeof(bytes)); CHECK(!fclose(file));
    initial=handles(); CHECK(initial>=0);
    first=context(argv[1]); second=context(argv[1]);
    a=open_stream(first,argv[2]); b=open_stream(second,argv[2]);
    read_one(first,a,"\0",1); read_one(second,b,"\0",1);
    read_one(first,a,"\xc3\xbf",2); read_one(second,b,"\xc3\xbf",2);
    CHECK(rxvml_call_method_descriptor(second,a,"rexxclassicstream.rexxclassicstream","rxsig1|ready|.boolean|",0,NULL,&out)!=0);
    if(out) rxvml_value_free(out);
    out=method(first,a,"rxsig1|close|.int|",0,NULL); rxvml_value_free(out);
    rxvml_value_free(a); rxvml_destroy(first);
    read_one(second,b,"A",1); rxvml_value_free(b);
    /* Warm registration is complete; each unclosed payload must finalize its FD. */
    baseline=handles(); CHECK(baseline>=0);
    for(i=0;i<20;++i) { a=open_stream(second,argv[2]); read_one(second,a,"\0",1); rxvml_value_free(a); }
    CHECK(handles()==baseline);
    /* An execution-owned value remains open until VM registry teardown. */
    a=open_stream(second,argv[2]); CHECK(handles()==baseline+1);
    CHECK(rxvml_reg_alloc(second,a,"rexxclassicstream.rexxclassicstream")>=0);
    rxvml_destroy(second); CHECK(handles()==initial); CHECK(!remove(argv[2]));
    puts("PASS: Classic stream sessions, exact NUL/FF and final handle teardown"); return 0;
}
