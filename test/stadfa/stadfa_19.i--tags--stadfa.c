/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --tags --stadfa

{
	YYCTYPE yych;
	unsigned int yyaccept = 0;
	if ((YYLIMIT - YYCURSOR) < 4) YYFILL(4);
	yych = *(YYMARKER = YYCURSOR);
	switch (yych) {
	case 'c':	goto yy4;
	default:	goto yy3;
	}
yy2:
	{}
yy3:
	yych = *++YYCURSOR;
	goto yy7;
yy4:
	yyaccept = 1;
	yych = *(YYMARKER = ++YYCURSOR);
	switch (yych) {
	case 'c':	goto yy10;
	default:	goto yy3;
	}
yy5:
	x = YYCURSOR - 1;
	{}
yy6:
	yyaccept = 2;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	yyt1 = YYCURSOR - 1;
yy7:
	switch (yych) {
	case 'c':	goto yy9;
	default:	goto yy6;
	}
yy8:
	y = yyt1;
	{}
yy9:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'c':	goto yy12;
	default:	goto yy6;
	}
yy10:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'c':	goto yy3;
	default:	goto yy11;
	}
yy11:
	YYCURSOR = YYMARKER;
	switch (yyaccept) {
	case 0:
		goto yy2;
	case 1:
		goto yy5;
	case 2:
		goto yy8;
	default:
		yyt1 = NULL;
		goto yy8;
	}
yy12:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'c':	goto yy13;
	default:	goto yy11;
	}
yy13:
	yyaccept = 3;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'c':	goto yy9;
	default:	goto yy6;
	}
}

stadfa/stadfa_19.i--tags--stadfa.re:5:3: warning: rule matches empty string [-Wmatch-empty-string]
