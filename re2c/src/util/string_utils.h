#ifndef _RE2C_UTIL_STRING_UTILS_
#define _RE2C_UTIL_STRING_UTILS_

#include <stdint.h>
#include <string.h>
#include <sstream>
#include <string>

#include "src/util/attribute.h"

namespace re2c {

uint32_t unesc_hex(const uint8_t* s, const uint8_t* s_end);
uint32_t unesc_oct(const uint8_t* s, const uint8_t* s_end);
std::string escape_backslashes(const std::string& str);
bool s_to_u32_unsafe(const uint8_t* s, const uint8_t* s_end, uint32_t& number) NODISCARD;
bool s_to_i32_unsafe(const uint8_t* s, const uint8_t* s_end, int32_t& number) NODISCARD;

template<typename type_t>
void strrreplace(std::string& s, const std::string& s1, const type_t& v) {
    std::ostringstream sv;
    sv << v;
    const std::string& s2 = sv.str();
    const size_t l1 = s1.length(), l2 = s2.length();
    if (l1 != 0) {
        std::string::size_type pos = s.find(s1);
        while (pos != std::string::npos) {
            s.replace(pos, l1, s2);
            pos = s.find(s1, pos + l2);
        }
    }
}

template<typename T>
std::string to_string(const T& v) {
    std::ostringstream s;
    s << v;
    return s.str();
}

inline std::string getstr(const uint8_t* s, const uint8_t* e) {
    return std::string(reinterpret_cast<const char*>(s), static_cast<size_t>(e - s));
}

inline std::string* newstr(const uint8_t* s, const uint8_t* e) {
    return new std::string(reinterpret_cast<const char*>(s), static_cast<size_t>(e - s));
}

template<typename allocator_t>
inline const char* newcstr(const uint8_t* s, const uint8_t* e, allocator_t& alc) {
    const size_t n = static_cast<size_t>(e - s);
    char* p = alc.template alloct<char>(n + 1);
    memcpy(p, s, n);
    p[n] = 0;
    return p;
}

template<typename allocator_t>
inline const char* copystr(const std::string& s, allocator_t& alc) {
    const size_t n = s.length() + 1;
    char* p = alc.template alloct<char>(n);
    memcpy(p, s.data(), n);
    return p;
}

inline constexpr uint8_t operator "" _u8(char c) noexcept
{
    return static_cast<uint8_t>(c);
}

} // namespace re2c

#endif // _RE2C_UTIL_STRING_UTILS_
