/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax --stadfa

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if (yych >= 0x01) goto yy3;
	yyt1 = yyt2 = yyt3 = yyt4 = yyt6 = yyt7 = NULL;
	yyt5 = YYCURSOR;
yy2:
	yynmatch = 4;
	yypmatch[0] = yyt5;
	yypmatch[2] = yyt7;
	yypmatch[3] = yyt6;
	yypmatch[4] = yyt1;
	yypmatch[5] = yyt2;
	yypmatch[6] = yyt3;
	yypmatch[7] = yyt4;
	yypmatch[1] = YYCURSOR;
	{}
yy3:
	yych = *++YYCURSOR;
	yyt1 = yyt2 = NULL;
	yyt3 = yyt5 = yyt6 = yyt7 = YYCURSOR - 1;
	if (yych <= 0x00) {
		yyt4 = yyt6 = YYCURSOR;
		goto yy2;
	}
yy4:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	yyt1 = yyt3;
	yyt5 = yyt6;
	if (yych <= 0x00) {
		yyt3 = yyt4 = NULL;
		yyt2 = yyt6 = YYCURSOR;
		goto yy2;
	}
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	yyt1 = yyt2 = NULL;
	yyt3 = yyt7 = YYCURSOR - 1;
	if (yych <= 0x00) {
		yyt4 = yyt6 = YYCURSOR;
		goto yy2;
	}
	goto yy4;
}

posix_captures/repetition/42.i--flex-syntax--stadfa.re:6:4: warning: rule matches empty string [-Wmatch-empty-string]
posix_captures/repetition/42.i--flex-syntax--stadfa.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
posix_captures/repetition/42.i--flex-syntax--stadfa.re:7:7: warning: unreachable rule (shadowed by rule at line 6) [-Wunreachable-rules]
