#include <charconv>
#include <cerrno>
#include <cmath>
#include <cstddef>
#include <system_error>

static bool ascii_space(unsigned char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
           c == '\f' || c == '\v';
}

extern "C" int bounded_from_chars_double(double *out,
                                            const char *text,
                                            size_t length) {
    const char *begin;
    const char *end;
    const char *parsed_begin;
    std::from_chars_result result;
    double value;
    bool explicit_plus = false;

    if (out == nullptr || text == nullptr) return 1;
    begin = text;
    end = text + length;
    while (begin != end && ascii_space(static_cast<unsigned char>(*begin))) {
        ++begin;
    }
    if (begin != end && *begin == '+') {
        explicit_plus = true;
        ++begin;
    }
    parsed_begin = begin;
    result = std::from_chars(begin, end, value, std::chars_format::general);
    if (result.ec != std::errc() || result.ptr == parsed_begin) return 1;
    begin = result.ptr;
    while (begin != end && ascii_space(static_cast<unsigned char>(*begin))) {
        ++begin;
    }
    if (begin != end) return 1;
    if (explicit_plus && std::signbit(value)) return 1;
    *out = value;
    return 0;
}
