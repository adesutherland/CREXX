/* Generated by re2c */
#line 1 "encodings/unicode_group_Lt.u--encoding-policy(ignore).re"
// re2c $INPUT -o $OUTPUT -u --encoding-policy ignore
#include <stdio.h>

#define YYCTYPE unsigned int
bool scan(const YYCTYPE * start, const YYCTYPE * const limit)
{
	__attribute__((unused)) const YYCTYPE * YYMARKER; // silence compiler warnings when YYMARKER is not used
#	define YYCURSOR start
Lt:
	
#line 14 "encodings/unicode_group_Lt.u--encoding-policy(ignore).c"
{
	YYCTYPE yych;
	yych = *YYCURSOR;
	if (yych <= 0x00001F8F) {
		if (yych <= 0x000001CA) {
			if (yych <= 0x000001C5) {
				if (yych >= 0x000001C5) goto yy4;
			} else {
				if (yych == 0x000001C8) goto yy4;
			}
		} else {
			if (yych <= 0x000001F1) {
				if (yych <= 0x000001CB) goto yy4;
			} else {
				if (yych <= 0x000001F2) goto yy4;
				if (yych >= 0x00001F88) goto yy4;
			}
		}
	} else {
		if (yych <= 0x00001FBB) {
			if (yych <= 0x00001F9F) {
				if (yych >= 0x00001F98) goto yy4;
			} else {
				if (yych <= 0x00001FA7) goto yy2;
				if (yych <= 0x00001FAF) goto yy4;
			}
		} else {
			if (yych <= 0x00001FCC) {
				if (yych <= 0x00001FBC) goto yy4;
				if (yych >= 0x00001FCC) goto yy4;
			} else {
				if (yych == 0x00001FFC) goto yy4;
			}
		}
	}
yy2:
	++YYCURSOR;
#line 14 "encodings/unicode_group_Lt.u--encoding-policy(ignore).re"
	{ return YYCURSOR == limit; }
#line 54 "encodings/unicode_group_Lt.u--encoding-policy(ignore).c"
yy4:
	++YYCURSOR;
#line 13 "encodings/unicode_group_Lt.u--encoding-policy(ignore).re"
	{ goto Lt; }
#line 59 "encodings/unicode_group_Lt.u--encoding-policy(ignore).c"
}
#line 15 "encodings/unicode_group_Lt.u--encoding-policy(ignore).re"

}
static const unsigned int chars_Lt [] = {0x1c5,0x1c5,  0x1c8,0x1c8,  0x1cb,0x1cb,  0x1f2,0x1f2,  0x1f88,0x1f8f,  0x1f98,0x1f9f,  0x1fa8,0x1faf,  0x1fbc,0x1fbc,  0x1fcc,0x1fcc,  0x1ffc,0x1ffc,  0x0,0x0};
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
	unsigned int * buffer_Lt = new unsigned int [32];
	YYCTYPE * s = (YYCTYPE *) buffer_Lt;
	unsigned int buffer_len = encode_utf32 (chars_Lt, sizeof (chars_Lt) / sizeof (unsigned int), buffer_Lt);
	/* convert 32-bit code units to YYCTYPE; reuse the same buffer */
	for (unsigned int i = 0; i < buffer_len; ++i) s[i] = buffer_Lt[i];
	if (!scan (s, s + buffer_len))
		printf("test 'Lt' failed\n");
	delete [] buffer_Lt;
	return 0;
}
