/* Exercise the production compressor with allocation/output fault injection.
 * No product fault switches or alternate compression implementation. */
#undef NDEBUG
#include "rxbin.h"
#include <assert.h>

static int allocations, releases, fail_allocation, writes, fail_write;
static void *test_malloc(size_t bytes) {
    if (++allocations == fail_allocation) return NULL;
    return malloc(bytes);
}
static void test_free(void *pointer) {
    if (pointer) releases++;
    free(pointer);
}
static int test_append(rxbin_byte_buffer *buffer, const unsigned char *data, size_t size) {
    if (++writes == fail_write) return 0;
    return rxbin_byte_buffer_append_bytes(buffer, data, size);
}
#define malloc test_malloc
#define free test_free
#define rxbin_byte_buffer_append_bytes test_append
#include "../rxbin007.c"
#undef malloc
#undef free
#undef rxbin_byte_buffer_append_bytes

static unsigned char input[20000], decoded[sizeof(input)];
static void reset(int allocation, int write) {
    allocations = releases = writes = 0;
    fail_allocation = allocation;
    fail_write = write;
}
int main(int argc, char **argv) {
    rxbin_byte_buffer output = {0};
    uint32_t seed = 0x12345678u;
    uint64_t digest = UINT64_C(1469598103934665603);
    size_t i;
    int count;
    (void)argv;
    for (i = 0; i < sizeof(input); i++) {
        seed = seed * 1664525u + 1013904223u;
        input[i] = i % 4096 < 2048 ? (unsigned char)(i % 19) : (unsigned char)(seed >> 24);
    }
    reset(0, 0);
    assert(rxbin007_lzss_compress(input, sizeof(input), &output));
    assert(rxbin007_lzss_decompress(output.data, output.size, decoded, sizeof(decoded)));
    assert(!memcmp(input, decoded, sizeof(input)));
    for (i = 0; i < output.size; i++) digest = (digest ^ output.data[i]) * UINT64_C(1099511628211);
    printf("compressed=%zu digest=%016llx workspace_allocations=%d\n", output.size,
           (unsigned long long)digest, allocations);
    assert(allocations == releases);
    count = allocations;
    free(output.data);
    if (argc > 1) return 0; /* Retain original-byte comparison before repair. */
    assert(count == 2);
    assert(digest == UINT64_C(0x3fc9210d4fbd2dba));
    assert(output.size == 12207);
    /* Empty input needs no workspace. */
    memset(&output, 0, sizeof(output)); reset(1, 1);
    assert(rxbin007_lzss_compress(input, 0, &output));
    assert(!allocations && !writes && !output.data);
    for (i = 1; i <= 2; i++) {
        memset(&output, 0, sizeof(output)); reset((int)i, 0);
        assert(!rxbin007_lzss_compress(input, sizeof(input), &output));
        assert(releases == allocations - 1 && !output.data);
    }
    /* First output byte and a later byte both exercise workspace cleanup. */
    for (i = 1; i <= 67; i += 66) {
        memset(&output, 0, sizeof(output)); reset(0, (int)i);
        assert(!rxbin007_lzss_compress(input, sizeof(input), &output));
        assert(releases == allocations && allocations == 2);
        free(output.data);
    }
    puts("PASS compression workspace allocation/output failures and round trip");
    return 0;
}
