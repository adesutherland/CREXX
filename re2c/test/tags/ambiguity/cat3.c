/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i
// default API: no warning (fixed tag => no nondeterminism)

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	switch (yych) {
		case 'a': goto yy3;
		default: goto yy1;
	}
yy1:
	++YYCURSOR;
yy2:
	{}
yy3:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'a': goto yy4;
		default: goto yy2;
	}
yy4:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'a': goto yy4;
		default: goto yy5;
	}
yy5:
	YYCURSOR -= 1;
	{}
}


// generic API: no warning (fixed tag => no nondeterminism)

{
	YYCTYPE yych;
	if (YYLESSTHAN(2)) YYFILL(2);
	yych = YYPEEK();
	switch (yych) {
		case 'a': goto yy9;
		default: goto yy7;
	}
yy7:
	YYSKIP();
yy8:
	{}
yy9:
	YYSKIP();
	yych = YYPEEK();
	switch (yych) {
		case 'a': goto yy10;
		default: goto yy8;
	}
yy10:
	YYSKIP();
	if (YYLESSTHAN(1)) YYFILL(1);
	yych = YYPEEK();
	switch (yych) {
		case 'a': goto yy10;
		default: goto yy11;
	}
yy11:
	YYSHIFT(-1);
	{}
}


// default API: warning (variable tag => nondeterminism), old-style context marker

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	switch (yych) {
		case 'a': goto yy15;
		default: goto yy13;
	}
yy13:
	++YYCURSOR;
yy14:
	{}
yy15:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'a':
			YYCTXMARKER = YYCURSOR;
			goto yy16;
		default: goto yy14;
	}
yy16:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'a':
			YYCTXMARKER = YYCURSOR;
			goto yy16;
		default: goto yy17;
	}
yy17:
	YYCURSOR = YYCTXMARKER;
	{}
}


// default API: warning (variable tag => nondeterminism), tag variable

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	switch (yych) {
		case 'a': goto yy21;
		default: goto yy19;
	}
yy19:
	++YYCURSOR;
yy20:
	{}
yy21:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'a':
			yyt1 = YYCURSOR;
			goto yy22;
		default: goto yy20;
	}
yy22:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'a':
			yyt1 = YYCURSOR;
			goto yy22;
		default: goto yy23;
	}
yy23:
	YYCURSOR = yyt1;
	{}
}


// generic API: warning (variable tag => nondeterminism), old-style context marker

{
	YYCTYPE yych;
	if (YYLESSTHAN(2)) YYFILL(2);
	yych = YYPEEK();
	switch (yych) {
		case 'a': goto yy27;
		default: goto yy25;
	}
yy25:
	YYSKIP();
yy26:
	{}
yy27:
	YYSKIP();
	yych = YYPEEK();
	switch (yych) {
		case 'a':
			YYBACKUPCTX();
			goto yy28;
		default: goto yy26;
	}
yy28:
	YYSKIP();
	if (YYLESSTHAN(1)) YYFILL(1);
	yych = YYPEEK();
	switch (yych) {
		case 'a':
			YYBACKUPCTX();
			goto yy28;
		default: goto yy29;
	}
yy29:
	YYRESTORECTX();
	{}
}


// generic API: warning (variable tag => nondeterminism), tag variable

{
	YYCTYPE yych;
	if (YYLESSTHAN(2)) YYFILL(2);
	yych = YYPEEK();
	switch (yych) {
		case 'a': goto yy33;
		default: goto yy31;
	}
yy31:
	YYSKIP();
yy32:
	{}
yy33:
	YYSKIP();
	yych = YYPEEK();
	switch (yych) {
		case 'a':
			YYSTAGP(yyt1);
			goto yy34;
		default: goto yy32;
	}
yy34:
	YYSKIP();
	if (YYLESSTHAN(1)) YYFILL(1);
	yych = YYPEEK();
	switch (yych) {
		case 'a':
			YYSTAGP(yyt1);
			goto yy34;
		default: goto yy35;
	}
yy35:
	YYRESTORETAG(yyt1);
	{}
}

tags/ambiguity/cat3.re:22:16: warning: trailing context has 2nd degree of nondeterminism [-Wnondeterministic-tags]
tags/ambiguity/cat3.re:30:16: warning: trailing context has 2nd degree of nondeterminism [-Wnondeterministic-tags]
tags/ambiguity/cat3.re:38:16: warning: trailing context has 2nd degree of nondeterminism [-Wnondeterministic-tags]
tags/ambiguity/cat3.re:46:16: warning: trailing context has 2nd degree of nondeterminism [-Wnondeterministic-tags]
