/* Generated by re2c */
#line 1 "error13_1.re"
// re2c $INPUT -o $OUTPUT -1
#line 5 "error13_1.c"
#define YYMAXFILL 3
#line 2 "error13_1.re"



#line 11 "error13_1.c"
{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 3) YYFILL(3);
	yych = *YYCURSOR;
	switch (yych) {
		case 'A': goto yy3;
		default: goto yy1;
	}
yy1:
	++YYCURSOR;
yy2:
#line 7 "error13_1.re"
	{ return 0; }
#line 25 "error13_1.c"
yy3:
	yych = *(YYMARKER = ++YYCURSOR);
	switch (yych) {
		case 'B': goto yy4;
		default: goto yy2;
	}
yy4:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'C': goto yy6;
		case 'D': goto yy7;
		default: goto yy5;
	}
yy5:
	YYCURSOR = YYMARKER;
	goto yy2;
yy6:
	++YYCURSOR;
#line 5 "error13_1.re"
	{ return 1; }
#line 46 "error13_1.c"
yy7:
	++YYCURSOR;
#line 6 "error13_1.re"
	{ return 2; }
#line 51 "error13_1.c"
}
#line 8 "error13_1.re"

