/*
 * macOS CoreFoundation NFD throughput sense-check.
 *
 * This is a native-platform control, not a CREXX implementation or an
 * equal-representation comparator. Source and expected CFStrings are built
 * before timing. Each timed operation makes a mutable copy, normalizes it to
 * Form D, materializes the complete result as UTF-8, observes that UTF-8
 * output, and releases the result.
 */

#include <CoreFoundation/CoreFoundation.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

struct Row {
    CFStringRef source;
    CFStringRef expected;
};

struct CorrectnessResult {
    std::uint64_t checksum_scalars = 0;
    std::uint64_t checksum_bytes = 0;
    std::size_t mismatches = 0;
    std::size_t first_mismatch_row = 0;
    std::string first_source;
    std::string first_expected;
    std::string first_actual;
};

struct PassResult {
    std::uint64_t checksum_scalars = 0;
    std::uint64_t checksum_bytes = 0;
};

std::string trim(const std::string &text) {
    const std::string whitespace = " \t\r\n";
    const std::size_t first = text.find_first_not_of(whitespace);
    if (first == std::string::npos) {
        return {};
    }
    const std::size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}

std::vector<std::string> split_semicolon(const std::string &line) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    while (true) {
        const std::size_t separator = line.find(';', start);
        if (separator == std::string::npos) {
            fields.push_back(line.substr(start));
            return fields;
        }
        fields.push_back(line.substr(start, separator - start));
        start = separator + 1;
    }
}

CFStringRef string_from_hex(const std::string &sequence) {
    std::istringstream words(sequence);
    std::vector<UInt32> codepoints;
    std::string word;
    while (words >> word) {
        std::size_t consumed = 0;
        const unsigned long value = std::stoul(word, &consumed, 16);
        if (consumed != word.size() || value > 0x10FFFFUL ||
            (value >= 0xD800UL && value <= 0xDFFFUL)) {
            throw std::runtime_error("invalid Unicode scalar in normalization corpus");
        }
        codepoints.push_back(static_cast<UInt32>(value));
    }
    if (codepoints.empty()) {
        throw std::runtime_error("empty normalization field");
    }
    CFStringRef result = CFStringCreateWithBytes(
        kCFAllocatorDefault,
        reinterpret_cast<const UInt8 *>(codepoints.data()),
        static_cast<CFIndex>(codepoints.size() * sizeof(UInt32)),
        kCFStringEncodingUTF32LE,
        false);
    if (result == nullptr) {
        throw std::runtime_error("CoreFoundation rejected normalization scalar sequence");
    }
    return result;
}

std::vector<Row> load_rows(const std::string &path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("cannot open NormalizationTest.txt");
    }
    std::vector<Row> rows;
    std::string raw;
    std::size_t line_number = 0;
    while (std::getline(input, raw)) {
        ++line_number;
        const std::string line = trim(raw);
        if (line.empty() || line.front() == '#' || line.front() == '@') {
            continue;
        }
        const std::vector<std::string> fields = split_semicolon(line);
        if (fields.size() < 3) {
            throw std::runtime_error("invalid normalization row at line " +
                                     std::to_string(line_number));
        }
        Row row{};
        row.source = string_from_hex(trim(fields[0]));
        try {
            row.expected = string_from_hex(trim(fields[2]));
        } catch (...) {
            CFRelease(row.source);
            throw;
        }
        rows.push_back(row);
    }
    if (rows.size() != 20034) {
        throw std::runtime_error("unexpected normalization row count");
    }
    return rows;
}

