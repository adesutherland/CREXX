/* cREXX License (MIT). Complete-prefix validation for token-piece output. */
#pragma once
#include <cstddef>
#include <string>
// Return the complete valid prefix; retain a trailing incomplete scalar for the
// next token. Invalid byte sequences fail explicitly rather than changing output.
inline ptrdiff_t rxllama_utf8_prefix(const std::string &text) {
    size_t index = 0;
    while (index < text.size()) {
        auto start = index;
        auto first = static_cast<unsigned char>(text[index++]);
        if (first < 0x80) continue;
        unsigned count, scalar, minimum;
        if (first >= 0xc2 && first <= 0xdf) { count = 1; scalar = first & 31; minimum = 0x80; }
        else if (first >= 0xe0 && first <= 0xef) { count = 2; scalar = first & 15; minimum = 0x800; }
        else if (first >= 0xf0 && first <= 0xf4) { count = 3; scalar = first & 7; minimum = 0x10000; }
        else return -1;
        if (count > text.size() - index) return start;
        while (count--) {
            auto next = static_cast<unsigned char>(text[index++]);
            if ((next & 0xc0) != 0x80) return -1;
            scalar = (scalar << 6) | (next & 63);
        }
        if (scalar < minimum || scalar > 0x10ffff || (scalar >= 0xd800 && scalar <= 0xdfff)) return -1;
    }
    return index;
}
