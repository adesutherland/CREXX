/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -cgir
// This test currently fails with error
// 're2c: error: line 13, column 9: code to default rule 'r1' is already defined'
// This must be fixed later




{
	YYCTYPE yych;
	static void *yyctable[2] = {
		&&yyc_r1,
		&&yyc_r2,
	};
	goto *yyctable[YYGETCONDITION()];
/* *********************************** */
yyc_r1:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	{
		static void *yytarget[256] = {
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy4,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy6,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy8,  &&yy10, &&yy12, &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,
			&&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2,  &&yy2
		};
		goto *yytarget[yych];
	}
yy2:
	++YYCURSOR;
	{ return "."; }
yy4:
	++YYCURSOR;
	{ return "DEFAULT - r1"; }
yy6:
	++YYCURSOR;
	{ return "1"; }
yy8:
	++YYCURSOR;
	{ return "a"; }
yy10:
	++YYCURSOR;
	{ return "b"; }
yy12:
	++YYCURSOR;
	{ return "c"; }
/* *********************************** */
yyc_r2:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '`') {
		if (yych != '\n') goto yy17;
	} else {
		if (yych <= 'a') goto yy19;
		if (yych == 'c') goto yy21;
		goto yy17;
	}
yy17:
	++YYCURSOR;
	{ return "."; }
yy19:
	++YYCURSOR;
	{ return "a"; }
yy21:
	++YYCURSOR;
	{ return "c"; }
}



{
	YYCTYPE yych;
	static void *yyctable[2] = {
		&&yyc_r1,
		&&yyc_r2,
	};
	goto *yyctable[YYGETCONDITION()];
/* *********************************** */
yyc_r1:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	{
		static void *yytarget[256] = {
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy25, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy28, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy30, &&yy32, &&yy34, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26,
			&&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26, &&yy26
		};
		goto *yytarget[yych];
	}
yy25:
yy26:
	++YYCURSOR;
	{ return "."; }
yy28:
	++YYCURSOR;
	{ return "2"; }
yy30:
	++YYCURSOR;
	{ return "a"; }
yy32:
	++YYCURSOR;
	{ return "b"; }
yy34:
	++YYCURSOR;
	{ return "c"; }
/* *********************************** */
yyc_r2:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '`') {
		if (yych == '\n') goto yy40;
	} else {
		if (yych <= 'a') goto yy42;
		if (yych == 'c') goto yy44;
	}
	++YYCURSOR;
	{ return "."; }
yy40:
	++YYCURSOR;
	{ return "DEFAULT - r2"; }
yy42:
	++YYCURSOR;
	{ return "a"; }
yy44:
	++YYCURSOR;
	{ return "c"; }
}

reuse/reuse_conds_default_0.cgir.re:16:2: warning: control flow in condition 'r2' is undefined for strings that match '\xA', use default rule '*' [-Wundefined-control-flow]
reuse/reuse_conds_default_0.cgir.re:21:2: warning: control flow in condition 'r1' is undefined for strings that match '\xA', use default rule '*' [-Wundefined-control-flow]
reuse/reuse_conds_default_0.cgir.re:16:2: warning: looks like you use hardcoded numbers instead of autogenerated condition names: better add '/*!types:re2c*/' directive or '-t, --type-header' option and don't rely on fixed condition order. [-Wcondition-order]
reuse/reuse_conds_default_0.cgir.re:21:2: warning: looks like you use hardcoded numbers instead of autogenerated condition names: better add '/*!types:re2c*/' directive or '-t, --type-header' option and don't rely on fixed condition order. [-Wcondition-order]
