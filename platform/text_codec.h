#ifndef CREXX_TEXT_CODEC_H
#define CREXX_TEXT_CODEC_H
#include <stddef.h>
#include <stdint.h>
/* Immutable process-lifetime decode table; NULL is validated UTF-8.
 * Undefined source bytes use UINT32_MAX. Selection failure leaves *map alone. */
int platform_text_codec_lookup(const char *name, const uint32_t **map);
/* Streaming UTF-8 state, initialized to all zero. A feed returns 1 and a
 * scalar when complete, 0 while incomplete, or -1/EILSEQ. Reset after error. */
typedef struct crexx_utf8_state {
    uint32_t value, minimum;
    unsigned remaining;
} crexx_utf8_state;
int crexx_utf8_feed(crexx_utf8_state *, unsigned char, uint32_t *scalar);
int crexx_utf8_finish(const crexx_utf8_state *);
/* Return encoded length 1..4, or -1/EILSEQ. These never allocate. */
int crexx_utf8_emit(uint32_t scalar, unsigned char output[4]);
int crexx_text_encode(const uint32_t *map, uint32_t scalar, unsigned char output[4]);
/* Whole-buffer strict conversion; decode != 0 means external -> UTF8.
 * E2BIG preserves *written as the complete output prefix; no partial scalar.
 * Input/output must not overlap. Conversion always reports exact byte lengths. */
int crexx_text_convert(const uint32_t *map, int decode, const unsigned char *input,
                       size_t count, unsigned char *output, size_t capacity,
                       size_t *written);
#endif
