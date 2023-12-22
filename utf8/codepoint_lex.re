/* Char Segmentation PoC */

#include <stdio.h>
#include <string.h>

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

int lex_codepoint(const char *str, char* out) {
    const char *start;
    int len = strlen(str);
    const char *YYLIMIT = str + len, *YYMARKER;
    int count = 0 ;
    out[0] = 0;

    for (;;) {
        start = str;
/*!re2c
        any { ++count; sprintf(out + strlen(out), "[%.*s]", (int)(str - start), start); continue; }
        $ { return count; }
        *   { sprintf(out + strlen(out), " ERROR"); return -1; }
*/
    }
}