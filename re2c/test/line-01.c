/* Generated by re2c */
#line 1 "line-01.re"
// re2c $INPUT -o $OUTPUT 
const char* scan(unsigned char* in)
{

#line 8 "line-01.c"
{
	YYCTYPE yych;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case '\n': goto yy1;
		case 'a': goto yy3;
		case 'b': goto yy4;
		case 'c': goto yy5;
		case 'd': goto yy6;
		default: goto yy2;
	}
yy1:
yy2:
	++YYCURSOR;
#line 6 "d"
	{
		return ".";
	}
#line 28 "line-01.c"
yy3:
	++YYCURSOR;
#line 1 "a"
	{
		return "a";
	}
#line 35 "line-01.c"
yy4:
	++YYCURSOR;
#line 2 "b"
	{
		return "b";
	}
#line 42 "line-01.c"
yy5:
	++YYCURSOR;
#line 5 "b"
	{
		return "c";
	}
#line 49 "line-01.c"
yy6:
	++YYCURSOR;
#line 2 "d"
	{
		return "d";
	}
#line 56 "line-01.c"
}
#line 1 "e"

}
line-01.re:4:0: warning: control flow is undefined for strings that match '\xA', use default rule '*' [-Wundefined-control-flow]
