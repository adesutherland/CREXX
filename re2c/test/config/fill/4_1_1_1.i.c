re2c: warning: line 8: control flow is undefined for strings that match '[\x0-\x60\x62-\xFF]', use default rule '*' [-Wundefined-control-flow]
/* Generated by re2c */
/* autogen */
// re2c:define:YYFILL = "fill (@@);";


{
	YYCTYPE yych;
	if (YYLIMIT <= YYCURSOR) fill (1);(1);
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

