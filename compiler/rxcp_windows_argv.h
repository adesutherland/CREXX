/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#ifndef RXCP_WINDOWS_ARGV_H
#define RXCP_WINDOWS_ARGV_H
#include <errno.h>
#include <stdlib.h>
#include <wchar.h>

/* _wspawnv joins arguments with spaces; preserve one CRT argument per entry.
 * Backslashes are special only before a quote, including the closing quote. */
static wchar_t *rxcp_windows_quote_argument(const wchar_t *argument) {
    size_t length = wcslen(argument), slashes = 0;
    wchar_t *quoted, *out;
    if (length > (((size_t)-1 / sizeof(wchar_t)) - 3) / 2) {
        errno = ENOMEM;
        return NULL;
    }
    quoted = malloc((2 * length + 3) * sizeof(wchar_t));
    if (!quoted) return NULL;
    out = quoted;
    *out++ = L'"';
    while (*argument) {
        if (*argument == L'\\') {
            ++slashes;
        } else {
            size_t count = *argument == L'"' ? 2 * slashes + 1 : slashes;
            while (count--) *out++ = L'\\';
            *out++ = *argument;
            slashes = 0;
        }
        ++argument;
    }
    slashes *= 2;
    while (slashes--) *out++ = L'\\';
    *out++ = L'"';
    *out = 0;
    return quoted;
}
#endif
