/* Char Segmentation PoC */

#include <stdio.h>
#include <string.h>

/* Character Encoding Specifics */
/*!include:re2c "utfwordbreak.re" */
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

int lex_word(const char *str, char* out) {
    const char *start;
    int len = strlen(str);
    const char *YYLIMIT = str + len, *YYMARKER, *YYCTXMARKER;
    int count = 0;
    out[0] = 0;
    const char *wstart;

    /* Loop for whole string */
    for (;;) {
        start = str;
        wstart = start;

        /* Loop for word fragment */
        for (;;) {

        printf("[%.*s]", (int)(str - wstart), wstart);
        wstart = str;

/*!re2c

            AHLetter = (ALetter | Hebrew_Letter);
            MidNumLetQ = (MidNumLet | Single_Quote);
            ignore = (Extend | Format | ZWJ)*;

            CR / LF { printf("WB3a "); continue; } // WB3
            (Newline | CR | LF)  { printf("WB3a "); break; } // WB3a
            (any \ Newline \ CR \ LF) / (Newline | CR | LF) { printf("WB3b "); break; } // WB3b

            // Rule WB3c has been added to to handle when the ZWJ starts the fragment - i.e. from a WB4 rule
            ZWJ Extended_Pictographic { printf("WB3c1 "); continue; } // WB3c1
            (any \ Newline \ CR \ LF) ZWJ Extended_Pictographic { printf("WB3c2 "); continue; } // WB3c2

            WSegSpace / WSegSpace { printf("WB3d "); continue; } // WB3d

            // The WB4 rules which say ignore (Extend | Format | ZWJ) which is "tricky" for regular expressions
            // This rule skips the "ignored" after most codepoints
            (any \ Newline \ CR \ LF) / (Extend | Format | ZWJ) { printf("WB4a "); continue; }
            // These rules break (overriding the above) in the case where the "non-break" rules following start to but fail to match
            Katakana (Extend | Format | ZWJ)+ / (any \ Extend \ Format \ ZWJ \ Katakana \ ExtendNumLet) { printf("WB4b "); break; }
            AHLetter (Extend | Format | ZWJ)+ / (any \ Extend \ Format \ ZWJ \ AHLetter \ MidLetter \ MidNumLetQ \ Numeric \ ExtendNumLet) { printf("WB4c "); break; }
            Hebrew_Letter (Extend | Format | ZWJ)+ / (any \ Extend \ Format \ ZWJ \ Single_Quote \ Double_Quote \ AHLetter \ MidLetter \ MidNumLetQ \ Numeric \ ExtendNumLet) { printf("WB4d "); break; }
            Numeric (Extend | Format | ZWJ)+ / (any \ Extend \ Format \ ZWJ \ Numeric \ AHLetter \ MidNum \ MidNumLetQ \ ExtendNumLet) { printf("WB4e "); break; }
            ExtendNumLet (Extend | Format | ZWJ)+ / (any \ Extend \ Format \ ZWJ \ Numeric \ AHLetter \ Katakana \ ExtendNumLet) { printf("WB4f] "); break; }

            // Rules showing where that a break should not split a word - these are the "non-break" rules that are addressed above
            // Note we have added ignores (a lot) implementing the rest of WB4
            AHLetter ignore / AHLetter { printf("WB5 "); continue; } // WB5
            AHLetter ignore (MidLetter | MidNumLetQ) ignore / AHLetter { printf("WB6/7 "); continue; } // WB6
            Hebrew_Letter ignore / Single_Quote { printf("WB7a "); continue; } // WB7a
            Hebrew_Letter ignore Double_Quote ignore / Hebrew_Letter { printf("WB7b/c "); continue; } // WB7c
            Numeric ignore / Numeric { printf("WB8 "); continue; } // WB8
            AHLetter ignore / Numeric { printf("WB9 "); continue; } // WB9
            Numeric ignore / AHLetter { printf("WB10 "); continue; } // WB10
            Numeric ignore (MidNum | MidNumLetQ) ignore / Numeric { printf("WB11/12 "); continue; } // WB11
            Katakana ignore / Katakana { printf("WB13 "); continue; } // WB13
            (AHLetter | Numeric | Katakana | ExtendNumLet) ignore / ExtendNumLet { printf("WB13a "); continue; } // WB13a
            ExtendNumLet ignore / (AHLetter | Numeric | Katakana) { printf("WB13b "); continue; } // WB13b

            // Weirdly simple compared to other implementations - although it seems to work!
            Regional_Indicator ignore Regional_Indicator ignore { printf("WB15/16 "); break; } // WB15/16

            // Anything else
            any { printf("WB999 "); break; } // WB999

            // Invalid stuff
            * { printf("ERROR "); break; }

            // End of Input
            $   {
                   printf("[%.*s]end", (int)(str - wstart), wstart);
                    if (str != start) {
                        ++count;
                        printf("+end ");
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
