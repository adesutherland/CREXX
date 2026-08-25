/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2026 Adrian Sutherland
 *
 * Phase-2 Unicode NFD reference proof. This is an experimental conformance
 * oracle, not a product Unicode implementation or a proposed packed layout.
 */

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

using CodePoint = uint32_t;
using Sequence = std::vector<CodePoint>;

constexpr CodePoint MAX_CODE_POINT = 0x10FFFF;
constexpr size_t CODE_POINT_COUNT = static_cast<size_t>(MAX_CODE_POINT) + 1;
constexpr CodePoint HANGUL_S_BASE = 0xAC00;
constexpr CodePoint HANGUL_L_BASE = 0x1100;
constexpr CodePoint HANGUL_V_BASE = 0x1161;
constexpr CodePoint HANGUL_T_BASE = 0x11A7;
constexpr CodePoint HANGUL_L_COUNT = 19;
constexpr CodePoint HANGUL_V_COUNT = 21;
constexpr CodePoint HANGUL_T_COUNT = 28;
constexpr CodePoint HANGUL_N_COUNT = HANGUL_V_COUNT * HANGUL_T_COUNT;
constexpr CodePoint HANGUL_S_COUNT = HANGUL_L_COUNT * HANGUL_N_COUNT;

bool is_scalar(CodePoint cp) {
    return cp <= MAX_CODE_POINT && !(cp >= 0xD800 && cp <= 0xDFFF);
}

bool is_hex(uint8_t c) {
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
}

uint32_t hex_digit(uint8_t c) {
    return c <= '9' ? static_cast<uint32_t>(c - '0')
                    : static_cast<uint32_t>(c - 'A' + 10);
}

uint32_t parse_hex(const uint8_t* first, const uint8_t* last) {
    if (first == last) throw std::runtime_error("empty hexadecimal field");
    uint32_t value = 0;
    for (const uint8_t* p = first; p != last; ++p) {
        if (!is_hex(*p)) throw std::runtime_error("invalid hexadecimal field");
        value = value * 16 + hex_digit(*p);
        if (value > MAX_CODE_POINT) {
            throw std::runtime_error("code point exceeds U+10FFFF");
        }
    }
    return value;
}

uint32_t parse_decimal(const uint8_t* first, const uint8_t* last) {
    if (first == last) throw std::runtime_error("empty decimal field");
    uint32_t value = 0;
    for (const uint8_t* p = first; p != last; ++p) {
        if (*p < '0' || *p > '9') {
            throw std::runtime_error("invalid decimal field");
        }
        value = value * 10 + static_cast<uint32_t>(*p - '0');
    }
    return value;
}

Sequence parse_sequence(const uint8_t* first, const uint8_t* last) {
    Sequence result;
    const uint8_t* p = first;
    while (p != last) {
        while (p != last && (*p == ' ' || *p == '\t')) ++p;
        const uint8_t* token = p;
        while (p != last && is_hex(*p)) ++p;
        if (token == p) throw std::runtime_error("expected code point sequence");
        CodePoint cp = parse_hex(token, p);
        if (!is_scalar(cp)) throw std::runtime_error("sequence contains a non-scalar value");
        result.push_back(cp);
        while (p != last && (*p == ' ' || *p == '\t')) ++p;
        if (p != last && !is_hex(*p)) {
            throw std::runtime_error("invalid character in code point sequence");
        }
    }
    if (result.empty()) throw std::runtime_error("empty code point sequence");
    return result;
}

struct Input {
    std::string path;
    std::vector<uint8_t> bytes;
    const uint8_t* cursor = nullptr;
    const uint8_t* marker = nullptr;
    const uint8_t* end = nullptr;
    size_t line = 1;

    explicit Input(const std::string& input_path) : path(input_path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) throw std::runtime_error("cannot open " + path);
        file.seekg(0, std::ios::end);
        const std::streamoff length = file.tellg();
        if (length < 0) throw std::runtime_error("cannot size " + path);
        file.seekg(0, std::ios::beg);
        bytes.resize(static_cast<size_t>(length) + 1);
        if (length != 0) {
            file.read(reinterpret_cast<char*>(bytes.data()), length);
            if (!file) throw std::runtime_error("cannot read " + path);
        }
        bytes.back() = 0;
        cursor = bytes.data();
        end = bytes.data() + static_cast<size_t>(length);
    }

    [[noreturn]] void fail(const std::string& message) const {
        std::ostringstream out;
        out << path << ':' << line << ": " << message;
        throw std::runtime_error(out.str());
    }
};

