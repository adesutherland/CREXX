/* MIT licence. Private byte transport for Classic streams. Text semantics,
 * codepage selection and character positions belong to rxfnsc, not the OS. */
#include "crexxpa.h"
#ifndef DECL_ONLY
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include <wchar.h>
#define cs_fileno _fileno
#define cs_seek _fseeki64
#define cs_tell _ftelli64
#define cs_stat _fstat64
#define cs_stat_t struct _stat64
#define cs_truncate(fp,n) _chsize_s(cs_fileno(fp),n)
#else
#include <unistd.h>
#define cs_fileno fileno
#define cs_seek fseeko
#define cs_tell ftello
#define cs_stat fstat
#define cs_stat_t struct stat
#define cs_truncate(fp,n) ftruncate(cs_fileno(fp),(off_t)(n))
#endif
#include "rxvmfile.h"

static FILE *cs_open_file(const char *name,const char *mode) {
#ifdef _WIN32
    int count=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,name,-1,NULL,0);
    wchar_t *path=count ? (wchar_t *)malloc((size_t)count*sizeof(*path)):NULL;
    wchar_t private_mode[8]; size_t i; FILE *result=NULL;
    if(!path) { errno=EINVAL; return NULL; }
    if(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,name,-1,path,count)) {
        for(i=0;mode[i] && i<6;++i) private_mode[i]=(wchar_t)mode[i];
        private_mode[i++]=L'N'; private_mode[i]=0;
        result=_wfopen(path,private_mode);
    } else errno=EINVAL;
    free(path); return result;
#else
    return rxvm_private_fopen(name,mode);
#endif
}

