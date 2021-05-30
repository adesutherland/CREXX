/* Generated by re2c */
#line 1 "../../../benchmarks/submatch_dfa_aot/src/re2c/14__alt2_4.re"
#include <assert.h>
#include <stdlib.h>
#include "common.h"

#line 15 "../../../benchmarks/submatch_dfa_aot/src/re2c/common.re"


typedef struct taglist_t {
    struct taglist_t *pred;
    long dist;
} taglist_t;

typedef struct taglistpool_t {
    taglist_t *head;
    taglist_t *next;
    taglist_t *last;
} taglistpool_t;

typedef struct {
    char *buf;
    char *lim;
    char *cur;
    char *mar;
    char *tok;
    char *yyt1;
char *yyt10;
char *yyt11;
char *yyt12;
char *yyt13;
char *yyt14;
char *yyt15;
char *yyt16;
char *yyt2;
char *yyt3;
char *yyt4;
char *yyt5;
char *yyt6;
char *yyt7;
char *yyt8;
char *yyt9;

    
    taglistpool_t tlp;
    int eof;
} input_t;

static inline void taglistpool_clear(taglistpool_t *tlp, input_t *in)
{
    tlp->next = tlp->head;
    
}

static inline void taglistpool_init(taglistpool_t *tlp)
{
    static const unsigned size = 1024 * 1024;
    tlp->head = (taglist_t*)malloc(size * sizeof(taglist_t));
    tlp->next = tlp->head;
    tlp->last = tlp->head + size;
}

static inline void taglistpool_free(taglistpool_t *tlp)
{
    free(tlp->head);
    tlp->head = tlp->next = tlp->last = NULL;
}

static inline void taglist(taglist_t **ptl, const char *b, const char *t, taglistpool_t *tlp)
{
#ifdef GROW_MTAG_LIST
    if (tlp->next >= tlp->last) {
        const unsigned size = tlp->last - tlp->head;
        taglist_t *head = (taglist_t*)malloc(2 * size * sizeof(taglist_t));
        memcpy(head, tlp->head, size * sizeof(taglist_t));
        free(tlp->head);
        tlp->head = head;
        tlp->next = head + size;
        tlp->last = head + size * 2;
    }
#else
    assert(tlp->next < tlp->last);
#endif
    taglist_t *tl = tlp->next++;
    tl->pred = *ptl;
    tl->dist = t - b;
    *ptl = tl;
}

#line 4 "../../../benchmarks/submatch_dfa_aot/src/re2c/include/fill.re"


#define YYMAXFILL 3


static inline int fill(input_t *in, size_t need)
{
    size_t free;
    if (in->eof) return 1;

    free = in->tok - in->buf;
    assert(free >= need);

    memmove(in->buf, in->tok, in->lim - in->tok);
    in->lim -= free;
    in->cur -= free;
    in->mar -= free;
    in->tok -= free;
    if (in->yyt1) in->yyt1 -= free;
if (in->yyt10) in->yyt10 -= free;
if (in->yyt11) in->yyt11 -= free;
if (in->yyt12) in->yyt12 -= free;
if (in->yyt13) in->yyt13 -= free;
if (in->yyt14) in->yyt14 -= free;
if (in->yyt15) in->yyt15 -= free;
if (in->yyt16) in->yyt16 -= free;
if (in->yyt2) in->yyt2 -= free;
if (in->yyt3) in->yyt3 -= free;
if (in->yyt4) in->yyt4 -= free;
if (in->yyt5) in->yyt5 -= free;
if (in->yyt6) in->yyt6 -= free;
if (in->yyt7) in->yyt7 -= free;
if (in->yyt8) in->yyt8 -= free;
if (in->yyt9) in->yyt9 -= free;


    in->lim += fread(in->lim, 1, free, stdin);

    if (in->lim < in->buf + SIZE) {
        in->eof = 1;
        memset(in->lim, 0, YYMAXFILL);
        in->lim += YYMAXFILL;
    }

    return 0;
}

