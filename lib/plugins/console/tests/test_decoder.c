#include "rxconsole_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static rxcon_decoder decoder;
static rxcon_event event;
static void feed(const char *s) {CHECK(rxcon_feed(&decoder,s,strlen(s))==0);}
int main(void) {
    int i;const char *mouse="\033[<0;12;7M";
    rxcon_decoder_init(&decoder);
    feed("\033");CHECK(rxcon_decode(&decoder,&event,0)==0);
    CHECK(rxcon_decode(&decoder,&event,1)==RXCON_KEY);CHECK(!strcmp(event.text,"Escape"));
    feed("\033[");CHECK(rxcon_decode(&decoder,&event,0)==0);feed("1;5A");
    CHECK(rxcon_decode(&decoder,&event,0)==RXCON_KEY);CHECK(!strcmp(event.text,"Up"));CHECK(event.fields[4]==4);
    feed("\xc3");CHECK(rxcon_decode(&decoder,&event,0)==0);feed("\xa9");
    CHECK(rxcon_decode(&decoder,&event,0)==RXCON_TEXT);CHECK(!strcmp(event.text,"\xc3\xa9"));
    feed("\xff");CHECK(rxcon_decode(&decoder,&event,0)==RXCON_TEXT);CHECK(!strcmp(event.text,"\xef\xbf\xbd"));
    for(i=0;mouse[i];++i) {
        CHECK(rxcon_feed(&decoder,mouse+i,1)==0);
        CHECK(rxcon_decode(&decoder,&event,0)==(mouse[i+1]?0:RXCON_MOUSE));
    }
    CHECK(event.fields[0]==11 && event.fields[1]==6 && event.fields[5]==1 && event.fields[7]==1);
    feed("\033[<32;13;7M");CHECK(rxcon_decode(&decoder,&event,0)==RXCON_MOUSE);CHECK(event.fields[7]==3);
    feed("\033[<0;13;7m");CHECK(rxcon_decode(&decoder,&event,0)==RXCON_MOUSE);CHECK(event.fields[5]==0 && event.fields[7]==2);
    feed("\033[<65;1;1M");CHECK(rxcon_decode(&decoder,&event,0)==RXCON_WHEEL);CHECK(event.fields[9]==1);
    feed("\033[200~q\n\033[A\033[20");CHECK(rxcon_decode(&decoder,&event,0)==0);
    feed("1~x");CHECK(rxcon_decode(&decoder,&event,0)==RXCON_PASTE);CHECK(!strcmp(event.text,"q\n\033[A"));
    CHECK(rxcon_decode(&decoder,&event,0)==RXCON_TEXT);CHECK(!strcmp(event.text,"x"));
    feed("\033[200~");CHECK(rxcon_decode(&decoder,&event,0)==0);
    for(i=0;i<RXCON_TEXT_MAX+1;++i){feed("a");CHECK(rxcon_decode(&decoder,&event,0)==0);}
    feed("\033[201~z");CHECK(rxcon_decode(&decoder,&event,0)==RXCON_LIMIT);
    CHECK(rxcon_decode(&decoder,&event,0)==RXCON_TEXT);CHECK(!strcmp(event.text,"z"));
    feed("\033[<0;0;1M");CHECK(rxcon_decode(&decoder,&event,0)==RXCON_UNSUPPORTED);
    feed("\033[<0;99999999999999999999;1M");CHECK(rxcon_decode(&decoder,&event,0)==RXCON_UNSUPPORTED);
    feed("\003");CHECK(rxcon_decode(&decoder,&event,0)==RXCON_INTERRUPT);
    puts("PASS: incremental console decoder");return 0;
}
