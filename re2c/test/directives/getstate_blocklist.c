/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -if
// This test is for `getstate:re2c` with a list of blocks.

// `getstate:re2c:y` (start in y)
switch (YYGETSTATE()) {
	default: goto yy2;
	case 1: goto yyFillLabel1;
}


// `getstate:re2c:z:x` (start in z)
switch (YYGETSTATE()) {
	default: goto yy4;
	case 2: goto yyFillLabel2;
	case 0: goto yyFillLabel0;
}


// `getstate:re2c:y:x:z` (start in y)
switch (YYGETSTATE()) {
	default: goto yy2;
	case 1: goto yyFillLabel1;
	case 0: goto yyFillLabel0;
	case 2: goto yyFillLabel2;
}


// global block w (no rules)


// global block x (no start label)

{
	if (YYLIMIT <= YYCURSOR) {
		YYSETSTATE(0);
		YYFILL(1);
	}
yyFillLabel0:
	++YYCURSOR;
	YYSETSTATE(-1);
	{ x }
}


// global block y (with start label)

{
yy2:
	if (YYLIMIT <= YYCURSOR) {
		YYSETSTATE(1);
		YYFILL(1);
	}
yyFillLabel1:
	++YYCURSOR;
	YYSETSTATE(-1);
	{ y }
}


// local block z (with start label)

{
yy4:
	if (YYLIMIT <= YYCURSOR) {
		YYSETSTATE(2);
		YYFILL(1);
	}
yyFillLabel2:
	++YYCURSOR;
	YYSETSTATE(-1);
	{ z }
}


// the same directives at the end of file (in different order)
// should be no different from the ones at the beginning of the file

// `getstate:re2c:y:x:z` (start in y)
switch (YYGETSTATE()) {
	default: goto yy2;
	case 1: goto yyFillLabel1;
	case 0: goto yyFillLabel0;
	case 2: goto yyFillLabel2;
}


// `getstate:re2c:z:x` (start in z)
switch (YYGETSTATE()) {
	default: goto yy4;
	case 2: goto yyFillLabel2;
	case 0: goto yyFillLabel0;
}


// `getstate:re2c:y` (start in y)
switch (YYGETSTATE()) {
	default: goto yy2;
	case 1: goto yyFillLabel1;
}

