/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 4) YYFILL(4);
	yych = *(YYMARKER = YYCURSOR);
	if (yych >= 0x01) {
		yyt2 = yyt3 = NULL;
		yyt1 = yyt4 = YYCURSOR;
		goto yy3;
	}
yy2:
	yynmatch = 1;
	yypmatch[0] = YYCURSOR;
	yypmatch[1] = YYCURSOR;
	{}
yy3:
	yych = *++YYCURSOR;
	if (yych >= 0x01) {
		yyt7 = yyt8 = NULL;
		yyt5 = yyt6 = yyt9 = YYCURSOR;
		goto yy5;
	}
	YYCURSOR = YYMARKER;
	goto yy2;
yy5:
	yych = *++YYCURSOR;
	if (yych >= 0x01) {
		yyt4 = yyt5 = yyt7 = yyt8 = NULL;
		yyt2 = yyt3 = yyt6 = yyt9 = YYCURSOR;
		goto yy7;
	}
	yyt10 = YYCURSOR;
yy6:
	yynmatch = 7;
	yypmatch[2] = yyt1;
	yypmatch[4] = yyt2;
	yypmatch[5] = yyt3;
	yypmatch[6] = yyt4;
	yypmatch[7] = yyt5;
	yypmatch[8] = yyt6;
	yypmatch[10] = yyt7;
	yypmatch[11] = yyt8;
	yypmatch[12] = yyt9;
	yypmatch[13] = yyt10;
	yypmatch[0] = yyt1;
	yypmatch[1] = YYCURSOR;
	yypmatch[3] = yyt6;
	yypmatch[9] = YYCURSOR;
	{}
yy7:
	yych = *++YYCURSOR;
	if (yych <= 0x00) {
		yyt2 = yyt1;
		yyt10 = YYCURSOR;
		goto yy6;
	}
	++YYCURSOR;
	yyt7 = yyt2;
	yyt2 = yyt1;
	yyt9 = yyt10 = NULL;
	yyt8 = YYCURSOR;
	goto yy6;
}

posix_captures/repetition/44.i--flex-syntax.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
