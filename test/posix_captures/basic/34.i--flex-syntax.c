/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 3) YYFILL(3);
	yych = *(YYMARKER = YYCURSOR);
	switch (yych) {
	case 'a':	goto yy3;
	default:	goto yy2;
	}
yy2:
	yynmatch = 1;
	yypmatch[0] = YYCURSOR;
	yypmatch[1] = YYCURSOR;
	{}
yy3:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'b':	goto yy5;
	default:	goto yy4;
	}
yy4:
	YYCURSOR = YYMARKER;
	goto yy2;
yy5:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'c':	goto yy6;
	default:	goto yy4;
	}
yy6:
	++YYCURSOR;
	yynmatch = 3;
	yypmatch[0] = YYCURSOR - 3;
	yypmatch[1] = YYCURSOR;
	yypmatch[2] = YYCURSOR - 3;
	yypmatch[3] = YYCURSOR - 2;
	yypmatch[4] = YYCURSOR - 1;
	yypmatch[5] = YYCURSOR;
	{}
}

posix_captures/basic/34.i--flex-syntax.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
