/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax --stadfa

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 4) YYFILL(4);
	yych = *(YYMARKER = YYCURSOR);
	if (yych >= 0x01) goto yy3;
yy2:
	yynmatch = 1;
	yypmatch[0] = YYCURSOR;
	yypmatch[1] = YYCURSOR;
	{}
yy3:
	yych = *++YYCURSOR;
	yyt1 = YYCURSOR - 1;
	if (yych >= 0x01) goto yy5;
	YYCURSOR = YYMARKER;
	goto yy2;
yy5:
	yych = *++YYCURSOR;
	yyt4 = yyt5 = NULL;
	yyt2 = yyt6 = YYCURSOR - 1;
	if (yych >= 0x01) goto yy7;
	yyt3 = yyt7 = YYCURSOR;
yy6:
	yynmatch = 4;
	yypmatch[0] = yyt1;
	yypmatch[2] = yyt2;
	yypmatch[3] = yyt3;
	yypmatch[4] = yyt4;
	yypmatch[5] = yyt5;
	yypmatch[6] = yyt6;
	yypmatch[7] = yyt7;
	yypmatch[1] = YYCURSOR;
	{}
yy7:
	yych = *++YYCURSOR;
	yyt3 = yyt1;
	yyt4 = yyt5 = NULL;
	yyt2 = yyt6 = YYCURSOR - 1;
	if (yych <= 0x00) {
		yyt3 = yyt7 = YYCURSOR;
		goto yy6;
	}
	++YYCURSOR;
	yyt4 = yyt2;
	yyt1 = yyt3;
	yyt6 = yyt7 = NULL;
	yyt3 = yyt5 = YYCURSOR;
	goto yy6;
}

posix_captures/repetition/26.i--flex-syntax--stadfa.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
