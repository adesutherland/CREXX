#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxvmintp.h"
/* Host console fixture only: the entry must select the raw SDK contract. */
static void custom_say(const char *text, size_t length) {
    fwrite(text, 1, length, stdout);
}
void mainframe_set_text_conversion(int enabled) {
    assert(!enabled);
    if (getenv("CREXX_TEST_CUSTOM_SAY")) rxvm_setsayexit_bytes(custom_say);
    if (getenv("CREXX_TEST_CONSOLE_UNBUFFERED")) setvbuf(stdout, NULL, _IONBF, 0);
}
