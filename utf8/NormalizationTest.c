/* Test Suite - Auto generated file! */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int nfd_normaliser(const char *str, int len, char* out, int *out_len);
int nfkd_normaliser(const char *str, int len, char* out, int *out_len);
int nfc_normaliser(const char *str, int len, char* out, int *out_len);
int nfkc_normaliser(const char *str, int len, char* out, int *out_len);

static int errors = 0;

/* Function to convert a utf8 string to a list of code points */
char* utf8_to_codepoints(const char *utf8_str) {
    unsigned char *ptr = (unsigned char *)utf8_str;
    int code_point = 0;
    int num_bytes = 0;
    int i;
    char buffer[10];  // Buffer for storing individual codepoints as strings
    static char output[1000];  // Buffer for storing the output string
    output[0] = 0;

    while (*ptr) {
        // Initialize code_point and num_bytes for each new character
        code_point = 0;
        num_bytes = 0;

        // Determine the number of bytes for this character
        if ((*ptr & 0x80) == 0) {        // 1-byte (ASCII)
            code_point = *ptr;
            num_bytes = 1;
        } else if ((*ptr & 0xE0) == 0xC0) { // 2-bytes
            code_point = *ptr & 0x1F;
            num_bytes = 2;
        } else if ((*ptr & 0xF0) == 0xE0) { // 3-bytes
            code_point = *ptr & 0x0F;
            num_bytes = 3;
        } else if ((*ptr & 0xF8) == 0xF0) { // 4-bytes
            code_point = *ptr & 0x07;
            num_bytes = 4;
        }

        // Read the remaining bytes for this character
        for (i = 1; i < num_bytes; i++) {
            ptr++;
            if ((*ptr & 0xC0) == 0x80) {  // Continuation byte
                code_point = (code_point << 6) | (*ptr & 0x3F);
            }
        }

        // Append the code point to the output string
        sprintf(buffer, "%04X ", code_point);
        strcat(output, buffer);

        // Move to the next character
        ptr++;
    }
    return output;
}

/* Function to check result */
void checkresult(int rc, char* input, char* expected, char* output, char* test_id) {
    if (rc > 0 || strcmp(output, expected) != 0) {
        errors++;
        printf("*** ERROR ***\n");
        printf("TEST     %s\n", test_id);
        printf("RC       %d\n", rc);
        printf("INPUT    \"%s\" ( %s)\n", input, utf8_to_codepoints(input));
        printf("EXPECTED \"%s\" ( %s)\n", expected, utf8_to_codepoints(expected));
        printf("OUTPUT   \"%s\" ( %s)\n", output, utf8_to_codepoints(output));
        printf("\n");
    }
    return;
}

