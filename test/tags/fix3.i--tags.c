/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --tags
#include <stddef.h>
#include <stdio.h>

static void lex(const char *YYCURSOR)
{
    const char *YYMARKER, *p0, *p1, *p2, *p3, *p4;
    const char *yyt1;const char *yyt2;const char *yyt3;const char *yyt4;const char *yyt5;
    
{
	char yych;
	yych = *YYCURSOR;
	switch (yych) {
	case '0':
		yyt1 = YYCURSOR;
		goto yy4;
	case '1':
		yyt1 = yyt2 = YYCURSOR;
		goto yy7;
	case '2':
		yyt1 = yyt2 = yyt3 = YYCURSOR;
		goto yy9;
	case '3':
		yyt1 = yyt2 = yyt3 = yyt4 = YYCURSOR;
		goto yy11;
	case '4':
		yyt1 = yyt2 = yyt3 = yyt4 = yyt5 = YYCURSOR;
		goto yy13;
	default:	goto yy2;
	}
yy2:
	++YYCURSOR;
	{ printf("error\n"); return; }
yy4:
	yych = *++YYCURSOR;
	switch (yych) {
	case '0':	goto yy4;
	case '1':
		yyt2 = YYCURSOR;
		goto yy7;
	case '2':
		yyt2 = yyt3 = YYCURSOR;
		goto yy9;
	case '3':
		yyt2 = yyt3 = yyt4 = YYCURSOR;
		goto yy11;
	case '4':
		yyt2 = yyt3 = yyt4 = yyt5 = YYCURSOR;
		goto yy13;
	default:
		yyt2 = yyt3 = yyt4 = yyt5 = YYCURSOR;
		goto yy6;
	}
yy6:
	p0 = yyt1;
	p1 = yyt2;
	p2 = yyt3;
	p3 = yyt4;
	p4 = yyt5;
	{
            printf("'%.*s', '%.*s', '%.*s', '%.*s', '%.*s'\n",
                p1 - p0, p0,
                p2 - p1, p1,
                p3 - p2, p2,
                p4 - p3, p3,
                YYCURSOR - p4, p4);
                return;
        }
yy7:
	yych = *++YYCURSOR;
	switch (yych) {
	case '1':	goto yy7;
	case '2':
		yyt3 = YYCURSOR;
		goto yy9;
	case '3':
		yyt3 = yyt4 = YYCURSOR;
		goto yy11;
	case '4':
		yyt3 = yyt4 = yyt5 = YYCURSOR;
		goto yy13;
	default:
		yyt3 = yyt4 = yyt5 = YYCURSOR;
		goto yy6;
	}
yy9:
	yych = *++YYCURSOR;
	switch (yych) {
	case '2':	goto yy9;
	case '3':
		yyt4 = YYCURSOR;
		goto yy11;
	case '4':
		yyt4 = yyt5 = YYCURSOR;
		goto yy13;
	default:
		yyt4 = yyt5 = YYCURSOR;
		goto yy6;
	}
yy11:
	yych = *++YYCURSOR;
	switch (yych) {
	case '3':	goto yy11;
	case '4':
		yyt5 = YYCURSOR;
		goto yy13;
	default:
		yyt5 = YYCURSOR;
		goto yy6;
	}
yy13:
	yych = *++YYCURSOR;
	switch (yych) {
	case '4':	goto yy13;
	default:	goto yy6;
	}
}

}

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i) {
        lex(argv[i]);
    }
    return 0;
}
tags/fix3.i--tags.re:18:17: warning: rule matches empty string [-Wmatch-empty-string]
