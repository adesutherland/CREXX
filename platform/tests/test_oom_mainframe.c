/* Allocation-free native diagnostics retain LF and preserve the caller errno. */
#undef NDEBUG
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
static unsigned char captured[4096];
static size_t captured_count;
static int calls;
static ssize_t panic_write(int fd, const void *bytes, size_t count) {
    assert(fd == 2);
    if (!calls++) { errno = EINTR; return -1; }
    if (count > 7) count = 7;
    assert(captured_count + count <= sizeof(captured));
    memcpy(captured + captured_count, bytes, count); captured_count += count;
    return (ssize_t)count;
}
#define write panic_write
#include "../oom.c"
#undef write
int main(void) {
    const uint32_t *map;
    char decoded[8192];
    size_t i, used = 0;
    errno = ENOMEM;
    rx_report_out_of_memory("A\xc3\xa9", 17, "\xe2\x82\xac", "unit.c", 41, "check");
    assert(errno == ENOMEM && calls > 1 && captured_count <= 2048);
    assert(captured[captured_count-1] == '\n');
    assert(!platform_text_codec_lookup("IBM1047", &map));
    for (i = 0; i < captured_count; ++i) {
        unsigned char bytes[4];
        int count = crexx_utf8_emit(captured[i] == '\n' ? '\n' : map[captured[i]], bytes);
        assert(count > 0 && used + (size_t)count < sizeof(decoded));
        memcpy(decoded + used, bytes, (size_t)count); used += (size_t)count;
    }
    decoded[used] = 0;
    assert(strstr(decoded, "PANIC: Out of memory\n"));
    assert(strstr(decoded, "allocation: A\xc3\xa9\n"));
    assert(strstr(decoded, "requested bytes: 17\n"));
    assert(strstr(decoded, "detail: ?\n"));
    assert(strstr(decoded, "unit.c:41 (check)\n  errno: "));
    return 0;
}
