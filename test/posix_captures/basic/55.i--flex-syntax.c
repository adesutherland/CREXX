/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax

{
	YYCTYPE yych;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':	goto yy3;
	default:	goto yy2;
	}
yy2:
	yynmatch = 1;
	yypmatch[0] = YYCURSOR;
	yypmatch[1] = YYCURSOR;
	{}
yy3:
	++YYCURSOR;
	yynmatch = 10;
	yypmatch[0] = yypmatch[2] = yypmatch[4] = yypmatch[6] = yypmatch[8] = yypmatch[10] = yypmatch[12] = yypmatch[14] = yypmatch[16] = yypmatch[18] = YYCURSOR - 1;
	yypmatch[1] = yypmatch[3] = yypmatch[5] = yypmatch[7] = yypmatch[9] = yypmatch[11] = yypmatch[13] = yypmatch[15] = yypmatch[17] = yypmatch[19] = YYCURSOR;
	{}
}

posix_captures/basic/55.i--flex-syntax.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
