/*
 * Experimental macOS CoreFoundation NFD control for the CREXX string ABI.
 *
 * This deliberately accepts and returns `.string` through RXPA so the timed
 * Level B caller pays the ordinary UTF-8 copy-out, native call, UTF-8
 * materialization, return copy/validation, and assignment path.  RXPA's
 * current string callbacks are NUL-terminated, so this control is valid for
 * NormalizationTest.txt (which contains no U+0000 input) but is not a proposed
 * general Unicode API.
 */

#include <CoreFoundation/CoreFoundation.h>

#include <cstdlib>
#include <cstring>

#include "crexxpa.h"

RXPA_PLUGIN_PROCESS_REENTRANT

static char procedure_name[] = "unicode_nfd_apple.normalize_utf8";
static char level_option[] = "b";
static char return_type[] = ".string";
static char arguments[] = "source = .string";

PROCEDURE(normalize_utf8)
{
    const char *input;
    CFStringRef source;
    CFMutableStringRef normalized;
    CFIndex utf16_units;
    CFIndex maximum_utf8_bytes;
    char *output;

    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "one UTF-8 string expected")

    input = GETSTRING(ARG(0));
    if (input == nullptr)
        RETURNSIGNAL(SIGNAL_FAILURE, "RXPA string copy-out failed")

    source = CFStringCreateWithBytes(
        kCFAllocatorDefault,
        reinterpret_cast<const UInt8 *>(input),
        static_cast<CFIndex>(std::strlen(input)),
        kCFStringEncodingUTF8,
        false);
    if (source == nullptr)
        RETURNSIGNAL(SIGNAL_UNICODE_ERROR, "CoreFoundation rejected UTF-8 input")

    normalized = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, source);
    CFRelease(source);
    if (normalized == nullptr)
        RETURNSIGNAL(SIGNAL_FAILURE, "CoreFoundation mutable string allocation failed")

    CFStringNormalize(normalized, kCFStringNormalizationFormD);
    utf16_units = CFStringGetLength(normalized);
    maximum_utf8_bytes = CFStringGetMaximumSizeForEncoding(
        utf16_units, kCFStringEncodingUTF8);
    if (maximum_utf8_bytes < 0) {
        CFRelease(normalized);
        RETURNSIGNAL(SIGNAL_FAILURE, "CoreFoundation UTF-8 size calculation failed")
    }

    output = static_cast<char *>(
        std::malloc(static_cast<size_t>(maximum_utf8_bytes) + 1u));
    if (output == nullptr) {
        CFRelease(normalized);
        RETURNSIGNAL(SIGNAL_FAILURE, "UTF-8 output allocation failed")
    }

    if (!CFStringGetCString(
            normalized, output, maximum_utf8_bytes + 1,
            kCFStringEncodingUTF8)) {
        std::free(output);
        CFRelease(normalized);
        RETURNSIGNAL(SIGNAL_UNICODE_ERROR, "CoreFoundation UTF-8 output failed")
    }

    SETSTRING(RETURN, output);
    std::free(output);
    CFRelease(normalized);
    RESETSIGNAL
}

LOADFUNCS
ADDPROC(normalize_utf8, procedure_name, level_option, return_type, arguments);
ENDLOADFUNCS
