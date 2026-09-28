/* Strict shared codec controls for each supported external file page. */
#undef NDEBUG
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "text_codec.h"

static void round_trip(const char *name) {
    const uint32_t *map = NULL;
    unsigned byte;
    assert(platform_text_codec_lookup(name, &map) == 0 && map);
    for (byte = 0; byte < 256; ++byte) {
        unsigned char external = (unsigned char)byte;
        unsigned char utf8[4], back[4];
        size_t length = 99, back_length = 99;
        int rc = crexx_text_convert(map, 1, &external, 1, utf8, sizeof(utf8), &length);
        if (!strcmp(name, "ASCII") && byte >= 128) {
            assert(rc == -1 && errno == EILSEQ && length == 0);
            continue;
        }
        assert(rc == 0 && length >= 1 && length <= 4);
        assert(crexx_text_convert(map, 0, utf8, length, back, sizeof(back), &back_length) == 0);
        assert(back_length == 1 && back[0] == external);
    }
}

static void fails(const uint32_t *map, int decode, const unsigned char *bytes,
                  size_t count, int expected_errno) {
    unsigned char output[8];
    size_t written = 99;
    errno = 0;
    assert(crexx_text_convert(map, decode, bytes, count,
                              output, sizeof(output), &written) == -1);
    assert(errno == expected_errno && written == 0);
}

int main(void) {
    const uint32_t *map, *saved;
    static const char *pages[] = {
        "ASCII", "Latin1", "Windows-1252", "IBM437", "IBM850", "IBM1047"
    };
    static const unsigned char invalid_lead[] = {0xc0, 0x80};
    static const unsigned char surrogate[] = {0xed, 0xa0, 0x80};
    static const unsigned char too_large[] = {0xf4, 0x90, 0x80, 0x80};
    static const unsigned char incomplete[] = {0xe2, 0x82};
    static const unsigned char euro[] = {0xe2, 0x82, 0xac};
    static const unsigned char emoji[] = {0xf0, 0x9f, 0x98, 0x80};
    unsigned char output[8];
    size_t written, i;
    crexx_utf8_state state = {0, 0, 0};
    uint32_t scalar = 0;

    for (i = 0; i < sizeof(pages)/sizeof(pages[0]); ++i) round_trip(pages[i]);
    assert(platform_text_codec_lookup("utf-8", &map) == 0 && !map);
    assert(platform_text_codec_lookup("US_ASCII", &map) == 0 && map);
    saved = map;
    errno = 0;
    assert(platform_text_codec_lookup("IBM-9999", &map) == -1 && errno == EINVAL && map == saved);

    assert(platform_text_codec_lookup("Windows-1252", &map) == 0);
    written = 0;
    assert(crexx_text_convert(map, 0, euro, sizeof(euro), output, sizeof(output), &written) == 0);
    assert(written == 1 && output[0] == 0x80);
    output[0] = 0x81; /* The retained CP1252 policy preserves undefined C1 positions. */
    assert(crexx_text_convert(map, 1, output, 1, output + 1, sizeof(output)-1, &written) == 0);
    assert(written == 2 && output[1] == 0xc2 && output[2] == 0x81);

    assert(platform_text_codec_lookup("IBM1047", &map) == 0);
    written = 0;
    assert(crexx_text_convert(map, 0, (const unsigned char *)"A", 1,
                              output, sizeof(output), &written) == 0);
    assert(written == 1 && output[0] == 0xc1);

    assert(platform_text_codec_lookup("ASCII", &map) == 0);
    fails(map, 1, (const unsigned char *)"\x80", 1, EILSEQ);
    fails(map, 0, euro, sizeof(euro), EILSEQ);
    fails(NULL, 1, invalid_lead, sizeof(invalid_lead), EILSEQ);
    fails(NULL, 1, surrogate, sizeof(surrogate), EILSEQ);
    fails(NULL, 1, too_large, sizeof(too_large), EILSEQ);
    fails(NULL, 1, incomplete, sizeof(incomplete), EILSEQ);
    fails(NULL, 0, incomplete, sizeof(incomplete), EILSEQ);
    written = 99;
    errno = 0;
    assert(crexx_text_convert(NULL, 1, euro, sizeof(euro), output, 2, &written) == -1);
    assert(errno == E2BIG && written == 0); /* No partial scalar. */
    assert(platform_text_codec_lookup("Latin1", &map) == 0);
    written = 99;
    errno = 0;
    assert(crexx_text_convert(map, 1, (const unsigned char *)"A\xe9", 2,
                              output, 2, &written) == -1);
    assert(errno == E2BIG && written == 1 && output[0] == 'A');

    for (i = 0; i < sizeof(emoji)-1; ++i) {
        assert(crexx_utf8_feed(&state, emoji[i], &scalar) == 0);
        assert(crexx_utf8_finish(&state) == -1 && errno == EILSEQ);
    }
    assert(crexx_utf8_feed(&state, emoji[3], &scalar) == 1 && scalar == 0x1f600);
    assert(crexx_utf8_finish(&state) == 0);
    assert(crexx_utf8_emit(0xd800, output) == -1 && errno == EILSEQ);
    assert(crexx_utf8_emit(0x110000, output) == -1 && errno == EILSEQ);
    puts("PASS strict shared text codecs across all selected pages and chunk boundaries");
    return 0;
}
