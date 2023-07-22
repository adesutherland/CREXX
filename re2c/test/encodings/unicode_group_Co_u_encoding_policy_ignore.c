/* Generated by re2c */
#line 1 "encodings/unicode_group_Co_u_encoding_policy_ignore.re"
// re2c $INPUT -o $OUTPUT -u --encoding-policy ignore
#include <stdio.h>

#define YYCTYPE unsigned int
bool scan(const YYCTYPE * start, const YYCTYPE * const limit)
{
	__attribute__((unused)) const YYCTYPE * YYMARKER; // silence compiler warnings when YYMARKER is not used
#	define YYCURSOR start
Co:
	
#line 14 "encodings/unicode_group_Co_u_encoding_policy_ignore.c"
{
	YYCTYPE yych;
	yych = *YYCURSOR;
	if (yych <= 0x000EFFFF) {
		if (yych <= 0x0000DFFF) goto yy1;
		if (yych <= 0x0000F8FF) goto yy2;
	} else {
		if (yych <= 0x000FFFFD) goto yy2;
		if (yych <= 0x000FFFFF) goto yy1;
		if (yych <= 0x0010FFFD) goto yy2;
	}
yy1:
	++YYCURSOR;
#line 14 "encodings/unicode_group_Co_u_encoding_policy_ignore.re"
	{ return YYCURSOR == limit; }
#line 30 "encodings/unicode_group_Co_u_encoding_policy_ignore.c"
yy2:
	++YYCURSOR;
#line 13 "encodings/unicode_group_Co_u_encoding_policy_ignore.re"
	{ goto Co; }
#line 35 "encodings/unicode_group_Co_u_encoding_policy_ignore.c"
}
#line 15 "encodings/unicode_group_Co_u_encoding_policy_ignore.re"

}
static const unsigned int chars_Co [] = {0xe000,0xf8ff,  0xf0000,0xffffd,  0x100000,0x10fffd,  0x0,0x0};
static unsigned int encode_utf32 (const unsigned int * ranges, unsigned int ranges_count, unsigned int * s)
{
	unsigned int * const s_start = s;
	for (unsigned int i = 0; i < ranges_count; i += 2)
		for (unsigned int j = ranges[i]; j <= ranges[i + 1]; ++j)
			*s++ = j;
	return s - s_start;
}

int main ()
{
	unsigned int * buffer_Co = new unsigned int [137469];
	YYCTYPE * s = (YYCTYPE *) buffer_Co;
	unsigned int buffer_len = encode_utf32 (chars_Co, sizeof (chars_Co) / sizeof (unsigned int), buffer_Co);
	/* convert 32-bit code units to YYCTYPE; reuse the same buffer */
	for (unsigned int i = 0; i < buffer_len; ++i) s[i] = buffer_Co[i];
	if (!scan (s, s + buffer_len))
		printf("test 'Co' failed\n");
	delete [] buffer_Co;
	return 0;
}
