/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --tags --no-lookahead
// incorrect automaton if mapping of TDFA states ignores version order

{
	YYCTYPE yych;
	yyt1 = NULL;
	yyt2 = YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 8) YYFILL(8);
	yych = *(YYMARKER = YYCURSOR);
	switch (yych) {
	case 'a':
		++YYCURSOR;
		goto yy3;
	default:	goto yy2;
	}
yy2:
	t = yyt1;
	{}
yy3:
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':
		++YYCURSOR;
		goto yy5;
	default:	goto yy4;
	}
yy4:
	YYCURSOR = YYMARKER;
	goto yy2;
yy5:
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':
		++YYCURSOR;
		goto yy6;
	default:
		yyt1 = yyt2;
		goto yy2;
	}
yy6:
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':
		++YYCURSOR;
		goto yy7;
	default:	goto yy2;
	}
yy7:
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':
		++YYCURSOR;
		goto yy8;
	default:	goto yy2;
	}
yy8:
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':
		++YYCURSOR;
		goto yy9;
	default:
		yyt1 = yyt2;
		goto yy2;
	}
yy9:
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':
		++YYCURSOR;
		goto yy10;
	default:
		yyt1 = yyt2;
		goto yy2;
	}
yy10:
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':
		++YYCURSOR;
		goto yy11;
	default:	goto yy2;
	}
yy11:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':
		++YYCURSOR;
		goto yy11;
	default:
		yyt1 = yyt2;
		goto yy2;
	}
}

tags/map_ord2.i--tags--no-lookahead.re:5:24: warning: rule matches empty string [-Wmatch-empty-string]
tags/map_ord2.i--tags--no-lookahead.re:5:24: warning: tag 't' has 2nd degree of nondeterminism [-Wnondeterministic-tags]
