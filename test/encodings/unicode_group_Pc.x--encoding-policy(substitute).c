/* Generated by re2c */
#line 1 "encodings/unicode_group_Pc.x--encoding-policy(substitute).re"
// re2c $INPUT -o $OUTPUT -x --encoding-policy substitute
#include <stdio.h>
#include "utf16.h"
#define YYCTYPE unsigned short
bool scan(const YYCTYPE * start, const YYCTYPE * const limit)
{
	__attribute__((unused)) const YYCTYPE * YYMARKER; // silence compiler warnings when YYMARKER is not used
#	define YYCURSOR start
Pc:
	
#line 14 "encodings/unicode_group_Pc.x--encoding-policy(substitute).c"
{
	YYCTYPE yych;
	yych = *YYCURSOR;
	if (yych <= 0x2054) {
		if (yych <= 0x203E) {
			if (yych == '_') goto yy4;
		} else {
			if (yych <= 0x2040) goto yy4;
			if (yych >= 0x2054) goto yy4;
		}
	} else {
		if (yych <= 0xFE4C) {
			if (yych <= 0xFE32) goto yy2;
			if (yych <= 0xFE34) goto yy4;
		} else {
			if (yych <= 0xFE4F) goto yy4;
			if (yych == 0xFF3F) goto yy4;
		}
	}
yy2:
	++YYCURSOR;
#line 14 "encodings/unicode_group_Pc.x--encoding-policy(substitute).re"
	{ return YYCURSOR == limit; }
#line 38 "encodings/unicode_group_Pc.x--encoding-policy(substitute).c"
yy4:
	++YYCURSOR;
#line 13 "encodings/unicode_group_Pc.x--encoding-policy(substitute).re"
	{ goto Pc; }
#line 43 "encodings/unicode_group_Pc.x--encoding-policy(substitute).c"
}
#line 15 "encodings/unicode_group_Pc.x--encoding-policy(substitute).re"

}
static const unsigned int chars_Pc [] = {0x5f,0x5f,  0x203f,0x2040,  0x2054,0x2054,  0xfe33,0xfe34,  0xfe4d,0xfe4f,  0xff3f,0xff3f,  0x0,0x0};
static unsigned int encode_utf16 (const unsigned int * ranges, unsigned int ranges_count, unsigned int * s)
{
	unsigned int * const s_start = s;
	for (unsigned int i = 0; i < ranges_count; i += 2)
		for (unsigned int j = ranges[i]; j <= ranges[i + 1]; ++j)
		{
			if (j <= re2c::utf16::MAX_1WORD_RUNE)
				*s++ = j;
			else
			{
				*s++ = re2c::utf16::lead_surr(j);
				*s++ = re2c::utf16::trail_surr(j);
			}
		}
	return s - s_start;
}

int main ()
{
	unsigned int * buffer_Pc = new unsigned int [22];
	YYCTYPE * s = (YYCTYPE *) buffer_Pc;
	unsigned int buffer_len = encode_utf16 (chars_Pc, sizeof (chars_Pc) / sizeof (unsigned int), buffer_Pc);
	/* convert 32-bit code units to YYCTYPE; reuse the same buffer */
	for (unsigned int i = 0; i < buffer_len; ++i) s[i] = buffer_Pc[i];
	if (!scan (s, s + buffer_len))
		printf("test 'Pc' failed\n");
	delete [] buffer_Pc;
	return 0;
}
