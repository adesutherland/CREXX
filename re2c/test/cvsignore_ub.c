/* Generated by re2c */
#line 1 "cvsignore_ub.re"
// re2c $INPUT -o $OUTPUT -ub

#define YYFILL(n) if (cursor >= limit) break;
#define YYCTYPE unsigned int
#define YYCURSOR cursor
#define YYLIMIT limit
#define YYMARKER marker

#line 17 "cvsignore_ub.re"


#define APPEND(text) \
	append(output, outsize, text, sizeof(text) - sizeof(YYCTYPE))

inline void append(YYCTYPE *output, size_t & outsize, const YYCTYPE * text, size_t len)
{
	memcpy(output + outsize, text, len);
	outsize += (len / sizeof(YYCTYPE));
}

void scan(YYCTYPE *pText, size_t *pSize, int *pbChanged)
{
	// rule
	// scan lines
	// find $ in lines
	//   compact $<keyword>: .. $ to $<keyword>$
  
	YYCTYPE *output;
	const YYCTYPE *cursor, *limit, *marker;

	cursor = marker = output = *pText;

	size_t insize = *pSize;
	size_t outsize = 0;

	limit = cursor + insize;

	while(1) {
loop:

#line 44 "cvsignore_ub.c"
{
	YYCTYPE yych;
	static const unsigned char yybm[] = {
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128,   0, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128,   0, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
	};
	if ((YYLIMIT - YYCURSOR) < 11) YYFILL(11);
	yych = *YYCURSOR;
	if (yych == '$') goto yy2;
	++YYCURSOR;
yy1:
#line 54 "cvsignore_ub.re"
	{ output[outsize++] = cursor[-1]; if (cursor >= limit) break; goto loop; }
#line 88 "cvsignore_ub.c"
yy2:
	yych = *(YYMARKER = ++YYCURSOR);
	switch (yych) {
		case 'D': goto yy3;
		case 'I': goto yy5;
		case 'L': goto yy6;
		case 'R': goto yy7;
		case 'S': goto yy8;
		default: goto yy1;
	}
yy3:
	yych = *++YYCURSOR;
	if (yych == 'a') goto yy9;
yy4:
	YYCURSOR = YYMARKER;
	goto yy1;
yy5:
	yych = *++YYCURSOR;
	if (yych == 'd') goto yy10;
	goto yy4;
yy6:
	yych = *++YYCURSOR;
	if (yych == 'o') goto yy11;
	goto yy4;
yy7:
	yych = *++YYCURSOR;
	if (yych == 'e') goto yy12;
	goto yy4;
yy8:
	yych = *++YYCURSOR;
	if (yych == 'o') goto yy13;
	goto yy4;
yy9:
	yych = *++YYCURSOR;
	if (yych == 't') goto yy14;
	goto yy4;
yy10:
	yych = *++YYCURSOR;
	if (yych == '$') goto yy15;
	if (yych == ':') goto yy16;
	goto yy4;
yy11:
	yych = *++YYCURSOR;
	if (yych == 'g') goto yy17;
	goto yy4;
yy12:
	yych = *++YYCURSOR;
	if (yych == 'v') goto yy18;
	goto yy4;
yy13:
	yych = *++YYCURSOR;
	if (yych == 'u') goto yy19;
	goto yy4;
yy14:
	yych = *++YYCURSOR;
	if (yych == 'e') goto yy20;
	goto yy4;
yy15:
	++YYCURSOR;
#line 50 "cvsignore_ub.re"
	{ APPEND(L"$" L"Id$"); goto loop; }
#line 150 "cvsignore_ub.c"
yy16:
	yych = *++YYCURSOR;
	if (yych == '$') goto yy4;
	goto yy22;
yy17:
	yych = *++YYCURSOR;
	if (yych == '$') goto yy23;
	if (yych == ':') goto yy24;
	goto yy4;
yy18:
	yych = *++YYCURSOR;
	if (yych == 'i') goto yy25;
	goto yy4;
yy19:
	yych = *++YYCURSOR;
	if (yych == 'r') goto yy26;
	goto yy4;
yy20:
	yych = *++YYCURSOR;
	if (yych == '$') goto yy27;
	if (yych == ':') goto yy28;
	goto yy4;
yy21:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy22:
	if (yych & ~0xFF) {
		goto yy21;
	} else if (yybm[0+yych] & 128) {
		goto yy21;
	}
	if (yych <= '\n') goto yy4;
	goto yy15;
yy23:
	++YYCURSOR;
#line 51 "cvsignore_ub.re"
	{ APPEND(L"$" L"Log$"); goto loop; }
#line 189 "cvsignore_ub.c"
yy24:
	yych = *++YYCURSOR;
	if (yych == '$') goto yy4;
	goto yy30;
yy25:
	yych = *++YYCURSOR;
	if (yych == 's') goto yy31;
	goto yy4;
yy26:
	yych = *++YYCURSOR;
	if (yych == 'c') goto yy32;
	goto yy4;
yy27:
	++YYCURSOR;
#line 49 "cvsignore_ub.re"
	{ APPEND(L"$" L"Date$"); goto loop; }
#line 206 "cvsignore_ub.c"
yy28:
	yych = *++YYCURSOR;
	if (yych == '$') goto yy4;
	goto yy34;
yy29:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy30:
	if (yych == '\n') goto yy4;
	if (yych == '$') goto yy23;
	goto yy29;
yy31:
	yych = *++YYCURSOR;
	if (yych == 'i') goto yy35;
	goto yy4;
yy32:
	yych = *++YYCURSOR;
	if (yych == 'e') goto yy36;
	goto yy4;
yy33:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy34:
	if (yych == '\n') goto yy4;
	if (yych == '$') goto yy27;
	goto yy33;
yy35:
	yych = *++YYCURSOR;
	if (yych == 'o') goto yy37;
	goto yy4;
yy36:
	yych = *++YYCURSOR;
	if (yych == '$') goto yy38;
	if (yych == ':') goto yy39;
	goto yy4;
yy37:
	yych = *++YYCURSOR;
	if (yych == 'n') goto yy40;
	goto yy4;
yy38:
	++YYCURSOR;
#line 53 "cvsignore_ub.re"
	{ APPEND(L"$" L"Source$"); goto loop; }
#line 252 "cvsignore_ub.c"
yy39:
	yych = *++YYCURSOR;
	if (yych == '$') goto yy4;
	goto yy42;
yy40:
	yych = *++YYCURSOR;
	if (yych == '$') goto yy43;
	if (yych == ':') goto yy44;
	goto yy4;
yy41:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy42:
	if (yych == '\n') goto yy4;
	if (yych == '$') goto yy38;
	goto yy41;
yy43:
	++YYCURSOR;
#line 52 "cvsignore_ub.re"
	{ APPEND(L"$" L"Revision$"); goto loop; }
#line 274 "cvsignore_ub.c"
yy44:
	yych = *++YYCURSOR;
	if (yych == '$') goto yy4;
	goto yy46;
yy45:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy46:
	if (yych == '\n') goto yy4;
	if (yych == '$') goto yy43;
	goto yy45;
}
#line 56 "cvsignore_ub.re"

	}
	output[outsize] = '\0';

	// set the new size
	*pSize = outsize;
	
	*pbChanged = (insize == outsize) ? 0 : 1;
}
