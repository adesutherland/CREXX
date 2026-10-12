#include "rxvml.h"
#undef NDEBUG
#include <assert.h>
#include <string.h>
#include <stdio.h>
static int calls;
static int callback(rxvml_context *ctx,const rxvml_address_request *q,rxvml_address_response *r,void *owner) {
    (void)ctx; (void)owner;
    assert(!strcmp(q->command,"QUERY DISK A"));
    ++calls; r->rc=47; r->condition_name="ERROR"; return 0;
}
static int probe(rxvml_context *ctx,const char *profile) {
    rxvml_value *arg=rxvml_value_new(ctx),*result=NULL; rxinteger rc=0;
    assert(arg && !rxvml_set_str(arg,profile,strlen(profile)));
    assert(!rxvml_call_procedure_descriptor(ctx,"rxsig1|native_address.probe|.int|target=.string",1,&arg,&result));
    assert(result && !rxvml_to_int(ctx,result,&rc));
    rxvml_value_free(result); rxvml_value_free(arg); return (int)rc;
}
int main(int argc,char **argv) {
    rxvml_context *first,*second; const char *args[2]; int rc,i;
    assert(argc==4);
    first=rxvml_create(NULL,0); second=rxvml_create(NULL,0); assert(first && second);
    assert(rxvml_load_module_file(first,argv[1])>0 && rxvml_load_module_file(second,argv[1])>0);
    args[0]=argv[2]; args[1]=argv[3];
    for(i=0;i<3;++i) {
        assert(!rxvml_run(first,2,args,&rc) && !rc);
        assert(!rxvml_run(second,2,args,&rc) && !rc);
    }
    assert(!rxvml_address_register_callback_environment(first,argv[2],"host-override",callback,NULL,NULL));
    assert(probe(first,argv[2])==47 && calls==1);
    assert(!probe(second,argv[2]) && calls==1);
    rxvml_destroy(first);
    assert(!probe(second,argv[2])); rxvml_destroy(second);
    puts("NATIVE EMBEDDING PASS"); return 0;
}
