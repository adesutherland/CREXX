/* MIT. RXPA binding for the generic immutable float32 owner.
 * Included by rxvector.c so one provider owns old and new declarations. */
#include "rxvector_index.h"
#if defined(_MSC_VER)
#define RXVI_LOCAL __declspec(thread)
#else
#define RXVI_LOCAL __thread
#endif
#if defined(_WIN32)
#include <windows.h>
typedef volatile LONG rxvi_refs;
static void rxvi_retain(rxvi_refs *n){InterlockedIncrement(n);}
static int rxvi_drop(rxvi_refs *n){return InterlockedDecrement(n)==0;}
#else
typedef size_t rxvi_refs;
static void rxvi_retain(rxvi_refs *n){__atomic_add_fetch(n,1,__ATOMIC_RELAXED);}
static int rxvi_drop(rxvi_refs *n){return __atomic_sub_fetch(n,1,__ATOMIC_ACQ_REL)==0;}
#endif
static RXVI_LOCAL const rxpa_host_services_v1 *rxvi_host;
static void *rxvi_create_old(void){return NULL;}
static void *rxvi_create(const rxpa_host_services_v1 *h){return (void *)h;}
static void rxvi_destroy(void *h){(void)h;}
static int rxvi_enter(void *h,uint32_t caps,void **previous){
    (void)caps;*previous=(void *)rxvi_host;rxvi_host=h;return 0;
}
static void rxvi_leave(void *previous){rxvi_host=previous;}
static uint32_t rxvi_caps(const char *name){
    if(name && (!strcmp(name,"rxvector.decodef32le") || !strcmp(name,"rxvector.encodef32le") ||
        !strcmp(name,"rxvector.cosine") || !strcmp(name,"rxvector.topkcosine")))
        return RXPA_PROCEDURE_CAP_PROCESS_REENTRANT;
    return RXPA_PROCEDURE_CAP_SESSION_AFFINE;
}
RXPA_PLUGIN_SESSION_WITH_HOST(rxvi_create_old,rxvi_destroy,rxvi_enter,rxvi_leave,rxvi_caps,rxvi_create)
typedef struct rxvi_owner {rxvi_refs refs;rxvector_index *index;} rxvi_owner;
static void rxvi_copy(void *,void *);
static void rxvi_finalize(void *);
static const rxpa_native_payload_ops rxvi_ops={"rxvector.vectorindex",rxvi_copy,rxvi_finalize};
static rxvi_owner *rxvi_get(void *value){
    size_t bytes=0;const rxpa_native_payload_ops *ops=NULL;rxvi_owner *result=NULL;
    void *p=GETNATIVEPAYLOAD(value,&bytes,&ops,NULL);
    if(p && bytes==sizeof(result) && ops==&rxvi_ops)memcpy(&result,p,bytes);
    return result;
}
static void rxvi_release(rxvi_owner *p){if(p && rxvi_drop(&p->refs)){rxvector_index_free(p->index);free(p);}}
static void rxvi_copy(void *dst,void *src){
    rxvi_owner *p=rxvi_get(src);
    if(p){rxvi_retain(&p->refs);if(SETNATIVEPAYLOAD(dst,&p,sizeof(p),&rxvi_ops,0))rxvi_release(p);}
}
static void rxvi_finalize(void *value){rxvi_release(rxvi_get(value));}
/* Takes ownership on both success and failure. */
static int rxvi_publish(void *value,rxvector_index *index){
    rxvi_owner *p=calloc(1,sizeof(*p));
    if(!p){rxvector_index_free(index);return -1;}
    p->refs=1;p->index=index;
    if(!rxpa_host_has_object_set_type(rxvi_host) ||
       SETOBJECTTYPE(rxvi_host,value,"rxvector.vectorindex") ||
       SETNATIVEPAYLOAD(value,&p,sizeof(p),&rxvi_ops,0)) {rxvi_release(p);return -1;}
    return 0;
}
static int rxvi_text(void *value,rxvector_text *text){
    return !rxpa_host_has_string_view(rxvi_host) ||
        rxvi_host->string_view(value,&text->data,&text->length);
}
PROCEDURE(make_vector_index){
    rxvector_index *index=NULL;rxvector_text *labels=NULL,metadata;const char *error="";
    size_t bytes=0,i;rxinteger rows=GETARRAYHI(ARG2),dim=GETINT(ARG1);int status;
    void *matrix=GETNATIVEPAYLOAD(ARG0,&bytes,NULL,NULL);
    if(dim<1 || rows<1 || (uint64_t)rows>SIZE_MAX/sizeof(*labels)) {
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"invalid float32 matrix dimensions")
    }
    if(rxvi_text(ARG3,&metadata)){RETURNSIGNAL(SIGNAL_FAILURE,"vector index requires RXPA string views")}
    labels=calloc((size_t)rows,sizeof(*labels));
    if(!labels){RETURNSIGNAL(SIGNAL_FAILURE,"cannot allocate vector labels")}
    for(i=0;i<(size_t)rows;i++)if(rxvi_text(GETATTR(ARG2,(rxinteger)i),&labels[i])) {
        free(labels);RETURNSIGNAL(SIGNAL_FAILURE,"cannot read vector label")
    }
    status=rxvector_index_make(matrix,bytes,(size_t)dim,labels,(size_t)rows,metadata,&index,&error);
    free(labels);
    if(status==-1){RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,error)}
    if(status){RETURNSIGNAL(SIGNAL_FAILURE,error)}
    if(rxvi_publish(RETURN,index)){RETURNSIGNAL(SIGNAL_FAILURE,"cannot publish vector index")}
    RESETSIGNAL
}
PROCEDURE(decode_vector_index){
    size_t bytes=0;void *wire=GETNATIVEPAYLOAD(ARG0,&bytes,NULL,NULL);
    rxvector_index *index=NULL;const char *error="";int status=rxvector_index_open(wire,bytes,&index,&error);
    if(status==-1){RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,error)}
    if(status){RETURNSIGNAL(SIGNAL_FAILURE,error)}
    if(rxvi_publish(RETURN,index)){RETURNSIGNAL(SIGNAL_FAILURE,"cannot publish vector index")}
    RESETSIGNAL
}
PROCEDURE(open_vector_index){
    size_t bytes=0;void *wire=GETNATIVEPAYLOAD(ARG0,&bytes,NULL,NULL);
    rxvector_index *index=NULL;const char *error="";int status;
    if(SETNATIVEPAYLOAD(ARG1,NULL,0,NULL,0)) {
        SETSTRING(ARG2,"cannot clear vector index");RETURNINT(-1);RESETSIGNAL;return;
    }
    status=rxvector_index_open(wire,bytes,&index,&error);
    if(!status && rxvi_publish(ARG1,index)){status=-2;error="cannot publish vector index";}
    SETSTRING(ARG2,error);RETURNINT(status? -1:0);RESETSIGNAL
}
#define RXVI_OWNER rxvi_owner *owner=rxvi_get(ARG0); \
    if(!owner){RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"closed or invalid vector index")}
