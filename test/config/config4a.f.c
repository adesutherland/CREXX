/* Generated by re2c */
#line 1 "config/config4a.f.re"
// re2c $INPUT -o $OUTPUT -f
#define	NULL		((char*) 0)
#define	YYCTYPE		char
#define	YYCURSOR	p
#define	YYLIMIT		p
#define	YYMARKER	q
#define	YYFILL(n)

char *scan(char *p)
{
	char *q;

#line 16 "config/config4a.f.c"

	switch (YYGETSTATE()) {
	default:
		goto yy0;
	case 0:
		goto yyFillLabel0;
	case 1:
		goto yyFillLabel1;
	}
yy0:
	YYSETSTATE(0);
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
yyFillLabel0:
	yych = *YYCURSOR;
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy5;
	default:	goto yy3;
	}
yy3:
	++YYCURSOR;
#line 15 "config/config4a.f.re"
	{ return NULL; }
#line 48 "config/config4a.f.c"
yy5:
	++YYCURSOR;
	YYSETSTATE(1);
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
yyFillLabel1:
	yych = *YYCURSOR;
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy5;
	default:	goto yy7;
	}
yy7:
#line 14 "config/config4a.f.re"
	{ return YYCURSOR; }
#line 71 "config/config4a.f.c"
#line 16 "config/config4a.f.re"

}
