/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax --stadfa

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
	case 'c':	goto yy6;
	default:	goto yy4;
	}
yy4:
	YYCURSOR = YYMARKER;
	goto yy2;
yy5:
	yych = *++YYCURSOR;
	yyt2 = YYCURSOR - 1;
	switch (yych) {
	case 'b':	goto yy7;
	case 'c':	goto yy9;
	default:	goto yy4;
	}
yy6:
	yych = *++YYCURSOR;
	yyt1 = yyt2 = YYCURSOR - 1;
	switch (yych) {
	case 'b':	goto yy7;
	case 'c':	goto yy9;
	case 'd':	goto yy11;
	default:	goto yy4;
	}
yy7:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'b':	goto yy7;
	case 'c':	goto yy9;
	default:	goto yy4;
	}
yy9:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	yyt1 = YYCURSOR - 1;
	switch (yych) {
	case 'b':	goto yy7;
	case 'c':	goto yy9;
	case 'd':	goto yy11;
	default:	goto yy4;
	}
yy11:
	++YYCURSOR;
	yynmatch = 3;
	yypmatch[2] = yyt2;
	yypmatch[4] = yyt1;
	yypmatch[0] = yyt2 - 1;
	yypmatch[1] = YYCURSOR;
	yypmatch[3] = yyt1;
	yypmatch[5] = YYCURSOR;
	{}
}

posix_captures/basic/51_stadfa.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
