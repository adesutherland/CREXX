/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	yyt1 = YYCURSOR;
	if (yych >= 0x01) goto yy3;
yy2:
	yynmatch = 2;
	yypmatch[2] = yyt1;
	yypmatch[0] = yyt1;
	yypmatch[1] = YYCURSOR;
	yypmatch[3] = YYCURSOR;
	{}
yy3:
	yych = *++YYCURSOR;
	if (yych <= 0x00) goto yy2;
	++YYCURSOR;
	goto yy2;
}

posix_captures/glennfowler/11.i--flex-syntax.re:6:4: warning: rule matches empty string [-Wmatch-empty-string]
posix_captures/glennfowler/11.i--flex-syntax.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
posix_captures/glennfowler/11.i--flex-syntax.re:7:7: warning: unreachable rule (shadowed by rule at line 6) [-Wunreachable-rules]
