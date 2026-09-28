/* Host-only raw backend with short I/O and record/error controls. */
#undef NDEBUG
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "platform_config.h"
#include "native_raw.h"
#include "platform_native.h"
#include "text_codec.h"
#if defined(CREXX_PLATFORM_CMS)
#define RAW(name) lab_cms_raw_##name
#else
#define RAW(name) lab_tso_raw_##name
#endif
int getEnvVal(char **value, char *name, size_t name_length);

static unsigned opened_flags;
static int standard_descriptor;
static unsigned char opened_name[256], output[256], pending[32], input[32];
static size_t opened_length, output_length, pending_length, input_length, input_position;
static unsigned char panic_output[4096];
static size_t panic_length;
static int active, writes, records, flush_error, close_error, read_error, write_error;
static int input_record, empty_record;
static int environment_unavailable, argument_bad_byte;
static size_t record_sizes[8];
#if defined(CREXX_PLATFORM_TSO) || defined(CREXX_PLATFORM_CMS)
static int directory_active, directory_index, directory_fault;
#endif

static void reset_mock(void) {
    opened_flags = opened_length = output_length = pending_length = input_length = input_position = 0;
    panic_length = 0; active = writes = records = flush_error = close_error = 0;
    read_error = write_error = input_record = empty_record = 0;
    standard_descriptor = -1;
    environment_unavailable = argument_bad_byte = 0;
    memset(record_sizes, 0, sizeof(record_sizes));
#if defined(CREXX_PLATFORM_TSO) || defined(CREXX_PLATFORM_CMS)
    directory_active = directory_index = directory_fault = 0;
#endif
}
lab_raw_file *RAW(open)(const unsigned char *name, size_t length, unsigned flags, size_t *capacity) {
    assert(!active && length <= sizeof(opened_name));
    memcpy(opened_name, name, length); opened_length = length; opened_flags = flags;
    *capacity = flags & LAB_RAW_RECORDS ? 4 : 0;
    active = 1;
    return (lab_raw_file *)&active;
}
lab_raw_file *RAW(standard)(int descriptor, unsigned flags, size_t *capacity) {
    assert(!active && descriptor >= 0 && descriptor <= 2 && (flags & LAB_RAW_RECORDS));
    standard_descriptor = descriptor;
    opened_flags = flags;
    *capacity = 4;
    active = 1;
    return (lab_raw_file *)&active;
}
int RAW(read)(lab_raw_file *file, unsigned char *bytes, size_t capacity,
              size_t *count, int *end, int *eof) {
    assert(file == (lab_raw_file *)&active && active && capacity);
    *count = 0; *end = 0; *eof = 0;
    if (read_error) { errno = EIO; return -1; }
    if (empty_record) { empty_record = 0; *end = 1; return 0; }
    if (input_position == input_length) { *eof = 1; return 0; }
    bytes[0] = input[input_position++]; *count = 1;
    if (input_record && input_position == input_length) { *end = 1; empty_record = 1; }
    return 0;
}
ptrdiff_t RAW(write)(lab_raw_file *file, const unsigned char *bytes, size_t count, int end) {
    size_t take = count > 2 ? 2 : count;
    assert(file == (lab_raw_file *)&active && active);
    ++writes;
    if (write_error) { errno = EIO; return -1; }
    if (opened_flags & LAB_RAW_RECORDS) {
        if (pending_length + take > 4) { errno = EOVERFLOW; return -1; }
        memcpy(pending + pending_length, bytes, take); pending_length += take;
        if (end && take == count) {
            assert(records < 8 && output_length + pending_length <= sizeof(output));
            memcpy(output + output_length, pending, pending_length);
            record_sizes[records++] = pending_length;
            output_length += pending_length; pending_length = 0;
        }
    } else {
        assert(!end && output_length + take <= sizeof(output));
        memcpy(output + output_length, bytes, take); output_length += take;
    }
    return (ptrdiff_t)take;
}
int RAW(flush)(lab_raw_file *file) {
    assert(file == (lab_raw_file *)&active && active);
    if (flush_error) { errno = EIO; return -1; }
    return 0;
}
int RAW(close)(lab_raw_file *file) {
    assert(file == (lab_raw_file *)&active && active);
    active = 0;
    if (pending_length) { pending_length = 0; errno = EINVAL; return -1; }
    if (close_error) { errno = EIO; return -1; }
    return 0;
}
ptrdiff_t RAW(stderr)(const unsigned char *bytes, size_t count, int end) {
    size_t take = count > 3 ? 3 : count;
    assert(panic_length + take < sizeof(panic_output));
    memcpy(panic_output + panic_length, bytes, take); panic_length += take;
    (void)end; /* The caller retries a short final chunk with the same flag. */
    return (ptrdiff_t)take;
}
int RAW(argument_count)(void) { return 3; }
int RAW(argument)(size_t index, unsigned char *bytes, size_t capacity, size_t *length) {
    size_t n;
    *length = index == 1 ? 70 : index == 2 ? 0 : 4;
    if (index >= 3) { *length = 0; errno = EINVAL; return -1; }
    if (capacity < *length) { errno = ERANGE; return -1; }
    if (index == 1) {
        for (n = 0; n < *length; ++n) bytes[n] = 0xc1;
        if (argument_bad_byte) bytes[0] = 0;
    } else if (index == 0) {
        assert(crexx_native_name("RXVM", bytes, capacity, &n) == 0 && n == *length);
    }
    return 0;
}
int RAW(environment)(const unsigned char *name, size_t name_length,
                     unsigned char *bytes, size_t capacity, size_t *length) {
    unsigned char decoded[32];
    size_t used;
    const uint32_t *map;
    if (environment_unavailable) { *length = 0; errno = ENOTSUP; return -1; }
    assert(platform_text_codec_lookup("IBM1047", &map) == 0);
    assert(crexx_text_convert(map, 1, name, name_length, decoded,
                              sizeof(decoded)-1, &used) == 0);
    decoded[used] = 0;
    if (!strcmp((char *)decoded, "ABSENT")) { *length = 0; return 0; }
    assert(!strcmp((char *)decoded, "PRESENT") || !strcmp((char *)decoded, "EMPTY"));
    *length = !strcmp((char *)decoded, "EMPTY") ? 0 : 1;
    if (capacity < *length) { errno = ERANGE; return -1; }
    if (*length) bytes[0] = 0xc1;
    return 1;
}
#if defined(CREXX_PLATFORM_TSO)
lab_raw_directory *lab_tso_raw_directory_open(const unsigned char *name, size_t length) {
    unsigned char decoded[64];
    size_t written;
    const uint32_t *map;
    assert(!directory_active && platform_text_codec_lookup("IBM1047", &map) == 0);
    assert(crexx_text_convert(map, 1, name, length, decoded, sizeof(decoded)-1, &written) == 0);
    decoded[written] = 0;
    assert(!strcmp((char *)decoded, "ROOT.TXT"));
    directory_active = 1;
    return (lab_raw_directory *)&directory_active;
}
int lab_tso_raw_directory_next(lab_raw_directory *directory, unsigned char *entry,
                                size_t capacity, size_t *length) {
    assert(directory == (lab_raw_directory *)&directory_active && directory_active);
    if (directory_fault) { *length = 0; errno = EIO; return -1; }
    if (directory_index++) { *length = 0; errno = 0; return 0; }
    assert(crexx_native_name("MEMBER", entry, capacity, length) == 0);
    return 1;
}
int lab_tso_raw_directory_close(lab_raw_directory *directory) {
    assert(directory == (lab_raw_directory *)&directory_active && directory_active);
    directory_active = 0;
    return 0;
}
#endif
#if defined(CREXX_PLATFORM_CMS)
lab_raw_directory *lab_cms_raw_directory_open(const unsigned char *name, size_t length) {
    unsigned char decoded[8];
    size_t written;
    const uint32_t *map;
    assert(!directory_active && platform_text_codec_lookup("IBM1047", &map) == 0);
    assert(crexx_text_convert(map, 1, name, length, decoded, sizeof(decoded)-1, &written) == 0);
    decoded[written] = 0;
    assert(!strcmp((char *)decoded, "A2"));
    directory_active = 1;
    return (lab_raw_directory *)&directory_active;
}
int lab_cms_raw_directory_next(lab_raw_directory *directory, unsigned char *entry,
                                size_t capacity, size_t *length) {
    size_t n;
    assert(directory == (lab_raw_directory *)&directory_active && directory_active);
    if (directory_fault) { *length = 0; errno = EIO; return -1; }
    if (directory_index++) { *length = 0; errno = 0; return 0; }
    assert(capacity >= 18);
    memset(entry, 0x40, 18);
    assert(crexx_native_name("MEMBER", entry, 8, &n) == 0 && n == 6);
    assert(crexx_native_name("TXT", entry + 8, 8, &n) == 0 && n == 3);
    assert(crexx_native_name("A2", entry + 16, 2, &n) == 0 && n == 2);
    *length = 18;
    return 1;
}
int lab_cms_raw_directory_close(lab_raw_directory *directory) {
    assert(directory == (lab_raw_directory *)&directory_active && directory_active);
    directory_active = 0;
    return 0;
}
#endif

