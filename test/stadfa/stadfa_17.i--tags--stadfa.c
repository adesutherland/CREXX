/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --tags --stadfa

{
	YYCTYPE yych;
	unsigned int yyaccept = 0;
	if ((YYLIMIT - YYCURSOR) < 3) YYFILL(3);
	yych = *(YYMARKER = YYCURSOR);
	switch (yych) {
	case 'a':	goto yy3;
	default:	goto yy2;
	}
yy2:
	{}
yy3:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'c':	goto yy7;
	default:	goto yy6;
	}
yy4:
	YYCURSOR = YYMARKER;
	if (yyaccept == 0) {
		goto yy2;
	} else {
		goto yy9;
	}
yy5:
	++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	yyt3 = YYCURSOR - 1;
yy6:
	switch (yych) {
	case 'a':	goto yy5;
	case 'c':	goto yy8;
	default:	goto yy4;
	}
yy7:
	yych = *++YYCURSOR;
	yyt3 = YYCURSOR - 1;
	switch (yych) {
	case 'c':	goto yy10;
	default:	goto yy4;
	}
yy8:
	yyaccept = 1;
	yych = *(YYMARKER = ++YYCURSOR);
	yyt2 = yyt3;
	yyt1 = yyt3;
	yyt3 = YYCURSOR - 1;
	switch (yych) {
	case 'a':	goto yy11;
	case 'c':	goto yy12;
	default:	goto yy9;
	}
yy9:
	t = yyt1;
	{}
yy10:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	yyt2 = yyt3;
	yyt1 = yyt3;
	switch (yych) {
	case 'a':
	case 'c':	goto yy11;
	default:	goto yy9;
	}
yy11:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	yyt3 = yyt2;
	switch (yych) {
	case 'c':	goto yy10;
	default:	goto yy4;
	}
yy12:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	yyt1 = yyt3;
	yyt4 = yyt2;
	yyt2 = yyt3;
	yyt3 = yyt4;
	switch (yych) {
	case 'a':	goto yy11;
	case 'c':	goto yy12;
	default:	goto yy9;
	}
}

stadfa/stadfa_17.i--tags--stadfa.re:4:3: warning: rule matches empty string [-Wmatch-empty-string]
