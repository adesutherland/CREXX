/* Generated by re2c */
#line 1 "encodings/default_full.w--encoding-policy(fail).re"
// re2c $INPUT -o $OUTPUT -w --encoding-policy fail

#line 6 "encodings/default_full.w--encoding-policy(fail).c"
{
	YYCTYPE yych;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= 0xD7FF) goto yy2;
	if (yych <= 0xDFFF) goto yy4;
yy2:
	++YYCURSOR;
#line 4 "encodings/default_full.w--encoding-policy(fail).re"
	{ return FULL; }
#line 17 "encodings/default_full.w--encoding-policy(fail).c"
yy4:
	++YYCURSOR;
#line 3 "encodings/default_full.w--encoding-policy(fail).re"
	{ return DEFAULT; }
#line 22 "encodings/default_full.w--encoding-policy(fail).c"
}
#line 5 "encodings/default_full.w--encoding-policy(fail).re"

