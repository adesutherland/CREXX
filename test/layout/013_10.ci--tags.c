/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -ci --tags

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
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':
		YYMTAGP(yyt2);
		goto yy4;
	default:	goto yy2;
	}
yy2:
	++YYCURSOR;
	;
yy4:
	++YYCURSOR;
	YYMTAGP(yyt1);
	x = yyt2;
	y = yyt1;
	YYSETCONDITION(yycz);
	;
/* *********************************** */
yyc_y:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR++;
	YYSETCONDITION(yycx);
	;
}