struct Rules {
    std::string version;
    std::vector<uint8_t> ccc = std::vector<uint8_t>(CODE_POINT_COUNT, 0);
    std::vector<uint8_t> ccc_seen = std::vector<uint8_t>(CODE_POINT_COUNT, 0);
    std::unordered_map<CodePoint, Sequence> decomposition;
    size_t ccc_records = 0;
    size_t ccc_code_points = 0;
    size_t two_way_mappings = 0;
    size_t one_way_mappings = 0;
    size_t mapping_values = 0;

    void add_ccc(CodePoint first, CodePoint last, uint32_t value) {
        if (!is_scalar(first) || !is_scalar(last) || last < first) {
            throw std::runtime_error("invalid canonical-combining-class range");
        }
        if (value == 0 || value > 255) {
            throw std::runtime_error("canonical combining class must be 1..255");
        }
        for (CodePoint cp = first; cp <= last; ++cp) {
            if (!is_scalar(cp)) {
                throw std::runtime_error("canonical-combining-class range contains a surrogate");
            }
            if (ccc_seen[cp]) {
                throw std::runtime_error("overlapping canonical-combining-class range");
            }
            ccc_seen[cp] = 1;
            ccc[cp] = static_cast<uint8_t>(value);
            ++ccc_code_points;
        }
        ++ccc_records;
    }

    void add_mapping(CodePoint source, Sequence mapping, bool two_way) {
        if (!is_scalar(source)) throw std::runtime_error("mapping source is not a scalar value");
        if (two_way && mapping.size() != 2) {
            throw std::runtime_error("two-way mapping must contain exactly two code points");
        }
        if (!decomposition.emplace(source, mapping).second) {
            throw std::runtime_error("duplicate decomposition mapping");
        }
        mapping_values += mapping.size();
        if (two_way) {
            ++two_way_mappings;
        } else {
            ++one_way_mappings;
        }
    }
};

Rules parse_gennorm2(const std::string& path) {
    Input in(path);
    Rules rules;
    const uint8_t *a, *b, *c, *d, *e, *f;
    /*!stags:re2c format = "[[maybe_unused]] const uint8_t* @@;"; */

scan:
/*!re2c
        re2c:yyfill:enable = 0;
        re2c:YYCTYPE = uint8_t;
        re2c:YYCURSOR = in.cursor;
        re2c:YYMARKER = in.marker;
        re2c:tags = 1;

        hex = [0-9A-F]+;
        dec = [0-9]+;
        h = [ \t]*;
        eol = "\r"? "\n";
        tail = h ("#" [^\x00\r\n]*)? eol;
        sequence = hex ([ \t]+ hex)*;

        "\x00" {
            if (in.cursor != in.end + 1) in.fail("embedded NUL in text input");
            goto done;
        }
        h ("#" [^\x00\r\n]*)? eol {
            ++in.line;
            goto scan;
        }
        h "*" [ \t]+ "Unicode" [ \t]+ @a [0-9.]+ @b tail {
            if (!rules.version.empty()) in.fail("duplicate Unicode version declaration");
            rules.version.assign(reinterpret_cast<const char*>(a),
                                 reinterpret_cast<const char*>(b));
            if (rules.version != "17.0.0") in.fail("expected Unicode version 17.0.0");
            ++in.line;
            goto scan;
        }
        h @a hex @b ".." @c hex @d h ":" h @e dec @f tail {
            try {
                rules.add_ccc(parse_hex(a, b), parse_hex(c, d), parse_decimal(e, f));
            } catch (const std::exception& error) {
                in.fail(error.what());
            }
            ++in.line;
            goto scan;
        }
        h @a hex @b h ":" h @c dec @d tail {
            try {
                const CodePoint cp = parse_hex(a, b);
                rules.add_ccc(cp, cp, parse_decimal(c, d));
            } catch (const std::exception& error) {
                in.fail(error.what());
            }
            ++in.line;
            goto scan;
        }
        h @a hex @b h "=" h @c sequence @d tail {
            try {
                rules.add_mapping(parse_hex(a, b), parse_sequence(c, d), true);
            } catch (const std::exception& error) {
                in.fail(error.what());
            }
            ++in.line;
            goto scan;
        }
        h @a hex @b h ">" h @c sequence @d tail {
            try {
                rules.add_mapping(parse_hex(a, b), parse_sequence(c, d), false);
            } catch (const std::exception& error) {
                in.fail(error.what());
            }
            ++in.line;
            goto scan;
        }
        * {
            in.fail("unrecognized gennorm2 input");
        }
    */

done:
    if (rules.version.empty()) in.fail("missing Unicode version declaration");
    if (rules.decomposition.empty() || rules.ccc_records == 0) {
        in.fail("incomplete normalization rules");
    }
    return rules;
}

