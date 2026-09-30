/* MIT. Portable bounded binary codec for the generic float32 owner. */
#include "rxvector_index.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <float.h>
#include <math.h>
static const unsigned char magic[8]={'R','X','V','I','D','X',1,0};
static uint64_t read64(const unsigned char *p) {
    uint64_t n=0; size_t i; for(i=0;i<8;i++) n|=(uint64_t)p[i]<<(8*i); return n;
}
static void write64(unsigned char *p,size_t n) {
    size_t i; for(i=0;i<8;i++) p[i]=(unsigned char)((uint64_t)n>>(8*i));
}
static int number(const unsigned char *p,size_t size,size_t *at,size_t *value) {
    uint64_t n;
    if(*at>size || size-*at<8) return 0;
    n=read64(p+*at); *at+=8;
    if(n>SIZE_MAX) return 0;
    *value=(size_t)n; return 1;
}
static int text(const unsigned char *p,size_t size,size_t *at,size_t *offset,size_t *length) {
    if(!number(p,size,at,length) || *length>size-*at) return 0;
    *offset=*at; *at+=*length; return 1;
}
void rxvector_index_free(rxvector_index *x) {
    if(x){free(x->wire);free(x->label_offsets);free(x);}
}
int rxvector_index_open(const void *data,size_t size,rxvector_index **out,const char **error) {
    const unsigned char *p=(const unsigned char *)data;
    rxvector_index *x=NULL; size_t at=8,row,offset,length;
    *out=NULL;*error="invalid vector index header";
    if(!p || size<32 || memcmp(p,magic,8)) return -1;
    x=(rxvector_index *)calloc(1,sizeof(*x));
    if(!x){*error="cannot allocate vector index";return -2;}
    *error="invalid vector index shape or metadata";
    if(!number(p,size,&at,&x->dimensions) || !number(p,size,&at,&x->rows) ||
       !text(p,size,&at,&x->metadata_offset,&x->metadata_length) || !x->dimensions ||
       !x->rows || x->rows>(size-at)/8 || x->rows>SIZE_MAX/sizeof(size_t)) goto invalid;
    x->label_offsets=(size_t *)malloc(x->rows*sizeof(size_t));
    if(!x->label_offsets) goto allocation;
    *error="truncated vector index label";
    for(row=0;row<x->rows;row++) {
        x->label_offsets[row]=at;
        if(!text(p,size,&at,&offset,&length)) goto invalid;
    }
    *error="invalid vector index matrix length";
    if((size-at)%4 || x->dimensions>(size-at)/4/x->rows ||
       x->dimensions*x->rows!=(size-at)/4) goto invalid;
    x->matrix_offset=at;
    *error="vector index requires finite nonzero float32 norms";
    for(row=0;row<x->rows;row++) {
        size_t j; double norm=0;
        for(j=0;j<x->dimensions;j++) {
            double v=rxvector_f32le_at(p+at+(row*x->dimensions+j)*4);
            if(!isfinite(v)) goto invalid;
            norm+=v*v;
        }
        if(norm<FLT_MIN || norm>FLT_MAX) goto invalid;
    }
    x->wire=(unsigned char *)malloc(size);
    if(!x->wire) goto allocation;
    memcpy(x->wire,p,size);x->bytes=size;*out=x;*error="";return 0;
invalid:
    rxvector_index_free(x);return -1;
allocation:
    *error="cannot allocate vector index";rxvector_index_free(x);return -2;
}
int rxvector_index_make(const void *matrix,size_t bytes,size_t dim,const rxvector_text *labels,
                       size_t rows,rxvector_text metadata,rxvector_index **out,const char **error) {
    size_t size=32,at,row;unsigned char *wire;int status;
    *out=NULL;*error="invalid float32 matrix shape";
    if(!dim || !rows || !matrix || !labels || bytes%4 || dim>bytes/4/rows || dim*rows!=bytes/4) return -1;
    *error="vector index length overflow";
    if(metadata.length>SIZE_MAX-size || (metadata.length && !metadata.data)) return -1;
    size+=metadata.length;
    for(row=0;row<rows;row++) {
        if(size>SIZE_MAX-8 || labels[row].length>SIZE_MAX-size-8 ||
           (labels[row].length && !labels[row].data)) return -1;
        size+=8+labels[row].length;
    }
    if(bytes>SIZE_MAX-size) return -1;
    size+=bytes;wire=(unsigned char *)malloc(size);
    if(!wire){*error="cannot allocate vector index encoding";return -2;}
    memcpy(wire,magic,8);write64(wire+8,dim);write64(wire+16,rows);
    write64(wire+24,metadata.length);at=32;
    if(metadata.length) memcpy(wire+at,metadata.data,metadata.length);
    at+=metadata.length;
    for(row=0;row<rows;row++) {
        write64(wire+at,labels[row].length);at+=8;
        if(labels[row].length) memcpy(wire+at,labels[row].data,labels[row].length);
        at+=labels[row].length;
    }
    memcpy(wire+at,matrix,bytes);
    status=rxvector_index_open(wire,size,out,error);free(wire);return status;
}
rxvector_text rxvector_index_label(const rxvector_index *x,size_t row) {
    rxvector_text result={NULL,0};size_t at;
    if(!x || row>=x->rows)return result;
    at=x->label_offsets[row];result.length=(size_t)read64(x->wire+at);
    result.data=(const char *)(x->wire+at+8);return result;
}
rxvector_status rxvector_index_search(const rxvector_index *x,const void *query,size_t bytes,
                                     size_t count,rxvector_hit *hits) {
    size_t j;double norm=0;
    if(!x || !query || !count || count>x->rows || bytes%4 || bytes/4!=x->dimensions)
        return RXVECTOR_INVALID_PAYLOAD;
    for(j=0;j<x->dimensions;j++) {
        double v=rxvector_f32le_at((const unsigned char *)query+j*4);
        if(!isfinite(v))return RXVECTOR_NONFINITE_INPUT;
        norm+=v*v;
    }
    if(norm<FLT_MIN || norm>FLT_MAX)return RXVECTOR_ZERO_NORM;
    return rxvector_topk_f32le_kernel(x->wire+x->matrix_offset,x->rows,x->dimensions,
                                    query,count,hits);
}
