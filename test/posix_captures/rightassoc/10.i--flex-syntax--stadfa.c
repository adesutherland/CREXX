/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax --stadfa

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 7) YYFILL(7);
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
	yyt1 = YYCURSOR - 1;
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
	switch (yych) {
	case 'b':	goto yy8;
	case 'c':	goto yy6;
	default:	goto yy4;
	}
yy6:
	yych = *++YYCURSOR;
	yyt2 = YYCURSOR - 1;
	if (yych >= 0x01) goto yy9;
	yyt3 = YYCURSOR;
yy7:
	yynmatch = 4;
	yypmatch[2] = yyt1;
	yypmatch[4] = yyt2;
	yypmatch[6] = yyt3;
	yypmatch[0] = yyt1;
	yypmatch[1] = YYCURSOR;
	yypmatch[3] = yyt2;
	yypmatch[5] = yyt3;
	yypmatch[7] = YYCURSOR;
	{}
yy8:
	yych = *++YYCURSOR;
	yyt2 = YYCURSOR - 1;
	switch (yych) {
	case 'c':	goto yy10;
	default:	goto yy4;
	}
yy9:
	yych = *++YYCURSOR;
	yyt3 = YYCURSOR - 1;
	if (yych <= 0x00) goto yy7;
	goto yy11;
yy10:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'd':	goto yy13;
	default:	goto yy4;
	}
yy11:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= 0x00) goto yy7;
	goto yy11;
yy13:
	yych = *++YYCURSOR;
	if (yych <= 0x00) {
		yyt3 = YYCURSOR;
		goto yy7;
	}
	goto yy9;
}

posix_captures/rightassoc/10.i--flex-syntax--stadfa.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