class NfdNormalizer {
public:
    explicit NfdNormalizer(const Rules& rules) : rules_(rules) {}

    Sequence normalize(const Sequence& source) const {
        Sequence result;
        result.reserve(source.size());
        Sequence path;
        for (CodePoint cp : source) decompose(cp, result, path);
        canonical_order(result);
        return result;
    }

private:
    const Rules& rules_;

    void decompose(CodePoint cp, Sequence& output, Sequence& path) const {
        if (!is_scalar(cp)) throw std::runtime_error("normalization input is not a scalar value");

        const CodePoint s_index = cp - HANGUL_S_BASE;
        if (s_index < HANGUL_S_COUNT) {
            output.push_back(HANGUL_L_BASE + s_index / HANGUL_N_COUNT);
            output.push_back(HANGUL_V_BASE + (s_index % HANGUL_N_COUNT) / HANGUL_T_COUNT);
            const CodePoint t_index = s_index % HANGUL_T_COUNT;
            if (t_index != 0) output.push_back(HANGUL_T_BASE + t_index);
            return;
        }

        const auto mapping = rules_.decomposition.find(cp);
        if (mapping == rules_.decomposition.end()) {
            output.push_back(cp);
            return;
        }
        if (path.size() >= 64 || std::find(path.begin(), path.end(), cp) != path.end()) {
            throw std::runtime_error("cyclic or excessively deep decomposition mapping");
        }
        path.push_back(cp);
        for (CodePoint mapped : mapping->second) decompose(mapped, output, path);
        path.pop_back();
    }

    void canonical_order(Sequence& sequence) const {
        for (size_t i = 1; i < sequence.size(); ++i) {
            const uint8_t current_ccc = rules_.ccc[sequence[i]];
            if (current_ccc == 0) continue;
            size_t j = i;
            while (j != 0) {
                const uint8_t previous_ccc = rules_.ccc[sequence[j - 1]];
                if (previous_ccc == 0 || previous_ccc <= current_ccc) break;
                std::swap(sequence[j], sequence[j - 1]);
                --j;
            }
        }
    }
};

std::string format_sequence(const Sequence& sequence) {
    std::ostringstream out;
    out << std::uppercase << std::hex << std::setfill('0');
    for (size_t i = 0; i < sequence.size(); ++i) {
        if (i != 0) out << ' ';
        out << std::setw(sequence[i] <= 0xFFFF ? 4 : 6) << sequence[i];
    }
    return out.str();
}

struct ConformanceResult {
    std::array<size_t, 6> part_rows{};
    size_t rows = 0;
    size_t invariant_checks = 0;
    size_t unlisted_identity_checks = 0;
};

void require_equal(const char* relation, const Sequence& actual,
                   const Sequence& expected) {
    if (actual == expected) return;
    std::ostringstream message;
    message << relation << " failed: expected [" << format_sequence(expected)
            << "] but got [" << format_sequence(actual) << ']';
    throw std::runtime_error(message.str());
}

