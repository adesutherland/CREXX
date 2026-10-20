/* cREXX License (MIT). Kernel-only NUTF8 byte-unit regression.
 * No VM value/provider ABI is mixed with a differently configured VM. */
#include "rxvmparse.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static int checks, failures;
#define CHECK(test) do { ++checks; if (!(test)) { \
    fprintf(stderr, "FAIL: line %d: %s\n", __LINE__, #test); ++failures; } } while (0)
int main(void) {
    static const unsigned char plan[] = {
        0x50,1,8,0,4,0,2,0,
        3,0,1,0,0,0,0,0,0,0, 1,1,
        3,0,3,0,0,0,0,0,0,0, 1,1
    };
    unsigned char huge[sizeof(plan)];
    const char source[] = {'\xff', 0, 'Z'};
    const char *text;
    size_t length;
    int status;
    rxvm_parse_cursor *cursor = rxvm_parse_create(source, sizeof(source), plan, sizeof(plan), &status);
    CHECK(cursor && status == 0);
    if (!cursor) return 1;
    CHECK(rxvm_parse_next(cursor) == RXVM_PARSE_FIELD && rxvm_parse_item(cursor) == 2);
    CHECK(rxvm_parse_text(cursor, &text, &length) == 0 && length == 2 && !memcmp(text, source, 2));
    CHECK(rxvm_parse_next(cursor) == RXVM_PARSE_FIELD && rxvm_parse_item(cursor) == 4);
    CHECK(rxvm_parse_text(cursor, &text, &length) == 0 && length == 1 && text[0] == 'Z');
    CHECK(rxvm_parse_next(cursor) == RXVM_PARSE_DONE);
    rxvm_parse_destroy(cursor);
    memcpy(huge, plan, sizeof(plan));
    huge[20] = 4;
    memset(huge + 22, 255, 8); huge[29] = 127;
    cursor = rxvm_parse_create(source, sizeof(source), huge, sizeof(huge), &status);
    CHECK(cursor && status == 0);
    if (cursor) {
        CHECK(rxvm_parse_next(cursor) == RXVM_PARSE_FIELD);
        CHECK(rxvm_parse_text(cursor, &text, &length) == 0 && length == 3 && !memcmp(text, source, 3));
        CHECK(rxvm_parse_next(cursor) == RXVM_PARSE_FIELD);
        CHECK(rxvm_parse_text(cursor, &text, &length) == 0 && length == 0);
        rxvm_parse_destroy(cursor);
    }
    printf("PARSE_CURSOR_BYTES|checks=%d|failures=%d\n", checks, failures);
    return failures ? 1 : 0;
}
