/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax --stadfa

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 4) YYFILL(4);
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
	yyt5 = YYCURSOR - 1;
	switch (yych) {
	case 'a':	goto yy5;
	case 'b':	goto yy7;
	default:	goto yy4;
	}
yy4:
	YYCURSOR = YYMARKER;
	goto yy2;
yy5:
	yych = *++YYCURSOR;
	yyt1 = NULL;
	yyt2 = YYCURSOR - 1;
	switch (yych) {
	case 'b':	goto yy8;
	default:
		yyt4 = NULL;
		yyt3 = yyt2;
		yyt2 = yyt1;
		yyt1 = yyt5;
		goto yy6;
	}
yy6:
	yynmatch = 5;
	yypmatch[2] = yyt1;
	yypmatch[5] = yyt2;
	yypmatch[6] = yyt3;
	yypmatch[9] = yyt4;
	yypmatch[0] = yyt1;
	yypmatch[1] = YYCURSOR;
	yypmatch[3] = yyt3;
	yypmatch[4] = yyt2;
	if (yyt2 != NULL) yypmatch[4] -= 1;
	yypmatch[7] = YYCURSOR;
	yypmatch[8] = yyt4;
	if (yyt4 != NULL) yypmatch[8] -= 1;
	{}
yy7:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':	goto yy9;
	default:	goto yy4;
	}
yy8:
	++YYCURSOR;
	yyt4 = YYCURSOR;
	yyt3 = yyt2;
	yyt2 = yyt1;
	yyt1 = yyt5;
	goto yy6;
yy9:
	yych = *++YYCURSOR;
	yyt1 = yyt2 = YYCURSOR - 1;
	switch (yych) {
	case 'b':	goto yy8;
	default:
		yyt4 = NULL;
		yyt3 = yyt2;
		yyt2 = yyt1;
		yyt1 = yyt5;
		goto yy6;
	}
}

posix_captures/glennfowler/35_stadfa.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]