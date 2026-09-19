/* MIT. Independent wire, numeric and exact-oracle controls; asserts stay active in Release. */
#include "rxvector_index.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#define CHECK(c) do {if(!(c)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#c);exit(1);}}while(0)
static void f32(unsigned char *p,float f){uint32_t n;size_t j;memcpy(&n,&f,4);for(j=0;j<4;j++)p[j]=(unsigned char)(n>>(8*j));}
static int rejects(const unsigned char *wire,size_t size){rxvector_index *x=NULL;const char *error="";int s=rxvector_index_open(wire,size,&x,&error);CHECK(!x && error[0]);return s!=0;}
int main(void){
 unsigned char matrix[4*2*4],q[8];rxvector_text labels[4]={{"alpha\0tail",10},{"duplicate",9},{"other",5},{"opposite",8}},metadata={"meta\0data",9};
 rxvector_index *x=NULL,*copy=NULL;const char *error="";rxvector_hit hits[4];size_t i;unsigned char *wire;
 float values[8]={1,0,1,0,0,1,-1,0};
 for(i=0;i<8;i++)f32(matrix+i*4,values[i]);f32(q,1);f32(q+4,0);
 CHECK(!rxvector_index_make(matrix,sizeof(matrix),2,labels,4,metadata,&x,&error));
 CHECK(x->rows==4 && x->dimensions==2 && x->metadata_length==9);
 CHECK(!memcmp(x->wire+x->metadata_offset,metadata.data,metadata.length));
 CHECK(rxvector_index_label(x,0).length==10 && !memcmp(rxvector_index_label(x,0).data,labels[0].data,10));
 CHECK(rxvector_index_search(x,q,8,4,hits)==RXVECTOR_OK);
 CHECK(hits[0].row==0 && hits[1].row==1 && hits[2].row==2 && hits[3].row==3 && hits[0].score==1 && hits[3].score==-1);
 CHECK(!rxvector_index_open(x->wire,x->bytes,&copy,&error) && copy->bytes==x->bytes && !memcmp(copy->wire,x->wire,x->bytes));
 rxvector_index_free(copy);copy=NULL;
 for(i=0;i<x->bytes;i++)CHECK(rejects(x->wire,i));
 wire=malloc(x->bytes+1);CHECK(wire);memcpy(wire,x->wire,x->bytes);wire[x->bytes]=0;CHECK(rejects(wire,x->bytes+1));
 wire[0]^=1;CHECK(rejects(wire,x->bytes));memcpy(wire,x->wire,x->bytes);
 memset(wire+8,255,8);CHECK(rejects(wire,x->bytes));memcpy(wire,x->wire,x->bytes);
 memset(wire+16,255,8);CHECK(rejects(wire,x->bytes));memcpy(wire,x->wire,x->bytes);
 memset(wire+24,255,8);CHECK(rejects(wire,x->bytes));memcpy(wire,x->wire,x->bytes);
 memset(wire+x->label_offsets[0],255,8);CHECK(rejects(wire,x->bytes));memcpy(wire,x->wire,x->bytes);
 f32(wire+x->matrix_offset,NAN);CHECK(rejects(wire,x->bytes));
 f32(wire+x->matrix_offset,INFINITY);CHECK(rejects(wire,x->bytes));
 f32(wire+x->matrix_offset,0);CHECK(rejects(wire,x->bytes));
 f32(wire+x->matrix_offset,FLT_MAX);CHECK(rejects(wire,x->bytes));
 f32(wire+x->matrix_offset,FLT_MIN);CHECK(rejects(wire,x->bytes));
 f32(q,0);CHECK(rxvector_index_search(x,q,8,4,hits)!=RXVECTOR_OK);
 f32(q,NAN);CHECK(rxvector_index_search(x,q,8,4,hits)!=RXVECTOR_OK);
 f32(q,1);CHECK(rxvector_index_search(x,q,7,4,hits)!=RXVECTOR_OK);
 CHECK(rxvector_index_search(x,q,8,0,hits)!=RXVECTOR_OK);
 free(wire);rxvector_index_free(x);x=NULL;
 {
  unsigned char many[257*8];rxvector_text names[257];rxvector_hit tied[12];
  for(i=0;i<257;i++){f32(many+i*8,1);f32(many+i*8+4,0);names[i].data="";names[i].length=0;}
  CHECK(!rxvector_index_make(many,sizeof(many),2,names,257,metadata,&x,&error));
  CHECK(rxvector_index_search(x,q,8,12,tied)==RXVECTOR_OK);
  for(i=0;i<12;i++)CHECK(tied[i].row==i && tied[i].score==1);
  rxvector_index_free(x);x=NULL;
 }
 {
  unsigned char m[19*7*4],query[7*4];double doubles[19*7],query_doubles[7];int64_t ids[19];
  rxvector_text names[19];rxvector_hit expected[7],actual[7];uint32_t random=7;size_t trial,j;
  rxvector_float_span dm={(unsigned char *)doubles,19*7},dq={(unsigned char *)query_doubles,7};rxvector_int_span di={(unsigned char *)ids,19};
  for(trial=0;trial<100;trial++){
   for(i=0;i<19*7;i++){float value;random=random*1664525u+1013904223u;value=(float)((int)(random%20001)-10000)/10000.0f;f32(m+i*4,value);doubles[i]=value;}
   for(j=0;j<7;j++){float value;random=random*1664525u+1013904223u;value=(float)((int)(random%20001)-10000)/10000.0f;f32(query+j*4,value);query_doubles[j]=value;}
   for(i=0;i<19;i++){ids[i]=(int64_t)i;names[i].data="";names[i].length=0;}
   CHECK(!rxvector_index_make(m,sizeof(m),7,names,19,metadata,&x,&error));
   CHECK(rxvector_topk_kernel(&dm,&di,7,&dq,7,expected)==RXVECTOR_OK);
   CHECK(rxvector_index_search(x,query,sizeof(query),7,actual)==RXVECTOR_OK);
   for(i=0;i<7;i++)CHECK(expected[i].row==actual[i].row && fabs(expected[i].score-actual[i].score)<1e-12);
   rxvector_index_free(x);x=NULL;
  }
 }
 puts("RXVECTOR_INDEX_CORE_OK");return 0;
}
