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
			goto yy2;
		case 'b':
			yyt1 = YYCURSOR;
			goto yy4;
		case 'c':
			yyt1 = YYCURSOR;
			goto yy5;
		default: goto yy1;
	}
yy1:
	yynmatch = 1;
	yypmatch[0] = YYCURSOR;
	yypmatch[1] = YYCURSOR;
	{}
yy2:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'a': goto yy6;
		case 'b': goto yy7;
		case 'c': goto yy8;
		default: goto yy3;
	}
yy3:
	YYCURSOR = YYMARKER;
	switch (yyaccept) {
		case 0: goto yy1;
		case 1:
			yyt3 = yyt4 = YYCURSOR;
			goto yy12;
		default:
			yyt2 = yyt4;
			yyt3 = yyt4 = YYCURSOR;
			goto yy12;
	}
yy4:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'c': goto yy9;
		default: goto yy3;
	}
yy5:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'a': goto yy6;
		case 'b': goto yy10;
		case 'c': goto yy8;
		default: goto yy3;
	}
yy6:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'a':
			yyt2 = YYCURSOR;
			goto yy11;
		case 'b':
			yyt2 = YYCURSOR;
			goto yy13;
		case 'c':
			yyt2 = YYCURSOR;
			goto yy14;
		default: goto yy3;
	}
yy7:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'a': goto yy6;
		case 'b': goto yy10;
		case 'c': goto yy16;
		default: goto yy3;
	}
yy8:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'a':
		case 'b':
		case 'c': goto yy15;
		default: goto yy3;
	}
yy9:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'd': goto yy5;
		default: goto yy3;
	}
yy10:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'c': goto yy18;
		default: goto yy3;
	}
yy11:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'a':
			yyt2 = YYCURSOR;
			goto yy11;
		case 'b':
			yyt3 = YYCURSOR;
			goto yy19;
		case 'c':
			yyt2 = YYCURSOR;
			goto yy14;
		case 'd':
			yyt3 = yyt4 = YYCURSOR;
			goto yy20;
		default:
			yyt3 = yyt4 = YYCURSOR;
			goto yy12;
	}
yy12:
	yynmatch = 3;
	yypmatch[0] = yyt1;
	yypmatch[2] = yyt2;
	yypmatch[3] = yyt3;
	yypmatch[4] = yyt4;
	yypmatch[1] = YYCURSOR;
	yypmatch[5] = YYCURSOR;
	{}
yy13:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'a':
			yyt2 = YYCURSOR;
			goto yy11;
		case 'b':
			yyt3 = YYCURSOR;
			goto yy17;
		case 'c':
			yyt4 = YYCURSOR;
			goto yy21;
		default: goto yy3;
	}
yy14:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy15:
	switch (yych) {
		case 'a':
			yyt2 = YYCURSOR;
			goto yy11;
		case 'b':
			yyt3 = YYCURSOR;
			goto yy17;
		case 'c':
			yyt2 = YYCURSOR;
			goto yy14;
		case 'd':
			yyt3 = yyt4 = YYCURSOR;
			goto yy20;
		default:
			yyt3 = yyt4 = YYCURSOR;
			goto yy12;
	}
yy16:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'a':
		case 'b':
		case 'c': goto yy15;
		case 'd': goto yy8;
		default: goto yy3;
	}
yy17:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'c': goto yy22;
		default: goto yy3;
	}
yy18:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'd': goto yy8;
		default: goto yy3;
	}
yy19:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'a':
			yyt2 = YYCURSOR;
			goto yy11;
		case 'b':
			yyt3 = YYCURSOR;
			goto yy17;
		case 'c':
			yyt2 = yyt3;
			yyt4 = YYCURSOR;
			goto yy21;
		case 'd':
			yyt3 = yyt4 = YYCURSOR;
			goto yy20;
		default:
			yyt3 = yyt4 = YYCURSOR;
			goto yy12;
	}
yy20:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'd': goto yy20;
		default: goto yy12;
	}
yy21:
	yyaccept = 2;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'a':
			yyt2 = YYCURSOR;
			goto yy11;
		case 'b':
			yyt3 = YYCURSOR;
			goto yy17;
		case 'c':
			yyt2 = YYCURSOR;
			goto yy14;
		case 'd': goto yy14;
		default:
			yyt2 = yyt4;
			yyt3 = yyt4 = YYCURSOR;
			goto yy12;
	}
yy22:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'd':
			yyt2 = yyt3;
			goto yy14;
		default: goto yy3;
	}
}

captures/posix/repetition/71.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
