/* Native libc contract simulation: IBM1047 bytes plus ASCII LF records.
 * This proves cREXX's boundaries on the host, not a mainframe guest. */
#undef NDEBUG
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "platform.h"

static int native_opens;
static int variadic_output(FILE *file, const char *format, ...) {
    va_list args;
    int result;
    va_start(args, format);
    result = vfprintf(file, format, args);
    va_end(args);
    return result;
}
FILE *crexx_cms_text_open(const char *path, const char *mode) {
    ++native_opens;
    return fopen(path, mode);
}
int crexx_cms_text_encoding(const char *encoding) {
    assert(!strcmp(encoding, "IBM1047"));
    return 0;
}
static void put_raw(const char *path, const unsigned char *bytes, size_t count) {
    FILE *file = fopen(path, "wb");
    assert(file && fwrite(bytes, 1, count, file) == count && !fclose(file));
}
static void expect_raw(const char *path, const unsigned char *expected, size_t count) {
    unsigned char bytes[1024];
    FILE *file = fopen(path, "rb");
    size_t size;
    assert(file);
    size = fread(bytes, 1, sizeof(bytes), file);
    assert(!ferror(file) && !fclose(file));
    assert(size == count && !memcmp(bytes, expected, count));
}
static void text_files(void) {
    static const unsigned char native[] = {0xc1, 0x51, 0x0a, 0x0a, 0xc2};
    static const unsigned char utf8[] = "A\xc3\xa9\n\nB";
    unsigned char bytes[256];
    unsigned i;
    FILE *file, *held;
    int calls;
    file = tmpfile();
    assert(file && variadic_output(file, "%s%d", "A", 2) == 2);
    rewind(file);
    assert(platform_text_getc(file) == 'A' && platform_text_getc(file) == '2');
    assert(platform_text_getc(file) == EOF && !fclose(file));
    for (i = 0; i < sizeof(bytes); ++i) bytes[i] = (unsigned char)i;
    put_raw("native.dat", native, sizeof(native));
    assert(!platform_text_encoding("IBM1047"));
    held = platform_fopen("native.dat", "r");
    assert(held);
    assert(!platform_text_encoding("UTF8"));
    assert(fread(bytes, 1, sizeof(bytes), held) == sizeof(utf8)-1);
    assert(!memcmp(bytes, utf8, sizeof(utf8)-1) && !ferror(held) && !fclose(held));
    assert(!platform_text_encoding("IBM1047"));
    file = platform_fopen("native-out.dat", "w");
    assert(file && fprintf(file, "%s", utf8) == (int)sizeof(utf8)-1 && !fclose(file));
    expect_raw("native-out.dat", native, sizeof(native));
    file = platform_fopen("native-out.dat", "a");
    assert(file && fputs("A", file) >= 0 && !fclose(file));
    memcpy(bytes, native, sizeof(native)); bytes[sizeof(native)] = 0xc1;
    expect_raw("native-out.dat", bytes, sizeof(native)+1);
    errno = 0;
    calls = native_opens;
    assert(!platform_fopen("native-out.dat", "w+") && errno == ENOTSUP);
    assert(calls == native_opens);
    expect_raw("native-out.dat", bytes, sizeof(native)+1);

    assert(!platform_text_encoding("UTF8"));
    file = platform_fopen("exchange.dat", "w");
    assert(file && fputs("A", file) >= 0 && !fclose(file));
    file = platform_fopen("exchange.dat", "a");
    assert(file && fputs("\xc3\xa9\n", file) >= 0 && !fclose(file));
    expect_raw("exchange.dat", (const unsigned char *)"A\xc3\xa9\n", 4);
    assert(calls == native_opens);
    assert(!platform_text_encoding("Windows1252"));
    file = platform_fopen("exchange.dat", "a");
    assert(file && fputs("\xe2\x82\xac", file) >= 0 && !fclose(file));
    expect_raw("exchange.dat", (const unsigned char *)"A\xc3\xa9\n\x80", 5);
    assert(calls == native_opens);

    /* Both binary modes bypass the currently selected external text codec. */
    for (i = 0; i < sizeof(bytes); ++i) bytes[i] = (unsigned char)i;
    file = platform_fopen("binary.dat", "wb");
    assert(file && fwrite(bytes, 1, sizeof(bytes), file) == sizeof(bytes) && !fclose(file));
    expect_raw("binary.dat", bytes, sizeof(bytes));
    file = platform_fopen("binary.dat", "rb");
    assert(file);
    for (i = 0; i < sizeof(bytes); ++i) assert(fgetc(file) == (int)i);
    assert(fgetc(file) == EOF && !ferror(file) && !fclose(file));
    assert(calls == native_opens);

    /* A malformed or unmappable text stream reports errors instead of
     * silently substituting or yielding success at close. */
    assert(!platform_text_encoding("UTF8"));
    put_raw("bad.dat", (const unsigned char *)"\xff", 1);
    file = platform_fopen("bad.dat", "r");
    assert(file && fgetc(file) == EOF && ferror(file));
    errno = 0; assert(fclose(file) == EOF && errno == EILSEQ);
    file = platform_fopen("bad-out.dat", "w");
    assert(file); setvbuf(file, NULL, _IONBF, 0);
    (void)fwrite("\xe2\x82", 1, 2, file);
    errno = 0; assert(fclose(file) == EOF && errno == EILSEQ);
    assert(!platform_text_encoding("IBM1047"));
    file = platform_fopen("bad-out.dat", "w");
    assert(file); setvbuf(file, NULL, _IONBF, 0);
    (void)fwrite("\xe2\x82\xac", 1, 3, file);
    errno = 0; assert(fclose(file) == EOF && errno == EILSEQ);
}
static void console(void) {
    static const unsigned char input[] = {0xff, 0x00, 0xc1, 0x51, 0x0a, 0x0a, 0xc2};
    static const unsigned char text[] = "A\xc3\xa9\n\nB";
    unsigned char expected[512];
    char long_text[301];
    size_t i, count = 0;
    int saved_in = dup(0), saved_out = dup(1), saved_err = dup(2);
    assert(saved_in >= 0 && saved_out >= 0 && saved_err >= 0);
    put_raw("stdin.dat", input, sizeof(input));
    assert(freopen("stdin.dat", "rb", stdin));
    assert(freopen("stdout.dat", "wb", stdout));
    assert(freopen("stderr.dat", "wb", stderr));
    assert(!platform_text_encoding("UTF8"));
    assert(fgetc(stdin) == 0xff && fgetc(stdin) == 0x00);
    for (i = 0; i < sizeof(text)-1; ++i) assert(platform_text_getc(stdin) == text[i]);
    assert(platform_text_getc(stdin) == EOF && !ferror(stdin));
    assert(printf("%s", "A\xc3\xa9\n") == 4);
    expected[count++] = 0xc1; expected[count++] = 0x51; expected[count++] = 0x0a;
    assert(fputs("B", stdout) >= 0); expected[count++] = 0xc2;
    assert(puts("") >= 0); expected[count++] = 0x0a;
    assert(platform_text_putc('{', stdout) == '{'); expected[count++] = 0xc0;
    memset(long_text, 'A', sizeof(long_text)-1); long_text[300] = 0;
    assert(printf("%s", long_text) == 300);
    memset(expected + count, 0xc1, 300); count += 300;
    /* Raw character/byte writes remain byte operations. */
    assert(fputc(0x81, stdout) == 0x81); expected[count++] = 0x81;
    assert(fwrite(input, 1, 2, stdout) == 2);
    expected[count++] = 0xff; expected[count++] = 0x00;
    assert(!platform_console_text_write(stdout, NULL, 0));
    errno = 0;
    assert(fprintf(stdout, "%s", "\xe2\x82\xac") < 0 && errno == EILSEQ);
    assert(fprintf(stderr, "%s", "B\xc3\xa9\n") == 4);
    assert(variadic_output(stderr, "%s", "A\n") == 2);
    assert(!fflush(stdout) && !fflush(stderr));
    assert(dup2(saved_in, 0) == 0 && dup2(saved_out, 1) == 1 && dup2(saved_err, 2) == 2);
    close(saved_in); close(saved_out); close(saved_err);
    clearerr(stdin);
    expect_raw("stdout.dat", expected, count);
    expect_raw("stderr.dat", (const unsigned char *)"\xc2\x51\x0a\xc1\x0a", 5);
}
int main(void) {
    text_files();
    console();
    return 0;
}
