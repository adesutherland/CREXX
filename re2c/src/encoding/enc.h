#ifndef _RE2C_RE_ENCODING_ENC_
#define _RE2C_RE_ENCODING_ENC_

#include "src/util/c99_stdint.h"


namespace re2c {

class Range;
class RangeMgr;

/*
 * note [encodings]
 *
 * Each encoding defines two concepts:
 *
 * 1) Code point -- abstract number, which represents single encoding symbol.
 *    E.g., Unicode defines code points in the range [0 - 0x10FFFF] , so each
 *    Unicode encoding must be capable of representing 0x110000 code points.
 *
 * 2) Code unit -- the smallest unit of memory, which is used in the encoded
 *    text. One or more code units can be needed to represent a single code
 *    point, depending on the encoding. For each encoding, all code points
 *    either are represented with equal number of code units (fixed-length
 *    encodings), or with variable number of code units (variable-length
 *    encodings).
 *
 * +----------+------------------+-----------------------+-----------------+----------------+
 * | encoding | code point range | code point size       | code unit range | code unit size |
 * +----------+------------------+-----------------------+-----------------+----------------+
 * | ASCII    | 0 - 0xFF         | fixed,        1 byte  | 0 - 0xFF        | 1 byte         |
 * | EBCDIC   | 0 - 0xFF         | fixed,        1 byte  | 0 - 0xFF        | 1 byte         |
 * | UCS2     | 0 - 0xFFFF       | fixed,        2 bytes | 0 - 0xFFFF      | 2 bytes        |
 * | UTF16    | 0 - 0x10FFFF     | variable, 2 - 4 bytes | 0 - 0xFFFF      | 2 bytes        |
 * | UTF32    | 0 - 0x10FFFF     | fixed,        4 bytes | 0 - 0x10FFFF    | 4 bytes        |
 * | UTF8     | 0 - 0x10FFFF     | variable, 1 - 4 bytes | 0 - 0xFF        | 1 byte         |
 * +----------+------------------+-----------------------+-----------------+----------------+
 */

class Enc
{
public:
    // Supported encodings.
    enum type_t
        { ASCII
        , EBCDIC
        , UCS2
        , UTF16
        , UTF32
        , UTF8
        };

    // What to do with invalid code points
    enum policy_t
        { POLICY_FAIL
        , POLICY_SUBSTITUTE
        , POLICY_IGNORE
        };

private:
    static const uint32_t SURR_MIN;
    static const uint32_t SURR_MAX;
    static const uint32_t UNICODE_ERROR;

    type_t type_;
    policy_t policy_;

public:
    Enc()
        : type_ (ASCII)
        , policy_ (POLICY_IGNORE)
    { }

    static const char * name (type_t t);

    bool operator != (const Enc & e) const { return type_ != e.type_; }

    inline uint32_t nCodePoints() const;
    inline uint32_t nCodeUnits() const;
    inline uint32_t szCodePoint() const;
    inline uint32_t szCodeUnit() const;
    inline bool multibyte_cunit() const;

    inline void set(type_t t);
    inline void unset(type_t);
    inline type_t type () const;

    inline void setPolicy(policy_t t);

    uint32_t decodeUnsafe(uint32_t c) const;
    bool validateChar(uint32_t & c) const;
    Range * validateRange(RangeMgr &rm, uint32_t l, uint32_t h) const;
    Range * fullRange(RangeMgr &rm) const;
};

inline const char * Enc::name (type_t t)
{
    switch (t) {
        case ASCII:  return "ASCII";
        case EBCDIC: return "EBCDIC";
        case UTF8:   return "UTF8";
        case UCS2:   return "USC2";
        case UTF16:  return "UTF16";
        case UTF32:  return "UTF32";
    }
    return "<bad encoding>"; /* error */
}

inline uint32_t Enc::nCodePoints() const
{
    switch (type_) {
        case ASCII:
        case EBCDIC: return 0x100;
        case UCS2:   return 0x10000;
        case UTF16:
        case UTF32:
        case UTF8:   return 0x110000;
    }
    return 0; /* error */
}

inline uint32_t Enc::nCodeUnits() const
{
    switch (type_) {
        case ASCII:
        case EBCDIC:
        case UTF8:   return 0x100;
        case UCS2:
        case UTF16:  return 0x10000;
        case UTF32:  return 0x110000;
    }
    return 0; /* error */
}

// returns *maximal* code point size for encoding
inline uint32_t Enc::szCodePoint() const
{
    switch (type_) {
        case ASCII:
        case EBCDIC: return 1;
        case UCS2:   return 2;
        case UTF16:
        case UTF32:
        case UTF8:   return 4;
    }
    return 0; /* error */
}

inline uint32_t Enc::szCodeUnit() const
{
    switch (type_) {
        case ASCII:
        case EBCDIC:
        case UTF8:   return 1;
        case UCS2:
        case UTF16:  return 2;
        case UTF32:  return 4;
    }
    return 0; /* error */
}

inline bool Enc::multibyte_cunit() const
{
    switch (type_) {
        case Enc::UCS2:
        case Enc::UTF16:
        case Enc::UTF32:
            return true;
        case Enc::ASCII:
        case Enc::EBCDIC:
        case Enc::UTF8:
            break;
    }
    return false;
}

inline void Enc::set(type_t t)
{
    type_ = t;
}

inline void Enc::unset(type_t t)
{
    if (type_ == t)
        type_ = ASCII;
}

inline Enc::type_t Enc::type () const
{
    return type_;
}

inline void Enc::setPolicy(policy_t t)
{
    policy_ = t;
}

} // namespace re2c

#endif // _RE2C_RE_ENCODING_ENC_
