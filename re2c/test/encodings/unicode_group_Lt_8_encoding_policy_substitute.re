// re2c $INPUT -o $OUTPUT -8 --encoding-policy substitute
#include <stdio.h>
#include "utf8.h"
#define YYCTYPE unsigned char
bool scan(const YYCTYPE * start, const YYCTYPE * const limit)
{
	__attribute__((unused)) const YYCTYPE * YYMARKER; // silence compiler warnings when YYMARKER is not used
#	define YYCURSOR start
Lt:
	/*!re2c
		re2c:yyfill:enable = 0;
		Lt = [\u01c5-\u01c5\u01c8-\u01c8\u01cb-\u01cb\u01f2-\u01f2\u1f88-\u1f8f\u1f98-\u1f9f\u1fa8-\u1faf\u1fbc-\u1fbc\u1fcc-\u1fcc\u1ffc-\u1ffc];
		Lt { goto Lt; }
		* { return YYCURSOR == limit; }
	*/
}
static const unsigned int chars_Lt [] = {0x1c5,0x1c5,  0x1c8,0x1c8,  0x1cb,0x1cb,  0x1f2,0x1f2,  0x1f88,0x1f8f,  0x1f98,0x1f9f,  0x1fa8,0x1faf,  0x1fbc,0x1fbc,  0x1fcc,0x1fcc,  0x1ffc,0x1ffc,  0x0,0x0};
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
	unsigned int * buffer_Lt = new unsigned int [128];
	YYCTYPE * s = (YYCTYPE *) buffer_Lt;
	unsigned int buffer_len = encode_utf8 (chars_Lt, sizeof (chars_Lt) / sizeof (unsigned int), buffer_Lt);
	/* convert 32-bit code units to YYCTYPE; reuse the same buffer */
	for (unsigned int i = 0; i < buffer_len; ++i) s[i] = buffer_Lt[i];
	if (!scan (s, s + buffer_len))
		printf("test 'Lt' failed\n");
	delete [] buffer_Lt;
	return 0;
}
