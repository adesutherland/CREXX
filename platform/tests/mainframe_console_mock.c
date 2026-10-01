#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxvmintp.h"
/* Host console fixture only: the entry must select the raw SDK contract. */
static void custom_say(char *text) { fwrite(text, 1, strlen(text), stdout); }
void mainframe_set_text_conversion(int enabled) {
    assert(!enabled);
    if (getenv("CREXX_TEST_CUSTOM_SAY")) rxvm_setsayexit(custom_say);
    if (getenv("CREXX_TEST_CONSOLE_UNBUFFERED")) setvbuf(stdout, NULL, _IONBF, 0);
}
