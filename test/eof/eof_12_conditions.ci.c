/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -ci
#include <assert.h>
#include <string.h>


enum YYCONDTYPE {
	yycinit,
	yyccomment,
};


int lex(const char* cur)
{
    const char* lim;
    const char* mrk;

    char yych;

    int condition = yycinit;

    lim = cur + strlen(cur);
loop:
    mrk = cur;

    
	switch (condition) {
	case yycinit:
		goto yyc_init;
	case yyccomment:
		goto yyc_comment;
	}
/* *********************************** */
yyc_init:
	yych = *cur;
	switch (yych) {
	case '\t':
	case ' ':	goto yy4;
	case '\n':	goto yy7;
	case '/':	goto yy10;
	default:
		if (lim <= cur) goto yyeof1_init;
		goto yy2;
	}
yy2:
	++cur;
yy3:
	{ return -1; }
yy4:
	yych = *++cur;
	switch (yych) {
	case '\t':
	case ' ':	goto yy4;
	default:	goto yy6;
	}
yy6:
	{ goto loop; }
yy7:
	yych = *++cur;
	switch (yych) {
	case '\n':	goto yy7;
	default:	goto yy9;
	}
yy9:
	{ goto loop; }
yy10:
	yych = *++cur;
	switch (yych) {
	case '*':	goto yy11;
	case '/':	goto yy13;
	default:	goto yy3;
	}
yy11:
	++cur;
	condition = yyccomment;
	goto yyc_comment;
yy13:
	yych = *++cur;
	switch (yych) {
	case '\n':	goto yy15;
	default:
		if (lim <= cur) goto yy15;
		goto yy13;
	}
yy15:
	{ goto loop; }
yyeof1_init:
	{ return  0; }
/* *********************************** */
yyc_comment:
	yych = *cur;
	switch (yych) {
	case '\n':	goto yy21;
	case '*':	goto yy24;
	default:
		if (lim <= cur) goto yyeof1_comment;
		goto yy18;
	}
yy18:
	yych = *++cur;
	switch (yych) {
	case '\n':	goto yy20;
	case '*':	goto yy27;
	default:
		if (lim <= cur) goto yy20;
		goto yy18;
	}
yy20:
	goto yyc_comment;
yy21:
	yych = *++cur;
	switch (yych) {
	case '\n':	goto yy21;
	default:	goto yy23;
	}
yy23:
	{ goto loop; }
yy24:
	yych = *++cur;
	switch (yych) {
	case '*':	goto yy24;
	case '/':	goto yy29;
	default:	goto yy26;
	}
yy26:
	goto yyc_comment;
yy27:
	yych = *++cur;
	switch (yych) {
	case '*':	goto yy27;
	case '/':	goto yy29;
	default:	goto yy20;
	}
yy29:
	++cur;
	condition = yycinit;
	{ goto loop; }
yyeof1_comment:
	{ return -1; }

}


int main(void)
{
    assert(!lex("/* hello, */ // world !"));

    return 0;
}