METHODPROCEDURE(encode_vector_index){
    RXVI_OWNER
    if(SETNATIVEPAYLOAD(RETURN,owner->index->wire,owner->index->bytes,NULL,0)) {
        RETURNSIGNAL(SIGNAL_FAILURE,"cannot encode vector index")
    }
    RESETSIGNAL
}
METHODPROCEDURE(vector_index_rows){RXVI_OWNER RETURNINT(owner->index->rows);RESETSIGNAL}
METHODPROCEDURE(vector_index_dimensions){RXVI_OWNER RETURNINT(owner->index->dimensions);RESETSIGNAL}
METHODPROCEDURE(vector_index_metadata){
    RXVI_OWNER
    if(SETSTRINGLENGTH(rxvi_host,RETURN,(const char *)owner->index->wire+owner->index->metadata_offset,owner->index->metadata_length)) {
        RETURNSIGNAL(SIGNAL_FAILURE,"cannot return vector metadata")
    }
    RESETSIGNAL
}
METHODPROCEDURE(vector_index_label){
    rxinteger row=GETINT(ARG1);rxvector_text label;RXVI_OWNER
    if(row<0 || (uint64_t)row>=owner->index->rows){RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"label row is out of range")}
    label=rxvector_index_label(owner->index,(size_t)row);
    if(SETSTRINGLENGTH(rxvi_host,RETURN,label.data,label.length)){RETURNSIGNAL(SIGNAL_FAILURE,"cannot return vector label")}
    RESETSIGNAL
}
METHODPROCEDURE(close_vector_index){
    if(SETNATIVEPAYLOAD(ARG0,NULL,0,NULL,0)){RETURNSIGNAL(SIGNAL_FAILURE,"cannot close vector index")}
    RETURNINT(0);RESETSIGNAL
}
METHODPROCEDURE(search_vector_index){
    rxvi_owner *owner;rxinteger requested=GETINT(ARG2);size_t bytes=0,count,i;
    void *query=GETNATIVEPAYLOAD(ARG1,&bytes,NULL,NULL);rxvector_hit *hits;
    int64_t *keys;double *scores;rxvector_status status;
    SETNATIVEPAYLOAD(ARG3,NULL,0,NULL,0);SETNATIVEPAYLOAD(ARG4,NULL,0,NULL,0);
    owner=rxvi_get(ARG0);
    if(!owner || requested<1){RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"invalid vector index or search count")}
    count=(uint64_t)requested>owner->index->rows?owner->index->rows:(size_t)requested;
    if(count>SIZE_MAX/sizeof(*hits)){RETURNSIGNAL(SIGNAL_FAILURE,"search result size overflow")}
    hits=malloc(count*sizeof(*hits));keys=malloc(count*sizeof(*keys));scores=malloc(count*sizeof(*scores));
    if(!hits || !keys || !scores){free(hits);free(keys);free(scores);RETURNSIGNAL(SIGNAL_FAILURE,"cannot allocate vector search results")}
    status=rxvector_index_search(owner->index,query,bytes,count,hits);
    if(status!=RXVECTOR_OK){free(hits);free(keys);free(scores);RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"invalid query shape, finite values or float32 norm")}
    for(i=0;i<count;i++){keys[i]=(int64_t)hits[i].row;scores[i]=hits[i].score;}
    free(hits);
    if(SETNATIVEPAYLOAD(ARG3,keys,count*sizeof(*keys),NULL,0) ||
       SETNATIVEPAYLOAD(ARG4,scores,count*sizeof(*scores),NULL,0)) {
        SETNATIVEPAYLOAD(ARG3,NULL,0,NULL,0);SETNATIVEPAYLOAD(ARG4,NULL,0,NULL,0);
        free(keys);free(scores);RETURNSIGNAL(SIGNAL_FAILURE,"cannot publish vector search results")
    }
    free(keys);free(scores);RESETSIGNAL
}
