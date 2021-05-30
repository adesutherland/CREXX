/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -dei
/* re2c lesson 001_upn_calculator, calc_001, (c) M. Boerger 2006 - 2007 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int scan(char *s, int l)
{
	char *p = s;
	char *q = 0;
#define YYCTYPE         char
#define YYCURSOR        p
#define YYLIMIT         (s+l)
#define YYMARKER        q
#define YYFILL(n)
	
	for(;;)
	{

		{
			YYCTYPE yych;
			YYDEBUG(0, *YYCURSOR);
			if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
			yych = *YYCURSOR;
			switch (yych) {
			case 0x00:	goto yy2;
			case 0x4E: /* + */	goto yy6;
			case 0x60: /* - */	goto yy8;
			case 0xF0: /* 0 */	goto yy10;
			case 0xF1: /* 1 */
			case 0xF2: /* 2 */
			case 0xF3: /* 3 */
			case 0xF4: /* 4 */
			case 0xF5: /* 5 */
			case 0xF6: /* 6 */
			case 0xF7: /* 7 */
			case 0xF8: /* 8 */
			case 0xF9: /* 9 */	goto yy12;
			default:	goto yy4;
			}
yy2:
			YYDEBUG(2, *YYCURSOR);
			++YYCURSOR;
			YYDEBUG(3, *YYCURSOR);
			{ printf("EOF\n");	return 0; }
yy4:
			YYDEBUG(4, *YYCURSOR);
			++YYCURSOR;
			YYDEBUG(5, *YYCURSOR);
			{ printf("ERR\n");	return 1; }
yy6:
			YYDEBUG(6, *YYCURSOR);
			++YYCURSOR;
			YYDEBUG(7, *YYCURSOR);
			{ printf("+\n");	continue; }
yy8:
			YYDEBUG(8, *YYCURSOR);
			++YYCURSOR;
			YYDEBUG(9, *YYCURSOR);
			{ printf("-\n");	continue; }
yy10:
			YYDEBUG(10, *YYCURSOR);
			yych = *++YYCURSOR;
			switch (yych) {
			case 0xF0: /* 0 */
			case 0xF1: /* 1 */
			case 0xF2: /* 2 */
			case 0xF3: /* 3 */
			case 0xF4: /* 4 */
			case 0xF5: /* 5 */
			case 0xF6: /* 6 */
			case 0xF7: /* 7 */
			case 0xF8: /* 8 */
			case 0xF9: /* 9 */	goto yy15;
			default:	goto yy11;
			}
yy11:
			YYDEBUG(11, *YYCURSOR);
			{ printf("Num\n");	continue; }
yy12:
			YYDEBUG(12, *YYCURSOR);
			++YYCURSOR;
			if (YYLIMIT <= YYCURSOR) YYFILL(1);
			yych = *YYCURSOR;
			YYDEBUG(13, *YYCURSOR);
			switch (yych) {
			case 0xF0: /* 0 */
			case 0xF1: /* 1 */
			case 0xF2: /* 2 */
			case 0xF3: /* 3 */
			case 0xF4: /* 4 */
			case 0xF5: /* 5 */
			case 0xF6: /* 6 */
			case 0xF7: /* 7 */
			case 0xF8: /* 8 */
			case 0xF9: /* 9 */	goto yy12;
			default:	goto yy14;
			}
yy14:
			YYDEBUG(14, *YYCURSOR);
			{ printf("Num\n");	continue; }
yy15:
			YYDEBUG(15, *YYCURSOR);
			++YYCURSOR;
			if (YYLIMIT <= YYCURSOR) YYFILL(1);
			yych = *YYCURSOR;
			YYDEBUG(16, *YYCURSOR);
			switch (yych) {
			case 0xF0: /* 0 */
			case 0xF1: /* 1 */
			case 0xF2: /* 2 */
			case 0xF3: /* 3 */
			case 0xF4: /* 4 */
			case 0xF5: /* 5 */
			case 0xF6: /* 6 */
			case 0xF7: /* 7 */
			case 0xF8: /* 8 */
			case 0xF9: /* 9 */	goto yy15;
			default:	goto yy17;
			}
yy17:
			YYDEBUG(17, *YYCURSOR);
			{ printf("Oct\n");	continue; }
		}

	}
}

int main(int argc, char **argv)
{
	if (argc > 1)
	{
		return scan(argv[1], strlen(argv[1]));
	}
	else
	{
		fprintf(stderr, "%s <expr>\n", argv[0]);
		return 1;
	}
}
