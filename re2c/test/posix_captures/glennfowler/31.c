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
	case 'a':
		yyt2 = YYCURSOR;
		goto yy5;
	case 'b':	goto yy7;
	default:	goto yy4;
	}
yy4:
	YYCURSOR = YYMARKER;
	goto yy2;
yy5:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'b':	goto yy8;
	default:	goto yy6;
	}
yy6:
	yynmatch = 3;
	yypmatch[2] = yyt1;
	yypmatch[4] = yyt2;
	yypmatch[0] = yyt1;
	yypmatch[1] = YYCURSOR;
	yypmatch[3] = yyt2;
	yypmatch[5] = YYCURSOR;
	{}
yy7:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':
		yyt2 = YYCURSOR;
		goto yy9;
	default:	goto yy4;
	}
yy8:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':	goto yy10;
	default:	goto yy6;
	}
yy9:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':
		yyt2 = YYCURSOR;
		goto yy5;
	case 'b':	goto yy8;
	default:	goto yy6;
	}
yy10:
	++YYCURSOR;
	goto yy6;
}

posix_captures/glennfowler/31.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
