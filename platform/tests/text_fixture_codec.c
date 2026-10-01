/* Small tool-interface fixture: use the product's shared codec rather than
 * duplicate a native mapping in the host orchestration script. */
#include <stdio.h>
#include <string.h>
#include "text_codec.h"
int main(int argc, char **argv) {
    const uint32_t *map;
    crexx_utf8_state state = {0, 0, 0};
    int byte, decode;
    if (argc != 2 || (strcmp(argv[1], "encode") && strcmp(argv[1], "decode"))) return 2;
    decode = !strcmp(argv[1], "decode");
    if (platform_text_codec_lookup("IBM1047", &map)) return 2;
    while ((byte = fgetc(stdin)) != EOF) {
        unsigned char bytes[4];
        uint32_t scalar;
        int n, ready;
        if (decode) {
            scalar = byte == '\n' ? '\n' : map[(unsigned char)byte];
            n = crexx_utf8_emit(scalar, bytes);
        } else {
            ready = crexx_utf8_feed(&state, (unsigned char)byte, &scalar);
            if (ready < 0) return 1;
            if (!ready) continue;
            if (scalar == '\n') { bytes[0] = '\n'; n = 1; }
            else n = crexx_text_encode(map, scalar, bytes);
        }
        if (n < 0 || fwrite(bytes, 1, (size_t)n, stdout) != (size_t)n) return 1;
    }
    return ferror(stdin) || crexx_utf8_finish(&state) || fflush(stdout) ? 1 : 0;
}
