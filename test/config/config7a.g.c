/* Generated by re2c */
#line 1 "config/config7a.g.re"
// re2c $INPUT -o $OUTPUT -g

#line 6 "config/config7a.g.c"
{
	YYCTYPE yych;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= 'E') {
		if (yych <= '@') goto yy2;
		if (yych <= 'D') goto yy4;
	} else {
		if (yych <= 'G') goto yy4;
		if (yych <= '`') goto yy2;
		if (yych <= 'g') goto yy4;
	}
yy2:
	++YYCURSOR;
#line 13 "config/config7a.g.re"
	{ return -1; }
#line 23 "config/config7a.g.c"
yy4:
	++YYCURSOR;
#line 11 "config/config7a.g.re"
	{ return 1; }
#line 28 "config/config7a.g.c"
}
#line 15 "config/config7a.g.re"

