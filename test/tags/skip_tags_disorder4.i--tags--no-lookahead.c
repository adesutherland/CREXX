/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --tags --no-lookahead

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR++;
	switch (yych) {
	case 'c':	goto yy4;
	default:	goto yy2;
	}
yy2:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR++;
	switch (yych) {
	case 'c':
		yyt1 = YYCURSOR;
		goto yy5;
	default:	goto yy2;
	}
yy4:
	yych = *YYCURSOR++;
	yyt1 = YYCURSOR;
	switch (yych) {
	case 'a':	goto yy10;
	case 'c':	goto yy5;
	default:	goto yy8;
	}
yy5:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':	goto yy7;
	default:
		++YYCURSOR;
		yyt1 = YYCURSOR;
		goto yy5;
	}
yy7:
	t = yyt1;
	{}
yy8:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR++;
	switch (yych) {
	case 'a':	goto yy10;
	case 'c':
		yyt1 = YYCURSOR;
		goto yy5;
	default:	goto yy8;
	}
yy10:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case 'c':	goto yy12;
	default:
		++YYCURSOR;
		goto yy10;
	}
yy12:
	t = yyt1;
	{}
}

tags/skip_tags_disorder4.i--tags--no-lookahead.re:4:17: warning: rule matches empty string [-Wmatch-empty-string]
tags/skip_tags_disorder4.i--tags--no-lookahead.re:4:17: warning: tag 't' has 2nd degree of nondeterminism [-Wnondeterministic-tags]
