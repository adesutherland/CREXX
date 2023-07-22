/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -if
// This test is for `getstate:re2c` and fill labels.

// global `getstate:re2c` (start in x)
switch (YYGETSTATE()) {
	default: goto yy0;
	case 0: goto yyFillLabel0;
	case 1: goto Ly1;
	case 2: goto Lz2;
}


// `getstate:re2c` for y, x, z (start in y)
switch (YYGETSTATE()) {
	default: goto yy2;
	case 1: goto Ly1;
	case 0: goto yyFillLabel0;
	case 2: goto Lz2;
}


// global block w (no rules)


// global block x (with start label)

{
yy0:
	if (YYLIMIT <= YYCURSOR) {
		YYSETSTATE(0);
		YYFILL(1);
	}
yyFillLabel0:
	++YYCURSOR;
	YYSETSTATE(-1);
	{ x }
}


// unnamed global block (no rules, but changes fill label to 'Ly')


// global block y (with start label)

{
yy2:
	if (YYLIMIT <= YYCURSOR) {
		YYSETSTATE(1);
		YYFILL(1);
	}
Ly1:
	++YYCURSOR;
	YYSETSTATE(-1);
	{ y }
}


// unnamed global block (no rules, but changes fill label to 'L??')


// local block z (no start label)

{
	if (YYLIMIT <= YYCURSOR) {
		YYSETSTATE(2);
		YYFILL(1);
	}
Lz2:
	++YYCURSOR;
	YYSETSTATE(-1);
	{ z }
}


// the same directives at the end of file (in different order)
// should be no different from the ones at the beginning of the file

// `getstate:re2c` for y, x, z (start in y)
switch (YYGETSTATE()) {
	default: goto yy2;
	case 1: goto Ly1;
	case 0: goto yyFillLabel0;
	case 2: goto Lz2;
}


// global `getstate:re2c` (start in x)
switch (YYGETSTATE()) {
	default: goto yy0;
	case 0: goto yyFillLabel0;
	case 1: goto Ly1;
	case 2: goto Lz2;
}

