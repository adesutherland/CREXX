/* Quoted operands reserve NUL before and after the ordinary 5000-byte buffer. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxdadism.h"

static int boundary(size_t encoded, int escaped) {
    unsigned char guarded[5002];
    char *source;
    char *expected;
    char *full;
    size_t length = encoded - (escaped ? 1u : 0u);
    size_t copied = encoded + 2u < 4999u ? encoded + 2u : 4999u;
    size_t needed;
    int failed = 0;
    source = (char *)malloc(length + 1u);
    expected = (char *)malloc(encoded + 3u);
    full = (char *)malloc(encoded + 3u);
    if (!source || !expected || !full) {
        free(source); free(expected); free(full); return 1;
    }
    memset(source, 'x', length); source[length] = 0;
    expected[0] = '"'; memset(expected + 1u, 'x', encoded);
    if (escaped) {
        source[length - 1u] = '\n';
        expected[encoded - 1u] = '\\'; expected[encoded] = 'n';
    }
    expected[encoded + 1u] = '"'; expected[encoded + 2u] = 0;
    memset(guarded, 0xa5, sizeof(guarded));
    needed = rxda_quote_text((char *)guarded + 1u, 5000u, source, length);
    if (needed != encoded + 2u || guarded[0] != 0xa5 || guarded[5001] != 0xa5 ||
        guarded[1u + copied] != 0 || memcmp(guarded + 1u, expected, copied)) failed = 1;
    if (rxda_quote_text(NULL, 0u, source, length) != needed) failed = 1;
    memset(guarded, 0xa5, sizeof(guarded));
    if (rxda_quote_text((char *)guarded + 1u, 1u, source, length) != needed ||
        guarded[0] != 0xa5 || guarded[1] != 0 || guarded[2] != 0xa5) failed = 1;
    if (rxda_quote_text(full, encoded + 3u, source, length) != needed ||
        memcmp(full, expected, encoded + 3u)) failed = 1;
    if (failed) fprintf(stderr, "quoted boundary failed: encoded=%lu escaped=%d\n",
                         (unsigned long)encoded, escaped);
    free(source); free(expected); free(full);
    return failed;
}

int main(void) {
    size_t encoded;
    int failures = 0;
    for (encoded = 4997u; encoded <= 4999u; ++encoded) {
        failures += boundary(encoded, 0);
        failures += boundary(encoded, 1);
    }
    printf("INLINE_QUOTE_BOUNDARY|failures=%d\n", failures);
    return failures ? 1 : 0;
}
