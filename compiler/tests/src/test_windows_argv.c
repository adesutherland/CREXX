/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#include <stdio.h>
#include "rxcp_windows_argv.h"

int main(void) {
    static const struct { const wchar_t *input, *expected; } cases[] = {
        {L"", L"\"\""},
        {L"plain", L"\"plain\""},
        {L"two words\tthree", L"\"two words\tthree\""},
        {L"D:\\sources \x00c9\\alias.crexx", L"\"D:\\sources \x00c9\\alias.crexx\""},
        {L"D:\\root with spaces\\", L"\"D:\\root with spaces\\\\\""},
        {L"a\"b", L"\"a\\\"b\""},
        {L"a\\\"b", L"\"a\\\\\\\"b\""},
        {L"a\\\\\"b", L"\"a\\\\\\\\\\\"b\""},
        {L"\x53d8\x91cf \xd83d\xde42", L"\"\x53d8\x91cf \xd83d\xde42\""}
    };
    size_t i;
    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        wchar_t *quoted = rxcp_windows_quote_argument(cases[i].input);
        if (!quoted || wcscmp(quoted, cases[i].expected)) {
            fprintf(stderr, "Windows argv case %lu failed\n", (unsigned long)i);
            free(quoted);
            return 1;
        }
        free(quoted);
    }
    puts("Windows CRT argv boundaries pass");
    return 0;
}
