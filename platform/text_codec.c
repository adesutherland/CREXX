/* Bootstrap text codec selection. Mapping authority is shared with rxunicode. */
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include "text_codec.h"
#include "text_codec_tables.h"
int platform_text_codec_lookup(const char *name, const uint32_t **map) {
    char key[32];
    size_t n = 0;
    if (!name || !map) { errno = EINVAL; return -1; }
    while (*name) {
        unsigned char c = (unsigned char)*name++;
        if (c == '-' || c == '_') continue;
        if (n + 1 >= sizeof(key)) { errno = EINVAL; return -1; }
        key[n++] = c >= 'a' && c <= 'z' ? (char)(c - 'a' + 'A') : (char)c;
    }
    key[n] = 0;
    if (!strcmp(key, "UTF8")) *map = NULL;
    else if (!strcmp(key, "ASCII") || !strcmp(key, "USASCII")) *map = codec_ascii;
    else if (!strcmp(key, "ISO88591") || !strcmp(key, "LATIN1")) *map = codec_latin1;
    else if (!strcmp(key, "WINDOWS1252") || !strcmp(key, "CP1252")) *map = codec_windows1252;
    else if (!strcmp(key, "IBM437") || !strcmp(key, "CP437")) *map = codec_ibm437;
    else if (!strcmp(key, "IBM850") || !strcmp(key, "CP850")) *map = codec_ibm850;
    else if (!strcmp(key, "IBM1047") || !strcmp(key, "CP1047")) *map = codec_ibm1047;
    else { errno = EINVAL; return -1; }
    return 0;
}

static int invalid_scalar(uint32_t value) {
    return value > 0x10ffffu || (value >= 0xd800u && value <= 0xdfffu);
}
int crexx_utf8_feed(crexx_utf8_state *state, unsigned char byte, uint32_t *scalar) {
    if (!state->remaining) {
        if (byte < 0x80) { *scalar = byte; return 1; }
        if (byte >= 0xc2 && byte <= 0xdf) {
            state->value = byte & 0x1f; state->remaining = 1; state->minimum = 0x80;
        } else if (byte >= 0xe0 && byte <= 0xef) {
            state->value = byte & 0x0f; state->remaining = 2; state->minimum = 0x800;
        } else if (byte >= 0xf0 && byte <= 0xf4) {
            state->value = byte & 0x07; state->remaining = 3; state->minimum = 0x10000;
        } else { errno = EILSEQ; return -1; }
        return 0;
    }
    if ((byte & 0xc0) != 0x80) { errno = EILSEQ; return -1; }
    state->value = (state->value << 6) | (byte & 0x3f);
    if (--state->remaining) return 0;
    if (state->value < state->minimum || invalid_scalar(state->value)) {
        errno = EILSEQ; return -1;
    }
    *scalar = state->value;
    return 1;
}
int crexx_utf8_finish(const crexx_utf8_state *state) {
    if (state->remaining) { errno = EILSEQ; return -1; }
    return 0;
}
int crexx_utf8_emit(uint32_t scalar, unsigned char output[4]) {
    if (invalid_scalar(scalar)) { errno = EILSEQ; return -1; }
    if (scalar < 0x80) { output[0] = (unsigned char)scalar; return 1; }
    if (scalar < 0x800) {
        output[0] = (unsigned char)(0xc0 | (scalar >> 6));
        output[1] = (unsigned char)(0x80 | (scalar & 0x3f)); return 2;
    }
    if (scalar < 0x10000) {
        output[0] = (unsigned char)(0xe0 | (scalar >> 12));
        output[1] = (unsigned char)(0x80 | ((scalar >> 6) & 0x3f));
        output[2] = (unsigned char)(0x80 | (scalar & 0x3f)); return 3;
    }
    output[0] = (unsigned char)(0xf0 | (scalar >> 18));
    output[1] = (unsigned char)(0x80 | ((scalar >> 12) & 0x3f));
    output[2] = (unsigned char)(0x80 | ((scalar >> 6) & 0x3f));
    output[3] = (unsigned char)(0x80 | (scalar & 0x3f)); return 4;
}
int crexx_text_encode(const uint32_t *map, uint32_t scalar, unsigned char output[4]) {
    unsigned i;
    if (!map) return crexx_utf8_emit(scalar, output);
    if (!invalid_scalar(scalar)) for (i = 0; i < 256; ++i) {
        if (map[i] == scalar) { output[0] = (unsigned char)i; return 1; }
    }
    errno = EILSEQ;
    return -1;
}
int crexx_text_convert(const uint32_t *map, int decode, const unsigned char *input,
                       size_t count, unsigned char *output, size_t capacity,
                       size_t *written) {
    crexx_utf8_state state = {0, 0, 0};
    size_t i;
    *written = 0;
    for (i = 0; i < count; ++i) {
        uint32_t scalar;
        unsigned char bytes[4];
        int status, length;
        if (decode && map) { scalar = map[input[i]]; status = 1; }
        else status = crexx_utf8_feed(&state, input[i], &scalar);
        if (status < 0) return -1;
        if (!status) continue;
        length = decode ? crexx_utf8_emit(scalar, bytes) : crexx_text_encode(map, scalar, bytes);
        if (length < 0) return -1;
        if ((size_t)length > capacity - *written) { errno = E2BIG; return -1; }
        memcpy(output + *written, bytes, (size_t)length);
        *written += (size_t)length;
    }
    return crexx_utf8_finish(&state);
}
