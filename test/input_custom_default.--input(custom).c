/* Generated by re2c */
#line 1 "input_custom_default.--input(custom).re"
// re2c $INPUT -o $OUTPUT  --input custom
bool lex (const char * cursor, const char * const limit)
{
    const char * marker;
    const char * ctxmarker;
#   define YYCTYPE        char
#   define YYPEEK()       *cursor
#   define YYSKIP()       ++cursor
#   define YYBACKUP()     marker = cursor
#   define YYBACKUPCTX()  ctxmarker = cursor
#   define YYRESTORE()    cursor = marker
#   define YYRESTORECTX() cursor = ctxmarker
#   define YYLESSTHAN(n)  limit - cursor < n
#   define YYFILL(n)      {}
    
#line 19 "input_custom_default.--input(custom).c"
{
	YYCTYPE yych;
	if (YYLESSTHAN(13)) YYFILL(13);
	yych = YYPEEK();
	switch (yych) {
	case 'i':	goto yy4;
	default:	goto yy2;
	}
yy2:
	YYSKIP();
yy3:
#line 17 "input_custom_default.--input(custom).re"
	{ return false; }
#line 33 "input_custom_default.--input(custom).c"
yy4:
	YYSKIP();
	YYBACKUP();
	yych = YYPEEK();
	switch (yych) {
	case 'n':	goto yy5;
	default:	goto yy3;
	}
yy5:
	YYSKIP();
	yych = YYPEEK();
	switch (yych) {
	case 't':	goto yy7;
	default:	goto yy6;
	}
yy6:
	YYRESTORE();
	goto yy3;
yy7:
	YYSKIP();
	yych = YYPEEK();
	switch (yych) {
	case ' ':	goto yy8;
	default:	goto yy6;
	}
yy8:
	YYSKIP();
	yych = YYPEEK();
	switch (yych) {
	case 'b':	goto yy9;
	default:	goto yy6;
	}
yy9:
	YYSKIP();
	yych = YYPEEK();
	switch (yych) {
	case 'u':	goto yy10;
	default:	goto yy6;
	}
yy10:
	YYSKIP();
	yych = YYPEEK();
	switch (yych) {
	case 'f':	goto yy11;
	default:	goto yy6;
	}
yy11:
	YYSKIP();
	yych = YYPEEK();
	switch (yych) {
	case 'f':	goto yy12;
	default:	goto yy6;
	}
yy12:
	YYSKIP();
	yych = YYPEEK();
	switch (yych) {
	case 'e':	goto yy13;
	default:	goto yy6;
	}
yy13:
	YYSKIP();
	yych = YYPEEK();
	switch (yych) {
	case 'r':	goto yy14;
	default:	goto yy6;
	}
yy14:
	YYSKIP();
	yych = YYPEEK();
	switch (yych) {
	case ' ':	goto yy15;
	default:	goto yy6;
	}
yy15:
	YYSKIP();
	yych = YYPEEK();
	switch (yych) {
	case '[':
		YYBACKUPCTX();
		goto yy16;
	default:	goto yy6;
	}
yy16:
	YYSKIP();
	yych = YYPEEK();
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy17;
	default:	goto yy6;
	}
yy17:
	YYSKIP();
	if (YYLESSTHAN(1)) YYFILL(1);
	yych = YYPEEK();
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy17;
	case ']':	goto yy19;
	default:	goto yy6;
	}
yy19:
	YYSKIP();
	YYRESTORECTX();
#line 16 "input_custom_default.--input(custom).re"
	{ return true; }
#line 156 "input_custom_default.--input(custom).c"
}
#line 18 "input_custom_default.--input(custom).re"

}

int main ()
{
    char buffer [] = "int buffer [1024]";
    return !lex (buffer, buffer + sizeof (buffer));
}
