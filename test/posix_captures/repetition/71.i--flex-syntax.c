/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax

{
	YYCTYPE yych;
	unsigned int yyaccept = 0;
	if ((YYLIMIT - YYCURSOR) < 7) YYFILL(7);
	yych = *(YYMARKER = YYCURSOR);
	switch (yych) {
	case 'a':
		yyt1 = YYCURSOR;
		goto yy3;
	case 'b':
		yyt1 = YYCURSOR;
		goto yy5;
	case 'c':
		yyt1 = YYCURSOR;
		goto yy6;
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
	case 'a':	goto yy7;
	case 'b':	goto yy8;
	case 'c':	goto yy9;
	default:	goto yy4;
	}
yy4:
	YYCURSOR = YYMARKER;
	switch (yyaccept) {
	case 0:
		goto yy2;
	case 1:
		yyt3 = yyt4 = YYCURSOR;
		goto yy14;
	default:
		yyt2 = yyt4;
		yyt3 = yyt4 = YYCURSOR;
		goto yy14;
	}
yy5:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'c':	goto yy10;
	default:	goto yy4;
	}
yy6:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':	goto yy7;
	case 'b':	goto yy11;
	case 'c':	goto yy9;
	default:	goto yy4;
	}
yy7:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':
		yyt2 = YYCURSOR;
		goto yy12;
	case 'b':
		yyt2 = YYCURSOR;
		goto yy15;
	case 'c':
		yyt2 = YYCURSOR;
		goto yy16;
	default:	goto yy4;
	}
yy8:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':	goto yy7;
	case 'b':	goto yy11;
	case 'c':	goto yy18;
	default:	goto yy4;
	}
yy9:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':
	case 'b':
	case 'c':	goto yy17;
	default:	goto yy4;
	}
yy10:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'd':	goto yy6;
	default:	goto yy4;
	}
yy11:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'c':	goto yy20;
	default:	goto yy4;
	}
yy12:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':
		yyt2 = YYCURSOR;
		goto yy12;
	case 'b':
		yyt3 = YYCURSOR;
		goto yy21;
	case 'c':
		yyt2 = YYCURSOR;
		goto yy16;
	case 'd':
		yyt3 = yyt4 = YYCURSOR;
		goto yy22;
	default:
		yyt3 = yyt4 = YYCURSOR;
		goto yy14;
	}
yy14:
	yynmatch = 3;
	yypmatch[0] = yyt1;
	yypmatch[2] = yyt2;
	yypmatch[3] = yyt3;
	yypmatch[4] = yyt4;
	yypmatch[1] = YYCURSOR;
	yypmatch[5] = YYCURSOR;
	{}
yy15:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':
		yyt2 = YYCURSOR;
		goto yy12;
	case 'b':
		yyt3 = YYCURSOR;
		goto yy19;
	case 'c':
		yyt4 = YYCURSOR;
		goto yy24;
	default:	goto yy4;
	}
yy16:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy17:
	switch (yych) {
	case 'a':
		yyt2 = YYCURSOR;
		goto yy12;
	case 'b':
		yyt3 = YYCURSOR;
		goto yy19;
	case 'c':
		yyt2 = YYCURSOR;
		goto yy16;
	case 'd':
		yyt3 = yyt4 = YYCURSOR;
		goto yy22;
	default:
		yyt3 = yyt4 = YYCURSOR;
		goto yy14;
	}
yy18:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':
	case 'b':
	case 'c':	goto yy17;
	case 'd':	goto yy9;
	default:	goto yy4;
	}
yy19:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'c':	goto yy25;
	default:	goto yy4;
	}
yy20:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'd':	goto yy9;
	default:	goto yy4;
	}
yy21:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':
		yyt2 = YYCURSOR;
		goto yy12;
	case 'b':
		yyt3 = YYCURSOR;
		goto yy19;
	case 'c':
		yyt2 = yyt3;
		yyt4 = YYCURSOR;
		goto yy24;
	case 'd':
		yyt3 = yyt4 = YYCURSOR;
		goto yy22;
	default:
		yyt3 = yyt4 = YYCURSOR;
		goto yy14;
	}
yy22:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'd':	goto yy22;
	default:	goto yy14;
	}
yy24:
	yyaccept = 2;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':
		yyt2 = YYCURSOR;
		goto yy12;
	case 'b':
		yyt3 = YYCURSOR;
		goto yy19;
	case 'c':
		yyt2 = YYCURSOR;
		goto yy16;
	case 'd':	goto yy16;
	default:
		yyt2 = yyt4;
		yyt3 = yyt4 = YYCURSOR;
		goto yy14;
	}
yy25:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'd':
		yyt2 = yyt3;
		goto yy16;
	default:	goto yy4;
	}
}

posix_captures/repetition/71.i--flex-syntax.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