static inline void init_input(input_t *in)
{
    in->buf = (char*) malloc(SIZE + YYMAXFILL);
    in->lim = in->buf + SIZE;
    in->cur = in->lim;
    in->mar = in->lim;
    in->tok = in->lim;
    in->yyt1 = 0;
in->yyt10 = 0;
in->yyt11 = 0;
in->yyt12 = 0;
in->yyt13 = 0;
in->yyt14 = 0;
in->yyt15 = 0;
in->yyt16 = 0;
in->yyt2 = 0;
in->yyt3 = 0;
in->yyt4 = 0;
in->yyt5 = 0;
in->yyt6 = 0;
in->yyt7 = 0;
in->yyt8 = 0;
in->yyt9 = 0;

    
    taglistpool_init(&in->tlp);
    in->eof = 0;
}


static inline void free_input(input_t *in)
{
    free(in->buf);
    taglistpool_free(&in->tlp);
}

static int lex(input_t *in, Output *out);

int main(int argc, char **argv)
{
    PRE;
    input_t in;
    Output out;

    init_input(&in);
    init_output(&out);

    switch (lex(&in, &out)) {
        case 0:  break;
        case 1:  fprintf(stderr, "*** %s: syntax error\n", argv[0]); break;
        case 2:  fprintf(stderr, "*** %s: yyfill error\n", argv[0]); break;
        default: fprintf(stderr, "*** %s: panic\n", argv[0]); break;
    }

    flush(&out);
    free_output(&out);
    free_input(&in);

    POST;
    return 0;
}


