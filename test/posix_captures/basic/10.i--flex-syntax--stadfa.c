/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax --stadfa

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 3) YYFILL(3);
	yych = *(YYMARKER = YYCURSOR);
	switch (yych) {
	case 'a':	goto yy3;
	case 'c':	goto yy5;
	default:	goto yy2;
	}
yy2:
	yynmatch = 1;
	yypmatch[0] = YYCURSOR;
	yypmatch[1] = YYCURSOR;
	{}
yy3:
	yych = *++YYCURSOR;
	yyt2 = yyt3 = yyt4 = yyt5 = NULL;
	yyt1 = YYCURSOR - 1;
	switch (yych) {
	case 'b':	goto yy6;
	case 'e':	goto yy8;
	default:	goto yy4;
	}
yy4:
	YYCURSOR = YYMARKER;
	goto yy2;
yy5:
	yych = *++YYCURSOR;
	yyt2 = yyt3 = NULL;
	yyt1 = YYCURSOR - 1;
	switch (yych) {
	case 'd':	goto yy9;
	default:	goto yy4;
	}
yy6:
	++YYCURSOR;
	yyt2 = YYCURSOR - 1;
	yyt4 = yyt5 = yyt6 = yyt7 = NULL;
	yyt3 = YYCURSOR;
yy7:
	yynmatch = 4;
	yypmatch[0] = yyt1;
	yypmatch[2] = yyt2;
	yypmatch[3] = yyt3;
	yypmatch[4] = yyt4;
	yypmatch[5] = yyt5;
	yypmatch[6] = yyt6;
	yypmatch[7] = yyt7;
	yypmatch[1] = YYCURSOR;
	{}
yy8:
	yych = *++YYCURSOR;
	yyt6 = YYCURSOR - 1;
	switch (yych) {
	case 'f':	goto yy10;
	default:	goto yy4;
	}
yy9:
	++YYCURSOR;
	yyt4 = YYCURSOR - 1;
	yyt6 = yyt7 = NULL;
	yyt5 = YYCURSOR;
	goto yy7;
yy10:
	++YYCURSOR;
	yyt7 = YYCURSOR - 1;
	goto yy7;
}

posix_captures/basic/10.i--flex-syntax--stadfa.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
