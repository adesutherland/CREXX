/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --posix-captures --dump-nfa --fixed-tags toplevel

{
	YYCTYPE yych;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'a':
			yyt1 = yyt2 = YYCURSOR;
			goto yy2;
		default:
			yyt2 = yyt3 = NULL;
			yyt1 = YYCURSOR;
			goto yy1;
	}
yy1:
	yynmatch = 2;
	yypmatch[0] = yyt1;
	yypmatch[2] = yyt2;
	yypmatch[3] = yyt3;
	yypmatch[1] = YYCURSOR;
	{}
yy2:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'a':
			yyt2 = YYCURSOR;
			goto yy2;
		default:
			yyt3 = YYCURSOR;
			goto yy1;
	}
}

debug/nfa.re:3:11: warning: rule matches empty string [-Wmatch-empty-string]
digraph NFA {
  rankdir=LR
  node[shape=Mrecord fontname=Courier height=0.2 width=0.2]
  edge[arrowhead=vee fontname=Courier label=" "]

  9 -> 8 [label="/0&uarr;(1)"]
  8 -> 5
  8 -> 7 [color=lightgray]
  5 -> 4 [label="/2&uarr;(2)"]
  4 -> 3 [label="97"]
  3 -> 2 [label="/3&uarr;(1)"]
  2 -> 5
  2 -> 1 [color=lightgray]
  1 -> 0 [label="/1&uarr;(0)"]
  0 [fillcolor=gray]
  7 -> 6 [label="/2&darr;(2)"]
  6 -> 1 [label="/3&darr;(1)"]
}