static int lex(input_t *in, Output *out)
{
    const char
        *a1, *b1, *c1, *d1,
        *a2, *b2, *c2, *d2,
        *a3, *b3, *c3, *d3,
        *a4, *b4, *c4, *d4;
loop:
    in->tok = in->cur;

#line 212 "gen/re2c/14__alt2_4-stadfa.c"
{
	char yych;
	if ((in->lim - in->cur) < 3) if (fill(in, 3) != 0) return 1;
	yych = *in->cur;
	switch (yych) {
	case 0x00:	goto yy2;
	case '\n':	goto yy6;
	case 'a':	goto yy8;
	case 'b':	goto yy9;
	case 'c':	goto yy10;
	case 'd':	goto yy11;
	default:	goto yy4;
	}
yy2:
	++in->cur;
#line 3 "../../../benchmarks/submatch_dfa_aot/src/re2c/include/fill.re"
	{ return 0; }
#line 230 "gen/re2c/14__alt2_4-stadfa.c"
yy4:
	++in->cur;
yy5:
#line 13 "../../../benchmarks/submatch_dfa_aot/src/re2c/14__alt2_4.re"
	{ return 1; }
#line 236 "gen/re2c/14__alt2_4-stadfa.c"
yy6:
	++in->cur;
	in->yyt3 = in->yyt4 = in->yyt5 = in->yyt6 = in->yyt7 = in->yyt8 = in->yyt11 = in->yyt12 = in->yyt13 = in->yyt14 = in->yyt15 = in->yyt16 = NULL;
	in->yyt1 = in->yyt2 = in->yyt9 = in->yyt10 = in->cur - 1;
yy7:
	a1 = in->yyt1;
	a2 = in->yyt2;
	b1 = in->yyt3;
	b2 = in->yyt4;
	c1 = in->yyt5;
	c2 = in->yyt6;
	d1 = in->yyt7;
	d2 = in->yyt8;
	a3 = in->yyt9;
	a4 = in->yyt10;
	b3 = in->yyt11;
	b4 = in->yyt12;
	c3 = in->yyt13;
	c4 = in->yyt14;
	d3 = in->yyt15;
	d4 = in->yyt16;
#line 25 "../../../benchmarks/submatch_dfa_aot/src/re2c/14__alt2_4.re"
	{
        if (a1)      { outc(out, 'A'); outs(out, a1, a2); }
        else if (b1) { outc(out, 'B'); outs(out, b1, b2); }
        else if (c1) { outc(out, 'C'); outs(out, c1, c2); }
        else if (d1) { outc(out, 'D'); outs(out, d1, d2); }
        if (a3)      { outc(out, 'A'); outs(out, a3, a4); }
        else if (b3) { outc(out, 'B'); outs(out, b3, b4); }
        else if (c3) { outc(out, 'C'); outs(out, c3, c4); }
        else if (d3) { outc(out, 'D'); outs(out, d3, d4); }
        outc(out, '\n');
        goto loop;
    }
#line 271 "gen/re2c/14__alt2_4-stadfa.c"
yy8:
	yych = *(in->mar = ++in->cur);
	in->yyt1 = in->cur - 1;
	switch (yych) {
	case '\n':	goto yy12;
	case 'a':	goto yy13;
	case 'b':	goto yy16;
	case 'c':	goto yy17;
	case 'd':	goto yy18;
	default:	goto yy5;
	}
yy9:
	yych = *(in->mar = ++in->cur);
	in->yyt3 = in->yyt4 = in->yyt5 = in->yyt6 = in->yyt7 = in->yyt8 = in->yyt12 = in->yyt14 = NULL;
	in->yyt1 = in->yyt2 = in->yyt10 = in->yyt11 = in->cur - 1;
	switch (yych) {
	case '\n':	goto yy19;
	case 'a':	goto yy20;
	case 'b':	goto yy21;
	case 'c':	goto yy23;
	case 'd':	goto yy24;
	default:	goto yy5;
	}
yy10:
	yych = *(in->mar = ++in->cur);
	in->yyt3 = in->yyt4 = in->yyt5 = in->yyt6 = in->yyt7 = in->yyt8 = in->yyt10 = in->yyt14 = NULL;
	in->yyt1 = in->yyt2 = in->yyt12 = in->yyt13 = in->cur - 1;
	switch (yych) {
	case '\n':	goto yy25;
	case 'a':	goto yy26;
	case 'b':	goto yy27;
	case 'c':	goto yy28;
	case 'd':	goto yy30;
	default:	goto yy5;
	}
yy11:
	yych = *(in->mar = ++in->cur);
	in->yyt3 = in->yyt4 = in->yyt5 = in->yyt6 = in->yyt7 = in->yyt8 = in->yyt10 = in->yyt12 = NULL;
	in->yyt1 = in->yyt2 = in->yyt14 = in->yyt15 = in->cur - 1;
	switch (yych) {
	case '\n':	goto yy31;
	case 'a':	goto yy32;
	case 'b':	goto yy33;
	case 'c':	goto yy34;
	case 'd':	goto yy35;
	default:	goto yy5;
	}
yy12:
	++in->cur;
	in->yyt3 = in->yyt4 = in->yyt5 = in->yyt6 = in->yyt7 = in->yyt8 = in->yyt11 = in->yyt12 = in->yyt13 = in->yyt14 = in->yyt15 = in->yyt16 = NULL;
	in->yyt2 = in->yyt9 = in->yyt10 = in->cur - 1;
	goto yy7;
yy13:
	++in->cur;
	if ((in->lim - in->cur) < 2) if (fill(in, 2) != 0) return 1;
	yych = *in->cur;
	switch (yych) {
	case '\n':	goto yy12;
	case 'a':	goto yy13;
	case 'b':	goto yy16;
	case 'c':	goto yy17;
	case 'd':	goto yy18;
	default:	goto yy15;
	}
yy15:
	in->cur = in->mar;
	goto yy5;
yy16:
	yych = *++in->cur;
	in->yyt3 = in->yyt4 = in->yyt5 = in->yyt6 = in->yyt7 = in->yyt8 = in->yyt12 = in->yyt14 = NULL;
	in->yyt2 = in->yyt11 = in->cur - 1;
	switch (yych) {
	case '\n':	goto yy19;
	case 'b':	goto yy37;
	default:	goto yy15;
	}
yy17:
	yych = *++in->cur;
	in->yyt3 = in->yyt4 = in->yyt5 = in->yyt6 = in->yyt7 = in->yyt8 = in->yyt10 = in->yyt14 = NULL;
	in->yyt2 = in->yyt13 = in->cur - 1;
	switch (yych) {
	case '\n':	goto yy25;
	case 'c':	goto yy39;
	default:	goto yy15;
	}
yy18:
	yych = *++in->cur;
	in->yyt3 = in->yyt4 = in->yyt5 = in->yyt6 = in->yyt7 = in->yyt8 = in->yyt10 = in->yyt12 = NULL;
	in->yyt2 = in->yyt15 = in->cur - 1;
	switch (yych) {
	case '\n':	goto yy31;
	case 'd':	goto yy41;
	default:	goto yy15;
	}
yy19:
	++in->cur;
	in->yyt9 = in->yyt10 = in->yyt13 = in->yyt14 = in->yyt15 = in->yyt16 = NULL;
	in->yyt12 = in->cur - 1;
	goto yy7;
yy20:
	yych = *++in->cur;
	in->yyt3 = in->yyt10;
	in->yyt1 = in->yyt2 = in->yyt5 = in->yyt6 = in->yyt7 = in->yyt8 = in->yyt12 = in->yyt14 = NULL;
	in->yyt4 = in->yyt9 = in->cur - 1;
	switch (yych) {
	case '\n':	goto yy43;
	case 'a':	goto yy44;
	default:	goto yy15;
	}
yy21:
	++in->cur;
	if ((in->lim - in->cur) < 2) if (fill(in, 2) != 0) return 1;
	yych = *in->cur;
	in->yyt7 = in->yyt14;
	in->yyt5 = in->yyt12;
	switch (yych) {
	case '\n':	goto yy19;
	case 'a':	goto yy20;
	case 'b':	goto yy21;
	case 'c':	goto yy23;
	case 'd':	goto yy24;
	default:	goto yy15;
	}
yy23:
	yych = *++in->cur;
	in->yyt3 = in->yyt10;
	in->yyt1 = in->yyt2 = in->yyt5 = in->yyt6 = in->yyt7 = in->yyt8 = in->yyt14 = NULL;
	in->yyt4 = in->yyt13 = in->cur - 1;
	switch (yych) {
	case '\n':	goto yy25;
	case 'c':	goto yy39;
	default:	goto yy15;
	}
yy24:
	yych = *++in->cur;
	in->yyt3 = in->yyt10;
	in->yyt1 = in->yyt2 = in->yyt5 = in->yyt6 = in->yyt7 = in->yyt8 = in->yyt12 = NULL;
	in->yyt4 = in->yyt15 = in->cur - 1;
	switch (yych) {
	case '\n':	goto yy31;
	case 'd':	goto yy41;
	default:	goto yy15;
	}
yy25:
	++in->cur;
	in->yyt9 = in->yyt10 = in->yyt11 = in->yyt12 = in->yyt15 = in->yyt16 = NULL;
	in->yyt14 = in->cur - 1;
	goto yy7;
yy26:
	yych = *++in->cur;
	in->yyt5 = in->yyt12;
	in->yyt1 = in->yyt2 = in->yyt3 = in->yyt4 = in->yyt7 = in->yyt8 = in->yyt10 = in->yyt14 = NULL;
	in->yyt6 = in->yyt9 = in->cur - 1;
	switch (yych) {
	case '\n':	goto yy43;
	case 'a':	goto yy44;
	default:	goto yy15;
	}
yy27:
	yych = *++in->cur;
	in->yyt5 = in->yyt12;
	in->yyt1 = in->yyt2 = in->yyt3 = in->yyt4 = in->yyt7 = in->yyt8 = in->yyt14 = NULL;
	in->yyt6 = in->yyt11 = in->cur - 1;
	switch (yych) {
	case '\n':	goto yy19;
	case 'b':	goto yy37;
	default:	goto yy15;
	}
yy28:
	++in->cur;
	if ((in->lim - in->cur) < 2) if (fill(in, 2) != 0) return 1;
	yych = *in->cur;
	in->yyt7 = in->yyt14;
	in->yyt3 = in->yyt10;
	switch (yych) {
	case '\n':	goto yy25;
	case 'a':	goto yy26;
	case 'b':	goto yy27;
	case 'c':	goto yy28;
	case 'd':	goto yy30;
	default:	goto yy15;
	}
yy30:
	yych = *++in->cur;
	in->yyt5 = in->yyt12;
	in->yyt1 = in->yyt2 = in->yyt3 = in->yyt4 = in->yyt7 = in->yyt8 = in->yyt10 = NULL;
	in->yyt6 = in->yyt15 = in->cur - 1;
	switch (yych) {
	case '\n':	goto yy31;
	case 'd':	goto yy41;
	default:	goto yy15;
	}
yy31:
	++in->cur;
	in->yyt9 = in->yyt10 = in->yyt11 = in->yyt12 = in->yyt13 = in->yyt14 = NULL;
	in->yyt16 = in->cur - 1;
	goto yy7;
yy32:
	yych = *++in->cur;
	in->yyt7 = in->yyt14;
	in->yyt1 = in->yyt2 = in->yyt3 = in->yyt4 = in->yyt5 = in->yyt6 = in->yyt10 = in->yyt12 = NULL;
	in->yyt8 = in->yyt9 = in->cur - 1;
	switch (yych) {
	case '\n':	goto yy43;
	case 'a':	goto yy44;
	default:	goto yy15;
	}
yy33:
	yych = *++in->cur;
	in->yyt7 = in->yyt14;
	in->yyt1 = in->yyt2 = in->yyt3 = in->yyt4 = in->yyt5 = in->yyt6 = in->yyt12 = NULL;
	in->yyt8 = in->yyt11 = in->cur - 1;
	switch (yych) {
	case '\n':	goto yy19;
	case 'b':	goto yy37;
	default:	goto yy15;
	}
yy34:
	yych = *++in->cur;
	in->yyt7 = in->yyt14;
	in->yyt1 = in->yyt2 = in->yyt3 = in->yyt4 = in->yyt5 = in->yyt6 = in->yyt10 = NULL;
	in->yyt8 = in->yyt13 = in->cur - 1;
	switch (yych) {
	case '\n':	goto yy25;
	case 'c':	goto yy39;
	default:	goto yy15;
	}
yy35:
	++in->cur;
	if ((in->lim - in->cur) < 2) if (fill(in, 2) != 0) return 1;
	yych = *in->cur;
	in->yyt5 = in->yyt12;
	in->yyt3 = in->yyt10;
	switch (yych) {
	case '\n':	goto yy31;
	case 'a':	goto yy32;
	case 'b':	goto yy33;
	case 'c':	goto yy34;
	case 'd':	goto yy35;
	default:	goto yy15;
	}
yy37:
	++in->cur;
	if (in->lim <= in->cur) if (fill(in, 1) != 0) return 1;
	yych = *in->cur;
	in->yyt7 = in->yyt14;
	in->yyt5 = in->yyt12;
	switch (yych) {
	case '\n':	goto yy19;
	case 'b':	goto yy37;
	default:	goto yy15;
	}
yy39:
	++in->cur;
	if (in->lim <= in->cur) if (fill(in, 1) != 0) return 1;
	yych = *in->cur;
	in->yyt7 = in->yyt14;
	in->yyt3 = in->yyt10;
	switch (yych) {
	case '\n':	goto yy25;
	case 'c':	goto yy39;
	default:	goto yy15;
	}
yy41:
	++in->cur;
	if (in->lim <= in->cur) if (fill(in, 1) != 0) return 1;
	yych = *in->cur;
	in->yyt5 = in->yyt12;
	in->yyt3 = in->yyt10;
	switch (yych) {
	case '\n':	goto yy31;
	case 'd':	goto yy41;
	default:	goto yy15;
	}
yy43:
	++in->cur;
	in->yyt11 = in->yyt12 = in->yyt13 = in->yyt14 = in->yyt15 = in->yyt16 = NULL;
	in->yyt10 = in->cur - 1;
	goto yy7;
yy44:
	++in->cur;
	if (in->lim <= in->cur) if (fill(in, 1) != 0) return 1;
	yych = *in->cur;
	in->yyt7 = in->yyt14;
	in->yyt5 = in->yyt12;
	in->yyt3 = in->yyt10;
	switch (yych) {
	case '\n':	goto yy43;
	case 'a':	goto yy44;
	default:	goto yy15;
	}
}
#line 37 "../../../benchmarks/submatch_dfa_aot/src/re2c/14__alt2_4.re"

}
