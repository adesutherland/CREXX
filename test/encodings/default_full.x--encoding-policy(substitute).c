/* Generated by re2c */
#line 1 "encodings/default_full.x--encoding-policy(substitute).re"
// re2c $INPUT -o $OUTPUT -x --encoding-policy substitute

#line 6 "encodings/default_full.x--encoding-policy(substitute).c"
{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if (yych <= 0xD7FF) goto yy2;
	if (yych <= 0xDBFF) goto yy4;
	if (yych <= 0xDFFF) goto yy6;
yy2:
	++YYCURSOR;
#line 4 "encodings/default_full.x--encoding-policy(substitute).re"
	{ return FULL; }
#line 18 "encodings/default_full.x--encoding-policy(substitute).c"
yy4:
	yych = *++YYCURSOR;
	if (yych <= 0xDBFF) goto yy5;
	if (yych <= 0xDFFF) goto yy2;
yy5:
#line 3 "encodings/default_full.x--encoding-policy(substitute).re"
	{ return DEFAULT; }
#line 26 "encodings/default_full.x--encoding-policy(substitute).c"
yy6:
	++YYCURSOR;
	goto yy5;
}
#line 5 "encodings/default_full.x--encoding-policy(substitute).re"

