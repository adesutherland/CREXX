/* Generated by re2c */
#line 1 "encodings/range_neg_lf.re"
// re2c $INPUT -o $OUTPUT 

#line 6 "encodings/range_neg_lf.c"
{
	YYCTYPE yych;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case '\n':	goto yy2;
	default:	goto yy3;
	}
yy2:
yy3:
	++YYCURSOR;
#line 3 "encodings/range_neg_lf.re"
	{return 0;}
#line 20 "encodings/range_neg_lf.c"
}
#line 4 "encodings/range_neg_lf.re"

encodings/range_neg_lf.re:4:2: warning: control flow is undefined for strings that match '\xA', use default rule '*' [-Wundefined-control-flow]
