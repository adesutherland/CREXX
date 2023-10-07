/* Char Segmentation PoC */

#include <stdio.h>
#include <string.h>

/* Character Encoding Specifics */
/*!include:re2c "utfcharbreak.re" */
/*!include:re2c "emoji-data.re" */

/*!re2c
   any = [^];
*/

/*!re2c
    re2c:define:YYCTYPE = 'unsigned char';
    re2c:yyfill:enable = 0;
    re2c:define:YYCURSOR  = str;
    re2c:encoding:utf8 = 1;
    re2c:eof = 0;
*/

int lex_grapheme(const char *str, char* out) {
    const char *start;
    int len = strlen(str);
    const char *YYLIMIT = str + len, *YYMARKER, *YYCTXMARKER;
    int count = 0;
    out[0] = 0;

    /* Loop for whole string */
    for (;;) {
        start = str;

        /* Loop for Grapheme fragment */
        for (;;) {

/*!re2c

            CR / LF { printf("GB3 "); continue; } // GB3

            (Control | CR | LF) { printf("GB4 "); break; } // GB4

            (any \ Control \ CR \ LF) / (Control | CR | LF) { printf("GB5 "); break; } // GB5

            L / (L | V | LV | LVT) { printf("GB6 "); continue; } // GB6

            (LV | V) / (V | T) { printf("GB7 "); continue; } // GB7

            (LVT | T) / T { printf("GB8 "); continue; } // GB8

            (any \ Control \ CR \ LF) / (Extend | ZWJ | SpacingMark) { printf("GB9/9a "); continue; } // GB9 & GB9a

            Prepend { printf("GB9b "); continue; } // GB9b

            Extended_Pictographic Extend* ZWJ / Extended_Pictographic { printf("GB11 "); continue; } // GB 11

            Regional_Indicator Regional_Indicator / (Extend | ZWJ | SpacingMark) { printf("GB12/13-GB9/9a "); continue; } // GB12 followed by GB9
            Regional_Indicator Regional_Indicator { printf("GB12/13 "); break; } // GB12

            // Anything else
            any { printf("GB999 "); break; } // GB 999

            // Invalid stuff
            * { printf("ERROR "); break; }

            // End of Input
            $   {
                    if (str != start) {
                        ++count;
                        printf("+ ");
                        sprintf(out + strlen(out), "[%.*s]", (int)(str - start), start);
                    }
                    printf("\n");
                    return count;
                }
*/
        }

        ++count;
        printf("+ ");
        sprintf(out + strlen(out), "[%.*s]", (int)(str - start), start);

    }
}
