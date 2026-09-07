/* Bounded, incremental UTF-8 / xterm input decoder, independently testable. */
#include "rxconsole_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void rxcon_decoder_init(rxcon_decoder *d) { memset(d, 0, sizeof(*d)); }
int rxcon_feed(rxcon_decoder *d, const void *bytes, size_t n) {
    if (n > sizeof(d->input)-d->used) return RXCON_LIMIT;
    memcpy(d->input+d->used, bytes, n); d->used += n; return 0;
}
static void consume(rxcon_decoder *d, size_t n) {
    memmove(d->input, d->input+n, d->used-n); d->used -= n;
}
static int named(rxcon_event *e, const char *key, int modifiers) {
    e->kind=RXCON_KEY; e->fields[4]=modifiers; e->fields[7]=1;
    snprintf(e->text, sizeof(e->text), "%s", key); return e->kind;
}
/* Return UTF-8 sequence length, zero if incomplete, -1 if malformed. */
static int utf8(const unsigned char *s, size_t n) {
    int count, i; unsigned cp;
    if (!n) return 0;
    if (s[0]<128) return 1;
    if (s[0]>=0xc2 && s[0]<=0xdf) { count=2; cp=s[0]&31; }
    else if (s[0]>=0xe0 && s[0]<=0xef) { count=3; cp=s[0]&15; }
    else if (s[0]>=0xf0 && s[0]<=0xf4) { count=4; cp=s[0]&7; }
    else return -1;
    for (i=1; i<count && (size_t)i<n; ++i) {
        if ((s[i]&0xc0)!=0x80) return -1;
        cp=(cp<<6)|(s[i]&63);
    }
    if (n<(size_t)count) return 0;
    if ((count==3 && cp<0x800) || (count==4 && cp<0x10000) ||
        (cp>=0xd800 && cp<=0xdfff) || cp>0x10ffff) return -1;
    return count;
}
static void sanitize_paste(rxcon_event *e, const char *bytes, size_t n) {
    size_t in=0, out=0;
    while (in<n) {
        int count=utf8((const unsigned char *)bytes+in,n-in);
        if (count<=0 || bytes[in]==0) {
            if (out+3>RXCON_TEXT_MAX) break;
            memcpy(e->text+out,"\xef\xbf\xbd",3); out+=3; ++in;
        } else {
            if (out+(size_t)count>RXCON_TEXT_MAX) break;
            memcpy(e->text+out,bytes+in,(size_t)count); out+=(size_t)count; in+=(size_t)count;
        }
    }
    e->text[out]=0;
}
static int csi(rxcon_decoder *d, rxcon_event *e, size_t n) {
    char seq[64], final; int a=0,b=0,c=0,mods=0; const char *key=NULL;
    if (n>=sizeof(seq)) { consume(d,n); return RXCON_UNSUPPORTED; }
    memcpy(seq,d->input,n); seq[n]=0; final=seq[n-1];
    /* Bound decimal runs before scanf: conversion outside int is undefined. */
    {
        size_t i;int digits=0;
        for(i=2;i+1<n;++i) {
            if(seq[i]>='0' && seq[i]<='9') {
                if(++digits>6){consume(d,n);return RXCON_UNSUPPORTED;}
            } else {
                digits=0;
                if(seq[i]!=';' && !(i==2 && seq[i]=='<')) {consume(d,n);return RXCON_UNSUPPORTED;}
            }
        }
    }
    if (n>=4 && seq[2]=='<') {
        char ending=0, extra=0;
        if (sscanf(seq+3,"%d;%d;%d%c%c",&a,&b,&c,&ending,&extra)!=4 ||
            a<0 || a>255 || b<1 || c<1 || b>100000 || c>100000 ||
            (ending!='M' && ending!='m')) { consume(d,n); return RXCON_UNSUPPORTED; }
        e->fields[0]=b-1; e->fields[1]=c-1;
        e->fields[4]=((a&4)?RXCON_SHIFT:0)|((a&8)?RXCON_ALTKEY:0)|((a&16)?RXCON_CTRL:0);
        if (a&64) {
            e->kind=RXCON_WHEEL;
            e->fields[(a&2)?8:9]=(a&1)?1:-1;
        } else {
            int button=(a&3)<3 ? (a&3)+1 : 0;
            e->kind=RXCON_MOUSE; e->fields[6]=button;
            e->fields[7]=(a&32)?3:(ending=='m'?2:1);
            if (button && !(a&32)) {
                if (ending=='m') d->buttons &= ~(1<<(button-1));
                else d->buttons |= 1<<(button-1);
            }
            if (!button && ending=='m') d->buttons=0;
            e->fields[5]=d->buttons;
        }
        consume(d,n); return e->kind;
    }
    (void)sscanf(seq+2,"%d;%d",&a,&b);
    if (b>=2 && b<=8) mods=b-1;
    switch(final) {
    case 'A': key="Up"; break; case 'B': key="Down"; break;
    case 'C': key="Right"; break; case 'D': key="Left"; break;
    case 'H': key="Home"; break; case 'F': key="End"; break;
    case 'Z': key="Tab"; mods=RXCON_SHIFT; break;
    case '~':
        switch(a) {
        case 1: case 7:key="Home";break; case 4:case 8:key="End";break;
        case 2:key="Insert";break; case 3:key="Delete";break;
        case 5:key="PageUp";break;case 6:key="PageDown";break;
        case 11:key="F1";break;case 12:key="F2";break;case 13:key="F3";break;
        case 14:key="F4";break;case 15:key="F5";break;case 17:key="F6";break;
        case 18:key="F7";break;case 19:key="F8";break;case 20:key="F9";break;
        case 21:key="F10";break;case 23:key="F11";break;case 24:key="F12";break;
        case 200:d->pasting=1; d->paste_used=0;d->overflow=0;break;
        }
        break;
    }
    consume(d,n);
    if (key) return named(e,key,mods);
    return d->pasting ? 0 : RXCON_UNSUPPORTED;
}
int rxcon_decode(rxcon_decoder *d, rxcon_event *e, int expire) {
    size_t n; int count; unsigned char ch;
    memset(e,0,sizeof(*e));
    if (d->pasting) {
        const unsigned char end[]={27,'[','2','0','1','~'};
        while (d->used) {
            size_t prefix=d->used<sizeof(end)?d->used:sizeof(end);
            if (!memcmp(d->input,end,prefix)) {
                if (d->used<sizeof(end)) return 0;
                consume(d,sizeof(end)); d->pasting=0;
                if (d->overflow) return RXCON_LIMIT;
                sanitize_paste(e,d->paste,d->paste_used); e->kind=RXCON_PASTE; return e->kind;
            }
            if (d->paste_used<RXCON_TEXT_MAX) d->paste[d->paste_used++]=(char)d->input[0];
            else d->overflow=1;
            consume(d,1);
        }
        return 0;
    }
    if (!d->used) return 0;
    ch=d->input[0];
    if (ch==27) {
        if (d->used==1) {
            if (!expire) return 0;
            consume(d,1); return named(e,"Escape",0);
        }
        if (d->input[1]=='[') {
            for (n=2;n<d->used;++n) if (d->input[n]>=0x40 && d->input[n]<=0x7e) {
                int result=csi(d,e,n+1);
                if (!result && d->pasting) return rxcon_decode(d,e,0);
                return result;
            }
            if (!expire && d->used<64) return 0;
            consume(d,d->used); return RXCON_UNSUPPORTED;
        }
        if (d->input[1]=='O') {
            const char *keys[]={"F1","F2","F3","F4"};
            if (d->used<3) { if (!expire) return 0; consume(d,d->used);return RXCON_UNSUPPORTED; }
            ch=d->input[2]; consume(d,3);
            if (ch>='P' && ch<='S') return named(e,keys[ch-'P'],0);
            if (ch=='H') return named(e,"Home",0);
            if (ch=='F') return named(e,"End",0);
            return RXCON_UNSUPPORTED;
        }
        count=utf8(d->input+1,d->used-1);
        if (!count && !expire) return 0;
        if (count<=0) { consume(d,1);return named(e,"Escape",0); }
        memcpy(e->text,d->input+1,(size_t)count);consume(d,(size_t)count+1);
        e->kind=RXCON_KEY;e->fields[4]=RXCON_ALTKEY;e->fields[7]=1;return e->kind;
    }
    if (ch<32 || ch==127) {
        char key[2]={(char)('a'+ch-1),0};consume(d,1);
        if (ch==3) { e->kind=RXCON_INTERRUPT;return e->kind; }
        if (ch==13 || ch==10) return named(e,"Enter",0);
        if (ch==9) return named(e,"Tab",0);
        if (ch==8 || ch==127) return named(e,"Backspace",0);
        if (!ch) return named(e,"Space",RXCON_CTRL);
        return named(e,key,RXCON_CTRL);
    }
    count=utf8(d->input,d->used);
    if (!count && !expire) return 0;
    if (count<=0) { memcpy(e->text,"\xef\xbf\xbd",4);consume(d,1); }
    else { memcpy(e->text,d->input,(size_t)count);consume(d,(size_t)count); }
    e->kind=RXCON_TEXT;return e->kind;
}
