/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -ci

{
	YYCTYPE yych;
	switch (YYGETCONDITION()) {
	case yycx:
		goto yyc_x;
	case yycy:
		goto yyc_y;
	}
/* *********************************** */
yyc_x:
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
	YYSETCONDITION(yycz);
	;
yy6:
	++YYCURSOR;
	goto yy5;
/* *********************************** */
yyc_y:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR++;
	YYSETCONDITION(yycx);
	;
}
