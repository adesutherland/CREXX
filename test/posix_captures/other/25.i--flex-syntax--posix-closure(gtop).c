/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax --posix-closure gtop

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 3) YYFILL(3);
	yych = *YYCURSOR;
	switch (yych) {
	case 'x':
		yyt1 = yyt2 = yyt4 = YYCURSOR;
		goto yy3;
	case 'y':
		yyt1 = yyt2 = YYCURSOR;
		goto yy4;
	default:
		yyt1 = yyt2 = yyt3 = yyt4 = yyt5 = YYCURSOR;
		goto yy2;
	}
yy2:
	yynmatch = 3;
	yypmatch[0] = yyt1;
	yypmatch[2] = yyt2;
	yypmatch[3] = yyt3;
	yypmatch[4] = yyt4;
	yypmatch[5] = yyt5;
	yypmatch[1] = YYCURSOR;
	{}
yy3:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'x':
		yyt2 = yyt4 = YYCURSOR;
		goto yy5;
	case 'y':
		yyt2 = YYCURSOR;
		goto yy6;
	default:
		yyt3 = yyt5 = YYCURSOR;
		goto yy2;
	}
yy4:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'x':
		yyt2 = yyt4 = YYCURSOR;
		goto yy5;
	case 'y':
		yyt2 = YYCURSOR;
		goto yy6;
	default:
		yyt4 = yyt5 = NULL;
		yyt3 = YYCURSOR;
		goto yy2;
	}
yy5:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'x':
		yyt2 = yyt4 = YYCURSOR;
		goto yy7;
	case 'y':
		yyt2 = YYCURSOR;
		goto yy8;
	default:
		yyt3 = yyt5 = YYCURSOR;
		goto yy2;
	}
yy6:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'x':
		yyt2 = yyt4 = YYCURSOR;
		goto yy7;
	case 'y':
		yyt2 = YYCURSOR;
		goto yy8;
	default:
		yyt4 = yyt5 = NULL;
		yyt3 = YYCURSOR;
		goto yy2;
	}
yy7:
	++YYCURSOR;
	yyt3 = yyt5 = YYCURSOR;
	goto yy2;
yy8:
	++YYCURSOR;
	yyt4 = yyt5 = NULL;
	yyt3 = YYCURSOR;
	goto yy2;
}

posix_captures/other/25.i--flex-syntax--posix-closure(gtop).re:6:4: warning: rule matches empty string [-Wmatch-empty-string]
posix_captures/other/25.i--flex-syntax--posix-closure(gtop).re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
posix_captures/other/25.i--flex-syntax--posix-closure(gtop).re:7:7: warning: unreachable rule (shadowed by rule at line 6) [-Wunreachable-rules]
