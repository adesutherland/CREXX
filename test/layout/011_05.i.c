/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':	goto yy4;
	default:	goto yy2;
	}
yy2:
	++YYCURSOR;
	;
yy4:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':	goto yy6;
	default:	goto yy5;
	}
yy5:
	;
yy6:
	++YYCURSOR;
	goto yy5;
}
