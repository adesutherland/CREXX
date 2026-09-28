#undef NDEBUG
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
static unsigned char captured[4096];
static size_t captured_count;
static int calls, mode;
static ssize_t panic_write(int fd, const void *bytes, size_t count) {
    assert(fd == 2);
    ++calls;
    if (mode == 1) { errno = EIO; return -1; }
    if (mode == 2) return 0;
    if (calls == 1) { errno = EINTR; return -1; }
    if (count > 7) count = 7;
    assert(captured_count + count < sizeof(captured));
    memcpy(captured + captured_count, bytes, count);
    captured_count += count;
    return (ssize_t)count;
}
#define write panic_write
#include "../oom.c"
#undef write

static void report(const char *detail) {
    captured_count = 0; calls = 0; errno = ENOMEM;
    rx_report_out_of_memory("test allocation", (size_t)-2, detail, "unit.c", 41, "check");
    assert(errno == ENOMEM);
    captured[captured_count] = 0;
}
int main(void) {
    char detail[4096];
    crexx_utf8_state state = {0, 0, 0};
    uint32_t scalar;
    size_t i;
    report("non-ASCII \xc3\xa9 \xf0\x9f\x98\x80");
    assert(strstr((char *)captured, "PANIC: Out of memory\n"));
    assert(strstr((char *)captured, "unit.c:41"));
    assert(strstr((char *)captured, "non-ASCII \xc3\xa9 \xf0\x9f\x98\x80"));
    /* Shift scalar alignment through every possible truncation boundary. */
    for (i = 0; i < 4; ++i) {
        size_t j;
        memset(detail, 'x', i);
        for (j = i; j + 4 < sizeof(detail); j += 4) memcpy(detail+j, "\xf0\x9f\x98\x80", 4);
        detail[j] = 0;
        report(detail);
        memset(&state, 0, sizeof(state));
        assert(captured_count <= 2048 && captured[captured_count-1] == '\n');
        assert(strstr((char *)captured, "PANIC: Out of memory\n"));
        for (j = 0; j < captured_count; ++j) assert(crexx_utf8_feed(&state, captured[j], &scalar) >= 0);
        assert(crexx_utf8_finish(&state) == 0);
    }
    report("invalid \xff" "detail");
    assert(strstr((char *)captured, "invalid ?detail"));
    mode = 1; report("write failure"); assert(calls == 1 && !captured_count);
    mode = 2; report("no progress"); assert(calls == 1 && !captured_count);
    return 0;
}
