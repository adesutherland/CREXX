/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -cgi
// multiple scanners

enum YYCONDTYPE {
	yycr1,
	yycr2
};


void scan(unsigned char* in)
{

{
	unsigned char yych;
	static void *yyctable[2] = {
		&&yyc_r1,
		&&yyc_r2,
	};
	goto *yyctable[YYGETCONDITION()];
/* *********************************** */
yyc_r1:
	if (limit1 <= cursor1) fill1(1);
	yych = *cursor1;
	if (yych <= '2') {
		if (yych <= '0') goto yy1;
		if (yych <= '1') goto yy2;
		goto yy3;
	} else {
		if (yych <= '`') goto yy1;
		if (yych <= 'a') goto yy4;
		if (yych <= 'b') goto yy5;
	}
yy1:
yy2:
	++cursor1;
	{ return "1"; }
yy3:
	++cursor1;
	{ return "2"; }
yy4:
	++cursor1;
	{ return "a"; }
yy5:
	++cursor1;
	{ return "b"; }
/* *********************************** */
yyc_r2:
	if (limit1 <= cursor1) fill1(1);
	yych = *cursor1;
	if (yych <= '2') {
		if (yych <= '0') goto yy7;
		if (yych <= '1') goto yy8;
		goto yy9;
	} else {
		if (yych == 'b') goto yy10;
	}
yy7:
yy8:
	++cursor1;
	{ return "1"; }
yy9:
	++cursor1;
	{ return "2"; }
yy10:
	++cursor1;
	{ return "b"; }
}

}

void scan(unsigned short* in)
{

{
	unsigned short yych;
	static void *yyctable[2] = {
		&&yyc_r1,
		&&yyc_r2,
	};
	goto *yyctable[YYGETCONDITION()];
/* *********************************** */
yyc_r1:
	if (limit2 <= cursor2) fill2(1);
	yych = *cursor2;
	if (yych <= '2') {
		if (yych <= '0') goto yy12;
		if (yych <= '1') goto yy13;
		goto yy14;
	} else {
		if (yych <= '`') goto yy12;
		if (yych <= 'a') goto yy15;
		if (yych <= 'b') goto yy16;
	}
yy12:
yy13:
	++cursor2;
	{ return "1"; }
yy14:
	++cursor2;
	{ return "2"; }
yy15:
	++cursor2;
	{ return "a"; }
yy16:
	++cursor2;
	{ return "b"; }
/* *********************************** */
yyc_r2:
	if (limit2 <= cursor2) fill2(1);
	yych = *cursor2;
	if (yych <= '2') {
		if (yych <= '0') goto yy18;
		if (yych <= '1') goto yy19;
		goto yy20;
	} else {
		if (yych == 'b') goto yy21;
	}
yy18:
yy19:
	++cursor2;
	{ return "1"; }
yy20:
	++cursor2;
	{ return "2"; }
yy21:
	++cursor2;
	{ return "b"; }
}

}

void scan(unsigned int* in)
{

{
	unsigned int yych;
	static void *yyctable[2] = {
		&&yyc_r1,
		&&yyc_r2,
	};
	goto *yyctable[YYGETCONDITION()];
/* *********************************** */
yyc_r1:
	if (limit3 <= cursor3) fill3(1);
	yych = *cursor3;
	if (yych <= '2') {
		if (yych <= '0') goto yy23;
		if (yych <= '1') goto yy24;
		goto yy25;
	} else {
		if (yych <= '`') goto yy23;
		if (yych <= 'a') goto yy26;
		if (yych <= 'b') goto yy27;
	}
yy23:
yy24:
	++cursor3;
	{ return "1"; }
yy25:
	++cursor3;
	{ return "2"; }
yy26:
	++cursor3;
	{ return "a"; }
yy27:
	++cursor3;
	{ return "b"; }
/* *********************************** */
yyc_r2:
	if (limit3 <= cursor3) fill3(1);
	yych = *cursor3;
	if (yych <= '2') {
		if (yych <= '0') goto yy29;
		if (yych <= '1') goto yy30;
		goto yy31;
	} else {
		if (yych == 'b') goto yy32;
	}
yy29:
yy30:
	++cursor3;
	{ return "1"; }
yy31:
	++cursor3;
	{ return "2"; }
yy32:
	++cursor3;
	{ return "b"; }
}

}

reuse/repeat-02.re:14:2: warning: control flow in condition 'r1' is undefined for strings that match '[\x0-\x30\x33-\x60\x63-\xFF]', use default rule '*' [-Wundefined-control-flow]
reuse/repeat-02.re:14:2: warning: control flow in condition 'r2' is undefined for strings that match '[\x0-\x30\x33-\x61\x63-\xFF]', use default rule '*' [-Wundefined-control-flow]
reuse/repeat-02.re:26:0: warning: control flow in condition 'r1' is undefined for strings that match '[\x0-\x30\x33-\x60\x63-\xFF]', use default rule '*' [-Wundefined-control-flow]
reuse/repeat-02.re:26:0: warning: control flow in condition 'r2' is undefined for strings that match '[\x0-\x30\x33-\x61\x63-\xFF]', use default rule '*' [-Wundefined-control-flow]
reuse/repeat-02.re:38:0: warning: control flow in condition 'r1' is undefined for strings that match '[\x0-\x30\x33-\x60\x63-\xFF]', use default rule '*' [-Wundefined-control-flow]
reuse/repeat-02.re:38:0: warning: control flow in condition 'r2' is undefined for strings that match '[\x0-\x30\x33-\x61\x63-\xFF]', use default rule '*' [-Wundefined-control-flow]
