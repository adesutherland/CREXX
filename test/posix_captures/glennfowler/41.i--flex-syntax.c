/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax

{
	YYCTYPE yych;
	unsigned int yyaccept = 0;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *(YYMARKER = YYCURSOR);
	switch (yych) {
	case 'a':
		yyt1 = yyt2 = YYCURSOR;
		goto yy3;
	case 'b':
		yyt1 = yyt2 = YYCURSOR;
		goto yy6;
	default:
		yyt2 = yyt3 = NULL;
		yyt1 = YYCURSOR;
		goto yy2;
	}
yy2:
	yynmatch = 2;
	yypmatch[0] = yyt1;
	yypmatch[2] = yyt2;
	yypmatch[3] = yyt3;
	yypmatch[1] = YYCURSOR;
	{}
yy3:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 0x00:	goto yy5;
	case 'z':	goto yy8;
	default:	goto yy3;
	}
yy5:
	YYCURSOR = YYMARKER;
	switch (yyaccept) {
	case 0:
		yyt2 = yyt3 = NULL;
		yyt1 = YYCURSOR;
		goto yy2;
	case 1:
		yyt3 = YYCURSOR;
		goto yy2;
	default:
		yyt2 = yyt3;
		yyt3 = YYCURSOR;
		goto yy2;
	}
yy6:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 0x00:	goto yy5;
	case 'y':	goto yy10;
	default:	goto yy6;
	}
yy8:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 0x00:
		yyt3 = YYCURSOR;
		goto yy2;
	case 'b':
		yyt3 = YYCURSOR;
		goto yy12;
	case 'z':	goto yy8;
	default:	goto yy3;
	}
yy10:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 0x00:
		yyt3 = YYCURSOR;
		goto yy2;
	case 'a':
		yyt3 = YYCURSOR;
		goto yy14;
	case 'y':	goto yy10;
	default:	goto yy6;
	}
yy12:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 0x00:	goto yy5;
	case 'y':	goto yy16;
	case 'z':	goto yy18;
	default:	goto yy12;
	}
yy14:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 0x00:	goto yy5;
	case 'y':	goto yy20;
	case 'z':	goto yy22;
	default:	goto yy14;
	}
yy16:
	yyaccept = 2;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 0x00:
		yyt2 = yyt3;
		yyt3 = YYCURSOR;
		goto yy2;
	case 'y':	goto yy16;
	case 'z':	goto yy18;
	default:	goto yy12;
	}
yy18:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 0x00:
		yyt3 = YYCURSOR;
		goto yy2;
	case 'b':
		yyt3 = YYCURSOR;
		goto yy12;
	case 'y':	goto yy16;
	case 'z':	goto yy18;
	default:	goto yy12;
	}
yy20:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 0x00:
		yyt3 = YYCURSOR;
		goto yy2;
	case 'a':
		yyt3 = YYCURSOR;
		goto yy14;
	case 'y':	goto yy20;
	case 'z':	goto yy22;
	default:	goto yy14;
	}
yy22:
	yyaccept = 2;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 0x00:
		yyt2 = yyt3;
		yyt3 = YYCURSOR;
		goto yy2;
	case 'y':	goto yy20;
	case 'z':	goto yy22;
	default:	goto yy14;
	}
}

posix_captures/glennfowler/41.i--flex-syntax.re:6:4: warning: rule matches empty string [-Wmatch-empty-string]
posix_captures/glennfowler/41.i--flex-syntax.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
posix_captures/glennfowler/41.i--flex-syntax.re:7:7: warning: unreachable rule (shadowed by rule at line 6) [-Wunreachable-rules]
