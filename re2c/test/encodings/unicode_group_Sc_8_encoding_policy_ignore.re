// re2c $INPUT -o $OUTPUT -8 --encoding-policy ignore
#include <stdio.h>
#include "utf8.h"
#define YYCTYPE unsigned char
bool scan(const YYCTYPE * start, const YYCTYPE * const limit)
{
	__attribute__((unused)) const YYCTYPE * YYMARKER; // silence compiler warnings when YYMARKER is not used
#	define YYCURSOR start
Sc:
	/*!re2c
		re2c:yyfill:enable = 0;
		Sc = [\x24-\x24\xa2-\xa5\u058f-\u058f\u060b-\u060b\u09f2-\u09f3\u09fb-\u09fb\u0af1-\u0af1\u0bf9-\u0bf9\u0e3f-\u0e3f\u17db-\u17db\u20a0-\u20bd\ua838-\ua838\ufdfc-\ufdfc\ufe69-\ufe69\uff04-\uff04\uffe0-\uffe1\uffe5-\uffe6];
		Sc { goto Sc; }
		* { return YYCURSOR == limit; }
	*/
}
static const unsigned int chars_Sc [] = {0x24,0x24,  0xa2,0xa5,  0x58f,0x58f,  0x60b,0x60b,  0x9f2,0x9f3,  0x9fb,0x9fb,  0xaf1,0xaf1,  0xbf9,0xbf9,  0xe3f,0xe3f,  0x17db,0x17db,  0x20a0,0x20bd,  0xa838,0xa838,  0xfdfc,0xfdfc,  0xfe69,0xfe69,  0xff04,0xff04,  0xffe0,0xffe1,  0xffe5,0xffe6,  0x0,0x0};
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
	unsigned int * buffer_Sc = new unsigned int [212];
	YYCTYPE * s = (YYCTYPE *) buffer_Sc;
	unsigned int buffer_len = encode_utf8 (chars_Sc, sizeof (chars_Sc) / sizeof (unsigned int), buffer_Sc);
	/* convert 32-bit code units to YYCTYPE; reuse the same buffer */
	for (unsigned int i = 0; i < buffer_len; ++i) s[i] = buffer_Sc[i];
	if (!scan (s, s + buffer_len))
		printf("test 'Sc' failed\n");
	delete [] buffer_Sc;
	return 0;
}
