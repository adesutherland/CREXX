/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax --stadfa

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 4) YYFILL(4);
	yych = *(YYMARKER = YYCURSOR);
	if (yych >= 0x01) goto yy3;
	yyt2 = yyt3 = yyt4 = yyt12 = NULL;
	yyt1 = YYCURSOR;
yy2:
	yynmatch = 3;
	yypmatch[0] = yyt1;
	yypmatch[2] = yyt2;
	yypmatch[3] = yyt3;
	yypmatch[4] = yyt4;
	yypmatch[5] = yyt12;
	yypmatch[1] = YYCURSOR;
	{}
yy3:
	yych = *++YYCURSOR;
	yyt6 = yyt10 = NULL;
	yyt5 = yyt7 = yyt12 = YYCURSOR - 1;
	if (yych >= 0x01) goto yy5;
	YYCURSOR = YYMARKER;
	yyt2 = yyt3 = yyt4 = yyt12 = NULL;
	yyt1 = YYCURSOR;
	goto yy2;
yy5:
	yych = *++YYCURSOR;
	yyt11 = yyt7;
	yyt9 = yyt6;
	yyt2 = yyt7;
	yyt8 = yyt5;
	yyt1 = yyt12;
	if (yych <= 0x00) {
		yyt4 = yyt12 = NULL;
		yyt3 = YYCURSOR;
		goto yy2;
	}
	yych = *++YYCURSOR;
	yyt4 = yyt7;
	yyt3 = yyt10;
	yyt6 = yyt11;
	yyt2 = yyt9;
	yyt5 = yyt12;
	yyt1 = yyt8;
	yyt7 = yyt10 = YYCURSOR - 1;
	if (yych <= 0x00) {
		yyt12 = YYCURSOR;
		goto yy2;
	}
yy7:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	yyt9 = yyt6;
	yyt2 = yyt7;
	yyt8 = yyt5;
	yyt1 = yyt12;
	yyt11 = yyt7;
	if (yych <= 0x00) {
		yyt4 = yyt12 = NULL;
		yyt3 = YYCURSOR;
		goto yy2;
	}
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	yyt4 = yyt7;
	yyt3 = yyt10;
	yyt2 = yyt9;
	yyt1 = yyt8;
	yyt7 = yyt10 = YYCURSOR - 1;
	yyt6 = yyt11;
	yyt5 = yyt12;
	if (yych <= 0x00) {
		yyt12 = YYCURSOR;
		goto yy2;
	}
	goto yy7;
}

posix_captures/basic/01.i--flex-syntax--stadfa.re:6:4: warning: rule matches empty string [-Wmatch-empty-string]
posix_captures/basic/01.i--flex-syntax--stadfa.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
posix_captures/basic/01.i--flex-syntax--stadfa.re:7:7: warning: unreachable rule (shadowed by rule at line 6) [-Wunreachable-rules]