PassResult observe_utf8(CFStringRef value) {
    const CFIndex character_count = CFStringGetLength(value);
    const CFIndex capacity =
        CFStringGetMaximumSizeForEncoding(character_count, kCFStringEncodingUTF8);
    if (capacity < 0) {
        throw std::runtime_error("cannot size CoreFoundation UTF-8 output");
    }
    std::vector<UInt8> bytes(static_cast<std::size_t>(std::max<CFIndex>(capacity, 1)));
    CFIndex used = 0;
    const CFIndex converted = CFStringGetBytes(
        value, CFRangeMake(0, character_count), kCFStringEncodingUTF8, 0,
        false, bytes.data(), capacity, &used);
    if (converted != character_count) {
        throw std::runtime_error("cannot materialize CoreFoundation UTF-8 output");
    }

    PassResult result;
    result.checksum_bytes = static_cast<std::uint64_t>(used);
    for (CFIndex index = 0; index < used; ++index) {
        if ((bytes[static_cast<std::size_t>(index)] & 0xC0U) != 0x80U) {
            ++result.checksum_scalars;
        }
    }
    return result;
}

PassResult normalize_pass(const std::vector<Row> &rows) {
    PassResult checksum;
    for (const Row &row : rows) {
        CFMutableStringRef actual =
            CFStringCreateMutableCopy(kCFAllocatorDefault, 0, row.source);
        if (actual == nullptr) {
            throw std::runtime_error("CFStringCreateMutableCopy failed");
        }
        CFStringNormalize(actual, kCFStringNormalizationFormD);
        const PassResult observed = observe_utf8(actual);
        checksum.checksum_scalars += observed.checksum_scalars;
        checksum.checksum_bytes += observed.checksum_bytes;
        CFRelease(actual);
    }
    return checksum;
}

std::string hex_sequence(CFStringRef value) {
    const CFIndex character_count = CFStringGetLength(value);
    const CFIndex capacity =
        CFStringGetMaximumSizeForEncoding(character_count, kCFStringEncodingUTF32LE);
    std::vector<UInt8> bytes(static_cast<std::size_t>(capacity));
    CFIndex used = 0;
    const CFIndex converted = CFStringGetBytes(
        value, CFRangeMake(0, character_count), kCFStringEncodingUTF32LE, 0,
        false, bytes.data(), capacity, &used);
    if (converted != character_count || (used % 4) != 0) {
        throw std::runtime_error("cannot render CoreFoundation mismatch as UTF-32");
    }
    std::ostringstream output;
    output << std::uppercase << std::hex << std::setfill('0');
    for (CFIndex offset = 0; offset < used; offset += 4) {
        UInt32 codepoint = 0;
        std::memcpy(&codepoint, bytes.data() + offset, sizeof(codepoint));
        if (offset != 0) {
            output << '-';
        }
        output << std::setw(codepoint > 0xFFFFU ? 6 : 4) << codepoint;
    }
    return output.str();
}

CorrectnessResult correctness_pass(const std::vector<Row> &rows) {
    CorrectnessResult result;
    for (std::size_t index = 0; index < rows.size(); ++index) {
        CFMutableStringRef actual =
            CFStringCreateMutableCopy(kCFAllocatorDefault, 0, rows[index].source);
        if (actual == nullptr) {
            throw std::runtime_error("CFStringCreateMutableCopy failed");
        }
        CFStringNormalize(actual, kCFStringNormalizationFormD);
        if (!CFEqual(actual, rows[index].expected)) {
            ++result.mismatches;
            if (result.first_mismatch_row == 0) {
                result.first_mismatch_row = index + 1;
                result.first_source = hex_sequence(rows[index].source);
                result.first_expected = hex_sequence(rows[index].expected);
                result.first_actual = hex_sequence(actual);
            }
        }
        const PassResult observed = observe_utf8(actual);
        result.checksum_scalars += observed.checksum_scalars;
        result.checksum_bytes += observed.checksum_bytes;
        CFRelease(actual);
    }
    return result;
}

std::int64_t median_us(std::vector<std::int64_t> samples) {
    std::sort(samples.begin(), samples.end());
    const std::size_t middle = samples.size() / 2;
    if ((samples.size() & 1U) != 0U) {
        return samples[middle];
    }
    return (samples[middle - 1] + samples[middle]) / 2;
}

}  // namespace

