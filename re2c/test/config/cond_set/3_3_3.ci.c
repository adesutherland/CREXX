re2c: warning: line 14: control flow in condition 'c1' is undefined for strings that match '[\x0-\x60\x62-\xFF]', use default rule '*' [-Wundefined-control-flow]
re2c: warning: line 14: control flow in condition 'c2' is undefined for strings that match '[\x0-\x61\x63-\xFF]', use default rule '*' [-Wundefined-control-flow]
/* Generated by re2c */
/* autogen */
// re2c:define:YYSETCONDITION = "cond = @@;";
// re2c:define:YYSETCONDITION@cond = #;
// re2c:define:YYSETCONDITION:naked = 1;


{
	YYCTYPE yych;
	switch (YYGETCONDITION()) {
	case yycc1: goto yyc_c1;
	case yycc2: goto yyc_c2;
	}
/* *********************************** */
yyc_c1:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':	goto yy4;
	default:	goto yy3;
	}
yy3:
yy4:
	++YYCURSOR;
	cond = @@;
	{ code1 }
/* *********************************** */
yyc_c2:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'b':	goto yy9;
	default:	goto yy8;
	}
yy8:
yy9:
	++YYCURSOR;
	cond = @@;
	{ code2 }
}

