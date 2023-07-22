/* Generated by re2c */
#line 1 "c/eof/03_eof_rule.re"
// re2c $INPUT -o $OUTPUT
#include <assert.h>

// Expect a null-terminated string.
static int lex(const char *str, unsigned int len) {
    const char *YYCURSOR = str, *YYLIMIT = str + len, *YYMARKER;
    int count = 0;

    for (;;) {
    
#line 14 "c/eof/03_eof_rule.c"
{
	char yych;
	yych = *YYCURSOR;
	switch (yych) {
		case ' ': goto yy3;
		case '\'': goto yy5;
		default:
			if (YYLIMIT <= YYCURSOR) goto yy10;
			goto yy1;
	}
yy1:
	++YYCURSOR;
yy2:
#line 17 "c/eof/03_eof_rule.re"
	{ return -1; }
#line 30 "c/eof/03_eof_rule.c"
yy3:
	yych = *++YYCURSOR;
	switch (yych) {
		case ' ': goto yy3;
		default: goto yy4;
	}
yy4:
#line 20 "c/eof/03_eof_rule.re"
	{ continue; }
#line 40 "c/eof/03_eof_rule.c"
yy5:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych >= 0x01) goto yy7;
	if (YYLIMIT <= YYCURSOR) goto yy2;
yy6:
	yych = *++YYCURSOR;
yy7:
	switch (yych) {
		case '\'': goto yy8;
		case '\\': goto yy9;
		default:
			if (YYLIMIT <= YYCURSOR) goto yy11;
			goto yy6;
	}
yy8:
	++YYCURSOR;
#line 19 "c/eof/03_eof_rule.re"
	{ ++count; continue; }
#line 59 "c/eof/03_eof_rule.c"
yy9:
	yych = *++YYCURSOR;
	if (yych <= 0x00) {
		if (YYLIMIT <= YYCURSOR) goto yy11;
		goto yy6;
	}
	goto yy6;
yy10:
#line 18 "c/eof/03_eof_rule.re"
	{ return count; }
#line 70 "c/eof/03_eof_rule.c"
yy11:
	YYCURSOR = YYMARKER;
	goto yy2;
}
#line 21 "c/eof/03_eof_rule.re"

    }
}

#define TEST(s, r) assert(lex(s, sizeof(s) - 1) == r)
int main() {
    TEST("", 0);
    TEST("'qu\0tes' 'are' 'fine: \\'' ", 3);
    TEST("'unterminated\\'", -1);
    return 0;
}
