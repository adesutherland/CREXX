/* cREXX License (MIT), Copyright (c) 2026 the cREXX contributors.
 * Host-only injectable stream provider: XOR is deliberately not a CMS codec.
 * Every byte crosses the hook, proving conversion precedes lexical analysis.
 */
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

typedef struct {
    FILE *file;
    int writing, encoded, fail;
    size_t transferred;
} text_stream;
static int encoded;
static unsigned matched_opens;

int crexx_cms_text_encoding(const char *name) {
    if (name && !strcmp(name, "TEST-XOR")) { encoded = 1; return 0; }
    if (name && (!strcmp(name,"UTF8") || !strcmp(name,"utf8") ||
                 !strcmp(name,"UTF-8") || !strcmp(name,"utf-8"))) {
        encoded = 0; return 0;
    }
    errno = EINVAL;
    return -1;
}

static ssize_t read_text(void *cookie, char *out, size_t size) {
    text_stream *s = cookie;
    size_t n, i;
    if (s->fail == 1 && s->transferred) { errno = EILSEQ; return -1; }
    if (size > 17) size = 17; /* Partial reads and buffer growth are intentional. */
    n = fread(out, 1, size, s->file);
    if (ferror(s->file)) return -1;
    if (s->encoded) for (i = 0; i < n; ++i) out[i] ^= 0x80;
    s->transferred += n;
    return (ssize_t)n;
}
static ssize_t write_text(void *cookie, const char *in, size_t size) {
    text_stream *s = cookie;
    size_t i;
    if (s->fail == 2 || s->fail == 5) {
        errno = s->fail == 2 ? EIO : EILSEQ;
        return -1;
    }
    for (i = 0; i < size; ++i)
        if (fputc((unsigned char)in[i] ^ (s->encoded ? 0x80 : 0), s->file) == EOF)
            return -1;
    s->transferred += size;
    return (ssize_t)size;
}
static int close_text(void *cookie) {
    text_stream *s = cookie;
    int fail = s->fail == (s->writing ? 4 : 3);
    int rc = fclose(s->file);
    free(s);
    if (fail) { errno = EIO; return -1; }
    return rc;
}
#ifdef __APPLE__
static int read_apple(void *s, char *p, int n) { return (int)read_text(s,p,(size_t)n); }
static int write_apple(void *s, const char *p, int n) { return (int)write_text(s,p,(size_t)n); }
#endif

FILE *crexx_cms_text_open(const char *path, const char *mode) {
    text_stream *s;
    FILE *stream, *file;
    const char *failure = getenv("CREXX_TEST_TEXT_FAILURE");
    const char *match = getenv("CREXX_TEST_TEXT_MATCH");
    const char *log = getenv("CREXX_TEST_TEXT_LOG");
    const char *skip = getenv("CREXX_TEST_TEXT_SKIP");
    int writing = mode[0] == 'w';
    if (strchr(mode,'b') || (mode[0] != 'r' && !writing)) { errno=EINVAL; return NULL; }
    file = fopen(path, writing ? "wb" : "rb");
    if (!file) return NULL;
    s = calloc(1,sizeof(*s));
    if (!s) { fclose(file); return NULL; }
    s->file=file; s->writing=writing; s->encoded=encoded;
    if (failure && (!match || strstr(path,match)) &&
        matched_opens++ >= (unsigned)(skip ? atoi(skip) : 0)) {
        if (!strcmp(failure,"read")) s->fail=1;
        if (!strcmp(failure,"write")) s->fail=2;
        if (!strcmp(failure,"close-read")) s->fail=3;
        if (!strcmp(failure,"close-write")) s->fail=4;
        if (!strcmp(failure,"convert-write")) s->fail=5;
    }
#ifdef __APPLE__
    stream=funopen(s,writing ? NULL : read_apple,writing ? write_apple : NULL,NULL,close_text);
#else
    {
        cookie_io_functions_t io = {0};
        io.read=writing ? NULL : read_text; io.write=writing ? write_text : NULL;
        io.close=close_text;
        stream=fopencookie(s,writing ? "w" : "r",io);
    }
#endif
    if (!stream) { fclose(file); free(s); return NULL; }
    if (log) {
        FILE *trace=fopen(log,"ab");
        if (trace) { fprintf(trace,"%s %s\n",mode,path); fclose(trace); }
    }
    /* Exercise both immediate write failures and buffered failures at close. */
    if (s->fail == 2) setvbuf(stream,NULL,_IONBF,0);
    return stream;
}
