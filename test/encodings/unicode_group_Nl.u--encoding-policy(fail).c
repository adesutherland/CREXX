/* Generated by re2c */
#line 1 "encodings/unicode_group_Nl.u--encoding-policy(fail).re"
// re2c $INPUT -o $OUTPUT -u --encoding-policy fail
#include <stdio.h>

#define YYCTYPE unsigned int
bool scan(const YYCTYPE * start, const YYCTYPE * const limit)
{
	__attribute__((unused)) const YYCTYPE * YYMARKER; // silence compiler warnings when YYMARKER is not used
#	define YYCURSOR start
Nl:
	
#line 14 "encodings/unicode_group_Nl.u--encoding-policy(fail).c"
{
	YYCTYPE yych;
	yych = *YYCURSOR;
	if (yych <= 0x0000303A) {
		if (yych <= 0x00002188) {
			if (yych <= 0x0000215F) {
				if (yych <= 0x000016ED) goto yy2;
				if (yych <= 0x000016F0) goto yy4;
			} else {
				if (yych <= 0x00002182) goto yy4;
				if (yych >= 0x00002185) goto yy4;
			}
		} else {
			if (yych <= 0x00003020) {
				if (yych == 0x00003007) goto yy4;
			} else {
				if (yych <= 0x00003029) goto yy4;
				if (yych >= 0x00003038) goto yy4;
			}
		}
	} else {
		if (yych <= 0x00010341) {
			if (yych <= 0x0001013F) {
				if (yych <= 0x0000A6E5) goto yy2;
				if (yych <= 0x0000A6EF) goto yy4;
			} else {
				if (yych <= 0x00010174) goto yy4;
				if (yych >= 0x00010341) goto yy4;
			}
		} else {
			if (yych <= 0x000103D0) {
				if (yych == 0x0001034A) goto yy4;
			} else {
				if (yych <= 0x000103D5) goto yy4;
				if (yych <= 0x000123FF) goto yy2;
				if (yych <= 0x0001246E) goto yy4;
			}
		}
	}
yy2:
	++YYCURSOR;
#line 14 "encodings/unicode_group_Nl.u--encoding-policy(fail).re"
	{ return YYCURSOR == limit; }
#line 58 "encodings/unicode_group_Nl.u--encoding-policy(fail).c"
yy4:
	++YYCURSOR;
#line 13 "encodings/unicode_group_Nl.u--encoding-policy(fail).re"
	{ goto Nl; }
#line 63 "encodings/unicode_group_Nl.u--encoding-policy(fail).c"
}
#line 15 "encodings/unicode_group_Nl.u--encoding-policy(fail).re"

}
static const unsigned int chars_Nl [] = {0x16ee,0x16f0,  0x2160,0x2182,  0x2185,0x2188,  0x3007,0x3007,  0x3021,0x3029,  0x3038,0x303a,  0xa6e6,0xa6ef,  0x10140,0x10174,  0x10341,0x10341,  0x1034a,0x1034a,  0x103d1,0x103d5,  0x12400,0x1246e,  0x0,0x0};
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
	unsigned int * buffer_Nl = new unsigned int [237];
	YYCTYPE * s = (YYCTYPE *) buffer_Nl;
	unsigned int buffer_len = encode_utf32 (chars_Nl, sizeof (chars_Nl) / sizeof (unsigned int), buffer_Nl);
	/* convert 32-bit code units to YYCTYPE; reuse the same buffer */
	for (unsigned int i = 0; i < buffer_len; ++i) s[i] = buffer_Nl[i];
	if (!scan (s, s + buffer_len))
		printf("test 'Nl' failed\n");
	delete [] buffer_Nl;
	return 0;
}
