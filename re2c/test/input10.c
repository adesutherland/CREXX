/* Generated by re2c */
#line 1 "input10.re"
// re2c $INPUT -o $OUTPUT 

#line 6 "input10.c"
{
	YYCTYPE yych;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'F':
		case 'G':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g': goto yy2;
		default: goto yy1;
	}
yy1:
	++YYCURSOR;
#line 11 "input10.re"
	{ return -1; }
#line 31 "input10.c"
yy2:
	++YYCURSOR;
#line 9 "input10.re"
	{ return 1; }
#line 36 "input10.c"
}
#line 13 "input10.re"

