/* Generated by re2c */
#line 1 "encodings/unicode_group_Pi.8--encoding-policy(fail).re"
// re2c $INPUT -o $OUTPUT -8 --encoding-policy fail
#include <stdio.h>
#include "utf8.h"
#define YYCTYPE unsigned char
bool scan(const YYCTYPE * start, const YYCTYPE * const limit)
{
	__attribute__((unused)) const YYCTYPE * YYMARKER; // silence compiler warnings when YYMARKER is not used
#	define YYCURSOR start
Pi:
	
#line 14 "encodings/unicode_group_Pi.8--encoding-policy(fail).c"
{
	YYCTYPE yych;
	yych = *YYCURSOR;
	switch (yych) {
	case 0xC2:	goto yy4;
	case 0xE2:	goto yy5;
	default:	goto yy2;
	}
yy2:
	++YYCURSOR;
yy3:
#line 14 "encodings/unicode_group_Pi.8--encoding-policy(fail).re"
	{ return YYCURSOR == limit; }
#line 28 "encodings/unicode_group_Pi.8--encoding-policy(fail).c"
yy4:
	yych = *++YYCURSOR;
	switch (yych) {
	case 0xAB:	goto yy6;
	default:	goto yy3;
	}
yy5:
	yych = *(YYMARKER = ++YYCURSOR);
	switch (yych) {
	case 0x80:	goto yy8;
	case 0xB8:	goto yy10;
	default:	goto yy3;
	}
yy6:
	++YYCURSOR;
#line 13 "encodings/unicode_group_Pi.8--encoding-policy(fail).re"
	{ goto Pi; }
#line 46 "encodings/unicode_group_Pi.8--encoding-policy(fail).c"
yy8:
	yych = *++YYCURSOR;
	switch (yych) {
	case 0x98:
	case 0x9B:
	case 0x9C:
	case 0x9F:
	case 0xB9:	goto yy6;
	default:	goto yy9;
	}
yy9:
	YYCURSOR = YYMARKER;
	goto yy3;
yy10:
	yych = *++YYCURSOR;
	switch (yych) {
	case 0x82:
	case 0x84:
	case 0x89:
	case 0x8C:
	case 0x9C:
	case 0xA0:	goto yy6;
	default:	goto yy9;
	}
}
#line 15 "encodings/unicode_group_Pi.8--encoding-policy(fail).re"

}
static const unsigned int chars_Pi [] = {0xab,0xab,  0x2018,0x2018,  0x201b,0x201c,  0x201f,0x201f,  0x2039,0x2039,  0x2e02,0x2e02,  0x2e04,0x2e04,  0x2e09,0x2e09,  0x2e0c,0x2e0c,  0x2e1c,0x2e1c,  0x2e20,0x2e20,  0x0,0x0};
static unsigned int encode_utf8 (const unsigned int * ranges, unsigned int ranges_count, unsigned int * s)
{
	unsigned int * const s_start = s;
	for (unsigned int i = 0; i < ranges_count - 2; i += 2)
		for (unsigned int j = ranges[i]; j <= ranges[i + 1]; ++j)
			s += re2c::utf8::rune_to_bytes (s, j);
	re2c::utf8::rune_to_bytes (s, ranges[ranges_count - 1]);
	return s - s_start + 1;
}

int main ()
{
	unsigned int * buffer_Pi = new unsigned int [52];
	YYCTYPE * s = (YYCTYPE *) buffer_Pi;
	unsigned int buffer_len = encode_utf8 (chars_Pi, sizeof (chars_Pi) / sizeof (unsigned int), buffer_Pi);
	/* convert 32-bit code units to YYCTYPE; reuse the same buffer */
	for (unsigned int i = 0; i < buffer_len; ++i) s[i] = buffer_Pi[i];
	if (!scan (s, s + buffer_len))
		printf("test 'Pi' failed\n");
	delete [] buffer_Pi;
	return 0;
}