int main() {
    int out_len;
    char out_buffer[250];
    int err = 0;

    err = nfd_normaliser("\xE1\xB8\x8A", strlen("\xE1\xB8\x8A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8A", "\x44" "\xCC\x87", out_buffer, "1 NFD \"(Ḋ; Ḋ; D◌̇; Ḋ; D◌̇; ) LATIN CAPITAL LETTER D WITH DOT ABOVE\"");

    err = nfkd_normaliser("\xE1\xB8\x8A", strlen("\xE1\xB8\x8A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8A", "\x44" "\xCC\x87", out_buffer, "2 NFKD \"(Ḋ; Ḋ; D◌̇; Ḋ; D◌̇; ) LATIN CAPITAL LETTER D WITH DOT ABOVE\"");

    err = nfc_normaliser("\xE1\xB8\x8A", strlen("\xE1\xB8\x8A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8A", "\xE1\xB8\x8A", out_buffer, "3 NFC \"(Ḋ; Ḋ; D◌̇; Ḋ; D◌̇; ) LATIN CAPITAL LETTER D WITH DOT ABOVE\"");

    err = nfd_normaliser("\xE1\xB8\x8C", strlen("\xE1\xB8\x8C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8C", "\x44" "\xCC\xA3", out_buffer, "4 NFD \"(Ḍ; Ḍ; D◌̣; Ḍ; D◌̣; ) LATIN CAPITAL LETTER D WITH DOT BELOW\"");

    err = nfkd_normaliser("\xE1\xB8\x8C", strlen("\xE1\xB8\x8C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8C", "\x44" "\xCC\xA3", out_buffer, "5 NFKD \"(Ḍ; Ḍ; D◌̣; Ḍ; D◌̣; ) LATIN CAPITAL LETTER D WITH DOT BELOW\"");

    err = nfc_normaliser("\xE1\xB8\x8C", strlen("\xE1\xB8\x8C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8C", "\xE1\xB8\x8C", out_buffer, "6 NFC \"(Ḍ; Ḍ; D◌̣; Ḍ; D◌̣; ) LATIN CAPITAL LETTER D WITH DOT BELOW\"");

    err = nfd_normaliser("\xE1\xB8\x8A" "\xCC\xA3", strlen("\xE1\xB8\x8A" "\xCC\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8A" "\xCC\xA3", "\x44" "\xCC\xA3" "\xCC\x87", out_buffer, "7 NFD \"(Ḋ◌̣; Ḍ◌̇; D◌̣◌̇; Ḍ◌̇; D◌̣◌̇; ) LATIN CAPITAL LETTER D WITH DOT ABOVE, COMBINING DOT BELOW\"");

    err = nfkd_normaliser("\xE1\xB8\x8A" "\xCC\xA3", strlen("\xE1\xB8\x8A" "\xCC\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8A" "\xCC\xA3", "\x44" "\xCC\xA3" "\xCC\x87", out_buffer, "8 NFKD \"(Ḋ◌̣; Ḍ◌̇; D◌̣◌̇; Ḍ◌̇; D◌̣◌̇; ) LATIN CAPITAL LETTER D WITH DOT ABOVE, COMBINING DOT BELOW\"");

    err = nfc_normaliser("\xE1\xB8\x8A" "\xCC\xA3", strlen("\xE1\xB8\x8A" "\xCC\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8A" "\xCC\xA3", "\xE1\xB8\x8C" "\xCC\x87", out_buffer, "9 NFC \"(Ḋ◌̣; Ḍ◌̇; D◌̣◌̇; Ḍ◌̇; D◌̣◌̇; ) LATIN CAPITAL LETTER D WITH DOT ABOVE, COMBINING DOT BELOW\"");

    err = nfd_normaliser("\xE1\xB8\x8C" "\xCC\x87", strlen("\xE1\xB8\x8C" "\xCC\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8C" "\xCC\x87", "\x44" "\xCC\xA3" "\xCC\x87", out_buffer, "10 NFD \"(Ḍ◌̇; Ḍ◌̇; D◌̣◌̇; Ḍ◌̇; D◌̣◌̇; ) LATIN CAPITAL LETTER D WITH DOT BELOW, COMBINING DOT ABOVE\"");

    err = nfkd_normaliser("\xE1\xB8\x8C" "\xCC\x87", strlen("\xE1\xB8\x8C" "\xCC\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8C" "\xCC\x87", "\x44" "\xCC\xA3" "\xCC\x87", out_buffer, "11 NFKD \"(Ḍ◌̇; Ḍ◌̇; D◌̣◌̇; Ḍ◌̇; D◌̣◌̇; ) LATIN CAPITAL LETTER D WITH DOT BELOW, COMBINING DOT ABOVE\"");

    err = nfc_normaliser("\xE1\xB8\x8C" "\xCC\x87", strlen("\xE1\xB8\x8C" "\xCC\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8C" "\xCC\x87", "\xE1\xB8\x8C" "\xCC\x87", out_buffer, "12 NFC \"(Ḍ◌̇; Ḍ◌̇; D◌̣◌̇; Ḍ◌̇; D◌̣◌̇; ) LATIN CAPITAL LETTER D WITH DOT BELOW, COMBINING DOT ABOVE\"");

    err = nfd_normaliser("\x44" "\xCC\x87" "\xCC\xA3", strlen("\x44" "\xCC\x87" "\xCC\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x44" "\xCC\x87" "\xCC\xA3", "\x44" "\xCC\xA3" "\xCC\x87", out_buffer, "13 NFD \"(D◌̇◌̣; Ḍ◌̇; D◌̣◌̇; Ḍ◌̇; D◌̣◌̇; ) LATIN CAPITAL LETTER D, COMBINING DOT ABOVE, COMBINING DOT BELOW\"");

    err = nfkd_normaliser("\x44" "\xCC\x87" "\xCC\xA3", strlen("\x44" "\xCC\x87" "\xCC\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x44" "\xCC\x87" "\xCC\xA3", "\x44" "\xCC\xA3" "\xCC\x87", out_buffer, "14 NFKD \"(D◌̇◌̣; Ḍ◌̇; D◌̣◌̇; Ḍ◌̇; D◌̣◌̇; ) LATIN CAPITAL LETTER D, COMBINING DOT ABOVE, COMBINING DOT BELOW\"");

    err = nfc_normaliser("\x44" "\xCC\x87" "\xCC\xA3", strlen("\x44" "\xCC\x87" "\xCC\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x44" "\xCC\x87" "\xCC\xA3", "\xE1\xB8\x8C" "\xCC\x87", out_buffer, "15 NFC \"(D◌̇◌̣; Ḍ◌̇; D◌̣◌̇; Ḍ◌̇; D◌̣◌̇; ) LATIN CAPITAL LETTER D, COMBINING DOT ABOVE, COMBINING DOT BELOW\"");

    err = nfd_normaliser("\x44" "\xCC\xA3" "\xCC\x87", strlen("\x44" "\xCC\xA3" "\xCC\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x44" "\xCC\xA3" "\xCC\x87", "\x44" "\xCC\xA3" "\xCC\x87", out_buffer, "16 NFD \"(D◌̣◌̇; Ḍ◌̇; D◌̣◌̇; Ḍ◌̇; D◌̣◌̇; ) LATIN CAPITAL LETTER D, COMBINING DOT BELOW, COMBINING DOT ABOVE\"");

    err = nfkd_normaliser("\x44" "\xCC\xA3" "\xCC\x87", strlen("\x44" "\xCC\xA3" "\xCC\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x44" "\xCC\xA3" "\xCC\x87", "\x44" "\xCC\xA3" "\xCC\x87", out_buffer, "17 NFKD \"(D◌̣◌̇; Ḍ◌̇; D◌̣◌̇; Ḍ◌̇; D◌̣◌̇; ) LATIN CAPITAL LETTER D, COMBINING DOT BELOW, COMBINING DOT ABOVE\"");

    err = nfc_normaliser("\x44" "\xCC\xA3" "\xCC\x87", strlen("\x44" "\xCC\xA3" "\xCC\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x44" "\xCC\xA3" "\xCC\x87", "\xE1\xB8\x8C" "\xCC\x87", out_buffer, "18 NFC \"(D◌̣◌̇; Ḍ◌̇; D◌̣◌̇; Ḍ◌̇; D◌̣◌̇; ) LATIN CAPITAL LETTER D, COMBINING DOT BELOW, COMBINING DOT ABOVE\"");

    err = nfd_normaliser("\xE1\xB8\x8A" "\xCC\x9B", strlen("\xE1\xB8\x8A" "\xCC\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8A" "\xCC\x9B", "\x44" "\xCC\x9B" "\xCC\x87", out_buffer, "19 NFD \"(Ḋ◌̛; Ḋ◌̛; D◌̛◌̇; Ḋ◌̛; D◌̛◌̇; ) LATIN CAPITAL LETTER D WITH DOT ABOVE, COMBINING HORN\"");

    err = nfkd_normaliser("\xE1\xB8\x8A" "\xCC\x9B", strlen("\xE1\xB8\x8A" "\xCC\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8A" "\xCC\x9B", "\x44" "\xCC\x9B" "\xCC\x87", out_buffer, "20 NFKD \"(Ḋ◌̛; Ḋ◌̛; D◌̛◌̇; Ḋ◌̛; D◌̛◌̇; ) LATIN CAPITAL LETTER D WITH DOT ABOVE, COMBINING HORN\"");

    err = nfc_normaliser("\xE1\xB8\x8A" "\xCC\x9B", strlen("\xE1\xB8\x8A" "\xCC\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8A" "\xCC\x9B", "\xE1\xB8\x8A" "\xCC\x9B", out_buffer, "21 NFC \"(Ḋ◌̛; Ḋ◌̛; D◌̛◌̇; Ḋ◌̛; D◌̛◌̇; ) LATIN CAPITAL LETTER D WITH DOT ABOVE, COMBINING HORN\"");

    err = nfd_normaliser("\xE1\xB8\x8C" "\xCC\x9B", strlen("\xE1\xB8\x8C" "\xCC\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8C" "\xCC\x9B", "\x44" "\xCC\x9B" "\xCC\xA3", out_buffer, "22 NFD \"(Ḍ◌̛; Ḍ◌̛; D◌̛◌̣; Ḍ◌̛; D◌̛◌̣; ) LATIN CAPITAL LETTER D WITH DOT BELOW, COMBINING HORN\"");

    err = nfkd_normaliser("\xE1\xB8\x8C" "\xCC\x9B", strlen("\xE1\xB8\x8C" "\xCC\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8C" "\xCC\x9B", "\x44" "\xCC\x9B" "\xCC\xA3", out_buffer, "23 NFKD \"(Ḍ◌̛; Ḍ◌̛; D◌̛◌̣; Ḍ◌̛; D◌̛◌̣; ) LATIN CAPITAL LETTER D WITH DOT BELOW, COMBINING HORN\"");

    err = nfc_normaliser("\xE1\xB8\x8C" "\xCC\x9B", strlen("\xE1\xB8\x8C" "\xCC\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8C" "\xCC\x9B", "\xE1\xB8\x8C" "\xCC\x9B", out_buffer, "24 NFC \"(Ḍ◌̛; Ḍ◌̛; D◌̛◌̣; Ḍ◌̛; D◌̛◌̣; ) LATIN CAPITAL LETTER D WITH DOT BELOW, COMBINING HORN\"");

    err = nfd_normaliser("\xE1\xB8\x8A" "\xCC\x9B" "\xCC\xA3", strlen("\xE1\xB8\x8A" "\xCC\x9B" "\xCC\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8A" "\xCC\x9B" "\xCC\xA3", "\x44" "\xCC\x9B" "\xCC\xA3" "\xCC\x87", out_buffer, "25 NFD \"(Ḋ◌̛◌̣; Ḍ◌̛◌̇; D◌̛◌̣◌̇; Ḍ◌̛◌̇; D◌̛◌̣◌̇; ) LATIN CAPITAL LETTER D WITH DOT ABOVE, COMBINING HORN, COMBINING DOT BELOW\"");

    err = nfkd_normaliser("\xE1\xB8\x8A" "\xCC\x9B" "\xCC\xA3", strlen("\xE1\xB8\x8A" "\xCC\x9B" "\xCC\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8A" "\xCC\x9B" "\xCC\xA3", "\x44" "\xCC\x9B" "\xCC\xA3" "\xCC\x87", out_buffer, "26 NFKD \"(Ḋ◌̛◌̣; Ḍ◌̛◌̇; D◌̛◌̣◌̇; Ḍ◌̛◌̇; D◌̛◌̣◌̇; ) LATIN CAPITAL LETTER D WITH DOT ABOVE, COMBINING HORN, COMBINING DOT BELOW\"");

    err = nfc_normaliser("\xE1\xB8\x8A" "\xCC\x9B" "\xCC\xA3", strlen("\xE1\xB8\x8A" "\xCC\x9B" "\xCC\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8A" "\xCC\x9B" "\xCC\xA3", "\xE1\xB8\x8C" "\xCC\x9B" "\xCC\x87", out_buffer, "27 NFC \"(Ḋ◌̛◌̣; Ḍ◌̛◌̇; D◌̛◌̣◌̇; Ḍ◌̛◌̇; D◌̛◌̣◌̇; ) LATIN CAPITAL LETTER D WITH DOT ABOVE, COMBINING HORN, COMBINING DOT BELOW\"");

    err = nfd_normaliser("\xE1\xB8\x8C" "\xCC\x9B" "\xCC\x87", strlen("\xE1\xB8\x8C" "\xCC\x9B" "\xCC\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8C" "\xCC\x9B" "\xCC\x87", "\x44" "\xCC\x9B" "\xCC\xA3" "\xCC\x87", out_buffer, "28 NFD \"(Ḍ◌̛◌̇; Ḍ◌̛◌̇; D◌̛◌̣◌̇; Ḍ◌̛◌̇; D◌̛◌̣◌̇; ) LATIN CAPITAL LETTER D WITH DOT BELOW, COMBINING HORN, COMBINING DOT ABOVE\"");

    err = nfkd_normaliser("\xE1\xB8\x8C" "\xCC\x9B" "\xCC\x87", strlen("\xE1\xB8\x8C" "\xCC\x9B" "\xCC\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8C" "\xCC\x9B" "\xCC\x87", "\x44" "\xCC\x9B" "\xCC\xA3" "\xCC\x87", out_buffer, "29 NFKD \"(Ḍ◌̛◌̇; Ḍ◌̛◌̇; D◌̛◌̣◌̇; Ḍ◌̛◌̇; D◌̛◌̣◌̇; ) LATIN CAPITAL LETTER D WITH DOT BELOW, COMBINING HORN, COMBINING DOT ABOVE\"");

    err = nfc_normaliser("\xE1\xB8\x8C" "\xCC\x9B" "\xCC\x87", strlen("\xE1\xB8\x8C" "\xCC\x9B" "\xCC\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x8C" "\xCC\x9B" "\xCC\x87", "\xE1\xB8\x8C" "\xCC\x9B" "\xCC\x87", out_buffer, "30 NFC \"(Ḍ◌̛◌̇; Ḍ◌̛◌̇; D◌̛◌̣◌̇; Ḍ◌̛◌̇; D◌̛◌̣◌̇; ) LATIN CAPITAL LETTER D WITH DOT BELOW, COMBINING HORN, COMBINING DOT ABOVE\"");

    err = nfd_normaliser("\x44" "\xCC\x9B" "\xCC\x87" "\xCC\xA3", strlen("\x44" "\xCC\x9B" "\xCC\x87" "\xCC\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x44" "\xCC\x9B" "\xCC\x87" "\xCC\xA3", "\x44" "\xCC\x9B" "\xCC\xA3" "\xCC\x87", out_buffer, "31 NFD \"(D◌̛◌̇◌̣; Ḍ◌̛◌̇; D◌̛◌̣◌̇; Ḍ◌̛◌̇; D◌̛◌̣◌̇; ) LATIN CAPITAL LETTER D, COMBINING HORN, COMBINING DOT ABOVE, COMBINING DOT BELOW\"");

    err = nfkd_normaliser("\x44" "\xCC\x9B" "\xCC\x87" "\xCC\xA3", strlen("\x44" "\xCC\x9B" "\xCC\x87" "\xCC\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x44" "\xCC\x9B" "\xCC\x87" "\xCC\xA3", "\x44" "\xCC\x9B" "\xCC\xA3" "\xCC\x87", out_buffer, "32 NFKD \"(D◌̛◌̇◌̣; Ḍ◌̛◌̇; D◌̛◌̣◌̇; Ḍ◌̛◌̇; D◌̛◌̣◌̇; ) LATIN CAPITAL LETTER D, COMBINING HORN, COMBINING DOT ABOVE, COMBINING DOT BELOW\"");

    err = nfc_normaliser("\x44" "\xCC\x9B" "\xCC\x87" "\xCC\xA3", strlen("\x44" "\xCC\x9B" "\xCC\x87" "\xCC\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x44" "\xCC\x9B" "\xCC\x87" "\xCC\xA3", "\xE1\xB8\x8C" "\xCC\x9B" "\xCC\x87", out_buffer, "33 NFC \"(D◌̛◌̇◌̣; Ḍ◌̛◌̇; D◌̛◌̣◌̇; Ḍ◌̛◌̇; D◌̛◌̣◌̇; ) LATIN CAPITAL LETTER D, COMBINING HORN, COMBINING DOT ABOVE, COMBINING DOT BELOW\"");

    err = nfd_normaliser("\x44" "\xCC\x9B" "\xCC\xA3" "\xCC\x87", strlen("\x44" "\xCC\x9B" "\xCC\xA3" "\xCC\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x44" "\xCC\x9B" "\xCC\xA3" "\xCC\x87", "\x44" "\xCC\x9B" "\xCC\xA3" "\xCC\x87", out_buffer, "34 NFD \"(D◌̛◌̣◌̇; Ḍ◌̛◌̇; D◌̛◌̣◌̇; Ḍ◌̛◌̇; D◌̛◌̣◌̇; ) LATIN CAPITAL LETTER D, COMBINING HORN, COMBINING DOT BELOW, COMBINING DOT ABOVE\"");

    err = nfkd_normaliser("\x44" "\xCC\x9B" "\xCC\xA3" "\xCC\x87", strlen("\x44" "\xCC\x9B" "\xCC\xA3" "\xCC\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x44" "\xCC\x9B" "\xCC\xA3" "\xCC\x87", "\x44" "\xCC\x9B" "\xCC\xA3" "\xCC\x87", out_buffer, "35 NFKD \"(D◌̛◌̣◌̇; Ḍ◌̛◌̇; D◌̛◌̣◌̇; Ḍ◌̛◌̇; D◌̛◌̣◌̇; ) LATIN CAPITAL LETTER D, COMBINING HORN, COMBINING DOT BELOW, COMBINING DOT ABOVE\"");

    err = nfc_normaliser("\x44" "\xCC\x9B" "\xCC\xA3" "\xCC\x87", strlen("\x44" "\xCC\x9B" "\xCC\xA3" "\xCC\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x44" "\xCC\x9B" "\xCC\xA3" "\xCC\x87", "\xE1\xB8\x8C" "\xCC\x9B" "\xCC\x87", out_buffer, "36 NFC \"(D◌̛◌̣◌̇; Ḍ◌̛◌̇; D◌̛◌̣◌̇; Ḍ◌̛◌̇; D◌̛◌̣◌̇; ) LATIN CAPITAL LETTER D, COMBINING HORN, COMBINING DOT BELOW, COMBINING DOT ABOVE\"");

    err = nfd_normaliser("\xC3\x88", strlen("\xC3\x88"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x88", "\x45" "\xCC\x80", out_buffer, "37 NFD \"(È; È; E◌̀; È; E◌̀; ) LATIN CAPITAL LETTER E WITH GRAVE\"");

    err = nfkd_normaliser("\xC3\x88", strlen("\xC3\x88"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x88", "\x45" "\xCC\x80", out_buffer, "38 NFKD \"(È; È; E◌̀; È; E◌̀; ) LATIN CAPITAL LETTER E WITH GRAVE\"");

    err = nfc_normaliser("\xC3\x88", strlen("\xC3\x88"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x88", "\xC3\x88", out_buffer, "39 NFC \"(È; È; E◌̀; È; E◌̀; ) LATIN CAPITAL LETTER E WITH GRAVE\"");

    err = nfd_normaliser("\xC4\x92", strlen("\xC4\x92"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x92", "\x45" "\xCC\x84", out_buffer, "40 NFD \"(Ē; Ē; E◌̄; Ē; E◌̄; ) LATIN CAPITAL LETTER E WITH MACRON\"");

    err = nfkd_normaliser("\xC4\x92", strlen("\xC4\x92"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x92", "\x45" "\xCC\x84", out_buffer, "41 NFKD \"(Ē; Ē; E◌̄; Ē; E◌̄; ) LATIN CAPITAL LETTER E WITH MACRON\"");

    err = nfc_normaliser("\xC4\x92", strlen("\xC4\x92"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x92", "\xC4\x92", out_buffer, "42 NFC \"(Ē; Ē; E◌̄; Ē; E◌̄; ) LATIN CAPITAL LETTER E WITH MACRON\"");

    err = nfd_normaliser("\x45" "\xCC\x80", strlen("\x45" "\xCC\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x45" "\xCC\x80", "\x45" "\xCC\x80", out_buffer, "43 NFD \"(E◌̀; È; E◌̀; È; E◌̀; ) LATIN CAPITAL LETTER E, COMBINING GRAVE ACCENT\"");

    err = nfkd_normaliser("\x45" "\xCC\x80", strlen("\x45" "\xCC\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x45" "\xCC\x80", "\x45" "\xCC\x80", out_buffer, "44 NFKD \"(E◌̀; È; E◌̀; È; E◌̀; ) LATIN CAPITAL LETTER E, COMBINING GRAVE ACCENT\"");

    err = nfc_normaliser("\x45" "\xCC\x80", strlen("\x45" "\xCC\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x45" "\xCC\x80", "\xC3\x88", out_buffer, "45 NFC \"(E◌̀; È; E◌̀; È; E◌̀; ) LATIN CAPITAL LETTER E, COMBINING GRAVE ACCENT\"");

    err = nfd_normaliser("\x45" "\xCC\x84", strlen("\x45" "\xCC\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x45" "\xCC\x84", "\x45" "\xCC\x84", out_buffer, "46 NFD \"(E◌̄; Ē; E◌̄; Ē; E◌̄; ) LATIN CAPITAL LETTER E, COMBINING MACRON\"");

    err = nfkd_normaliser("\x45" "\xCC\x84", strlen("\x45" "\xCC\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x45" "\xCC\x84", "\x45" "\xCC\x84", out_buffer, "47 NFKD \"(E◌̄; Ē; E◌̄; Ē; E◌̄; ) LATIN CAPITAL LETTER E, COMBINING MACRON\"");

    err = nfc_normaliser("\x45" "\xCC\x84", strlen("\x45" "\xCC\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x45" "\xCC\x84", "\xC4\x92", out_buffer, "48 NFC \"(E◌̄; Ē; E◌̄; Ē; E◌̄; ) LATIN CAPITAL LETTER E, COMBINING MACRON\"");

    err = nfd_normaliser("\xE1\xB8\x94", strlen("\xE1\xB8\x94"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x94", "\x45" "\xCC\x84" "\xCC\x80", out_buffer, "49 NFD \"(Ḕ; Ḕ; E◌̄◌̀; Ḕ; E◌̄◌̀; ) LATIN CAPITAL LETTER E WITH MACRON AND GRAVE\"");

    err = nfkd_normaliser("\xE1\xB8\x94", strlen("\xE1\xB8\x94"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x94", "\x45" "\xCC\x84" "\xCC\x80", out_buffer, "50 NFKD \"(Ḕ; Ḕ; E◌̄◌̀; Ḕ; E◌̄◌̀; ) LATIN CAPITAL LETTER E WITH MACRON AND GRAVE\"");

    err = nfc_normaliser("\xE1\xB8\x94", strlen("\xE1\xB8\x94"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x94", "\xE1\xB8\x94", out_buffer, "51 NFC \"(Ḕ; Ḕ; E◌̄◌̀; Ḕ; E◌̄◌̀; ) LATIN CAPITAL LETTER E WITH MACRON AND GRAVE\"");

    err = nfd_normaliser("\xC4\x92" "\xCC\x80", strlen("\xC4\x92" "\xCC\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x92" "\xCC\x80", "\x45" "\xCC\x84" "\xCC\x80", out_buffer, "52 NFD \"(Ē◌̀; Ḕ; E◌̄◌̀; Ḕ; E◌̄◌̀; ) LATIN CAPITAL LETTER E WITH MACRON, COMBINING GRAVE ACCENT\"");

    err = nfkd_normaliser("\xC4\x92" "\xCC\x80", strlen("\xC4\x92" "\xCC\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x92" "\xCC\x80", "\x45" "\xCC\x84" "\xCC\x80", out_buffer, "53 NFKD \"(Ē◌̀; Ḕ; E◌̄◌̀; Ḕ; E◌̄◌̀; ) LATIN CAPITAL LETTER E WITH MACRON, COMBINING GRAVE ACCENT\"");

    err = nfc_normaliser("\xC4\x92" "\xCC\x80", strlen("\xC4\x92" "\xCC\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x92" "\xCC\x80", "\xE1\xB8\x94", out_buffer, "54 NFC \"(Ē◌̀; Ḕ; E◌̄◌̀; Ḕ; E◌̄◌̀; ) LATIN CAPITAL LETTER E WITH MACRON, COMBINING GRAVE ACCENT\"");

    err = nfd_normaliser("\xE1\xB8\x94" "\xCC\x84", strlen("\xE1\xB8\x94" "\xCC\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x94" "\xCC\x84", "\x45" "\xCC\x84" "\xCC\x80" "\xCC\x84", out_buffer, "55 NFD \"(Ḕ◌̄; Ḕ◌̄; E◌̄◌̀◌̄; Ḕ◌̄; E◌̄◌̀◌̄; ) LATIN CAPITAL LETTER E WITH MACRON AND GRAVE, COMBINING MACRON\"");

    err = nfkd_normaliser("\xE1\xB8\x94" "\xCC\x84", strlen("\xE1\xB8\x94" "\xCC\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x94" "\xCC\x84", "\x45" "\xCC\x84" "\xCC\x80" "\xCC\x84", out_buffer, "56 NFKD \"(Ḕ◌̄; Ḕ◌̄; E◌̄◌̀◌̄; Ḕ◌̄; E◌̄◌̀◌̄; ) LATIN CAPITAL LETTER E WITH MACRON AND GRAVE, COMBINING MACRON\"");

    err = nfc_normaliser("\xE1\xB8\x94" "\xCC\x84", strlen("\xE1\xB8\x94" "\xCC\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\xB8\x94" "\xCC\x84", "\xE1\xB8\x94" "\xCC\x84", out_buffer, "57 NFC \"(Ḕ◌̄; Ḕ◌̄; E◌̄◌̀◌̄; Ḕ◌̄; E◌̄◌̀◌̄; ) LATIN CAPITAL LETTER E WITH MACRON AND GRAVE, COMBINING MACRON\"");

    err = nfd_normaliser("\x45" "\xCC\x84" "\xCC\x80", strlen("\x45" "\xCC\x84" "\xCC\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x45" "\xCC\x84" "\xCC\x80", "\x45" "\xCC\x84" "\xCC\x80", out_buffer, "58 NFD \"(E◌̄◌̀; Ḕ; E◌̄◌̀; Ḕ; E◌̄◌̀; ) LATIN CAPITAL LETTER E, COMBINING MACRON, COMBINING GRAVE ACCENT\"");

    err = nfkd_normaliser("\x45" "\xCC\x84" "\xCC\x80", strlen("\x45" "\xCC\x84" "\xCC\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x45" "\xCC\x84" "\xCC\x80", "\x45" "\xCC\x84" "\xCC\x80", out_buffer, "59 NFKD \"(E◌̄◌̀; Ḕ; E◌̄◌̀; Ḕ; E◌̄◌̀; ) LATIN CAPITAL LETTER E, COMBINING MACRON, COMBINING GRAVE ACCENT\"");

    err = nfc_normaliser("\x45" "\xCC\x84" "\xCC\x80", strlen("\x45" "\xCC\x84" "\xCC\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x45" "\xCC\x84" "\xCC\x80", "\xE1\xB8\x94", out_buffer, "60 NFC \"(E◌̄◌̀; Ḕ; E◌̄◌̀; Ḕ; E◌̄◌̀; ) LATIN CAPITAL LETTER E, COMBINING MACRON, COMBINING GRAVE ACCENT\"");

    err = nfd_normaliser("\x45" "\xCC\x80" "\xCC\x84", strlen("\x45" "\xCC\x80" "\xCC\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x45" "\xCC\x80" "\xCC\x84", "\x45" "\xCC\x80" "\xCC\x84", out_buffer, "61 NFD \"(E◌̀◌̄; È◌̄; E◌̀◌̄; È◌̄; E◌̀◌̄; ) LATIN CAPITAL LETTER E, COMBINING GRAVE ACCENT, COMBINING MACRON\"");

    err = nfkd_normaliser("\x45" "\xCC\x80" "\xCC\x84", strlen("\x45" "\xCC\x80" "\xCC\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x45" "\xCC\x80" "\xCC\x84", "\x45" "\xCC\x80" "\xCC\x84", out_buffer, "62 NFKD \"(E◌̀◌̄; È◌̄; E◌̀◌̄; È◌̄; E◌̀◌̄; ) LATIN CAPITAL LETTER E, COMBINING GRAVE ACCENT, COMBINING MACRON\"");

    err = nfc_normaliser("\x45" "\xCC\x80" "\xCC\x84", strlen("\x45" "\xCC\x80" "\xCC\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\x45" "\xCC\x80" "\xCC\x84", "\xC3\x88" "\xCC\x84", out_buffer, "63 NFC \"(E◌̀◌̄; È◌̄; E◌̀◌̄; È◌̄; E◌̀◌̄; ) LATIN CAPITAL LETTER E, COMBINING GRAVE ACCENT, COMBINING MACRON\"");

    err = nfd_normaliser("\xD6\xB8" "\xD6\xB9" "\xD6\xB1" "\xD6\x91" "\xD7\x83" "\xD6\xB0" "\xD6\xAC" "\xD6\x9F", strlen("\xD6\xB8" "\xD6\xB9" "\xD6\xB1" "\xD6\x91" "\xD7\x83" "\xD6\xB0" "\xD6\xAC" "\xD6\x9F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xD6\xB8" "\xD6\xB9" "\xD6\xB1" "\xD6\x91" "\xD7\x83" "\xD6\xB0" "\xD6\xAC" "\xD6\x9F", "\xD6\xB1" "\xD6\xB8" "\xD6\xB9" "\xD6\x91" "\xD7\x83" "\xD6\xB0" "\xD6\xAC" "\xD6\x9F", out_buffer, "64 NFD \"(◌ָ◌ֹ◌ֱ◌֑׃◌ְ◌֬◌֟; ◌ֱ◌ָ◌ֹ◌֑׃◌ְ◌֬◌֟; ◌ֱ◌ָ◌ֹ◌֑׃◌ְ◌֬◌֟; ◌ֱ◌ָ◌ֹ◌֑׃◌ְ◌֬◌֟; ◌ֱ◌ָ◌ֹ◌֑׃◌ְ◌֬◌֟; ) HEBREW POINT QAMATS, HEBREW POINT HOLAM, HEBREW POINT HATAF SEGOL, HEBREW ACCENT ETNAHTA, HEBREW PUNCTUATION SOF PASUQ, HEBREW POINT SHEVA, HEBREW ACCENT ILUY, HEBREW ACCENT QARNEY PARA\"");

    err = nfkd_normaliser("\xD6\xB8" "\xD6\xB9" "\xD6\xB1" "\xD6\x91" "\xD7\x83" "\xD6\xB0" "\xD6\xAC" "\xD6\x9F", strlen("\xD6\xB8" "\xD6\xB9" "\xD6\xB1" "\xD6\x91" "\xD7\x83" "\xD6\xB0" "\xD6\xAC" "\xD6\x9F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xD6\xB8" "\xD6\xB9" "\xD6\xB1" "\xD6\x91" "\xD7\x83" "\xD6\xB0" "\xD6\xAC" "\xD6\x9F", "\xD6\xB1" "\xD6\xB8" "\xD6\xB9" "\xD6\x91" "\xD7\x83" "\xD6\xB0" "\xD6\xAC" "\xD6\x9F", out_buffer, "65 NFKD \"(◌ָ◌ֹ◌ֱ◌֑׃◌ְ◌֬◌֟; ◌ֱ◌ָ◌ֹ◌֑׃◌ְ◌֬◌֟; ◌ֱ◌ָ◌ֹ◌֑׃◌ְ◌֬◌֟; ◌ֱ◌ָ◌ֹ◌֑׃◌ְ◌֬◌֟; ◌ֱ◌ָ◌ֹ◌֑׃◌ְ◌֬◌֟; ) HEBREW POINT QAMATS, HEBREW POINT HOLAM, HEBREW POINT HATAF SEGOL, HEBREW ACCENT ETNAHTA, HEBREW PUNCTUATION SOF PASUQ, HEBREW POINT SHEVA, HEBREW ACCENT ILUY, HEBREW ACCENT QARNEY PARA\"");

    err = nfc_normaliser("\xD6\xB8" "\xD6\xB9" "\xD6\xB1" "\xD6\x91" "\xD7\x83" "\xD6\xB0" "\xD6\xAC" "\xD6\x9F", strlen("\xD6\xB8" "\xD6\xB9" "\xD6\xB1" "\xD6\x91" "\xD7\x83" "\xD6\xB0" "\xD6\xAC" "\xD6\x9F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xD6\xB8" "\xD6\xB9" "\xD6\xB1" "\xD6\x91" "\xD7\x83" "\xD6\xB0" "\xD6\xAC" "\xD6\x9F", "\xD6\xB1" "\xD6\xB8" "\xD6\xB9" "\xD6\x91" "\xD7\x83" "\xD6\xB0" "\xD6\xAC" "\xD6\x9F", out_buffer, "66 NFC \"(◌ָ◌ֹ◌ֱ◌֑׃◌ְ◌֬◌֟; ◌ֱ◌ָ◌ֹ◌֑׃◌ְ◌֬◌֟; ◌ֱ◌ָ◌ֹ◌֑׃◌ְ◌֬◌֟; ◌ֱ◌ָ◌ֹ◌֑׃◌ְ◌֬◌֟; ◌ֱ◌ָ◌ֹ◌֑׃◌ְ◌֬◌֟; ) HEBREW POINT QAMATS, HEBREW POINT HOLAM, HEBREW POINT HATAF SEGOL, HEBREW ACCENT ETNAHTA, HEBREW PUNCTUATION SOF PASUQ, HEBREW POINT SHEVA, HEBREW ACCENT ILUY, HEBREW ACCENT QARNEY PARA\"");

    err = nfd_normaliser("\xD6\x92" "\xD6\xB7" "\xD6\xBC" "\xD6\xA5" "\xD6\xB0" "\xD7\x80" "\xD7\x84" "\xD6\xAD", strlen("\xD6\x92" "\xD6\xB7" "\xD6\xBC" "\xD6\xA5" "\xD6\xB0" "\xD7\x80" "\xD7\x84" "\xD6\xAD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xD6\x92" "\xD6\xB7" "\xD6\xBC" "\xD6\xA5" "\xD6\xB0" "\xD7\x80" "\xD7\x84" "\xD6\xAD", "\xD6\xB0" "\xD6\xB7" "\xD6\xBC" "\xD6\xA5" "\xD6\x92" "\xD7\x80" "\xD6\xAD" "\xD7\x84", out_buffer, "67 NFD \"(◌֒◌ַ◌ּ◌֥◌ְ׀◌ׄ◌֭; ◌ְ◌ַ◌ּ◌֥◌֒׀◌֭◌ׄ; ◌ְ◌ַ◌ּ◌֥◌֒׀◌֭◌ׄ; ◌ְ◌ַ◌ּ◌֥◌֒׀◌֭◌ׄ; ◌ְ◌ַ◌ּ◌֥◌֒׀◌֭◌ׄ; ) HEBREW ACCENT SEGOL, HEBREW POINT PATAH, HEBREW POINT DAGESH OR MAPIQ, HEBREW ACCENT MERKHA, HEBREW POINT SHEVA, HEBREW PUNCTUATION PASEQ, HEBREW MARK UPPER DOT, HEBREW ACCENT DEHI\"");

    err = nfkd_normaliser("\xD6\x92" "\xD6\xB7" "\xD6\xBC" "\xD6\xA5" "\xD6\xB0" "\xD7\x80" "\xD7\x84" "\xD6\xAD", strlen("\xD6\x92" "\xD6\xB7" "\xD6\xBC" "\xD6\xA5" "\xD6\xB0" "\xD7\x80" "\xD7\x84" "\xD6\xAD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xD6\x92" "\xD6\xB7" "\xD6\xBC" "\xD6\xA5" "\xD6\xB0" "\xD7\x80" "\xD7\x84" "\xD6\xAD", "\xD6\xB0" "\xD6\xB7" "\xD6\xBC" "\xD6\xA5" "\xD6\x92" "\xD7\x80" "\xD6\xAD" "\xD7\x84", out_buffer, "68 NFKD \"(◌֒◌ַ◌ּ◌֥◌ְ׀◌ׄ◌֭; ◌ְ◌ַ◌ּ◌֥◌֒׀◌֭◌ׄ; ◌ְ◌ַ◌ּ◌֥◌֒׀◌֭◌ׄ; ◌ְ◌ַ◌ּ◌֥◌֒׀◌֭◌ׄ; ◌ְ◌ַ◌ּ◌֥◌֒׀◌֭◌ׄ; ) HEBREW ACCENT SEGOL, HEBREW POINT PATAH, HEBREW POINT DAGESH OR MAPIQ, HEBREW ACCENT MERKHA, HEBREW POINT SHEVA, HEBREW PUNCTUATION PASEQ, HEBREW MARK UPPER DOT, HEBREW ACCENT DEHI\"");

    err = nfc_normaliser("\xD6\x92" "\xD6\xB7" "\xD6\xBC" "\xD6\xA5" "\xD6\xB0" "\xD7\x80" "\xD7\x84" "\xD6\xAD", strlen("\xD6\x92" "\xD6\xB7" "\xD6\xBC" "\xD6\xA5" "\xD6\xB0" "\xD7\x80" "\xD7\x84" "\xD6\xAD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xD6\x92" "\xD6\xB7" "\xD6\xBC" "\xD6\xA5" "\xD6\xB0" "\xD7\x80" "\xD7\x84" "\xD6\xAD", "\xD6\xB0" "\xD6\xB7" "\xD6\xBC" "\xD6\xA5" "\xD6\x92" "\xD7\x80" "\xD6\xAD" "\xD7\x84", out_buffer, "69 NFC \"(◌֒◌ַ◌ּ◌֥◌ְ׀◌ׄ◌֭; ◌ְ◌ַ◌ּ◌֥◌֒׀◌֭◌ׄ; ◌ְ◌ַ◌ּ◌֥◌֒׀◌֭◌ׄ; ◌ְ◌ַ◌ּ◌֥◌֒׀◌֭◌ׄ; ◌ְ◌ַ◌ּ◌֥◌֒׀◌֭◌ׄ; ) HEBREW ACCENT SEGOL, HEBREW POINT PATAH, HEBREW POINT DAGESH OR MAPIQ, HEBREW ACCENT MERKHA, HEBREW POINT SHEVA, HEBREW PUNCTUATION PASEQ, HEBREW MARK UPPER DOT, HEBREW ACCENT DEHI\"");

    err = nfd_normaliser("\xE1\x84\x80" "\xEA\xB0\x80" "\xE1\x86\xA8", strlen("\xE1\x84\x80" "\xEA\xB0\x80" "\xE1\x86\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\x84\x80" "\xEA\xB0\x80" "\xE1\x86\xA8", "\xE1\x84\x80" "\xE1\x84\x80" "\xE1\x85\xA1" "\xE1\x86\xA8", out_buffer, "70 NFD \"(ᄀ각; ᄀ각; ᄀ각; ᄀ각; ᄀ각; ) HANGUL CHOSEONG KIYEOK, HANGUL SYLLABLE GA, HANGUL JONGSEONG KIYEOK\"");

    err = nfkd_normaliser("\xE1\x84\x80" "\xEA\xB0\x80" "\xE1\x86\xA8", strlen("\xE1\x84\x80" "\xEA\xB0\x80" "\xE1\x86\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\x84\x80" "\xEA\xB0\x80" "\xE1\x86\xA8", "\xE1\x84\x80" "\xE1\x84\x80" "\xE1\x85\xA1" "\xE1\x86\xA8", out_buffer, "71 NFKD \"(ᄀ각; ᄀ각; ᄀ각; ᄀ각; ᄀ각; ) HANGUL CHOSEONG KIYEOK, HANGUL SYLLABLE GA, HANGUL JONGSEONG KIYEOK\"");

    err = nfc_normaliser("\xE1\x84\x80" "\xEA\xB0\x80" "\xE1\x86\xA8", strlen("\xE1\x84\x80" "\xEA\xB0\x80" "\xE1\x86\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\x84\x80" "\xEA\xB0\x80" "\xE1\x86\xA8", "\xE1\x84\x80" "\xEA\xB0\x81", out_buffer, "72 NFC \"(ᄀ각; ᄀ각; ᄀ각; ᄀ각; ᄀ각; ) HANGUL CHOSEONG KIYEOK, HANGUL SYLLABLE GA, HANGUL JONGSEONG KIYEOK\"");

    err = nfd_normaliser("\xE1\x84\x80" "\xEA\xB0\x80" "\xE1\x86\xA8" "\xE1\x86\xA8", strlen("\xE1\x84\x80" "\xEA\xB0\x80" "\xE1\x86\xA8" "\xE1\x86\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\x84\x80" "\xEA\xB0\x80" "\xE1\x86\xA8" "\xE1\x86\xA8", "\xE1\x84\x80" "\xE1\x84\x80" "\xE1\x85\xA1" "\xE1\x86\xA8" "\xE1\x86\xA8", out_buffer, "73 NFD \"(ᄀ각ᆨ; ᄀ각ᆨ; ᄀ각ᆨ; ᄀ각ᆨ; ᄀ각ᆨ; ) HANGUL CHOSEONG KIYEOK, HANGUL SYLLABLE GA, HANGUL JONGSEONG KIYEOK, HANGUL JONGSEONG KIYEOK\"");

    err = nfkd_normaliser("\xE1\x84\x80" "\xEA\xB0\x80" "\xE1\x86\xA8" "\xE1\x86\xA8", strlen("\xE1\x84\x80" "\xEA\xB0\x80" "\xE1\x86\xA8" "\xE1\x86\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\x84\x80" "\xEA\xB0\x80" "\xE1\x86\xA8" "\xE1\x86\xA8", "\xE1\x84\x80" "\xE1\x84\x80" "\xE1\x85\xA1" "\xE1\x86\xA8" "\xE1\x86\xA8", out_buffer, "74 NFKD \"(ᄀ각ᆨ; ᄀ각ᆨ; ᄀ각ᆨ; ᄀ각ᆨ; ᄀ각ᆨ; ) HANGUL CHOSEONG KIYEOK, HANGUL SYLLABLE GA, HANGUL JONGSEONG KIYEOK, HANGUL JONGSEONG KIYEOK\"");

    err = nfc_normaliser("\xE1\x84\x80" "\xEA\xB0\x80" "\xE1\x86\xA8" "\xE1\x86\xA8", strlen("\xE1\x84\x80" "\xEA\xB0\x80" "\xE1\x86\xA8" "\xE1\x86\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xE1\x84\x80" "\xEA\xB0\x80" "\xE1\x86\xA8" "\xE1\x86\xA8", "\xE1\x84\x80" "\xEA\xB0\x81" "\xE1\x86\xA8", out_buffer, "75 NFC \"(ᄀ각ᆨ; ᄀ각ᆨ; ᄀ각ᆨ; ᄀ각ᆨ; ᄀ각ᆨ; ) HANGUL CHOSEONG KIYEOK, HANGUL SYLLABLE GA, HANGUL JONGSEONG KIYEOK, HANGUL JONGSEONG KIYEOK\"");

    err = nfd_normaliser("\xC2\xA0", strlen("\xC2\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xA0", "\xC2\xA0", out_buffer, "76 NFD \"( ;  ;  ;  ;  ; ) NO-BREAK SPACE\"");

    err = nfkd_normaliser("\xC2\xA0", strlen("\xC2\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xA0", "\x20", out_buffer, "77 NFKD \"( ;  ;  ;  ;  ; ) NO-BREAK SPACE\"");

    err = nfc_normaliser("\xC2\xA0", strlen("\xC2\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xA0", "\xC2\xA0", out_buffer, "78 NFC \"( ;  ;  ;  ;  ; ) NO-BREAK SPACE\"");

    err = nfd_normaliser("\xC2\xA8", strlen("\xC2\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xA8", "\xC2\xA8", out_buffer, "79 NFD \"(¨; ¨; ¨;  ◌̈;  ◌̈; ) DIAERESIS\"");

    err = nfkd_normaliser("\xC2\xA8", strlen("\xC2\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xA8", "\x20" "\xCC\x88", out_buffer, "80 NFKD \"(¨; ¨; ¨;  ◌̈;  ◌̈; ) DIAERESIS\"");

    err = nfc_normaliser("\xC2\xA8", strlen("\xC2\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xA8", "\xC2\xA8", out_buffer, "81 NFC \"(¨; ¨; ¨;  ◌̈;  ◌̈; ) DIAERESIS\"");

    err = nfd_normaliser("\xC2\xAA", strlen("\xC2\xAA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xAA", "\xC2\xAA", out_buffer, "82 NFD \"(ª; ª; ª; a; a; ) FEMININE ORDINAL INDICATOR\"");

    err = nfkd_normaliser("\xC2\xAA", strlen("\xC2\xAA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xAA", "\x61", out_buffer, "83 NFKD \"(ª; ª; ª; a; a; ) FEMININE ORDINAL INDICATOR\"");

    err = nfc_normaliser("\xC2\xAA", strlen("\xC2\xAA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xAA", "\xC2\xAA", out_buffer, "84 NFC \"(ª; ª; ª; a; a; ) FEMININE ORDINAL INDICATOR\"");

    err = nfd_normaliser("\xC2\xAF", strlen("\xC2\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xAF", "\xC2\xAF", out_buffer, "85 NFD \"(¯; ¯; ¯;  ◌̄;  ◌̄; ) MACRON\"");

    err = nfkd_normaliser("\xC2\xAF", strlen("\xC2\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xAF", "\x20" "\xCC\x84", out_buffer, "86 NFKD \"(¯; ¯; ¯;  ◌̄;  ◌̄; ) MACRON\"");

    err = nfc_normaliser("\xC2\xAF", strlen("\xC2\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xAF", "\xC2\xAF", out_buffer, "87 NFC \"(¯; ¯; ¯;  ◌̄;  ◌̄; ) MACRON\"");

    err = nfd_normaliser("\xC2\xB2", strlen("\xC2\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xB2", "\xC2\xB2", out_buffer, "88 NFD \"(²; ²; ²; 2; 2; ) SUPERSCRIPT TWO\"");

    err = nfkd_normaliser("\xC2\xB2", strlen("\xC2\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xB2", "\x32", out_buffer, "89 NFKD \"(²; ²; ²; 2; 2; ) SUPERSCRIPT TWO\"");

    err = nfc_normaliser("\xC2\xB2", strlen("\xC2\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xB2", "\xC2\xB2", out_buffer, "90 NFC \"(²; ²; ²; 2; 2; ) SUPERSCRIPT TWO\"");

    err = nfd_normaliser("\xC2\xB3", strlen("\xC2\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xB3", "\xC2\xB3", out_buffer, "91 NFD \"(³; ³; ³; 3; 3; ) SUPERSCRIPT THREE\"");

    err = nfkd_normaliser("\xC2\xB3", strlen("\xC2\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xB3", "\x33", out_buffer, "92 NFKD \"(³; ³; ³; 3; 3; ) SUPERSCRIPT THREE\"");

    err = nfc_normaliser("\xC2\xB3", strlen("\xC2\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xB3", "\xC2\xB3", out_buffer, "93 NFC \"(³; ³; ³; 3; 3; ) SUPERSCRIPT THREE\"");

    err = nfd_normaliser("\xC2\xB4", strlen("\xC2\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xB4", "\xC2\xB4", out_buffer, "94 NFD \"(´; ´; ´;  ◌́;  ◌́; ) ACUTE ACCENT\"");

    err = nfkd_normaliser("\xC2\xB4", strlen("\xC2\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xB4", "\x20" "\xCC\x81", out_buffer, "95 NFKD \"(´; ´; ´;  ◌́;  ◌́; ) ACUTE ACCENT\"");

    err = nfc_normaliser("\xC2\xB4", strlen("\xC2\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xB4", "\xC2\xB4", out_buffer, "96 NFC \"(´; ´; ´;  ◌́;  ◌́; ) ACUTE ACCENT\"");

    err = nfd_normaliser("\xC2\xB5", strlen("\xC2\xB5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xB5", "\xC2\xB5", out_buffer, "97 NFD \"(µ; µ; µ; μ; μ; ) MICRO SIGN\"");

    err = nfkd_normaliser("\xC2\xB5", strlen("\xC2\xB5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xB5", "\xCE\xBC", out_buffer, "98 NFKD \"(µ; µ; µ; μ; μ; ) MICRO SIGN\"");

    err = nfc_normaliser("\xC2\xB5", strlen("\xC2\xB5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xB5", "\xC2\xB5", out_buffer, "99 NFC \"(µ; µ; µ; μ; μ; ) MICRO SIGN\"");

    err = nfd_normaliser("\xC2\xB8", strlen("\xC2\xB8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xB8", "\xC2\xB8", out_buffer, "100 NFD \"(¸; ¸; ¸;  ◌̧;  ◌̧; ) CEDILLA\"");

    err = nfkd_normaliser("\xC2\xB8", strlen("\xC2\xB8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xB8", "\x20" "\xCC\xA7", out_buffer, "101 NFKD \"(¸; ¸; ¸;  ◌̧;  ◌̧; ) CEDILLA\"");

    err = nfc_normaliser("\xC2\xB8", strlen("\xC2\xB8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xB8", "\xC2\xB8", out_buffer, "102 NFC \"(¸; ¸; ¸;  ◌̧;  ◌̧; ) CEDILLA\"");

    err = nfd_normaliser("\xC2\xB9", strlen("\xC2\xB9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xB9", "\xC2\xB9", out_buffer, "103 NFD \"(¹; ¹; ¹; 1; 1; ) SUPERSCRIPT ONE\"");

    err = nfkd_normaliser("\xC2\xB9", strlen("\xC2\xB9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xB9", "\x31", out_buffer, "104 NFKD \"(¹; ¹; ¹; 1; 1; ) SUPERSCRIPT ONE\"");

    err = nfc_normaliser("\xC2\xB9", strlen("\xC2\xB9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xB9", "\xC2\xB9", out_buffer, "105 NFC \"(¹; ¹; ¹; 1; 1; ) SUPERSCRIPT ONE\"");

    err = nfd_normaliser("\xC2\xBA", strlen("\xC2\xBA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xBA", "\xC2\xBA", out_buffer, "106 NFD \"(º; º; º; o; o; ) MASCULINE ORDINAL INDICATOR\"");

    err = nfkd_normaliser("\xC2\xBA", strlen("\xC2\xBA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xBA", "\x6F", out_buffer, "107 NFKD \"(º; º; º; o; o; ) MASCULINE ORDINAL INDICATOR\"");

    err = nfc_normaliser("\xC2\xBA", strlen("\xC2\xBA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xBA", "\xC2\xBA", out_buffer, "108 NFC \"(º; º; º; o; o; ) MASCULINE ORDINAL INDICATOR\"");

    err = nfd_normaliser("\xC2\xBC", strlen("\xC2\xBC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xBC", "\xC2\xBC", out_buffer, "109 NFD \"(¼; ¼; ¼; 1⁄4; 1⁄4; ) VULGAR FRACTION ONE QUARTER\"");

    err = nfkd_normaliser("\xC2\xBC", strlen("\xC2\xBC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xBC", "\x31" "\xE2\x81\x84" "\x34", out_buffer, "110 NFKD \"(¼; ¼; ¼; 1⁄4; 1⁄4; ) VULGAR FRACTION ONE QUARTER\"");

    err = nfc_normaliser("\xC2\xBC", strlen("\xC2\xBC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xBC", "\xC2\xBC", out_buffer, "111 NFC \"(¼; ¼; ¼; 1⁄4; 1⁄4; ) VULGAR FRACTION ONE QUARTER\"");

    err = nfd_normaliser("\xC2\xBD", strlen("\xC2\xBD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xBD", "\xC2\xBD", out_buffer, "112 NFD \"(½; ½; ½; 1⁄2; 1⁄2; ) VULGAR FRACTION ONE HALF\"");

    err = nfkd_normaliser("\xC2\xBD", strlen("\xC2\xBD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xBD", "\x31" "\xE2\x81\x84" "\x32", out_buffer, "113 NFKD \"(½; ½; ½; 1⁄2; 1⁄2; ) VULGAR FRACTION ONE HALF\"");

    err = nfc_normaliser("\xC2\xBD", strlen("\xC2\xBD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xBD", "\xC2\xBD", out_buffer, "114 NFC \"(½; ½; ½; 1⁄2; 1⁄2; ) VULGAR FRACTION ONE HALF\"");

    err = nfd_normaliser("\xC2\xBE", strlen("\xC2\xBE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xBE", "\xC2\xBE", out_buffer, "115 NFD \"(¾; ¾; ¾; 3⁄4; 3⁄4; ) VULGAR FRACTION THREE QUARTERS\"");

    err = nfkd_normaliser("\xC2\xBE", strlen("\xC2\xBE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xBE", "\x33" "\xE2\x81\x84" "\x34", out_buffer, "116 NFKD \"(¾; ¾; ¾; 3⁄4; 3⁄4; ) VULGAR FRACTION THREE QUARTERS\"");

    err = nfc_normaliser("\xC2\xBE", strlen("\xC2\xBE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC2\xBE", "\xC2\xBE", out_buffer, "117 NFC \"(¾; ¾; ¾; 3⁄4; 3⁄4; ) VULGAR FRACTION THREE QUARTERS\"");

    err = nfd_normaliser("\xC3\x80", strlen("\xC3\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x80", "\x41" "\xCC\x80", out_buffer, "118 NFD \"(À; À; A◌̀; À; A◌̀; ) LATIN CAPITAL LETTER A WITH GRAVE\"");

    err = nfkd_normaliser("\xC3\x80", strlen("\xC3\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x80", "\x41" "\xCC\x80", out_buffer, "119 NFKD \"(À; À; A◌̀; À; A◌̀; ) LATIN CAPITAL LETTER A WITH GRAVE\"");

    err = nfc_normaliser("\xC3\x80", strlen("\xC3\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x80", "\xC3\x80", out_buffer, "120 NFC \"(À; À; A◌̀; À; A◌̀; ) LATIN CAPITAL LETTER A WITH GRAVE\"");

    err = nfd_normaliser("\xC3\x81", strlen("\xC3\x81"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x81", "\x41" "\xCC\x81", out_buffer, "121 NFD \"(Á; Á; A◌́; Á; A◌́; ) LATIN CAPITAL LETTER A WITH ACUTE\"");

    err = nfkd_normaliser("\xC3\x81", strlen("\xC3\x81"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x81", "\x41" "\xCC\x81", out_buffer, "122 NFKD \"(Á; Á; A◌́; Á; A◌́; ) LATIN CAPITAL LETTER A WITH ACUTE\"");

    err = nfc_normaliser("\xC3\x81", strlen("\xC3\x81"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x81", "\xC3\x81", out_buffer, "123 NFC \"(Á; Á; A◌́; Á; A◌́; ) LATIN CAPITAL LETTER A WITH ACUTE\"");

    err = nfd_normaliser("\xC3\x82", strlen("\xC3\x82"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x82", "\x41" "\xCC\x82", out_buffer, "124 NFD \"(Â; Â; A◌̂; Â; A◌̂; ) LATIN CAPITAL LETTER A WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC3\x82", strlen("\xC3\x82"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x82", "\x41" "\xCC\x82", out_buffer, "125 NFKD \"(Â; Â; A◌̂; Â; A◌̂; ) LATIN CAPITAL LETTER A WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC3\x82", strlen("\xC3\x82"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x82", "\xC3\x82", out_buffer, "126 NFC \"(Â; Â; A◌̂; Â; A◌̂; ) LATIN CAPITAL LETTER A WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC3\x83", strlen("\xC3\x83"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x83", "\x41" "\xCC\x83", out_buffer, "127 NFD \"(Ã; Ã; A◌̃; Ã; A◌̃; ) LATIN CAPITAL LETTER A WITH TILDE\"");

    err = nfkd_normaliser("\xC3\x83", strlen("\xC3\x83"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x83", "\x41" "\xCC\x83", out_buffer, "128 NFKD \"(Ã; Ã; A◌̃; Ã; A◌̃; ) LATIN CAPITAL LETTER A WITH TILDE\"");

    err = nfc_normaliser("\xC3\x83", strlen("\xC3\x83"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x83", "\xC3\x83", out_buffer, "129 NFC \"(Ã; Ã; A◌̃; Ã; A◌̃; ) LATIN CAPITAL LETTER A WITH TILDE\"");

    err = nfd_normaliser("\xC3\x84", strlen("\xC3\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x84", "\x41" "\xCC\x88", out_buffer, "130 NFD \"(Ä; Ä; A◌̈; Ä; A◌̈; ) LATIN CAPITAL LETTER A WITH DIAERESIS\"");

    err = nfkd_normaliser("\xC3\x84", strlen("\xC3\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x84", "\x41" "\xCC\x88", out_buffer, "131 NFKD \"(Ä; Ä; A◌̈; Ä; A◌̈; ) LATIN CAPITAL LETTER A WITH DIAERESIS\"");

    err = nfc_normaliser("\xC3\x84", strlen("\xC3\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x84", "\xC3\x84", out_buffer, "132 NFC \"(Ä; Ä; A◌̈; Ä; A◌̈; ) LATIN CAPITAL LETTER A WITH DIAERESIS\"");

    err = nfd_normaliser("\xC3\x85", strlen("\xC3\x85"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x85", "\x41" "\xCC\x8A", out_buffer, "133 NFD \"(Å; Å; A◌̊; Å; A◌̊; ) LATIN CAPITAL LETTER A WITH RING ABOVE\"");

    err = nfkd_normaliser("\xC3\x85", strlen("\xC3\x85"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x85", "\x41" "\xCC\x8A", out_buffer, "134 NFKD \"(Å; Å; A◌̊; Å; A◌̊; ) LATIN CAPITAL LETTER A WITH RING ABOVE\"");

    err = nfc_normaliser("\xC3\x85", strlen("\xC3\x85"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x85", "\xC3\x85", out_buffer, "135 NFC \"(Å; Å; A◌̊; Å; A◌̊; ) LATIN CAPITAL LETTER A WITH RING ABOVE\"");

    err = nfd_normaliser("\xC3\x87", strlen("\xC3\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x87", "\x43" "\xCC\xA7", out_buffer, "136 NFD \"(Ç; Ç; C◌̧; Ç; C◌̧; ) LATIN CAPITAL LETTER C WITH CEDILLA\"");

    err = nfkd_normaliser("\xC3\x87", strlen("\xC3\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x87", "\x43" "\xCC\xA7", out_buffer, "137 NFKD \"(Ç; Ç; C◌̧; Ç; C◌̧; ) LATIN CAPITAL LETTER C WITH CEDILLA\"");

    err = nfc_normaliser("\xC3\x87", strlen("\xC3\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x87", "\xC3\x87", out_buffer, "138 NFC \"(Ç; Ç; C◌̧; Ç; C◌̧; ) LATIN CAPITAL LETTER C WITH CEDILLA\"");

    err = nfd_normaliser("\xC3\x88", strlen("\xC3\x88"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x88", "\x45" "\xCC\x80", out_buffer, "139 NFD \"(È; È; E◌̀; È; E◌̀; ) LATIN CAPITAL LETTER E WITH GRAVE\"");

    err = nfkd_normaliser("\xC3\x88", strlen("\xC3\x88"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x88", "\x45" "\xCC\x80", out_buffer, "140 NFKD \"(È; È; E◌̀; È; E◌̀; ) LATIN CAPITAL LETTER E WITH GRAVE\"");

    err = nfc_normaliser("\xC3\x88", strlen("\xC3\x88"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x88", "\xC3\x88", out_buffer, "141 NFC \"(È; È; E◌̀; È; E◌̀; ) LATIN CAPITAL LETTER E WITH GRAVE\"");

    err = nfd_normaliser("\xC3\x89", strlen("\xC3\x89"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x89", "\x45" "\xCC\x81", out_buffer, "142 NFD \"(É; É; E◌́; É; E◌́; ) LATIN CAPITAL LETTER E WITH ACUTE\"");

    err = nfkd_normaliser("\xC3\x89", strlen("\xC3\x89"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x89", "\x45" "\xCC\x81", out_buffer, "143 NFKD \"(É; É; E◌́; É; E◌́; ) LATIN CAPITAL LETTER E WITH ACUTE\"");

    err = nfc_normaliser("\xC3\x89", strlen("\xC3\x89"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x89", "\xC3\x89", out_buffer, "144 NFC \"(É; É; E◌́; É; E◌́; ) LATIN CAPITAL LETTER E WITH ACUTE\"");

    err = nfd_normaliser("\xC3\x8A", strlen("\xC3\x8A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x8A", "\x45" "\xCC\x82", out_buffer, "145 NFD \"(Ê; Ê; E◌̂; Ê; E◌̂; ) LATIN CAPITAL LETTER E WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC3\x8A", strlen("\xC3\x8A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x8A", "\x45" "\xCC\x82", out_buffer, "146 NFKD \"(Ê; Ê; E◌̂; Ê; E◌̂; ) LATIN CAPITAL LETTER E WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC3\x8A", strlen("\xC3\x8A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x8A", "\xC3\x8A", out_buffer, "147 NFC \"(Ê; Ê; E◌̂; Ê; E◌̂; ) LATIN CAPITAL LETTER E WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC3\x8B", strlen("\xC3\x8B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x8B", "\x45" "\xCC\x88", out_buffer, "148 NFD \"(Ë; Ë; E◌̈; Ë; E◌̈; ) LATIN CAPITAL LETTER E WITH DIAERESIS\"");

    err = nfkd_normaliser("\xC3\x8B", strlen("\xC3\x8B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x8B", "\x45" "\xCC\x88", out_buffer, "149 NFKD \"(Ë; Ë; E◌̈; Ë; E◌̈; ) LATIN CAPITAL LETTER E WITH DIAERESIS\"");

    err = nfc_normaliser("\xC3\x8B", strlen("\xC3\x8B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x8B", "\xC3\x8B", out_buffer, "150 NFC \"(Ë; Ë; E◌̈; Ë; E◌̈; ) LATIN CAPITAL LETTER E WITH DIAERESIS\"");

    err = nfd_normaliser("\xC3\x8C", strlen("\xC3\x8C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x8C", "\x49" "\xCC\x80", out_buffer, "151 NFD \"(Ì; Ì; I◌̀; Ì; I◌̀; ) LATIN CAPITAL LETTER I WITH GRAVE\"");

    err = nfkd_normaliser("\xC3\x8C", strlen("\xC3\x8C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x8C", "\x49" "\xCC\x80", out_buffer, "152 NFKD \"(Ì; Ì; I◌̀; Ì; I◌̀; ) LATIN CAPITAL LETTER I WITH GRAVE\"");

    err = nfc_normaliser("\xC3\x8C", strlen("\xC3\x8C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x8C", "\xC3\x8C", out_buffer, "153 NFC \"(Ì; Ì; I◌̀; Ì; I◌̀; ) LATIN CAPITAL LETTER I WITH GRAVE\"");

    err = nfd_normaliser("\xC3\x8D", strlen("\xC3\x8D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x8D", "\x49" "\xCC\x81", out_buffer, "154 NFD \"(Í; Í; I◌́; Í; I◌́; ) LATIN CAPITAL LETTER I WITH ACUTE\"");

    err = nfkd_normaliser("\xC3\x8D", strlen("\xC3\x8D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x8D", "\x49" "\xCC\x81", out_buffer, "155 NFKD \"(Í; Í; I◌́; Í; I◌́; ) LATIN CAPITAL LETTER I WITH ACUTE\"");

    err = nfc_normaliser("\xC3\x8D", strlen("\xC3\x8D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x8D", "\xC3\x8D", out_buffer, "156 NFC \"(Í; Í; I◌́; Í; I◌́; ) LATIN CAPITAL LETTER I WITH ACUTE\"");

    err = nfd_normaliser("\xC3\x8E", strlen("\xC3\x8E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x8E", "\x49" "\xCC\x82", out_buffer, "157 NFD \"(Î; Î; I◌̂; Î; I◌̂; ) LATIN CAPITAL LETTER I WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC3\x8E", strlen("\xC3\x8E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x8E", "\x49" "\xCC\x82", out_buffer, "158 NFKD \"(Î; Î; I◌̂; Î; I◌̂; ) LATIN CAPITAL LETTER I WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC3\x8E", strlen("\xC3\x8E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x8E", "\xC3\x8E", out_buffer, "159 NFC \"(Î; Î; I◌̂; Î; I◌̂; ) LATIN CAPITAL LETTER I WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC3\x8F", strlen("\xC3\x8F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x8F", "\x49" "\xCC\x88", out_buffer, "160 NFD \"(Ï; Ï; I◌̈; Ï; I◌̈; ) LATIN CAPITAL LETTER I WITH DIAERESIS\"");

    err = nfkd_normaliser("\xC3\x8F", strlen("\xC3\x8F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x8F", "\x49" "\xCC\x88", out_buffer, "161 NFKD \"(Ï; Ï; I◌̈; Ï; I◌̈; ) LATIN CAPITAL LETTER I WITH DIAERESIS\"");

    err = nfc_normaliser("\xC3\x8F", strlen("\xC3\x8F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x8F", "\xC3\x8F", out_buffer, "162 NFC \"(Ï; Ï; I◌̈; Ï; I◌̈; ) LATIN CAPITAL LETTER I WITH DIAERESIS\"");

    err = nfd_normaliser("\xC3\x91", strlen("\xC3\x91"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x91", "\x4E" "\xCC\x83", out_buffer, "163 NFD \"(Ñ; Ñ; N◌̃; Ñ; N◌̃; ) LATIN CAPITAL LETTER N WITH TILDE\"");

    err = nfkd_normaliser("\xC3\x91", strlen("\xC3\x91"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x91", "\x4E" "\xCC\x83", out_buffer, "164 NFKD \"(Ñ; Ñ; N◌̃; Ñ; N◌̃; ) LATIN CAPITAL LETTER N WITH TILDE\"");

    err = nfc_normaliser("\xC3\x91", strlen("\xC3\x91"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x91", "\xC3\x91", out_buffer, "165 NFC \"(Ñ; Ñ; N◌̃; Ñ; N◌̃; ) LATIN CAPITAL LETTER N WITH TILDE\"");

    err = nfd_normaliser("\xC3\x92", strlen("\xC3\x92"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x92", "\x4F" "\xCC\x80", out_buffer, "166 NFD \"(Ò; Ò; O◌̀; Ò; O◌̀; ) LATIN CAPITAL LETTER O WITH GRAVE\"");

    err = nfkd_normaliser("\xC3\x92", strlen("\xC3\x92"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x92", "\x4F" "\xCC\x80", out_buffer, "167 NFKD \"(Ò; Ò; O◌̀; Ò; O◌̀; ) LATIN CAPITAL LETTER O WITH GRAVE\"");

    err = nfc_normaliser("\xC3\x92", strlen("\xC3\x92"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x92", "\xC3\x92", out_buffer, "168 NFC \"(Ò; Ò; O◌̀; Ò; O◌̀; ) LATIN CAPITAL LETTER O WITH GRAVE\"");

    err = nfd_normaliser("\xC3\x93", strlen("\xC3\x93"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x93", "\x4F" "\xCC\x81", out_buffer, "169 NFD \"(Ó; Ó; O◌́; Ó; O◌́; ) LATIN CAPITAL LETTER O WITH ACUTE\"");

    err = nfkd_normaliser("\xC3\x93", strlen("\xC3\x93"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x93", "\x4F" "\xCC\x81", out_buffer, "170 NFKD \"(Ó; Ó; O◌́; Ó; O◌́; ) LATIN CAPITAL LETTER O WITH ACUTE\"");

    err = nfc_normaliser("\xC3\x93", strlen("\xC3\x93"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x93", "\xC3\x93", out_buffer, "171 NFC \"(Ó; Ó; O◌́; Ó; O◌́; ) LATIN CAPITAL LETTER O WITH ACUTE\"");

    err = nfd_normaliser("\xC3\x94", strlen("\xC3\x94"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x94", "\x4F" "\xCC\x82", out_buffer, "172 NFD \"(Ô; Ô; O◌̂; Ô; O◌̂; ) LATIN CAPITAL LETTER O WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC3\x94", strlen("\xC3\x94"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x94", "\x4F" "\xCC\x82", out_buffer, "173 NFKD \"(Ô; Ô; O◌̂; Ô; O◌̂; ) LATIN CAPITAL LETTER O WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC3\x94", strlen("\xC3\x94"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x94", "\xC3\x94", out_buffer, "174 NFC \"(Ô; Ô; O◌̂; Ô; O◌̂; ) LATIN CAPITAL LETTER O WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC3\x95", strlen("\xC3\x95"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x95", "\x4F" "\xCC\x83", out_buffer, "175 NFD \"(Õ; Õ; O◌̃; Õ; O◌̃; ) LATIN CAPITAL LETTER O WITH TILDE\"");

    err = nfkd_normaliser("\xC3\x95", strlen("\xC3\x95"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x95", "\x4F" "\xCC\x83", out_buffer, "176 NFKD \"(Õ; Õ; O◌̃; Õ; O◌̃; ) LATIN CAPITAL LETTER O WITH TILDE\"");

    err = nfc_normaliser("\xC3\x95", strlen("\xC3\x95"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x95", "\xC3\x95", out_buffer, "177 NFC \"(Õ; Õ; O◌̃; Õ; O◌̃; ) LATIN CAPITAL LETTER O WITH TILDE\"");

    err = nfd_normaliser("\xC3\x96", strlen("\xC3\x96"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x96", "\x4F" "\xCC\x88", out_buffer, "178 NFD \"(Ö; Ö; O◌̈; Ö; O◌̈; ) LATIN CAPITAL LETTER O WITH DIAERESIS\"");

    err = nfkd_normaliser("\xC3\x96", strlen("\xC3\x96"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x96", "\x4F" "\xCC\x88", out_buffer, "179 NFKD \"(Ö; Ö; O◌̈; Ö; O◌̈; ) LATIN CAPITAL LETTER O WITH DIAERESIS\"");

    err = nfc_normaliser("\xC3\x96", strlen("\xC3\x96"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x96", "\xC3\x96", out_buffer, "180 NFC \"(Ö; Ö; O◌̈; Ö; O◌̈; ) LATIN CAPITAL LETTER O WITH DIAERESIS\"");

    err = nfd_normaliser("\xC3\x99", strlen("\xC3\x99"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x99", "\x55" "\xCC\x80", out_buffer, "181 NFD \"(Ù; Ù; U◌̀; Ù; U◌̀; ) LATIN CAPITAL LETTER U WITH GRAVE\"");

    err = nfkd_normaliser("\xC3\x99", strlen("\xC3\x99"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x99", "\x55" "\xCC\x80", out_buffer, "182 NFKD \"(Ù; Ù; U◌̀; Ù; U◌̀; ) LATIN CAPITAL LETTER U WITH GRAVE\"");

    err = nfc_normaliser("\xC3\x99", strlen("\xC3\x99"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x99", "\xC3\x99", out_buffer, "183 NFC \"(Ù; Ù; U◌̀; Ù; U◌̀; ) LATIN CAPITAL LETTER U WITH GRAVE\"");

    err = nfd_normaliser("\xC3\x9A", strlen("\xC3\x9A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x9A", "\x55" "\xCC\x81", out_buffer, "184 NFD \"(Ú; Ú; U◌́; Ú; U◌́; ) LATIN CAPITAL LETTER U WITH ACUTE\"");

    err = nfkd_normaliser("\xC3\x9A", strlen("\xC3\x9A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x9A", "\x55" "\xCC\x81", out_buffer, "185 NFKD \"(Ú; Ú; U◌́; Ú; U◌́; ) LATIN CAPITAL LETTER U WITH ACUTE\"");

    err = nfc_normaliser("\xC3\x9A", strlen("\xC3\x9A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x9A", "\xC3\x9A", out_buffer, "186 NFC \"(Ú; Ú; U◌́; Ú; U◌́; ) LATIN CAPITAL LETTER U WITH ACUTE\"");

    err = nfd_normaliser("\xC3\x9B", strlen("\xC3\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x9B", "\x55" "\xCC\x82", out_buffer, "187 NFD \"(Û; Û; U◌̂; Û; U◌̂; ) LATIN CAPITAL LETTER U WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC3\x9B", strlen("\xC3\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x9B", "\x55" "\xCC\x82", out_buffer, "188 NFKD \"(Û; Û; U◌̂; Û; U◌̂; ) LATIN CAPITAL LETTER U WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC3\x9B", strlen("\xC3\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x9B", "\xC3\x9B", out_buffer, "189 NFC \"(Û; Û; U◌̂; Û; U◌̂; ) LATIN CAPITAL LETTER U WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC3\x9C", strlen("\xC3\x9C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x9C", "\x55" "\xCC\x88", out_buffer, "190 NFD \"(Ü; Ü; U◌̈; Ü; U◌̈; ) LATIN CAPITAL LETTER U WITH DIAERESIS\"");

    err = nfkd_normaliser("\xC3\x9C", strlen("\xC3\x9C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x9C", "\x55" "\xCC\x88", out_buffer, "191 NFKD \"(Ü; Ü; U◌̈; Ü; U◌̈; ) LATIN CAPITAL LETTER U WITH DIAERESIS\"");

    err = nfc_normaliser("\xC3\x9C", strlen("\xC3\x9C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x9C", "\xC3\x9C", out_buffer, "192 NFC \"(Ü; Ü; U◌̈; Ü; U◌̈; ) LATIN CAPITAL LETTER U WITH DIAERESIS\"");

    err = nfd_normaliser("\xC3\x9D", strlen("\xC3\x9D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x9D", "\x59" "\xCC\x81", out_buffer, "193 NFD \"(Ý; Ý; Y◌́; Ý; Y◌́; ) LATIN CAPITAL LETTER Y WITH ACUTE\"");

    err = nfkd_normaliser("\xC3\x9D", strlen("\xC3\x9D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x9D", "\x59" "\xCC\x81", out_buffer, "194 NFKD \"(Ý; Ý; Y◌́; Ý; Y◌́; ) LATIN CAPITAL LETTER Y WITH ACUTE\"");

    err = nfc_normaliser("\xC3\x9D", strlen("\xC3\x9D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\x9D", "\xC3\x9D", out_buffer, "195 NFC \"(Ý; Ý; Y◌́; Ý; Y◌́; ) LATIN CAPITAL LETTER Y WITH ACUTE\"");

    err = nfd_normaliser("\xC3\xA0", strlen("\xC3\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA0", "\x61" "\xCC\x80", out_buffer, "196 NFD \"(à; à; a◌̀; à; a◌̀; ) LATIN SMALL LETTER A WITH GRAVE\"");

    err = nfkd_normaliser("\xC3\xA0", strlen("\xC3\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA0", "\x61" "\xCC\x80", out_buffer, "197 NFKD \"(à; à; a◌̀; à; a◌̀; ) LATIN SMALL LETTER A WITH GRAVE\"");

    err = nfc_normaliser("\xC3\xA0", strlen("\xC3\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA0", "\xC3\xA0", out_buffer, "198 NFC \"(à; à; a◌̀; à; a◌̀; ) LATIN SMALL LETTER A WITH GRAVE\"");

    err = nfd_normaliser("\xC3\xA1", strlen("\xC3\xA1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA1", "\x61" "\xCC\x81", out_buffer, "199 NFD \"(á; á; a◌́; á; a◌́; ) LATIN SMALL LETTER A WITH ACUTE\"");

    err = nfkd_normaliser("\xC3\xA1", strlen("\xC3\xA1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA1", "\x61" "\xCC\x81", out_buffer, "200 NFKD \"(á; á; a◌́; á; a◌́; ) LATIN SMALL LETTER A WITH ACUTE\"");

    err = nfc_normaliser("\xC3\xA1", strlen("\xC3\xA1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA1", "\xC3\xA1", out_buffer, "201 NFC \"(á; á; a◌́; á; a◌́; ) LATIN SMALL LETTER A WITH ACUTE\"");

    err = nfd_normaliser("\xC3\xA2", strlen("\xC3\xA2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA2", "\x61" "\xCC\x82", out_buffer, "202 NFD \"(â; â; a◌̂; â; a◌̂; ) LATIN SMALL LETTER A WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC3\xA2", strlen("\xC3\xA2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA2", "\x61" "\xCC\x82", out_buffer, "203 NFKD \"(â; â; a◌̂; â; a◌̂; ) LATIN SMALL LETTER A WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC3\xA2", strlen("\xC3\xA2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA2", "\xC3\xA2", out_buffer, "204 NFC \"(â; â; a◌̂; â; a◌̂; ) LATIN SMALL LETTER A WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC3\xA3", strlen("\xC3\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA3", "\x61" "\xCC\x83", out_buffer, "205 NFD \"(ã; ã; a◌̃; ã; a◌̃; ) LATIN SMALL LETTER A WITH TILDE\"");

    err = nfkd_normaliser("\xC3\xA3", strlen("\xC3\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA3", "\x61" "\xCC\x83", out_buffer, "206 NFKD \"(ã; ã; a◌̃; ã; a◌̃; ) LATIN SMALL LETTER A WITH TILDE\"");

    err = nfc_normaliser("\xC3\xA3", strlen("\xC3\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA3", "\xC3\xA3", out_buffer, "207 NFC \"(ã; ã; a◌̃; ã; a◌̃; ) LATIN SMALL LETTER A WITH TILDE\"");

    err = nfd_normaliser("\xC3\xA4", strlen("\xC3\xA4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA4", "\x61" "\xCC\x88", out_buffer, "208 NFD \"(ä; ä; a◌̈; ä; a◌̈; ) LATIN SMALL LETTER A WITH DIAERESIS\"");

    err = nfkd_normaliser("\xC3\xA4", strlen("\xC3\xA4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA4", "\x61" "\xCC\x88", out_buffer, "209 NFKD \"(ä; ä; a◌̈; ä; a◌̈; ) LATIN SMALL LETTER A WITH DIAERESIS\"");

    err = nfc_normaliser("\xC3\xA4", strlen("\xC3\xA4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA4", "\xC3\xA4", out_buffer, "210 NFC \"(ä; ä; a◌̈; ä; a◌̈; ) LATIN SMALL LETTER A WITH DIAERESIS\"");

    err = nfd_normaliser("\xC3\xA5", strlen("\xC3\xA5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA5", "\x61" "\xCC\x8A", out_buffer, "211 NFD \"(å; å; a◌̊; å; a◌̊; ) LATIN SMALL LETTER A WITH RING ABOVE\"");

    err = nfkd_normaliser("\xC3\xA5", strlen("\xC3\xA5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA5", "\x61" "\xCC\x8A", out_buffer, "212 NFKD \"(å; å; a◌̊; å; a◌̊; ) LATIN SMALL LETTER A WITH RING ABOVE\"");

    err = nfc_normaliser("\xC3\xA5", strlen("\xC3\xA5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA5", "\xC3\xA5", out_buffer, "213 NFC \"(å; å; a◌̊; å; a◌̊; ) LATIN SMALL LETTER A WITH RING ABOVE\"");

    err = nfd_normaliser("\xC3\xA7", strlen("\xC3\xA7"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA7", "\x63" "\xCC\xA7", out_buffer, "214 NFD \"(ç; ç; c◌̧; ç; c◌̧; ) LATIN SMALL LETTER C WITH CEDILLA\"");

    err = nfkd_normaliser("\xC3\xA7", strlen("\xC3\xA7"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA7", "\x63" "\xCC\xA7", out_buffer, "215 NFKD \"(ç; ç; c◌̧; ç; c◌̧; ) LATIN SMALL LETTER C WITH CEDILLA\"");

    err = nfc_normaliser("\xC3\xA7", strlen("\xC3\xA7"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA7", "\xC3\xA7", out_buffer, "216 NFC \"(ç; ç; c◌̧; ç; c◌̧; ) LATIN SMALL LETTER C WITH CEDILLA\"");

    err = nfd_normaliser("\xC3\xA8", strlen("\xC3\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA8", "\x65" "\xCC\x80", out_buffer, "217 NFD \"(è; è; e◌̀; è; e◌̀; ) LATIN SMALL LETTER E WITH GRAVE\"");

    err = nfkd_normaliser("\xC3\xA8", strlen("\xC3\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA8", "\x65" "\xCC\x80", out_buffer, "218 NFKD \"(è; è; e◌̀; è; e◌̀; ) LATIN SMALL LETTER E WITH GRAVE\"");

    err = nfc_normaliser("\xC3\xA8", strlen("\xC3\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA8", "\xC3\xA8", out_buffer, "219 NFC \"(è; è; e◌̀; è; e◌̀; ) LATIN SMALL LETTER E WITH GRAVE\"");

    err = nfd_normaliser("\xC3\xA9", strlen("\xC3\xA9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA9", "\x65" "\xCC\x81", out_buffer, "220 NFD \"(é; é; e◌́; é; e◌́; ) LATIN SMALL LETTER E WITH ACUTE\"");

    err = nfkd_normaliser("\xC3\xA9", strlen("\xC3\xA9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA9", "\x65" "\xCC\x81", out_buffer, "221 NFKD \"(é; é; e◌́; é; e◌́; ) LATIN SMALL LETTER E WITH ACUTE\"");

    err = nfc_normaliser("\xC3\xA9", strlen("\xC3\xA9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xA9", "\xC3\xA9", out_buffer, "222 NFC \"(é; é; e◌́; é; e◌́; ) LATIN SMALL LETTER E WITH ACUTE\"");

    err = nfd_normaliser("\xC3\xAA", strlen("\xC3\xAA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xAA", "\x65" "\xCC\x82", out_buffer, "223 NFD \"(ê; ê; e◌̂; ê; e◌̂; ) LATIN SMALL LETTER E WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC3\xAA", strlen("\xC3\xAA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xAA", "\x65" "\xCC\x82", out_buffer, "224 NFKD \"(ê; ê; e◌̂; ê; e◌̂; ) LATIN SMALL LETTER E WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC3\xAA", strlen("\xC3\xAA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xAA", "\xC3\xAA", out_buffer, "225 NFC \"(ê; ê; e◌̂; ê; e◌̂; ) LATIN SMALL LETTER E WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC3\xAB", strlen("\xC3\xAB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xAB", "\x65" "\xCC\x88", out_buffer, "226 NFD \"(ë; ë; e◌̈; ë; e◌̈; ) LATIN SMALL LETTER E WITH DIAERESIS\"");

    err = nfkd_normaliser("\xC3\xAB", strlen("\xC3\xAB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xAB", "\x65" "\xCC\x88", out_buffer, "227 NFKD \"(ë; ë; e◌̈; ë; e◌̈; ) LATIN SMALL LETTER E WITH DIAERESIS\"");

    err = nfc_normaliser("\xC3\xAB", strlen("\xC3\xAB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xAB", "\xC3\xAB", out_buffer, "228 NFC \"(ë; ë; e◌̈; ë; e◌̈; ) LATIN SMALL LETTER E WITH DIAERESIS\"");

    err = nfd_normaliser("\xC3\xAC", strlen("\xC3\xAC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xAC", "\x69" "\xCC\x80", out_buffer, "229 NFD \"(ì; ì; i◌̀; ì; i◌̀; ) LATIN SMALL LETTER I WITH GRAVE\"");

    err = nfkd_normaliser("\xC3\xAC", strlen("\xC3\xAC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xAC", "\x69" "\xCC\x80", out_buffer, "230 NFKD \"(ì; ì; i◌̀; ì; i◌̀; ) LATIN SMALL LETTER I WITH GRAVE\"");

    err = nfc_normaliser("\xC3\xAC", strlen("\xC3\xAC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xAC", "\xC3\xAC", out_buffer, "231 NFC \"(ì; ì; i◌̀; ì; i◌̀; ) LATIN SMALL LETTER I WITH GRAVE\"");

    err = nfd_normaliser("\xC3\xAD", strlen("\xC3\xAD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xAD", "\x69" "\xCC\x81", out_buffer, "232 NFD \"(í; í; i◌́; í; i◌́; ) LATIN SMALL LETTER I WITH ACUTE\"");

    err = nfkd_normaliser("\xC3\xAD", strlen("\xC3\xAD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xAD", "\x69" "\xCC\x81", out_buffer, "233 NFKD \"(í; í; i◌́; í; i◌́; ) LATIN SMALL LETTER I WITH ACUTE\"");

    err = nfc_normaliser("\xC3\xAD", strlen("\xC3\xAD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xAD", "\xC3\xAD", out_buffer, "234 NFC \"(í; í; i◌́; í; i◌́; ) LATIN SMALL LETTER I WITH ACUTE\"");

    err = nfd_normaliser("\xC3\xAE", strlen("\xC3\xAE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xAE", "\x69" "\xCC\x82", out_buffer, "235 NFD \"(î; î; i◌̂; î; i◌̂; ) LATIN SMALL LETTER I WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC3\xAE", strlen("\xC3\xAE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xAE", "\x69" "\xCC\x82", out_buffer, "236 NFKD \"(î; î; i◌̂; î; i◌̂; ) LATIN SMALL LETTER I WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC3\xAE", strlen("\xC3\xAE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xAE", "\xC3\xAE", out_buffer, "237 NFC \"(î; î; i◌̂; î; i◌̂; ) LATIN SMALL LETTER I WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC3\xAF", strlen("\xC3\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xAF", "\x69" "\xCC\x88", out_buffer, "238 NFD \"(ï; ï; i◌̈; ï; i◌̈; ) LATIN SMALL LETTER I WITH DIAERESIS\"");

    err = nfkd_normaliser("\xC3\xAF", strlen("\xC3\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xAF", "\x69" "\xCC\x88", out_buffer, "239 NFKD \"(ï; ï; i◌̈; ï; i◌̈; ) LATIN SMALL LETTER I WITH DIAERESIS\"");

    err = nfc_normaliser("\xC3\xAF", strlen("\xC3\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xAF", "\xC3\xAF", out_buffer, "240 NFC \"(ï; ï; i◌̈; ï; i◌̈; ) LATIN SMALL LETTER I WITH DIAERESIS\"");

    err = nfd_normaliser("\xC3\xB1", strlen("\xC3\xB1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB1", "\x6E" "\xCC\x83", out_buffer, "241 NFD \"(ñ; ñ; n◌̃; ñ; n◌̃; ) LATIN SMALL LETTER N WITH TILDE\"");

    err = nfkd_normaliser("\xC3\xB1", strlen("\xC3\xB1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB1", "\x6E" "\xCC\x83", out_buffer, "242 NFKD \"(ñ; ñ; n◌̃; ñ; n◌̃; ) LATIN SMALL LETTER N WITH TILDE\"");

    err = nfc_normaliser("\xC3\xB1", strlen("\xC3\xB1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB1", "\xC3\xB1", out_buffer, "243 NFC \"(ñ; ñ; n◌̃; ñ; n◌̃; ) LATIN SMALL LETTER N WITH TILDE\"");

    err = nfd_normaliser("\xC3\xB2", strlen("\xC3\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB2", "\x6F" "\xCC\x80", out_buffer, "244 NFD \"(ò; ò; o◌̀; ò; o◌̀; ) LATIN SMALL LETTER O WITH GRAVE\"");

    err = nfkd_normaliser("\xC3\xB2", strlen("\xC3\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB2", "\x6F" "\xCC\x80", out_buffer, "245 NFKD \"(ò; ò; o◌̀; ò; o◌̀; ) LATIN SMALL LETTER O WITH GRAVE\"");

    err = nfc_normaliser("\xC3\xB2", strlen("\xC3\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB2", "\xC3\xB2", out_buffer, "246 NFC \"(ò; ò; o◌̀; ò; o◌̀; ) LATIN SMALL LETTER O WITH GRAVE\"");

    err = nfd_normaliser("\xC3\xB3", strlen("\xC3\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB3", "\x6F" "\xCC\x81", out_buffer, "247 NFD \"(ó; ó; o◌́; ó; o◌́; ) LATIN SMALL LETTER O WITH ACUTE\"");

    err = nfkd_normaliser("\xC3\xB3", strlen("\xC3\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB3", "\x6F" "\xCC\x81", out_buffer, "248 NFKD \"(ó; ó; o◌́; ó; o◌́; ) LATIN SMALL LETTER O WITH ACUTE\"");

    err = nfc_normaliser("\xC3\xB3", strlen("\xC3\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB3", "\xC3\xB3", out_buffer, "249 NFC \"(ó; ó; o◌́; ó; o◌́; ) LATIN SMALL LETTER O WITH ACUTE\"");

    err = nfd_normaliser("\xC3\xB4", strlen("\xC3\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB4", "\x6F" "\xCC\x82", out_buffer, "250 NFD \"(ô; ô; o◌̂; ô; o◌̂; ) LATIN SMALL LETTER O WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC3\xB4", strlen("\xC3\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB4", "\x6F" "\xCC\x82", out_buffer, "251 NFKD \"(ô; ô; o◌̂; ô; o◌̂; ) LATIN SMALL LETTER O WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC3\xB4", strlen("\xC3\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB4", "\xC3\xB4", out_buffer, "252 NFC \"(ô; ô; o◌̂; ô; o◌̂; ) LATIN SMALL LETTER O WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC3\xB5", strlen("\xC3\xB5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB5", "\x6F" "\xCC\x83", out_buffer, "253 NFD \"(õ; õ; o◌̃; õ; o◌̃; ) LATIN SMALL LETTER O WITH TILDE\"");

    err = nfkd_normaliser("\xC3\xB5", strlen("\xC3\xB5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB5", "\x6F" "\xCC\x83", out_buffer, "254 NFKD \"(õ; õ; o◌̃; õ; o◌̃; ) LATIN SMALL LETTER O WITH TILDE\"");

    err = nfc_normaliser("\xC3\xB5", strlen("\xC3\xB5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB5", "\xC3\xB5", out_buffer, "255 NFC \"(õ; õ; o◌̃; õ; o◌̃; ) LATIN SMALL LETTER O WITH TILDE\"");

    err = nfd_normaliser("\xC3\xB6", strlen("\xC3\xB6"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB6", "\x6F" "\xCC\x88", out_buffer, "256 NFD \"(ö; ö; o◌̈; ö; o◌̈; ) LATIN SMALL LETTER O WITH DIAERESIS\"");

    err = nfkd_normaliser("\xC3\xB6", strlen("\xC3\xB6"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB6", "\x6F" "\xCC\x88", out_buffer, "257 NFKD \"(ö; ö; o◌̈; ö; o◌̈; ) LATIN SMALL LETTER O WITH DIAERESIS\"");

    err = nfc_normaliser("\xC3\xB6", strlen("\xC3\xB6"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB6", "\xC3\xB6", out_buffer, "258 NFC \"(ö; ö; o◌̈; ö; o◌̈; ) LATIN SMALL LETTER O WITH DIAERESIS\"");

    err = nfd_normaliser("\xC3\xB9", strlen("\xC3\xB9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB9", "\x75" "\xCC\x80", out_buffer, "259 NFD \"(ù; ù; u◌̀; ù; u◌̀; ) LATIN SMALL LETTER U WITH GRAVE\"");

    err = nfkd_normaliser("\xC3\xB9", strlen("\xC3\xB9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB9", "\x75" "\xCC\x80", out_buffer, "260 NFKD \"(ù; ù; u◌̀; ù; u◌̀; ) LATIN SMALL LETTER U WITH GRAVE\"");

    err = nfc_normaliser("\xC3\xB9", strlen("\xC3\xB9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xB9", "\xC3\xB9", out_buffer, "261 NFC \"(ù; ù; u◌̀; ù; u◌̀; ) LATIN SMALL LETTER U WITH GRAVE\"");

    err = nfd_normaliser("\xC3\xBA", strlen("\xC3\xBA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xBA", "\x75" "\xCC\x81", out_buffer, "262 NFD \"(ú; ú; u◌́; ú; u◌́; ) LATIN SMALL LETTER U WITH ACUTE\"");

    err = nfkd_normaliser("\xC3\xBA", strlen("\xC3\xBA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xBA", "\x75" "\xCC\x81", out_buffer, "263 NFKD \"(ú; ú; u◌́; ú; u◌́; ) LATIN SMALL LETTER U WITH ACUTE\"");

    err = nfc_normaliser("\xC3\xBA", strlen("\xC3\xBA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xBA", "\xC3\xBA", out_buffer, "264 NFC \"(ú; ú; u◌́; ú; u◌́; ) LATIN SMALL LETTER U WITH ACUTE\"");

    err = nfd_normaliser("\xC3\xBB", strlen("\xC3\xBB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xBB", "\x75" "\xCC\x82", out_buffer, "265 NFD \"(û; û; u◌̂; û; u◌̂; ) LATIN SMALL LETTER U WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC3\xBB", strlen("\xC3\xBB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xBB", "\x75" "\xCC\x82", out_buffer, "266 NFKD \"(û; û; u◌̂; û; u◌̂; ) LATIN SMALL LETTER U WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC3\xBB", strlen("\xC3\xBB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xBB", "\xC3\xBB", out_buffer, "267 NFC \"(û; û; u◌̂; û; u◌̂; ) LATIN SMALL LETTER U WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC3\xBC", strlen("\xC3\xBC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xBC", "\x75" "\xCC\x88", out_buffer, "268 NFD \"(ü; ü; u◌̈; ü; u◌̈; ) LATIN SMALL LETTER U WITH DIAERESIS\"");

    err = nfkd_normaliser("\xC3\xBC", strlen("\xC3\xBC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xBC", "\x75" "\xCC\x88", out_buffer, "269 NFKD \"(ü; ü; u◌̈; ü; u◌̈; ) LATIN SMALL LETTER U WITH DIAERESIS\"");

    err = nfc_normaliser("\xC3\xBC", strlen("\xC3\xBC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xBC", "\xC3\xBC", out_buffer, "270 NFC \"(ü; ü; u◌̈; ü; u◌̈; ) LATIN SMALL LETTER U WITH DIAERESIS\"");

    err = nfd_normaliser("\xC3\xBD", strlen("\xC3\xBD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xBD", "\x79" "\xCC\x81", out_buffer, "271 NFD \"(ý; ý; y◌́; ý; y◌́; ) LATIN SMALL LETTER Y WITH ACUTE\"");

    err = nfkd_normaliser("\xC3\xBD", strlen("\xC3\xBD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xBD", "\x79" "\xCC\x81", out_buffer, "272 NFKD \"(ý; ý; y◌́; ý; y◌́; ) LATIN SMALL LETTER Y WITH ACUTE\"");

    err = nfc_normaliser("\xC3\xBD", strlen("\xC3\xBD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xBD", "\xC3\xBD", out_buffer, "273 NFC \"(ý; ý; y◌́; ý; y◌́; ) LATIN SMALL LETTER Y WITH ACUTE\"");

    err = nfd_normaliser("\xC3\xBF", strlen("\xC3\xBF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xBF", "\x79" "\xCC\x88", out_buffer, "274 NFD \"(ÿ; ÿ; y◌̈; ÿ; y◌̈; ) LATIN SMALL LETTER Y WITH DIAERESIS\"");

    err = nfkd_normaliser("\xC3\xBF", strlen("\xC3\xBF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xBF", "\x79" "\xCC\x88", out_buffer, "275 NFKD \"(ÿ; ÿ; y◌̈; ÿ; y◌̈; ) LATIN SMALL LETTER Y WITH DIAERESIS\"");

    err = nfc_normaliser("\xC3\xBF", strlen("\xC3\xBF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC3\xBF", "\xC3\xBF", out_buffer, "276 NFC \"(ÿ; ÿ; y◌̈; ÿ; y◌̈; ) LATIN SMALL LETTER Y WITH DIAERESIS\"");

    err = nfd_normaliser("\xC4\x80", strlen("\xC4\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x80", "\x41" "\xCC\x84", out_buffer, "277 NFD \"(Ā; Ā; A◌̄; Ā; A◌̄; ) LATIN CAPITAL LETTER A WITH MACRON\"");

    err = nfkd_normaliser("\xC4\x80", strlen("\xC4\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x80", "\x41" "\xCC\x84", out_buffer, "278 NFKD \"(Ā; Ā; A◌̄; Ā; A◌̄; ) LATIN CAPITAL LETTER A WITH MACRON\"");

    err = nfc_normaliser("\xC4\x80", strlen("\xC4\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x80", "\xC4\x80", out_buffer, "279 NFC \"(Ā; Ā; A◌̄; Ā; A◌̄; ) LATIN CAPITAL LETTER A WITH MACRON\"");

    err = nfd_normaliser("\xC4\x81", strlen("\xC4\x81"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x81", "\x61" "\xCC\x84", out_buffer, "280 NFD \"(ā; ā; a◌̄; ā; a◌̄; ) LATIN SMALL LETTER A WITH MACRON\"");

    err = nfkd_normaliser("\xC4\x81", strlen("\xC4\x81"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x81", "\x61" "\xCC\x84", out_buffer, "281 NFKD \"(ā; ā; a◌̄; ā; a◌̄; ) LATIN SMALL LETTER A WITH MACRON\"");

    err = nfc_normaliser("\xC4\x81", strlen("\xC4\x81"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x81", "\xC4\x81", out_buffer, "282 NFC \"(ā; ā; a◌̄; ā; a◌̄; ) LATIN SMALL LETTER A WITH MACRON\"");

    err = nfd_normaliser("\xC4\x82", strlen("\xC4\x82"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x82", "\x41" "\xCC\x86", out_buffer, "283 NFD \"(Ă; Ă; A◌̆; Ă; A◌̆; ) LATIN CAPITAL LETTER A WITH BREVE\"");

    err = nfkd_normaliser("\xC4\x82", strlen("\xC4\x82"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x82", "\x41" "\xCC\x86", out_buffer, "284 NFKD \"(Ă; Ă; A◌̆; Ă; A◌̆; ) LATIN CAPITAL LETTER A WITH BREVE\"");

    err = nfc_normaliser("\xC4\x82", strlen("\xC4\x82"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x82", "\xC4\x82", out_buffer, "285 NFC \"(Ă; Ă; A◌̆; Ă; A◌̆; ) LATIN CAPITAL LETTER A WITH BREVE\"");

    err = nfd_normaliser("\xC4\x83", strlen("\xC4\x83"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x83", "\x61" "\xCC\x86", out_buffer, "286 NFD \"(ă; ă; a◌̆; ă; a◌̆; ) LATIN SMALL LETTER A WITH BREVE\"");

    err = nfkd_normaliser("\xC4\x83", strlen("\xC4\x83"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x83", "\x61" "\xCC\x86", out_buffer, "287 NFKD \"(ă; ă; a◌̆; ă; a◌̆; ) LATIN SMALL LETTER A WITH BREVE\"");

    err = nfc_normaliser("\xC4\x83", strlen("\xC4\x83"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x83", "\xC4\x83", out_buffer, "288 NFC \"(ă; ă; a◌̆; ă; a◌̆; ) LATIN SMALL LETTER A WITH BREVE\"");

    err = nfd_normaliser("\xC4\x84", strlen("\xC4\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x84", "\x41" "\xCC\xA8", out_buffer, "289 NFD \"(Ą; Ą; A◌̨; Ą; A◌̨; ) LATIN CAPITAL LETTER A WITH OGONEK\"");

    err = nfkd_normaliser("\xC4\x84", strlen("\xC4\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x84", "\x41" "\xCC\xA8", out_buffer, "290 NFKD \"(Ą; Ą; A◌̨; Ą; A◌̨; ) LATIN CAPITAL LETTER A WITH OGONEK\"");

    err = nfc_normaliser("\xC4\x84", strlen("\xC4\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x84", "\xC4\x84", out_buffer, "291 NFC \"(Ą; Ą; A◌̨; Ą; A◌̨; ) LATIN CAPITAL LETTER A WITH OGONEK\"");

    err = nfd_normaliser("\xC4\x85", strlen("\xC4\x85"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x85", "\x61" "\xCC\xA8", out_buffer, "292 NFD \"(ą; ą; a◌̨; ą; a◌̨; ) LATIN SMALL LETTER A WITH OGONEK\"");

    err = nfkd_normaliser("\xC4\x85", strlen("\xC4\x85"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x85", "\x61" "\xCC\xA8", out_buffer, "293 NFKD \"(ą; ą; a◌̨; ą; a◌̨; ) LATIN SMALL LETTER A WITH OGONEK\"");

    err = nfc_normaliser("\xC4\x85", strlen("\xC4\x85"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x85", "\xC4\x85", out_buffer, "294 NFC \"(ą; ą; a◌̨; ą; a◌̨; ) LATIN SMALL LETTER A WITH OGONEK\"");

    err = nfd_normaliser("\xC4\x86", strlen("\xC4\x86"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x86", "\x43" "\xCC\x81", out_buffer, "295 NFD \"(Ć; Ć; C◌́; Ć; C◌́; ) LATIN CAPITAL LETTER C WITH ACUTE\"");

    err = nfkd_normaliser("\xC4\x86", strlen("\xC4\x86"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x86", "\x43" "\xCC\x81", out_buffer, "296 NFKD \"(Ć; Ć; C◌́; Ć; C◌́; ) LATIN CAPITAL LETTER C WITH ACUTE\"");

    err = nfc_normaliser("\xC4\x86", strlen("\xC4\x86"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x86", "\xC4\x86", out_buffer, "297 NFC \"(Ć; Ć; C◌́; Ć; C◌́; ) LATIN CAPITAL LETTER C WITH ACUTE\"");

    err = nfd_normaliser("\xC4\x87", strlen("\xC4\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x87", "\x63" "\xCC\x81", out_buffer, "298 NFD \"(ć; ć; c◌́; ć; c◌́; ) LATIN SMALL LETTER C WITH ACUTE\"");

    err = nfkd_normaliser("\xC4\x87", strlen("\xC4\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x87", "\x63" "\xCC\x81", out_buffer, "299 NFKD \"(ć; ć; c◌́; ć; c◌́; ) LATIN SMALL LETTER C WITH ACUTE\"");

    err = nfc_normaliser("\xC4\x87", strlen("\xC4\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x87", "\xC4\x87", out_buffer, "300 NFC \"(ć; ć; c◌́; ć; c◌́; ) LATIN SMALL LETTER C WITH ACUTE\"");

    err = nfd_normaliser("\xC4\x88", strlen("\xC4\x88"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x88", "\x43" "\xCC\x82", out_buffer, "301 NFD \"(Ĉ; Ĉ; C◌̂; Ĉ; C◌̂; ) LATIN CAPITAL LETTER C WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC4\x88", strlen("\xC4\x88"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x88", "\x43" "\xCC\x82", out_buffer, "302 NFKD \"(Ĉ; Ĉ; C◌̂; Ĉ; C◌̂; ) LATIN CAPITAL LETTER C WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC4\x88", strlen("\xC4\x88"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x88", "\xC4\x88", out_buffer, "303 NFC \"(Ĉ; Ĉ; C◌̂; Ĉ; C◌̂; ) LATIN CAPITAL LETTER C WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC4\x89", strlen("\xC4\x89"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x89", "\x63" "\xCC\x82", out_buffer, "304 NFD \"(ĉ; ĉ; c◌̂; ĉ; c◌̂; ) LATIN SMALL LETTER C WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC4\x89", strlen("\xC4\x89"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x89", "\x63" "\xCC\x82", out_buffer, "305 NFKD \"(ĉ; ĉ; c◌̂; ĉ; c◌̂; ) LATIN SMALL LETTER C WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC4\x89", strlen("\xC4\x89"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x89", "\xC4\x89", out_buffer, "306 NFC \"(ĉ; ĉ; c◌̂; ĉ; c◌̂; ) LATIN SMALL LETTER C WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC4\x8A", strlen("\xC4\x8A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x8A", "\x43" "\xCC\x87", out_buffer, "307 NFD \"(Ċ; Ċ; C◌̇; Ċ; C◌̇; ) LATIN CAPITAL LETTER C WITH DOT ABOVE\"");

    err = nfkd_normaliser("\xC4\x8A", strlen("\xC4\x8A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x8A", "\x43" "\xCC\x87", out_buffer, "308 NFKD \"(Ċ; Ċ; C◌̇; Ċ; C◌̇; ) LATIN CAPITAL LETTER C WITH DOT ABOVE\"");

    err = nfc_normaliser("\xC4\x8A", strlen("\xC4\x8A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x8A", "\xC4\x8A", out_buffer, "309 NFC \"(Ċ; Ċ; C◌̇; Ċ; C◌̇; ) LATIN CAPITAL LETTER C WITH DOT ABOVE\"");

    err = nfd_normaliser("\xC4\x8B", strlen("\xC4\x8B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x8B", "\x63" "\xCC\x87", out_buffer, "310 NFD \"(ċ; ċ; c◌̇; ċ; c◌̇; ) LATIN SMALL LETTER C WITH DOT ABOVE\"");

    err = nfkd_normaliser("\xC4\x8B", strlen("\xC4\x8B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x8B", "\x63" "\xCC\x87", out_buffer, "311 NFKD \"(ċ; ċ; c◌̇; ċ; c◌̇; ) LATIN SMALL LETTER C WITH DOT ABOVE\"");

    err = nfc_normaliser("\xC4\x8B", strlen("\xC4\x8B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x8B", "\xC4\x8B", out_buffer, "312 NFC \"(ċ; ċ; c◌̇; ċ; c◌̇; ) LATIN SMALL LETTER C WITH DOT ABOVE\"");

    err = nfd_normaliser("\xC4\x8C", strlen("\xC4\x8C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x8C", "\x43" "\xCC\x8C", out_buffer, "313 NFD \"(Č; Č; C◌̌; Č; C◌̌; ) LATIN CAPITAL LETTER C WITH CARON\"");

    err = nfkd_normaliser("\xC4\x8C", strlen("\xC4\x8C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x8C", "\x43" "\xCC\x8C", out_buffer, "314 NFKD \"(Č; Č; C◌̌; Č; C◌̌; ) LATIN CAPITAL LETTER C WITH CARON\"");

    err = nfc_normaliser("\xC4\x8C", strlen("\xC4\x8C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x8C", "\xC4\x8C", out_buffer, "315 NFC \"(Č; Č; C◌̌; Č; C◌̌; ) LATIN CAPITAL LETTER C WITH CARON\"");

    err = nfd_normaliser("\xC4\x8D", strlen("\xC4\x8D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x8D", "\x63" "\xCC\x8C", out_buffer, "316 NFD \"(č; č; c◌̌; č; c◌̌; ) LATIN SMALL LETTER C WITH CARON\"");

    err = nfkd_normaliser("\xC4\x8D", strlen("\xC4\x8D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x8D", "\x63" "\xCC\x8C", out_buffer, "317 NFKD \"(č; č; c◌̌; č; c◌̌; ) LATIN SMALL LETTER C WITH CARON\"");

    err = nfc_normaliser("\xC4\x8D", strlen("\xC4\x8D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x8D", "\xC4\x8D", out_buffer, "318 NFC \"(č; č; c◌̌; č; c◌̌; ) LATIN SMALL LETTER C WITH CARON\"");

    err = nfd_normaliser("\xC4\x8E", strlen("\xC4\x8E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x8E", "\x44" "\xCC\x8C", out_buffer, "319 NFD \"(Ď; Ď; D◌̌; Ď; D◌̌; ) LATIN CAPITAL LETTER D WITH CARON\"");

    err = nfkd_normaliser("\xC4\x8E", strlen("\xC4\x8E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x8E", "\x44" "\xCC\x8C", out_buffer, "320 NFKD \"(Ď; Ď; D◌̌; Ď; D◌̌; ) LATIN CAPITAL LETTER D WITH CARON\"");

    err = nfc_normaliser("\xC4\x8E", strlen("\xC4\x8E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x8E", "\xC4\x8E", out_buffer, "321 NFC \"(Ď; Ď; D◌̌; Ď; D◌̌; ) LATIN CAPITAL LETTER D WITH CARON\"");

    err = nfd_normaliser("\xC4\x8F", strlen("\xC4\x8F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x8F", "\x64" "\xCC\x8C", out_buffer, "322 NFD \"(ď; ď; d◌̌; ď; d◌̌; ) LATIN SMALL LETTER D WITH CARON\"");

    err = nfkd_normaliser("\xC4\x8F", strlen("\xC4\x8F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x8F", "\x64" "\xCC\x8C", out_buffer, "323 NFKD \"(ď; ď; d◌̌; ď; d◌̌; ) LATIN SMALL LETTER D WITH CARON\"");

    err = nfc_normaliser("\xC4\x8F", strlen("\xC4\x8F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x8F", "\xC4\x8F", out_buffer, "324 NFC \"(ď; ď; d◌̌; ď; d◌̌; ) LATIN SMALL LETTER D WITH CARON\"");

    err = nfd_normaliser("\xC4\x92", strlen("\xC4\x92"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x92", "\x45" "\xCC\x84", out_buffer, "325 NFD \"(Ē; Ē; E◌̄; Ē; E◌̄; ) LATIN CAPITAL LETTER E WITH MACRON\"");

    err = nfkd_normaliser("\xC4\x92", strlen("\xC4\x92"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x92", "\x45" "\xCC\x84", out_buffer, "326 NFKD \"(Ē; Ē; E◌̄; Ē; E◌̄; ) LATIN CAPITAL LETTER E WITH MACRON\"");

    err = nfc_normaliser("\xC4\x92", strlen("\xC4\x92"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x92", "\xC4\x92", out_buffer, "327 NFC \"(Ē; Ē; E◌̄; Ē; E◌̄; ) LATIN CAPITAL LETTER E WITH MACRON\"");

    err = nfd_normaliser("\xC4\x93", strlen("\xC4\x93"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x93", "\x65" "\xCC\x84", out_buffer, "328 NFD \"(ē; ē; e◌̄; ē; e◌̄; ) LATIN SMALL LETTER E WITH MACRON\"");

    err = nfkd_normaliser("\xC4\x93", strlen("\xC4\x93"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x93", "\x65" "\xCC\x84", out_buffer, "329 NFKD \"(ē; ē; e◌̄; ē; e◌̄; ) LATIN SMALL LETTER E WITH MACRON\"");

    err = nfc_normaliser("\xC4\x93", strlen("\xC4\x93"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x93", "\xC4\x93", out_buffer, "330 NFC \"(ē; ē; e◌̄; ē; e◌̄; ) LATIN SMALL LETTER E WITH MACRON\"");

    err = nfd_normaliser("\xC4\x94", strlen("\xC4\x94"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x94", "\x45" "\xCC\x86", out_buffer, "331 NFD \"(Ĕ; Ĕ; E◌̆; Ĕ; E◌̆; ) LATIN CAPITAL LETTER E WITH BREVE\"");

    err = nfkd_normaliser("\xC4\x94", strlen("\xC4\x94"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x94", "\x45" "\xCC\x86", out_buffer, "332 NFKD \"(Ĕ; Ĕ; E◌̆; Ĕ; E◌̆; ) LATIN CAPITAL LETTER E WITH BREVE\"");

    err = nfc_normaliser("\xC4\x94", strlen("\xC4\x94"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x94", "\xC4\x94", out_buffer, "333 NFC \"(Ĕ; Ĕ; E◌̆; Ĕ; E◌̆; ) LATIN CAPITAL LETTER E WITH BREVE\"");

    err = nfd_normaliser("\xC4\x95", strlen("\xC4\x95"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x95", "\x65" "\xCC\x86", out_buffer, "334 NFD \"(ĕ; ĕ; e◌̆; ĕ; e◌̆; ) LATIN SMALL LETTER E WITH BREVE\"");

    err = nfkd_normaliser("\xC4\x95", strlen("\xC4\x95"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x95", "\x65" "\xCC\x86", out_buffer, "335 NFKD \"(ĕ; ĕ; e◌̆; ĕ; e◌̆; ) LATIN SMALL LETTER E WITH BREVE\"");

    err = nfc_normaliser("\xC4\x95", strlen("\xC4\x95"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x95", "\xC4\x95", out_buffer, "336 NFC \"(ĕ; ĕ; e◌̆; ĕ; e◌̆; ) LATIN SMALL LETTER E WITH BREVE\"");

    err = nfd_normaliser("\xC4\x96", strlen("\xC4\x96"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x96", "\x45" "\xCC\x87", out_buffer, "337 NFD \"(Ė; Ė; E◌̇; Ė; E◌̇; ) LATIN CAPITAL LETTER E WITH DOT ABOVE\"");

    err = nfkd_normaliser("\xC4\x96", strlen("\xC4\x96"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x96", "\x45" "\xCC\x87", out_buffer, "338 NFKD \"(Ė; Ė; E◌̇; Ė; E◌̇; ) LATIN CAPITAL LETTER E WITH DOT ABOVE\"");

    err = nfc_normaliser("\xC4\x96", strlen("\xC4\x96"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x96", "\xC4\x96", out_buffer, "339 NFC \"(Ė; Ė; E◌̇; Ė; E◌̇; ) LATIN CAPITAL LETTER E WITH DOT ABOVE\"");

    err = nfd_normaliser("\xC4\x97", strlen("\xC4\x97"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x97", "\x65" "\xCC\x87", out_buffer, "340 NFD \"(ė; ė; e◌̇; ė; e◌̇; ) LATIN SMALL LETTER E WITH DOT ABOVE\"");

    err = nfkd_normaliser("\xC4\x97", strlen("\xC4\x97"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x97", "\x65" "\xCC\x87", out_buffer, "341 NFKD \"(ė; ė; e◌̇; ė; e◌̇; ) LATIN SMALL LETTER E WITH DOT ABOVE\"");

    err = nfc_normaliser("\xC4\x97", strlen("\xC4\x97"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x97", "\xC4\x97", out_buffer, "342 NFC \"(ė; ė; e◌̇; ė; e◌̇; ) LATIN SMALL LETTER E WITH DOT ABOVE\"");

    err = nfd_normaliser("\xC4\x98", strlen("\xC4\x98"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x98", "\x45" "\xCC\xA8", out_buffer, "343 NFD \"(Ę; Ę; E◌̨; Ę; E◌̨; ) LATIN CAPITAL LETTER E WITH OGONEK\"");

    err = nfkd_normaliser("\xC4\x98", strlen("\xC4\x98"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x98", "\x45" "\xCC\xA8", out_buffer, "344 NFKD \"(Ę; Ę; E◌̨; Ę; E◌̨; ) LATIN CAPITAL LETTER E WITH OGONEK\"");

    err = nfc_normaliser("\xC4\x98", strlen("\xC4\x98"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x98", "\xC4\x98", out_buffer, "345 NFC \"(Ę; Ę; E◌̨; Ę; E◌̨; ) LATIN CAPITAL LETTER E WITH OGONEK\"");

    err = nfd_normaliser("\xC4\x99", strlen("\xC4\x99"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x99", "\x65" "\xCC\xA8", out_buffer, "346 NFD \"(ę; ę; e◌̨; ę; e◌̨; ) LATIN SMALL LETTER E WITH OGONEK\"");

    err = nfkd_normaliser("\xC4\x99", strlen("\xC4\x99"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x99", "\x65" "\xCC\xA8", out_buffer, "347 NFKD \"(ę; ę; e◌̨; ę; e◌̨; ) LATIN SMALL LETTER E WITH OGONEK\"");

    err = nfc_normaliser("\xC4\x99", strlen("\xC4\x99"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x99", "\xC4\x99", out_buffer, "348 NFC \"(ę; ę; e◌̨; ę; e◌̨; ) LATIN SMALL LETTER E WITH OGONEK\"");

    err = nfd_normaliser("\xC4\x9A", strlen("\xC4\x9A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x9A", "\x45" "\xCC\x8C", out_buffer, "349 NFD \"(Ě; Ě; E◌̌; Ě; E◌̌; ) LATIN CAPITAL LETTER E WITH CARON\"");

    err = nfkd_normaliser("\xC4\x9A", strlen("\xC4\x9A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x9A", "\x45" "\xCC\x8C", out_buffer, "350 NFKD \"(Ě; Ě; E◌̌; Ě; E◌̌; ) LATIN CAPITAL LETTER E WITH CARON\"");

    err = nfc_normaliser("\xC4\x9A", strlen("\xC4\x9A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x9A", "\xC4\x9A", out_buffer, "351 NFC \"(Ě; Ě; E◌̌; Ě; E◌̌; ) LATIN CAPITAL LETTER E WITH CARON\"");

    err = nfd_normaliser("\xC4\x9B", strlen("\xC4\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x9B", "\x65" "\xCC\x8C", out_buffer, "352 NFD \"(ě; ě; e◌̌; ě; e◌̌; ) LATIN SMALL LETTER E WITH CARON\"");

    err = nfkd_normaliser("\xC4\x9B", strlen("\xC4\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x9B", "\x65" "\xCC\x8C", out_buffer, "353 NFKD \"(ě; ě; e◌̌; ě; e◌̌; ) LATIN SMALL LETTER E WITH CARON\"");

    err = nfc_normaliser("\xC4\x9B", strlen("\xC4\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x9B", "\xC4\x9B", out_buffer, "354 NFC \"(ě; ě; e◌̌; ě; e◌̌; ) LATIN SMALL LETTER E WITH CARON\"");

    err = nfd_normaliser("\xC4\x9C", strlen("\xC4\x9C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x9C", "\x47" "\xCC\x82", out_buffer, "355 NFD \"(Ĝ; Ĝ; G◌̂; Ĝ; G◌̂; ) LATIN CAPITAL LETTER G WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC4\x9C", strlen("\xC4\x9C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x9C", "\x47" "\xCC\x82", out_buffer, "356 NFKD \"(Ĝ; Ĝ; G◌̂; Ĝ; G◌̂; ) LATIN CAPITAL LETTER G WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC4\x9C", strlen("\xC4\x9C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x9C", "\xC4\x9C", out_buffer, "357 NFC \"(Ĝ; Ĝ; G◌̂; Ĝ; G◌̂; ) LATIN CAPITAL LETTER G WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC4\x9D", strlen("\xC4\x9D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x9D", "\x67" "\xCC\x82", out_buffer, "358 NFD \"(ĝ; ĝ; g◌̂; ĝ; g◌̂; ) LATIN SMALL LETTER G WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC4\x9D", strlen("\xC4\x9D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x9D", "\x67" "\xCC\x82", out_buffer, "359 NFKD \"(ĝ; ĝ; g◌̂; ĝ; g◌̂; ) LATIN SMALL LETTER G WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC4\x9D", strlen("\xC4\x9D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x9D", "\xC4\x9D", out_buffer, "360 NFC \"(ĝ; ĝ; g◌̂; ĝ; g◌̂; ) LATIN SMALL LETTER G WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC4\x9E", strlen("\xC4\x9E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x9E", "\x47" "\xCC\x86", out_buffer, "361 NFD \"(Ğ; Ğ; G◌̆; Ğ; G◌̆; ) LATIN CAPITAL LETTER G WITH BREVE\"");

    err = nfkd_normaliser("\xC4\x9E", strlen("\xC4\x9E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x9E", "\x47" "\xCC\x86", out_buffer, "362 NFKD \"(Ğ; Ğ; G◌̆; Ğ; G◌̆; ) LATIN CAPITAL LETTER G WITH BREVE\"");

    err = nfc_normaliser("\xC4\x9E", strlen("\xC4\x9E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x9E", "\xC4\x9E", out_buffer, "363 NFC \"(Ğ; Ğ; G◌̆; Ğ; G◌̆; ) LATIN CAPITAL LETTER G WITH BREVE\"");

    err = nfd_normaliser("\xC4\x9F", strlen("\xC4\x9F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x9F", "\x67" "\xCC\x86", out_buffer, "364 NFD \"(ğ; ğ; g◌̆; ğ; g◌̆; ) LATIN SMALL LETTER G WITH BREVE\"");

    err = nfkd_normaliser("\xC4\x9F", strlen("\xC4\x9F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x9F", "\x67" "\xCC\x86", out_buffer, "365 NFKD \"(ğ; ğ; g◌̆; ğ; g◌̆; ) LATIN SMALL LETTER G WITH BREVE\"");

    err = nfc_normaliser("\xC4\x9F", strlen("\xC4\x9F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\x9F", "\xC4\x9F", out_buffer, "366 NFC \"(ğ; ğ; g◌̆; ğ; g◌̆; ) LATIN SMALL LETTER G WITH BREVE\"");

    err = nfd_normaliser("\xC4\xA0", strlen("\xC4\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA0", "\x47" "\xCC\x87", out_buffer, "367 NFD \"(Ġ; Ġ; G◌̇; Ġ; G◌̇; ) LATIN CAPITAL LETTER G WITH DOT ABOVE\"");

    err = nfkd_normaliser("\xC4\xA0", strlen("\xC4\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA0", "\x47" "\xCC\x87", out_buffer, "368 NFKD \"(Ġ; Ġ; G◌̇; Ġ; G◌̇; ) LATIN CAPITAL LETTER G WITH DOT ABOVE\"");

    err = nfc_normaliser("\xC4\xA0", strlen("\xC4\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA0", "\xC4\xA0", out_buffer, "369 NFC \"(Ġ; Ġ; G◌̇; Ġ; G◌̇; ) LATIN CAPITAL LETTER G WITH DOT ABOVE\"");

    err = nfd_normaliser("\xC4\xA1", strlen("\xC4\xA1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA1", "\x67" "\xCC\x87", out_buffer, "370 NFD \"(ġ; ġ; g◌̇; ġ; g◌̇; ) LATIN SMALL LETTER G WITH DOT ABOVE\"");

    err = nfkd_normaliser("\xC4\xA1", strlen("\xC4\xA1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA1", "\x67" "\xCC\x87", out_buffer, "371 NFKD \"(ġ; ġ; g◌̇; ġ; g◌̇; ) LATIN SMALL LETTER G WITH DOT ABOVE\"");

    err = nfc_normaliser("\xC4\xA1", strlen("\xC4\xA1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA1", "\xC4\xA1", out_buffer, "372 NFC \"(ġ; ġ; g◌̇; ġ; g◌̇; ) LATIN SMALL LETTER G WITH DOT ABOVE\"");

    err = nfd_normaliser("\xC4\xA2", strlen("\xC4\xA2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA2", "\x47" "\xCC\xA7", out_buffer, "373 NFD \"(Ģ; Ģ; G◌̧; Ģ; G◌̧; ) LATIN CAPITAL LETTER G WITH CEDILLA\"");

    err = nfkd_normaliser("\xC4\xA2", strlen("\xC4\xA2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA2", "\x47" "\xCC\xA7", out_buffer, "374 NFKD \"(Ģ; Ģ; G◌̧; Ģ; G◌̧; ) LATIN CAPITAL LETTER G WITH CEDILLA\"");

    err = nfc_normaliser("\xC4\xA2", strlen("\xC4\xA2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA2", "\xC4\xA2", out_buffer, "375 NFC \"(Ģ; Ģ; G◌̧; Ģ; G◌̧; ) LATIN CAPITAL LETTER G WITH CEDILLA\"");

    err = nfd_normaliser("\xC4\xA3", strlen("\xC4\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA3", "\x67" "\xCC\xA7", out_buffer, "376 NFD \"(ģ; ģ; g◌̧; ģ; g◌̧; ) LATIN SMALL LETTER G WITH CEDILLA\"");

    err = nfkd_normaliser("\xC4\xA3", strlen("\xC4\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA3", "\x67" "\xCC\xA7", out_buffer, "377 NFKD \"(ģ; ģ; g◌̧; ģ; g◌̧; ) LATIN SMALL LETTER G WITH CEDILLA\"");

    err = nfc_normaliser("\xC4\xA3", strlen("\xC4\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA3", "\xC4\xA3", out_buffer, "378 NFC \"(ģ; ģ; g◌̧; ģ; g◌̧; ) LATIN SMALL LETTER G WITH CEDILLA\"");

    err = nfd_normaliser("\xC4\xA4", strlen("\xC4\xA4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA4", "\x48" "\xCC\x82", out_buffer, "379 NFD \"(Ĥ; Ĥ; H◌̂; Ĥ; H◌̂; ) LATIN CAPITAL LETTER H WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC4\xA4", strlen("\xC4\xA4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA4", "\x48" "\xCC\x82", out_buffer, "380 NFKD \"(Ĥ; Ĥ; H◌̂; Ĥ; H◌̂; ) LATIN CAPITAL LETTER H WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC4\xA4", strlen("\xC4\xA4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA4", "\xC4\xA4", out_buffer, "381 NFC \"(Ĥ; Ĥ; H◌̂; Ĥ; H◌̂; ) LATIN CAPITAL LETTER H WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC4\xA5", strlen("\xC4\xA5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA5", "\x68" "\xCC\x82", out_buffer, "382 NFD \"(ĥ; ĥ; h◌̂; ĥ; h◌̂; ) LATIN SMALL LETTER H WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC4\xA5", strlen("\xC4\xA5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA5", "\x68" "\xCC\x82", out_buffer, "383 NFKD \"(ĥ; ĥ; h◌̂; ĥ; h◌̂; ) LATIN SMALL LETTER H WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC4\xA5", strlen("\xC4\xA5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA5", "\xC4\xA5", out_buffer, "384 NFC \"(ĥ; ĥ; h◌̂; ĥ; h◌̂; ) LATIN SMALL LETTER H WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC4\xA8", strlen("\xC4\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA8", "\x49" "\xCC\x83", out_buffer, "385 NFD \"(Ĩ; Ĩ; I◌̃; Ĩ; I◌̃; ) LATIN CAPITAL LETTER I WITH TILDE\"");

    err = nfkd_normaliser("\xC4\xA8", strlen("\xC4\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA8", "\x49" "\xCC\x83", out_buffer, "386 NFKD \"(Ĩ; Ĩ; I◌̃; Ĩ; I◌̃; ) LATIN CAPITAL LETTER I WITH TILDE\"");

    err = nfc_normaliser("\xC4\xA8", strlen("\xC4\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA8", "\xC4\xA8", out_buffer, "387 NFC \"(Ĩ; Ĩ; I◌̃; Ĩ; I◌̃; ) LATIN CAPITAL LETTER I WITH TILDE\"");

    err = nfd_normaliser("\xC4\xA9", strlen("\xC4\xA9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA9", "\x69" "\xCC\x83", out_buffer, "388 NFD \"(ĩ; ĩ; i◌̃; ĩ; i◌̃; ) LATIN SMALL LETTER I WITH TILDE\"");

    err = nfkd_normaliser("\xC4\xA9", strlen("\xC4\xA9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA9", "\x69" "\xCC\x83", out_buffer, "389 NFKD \"(ĩ; ĩ; i◌̃; ĩ; i◌̃; ) LATIN SMALL LETTER I WITH TILDE\"");

    err = nfc_normaliser("\xC4\xA9", strlen("\xC4\xA9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xA9", "\xC4\xA9", out_buffer, "390 NFC \"(ĩ; ĩ; i◌̃; ĩ; i◌̃; ) LATIN SMALL LETTER I WITH TILDE\"");

    err = nfd_normaliser("\xC4\xAA", strlen("\xC4\xAA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xAA", "\x49" "\xCC\x84", out_buffer, "391 NFD \"(Ī; Ī; I◌̄; Ī; I◌̄; ) LATIN CAPITAL LETTER I WITH MACRON\"");

    err = nfkd_normaliser("\xC4\xAA", strlen("\xC4\xAA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xAA", "\x49" "\xCC\x84", out_buffer, "392 NFKD \"(Ī; Ī; I◌̄; Ī; I◌̄; ) LATIN CAPITAL LETTER I WITH MACRON\"");

    err = nfc_normaliser("\xC4\xAA", strlen("\xC4\xAA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xAA", "\xC4\xAA", out_buffer, "393 NFC \"(Ī; Ī; I◌̄; Ī; I◌̄; ) LATIN CAPITAL LETTER I WITH MACRON\"");

    err = nfd_normaliser("\xC4\xAB", strlen("\xC4\xAB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xAB", "\x69" "\xCC\x84", out_buffer, "394 NFD \"(ī; ī; i◌̄; ī; i◌̄; ) LATIN SMALL LETTER I WITH MACRON\"");

    err = nfkd_normaliser("\xC4\xAB", strlen("\xC4\xAB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xAB", "\x69" "\xCC\x84", out_buffer, "395 NFKD \"(ī; ī; i◌̄; ī; i◌̄; ) LATIN SMALL LETTER I WITH MACRON\"");

    err = nfc_normaliser("\xC4\xAB", strlen("\xC4\xAB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xAB", "\xC4\xAB", out_buffer, "396 NFC \"(ī; ī; i◌̄; ī; i◌̄; ) LATIN SMALL LETTER I WITH MACRON\"");

    err = nfd_normaliser("\xC4\xAC", strlen("\xC4\xAC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xAC", "\x49" "\xCC\x86", out_buffer, "397 NFD \"(Ĭ; Ĭ; I◌̆; Ĭ; I◌̆; ) LATIN CAPITAL LETTER I WITH BREVE\"");

    err = nfkd_normaliser("\xC4\xAC", strlen("\xC4\xAC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xAC", "\x49" "\xCC\x86", out_buffer, "398 NFKD \"(Ĭ; Ĭ; I◌̆; Ĭ; I◌̆; ) LATIN CAPITAL LETTER I WITH BREVE\"");

    err = nfc_normaliser("\xC4\xAC", strlen("\xC4\xAC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xAC", "\xC4\xAC", out_buffer, "399 NFC \"(Ĭ; Ĭ; I◌̆; Ĭ; I◌̆; ) LATIN CAPITAL LETTER I WITH BREVE\"");

    err = nfd_normaliser("\xC4\xAD", strlen("\xC4\xAD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xAD", "\x69" "\xCC\x86", out_buffer, "400 NFD \"(ĭ; ĭ; i◌̆; ĭ; i◌̆; ) LATIN SMALL LETTER I WITH BREVE\"");

    err = nfkd_normaliser("\xC4\xAD", strlen("\xC4\xAD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xAD", "\x69" "\xCC\x86", out_buffer, "401 NFKD \"(ĭ; ĭ; i◌̆; ĭ; i◌̆; ) LATIN SMALL LETTER I WITH BREVE\"");

    err = nfc_normaliser("\xC4\xAD", strlen("\xC4\xAD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xAD", "\xC4\xAD", out_buffer, "402 NFC \"(ĭ; ĭ; i◌̆; ĭ; i◌̆; ) LATIN SMALL LETTER I WITH BREVE\"");

    err = nfd_normaliser("\xC4\xAE", strlen("\xC4\xAE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xAE", "\x49" "\xCC\xA8", out_buffer, "403 NFD \"(Į; Į; I◌̨; Į; I◌̨; ) LATIN CAPITAL LETTER I WITH OGONEK\"");

    err = nfkd_normaliser("\xC4\xAE", strlen("\xC4\xAE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xAE", "\x49" "\xCC\xA8", out_buffer, "404 NFKD \"(Į; Į; I◌̨; Į; I◌̨; ) LATIN CAPITAL LETTER I WITH OGONEK\"");

    err = nfc_normaliser("\xC4\xAE", strlen("\xC4\xAE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xAE", "\xC4\xAE", out_buffer, "405 NFC \"(Į; Į; I◌̨; Į; I◌̨; ) LATIN CAPITAL LETTER I WITH OGONEK\"");

    err = nfd_normaliser("\xC4\xAF", strlen("\xC4\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xAF", "\x69" "\xCC\xA8", out_buffer, "406 NFD \"(į; į; i◌̨; į; i◌̨; ) LATIN SMALL LETTER I WITH OGONEK\"");

    err = nfkd_normaliser("\xC4\xAF", strlen("\xC4\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xAF", "\x69" "\xCC\xA8", out_buffer, "407 NFKD \"(į; į; i◌̨; į; i◌̨; ) LATIN SMALL LETTER I WITH OGONEK\"");

    err = nfc_normaliser("\xC4\xAF", strlen("\xC4\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xAF", "\xC4\xAF", out_buffer, "408 NFC \"(į; į; i◌̨; į; i◌̨; ) LATIN SMALL LETTER I WITH OGONEK\"");

    err = nfd_normaliser("\xC4\xB0", strlen("\xC4\xB0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB0", "\x49" "\xCC\x87", out_buffer, "409 NFD \"(İ; İ; I◌̇; İ; I◌̇; ) LATIN CAPITAL LETTER I WITH DOT ABOVE\"");

    err = nfkd_normaliser("\xC4\xB0", strlen("\xC4\xB0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB0", "\x49" "\xCC\x87", out_buffer, "410 NFKD \"(İ; İ; I◌̇; İ; I◌̇; ) LATIN CAPITAL LETTER I WITH DOT ABOVE\"");

    err = nfc_normaliser("\xC4\xB0", strlen("\xC4\xB0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB0", "\xC4\xB0", out_buffer, "411 NFC \"(İ; İ; I◌̇; İ; I◌̇; ) LATIN CAPITAL LETTER I WITH DOT ABOVE\"");

    err = nfd_normaliser("\xC4\xB2", strlen("\xC4\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB2", "\xC4\xB2", out_buffer, "412 NFD \"(Ĳ; Ĳ; Ĳ; IJ; IJ; ) LATIN CAPITAL LIGATURE IJ\"");

    err = nfkd_normaliser("\xC4\xB2", strlen("\xC4\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB2", "\x49" "\x4A", out_buffer, "413 NFKD \"(Ĳ; Ĳ; Ĳ; IJ; IJ; ) LATIN CAPITAL LIGATURE IJ\"");

    err = nfc_normaliser("\xC4\xB2", strlen("\xC4\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB2", "\xC4\xB2", out_buffer, "414 NFC \"(Ĳ; Ĳ; Ĳ; IJ; IJ; ) LATIN CAPITAL LIGATURE IJ\"");

    err = nfd_normaliser("\xC4\xB3", strlen("\xC4\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB3", "\xC4\xB3", out_buffer, "415 NFD \"(ĳ; ĳ; ĳ; ij; ij; ) LATIN SMALL LIGATURE IJ\"");

    err = nfkd_normaliser("\xC4\xB3", strlen("\xC4\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB3", "\x69" "\x6A", out_buffer, "416 NFKD \"(ĳ; ĳ; ĳ; ij; ij; ) LATIN SMALL LIGATURE IJ\"");

    err = nfc_normaliser("\xC4\xB3", strlen("\xC4\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB3", "\xC4\xB3", out_buffer, "417 NFC \"(ĳ; ĳ; ĳ; ij; ij; ) LATIN SMALL LIGATURE IJ\"");

    err = nfd_normaliser("\xC4\xB4", strlen("\xC4\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB4", "\x4A" "\xCC\x82", out_buffer, "418 NFD \"(Ĵ; Ĵ; J◌̂; Ĵ; J◌̂; ) LATIN CAPITAL LETTER J WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC4\xB4", strlen("\xC4\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB4", "\x4A" "\xCC\x82", out_buffer, "419 NFKD \"(Ĵ; Ĵ; J◌̂; Ĵ; J◌̂; ) LATIN CAPITAL LETTER J WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC4\xB4", strlen("\xC4\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB4", "\xC4\xB4", out_buffer, "420 NFC \"(Ĵ; Ĵ; J◌̂; Ĵ; J◌̂; ) LATIN CAPITAL LETTER J WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC4\xB5", strlen("\xC4\xB5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB5", "\x6A" "\xCC\x82", out_buffer, "421 NFD \"(ĵ; ĵ; j◌̂; ĵ; j◌̂; ) LATIN SMALL LETTER J WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC4\xB5", strlen("\xC4\xB5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB5", "\x6A" "\xCC\x82", out_buffer, "422 NFKD \"(ĵ; ĵ; j◌̂; ĵ; j◌̂; ) LATIN SMALL LETTER J WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC4\xB5", strlen("\xC4\xB5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB5", "\xC4\xB5", out_buffer, "423 NFC \"(ĵ; ĵ; j◌̂; ĵ; j◌̂; ) LATIN SMALL LETTER J WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC4\xB6", strlen("\xC4\xB6"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB6", "\x4B" "\xCC\xA7", out_buffer, "424 NFD \"(Ķ; Ķ; K◌̧; Ķ; K◌̧; ) LATIN CAPITAL LETTER K WITH CEDILLA\"");

    err = nfkd_normaliser("\xC4\xB6", strlen("\xC4\xB6"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB6", "\x4B" "\xCC\xA7", out_buffer, "425 NFKD \"(Ķ; Ķ; K◌̧; Ķ; K◌̧; ) LATIN CAPITAL LETTER K WITH CEDILLA\"");

    err = nfc_normaliser("\xC4\xB6", strlen("\xC4\xB6"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB6", "\xC4\xB6", out_buffer, "426 NFC \"(Ķ; Ķ; K◌̧; Ķ; K◌̧; ) LATIN CAPITAL LETTER K WITH CEDILLA\"");

    err = nfd_normaliser("\xC4\xB7", strlen("\xC4\xB7"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB7", "\x6B" "\xCC\xA7", out_buffer, "427 NFD \"(ķ; ķ; k◌̧; ķ; k◌̧; ) LATIN SMALL LETTER K WITH CEDILLA\"");

    err = nfkd_normaliser("\xC4\xB7", strlen("\xC4\xB7"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB7", "\x6B" "\xCC\xA7", out_buffer, "428 NFKD \"(ķ; ķ; k◌̧; ķ; k◌̧; ) LATIN SMALL LETTER K WITH CEDILLA\"");

    err = nfc_normaliser("\xC4\xB7", strlen("\xC4\xB7"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB7", "\xC4\xB7", out_buffer, "429 NFC \"(ķ; ķ; k◌̧; ķ; k◌̧; ) LATIN SMALL LETTER K WITH CEDILLA\"");

    err = nfd_normaliser("\xC4\xB9", strlen("\xC4\xB9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB9", "\x4C" "\xCC\x81", out_buffer, "430 NFD \"(Ĺ; Ĺ; L◌́; Ĺ; L◌́; ) LATIN CAPITAL LETTER L WITH ACUTE\"");

    err = nfkd_normaliser("\xC4\xB9", strlen("\xC4\xB9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB9", "\x4C" "\xCC\x81", out_buffer, "431 NFKD \"(Ĺ; Ĺ; L◌́; Ĺ; L◌́; ) LATIN CAPITAL LETTER L WITH ACUTE\"");

    err = nfc_normaliser("\xC4\xB9", strlen("\xC4\xB9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xB9", "\xC4\xB9", out_buffer, "432 NFC \"(Ĺ; Ĺ; L◌́; Ĺ; L◌́; ) LATIN CAPITAL LETTER L WITH ACUTE\"");

    err = nfd_normaliser("\xC4\xBA", strlen("\xC4\xBA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xBA", "\x6C" "\xCC\x81", out_buffer, "433 NFD \"(ĺ; ĺ; l◌́; ĺ; l◌́; ) LATIN SMALL LETTER L WITH ACUTE\"");

    err = nfkd_normaliser("\xC4\xBA", strlen("\xC4\xBA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xBA", "\x6C" "\xCC\x81", out_buffer, "434 NFKD \"(ĺ; ĺ; l◌́; ĺ; l◌́; ) LATIN SMALL LETTER L WITH ACUTE\"");

    err = nfc_normaliser("\xC4\xBA", strlen("\xC4\xBA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xBA", "\xC4\xBA", out_buffer, "435 NFC \"(ĺ; ĺ; l◌́; ĺ; l◌́; ) LATIN SMALL LETTER L WITH ACUTE\"");

    err = nfd_normaliser("\xC4\xBB", strlen("\xC4\xBB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xBB", "\x4C" "\xCC\xA7", out_buffer, "436 NFD \"(Ļ; Ļ; L◌̧; Ļ; L◌̧; ) LATIN CAPITAL LETTER L WITH CEDILLA\"");

    err = nfkd_normaliser("\xC4\xBB", strlen("\xC4\xBB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xBB", "\x4C" "\xCC\xA7", out_buffer, "437 NFKD \"(Ļ; Ļ; L◌̧; Ļ; L◌̧; ) LATIN CAPITAL LETTER L WITH CEDILLA\"");

    err = nfc_normaliser("\xC4\xBB", strlen("\xC4\xBB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xBB", "\xC4\xBB", out_buffer, "438 NFC \"(Ļ; Ļ; L◌̧; Ļ; L◌̧; ) LATIN CAPITAL LETTER L WITH CEDILLA\"");

    err = nfd_normaliser("\xC4\xBC", strlen("\xC4\xBC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xBC", "\x6C" "\xCC\xA7", out_buffer, "439 NFD \"(ļ; ļ; l◌̧; ļ; l◌̧; ) LATIN SMALL LETTER L WITH CEDILLA\"");

    err = nfkd_normaliser("\xC4\xBC", strlen("\xC4\xBC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xBC", "\x6C" "\xCC\xA7", out_buffer, "440 NFKD \"(ļ; ļ; l◌̧; ļ; l◌̧; ) LATIN SMALL LETTER L WITH CEDILLA\"");

    err = nfc_normaliser("\xC4\xBC", strlen("\xC4\xBC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xBC", "\xC4\xBC", out_buffer, "441 NFC \"(ļ; ļ; l◌̧; ļ; l◌̧; ) LATIN SMALL LETTER L WITH CEDILLA\"");

    err = nfd_normaliser("\xC4\xBD", strlen("\xC4\xBD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xBD", "\x4C" "\xCC\x8C", out_buffer, "442 NFD \"(Ľ; Ľ; L◌̌; Ľ; L◌̌; ) LATIN CAPITAL LETTER L WITH CARON\"");

    err = nfkd_normaliser("\xC4\xBD", strlen("\xC4\xBD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xBD", "\x4C" "\xCC\x8C", out_buffer, "443 NFKD \"(Ľ; Ľ; L◌̌; Ľ; L◌̌; ) LATIN CAPITAL LETTER L WITH CARON\"");

    err = nfc_normaliser("\xC4\xBD", strlen("\xC4\xBD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xBD", "\xC4\xBD", out_buffer, "444 NFC \"(Ľ; Ľ; L◌̌; Ľ; L◌̌; ) LATIN CAPITAL LETTER L WITH CARON\"");

    err = nfd_normaliser("\xC4\xBE", strlen("\xC4\xBE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xBE", "\x6C" "\xCC\x8C", out_buffer, "445 NFD \"(ľ; ľ; l◌̌; ľ; l◌̌; ) LATIN SMALL LETTER L WITH CARON\"");

    err = nfkd_normaliser("\xC4\xBE", strlen("\xC4\xBE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xBE", "\x6C" "\xCC\x8C", out_buffer, "446 NFKD \"(ľ; ľ; l◌̌; ľ; l◌̌; ) LATIN SMALL LETTER L WITH CARON\"");

    err = nfc_normaliser("\xC4\xBE", strlen("\xC4\xBE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xBE", "\xC4\xBE", out_buffer, "447 NFC \"(ľ; ľ; l◌̌; ľ; l◌̌; ) LATIN SMALL LETTER L WITH CARON\"");

    err = nfd_normaliser("\xC4\xBF", strlen("\xC4\xBF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xBF", "\xC4\xBF", out_buffer, "448 NFD \"(Ŀ; Ŀ; Ŀ; L·; L·; ) LATIN CAPITAL LETTER L WITH MIDDLE DOT\"");

    err = nfkd_normaliser("\xC4\xBF", strlen("\xC4\xBF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xBF", "\x4C" "\xC2\xB7", out_buffer, "449 NFKD \"(Ŀ; Ŀ; Ŀ; L·; L·; ) LATIN CAPITAL LETTER L WITH MIDDLE DOT\"");

    err = nfc_normaliser("\xC4\xBF", strlen("\xC4\xBF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC4\xBF", "\xC4\xBF", out_buffer, "450 NFC \"(Ŀ; Ŀ; Ŀ; L·; L·; ) LATIN CAPITAL LETTER L WITH MIDDLE DOT\"");

    err = nfd_normaliser("\xC5\x80", strlen("\xC5\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x80", "\xC5\x80", out_buffer, "451 NFD \"(ŀ; ŀ; ŀ; l·; l·; ) LATIN SMALL LETTER L WITH MIDDLE DOT\"");

    err = nfkd_normaliser("\xC5\x80", strlen("\xC5\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x80", "\x6C" "\xC2\xB7", out_buffer, "452 NFKD \"(ŀ; ŀ; ŀ; l·; l·; ) LATIN SMALL LETTER L WITH MIDDLE DOT\"");

    err = nfc_normaliser("\xC5\x80", strlen("\xC5\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x80", "\xC5\x80", out_buffer, "453 NFC \"(ŀ; ŀ; ŀ; l·; l·; ) LATIN SMALL LETTER L WITH MIDDLE DOT\"");

    err = nfd_normaliser("\xC5\x83", strlen("\xC5\x83"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x83", "\x4E" "\xCC\x81", out_buffer, "454 NFD \"(Ń; Ń; N◌́; Ń; N◌́; ) LATIN CAPITAL LETTER N WITH ACUTE\"");

    err = nfkd_normaliser("\xC5\x83", strlen("\xC5\x83"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x83", "\x4E" "\xCC\x81", out_buffer, "455 NFKD \"(Ń; Ń; N◌́; Ń; N◌́; ) LATIN CAPITAL LETTER N WITH ACUTE\"");

    err = nfc_normaliser("\xC5\x83", strlen("\xC5\x83"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x83", "\xC5\x83", out_buffer, "456 NFC \"(Ń; Ń; N◌́; Ń; N◌́; ) LATIN CAPITAL LETTER N WITH ACUTE\"");

    err = nfd_normaliser("\xC5\x84", strlen("\xC5\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x84", "\x6E" "\xCC\x81", out_buffer, "457 NFD \"(ń; ń; n◌́; ń; n◌́; ) LATIN SMALL LETTER N WITH ACUTE\"");

    err = nfkd_normaliser("\xC5\x84", strlen("\xC5\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x84", "\x6E" "\xCC\x81", out_buffer, "458 NFKD \"(ń; ń; n◌́; ń; n◌́; ) LATIN SMALL LETTER N WITH ACUTE\"");

    err = nfc_normaliser("\xC5\x84", strlen("\xC5\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x84", "\xC5\x84", out_buffer, "459 NFC \"(ń; ń; n◌́; ń; n◌́; ) LATIN SMALL LETTER N WITH ACUTE\"");

    err = nfd_normaliser("\xC5\x85", strlen("\xC5\x85"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x85", "\x4E" "\xCC\xA7", out_buffer, "460 NFD \"(Ņ; Ņ; N◌̧; Ņ; N◌̧; ) LATIN CAPITAL LETTER N WITH CEDILLA\"");

    err = nfkd_normaliser("\xC5\x85", strlen("\xC5\x85"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x85", "\x4E" "\xCC\xA7", out_buffer, "461 NFKD \"(Ņ; Ņ; N◌̧; Ņ; N◌̧; ) LATIN CAPITAL LETTER N WITH CEDILLA\"");

    err = nfc_normaliser("\xC5\x85", strlen("\xC5\x85"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x85", "\xC5\x85", out_buffer, "462 NFC \"(Ņ; Ņ; N◌̧; Ņ; N◌̧; ) LATIN CAPITAL LETTER N WITH CEDILLA\"");

    err = nfd_normaliser("\xC5\x86", strlen("\xC5\x86"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x86", "\x6E" "\xCC\xA7", out_buffer, "463 NFD \"(ņ; ņ; n◌̧; ņ; n◌̧; ) LATIN SMALL LETTER N WITH CEDILLA\"");

    err = nfkd_normaliser("\xC5\x86", strlen("\xC5\x86"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x86", "\x6E" "\xCC\xA7", out_buffer, "464 NFKD \"(ņ; ņ; n◌̧; ņ; n◌̧; ) LATIN SMALL LETTER N WITH CEDILLA\"");

    err = nfc_normaliser("\xC5\x86", strlen("\xC5\x86"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x86", "\xC5\x86", out_buffer, "465 NFC \"(ņ; ņ; n◌̧; ņ; n◌̧; ) LATIN SMALL LETTER N WITH CEDILLA\"");

    err = nfd_normaliser("\xC5\x87", strlen("\xC5\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x87", "\x4E" "\xCC\x8C", out_buffer, "466 NFD \"(Ň; Ň; N◌̌; Ň; N◌̌; ) LATIN CAPITAL LETTER N WITH CARON\"");

    err = nfkd_normaliser("\xC5\x87", strlen("\xC5\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x87", "\x4E" "\xCC\x8C", out_buffer, "467 NFKD \"(Ň; Ň; N◌̌; Ň; N◌̌; ) LATIN CAPITAL LETTER N WITH CARON\"");

    err = nfc_normaliser("\xC5\x87", strlen("\xC5\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x87", "\xC5\x87", out_buffer, "468 NFC \"(Ň; Ň; N◌̌; Ň; N◌̌; ) LATIN CAPITAL LETTER N WITH CARON\"");

    err = nfd_normaliser("\xC5\x88", strlen("\xC5\x88"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x88", "\x6E" "\xCC\x8C", out_buffer, "469 NFD \"(ň; ň; n◌̌; ň; n◌̌; ) LATIN SMALL LETTER N WITH CARON\"");

    err = nfkd_normaliser("\xC5\x88", strlen("\xC5\x88"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x88", "\x6E" "\xCC\x8C", out_buffer, "470 NFKD \"(ň; ň; n◌̌; ň; n◌̌; ) LATIN SMALL LETTER N WITH CARON\"");

    err = nfc_normaliser("\xC5\x88", strlen("\xC5\x88"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x88", "\xC5\x88", out_buffer, "471 NFC \"(ň; ň; n◌̌; ň; n◌̌; ) LATIN SMALL LETTER N WITH CARON\"");

    err = nfd_normaliser("\xC5\x89", strlen("\xC5\x89"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x89", "\xC5\x89", out_buffer, "472 NFD \"(ŉ; ŉ; ŉ; ʼn; ʼn; ) LATIN SMALL LETTER N PRECEDED BY APOSTROPHE\"");

    err = nfkd_normaliser("\xC5\x89", strlen("\xC5\x89"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x89", "\xCA\xBC" "\x6E", out_buffer, "473 NFKD \"(ŉ; ŉ; ŉ; ʼn; ʼn; ) LATIN SMALL LETTER N PRECEDED BY APOSTROPHE\"");

    err = nfc_normaliser("\xC5\x89", strlen("\xC5\x89"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x89", "\xC5\x89", out_buffer, "474 NFC \"(ŉ; ŉ; ŉ; ʼn; ʼn; ) LATIN SMALL LETTER N PRECEDED BY APOSTROPHE\"");

    err = nfd_normaliser("\xC5\x8C", strlen("\xC5\x8C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x8C", "\x4F" "\xCC\x84", out_buffer, "475 NFD \"(Ō; Ō; O◌̄; Ō; O◌̄; ) LATIN CAPITAL LETTER O WITH MACRON\"");

    err = nfkd_normaliser("\xC5\x8C", strlen("\xC5\x8C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x8C", "\x4F" "\xCC\x84", out_buffer, "476 NFKD \"(Ō; Ō; O◌̄; Ō; O◌̄; ) LATIN CAPITAL LETTER O WITH MACRON\"");

    err = nfc_normaliser("\xC5\x8C", strlen("\xC5\x8C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x8C", "\xC5\x8C", out_buffer, "477 NFC \"(Ō; Ō; O◌̄; Ō; O◌̄; ) LATIN CAPITAL LETTER O WITH MACRON\"");

    err = nfd_normaliser("\xC5\x8D", strlen("\xC5\x8D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x8D", "\x6F" "\xCC\x84", out_buffer, "478 NFD \"(ō; ō; o◌̄; ō; o◌̄; ) LATIN SMALL LETTER O WITH MACRON\"");

    err = nfkd_normaliser("\xC5\x8D", strlen("\xC5\x8D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x8D", "\x6F" "\xCC\x84", out_buffer, "479 NFKD \"(ō; ō; o◌̄; ō; o◌̄; ) LATIN SMALL LETTER O WITH MACRON\"");

    err = nfc_normaliser("\xC5\x8D", strlen("\xC5\x8D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x8D", "\xC5\x8D", out_buffer, "480 NFC \"(ō; ō; o◌̄; ō; o◌̄; ) LATIN SMALL LETTER O WITH MACRON\"");

    err = nfd_normaliser("\xC5\x8E", strlen("\xC5\x8E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x8E", "\x4F" "\xCC\x86", out_buffer, "481 NFD \"(Ŏ; Ŏ; O◌̆; Ŏ; O◌̆; ) LATIN CAPITAL LETTER O WITH BREVE\"");

    err = nfkd_normaliser("\xC5\x8E", strlen("\xC5\x8E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x8E", "\x4F" "\xCC\x86", out_buffer, "482 NFKD \"(Ŏ; Ŏ; O◌̆; Ŏ; O◌̆; ) LATIN CAPITAL LETTER O WITH BREVE\"");

    err = nfc_normaliser("\xC5\x8E", strlen("\xC5\x8E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x8E", "\xC5\x8E", out_buffer, "483 NFC \"(Ŏ; Ŏ; O◌̆; Ŏ; O◌̆; ) LATIN CAPITAL LETTER O WITH BREVE\"");

    err = nfd_normaliser("\xC5\x8F", strlen("\xC5\x8F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x8F", "\x6F" "\xCC\x86", out_buffer, "484 NFD \"(ŏ; ŏ; o◌̆; ŏ; o◌̆; ) LATIN SMALL LETTER O WITH BREVE\"");

    err = nfkd_normaliser("\xC5\x8F", strlen("\xC5\x8F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x8F", "\x6F" "\xCC\x86", out_buffer, "485 NFKD \"(ŏ; ŏ; o◌̆; ŏ; o◌̆; ) LATIN SMALL LETTER O WITH BREVE\"");

    err = nfc_normaliser("\xC5\x8F", strlen("\xC5\x8F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x8F", "\xC5\x8F", out_buffer, "486 NFC \"(ŏ; ŏ; o◌̆; ŏ; o◌̆; ) LATIN SMALL LETTER O WITH BREVE\"");

    err = nfd_normaliser("\xC5\x90", strlen("\xC5\x90"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x90", "\x4F" "\xCC\x8B", out_buffer, "487 NFD \"(Ő; Ő; O◌̋; Ő; O◌̋; ) LATIN CAPITAL LETTER O WITH DOUBLE ACUTE\"");

    err = nfkd_normaliser("\xC5\x90", strlen("\xC5\x90"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x90", "\x4F" "\xCC\x8B", out_buffer, "488 NFKD \"(Ő; Ő; O◌̋; Ő; O◌̋; ) LATIN CAPITAL LETTER O WITH DOUBLE ACUTE\"");

    err = nfc_normaliser("\xC5\x90", strlen("\xC5\x90"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x90", "\xC5\x90", out_buffer, "489 NFC \"(Ő; Ő; O◌̋; Ő; O◌̋; ) LATIN CAPITAL LETTER O WITH DOUBLE ACUTE\"");

    err = nfd_normaliser("\xC5\x91", strlen("\xC5\x91"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x91", "\x6F" "\xCC\x8B", out_buffer, "490 NFD \"(ő; ő; o◌̋; ő; o◌̋; ) LATIN SMALL LETTER O WITH DOUBLE ACUTE\"");

    err = nfkd_normaliser("\xC5\x91", strlen("\xC5\x91"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x91", "\x6F" "\xCC\x8B", out_buffer, "491 NFKD \"(ő; ő; o◌̋; ő; o◌̋; ) LATIN SMALL LETTER O WITH DOUBLE ACUTE\"");

    err = nfc_normaliser("\xC5\x91", strlen("\xC5\x91"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x91", "\xC5\x91", out_buffer, "492 NFC \"(ő; ő; o◌̋; ő; o◌̋; ) LATIN SMALL LETTER O WITH DOUBLE ACUTE\"");

    err = nfd_normaliser("\xC5\x94", strlen("\xC5\x94"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x94", "\x52" "\xCC\x81", out_buffer, "493 NFD \"(Ŕ; Ŕ; R◌́; Ŕ; R◌́; ) LATIN CAPITAL LETTER R WITH ACUTE\"");

    err = nfkd_normaliser("\xC5\x94", strlen("\xC5\x94"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x94", "\x52" "\xCC\x81", out_buffer, "494 NFKD \"(Ŕ; Ŕ; R◌́; Ŕ; R◌́; ) LATIN CAPITAL LETTER R WITH ACUTE\"");

    err = nfc_normaliser("\xC5\x94", strlen("\xC5\x94"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x94", "\xC5\x94", out_buffer, "495 NFC \"(Ŕ; Ŕ; R◌́; Ŕ; R◌́; ) LATIN CAPITAL LETTER R WITH ACUTE\"");

    err = nfd_normaliser("\xC5\x95", strlen("\xC5\x95"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x95", "\x72" "\xCC\x81", out_buffer, "496 NFD \"(ŕ; ŕ; r◌́; ŕ; r◌́; ) LATIN SMALL LETTER R WITH ACUTE\"");

    err = nfkd_normaliser("\xC5\x95", strlen("\xC5\x95"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x95", "\x72" "\xCC\x81", out_buffer, "497 NFKD \"(ŕ; ŕ; r◌́; ŕ; r◌́; ) LATIN SMALL LETTER R WITH ACUTE\"");

    err = nfc_normaliser("\xC5\x95", strlen("\xC5\x95"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x95", "\xC5\x95", out_buffer, "498 NFC \"(ŕ; ŕ; r◌́; ŕ; r◌́; ) LATIN SMALL LETTER R WITH ACUTE\"");

    err = nfd_normaliser("\xC5\x96", strlen("\xC5\x96"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x96", "\x52" "\xCC\xA7", out_buffer, "499 NFD \"(Ŗ; Ŗ; R◌̧; Ŗ; R◌̧; ) LATIN CAPITAL LETTER R WITH CEDILLA\"");

    err = nfkd_normaliser("\xC5\x96", strlen("\xC5\x96"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x96", "\x52" "\xCC\xA7", out_buffer, "500 NFKD \"(Ŗ; Ŗ; R◌̧; Ŗ; R◌̧; ) LATIN CAPITAL LETTER R WITH CEDILLA\"");

    err = nfc_normaliser("\xC5\x96", strlen("\xC5\x96"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x96", "\xC5\x96", out_buffer, "501 NFC \"(Ŗ; Ŗ; R◌̧; Ŗ; R◌̧; ) LATIN CAPITAL LETTER R WITH CEDILLA\"");

    err = nfd_normaliser("\xC5\x97", strlen("\xC5\x97"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x97", "\x72" "\xCC\xA7", out_buffer, "502 NFD \"(ŗ; ŗ; r◌̧; ŗ; r◌̧; ) LATIN SMALL LETTER R WITH CEDILLA\"");

    err = nfkd_normaliser("\xC5\x97", strlen("\xC5\x97"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x97", "\x72" "\xCC\xA7", out_buffer, "503 NFKD \"(ŗ; ŗ; r◌̧; ŗ; r◌̧; ) LATIN SMALL LETTER R WITH CEDILLA\"");

    err = nfc_normaliser("\xC5\x97", strlen("\xC5\x97"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x97", "\xC5\x97", out_buffer, "504 NFC \"(ŗ; ŗ; r◌̧; ŗ; r◌̧; ) LATIN SMALL LETTER R WITH CEDILLA\"");

    err = nfd_normaliser("\xC5\x98", strlen("\xC5\x98"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x98", "\x52" "\xCC\x8C", out_buffer, "505 NFD \"(Ř; Ř; R◌̌; Ř; R◌̌; ) LATIN CAPITAL LETTER R WITH CARON\"");

    err = nfkd_normaliser("\xC5\x98", strlen("\xC5\x98"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x98", "\x52" "\xCC\x8C", out_buffer, "506 NFKD \"(Ř; Ř; R◌̌; Ř; R◌̌; ) LATIN CAPITAL LETTER R WITH CARON\"");

    err = nfc_normaliser("\xC5\x98", strlen("\xC5\x98"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x98", "\xC5\x98", out_buffer, "507 NFC \"(Ř; Ř; R◌̌; Ř; R◌̌; ) LATIN CAPITAL LETTER R WITH CARON\"");

    err = nfd_normaliser("\xC5\x99", strlen("\xC5\x99"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x99", "\x72" "\xCC\x8C", out_buffer, "508 NFD \"(ř; ř; r◌̌; ř; r◌̌; ) LATIN SMALL LETTER R WITH CARON\"");

    err = nfkd_normaliser("\xC5\x99", strlen("\xC5\x99"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x99", "\x72" "\xCC\x8C", out_buffer, "509 NFKD \"(ř; ř; r◌̌; ř; r◌̌; ) LATIN SMALL LETTER R WITH CARON\"");

    err = nfc_normaliser("\xC5\x99", strlen("\xC5\x99"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x99", "\xC5\x99", out_buffer, "510 NFC \"(ř; ř; r◌̌; ř; r◌̌; ) LATIN SMALL LETTER R WITH CARON\"");

    err = nfd_normaliser("\xC5\x9A", strlen("\xC5\x9A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x9A", "\x53" "\xCC\x81", out_buffer, "511 NFD \"(Ś; Ś; S◌́; Ś; S◌́; ) LATIN CAPITAL LETTER S WITH ACUTE\"");

    err = nfkd_normaliser("\xC5\x9A", strlen("\xC5\x9A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x9A", "\x53" "\xCC\x81", out_buffer, "512 NFKD \"(Ś; Ś; S◌́; Ś; S◌́; ) LATIN CAPITAL LETTER S WITH ACUTE\"");

    err = nfc_normaliser("\xC5\x9A", strlen("\xC5\x9A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x9A", "\xC5\x9A", out_buffer, "513 NFC \"(Ś; Ś; S◌́; Ś; S◌́; ) LATIN CAPITAL LETTER S WITH ACUTE\"");

    err = nfd_normaliser("\xC5\x9B", strlen("\xC5\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x9B", "\x73" "\xCC\x81", out_buffer, "514 NFD \"(ś; ś; s◌́; ś; s◌́; ) LATIN SMALL LETTER S WITH ACUTE\"");

    err = nfkd_normaliser("\xC5\x9B", strlen("\xC5\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x9B", "\x73" "\xCC\x81", out_buffer, "515 NFKD \"(ś; ś; s◌́; ś; s◌́; ) LATIN SMALL LETTER S WITH ACUTE\"");

    err = nfc_normaliser("\xC5\x9B", strlen("\xC5\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x9B", "\xC5\x9B", out_buffer, "516 NFC \"(ś; ś; s◌́; ś; s◌́; ) LATIN SMALL LETTER S WITH ACUTE\"");

    err = nfd_normaliser("\xC5\x9C", strlen("\xC5\x9C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x9C", "\x53" "\xCC\x82", out_buffer, "517 NFD \"(Ŝ; Ŝ; S◌̂; Ŝ; S◌̂; ) LATIN CAPITAL LETTER S WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC5\x9C", strlen("\xC5\x9C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x9C", "\x53" "\xCC\x82", out_buffer, "518 NFKD \"(Ŝ; Ŝ; S◌̂; Ŝ; S◌̂; ) LATIN CAPITAL LETTER S WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC5\x9C", strlen("\xC5\x9C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x9C", "\xC5\x9C", out_buffer, "519 NFC \"(Ŝ; Ŝ; S◌̂; Ŝ; S◌̂; ) LATIN CAPITAL LETTER S WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC5\x9D", strlen("\xC5\x9D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x9D", "\x73" "\xCC\x82", out_buffer, "520 NFD \"(ŝ; ŝ; s◌̂; ŝ; s◌̂; ) LATIN SMALL LETTER S WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC5\x9D", strlen("\xC5\x9D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x9D", "\x73" "\xCC\x82", out_buffer, "521 NFKD \"(ŝ; ŝ; s◌̂; ŝ; s◌̂; ) LATIN SMALL LETTER S WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC5\x9D", strlen("\xC5\x9D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x9D", "\xC5\x9D", out_buffer, "522 NFC \"(ŝ; ŝ; s◌̂; ŝ; s◌̂; ) LATIN SMALL LETTER S WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC5\x9E", strlen("\xC5\x9E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x9E", "\x53" "\xCC\xA7", out_buffer, "523 NFD \"(Ş; Ş; S◌̧; Ş; S◌̧; ) LATIN CAPITAL LETTER S WITH CEDILLA\"");

    err = nfkd_normaliser("\xC5\x9E", strlen("\xC5\x9E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x9E", "\x53" "\xCC\xA7", out_buffer, "524 NFKD \"(Ş; Ş; S◌̧; Ş; S◌̧; ) LATIN CAPITAL LETTER S WITH CEDILLA\"");

    err = nfc_normaliser("\xC5\x9E", strlen("\xC5\x9E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x9E", "\xC5\x9E", out_buffer, "525 NFC \"(Ş; Ş; S◌̧; Ş; S◌̧; ) LATIN CAPITAL LETTER S WITH CEDILLA\"");

    err = nfd_normaliser("\xC5\x9F", strlen("\xC5\x9F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x9F", "\x73" "\xCC\xA7", out_buffer, "526 NFD \"(ş; ş; s◌̧; ş; s◌̧; ) LATIN SMALL LETTER S WITH CEDILLA\"");

    err = nfkd_normaliser("\xC5\x9F", strlen("\xC5\x9F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x9F", "\x73" "\xCC\xA7", out_buffer, "527 NFKD \"(ş; ş; s◌̧; ş; s◌̧; ) LATIN SMALL LETTER S WITH CEDILLA\"");

    err = nfc_normaliser("\xC5\x9F", strlen("\xC5\x9F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\x9F", "\xC5\x9F", out_buffer, "528 NFC \"(ş; ş; s◌̧; ş; s◌̧; ) LATIN SMALL LETTER S WITH CEDILLA\"");

    err = nfd_normaliser("\xC5\xA0", strlen("\xC5\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA0", "\x53" "\xCC\x8C", out_buffer, "529 NFD \"(Š; Š; S◌̌; Š; S◌̌; ) LATIN CAPITAL LETTER S WITH CARON\"");

    err = nfkd_normaliser("\xC5\xA0", strlen("\xC5\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA0", "\x53" "\xCC\x8C", out_buffer, "530 NFKD \"(Š; Š; S◌̌; Š; S◌̌; ) LATIN CAPITAL LETTER S WITH CARON\"");

    err = nfc_normaliser("\xC5\xA0", strlen("\xC5\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA0", "\xC5\xA0", out_buffer, "531 NFC \"(Š; Š; S◌̌; Š; S◌̌; ) LATIN CAPITAL LETTER S WITH CARON\"");

    err = nfd_normaliser("\xC5\xA1", strlen("\xC5\xA1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA1", "\x73" "\xCC\x8C", out_buffer, "532 NFD \"(š; š; s◌̌; š; s◌̌; ) LATIN SMALL LETTER S WITH CARON\"");

    err = nfkd_normaliser("\xC5\xA1", strlen("\xC5\xA1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA1", "\x73" "\xCC\x8C", out_buffer, "533 NFKD \"(š; š; s◌̌; š; s◌̌; ) LATIN SMALL LETTER S WITH CARON\"");

    err = nfc_normaliser("\xC5\xA1", strlen("\xC5\xA1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA1", "\xC5\xA1", out_buffer, "534 NFC \"(š; š; s◌̌; š; s◌̌; ) LATIN SMALL LETTER S WITH CARON\"");

    err = nfd_normaliser("\xC5\xA2", strlen("\xC5\xA2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA2", "\x54" "\xCC\xA7", out_buffer, "535 NFD \"(Ţ; Ţ; T◌̧; Ţ; T◌̧; ) LATIN CAPITAL LETTER T WITH CEDILLA\"");

    err = nfkd_normaliser("\xC5\xA2", strlen("\xC5\xA2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA2", "\x54" "\xCC\xA7", out_buffer, "536 NFKD \"(Ţ; Ţ; T◌̧; Ţ; T◌̧; ) LATIN CAPITAL LETTER T WITH CEDILLA\"");

    err = nfc_normaliser("\xC5\xA2", strlen("\xC5\xA2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA2", "\xC5\xA2", out_buffer, "537 NFC \"(Ţ; Ţ; T◌̧; Ţ; T◌̧; ) LATIN CAPITAL LETTER T WITH CEDILLA\"");

    err = nfd_normaliser("\xC5\xA3", strlen("\xC5\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA3", "\x74" "\xCC\xA7", out_buffer, "538 NFD \"(ţ; ţ; t◌̧; ţ; t◌̧; ) LATIN SMALL LETTER T WITH CEDILLA\"");

    err = nfkd_normaliser("\xC5\xA3", strlen("\xC5\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA3", "\x74" "\xCC\xA7", out_buffer, "539 NFKD \"(ţ; ţ; t◌̧; ţ; t◌̧; ) LATIN SMALL LETTER T WITH CEDILLA\"");

    err = nfc_normaliser("\xC5\xA3", strlen("\xC5\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA3", "\xC5\xA3", out_buffer, "540 NFC \"(ţ; ţ; t◌̧; ţ; t◌̧; ) LATIN SMALL LETTER T WITH CEDILLA\"");

    err = nfd_normaliser("\xC5\xA4", strlen("\xC5\xA4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA4", "\x54" "\xCC\x8C", out_buffer, "541 NFD \"(Ť; Ť; T◌̌; Ť; T◌̌; ) LATIN CAPITAL LETTER T WITH CARON\"");

    err = nfkd_normaliser("\xC5\xA4", strlen("\xC5\xA4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA4", "\x54" "\xCC\x8C", out_buffer, "542 NFKD \"(Ť; Ť; T◌̌; Ť; T◌̌; ) LATIN CAPITAL LETTER T WITH CARON\"");

    err = nfc_normaliser("\xC5\xA4", strlen("\xC5\xA4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA4", "\xC5\xA4", out_buffer, "543 NFC \"(Ť; Ť; T◌̌; Ť; T◌̌; ) LATIN CAPITAL LETTER T WITH CARON\"");

    err = nfd_normaliser("\xC5\xA5", strlen("\xC5\xA5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA5", "\x74" "\xCC\x8C", out_buffer, "544 NFD \"(ť; ť; t◌̌; ť; t◌̌; ) LATIN SMALL LETTER T WITH CARON\"");

    err = nfkd_normaliser("\xC5\xA5", strlen("\xC5\xA5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA5", "\x74" "\xCC\x8C", out_buffer, "545 NFKD \"(ť; ť; t◌̌; ť; t◌̌; ) LATIN SMALL LETTER T WITH CARON\"");

    err = nfc_normaliser("\xC5\xA5", strlen("\xC5\xA5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA5", "\xC5\xA5", out_buffer, "546 NFC \"(ť; ť; t◌̌; ť; t◌̌; ) LATIN SMALL LETTER T WITH CARON\"");

    err = nfd_normaliser("\xC5\xA8", strlen("\xC5\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA8", "\x55" "\xCC\x83", out_buffer, "547 NFD \"(Ũ; Ũ; U◌̃; Ũ; U◌̃; ) LATIN CAPITAL LETTER U WITH TILDE\"");

    err = nfkd_normaliser("\xC5\xA8", strlen("\xC5\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA8", "\x55" "\xCC\x83", out_buffer, "548 NFKD \"(Ũ; Ũ; U◌̃; Ũ; U◌̃; ) LATIN CAPITAL LETTER U WITH TILDE\"");

    err = nfc_normaliser("\xC5\xA8", strlen("\xC5\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA8", "\xC5\xA8", out_buffer, "549 NFC \"(Ũ; Ũ; U◌̃; Ũ; U◌̃; ) LATIN CAPITAL LETTER U WITH TILDE\"");

    err = nfd_normaliser("\xC5\xA9", strlen("\xC5\xA9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA9", "\x75" "\xCC\x83", out_buffer, "550 NFD \"(ũ; ũ; u◌̃; ũ; u◌̃; ) LATIN SMALL LETTER U WITH TILDE\"");

    err = nfkd_normaliser("\xC5\xA9", strlen("\xC5\xA9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA9", "\x75" "\xCC\x83", out_buffer, "551 NFKD \"(ũ; ũ; u◌̃; ũ; u◌̃; ) LATIN SMALL LETTER U WITH TILDE\"");

    err = nfc_normaliser("\xC5\xA9", strlen("\xC5\xA9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xA9", "\xC5\xA9", out_buffer, "552 NFC \"(ũ; ũ; u◌̃; ũ; u◌̃; ) LATIN SMALL LETTER U WITH TILDE\"");

    err = nfd_normaliser("\xC5\xAA", strlen("\xC5\xAA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xAA", "\x55" "\xCC\x84", out_buffer, "553 NFD \"(Ū; Ū; U◌̄; Ū; U◌̄; ) LATIN CAPITAL LETTER U WITH MACRON\"");

    err = nfkd_normaliser("\xC5\xAA", strlen("\xC5\xAA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xAA", "\x55" "\xCC\x84", out_buffer, "554 NFKD \"(Ū; Ū; U◌̄; Ū; U◌̄; ) LATIN CAPITAL LETTER U WITH MACRON\"");

    err = nfc_normaliser("\xC5\xAA", strlen("\xC5\xAA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xAA", "\xC5\xAA", out_buffer, "555 NFC \"(Ū; Ū; U◌̄; Ū; U◌̄; ) LATIN CAPITAL LETTER U WITH MACRON\"");

    err = nfd_normaliser("\xC5\xAB", strlen("\xC5\xAB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xAB", "\x75" "\xCC\x84", out_buffer, "556 NFD \"(ū; ū; u◌̄; ū; u◌̄; ) LATIN SMALL LETTER U WITH MACRON\"");

    err = nfkd_normaliser("\xC5\xAB", strlen("\xC5\xAB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xAB", "\x75" "\xCC\x84", out_buffer, "557 NFKD \"(ū; ū; u◌̄; ū; u◌̄; ) LATIN SMALL LETTER U WITH MACRON\"");

    err = nfc_normaliser("\xC5\xAB", strlen("\xC5\xAB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xAB", "\xC5\xAB", out_buffer, "558 NFC \"(ū; ū; u◌̄; ū; u◌̄; ) LATIN SMALL LETTER U WITH MACRON\"");

    err = nfd_normaliser("\xC5\xAC", strlen("\xC5\xAC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xAC", "\x55" "\xCC\x86", out_buffer, "559 NFD \"(Ŭ; Ŭ; U◌̆; Ŭ; U◌̆; ) LATIN CAPITAL LETTER U WITH BREVE\"");

    err = nfkd_normaliser("\xC5\xAC", strlen("\xC5\xAC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xAC", "\x55" "\xCC\x86", out_buffer, "560 NFKD \"(Ŭ; Ŭ; U◌̆; Ŭ; U◌̆; ) LATIN CAPITAL LETTER U WITH BREVE\"");

    err = nfc_normaliser("\xC5\xAC", strlen("\xC5\xAC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xAC", "\xC5\xAC", out_buffer, "561 NFC \"(Ŭ; Ŭ; U◌̆; Ŭ; U◌̆; ) LATIN CAPITAL LETTER U WITH BREVE\"");

    err = nfd_normaliser("\xC5\xAD", strlen("\xC5\xAD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xAD", "\x75" "\xCC\x86", out_buffer, "562 NFD \"(ŭ; ŭ; u◌̆; ŭ; u◌̆; ) LATIN SMALL LETTER U WITH BREVE\"");

    err = nfkd_normaliser("\xC5\xAD", strlen("\xC5\xAD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xAD", "\x75" "\xCC\x86", out_buffer, "563 NFKD \"(ŭ; ŭ; u◌̆; ŭ; u◌̆; ) LATIN SMALL LETTER U WITH BREVE\"");

    err = nfc_normaliser("\xC5\xAD", strlen("\xC5\xAD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xAD", "\xC5\xAD", out_buffer, "564 NFC \"(ŭ; ŭ; u◌̆; ŭ; u◌̆; ) LATIN SMALL LETTER U WITH BREVE\"");

    err = nfd_normaliser("\xC5\xAE", strlen("\xC5\xAE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xAE", "\x55" "\xCC\x8A", out_buffer, "565 NFD \"(Ů; Ů; U◌̊; Ů; U◌̊; ) LATIN CAPITAL LETTER U WITH RING ABOVE\"");

    err = nfkd_normaliser("\xC5\xAE", strlen("\xC5\xAE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xAE", "\x55" "\xCC\x8A", out_buffer, "566 NFKD \"(Ů; Ů; U◌̊; Ů; U◌̊; ) LATIN CAPITAL LETTER U WITH RING ABOVE\"");

    err = nfc_normaliser("\xC5\xAE", strlen("\xC5\xAE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xAE", "\xC5\xAE", out_buffer, "567 NFC \"(Ů; Ů; U◌̊; Ů; U◌̊; ) LATIN CAPITAL LETTER U WITH RING ABOVE\"");

    err = nfd_normaliser("\xC5\xAF", strlen("\xC5\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xAF", "\x75" "\xCC\x8A", out_buffer, "568 NFD \"(ů; ů; u◌̊; ů; u◌̊; ) LATIN SMALL LETTER U WITH RING ABOVE\"");

    err = nfkd_normaliser("\xC5\xAF", strlen("\xC5\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xAF", "\x75" "\xCC\x8A", out_buffer, "569 NFKD \"(ů; ů; u◌̊; ů; u◌̊; ) LATIN SMALL LETTER U WITH RING ABOVE\"");

    err = nfc_normaliser("\xC5\xAF", strlen("\xC5\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xAF", "\xC5\xAF", out_buffer, "570 NFC \"(ů; ů; u◌̊; ů; u◌̊; ) LATIN SMALL LETTER U WITH RING ABOVE\"");

    err = nfd_normaliser("\xC5\xB0", strlen("\xC5\xB0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB0", "\x55" "\xCC\x8B", out_buffer, "571 NFD \"(Ű; Ű; U◌̋; Ű; U◌̋; ) LATIN CAPITAL LETTER U WITH DOUBLE ACUTE\"");

    err = nfkd_normaliser("\xC5\xB0", strlen("\xC5\xB0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB0", "\x55" "\xCC\x8B", out_buffer, "572 NFKD \"(Ű; Ű; U◌̋; Ű; U◌̋; ) LATIN CAPITAL LETTER U WITH DOUBLE ACUTE\"");

    err = nfc_normaliser("\xC5\xB0", strlen("\xC5\xB0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB0", "\xC5\xB0", out_buffer, "573 NFC \"(Ű; Ű; U◌̋; Ű; U◌̋; ) LATIN CAPITAL LETTER U WITH DOUBLE ACUTE\"");

    err = nfd_normaliser("\xC5\xB1", strlen("\xC5\xB1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB1", "\x75" "\xCC\x8B", out_buffer, "574 NFD \"(ű; ű; u◌̋; ű; u◌̋; ) LATIN SMALL LETTER U WITH DOUBLE ACUTE\"");

    err = nfkd_normaliser("\xC5\xB1", strlen("\xC5\xB1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB1", "\x75" "\xCC\x8B", out_buffer, "575 NFKD \"(ű; ű; u◌̋; ű; u◌̋; ) LATIN SMALL LETTER U WITH DOUBLE ACUTE\"");

    err = nfc_normaliser("\xC5\xB1", strlen("\xC5\xB1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB1", "\xC5\xB1", out_buffer, "576 NFC \"(ű; ű; u◌̋; ű; u◌̋; ) LATIN SMALL LETTER U WITH DOUBLE ACUTE\"");

    err = nfd_normaliser("\xC5\xB2", strlen("\xC5\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB2", "\x55" "\xCC\xA8", out_buffer, "577 NFD \"(Ų; Ų; U◌̨; Ų; U◌̨; ) LATIN CAPITAL LETTER U WITH OGONEK\"");

    err = nfkd_normaliser("\xC5\xB2", strlen("\xC5\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB2", "\x55" "\xCC\xA8", out_buffer, "578 NFKD \"(Ų; Ų; U◌̨; Ų; U◌̨; ) LATIN CAPITAL LETTER U WITH OGONEK\"");

    err = nfc_normaliser("\xC5\xB2", strlen("\xC5\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB2", "\xC5\xB2", out_buffer, "579 NFC \"(Ų; Ų; U◌̨; Ų; U◌̨; ) LATIN CAPITAL LETTER U WITH OGONEK\"");

    err = nfd_normaliser("\xC5\xB3", strlen("\xC5\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB3", "\x75" "\xCC\xA8", out_buffer, "580 NFD \"(ų; ų; u◌̨; ų; u◌̨; ) LATIN SMALL LETTER U WITH OGONEK\"");

    err = nfkd_normaliser("\xC5\xB3", strlen("\xC5\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB3", "\x75" "\xCC\xA8", out_buffer, "581 NFKD \"(ų; ų; u◌̨; ų; u◌̨; ) LATIN SMALL LETTER U WITH OGONEK\"");

    err = nfc_normaliser("\xC5\xB3", strlen("\xC5\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB3", "\xC5\xB3", out_buffer, "582 NFC \"(ų; ų; u◌̨; ų; u◌̨; ) LATIN SMALL LETTER U WITH OGONEK\"");

    err = nfd_normaliser("\xC5\xB4", strlen("\xC5\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB4", "\x57" "\xCC\x82", out_buffer, "583 NFD \"(Ŵ; Ŵ; W◌̂; Ŵ; W◌̂; ) LATIN CAPITAL LETTER W WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC5\xB4", strlen("\xC5\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB4", "\x57" "\xCC\x82", out_buffer, "584 NFKD \"(Ŵ; Ŵ; W◌̂; Ŵ; W◌̂; ) LATIN CAPITAL LETTER W WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC5\xB4", strlen("\xC5\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB4", "\xC5\xB4", out_buffer, "585 NFC \"(Ŵ; Ŵ; W◌̂; Ŵ; W◌̂; ) LATIN CAPITAL LETTER W WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC5\xB5", strlen("\xC5\xB5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB5", "\x77" "\xCC\x82", out_buffer, "586 NFD \"(ŵ; ŵ; w◌̂; ŵ; w◌̂; ) LATIN SMALL LETTER W WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC5\xB5", strlen("\xC5\xB5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB5", "\x77" "\xCC\x82", out_buffer, "587 NFKD \"(ŵ; ŵ; w◌̂; ŵ; w◌̂; ) LATIN SMALL LETTER W WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC5\xB5", strlen("\xC5\xB5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB5", "\xC5\xB5", out_buffer, "588 NFC \"(ŵ; ŵ; w◌̂; ŵ; w◌̂; ) LATIN SMALL LETTER W WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC5\xB6", strlen("\xC5\xB6"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB6", "\x59" "\xCC\x82", out_buffer, "589 NFD \"(Ŷ; Ŷ; Y◌̂; Ŷ; Y◌̂; ) LATIN CAPITAL LETTER Y WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC5\xB6", strlen("\xC5\xB6"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB6", "\x59" "\xCC\x82", out_buffer, "590 NFKD \"(Ŷ; Ŷ; Y◌̂; Ŷ; Y◌̂; ) LATIN CAPITAL LETTER Y WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC5\xB6", strlen("\xC5\xB6"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB6", "\xC5\xB6", out_buffer, "591 NFC \"(Ŷ; Ŷ; Y◌̂; Ŷ; Y◌̂; ) LATIN CAPITAL LETTER Y WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC5\xB7", strlen("\xC5\xB7"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB7", "\x79" "\xCC\x82", out_buffer, "592 NFD \"(ŷ; ŷ; y◌̂; ŷ; y◌̂; ) LATIN SMALL LETTER Y WITH CIRCUMFLEX\"");

    err = nfkd_normaliser("\xC5\xB7", strlen("\xC5\xB7"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB7", "\x79" "\xCC\x82", out_buffer, "593 NFKD \"(ŷ; ŷ; y◌̂; ŷ; y◌̂; ) LATIN SMALL LETTER Y WITH CIRCUMFLEX\"");

    err = nfc_normaliser("\xC5\xB7", strlen("\xC5\xB7"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB7", "\xC5\xB7", out_buffer, "594 NFC \"(ŷ; ŷ; y◌̂; ŷ; y◌̂; ) LATIN SMALL LETTER Y WITH CIRCUMFLEX\"");

    err = nfd_normaliser("\xC5\xB8", strlen("\xC5\xB8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB8", "\x59" "\xCC\x88", out_buffer, "595 NFD \"(Ÿ; Ÿ; Y◌̈; Ÿ; Y◌̈; ) LATIN CAPITAL LETTER Y WITH DIAERESIS\"");

    err = nfkd_normaliser("\xC5\xB8", strlen("\xC5\xB8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB8", "\x59" "\xCC\x88", out_buffer, "596 NFKD \"(Ÿ; Ÿ; Y◌̈; Ÿ; Y◌̈; ) LATIN CAPITAL LETTER Y WITH DIAERESIS\"");

    err = nfc_normaliser("\xC5\xB8", strlen("\xC5\xB8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB8", "\xC5\xB8", out_buffer, "597 NFC \"(Ÿ; Ÿ; Y◌̈; Ÿ; Y◌̈; ) LATIN CAPITAL LETTER Y WITH DIAERESIS\"");

    err = nfd_normaliser("\xC5\xB9", strlen("\xC5\xB9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB9", "\x5A" "\xCC\x81", out_buffer, "598 NFD \"(Ź; Ź; Z◌́; Ź; Z◌́; ) LATIN CAPITAL LETTER Z WITH ACUTE\"");

    err = nfkd_normaliser("\xC5\xB9", strlen("\xC5\xB9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB9", "\x5A" "\xCC\x81", out_buffer, "599 NFKD \"(Ź; Ź; Z◌́; Ź; Z◌́; ) LATIN CAPITAL LETTER Z WITH ACUTE\"");

    err = nfc_normaliser("\xC5\xB9", strlen("\xC5\xB9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xB9", "\xC5\xB9", out_buffer, "600 NFC \"(Ź; Ź; Z◌́; Ź; Z◌́; ) LATIN CAPITAL LETTER Z WITH ACUTE\"");

    err = nfd_normaliser("\xC5\xBA", strlen("\xC5\xBA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xBA", "\x7A" "\xCC\x81", out_buffer, "601 NFD \"(ź; ź; z◌́; ź; z◌́; ) LATIN SMALL LETTER Z WITH ACUTE\"");

    err = nfkd_normaliser("\xC5\xBA", strlen("\xC5\xBA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xBA", "\x7A" "\xCC\x81", out_buffer, "602 NFKD \"(ź; ź; z◌́; ź; z◌́; ) LATIN SMALL LETTER Z WITH ACUTE\"");

    err = nfc_normaliser("\xC5\xBA", strlen("\xC5\xBA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xBA", "\xC5\xBA", out_buffer, "603 NFC \"(ź; ź; z◌́; ź; z◌́; ) LATIN SMALL LETTER Z WITH ACUTE\"");

    err = nfd_normaliser("\xC5\xBB", strlen("\xC5\xBB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xBB", "\x5A" "\xCC\x87", out_buffer, "604 NFD \"(Ż; Ż; Z◌̇; Ż; Z◌̇; ) LATIN CAPITAL LETTER Z WITH DOT ABOVE\"");

    err = nfkd_normaliser("\xC5\xBB", strlen("\xC5\xBB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xBB", "\x5A" "\xCC\x87", out_buffer, "605 NFKD \"(Ż; Ż; Z◌̇; Ż; Z◌̇; ) LATIN CAPITAL LETTER Z WITH DOT ABOVE\"");

    err = nfc_normaliser("\xC5\xBB", strlen("\xC5\xBB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xBB", "\xC5\xBB", out_buffer, "606 NFC \"(Ż; Ż; Z◌̇; Ż; Z◌̇; ) LATIN CAPITAL LETTER Z WITH DOT ABOVE\"");

    err = nfd_normaliser("\xC5\xBC", strlen("\xC5\xBC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xBC", "\x7A" "\xCC\x87", out_buffer, "607 NFD \"(ż; ż; z◌̇; ż; z◌̇; ) LATIN SMALL LETTER Z WITH DOT ABOVE\"");

    err = nfkd_normaliser("\xC5\xBC", strlen("\xC5\xBC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xBC", "\x7A" "\xCC\x87", out_buffer, "608 NFKD \"(ż; ż; z◌̇; ż; z◌̇; ) LATIN SMALL LETTER Z WITH DOT ABOVE\"");

    err = nfc_normaliser("\xC5\xBC", strlen("\xC5\xBC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xBC", "\xC5\xBC", out_buffer, "609 NFC \"(ż; ż; z◌̇; ż; z◌̇; ) LATIN SMALL LETTER Z WITH DOT ABOVE\"");

    err = nfd_normaliser("\xC5\xBD", strlen("\xC5\xBD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xBD", "\x5A" "\xCC\x8C", out_buffer, "610 NFD \"(Ž; Ž; Z◌̌; Ž; Z◌̌; ) LATIN CAPITAL LETTER Z WITH CARON\"");

    err = nfkd_normaliser("\xC5\xBD", strlen("\xC5\xBD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xBD", "\x5A" "\xCC\x8C", out_buffer, "611 NFKD \"(Ž; Ž; Z◌̌; Ž; Z◌̌; ) LATIN CAPITAL LETTER Z WITH CARON\"");

    err = nfc_normaliser("\xC5\xBD", strlen("\xC5\xBD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xBD", "\xC5\xBD", out_buffer, "612 NFC \"(Ž; Ž; Z◌̌; Ž; Z◌̌; ) LATIN CAPITAL LETTER Z WITH CARON\"");

    err = nfd_normaliser("\xC5\xBE", strlen("\xC5\xBE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xBE", "\x7A" "\xCC\x8C", out_buffer, "613 NFD \"(ž; ž; z◌̌; ž; z◌̌; ) LATIN SMALL LETTER Z WITH CARON\"");

    err = nfkd_normaliser("\xC5\xBE", strlen("\xC5\xBE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xBE", "\x7A" "\xCC\x8C", out_buffer, "614 NFKD \"(ž; ž; z◌̌; ž; z◌̌; ) LATIN SMALL LETTER Z WITH CARON\"");

    err = nfc_normaliser("\xC5\xBE", strlen("\xC5\xBE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xBE", "\xC5\xBE", out_buffer, "615 NFC \"(ž; ž; z◌̌; ž; z◌̌; ) LATIN SMALL LETTER Z WITH CARON\"");

    err = nfd_normaliser("\xC5\xBF", strlen("\xC5\xBF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xBF", "\xC5\xBF", out_buffer, "616 NFD \"(ſ; ſ; ſ; s; s; ) LATIN SMALL LETTER LONG S\"");

    err = nfkd_normaliser("\xC5\xBF", strlen("\xC5\xBF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xBF", "\x73", out_buffer, "617 NFKD \"(ſ; ſ; ſ; s; s; ) LATIN SMALL LETTER LONG S\"");

    err = nfc_normaliser("\xC5\xBF", strlen("\xC5\xBF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC5\xBF", "\xC5\xBF", out_buffer, "618 NFC \"(ſ; ſ; ſ; s; s; ) LATIN SMALL LETTER LONG S\"");

    err = nfd_normaliser("\xC6\xA0", strlen("\xC6\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC6\xA0", "\x4F" "\xCC\x9B", out_buffer, "619 NFD \"(Ơ; Ơ; O◌̛; Ơ; O◌̛; ) LATIN CAPITAL LETTER O WITH HORN\"");

    err = nfkd_normaliser("\xC6\xA0", strlen("\xC6\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC6\xA0", "\x4F" "\xCC\x9B", out_buffer, "620 NFKD \"(Ơ; Ơ; O◌̛; Ơ; O◌̛; ) LATIN CAPITAL LETTER O WITH HORN\"");

    err = nfc_normaliser("\xC6\xA0", strlen("\xC6\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC6\xA0", "\xC6\xA0", out_buffer, "621 NFC \"(Ơ; Ơ; O◌̛; Ơ; O◌̛; ) LATIN CAPITAL LETTER O WITH HORN\"");

    err = nfd_normaliser("\xC6\xA1", strlen("\xC6\xA1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC6\xA1", "\x6F" "\xCC\x9B", out_buffer, "622 NFD \"(ơ; ơ; o◌̛; ơ; o◌̛; ) LATIN SMALL LETTER O WITH HORN\"");

    err = nfkd_normaliser("\xC6\xA1", strlen("\xC6\xA1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC6\xA1", "\x6F" "\xCC\x9B", out_buffer, "623 NFKD \"(ơ; ơ; o◌̛; ơ; o◌̛; ) LATIN SMALL LETTER O WITH HORN\"");

    err = nfc_normaliser("\xC6\xA1", strlen("\xC6\xA1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC6\xA1", "\xC6\xA1", out_buffer, "624 NFC \"(ơ; ơ; o◌̛; ơ; o◌̛; ) LATIN SMALL LETTER O WITH HORN\"");

    err = nfd_normaliser("\xC6\xAF", strlen("\xC6\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC6\xAF", "\x55" "\xCC\x9B", out_buffer, "625 NFD \"(Ư; Ư; U◌̛; Ư; U◌̛; ) LATIN CAPITAL LETTER U WITH HORN\"");

    err = nfkd_normaliser("\xC6\xAF", strlen("\xC6\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC6\xAF", "\x55" "\xCC\x9B", out_buffer, "626 NFKD \"(Ư; Ư; U◌̛; Ư; U◌̛; ) LATIN CAPITAL LETTER U WITH HORN\"");

    err = nfc_normaliser("\xC6\xAF", strlen("\xC6\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC6\xAF", "\xC6\xAF", out_buffer, "627 NFC \"(Ư; Ư; U◌̛; Ư; U◌̛; ) LATIN CAPITAL LETTER U WITH HORN\"");

    err = nfd_normaliser("\xC6\xB0", strlen("\xC6\xB0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC6\xB0", "\x75" "\xCC\x9B", out_buffer, "628 NFD \"(ư; ư; u◌̛; ư; u◌̛; ) LATIN SMALL LETTER U WITH HORN\"");

    err = nfkd_normaliser("\xC6\xB0", strlen("\xC6\xB0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC6\xB0", "\x75" "\xCC\x9B", out_buffer, "629 NFKD \"(ư; ư; u◌̛; ư; u◌̛; ) LATIN SMALL LETTER U WITH HORN\"");

    err = nfc_normaliser("\xC6\xB0", strlen("\xC6\xB0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC6\xB0", "\xC6\xB0", out_buffer, "630 NFC \"(ư; ư; u◌̛; ư; u◌̛; ) LATIN SMALL LETTER U WITH HORN\"");

    err = nfd_normaliser("\xC7\x84", strlen("\xC7\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x84", "\xC7\x84", out_buffer, "631 NFD \"(Ǆ; Ǆ; Ǆ; DŽ; DZ◌̌; ) LATIN CAPITAL LETTER DZ WITH CARON\"");

    err = nfkd_normaliser("\xC7\x84", strlen("\xC7\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x84", "\x44" "\x5A" "\xCC\x8C", out_buffer, "632 NFKD \"(Ǆ; Ǆ; Ǆ; DŽ; DZ◌̌; ) LATIN CAPITAL LETTER DZ WITH CARON\"");

    err = nfc_normaliser("\xC7\x84", strlen("\xC7\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x84", "\xC7\x84", out_buffer, "633 NFC \"(Ǆ; Ǆ; Ǆ; DŽ; DZ◌̌; ) LATIN CAPITAL LETTER DZ WITH CARON\"");

    err = nfd_normaliser("\xC7\x85", strlen("\xC7\x85"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x85", "\xC7\x85", out_buffer, "634 NFD \"(ǅ; ǅ; ǅ; Dž; Dz◌̌; ) LATIN CAPITAL LETTER D WITH SMALL LETTER Z WITH CARON\"");

    err = nfkd_normaliser("\xC7\x85", strlen("\xC7\x85"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x85", "\x44" "\x7A" "\xCC\x8C", out_buffer, "635 NFKD \"(ǅ; ǅ; ǅ; Dž; Dz◌̌; ) LATIN CAPITAL LETTER D WITH SMALL LETTER Z WITH CARON\"");

    err = nfc_normaliser("\xC7\x85", strlen("\xC7\x85"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x85", "\xC7\x85", out_buffer, "636 NFC \"(ǅ; ǅ; ǅ; Dž; Dz◌̌; ) LATIN CAPITAL LETTER D WITH SMALL LETTER Z WITH CARON\"");

    err = nfd_normaliser("\xC7\x86", strlen("\xC7\x86"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x86", "\xC7\x86", out_buffer, "637 NFD \"(ǆ; ǆ; ǆ; dž; dz◌̌; ) LATIN SMALL LETTER DZ WITH CARON\"");

    err = nfkd_normaliser("\xC7\x86", strlen("\xC7\x86"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x86", "\x64" "\x7A" "\xCC\x8C", out_buffer, "638 NFKD \"(ǆ; ǆ; ǆ; dž; dz◌̌; ) LATIN SMALL LETTER DZ WITH CARON\"");

    err = nfc_normaliser("\xC7\x86", strlen("\xC7\x86"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x86", "\xC7\x86", out_buffer, "639 NFC \"(ǆ; ǆ; ǆ; dž; dz◌̌; ) LATIN SMALL LETTER DZ WITH CARON\"");

    err = nfd_normaliser("\xC7\x87", strlen("\xC7\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x87", "\xC7\x87", out_buffer, "640 NFD \"(Ǉ; Ǉ; Ǉ; LJ; LJ; ) LATIN CAPITAL LETTER LJ\"");

    err = nfkd_normaliser("\xC7\x87", strlen("\xC7\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x87", "\x4C" "\x4A", out_buffer, "641 NFKD \"(Ǉ; Ǉ; Ǉ; LJ; LJ; ) LATIN CAPITAL LETTER LJ\"");

    err = nfc_normaliser("\xC7\x87", strlen("\xC7\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x87", "\xC7\x87", out_buffer, "642 NFC \"(Ǉ; Ǉ; Ǉ; LJ; LJ; ) LATIN CAPITAL LETTER LJ\"");

    err = nfd_normaliser("\xC7\x88", strlen("\xC7\x88"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x88", "\xC7\x88", out_buffer, "643 NFD \"(ǈ; ǈ; ǈ; Lj; Lj; ) LATIN CAPITAL LETTER L WITH SMALL LETTER J\"");

    err = nfkd_normaliser("\xC7\x88", strlen("\xC7\x88"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x88", "\x4C" "\x6A", out_buffer, "644 NFKD \"(ǈ; ǈ; ǈ; Lj; Lj; ) LATIN CAPITAL LETTER L WITH SMALL LETTER J\"");

    err = nfc_normaliser("\xC7\x88", strlen("\xC7\x88"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x88", "\xC7\x88", out_buffer, "645 NFC \"(ǈ; ǈ; ǈ; Lj; Lj; ) LATIN CAPITAL LETTER L WITH SMALL LETTER J\"");

    err = nfd_normaliser("\xC7\x89", strlen("\xC7\x89"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x89", "\xC7\x89", out_buffer, "646 NFD \"(ǉ; ǉ; ǉ; lj; lj; ) LATIN SMALL LETTER LJ\"");

    err = nfkd_normaliser("\xC7\x89", strlen("\xC7\x89"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x89", "\x6C" "\x6A", out_buffer, "647 NFKD \"(ǉ; ǉ; ǉ; lj; lj; ) LATIN SMALL LETTER LJ\"");

    err = nfc_normaliser("\xC7\x89", strlen("\xC7\x89"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x89", "\xC7\x89", out_buffer, "648 NFC \"(ǉ; ǉ; ǉ; lj; lj; ) LATIN SMALL LETTER LJ\"");

    err = nfd_normaliser("\xC7\x8A", strlen("\xC7\x8A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x8A", "\xC7\x8A", out_buffer, "649 NFD \"(Ǌ; Ǌ; Ǌ; NJ; NJ; ) LATIN CAPITAL LETTER NJ\"");

    err = nfkd_normaliser("\xC7\x8A", strlen("\xC7\x8A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x8A", "\x4E" "\x4A", out_buffer, "650 NFKD \"(Ǌ; Ǌ; Ǌ; NJ; NJ; ) LATIN CAPITAL LETTER NJ\"");

    err = nfc_normaliser("\xC7\x8A", strlen("\xC7\x8A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x8A", "\xC7\x8A", out_buffer, "651 NFC \"(Ǌ; Ǌ; Ǌ; NJ; NJ; ) LATIN CAPITAL LETTER NJ\"");

    err = nfd_normaliser("\xC7\x8B", strlen("\xC7\x8B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x8B", "\xC7\x8B", out_buffer, "652 NFD \"(ǋ; ǋ; ǋ; Nj; Nj; ) LATIN CAPITAL LETTER N WITH SMALL LETTER J\"");

    err = nfkd_normaliser("\xC7\x8B", strlen("\xC7\x8B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x8B", "\x4E" "\x6A", out_buffer, "653 NFKD \"(ǋ; ǋ; ǋ; Nj; Nj; ) LATIN CAPITAL LETTER N WITH SMALL LETTER J\"");

    err = nfc_normaliser("\xC7\x8B", strlen("\xC7\x8B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x8B", "\xC7\x8B", out_buffer, "654 NFC \"(ǋ; ǋ; ǋ; Nj; Nj; ) LATIN CAPITAL LETTER N WITH SMALL LETTER J\"");

    err = nfd_normaliser("\xC7\x8C", strlen("\xC7\x8C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x8C", "\xC7\x8C", out_buffer, "655 NFD \"(ǌ; ǌ; ǌ; nj; nj; ) LATIN SMALL LETTER NJ\"");

    err = nfkd_normaliser("\xC7\x8C", strlen("\xC7\x8C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x8C", "\x6E" "\x6A", out_buffer, "656 NFKD \"(ǌ; ǌ; ǌ; nj; nj; ) LATIN SMALL LETTER NJ\"");

    err = nfc_normaliser("\xC7\x8C", strlen("\xC7\x8C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x8C", "\xC7\x8C", out_buffer, "657 NFC \"(ǌ; ǌ; ǌ; nj; nj; ) LATIN SMALL LETTER NJ\"");

    err = nfd_normaliser("\xC7\x8D", strlen("\xC7\x8D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x8D", "\x41" "\xCC\x8C", out_buffer, "658 NFD \"(Ǎ; Ǎ; A◌̌; Ǎ; A◌̌; ) LATIN CAPITAL LETTER A WITH CARON\"");

    err = nfkd_normaliser("\xC7\x8D", strlen("\xC7\x8D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x8D", "\x41" "\xCC\x8C", out_buffer, "659 NFKD \"(Ǎ; Ǎ; A◌̌; Ǎ; A◌̌; ) LATIN CAPITAL LETTER A WITH CARON\"");

    err = nfc_normaliser("\xC7\x8D", strlen("\xC7\x8D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x8D", "\xC7\x8D", out_buffer, "660 NFC \"(Ǎ; Ǎ; A◌̌; Ǎ; A◌̌; ) LATIN CAPITAL LETTER A WITH CARON\"");

    err = nfd_normaliser("\xC7\x8E", strlen("\xC7\x8E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x8E", "\x61" "\xCC\x8C", out_buffer, "661 NFD \"(ǎ; ǎ; a◌̌; ǎ; a◌̌; ) LATIN SMALL LETTER A WITH CARON\"");

    err = nfkd_normaliser("\xC7\x8E", strlen("\xC7\x8E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x8E", "\x61" "\xCC\x8C", out_buffer, "662 NFKD \"(ǎ; ǎ; a◌̌; ǎ; a◌̌; ) LATIN SMALL LETTER A WITH CARON\"");

    err = nfc_normaliser("\xC7\x8E", strlen("\xC7\x8E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x8E", "\xC7\x8E", out_buffer, "663 NFC \"(ǎ; ǎ; a◌̌; ǎ; a◌̌; ) LATIN SMALL LETTER A WITH CARON\"");

    err = nfd_normaliser("\xC7\x8F", strlen("\xC7\x8F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x8F", "\x49" "\xCC\x8C", out_buffer, "664 NFD \"(Ǐ; Ǐ; I◌̌; Ǐ; I◌̌; ) LATIN CAPITAL LETTER I WITH CARON\"");

    err = nfkd_normaliser("\xC7\x8F", strlen("\xC7\x8F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x8F", "\x49" "\xCC\x8C", out_buffer, "665 NFKD \"(Ǐ; Ǐ; I◌̌; Ǐ; I◌̌; ) LATIN CAPITAL LETTER I WITH CARON\"");

    err = nfc_normaliser("\xC7\x8F", strlen("\xC7\x8F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x8F", "\xC7\x8F", out_buffer, "666 NFC \"(Ǐ; Ǐ; I◌̌; Ǐ; I◌̌; ) LATIN CAPITAL LETTER I WITH CARON\"");

    err = nfd_normaliser("\xC7\x90", strlen("\xC7\x90"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x90", "\x69" "\xCC\x8C", out_buffer, "667 NFD \"(ǐ; ǐ; i◌̌; ǐ; i◌̌; ) LATIN SMALL LETTER I WITH CARON\"");

    err = nfkd_normaliser("\xC7\x90", strlen("\xC7\x90"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x90", "\x69" "\xCC\x8C", out_buffer, "668 NFKD \"(ǐ; ǐ; i◌̌; ǐ; i◌̌; ) LATIN SMALL LETTER I WITH CARON\"");

    err = nfc_normaliser("\xC7\x90", strlen("\xC7\x90"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x90", "\xC7\x90", out_buffer, "669 NFC \"(ǐ; ǐ; i◌̌; ǐ; i◌̌; ) LATIN SMALL LETTER I WITH CARON\"");

    err = nfd_normaliser("\xC7\x91", strlen("\xC7\x91"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x91", "\x4F" "\xCC\x8C", out_buffer, "670 NFD \"(Ǒ; Ǒ; O◌̌; Ǒ; O◌̌; ) LATIN CAPITAL LETTER O WITH CARON\"");

    err = nfkd_normaliser("\xC7\x91", strlen("\xC7\x91"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x91", "\x4F" "\xCC\x8C", out_buffer, "671 NFKD \"(Ǒ; Ǒ; O◌̌; Ǒ; O◌̌; ) LATIN CAPITAL LETTER O WITH CARON\"");

    err = nfc_normaliser("\xC7\x91", strlen("\xC7\x91"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x91", "\xC7\x91", out_buffer, "672 NFC \"(Ǒ; Ǒ; O◌̌; Ǒ; O◌̌; ) LATIN CAPITAL LETTER O WITH CARON\"");

    err = nfd_normaliser("\xC7\x92", strlen("\xC7\x92"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x92", "\x6F" "\xCC\x8C", out_buffer, "673 NFD \"(ǒ; ǒ; o◌̌; ǒ; o◌̌; ) LATIN SMALL LETTER O WITH CARON\"");

    err = nfkd_normaliser("\xC7\x92", strlen("\xC7\x92"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x92", "\x6F" "\xCC\x8C", out_buffer, "674 NFKD \"(ǒ; ǒ; o◌̌; ǒ; o◌̌; ) LATIN SMALL LETTER O WITH CARON\"");

    err = nfc_normaliser("\xC7\x92", strlen("\xC7\x92"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x92", "\xC7\x92", out_buffer, "675 NFC \"(ǒ; ǒ; o◌̌; ǒ; o◌̌; ) LATIN SMALL LETTER O WITH CARON\"");

    err = nfd_normaliser("\xC7\x93", strlen("\xC7\x93"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x93", "\x55" "\xCC\x8C", out_buffer, "676 NFD \"(Ǔ; Ǔ; U◌̌; Ǔ; U◌̌; ) LATIN CAPITAL LETTER U WITH CARON\"");

    err = nfkd_normaliser("\xC7\x93", strlen("\xC7\x93"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x93", "\x55" "\xCC\x8C", out_buffer, "677 NFKD \"(Ǔ; Ǔ; U◌̌; Ǔ; U◌̌; ) LATIN CAPITAL LETTER U WITH CARON\"");

    err = nfc_normaliser("\xC7\x93", strlen("\xC7\x93"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x93", "\xC7\x93", out_buffer, "678 NFC \"(Ǔ; Ǔ; U◌̌; Ǔ; U◌̌; ) LATIN CAPITAL LETTER U WITH CARON\"");

    err = nfd_normaliser("\xC7\x94", strlen("\xC7\x94"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x94", "\x75" "\xCC\x8C", out_buffer, "679 NFD \"(ǔ; ǔ; u◌̌; ǔ; u◌̌; ) LATIN SMALL LETTER U WITH CARON\"");

    err = nfkd_normaliser("\xC7\x94", strlen("\xC7\x94"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x94", "\x75" "\xCC\x8C", out_buffer, "680 NFKD \"(ǔ; ǔ; u◌̌; ǔ; u◌̌; ) LATIN SMALL LETTER U WITH CARON\"");

    err = nfc_normaliser("\xC7\x94", strlen("\xC7\x94"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x94", "\xC7\x94", out_buffer, "681 NFC \"(ǔ; ǔ; u◌̌; ǔ; u◌̌; ) LATIN SMALL LETTER U WITH CARON\"");

    err = nfd_normaliser("\xC7\x95", strlen("\xC7\x95"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x95", "\x55" "\xCC\x88" "\xCC\x84", out_buffer, "682 NFD \"(Ǖ; Ǖ; U◌̈◌̄; Ǖ; U◌̈◌̄; ) LATIN CAPITAL LETTER U WITH DIAERESIS AND MACRON\"");

    err = nfkd_normaliser("\xC7\x95", strlen("\xC7\x95"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x95", "\x55" "\xCC\x88" "\xCC\x84", out_buffer, "683 NFKD \"(Ǖ; Ǖ; U◌̈◌̄; Ǖ; U◌̈◌̄; ) LATIN CAPITAL LETTER U WITH DIAERESIS AND MACRON\"");

    err = nfc_normaliser("\xC7\x95", strlen("\xC7\x95"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x95", "\xC7\x95", out_buffer, "684 NFC \"(Ǖ; Ǖ; U◌̈◌̄; Ǖ; U◌̈◌̄; ) LATIN CAPITAL LETTER U WITH DIAERESIS AND MACRON\"");

    err = nfd_normaliser("\xC7\x96", strlen("\xC7\x96"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x96", "\x75" "\xCC\x88" "\xCC\x84", out_buffer, "685 NFD \"(ǖ; ǖ; u◌̈◌̄; ǖ; u◌̈◌̄; ) LATIN SMALL LETTER U WITH DIAERESIS AND MACRON\"");

    err = nfkd_normaliser("\xC7\x96", strlen("\xC7\x96"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x96", "\x75" "\xCC\x88" "\xCC\x84", out_buffer, "686 NFKD \"(ǖ; ǖ; u◌̈◌̄; ǖ; u◌̈◌̄; ) LATIN SMALL LETTER U WITH DIAERESIS AND MACRON\"");

    err = nfc_normaliser("\xC7\x96", strlen("\xC7\x96"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x96", "\xC7\x96", out_buffer, "687 NFC \"(ǖ; ǖ; u◌̈◌̄; ǖ; u◌̈◌̄; ) LATIN SMALL LETTER U WITH DIAERESIS AND MACRON\"");

    err = nfd_normaliser("\xC7\x97", strlen("\xC7\x97"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x97", "\x55" "\xCC\x88" "\xCC\x81", out_buffer, "688 NFD \"(Ǘ; Ǘ; U◌̈◌́; Ǘ; U◌̈◌́; ) LATIN CAPITAL LETTER U WITH DIAERESIS AND ACUTE\"");

    err = nfkd_normaliser("\xC7\x97", strlen("\xC7\x97"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x97", "\x55" "\xCC\x88" "\xCC\x81", out_buffer, "689 NFKD \"(Ǘ; Ǘ; U◌̈◌́; Ǘ; U◌̈◌́; ) LATIN CAPITAL LETTER U WITH DIAERESIS AND ACUTE\"");

    err = nfc_normaliser("\xC7\x97", strlen("\xC7\x97"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x97", "\xC7\x97", out_buffer, "690 NFC \"(Ǘ; Ǘ; U◌̈◌́; Ǘ; U◌̈◌́; ) LATIN CAPITAL LETTER U WITH DIAERESIS AND ACUTE\"");

    err = nfd_normaliser("\xC7\x98", strlen("\xC7\x98"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x98", "\x75" "\xCC\x88" "\xCC\x81", out_buffer, "691 NFD \"(ǘ; ǘ; u◌̈◌́; ǘ; u◌̈◌́; ) LATIN SMALL LETTER U WITH DIAERESIS AND ACUTE\"");

    err = nfkd_normaliser("\xC7\x98", strlen("\xC7\x98"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x98", "\x75" "\xCC\x88" "\xCC\x81", out_buffer, "692 NFKD \"(ǘ; ǘ; u◌̈◌́; ǘ; u◌̈◌́; ) LATIN SMALL LETTER U WITH DIAERESIS AND ACUTE\"");

    err = nfc_normaliser("\xC7\x98", strlen("\xC7\x98"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x98", "\xC7\x98", out_buffer, "693 NFC \"(ǘ; ǘ; u◌̈◌́; ǘ; u◌̈◌́; ) LATIN SMALL LETTER U WITH DIAERESIS AND ACUTE\"");

    err = nfd_normaliser("\xC7\x99", strlen("\xC7\x99"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x99", "\x55" "\xCC\x88" "\xCC\x8C", out_buffer, "694 NFD \"(Ǚ; Ǚ; U◌̈◌̌; Ǚ; U◌̈◌̌; ) LATIN CAPITAL LETTER U WITH DIAERESIS AND CARON\"");

    err = nfkd_normaliser("\xC7\x99", strlen("\xC7\x99"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x99", "\x55" "\xCC\x88" "\xCC\x8C", out_buffer, "695 NFKD \"(Ǚ; Ǚ; U◌̈◌̌; Ǚ; U◌̈◌̌; ) LATIN CAPITAL LETTER U WITH DIAERESIS AND CARON\"");

    err = nfc_normaliser("\xC7\x99", strlen("\xC7\x99"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x99", "\xC7\x99", out_buffer, "696 NFC \"(Ǚ; Ǚ; U◌̈◌̌; Ǚ; U◌̈◌̌; ) LATIN CAPITAL LETTER U WITH DIAERESIS AND CARON\"");

    err = nfd_normaliser("\xC7\x9A", strlen("\xC7\x9A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x9A", "\x75" "\xCC\x88" "\xCC\x8C", out_buffer, "697 NFD \"(ǚ; ǚ; u◌̈◌̌; ǚ; u◌̈◌̌; ) LATIN SMALL LETTER U WITH DIAERESIS AND CARON\"");

    err = nfkd_normaliser("\xC7\x9A", strlen("\xC7\x9A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x9A", "\x75" "\xCC\x88" "\xCC\x8C", out_buffer, "698 NFKD \"(ǚ; ǚ; u◌̈◌̌; ǚ; u◌̈◌̌; ) LATIN SMALL LETTER U WITH DIAERESIS AND CARON\"");

    err = nfc_normaliser("\xC7\x9A", strlen("\xC7\x9A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x9A", "\xC7\x9A", out_buffer, "699 NFC \"(ǚ; ǚ; u◌̈◌̌; ǚ; u◌̈◌̌; ) LATIN SMALL LETTER U WITH DIAERESIS AND CARON\"");

    err = nfd_normaliser("\xC7\x9B", strlen("\xC7\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x9B", "\x55" "\xCC\x88" "\xCC\x80", out_buffer, "700 NFD \"(Ǜ; Ǜ; U◌̈◌̀; Ǜ; U◌̈◌̀; ) LATIN CAPITAL LETTER U WITH DIAERESIS AND GRAVE\"");

    err = nfkd_normaliser("\xC7\x9B", strlen("\xC7\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x9B", "\x55" "\xCC\x88" "\xCC\x80", out_buffer, "701 NFKD \"(Ǜ; Ǜ; U◌̈◌̀; Ǜ; U◌̈◌̀; ) LATIN CAPITAL LETTER U WITH DIAERESIS AND GRAVE\"");

    err = nfc_normaliser("\xC7\x9B", strlen("\xC7\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x9B", "\xC7\x9B", out_buffer, "702 NFC \"(Ǜ; Ǜ; U◌̈◌̀; Ǜ; U◌̈◌̀; ) LATIN CAPITAL LETTER U WITH DIAERESIS AND GRAVE\"");

    err = nfd_normaliser("\xC7\x9C", strlen("\xC7\x9C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x9C", "\x75" "\xCC\x88" "\xCC\x80", out_buffer, "703 NFD \"(ǜ; ǜ; u◌̈◌̀; ǜ; u◌̈◌̀; ) LATIN SMALL LETTER U WITH DIAERESIS AND GRAVE\"");

    err = nfkd_normaliser("\xC7\x9C", strlen("\xC7\x9C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x9C", "\x75" "\xCC\x88" "\xCC\x80", out_buffer, "704 NFKD \"(ǜ; ǜ; u◌̈◌̀; ǜ; u◌̈◌̀; ) LATIN SMALL LETTER U WITH DIAERESIS AND GRAVE\"");

    err = nfc_normaliser("\xC7\x9C", strlen("\xC7\x9C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x9C", "\xC7\x9C", out_buffer, "705 NFC \"(ǜ; ǜ; u◌̈◌̀; ǜ; u◌̈◌̀; ) LATIN SMALL LETTER U WITH DIAERESIS AND GRAVE\"");

    err = nfd_normaliser("\xC7\x9E", strlen("\xC7\x9E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x9E", "\x41" "\xCC\x88" "\xCC\x84", out_buffer, "706 NFD \"(Ǟ; Ǟ; A◌̈◌̄; Ǟ; A◌̈◌̄; ) LATIN CAPITAL LETTER A WITH DIAERESIS AND MACRON\"");

    err = nfkd_normaliser("\xC7\x9E", strlen("\xC7\x9E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x9E", "\x41" "\xCC\x88" "\xCC\x84", out_buffer, "707 NFKD \"(Ǟ; Ǟ; A◌̈◌̄; Ǟ; A◌̈◌̄; ) LATIN CAPITAL LETTER A WITH DIAERESIS AND MACRON\"");

    err = nfc_normaliser("\xC7\x9E", strlen("\xC7\x9E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x9E", "\xC7\x9E", out_buffer, "708 NFC \"(Ǟ; Ǟ; A◌̈◌̄; Ǟ; A◌̈◌̄; ) LATIN CAPITAL LETTER A WITH DIAERESIS AND MACRON\"");

    err = nfd_normaliser("\xC7\x9F", strlen("\xC7\x9F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x9F", "\x61" "\xCC\x88" "\xCC\x84", out_buffer, "709 NFD \"(ǟ; ǟ; a◌̈◌̄; ǟ; a◌̈◌̄; ) LATIN SMALL LETTER A WITH DIAERESIS AND MACRON\"");

    err = nfkd_normaliser("\xC7\x9F", strlen("\xC7\x9F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x9F", "\x61" "\xCC\x88" "\xCC\x84", out_buffer, "710 NFKD \"(ǟ; ǟ; a◌̈◌̄; ǟ; a◌̈◌̄; ) LATIN SMALL LETTER A WITH DIAERESIS AND MACRON\"");

    err = nfc_normaliser("\xC7\x9F", strlen("\xC7\x9F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\x9F", "\xC7\x9F", out_buffer, "711 NFC \"(ǟ; ǟ; a◌̈◌̄; ǟ; a◌̈◌̄; ) LATIN SMALL LETTER A WITH DIAERESIS AND MACRON\"");

    err = nfd_normaliser("\xC7\xA0", strlen("\xC7\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA0", "\x41" "\xCC\x87" "\xCC\x84", out_buffer, "712 NFD \"(Ǡ; Ǡ; A◌̇◌̄; Ǡ; A◌̇◌̄; ) LATIN CAPITAL LETTER A WITH DOT ABOVE AND MACRON\"");

    err = nfkd_normaliser("\xC7\xA0", strlen("\xC7\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA0", "\x41" "\xCC\x87" "\xCC\x84", out_buffer, "713 NFKD \"(Ǡ; Ǡ; A◌̇◌̄; Ǡ; A◌̇◌̄; ) LATIN CAPITAL LETTER A WITH DOT ABOVE AND MACRON\"");

    err = nfc_normaliser("\xC7\xA0", strlen("\xC7\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA0", "\xC7\xA0", out_buffer, "714 NFC \"(Ǡ; Ǡ; A◌̇◌̄; Ǡ; A◌̇◌̄; ) LATIN CAPITAL LETTER A WITH DOT ABOVE AND MACRON\"");

    err = nfd_normaliser("\xC7\xA1", strlen("\xC7\xA1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA1", "\x61" "\xCC\x87" "\xCC\x84", out_buffer, "715 NFD \"(ǡ; ǡ; a◌̇◌̄; ǡ; a◌̇◌̄; ) LATIN SMALL LETTER A WITH DOT ABOVE AND MACRON\"");

    err = nfkd_normaliser("\xC7\xA1", strlen("\xC7\xA1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA1", "\x61" "\xCC\x87" "\xCC\x84", out_buffer, "716 NFKD \"(ǡ; ǡ; a◌̇◌̄; ǡ; a◌̇◌̄; ) LATIN SMALL LETTER A WITH DOT ABOVE AND MACRON\"");

    err = nfc_normaliser("\xC7\xA1", strlen("\xC7\xA1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA1", "\xC7\xA1", out_buffer, "717 NFC \"(ǡ; ǡ; a◌̇◌̄; ǡ; a◌̇◌̄; ) LATIN SMALL LETTER A WITH DOT ABOVE AND MACRON\"");

    err = nfd_normaliser("\xC7\xA2", strlen("\xC7\xA2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA2", "\xC3\x86" "\xCC\x84", out_buffer, "718 NFD \"(Ǣ; Ǣ; Æ◌̄; Ǣ; Æ◌̄; ) LATIN CAPITAL LETTER AE WITH MACRON\"");

    err = nfkd_normaliser("\xC7\xA2", strlen("\xC7\xA2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA2", "\xC3\x86" "\xCC\x84", out_buffer, "719 NFKD \"(Ǣ; Ǣ; Æ◌̄; Ǣ; Æ◌̄; ) LATIN CAPITAL LETTER AE WITH MACRON\"");

    err = nfc_normaliser("\xC7\xA2", strlen("\xC7\xA2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA2", "\xC7\xA2", out_buffer, "720 NFC \"(Ǣ; Ǣ; Æ◌̄; Ǣ; Æ◌̄; ) LATIN CAPITAL LETTER AE WITH MACRON\"");

    err = nfd_normaliser("\xC7\xA3", strlen("\xC7\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA3", "\xC3\xA6" "\xCC\x84", out_buffer, "721 NFD \"(ǣ; ǣ; æ◌̄; ǣ; æ◌̄; ) LATIN SMALL LETTER AE WITH MACRON\"");

    err = nfkd_normaliser("\xC7\xA3", strlen("\xC7\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA3", "\xC3\xA6" "\xCC\x84", out_buffer, "722 NFKD \"(ǣ; ǣ; æ◌̄; ǣ; æ◌̄; ) LATIN SMALL LETTER AE WITH MACRON\"");

    err = nfc_normaliser("\xC7\xA3", strlen("\xC7\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA3", "\xC7\xA3", out_buffer, "723 NFC \"(ǣ; ǣ; æ◌̄; ǣ; æ◌̄; ) LATIN SMALL LETTER AE WITH MACRON\"");

    err = nfd_normaliser("\xC7\xA6", strlen("\xC7\xA6"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA6", "\x47" "\xCC\x8C", out_buffer, "724 NFD \"(Ǧ; Ǧ; G◌̌; Ǧ; G◌̌; ) LATIN CAPITAL LETTER G WITH CARON\"");

    err = nfkd_normaliser("\xC7\xA6", strlen("\xC7\xA6"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA6", "\x47" "\xCC\x8C", out_buffer, "725 NFKD \"(Ǧ; Ǧ; G◌̌; Ǧ; G◌̌; ) LATIN CAPITAL LETTER G WITH CARON\"");

    err = nfc_normaliser("\xC7\xA6", strlen("\xC7\xA6"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA6", "\xC7\xA6", out_buffer, "726 NFC \"(Ǧ; Ǧ; G◌̌; Ǧ; G◌̌; ) LATIN CAPITAL LETTER G WITH CARON\"");

    err = nfd_normaliser("\xC7\xA7", strlen("\xC7\xA7"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA7", "\x67" "\xCC\x8C", out_buffer, "727 NFD \"(ǧ; ǧ; g◌̌; ǧ; g◌̌; ) LATIN SMALL LETTER G WITH CARON\"");

    err = nfkd_normaliser("\xC7\xA7", strlen("\xC7\xA7"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA7", "\x67" "\xCC\x8C", out_buffer, "728 NFKD \"(ǧ; ǧ; g◌̌; ǧ; g◌̌; ) LATIN SMALL LETTER G WITH CARON\"");

    err = nfc_normaliser("\xC7\xA7", strlen("\xC7\xA7"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA7", "\xC7\xA7", out_buffer, "729 NFC \"(ǧ; ǧ; g◌̌; ǧ; g◌̌; ) LATIN SMALL LETTER G WITH CARON\"");

    err = nfd_normaliser("\xC7\xA8", strlen("\xC7\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA8", "\x4B" "\xCC\x8C", out_buffer, "730 NFD \"(Ǩ; Ǩ; K◌̌; Ǩ; K◌̌; ) LATIN CAPITAL LETTER K WITH CARON\"");

    err = nfkd_normaliser("\xC7\xA8", strlen("\xC7\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA8", "\x4B" "\xCC\x8C", out_buffer, "731 NFKD \"(Ǩ; Ǩ; K◌̌; Ǩ; K◌̌; ) LATIN CAPITAL LETTER K WITH CARON\"");

    err = nfc_normaliser("\xC7\xA8", strlen("\xC7\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA8", "\xC7\xA8", out_buffer, "732 NFC \"(Ǩ; Ǩ; K◌̌; Ǩ; K◌̌; ) LATIN CAPITAL LETTER K WITH CARON\"");

    err = nfd_normaliser("\xC7\xA9", strlen("\xC7\xA9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA9", "\x6B" "\xCC\x8C", out_buffer, "733 NFD \"(ǩ; ǩ; k◌̌; ǩ; k◌̌; ) LATIN SMALL LETTER K WITH CARON\"");

    err = nfkd_normaliser("\xC7\xA9", strlen("\xC7\xA9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA9", "\x6B" "\xCC\x8C", out_buffer, "734 NFKD \"(ǩ; ǩ; k◌̌; ǩ; k◌̌; ) LATIN SMALL LETTER K WITH CARON\"");

    err = nfc_normaliser("\xC7\xA9", strlen("\xC7\xA9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xA9", "\xC7\xA9", out_buffer, "735 NFC \"(ǩ; ǩ; k◌̌; ǩ; k◌̌; ) LATIN SMALL LETTER K WITH CARON\"");

    err = nfd_normaliser("\xC7\xAA", strlen("\xC7\xAA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xAA", "\x4F" "\xCC\xA8", out_buffer, "736 NFD \"(Ǫ; Ǫ; O◌̨; Ǫ; O◌̨; ) LATIN CAPITAL LETTER O WITH OGONEK\"");

    err = nfkd_normaliser("\xC7\xAA", strlen("\xC7\xAA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xAA", "\x4F" "\xCC\xA8", out_buffer, "737 NFKD \"(Ǫ; Ǫ; O◌̨; Ǫ; O◌̨; ) LATIN CAPITAL LETTER O WITH OGONEK\"");

    err = nfc_normaliser("\xC7\xAA", strlen("\xC7\xAA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xAA", "\xC7\xAA", out_buffer, "738 NFC \"(Ǫ; Ǫ; O◌̨; Ǫ; O◌̨; ) LATIN CAPITAL LETTER O WITH OGONEK\"");

    err = nfd_normaliser("\xC7\xAB", strlen("\xC7\xAB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xAB", "\x6F" "\xCC\xA8", out_buffer, "739 NFD \"(ǫ; ǫ; o◌̨; ǫ; o◌̨; ) LATIN SMALL LETTER O WITH OGONEK\"");

    err = nfkd_normaliser("\xC7\xAB", strlen("\xC7\xAB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xAB", "\x6F" "\xCC\xA8", out_buffer, "740 NFKD \"(ǫ; ǫ; o◌̨; ǫ; o◌̨; ) LATIN SMALL LETTER O WITH OGONEK\"");

    err = nfc_normaliser("\xC7\xAB", strlen("\xC7\xAB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xAB", "\xC7\xAB", out_buffer, "741 NFC \"(ǫ; ǫ; o◌̨; ǫ; o◌̨; ) LATIN SMALL LETTER O WITH OGONEK\"");

    err = nfd_normaliser("\xC7\xAC", strlen("\xC7\xAC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xAC", "\x4F" "\xCC\xA8" "\xCC\x84", out_buffer, "742 NFD \"(Ǭ; Ǭ; O◌̨◌̄; Ǭ; O◌̨◌̄; ) LATIN CAPITAL LETTER O WITH OGONEK AND MACRON\"");

    err = nfkd_normaliser("\xC7\xAC", strlen("\xC7\xAC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xAC", "\x4F" "\xCC\xA8" "\xCC\x84", out_buffer, "743 NFKD \"(Ǭ; Ǭ; O◌̨◌̄; Ǭ; O◌̨◌̄; ) LATIN CAPITAL LETTER O WITH OGONEK AND MACRON\"");

    err = nfc_normaliser("\xC7\xAC", strlen("\xC7\xAC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xAC", "\xC7\xAC", out_buffer, "744 NFC \"(Ǭ; Ǭ; O◌̨◌̄; Ǭ; O◌̨◌̄; ) LATIN CAPITAL LETTER O WITH OGONEK AND MACRON\"");

    err = nfd_normaliser("\xC7\xAD", strlen("\xC7\xAD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xAD", "\x6F" "\xCC\xA8" "\xCC\x84", out_buffer, "745 NFD \"(ǭ; ǭ; o◌̨◌̄; ǭ; o◌̨◌̄; ) LATIN SMALL LETTER O WITH OGONEK AND MACRON\"");

    err = nfkd_normaliser("\xC7\xAD", strlen("\xC7\xAD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xAD", "\x6F" "\xCC\xA8" "\xCC\x84", out_buffer, "746 NFKD \"(ǭ; ǭ; o◌̨◌̄; ǭ; o◌̨◌̄; ) LATIN SMALL LETTER O WITH OGONEK AND MACRON\"");

    err = nfc_normaliser("\xC7\xAD", strlen("\xC7\xAD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xAD", "\xC7\xAD", out_buffer, "747 NFC \"(ǭ; ǭ; o◌̨◌̄; ǭ; o◌̨◌̄; ) LATIN SMALL LETTER O WITH OGONEK AND MACRON\"");

    err = nfd_normaliser("\xC7\xAE", strlen("\xC7\xAE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xAE", "\xC6\xB7" "\xCC\x8C", out_buffer, "748 NFD \"(Ǯ; Ǯ; Ʒ◌̌; Ǯ; Ʒ◌̌; ) LATIN CAPITAL LETTER EZH WITH CARON\"");

    err = nfkd_normaliser("\xC7\xAE", strlen("\xC7\xAE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xAE", "\xC6\xB7" "\xCC\x8C", out_buffer, "749 NFKD \"(Ǯ; Ǯ; Ʒ◌̌; Ǯ; Ʒ◌̌; ) LATIN CAPITAL LETTER EZH WITH CARON\"");

    err = nfc_normaliser("\xC7\xAE", strlen("\xC7\xAE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xAE", "\xC7\xAE", out_buffer, "750 NFC \"(Ǯ; Ǯ; Ʒ◌̌; Ǯ; Ʒ◌̌; ) LATIN CAPITAL LETTER EZH WITH CARON\"");

    err = nfd_normaliser("\xC7\xAF", strlen("\xC7\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xAF", "\xCA\x92" "\xCC\x8C", out_buffer, "751 NFD \"(ǯ; ǯ; ʒ◌̌; ǯ; ʒ◌̌; ) LATIN SMALL LETTER EZH WITH CARON\"");

    err = nfkd_normaliser("\xC7\xAF", strlen("\xC7\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xAF", "\xCA\x92" "\xCC\x8C", out_buffer, "752 NFKD \"(ǯ; ǯ; ʒ◌̌; ǯ; ʒ◌̌; ) LATIN SMALL LETTER EZH WITH CARON\"");

    err = nfc_normaliser("\xC7\xAF", strlen("\xC7\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xAF", "\xC7\xAF", out_buffer, "753 NFC \"(ǯ; ǯ; ʒ◌̌; ǯ; ʒ◌̌; ) LATIN SMALL LETTER EZH WITH CARON\"");

    err = nfd_normaliser("\xC7\xB0", strlen("\xC7\xB0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB0", "\x6A" "\xCC\x8C", out_buffer, "754 NFD \"(ǰ; ǰ; j◌̌; ǰ; j◌̌; ) LATIN SMALL LETTER J WITH CARON\"");

    err = nfkd_normaliser("\xC7\xB0", strlen("\xC7\xB0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB0", "\x6A" "\xCC\x8C", out_buffer, "755 NFKD \"(ǰ; ǰ; j◌̌; ǰ; j◌̌; ) LATIN SMALL LETTER J WITH CARON\"");

    err = nfc_normaliser("\xC7\xB0", strlen("\xC7\xB0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB0", "\xC7\xB0", out_buffer, "756 NFC \"(ǰ; ǰ; j◌̌; ǰ; j◌̌; ) LATIN SMALL LETTER J WITH CARON\"");

    err = nfd_normaliser("\xC7\xB1", strlen("\xC7\xB1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB1", "\xC7\xB1", out_buffer, "757 NFD \"(Ǳ; Ǳ; Ǳ; DZ; DZ; ) LATIN CAPITAL LETTER DZ\"");

    err = nfkd_normaliser("\xC7\xB1", strlen("\xC7\xB1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB1", "\x44" "\x5A", out_buffer, "758 NFKD \"(Ǳ; Ǳ; Ǳ; DZ; DZ; ) LATIN CAPITAL LETTER DZ\"");

    err = nfc_normaliser("\xC7\xB1", strlen("\xC7\xB1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB1", "\xC7\xB1", out_buffer, "759 NFC \"(Ǳ; Ǳ; Ǳ; DZ; DZ; ) LATIN CAPITAL LETTER DZ\"");

    err = nfd_normaliser("\xC7\xB2", strlen("\xC7\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB2", "\xC7\xB2", out_buffer, "760 NFD \"(ǲ; ǲ; ǲ; Dz; Dz; ) LATIN CAPITAL LETTER D WITH SMALL LETTER Z\"");

    err = nfkd_normaliser("\xC7\xB2", strlen("\xC7\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB2", "\x44" "\x7A", out_buffer, "761 NFKD \"(ǲ; ǲ; ǲ; Dz; Dz; ) LATIN CAPITAL LETTER D WITH SMALL LETTER Z\"");

    err = nfc_normaliser("\xC7\xB2", strlen("\xC7\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB2", "\xC7\xB2", out_buffer, "762 NFC \"(ǲ; ǲ; ǲ; Dz; Dz; ) LATIN CAPITAL LETTER D WITH SMALL LETTER Z\"");

    err = nfd_normaliser("\xC7\xB3", strlen("\xC7\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB3", "\xC7\xB3", out_buffer, "763 NFD \"(ǳ; ǳ; ǳ; dz; dz; ) LATIN SMALL LETTER DZ\"");

    err = nfkd_normaliser("\xC7\xB3", strlen("\xC7\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB3", "\x64" "\x7A", out_buffer, "764 NFKD \"(ǳ; ǳ; ǳ; dz; dz; ) LATIN SMALL LETTER DZ\"");

    err = nfc_normaliser("\xC7\xB3", strlen("\xC7\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB3", "\xC7\xB3", out_buffer, "765 NFC \"(ǳ; ǳ; ǳ; dz; dz; ) LATIN SMALL LETTER DZ\"");

    err = nfd_normaliser("\xC7\xB4", strlen("\xC7\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB4", "\x47" "\xCC\x81", out_buffer, "766 NFD \"(Ǵ; Ǵ; G◌́; Ǵ; G◌́; ) LATIN CAPITAL LETTER G WITH ACUTE\"");

    err = nfkd_normaliser("\xC7\xB4", strlen("\xC7\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB4", "\x47" "\xCC\x81", out_buffer, "767 NFKD \"(Ǵ; Ǵ; G◌́; Ǵ; G◌́; ) LATIN CAPITAL LETTER G WITH ACUTE\"");

    err = nfc_normaliser("\xC7\xB4", strlen("\xC7\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB4", "\xC7\xB4", out_buffer, "768 NFC \"(Ǵ; Ǵ; G◌́; Ǵ; G◌́; ) LATIN CAPITAL LETTER G WITH ACUTE\"");

    err = nfd_normaliser("\xC7\xB5", strlen("\xC7\xB5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB5", "\x67" "\xCC\x81", out_buffer, "769 NFD \"(ǵ; ǵ; g◌́; ǵ; g◌́; ) LATIN SMALL LETTER G WITH ACUTE\"");

    err = nfkd_normaliser("\xC7\xB5", strlen("\xC7\xB5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB5", "\x67" "\xCC\x81", out_buffer, "770 NFKD \"(ǵ; ǵ; g◌́; ǵ; g◌́; ) LATIN SMALL LETTER G WITH ACUTE\"");

    err = nfc_normaliser("\xC7\xB5", strlen("\xC7\xB5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB5", "\xC7\xB5", out_buffer, "771 NFC \"(ǵ; ǵ; g◌́; ǵ; g◌́; ) LATIN SMALL LETTER G WITH ACUTE\"");

    err = nfd_normaliser("\xC7\xB8", strlen("\xC7\xB8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB8", "\x4E" "\xCC\x80", out_buffer, "772 NFD \"(Ǹ; Ǹ; N◌̀; Ǹ; N◌̀; ) LATIN CAPITAL LETTER N WITH GRAVE\"");

    err = nfkd_normaliser("\xC7\xB8", strlen("\xC7\xB8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB8", "\x4E" "\xCC\x80", out_buffer, "773 NFKD \"(Ǹ; Ǹ; N◌̀; Ǹ; N◌̀; ) LATIN CAPITAL LETTER N WITH GRAVE\"");

    err = nfc_normaliser("\xC7\xB8", strlen("\xC7\xB8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB8", "\xC7\xB8", out_buffer, "774 NFC \"(Ǹ; Ǹ; N◌̀; Ǹ; N◌̀; ) LATIN CAPITAL LETTER N WITH GRAVE\"");

    err = nfd_normaliser("\xC7\xB9", strlen("\xC7\xB9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB9", "\x6E" "\xCC\x80", out_buffer, "775 NFD \"(ǹ; ǹ; n◌̀; ǹ; n◌̀; ) LATIN SMALL LETTER N WITH GRAVE\"");

    err = nfkd_normaliser("\xC7\xB9", strlen("\xC7\xB9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB9", "\x6E" "\xCC\x80", out_buffer, "776 NFKD \"(ǹ; ǹ; n◌̀; ǹ; n◌̀; ) LATIN SMALL LETTER N WITH GRAVE\"");

    err = nfc_normaliser("\xC7\xB9", strlen("\xC7\xB9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xB9", "\xC7\xB9", out_buffer, "777 NFC \"(ǹ; ǹ; n◌̀; ǹ; n◌̀; ) LATIN SMALL LETTER N WITH GRAVE\"");

    err = nfd_normaliser("\xC7\xBA", strlen("\xC7\xBA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xBA", "\x41" "\xCC\x8A" "\xCC\x81", out_buffer, "778 NFD \"(Ǻ; Ǻ; A◌̊◌́; Ǻ; A◌̊◌́; ) LATIN CAPITAL LETTER A WITH RING ABOVE AND ACUTE\"");

    err = nfkd_normaliser("\xC7\xBA", strlen("\xC7\xBA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xBA", "\x41" "\xCC\x8A" "\xCC\x81", out_buffer, "779 NFKD \"(Ǻ; Ǻ; A◌̊◌́; Ǻ; A◌̊◌́; ) LATIN CAPITAL LETTER A WITH RING ABOVE AND ACUTE\"");

    err = nfc_normaliser("\xC7\xBA", strlen("\xC7\xBA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xBA", "\xC7\xBA", out_buffer, "780 NFC \"(Ǻ; Ǻ; A◌̊◌́; Ǻ; A◌̊◌́; ) LATIN CAPITAL LETTER A WITH RING ABOVE AND ACUTE\"");

    err = nfd_normaliser("\xC7\xBB", strlen("\xC7\xBB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xBB", "\x61" "\xCC\x8A" "\xCC\x81", out_buffer, "781 NFD \"(ǻ; ǻ; a◌̊◌́; ǻ; a◌̊◌́; ) LATIN SMALL LETTER A WITH RING ABOVE AND ACUTE\"");

    err = nfkd_normaliser("\xC7\xBB", strlen("\xC7\xBB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xBB", "\x61" "\xCC\x8A" "\xCC\x81", out_buffer, "782 NFKD \"(ǻ; ǻ; a◌̊◌́; ǻ; a◌̊◌́; ) LATIN SMALL LETTER A WITH RING ABOVE AND ACUTE\"");

    err = nfc_normaliser("\xC7\xBB", strlen("\xC7\xBB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xBB", "\xC7\xBB", out_buffer, "783 NFC \"(ǻ; ǻ; a◌̊◌́; ǻ; a◌̊◌́; ) LATIN SMALL LETTER A WITH RING ABOVE AND ACUTE\"");

    err = nfd_normaliser("\xC7\xBC", strlen("\xC7\xBC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xBC", "\xC3\x86" "\xCC\x81", out_buffer, "784 NFD \"(Ǽ; Ǽ; Æ◌́; Ǽ; Æ◌́; ) LATIN CAPITAL LETTER AE WITH ACUTE\"");

    err = nfkd_normaliser("\xC7\xBC", strlen("\xC7\xBC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xBC", "\xC3\x86" "\xCC\x81", out_buffer, "785 NFKD \"(Ǽ; Ǽ; Æ◌́; Ǽ; Æ◌́; ) LATIN CAPITAL LETTER AE WITH ACUTE\"");

    err = nfc_normaliser("\xC7\xBC", strlen("\xC7\xBC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xBC", "\xC7\xBC", out_buffer, "786 NFC \"(Ǽ; Ǽ; Æ◌́; Ǽ; Æ◌́; ) LATIN CAPITAL LETTER AE WITH ACUTE\"");

    err = nfd_normaliser("\xC7\xBD", strlen("\xC7\xBD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xBD", "\xC3\xA6" "\xCC\x81", out_buffer, "787 NFD \"(ǽ; ǽ; æ◌́; ǽ; æ◌́; ) LATIN SMALL LETTER AE WITH ACUTE\"");

    err = nfkd_normaliser("\xC7\xBD", strlen("\xC7\xBD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xBD", "\xC3\xA6" "\xCC\x81", out_buffer, "788 NFKD \"(ǽ; ǽ; æ◌́; ǽ; æ◌́; ) LATIN SMALL LETTER AE WITH ACUTE\"");

    err = nfc_normaliser("\xC7\xBD", strlen("\xC7\xBD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xBD", "\xC7\xBD", out_buffer, "789 NFC \"(ǽ; ǽ; æ◌́; ǽ; æ◌́; ) LATIN SMALL LETTER AE WITH ACUTE\"");

    err = nfd_normaliser("\xC7\xBE", strlen("\xC7\xBE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xBE", "\xC3\x98" "\xCC\x81", out_buffer, "790 NFD \"(Ǿ; Ǿ; Ø◌́; Ǿ; Ø◌́; ) LATIN CAPITAL LETTER O WITH STROKE AND ACUTE\"");

    err = nfkd_normaliser("\xC7\xBE", strlen("\xC7\xBE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xBE", "\xC3\x98" "\xCC\x81", out_buffer, "791 NFKD \"(Ǿ; Ǿ; Ø◌́; Ǿ; Ø◌́; ) LATIN CAPITAL LETTER O WITH STROKE AND ACUTE\"");

    err = nfc_normaliser("\xC7\xBE", strlen("\xC7\xBE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xBE", "\xC7\xBE", out_buffer, "792 NFC \"(Ǿ; Ǿ; Ø◌́; Ǿ; Ø◌́; ) LATIN CAPITAL LETTER O WITH STROKE AND ACUTE\"");

    err = nfd_normaliser("\xC7\xBF", strlen("\xC7\xBF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xBF", "\xC3\xB8" "\xCC\x81", out_buffer, "793 NFD \"(ǿ; ǿ; ø◌́; ǿ; ø◌́; ) LATIN SMALL LETTER O WITH STROKE AND ACUTE\"");

    err = nfkd_normaliser("\xC7\xBF", strlen("\xC7\xBF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xBF", "\xC3\xB8" "\xCC\x81", out_buffer, "794 NFKD \"(ǿ; ǿ; ø◌́; ǿ; ø◌́; ) LATIN SMALL LETTER O WITH STROKE AND ACUTE\"");

    err = nfc_normaliser("\xC7\xBF", strlen("\xC7\xBF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC7\xBF", "\xC7\xBF", out_buffer, "795 NFC \"(ǿ; ǿ; ø◌́; ǿ; ø◌́; ) LATIN SMALL LETTER O WITH STROKE AND ACUTE\"");

    err = nfd_normaliser("\xC8\x80", strlen("\xC8\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x80", "\x41" "\xCC\x8F", out_buffer, "796 NFD \"(Ȁ; Ȁ; A◌̏; Ȁ; A◌̏; ) LATIN CAPITAL LETTER A WITH DOUBLE GRAVE\"");

    err = nfkd_normaliser("\xC8\x80", strlen("\xC8\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x80", "\x41" "\xCC\x8F", out_buffer, "797 NFKD \"(Ȁ; Ȁ; A◌̏; Ȁ; A◌̏; ) LATIN CAPITAL LETTER A WITH DOUBLE GRAVE\"");

    err = nfc_normaliser("\xC8\x80", strlen("\xC8\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x80", "\xC8\x80", out_buffer, "798 NFC \"(Ȁ; Ȁ; A◌̏; Ȁ; A◌̏; ) LATIN CAPITAL LETTER A WITH DOUBLE GRAVE\"");

    err = nfd_normaliser("\xC8\x81", strlen("\xC8\x81"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x81", "\x61" "\xCC\x8F", out_buffer, "799 NFD \"(ȁ; ȁ; a◌̏; ȁ; a◌̏; ) LATIN SMALL LETTER A WITH DOUBLE GRAVE\"");

    err = nfkd_normaliser("\xC8\x81", strlen("\xC8\x81"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x81", "\x61" "\xCC\x8F", out_buffer, "800 NFKD \"(ȁ; ȁ; a◌̏; ȁ; a◌̏; ) LATIN SMALL LETTER A WITH DOUBLE GRAVE\"");

    err = nfc_normaliser("\xC8\x81", strlen("\xC8\x81"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x81", "\xC8\x81", out_buffer, "801 NFC \"(ȁ; ȁ; a◌̏; ȁ; a◌̏; ) LATIN SMALL LETTER A WITH DOUBLE GRAVE\"");

    err = nfd_normaliser("\xC8\x82", strlen("\xC8\x82"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x82", "\x41" "\xCC\x91", out_buffer, "802 NFD \"(Ȃ; Ȃ; A◌̑; Ȃ; A◌̑; ) LATIN CAPITAL LETTER A WITH INVERTED BREVE\"");

    err = nfkd_normaliser("\xC8\x82", strlen("\xC8\x82"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x82", "\x41" "\xCC\x91", out_buffer, "803 NFKD \"(Ȃ; Ȃ; A◌̑; Ȃ; A◌̑; ) LATIN CAPITAL LETTER A WITH INVERTED BREVE\"");

    err = nfc_normaliser("\xC8\x82", strlen("\xC8\x82"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x82", "\xC8\x82", out_buffer, "804 NFC \"(Ȃ; Ȃ; A◌̑; Ȃ; A◌̑; ) LATIN CAPITAL LETTER A WITH INVERTED BREVE\"");

    err = nfd_normaliser("\xC8\x83", strlen("\xC8\x83"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x83", "\x61" "\xCC\x91", out_buffer, "805 NFD \"(ȃ; ȃ; a◌̑; ȃ; a◌̑; ) LATIN SMALL LETTER A WITH INVERTED BREVE\"");

    err = nfkd_normaliser("\xC8\x83", strlen("\xC8\x83"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x83", "\x61" "\xCC\x91", out_buffer, "806 NFKD \"(ȃ; ȃ; a◌̑; ȃ; a◌̑; ) LATIN SMALL LETTER A WITH INVERTED BREVE\"");

    err = nfc_normaliser("\xC8\x83", strlen("\xC8\x83"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x83", "\xC8\x83", out_buffer, "807 NFC \"(ȃ; ȃ; a◌̑; ȃ; a◌̑; ) LATIN SMALL LETTER A WITH INVERTED BREVE\"");

    err = nfd_normaliser("\xC8\x84", strlen("\xC8\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x84", "\x45" "\xCC\x8F", out_buffer, "808 NFD \"(Ȅ; Ȅ; E◌̏; Ȅ; E◌̏; ) LATIN CAPITAL LETTER E WITH DOUBLE GRAVE\"");

    err = nfkd_normaliser("\xC8\x84", strlen("\xC8\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x84", "\x45" "\xCC\x8F", out_buffer, "809 NFKD \"(Ȅ; Ȅ; E◌̏; Ȅ; E◌̏; ) LATIN CAPITAL LETTER E WITH DOUBLE GRAVE\"");

    err = nfc_normaliser("\xC8\x84", strlen("\xC8\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x84", "\xC8\x84", out_buffer, "810 NFC \"(Ȅ; Ȅ; E◌̏; Ȅ; E◌̏; ) LATIN CAPITAL LETTER E WITH DOUBLE GRAVE\"");

    err = nfd_normaliser("\xC8\x85", strlen("\xC8\x85"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x85", "\x65" "\xCC\x8F", out_buffer, "811 NFD \"(ȅ; ȅ; e◌̏; ȅ; e◌̏; ) LATIN SMALL LETTER E WITH DOUBLE GRAVE\"");

    err = nfkd_normaliser("\xC8\x85", strlen("\xC8\x85"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x85", "\x65" "\xCC\x8F", out_buffer, "812 NFKD \"(ȅ; ȅ; e◌̏; ȅ; e◌̏; ) LATIN SMALL LETTER E WITH DOUBLE GRAVE\"");

    err = nfc_normaliser("\xC8\x85", strlen("\xC8\x85"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x85", "\xC8\x85", out_buffer, "813 NFC \"(ȅ; ȅ; e◌̏; ȅ; e◌̏; ) LATIN SMALL LETTER E WITH DOUBLE GRAVE\"");

    err = nfd_normaliser("\xC8\x86", strlen("\xC8\x86"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x86", "\x45" "\xCC\x91", out_buffer, "814 NFD \"(Ȇ; Ȇ; E◌̑; Ȇ; E◌̑; ) LATIN CAPITAL LETTER E WITH INVERTED BREVE\"");

    err = nfkd_normaliser("\xC8\x86", strlen("\xC8\x86"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x86", "\x45" "\xCC\x91", out_buffer, "815 NFKD \"(Ȇ; Ȇ; E◌̑; Ȇ; E◌̑; ) LATIN CAPITAL LETTER E WITH INVERTED BREVE\"");

    err = nfc_normaliser("\xC8\x86", strlen("\xC8\x86"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x86", "\xC8\x86", out_buffer, "816 NFC \"(Ȇ; Ȇ; E◌̑; Ȇ; E◌̑; ) LATIN CAPITAL LETTER E WITH INVERTED BREVE\"");

    err = nfd_normaliser("\xC8\x87", strlen("\xC8\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x87", "\x65" "\xCC\x91", out_buffer, "817 NFD \"(ȇ; ȇ; e◌̑; ȇ; e◌̑; ) LATIN SMALL LETTER E WITH INVERTED BREVE\"");

    err = nfkd_normaliser("\xC8\x87", strlen("\xC8\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x87", "\x65" "\xCC\x91", out_buffer, "818 NFKD \"(ȇ; ȇ; e◌̑; ȇ; e◌̑; ) LATIN SMALL LETTER E WITH INVERTED BREVE\"");

    err = nfc_normaliser("\xC8\x87", strlen("\xC8\x87"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x87", "\xC8\x87", out_buffer, "819 NFC \"(ȇ; ȇ; e◌̑; ȇ; e◌̑; ) LATIN SMALL LETTER E WITH INVERTED BREVE\"");

    err = nfd_normaliser("\xC8\x88", strlen("\xC8\x88"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x88", "\x49" "\xCC\x8F", out_buffer, "820 NFD \"(Ȉ; Ȉ; I◌̏; Ȉ; I◌̏; ) LATIN CAPITAL LETTER I WITH DOUBLE GRAVE\"");

    err = nfkd_normaliser("\xC8\x88", strlen("\xC8\x88"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x88", "\x49" "\xCC\x8F", out_buffer, "821 NFKD \"(Ȉ; Ȉ; I◌̏; Ȉ; I◌̏; ) LATIN CAPITAL LETTER I WITH DOUBLE GRAVE\"");

    err = nfc_normaliser("\xC8\x88", strlen("\xC8\x88"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x88", "\xC8\x88", out_buffer, "822 NFC \"(Ȉ; Ȉ; I◌̏; Ȉ; I◌̏; ) LATIN CAPITAL LETTER I WITH DOUBLE GRAVE\"");

    err = nfd_normaliser("\xC8\x89", strlen("\xC8\x89"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x89", "\x69" "\xCC\x8F", out_buffer, "823 NFD \"(ȉ; ȉ; i◌̏; ȉ; i◌̏; ) LATIN SMALL LETTER I WITH DOUBLE GRAVE\"");

    err = nfkd_normaliser("\xC8\x89", strlen("\xC8\x89"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x89", "\x69" "\xCC\x8F", out_buffer, "824 NFKD \"(ȉ; ȉ; i◌̏; ȉ; i◌̏; ) LATIN SMALL LETTER I WITH DOUBLE GRAVE\"");

    err = nfc_normaliser("\xC8\x89", strlen("\xC8\x89"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x89", "\xC8\x89", out_buffer, "825 NFC \"(ȉ; ȉ; i◌̏; ȉ; i◌̏; ) LATIN SMALL LETTER I WITH DOUBLE GRAVE\"");

    err = nfd_normaliser("\xC8\x8A", strlen("\xC8\x8A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x8A", "\x49" "\xCC\x91", out_buffer, "826 NFD \"(Ȋ; Ȋ; I◌̑; Ȋ; I◌̑; ) LATIN CAPITAL LETTER I WITH INVERTED BREVE\"");

    err = nfkd_normaliser("\xC8\x8A", strlen("\xC8\x8A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x8A", "\x49" "\xCC\x91", out_buffer, "827 NFKD \"(Ȋ; Ȋ; I◌̑; Ȋ; I◌̑; ) LATIN CAPITAL LETTER I WITH INVERTED BREVE\"");

    err = nfc_normaliser("\xC8\x8A", strlen("\xC8\x8A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x8A", "\xC8\x8A", out_buffer, "828 NFC \"(Ȋ; Ȋ; I◌̑; Ȋ; I◌̑; ) LATIN CAPITAL LETTER I WITH INVERTED BREVE\"");

    err = nfd_normaliser("\xC8\x8B", strlen("\xC8\x8B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x8B", "\x69" "\xCC\x91", out_buffer, "829 NFD \"(ȋ; ȋ; i◌̑; ȋ; i◌̑; ) LATIN SMALL LETTER I WITH INVERTED BREVE\"");

    err = nfkd_normaliser("\xC8\x8B", strlen("\xC8\x8B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x8B", "\x69" "\xCC\x91", out_buffer, "830 NFKD \"(ȋ; ȋ; i◌̑; ȋ; i◌̑; ) LATIN SMALL LETTER I WITH INVERTED BREVE\"");

    err = nfc_normaliser("\xC8\x8B", strlen("\xC8\x8B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x8B", "\xC8\x8B", out_buffer, "831 NFC \"(ȋ; ȋ; i◌̑; ȋ; i◌̑; ) LATIN SMALL LETTER I WITH INVERTED BREVE\"");

    err = nfd_normaliser("\xC8\x8C", strlen("\xC8\x8C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x8C", "\x4F" "\xCC\x8F", out_buffer, "832 NFD \"(Ȍ; Ȍ; O◌̏; Ȍ; O◌̏; ) LATIN CAPITAL LETTER O WITH DOUBLE GRAVE\"");

    err = nfkd_normaliser("\xC8\x8C", strlen("\xC8\x8C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x8C", "\x4F" "\xCC\x8F", out_buffer, "833 NFKD \"(Ȍ; Ȍ; O◌̏; Ȍ; O◌̏; ) LATIN CAPITAL LETTER O WITH DOUBLE GRAVE\"");

    err = nfc_normaliser("\xC8\x8C", strlen("\xC8\x8C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x8C", "\xC8\x8C", out_buffer, "834 NFC \"(Ȍ; Ȍ; O◌̏; Ȍ; O◌̏; ) LATIN CAPITAL LETTER O WITH DOUBLE GRAVE\"");

    err = nfd_normaliser("\xC8\x8D", strlen("\xC8\x8D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x8D", "\x6F" "\xCC\x8F", out_buffer, "835 NFD \"(ȍ; ȍ; o◌̏; ȍ; o◌̏; ) LATIN SMALL LETTER O WITH DOUBLE GRAVE\"");

    err = nfkd_normaliser("\xC8\x8D", strlen("\xC8\x8D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x8D", "\x6F" "\xCC\x8F", out_buffer, "836 NFKD \"(ȍ; ȍ; o◌̏; ȍ; o◌̏; ) LATIN SMALL LETTER O WITH DOUBLE GRAVE\"");

    err = nfc_normaliser("\xC8\x8D", strlen("\xC8\x8D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x8D", "\xC8\x8D", out_buffer, "837 NFC \"(ȍ; ȍ; o◌̏; ȍ; o◌̏; ) LATIN SMALL LETTER O WITH DOUBLE GRAVE\"");

    err = nfd_normaliser("\xC8\x8E", strlen("\xC8\x8E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x8E", "\x4F" "\xCC\x91", out_buffer, "838 NFD \"(Ȏ; Ȏ; O◌̑; Ȏ; O◌̑; ) LATIN CAPITAL LETTER O WITH INVERTED BREVE\"");

    err = nfkd_normaliser("\xC8\x8E", strlen("\xC8\x8E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x8E", "\x4F" "\xCC\x91", out_buffer, "839 NFKD \"(Ȏ; Ȏ; O◌̑; Ȏ; O◌̑; ) LATIN CAPITAL LETTER O WITH INVERTED BREVE\"");

    err = nfc_normaliser("\xC8\x8E", strlen("\xC8\x8E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x8E", "\xC8\x8E", out_buffer, "840 NFC \"(Ȏ; Ȏ; O◌̑; Ȏ; O◌̑; ) LATIN CAPITAL LETTER O WITH INVERTED BREVE\"");

    err = nfd_normaliser("\xC8\x8F", strlen("\xC8\x8F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x8F", "\x6F" "\xCC\x91", out_buffer, "841 NFD \"(ȏ; ȏ; o◌̑; ȏ; o◌̑; ) LATIN SMALL LETTER O WITH INVERTED BREVE\"");

    err = nfkd_normaliser("\xC8\x8F", strlen("\xC8\x8F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x8F", "\x6F" "\xCC\x91", out_buffer, "842 NFKD \"(ȏ; ȏ; o◌̑; ȏ; o◌̑; ) LATIN SMALL LETTER O WITH INVERTED BREVE\"");

    err = nfc_normaliser("\xC8\x8F", strlen("\xC8\x8F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x8F", "\xC8\x8F", out_buffer, "843 NFC \"(ȏ; ȏ; o◌̑; ȏ; o◌̑; ) LATIN SMALL LETTER O WITH INVERTED BREVE\"");

    err = nfd_normaliser("\xC8\x90", strlen("\xC8\x90"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x90", "\x52" "\xCC\x8F", out_buffer, "844 NFD \"(Ȑ; Ȑ; R◌̏; Ȑ; R◌̏; ) LATIN CAPITAL LETTER R WITH DOUBLE GRAVE\"");

    err = nfkd_normaliser("\xC8\x90", strlen("\xC8\x90"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x90", "\x52" "\xCC\x8F", out_buffer, "845 NFKD \"(Ȑ; Ȑ; R◌̏; Ȑ; R◌̏; ) LATIN CAPITAL LETTER R WITH DOUBLE GRAVE\"");

    err = nfc_normaliser("\xC8\x90", strlen("\xC8\x90"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x90", "\xC8\x90", out_buffer, "846 NFC \"(Ȑ; Ȑ; R◌̏; Ȑ; R◌̏; ) LATIN CAPITAL LETTER R WITH DOUBLE GRAVE\"");

    err = nfd_normaliser("\xC8\x91", strlen("\xC8\x91"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x91", "\x72" "\xCC\x8F", out_buffer, "847 NFD \"(ȑ; ȑ; r◌̏; ȑ; r◌̏; ) LATIN SMALL LETTER R WITH DOUBLE GRAVE\"");

    err = nfkd_normaliser("\xC8\x91", strlen("\xC8\x91"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x91", "\x72" "\xCC\x8F", out_buffer, "848 NFKD \"(ȑ; ȑ; r◌̏; ȑ; r◌̏; ) LATIN SMALL LETTER R WITH DOUBLE GRAVE\"");

    err = nfc_normaliser("\xC8\x91", strlen("\xC8\x91"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x91", "\xC8\x91", out_buffer, "849 NFC \"(ȑ; ȑ; r◌̏; ȑ; r◌̏; ) LATIN SMALL LETTER R WITH DOUBLE GRAVE\"");

    err = nfd_normaliser("\xC8\x92", strlen("\xC8\x92"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x92", "\x52" "\xCC\x91", out_buffer, "850 NFD \"(Ȓ; Ȓ; R◌̑; Ȓ; R◌̑; ) LATIN CAPITAL LETTER R WITH INVERTED BREVE\"");

    err = nfkd_normaliser("\xC8\x92", strlen("\xC8\x92"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x92", "\x52" "\xCC\x91", out_buffer, "851 NFKD \"(Ȓ; Ȓ; R◌̑; Ȓ; R◌̑; ) LATIN CAPITAL LETTER R WITH INVERTED BREVE\"");

    err = nfc_normaliser("\xC8\x92", strlen("\xC8\x92"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x92", "\xC8\x92", out_buffer, "852 NFC \"(Ȓ; Ȓ; R◌̑; Ȓ; R◌̑; ) LATIN CAPITAL LETTER R WITH INVERTED BREVE\"");

    err = nfd_normaliser("\xC8\x93", strlen("\xC8\x93"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x93", "\x72" "\xCC\x91", out_buffer, "853 NFD \"(ȓ; ȓ; r◌̑; ȓ; r◌̑; ) LATIN SMALL LETTER R WITH INVERTED BREVE\"");

    err = nfkd_normaliser("\xC8\x93", strlen("\xC8\x93"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x93", "\x72" "\xCC\x91", out_buffer, "854 NFKD \"(ȓ; ȓ; r◌̑; ȓ; r◌̑; ) LATIN SMALL LETTER R WITH INVERTED BREVE\"");

    err = nfc_normaliser("\xC8\x93", strlen("\xC8\x93"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x93", "\xC8\x93", out_buffer, "855 NFC \"(ȓ; ȓ; r◌̑; ȓ; r◌̑; ) LATIN SMALL LETTER R WITH INVERTED BREVE\"");

    err = nfd_normaliser("\xC8\x94", strlen("\xC8\x94"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x94", "\x55" "\xCC\x8F", out_buffer, "856 NFD \"(Ȕ; Ȕ; U◌̏; Ȕ; U◌̏; ) LATIN CAPITAL LETTER U WITH DOUBLE GRAVE\"");

    err = nfkd_normaliser("\xC8\x94", strlen("\xC8\x94"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x94", "\x55" "\xCC\x8F", out_buffer, "857 NFKD \"(Ȕ; Ȕ; U◌̏; Ȕ; U◌̏; ) LATIN CAPITAL LETTER U WITH DOUBLE GRAVE\"");

    err = nfc_normaliser("\xC8\x94", strlen("\xC8\x94"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x94", "\xC8\x94", out_buffer, "858 NFC \"(Ȕ; Ȕ; U◌̏; Ȕ; U◌̏; ) LATIN CAPITAL LETTER U WITH DOUBLE GRAVE\"");

    err = nfd_normaliser("\xC8\x95", strlen("\xC8\x95"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x95", "\x75" "\xCC\x8F", out_buffer, "859 NFD \"(ȕ; ȕ; u◌̏; ȕ; u◌̏; ) LATIN SMALL LETTER U WITH DOUBLE GRAVE\"");

    err = nfkd_normaliser("\xC8\x95", strlen("\xC8\x95"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x95", "\x75" "\xCC\x8F", out_buffer, "860 NFKD \"(ȕ; ȕ; u◌̏; ȕ; u◌̏; ) LATIN SMALL LETTER U WITH DOUBLE GRAVE\"");

    err = nfc_normaliser("\xC8\x95", strlen("\xC8\x95"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x95", "\xC8\x95", out_buffer, "861 NFC \"(ȕ; ȕ; u◌̏; ȕ; u◌̏; ) LATIN SMALL LETTER U WITH DOUBLE GRAVE\"");

    err = nfd_normaliser("\xC8\x96", strlen("\xC8\x96"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x96", "\x55" "\xCC\x91", out_buffer, "862 NFD \"(Ȗ; Ȗ; U◌̑; Ȗ; U◌̑; ) LATIN CAPITAL LETTER U WITH INVERTED BREVE\"");

    err = nfkd_normaliser("\xC8\x96", strlen("\xC8\x96"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x96", "\x55" "\xCC\x91", out_buffer, "863 NFKD \"(Ȗ; Ȗ; U◌̑; Ȗ; U◌̑; ) LATIN CAPITAL LETTER U WITH INVERTED BREVE\"");

    err = nfc_normaliser("\xC8\x96", strlen("\xC8\x96"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x96", "\xC8\x96", out_buffer, "864 NFC \"(Ȗ; Ȗ; U◌̑; Ȗ; U◌̑; ) LATIN CAPITAL LETTER U WITH INVERTED BREVE\"");

    err = nfd_normaliser("\xC8\x97", strlen("\xC8\x97"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x97", "\x75" "\xCC\x91", out_buffer, "865 NFD \"(ȗ; ȗ; u◌̑; ȗ; u◌̑; ) LATIN SMALL LETTER U WITH INVERTED BREVE\"");

    err = nfkd_normaliser("\xC8\x97", strlen("\xC8\x97"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x97", "\x75" "\xCC\x91", out_buffer, "866 NFKD \"(ȗ; ȗ; u◌̑; ȗ; u◌̑; ) LATIN SMALL LETTER U WITH INVERTED BREVE\"");

    err = nfc_normaliser("\xC8\x97", strlen("\xC8\x97"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x97", "\xC8\x97", out_buffer, "867 NFC \"(ȗ; ȗ; u◌̑; ȗ; u◌̑; ) LATIN SMALL LETTER U WITH INVERTED BREVE\"");

    err = nfd_normaliser("\xC8\x98", strlen("\xC8\x98"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x98", "\x53" "\xCC\xA6", out_buffer, "868 NFD \"(Ș; Ș; S◌̦; Ș; S◌̦; ) LATIN CAPITAL LETTER S WITH COMMA BELOW\"");

    err = nfkd_normaliser("\xC8\x98", strlen("\xC8\x98"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x98", "\x53" "\xCC\xA6", out_buffer, "869 NFKD \"(Ș; Ș; S◌̦; Ș; S◌̦; ) LATIN CAPITAL LETTER S WITH COMMA BELOW\"");

    err = nfc_normaliser("\xC8\x98", strlen("\xC8\x98"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x98", "\xC8\x98", out_buffer, "870 NFC \"(Ș; Ș; S◌̦; Ș; S◌̦; ) LATIN CAPITAL LETTER S WITH COMMA BELOW\"");

    err = nfd_normaliser("\xC8\x99", strlen("\xC8\x99"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x99", "\x73" "\xCC\xA6", out_buffer, "871 NFD \"(ș; ș; s◌̦; ș; s◌̦; ) LATIN SMALL LETTER S WITH COMMA BELOW\"");

    err = nfkd_normaliser("\xC8\x99", strlen("\xC8\x99"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x99", "\x73" "\xCC\xA6", out_buffer, "872 NFKD \"(ș; ș; s◌̦; ș; s◌̦; ) LATIN SMALL LETTER S WITH COMMA BELOW\"");

    err = nfc_normaliser("\xC8\x99", strlen("\xC8\x99"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x99", "\xC8\x99", out_buffer, "873 NFC \"(ș; ș; s◌̦; ș; s◌̦; ) LATIN SMALL LETTER S WITH COMMA BELOW\"");

    err = nfd_normaliser("\xC8\x9A", strlen("\xC8\x9A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x9A", "\x54" "\xCC\xA6", out_buffer, "874 NFD \"(Ț; Ț; T◌̦; Ț; T◌̦; ) LATIN CAPITAL LETTER T WITH COMMA BELOW\"");

    err = nfkd_normaliser("\xC8\x9A", strlen("\xC8\x9A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x9A", "\x54" "\xCC\xA6", out_buffer, "875 NFKD \"(Ț; Ț; T◌̦; Ț; T◌̦; ) LATIN CAPITAL LETTER T WITH COMMA BELOW\"");

    err = nfc_normaliser("\xC8\x9A", strlen("\xC8\x9A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x9A", "\xC8\x9A", out_buffer, "876 NFC \"(Ț; Ț; T◌̦; Ț; T◌̦; ) LATIN CAPITAL LETTER T WITH COMMA BELOW\"");

    err = nfd_normaliser("\xC8\x9B", strlen("\xC8\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x9B", "\x74" "\xCC\xA6", out_buffer, "877 NFD \"(ț; ț; t◌̦; ț; t◌̦; ) LATIN SMALL LETTER T WITH COMMA BELOW\"");

    err = nfkd_normaliser("\xC8\x9B", strlen("\xC8\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x9B", "\x74" "\xCC\xA6", out_buffer, "878 NFKD \"(ț; ț; t◌̦; ț; t◌̦; ) LATIN SMALL LETTER T WITH COMMA BELOW\"");

    err = nfc_normaliser("\xC8\x9B", strlen("\xC8\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x9B", "\xC8\x9B", out_buffer, "879 NFC \"(ț; ț; t◌̦; ț; t◌̦; ) LATIN SMALL LETTER T WITH COMMA BELOW\"");

    err = nfd_normaliser("\xC8\x9E", strlen("\xC8\x9E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x9E", "\x48" "\xCC\x8C", out_buffer, "880 NFD \"(Ȟ; Ȟ; H◌̌; Ȟ; H◌̌; ) LATIN CAPITAL LETTER H WITH CARON\"");

    err = nfkd_normaliser("\xC8\x9E", strlen("\xC8\x9E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x9E", "\x48" "\xCC\x8C", out_buffer, "881 NFKD \"(Ȟ; Ȟ; H◌̌; Ȟ; H◌̌; ) LATIN CAPITAL LETTER H WITH CARON\"");

    err = nfc_normaliser("\xC8\x9E", strlen("\xC8\x9E"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x9E", "\xC8\x9E", out_buffer, "882 NFC \"(Ȟ; Ȟ; H◌̌; Ȟ; H◌̌; ) LATIN CAPITAL LETTER H WITH CARON\"");

    err = nfd_normaliser("\xC8\x9F", strlen("\xC8\x9F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x9F", "\x68" "\xCC\x8C", out_buffer, "883 NFD \"(ȟ; ȟ; h◌̌; ȟ; h◌̌; ) LATIN SMALL LETTER H WITH CARON\"");

    err = nfkd_normaliser("\xC8\x9F", strlen("\xC8\x9F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x9F", "\x68" "\xCC\x8C", out_buffer, "884 NFKD \"(ȟ; ȟ; h◌̌; ȟ; h◌̌; ) LATIN SMALL LETTER H WITH CARON\"");

    err = nfc_normaliser("\xC8\x9F", strlen("\xC8\x9F"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\x9F", "\xC8\x9F", out_buffer, "885 NFC \"(ȟ; ȟ; h◌̌; ȟ; h◌̌; ) LATIN SMALL LETTER H WITH CARON\"");

    err = nfd_normaliser("\xC8\xA6", strlen("\xC8\xA6"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xA6", "\x41" "\xCC\x87", out_buffer, "886 NFD \"(Ȧ; Ȧ; A◌̇; Ȧ; A◌̇; ) LATIN CAPITAL LETTER A WITH DOT ABOVE\"");

    err = nfkd_normaliser("\xC8\xA6", strlen("\xC8\xA6"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xA6", "\x41" "\xCC\x87", out_buffer, "887 NFKD \"(Ȧ; Ȧ; A◌̇; Ȧ; A◌̇; ) LATIN CAPITAL LETTER A WITH DOT ABOVE\"");

    err = nfc_normaliser("\xC8\xA6", strlen("\xC8\xA6"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xA6", "\xC8\xA6", out_buffer, "888 NFC \"(Ȧ; Ȧ; A◌̇; Ȧ; A◌̇; ) LATIN CAPITAL LETTER A WITH DOT ABOVE\"");

    err = nfd_normaliser("\xC8\xA7", strlen("\xC8\xA7"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xA7", "\x61" "\xCC\x87", out_buffer, "889 NFD \"(ȧ; ȧ; a◌̇; ȧ; a◌̇; ) LATIN SMALL LETTER A WITH DOT ABOVE\"");

    err = nfkd_normaliser("\xC8\xA7", strlen("\xC8\xA7"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xA7", "\x61" "\xCC\x87", out_buffer, "890 NFKD \"(ȧ; ȧ; a◌̇; ȧ; a◌̇; ) LATIN SMALL LETTER A WITH DOT ABOVE\"");

    err = nfc_normaliser("\xC8\xA7", strlen("\xC8\xA7"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xA7", "\xC8\xA7", out_buffer, "891 NFC \"(ȧ; ȧ; a◌̇; ȧ; a◌̇; ) LATIN SMALL LETTER A WITH DOT ABOVE\"");

    err = nfd_normaliser("\xC8\xA8", strlen("\xC8\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xA8", "\x45" "\xCC\xA7", out_buffer, "892 NFD \"(Ȩ; Ȩ; E◌̧; Ȩ; E◌̧; ) LATIN CAPITAL LETTER E WITH CEDILLA\"");

    err = nfkd_normaliser("\xC8\xA8", strlen("\xC8\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xA8", "\x45" "\xCC\xA7", out_buffer, "893 NFKD \"(Ȩ; Ȩ; E◌̧; Ȩ; E◌̧; ) LATIN CAPITAL LETTER E WITH CEDILLA\"");

    err = nfc_normaliser("\xC8\xA8", strlen("\xC8\xA8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xA8", "\xC8\xA8", out_buffer, "894 NFC \"(Ȩ; Ȩ; E◌̧; Ȩ; E◌̧; ) LATIN CAPITAL LETTER E WITH CEDILLA\"");

    err = nfd_normaliser("\xC8\xA9", strlen("\xC8\xA9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xA9", "\x65" "\xCC\xA7", out_buffer, "895 NFD \"(ȩ; ȩ; e◌̧; ȩ; e◌̧; ) LATIN SMALL LETTER E WITH CEDILLA\"");

    err = nfkd_normaliser("\xC8\xA9", strlen("\xC8\xA9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xA9", "\x65" "\xCC\xA7", out_buffer, "896 NFKD \"(ȩ; ȩ; e◌̧; ȩ; e◌̧; ) LATIN SMALL LETTER E WITH CEDILLA\"");

    err = nfc_normaliser("\xC8\xA9", strlen("\xC8\xA9"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xA9", "\xC8\xA9", out_buffer, "897 NFC \"(ȩ; ȩ; e◌̧; ȩ; e◌̧; ) LATIN SMALL LETTER E WITH CEDILLA\"");

    err = nfd_normaliser("\xC8\xAA", strlen("\xC8\xAA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xAA", "\x4F" "\xCC\x88" "\xCC\x84", out_buffer, "898 NFD \"(Ȫ; Ȫ; O◌̈◌̄; Ȫ; O◌̈◌̄; ) LATIN CAPITAL LETTER O WITH DIAERESIS AND MACRON\"");

    err = nfkd_normaliser("\xC8\xAA", strlen("\xC8\xAA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xAA", "\x4F" "\xCC\x88" "\xCC\x84", out_buffer, "899 NFKD \"(Ȫ; Ȫ; O◌̈◌̄; Ȫ; O◌̈◌̄; ) LATIN CAPITAL LETTER O WITH DIAERESIS AND MACRON\"");

    err = nfc_normaliser("\xC8\xAA", strlen("\xC8\xAA"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xAA", "\xC8\xAA", out_buffer, "900 NFC \"(Ȫ; Ȫ; O◌̈◌̄; Ȫ; O◌̈◌̄; ) LATIN CAPITAL LETTER O WITH DIAERESIS AND MACRON\"");

    err = nfd_normaliser("\xC8\xAB", strlen("\xC8\xAB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xAB", "\x6F" "\xCC\x88" "\xCC\x84", out_buffer, "901 NFD \"(ȫ; ȫ; o◌̈◌̄; ȫ; o◌̈◌̄; ) LATIN SMALL LETTER O WITH DIAERESIS AND MACRON\"");

    err = nfkd_normaliser("\xC8\xAB", strlen("\xC8\xAB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xAB", "\x6F" "\xCC\x88" "\xCC\x84", out_buffer, "902 NFKD \"(ȫ; ȫ; o◌̈◌̄; ȫ; o◌̈◌̄; ) LATIN SMALL LETTER O WITH DIAERESIS AND MACRON\"");

    err = nfc_normaliser("\xC8\xAB", strlen("\xC8\xAB"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xAB", "\xC8\xAB", out_buffer, "903 NFC \"(ȫ; ȫ; o◌̈◌̄; ȫ; o◌̈◌̄; ) LATIN SMALL LETTER O WITH DIAERESIS AND MACRON\"");

    err = nfd_normaliser("\xC8\xAC", strlen("\xC8\xAC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xAC", "\x4F" "\xCC\x83" "\xCC\x84", out_buffer, "904 NFD \"(Ȭ; Ȭ; O◌̃◌̄; Ȭ; O◌̃◌̄; ) LATIN CAPITAL LETTER O WITH TILDE AND MACRON\"");

    err = nfkd_normaliser("\xC8\xAC", strlen("\xC8\xAC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xAC", "\x4F" "\xCC\x83" "\xCC\x84", out_buffer, "905 NFKD \"(Ȭ; Ȭ; O◌̃◌̄; Ȭ; O◌̃◌̄; ) LATIN CAPITAL LETTER O WITH TILDE AND MACRON\"");

    err = nfc_normaliser("\xC8\xAC", strlen("\xC8\xAC"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xAC", "\xC8\xAC", out_buffer, "906 NFC \"(Ȭ; Ȭ; O◌̃◌̄; Ȭ; O◌̃◌̄; ) LATIN CAPITAL LETTER O WITH TILDE AND MACRON\"");

    err = nfd_normaliser("\xC8\xAD", strlen("\xC8\xAD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xAD", "\x6F" "\xCC\x83" "\xCC\x84", out_buffer, "907 NFD \"(ȭ; ȭ; o◌̃◌̄; ȭ; o◌̃◌̄; ) LATIN SMALL LETTER O WITH TILDE AND MACRON\"");

    err = nfkd_normaliser("\xC8\xAD", strlen("\xC8\xAD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xAD", "\x6F" "\xCC\x83" "\xCC\x84", out_buffer, "908 NFKD \"(ȭ; ȭ; o◌̃◌̄; ȭ; o◌̃◌̄; ) LATIN SMALL LETTER O WITH TILDE AND MACRON\"");

    err = nfc_normaliser("\xC8\xAD", strlen("\xC8\xAD"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xAD", "\xC8\xAD", out_buffer, "909 NFC \"(ȭ; ȭ; o◌̃◌̄; ȭ; o◌̃◌̄; ) LATIN SMALL LETTER O WITH TILDE AND MACRON\"");

    err = nfd_normaliser("\xC8\xAE", strlen("\xC8\xAE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xAE", "\x4F" "\xCC\x87", out_buffer, "910 NFD \"(Ȯ; Ȯ; O◌̇; Ȯ; O◌̇; ) LATIN CAPITAL LETTER O WITH DOT ABOVE\"");

    err = nfkd_normaliser("\xC8\xAE", strlen("\xC8\xAE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xAE", "\x4F" "\xCC\x87", out_buffer, "911 NFKD \"(Ȯ; Ȯ; O◌̇; Ȯ; O◌̇; ) LATIN CAPITAL LETTER O WITH DOT ABOVE\"");

    err = nfc_normaliser("\xC8\xAE", strlen("\xC8\xAE"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xAE", "\xC8\xAE", out_buffer, "912 NFC \"(Ȯ; Ȯ; O◌̇; Ȯ; O◌̇; ) LATIN CAPITAL LETTER O WITH DOT ABOVE\"");

    err = nfd_normaliser("\xC8\xAF", strlen("\xC8\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xAF", "\x6F" "\xCC\x87", out_buffer, "913 NFD \"(ȯ; ȯ; o◌̇; ȯ; o◌̇; ) LATIN SMALL LETTER O WITH DOT ABOVE\"");

    err = nfkd_normaliser("\xC8\xAF", strlen("\xC8\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xAF", "\x6F" "\xCC\x87", out_buffer, "914 NFKD \"(ȯ; ȯ; o◌̇; ȯ; o◌̇; ) LATIN SMALL LETTER O WITH DOT ABOVE\"");

    err = nfc_normaliser("\xC8\xAF", strlen("\xC8\xAF"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xAF", "\xC8\xAF", out_buffer, "915 NFC \"(ȯ; ȯ; o◌̇; ȯ; o◌̇; ) LATIN SMALL LETTER O WITH DOT ABOVE\"");

    err = nfd_normaliser("\xC8\xB0", strlen("\xC8\xB0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xB0", "\x4F" "\xCC\x87" "\xCC\x84", out_buffer, "916 NFD \"(Ȱ; Ȱ; O◌̇◌̄; Ȱ; O◌̇◌̄; ) LATIN CAPITAL LETTER O WITH DOT ABOVE AND MACRON\"");

    err = nfkd_normaliser("\xC8\xB0", strlen("\xC8\xB0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xB0", "\x4F" "\xCC\x87" "\xCC\x84", out_buffer, "917 NFKD \"(Ȱ; Ȱ; O◌̇◌̄; Ȱ; O◌̇◌̄; ) LATIN CAPITAL LETTER O WITH DOT ABOVE AND MACRON\"");

    err = nfc_normaliser("\xC8\xB0", strlen("\xC8\xB0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xB0", "\xC8\xB0", out_buffer, "918 NFC \"(Ȱ; Ȱ; O◌̇◌̄; Ȱ; O◌̇◌̄; ) LATIN CAPITAL LETTER O WITH DOT ABOVE AND MACRON\"");

    err = nfd_normaliser("\xC8\xB1", strlen("\xC8\xB1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xB1", "\x6F" "\xCC\x87" "\xCC\x84", out_buffer, "919 NFD \"(ȱ; ȱ; o◌̇◌̄; ȱ; o◌̇◌̄; ) LATIN SMALL LETTER O WITH DOT ABOVE AND MACRON\"");

    err = nfkd_normaliser("\xC8\xB1", strlen("\xC8\xB1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xB1", "\x6F" "\xCC\x87" "\xCC\x84", out_buffer, "920 NFKD \"(ȱ; ȱ; o◌̇◌̄; ȱ; o◌̇◌̄; ) LATIN SMALL LETTER O WITH DOT ABOVE AND MACRON\"");

    err = nfc_normaliser("\xC8\xB1", strlen("\xC8\xB1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xB1", "\xC8\xB1", out_buffer, "921 NFC \"(ȱ; ȱ; o◌̇◌̄; ȱ; o◌̇◌̄; ) LATIN SMALL LETTER O WITH DOT ABOVE AND MACRON\"");

    err = nfd_normaliser("\xC8\xB2", strlen("\xC8\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xB2", "\x59" "\xCC\x84", out_buffer, "922 NFD \"(Ȳ; Ȳ; Y◌̄; Ȳ; Y◌̄; ) LATIN CAPITAL LETTER Y WITH MACRON\"");

    err = nfkd_normaliser("\xC8\xB2", strlen("\xC8\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xB2", "\x59" "\xCC\x84", out_buffer, "923 NFKD \"(Ȳ; Ȳ; Y◌̄; Ȳ; Y◌̄; ) LATIN CAPITAL LETTER Y WITH MACRON\"");

    err = nfc_normaliser("\xC8\xB2", strlen("\xC8\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xB2", "\xC8\xB2", out_buffer, "924 NFC \"(Ȳ; Ȳ; Y◌̄; Ȳ; Y◌̄; ) LATIN CAPITAL LETTER Y WITH MACRON\"");

    err = nfd_normaliser("\xC8\xB3", strlen("\xC8\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xB3", "\x79" "\xCC\x84", out_buffer, "925 NFD \"(ȳ; ȳ; y◌̄; ȳ; y◌̄; ) LATIN SMALL LETTER Y WITH MACRON\"");

    err = nfkd_normaliser("\xC8\xB3", strlen("\xC8\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xB3", "\x79" "\xCC\x84", out_buffer, "926 NFKD \"(ȳ; ȳ; y◌̄; ȳ; y◌̄; ) LATIN SMALL LETTER Y WITH MACRON\"");

    err = nfc_normaliser("\xC8\xB3", strlen("\xC8\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xC8\xB3", "\xC8\xB3", out_buffer, "927 NFC \"(ȳ; ȳ; y◌̄; ȳ; y◌̄; ) LATIN SMALL LETTER Y WITH MACRON\"");

    err = nfd_normaliser("\xCA\xB0", strlen("\xCA\xB0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB0", "\xCA\xB0", out_buffer, "928 NFD \"(ʰ; ʰ; ʰ; h; h; ) MODIFIER LETTER SMALL H\"");

    err = nfkd_normaliser("\xCA\xB0", strlen("\xCA\xB0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB0", "\x68", out_buffer, "929 NFKD \"(ʰ; ʰ; ʰ; h; h; ) MODIFIER LETTER SMALL H\"");

    err = nfc_normaliser("\xCA\xB0", strlen("\xCA\xB0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB0", "\xCA\xB0", out_buffer, "930 NFC \"(ʰ; ʰ; ʰ; h; h; ) MODIFIER LETTER SMALL H\"");

    err = nfd_normaliser("\xCA\xB1", strlen("\xCA\xB1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB1", "\xCA\xB1", out_buffer, "931 NFD \"(ʱ; ʱ; ʱ; ɦ; ɦ; ) MODIFIER LETTER SMALL H WITH HOOK\"");

    err = nfkd_normaliser("\xCA\xB1", strlen("\xCA\xB1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB1", "\xC9\xA6", out_buffer, "932 NFKD \"(ʱ; ʱ; ʱ; ɦ; ɦ; ) MODIFIER LETTER SMALL H WITH HOOK\"");

    err = nfc_normaliser("\xCA\xB1", strlen("\xCA\xB1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB1", "\xCA\xB1", out_buffer, "933 NFC \"(ʱ; ʱ; ʱ; ɦ; ɦ; ) MODIFIER LETTER SMALL H WITH HOOK\"");

    err = nfd_normaliser("\xCA\xB2", strlen("\xCA\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB2", "\xCA\xB2", out_buffer, "934 NFD \"(ʲ; ʲ; ʲ; j; j; ) MODIFIER LETTER SMALL J\"");

    err = nfkd_normaliser("\xCA\xB2", strlen("\xCA\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB2", "\x6A", out_buffer, "935 NFKD \"(ʲ; ʲ; ʲ; j; j; ) MODIFIER LETTER SMALL J\"");

    err = nfc_normaliser("\xCA\xB2", strlen("\xCA\xB2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB2", "\xCA\xB2", out_buffer, "936 NFC \"(ʲ; ʲ; ʲ; j; j; ) MODIFIER LETTER SMALL J\"");

    err = nfd_normaliser("\xCA\xB3", strlen("\xCA\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB3", "\xCA\xB3", out_buffer, "937 NFD \"(ʳ; ʳ; ʳ; r; r; ) MODIFIER LETTER SMALL R\"");

    err = nfkd_normaliser("\xCA\xB3", strlen("\xCA\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB3", "\x72", out_buffer, "938 NFKD \"(ʳ; ʳ; ʳ; r; r; ) MODIFIER LETTER SMALL R\"");

    err = nfc_normaliser("\xCA\xB3", strlen("\xCA\xB3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB3", "\xCA\xB3", out_buffer, "939 NFC \"(ʳ; ʳ; ʳ; r; r; ) MODIFIER LETTER SMALL R\"");

    err = nfd_normaliser("\xCA\xB4", strlen("\xCA\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB4", "\xCA\xB4", out_buffer, "940 NFD \"(ʴ; ʴ; ʴ; ɹ; ɹ; ) MODIFIER LETTER SMALL TURNED R\"");

    err = nfkd_normaliser("\xCA\xB4", strlen("\xCA\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB4", "\xC9\xB9", out_buffer, "941 NFKD \"(ʴ; ʴ; ʴ; ɹ; ɹ; ) MODIFIER LETTER SMALL TURNED R\"");

    err = nfc_normaliser("\xCA\xB4", strlen("\xCA\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB4", "\xCA\xB4", out_buffer, "942 NFC \"(ʴ; ʴ; ʴ; ɹ; ɹ; ) MODIFIER LETTER SMALL TURNED R\"");

    err = nfd_normaliser("\xCA\xB5", strlen("\xCA\xB5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB5", "\xCA\xB5", out_buffer, "943 NFD \"(ʵ; ʵ; ʵ; ɻ; ɻ; ) MODIFIER LETTER SMALL TURNED R WITH HOOK\"");

    err = nfkd_normaliser("\xCA\xB5", strlen("\xCA\xB5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB5", "\xC9\xBB", out_buffer, "944 NFKD \"(ʵ; ʵ; ʵ; ɻ; ɻ; ) MODIFIER LETTER SMALL TURNED R WITH HOOK\"");

    err = nfc_normaliser("\xCA\xB5", strlen("\xCA\xB5"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB5", "\xCA\xB5", out_buffer, "945 NFC \"(ʵ; ʵ; ʵ; ɻ; ɻ; ) MODIFIER LETTER SMALL TURNED R WITH HOOK\"");

    err = nfd_normaliser("\xCA\xB6", strlen("\xCA\xB6"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB6", "\xCA\xB6", out_buffer, "946 NFD \"(ʶ; ʶ; ʶ; ʁ; ʁ; ) MODIFIER LETTER SMALL CAPITAL INVERTED R\"");

    err = nfkd_normaliser("\xCA\xB6", strlen("\xCA\xB6"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB6", "\xCA\x81", out_buffer, "947 NFKD \"(ʶ; ʶ; ʶ; ʁ; ʁ; ) MODIFIER LETTER SMALL CAPITAL INVERTED R\"");

    err = nfc_normaliser("\xCA\xB6", strlen("\xCA\xB6"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB6", "\xCA\xB6", out_buffer, "948 NFC \"(ʶ; ʶ; ʶ; ʁ; ʁ; ) MODIFIER LETTER SMALL CAPITAL INVERTED R\"");

    err = nfd_normaliser("\xCA\xB7", strlen("\xCA\xB7"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB7", "\xCA\xB7", out_buffer, "949 NFD \"(ʷ; ʷ; ʷ; w; w; ) MODIFIER LETTER SMALL W\"");

    err = nfkd_normaliser("\xCA\xB7", strlen("\xCA\xB7"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB7", "\x77", out_buffer, "950 NFKD \"(ʷ; ʷ; ʷ; w; w; ) MODIFIER LETTER SMALL W\"");

    err = nfc_normaliser("\xCA\xB7", strlen("\xCA\xB7"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB7", "\xCA\xB7", out_buffer, "951 NFC \"(ʷ; ʷ; ʷ; w; w; ) MODIFIER LETTER SMALL W\"");

    err = nfd_normaliser("\xCA\xB8", strlen("\xCA\xB8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB8", "\xCA\xB8", out_buffer, "952 NFD \"(ʸ; ʸ; ʸ; y; y; ) MODIFIER LETTER SMALL Y\"");

    err = nfkd_normaliser("\xCA\xB8", strlen("\xCA\xB8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB8", "\x79", out_buffer, "953 NFKD \"(ʸ; ʸ; ʸ; y; y; ) MODIFIER LETTER SMALL Y\"");

    err = nfc_normaliser("\xCA\xB8", strlen("\xCA\xB8"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCA\xB8", "\xCA\xB8", out_buffer, "954 NFC \"(ʸ; ʸ; ʸ; y; y; ) MODIFIER LETTER SMALL Y\"");

    err = nfd_normaliser("\xCB\x98", strlen("\xCB\x98"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\x98", "\xCB\x98", out_buffer, "955 NFD \"(˘; ˘; ˘;  ◌̆;  ◌̆; ) BREVE\"");

    err = nfkd_normaliser("\xCB\x98", strlen("\xCB\x98"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\x98", "\x20" "\xCC\x86", out_buffer, "956 NFKD \"(˘; ˘; ˘;  ◌̆;  ◌̆; ) BREVE\"");

    err = nfc_normaliser("\xCB\x98", strlen("\xCB\x98"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\x98", "\xCB\x98", out_buffer, "957 NFC \"(˘; ˘; ˘;  ◌̆;  ◌̆; ) BREVE\"");

    err = nfd_normaliser("\xCB\x99", strlen("\xCB\x99"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\x99", "\xCB\x99", out_buffer, "958 NFD \"(˙; ˙; ˙;  ◌̇;  ◌̇; ) DOT ABOVE\"");

    err = nfkd_normaliser("\xCB\x99", strlen("\xCB\x99"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\x99", "\x20" "\xCC\x87", out_buffer, "959 NFKD \"(˙; ˙; ˙;  ◌̇;  ◌̇; ) DOT ABOVE\"");

    err = nfc_normaliser("\xCB\x99", strlen("\xCB\x99"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\x99", "\xCB\x99", out_buffer, "960 NFC \"(˙; ˙; ˙;  ◌̇;  ◌̇; ) DOT ABOVE\"");

    err = nfd_normaliser("\xCB\x9A", strlen("\xCB\x9A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\x9A", "\xCB\x9A", out_buffer, "961 NFD \"(˚; ˚; ˚;  ◌̊;  ◌̊; ) RING ABOVE\"");

    err = nfkd_normaliser("\xCB\x9A", strlen("\xCB\x9A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\x9A", "\x20" "\xCC\x8A", out_buffer, "962 NFKD \"(˚; ˚; ˚;  ◌̊;  ◌̊; ) RING ABOVE\"");

    err = nfc_normaliser("\xCB\x9A", strlen("\xCB\x9A"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\x9A", "\xCB\x9A", out_buffer, "963 NFC \"(˚; ˚; ˚;  ◌̊;  ◌̊; ) RING ABOVE\"");

    err = nfd_normaliser("\xCB\x9B", strlen("\xCB\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\x9B", "\xCB\x9B", out_buffer, "964 NFD \"(˛; ˛; ˛;  ◌̨;  ◌̨; ) OGONEK\"");

    err = nfkd_normaliser("\xCB\x9B", strlen("\xCB\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\x9B", "\x20" "\xCC\xA8", out_buffer, "965 NFKD \"(˛; ˛; ˛;  ◌̨;  ◌̨; ) OGONEK\"");

    err = nfc_normaliser("\xCB\x9B", strlen("\xCB\x9B"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\x9B", "\xCB\x9B", out_buffer, "966 NFC \"(˛; ˛; ˛;  ◌̨;  ◌̨; ) OGONEK\"");

    err = nfd_normaliser("\xCB\x9C", strlen("\xCB\x9C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\x9C", "\xCB\x9C", out_buffer, "967 NFD \"(˜; ˜; ˜;  ◌̃;  ◌̃; ) SMALL TILDE\"");

    err = nfkd_normaliser("\xCB\x9C", strlen("\xCB\x9C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\x9C", "\x20" "\xCC\x83", out_buffer, "968 NFKD \"(˜; ˜; ˜;  ◌̃;  ◌̃; ) SMALL TILDE\"");

    err = nfc_normaliser("\xCB\x9C", strlen("\xCB\x9C"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\x9C", "\xCB\x9C", out_buffer, "969 NFC \"(˜; ˜; ˜;  ◌̃;  ◌̃; ) SMALL TILDE\"");

    err = nfd_normaliser("\xCB\x9D", strlen("\xCB\x9D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\x9D", "\xCB\x9D", out_buffer, "970 NFD \"(˝; ˝; ˝;  ◌̋;  ◌̋; ) DOUBLE ACUTE ACCENT\"");

    err = nfkd_normaliser("\xCB\x9D", strlen("\xCB\x9D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\x9D", "\x20" "\xCC\x8B", out_buffer, "971 NFKD \"(˝; ˝; ˝;  ◌̋;  ◌̋; ) DOUBLE ACUTE ACCENT\"");

    err = nfc_normaliser("\xCB\x9D", strlen("\xCB\x9D"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\x9D", "\xCB\x9D", out_buffer, "972 NFC \"(˝; ˝; ˝;  ◌̋;  ◌̋; ) DOUBLE ACUTE ACCENT\"");

    err = nfd_normaliser("\xCB\xA0", strlen("\xCB\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\xA0", "\xCB\xA0", out_buffer, "973 NFD \"(ˠ; ˠ; ˠ; ɣ; ɣ; ) MODIFIER LETTER SMALL GAMMA\"");

    err = nfkd_normaliser("\xCB\xA0", strlen("\xCB\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\xA0", "\xC9\xA3", out_buffer, "974 NFKD \"(ˠ; ˠ; ˠ; ɣ; ɣ; ) MODIFIER LETTER SMALL GAMMA\"");

    err = nfc_normaliser("\xCB\xA0", strlen("\xCB\xA0"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\xA0", "\xCB\xA0", out_buffer, "975 NFC \"(ˠ; ˠ; ˠ; ɣ; ɣ; ) MODIFIER LETTER SMALL GAMMA\"");

    err = nfd_normaliser("\xCB\xA1", strlen("\xCB\xA1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\xA1", "\xCB\xA1", out_buffer, "976 NFD \"(ˡ; ˡ; ˡ; l; l; ) MODIFIER LETTER SMALL L\"");

    err = nfkd_normaliser("\xCB\xA1", strlen("\xCB\xA1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\xA1", "\x6C", out_buffer, "977 NFKD \"(ˡ; ˡ; ˡ; l; l; ) MODIFIER LETTER SMALL L\"");

    err = nfc_normaliser("\xCB\xA1", strlen("\xCB\xA1"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\xA1", "\xCB\xA1", out_buffer, "978 NFC \"(ˡ; ˡ; ˡ; l; l; ) MODIFIER LETTER SMALL L\"");

    err = nfd_normaliser("\xCB\xA2", strlen("\xCB\xA2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\xA2", "\xCB\xA2", out_buffer, "979 NFD \"(ˢ; ˢ; ˢ; s; s; ) MODIFIER LETTER SMALL S\"");

    err = nfkd_normaliser("\xCB\xA2", strlen("\xCB\xA2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\xA2", "\x73", out_buffer, "980 NFKD \"(ˢ; ˢ; ˢ; s; s; ) MODIFIER LETTER SMALL S\"");

    err = nfc_normaliser("\xCB\xA2", strlen("\xCB\xA2"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\xA2", "\xCB\xA2", out_buffer, "981 NFC \"(ˢ; ˢ; ˢ; s; s; ) MODIFIER LETTER SMALL S\"");

    err = nfd_normaliser("\xCB\xA3", strlen("\xCB\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\xA3", "\xCB\xA3", out_buffer, "982 NFD \"(ˣ; ˣ; ˣ; x; x; ) MODIFIER LETTER SMALL X\"");

    err = nfkd_normaliser("\xCB\xA3", strlen("\xCB\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\xA3", "\x78", out_buffer, "983 NFKD \"(ˣ; ˣ; ˣ; x; x; ) MODIFIER LETTER SMALL X\"");

    err = nfc_normaliser("\xCB\xA3", strlen("\xCB\xA3"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\xA3", "\xCB\xA3", out_buffer, "984 NFC \"(ˣ; ˣ; ˣ; x; x; ) MODIFIER LETTER SMALL X\"");

    err = nfd_normaliser("\xCB\xA4", strlen("\xCB\xA4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\xA4", "\xCB\xA4", out_buffer, "985 NFD \"(ˤ; ˤ; ˤ; ʕ; ʕ; ) MODIFIER LETTER SMALL REVERSED GLOTTAL STOP\"");

    err = nfkd_normaliser("\xCB\xA4", strlen("\xCB\xA4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\xA4", "\xCA\x95", out_buffer, "986 NFKD \"(ˤ; ˤ; ˤ; ʕ; ʕ; ) MODIFIER LETTER SMALL REVERSED GLOTTAL STOP\"");

    err = nfc_normaliser("\xCB\xA4", strlen("\xCB\xA4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCB\xA4", "\xCB\xA4", out_buffer, "987 NFC \"(ˤ; ˤ; ˤ; ʕ; ʕ; ) MODIFIER LETTER SMALL REVERSED GLOTTAL STOP\"");

    err = nfd_normaliser("\xCD\x80", strlen("\xCD\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCD\x80", "\xCC\x80", out_buffer, "988 NFD \"(◌̀; ◌̀; ◌̀; ◌̀; ◌̀; ) COMBINING GRAVE TONE MARK\"");

    err = nfkd_normaliser("\xCD\x80", strlen("\xCD\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCD\x80", "\xCC\x80", out_buffer, "989 NFKD \"(◌̀; ◌̀; ◌̀; ◌̀; ◌̀; ) COMBINING GRAVE TONE MARK\"");

    err = nfc_normaliser("\xCD\x80", strlen("\xCD\x80"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCD\x80", "\xCC\x80", out_buffer, "990 NFC \"(◌̀; ◌̀; ◌̀; ◌̀; ◌̀; ) COMBINING GRAVE TONE MARK\"");

    err = nfd_normaliser("\xCD\x81", strlen("\xCD\x81"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCD\x81", "\xCC\x81", out_buffer, "991 NFD \"(◌́; ◌́; ◌́; ◌́; ◌́; ) COMBINING ACUTE TONE MARK\"");

    err = nfkd_normaliser("\xCD\x81", strlen("\xCD\x81"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCD\x81", "\xCC\x81", out_buffer, "992 NFKD \"(◌́; ◌́; ◌́; ◌́; ◌́; ) COMBINING ACUTE TONE MARK\"");

    err = nfc_normaliser("\xCD\x81", strlen("\xCD\x81"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCD\x81", "\xCC\x81", out_buffer, "993 NFC \"(◌́; ◌́; ◌́; ◌́; ◌́; ) COMBINING ACUTE TONE MARK\"");

    err = nfd_normaliser("\xCD\x83", strlen("\xCD\x83"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCD\x83", "\xCC\x93", out_buffer, "994 NFD \"(◌̓; ◌̓; ◌̓; ◌̓; ◌̓; ) COMBINING GREEK KORONIS\"");

    err = nfkd_normaliser("\xCD\x83", strlen("\xCD\x83"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCD\x83", "\xCC\x93", out_buffer, "995 NFKD \"(◌̓; ◌̓; ◌̓; ◌̓; ◌̓; ) COMBINING GREEK KORONIS\"");

    err = nfc_normaliser("\xCD\x83", strlen("\xCD\x83"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCD\x83", "\xCC\x93", out_buffer, "996 NFC \"(◌̓; ◌̓; ◌̓; ◌̓; ◌̓; ) COMBINING GREEK KORONIS\"");

    err = nfd_normaliser("\xCD\x84", strlen("\xCD\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCD\x84", "\xCC\x88" "\xCC\x81", out_buffer, "997 NFD \"(◌̈́; ◌̈◌́; ◌̈◌́; ◌̈◌́; ◌̈◌́; ) COMBINING GREEK DIALYTIKA TONOS\"");

    err = nfkd_normaliser("\xCD\x84", strlen("\xCD\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCD\x84", "\xCC\x88" "\xCC\x81", out_buffer, "998 NFKD \"(◌̈́; ◌̈◌́; ◌̈◌́; ◌̈◌́; ◌̈◌́; ) COMBINING GREEK DIALYTIKA TONOS\"");

    err = nfc_normaliser("\xCD\x84", strlen("\xCD\x84"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCD\x84", "\xCC\x88" "\xCC\x81", out_buffer, "999 NFC \"(◌̈́; ◌̈◌́; ◌̈◌́; ◌̈◌́; ◌̈◌́; ) COMBINING GREEK DIALYTIKA TONOS\"");

    err = nfd_normaliser("\xCD\xB4", strlen("\xCD\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCD\xB4", "\xCA\xB9", out_buffer, "1000 NFD \"(ʹ; ʹ; ʹ; ʹ; ʹ; ) GREEK NUMERAL SIGN\"");

    err = nfkd_normaliser("\xCD\xB4", strlen("\xCD\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCD\xB4", "\xCA\xB9", out_buffer, "1001 NFKD \"(ʹ; ʹ; ʹ; ʹ; ʹ; ) GREEK NUMERAL SIGN\"");

    err = nfc_normaliser("\xCD\xB4", strlen("\xCD\xB4"), out_buffer, &out_len); out_buffer[out_len] = 0;
    checkresult(err, "\xCD\xB4", "\xCA\xB9", out_buffer, "1002 NFC \"(ʹ; ʹ; ʹ; ʹ; ʹ; ) GREEK NUMERAL SIGN\"");

    printf("1002 tests run\n");
    if (errors) printf("*** %d ERRORS ***\n", errors);
    else printf("*** NO ERRORS ***\n");
    return errors;
}

