// re2c $INPUT -o $OUTPUT -8 --case-ranges -i
//
// Simplified "Unicode Identifier and Pattern Syntax"
// (see https://unicode.org/reports/tr31)

#include <assert.h>
#include <stdint.h>

/*!include:re2c "unicode_categories.re" */

static int lex(const char *YYCURSOR)
{
    const char *YYMARKER;
    /*!re2c
    re2c:define:YYCTYPE = 'unsigned char';
    re2c:yyfill:enable  = 0;

    id_start    = L | Nl | [$_];
    id_continue = id_start | Mn | Mc | Nd | Pc | [\u200D\u05F3];
    identifier  = id_start id_continue*;

    identifier { return 0; }
    *          { return 1; }
    */
}

int main()
{
    assert(lex("_Ыдентификатор") == 0);
    return 0;
}
