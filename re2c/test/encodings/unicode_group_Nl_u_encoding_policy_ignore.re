// re2c $INPUT -o $OUTPUT -u --encoding-policy ignore
#include <stdio.h>

#define YYCTYPE unsigned int
bool scan(const YYCTYPE * start, const YYCTYPE * const limit)
{
	__attribute__((unused)) const YYCTYPE * YYMARKER; // silence compiler warnings when YYMARKER is not used
#	define YYCURSOR start
Nl:
	/*!re2c
		re2c:yyfill:enable = 0;
		Nl = [\u16ee-\u16f0\u2160-\u2182\u2185-\u2188\u3007-\u3007\u3021-\u3029\u3038-\u303a\ua6e6-\ua6ef\U00010140-\U00010174\U00010341-\U00010341\U0001034a-\U0001034a\U000103d1-\U000103d5\U00012400-\U0001246e];
		Nl { goto Nl; }
		* { return YYCURSOR == limit; }
	*/
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
