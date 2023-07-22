/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -ir --case-ranges
#include <assert.h>

#define YYCTYPE char



static int lex1(const char *YYCURSOR)
{
    
{
	YYCTYPE yych;
	yych = *YYCURSOR;
	switch (yych) {
		case '!':
		case '#' ... '&':
		case '*':
		case '@':
		case '^': goto yy2;
		case '0' ... '9': goto yy3;
		case 'A' ... 'Z':
		case 'a' ... 'z': goto yy4;
		default: goto yy1;
	}
yy1:
	++YYCURSOR;
	{ return -1; }
yy2:
	++YYCURSOR;
	{ return 2; }
yy3:
	++YYCURSOR;
	{ return 1; }
yy4:
	++YYCURSOR;
	{ return 0; }
}

}

static int lex2(const char *YYCURSOR)
{
    
{
	YYCTYPE yych;
	yych = *YYCURSOR;
	switch (yych) {
		case '!':
		case '#':
		case '$':
		case '%':
		case '&':
		case '*':
		case '@':
		case '^': goto yy7;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': goto yy8;
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy9;
		default: goto yy6;
	}
yy6:
	++YYCURSOR;
	{ return -1; }
yy7:
	++YYCURSOR;
	{ return 2; }
yy8:
	++YYCURSOR;
	{ return 1; }
yy9:
	++YYCURSOR;
	{ return 0; }
}

}

static int lex3(const char *YYCURSOR)
{
    
{
	YYCTYPE yych;
	yych = *YYCURSOR;
	switch (yych) {
		case '!':
		case '#' ... '&':
		case '*':
		case '@':
		case '^': goto yy12;
		case '0' ... '9': goto yy13;
		case 'A' ... 'Z':
		case 'a' ... 'z': goto yy14;
		default: goto yy11;
	}
yy11:
	++YYCURSOR;
	{ return -1; }
yy12:
	++YYCURSOR;
	{ return 2; }
yy13:
	++YYCURSOR;
	{ return 1; }
yy14:
	++YYCURSOR;
	{ return 0; }
}

}

#define TEST(s, i) assert(\
    lex1(s) == i && \
    lex2(s) == i && \
    lex3(s) == i \
);

int main()
{
    TEST("a", 0);
    TEST("0", 1);
    TEST("!", 2);
    TEST("", -1);
    return 0;
}