typedef struct cs_session { const rxpa_host_services_v1 *host; size_t refs; } cs_session;
typedef struct cs_file {
    cs_session *owner;
    size_t refs;
    FILE *file;
    int borrowed, regular, readable, writable, status, eof;
} cs_file;
#ifdef _MSC_VER
#define CS_LOCAL __declspec(thread)
#else
#define CS_LOCAL __thread
#endif
static CS_LOCAL cs_session *cs_current;
static void *cs_create(const rxpa_host_services_v1 *host) {
    cs_session *s;
    if (!rxpa_host_has_string_view(host) || !rxpa_host_has_string_set(host) || !rxpa_host_has_object_set_type(host)) return NULL;
    s = (cs_session *)calloc(1,sizeof(*s));
    if (s) { s->host = host; s->refs = 1; }
    return s;
}
static void *cs_old_create(void) { return NULL; }
static void cs_destroy(void *opaque) { cs_session *s=(cs_session *)opaque; if(s && !--s->refs) free(s); }
static int cs_enter(void *s,uint32_t caps,void **previous) { (void)caps; *previous=cs_current; cs_current=(cs_session *)s; return 0; }
static void cs_leave(void *previous) { cs_current=(cs_session *)previous; }
static uint32_t cs_caps(const char *name) { (void)name; return RXPA_PROCEDURE_CAP_SESSION_AFFINE; }
RXPA_PLUGIN_SESSION_WITH_HOST(cs_old_create,cs_destroy,cs_enter,cs_leave,cs_caps,cs_create)
static void cs_copy(void *,void *);
static void cs_finalize(void *);
static const rxpa_native_payload_ops cs_ops={"_rxcstream.transport",cs_copy,cs_finalize};
static cs_file *cs_resource(rxpa_attribute_value value) {
    size_t n=0; const rxpa_native_payload_ops *ops=NULL; cs_file *f=NULL;
    void *p=GETNATIVEPAYLOAD(value,&n,&ops,NULL);
    if(p && n==sizeof(f) && ops==&cs_ops) memcpy(&f,p,n);
    return f && f->owner==cs_current ? f : NULL;
}
static int cs_close(cs_file *f) {
    int result=0;
    if(f && f->file) {
        if(f->borrowed) { if(f->writable && fflush(f->file)) result=errno ? errno:EIO; }
        else if(fclose(f->file)) result=errno ? errno:EIO;
        f->file=NULL; f->status=result;
    }
    return result;
}
static void cs_release(cs_file *f) { if(f && !--f->refs) { cs_close(f); cs_destroy(f->owner); free(f); } }
static cs_file *cs_payload(rxpa_attribute_value value) {
    size_t n=0; const rxpa_native_payload_ops *ops=NULL; cs_file *f=NULL;
    void *p=GETNATIVEPAYLOAD(value,&n,&ops,NULL);
    if(p && n==sizeof(f) && ops==&cs_ops) memcpy(&f,p,n);
    return f;
}
static void cs_copy(void *destination,void *source) {
    cs_file *f=cs_payload(source);
    if(f) { ++f->refs; if(SETNATIVEPAYLOAD(destination,&f,sizeof(f),&cs_ops,0)) cs_release(f); }
}
static void cs_finalize(void *value) { cs_release(cs_payload(value)); }
static int cs_view(rxpa_attribute_value value,const char **p,size_t *n) { return cs_current->host->string_view(value,p,n); }
/* Ordinal text is the public binary boundary, including embedded NUL. */
static unsigned char *cs_bytes(rxpa_attribute_value value,size_t *length) {
    const char *text; size_t n,i=0,j=0; unsigned char *bytes;
    if(cs_view(value,&text,&n)) return NULL;
    bytes=(unsigned char *)malloc(n ? n:1);
    if(!bytes) return NULL;
    while(i<n) {
        unsigned int c=(unsigned char)text[i++];
        if(c>=0xc2 && c<=0xc3 && i<n && ((unsigned char)text[i]&0xc0)==0x80) c=((c&0x1f)<<6)|((unsigned char)text[i++]&0x3f);
        else if(c>=0x80) { free(bytes); return NULL; }
        bytes[j++]=(unsigned char)c;
    }
    *length=j; return bytes;
}
static int cs_return_bytes(rxpa_attribute_value value,const unsigned char *bytes,size_t n) {
    char *text; size_t i,j=0; int result;
    if(n>SIZE_MAX/2) return -1;
    text=(char *)malloc(n ? n*2:1);
    if(!text) return -1;
    for(i=0;i<n;++i) { unsigned int c=bytes[i]; if(c<128) text[j++]=(char)c; else { text[j++]=(char)(0xc0|(c>>6)); text[j++]=(char)(0x80|(c&63)); } }
    result=SETSTRINGLENGTH(cs_current->host,value,text,j); free(text); return result;
}
#define CS_RECEIVER cs_file *f=cs_resource(ARG0); if(!f) { RETURNSIGNAL(SIGNAL_FAILURE,"Invalid Classic stream owner") }
PROCEDURE(cs_open) {
    const char *name_view,*mode_view; char *name,*mode; size_t n,m; cs_file *f; cs_stat_t st;
    if(NUM_ARGS!=2 || cs_view(ARG0,&name_view,&n) || cs_view(ARG1,&mode_view,&m) || memchr(name_view,0,n) || memchr(mode_view,0,m) || n==SIZE_MAX || m==SIZE_MAX) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Invalid stream path/mode") }
    name=(char *)malloc(n+1); mode=(char *)malloc(m+1);
    if(!name || !mode) { free(name); free(mode); RETURNSIGNAL(SIGNAL_FAILURE,"Stream allocation failed") }
    memcpy(name,name_view,n); name[n]=0; memcpy(mode,mode_view,m); mode[m]=0;
    if(strcmp(mode,"READ") && strcmp(mode,"WRITE") && strcmp(mode,"BOTH") && strcmp(mode,"REPLACE") && strcmp(mode,"AUTO")) { free(name); free(mode); RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Invalid stream mode") }
    f=(cs_file *)calloc(1,sizeof(*f));
    if(!f) { free(name); free(mode); RETURNSIGNAL(SIGNAL_FAILURE,"Stream allocation failed") }
    f->owner=cs_current; ++cs_current->refs; f->refs=1;
    f->readable=1;
    f->writable=strcmp(mode,"READ")!=0;
    if(!n) { f->borrowed=1; f->file=f->writable ? stdout:stdin; f->regular=0;
#ifdef _WIN32
        /* A private CRT stream preserves binary ordinals without changing
         * the process-global stdin/stdout text mode used by B/G console I/O. */
        {
            HANDLE duplicate=NULL;
            intptr_t original=_get_osfhandle(cs_fileno(f->file));
            int fd=-1;
            f->file=NULL; f->borrowed=0;
            if(original!=-1 && DuplicateHandle(GetCurrentProcess(),(HANDLE)original,
                    GetCurrentProcess(),&duplicate,0,FALSE,DUPLICATE_SAME_ACCESS)) {
                fd=_open_osfhandle((intptr_t)duplicate,_O_BINARY | (f->writable ? _O_WRONLY:_O_RDONLY));
                if(fd>=0) { f->file=_fdopen(fd,f->writable ? "wb":"rb"); if(!f->file) _close(fd); }
                else CloseHandle(duplicate);
            }
            if(!f->file) f->status=errno ? errno:EIO;
        }
#endif
    } else {
        if(!strcmp(mode,"READ")) f->file=cs_open_file(name,"rb");
        else if(!strcmp(mode,"REPLACE")) f->file=cs_open_file(name,"w+b");
        else { f->file=cs_open_file(name,"r+b"); if(!f->file && !strcmp(mode,"AUTO")) { f->file=cs_open_file(name,"rb"); f->writable=0; } else if(!f->file && errno==ENOENT) f->file=cs_open_file(name,"w+b"); }
        if(!f->file) f->status=errno ? errno:EIO;
        else if(cs_stat(cs_fileno(f->file),&st)) { f->status=errno; cs_close(f); }
        else {
#ifdef _WIN32
            f->regular=(st.st_mode&_S_IFMT)==_S_IFREG;
#else
            f->regular=S_ISREG(st.st_mode);
#endif
        }
    }
    free(name); free(mode);
    if(SETNATIVEPAYLOAD(RETURN,&f,sizeof(f),&cs_ops,0)) { cs_release(f); RETURNSIGNAL(SIGNAL_FAILURE,"Cannot own Classic stream") }
    if(SETOBJECTTYPE(cs_current->host,RETURN,"_rxcstream.transport")) { RETURNSIGNAL(SIGNAL_FAILURE,"Cannot type Classic stream") }
    RESETSIGNAL
}
METHODPROCEDURE(cs_ready) { CS_RECEIVER RETURNINT(f->file!=NULL && !f->status); RESETSIGNAL }
METHODPROCEDURE(cs_regular) { CS_RECEIVER RETURNINT(f->regular); RESETSIGNAL }
METHODPROCEDURE(cs_error) { CS_RECEIVER RETURNINT(f->status); RESETSIGNAL }
METHODPROCEDURE(cs_eof) { CS_RECEIVER RETURNINT(f->eof); RESETSIGNAL }
METHODPROCEDURE(cs_end) { CS_RECEIVER RETURNINT(cs_close(f)); RESETSIGNAL }
METHODPROCEDURE(cs_load) {
    unsigned char *data=NULL,*next; size_t n=0,capacity=0,got; CS_RECEIVER
    if(!f->file || !f->readable || !f->regular || cs_seek(f->file,0,SEEK_SET)) { f->status=errno ? errno:EBADF; RETURNSTR(""); RESETSIGNAL return; }
    clearerr(f->file); f->status=0;
    do {
        if(capacity-n<32768) { if(capacity>SIZE_MAX-32768) { free(data); RETURNSIGNAL(SIGNAL_FAILURE,"Stream too large") } capacity+=32768; next=(unsigned char *)realloc(data,capacity); if(!next) { free(data); RETURNSIGNAL(SIGNAL_FAILURE,"Stream allocation failed") } data=next; }
        got=fread(data+n,1,32768,f->file); n+=got;
    } while(got==32768);
    if(ferror(f->file)) f->status=errno ? errno:EIO;
    if(cs_return_bytes(RETURN,data,n)) { free(data); RETURNSIGNAL(SIGNAL_FAILURE,"Cannot return stream bytes") }
    free(data); RESETSIGNAL
}
METHODPROCEDURE(cs_read) {
    unsigned char *data; size_t n,got; rxinteger count; CS_RECEIVER
    count=GETINT(ARG1);
    if(count<0 || (uint64_t)count>SIZE_MAX) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Invalid stream count") }
    n=(size_t)count; data=(unsigned char *)malloc(n ? n:1);
    if(!data) { RETURNSIGNAL(SIGNAL_FAILURE,"Stream allocation failed") }
    if(!f->file || !f->readable) { f->status=EBADF; got=0; }
    else { clearerr(f->file); got=fread(data,1,n,f->file); f->status=ferror(f->file) ? (errno ? errno:EIO):0; f->eof=feof(f->file)!=0; }
    if(cs_return_bytes(RETURN,data,got)) { free(data); RETURNSIGNAL(SIGNAL_FAILURE,"Cannot return stream bytes") }
    free(data); RESETSIGNAL
}
METHODPROCEDURE(cs_record) {
    unsigned char *delimiter,*data=NULL,*next; size_t d,n=0,capacity=0; int c; CS_RECEIVER
    delimiter=cs_bytes(ARG1,&d);
    if(!delimiter || !d) { free(delimiter); RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Invalid record delimiter") }
    if(!f->file || !f->readable) f->status=EBADF;
    else {
        clearerr(f->file); f->status=0;
        while((c=fgetc(f->file))!=EOF) {
            if(n==capacity) { if(capacity>SIZE_MAX-4096) { free(data); free(delimiter); RETURNSIGNAL(SIGNAL_FAILURE,"Record too large") } capacity+=4096; next=(unsigned char *)realloc(data,capacity); if(!next) { free(data); free(delimiter); RETURNSIGNAL(SIGNAL_FAILURE,"Record allocation failed") } data=next; }
            data[n++]=(unsigned char)c;
            if(n>=d && !memcmp(data+n-d,delimiter,d) && (d==1 || n%d==0)) break;
        }
        f->eof=feof(f->file)!=0; if(ferror(f->file)) f->status=errno ? errno:EIO;
    }
    free(delimiter);
    if(cs_return_bytes(RETURN,data,n)) { free(data); RETURNSIGNAL(SIGNAL_FAILURE,"Cannot return record bytes") }
    free(data); RESETSIGNAL
}
static rxinteger cs_write_bytes(cs_file *f,rxpa_attribute_value value,rxinteger offset,int replace) {
    unsigned char *data; size_t n,got=0; data=cs_bytes(value,&n);
    if(!data) { f->status=EINVAL; return -1; }
    /* Keep prior SAY output ordered before a private Windows CRT transport. */
    if(!f->regular && f->writable) fflush(stdout);
    if(!f->file || !f->writable) f->status=EBADF;
    else if(offset>=0 && (!f->regular || cs_seek(f->file,offset,SEEK_SET))) f->status=errno ? errno:ESPIPE;
    else {
        clearerr(f->file); got=fwrite(data,1,n,f->file); f->status=got!=n ? (errno ? errno:EIO):0;
        if(fflush(f->file)) f->status=errno ? errno:EIO;
        if(replace && !f->status && cs_truncate(f->file,n)) f->status=errno ? errno:EIO;
    }
    free(data); return f->status ? -1:(rxinteger)got;
}
METHODPROCEDURE(cs_write) { CS_RECEIVER RETURNINT(cs_write_bytes(f,ARG1,GETINT(ARG2),0)); RESETSIGNAL }
METHODPROCEDURE(cs_replace) { CS_RECEIVER RETURNINT(cs_write_bytes(f,ARG1,0,1)); RESETSIGNAL }
#endif
LOADFUNCS
ADDCLASS("_rxcstream.transport");
ADDFACTORYPROC(cs_open,"_rxcstream.transport","._rxcstream..transport","name=.string,mode=.string");
ADDMETHODPROC(cs_ready,"_rxcstream.transport","ready",".int","");
ADDMETHODPROC(cs_regular,"_rxcstream.transport","positionable",".int","");
ADDMETHODPROC(cs_error,"_rxcstream.transport","error",".int","");
ADDMETHODPROC(cs_eof,"_rxcstream.transport","eof",".int","");
ADDMETHODPROC(cs_end,"_rxcstream.transport","close",".int","");
ADDMETHODPROC(cs_load,"_rxcstream.transport","load",".string","");
ADDMETHODPROC(cs_read,"_rxcstream.transport","read",".string","count=.int");
ADDMETHODPROC(cs_record,"_rxcstream.transport","record",".string","delimiter=.string");
ADDMETHODPROC(cs_write,"_rxcstream.transport","write",".int","bytes=.string,offset=.int");
ADDMETHODPROC(cs_replace,"_rxcstream.transport","replace",".int","bytes=.string");
ENDLOADFUNCS
