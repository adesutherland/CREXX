/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i

{
	YYCTYPE yych;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':	goto yy3;
	default:	goto yy2;
	}
yy2:
	;
	*:=;
yy3:
	++YYCURSOR;
	goto yy2;
}
layout/006_04.i.re:3:6: warning: rule matches empty string [-Wmatch-empty-string]