static void choose(const char *page) {
    const uint32_t *map;
    assert(platform_text_codec_lookup(page, &map) == 0);
    crexx_native_select_file_codec(map);
}
int main(void) {
    FILE *file;
    char **arguments, *environment_value;
    int argument_count;
    unsigned char readback[32];
    size_t got;
    static const unsigned char binary[] = {0, 0x80, 0xff, '\n'};
    static const unsigned char cp1252_text[] = {'A', 'B', 0xe2, 0x82, 0xac, '\n', 'Z'};
    const uint32_t *native_map;
    unsigned char decoded[4096];
    size_t decoded_length;

    reset_mock(); choose("Windows-1252");
#if defined(CREXX_PLATFORM_CMS)
    file = crexx_native_fopen_storage("A2/mixed.txt", "w", CREXX_TEXT_STORAGE_RECORDS);
#else
    file = crexx_native_fopen_storage("ROOT.TXT(MIXED)", "w", CREXX_TEXT_STORAGE_RECORDS);
#endif
    assert(file && (opened_flags & (LAB_RAW_WRITE | LAB_RAW_RECORDS)) ==
                   (LAB_RAW_WRITE | LAB_RAW_RECORDS));
    assert(fwrite(cp1252_text, 1, sizeof(cp1252_text), file) == sizeof(cp1252_text));
    assert(fclose(file) == 0 && !active);

    assert(records == 2 && record_sizes[0] == 3 && record_sizes[1] == 1);
    assert(output_length == 4 && output[0] == 'A' && output[1] == 'B' &&
           output[2] == 0x80 && output[3] == 'Z');
    assert(writes >= 3); /* The backend consumed short chunks. */
    assert(platform_text_codec_lookup("IBM1047", &native_map) == 0);
    assert(crexx_text_convert(native_map, 1, opened_name, opened_length,
                              decoded, sizeof(decoded)-1, &decoded_length) == 0);
    decoded[decoded_length] = 0;
#if defined(CREXX_PLATFORM_CMS)
    assert(!strcmp((char *)decoded, "MIXED TXT A2"));
#else
    assert(!strcmp((char *)decoded, "ROOT.TXT(MIXED)"));
#endif

    reset_mock(); choose("IBM1047");
    input[0] = 0xc1; input_length = 1; input_record = 1;
    file = crexx_native_fopen("seq.txt", "r"); assert(file);
    {
        size_t length = 99;
        char *source = file2buf(file, &length);
        assert(source && length == 3 && !memcmp(source, "A\n\n", 3) &&
               source[3] == 0 && source[4] == 0);
        free(source);
    }
    assert(fclose(file) == 0 && !active);

    reset_mock(); choose("Windows-1252");
    input[0] = 0x80; input_length = 1; input_record = 1;
    file = crexx_native_fopen_storage("read.txt", "r", CREXX_TEXT_STORAGE_RECORDS); assert(file);
    got = fread(readback, 1, sizeof(readback), file);
    assert(got == 5 && !memcmp(readback, "\xe2\x82\xac\n\n", 5));
    assert(fclose(file) == 0 && !active);

    reset_mock(); choose("UTF8");
    file = crexx_native_fopen("exchange.txt", "w"); assert(file);
    assert(!(opened_flags & LAB_RAW_RECORDS));
    assert(fwrite(cp1252_text, 1, sizeof(cp1252_text), file) == sizeof(cp1252_text));
    assert(fclose(file) == 0 && output_length == sizeof(cp1252_text));
    assert(!memcmp(output, cp1252_text, sizeof(cp1252_text))); /* Explicit LF bytes. */

    reset_mock(); choose("Windows-1252");
    file = crexx_native_fopen("mixed.txt", "w"); assert(file);
    assert(!(opened_flags & LAB_RAW_RECORDS));
    choose("IBM1047"); /* An already-open stream keeps its selected page. */
    assert(fwrite("\xe2\x82\xac", 1, 3, file) == 3);
    assert(fclose(file) == 0 && output_length == 1 && output[0] == 0x80);

    reset_mock();
    file = crexx_native_fopen("native.txt", "w"); assert(file);
    assert(opened_flags & LAB_RAW_RECORDS);
    assert(fwrite("A\n", 1, 2, file) == 2);
    assert(fclose(file) == 0 && records == 1 && output[0] == 0xc1);

    reset_mock();
    file = crexx_native_fopen("binary.dat", "wb"); assert(file);
    assert(!(opened_flags & LAB_RAW_RECORDS));
    assert(fwrite(binary, 1, sizeof(binary), file) == sizeof(binary));
    assert(fclose(file) == 0 && output_length == sizeof(binary));
    assert(!memcmp(output, binary, sizeof(binary)));

    reset_mock(); choose("Windows-1252");
    file = crexx_native_standard(1); assert(file && standard_descriptor == 1);
    assert((opened_flags & (LAB_RAW_WRITE | LAB_RAW_RECORDS)) ==
           (LAB_RAW_WRITE | LAB_RAW_RECORDS));
    assert(fwrite("A\n", 1, 2, file) == 2);
    assert(fclose(file) == 0 && !active && records == 1);
    assert(output_length == 1 && output[0] == 0xc1); /* IBM1047, not CP1252. */

    reset_mock(); choose("Windows-1252");
    input[0] = 0xc1; input_length = 1; input_record = 1;
    file = crexx_native_standard(0); assert(file && standard_descriptor == 0);
    got = fread(readback, 1, sizeof(readback), file);
    assert(got == 3 && !memcmp(readback, "A\n\n", 3));
    assert(fclose(file) == 0 && !active);
    errno = 0;
    assert(!crexx_native_standard(3) && errno == EINVAL);

    reset_mock(); choose("Windows-1252");
    assert(crexx_native_arguments(&argument_count, &arguments) == 0);
    assert(argument_count == 3 && !strcmp(arguments[0], "RXVM") &&
           strlen(arguments[1]) == 70 && arguments[1][0] == 'A' &&
           !strcmp(arguments[2], "") && !arguments[3]);
    crexx_native_free_arguments(argument_count, arguments);
    assert(crexx_native_environment("PRESENT", &environment_value) == 1 &&
           !strcmp(environment_value, "A"));
    free(environment_value);
    assert(crexx_native_environment("EMPTY", &environment_value) == 1 &&
           !strcmp(environment_value, ""));
    free(environment_value);
    assert(crexx_native_environment("ABSENT", &environment_value) == 0 && !environment_value);
    {
        char present[] = "PRESENT", absent[] = "ABSENT", embedded[] = {'A', 0, 'B'};
        assert(getEnvVal(&environment_value, present, strlen(present)) == 1 &&
               !strcmp(environment_value, "A"));
        free(environment_value);
        assert(getEnvVal(&environment_value, absent, strlen(absent)) == 0 &&
               !strcmp(environment_value, ""));
        assert(getEnvVal(&environment_value, embedded, sizeof(embedded)) == -1 &&
               errno == EINVAL);
    }
    environment_unavailable = 1;
    assert(crexx_native_environment("PRESENT", &environment_value) == -1 &&
           errno == ENOTSUP && !environment_value);
    {
        char present[] = "PRESENT";
        assert(getEnvVal(&environment_value, present, strlen(present)) == -1 &&
               errno == ENOTSUP);
    }
    argument_bad_byte = 1;
    assert(crexx_native_arguments(&argument_count, &arguments) == -1 &&
           errno == EINVAL && !arguments && argument_count == 0);

    reset_mock(); choose("ASCII");
    file = crexx_native_fopen_storage("long.txt", "w", CREXX_TEXT_STORAGE_RECORDS); assert(file);
    (void)fwrite("ABCDE\n", 1, 6, file);
    assert(fclose(file) != 0 && !active && records == 0); /* No partial record. */

    reset_mock(); flush_error = 1;
    file = crexx_native_fopen("flush.txt", "w"); assert(file);
    assert(fwrite("A\n", 1, 2, file) == 2);
    assert(fclose(file) != 0 && !active);

    reset_mock(); close_error = 1;
    file = crexx_native_fopen("close.txt", "w"); assert(file);
    assert(fwrite("A\n", 1, 2, file) == 2);
    assert(fclose(file) != 0 && !active);

    reset_mock(); read_error = 1;
    file = crexx_native_fopen("readerr.txt", "r"); assert(file);
    {
        size_t length = 99;
        assert(!file2buf(file, &length) && length == 0 && ferror(file));
    }
    assert(fclose(file) != 0 && !active);

    reset_mock(); write_error = 1;
    file = crexx_native_fopen("writeerr.txt", "w"); assert(file);
    (void)fwrite("A\n", 1, 2, file);
    assert(fclose(file) != 0 && !active && records == 0);

    reset_mock();
    crexx_native_panic("PANIC: \xc3\xa9\n", 10);
    assert(platform_text_codec_lookup("IBM1047", &native_map) == 0);
    assert(crexx_text_convert(native_map, 1, panic_output, panic_length,
                              decoded, sizeof(decoded)-1, &decoded_length) == 0);
    decoded[decoded_length] = 0;
    assert(!strcmp((char *)decoded, "PANIC: \xc3\xa9"));
#if defined(CREXX_PLATFORM_TSO)
    {
        void *directory = NULL;
        char *member;
        reset_mock();
        member = dirfstfl("ROOT", "ME", "TXT", &directory);
        assert(member && !strcmp(member, "member.txt") && directory_active);
        assert(!dirnxtfl(&directory) && errno == 0);
        dirclose(&directory);
        assert(!directory && !directory_active);
        reset_mock();
        directory_fault = 1;
        assert(!dirfstfl("ROOT", NULL, "TXT", &directory));
        assert(errno == EIO && directory_active);
        dirclose(&directory);
        assert(!directory && !directory_active);
    }
#endif
#if defined(CREXX_PLATFORM_CMS)
    {
        char exists_name[] = "exists", exists_type[] = "txt";
        reset_mock();
        assert(fileexists(exists_name, exists_type, NULL) && !active);
        assert((opened_flags & (LAB_RAW_READ | LAB_RAW_RECORDS)) == LAB_RAW_READ);
    }
    {
        void *directory = NULL;
        char *entry;
        reset_mock();
        entry = dirfstfl("A2", "ME", "TXT", &directory);
        assert(entry && !strcmp(entry, "member.txt") && directory_active);
        assert(!dirnxtfl(&directory) && errno == 0);
        dirclose(&directory);
        assert(!directory && !directory_active);
        reset_mock();
        directory_fault = 1;
        assert(!dirfstfl("A2", NULL, "TXT", &directory));
        assert(errno == EIO && directory_active);
        dirclose(&directory);
        assert(!directory && !directory_active);
    }
#endif
    puts("PASS raw native text, records, binary, names, close errors and panic output");
    return 0;
}