int main(int argc, char **argv) {
    if (argc != 4) {
        std::cerr << "usage: apple_corefoundation_nfd NormalizationTest.txt warmups samples\n";
        return 2;
    }
    try {
        const int warmups = std::stoi(argv[2]);
        const int sample_count = std::stoi(argv[3]);
        if (warmups < 0 || sample_count < 1) {
            throw std::runtime_error("invalid warmup or sample count");
        }

        const auto prepare_started = std::chrono::steady_clock::now();
        std::vector<Row> rows = load_rows(argv[1]);
        const auto prepare_finished = std::chrono::steady_clock::now();
        const auto prepare_us = std::chrono::duration_cast<std::chrono::microseconds>(
                                    prepare_finished - prepare_started)
                                    .count();

        const CorrectnessResult correctness = correctness_pass(rows);
        std::cout << "benchmark=unicode_nfd phase=corpus_prepare"
                  << " variant=corefoundation-utf8 vm=apple-corefoundation"
                  << " rows=" << rows.size() << " elapsed_us=" << prepare_us
                  << " table_bytes=system int_bytes=" << sizeof(std::intptr_t) << '\n';
        std::cout << "benchmark=unicode_nfd phase=correctness"
                  << " variant=corefoundation-utf8 vm=apple-corefoundation"
                  << " rows=" << rows.size()
                  << " unicode17_mismatches=" << correctness.mismatches;
        if (correctness.mismatches != 0) {
            std::cout << " first_mismatch_row=" << correctness.first_mismatch_row
                      << " first_source=" << correctness.first_source
                      << " first_expected=" << correctness.first_expected
                      << " first_actual=" << correctness.first_actual;
        }
        std::cout << '\n';

        for (int warmup = 0; warmup < warmups; ++warmup) {
            const PassResult actual = normalize_pass(rows);
            if (actual.checksum_scalars != correctness.checksum_scalars ||
                actual.checksum_bytes != correctness.checksum_bytes) {
                throw std::runtime_error("CoreFoundation warmup checksum mismatch");
            }
        }

        std::vector<std::int64_t> samples;
        samples.reserve(static_cast<std::size_t>(sample_count));
        for (int sample = 1; sample <= sample_count; ++sample) {
            const auto started = std::chrono::steady_clock::now();
            const PassResult actual_checksum = normalize_pass(rows);
            const auto finished = std::chrono::steady_clock::now();
            const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                                     finished - started)
                                     .count();
            if (actual_checksum.checksum_scalars != correctness.checksum_scalars ||
                actual_checksum.checksum_bytes != correctness.checksum_bytes) {
                throw std::runtime_error("CoreFoundation sample checksum mismatch");
            }
            samples.push_back(elapsed);
            std::cout << "benchmark=unicode_nfd phase=normalize"
                      << " variant=corefoundation-utf8 vm=apple-corefoundation"
                      << " sample=" << sample << " rows=" << rows.size()
                      << " elapsed_us=" << elapsed
                      << " checksum_utf8_scalars=" << actual_checksum.checksum_scalars
                      << " checksum_utf8_bytes=" << actual_checksum.checksum_bytes << '\n';
        }
        std::cout << "benchmark=unicode_nfd phase=summary"
                  << " variant=corefoundation-utf8 vm=apple-corefoundation"
                  << " warmups=" << warmups << " samples=" << sample_count
                  << " rows=" << rows.size() << " median_us=" << median_us(samples)
                  << " checksum_utf8_scalars=" << correctness.checksum_scalars
                  << " checksum_utf8_bytes=" << correctness.checksum_bytes
                  << " unicode17_mismatches=" << correctness.mismatches << '\n';
        std::cout << "PASS: Apple CoreFoundation Unicode NFD timing cell"
                  << " (Unicode 17 mismatches=" << correctness.mismatches << ")\n";

        for (const Row &row : rows) {
            CFRelease(row.source);
            CFRelease(row.expected);
        }
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
