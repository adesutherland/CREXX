/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 6) YYFILL(6);
	yych = *(YYMARKER = YYCURSOR);
	switch (yych) {
	case 'a':
		yyt1 = YYCURSOR;
		goto yy3;
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
	case 'c':
		yyt3 = YYCURSOR;
		goto yy6;
	default:	goto yy4;
	}
yy4:
	YYCURSOR = YYMARKER;
	goto yy2;
yy5:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'b':
		yyt3 = YYCURSOR;
		goto yy8;
	case 'c':
		yyt3 = YYCURSOR;
		goto yy6;
	default:	goto yy4;
	}
yy6:
	yych = *++YYCURSOR;
	yyt2 = YYCURSOR;
	goto yy10;
yy7:
	yynmatch = 5;
	yypmatch[2] = yyt1;
	yypmatch[6] = yyt3;
	yypmatch[8] = yyt2;
	yypmatch[0] = yyt1;
	yypmatch[1] = YYCURSOR;
	yypmatch[3] = yyt3;
	yypmatch[4] = yyt3;
	yypmatch[5] = YYCURSOR;
	yypmatch[7] = yyt2;
	yypmatch[9] = YYCURSOR;
	{}
yy8:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'c':	goto yy11;
	default:	goto yy4;
	}
yy9:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy10:
	switch (yych) {
	case 'd':	goto yy9;
	default:	goto yy7;
	}
yy11:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'd':	goto yy6;
	default:	goto yy4;
	}
}

posix_captures/forcedassoc/12.i--flex-syntax.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
