/* Generated by re2c */
#line 1 "conditions/condtype.c.re"
// re2c $INPUT -o $OUTPUT -c
int main ()
{
	YYCONDTYPE cond;
	char * YYCURSOR;
#define YYGETCONDITION() cond

#line 11 "conditions/condtype.c.c"
{
	unsigned char yych;
	switch (YYGETCONDITION()) {
	case yyca:
		goto yyc_a;
	case yycb:
		goto yyc_b;
	}
/* *********************************** */
yyc_a:
	yych = *YYCURSOR;
	switch (yych) {
	case 'a':	goto yy3;
	default:	goto yy2;
	}
yy2:
yy3:
	++YYCURSOR;
#line 10 "conditions/condtype.c.re"
	{}
#line 32 "conditions/condtype.c.c"
/* *********************************** */
yyc_b:
	yych = *YYCURSOR;
	switch (yych) {
	case 'b':	goto yy8;
	default:	goto yy7;
	}
yy7:
yy8:
	++YYCURSOR;
#line 11 "conditions/condtype.c.re"
	{}
#line 45 "conditions/condtype.c.c"
}
#line 12 "conditions/condtype.c.re"

	return 0;
}
conditions/condtype.c.re:12:2: warning: control flow in condition 'a' is undefined for strings that match '[\x0-\x60\x62-\xFF]', use default rule '*' [-Wundefined-control-flow]
conditions/condtype.c.re:12:2: warning: control flow in condition 'b' is undefined for strings that match '[\x0-\x61\x63-\xFF]', use default rule '*' [-Wundefined-control-flow]
