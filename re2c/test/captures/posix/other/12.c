/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax

{
	YYCTYPE yych;
	unsigned int yyaccept = 0;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *(YYMARKER = YYCURSOR);
	switch (yych) {
		case 'X':
			yyt1 = yyt3 = YYCURSOR;
			goto yy2;
		case 'Y':
			yyt1 = yyt3 = YYCURSOR;
			goto yy3;
		case 'a':
			yyt1 = yyt3 = YYCURSOR;
			goto yy4;
		case 'b':
			yyt1 = yyt3 = YYCURSOR;
			goto yy6;
		default:
			yyt2 = yyt3 = NULL;
			yyt1 = YYCURSOR;
			goto yy1;
	}
yy1:
	yynmatch = 2;
	yypmatch[0] = yyt1;
	yypmatch[2] = yyt2;
	yypmatch[3] = yyt3;
	yypmatch[1] = YYCURSOR;
	{}
yy2:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'X':
			yyt3 = YYCURSOR;
			goto yy2;
		case 'Y':
			yyt3 = YYCURSOR;
			goto yy3;
		case 'a':
			yyt4 = YYCURSOR;
			goto yy7;
		case 'b':
			yyt2 = yyt3;
			yyt3 = YYCURSOR;
			goto yy6;
		default:
			yyt2 = yyt3;
			yyt3 = YYCURSOR;
			goto yy1;
	}
yy3:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'X':
			yyt3 = YYCURSOR;
			goto yy2;
		case 'Y':
			yyt3 = YYCURSOR;
			goto yy3;
		case 'a':
			yyt2 = yyt3;
			yyt3 = YYCURSOR;
			goto yy4;
		case 'b':
			yyt2 = yyt3;
			yyt3 = YYCURSOR;
			goto yy6;
		default:
			yyt2 = yyt3;
			yyt3 = YYCURSOR;
			goto yy1;
	}
yy4:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'b': goto yy8;
		default: goto yy5;
	}
yy5:
	YYCURSOR = YYMARKER;
	switch (yyaccept) {
		case 0:
			yyt2 = yyt3 = NULL;
			yyt1 = YYCURSOR;
			goto yy1;
		case 1:
			yyt3 = YYCURSOR;
			goto yy1;
		case 2:
			yyt2 = yyt4;
			yyt3 = YYCURSOR;
			goto yy1;
		default:
			yyt2 = yyt5;
			yyt3 = YYCURSOR;
			goto yy1;
	}
yy6:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'Y': goto yy3;
		case 'a': goto yy9;
		default: goto yy5;
	}
yy7:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'X':
			yyt3 = YYCURSOR;
			goto yy2;
		case 'Y':
			yyt3 = YYCURSOR;
			goto yy3;
		case 'a':
			yyt2 = yyt3;
			yyt3 = YYCURSOR;
			goto yy4;
		case 'b':
			yyt5 = YYCURSOR;
			goto yy10;
		default:
			yyt2 = yyt3;
			yyt3 = YYCURSOR;
			goto yy1;
	}
yy8:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'a': goto yy11;
		default: goto yy5;
	}
yy9:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'b': goto yy12;
		default: goto yy5;
	}
yy10:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'X':
			yyt3 = YYCURSOR;
			goto yy2;
		case 'Y':
			yyt3 = YYCURSOR;
			goto yy3;
		case 'a':
			yyt2 = YYCURSOR;
			goto yy13;
		case 'b':
			yyt2 = yyt3;
			yyt3 = YYCURSOR;
			goto yy6;
		default:
			yyt2 = yyt3;
			yyt3 = YYCURSOR;
			goto yy1;
	}
yy11:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'b': goto yy3;
		default: goto yy5;
	}
yy12:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'a': goto yy3;
		default: goto yy5;
	}
yy13:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'X':
			yyt3 = YYCURSOR;
			goto yy2;
		case 'Y':
			yyt3 = YYCURSOR;
			goto yy3;
		case 'a':
			yyt2 = yyt3;
			yyt3 = YYCURSOR;
			goto yy4;
		case 'b':
			yyt3 = YYCURSOR;
			goto yy14;
		default:
			yyt2 = yyt3;
			yyt3 = YYCURSOR;
			goto yy1;
	}
yy14:
	yyaccept = 2;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'X':
			yyt3 = YYCURSOR;
			goto yy2;
		case 'Y': goto yy3;
		case 'a':
			yyt4 = YYCURSOR;
			goto yy15;
		case 'b':
			yyt3 = YYCURSOR;
			goto yy6;
		default:
			yyt2 = yyt4;
			yyt3 = YYCURSOR;
			goto yy1;
	}
yy15:
	yyaccept = 3;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'X':
			yyt3 = YYCURSOR;
			goto yy2;
		case 'Y':
			yyt3 = YYCURSOR;
			goto yy3;
		case 'a':
			yyt3 = YYCURSOR;
			goto yy4;
		case 'b':
			yyt5 = YYCURSOR;
			goto yy16;
		default:
			yyt2 = yyt5;
			yyt3 = YYCURSOR;
			goto yy1;
	}
yy16:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'X':
			yyt3 = YYCURSOR;
			goto yy2;
		case 'Y':
			yyt3 = YYCURSOR;
			goto yy3;
		case 'a':
			yyt2 = YYCURSOR;
			goto yy13;
		case 'b':
			yyt3 = YYCURSOR;
			goto yy6;
		default:
			yyt3 = YYCURSOR;
			goto yy1;
	}
}

captures/posix/other/12.re:6:4: warning: rule matches empty string [-Wmatch-empty-string]
captures/posix/other/12.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
captures/posix/other/12.re:7:7: warning: unreachable rule (shadowed by rule at line 6) [-Wunreachable-rules]
