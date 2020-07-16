/* Generated by re2c */
#line 1 "c/submatch/01_stags.--lang(c).re"
// re2c $INPUT -o $OUTPUT 
#include <assert.h>
#include <stdint.h>

static uint32_t num(const char *s, const char *e)
{
    uint32_t n = 0;
    for (; s < e; ++s) n = n * 10 + (*s - '0');
    return n;
}

static const uint64_t ERROR = ~0lu;

static uint64_t lex(const char *YYCURSOR)
{
    const char *YYMARKER, *o1, *o2, *o3, *o4;
    const char *yyt1;const char *yyt2;const char *yyt3;const char *yyt4;

    
#line 23 "c/submatch/01_stags.--lang(c).c"
{
	char yych;
	yych = *YYCURSOR;
	switch (yych) {
	case '0':
		yyt1 = YYCURSOR;
		goto yy4;
	case '1':
		yyt1 = YYCURSOR;
		goto yy5;
	case '2':
		yyt1 = YYCURSOR;
		goto yy6;
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		yyt1 = YYCURSOR;
		goto yy7;
	default:	goto yy2;
	}
yy2:
	++YYCURSOR;
yy3:
#line 34 "c/submatch/01_stags.--lang(c).re"
	{ return ERROR; }
#line 53 "c/submatch/01_stags.--lang(c).c"
yy4:
	yych = *(YYMARKER = ++YYCURSOR);
	switch (yych) {
	case '.':	goto yy8;
	default:	goto yy3;
	}
yy5:
	yych = *(YYMARKER = ++YYCURSOR);
	switch (yych) {
	case '.':	goto yy8;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy10;
	default:	goto yy3;
	}
yy6:
	yych = *(YYMARKER = ++YYCURSOR);
	switch (yych) {
	case '.':	goto yy8;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':	goto yy10;
	case '5':	goto yy11;
	case '6':
	case '7':
	case '8':
	case '9':	goto yy12;
	default:	goto yy3;
	}
yy7:
	yych = *(YYMARKER = ++YYCURSOR);
	switch (yych) {
	case '.':	goto yy8;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy12;
	default:	goto yy3;
	}
yy8:
	yych = *++YYCURSOR;
	switch (yych) {
	case '0':
		yyt2 = YYCURSOR;
		goto yy13;
	case '1':
		yyt2 = YYCURSOR;
		goto yy14;
	case '2':
		yyt2 = YYCURSOR;
		goto yy15;
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		yyt2 = YYCURSOR;
		goto yy16;
	default:	goto yy9;
	}
yy9:
	YYCURSOR = YYMARKER;
	goto yy3;
yy10:
	yych = *++YYCURSOR;
	switch (yych) {
	case '.':	goto yy8;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy12;
	default:	goto yy9;
	}
yy11:
	yych = *++YYCURSOR;
	switch (yych) {
	case '.':	goto yy8;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':	goto yy12;
	default:	goto yy9;
	}
yy12:
	yych = *++YYCURSOR;
	switch (yych) {
	case '.':	goto yy8;
	default:	goto yy9;
	}
yy13:
	yych = *++YYCURSOR;
	switch (yych) {
	case '.':	goto yy17;
	default:	goto yy9;
	}
yy14:
	yych = *++YYCURSOR;
	switch (yych) {
	case '.':	goto yy17;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy16;
	default:	goto yy9;
	}
yy15:
	yych = *++YYCURSOR;
	switch (yych) {
	case '.':	goto yy17;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':	goto yy16;
	case '5':	goto yy18;
	case '6':
	case '7':
	case '8':
	case '9':	goto yy13;
	default:	goto yy9;
	}
yy16:
	yych = *++YYCURSOR;
	switch (yych) {
	case '.':	goto yy17;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy13;
	default:	goto yy9;
	}
yy17:
	yych = *++YYCURSOR;
	switch (yych) {
	case '0':
		yyt3 = YYCURSOR;
		goto yy19;
	case '1':
		yyt3 = YYCURSOR;
		goto yy20;
	case '2':
		yyt3 = YYCURSOR;
		goto yy21;
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		yyt3 = YYCURSOR;
		goto yy22;
	default:	goto yy9;
	}
yy18:
	yych = *++YYCURSOR;
	switch (yych) {
	case '.':	goto yy17;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':	goto yy13;
	default:	goto yy9;
	}
yy19:
	yych = *++YYCURSOR;
	switch (yych) {
	case '.':	goto yy23;
	default:	goto yy9;
	}
yy20:
	yych = *++YYCURSOR;
	switch (yych) {
	case '.':	goto yy23;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy22;
	default:	goto yy9;
	}
yy21:
	yych = *++YYCURSOR;
	switch (yych) {
	case '.':	goto yy23;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':	goto yy22;
	case '5':	goto yy24;
	case '6':
	case '7':
	case '8':
	case '9':	goto yy19;
	default:	goto yy9;
	}
yy22:
	yych = *++YYCURSOR;
	switch (yych) {
	case '.':	goto yy23;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy19;
	default:	goto yy9;
	}
yy23:
	yych = *++YYCURSOR;
	switch (yych) {
	case '0':
		yyt4 = YYCURSOR;
		goto yy25;
	case '1':
		yyt4 = YYCURSOR;
		goto yy26;
	case '2':
		yyt4 = YYCURSOR;
		goto yy27;
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		yyt4 = YYCURSOR;
		goto yy28;
	default:	goto yy9;
	}
yy24:
	yych = *++YYCURSOR;
	switch (yych) {
	case '.':	goto yy23;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':	goto yy19;
	default:	goto yy9;
	}
yy25:
	yych = *++YYCURSOR;
	if (yych <= 0x00) goto yy29;
	goto yy9;
yy26:
	yych = *++YYCURSOR;
	switch (yych) {
	case 0x00:	goto yy29;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy28;
	default:	goto yy9;
	}
yy27:
	yych = *++YYCURSOR;
	switch (yych) {
	case 0x00:	goto yy29;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':	goto yy28;
	case '5':	goto yy31;
	case '6':
	case '7':
	case '8':
	case '9':	goto yy25;
	default:	goto yy9;
	}
yy28:
	yych = *++YYCURSOR;
	switch (yych) {
	case 0x00:	goto yy29;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy25;
	default:	goto yy9;
	}
yy29:
	++YYCURSOR;
	o1 = yyt1;
	o2 = yyt2;
	o3 = yyt3;
	o4 = yyt4;
#line 28 "c/submatch/01_stags.--lang(c).re"
	{
        return num(o4, YYCURSOR - 1)
            + (num(o3, o4 - 1) << 8)
            + (num(o2, o3 - 1) << 16)
            + (num(o1, o2 - 1) << 24);
    }
#line 411 "c/submatch/01_stags.--lang(c).c"
yy31:
	yych = *++YYCURSOR;
	switch (yych) {
	case 0x00:	goto yy29;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':	goto yy25;
	default:	goto yy9;
	}
}
#line 35 "c/submatch/01_stags.--lang(c).re"

}

int main()
{
    assert(lex("1.2.3.4") == 0x01020304);
    assert(lex("127.0.0.1") == 0x7f000001);
    assert(lex("255.255.255.255") == 0xffffffff);
    assert(lex("1.2.3.") == ERROR);
    assert(lex("1.2.3.256") == ERROR);
    return 0;
}
