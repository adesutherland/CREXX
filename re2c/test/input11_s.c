/* Generated by re2c */
#line 1 "input11_s.re"
// re2c $INPUT -o $OUTPUT -s

#line 6 "input11_s.c"
{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 6) YYFILL(6);
	yych = *YYCURSOR;
	if (yych <= '@') {
		if (yych <= '/') {
			if (yych == '-') goto yy4;
		} else {
			if (yych <= '0') goto yy5;
			if (yych <= '9') goto yy7;
		}
	} else {
		if (yych <= 'q') {
			if (yych <= 'Z') goto yy9;
			if (yych >= 'a') goto yy9;
		} else {
			if (yych <= 'r') goto yy12;
			if (yych <= 'z') goto yy9;
		}
	}
	++YYCURSOR;
yy3:
#line 14 "input11_s.re"
	{ return -1; }
#line 31 "input11_s.c"
yy4:
	yych = *++YYCURSOR;
	if (yych <= '0') goto yy3;
	if (yych <= '9') goto yy7;
	goto yy3;
yy5:
	++YYCURSOR;
yy6:
#line 12 "input11_s.re"
	{ return 2; }
#line 42 "input11_s.c"
yy7:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '/') goto yy6;
	if (yych <= '9') goto yy7;
	goto yy6;
yy9:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy10:
	if (yych <= '@') {
		if (yych <= '/') goto yy11;
		if (yych <= '9') goto yy9;
	} else {
		if (yych <= 'Z') goto yy9;
		if (yych <= '`') goto yy11;
		if (yych <= 'z') goto yy9;
	}
yy11:
#line 11 "input11_s.re"
	{ return 1; }
#line 66 "input11_s.c"
yy12:
	yych = *++YYCURSOR;
	if (yych != 'e') goto yy10;
	yych = *++YYCURSOR;
	if (yych != '2') goto yy10;
	yych = *++YYCURSOR;
	if (yych != 'c') goto yy10;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych != ':') goto yy10;
	yych = *++YYCURSOR;
	if (yych <= '^') {
		if (yych <= '@') goto yy17;
		if (yych <= 'Z') goto yy18;
	} else {
		if (yych == '`') goto yy17;
		if (yych <= 'z') goto yy18;
	}
yy17:
	YYCURSOR = YYMARKER;
	goto yy11;
yy18:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= 'Z') {
		if (yych <= '/') goto yy20;
		if (yych <= ':') goto yy18;
		if (yych >= 'A') goto yy18;
	} else {
		if (yych <= '_') {
			if (yych >= '_') goto yy18;
		} else {
			if (yych <= '`') goto yy20;
			if (yych <= 'z') goto yy18;
		}
	}
yy20:
#line 10 "input11_s.re"
	{ return 0; }
#line 106 "input11_s.c"
}
#line 16 "input11_s.re"

