/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --flex-syntax --stadfa

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 9) YYFILL(9);
	yych = *(YYMARKER = YYCURSOR);
	switch (yych) {
	case 'a':	goto yy3;
	case 'b':	goto yy5;
	default:	goto yy2;
	}
yy2:
	yynmatch = 1;
	yypmatch[0] = YYCURSOR;
	yypmatch[1] = YYCURSOR;
	{}
yy3:
	yych = *++YYCURSOR;
	yyt1 = yyt2 = YYCURSOR - 1;
	switch (yych) {
	case 'a':	goto yy6;
	case 'b':	goto yy8;
	default:	goto yy4;
	}
yy4:
	YYCURSOR = YYMARKER;
	goto yy2;
yy5:
	yych = *++YYCURSOR;
	yyt1 = yyt3 = YYCURSOR - 1;
	switch (yych) {
	case 'a':	goto yy9;
	case 'b':	goto yy10;
	default:	goto yy4;
	}
yy6:
	++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 7) YYFILL(7);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':	goto yy6;
	case 'b':	goto yy11;
	default:	goto yy4;
	}
yy8:
	yych = *++YYCURSOR;
	yyt3 = yyt1;
	switch (yych) {
	case 'a':	goto yy12;
	case 'b':	goto yy10;
	default:	goto yy4;
	}
yy9:
	yych = *++YYCURSOR;
	yyt6 = yyt3;
	yyt2 = yyt7 = YYCURSOR - 1;
	switch (yych) {
	case 'a':	goto yy13;
	case 'b':	goto yy15;
	default:	goto yy4;
	}
yy10:
	yych = *++YYCURSOR;
	yyt5 = yyt1;
	yyt2 = yyt4 = YYCURSOR - 1;
	switch (yych) {
	case 'a':	goto yy16;
	case 'b':	goto yy17;
	default:	goto yy4;
	}
yy11:
	yych = *++YYCURSOR;
	yyt3 = yyt1;
	switch (yych) {
	case 'a':	goto yy9;
	case 'b':	goto yy10;
	default:	goto yy4;
	}
yy12:
	yych = *++YYCURSOR;
	yyt1 = yyt2;
	yyt6 = yyt3;
	yyt7 = YYCURSOR - 1;
	yyt3 = yyt2;
	switch (yych) {
	case 'a':	goto yy9;
	case 'b':	goto yy19;
	default:	goto yy4;
	}
yy13:
	++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 4) YYFILL(4);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':	goto yy13;
	case 'b':	goto yy20;
	default:	goto yy4;
	}
yy15:
	yych = *++YYCURSOR;
	yyt4 = yyt2;
	yyt5 = yyt1;
	switch (yych) {
	case 'a':	goto yy21;
	case 'b':	goto yy17;
	default:	goto yy4;
	}
yy16:
	yych = *++YYCURSOR;
	yyt9 = yyt4;
	yyt8 = yyt5;
	yyt3 = yyt10 = YYCURSOR - 1;
	switch (yych) {
	case 'a':	goto yy22;
	case 'b':	goto yy24;
	default:	goto yy4;
	}
yy17:
	++YYCURSOR;
	yyt3 = YYCURSOR - 1;
yy18:
	yynmatch = 4;
	yypmatch[2] = yyt1;
	yypmatch[4] = yyt2;
	yypmatch[6] = yyt3;
	yypmatch[0] = yyt1;
	yypmatch[1] = YYCURSOR;
	yypmatch[3] = yyt2;
	yypmatch[5] = yyt3;
	yypmatch[7] = YYCURSOR;
	{}
yy19:
	yych = *++YYCURSOR;
	yyt5 = yyt1;
	yyt2 = yyt4 = YYCURSOR - 1;
	switch (yych) {
	case 'a':	goto yy25;
	case 'b':	goto yy17;
	default:	goto yy4;
	}
yy20:
	yych = *++YYCURSOR;
	yyt4 = yyt2;
	yyt5 = yyt1;
	switch (yych) {
	case 'a':	goto yy16;
	case 'b':	goto yy17;
	default:	goto yy4;
	}
yy21:
	yych = *++YYCURSOR;
	yyt2 = yyt7;
	yyt9 = yyt4;
	yyt1 = yyt6;
	yyt8 = yyt5;
	yyt10 = YYCURSOR - 1;
	yyt4 = yyt7;
	yyt5 = yyt6;
	switch (yych) {
	case 'a':	goto yy16;
	case 'b':	goto yy26;
	default:	goto yy4;
	}
yy22:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':	goto yy22;
	case 'b':	goto yy27;
	default:	goto yy4;
	}
yy24:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':	goto yy28;
	default:	goto yy18;
	}
yy25:
	yych = *++YYCURSOR;
	yyt9 = yyt4;
	yyt8 = yyt5;
	yyt3 = yyt10 = YYCURSOR - 1;
	yyt4 = yyt7;
	yyt5 = yyt6;
	switch (yych) {
	case 'a':	goto yy29;
	case 'b':	goto yy24;
	default:	goto yy4;
	}
yy26:
	yych = *++YYCURSOR;
	yyt3 = YYCURSOR - 1;
	switch (yych) {
	case 'a':	goto yy28;
	default:	goto yy18;
	}
yy27:
	++YYCURSOR;
	goto yy18;
yy28:
	++YYCURSOR;
	yyt3 = yyt10;
	yyt2 = yyt9;
	yyt1 = yyt8;
	goto yy18;
yy29:
	yych = *++YYCURSOR;
	yyt9 = yyt4;
	yyt8 = yyt5;
	yyt10 = YYCURSOR - 1;
	switch (yych) {
	case 'a':	goto yy22;
	case 'b':	goto yy24;
	default:	goto yy4;
	}
}

posix_captures/glennfowler/28.i--flex-syntax--stadfa.re:7:7: warning: rule matches empty string [-Wmatch-empty-string]
