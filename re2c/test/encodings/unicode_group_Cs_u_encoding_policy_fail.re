// re2c $INPUT -o $OUTPUT -u --encoding-policy fail
#include <stdio.h>

#define YYCTYPE unsigned int
bool scan(const YYCTYPE * start, const YYCTYPE * const limit)
{
	__attribute__((unused)) const YYCTYPE * YYMARKER; // silence compiler warnings when YYMARKER is not used
#	define YYCURSOR start
Cs:
	/*!re2c
		re2c:yyfill:enable = 0;
		Cs = [\ud800-\udfff];
		Cs { goto Cs; }
		* { return YYCURSOR == limit; }
	*/
}
static const unsigned int chars_Cs [] = {0xd800,0xdfff,  0x0,0x0};
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
	unsigned int * buffer_Cs = new unsigned int [2049];
	YYCTYPE * s = (YYCTYPE *) buffer_Cs;
	unsigned int buffer_len = encode_utf32 (chars_Cs, sizeof (chars_Cs) / sizeof (unsigned int), buffer_Cs);
	/* convert 32-bit code units to YYCTYPE; reuse the same buffer */
	for (unsigned int i = 0; i < buffer_len; ++i) s[i] = buffer_Cs[i];
	if (!scan (s, s + buffer_len))
		printf("test 'Cs' failed\n");
	delete [] buffer_Cs;
	return 0;
}
