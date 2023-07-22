// re2c $INPUT -o $OUTPUT -ci
#include <stdint.h>
#include <limits.h>
#include <assert.h>

static const uint64_t ERROR = UINT64_MAX;
/*!conditions:re2c*/

template<int BASE> static void add(uint64_t &u, char d) {
    u = u * BASE + d;
    if (u > UINT32_MAX) u = ERROR;
}

static uint64_t parse_u32(const char *s) {
    const char *YYCURSOR = s, *YYMARKER;
    int c = yycinit;
    uint64_t u = 0;

    /*!re2c
        re2c:api:style = free-form;
        re2c:define:YYCTYPE        = char;
        re2c:define:YYGETCONDITION = "c";
        re2c:define:YYSETCONDITION = "c = @@;";
        re2c:yyfill:enable = 0;

        <*> * { return ERROR; }

        <init> '0b' / [01]        :=> bin
        <init> "0"                :=> oct
        <init> "" / [1-9]         :=> dec
        <init> '0x' / [0-9a-fA-F] :=> hex

        <bin, oct, dec, hex> "\x00" { return u; }

        <bin> [01]  { add<2>(u,  YYCURSOR[-1] - '0');      goto yyc_bin; }
        <oct> [0-7] { add<8>(u,  YYCURSOR[-1] - '0');      goto yyc_oct; }
        <dec> [0-9] { add<10>(u, YYCURSOR[-1] - '0');      goto yyc_dec; }
        <hex> [0-9] { add<16>(u, YYCURSOR[-1] - '0');      goto yyc_hex; }
        <hex> [a-f] { add<16>(u, YYCURSOR[-1] - 'a' + 10); goto yyc_hex; }
        <hex> [A-F] { add<16>(u, YYCURSOR[-1] - 'A' + 10); goto yyc_hex; }
    */
}

int main() {
    assert(parse_u32("") == ERROR);
    assert(parse_u32("1234567890") == 1234567890);
    assert(parse_u32("0b1101") == 13);
    assert(parse_u32("0x7Fe") == 2046);
    assert(parse_u32("0644") == 420);
    assert(parse_u32("9999999999") == ERROR);
    return 0;
}
