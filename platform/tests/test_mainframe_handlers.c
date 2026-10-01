/* Compile the real three text-read handlers against a small value fixture.
 * This checks boundary wiring and BYTE gating without a nested VM build. */
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "platform.h"
#include "text_codec.h"

typedef size_t rxvm_string_metric;
typedef struct fixture_value {
    char *string_value;
    size_t string_length, string_chars;
    long int_value;
} fixture_value;
static char buffer[256];
static fixture_value output = {buffer, 0, 0, 0}, input;
static fixture_value *op1R = &output, *op2R = &input;
static int observed_signal;
#define RXSIGNAL_FAILURE 1
#define RXSIGNAL_UNICODE_ERROR 2
#define SET_SIGNAL_MSG(signal, message) do { observed_signal = signal; } while (0)
#define DEBUG(...)
#define VM_ADVANCE(count)
#define DISPATCH return
#ifdef NUTF8
#define RXVM_UTF8_ONLY(...)
#define RXVM_BYTE_ONLY(...) __VA_ARGS__
#else
#define RXVM_UTF8_ONLY(...) __VA_ARGS__
#define RXVM_BYTE_ONLY(...)
#endif
#define string_cache_reset(value) ((void)0)
#define clear_vm_private_flags(value) ((void)0)
#define mark_ascii_string_valid_count(value) ((void)0)
#define mark_utf8_valid_count(value) ((void)0)
static void rxvm_value_set_string_length_known(fixture_value *value, size_t length) {
    value->string_length = length;
}
static void rxvm_value_set_string_chars_known(fixture_value *value, size_t chars) {
    value->string_chars = chars;
}
static size_t rxvm_value_string_size_add_or_fail(size_t size, size_t extra) {
    assert(size + extra < sizeof(buffer)); return size + extra;
}
static void extend_string_buffer_metric(fixture_value *value, size_t length) {
    assert(length <= sizeof(buffer)); value->string_length = length;
}
static void prep_string_buffer(fixture_value *value, size_t length) {
    assert(length <= sizeof(buffer)); value->string_length = length;
}
static int validate_utf8_bytes(const char *text, size_t length, size_t *chars) {
    crexx_utf8_state state = {0, 0, 0};
    uint32_t scalar;
    size_t i;
    *chars = 0;
    for (i = 0; i < length; ++i) {
        int ready = crexx_utf8_feed(&state, (unsigned char)text[i], &scalar);
        if (ready < 0) return -1;
        if (ready) ++*chars;
    }
    return crexx_utf8_finish(&state);
}
static void refresh_utf8_flags(fixture_value *value) {
    assert(!validate_utf8_bytes(value->string_value, value->string_length, &value->string_chars));
}
static void utf8codepoint(const char *text, int *scalar) {
    crexx_utf8_state state = {0, 0, 0};
    uint32_t decoded;
    size_t i;
    for (i = 0; i < 4; ++i) {
        int ready = crexx_utf8_feed(&state, (unsigned char)text[i], &decoded);
        assert(ready >= 0);
        if (ready) { *scalar = (int)decoded; return; }
    }
    assert(0);
}
#include "mainframe_handler_selectors.h"
#undef FIXTURE_READLINE_REG
#undef FIXTURE_FREADLINE_REG_REG
#undef FIXTURE_FREADCDPT_REG_REG
#define FIXTURE_READLINE_REG(...) static void run_readline(void) { __VA_ARGS__ }
#define FIXTURE_FREADLINE_REG_REG(...) static void run_freadline(void) { __VA_ARGS__ }
#define FIXTURE_FREADCDPT_REG_REG(...) static void run_freadcdpt(void) { __VA_ARGS__ }
#define RXVM_HANDLER(name, ...) FIXTURE_##name(__VA_ARGS__)
#define RXVM_PRIVATE_HANDLER(...)
#include "../../interpreter/rxvmhandlers_control.inc"
#include "../../interpreter/rxvmhandlers_string.inc"
#include "../../interpreter/rxvmhandlers_system.inc"

static void reset_stdin(void) {
    static const unsigned char native[] = {0xc1, 0x51, 0x0a, 0x0a, 0xc2};
    FILE *file = fopen("handler-stdin.dat", "wb");
    assert(file && fwrite(native, 1, sizeof(native), file) == sizeof(native) && !fclose(file));
    assert(freopen("handler-stdin.dat", "rb", stdin));
    input.int_value = (long)stdin;
    observed_signal = 0;
}
static void expect_line(int which, const unsigned char *bytes, size_t count) {
    if (which) run_freadline(); else run_readline();
    assert(!observed_signal && output.string_length == count);
    assert(!memcmp(output.string_value, bytes, count));
}
int main(void) {
    int which, saved = dup(0);
    assert(saved >= 0);
    for (which = 0; which < 2; ++which) {
        reset_stdin();
#ifdef NUTF8
        expect_line(which, (const unsigned char *)"\xc1\x51", 2);
#else
        expect_line(which, (const unsigned char *)"A\xc3\xa9", 3);
        assert(output.string_chars == 2);
#endif
        expect_line(which, (const unsigned char *)"", 0);
#ifdef NUTF8
        expect_line(which, (const unsigned char *)"\xc2", 1);
#else
        expect_line(which, (const unsigned char *)"B", 1);
#endif
        expect_line(which, (const unsigned char *)"", 0);
    }
    reset_stdin();
    run_freadcdpt();
#ifdef NUTF8
    assert(output.int_value == 0xc1 && output.string_length == 1 && (unsigned char)buffer[0] == 0xc1);
    run_freadcdpt();
    assert(output.int_value == 0x51 && output.string_length == 1 && (unsigned char)buffer[0] == 0x51);
#else
    assert(output.int_value == 'A' && output.string_length == 1 && buffer[0] == 'A');
    run_freadcdpt();
    assert(output.int_value == 0xe9 && output.string_length == 2 && !memcmp(buffer, "\xc3\xa9", 2));
#endif
    run_freadcdpt(); assert(output.int_value == '\n');
    run_freadcdpt(); assert(output.int_value == '\n');
    run_freadcdpt();
#ifdef NUTF8
    assert(output.int_value == 0xc2);
#else
    assert(output.int_value == 'B');
#endif
    run_freadcdpt(); assert(output.int_value == -1 && !output.string_length && !observed_signal);
    assert(dup2(saved, 0) == 0); close(saved);
    return 0;
}
