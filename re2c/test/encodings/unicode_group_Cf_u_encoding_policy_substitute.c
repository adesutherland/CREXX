/* Generated by re2c */
#line 1 "encodings/unicode_group_Cf_u_encoding_policy_substitute.re"
// re2c $INPUT -o $OUTPUT -u --encoding-policy substitute
#include <stdio.h>

#define YYCTYPE unsigned int
bool scan(const YYCTYPE * start, const YYCTYPE * const limit)
{
	__attribute__((unused)) const YYCTYPE * YYMARKER; // silence compiler warnings when YYMARKER is not used
#	define YYCURSOR start
Cf:
	
#line 14 "encodings/unicode_group_Cf_u_encoding_policy_substitute.c"
{
	YYCTYPE yych;
	yych = *YYCURSOR;
	if (yych <= 0x0000205F) {
		if (yych <= 0x000006DD) {
			if (yych <= 0x00000605) {
				if (yych == 0x000000AD) goto yy4;
				if (yych >= 0x00000600) goto yy4;
			} else {
				if (yych == 0x0000061C) goto yy4;
				if (yych >= 0x000006DD) goto yy4;
			}
		} else {
			if (yych <= 0x0000180E) {
				if (yych == 0x0000070F) goto yy4;
				if (yych >= 0x0000180E) goto yy4;
			} else {
				if (yych <= 0x0000200F) {
					if (yych >= 0x0000200B) goto yy4;
				} else {
					if (yych <= 0x00002029) goto yy2;
					if (yych <= 0x0000202E) goto yy4;
				}
			}
		}
	} else {
		if (yych <= 0x000110BD) {
			if (yych <= 0x0000FEFE) {
				if (yych == 0x00002065) goto yy2;
				if (yych <= 0x0000206F) goto yy4;
			} else {
				if (yych <= 0x0000FFF8) {
					if (yych <= 0x0000FEFF) goto yy4;
				} else {
					if (yych <= 0x0000FFFB) goto yy4;
					if (yych >= 0x000110BD) goto yy4;
				}
			}
		} else {
			if (yych <= 0x0001D17A) {
				if (yych <= 0x0001BC9F) goto yy2;
				if (yych <= 0x0001BCA3) goto yy4;
				if (yych >= 0x0001D173) goto yy4;
			} else {
				if (yych <= 0x000E0001) {
					if (yych >= 0x000E0001) goto yy4;
				} else {
					if (yych <= 0x000E001F) goto yy2;
					if (yych <= 0x000E007F) goto yy4;
				}
			}
		}
	}
yy2:
	++YYCURSOR;
#line 14 "encodings/unicode_group_Cf_u_encoding_policy_substitute.re"
	{ return YYCURSOR == limit; }
#line 72 "encodings/unicode_group_Cf_u_encoding_policy_substitute.c"
yy4:
	++YYCURSOR;
#line 13 "encodings/unicode_group_Cf_u_encoding_policy_substitute.re"
	{ goto Cf; }
#line 77 "encodings/unicode_group_Cf_u_encoding_policy_substitute.c"
}
#line 15 "encodings/unicode_group_Cf_u_encoding_policy_substitute.re"

}
static const unsigned int chars_Cf [] = {0xad,0xad,  0x600,0x605,  0x61c,0x61c,  0x6dd,0x6dd,  0x70f,0x70f,  0x180e,0x180e,  0x200b,0x200f,  0x202a,0x202e,  0x2060,0x2064,  0x2066,0x206f,  0xfeff,0xfeff,  0xfff9,0xfffb,  0x110bd,0x110bd,  0x1bca0,0x1bca3,  0x1d173,0x1d17a,  0xe0001,0xe0001,  0xe0020,0xe007f,  0x0,0x0};
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
	unsigned int * buffer_Cf = new unsigned int [151];
	YYCTYPE * s = (YYCTYPE *) buffer_Cf;
	unsigned int buffer_len = encode_utf32 (chars_Cf, sizeof (chars_Cf) / sizeof (unsigned int), buffer_Cf);
	/* convert 32-bit code units to YYCTYPE; reuse the same buffer */
	for (unsigned int i = 0; i < buffer_len; ++i) s[i] = buffer_Cf[i];
	if (!scan (s, s + buffer_len))
		printf("test 'Cf' failed\n");
	delete [] buffer_Cf;
	return 0;
}
