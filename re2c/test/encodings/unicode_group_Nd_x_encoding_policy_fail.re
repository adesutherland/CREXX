// re2c $INPUT -o $OUTPUT -x --encoding-policy fail
#include <stdio.h>
#include "utf16.h"
#define YYCTYPE unsigned short
bool scan(const YYCTYPE * start, const YYCTYPE * const limit)
{
	__attribute__((unused)) const YYCTYPE * YYMARKER; // silence compiler warnings when YYMARKER is not used
#	define YYCURSOR start
Nd:
	/*!re2c
		re2c:yyfill:enable = 0;
		Nd = [\x30-\x39\u0660-\u0669\u06f0-\u06f9\u07c0-\u07c9\u0966-\u096f\u09e6-\u09ef\u0a66-\u0a6f\u0ae6-\u0aef\u0b66-\u0b6f\u0be6-\u0bef\u0c66-\u0c6f\u0ce6-\u0cef\u0d66-\u0d6f\u0de6-\u0def\u0e50-\u0e59\u0ed0-\u0ed9\u0f20-\u0f29\u1040-\u1049\u1090-\u1099\u17e0-\u17e9\u1810-\u1819\u1946-\u194f\u19d0-\u19d9\u1a80-\u1a89\u1a90-\u1a99\u1b50-\u1b59\u1bb0-\u1bb9\u1c40-\u1c49\u1c50-\u1c59\ua620-\ua629\ua8d0-\ua8d9\ua900-\ua909\ua9d0-\ua9d9\ua9f0-\ua9f9\uaa50-\uaa59\uabf0-\uabf9\uff10-\uff19\U000104a0-\U000104a9\U00011066-\U0001106f\U000110f0-\U000110f9\U00011136-\U0001113f\U000111d0-\U000111d9\U000112f0-\U000112f9\U000114d0-\U000114d9\U00011650-\U00011659\U000116c0-\U000116c9\U000118e0-\U000118e9\U00016a60-\U00016a69\U00016b50-\U00016b59\U0001d7ce-\U0001d7ff];
		Nd { goto Nd; }
		* { return YYCURSOR == limit; }
	*/
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
