/* Generated by re2c */
#line 1 "input1.re"
// re2c $INPUT -o $OUTPUT 

#line 6 "input1.c"
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
yy3:
#line 5 "input1.re"
	{ return 0; }
#line 20 "input1.c"
yy4:
	yych = *(YYMARKER = ++YYCURSOR);
	switch (yych) {
	case 'b':	goto yy5;
	default:	goto yy3;
	}
yy5:
	++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	switch (yych) {
	case 'b':	goto yy5;
	case 'c':	goto yy8;
	default:	goto yy7;
	}
yy7:
	YYCURSOR = YYMARKER;
	goto yy3;
yy8:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'c':	goto yy9;
	default:	goto yy7;
	}
yy9:
	++YYCURSOR;
#line 4 "input1.re"
	{ return 1; }
#line 49 "input1.c"
}
#line 7 "input1.re"

