/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax --stadfa

{
	YYCTYPE yych;
	unsigned int yyaccept = 0;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *(YYMARKER = YYCURSOR);
	switch (yych) {
	case 'a':	goto yy3;
	default:
		yyt2 = yyt3 = yyt5 = yyt6 = NULL;
		yyt1 = YYCURSOR;
		goto yy2;
	}
yy2:
	yynmatch = 3;
	yypmatch[0] = yyt1;
	yypmatch[2] = yyt2;
	yypmatch[3] = yyt5;
	yypmatch[4] = yyt6;
	yypmatch[5] = yyt3;
	yypmatch[1] = YYCURSOR;
	{}
yy3:
	yych = *++YYCURSOR;
	yyt5 = NULL;
	yyt3 = yyt4 = yyt6 = YYCURSOR - 1;
	switch (yych) {
	case 'a':	goto yy5;
	default:	goto yy4;
	}
yy4:
	YYCURSOR = YYMARKER;
	if (yyaccept == 0) {
		yyt2 = yyt3 = yyt5 = yyt6 = NULL;
		yyt1 = YYCURSOR;
		goto yy2;
	} else {
		yyt3 = yyt6 = NULL;
		yyt5 = YYCURSOR;
		goto yy2;
	}
yy5:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	yyt2 = yyt4;
	yyt1 = yyt3;
	switch (yych) {
	case 'a':	goto yy6;
	default:
		yyt3 = yyt6 = NULL;
		yyt5 = YYCURSOR;
		goto yy2;
	}
yy6:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	yyt2 = yyt5;
	yyt1 = yyt3;
	yyt4 = YYCURSOR - 1;
	switch (yych) {
	case 'a':	goto yy7;
	default:
		yyt3 = YYCURSOR;
		goto yy2;
	}
yy7:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	yyt2 = yyt4;
	yyt1 = yyt3;
	switch (yych) {
	case 'a':	goto yy8;
	default:
		yyt3 = yyt6 = NULL;
		yyt5 = YYCURSOR;
		goto yy2;
	}
yy8:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	yyt4 = YYCURSOR - 1;
	switch (yych) {
	case 'a':	goto yy9;
	default:	goto yy4;
	}
yy9:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	yyt2 = yyt4;
	yyt1 = yyt3;
	switch (yych) {
	case 'a':	goto yy10;
	default:
		yyt3 = yyt6 = NULL;
		yyt5 = YYCURSOR;
		goto yy2;
	}
yy10:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	yyt4 = yyt6 = YYCURSOR - 1;
	switch (yych) {
	case 'a':	goto yy5;
	default:	goto yy4;
	}
}

posix_captures/other/11.i--flex-syntax--stadfa.re:6:4: warning: rule matches empty string [-Wmatch-empty-string]
posix_captures/other/11.i--flex-syntax--stadfa.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
posix_captures/other/11.i--flex-syntax--stadfa.re:7:7: warning: unreachable rule (shadowed by rule at line 6) [-Wunreachable-rules]