ConformanceResult run_normalization_test(const std::string& path,
                                         const NfdNormalizer& normalizer) {
    Input in(path);
    ConformanceResult result;
    std::vector<uint8_t> part1_sources(CODE_POINT_COUNT, 0);
    int part = -1;
    const uint8_t *a, *b, *c, *d, *e, *f, *g, *h, *i, *j;
    /*!stags:re2c format = "[[maybe_unused]] const uint8_t* @@;"; */

scan:
/*!re2c
        re2c:yyfill:enable = 0;
        re2c:YYCTYPE = uint8_t;
        re2c:YYCURSOR = in.cursor;
        re2c:YYMARKER = in.marker;
        re2c:tags = 1;

        test_hex = [0-9A-F]+;
        test_dec = [0-9]+;
        test_ws = [ \t]*;
        test_field = test_hex ([ \t]+ test_hex)*;
        test_eol = "\r"? "\n";
        test_tail = test_ws ("#" [^\x00\r\n]*)? test_eol;

        "\x00" {
            if (in.cursor != in.end + 1) in.fail("embedded NUL in text input");
            goto done;
        }
        test_ws ("#" [^\x00\r\n]*)? test_eol {
            ++in.line;
            goto scan;
        }
        test_ws "@Part" @a test_dec @b test_tail {
            const uint32_t parsed = parse_decimal(a, b);
            if (parsed > 5 || static_cast<int>(parsed) <= part) {
                in.fail("unexpected normalization-test part marker");
            }
            part = static_cast<int>(parsed);
            ++in.line;
            goto scan;
        }
        test_ws @a test_field @b test_ws ";" test_ws
           @c test_field @d test_ws ";" test_ws
           @e test_field @f test_ws ";" test_ws
           @g test_field @h test_ws ";" test_ws
           @i test_field @j test_ws ";" test_tail {
            if (part < 0 || part > 5) in.fail("test row precedes a valid part marker");
            try {
                const Sequence c1 = parse_sequence(a, b);
                const Sequence c2 = parse_sequence(c, d);
                const Sequence c3 = parse_sequence(e, f);
                const Sequence c4 = parse_sequence(g, h);
                const Sequence c5 = parse_sequence(i, j);

                require_equal("toNFD(c1) == c3", normalizer.normalize(c1), c3);
                require_equal("toNFD(c2) == c3", normalizer.normalize(c2), c3);
                require_equal("toNFD(c3) == c3", normalizer.normalize(c3), c3);
                require_equal("toNFD(c4) == c5", normalizer.normalize(c4), c5);
                require_equal("toNFD(c5) == c5", normalizer.normalize(c5), c5);

                if (part == 1) {
                    if (c1.size() != 1) in.fail("Part 1 source is not one code point");
                    part1_sources[c1[0]] = 1;
                }
            } catch (const std::exception& error) {
                in.fail(error.what());
            }
            ++result.rows;
            ++result.part_rows[static_cast<size_t>(part)];
            result.invariant_checks += 5;
            ++in.line;
            goto scan;
        }
        * {
            in.fail("unrecognized NormalizationTest input");
        }
    */

done:
    if (part != 5) in.fail("normalization test ended before Part 5");

    for (CodePoint cp = 0; cp <= MAX_CODE_POINT; ++cp) {
        if (!is_scalar(cp) || part1_sources[cp]) continue;
        const Sequence singleton{cp};
        const Sequence actual = normalizer.normalize(singleton);
        if (actual != singleton) {
            std::ostringstream message;
            message << "unlisted scalar U+" << std::uppercase << std::hex
                    << std::setfill('0') << std::setw(cp <= 0xFFFF ? 4 : 6) << cp
                    << " is not an NFD identity";
            in.fail(message.str());
        }
        ++result.unlisted_identity_checks;
    }
    return result;
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "usage: nfd_reference ICU_NFC_RULES NORMALIZATION_TEST\n";
        return 2;
    }
    try {
        const Rules rules = parse_gennorm2(argv[1]);
        const NfdNormalizer normalizer(rules);
        const ConformanceResult result = run_normalization_test(argv[2], normalizer);

        std::cout << "Unicode version: " << rules.version << '\n';
        std::cout << "gennorm2 ccc records/code points: " << rules.ccc_records
                  << '/' << rules.ccc_code_points << '\n';
        std::cout << "gennorm2 two-way/one-way mappings: "
                  << rules.two_way_mappings << '/' << rules.one_way_mappings << '\n';
        std::cout << "gennorm2 mapping values: " << rules.mapping_values << '\n';
        std::cout << "NormalizationTest rows by Part 0..5: ";
        for (size_t n = 0; n < result.part_rows.size(); ++n) {
            if (n != 0) std::cout << '/';
            std::cout << result.part_rows[n];
        }
        std::cout << '\n';
        std::cout << "NFD corpus invariant checks: " << result.invariant_checks << '\n';
        std::cout << "Unlisted scalar identity checks: "
                  << result.unlisted_identity_checks << '\n';
        std::cout << "Result: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
