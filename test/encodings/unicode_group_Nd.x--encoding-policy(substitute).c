/* Generated by re2c */
#line 1 "encodings/unicode_group_Nd.x--encoding-policy(substitute).re"
// re2c $INPUT -o $OUTPUT -x --encoding-policy substitute
#include <stdio.h>
#include "utf16.h"
#define YYCTYPE unsigned short
bool scan(const YYCTYPE * start, const YYCTYPE * const limit)
{
	__attribute__((unused)) const YYCTYPE * YYMARKER; // silence compiler warnings when YYMARKER is not used
#	define YYCURSOR start
Nd:
	
#line 14 "encodings/unicode_group_Nd.x--encoding-policy(substitute).c"
{
	YYCTYPE yych;
	yych = *YYCURSOR;
	if (yych <= 0x1819) {
		if (yych <= 0x0C65) {
			if (yych <= 0x096F) {
				if (yych <= 0x06EF) {
					if (yych <= '9') {
						if (yych >= '0') goto yy4;
					} else {
						if (yych <= 0x065F) goto yy2;
						if (yych <= 0x0669) goto yy4;
					}
				} else {
					if (yych <= 0x07BF) {
						if (yych <= 0x06F9) goto yy4;
					} else {
						if (yych <= 0x07C9) goto yy4;
						if (yych >= 0x0966) goto yy4;
					}
				}
			} else {
				if (yych <= 0x0AE5) {
					if (yych <= 0x09EF) {
						if (yych >= 0x09E6) goto yy4;
					} else {
						if (yych <= 0x0A65) goto yy2;
						if (yych <= 0x0A6F) goto yy4;
					}
				} else {
					if (yych <= 0x0B6F) {
						if (yych <= 0x0AEF) goto yy4;
						if (yych >= 0x0B66) goto yy4;
					} else {
						if (yych <= 0x0BE5) goto yy2;
						if (yych <= 0x0BEF) goto yy4;
					}
				}
			}
		} else {
			if (yych <= 0x0ECF) {
				if (yych <= 0x0D6F) {
					if (yych <= 0x0CE5) {
						if (yych <= 0x0C6F) goto yy4;
					} else {
						if (yych <= 0x0CEF) goto yy4;
						if (yych >= 0x0D66) goto yy4;
					}
				} else {
					if (yych <= 0x0DEF) {
						if (yych >= 0x0DE6) goto yy4;
					} else {
						if (yych <= 0x0E4F) goto yy2;
						if (yych <= 0x0E59) goto yy4;
					}
				}
			} else {
				if (yych <= 0x1049) {
					if (yych <= 0x0F1F) {
						if (yych <= 0x0ED9) goto yy4;
					} else {
						if (yych <= 0x0F29) goto yy4;
						if (yych >= 0x1040) goto yy4;
					}
				} else {
					if (yych <= 0x17DF) {
						if (yych <= 0x108F) goto yy2;
						if (yych <= 0x1099) goto yy4;
					} else {
						if (yych <= 0x17E9) goto yy4;
						if (yych >= 0x1810) goto yy4;
					}
				}
			}
		}
	} else {
		if (yych <= 0xA8FF) {
			if (yych <= 0x1B59) {
				if (yych <= 0x1A7F) {
					if (yych <= 0x194F) {
						if (yych >= 0x1946) goto yy4;
					} else {
						if (yych <= 0x19CF) goto yy2;
						if (yych <= 0x19D9) goto yy4;
					}
				} else {
					if (yych <= 0x1A8F) {
						if (yych <= 0x1A89) goto yy4;
					} else {
						if (yych <= 0x1A99) goto yy4;
						if (yych >= 0x1B50) goto yy4;
					}
				}
			} else {
				if (yych <= 0x1C4F) {
					if (yych <= 0x1BB9) {
						if (yych >= 0x1BB0) goto yy4;
					} else {
						if (yych <= 0x1C3F) goto yy2;
						if (yych <= 0x1C49) goto yy4;
					}
				} else {
					if (yych <= 0xA629) {
						if (yych <= 0x1C59) goto yy4;
						if (yych >= 0xA620) goto yy4;
					} else {
						if (yych <= 0xA8CF) goto yy2;
						if (yych <= 0xA8D9) goto yy4;
					}
				}
			}
		} else {
			if (yych <= 0xD801) {
				if (yych <= 0xA9F9) {
					if (yych <= 0xA9CF) {
						if (yych <= 0xA909) goto yy4;
					} else {
						if (yych <= 0xA9D9) goto yy4;
						if (yych >= 0xA9F0) goto yy4;
					}
				} else {
					if (yych <= 0xABEF) {
						if (yych <= 0xAA4F) goto yy2;
						if (yych <= 0xAA59) goto yy4;
					} else {
						if (yych <= 0xABF9) goto yy4;
						if (yych >= 0xD801) goto yy6;
					}
				}
			} else {
				if (yych <= 0xD819) {
					if (yych <= 0xD804) {
						if (yych >= 0xD804) goto yy7;
					} else {
						if (yych <= 0xD805) goto yy8;
						if (yych <= 0xD806) goto yy9;
					}
				} else {
					if (yych <= 0xD835) {
						if (yych <= 0xD81A) goto yy10;
						if (yych >= 0xD835) goto yy11;
					} else {
						if (yych <= 0xFF0F) goto yy2;
						if (yych <= 0xFF19) goto yy4;
					}
				}
			}
		}
	}
yy2:
	++YYCURSOR;
yy3:
#line 14 "encodings/unicode_group_Nd.x--encoding-policy(substitute).re"
	{ return YYCURSOR == limit; }
#line 169 "encodings/unicode_group_Nd.x--encoding-policy(substitute).c"
yy4:
	++YYCURSOR;
#line 13 "encodings/unicode_group_Nd.x--encoding-policy(substitute).re"
	{ goto Nd; }
#line 174 "encodings/unicode_group_Nd.x--encoding-policy(substitute).c"
yy6:
	yych = *++YYCURSOR;
	if (yych <= 0xDC9F) goto yy3;
	if (yych <= 0xDCA9) goto yy4;
	goto yy3;
yy7:
	yych = *++YYCURSOR;
	if (yych <= 0xDD35) {
		if (yych <= 0xDC6F) {
			if (yych <= 0xDC65) goto yy3;
			goto yy4;
		} else {
			if (yych <= 0xDCEF) goto yy3;
			if (yych <= 0xDCF9) goto yy4;
			goto yy3;
		}
	} else {
		if (yych <= 0xDDD9) {
			if (yych <= 0xDD3F) goto yy4;
			if (yych <= 0xDDCF) goto yy3;
			goto yy4;
		} else {
			if (yych <= 0xDEEF) goto yy3;
			if (yych <= 0xDEF9) goto yy4;
			goto yy3;
		}
	}
yy8:
	yych = *++YYCURSOR;
	if (yych <= 0xDE4F) {
		if (yych <= 0xDCCF) goto yy3;
		if (yych <= 0xDCD9) goto yy4;
		goto yy3;
	} else {
		if (yych <= 0xDE59) goto yy4;
		if (yych <= 0xDEBF) goto yy3;
		if (yych <= 0xDEC9) goto yy4;
		goto yy3;
	}
yy9:
	yych = *++YYCURSOR;
	if (yych <= 0xDCDF) goto yy3;
	if (yych <= 0xDCE9) goto yy4;
	goto yy3;
yy10:
	yych = *++YYCURSOR;
	if (yych <= 0xDE5F) goto yy3;
	if (yych <= 0xDE69) goto yy4;
	if (yych <= 0xDF4F) goto yy3;
	if (yych <= 0xDF59) goto yy4;
	goto yy3;
yy11:
	yych = *++YYCURSOR;
	if (yych <= 0xDFCD) goto yy3;
	if (yych <= 0xDFFF) goto yy4;
	goto yy3;
}
#line 15 "encodings/unicode_group_Nd.x--encoding-policy(substitute).re"

}
static const unsigned int chars_Nd [] = {0x30,0x39,  0x660,0x669,  0x6f0,0x6f9,  0x7c0,0x7c9,  0x966,0x96f,  0x9e6,0x9ef,  0xa66,0xa6f,  0xae6,0xaef,  0xb66,0xb6f,  0xbe6,0xbef,  0xc66,0xc6f,  0xce6,0xcef,  0xd66,0xd6f,  0xde6,0xdef,  0xe50,0xe59,  0xed0,0xed9,  0xf20,0xf29,  0x1040,0x1049,  0x1090,0x1099,  0x17e0,0x17e9,  0x1810,0x1819,  0x1946,0x194f,  0x19d0,0x19d9,  0x1a80,0x1a89,  0x1a90,0x1a99,  0x1b50,0x1b59,  0x1bb0,0x1bb9,  0x1c40,0x1c49,  0x1c50,0x1c59,  0xa620,0xa629,  0xa8d0,0xa8d9,  0xa900,0xa909,  0xa9d0,0xa9d9,  0xa9f0,0xa9f9,  0xaa50,0xaa59,  0xabf0,0xabf9,  0xff10,0xff19,  0x104a0,0x104a9,  0x11066,0x1106f,  0x110f0,0x110f9,  0x11136,0x1113f,  0x111d0,0x111d9,  0x112f0,0x112f9,  0x114d0,0x114d9,  0x11650,0x11659,  0x116c0,0x116c9,  0x118e0,0x118e9,  0x16a60,0x16a69,  0x16b50,0x16b59,  0x1d7ce,0x1d7ff,  0x0,0x0};
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
	unsigned int * buffer_Nd = new unsigned int [1082];
	YYCTYPE * s = (YYCTYPE *) buffer_Nd;
	unsigned int buffer_len = encode_utf16 (chars_Nd, sizeof (chars_Nd) / sizeof (unsigned int), buffer_Nd);
	/* convert 32-bit code units to YYCTYPE; reuse the same buffer */
	for (unsigned int i = 0; i < buffer_len; ++i) s[i] = buffer_Nd[i];
	if (!scan (s, s + buffer_len))
		printf("test 'Nd' failed\n");
	delete [] buffer_Nd;
	return 0;
}
