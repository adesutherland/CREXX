/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax --stadfa

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 3) YYFILL(3);
	yych = *YYCURSOR;
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
	default:	goto yy4;
	}
yy4:
	yynmatch = 2;
	yypmatch[0] = yypmatch[2] = yyt1;
	yypmatch[1] = yypmatch[3] = YYCURSOR;
	{}
yy5:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':	goto yy6;
	default:	goto yy4;
	}
yy6:
	++YYCURSOR;
	goto yy4;
}

posix_captures/glennfowler/30.i--flex-syntax--stadfa.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
