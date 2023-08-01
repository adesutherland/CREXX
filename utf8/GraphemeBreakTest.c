/* Test Suite - Auto generated file! */
#include <stdio.h>
#include <string.h>

void encodechar_utf32_8(unsigned int cp, char **buffer);
void append_to_buffer(char* to_append, char **buffer);
int lex(char *str, char* out);

int tests() {
    char in_buffer[250];
    char out_buffer[250];
    char expected_buffer[250];
    char* in;
    char* expected;
    int errors = 0;

    // TEST 1
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     1 \"÷ [0.2] SPACE (Other) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 2
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     2 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 3
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     3 \"÷ [0.2] SPACE (Other) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 4
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     4 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 5
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     5 \"÷ [0.2] SPACE (Other) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 6
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     6 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 7
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     7 \"÷ [0.2] SPACE (Other) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 8
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     8 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 9
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     9 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 10
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     10 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 11
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     11 \"÷ [0.2] SPACE (Other) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 12
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     12 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 13
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     13 \"÷ [0.2] SPACE (Other) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 14
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     14 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 15
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     15 \"÷ [0.2] SPACE (Other) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 16
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     16 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 17
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     17 \"÷ [0.2] SPACE (Other) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 18
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     18 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 19
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     19 \"÷ [0.2] SPACE (Other) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 20
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     20 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 21
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     21 \"÷ [0.2] SPACE (Other) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 22
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     22 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 23
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     23 \"÷ [0.2] SPACE (Other) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 24
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     24 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 25
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     25 \"÷ [0.2] SPACE (Other) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 26
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     26 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 27
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     27 \"÷ [0.2] SPACE (Other) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 28
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     28 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 29
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     29 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 30
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     30 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 31
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     31 \"÷ [0.2] SPACE (Other) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 32
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     32 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 33
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     33 \"÷ [0.2] SPACE (Other) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 34
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     34 \"÷ [0.2] SPACE (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 35
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     35 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 36
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     36 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 37
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     37 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 38
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     38 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 39
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     39 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) × [3.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 40
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     40 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 41
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     41 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 42
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     42 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 43
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     43 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 44
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     44 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 45
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     45 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 46
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     46 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 47
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     47 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 48
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     48 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 49
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     49 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 50
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     50 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 51
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     51 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 52
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     52 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 53
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     53 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 54
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     54 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 55
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     55 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 56
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     56 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 57
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     57 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 58
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     58 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 59
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     59 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 60
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     60 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 61
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     61 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 62
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     62 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 63
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     63 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 64
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     64 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 65
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     65 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 66
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     66 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 67
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     67 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 68
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     68 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 69
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     69 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 70
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     70 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 71
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     71 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 72
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     72 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 73
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     73 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 74
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     74 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 75
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     75 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 76
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     76 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 77
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     77 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 78
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     78 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 79
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     79 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 80
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     80 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 81
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     81 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 82
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     82 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 83
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     83 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 84
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     84 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 85
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     85 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 86
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     86 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 87
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     87 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 88
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     88 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 89
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     89 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 90
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     90 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 91
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     91 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 92
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     92 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 93
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     93 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 94
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     94 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 95
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     95 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 96
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     96 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 97
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     97 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 98
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     98 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 99
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     99 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 100
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     100 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 101
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     101 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 102
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     102 \"÷ [0.2] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 103
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     103 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 104
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     104 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 105
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     105 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 106
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     106 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 107
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     107 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 108
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     108 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 109
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     109 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 110
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     110 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 111
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     111 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 112
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     112 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 113
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     113 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 114
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     114 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 115
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     115 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 116
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     116 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 117
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     117 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 118
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     118 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 119
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     119 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 120
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     120 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 121
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     121 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 122
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     122 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 123
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     123 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 124
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     124 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 125
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     125 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 126
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     126 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 127
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     127 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 128
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     128 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 129
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     129 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 130
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     130 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 131
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     131 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 132
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     132 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 133
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     133 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 134
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     134 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 135
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     135 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 136
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0001, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     136 \"÷ [0.2] <START OF HEADING> (Control) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 137
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     137 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 138
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     138 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 139
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     139 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 140
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     140 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 141
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     141 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 142
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     142 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 143
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     143 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 144
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     144 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 145
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     145 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 146
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     146 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 147
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     147 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 148
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     148 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 149
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     149 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 150
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     150 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 151
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     151 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 152
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     152 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 153
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     153 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 154
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     154 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 155
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     155 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 156
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     156 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 157
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     157 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 158
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     158 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 159
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     159 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 160
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     160 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 161
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     161 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 162
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     162 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 163
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     163 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 164
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     164 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 165
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     165 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 166
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     166 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 167
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     167 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 168
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     168 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 169
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     169 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 170
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x034F, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x034F, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     170 \"÷ [0.2] COMBINING GRAPHEME JOINER (Extend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 171
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     171 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 172
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     172 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 173
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     173 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 174
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     174 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 175
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     175 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 176
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     176 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 177
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     177 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 178
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     178 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 179
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     179 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 180
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     180 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 181
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     181 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [12.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 182
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     182 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 183
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     183 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 184
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     184 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 185
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     185 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 186
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     186 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 187
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     187 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 188
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     188 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 189
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     189 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 190
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     190 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 191
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     191 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 192
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     192 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 193
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     193 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 194
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     194 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 195
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     195 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 196
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     196 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 197
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     197 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 198
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     198 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 199
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     199 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 200
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     200 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 201
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     201 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 202
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     202 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 203
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     203 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 204
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     204 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 205
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     205 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.2] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 206
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     206 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 207
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     207 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 208
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     208 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 209
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     209 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 210
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     210 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 211
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     211 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 212
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     212 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 213
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     213 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 214
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     214 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 215
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     215 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 216
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     216 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 217
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     217 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.2] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 218
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     218 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 219
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     219 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 220
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     220 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 221
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     221 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.2] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 222
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     222 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 223
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     223 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.2] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 224
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     224 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 225
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     225 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.2] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 226
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     226 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 227
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     227 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.2] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 228
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     228 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 229
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     229 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.2] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 230
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     230 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 231
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     231 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.2] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 232
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     232 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 233
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     233 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 234
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     234 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 235
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     235 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 236
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     236 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 237
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     237 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.2] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 238
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     238 \"÷ [0.2] ARABIC NUMBER SIGN (Prepend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 239
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     239 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 240
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     240 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 241
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     241 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 242
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     242 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 243
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     243 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 244
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     244 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 245
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     245 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 246
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     246 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 247
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     247 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 248
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     248 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 249
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     249 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 250
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     250 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 251
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     251 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 252
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     252 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 253
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     253 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 254
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     254 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 255
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     255 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 256
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     256 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 257
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     257 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 258
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     258 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 259
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     259 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 260
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     260 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 261
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     261 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 262
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     262 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 263
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     263 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 264
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     264 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 265
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     265 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 266
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     266 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 267
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     267 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 268
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     268 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 269
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     269 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 270
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     270 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 271
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     271 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 272
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0903, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     272 \"÷ [0.2] DEVANAGARI SIGN VISARGA (SpacingMark) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 273
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     273 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 274
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     274 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 275
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     275 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 276
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     276 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 277
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     277 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 278
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     278 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 279
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     279 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 280
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     280 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 281
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     281 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 282
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     282 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 283
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     283 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 284
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     284 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 285
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     285 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 286
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     286 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 287
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     287 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 288
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     288 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 289
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     289 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [6.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 290
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     290 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 291
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     291 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [6.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 292
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     292 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 293
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     293 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 294
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     294 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 295
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     295 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [6.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 296
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     296 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 297
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     297 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [6.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 298
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     298 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 299
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     299 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 300
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     300 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 301
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     301 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 302
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     302 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 303
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     303 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 304
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     304 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 305
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     305 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 306
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     306 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 307
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     307 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 308
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     308 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 309
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     309 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 310
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     310 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 311
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     311 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 312
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     312 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 313
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     313 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 314
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     314 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 315
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     315 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 316
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     316 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 317
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     317 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 318
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     318 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 319
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     319 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 320
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     320 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 321
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     321 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 322
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     322 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 323
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     323 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 324
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     324 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 325
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     325 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [7.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 326
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     326 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 327
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     327 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [7.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 328
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     328 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 329
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     329 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 330
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     330 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 331
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     331 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 332
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     332 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 333
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     333 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 334
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     334 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 335
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     335 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 336
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     336 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 337
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     337 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 338
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     338 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 339
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     339 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 340
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1160, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     340 \"÷ [0.2] HANGUL JUNGSEONG FILLER (V) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 341
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     341 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 342
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     342 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 343
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     343 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 344
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     344 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 345
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     345 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 346
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     346 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 347
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     347 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 348
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     348 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 349
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     349 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 350
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     350 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 351
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     351 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 352
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     352 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 353
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     353 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 354
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     354 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 355
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     355 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 356
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     356 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 357
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     357 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 358
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     358 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 359
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     359 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 360
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     360 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 361
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     361 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [8.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 362
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     362 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 363
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     363 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 364
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     364 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 365
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     365 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 366
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     366 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 367
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     367 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 368
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     368 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 369
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     369 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 370
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     370 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 371
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     371 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 372
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     372 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 373
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     373 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 374
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     374 \"÷ [0.2] HANGUL JONGSEONG KIYEOK (T) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 375
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     375 \"÷ [0.2] HANGUL SYLLABLE GA (LV) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 376
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     376 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 377
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     377 \"÷ [0.2] HANGUL SYLLABLE GA (LV) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 378
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     378 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 379
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     379 \"÷ [0.2] HANGUL SYLLABLE GA (LV) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 380
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     380 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 381
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     381 \"÷ [0.2] HANGUL SYLLABLE GA (LV) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 382
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     382 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 383
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     383 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 384
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     384 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 385
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     385 \"÷ [0.2] HANGUL SYLLABLE GA (LV) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 386
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     386 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 387
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     387 \"÷ [0.2] HANGUL SYLLABLE GA (LV) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 388
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     388 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 389
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     389 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 390
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     390 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 391
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     391 \"÷ [0.2] HANGUL SYLLABLE GA (LV) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 392
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     392 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 393
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     393 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [7.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 394
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     394 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 395
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     395 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [7.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 396
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     396 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 397
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     397 \"÷ [0.2] HANGUL SYLLABLE GA (LV) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 398
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     398 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 399
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     399 \"÷ [0.2] HANGUL SYLLABLE GA (LV) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 400
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     400 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 401
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     401 \"÷ [0.2] HANGUL SYLLABLE GA (LV) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 402
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     402 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 403
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     403 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 404
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     404 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 405
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     405 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 406
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     406 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 407
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     407 \"÷ [0.2] HANGUL SYLLABLE GA (LV) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 408
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     408 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 409
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     409 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 410
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     410 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 411
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     411 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 412
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     412 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 413
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     413 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 414
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     414 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 415
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     415 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 416
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     416 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 417
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     417 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 418
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     418 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 419
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     419 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 420
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     420 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 421
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     421 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 422
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     422 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 423
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     423 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 424
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     424 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 425
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     425 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 426
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     426 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 427
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     427 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 428
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     428 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 429
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     429 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [8.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 430
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     430 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 431
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     431 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 432
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     432 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 433
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     433 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 434
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     434 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 435
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     435 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 436
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     436 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 437
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     437 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 438
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     438 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 439
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     439 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 440
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     440 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 441
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     441 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 442
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     442 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 443
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     443 \"÷ [0.2] WATCH (ExtPict) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 444
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     444 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 445
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     445 \"÷ [0.2] WATCH (ExtPict) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 446
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     446 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 447
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     447 \"÷ [0.2] WATCH (ExtPict) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 448
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     448 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 449
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     449 \"÷ [0.2] WATCH (ExtPict) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 450
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     450 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 451
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     451 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 452
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     452 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 453
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     453 \"÷ [0.2] WATCH (ExtPict) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 454
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     454 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 455
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     455 \"÷ [0.2] WATCH (ExtPict) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 456
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     456 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 457
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     457 \"÷ [0.2] WATCH (ExtPict) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 458
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     458 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 459
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     459 \"÷ [0.2] WATCH (ExtPict) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 460
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     460 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 461
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     461 \"÷ [0.2] WATCH (ExtPict) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 462
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     462 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 463
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     463 \"÷ [0.2] WATCH (ExtPict) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 464
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     464 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 465
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     465 \"÷ [0.2] WATCH (ExtPict) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 466
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     466 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 467
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     467 \"÷ [0.2] WATCH (ExtPict) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 468
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     468 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 469
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     469 \"÷ [0.2] WATCH (ExtPict) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 470
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     470 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 471
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     471 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 472
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     472 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 473
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     473 \"÷ [0.2] WATCH (ExtPict) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 474
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     474 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 475
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     475 \"÷ [0.2] WATCH (ExtPict) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 476
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x231A, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     476 \"÷ [0.2] WATCH (ExtPict) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 477
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     477 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 478
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     478 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 479
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     479 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 480
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     480 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 481
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     481 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 482
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     482 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 483
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     483 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 484
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     484 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 485
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     485 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 486
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     486 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 487
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     487 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 488
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     488 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 489
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     489 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 490
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     490 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 491
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     491 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 492
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     492 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 493
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     493 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 494
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     494 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 495
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     495 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 496
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     496 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 497
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     497 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 498
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     498 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 499
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     499 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 500
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     500 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 501
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     501 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 502
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     502 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 503
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     503 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 504
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     504 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 505
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     505 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 506
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     506 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 507
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     507 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 508
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     508 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 509
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     509 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 510
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0300, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0300, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     510 \"÷ [0.2] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 511
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     511 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 512
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     512 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 513
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     513 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 514
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     514 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 515
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     515 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 516
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     516 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 517
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     517 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 518
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     518 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 519
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     519 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 520
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     520 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 521
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     521 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 522
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     522 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 523
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     523 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 524
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     524 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 525
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     525 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 526
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     526 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 527
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     527 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 528
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     528 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 529
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     529 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 530
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     530 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 531
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     531 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 532
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     532 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 533
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     533 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 534
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     534 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 535
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     535 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 536
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     536 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 537
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     537 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 538
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     538 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 539
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     539 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 540
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     540 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 541
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     541 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 542
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     542 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 543
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     543 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 544
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     544 \"÷ [0.2] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 545
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     545 \"÷ [0.2] <reserved-0378> (Other) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 546
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     546 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 547
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     547 \"÷ [0.2] <reserved-0378> (Other) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 548
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     548 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 549
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     549 \"÷ [0.2] <reserved-0378> (Other) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 550
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x000A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     550 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 551
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     551 \"÷ [0.2] <reserved-0378> (Other) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 552
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0001, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0001, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     552 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [5.0] <START OF HEADING> (Control) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 553
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     553 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 554
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x034F, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x034F, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     554 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAPHEME JOINER (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 555
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     555 \"÷ [0.2] <reserved-0378> (Other) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 556
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     556 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 557
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     557 \"÷ [0.2] <reserved-0378> (Other) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 558
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0600, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     558 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 559
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     559 \"÷ [0.2] <reserved-0378> (Other) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 560
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0903, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     560 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 561
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     561 \"÷ [0.2] <reserved-0378> (Other) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 562
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     562 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 563
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     563 \"÷ [0.2] <reserved-0378> (Other) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 564
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x1160, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1160, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     564 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JUNGSEONG FILLER (V) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 565
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     565 \"÷ [0.2] <reserved-0378> (Other) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 566
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x11A8, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     566 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL JONGSEONG KIYEOK (T) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 567
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     567 \"÷ [0.2] <reserved-0378> (Other) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 568
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC00, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     568 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GA (LV) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 569
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     569 \"÷ [0.2] <reserved-0378> (Other) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 570
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0xAC01, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     570 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] HANGUL SYLLABLE GAG (LVT) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 571
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     571 \"÷ [0.2] <reserved-0378> (Other) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 572
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x231A, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x231A, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     572 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] WATCH (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 573
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     573 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 574
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0300, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x0300, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     574 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] COMBINING GRAVE ACCENT (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 575
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     575 \"÷ [0.2] <reserved-0378> (Other) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 576
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     576 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 577
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     577 \"÷ [0.2] <reserved-0378> (Other) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 578
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0378, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0378, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0378, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     578 \"÷ [0.2] <reserved-0378> (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] <reserved-0378> (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 579
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x000D, &in);
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0061, &in);
    encodechar_utf32_8(0x000A, &in);
    encodechar_utf32_8(0x0308, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000D, &expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0061, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x000A, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     579 \"÷ [0.2] <CARRIAGE RETURN (CR)> (CR) × [3.0] <LINE FEED (LF)> (LF) ÷ [4.0] LATIN SMALL LETTER A (Other) ÷ [5.0] <LINE FEED (LF)> (LF) ÷ [4.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 580
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0061, &in);
    encodechar_utf32_8(0x0308, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0061, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     580 \"÷ [0.2] LATIN SMALL LETTER A (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 581
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0020, &in);
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0646, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0646, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     581 \"÷ [0.2] SPACE (Other) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [999.0] ARABIC LETTER NOON (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 582
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0646, &in);
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x0020, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0646, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0020, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     582 \"÷ [0.2] ARABIC LETTER NOON (Other) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [999.0] SPACE (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 583
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1100, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     583 \"÷ [0.2] HANGUL CHOSEONG KIYEOK (L) × [6.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 584
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC00, &in);
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC00, &expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     584 \"÷ [0.2] HANGUL SYLLABLE GA (LV) × [7.0] HANGUL JONGSEONG KIYEOK (T) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 585
    // Input
    in = in_buffer;
    encodechar_utf32_8(0xAC01, &in);
    encodechar_utf32_8(0x11A8, &in);
    encodechar_utf32_8(0x1100, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0xAC01, &expected);
    encodechar_utf32_8(0x11A8, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1100, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     585 \"÷ [0.2] HANGUL SYLLABLE GAG (LVT) × [8.0] HANGUL JONGSEONG KIYEOK (T) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 586
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x1F1E7, &in);
    encodechar_utf32_8(0x1F1E8, &in);
    encodechar_utf32_8(0x0062, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x1F1E7, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E8, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0062, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     586 \"÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [12.0] REGIONAL INDICATOR SYMBOL LETTER B (RI) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER C (RI) ÷ [999.0] LATIN SMALL LETTER B (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 587
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0061, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x1F1E7, &in);
    encodechar_utf32_8(0x1F1E8, &in);
    encodechar_utf32_8(0x0062, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0061, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x1F1E7, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E8, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0062, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     587 \"÷ [0.2] LATIN SMALL LETTER A (Other) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [13.0] REGIONAL INDICATOR SYMBOL LETTER B (RI) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER C (RI) ÷ [999.0] LATIN SMALL LETTER B (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 588
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0061, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x1F1E7, &in);
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x1F1E8, &in);
    encodechar_utf32_8(0x0062, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0061, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x1F1E7, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E8, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0062, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     588 \"÷ [0.2] LATIN SMALL LETTER A (Other) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [13.0] REGIONAL INDICATOR SYMBOL LETTER B (RI) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER C (RI) ÷ [999.0] LATIN SMALL LETTER B (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 589
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0061, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x1F1E7, &in);
    encodechar_utf32_8(0x1F1E8, &in);
    encodechar_utf32_8(0x0062, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0061, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E7, &expected);
    encodechar_utf32_8(0x1F1E8, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0062, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     589 \"÷ [0.2] LATIN SMALL LETTER A (Other) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER B (RI) × [13.0] REGIONAL INDICATOR SYMBOL LETTER C (RI) ÷ [999.0] LATIN SMALL LETTER B (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 590
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0061, &in);
    encodechar_utf32_8(0x1F1E6, &in);
    encodechar_utf32_8(0x1F1E7, &in);
    encodechar_utf32_8(0x1F1E8, &in);
    encodechar_utf32_8(0x1F1E9, &in);
    encodechar_utf32_8(0x0062, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0061, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E6, &expected);
    encodechar_utf32_8(0x1F1E7, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F1E8, &expected);
    encodechar_utf32_8(0x1F1E9, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0062, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     590 \"÷ [0.2] LATIN SMALL LETTER A (Other) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (RI) × [13.0] REGIONAL INDICATOR SYMBOL LETTER B (RI) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER C (RI) × [13.0] REGIONAL INDICATOR SYMBOL LETTER D (RI) ÷ [999.0] LATIN SMALL LETTER B (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 591
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0061, &in);
    encodechar_utf32_8(0x200D, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0061, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     591 \"÷ [0.2] LATIN SMALL LETTER A (Other) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 592
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0061, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x0062, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0061, &expected);
    encodechar_utf32_8(0x0308, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0062, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     592 \"÷ [0.2] LATIN SMALL LETTER A (Other) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) ÷ [999.0] LATIN SMALL LETTER B (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 593
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0061, &in);
    encodechar_utf32_8(0x0903, &in);
    encodechar_utf32_8(0x0062, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0061, &expected);
    encodechar_utf32_8(0x0903, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0062, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     593 \"÷ [0.2] LATIN SMALL LETTER A (Other) × [9.1] DEVANAGARI SIGN VISARGA (SpacingMark) ÷ [999.0] LATIN SMALL LETTER B (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 594
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0061, &in);
    encodechar_utf32_8(0x0600, &in);
    encodechar_utf32_8(0x0062, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0061, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0600, &expected);
    encodechar_utf32_8(0x0062, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     594 \"÷ [0.2] LATIN SMALL LETTER A (Other) ÷ [999.0] ARABIC NUMBER SIGN (Prepend) × [9.2] LATIN SMALL LETTER B (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 595
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F476, &in);
    encodechar_utf32_8(0x1F3FF, &in);
    encodechar_utf32_8(0x1F476, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F476, &expected);
    encodechar_utf32_8(0x1F3FF, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F476, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     595 \"÷ [0.2] BABY (ExtPict) × [9.0] EMOJI MODIFIER FITZPATRICK TYPE-6 (Extend) ÷ [999.0] BABY (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 596
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0061, &in);
    encodechar_utf32_8(0x1F3FF, &in);
    encodechar_utf32_8(0x1F476, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0061, &expected);
    encodechar_utf32_8(0x1F3FF, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F476, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     596 \"÷ [0.2] LATIN SMALL LETTER A (Other) × [9.0] EMOJI MODIFIER FITZPATRICK TYPE-6 (Extend) ÷ [999.0] BABY (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 597
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0061, &in);
    encodechar_utf32_8(0x1F3FF, &in);
    encodechar_utf32_8(0x1F476, &in);
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x1F6D1, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0061, &expected);
    encodechar_utf32_8(0x1F3FF, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F476, &expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x1F6D1, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     597 \"÷ [0.2] LATIN SMALL LETTER A (Other) × [9.0] EMOJI MODIFIER FITZPATRICK TYPE-6 (Extend) ÷ [999.0] BABY (ExtPict) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [11.0] OCTAGONAL SIGN (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 598
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F476, &in);
    encodechar_utf32_8(0x1F3FF, &in);
    encodechar_utf32_8(0x0308, &in);
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x1F476, &in);
    encodechar_utf32_8(0x1F3FF, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F476, &expected);
    encodechar_utf32_8(0x1F3FF, &expected);
    encodechar_utf32_8(0x0308, &expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x1F476, &expected);
    encodechar_utf32_8(0x1F3FF, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     598 \"÷ [0.2] BABY (ExtPict) × [9.0] EMOJI MODIFIER FITZPATRICK TYPE-6 (Extend) × [9.0] COMBINING DIAERESIS (Extend_ExtCccZwj) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [11.0] BABY (ExtPict) × [9.0] EMOJI MODIFIER FITZPATRICK TYPE-6 (Extend) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 599
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x1F6D1, &in);
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x1F6D1, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F6D1, &expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x1F6D1, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     599 \"÷ [0.2] OCTAGONAL SIGN (ExtPict) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [11.0] OCTAGONAL SIGN (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 600
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0061, &in);
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x1F6D1, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0061, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x1F6D1, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     600 \"÷ [0.2] LATIN SMALL LETTER A (Other) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [999.0] OCTAGONAL SIGN (ExtPict) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 601
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x2701, &in);
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x2701, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x2701, &expected);
    encodechar_utf32_8(0x200D, &expected);
    encodechar_utf32_8(0x2701, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     601 \"÷ [0.2] UPPER BLADE SCISSORS (Other) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) × [11.0] UPPER BLADE SCISSORS (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    // TEST 602
    // Input
    in = in_buffer;
    encodechar_utf32_8(0x0061, &in);
    encodechar_utf32_8(0x200D, &in);
    encodechar_utf32_8(0x2701, &in);
    *in = 0;
    // Expected output
    expected = expected_buffer;
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x0061, &expected);
    encodechar_utf32_8(0x200D, &expected);
    append_to_buffer("]",&expected);
    append_to_buffer("[",&expected);
    encodechar_utf32_8(0x2701, &expected);
    append_to_buffer("]",&expected);
    *expected = 0;
    lex(in_buffer, out_buffer);
    if (strcmp(out_buffer, expected_buffer) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     602 \"÷ [0.2] LATIN SMALL LETTER A (Other) × [9.0] ZERO WIDTH JOINER (ZWJ_ExtCccZwj) ÷ [999.0] UPPER BLADE SCISSORS (Other) ÷ [0.3]\"\n");
        printf("INPUT    \"%s\"\n", in_buffer);
        printf("EXPECTED \"%s\"\n", expected_buffer);
        printf("OUTPUT   \"%s\"\n", out_buffer);
        printf("\n");
    }

    printf("602 tests run\n");
    if (errors) printf("*** %d ERRORS ***\n", errors);
    return errors;
}

