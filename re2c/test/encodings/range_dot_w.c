/* Generated by re2c */
#line 1 "encodings/range_dot_w.re"
// re2c $INPUT -o $OUTPUT -w

#line 6 "encodings/range_dot_w.c"
{
	YYCTYPE yych;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych != '\n') goto yy3;
yy3:
	++YYCURSOR;
#line 3 "encodings/range_dot_w.re"
	{return 0;}
#line 16 "encodings/range_dot_w.c"
}
#line 4 "encodings/range_dot_w.re"

encodings/range_dot_w.re:4:2: warning: control flow is undefined for strings that match '\xA', use default rule '*' [-Wundefined-control-flow]
