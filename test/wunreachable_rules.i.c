/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i

{
	YYCTYPE yych;
	goto yy0;
yy1:
	++YYCURSOR;
yy0:
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':	goto yy3;
	default:	goto yy1;
	}
yy3:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':	goto yy5;
	default:	goto yy4;
	}
yy4:
	{ 0 }
yy5:
	++YYCURSOR;
	goto yy4;
}



{
	YYCTYPE yych;
	goto yy6;
yy7:
	++YYCURSOR;
yy6:
	if ((YYLIMIT - YYCURSOR) < 3) YYFILL(3);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':	goto yy9;
	default:	goto yy7;
	}
yy9:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':	goto yy11;
	default:	goto yy10;
	}
yy10:
	{ 0 }
yy11:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':	goto yy12;
	default:	goto yy10;
	}
yy12:
	++YYCURSOR;
	goto yy10;
}



{
	YYCTYPE yych;
	goto yy13;
yy14:
	++YYCURSOR;
yy13:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':	goto yy16;
	default:	goto yy14;
	}
yy16:
	++YYCURSOR;
	{ 0 }
}



{
	YYCTYPE yych;
	goto yy18;
yy19:
	++YYCURSOR;
yy18:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':	goto yy21;
	default:	goto yy19;
	}
yy21:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':	goto yy21;
	default:	goto yy23;
	}
yy23:
	{ 0 }
}

wunreachable_rules.i.re:4:21: warning: unreachable rule (shadowed by rule at line 3) [-Wunreachable-rules]
wunreachable_rules.i.re:9:19: warning: unreachable rule (shadowed by rule at line 8) [-Wunreachable-rules]
wunreachable_rules.i.re:13:15: warning: rule matches empty string [-Wmatch-empty-string]
wunreachable_rules.i.re:14:15: warning: unreachable rule (shadowed by rule at line 13) [-Wunreachable-rules]
wunreachable_rules.i.re:19:15: warning: unreachable rule (shadowed by rule at line 18) [-Wunreachable-rules]
