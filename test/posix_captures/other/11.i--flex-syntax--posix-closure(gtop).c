/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax --posix-closure gtop

{
	YYCTYPE yych;
	unsigned int yyaccept = 0;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *(YYMARKER = YYCURSOR);
	switch (yych) {
	case 'a':
		yyt3 = yyt4 = NULL;
		yyt1 = yyt2 = yyt5 = YYCURSOR;
		goto yy3;
	default:
		yyt2 = yyt3 = yyt4 = yyt5 = NULL;
		yyt1 = YYCURSOR;
		goto yy2;
	}
yy2:
	yynmatch = 3;
	yypmatch[0] = yyt1;
	yypmatch[2] = yyt2;
	yypmatch[3] = yyt3;
	yypmatch[4] = yyt5;
	yypmatch[5] = yyt4;
	yypmatch[1] = YYCURSOR;
	{}
yy3:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':	goto yy5;
	default:	goto yy4;
	}
yy4:
	YYCURSOR = YYMARKER;
	switch (yyaccept) {
	case 0:
		yyt2 = yyt3 = yyt4 = yyt5 = NULL;
		yyt1 = YYCURSOR;
		goto yy2;
	case 1:
		yyt4 = yyt5 = NULL;
		yyt3 = YYCURSOR;
		goto yy2;
	default:
		yyt2 = yyt6;
		yyt4 = yyt5 = NULL;
		yyt3 = YYCURSOR;
		goto yy2;
	}
yy5:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':
		yyt2 = YYCURSOR;
		goto yy6;
	default:
		yyt4 = yyt5 = NULL;
		yyt3 = YYCURSOR;
		goto yy2;
	}
yy6:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':	goto yy7;
	default:
		yyt2 = yyt4;
		yyt4 = YYCURSOR;
		goto yy2;
	}
yy7:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':
		yyt6 = YYCURSOR;
		goto yy8;
	default:
		yyt4 = yyt5 = NULL;
		yyt3 = YYCURSOR;
		goto yy2;
	}
yy8:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':	goto yy9;
	default:	goto yy4;
	}
yy9:
	yyaccept = 2;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':
		yyt2 = yyt5 = YYCURSOR;
		goto yy3;
	default:
		yyt2 = yyt6;
		yyt4 = yyt5 = NULL;
		yyt3 = YYCURSOR;
		goto yy2;
	}
}

posix_captures/other/11.i--flex-syntax--posix-closure(gtop).re:6:4: warning: rule matches empty string [-Wmatch-empty-string]
posix_captures/other/11.i--flex-syntax--posix-closure(gtop).re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
posix_captures/other/11.i--flex-syntax--posix-closure(gtop).re:7:7: warning: unreachable rule (shadowed by rule at line 6) [-Wunreachable-rules]
