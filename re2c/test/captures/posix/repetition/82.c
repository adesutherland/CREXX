/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax

{
	YYCTYPE yych;
	unsigned int yyaccept = 0;
	if ((YYLIMIT - YYCURSOR) < 4) YYFILL(4);
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
		case 'a':
			yyt2 = YYCURSOR;
			goto yy6;
		case 'b':
			yyt2 = YYCURSOR;
			goto yy8;
		case 'c':
			yyt2 = YYCURSOR;
			goto yy9;
		default: goto yy3;
	}
yy3:
	YYCURSOR = YYMARKER;
	switch (yyaccept) {
		case 0: goto yy1;
		case 1:
			yyt3 = yyt4 = YYCURSOR;
			goto yy7;
		default:
			yyt2 = yyt4;
			yyt3 = yyt4 = YYCURSOR;
			goto yy7;
	}
yy4:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'c': goto yy11;
		default: goto yy3;
	}
yy5:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'a':
		case 'b':
		case 'c': goto yy10;
		default: goto yy3;
	}
yy6:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'a':
			yyt2 = YYCURSOR;
			goto yy6;
		case 'b':
			yyt3 = YYCURSOR;
			goto yy13;
		case 'c':
			yyt2 = YYCURSOR;
			goto yy9;
		case 'd':
			yyt3 = yyt4 = YYCURSOR;
			goto yy14;
		default:
			yyt3 = yyt4 = YYCURSOR;
			goto yy7;
	}
yy7:
	yynmatch = 3;
	yypmatch[0] = yyt1;
	yypmatch[2] = yyt2;
	yypmatch[3] = yyt3;
	yypmatch[4] = yyt4;
	yypmatch[1] = YYCURSOR;
	yypmatch[5] = YYCURSOR;
	{}
yy8:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'a':
			yyt2 = YYCURSOR;
			goto yy6;
		case 'b':
			yyt3 = YYCURSOR;
			goto yy12;
		case 'c':
			yyt4 = YYCURSOR;
			goto yy15;
		default: goto yy3;
	}
yy9:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy10:
	switch (yych) {
		case 'a':
			yyt2 = YYCURSOR;
			goto yy6;
		case 'b':
			yyt3 = YYCURSOR;
			goto yy12;
		case 'c':
			yyt2 = YYCURSOR;
			goto yy9;
		case 'd':
			yyt3 = yyt4 = YYCURSOR;
			goto yy14;
		default:
			yyt3 = yyt4 = YYCURSOR;
			goto yy7;
	}
yy11:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'd': goto yy5;
		default: goto yy3;
	}
yy12:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'c': goto yy16;
		default: goto yy3;
	}
yy13:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'a':
			yyt2 = YYCURSOR;
			goto yy6;
		case 'b':
			yyt3 = YYCURSOR;
			goto yy12;
		case 'c':
			yyt2 = yyt3;
			yyt4 = YYCURSOR;
			goto yy15;
		case 'd':
			yyt3 = yyt4 = YYCURSOR;
			goto yy14;
		default:
			yyt3 = yyt4 = YYCURSOR;
			goto yy7;
	}
yy14:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'd': goto yy14;
		default: goto yy7;
	}
yy15:
	yyaccept = 2;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'a':
			yyt2 = YYCURSOR;
			goto yy6;
		case 'b':
			yyt3 = YYCURSOR;
			goto yy12;
		case 'c':
			yyt2 = YYCURSOR;
			goto yy9;
		case 'd': goto yy9;
		default:
			yyt2 = yyt4;
			yyt3 = yyt4 = YYCURSOR;
			goto yy7;
	}
yy16:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'd':
			yyt2 = yyt3;
			goto yy9;
		default: goto yy3;
	}
}

captures/posix/repetition/82.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
