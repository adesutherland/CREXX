re2c: warning: line 12: control flow is undefined for strings that match '[\x0-\x60\x62-\xFF]', use default rule '*' [-Wundefined-control-flow]
/* Generated by re2c */
/* autogen */
// re2c:define:YYFILL = "YYFILL";
// re2c:define:YYFILL@len = @@;
// re2c:define:YYFILL:naked = 0;


{
	YYCTYPE yych;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':	goto yy3;
	default:	goto yy2;
	}
yy2:
yy3:
	++YYCURSOR;
	{ code }
}

