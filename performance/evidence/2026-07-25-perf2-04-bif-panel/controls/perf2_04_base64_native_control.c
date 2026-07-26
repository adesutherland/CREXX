/* PERF2-04 fixed-valid-input native ceiling for the Base64 control.
 *
 * This is an attribution-only control, not a product implementation or API.
 * It repeats the same deterministic 1,024-byte encode/decode/checksum cell as
 * perf2_04_base64_controls.crexx. It deliberately proves no malformed-input
 * contract and therefore cannot select native ownership.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const unsigned char alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static size_t encode64(const uint8_t *input, size_t input_length,
                       unsigned char *output) {
    size_t input_offset = 0;
    size_t output_offset = 0;
    while (input_offset < input_length) {
        size_t remaining = input_length - input_offset;
        unsigned byte1 = input[input_offset];
        unsigned byte2 = remaining > 1 ? input[input_offset + 1] : 0;
        unsigned byte3 = remaining > 2 ? input[input_offset + 2] : 0;
        output[output_offset] = alphabet[byte1 / 4];
        output[output_offset + 1] = alphabet[(byte1 % 4) * 16 + byte2 / 16];
        output[output_offset + 2] = remaining > 1
            ? alphabet[(byte2 % 16) * 4 + byte3 / 64] : '=';
        output[output_offset + 3] = remaining > 2
            ? alphabet[byte3 % 64] : '=';
        input_offset += 3;
        output_offset += 4;
    }
    output[output_offset] = 0;
    return output_offset;
}

static int digit64(unsigned char codepoint) {
    if (codepoint >= 'A' && codepoint <= 'Z') return codepoint - 'A';
    if (codepoint >= 'a' && codepoint <= 'z') return codepoint - 'a' + 26;
    if (codepoint >= '0' && codepoint <= '9') return codepoint - '0' + 52;
    if (codepoint == '+') return 62;
    if (codepoint == '/') return 63;
    return 0;
}

static size_t decode64(const unsigned char *input, size_t input_length,
                       uint8_t *output) {
    size_t padding = input_length && input[input_length - 1] == '=';
    if (input_length > 1 && input[input_length - 2] == '=') padding++;
    size_t output_length = (input_length / 4) * 3 - padding;
    size_t input_offset = 0;
    size_t output_offset = 0;
    while (input_offset < input_length) {
        int digit1 = digit64(input[input_offset]);
        int digit2 = digit64(input[input_offset + 1]);
        int digit3 = input[input_offset + 2] == '='
            ? 0 : digit64(input[input_offset + 2]);
        int digit4 = input[input_offset + 3] == '='
            ? 0 : digit64(input[input_offset + 3]);
        output[output_offset++] = (uint8_t)(digit1 * 4 + digit2 / 16);
        if (output_offset < output_length + 1 && input[input_offset + 2] != '=')
            output[output_offset++] = (uint8_t)((digit2 % 16) * 16 + digit3 / 4);
        if (output_offset < output_length + 1 && input[input_offset + 3] != '=')
            output[output_offset++] = (uint8_t)((digit3 % 4) * 64 + digit4);
        input_offset += 4;
    }
    return output_length;
}

int main(int argc, char **argv) {
    long repetitions = 2500;
    uint8_t input[1024];
    uint8_t decoded[1024];
    unsigned char encoded[1369];
    size_t encoded_length = 0;
    size_t decoded_length = 0;
    unsigned long checksum = 0;

    if (argc > 2) return 2;
    if (argc == 2) {
        char *end = NULL;
        repetitions = strtol(argv[1], &end, 10);
        if (!end || *end || repetitions < 1) return 2;
    }
    for (size_t offset = 0; offset < sizeof(input); ++offset)
        input[offset] = (uint8_t)((offset * 31 + 7) % 256);

    for (long iteration = 0; iteration < repetitions; ++iteration) {
        encoded_length = encode64(input, sizeof(input), encoded);
        decoded_length = decode64(encoded, encoded_length, decoded);
    }
    if (encoded_length != 1368 || decoded_length != sizeof(input) ||
        memcmp(input, decoded, sizeof(input)) != 0) {
        fputs("FAIL: PERF2-04 native Base64 bytes\n", stderr);
        return 1;
    }
    for (size_t offset = 0; offset < decoded_length; ++offset)
        checksum += decoded[offset];
    if (checksum != 130560) {
        fputs("FAIL: PERF2-04 native Base64 checksum\n", stderr);
        return 1;
    }
    printf("PERF2-04-BASE64-NATIVE-CONTROL repetitions=%ld encoded_length=%zu checksum=%lu publication=attribution-only\n",
           repetitions, encoded_length, checksum);
    puts("PASS: PERF2-04 Base64 native control");
    return 0;
}
